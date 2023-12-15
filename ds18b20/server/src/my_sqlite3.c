#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "logger.h"
#include "pack_data.h"

#define DEBUG_OUT 0
#if		DEBUG_OUT

#define DEBUG_PRINT(fmt, args...)	printf(fmt, ##args)

#else

#define DEBUG_PRINT(fmt, args...)	do{} while(0)

#endif

static sqlite3		*s_db;

/* 初始化数据库 */
int db_init(void)
{
	char		*create_table = NULL;		//create table statement
	char		*zErrMsg = NULL;			//Error messages
	int		 	 rv = 0;					//create table falg

	/* 打开/创建数据库 */
	rv = sqlite3_open("ds18b20.db", &s_db);
	if( rv )
	{
		LOGGER_ERROR("open database [ds18b20.db] failure: %s", sqlite3_errmsg(s_db));
		return -1;
	}
	LOGGER_DEBUG("open database [ds18b20.db] successfully");

	/* 创建表 */
	create_table = "CREATE TABLE IF NOT EXISTS TEMP\
			    ("\
			       "ID TEXT NOT NULL,"\
			       "TIME TEXT PRIMARY KEY NOT NULL,"\
			       "TEMP FLOAT NOT NULL\
			    );";
	rv = sqlite3_exec(s_db, create_table, NULL, NULL, &zErrMsg);
	if( rv != SQLITE_OK )
	{
		LOGGER_ERROR("create table [TEMP] failure: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	LOGGER_DEBUG("create table [TEMP] successfully");
	return 0;
}

/* 插入数据 */
int db_insert(packet_t *pack)
{
	if ( NULL == pack )
	{
		LOGGER_ERROR("NULL pointer");
		return -1;
	}
	char		 insert_records[128] = {0};	//insert records statement
	char		*zErrMsg = NULL;			//Error messages
	int		 	 rv = 0;					//insert records flag

	snprintf(insert_records, sizeof(insert_records), "INSERT INTO TEMP VALUES('%s', '%s', '%.3f');", pack->dev_id, pack->dev_time, pack->dev_temp);
	rv = sqlite3_exec(s_db, insert_records, NULL, NULL, &zErrMsg);
	if( rv != SQLITE_OK )
	{
		LOGGER_ERROR("insert records failure: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	LOGGER_DEBUG("insert records successfully");
	return 0;
}

/* 查询数据 */
int db_query(packet_t *pack)
{
	if ( NULL == pack )
	{
		LOGGER_ERROR("NULL pointer");
		return -1;
	}
	char			*query_records = NULL;	//query records statement
	sqlite3_stmt 	*stmt;
	int		 	 	 rv = 0;				//query records falg

	if ( sqlite3_errcode(s_db) == SQLITE_NOTADB )
	{
		return -1;
	}
	query_records = "SELECT * FROM TEMP	LIMIT 1;";
	rv = sqlite3_prepare_v2(s_db, query_records, -1, &stmt, NULL);
	if( rv != SQLITE_OK )
	{
		LOGGER_ERROR("sqlite3_prepare_v2 failure: %s", sqlite3_errmsg(s_db));
		return -1;
	}
	LOGGER_DEBUG("sqlite3_prepare_v2 successfully");

	memset(pack, 0, sizeof(pack));
	rv = sqlite3_step(stmt);
	if ( rv == SQLITE_ROW ) 
	{	/* 数据库有数据 */
		snprintf(pack->dev_id, sizeof(pack->dev_id), "%s", sqlite3_column_text(stmt, 0));
		snprintf(pack->dev_time, sizeof(pack->dev_time), "%s", sqlite3_column_text(stmt, 1));
		pack->dev_temp = sqlite3_column_double(stmt, 2);
		LOGGER_DEBUG("query records successfully");
		sqlite3_finalize(stmt);
		return 0;
	}
	else
	{	/* 查询出错，数据库空 */
		LOGGER_ERROR("sqlite3_step failure: %s", sqlite3_errmsg(s_db));
		sqlite3_finalize(stmt);
		return -2;
	}
}

/* 删除数据 */
int db_delete(void)
{
	char		 delete_records[128] = {0};		//delete records statement
	char		*zErrMsg = NULL;				//Error messages
	int		 	 rv = 0;						//delete records flag

	snprintf(delete_records, sizeof(delete_records), "DELETE FROM TEMP WHERE TIME IN (SELECT TIME FROM TEMP LIMIT 1);");
	//snprintf(delete_records, sizeof(delete_records), "DELETE FROM TEMP;");
	rv = sqlite3_exec(s_db, delete_records, NULL, NULL, &zErrMsg);
	if( rv != SQLITE_OK )
	{
		LOGGER_ERROR("delete records failure: %s", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	LOGGER_DEBUG("delete records successfully");
	return 0;
}

/* 关闭数据库 */
int db_close()
{
	sqlite3_close(s_db);
	return 0;
}


































