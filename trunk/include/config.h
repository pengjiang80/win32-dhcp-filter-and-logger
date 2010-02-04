#ifndef CONFIG_H
#define CONFIG_H
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
typedef struct config_binding_a{
	char*  section;
	char*  key;
	union{
		char** asString;
		int* asInt;
	} val;
	union{
		int asInt;
		char* asString;
	}  def;	
	int    type;
}CONFIG_BINDING_A, *PCONFIG_BINDING_A;

typedef struct config_binding_w{
	wchar_t* section;
	wchar_t* key;
	union{
		int* asInt;
		wchar_t** asString;
	} val;
	union{
		int asInt;
		wchar_t* asString;
	} def;
	int		 type;
}CONFIG_BINDING_W, *PCONFIG_BINDING_W;


typedef CONFIG_BINDING_A *CONFIG_BINDINGS_A;
typedef CONFIG_BINDING_W *CONFIG_BINDINGS_W;

extern BOOL config_load_a(HMODULE module,CONFIG_BINDING_A* binding);
extern BOOL config_load_w(HMODULE module,CONFIG_BINDINGS_W binding);

#define CONFIG_TYPE_INT 1
#define CONFIG_TYPE_STRING 0
//extern BOOL config_load(HMODULE module,CONFIG_BINDINGS binding);
#ifdef UNICODE
#define CONFIG_BINDING  CONFIG_BINDING_W
#define CONFIG_BINDINGS CONFIG_BINDINGS_W
#define config_load config_load_w
#else
#define CONFIG_BINDING  CONFIG_BINDING_A
#define CONFIG_BINDINGS CONFIG_BINDINGS_A
#define config_load config_load_a
#endif
#ifdef __cplusplus
}
#endif //__cplusplus
#endif