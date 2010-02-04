#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <tchar.h>
//#include "dhcp.h"
const TCHAR* _dhcp_hw_type_name[] = {
	_T("(Unknown)"),\
	_T("Ethernet"),\
	_T("(Unknown)"),\
	_T("(Unknown)"),\
	_T("(Unknown)"),\
	_T("(Unknown)"),\
	_T("IEEE 802"),\
	_T("(Unknown)"),\
	_T("FDDI"),\
	_T("(Unknown)"),\
};
const TCHAR* _dhcp_msg_type_name[] = {
	_T("(Unknown)"),
	_T("Discovery"),
	_T("Offer"),
	_T("Request"),
	_T("Decline"),
	_T("Ack"),
	_T("Nak"),
	_T("Release"),
	_T("Inform"),
	_T("LeaseQuery"),
	_T("LeaseUnassigned"),
	_T("LeaseUnknow"),
	_T("LeaseActive"),
	_T("(Unknown)"),
	_T("(Unknown)"),
};

char* dhcp_hw_addr_ansi(char* buff,const unsigned char* addr,int len){
	int i=0;
	for(;i<len;i++){
		buff[i*3+0]="0123456789abcdef"[(addr[i]>>4)&0x0F];
		buff[i*3+1]="0123456789abcdef"[(addr[i]   )&0x0F];
		buff[i*3+2]=':';
	}
	buff[len*3-1]=0;
	return buff;
}

wchar_t* dhcp_hw_addr_unicode(wchar_t* buff,const unsigned char* addr,int len){
	int i=0;
	for(;i<len;i++){
		buff[i*3+0]=L"0123456789abcdef"[(addr[i]>>4)&0x0F];
		buff[i*3+1]=L"0123456789abcdef"[(addr[i]   )&0x0F];
		buff[i*3+2]=L':';
	}
	buff[len*3-1]=0;
	return buff;
}

char* dhcp_find_option(const unsigned char* data,const size_t size,const unsigned char option,size_t* datalen){
	unsigned int idx = 0;
	unsigned char cop = 0;
	unsigned char dlen = 0;
	while(cop!=0xFF&&idx<size){
		cop = data[idx];
		dlen = data[idx+1];
		if(cop==option){
			if(datalen)*datalen = dlen;
			return data+idx+2;
		}else{
			idx += dlen+2;
		}
	}
	return NULL;
}