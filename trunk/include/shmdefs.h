#ifndef SHMDEFS_H
#define SHMDEFS_H

#include "utils.h"
#include "lock.h"
#include <tchar.h>
#include <stdlib.h>
#include <debug.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHM_OK				 0
#define SHM_LOCK_FAILED		-1
#define SHM_MMAP_FAILED		-2
#define SHM_WRITE_TIMEOUT	-3
#define SHM_PEER_DEAD		-4

#define SHM_LOCK_NONE _T("")

typedef struct _shm_locknames{
	TCHAR lockname[256];
	TCHAR esndname[256];
	TCHAR eackname[256];
} _SHM_LOCKDEF, *_PSHM_LOCKDEF;

typedef struct shm_block{
	TCHAR filename[256];
	_PSHM_LOCKDEF lockdef;
	BOOL master;
	size_t len;
	HANDLE mapfile;
	LOCK lock;//Mutex
	HANDLE esnd;//Send Event
	HANDLE eack;//Ack Event
	void* data;
} SHM_BLOCK, *PSHM_BLOCK;

/**
 * Create a master if lockdef is not null
 **/
extern DWORD shm_init(PSHM_BLOCK shm,_PSHM_LOCKDEF lockdef);

//backport
#define shm_init_client(shm) shm_init(shm,NULL)
#define shm_init_server(shm,lockdef) shm_init(shm,lockdef)

extern DWORD shm_reinit(PSHM_BLOCK shm);
extern DWORD shm_close(PSHM_BLOCK shm);
extern BOOL  shm_lock(PSHM_BLOCK shm,DWORD time);
extern BOOL  shm_unlock(PSHM_BLOCK shm);
extern DWORD shm_write(PSHM_BLOCK shm,void* data,size_t len);
extern DWORD shm_write_or_skip(PSHM_BLOCK shm,void* data,size_t len);

#ifdef __cplusplus
}
#endif
#endif