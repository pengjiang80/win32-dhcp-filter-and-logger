// FakeClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "shmdefs.h"
#include "dhcp.h"

DWORD send_message(void*){
	SHM_BLOCK shm;
	ZeroMemory(&shm,sizeof(shm));
	shm.len = 2048;
	_tcscpy(shm.filename,SHM_CALLOUT_NAME);
	if(FAILED(shm_init_client(&shm))){
		DEBUG_PRINT_LASTERROR("Fail to open shared memory:%s");
		return -1;
	}
	
	while(1){
		if(FAILED(shm_write(&shm,"Hello world",sizeof("hello world")))){
			DEBUG_PRINT_LASTERROR("Write error: %s");
		}else{
			puts("Send Message");
		}
		Sleep(1000);
	}
	return 0;
}
DWORD threadproc(void*){
	while(1){
		send_message(NULL);
		Sleep(1000);
	}
	return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
	return threadproc(NULL);
}

