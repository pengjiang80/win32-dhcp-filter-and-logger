#include "acl.h"
#include "shmdefs.h"
#include "dhcp.h"

SHM_BLOCK acl_shm;

PACL_LIST acl_init(){
	_SHM_LOCKDEF lockdef;
	ZeroMemory(&acl_shm,sizeof(acl_shm));
	acl_shm.len = 2048*sizeof(ACL_ENTRY)+sizeof(int);
	_tcscpy(acl_shm.filename,SHM_ACL_NAME);
	_tcscpy(lockdef.lockname,SHM_LOCK_NONE);
	_tcscpy(lockdef.eackname,SHM_LOCK_NONE);
	_tcscpy(lockdef.esndname,SHM_LOCK_NONE);
	if(FAILED(shm_init(&acl_shm,&lockdef))){
		DEBUG_PRINT_LASTERROR("[ACL]Fail to open shared memory: %s\n");
		return NULL;
	}
	PACL_LIST ret = (PACL_LIST)acl_shm.data;
	InitializeSRWLock(&ret->lock);
	return ret;
}

PACL_LIST acl_connect(){
	ZeroMemory(&acl_shm,sizeof(acl_shm));
	acl_shm.len = 2048*sizeof(ACL_ENTRY)+sizeof(int);
	_tcscpy(acl_shm.filename,SHM_ACL_NAME);
	if(FAILED(shm_init_client(&acl_shm))){
		DEBUG_PRINT_LASTERROR("[ACL]Fail to open shared memory: %s\n");
		return NULL;
	}
	PACL_LIST ret = (PACL_LIST)acl_shm.data;
	return ret;
}

VOID acl_write_lock(PACL_LIST acl){
	AcquireSRWLockExclusive(&acl->lock);
}

VOID acl_write_unlock(PACL_LIST acl){
	ReleaseSRWLockExclusive(&acl->lock);
}
int acl_compartor(const void* A,const void* B){
	PACL_ENTRY a = (PACL_ENTRY)A,b=(PACL_ENTRY)B;
	if(a->HwType!=b->HwType)
		return a->HwType-b->HwType;
	for(int i=0;i<16;i++){
		if(a->HwMask[i]!=b->HwMask[i])
			return (a->HwMask[i]-b->HwMask[i]);
	}
	for(int i=0;i<16;i++){
		if(a->HwAddr[i]!=b->HwAddr[i])
			return (a->HwAddr[i]-b->HwAddr[i]);
	}
	return 0;
}
//Need lock before call;
VOID acl_sort_entry(PACL_ENTRY entries,int count){
	qsort(entries,count,sizeof(ACL_ENTRY),acl_compartor);
}

BOOL acl_filter(PACL_LIST acl,const unsigned char HwType,const unsigned char* HwAddr,unsigned char HwAddrLen){
	AcquireSRWLockShared(&acl->lock);
	DEBUG("acl_filter: AcquireSRWLockShared()\n");
	__try{
		char buf[16*2+1];
		DEBUG("acl_filter(HwType=%d, HwAddr=%s)\n",(int)HwType,dhcp_hw_addr(buf,HwAddr,(int)HwAddrLen));
		unsigned int len = acl->length,i,j;
		PACL_ENTRY entries = acl->entry;
		for(i = 0;i<len;i++){
			DEBUG("acl_filter: entry[%d].HwType=%d arg.HwType=%d\n",i,(int)entries[i].HwType,(int)HwType);
			if(entries[i].HwType!=HwType)
				continue;
			
			for(j=0;j<HwAddrLen;j++){
				DEBUG("acl_filter: entry[%d].HwAddr[%d]=0x%02x arg.HwAddr[%d]=0x%02x\n",i,j,(int)entries[i].HwAddr[j],j,(int)HwAddr[j]);
				if((HwAddr[j]&entries[i].HwMask[j])!=entries[i].HwAddr[j]){
					goto next;
				}
			}
			return entries[i].access;
	next:	
			DEBUG("Goto Next\n");
			(void)0;
		}
		return TRUE;
	}__finally{
		ReleaseSRWLockShared(&acl->lock);
		DEBUG("acl_filter: ReleaseSRWLockShared()\n");
	}
}