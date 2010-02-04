#ifndef DB_H
#define DB_H
#ifdef _LIB_DB
#include "mysql.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct db_connection_cfg{
	char  host[256];
	int   port;
	char  user[256];
	char  pass[256];
	char  dbname[256];
} DB_CONNECTION_CFG,*PDB_CONNECTION_CFG;

typedef struct db_connection {
	DB_CONNECTION_CFG cfg;
	USHORT status;
#ifdef _LIB_DB
	MYSQL* conn;
#else
	void* conn;
#endif
}DB_CONNECTION, *PDB_CONNECTION;

extern BOOL db_init(PDB_CONNECTION db);
extern BOOL db_close(PDB_CONNECTION db);
extern DWORD db_query(PDB_CONNECTION db,char* sql);
extern LONGLONG db_affected_rows(PDB_CONNECTION db);
extern size_t db_hex_string(char* buff,char* data,size_t len);
extern const char* db_error(PDB_CONNECTION db);
#ifdef __cplusplus
}
#endif
#endif