#ifndef __SQLAPI_H__
#define __SQLAPI_H__

#include <vector>
#include <memory>
#include <stdarg.h>
#include "log.h"
#include "struct_offset.h"

#ifndef SQL_TABLE_MAX
#define SQL_TABLE_MAX	8
#endif

#define SQLAPI_OK		0
#define SQLAPI_ERR		-1
#define SQLAPI_TRUE		1
#define SQLAPI_FALSE	0

namespace sqlapi{

class sql_db;
class sql_exec;
class sql_cmd;
class sql_stmt;
class sql_gen ;

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
	int			type;	//  flag |  SQL_TYPE & 0x00ff
}sql_dec_t;

typedef struct sql_data
{
	char*	column;
	char*	value;
}sql_data_t;

typedef int(*db_cb)(void*, int, char**, char**);
typedef std::vector<sql_data_t>	sql_data_list;
typedef std::vector<void*>		stmt_data_list;
typedef std::vector<std::weak_ptr<sql_exec>>	sql_exec_list;

class sql_db : public std::enable_shared_from_this<sql_db>
{
public:
	sql_db(const char* type, const char* id, log_cb_v cb);
	virtual ~sql_db();

	virtual int open(const void* config) = 0;
	virtual int close() = 0;
	virtual void* handle() = 0;
	std::shared_ptr<sql_db> self();
	

	virtual int begin() = 0;
	virtual int end() = 0;
	virtual int rollback() = 0;
	
	virtual int clear_all();
	virtual int init_all();
	virtual int release_all();
	
	virtual int exec(const char* sql, db_cb cb, void* cb_data) = 0;
	virtual int check(int rc, enum SQL_CHECK chk) = 0;

	int  reg(std::shared_ptr<sql_exec>& ref);
	void unreg();
	const char* type() { return _type; }
	const char* id(){ return _id; }
	
protected:
	char*			_type;
	char*			_id;
	log_cb_v		_log_cb;
	void*			_config;
	int				_trans;
	sql_exec_list	_exec_list;
};

class sql_exec : public std::enable_shared_from_this<sql_exec>
{
public:
	sql_exec(const char* type, const char* id, std::shared_ptr<sql_db>& db, log_cb_v cb);
	virtual ~sql_exec();

	virtual int init(){return SQLAPI_OK;}
	virtual void release(){}
	virtual void clear(){}

	virtual int exec() { return SQLAPI_OK; };
	virtual int exec(const void* in) { return SQLAPI_OK; }
	virtual int exec(db_cb cb, void* cb_data) { return SQLAPI_OK; }
	virtual int check(enum SQL_CHECK chk);

	std::shared_ptr<sql_exec> self();
	int reg();
	void unreg();

	const char* type() { return _type; }
	const char* id() { return _id; }
	
protected:
	char*		_type;
	char*		_id;
	
	std::weak_ptr<sql_db> _db;
	log_cb_v	_log_cb;
	int			_rc;
};


class sql_cmd : public sql_exec
{
public:
	sql_cmd(const char* type, const char* id, 
				std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
				const char* tables[], int size);
	virtual ~sql_cmd();

	virtual int init();
	virtual void release();
	virtual void clear();
	
	virtual int exec();
	virtual int exec(db_cb cb, void* cb_data);

	virtual sql_data_list& result();
	
static int sql_cmd_cb(void* user, int argc, char** value, char** name);

protected:
	void do_default();
	
protected:
	char*			_sql;
	sql_data_list	_result;
};


class sql_stmt : public sql_exec
{
public:
	sql_stmt(const char* type, const char* id, 
				std::shared_ptr<sql_db>&, log_cb_v cb, const char* sql, 
				const char* tables[], int count);
	virtual ~sql_stmt();

	int* struct_offset_calc(const sql_dec_t* dec, int size, st_calc* _st);
	int struct_descript(const sql_dec_t* src_dec, int src_size,
			 			const sql_dec_t* out_dec, int out_size);
	void struct_descript_clear();
	
	virtual int init();
	virtual void release();
	virtual void clear();
	virtual void clear(void*);

	virtual int exec(const void* in) = 0;
	
	virtual stmt_data_list& result();

protected:
	void do_default();
	virtual int bind(const void* in) = 0;
	virtual int column(stmt_data_list& out) = 0;
	
protected:
	char*			_sql;
	sql_dec_t*		_in;
	sql_dec_t*		_out;
	int*			_in_offset;
	int*			_out_offset;
	int				_in_size;
	int				_out_size;
	stmt_data_list	_result;
	bool			_bdes;
};



char* sql_gen(log_cb_v cb, const char* sql, ...);
char* sql_gen(log_cb_v cb, const char* sql, va_list ap);
char* sql_gen(log_cb_v cb, const char* sql, const char* table[], int num);


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

}



#endif //__SQLAPI_H__
