// DHCPCalloutHook.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <stdlib.h>
#include <share.h>
#include <tchar.h>
#include <Winsock2.h>
#include "CalloutHook.h"

DHCP_CALLOUT_TABLE callout_table = {
	DhcpControlHook,		//LPDHCP_CONTROL                 DhcpControlHook;
    NULL,//DhcpNewPktHook,			//LPDHCP_NEWPKT                  DhcpNewPktHook;
    NULL,					//LPDHCP_DROP_SEND               DhcpPktDropHook;
    NULL,					//LPDHCP_DROP_SEND               DhcpPktSendHook;
	DhcpAddressDelHook,		//LPDHCP_PROB                    DhcpAddressDelHook;
    DhcpAddressOfferHook,	//LPDHCP_GIVE_ADDRESS            DhcpAddressOfferHook;
    NULL,					//LPDHCP_HANDLE_OPTIONS          DhcpHandleOptionsHook;
    DhcpDeleteClientHook,	//LPDHCP_DELETE_CLIENT           DhcpDeleteClientHook;
    NULL,					//LPVOID                         DhcpExtensionHook;
	NULL,					//LPVOID                         DhcpReservedHook;
};



FILE* outfile;
PSHM_BLOCK shm = NULL;
LOCK _shm_init_lock;
PACL_LIST acl;
BOOL InitSHM(){
	DEBUG("InitSHM(): shm=0x%08x\n",shm);
	if(shm)return TRUE;
	if(!lock_wait(&_shm_init_lock,50000)){
		DEBUG("InitSHM(): Lock Failed!");
		return FALSE;
	}
	if(shm)return TRUE;//double check
	shm = (PSHM_BLOCK)malloc(sizeof(SHM_BLOCK));
	ZeroMemory(shm,sizeof(SHM_BLOCK));
	shm->len = 2048;
	_tcscpy(shm->filename,SHM_CALLOUT_NAME);
	if(FAILED(shm_init_client(shm))){
		DEBUG_PRINT_LASTERROR_API("Fail to open shared memory:%s");
		free(shm);
		shm=NULL;
	}
	lock_release(&_shm_init_lock);
	return TRUE;
}
HOOK_API DWORD CALLBACK DhcpServerCalloutEntry(
  __in   LPWSTR ChainDlls,
  __in   DWORD CalloutVersion,
  __out  LPDHCP_CALLOUT_TABLE CalloutTbl
)
{
	*CalloutTbl = callout_table;
	//outfile = _fsopen("C:\\dhcp.log","a",_SH_DENYNO);
	return 0;
}

HOOK_API DWORD CALLBACK DhcpControlHook(
  __in  DWORD dwControlCode,
  __in  LPVOID lpReserved
)
{
	switch(dwControlCode){
	case DHCP_CONTROL_START :
		LOG("DHCP Service started!\n");
		lock_init(&_shm_init_lock,NULL);
		acl = acl_init();
		if(!acl){
			DEBUG("acl_init failed.");
		}
		break;
	case DHCP_CONTROL_STOP :
		LOG("DHCP Service stoped!\n");
		if(outfile){
			fclose(outfile);
		}
		if(shm){
			shm_close(shm);
		}
		lock_destory(&_shm_init_lock);
		break;
	case DHCP_CONTROL_CONTINUE:
		LOG("DHCP Service contunied!\n");
		if(outfile){
			fflush(outfile);
		}
		break;
	case DHCP_CONTROL_PAUSE:
		LOG("DHCP Service paused!\n");
		break;
	}
	return 0;
}

