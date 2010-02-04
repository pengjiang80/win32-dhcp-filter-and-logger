#ifndef DEBUG_H
#define DEBUG_H
#include <tchar.h>
#ifdef __cplusplus
extern "C" {
#endif
#define LOG DEBUG
#ifdef UNICODE
#define DEBUG(fmt,...) DEBUG_W(_T(fmt),__VA_ARGS__)
#else
#define DEBUG(fmt,...) DEBUG_A(_T(fmt),__VA_ARGS__)
#endif
#define DEBUG_A(fmt,...) debug_printf_a(fmt,__VA_ARGS__);
#define DEBUG_W(fmt,...) debug_printf_w(fmt,__VA_ARGS__);
#define DEBUG_PRINT_LASTERROR(txt) DEBUG_PRINT_LASTERROR_API(txt)
/*
{\
		LPTSTR __buf;\
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,NULL,GetLastError(),MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),(LPTSTR)&__buf,0,NULL);\
		_tprintf(_T(txt),__buf);\
		LocalFree(__buf);\
}
*/
#define DEBUG_PRINT_LASTERROR_STREAM(s,txt)\
{\
	if((s)){\
		LPTSTR __buf;\
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,NULL,GetLastError(),NULL,(LPTSTR)&__buf,0,NULL);\
		_ftprintf((s),_T(txt),__buf);\
		LocalFree(__buf);\
	}\
}
#define DEBUG_PRINT_LASTERROR_API(txt)\
{\
		LPTSTR __DEBUG_PRINT_LASTERROR_API_buf;\
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,NULL,GetLastError(),NULL,(LPTSTR)&__DEBUG_PRINT_LASTERROR_API_buf,0,NULL);\
		DEBUG(txt,__DEBUG_PRINT_LASTERROR_API_buf);\
		LocalFree(__DEBUG_PRINT_LASTERROR_API_buf);\
}
void debug_printf_a(char* format,...);
void debug_printf_w(wchar_t* format,...);
#ifdef __cplusplus
}
#endif
#endif