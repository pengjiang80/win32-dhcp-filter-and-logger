// Monitor.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "acl_update.h"
#include "linked_buffer.h"
using namespace acl_update;
MYSQL* conn;
SHM_BLOCK shm;
PLINKED_BUFFER buffer;
char* db_host, *db_user, *db_pass, *db_name;
int db_port;
CONFIG_BINDING_A bindings[] = {
	{"db","host",&db_host},
	{"db","user",&db_user},
	{"db","pass",&db_pass},
	{"db","name",&db_name},
	{"db","port",(char**)&db_port,3306,CONFIG_TYPE_INT},
	NULL,
};
void handle_offer(PDHCP_ADDR_OFFER_HOOK_ARGS buf){
	PDHCP_PACKET pkt;
	char hwaddr_buf[2*(16*3+1)];//it's big enough
	char hnhex[512];
	char query[65536];
	size_t hnl;
	unsigned char* inaddr;
	pkt = (PDHCP_PACKET)buf->Packet;
	inaddr = (u_char*)&buf->AltAddress;
	char* pmt = dhcp_find_option(pkt->options+4,DHCP_MAX_OPTION_LEN-4,DHO_DHCP_MESSAGE_TYPE,NULL);
	char msg_type = pmt?*pmt:0;
	_tprintf(
		L"[%s]Request Detail:\n"
		L"          Message type: %s(%d)\n"
		L"          Hw type: %s(%d)\n"
		L"          Hw address length: %d\n"
		L"          Hops: %d\n"
		L"          Tx ID: 0x%08x\n"
		L"          Seconds waited: %d\n"
		L"          Flag: 0x%04x\n"
		L"          Client Mac: %s\n"
		L"          Offered Address: %u.%u.%u.%u\n",
		getTimeStringNow(_T("%H:%M:%S")),
		DHCP_MSG_TYPE(msg_type),msg_type,
		DHCP_HW_TYPE(pkt->htype),pkt->htype,
		pkt->hlen,
		pkt->hops,
		pkt->xid,
		pkt->secs,
		pkt->flags,
		dhcp_hw_addr(hwaddr_buf,pkt->chaddr,pkt->hlen),
		inaddr[3],inaddr[2],inaddr[1],inaddr[0]);
	//SetEvent(shm.eack);
	char* ohn = dhcp_find_option(pkt->options+4,DHCP_MAX_OPTION_LEN-4,DHO_HOST_NAME,&hnl);
	if(ohn[hnl-1]==0)hnl--;// skip last '\0'
	mysql_hex_string(hnhex,ohn,hnl);
	sprintf(query,"INSERT INTO offer_log(hwaddr,ip,lease_time,hostname) VALUES ('%s',0x%08X,%d,0x%s)",dhcp_hw_addr_ansi(hwaddr_buf,pkt->chaddr,pkt->hlen),buf->AltAddress,buf->LeaseTime,hnhex);
	printf("SQL: %s\n",query);
	if(mysql_query(conn,query)!=0){
		printf("Failed: %s\n",mysql_error(conn));
	}else{
		printf("OK! Affected %d row(s)\n",mysql_affected_rows(conn));
	}
}
void handle_lease(PDHCP_ADDR_DELETE_HOOK_ARGS buf){
	if(buf->ControlCode!=DHCP_PROB_RELEASE)return;
	char hwaddr_buf[2*(16*3+1)];//it's big enough
	char query[65536];
	PDHCP_PACKET pkt = (PDHCP_PACKET)buf->Packet;
	sprintf(query,"INSERT INTO lease_log(hwaddr,ip,lease_type) VALUES ('%s',0x%08X,'requested')",dhcp_hw_addr_ansi(hwaddr_buf,pkt->chaddr,pkt->hlen),buf->AltAddress);
	printf("SQL: %s\n",query);
	if(mysql_query(conn,query)!=0){
		printf("Failed: %s\n",mysql_error(conn));
	}else{
		printf("OK! Affected %d row(s)\n",mysql_affected_rows(conn));
	}
}
void handle_delete(PDHCP_CLIENT_DELETE_HOOK_ARGS buf){
	char hwaddr_buf[2*(16*3+1)];//it's big enough
	char query[65536];
	sprintf(query,"INSERT INTO lease_log(hwaddr,ip,lease_type) VALUES ('%s',0x%08X,'timeout')",dhcp_hw_addr_ansi(hwaddr_buf,buf->HwAddress,buf->HwAddressLength),buf->IpAddress);
	printf("SQL: %s\n",query);
	if(mysql_query(conn,query)!=0){
		printf("Failed: %s\n",mysql_error(conn));
	}else{
		printf("OK! Affected %d row(s)\n",mysql_affected_rows(conn));
	}
}
DWORD WINAPI handle_thread(void*){
	void* buf;
	DWORD* HookType;
	while(TRUE){
		buf = buffer_remove_head(buffer,5000);
		if(buf==NULL){
			DEBUG("handle_thread: Waiting...\n");
		}else{
			HookType = (DWORD*)buf;
			switch(*HookType){
			case CALLOUT_ADDRESS_OFFER:
				handle_offer((PDHCP_ADDR_OFFER_HOOK_ARGS)buf);
				break;
			case CALLOUT_ADDRESS_DELETE:
				handle_lease((PDHCP_ADDR_DELETE_HOOK_ARGS)buf);
				break;
			case CALLOUT_CLIENT_DELETE:
				handle_delete((PDHCP_CLIENT_DELETE_HOOK_ARGS)buf);
				break;
			}
			free(buf);
		}
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	config_load_a(GetModuleHandle(NULL),bindings);
	printf("mysql://%s:%d/%s;user=%s;pass=%s\n",db_host,db_port,db_name,db_user,db_pass);

	_SHM_LOCKDEF lockdef;
	ZeroMemory(&shm,sizeof(shm));
	shm.len = 2048;
	_tcscpy(shm.filename,SHM_CALLOUT_NAME);
	_tcscpy(lockdef.lockname,SHM_CALLOUT_LOCK);
	_tcscpy(lockdef.eackname,SHM_LOCK_NONE);
	_tcscpy(lockdef.esndname,SHM_CALLOUT_EVENT_SEND);
	int ret = shm_init(&shm,&lockdef);
	if(FAILED(ret)){
		DEBUG_PRINT_LASTERROR("Fail to open shared memory: %s");
		return -1;
	}
	
	conn = mysql_init(NULL);
	
	if(!mysql_real_connect(conn,db_host,db_user,db_pass,db_name,db_port,NULL,0)){
		printf("DB_Init failed: %s\n",mysql_error(conn));	
		return -2;
	}

	buffer = buffer_init(65536);

	//acl_update_start();
	CreateThread(NULL,NULL,handle_thread,NULL,NULL,NULL);
	void* buf = malloc(2048);
	DWORD* HookType = (DWORD*)buf;
	while (TRUE) {
		switch(WaitForSingleObject(shm.esnd,1000)){
		case WAIT_TIMEOUT:
			DEBUG(".");
			break;
		case WAIT_OBJECT_0:
			BOOL ret = shm_lock(&shm,1500);
			DEBUG("shm_lock() => %d\n",ret);
			if(ret){
				__try{
					memcpy(buf,shm.data,2048);
					buffer_add_tail(buffer,buf);
				}__finally{
					shm_unlock(&shm);
				}
			}
		}
	};
	return 0;
}

