#ifndef __SQLAPI_SQLITE3_H__
#define __SQLAPI_SQLITE3_H__

#include "sqlapi.h"
#include "sqlite3.h"


namespace sqlapi{

#define SQL_FILE_MAX		260
#define SQL_TYPE_SQLITE3	"sqlite3\0"

struct sql_db_config_sqlite3
{
	char	file[SQL_FILE_MAX];
};


// db
class sql_db_sqlite3 : public sql_db
{
public:
	sql_db_sqlite3(const char* type, const char* id, log_cb_v cb);
	~sql_db_sqlite3();

	int open(const void* config);
	int close();
	void* handle();

	int begin();
	int end();
	int rollback();
	
	int exec(const char* sql, db_cb cb, void* cb_data);
	int check(int rc, enum SQL_CHECK chk);

private:
	sqlite3*	_db;
};


// cmd
class sql_cmd_sqlite3 : public sql_cmd
{
public:
	sql_cmd_sqlite3(const char* type, const char* id, 
					std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
					const char* tables[], int size);
	~sql_cmd_sqlite3();
};


// stmt
class sql_stmt_sqlite3 : public sql_stmt
{
public:
	sql_stmt_sqlite3(const char* type, const char* id, 
					 std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
					 const char* tables[], int count);
	virtual ~sql_stmt_sqlite3();
	
	int init();
	void release();

	int exec(const void* in);

protected:
	int bind(const void* in);
	int column(stmt_data_list& out);

protected:
	sqlite3_stmt*	_stmt;
};


}


#endif //__SQLAPI_SQLITE3_H__
