#include <windows.h>
#include <shmdefs.h>
#include <tchar.h>
DWORD shm_init(
	__in  size_t len,
	__in  BOOL master,
	__out PSHM_BLOCK block
)
{
	if(master){
		block->mapfile = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,len,SHM_FILE_NAME);
	}else{
		block->mapfile = OpenFileMapping(FILE_MAP_WRITE,FALSE,SHM_FILE_NAME);
	}
	if (!block->mapfile){
		return -1;
	}
	block->block = MapViewOfFile(block->mapfile,FILE_MAP_ALL_ACCESS,0,0,len);
	return 0;
}