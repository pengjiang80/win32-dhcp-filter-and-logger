#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>
#include <utils.h>

const TCHAR* getTimeStringNow(TCHAR* format){
	__time64_t time;
	_time64(&time);
	return getTimeString(&time,format);
}
const TCHAR* getTimeString(__time64_t* time,TCHAR* format){
	static TCHAR retBuf[16][256];
	static int bufIdx = 0;
	struct tm t;
	_localtime64_s(&t,time);
	bufIdx = bufIdx==15?0:bufIdx+1;
#ifdef UNICODE
	wcsftime(retBuf[bufIdx],256,format,&t);
#else
	strftime(retBuf[bufIdx],256,format,&t);
#endif
	return retBuf[bufIdx];
}