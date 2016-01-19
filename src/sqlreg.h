#ifndef __SQLREG_H__
#define __SQLREG_H__

#include "sqlapi.h"
#include "register.h"
#include "hash.h"

namespace sqlapi{

typedef unsigned int sql_id_t;
typedef sql_db* (*sql_db_create)(const char* type, const char* id, log_cb_v cb);
typedef sql_exec* (*sql_exec_create)(const char* type, const char* id, 
					std::shared_ptr<sql_db>& db, log_cb_v cb, 
					const char* sql, const char* table[], int size);

enum SQL_EXEC_TYPE{
	SQL_EXEC_TYPE_START = 0,
	SQL_EXEC_TYPE_CMD,
	SQL_EXEC_TYPE_END,

	SQL_EXEC_TYPE_ONCE	  = 0x40000000,
	SQL_EXEC_TYPE_STARTUP = 0x80000000,
	SQL_EXEC_TYPE_STARTUP_ONCE = 0xc0000000,
	SQL_EXEC_TYPE_MASK 	  = 0x0fffffff,
};		

// db_type
class sql_db_type_reg 
		: public registe<sql_id_t, sql_db_type_reg*>
{
public:
	sql_db_type_reg(sql_id_t id, const char* type, 
						sql_db_create db_create, 
						sql_exec_create cmd_create,
						sql_exec_create stmt_create);
	
	sql_id_t id() { return _id; }
	const char* type() { return _type; };

	sql_id_t		_id;
	const char*		_type;
	sql_db_create	_db_create;
	sql_exec_create _cmd_create;
	sql_exec_create _stmt_create;
};

// db_id
class sql_db_id_reg
		: public registe<sql_id_t, sql_db_id_reg*>
{
public:
	sql_db_id_reg(sql_id_t type_id, sql_id_t db_id, 
				const char* name, const void* cfg, std::shared_ptr<sql_db>& obj);

	sql_id_t type_id() { return _type_id; }
	const char* type() ;
	sql_db_type_reg* type_get();

	sql_id_t id() { return _id; }
	const char* name() { return _name; }

	static int init(log_cb_v cb);
	static int release();

	sql_id_t		_type_id;
	sql_id_t		_id;
	const char*		_name;
	void*			_config;
	std::shared_ptr<sql_db>& _obj;
};

// stmt_type
class sql_stmt_type_reg
		: public registe<sql_id_t, sql_stmt_type_reg*>
{
public:
	sql_stmt_type_reg(sql_id_t id, const char* type, 
				sql_dec_t* in, int in_size, sql_dec_t* out, int out_size);

	sql_id_t id() const { return _id; }
	const char* type() const { return _type; }

	sql_id_t	_id;
	const char*	_type;

	sql_dec_t*	_in;
	int			_in_size;
	sql_dec_t*	_out;
	int			_out_size;
};

// table_id
class sql_exec_id_reg
			: public registe<sql_id_t, sql_exec_id_reg*>
{
public:
	sql_exec_id_reg(sql_id_t db_id, sql_id_t type_id, sql_id_t id,
				const char* name, const char* sql, 
				const char* table[], int size, std::shared_ptr<sql_exec>& obj);

	sql_id_t    db_type_id();
	const char* db_type() ;
	sql_db_type_reg* db_type_get();

	sql_id_t	db_id() { return _db_id; }
	const char* db_name() ;
	sql_db_id_reg* db_id_get();

	sql_id_t	stmt_type_id() { return _type_id; }
	const char* stmt_type() ;
	sql_stmt_type_reg* stmt_type_get();

	sql_id_t    id() { return _id; }
	const char* name() { return _name; }

	static int init(log_cb_v cb);
	static int release();
		
	sql_id_t	_db_id;
	sql_id_t	_type_id;
	sql_id_t	_id;
	const char*	_name;
	const char*	_sql;
	const char*	_table[SQL_TABLE_MAX];
	int			_size;
	std::shared_ptr<sql_exec>&	_obj;
};


/* help for reg */
typedef struct{
	sql_id_t		_type_id;
	sql_id_t		_id;
	const char*		_name;
	void*			_config;
	std::shared_ptr<sql_db>& _obj;
	
	sql_db_id_reg*	_reg;
}sql_db_id_reg_t;


typedef struct{
	sql_id_t	_id;
	const char*	_name;

	sql_dec_t*	_in;
	int			_in_size;
	sql_dec_t*	_out;
	int			_out_size;

	sql_stmt_type_reg*	_reg;
}sql_stmt_type_reg_t;

typedef struct{
	sql_id_t	_db_id;
	sql_id_t	_type_id;
	sql_id_t	_id;
	const char*	_name;
	const char*	_sql;
	const char*	_table[SQL_TABLE_MAX];
	int			_size;
	std::shared_ptr<sql_exec>&	_obj;

	sql_exec_id_reg* _reg;
}sql_exec_id_reg_t;

int sql_db_id_reg_do(sql_db_id_reg_t* list, int size);
int sql_db_id_reg_undo(sql_db_id_reg_t* list, int size);
int sql_stmt_type_reg_do(sql_stmt_type_reg_t* list, int size);
int sql_stmt_type_reg_undo(sql_stmt_type_reg_t* list, int size);
int sql_exec_id_reg_do(sql_exec_id_reg_t* list, int size);
int sql_exec_id_reg_undo(sql_exec_id_reg_t* list, int size);

/* warning: SQL_REG_ID32 may cause the compiler error.
   because after precompiler, the text replaced by macro is too long */
#define SQL_REG_ID4(x)			(HASH4(x) & SQL_EXEC_TYPE_MASK)
#define SQL_REG_ID8(x)			(HASH8(x) & SQL_EXEC_TYPE_MASK)
#define SQL_REG_ID16(x)			(HASH16(x)& SQL_EXEC_TYPE_MASK)
#define SQL_REG_ID32(x)			(HASH32(x)& SQL_EXEC_TYPE_MASK)	

#define SQL_REG_ID4_TYPE(x)		SQL_REG_ID4(x), x
#define SQL_REG_ID8_TYPE(x)		SQL_REG_ID8(x), x
#define SQL_REG_ID16_TYPE(x)	SQL_REG_ID16(x), x
#define SQL_REG_ID32_TYPE(x)	SQL_REG_ID32(x), x

}

#endif //__SQLREG_H__
