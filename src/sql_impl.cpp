#include "sql_impl.h"
#include "buffer.h"
#include "struct_offset.h"
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

char* sql_gen(log_cb_v cb, char* table_out[], int table_out_size, const char* sql, va_list ap)
{
	int count = 0, size = 0, id, i;
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
	if (NULL != table_out && count > 0 && table_out_size > 0){
		memset(table_out, NULL, size * sizeof(char*));
		for (i = 0; i < count && i < table_out_size; ++i){
			table_out[i] = _strdup(table[i]);
		}
	}
	
	LOG_CB_V(cb, LOG_DEBUG, "sql result: %s", result);
	return result;
id_to_big:

	LOG_CB_V(cb, LOG_DEBUG, "table id too big(%d >= %d) at %d of %s", 
					id, SQL_TABLE_MAX, p-sql, sql);
	return NULL;	
}

static char* sql_gen2(log_cb_v cb, const char* table_in[], int table_in_size, const char* sql)
{
	const char *p;
	char *pend, *result, *p2;
	int size = 0, count = 0, i, id;
	int table_size[SQL_TABLE_MAX*2] = {0};
	bool cache_size = false;

	if (NULL == sql)
		return NULL;

	if (NULL != table_in && table_in_size <= SQL_TABLE_MAX*2){
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
				size += table_size[id] - (pend-p);
			else
				size += strlen(table_in[id]) - (pend-p);
			p = pend;
			continue;
		}
		p++;
	}

	if (0 == count)
		return (char*)sql;

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
	*p2 = 0;

	LOG_CB_V(cb, LOG_DEBUG, "sql result: %s", result);
	return result;

table_in_null:
	LOG_CB_V(cb, LOG_DEBUG, "table_in is null for sql %s", sql);
	return NULL;
	
id_too_big:
	LOG_CB_V(cb, LOG_DEBUG, "table id too big(%d >= %d) at %d of %s", 
					id, table_in_size, p-sql, sql);
	return NULL;
	
id_table_null:
	LOG_CB_V(cb, LOG_DEBUG, "table_in at %d is null at %d of %s", 
					id, p-sql, sql);
	return NULL;
}

static int sql_db_cb(void* user, int argc, char** argv, char** name)
{
	sql_db* db = (sql_db*)user;
	if (NULL == db)
		return 0;

	for (int index = 0; index < argc; ++index){
		sql_data_t data = {0};
		if (argv[index])
			data.value = _strdup(argv[index]);
		if (name[index])
			data.column = _strdup(name[index]);
		db->result().push_back(data);
	}
	return 0;
}

static int sql_err_check(sqlite3* db, int rc, enum SQL_CHECK chk)
{
	int ret = 1;
	int err = sqlite3_extended_errcode(db);

	switch(chk)
	{
		case SQL_CHECK_EXIST:
			if (SQLITE_CONSTRAINT == rc && 
				(SQLITE_CONSTRAINT_PRIMARYKEY == err 
				 || SQLITE_CONSTRAINT_UNIQUE == err))
				ret = 0;
			break;
		default:
			break;
	}

	return ret;
}

// sql_db
sql_db::sql_db(sqlite3* db /*= NULL*/, log_cb_v cb /*= NULL*/)
	:_db(db),
	_log_cb(cb),
	_trans(0)
{
}

sql_db::~sql_db()
{	
	close();
}

int sql_db::open(const char* file)
{
	if (_db)
		return 0;
	
	int rc = sqlite3_open(file, &_db);
	if (rc){
		LOG_CB_V(_log_cb, LOG_ERR, "Can't open database %s, rc=%d\n", file, rc);
		return -1;
	}

	return 0;
}

int sql_db::close()
{
	while(_trans){
		end();
	}

	free_result();
	
	if (_init.size() > 0)
		release_table();

	if (_db)
		sqlite3_close(_db);
	_db = NULL;
	
	return 0;
}

sqlite3* sql_db::handle()
{
	return _db;
}


