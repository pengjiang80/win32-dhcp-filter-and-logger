#ifndef LINKEDBUFFER_H
#define LINKEDBUFFER_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct linked_buffer_entry{
	void* ptr;
	struct linked_buffer_entry* next;
}LINKED_BUFFER_ENTRY,*PLINKED_BUFFER_ENTRY;

typedef struct linkedbuffer{
	size_t size;
	size_t max_size;
	SRWLOCK lock;
	PLINKED_BUFFER_ENTRY head;
	PLINKED_BUFFER_ENTRY tail;
	HANDLE evt;
}LINKED_BUFFER,*PLINKED_BUFFER;

PLINKED_BUFFER buffer_init(size_t max_size);
BOOL buffer_add_tail(PLINKED_BUFFER buff,void* data);
void* buffer_remove_head(PLINKED_BUFFER buff,DWORD timeout);

#ifdef __cplusplus
}
#endif
#endif