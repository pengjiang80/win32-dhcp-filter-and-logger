#include <config.h>
#include <debug.h>
BOOL config_load_a(HMODULE module,CONFIG_BINDING_A* binding){
	char path[MAX_PATH];
	GetModuleFileNameA(module,path,MAX_PATH);
	for(int i=strlen(path)-1;i>=0;i--){
		if(path[i]=='\\'){
			path[i]='\0';
			break;
		}
	}
	strcat(path,"\\config.ini");
	PCONFIG_BINDING_A b;
	int i=0;
	char buffer[32767];
	int l;
	while((b=binding+(i++))->section){
		switch(b->type){
			case CONFIG_TYPE_STRING:
				l = GetPrivateProfileStringA(b->section,b->key,b->def.asString,buffer,32767,path);
				if(buffer[0]==0&&b->def.asString==NULL){
					*(b->val.asString)=NULL;
				}else{
					*(b->val.asString)=(char*)malloc(sizeof(char)*(strlen(buffer)+1));
					strcpy(*(b->val.asString),buffer);
				}
				break;
			case CONFIG_TYPE_INT:
				*(b->val.asInt) = GetPrivateProfileIntA(b->section,b->key,b->def.asInt,path);
				break;
		}
	}
	return TRUE;
}
BOOL config_load_w(HMODULE module,CONFIG_BINDING_W* binding){
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(module,path,MAX_PATH);
	for(int i=wcslen(path)-1;i>=0;i--){
		if(path[i]==L'\\'){
			path[i]=L'\0';
			break;
		}
	}
	wcscat(path,L"\\config.ini");
	PCONFIG_BINDING_W b;
	int i=0;
	wchar_t buffer[32767];
	int l;
	while((b=binding+(i++))->section){
		switch(b->type){
			case CONFIG_TYPE_STRING:
				l = GetPrivateProfileStringW(b->section,b->key,b->def.asString,buffer,32767,path);
				if(buffer[0]==0&&b->def.asString==NULL){
					*(b->val.asString)=NULL;
				}else{
					*(b->val.asString)=(wchar_t*)malloc(sizeof(wchar_t)*(wcslen(buffer)+1));
					wcscpy(*(b->val.asString),buffer);
				}
				break;
			case CONFIG_TYPE_INT:
				*(b->val.asInt) = GetPrivateProfileIntW(b->section,b->key,b->def.asInt,path);
				break;
		}
	}
	return TRUE;
}