int sql_db::exec(const char* sql, db_cb cb/*= NULL*/, void* cb_data/*= NULL*/)
{
	char* err;

	if (NULL == handle())
		return -1;

	free_result();
	
	int rc = sqlite3_exec(_db, sql, cb, cb_data, &err);
	if (rc){
		LOG_CB_V(_log_cb, LOG_ERR, "sqlite3_exec(%s) return %d: %s",
					sql, rc, err);
		sqlite3_free(err);
		return rc;
	}

	return 0;
}

int sql_db::exec(const char* sql, ...)
{
	if (NULL == handle())
		return -1;

	va_list ap;
	va_start(ap, sql);
	char* do_sql = sql_gen(_log_cb, NULL, 0, sql, ap);
	va_end(ap);
	
	if (NULL == do_sql){
		LOG_CB_V(_log_cb, LOG_DEBUG, "sql_gen %s fail", sql);
		return -1;
	}

	int rc = exec(do_sql, NULL, NULL);
	if (sql != do_sql)
		free(do_sql);
	return rc;
}

int sql_db::exec(db_cb cb, void* cb_data, const char* sql, ...)
{	
	if (NULL == handle())
		return -1;

	if (NULL == cb){
		cb = sql_db_cb;
		cb_data = this;
	}

	va_list ap;
	va_start(ap, sql);
	char* do_sql = sql_gen(_log_cb, NULL, 0, sql, ap);
	va_end(ap);
	
	if (NULL == do_sql){
		LOG_CB_V(_log_cb, LOG_DEBUG, "sql_gen %s fail", sql);
		return -1;
	}

	int rc = exec(do_sql, cb, cb_data);
	if (do_sql != sql)
		free(do_sql);
	return rc;
}

int sql_db::exec(db_cb cb, void* cb_data, const char* sql, const char* tables[], int size)
{
	if (NULL == handle())
		return -1;

	if (NULL == cb){
		cb = sql_db_cb;
		cb_data = this;
	}

	char* do_sql = sql_gen2(_log_cb, tables, size, sql);
	if (NULL == do_sql){
		LOG_CB_V(_log_cb, LOG_DEBUG, "sql_gen2 %s fail", sql);
		return -1;
	}

	int rc = exec(do_sql, cb, cb_data);
	if (do_sql != sql)
		free(do_sql);
	return rc;
}


int sql_db::begin()
{
	int rc = exec("begin", NULL, NULL);
	if (0 == rc)
		_trans++;

	return rc;
}

int sql_db::end()
{
	if (0 == _trans)
		return 0;

	int rc = exec("end", NULL, NULL);
	if (0 == rc)
		_trans--;

	return rc;
}

int sql_db::rollback()
{
	if (0 == _trans)
		return 0;

	int rc = exec("rollback", NULL, NULL);
	if (0 == rc)
		_trans--;

	return rc;
}

int sql_db::error_check(int rc, enum SQL_CHECK chk)
{
	if (NULL == handle())
		return -1;
	
	return sql_err_check(_db, rc, chk);
}

sql_data_list& sql_db::result()
{
	return _result;
}

void sql_db::free_result()
{
	for (size_t index = 0; index < _result.size(); ++index){
		sql_data_t& data = _result[index];
		if (data.column)
			free(data.column);
		if (data.value)
			free(data.value);
	}
	_result.clear();
}

