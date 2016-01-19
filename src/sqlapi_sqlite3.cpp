#include "sqlapi_sqlite3.h"
#include "sqlreg.h"
#include "buffer.h"

namespace sqlapi{


/* sql_db_sqlite3 */
sql_db* sql_db_sqlite3_create(const char* type, const char* id, log_cb_v cb)
{
	return new sql_db_sqlite3(type, id, cb);
}

sql_db_sqlite3::sql_db_sqlite3(const char* type, const char* id, log_cb_v cb)
	: sql_db(type, id, cb),
	_db(NULL)
{
}

sql_db_sqlite3::~sql_db_sqlite3()
{
	close();
}

int sql_db_sqlite3::open(const void* config)
{
	sql_db_config_sqlite3* cfg = (sql_db_config_sqlite3*)config;
	sql_db_config_sqlite3* _cfg;
	
	if (NULL == config)
		return SQLAPI_ERR;

	close();
	
	_config = malloc(sizeof(sql_db_config_sqlite3));
	if (NULL == _config){
		LOG_CB_V(_log_cb, LOG_ERR, "malloc %d fail", sizeof(sql_db_config_sqlite3));
		return SQLAPI_ERR;
	}
	_cfg = (sql_db_config_sqlite3*)_config;
	memcpy(_cfg, cfg, sizeof(sql_db_config_sqlite3));
	
	int rc = sqlite3_open(_cfg->file, &_db);
	if (rc){
		LOG_CB_V(_log_cb, LOG_ERR, "Can't open database %s, rc=%d\n", _cfg->file, rc);
		return SQLAPI_ERR;
	}

	return SQLAPI_OK;
}

int sql_db_sqlite3::close()
{
	while(_trans)
		end();
	
	unreg();
	
	if (_db)
		sqlite3_close(_db);
	if (_config)
		free(_config);

	_db = NULL;
	_config = NULL;
	
	return SQLAPI_OK;
}

void* sql_db_sqlite3::handle()
{
	return _db;
}

int sql_db_sqlite3::begin()
{
	int rc = exec("begin", NULL, NULL);
	if (SQLITE_OK == rc)
		++_trans;

	return rc;
}

int sql_db_sqlite3::end()
{
	if (0 == _trans)
		return SQLITE_OK;
	
	int rc = exec("end", NULL, NULL);
	if (SQLITE_OK == rc)
		--_trans;

	return rc;
}

int sql_db_sqlite3::rollback()
{
	if (0 == _trans)
		return SQLITE_OK;
	
	int rc = exec("rollback", NULL, NULL);
	if (SQLITE_OK == rc)
		--_trans;

	return rc;
}

int sql_db_sqlite3::exec(const char* sql, db_cb cb, void* cb_data)
{
	char* err;
	
	if (NULL == handle())
		return SQLAPI_ERR;

	int rc = sqlite3_exec(_db, sql, cb, cb_data, &err);
	if (rc){
		LOG_CB_V(_log_cb, LOG_ERR, "sqlite3_exec(%s) return %d: %s",
				sql, rc, err);
		sqlite3_free(err);
		return rc;
	}
	return SQLAPI_OK;
}

int sql_db_sqlite3::check(int rc, enum SQL_CHECK chk)
{
	int ret = SQLAPI_FALSE;
	
	if (NULL == handle())
		return SQLAPI_FALSE;

	int err = sqlite3_extended_errcode(_db);

	switch(chk){
		case SQL_CHECK_EXIST:
			if (SQLITE_CONSTRAINT == rc
				&& (SQLITE_CONSTRAINT_PRIMARYKEY == err
				|| SQLITE_CONSTRAINT_UNIQUE == err))
				ret = SQLAPI_TRUE;
			break;
		default:
			LOG_CB_V(_log_cb, LOG_DEBUG_ERR, "unknow sql_check %d", chk);
			break;
	}

	return ret;
}


/* sql_cmd_sqlite3 */
sql_exec* sql_cmd_sqlite3_creater(const char* type, const char* id, 
				std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
				const char* table[], int size)
{
	return new sql_cmd_sqlite3(type, id, db, cb, sql, table, size);
}

sql_cmd_sqlite3::sql_cmd_sqlite3(const char* type, const char* id, 
				std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
				const char* tables[], int size)
			: sql_cmd(type, id, db, cb, sql, tables, size)
{
}

sql_cmd_sqlite3::~sql_cmd_sqlite3()
{
}



/* sql_stmt_sqlite3 */
#define COLUMN_ARRAY(fn, fnbytes)		\
	buf = (buffer_t*)(outst+_out_offset[index]);\
	if (SQLITE_NULL != sqlite3_column_type(_stmt, index)\
		&& 0 != (used = fnbytes(_stmt, index))){\
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

sql_exec* sql_stmt_sqlite3_creater(const char* type, const char* id, 
					std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
					const char* table[], int size)
{
	return new sql_stmt_sqlite3(type, id, db, cb, sql, table, size);
}


sql_stmt_sqlite3::sql_stmt_sqlite3(const char* type, const char* id, 
					std::shared_ptr<sql_db>& db, log_cb_v cb, const char* sql, 
					const char* tables[], int size)
			: sql_stmt(type, id, db, cb, sql, tables, size),
			_stmt(NULL)
{
}

sql_stmt_sqlite3::~sql_stmt_sqlite3()
{
	release();
}

int sql_stmt_sqlite3::init()
{
	const char* tail;

	// do self
	if (NULL != _stmt)
		return SQLAPI_OK;

	if (NULL == _sql){
		LOG_CB_V(_log_cb, LOG_ERR, "sql is null");
		return SQLAPI_ERR;
	}
	if (_db.expired()){
		LOG_CB_V(_log_cb, LOG_ERR, "database weak_ptr is expired");
		return SQLAPI_ERR;
	}

	std::shared_ptr<sql_db> sp = _db.lock();
	_rc = sqlite3_prepare_v2((sqlite3*)sp->handle(), _sql, strlen(_sql), &_stmt, &tail);
	if (_rc){
		LOG_CB_V(_log_cb, LOG_ERR, "sqlite3_prepare_v2(%s) fail %d at %s, msg:%s",
						_sql, _rc, tail, sqlite3_errmsg((sqlite3*)sp->handle()));
		return SQLAPI_ERR;
	}

	// do father
	sql_stmt::init();

	return SQLAPI_OK;
}

void sql_stmt_sqlite3::release()
{
	// do self
	if (_stmt)
		sqlite3_finalize(_stmt);
	
	_stmt = NULL;

	// do father
	sql_stmt::release();
}

int sql_stmt_sqlite3::exec(const void* in)
{
	if (_in_size > 0 && NULL == in){
		LOG_CB_V(_log_cb, LOG_ERR, "pass_in paramet is null");
		return SQLAPI_ERR;
	}

	if (!_bdes){
		LOG_CB_V(_log_cb, LOG_ERR, "struct descript is not done");
		return SQLAPI_ERR;
	}

	if (_db.expired()){
		LOG_CB_V(_log_cb, LOG_ERR, "database weak_ptr is expired");
		return SQLAPI_ERR;
	}

	if(SQLAPI_OK != init()){
		LOG_CB_V(_log_cb, LOG_ERR, "stmt init failed");
		return SQLAPI_ERR;
	}

	std::shared_ptr<sql_db> sp = _db.lock();
	clear();

	// bind
	_rc = bind(in);
	if (_rc)
		goto end;

	_rc = sqlite3_step(_stmt);
	switch(_rc){
		case SQLITE_DONE:
			_rc = 0;
			break;
		case SQLITE_ROW:
			_rc = column(result());
			break;
		default:
			{
			LOG_CB_V(_log_cb, LOG_ERR, "sqlite3_step return %d: %s",
						_rc, sqlite3_errmsg((sqlite3*)sp->handle()));
			}
			break;
	}

end:
	sqlite3_reset(_stmt);
	return _rc;
}

int sql_stmt_sqlite3::bind(const void* in)
{
	int index;
	char *inst = (char*)in;
	buffer_t* buf;
	
	// bind
	for (index = 0; index < _in_size;++index){
		switch(_in[index].type){
			case SQL_INT:
				sqlite3_bind_int(_stmt, index+1, *(int*)(inst+_in_offset[index]));
				break;
			case SQL_INT64:
				sqlite3_bind_int64(_stmt, index+1, *(sqlite3_int64*)(inst+_in_offset[index]));
				break;
			case SQL_DOUBLE:
				sqlite3_bind_double(_stmt, index+1, *(double*)(inst+_in_offset[index]));
				break;
			case SQL_TEXT:
				buf = (buffer_t*)(inst+_in_offset[index]);
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
				buf = (buffer_t*)(inst+_in_offset[index]);
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
				buf = (buffer_t*)(inst+_in_offset[index]);
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
				buf = (buffer_t*)(inst+_in_offset[index]);
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
				buf = (buffer_t*)(inst+_in_offset[index]);
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
				return SQLAPI_ERR;
		}
	}
	
	return SQLAPI_OK;
}

int sql_stmt_sqlite3::column(stmt_data_list& out)
{
	int index, rc, used;
	char *outst;
	const char *rs;
	buffer_t* buf;
	
	do{
		rc = sqlite3_column_count(_stmt);
		if (0 == rc){
			LOG_CB_V(_log_cb, LOG_DEBUG, "status SQLITE_ROW, but column_count == 0");
			break;
		}

		outst = (char*)malloc(_out_offset[_out_size]);
		if (NULL == outst){
			LOG_CB_V(_log_cb, LOG_ERR, "malloc out struct memory %d fail", 
						_out_offset[_out_size]);
			break;
		}
		memset(outst, 0, _out_offset[_out_size]);
		
		for (index = 0; index < _out_size; ++index){
			switch(_out[index].type){
				case SQL_INT:
					*(int*)(outst+_out_offset[index]) = sqlite3_column_int(_stmt, index);
					break;
				case SQL_INT64:
					*(sqlite3_int64*)(outst+_out_offset[index]) = sqlite3_column_int64(_stmt, index);
					break;
				case SQL_DOUBLE:
					*(double*)(outst+_out_offset[index]) = sqlite3_column_double(_stmt, index);
					break;
				case SQL_TEXT:
					COLUMN_ARRAY(sqlite3_column_text, sqlite3_column_bytes);
					break;
				case SQL_TEXT16:
					COLUMN_ARRAY(sqlite3_column_text16, sqlite3_column_bytes16);
					break;
				case SQL_TEXT64:
					LOG_CB_V(_log_cb, LOG_ERR, "SQL_TEXT64 is not impl");
					break;
				case SQL_BLOB:
					COLUMN_ARRAY(sqlite3_column_blob, sqlite3_column_bytes);
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
		clear(outst);
		outst = NULL;
		break;
	}while(SQLITE_ROW == (_rc = sqlite3_step(_stmt)));

	return (out.size() ? SQLAPI_OK : _rc);
}

static sql_db_type_reg _sql_db_sqlite3(
					SQL_REG_ID8_TYPE(SQL_TYPE_SQLITE3), sql_db_sqlite3_create,
					sql_cmd_sqlite3_creater, sql_stmt_sqlite3_creater);

} // namespace sqlapi

