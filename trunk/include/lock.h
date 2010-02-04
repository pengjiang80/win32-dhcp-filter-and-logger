#ifndef LOCK_H
#define LOCK_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct lock_lock {
	BOOL init;
	HANDLE lock;
} LOCK,*PLOCK;

extern BOOL lock_init(PLOCK lock,TCHAR* name);

extern BOOL lock_init_ex(PLOCK lock,HANDLE handle);

extern BOOL lock_wait(PLOCK lock,DWORD time);

extern BOOL lock_release(PLOCK lock);

extern VOID lock_destory(PLOCK lock);
#ifdef __cplusplus
}
#endif //__cplusplus
#endif //LOCK_H