int sql_db::init_table(sql_init* init, int size)
{
	int ok = 0;
	int rc;
	char buffer[2048];
	const char* p;
	char* msg;
	
	if (NULL == init || 0 == size)
		return 0;
	
	if (NULL == _db)
		return -1;

	for (int index = 0; index < size; ++index){
		if (NULL == init[index].sql){
			LOG_CB_V(_log_cb, LOG_ERR, "index[%d] sql is null", index);
			continue;
		}

		p = &buffer[0];
		if (init[index].table)
			rc = snprintf_safe(buffer, sizeof(buffer),
					init[index].sql, init[index].table);
		else
			p = init[index].sql;

		if (rc < 0){
			LOG_CB_V(_log_cb, LOG_ERR, "index[%d] sql[%s] maybe to large", index, init[index].sql);
			continue;
		}
		
		switch(init[index].init_type){
			case sql_init_exec:
				rc = sqlite3_exec(_db, p, NULL, NULL, &msg);
				if (rc){
					LOG_CB_V(_log_cb, LOG_ERR, "index[%d] exec return %d:% s", index, rc, msg);
					sqlite3_free(msg);
					continue;
				}
				break;
			case sql_init_stmt:
				if (NULL == init[index].init_data.stmt.stmt){
					LOG_CB_V(_log_cb, LOG_ERR, "index[%d] stmt pointer is null", index);
					continue;
				}
				{
					sql_stmt* _stmt = new sql_stmt(
								_db, _log_cb, init[index].sql, init[index].table, SQL_TABLE_MAX);
					if (NULL == _stmt){
						LOG_CB_V(_log_cb, LOG_ERR, "index[%d] new stmt fail", index);
						continue;
					}
					*init[index].init_data.stmt.stmt = _stmt;
					
					rc = _stmt->init(init[index].init_data.stmt.in,
								init[index].init_data.stmt.in_size,
								init[index].init_data.stmt.out,
								init[index].init_data.stmt.out_size);
					if (rc){
						LOG_CB_V(_log_cb, LOG_ERR, "index[%d] stmt init fail", index);
						continue;
					}
				}
				break;
			default:
				LOG_CB_V(_log_cb, LOG_ERR, "index[%d] unknow type[%d]", index, init[index].init_type);
				continue;
		}
		ok++;
	}

	_init.push_back(init);
	_init_size.push_back(size);
	
	return ok;
}

int sql_db::release_table(sql_init* initlist, int size)
{
	if (NULL == _db)
		return 0;

	if (NULL == initlist || 0 == size)
		return 0;

	for (int index = 0; index < size; ++index){
		sql_init* init = &initlist[index];
		switch(init->init_type){
			case sql_init_exec:
				break;
			case sql_init_stmt:
				if (NULL == init->init_data.stmt.stmt
					|| NULL == *init->init_data.stmt.stmt)
					break;
				
				delete (*init->init_data.stmt.stmt);
				*init->init_data.stmt.stmt = NULL;
				break;
			default:
				LOG_CB_V(_log_cb, LOG_ERR, "init_type %d unknow", 
							init->init_type);
				break;
		}
	}

	return 0;
}

int sql_db::release_table()
{
	int rc = 0;
	
	if (NULL == _db)
		return 0;
	
	if (_init.size() != _init_size.size())
		return -1;

	for (size_t index = 0; index < _init.size(); ++index){
		rc += release_table(_init[index], _init_size[index]);
	}

	_init.clear();
	_init_size.clear();

	return rc;
}


#define COLUMN_ARRAY(fn)		\
	buf = (buffer_t*)(outst+ext->_out_offset[index]);\
	if (SQLITE_NULL != sqlite3_column_type(_stmt, index)\
		&& 0 != (used = sqlite3_column_bytes(_stmt, index))){\
		rs = (const char*)fn(_stmt, index);\
		if (NULL != buf->get(used))\
			buf->save(rs, used);\
		else{\
			LOG_CB_V(_log_cb, LOG_ERR, "buffer_t get %d fail", used);\
			goto err_malloc;\
		}\
	}else{\
		buf->clear();\
	}

typedef struct
{
	int*	_in_offset;
	int*	_out_offset;
	st_calc	_st_calc;
}sql_impl_ext;


sql_stmt::sql_stmt(sqlite3* db, log_cb_v cb, const char* sql, ...)
	:_inited(false),
	_log_cb(cb),
	_db(db),
	_stmt(NULL),
	_sql(NULL),
	_in(NULL),
	_out(NULL),
	_in_size(0),
	_out_size(0),
	_ext(NULL)
{	
	memset(	_table, 0, sizeof(_table));
	va_list ap;
	va_start(ap, sql);
	_sql = sql_gen(cb, _table, SQL_TABLE_MAX, sql, ap);
	va_end(ap);

	if (sql == _sql)
		_sql = _strdup(sql);
}

