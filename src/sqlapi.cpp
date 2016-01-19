#include "sqlapi.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "buffer.h"
#include "assert.h"

namespace sqlapi{

/* sql_db */
sql_db::sql_db(const char* type, const char* id, log_cb_v cb)
	:_type(NULL),
	_id(NULL),
	_log_cb(cb),
	_trans(0),
	_config(NULL)
{
	if (type)
		_type = _strdup(type);
	if (id)
		_id = _strdup(id);
	
	LOG_CB_V(_log_cb, LOG_DEBUG, "Constructors: type: %s name: %s", _type, _id);
}

sql_db::~sql_db()
{
	LOG_CB_V(_log_cb, LOG_DEBUG, "Destructor:   type: %s name: %s", _type, _id);
	
	if (_type)
		free(_type);
	if (_id)
		free(_id);
}

std::shared_ptr<sql_db> sql_db::self()
{
	return shared_from_this();
}

int sql_db::clear_all()
{
	int ok = 0;
	for (size_t i = 0; i < _exec_list.size(); ++i){
		std::weak_ptr<sql_exec>& wp = _exec_list[i];
		if (wp.expired())
			continue;
		std::shared_ptr<sql_exec> sp = wp.lock();
		sp->clear();
		ok++;
	}
	return _exec_list.size() - ok;
}

int sql_db::init_all()
{
	int ok = 0;
	for (size_t i = 0; i < _exec_list.size(); ++i){
		std::weak_ptr<sql_exec>& wp = _exec_list[i];
		if (wp.expired())
			continue;
		std::shared_ptr<sql_exec> sp = wp.lock();
		sp->init();
		ok++;
	}
	return _exec_list.size() - ok;
}

int sql_db::release_all()
{
	int ok = 0;
	for (size_t i = 0; i < _exec_list.size(); ++i){
		std::weak_ptr<sql_exec>& wp = _exec_list[i];
		if (wp.expired())
			continue;
		std::shared_ptr<sql_exec> sp = wp.lock();
		sp->release();
		ok++;
	}
	return _exec_list.size() - ok;
}

int sql_db::reg(std::shared_ptr<sql_exec>& ref)
{
	if (!ref.get())
		return SQLAPI_ERR;

	_exec_list.push_back(ref);
	return SQLAPI_OK;
}

void sql_db::unreg()
{
	for (size_t i = 0; i < _exec_list.size(); ++i){
		std::weak_ptr<sql_exec>& wp = _exec_list[i];
		if (wp.expired())
			continue;
		std::shared_ptr<sql_exec> sp = wp.lock();
		sp->release();
	}
	_exec_list.clear();
}


/* sql_exec */
sql_exec::sql_exec(const char* type, const char* id, std::shared_ptr<sql_db>& db, log_cb_v cb)
	: _type(NULL),
	_id(NULL),
	_db(db),
	_log_cb(cb),
	_rc(0)
{
	if (type)
		_type = _strdup(type);
	if (id)
		_id = _strdup(id);

	LOG_CB_V(_log_cb, LOG_DEBUG, "Constructors: type: %s name: %s", _type, _id);
}

sql_exec::~sql_exec()
{
	LOG_CB_V(_log_cb, LOG_DEBUG, "Destructor:   type: %s name: %s", _type, _id);
	
	unreg();
	if (_type)
		free(_type);
	if (_id)
		free(_id);
}

int sql_exec::check(enum SQL_CHECK chk)
{
	if (_db.expired()){
		LOG_CB_V(_log_cb, LOG_ERR, "database weak_ptr is expired");
		return SQLAPI_ERR;
	}

	std::shared_ptr<sql_db> sp = _db.lock();
	return sp->check(_rc, chk); 
}

std::shared_ptr<sql_exec> sql_exec::self()
{
	return shared_from_this();
}

int sql_exec::reg()
{
	if (_db.expired()){
		LOG_CB_V(_log_cb, LOG_ERR, "database weak_ptr is expired");
		return SQLAPI_ERR;
	}
	
	std::shared_ptr<sql_db> sp = _db.lock();
	sp->reg(self());
	return SQLAPI_OK;
}

void sql_exec::unreg()
{
}


/* sql_cmd */
void sql_cmd::do_default()
{
	_sql = NULL;
}

sql_cmd::sql_cmd(const char* type, const char* id, 
					std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
					const char* tables[], int size)
	:sql_exec(type, id, db, cb)
{
	do_default();
	
	_sql = sql_gen(cb, sql, tables, size);
	if (NULL != _sql && _sql == sql)
		_sql = _strdup(sql);
}

sql_cmd::~sql_cmd()
{
	release();
	if (_sql)
		free(_sql);
}

int sql_cmd::init()
{
	// do self
	// nothing
	
	// do father
	sql_exec::init();
	
	return SQLAPI_OK;
}

void sql_cmd::release()
{
	// do self
	clear();

	// do father
	sql_exec::release();
}

void sql_cmd::clear()
{
	// do self
	for (size_t i = 0; i < _result.size(); ++i){
		sql_data_t& data = _result[i];
		if (data.column)
			free(data.column);
		if (data.value)
			free(data.value);
	}
	_result.clear();

	// do father
	sql_exec::clear();
}

int sql_cmd::exec()
{
	if (NULL == _sql){
		LOG_CB_V(_log_cb, LOG_ERR, "sql is null");
		return SQLAPI_ERR;
	}

	if (_db.expired()){
		LOG_CB_V(_log_cb, LOG_ERR, "database weak_ptr is expired");
		return SQLAPI_ERR;
	}

	clear();
	std::shared_ptr<sql_db> sp = _db.lock();
	_rc = sp->exec(_sql, sql_cmd_cb, this);
	return _rc;
}

int sql_cmd::exec(db_cb cb, void* cb_data)
{
	if (NULL == _sql){
		LOG_CB_V(_log_cb, LOG_ERR, "sql is null");
		return SQLAPI_ERR;
	}

	if (_db.expired()){
		LOG_CB_V(_log_cb, LOG_ERR, "database weak_ptr is expired");
		return SQLAPI_ERR;
	}

	clear();
	std::shared_ptr<sql_db> sp = _db.lock();
	_rc = sp->exec(_sql, cb, cb_data);
	return _rc;
}

sql_data_list& sql_cmd::result()
{
	return _result;
}

int sql_cmd::sql_cmd_cb(void* user, int argc, char** value, char** name)
{
	sql_cmd* ref = (sql_cmd*)user;
	if (NULL == ref || 0 == argc || NULL == value || NULL == name)
		return 0;

	for (int i = 0; i < argc; ++i){
		sql_data_t data = {NULL};
		if (value[i])
			data.value = _strdup(value[i]);
		if (name[i])
			data.column = _strdup(name[i]);
		ref->result().push_back(data);
	}
	return 0;
}



/* sql_stmt */
void sql_stmt::do_default()
{
	_sql = NULL;
	_in = NULL;
	_out = NULL;
	_in_offset = NULL;
	_out_offset = NULL;
	_in_size = 0;
	_out_size = 0;
	_bdes = false;
}

sql_stmt::sql_stmt(const char* type, const char* id, 
					std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
					const char* tables[], int count)
	:sql_exec(type, id, db, cb)
{
	do_default();

	_sql = sql_gen(cb, sql, tables, count);
	if (NULL != _sql && _sql == sql)
		_sql = _strdup(sql);
}

sql_stmt::~sql_stmt()
{
	release();
	
	// release struct_descript
	struct_descript_clear();

	// sql
	if (_sql)
		free(_sql);
}


int* sql_stmt::struct_offset_calc(const sql_dec_t* dec, int size, st_calc* _st_calc)
{
	st_calc calc = {0};
	int* 	offset;

	offset = (int*)malloc((size+1)*sizeof(int));
	if (NULL == offset){
		LOG_CB_V(_log_cb, LOG_ERR, "malloc %d*%d fail", size*sizeof(int));
		return NULL;
	}

	st_calc_begin(&calc);
	for (int index = 0; index < size; ++index){
		switch(dec[index].type)
		{
			case SQL_INT:
				offset[index] = st_calc_type(&calc, st_int);
				break;
			case SQL_INT64:
				offset[index] = st_calc_type(&calc, st_int64);
				break;
			case SQL_DOUBLE:
				offset[index] = st_calc_type(&calc, st_double);
				break;
			case SQL_TEXT:
			case SQL_TEXT16:
			case SQL_TEXT64:
			case SQL_BLOB:
			case SQL_BLOB64:
				offset[index] = st_calc_child(&calc, _st_calc);
				break;
			default:
				LOG_CB_V(_log_cb, LOG_ERR, "SQL_TYPE[%d %s] unknow",
						  dec[index].type, dec[index].name);
				free(offset);
				return NULL;
		}
	}
	st_calc_end(&calc);
	offset[size] = calc.size;

	return offset;
}

int sql_stmt::struct_descript(const sql_dec_t* src_dec, int src_size,
		 const sql_dec_t* out_dec, int out_size)
{
	st_calc	_st_calc;
	
	struct_descript_clear();

	if (NULL == _sql){
		LOG_CB_V(_log_cb, LOG_ERR, "sql is null");
		return SQLAPI_ERR;
	}

	st_calc_begin(&_st_calc);
	st_calc_type(&_st_calc, st_ptr);
	st_calc_type(&_st_calc, st_int);
	st_calc_type(&_st_calc, st_int);
	st_calc_type(&_st_calc, st_bool);
	st_calc_end(&_st_calc);
	
	if (src_dec && src_size > 0){
		_in = (sql_dec_t*)malloc(sizeof(sql_dec_t)*src_size);
		if (NULL == _in){
			LOG_CB_V(_log_cb, LOG_ERR, "malloc src_dec %d*%d fail", 
						sizeof(sql_dec_t), src_size);
			return SQLAPI_ERR;
		}
		memcpy(_in, src_dec, sizeof(sql_dec_t)*src_size);
		_in_size = src_size;
		
		_in_offset = struct_offset_calc(_in, _in_size, &_st_calc);
		if (NULL == _in_offset)
			return SQLAPI_ERR;
	}

	if (out_dec && out_size > 0){
		_out = (sql_dec_t*)malloc(sizeof(sql_dec_t)*out_size);
		if (NULL == _out){
			LOG_CB_V(_log_cb, LOG_ERR, "malloc out_dec %d*%d fail", 
						sizeof(sql_dec_t), src_size);
			return SQLAPI_ERR;
		}
		memcpy(_out, out_dec, sizeof(sql_dec_t)*out_size);
		_out_size = out_size;

		_out_offset = struct_offset_calc(_out, _out_size, &_st_calc);
		if (NULL == _out_offset)
			return SQLAPI_ERR;
	}

	_bdes = true;
	return SQLAPI_OK;
}

void sql_stmt::struct_descript_clear()
{
	clear();
	
	if (_in) 
		free(_in);
	if (_out) 
		free(_out);
	if (_in_offset)
		free(_in_offset);
	if (_out_offset)
		free(_out_offset);

	_in = NULL;
	_out = NULL;
	_in_offset = NULL;
	_out_offset = NULL;
	_in_size = 0;
	_out_size = 0;
	_bdes = false;
}

int sql_stmt::init()
{
	// do self
	// nothing

	// do father
	sql_exec::init();
	
	return SQLAPI_OK;
}

void sql_stmt::release()
{
	// do self
	clear();

	// do father release
	sql_exec::release();
}

void sql_stmt::clear()
{
	// do self
	for (size_t i = 0; i < _result.size(); ++i){
		if (_result[i])
			clear(_result[i]);
	}
	_result.clear();

	// do father release
	sql_exec::clear();
}

void sql_stmt::clear(void* in)
{
	int index;
	char* st = (char*)in;
	buffer_t* buf;

	if (!_bdes)
		return;
	
	for (index = 0; index < _out_size; ++index){
		switch(_out[index].type){
			case SQL_INT:
			case SQL_INT64:
			case SQL_DOUBLE:
				break;
			case SQL_TEXT:
			case SQL_TEXT16:
			case SQL_TEXT64:
			case SQL_BLOB:
			case SQL_BLOB64:
				buf = (buffer_t*)(st+_out_offset[index]);
				buf->clear();
				break;
			default:
				break;
		}
	}
	free(in);
}

stmt_data_list& sql_stmt::result()
{
	return _result;
}

char* sql_gen(log_cb_v cb, const char* sql, ...)
{
	va_list ap;
	va_start(ap, sql);
	char* new_sql = sql_gen(cb, sql, ap);
	va_end(ap);
	return new_sql;
}

char* sql_gen(log_cb_v cb, const char* sql, va_list ap)
{
	int count = 0, size = 0, id;
	char* table[SQL_TABLE_MAX] = {NULL};
	int   table_size[SQL_TABLE_MAX] = {0};
	const char *p;
	char *pend, *result, *p2;

	if (NULL == sql)
		return NULL;

	p = sql;
	while(*p){
		if (*p == '%' && *(p+1) == 's'){
			id = 0;
			if (isdigit(*(p+2)))
				id = strtol(p+2, &pend, 10);
			else
				pend = (char*)p+2;
			if (id >= SQL_TABLE_MAX)
				goto id_to_big;
			while(id >= count){
				p2 = va_arg(ap, char*);
				table[count] = p2;
				table_size[count] = strlen(p2);
				count++;
			}
			size += table_size[id];
			p = pend;
			continue;
		}
		p++;
		size++;
	}

	if (0 == count){
		result = (char*)sql;
		goto end;
	}

	size++;
	result = (char*)malloc(size);
	if (NULL == result){
		LOG_CB_V(cb, LOG_ERR, "malloc %d for sql (%s) fail", size, sql);
		return NULL;
	}

	p = sql;
	p2 = result;
	while(*p){
		if (*p == '%' && *(p+1) == 's'){
			id = 0;
			if (isdigit(*(p+2)))
				id = strtol(p+2, &pend, 10);
			else
				pend = (char*)p+2;
			strcpy(p2, table[id]);
			p2 += table_size[id];
			p = pend;
			continue;
		}
		*p2++ = *p++;
	}
	*p2++ = 0;
	assert(p2-result == size);

end:
	
	LOG_CB_V(cb, LOG_DEBUG, "sql result: %s", result);
	return result;
	
id_to_big:

	LOG_CB_V(cb, LOG_ERR, "table id too big(%d >= %d) at %d of %s", 
					id, SQL_TABLE_MAX, p-sql, sql);
	return NULL;	
}

char* sql_gen(log_cb_v cb, const char* sql, const char* table_in[], int table_in_size)
{
	const char *p;
	char *pend, *result, *p2;
	int size = 0, count = 0, i, id;
	int table_size[SQL_TABLE_MAX] = {0};
	bool cache_size = false;

	if (NULL == sql)
		return NULL;

	if (NULL != table_in && table_in_size <= SQL_TABLE_MAX){
		cache_size = true;
		for (i = 0; i < table_in_size; ++i){
			if (NULL != table_in[i])
				table_size[i] = strlen(table_in[i]);
		}
	}

	p = sql;
	while(*p){
		if (*p == '%' && *(p+1) == 's'){
			id = 0; count++;
			if (isdigit(*(p+2)))
				id = strtol(p+2, &pend, 10);
			else
				pend = (char*)p+2;
			if (NULL == table_in)
				goto table_in_null;
			if (id >= table_in_size)
				goto id_too_big;
			if (NULL == table_in[id])
				goto id_table_null;
			if (cache_size)
				size += table_size[id];
			else
				size += strlen(table_in[id]);
			p = pend;
			continue;
		}
		size++;
		p++;
	}

	if (0 == count){
		result = (char*)sql;
		goto end;
	}

	size++;
	result = (char*)malloc(size);
	if (NULL == result){
		LOG_CB_V(cb, LOG_ERR, "malloc %d for sql (%s) fail", size, sql);
		return NULL;
	}

	p = sql;
	p2 = result;
	while(*p){
		if (*p == '%' && *(p+1) == 's'){
			id = 0;
			if (isdigit(*(p+2)))
				id = strtol(p+2, &pend, 10);
			else
				pend = (char*)p+2;
			strcpy(p2, table_in[id]);
			if (cache_size)
				p2 += table_size[id];
			else
				p2 += strlen(table_in[id]);
			p = pend;
			continue;
		}
		*p2++ = *p++;
	}
	*p2++ = 0;
	assert(p2 - result == size);

end:
	LOG_CB_V(cb, LOG_DEBUG, "sql result: %s", result);
	return result;

table_in_null:
	LOG_CB_V(cb, LOG_ERR, "table_in is null for sql %s", sql);
	return NULL;
	
id_too_big:
	LOG_CB_V(cb, LOG_ERR, "table id too big(%d >= %d) at %d of %s", 
					id, table_in_size, p-sql, sql);
	return NULL;
	
id_table_null:
	LOG_CB_V(cb, LOG_ERR, "table_in at %d is null at %d of %s", 
					id, p-sql, sql);
	return NULL;
}

}