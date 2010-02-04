#include <stdafx.h>
#include "acl_update.h"
#include <shmdefs.h>
#include <mysql.h>
#include <acl.h>
extern char* db_host, *db_user, *db_pass, *db_name;
extern int db_port;
namespace acl_update{
MYSQL *conn;
HANDLE acl_update_event_exit;
char last_update[16]="0";
PACL_LIST acl;

void acl_update_start(){
	acl_update_event_exit = CreateEvent(NULL,FALSE,FALSE,NULL);
	CreateThread(NULL,NULL,acl_update_thread,NULL,NULL,NULL);
}
void acl_update_stop(){
	SetEvent(acl_update_event_exit);
}
BOOL acl_check_last_update(){
	char query[256];
	sprintf(query,"SELECT MAX(ts+0) FROM access_list where (ts+0)>%s",last_update);
	mysql_query(conn,query);
	MYSQL_RES* res = mysql_store_result(conn);
	if(res==NULL){
		DEBUG_A("mysql_store_result() failed: %s\n",mysql_error(conn));
		return FALSE;
	}
	__try{
		MYSQL_ROW row = mysql_fetch_row(res);
		if(row[0]==NULL){
			DEBUG("acl_check_last_update: no more new acl(s) found\n");
			return FALSE;
		}else{
			DEBUG_A("acl_check_last_update: found newly added or updated acl(s) with timestamp: %s\n",row[0]);
			strncpy(last_update,row[0],16);
			return TRUE;
		}
	}__finally{
		mysql_free_result(res);
	}
}
VOID acl_parse_hwaddr(unsigned char* buf,size_t bufsize,char* str){
	size_t idx = 0;
	char* endptr = str;
	while(idx<bufsize){
		buf[idx++] = strtoul(endptr,&endptr,16);
		if(*endptr==0)
			break; 
		else 
			endptr+=1;//skip ":"
	}
}
VOID acl_do_update(){
	mysql_query(conn,"SELECT hwtype,hwaddr,hwmask,access FROM access_list ORDER BY hwmask DESC , hwaddr");
	MYSQL_RES* res = mysql_store_result(conn);
	if(res==NULL){
		DEBUG_A("mysql_store_result() failed: %s\n",mysql_error(conn));
		return;
	}

	int count = 0;
	ACL_ENTRY buff[2048];
	ZeroMemory(buff,2048*sizeof(ACL_ENTRY));
	__try{
		MYSQL_ROW row;
		while((row = mysql_fetch_row(res))&&(count<2048)){
			PACL_ENTRY entry = buff+count;
			entry->HwType = atoi(row[0]);
			acl_parse_hwaddr(entry->HwAddr,16,row[1]);
			acl_parse_hwaddr(entry->HwMask,16,row[2]);
			entry->access = atoi(row[3]);
			count++;
		}
		DEBUG("acl_do_update: loaded %d acl(s) from database\n",count);
		acl_sort_entry(buff,count);
		char hwbuff1[64],hwbuff2[64];
		for(int i=0;i<count;i++){
			DEBUG_A(
				"ACL[%d]{\n"
				"\taccess=%d\n"
				"\tHwType=%d\n"
				"\tHwAddr=%s\n"
				"\tHwMask=%s\n"
				"}\n",
				i,
				buff[i].access,
				buff[i].HwType,
				dhcp_hw_addr_ansi(hwbuff1,buff[i].HwAddr,16),
				dhcp_hw_addr_ansi(hwbuff2,buff[i].HwMask,16)
				)
		}
		acl_write_lock(acl);
		acl->length = count;
		memcpy(acl->entry,buff,sizeof(ACL_ENTRY)*count);
	}__finally{
		acl_write_unlock(acl);
		//free(buff);
		mysql_free_result(res);
	}
}
DWORD WINAPI acl_update_thread(void* arg){
	int counter = 0;
	conn = mysql_init(NULL);
	if(!mysql_real_connect(conn,db_host,db_user,db_pass,db_name,db_port,NULL,0)){
		DEBUG_A("mysql_real_connect() failed: %s\n",mysql_error(conn));
		return -1;
	}
	acl = acl_connect();
	if(!acl){
		DEBUG_PRINT_LASTERROR("acl_connect() failed: %s\n");
		return -2;
	}
	while(TRUE){
		switch(WaitForSingleObject(acl_update_event_exit,1000)){
			case WAIT_OBJECT_0:
				break;
			case WAIT_TIMEOUT:
				if(counter++%3==0){
					if(acl_check_last_update()){
						acl_do_update();
					}
				}
				break;
		}
	}
	return 0;
}
}