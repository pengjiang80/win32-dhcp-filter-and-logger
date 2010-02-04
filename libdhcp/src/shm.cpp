#define _CRT_SECURE_NO_WARNINGS
#include <tchar.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <Aclapi.h>
#include <AccCtrl.h>
#include "shmdefs.h"

DWORD shm_enable_privileges(){
	HANDLE process = GetCurrentProcess();
	HANDLE token;
	if(!OpenProcessToken(process,TOKEN_ADJUST_PRIVILEGES,&token)){
		DEBUG_PRINT_LASTERROR("OpenProcessToken(): %s");
		return -1;
	}
	TOKEN_PRIVILEGES privileges;
	privileges.PrivilegeCount=1;
	privileges.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	LookupPrivilegeValue(NULL,SE_CREATE_GLOBAL_NAME,&privileges.Privileges[0].Luid);
	if(!AdjustTokenPrivileges(token,FALSE,&privileges,NULL,NULL,NULL)){
		DEBUG_PRINT_LASTERROR("AdjustTokenPrivileges(): %s");
		return -2;
	}
	return 0;
}

BOOL shm_create_security_attr(PSECURITY_ATTRIBUTES sa){
	DWORD dwRes;
    PSID pSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea[1];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

    // Create a SID for the BUILTIN\Administrators group.
    if(! AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pSID)) 
    {
		DEBUG_PRINT_LASTERROR("AllocateAndInitializeSid(): %s\n");
        goto Cleanup; 
    }

    // Initialize an EXPLICIT_ACCESS structure for an ACE.
    // The ACE will allow the Administrators group full access to
    // the object.
    ea[0].grfAccessPermissions = FILE_MAP_ALL_ACCESS|SYNCHRONIZE|MUTEX_ALL_ACCESS|EVENT_ALL_ACCESS;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pSID;

    // Create a new ACL that contains the new ACEs.
    dwRes = SetEntriesInAcl(1, ea, NULL, &pACL);
    if (ERROR_SUCCESS != dwRes) 
    {
		DEBUG_PRINT_LASTERROR("SetEntriesInAcl(): %s\n");
        goto Cleanup;
    }

    // Initialize a security descriptor.  
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (NULL == pSD) 
    { 
		DEBUG_PRINT_LASTERROR("LocalAlloc(): %s\n");
        goto Cleanup; 
    } 
 
    if (!InitializeSecurityDescriptor(pSD,SECURITY_DESCRIPTOR_REVISION)) 
    {  
		DEBUG_PRINT_LASTERROR("InitializeSecurityDescriptor(): %s\n");
        goto Cleanup; 
    } 
 
    // Add the ACL to the security descriptor. 
    if (!SetSecurityDescriptorDacl(pSD, 
            TRUE,     // bDaclPresent flag   
            pACL, 
            FALSE))   // not a default DACL 
    {  
		DEBUG_PRINT_LASTERROR("SetSecurityDescriptorDacl(): %s\n");
        goto Cleanup; 
    } 

    // Initialize a security attributes structure.
    sa->nLength = sizeof (SECURITY_ATTRIBUTES);
    sa->lpSecurityDescriptor = pSD;
    sa->bInheritHandle = FALSE;

	return TRUE;