sql_stmt::sql_stmt(sqlite3* db, log_cb_v cb, 
		const char* sql, const char* tables[], int count)
	:_inited(false),
	_log_cb(cb),
	_db(db),
	_stmt(NULL),
	_sql(NULL),
	_in(NULL),
	_out(NULL),
	_in_size(0),
	_out_size(0),
	_ext(NULL)
{
	memset(	_table, 0, sizeof(_table));
	_sql = sql_gen2(cb, tables, count, sql);

	if (sql == _sql)
		_sql = _strdup(sql);
	if (NULL != _sql && NULL != tables){
		for (int i = 0; i < count && i < SQL_TABLE_MAX; ++i){
			if (NULL != tables[i])
				_table[i] = _strdup(tables[i]);
		}
	}
}

sql_stmt::~sql_stmt()
{
	release();
	if (_sql)
		free(_sql);

	for (int i = 0; i < SQL_TABLE_MAX; ++i){
		if (_table[i])
			free(_table[i]);
	}
}

int* sql_stmt::struct_offset_calc(const sql_dec_t* dec, int size)
{
	st_calc calc = {0};
	int* offset;
	sql_impl_ext* ext = (sql_impl_ext*)_ext;

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
				offset[index] = st_calc_child(&calc, &ext->_st_calc);
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

int sql_stmt::init(const sql_dec_t* src_dec, int src_size,
		 const sql_dec_t* out_dec, int out_size)
{
	int rc;
	const char* tail;
	sql_impl_ext* ext;
	
	release();

	if (NULL == _sql){
		LOG_CB_V(_log_cb, LOG_ERR, "sql is null");
		return -1;
	}

	_ext = malloc(sizeof(sql_impl_ext));
	if (NULL == _ext){
		LOG_CB_V(_log_cb, LOG_ERR, "malloc ext %d fail", sizeof(sql_impl_ext));
		return -1;
	}
	memset(_ext, 0, sizeof(sql_impl_ext));
	ext = (sql_impl_ext*)_ext;

	st_calc_begin(&ext->_st_calc);
	st_calc_type(&ext->_st_calc, st_ptr);
	st_calc_type(&ext->_st_calc, st_int);
	st_calc_type(&ext->_st_calc, st_int);
	st_calc_type(&ext->_st_calc, st_bool);
	st_calc_end(&ext->_st_calc);
	
	if (src_dec && src_size > 0){
		_in = (sql_dec_t*)malloc(sizeof(sql_dec_t)*src_size);
		if (NULL == _in){
			LOG_CB_V(_log_cb, LOG_ERR, "malloc src_dec %d*%d fail", 
						sizeof(sql_dec_t), src_size);
			return -1;
		}
		memcpy(_in, src_dec, sizeof(sql_dec_t)*src_size);
		_in_size = src_size;
		
		ext->_in_offset = struct_offset_calc(_in, _in_size);
		if (NULL == ext->_in_offset)
			return -1;
	}

	if (out_dec && out_size > 0){
		_out = (sql_dec_t*)malloc(sizeof(sql_dec_t)*out_size);
		if (NULL == _out){
			LOG_CB_V(_log_cb, LOG_ERR, "malloc out_dec %d*%d fail", 
						sizeof(sql_dec_t), src_size);
			return -1;
		}
		memcpy(_out, out_dec, sizeof(sql_dec_t)*out_size);
		_out_size = out_size;

		ext->_out_offset = struct_offset_calc(_out, _out_size);
		if (NULL == ext->_out_offset)
			return -1;
	}

	rc = sqlite3_prepare_v2(_db, _sql, strlen(_sql), &_stmt, &tail);
	if (SQLITE_OK != rc){
		LOG_CB_V(_log_cb, LOG_ERR, "sqlite3_prepare_v(%s) fail(%d) at %s", 
					_sql, rc, tail);
		return -1;
	}

	_inited = true;
	return 0;
}

int sql_stmt::release()
{
	free_result(_result);
	
	_inited = false;

	if (_stmt) 
		sqlite3_finalize(_stmt);
	if (_out) 
		free(_out);
	if (_in) 
		free(_in);
	if (_ext){
		sql_impl_ext* ext = (sql_impl_ext*)_ext;
		if (ext->_in_offset)
			free(ext->_in_offset);
		if (ext->_out_offset)
			free(ext->_out_offset);
		free(_ext);
	}

	_stmt = NULL;
	_in = NULL;
	_out = NULL;
	_in_size = 0;
	_out_size = 0;
	_ext = NULL;

	return 0;
}

int sql_stmt::bind(const void *in)
{
	int index;
	char *inst = (char*)in;
	sql_impl_ext* ext = (sql_impl_ext*)_ext;
	buffer_t* buf;
	
	// bind
	for (index = 0; index < _in_size;++index){
		switch(_in[index].type){
			case SQL_INT:
				sqlite3_bind_int(_stmt, index+1, *(int*)(inst+ext->_in_offset[index]));
				break;
			case SQL_INT64:
				sqlite3_bind_int64(_stmt, index+1, *(sqlite3_int64*)(inst+ext->_in_offset[index]));
				break;
			case SQL_DOUBLE:
				sqlite3_bind_double(_stmt, index+1, *(double*)(inst+ext->_in_offset[index]));
				break;
			case SQL_TEXT:
				buf = (buffer_t*)(inst+ext->_in_offset[index]);
				if (0 == buf->length()){
					sqlite3_bind_null(_stmt, index+1);
				}
				else if (buf->iszero()){
					sqlite3_bind_zeroblob(_stmt, index+1, buf->length());
				}
				else{
					sqlite3_bind_text(_stmt, index+1, (const char*)buf->get(), buf->length(), SQLITE_STATIC);
				}
				break;
			case SQL_TEXT16:
				buf = (buffer_t*)(inst+ext->_in_offset[index]);
				if (0 == buf->length()){
					sqlite3_bind_null(_stmt, index+1);
				}
				else if (buf->iszero()){
					sqlite3_bind_zeroblob(_stmt, index+1, buf->length());
				}
				else{
					sqlite3_bind_text16(_stmt, index+1, buf->get(), buf->length(), SQLITE_STATIC);
				}
				break;
			case SQL_TEXT64:
				buf = (buffer_t*)(inst+ext->_in_offset[index]);
				if (0 == buf->length()){
					sqlite3_bind_null(_stmt, index+1);
				}
				else if (buf->iszero()){
					sqlite3_bind_zeroblob(_stmt, index+1, buf->length());
				}
				else{
					sqlite3_bind_text64(_stmt, index+1, (const char*)buf->get(), buf->length(), SQLITE_STATIC, 0);
				}
				break;
			case SQL_BLOB:
				buf = (buffer_t*)(inst+ext->_in_offset[index]);
				if (0 == buf->length()){
					sqlite3_bind_null(_stmt, index+1);
				}
				else if (buf->iszero()){
					sqlite3_bind_zeroblob(_stmt, index+1, buf->length());
				}
				else{
					sqlite3_bind_blob(_stmt, index+1, buf->get(), buf->length(), SQLITE_STATIC);
				}
				break;
			case SQL_BLOB64:
				buf = (buffer_t*)(inst+ext->_in_offset[index]);
				if (0 == buf->length()){
					sqlite3_bind_null(_stmt, index+1);
				}
				else if (buf->iszero()){
					sqlite3_bind_zeroblob(_stmt, index+1, buf->length());
				}
				else{
					sqlite3_bind_blob64(_stmt, index+1, buf->get(), buf->length(), SQLITE_STATIC);
				}
				break;
			default:
				LOG_CB_V(_log_cb, LOG_ERR, "SQL_TYPE[%d %s] unknow",
							_in[index].type, _in[index].name);
				return -1;
		}
	}
	
	return 0;
}

int sql_stmt::column(stmt_data_list& out)
{
	int index, rc, used;
	char *outst;
	const char *rs;
	sql_impl_ext* ext = (sql_impl_ext*)_ext;
	buffer_t* buf;
	
	do{
		rc = sqlite3_column_count(_stmt);
		if (0 == rc){
			LOG_CB_V(_log_cb, LOG_DEBUG, "status SQLITE_ROW, but column_count == 0");
			break;
		}

		outst = (char*)malloc(ext->_out_offset[_out_size]);
		if (NULL == outst){
			LOG_CB_V(_log_cb, LOG_ERR, "malloc out_offset %d fail", 
						ext->_out_offset[_out_size]);
			break;
		}
		memset(outst, 0, ext->_out_offset[_out_size]);
		
		for (index = 0; index < _out_size; ++index){
			switch(_out[index].type){
				case SQL_INT:
					*(int*)(outst+ext->_out_offset[index]) = sqlite3_column_int(_stmt, index);
					break;
				case SQL_INT64:
					*(sqlite3_int64*)(outst+ext->_out_offset[index]) = sqlite3_column_int64(_stmt, index);
					break;
				case SQL_DOUBLE:
					*(double*)(outst+ext->_out_offset[index]) = sqlite3_column_double(_stmt, index);
					break;
				case SQL_TEXT:
					COLUMN_ARRAY(sqlite3_column_text);
					break;
				case SQL_TEXT16:
					COLUMN_ARRAY(sqlite3_column_text);
					break;
				case SQL_TEXT64:
					LOG_CB_V(_log_cb, LOG_ERR, "SQL_TEXT64 is not impl");
					break;
				case SQL_BLOB:
					COLUMN_ARRAY(sqlite3_column_blob);
					break;
				case SQL_BLOB64:
					LOG_CB_V(_log_cb, LOG_ERR, "SQL_TEXT64 is not impl");
					break;
				default:
					LOG_CB_V(_log_cb, LOG_ERR, "SQL_TYPE[%d %s] unknow",
								_in[index].type, _in[index].name);
					goto err;
			}
		}
		out.push_back(outst);
		outst = NULL;
		continue;
		
err_malloc:	
err:	
		free_result(outst);
		outst = NULL;
		break;
	}while(SQLITE_ROW == (rc = sqlite3_step(_stmt)));

	return (out.size() ? 0 : -1);
}

int sql_stmt::exec(const void* in, stmt_data_list* pout/*= NULL*/)
{
	int rc;

	if (_in_size > 0 && NULL == in){
		LOG_CB_V(_log_cb, LOG_ERR, "pass in paramet is null");
		return -1;
	}
	
	if (!_inited){
		LOG_CB_V(_log_cb, LOG_ERR, "stmt is not inited");
		return -1;
	}

	free_result(_result);
	if (pout)
		free_result(*pout);
	else
		pout = &_result;

	// bind
	rc = bind(in);
	if (rc)
		goto end;

	// exec
	rc = sqlite3_step(_stmt);
	switch(rc)
	{
		case SQLITE_DONE:
			rc = 0;
			break;
		case SQLITE_ROW:
			rc = column(*pout);
			break;
		default:
			LOG_CB_V(_log_cb, LOG_ERR, "sqlite3_step return %d: %s", 
						rc, sqlite3_errmsg(_db));
			break;
	}
	
end:
	sqlite3_reset(_stmt);
	return rc;
}

int sql_stmt::error_check(int rc, enum SQL_CHECK chk)
{
	if (!_inited){
		LOG_CB_V(_log_cb, LOG_ERR, "stmt is not inited");
		return -1;
	}
	
	return sql_err_check(_db, rc, chk);
}

stmt_data_list& sql_stmt::result()
{
	return _result;
}

void sql_stmt::free_result()
{
	free_result(_result);
}


void sql_stmt::free_result(void* in)
{
	int index;
	char* st = (char*)in;
	buffer_t* buf;
	sql_impl_ext* ext = (sql_impl_ext*)_ext;

	if (_inited)
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
				buf = (buffer_t*)(st+ext->_out_offset[index]);
				buf->clear();
				break;
			default:
				break;
		}
	}
	free(in);
}

void sql_stmt::free_result(stmt_data_list& ins)
{
	int index, size = ins.size();
	
	if (!_inited)
		return;

	for (index = 0; index < size; ++index){
		if (ins[index])
			free_result(ins[index]);
	}
	ins.clear();
}

