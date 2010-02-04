#ifndef UTILS_H
#define UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

extern const TCHAR* getTimeStringNow(TCHAR* format);
extern const TCHAR* getTimeString(__time64_t* time,TCHAR* format);

#ifdef __cplusplus
}
#endif
#endif