Cleanup:

    if (pSID) 
        FreeSid(pSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD) 
        LocalFree(pSD);

    return FALSE;

}
SECURITY_ATTRIBUTES sattr;
DWORD shm_init(PSHM_BLOCK shm,_PSHM_LOCKDEF lockdef){
	DEBUG("shm_enable_privileges: %d\n",shm_enable_privileges());
	size_t size = shm->len + sizeof(_SHM_LOCKDEF);
	LPSECURITY_ATTRIBUTES psa = NULL;
	if(lockdef){
		BOOL r = shm_create_security_attr(&sattr);
		DEBUG("shm_create_security_attr: %d\n",r);
		if(r)psa = & sattr;
		DEBUG("CreateFileMapping: %s\n",shm->filename);
		shm->mapfile = CreateFileMapping(INVALID_HANDLE_VALUE,psa,PAGE_READWRITE,0,size,shm->filename);
	}else{
		DEBUG("OpenFileMapping: %s\n",shm->filename);
		shm->mapfile = OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,shm->filename);
	}
	if (!shm->mapfile){
		return SHM_MMAP_FAILED;
	}
	DEBUG("MapViewOfFile() ... ");
	shm->lockdef = (_PSHM_LOCKDEF)MapViewOfFile(shm->mapfile,FILE_MAP_ALL_ACCESS,0,0,size);
	if(!shm->lockdef)
		return SHM_MMAP_FAILED;
	DEBUG("0x%08X OK!\n",shm->lockdef);
	if(lockdef)
		memcpy(shm->lockdef,lockdef,sizeof(_SHM_LOCKDEF));
	shm->data = ((char*)shm->lockdef)+sizeof(_SHM_LOCKDEF);
	if(0!=_tcscmp(SHM_LOCK_NONE,shm->lockdef->lockname)){
		DEBUG("CreateMutex: %s\n",shm->lockdef->lockname);
		if(!lock_init_ex(&shm->lock,CreateMutexEx(psa,shm->lockdef->lockname,0,SYNCHRONIZE|MUTEX_ALL_ACCESS)))
			return SHM_LOCK_FAILED;
	}
	if(0!=_tcscmp(SHM_LOCK_NONE,shm->lockdef->esndname)){
		DEBUG("CreateEvent: %s\n",shm->lockdef->esndname);
		if(!(shm->esnd = CreateEventEx(psa,shm->lockdef->esndname,0,SYNCHRONIZE|EVENT_ALL_ACCESS)))
			return SHM_LOCK_FAILED;
	}
	if(0!=_tcscmp(SHM_LOCK_NONE,shm->lockdef->eackname)){
		DEBUG("CreateEvent: %s\n",shm->lockdef->eackname);
		if(!(shm->eack = CreateEventEx(psa,shm->lockdef->eackname,0,SYNCHRONIZE|EVENT_ALL_ACCESS)))
			return SHM_LOCK_FAILED;
	}
	DEBUG("shm_init: lock=0x%08x, esnd=0x%08x, eack=0x%08x",shm->lock.lock,shm->esnd,shm->eack);
	return SHM_OK;
}

/*
DWORD shm_reinit(PSHM_BLOCK shm){
	if(shm->mapfile)return SHM_OK;
	if(shm->master){
		shm->mapfile = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,shm->len,shm->filename);
	}else{
		shm->mapfile = OpenFileMapping(FILE_MAP_WRITE,FALSE,shm->filename);
	}
	if (!shm->mapfile){
		return SHM_MMAP_FAILED;
	}
	shm->block = MapViewOfFile(shm->mapfile,FILE_MAP_ALL_ACCESS,0,0,shm->len);
	return SHM_OK;
}
*/
DWORD shm_close(PSHM_BLOCK shm){
	assert(shm!=NULL);
	UnmapViewOfFile(shm->lockdef);
	CloseHandle(shm->mapfile);
	shm->mapfile = NULL;
	shm->lockdef = NULL;
	return 0;
}

BOOL shm_lock(PSHM_BLOCK shm, DWORD time){
	DEBUG("shm_lock: shm->lock.init=%d\n",shm->lock.init);
	if(shm->lock.init)
		return lock_wait(&shm->lock,time);
	else 
		return TRUE;
}

BOOL shm_unlock(PSHM_BLOCK shm){
	DEBUG("shm_unlock: shm->lock.init=%d\n",shm->lock.init);
	if(shm->lock.init)
		return lock_release(&shm->lock);
	else 
		return TRUE;
}

DWORD shm_write(PSHM_BLOCK shm,void* data,size_t len){
	if(!shm_lock(shm,2500)){
		DEBUG_PRINT_LASTERROR("shm_lock failed:%s");
		return SHM_LOCK_FAILED;
	}
	len = min(len,shm->len);
	memcpy(shm->data,data,len);
	shm_unlock(shm);
	if(shm->esnd){
		SetEvent(shm->esnd);
		DEBUG_PRINT_LASTERROR("SetEvent(): %s");
	}
	return len;
}

DWORD shm_write_or_skip(PSHM_BLOCK shm,void* data,size_t len){
	DWORD ret = shm_write(shm,data,len);
	if(FAILED(ret)||(WAIT_OBJECT_0 == WaitForSingleObject(shm->eack,25))){
		return ret;
	}else{
		return SHM_WRITE_TIMEOUT;
	}
}