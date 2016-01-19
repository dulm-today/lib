#include "sqlreg.h"

namespace sqlapi{

// db_type
sql_db_type_reg::sql_db_type_reg(sql_id_t id, const char* type, 
				sql_db_create db_create, 
				sql_exec_create cmd_create,
				sql_exec_create stmt_create)
	: registe<sql_id_t, sql_db_type_reg*>(id, this),
	_id(id),
	_type(type),
	_db_create(db_create),
	_cmd_create(cmd_create),
	_stmt_create(stmt_create)
{
}



// db_id
sql_db_id_reg::sql_db_id_reg(sql_id_t type_id, sql_id_t db_id, 
			const char* name, const void* cfg, std::shared_ptr<sql_db>& obj)
		: registe<sql_id_t, sql_db_id_reg*>(db_id, this),
		_type_id(type_id),
		_id(db_id),
		_name(name),
		_config((void*)cfg),
		_obj(obj)
{
}


const char* sql_db_id_reg::type()
{
	sql_db_type_reg* reg = type_get();
	if (NULL == reg)
		return NULL;
	return reg->type();
}

sql_db_type_reg* 
			sql_db_id_reg::type_get()
{
	sql_db_type_reg** type_reg = sql_db_type_reg::search(type_id());
	if (NULL == type_reg)
		return NULL;
	return *type_reg;
}

int sql_db_id_reg::init(log_cb_v cb)
{
	int i, ok = 0;
	int num = sql_db_id_reg::num();
	sql_db_id_reg** id_pptr;
	sql_db_id_reg*  id_ptr;
	sql_db_type_reg* type_ptr;
	sql_db* db;
	
	for (i = 0; i < num; ++i){
		// get id
		id_pptr = sql_db_id_reg::get(i);
		if (NULL == id_pptr || NULL == *id_pptr){
			LOG_CB_V(cb, LOG_ERR, "sql_db_id_reg::get %d fail", i);
			continue;
		}
		id_ptr = *id_pptr;

		// search type
		type_ptr = id_ptr->type_get();
		if (NULL == type_ptr){
			LOG_CB_V(cb, LOG_ERR, "sql_db_type_reg::search %d fail, when %d %s", 
						id_ptr->type_id(), id_ptr->id(), id_ptr->name());
			continue;
		}

		if (NULL == type_ptr->_db_create){
			LOG_CB_V(cb, LOG_ERR, "%s's database create function is null, when %d %s", 
						type_ptr->type(), id_ptr->id(), id_ptr->name());
			continue;
		}

		// create object by type
		db = type_ptr->_db_create(type_ptr->type(), id_ptr->name(), cb);
		if (NULL == db){
			LOG_CB_V(cb, LOG_ERR, "db_creater %s %s fail",
							type_ptr->type(), id_ptr->name());
			continue;
		}
		id_ptr->_obj.reset(db);
		
		// open
		int rc = db->open(id_ptr->_config);
		if (rc){
			LOG_CB_V(cb, LOG_ERR, "open database %s::%s fail",
							type_ptr->type(), id_ptr->name());
			id_ptr->_obj.reset();
			continue;
		}
		ok++;
	}

	return sql_db_id_reg::num() - ok;
}

int sql_db_id_reg::release()
{
	int i, ok = 0;
	int num = sql_db_id_reg::num();
	for (i = 0; i < num; ++i){
		// get id
		sql_db_id_reg** id_reg = sql_db_id_reg::get(i);
		if (NULL == id_reg || NULL == *id_reg){
			continue;
		}

		// reset
		(*id_reg)->_obj.reset();
		ok++;
	}

	return ok;
}



// stmt_type
sql_stmt_type_reg::sql_stmt_type_reg(sql_id_t id, const char* type, 
			sql_dec_t* in, int in_size, sql_dec_t* out, int out_size)
		: registe<sql_id_t, sql_stmt_type_reg*>(id, this),
		_id(id),
		_type(type),
		_in(in),
		_in_size(in_size),
		_out(out),
		_out_size(out_size)
{
}


// table_id
sql_exec_id_reg::sql_exec_id_reg(sql_id_t db_id, sql_id_t type_id, sql_id_t id,
			const char* name, const char* sql, 
			const char* table[], int size, std::shared_ptr<sql_exec>& obj)
		: registe<sql_id_t, sql_exec_id_reg*>(id, this),
		_db_id(db_id),
		_type_id(type_id),
		_id(id),
		_name((char*)name),
		_sql((char*)sql),
		_size(size),
		_obj(obj)
{
	int i;
	sql_exec_id_reg::sort = false;
	
	if (NULL == table)
		return;
	
	for (i = 0; i < size && i < SQL_TABLE_MAX; ++i){
		_table[i] = (char*)table[i];
	}

	while(i < SQL_TABLE_MAX)
		_table[i++] = NULL;
}

sql_id_t    sql_exec_id_reg::db_type_id()
{
	sql_db_id_reg* reg = db_id_get();
	if (NULL == reg)
		return NULL;
	return reg->type_id();
}

const char* sql_exec_id_reg::db_type()
{
	sql_db_id_reg* reg = db_id_get();
	if (NULL == reg)
		return NULL;
	return reg->type();
}

sql_db_type_reg* 
			sql_exec_id_reg::db_type_get()
{
	sql_db_id_reg* reg = db_id_get();
	if (NULL == reg)
		return NULL;
	return reg->type_get();
}

const char* sql_exec_id_reg::db_name()
{
	sql_db_id_reg* reg = db_id_get();
	if (NULL == reg)
		return NULL;
	return reg->name();
}

sql_db_id_reg* 
			sql_exec_id_reg::db_id_get()
{
	sql_db_id_reg** reg = sql_db_id_reg::search(_db_id);
	if (NULL == reg)
		return NULL;
	return *reg;
}

const char* sql_exec_id_reg::stmt_type()
{
	sql_stmt_type_reg* reg = stmt_type_get();
	if (NULL == reg)
		return NULL;
	return reg->type();
}

sql_stmt_type_reg* 
			sql_exec_id_reg::stmt_type_get()
{
	sql_stmt_type_reg** reg = sql_stmt_type_reg::search(_type_id & SQL_EXEC_TYPE_MASK);
	if (NULL == reg)
		return NULL;

	return *reg;
}


int sql_exec_id_reg::init(log_cb_v cb)
{
	int i, rc, ok = 0;
	int num = sql_exec_id_reg::num();
	sql_exec_id_reg** exec_id_pptr;
	sql_exec_id_reg*  exec_id_ptr;

	sql_db_id_reg**   db_id_pptr;
	sql_db_id_reg*    db_id_ptr;

	sql_db_type_reg*  db_type_ptr;
	sql_stmt_type_reg* exec_type_ptr;
	sql_exec_create   function;
	sql_exec* exec;
	
	for (i = 0; i < num; ++i){
		// get exec_id
		exec_id_pptr = sql_exec_id_reg::get(i);
		if (NULL == exec_id_pptr || NULL == *exec_id_pptr){
			LOG_CB_V(cb, LOG_ERR, "sql_exec_id_reg::get %d fail", i);
			continue;
		}
		exec_id_ptr = *exec_id_pptr;

		// get db_id
		db_id_pptr = sql_db_id_reg::search(exec_id_ptr->db_id());
		if (NULL == db_id_pptr || NULL == *db_id_pptr){
			LOG_CB_V(cb, LOG_ERR, "sql_db_id_reg::search %d fail, when %d %s", 
						exec_id_ptr->db_id(), exec_id_ptr->id(), exec_id_ptr->name());
			continue;
		}
		db_id_ptr = *db_id_pptr;

		// check db
		if (NULL == db_id_ptr->_obj.get()){
			LOG_CB_V(cb, LOG_ERR, "database shared_ptr is null %s, when %d %s",
						db_id_ptr->name(), exec_id_ptr->id(), exec_id_ptr->name());
			continue;
		}

		// check db is open
		if (NULL == db_id_ptr->_obj->handle()){
			LOG_CB_V(cb, LOG_ERR, "database is close %s, when %d %s",
						db_id_ptr->name(), exec_id_ptr->id(), exec_id_ptr->name());
			continue;
		}

		// get db_type
		db_type_ptr = db_id_ptr->type_get();
		if (NULL == db_type_ptr){
			LOG_CB_V(cb, LOG_ERR, "sql_db_id_reg::type_get %d fail, when %d %s", 
						db_id_ptr->type_id(), exec_id_ptr->id(), exec_id_ptr->name());
			continue;
		}

		// get exec_type
		exec_type_ptr = exec_id_ptr->stmt_type_get();
		if (NULL == exec_type_ptr){
			LOG_CB_V(cb, LOG_ERR, "sql_exec_id_reg::stmt_type_get %d fail, when %d %s", 
						exec_id_ptr->stmt_type_id(), exec_id_ptr->id(), exec_id_ptr->name());
			continue;
		}

		// create object by type
		unsigned int id = exec_id_ptr->_type_id & SQL_EXEC_TYPE_MASK;
		if (id > SQL_EXEC_TYPE_START  
			&& id < SQL_EXEC_TYPE_END)
			function = db_type_ptr->_cmd_create;
		else
			function = db_type_ptr->_stmt_create;

		if (NULL == function){
			LOG_CB_V(cb, LOG_ERR, "%s's exec create function is null, when %d %s",
						db_type_ptr->type(), exec_id_ptr->id(), exec_id_ptr->name());
			continue;
		}

		exec = function(exec_type_ptr->type(),
						exec_id_ptr->name(),
						db_id_ptr->_obj,
						cb,
						exec_id_ptr->_sql,
						exec_id_ptr->_table,
						exec_id_ptr->_size);
		
		if (NULL == exec){
			LOG_CB_V(cb, LOG_ERR, "exec_create %s::%s %s fail, when %d %s",
							db_type_ptr->type(), db_id_ptr->name(), exec_type_ptr->type(),
							exec_id_ptr->id(), exec_id_ptr->name());
			continue;
		}
		exec_id_ptr->_obj.reset(exec);
		exec_id_ptr->_obj->reg();

		// init exec
		rc = exec->init();
		if (rc){
			LOG_CB_V(cb, LOG_ERR, "sql_exec init fail, when %d %s",
							exec_id_ptr->id(), exec_id_ptr->name());
			goto fail;
		}

		// struct descript
		if (id > SQL_EXEC_TYPE_END){
			rc = dynamic_cast<sql_stmt*>(exec)->struct_descript(
						exec_type_ptr->_in, exec_type_ptr->_in_size,
						exec_type_ptr->_out, exec_type_ptr->_out_size);
			if (rc){
				LOG_CB_V(cb, LOG_ERR, "sql_exec set struct_descript fail, when %d %s",
							exec_id_ptr->id(), exec_id_ptr->name());
				goto fail;
			}
		}

		// run when start
		if (exec_id_ptr->_type_id & SQL_EXEC_TYPE_STARTUP){
			rc = exec->exec();
			if (rc){
				LOG_CB_V(cb, LOG_ERR, "sql_exec run  fail %d, when %d %s",
							rc, exec_id_ptr->id(), exec_id_ptr->name());
			}
		}

		// only run once
		if (exec_id_ptr->_type_id & SQL_EXEC_TYPE_ONCE){
			exec_id_ptr->_obj.reset();
		}
		ok++;
		continue;
fail:
		exec_id_ptr->_obj.reset();
	}

	return sql_exec_id_reg::num() - ok;
}

int sql_exec_id_reg::release()
{
	int i, ok = 0;
	int num = sql_exec_id_reg::num();
	for (i = 0; i < num; ++i){
		// get id
		sql_exec_id_reg** id_reg = sql_exec_id_reg::get(i);
		if (NULL == id_reg || NULL == *id_reg){
			continue;
		}
		sql_exec_id_reg& id_ref = **id_reg;

		// reset
		if (id_ref._obj.get())
			id_ref._obj.reset();
		ok++;
	}

	return ok;
}

static sql_stmt_type_reg _stmt_type_cmd(SQL_EXEC_TYPE_CMD, "sql_cmd", NULL, 0, NULL, 0);


int sql_db_id_reg_do(sql_db_id_reg_t* list, int size)
{
	int i;
	if (NULL == list || 0 == size)
		return 0;

	for (i = 0; i < size; ++i){
		list[i]._reg = new sql_db_id_reg(list[i]._type_id, 
						  list[i]._id,
						  list[i]._name,
						  list[i]._config,
						  list[i]._obj);
	}
	return 0;
}

int sql_db_id_reg_undo(sql_db_id_reg_t* list, int size)
{
	int i;
	if (NULL == list || 0 == size)
		return 0;

	for (i = 0; i < size; ++i){
		if (list[i]._reg){
			delete list[i]._reg;
			list[i]._reg = NULL;
		}
	}
	return 0;
}

int sql_stmt_type_reg_do(sql_stmt_type_reg_t* list, int size)
{
	int i;
	if (NULL == list || 0 == size)
		return 0;

	for (i = 0; i < size; ++i){
		list[i]._reg = new sql_stmt_type_reg(
						  list[i]._id, 
						  list[i]._name,
						  list[i]._in,
						  list[i]._in_size,
						  list[i]._out,
						  list[i]._out_size);
	}
	return 0;
}

int sql_stmt_type_reg_undo(sql_stmt_type_reg_t* list, int size)
{
	int i;
	if (NULL == list || 0 == size)
		return 0;

	for (i = 0; i < size; ++i){
		if(list[i]._reg){
			delete list[i]._reg;
			list[i]._reg = NULL;
		}
	}
	return i;
}

int sql_exec_id_reg_do(sql_exec_id_reg_t* list, int size)
{
	int i;
	if (NULL == list || 0 == size)
		return 0;

	for (i = 0; i < size; ++i){
		list[i]._reg = new sql_exec_id_reg(
						  list[i]._db_id, 
						  list[i]._type_id,
						  list[i]._id,
						  list[i]._name,
						  list[i]._sql,
						  list[i]._table,
						  list[i]._size,
						  list[i]._obj);
	}
	return 0;
}

int sql_exec_id_reg_undo(sql_exec_id_reg_t* list, int size)
{
	int i;
	if (NULL == list || 0 == size)
		return 0;

	for (i = 0; i < size; ++i){
		if(list[i]._reg){
			delete list[i]._reg;
			list[i]._reg = NULL;
		}
	}
	return 0;
}

}
