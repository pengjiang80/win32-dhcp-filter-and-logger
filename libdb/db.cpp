#include <stdafx.h>
#include <db.h>
BOOL db_init(PDB_CONNECTION db){
	if((db->conn=mysql_init(NULL))==NULL)return FALSE;
	if(mysql_real_connect(db->conn,db->cfg.host,db->cfg.user,db->cfg.pass,db->cfg.dbname,db->cfg.port,0,0)==NULL)return FALSE;
	return TRUE;
}

BOOL db_close(PDB_CONNECTION db){
	mysql_close(db->conn);
	return TRUE;
}

DWORD db_query(PDB_CONNECTION db,char* sql){
	return mysql_real_query(db->conn,sql,strlen(sql));
}

LONGLONG db_affected_rows(PDB_CONNECTION db){
	return mysql_affected_rows(db->conn);
}

size_t db_hex_string(char* buff,char* data,size_t len){
	return mysql_hex_string(buff,data,len);
}

const char* db_error(PDB_CONNECTION db){
	return mysql_error(db->conn);
}