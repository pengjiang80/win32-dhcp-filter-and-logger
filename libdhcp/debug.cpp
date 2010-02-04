#include <stdarg.h>
#include <windows.h>
#include <stdio.h>
#include <debug.h>
void debug_printf_a(char* fmt,...){
	char buf[65536];
	va_list args;
	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	OutputDebugStringA(buf);\
	printf("%s",buf);\
}
void debug_printf_w(wchar_t* fmt,...){
	wchar_t buf[65536];
	va_list args;
	va_start(args,fmt);
	wvsprintf(buf,fmt,args);
	OutputDebugStringW(buf);\
	wprintf(L"%s",buf);\
}