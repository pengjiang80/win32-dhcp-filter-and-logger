#ifndef ACL_H
#define ACL_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
#define ACCESS_DENY FALSE
#define ACCESS_PERMIT TRUE

typedef struct acl_entry{
	unsigned char HwType;
	unsigned char HwAddr[16];
	unsigned char HwMask[16];
	BOOL access;
}ACL_ENTRY,*PACL_ENTRY;

typedef struct acl_list{
	unsigned int length;
	SRWLOCK lock;
	ACL_ENTRY entry[1];
}ACL_LIST,*PACL_LIST;


#define SHM_ACL_NAME		_T("Global\\SHM_ACL_NAME_{6949ECA7-4143-4cf8-81D6-1854F1225CAE}")
#define SHM_ACL_LOCK		_T("Global\\SHM_ACL_LOCK_{6949ECA7-4143-4cf8-81D6-1854F1225CAE}")
#define SHM_ACL_EVENT_SEND  _T("Global\\SHM_ACL_EVENT_SEND_{6949ECA7-4143-4cf8-81D6-1854F1225CAE}")

extern PACL_LIST acl_connect();
extern PACL_LIST acl_init();
extern BOOL acl_filter(	PACL_LIST acl,
						const unsigned char HwType,
						const unsigned char* HwAddr,
						unsigned char HwAddrLen);
extern VOID acl_write_lock(PACL_LIST acl);
extern VOID acl_write_unlock(PACL_LIST acl);
extern VOID acl_sort_entry(PACL_ENTRY entries,int count);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //ACL_H