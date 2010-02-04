#include <lock.h>
#include <debug.h>
BOOL lock_init(PLOCK lock,TCHAR* name){
	if(lock->lock = CreateMutex(NULL,FALSE,name)){
		lock->init = TRUE;
		return TRUE;
	}else
		return FALSE;
}
BOOL lock_init_ex(PLOCK lock,HANDLE handle){
	lock->lock = handle;
	lock->init = TRUE;
	return TRUE;
}
BOOL lock_wait(PLOCK lock,DWORD time){
	DEBUG("lock_wait: lock=%d lock->init=%d lock->lock=%d\n",lock,lock->init,lock->lock);
	if(lock&&lock->init){
		DWORD ret = WaitForSingleObject(lock->lock,time);
		DEBUG("WaitForSingleObject() => %d\n",ret);
		if(WAIT_OBJECT_0 != ret)
			DEBUG_PRINT_LASTERROR("WaitForSingleObject:%s\n");
		return WAIT_OBJECT_0 == ret;
	}else
		return FALSE;
}

BOOL lock_release(PLOCK lock){
	if(lock&&lock->init)
		return ReleaseMutex(lock->lock);
	else
		return FALSE;
}
VOID lock_destory(PLOCK lock){
	if(lock&&lock->init)CloseHandle(lock->lock);
}