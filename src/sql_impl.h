#ifndef __SQL_IMPL_H__
#define __SQL_IMPL_H__

#include "sqlite3.h"
#include "log.h"
#include <vector>

#ifndef SQL_TABLE_MAX
#define SQL_TABLE_MAX	5
#endif


struct sql_init;

enum SQL_TYPE
{
	SQL_UNKNOW = 0,
	SQL_INT,
	SQL_INT64,
	SQL_DOUBLE,
	SQL_TEXT,
	SQL_TEXT16,
	SQL_TEXT64,
	SQL_BLOB,
	SQL_BLOB64,
};

enum SQL_CHECK
{
	SQL_CHECK_EXIST = 1,
};

typedef struct sql_dec
{
	const char* name;
	SQL_TYPE	type;
}sql_dec_t;


typedef struct sql_data
{
	char*	column;
	char*	value;
}sql_data_t;

typedef int(*db_cb)(void*, int, char**, char**);
typedef std::vector<sql_data_t>	sql_data_list;
typedef std::vector<void*>		stmt_data_list;

class sql_db
{
public:
	sql_db(sqlite3* db = NULL, log_cb_v cb = NULL);
	~sql_db();

	int open(const char* file);
	int close();
	sqlite3* handle();

private:
	int exec(const char* sql, db_cb cb = NULL, void* cb_data = NULL);
	
public:
	int exec(const char* sql, .../*table list*/);
	int exec(db_cb cb, void* cb_data, const char* sql, .../*table list*/);
	int exec(db_cb cb, void* cb_data, const char* sql, const char* tables[], int size);

	int error_check(int rc, enum SQL_CHECK chk);

	int begin();
	int end();
	int rollback();

	sql_data_list& result();
	void free_result();
	
	int init_table(sql_init* init, int size);
	int release_table(sql_init* init, int size);
	int release_table();

private:
	sqlite3*	_db;
	log_cb_v	_log_cb;
	int			_trans;
	std::vector<sql_init*>	_init;
	std::vector<int>		_init_size;
	sql_data_list			_result;
};


class sql_stmt
{
public:
	sql_stmt(sqlite3* db, log_cb_v cb, const char* sql, .../* table list */);
	sql_stmt(sqlite3* db, log_cb_v cb, const char* sql, const char* tables[], int count);
	~sql_stmt();
	
	int init(const sql_dec_t* src_dec, int src_size,
			 const sql_dec_t* out_dec, int out_size);

	int release();

	int exec(const void* in, stmt_data_list* pout = NULL);
	
	int error_check(int rc, enum SQL_CHECK chk);
	
	stmt_data_list& result();
	void free_result();
	void free_result(stmt_data_list& ins);
	void free_result(void* in);

protected:
	int bind(const void* in);
	int column(stmt_data_list& out);
	int* struct_offset_calc(const sql_dec_t* dec, int size);

private:
	bool		_inited;
	
	log_cb_v	_log_cb;
	sqlite3*	_db;
	sqlite3_stmt* _stmt;
	char*		_table[SQL_TABLE_MAX];
	char*		_sql;
	sql_dec_t*	_in;
	sql_dec_t*	_out;
	int			_in_size;
	int			_out_size;
	void*		_ext;
	stmt_data_list	_result;
};



enum sql_init_type
{
	sql_init_start = 0,
	sql_init_exec = 0,
	sql_init_stmt,
	sql_init_end,
};

struct stmt_init{
	sql_stmt**	stmt;
	sql_dec_t*	in;
	int			in_size;
	sql_dec_t*	out;
	int			out_size;
};

struct sql_init
{
	const char*		sql;
	const char* 	table[SQL_TABLE_MAX];
	int				init_type;
	union{
		stmt_init	stmt;
	}init_data;
};


/* sql_dec_t */
#define SQL_DEC_BEGIN(table, func)	_SQL_DEC_BEGIN(table, func)
#define SQL_DEC(name, type)			_SQL_DEC(name, type)
#define SQL_DEC_END(table, func)	_SQL_DEC_END(table, func)
#define SQL_DEC_OBJ(table, func)	_SQL_DEC_OBJ(table, func)

#define _SQL_DEC_BEGIN(table, func)	\
		sql_dec_t __##table##_##func##_dec[] = {


#define _SQL_DEC(name, type)	\
		{#name, type},

#define _SQL_DEC_END(table, func)	\
		};\
		int __##table##_##func##_dec_size = \
				sizeof(__##table##_##func##_dec)/\
				sizeof(__##table##_##func##_dec[0]);

#define _SQL_DEC_OBJ(table, func)	\
		 __##table##_##func##_dec, __##table##_##func##_dec_size


/* sql_init */
#define SQL_INIT_CREATE(type, id, table)	_SQL_INIT_CREATE(type, id, table)
#define SQL_INIT_STMT(type, id, table, func, in, in_size, out, out_size)  \
			_SQL_INIT_STMT(type, id, table, func, in, in_size, out, out_size)

#define SQL_INIT_STMT10(type, id, table, func, in)	\
			_SQL_INIT_STMT10(type, id, table, func, in)

#define SQL_INIT_STMT01(type, id, table, func, out)	\
			_SQL_INIT_STMT01(type, id, table, func, out)

#define SQL_INIT_STMT11(type, id, table, func, in, out)	\
			_SQL_INIT_STMT11(type, id, table, func, in, out)

#define _SQL_INIT_CREATE(type, id, table)	\
		{table_##type##_create, {table}, sql_init_exec, {0}}
#define _SQL_INIT_STMT(type, id, table, func, in, in_size, out, out_size)	\
		{table_##type##_##func, {table}, sql_init_stmt, {&(g_##id##_##func), in, in_size, out, out_size}}

#define _SQL_INIT_STMT01(type, id, table, func, out)	\
		{table_##type##_##func, {table}, sql_init_stmt, {&(g_##id##_##func), NULL, 0, out}}

#define _SQL_INIT_STMT10(type, id, table, func, in)	\
		{table_##type##_##func, {table}, sql_init_stmt, {&(g_##id##_##func), in, NULL, 0}}

#define _SQL_INIT_STMT11(type, id, table, func, in, out)	\
		{table_##type##_##func, {table}, sql_init_stmt, {&(g_##id##_##func), in, out}}



/* table */
#define SQL_TABLE(t)				_SQL_TABLE(t)
#define SQL_TABLE2(t1, t2)			_SQL_TABLE2(t1, t2)
#define SQL_TABLE3(t1, t2, t3)		_SQL_TABLE3(t1, t2, t3)
#define SQL_TABLE4(t1, t2, t3, t4)	_SQL_TABLE4(t1, t2, t3, t4)

#define _SQL_TABLE(t)				#t
#define _SQL_TABLE2(t1, t2)			#t1, #t2
#define _SQL_TABLE3(t1, t2, t3)		#t1, #t2, #t3
#define _SQL_TABLE4(t1, t2, t3, t4)	#t1, #t2, #t3, #t4



#endif // __SQLITE3_IMPL_H__
