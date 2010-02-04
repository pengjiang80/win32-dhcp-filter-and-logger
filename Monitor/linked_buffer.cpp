#include "stdafx.h"
#include "linked_buffer.h"

PLINKED_BUFFER buffer_init(size_t max_size){
	PLINKED_BUFFER ret = (PLINKED_BUFFER)malloc(sizeof(LINKED_BUFFER));
	ret->max_size = max_size;
	ret->size = 0;
	ret->head = ret->tail = NULL;
	ret->evt = CreateEvent(NULL,FALSE,FALSE,NULL);
	InitializeSRWLock(&ret->lock);
	return ret;
}

BOOL buffer_add_tail(PLINKED_BUFFER buff,void* data){
	AcquireSRWLockExclusive(&buff->lock);
	__try{
		if(buff->size>=buff->max_size)
			return FALSE;
		PLINKED_BUFFER_ENTRY entry = (PLINKED_BUFFER_ENTRY)malloc(sizeof(LINKED_BUFFER_ENTRY));
		if(entry==NULL)return FALSE;
		entry->ptr = data;
		if(buff->tail){
			buff->tail->next = entry;
			buff->tail = entry;
		}else{
			buff->head=buff->tail=entry;
		}
		buff->size++;
		DEBUG("SetEvent()=%d\n",SetEvent(buff->evt));
		return TRUE;
	}__finally{
		ReleaseSRWLockExclusive(&buff->lock);
	}
}

void* buffer_remove_head(PLINKED_BUFFER buff,DWORD timeout){
	if(WaitForSingleObject(buff->evt,timeout)!=WAIT_OBJECT_0)
		return NULL;
	AcquireSRWLockExclusive(&buff->lock);
	__try{
		PLINKED_BUFFER_ENTRY entry = buff->head;
		if(entry){
			void* ret = entry->ptr;
			if(buff->tail==entry){
				buff->head=buff->tail=NULL;
			}else{
				buff->head = entry->next;
			}
			buff->size--;
			free(entry);
			return ret;
		}
		return NULL;
	}__finally{
		ReleaseSRWLockExclusive(&buff->lock);
	}
}