HOOK_API DWORD CALLBACK DhcpNewPktHook(
  __inout  LPBYTE *Packet,
  __inout  DWORD *PacketSize,
  __in     DWORD IpAddress,
  __in     LPVOID Reserved,
  __inout  LPVOID *PktContext,
  __out    LPBOOL ProcessIt
 )
{
	PDHCP_PACKET pkt = (PDHCP_PACKET)*Packet;
	/*char hwaddr_buf[2*(16*3+1)];
	char* pmt = dhcp_find_option(pkt->options+4,DHCP_MAX_OPTION_LEN-4,DHO_DHCP_MESSAGE_TYPE,NULL);
	char msg_type = pmt?*pmt:0;
	DEBUG(
		"[%s]Request Detail:\n"
		L"          Message type: %s(%d)\n"
		L"          Hw type: %s(%d)\n"
		L"          Hw address length: %d\n"
		L"          Hops: %d\n"
		L"          Tx ID: 0x%08x\n"
		L"          Seconds waited: %d\n"
		L"          Flag: 0x%04x\n"
		L"          Client Mac: %s\n",
		getTimeStringNow(_T("%H:%M:%S")),
		DHCP_MSG_TYPE(msg_type),msg_type,
		DHCP_HW_TYPE(pkt->htype),pkt->htype,
		pkt->hlen,
		pkt->hops,
		pkt->xid,
		pkt->secs,
		pkt->flags,
		dhcp_hw_addr(hwaddr_buf,pkt->chaddr,pkt->hlen));
		*/
	(*ProcessIt) = acl_filter(acl,pkt->htype,pkt->chaddr,pkt->hlen);
	DEBUG("acl_filter()=>%s\n",(*ProcessIt)?_T("TRUE"):_T("FALSE"));
	return 0;
}

HOOK_API DWORD CALLBACK DhcpAddressOfferHook(
  __in  LPBYTE Packet,
  __in  DWORD PacketSize,
  __in  DWORD ControlCode,
  __in  DWORD IpAddress,
  __in  DWORD AltAddress,
  __in  DWORD AddrType,
  __in  DWORD LeaseTime,
  __in  LPVOID Reserved,
  __in  LPVOID PktContext
){
	LOG("DhcpAddressOfferHook(PacketSize=%d,ControlCode=0x%08x,IpAddress=%08x,AltAddress=%08x,AddrType=%08x,LeaseTime=%d)\n",
							  PacketSize,	ControlCode,	   IpAddress,     AltAddress,     AddrType,     LeaseTime);
	if(InitSHM()){
		DHCP_ADDR_OFFER_HOOK_ARGS args;
		args.HookType = CALLOUT_ADDRESS_OFFER;
		args.PacketSize  = PacketSize;
		args.ControlCode = ControlCode;
		args.IpAddress   = IpAddress;
		args.AltAddress  = AltAddress;
		args.AddrType	 = AddrType;
		args.LeaseTime   = LeaseTime;
		memcpy(args.Packet,Packet,PacketSize);
		if(FAILED(shm_write(shm,&args,sizeof(args)))){
			DEBUG_PRINT_LASTERROR_API("Write error: %s");
		}
	}

	return 0;
}
DWORD CALLBACK DhcpAddressDelHook(
  __in  LPBYTE Packet,
  __in  DWORD PacketSize,
  __in  DWORD ControlCode,
  __in  DWORD IpAddress,
  __in  DWORD AltAddress,
  __in  LPVOID Reserved,
  __in  LPVOID PktContext
)
{
	LOG("DhcpAddressDelHook(ControlCode=%08x, IpAddress=%08x, AltAddress=%08x)\n",ControlCode,IpAddress,AltAddress);
	if(InitSHM()){
		DHCP_ADDR_DELETE_HOOK_ARGS args;
		args.HookType = CALLOUT_ADDRESS_DELETE;
		args.ControlCode  = ControlCode;
		args.AltAddress   = AltAddress;
		args.IpAddress    = IpAddress;
		args.PacketSize   = PacketSize;
		memcpy(args.Packet,Packet,PacketSize);
		if(FAILED(shm_write(shm,&args,sizeof(args)))){
			DEBUG_PRINT_LASTERROR_API("Write error: %s");
		}
	}
	return 0;
}

DWORD CALLBACK DhcpDeleteClientHook(
  __in  DWORD IpAddress,
  __in  LPBYTE HwAddress,
  __in  ULONG HwAddressLength,
  __in  DWORD Reserved,
  __in  DWORD ClientType
)
{
	LOG("DhcpDeleteClientHook(IpAddress=%08x)\n",IpAddress);

	if(InitSHM()){
		DHCP_CLIENT_DELETE_HOOK_ARGS args;
		args.HookType = CALLOUT_CLIENT_DELETE;
		args.IpAddress   = IpAddress;
		memcpy(args.HwAddress,HwAddress,HwAddressLength);
		args.HwAddressLength = HwAddressLength;
		args.ClientType  = ClientType;
		if(FAILED(shm_write(shm,&args,sizeof(args)))){
			DEBUG_PRINT_LASTERROR_API("Write error: %s");
		}
	}
	return 0;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}