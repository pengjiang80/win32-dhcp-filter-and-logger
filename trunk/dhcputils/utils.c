#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <utils.h>

const char* getTimeStringNow(char* format){
	__time64_t time;
	_time64(&time);
	return getTimeString(&time,format);
}
const char* getTimeString(__time64_t* time,char* format){
	static char retBuf[16][256];
	static int bufIdx = 0;
	struct tm t;
	_localtime64_s(&t,time);
	bufIdx = bufIdx==15?0:bufIdx+1;
	strftime(retBuf[bufIdx],256,format,&t);
	return retBuf[bufIdx];
}