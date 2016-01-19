#include <stdio.h>
#include <stdarg.h>
#include "test_core.h"
#include "sql_impl.h"
#include "sqlite3.h"
#include "printf_safe.h"
#include "buffer.h"

const char* sql_create = "create table if not exists user"
				"(id INTEGER, name CHARACTER(20), password CHARACTER(20), date text,"
				"PRIMARY KEY(id))";

const char* sql_insert = "replace into user values(?,?,?,datetime('now'))";
const char* sql_update = "";
const char* sql_select = "select * from user where id >= ?";
const char* sql_delete = "";
const char* sql_count = "select count(*) from user";
const char* sql_twocmd = "insert into user(name, password, date) values('hello','world',datetime('now'));select last_insert_rowid()";


static LOG_V_MODULE(sql_log_cb, "sql_impl_test", g_global_log_output, 0);

// insert
struct st_insert{
	unsigned int	id;
	buffer_t		name;
	buffer_t		passwd;
};

sql_dec_t st_insert_dec[] = {
	{"id", 		SQL_INT},
	{"name", 	SQL_TEXT},	
	{"passwd", 	SQL_TEXT},
};

// select
struct st_select_in{
	unsigned int	id;
};


//const char* name = CAT4('a', 'b', 'c', 'd');

sql_dec_t st_select_in_dec[] = {
	{"id", 		SQL_INT},
};

struct st_select_out{
	unsigned int	id;
	buffer_t		name;
	buffer_t		passwd;
	buffer_t		date;
};

sql_dec_t st_select_out_dec[] = {
	{"id", 		SQL_INT},
	{"name", 	SQL_TEXT},	
	{"passwd", 	SQL_TEXT},
	{"date", 	SQL_TEXT},	
};


// count
struct st_count_out{
	unsigned int count;
};

sql_dec_t st_count_out_dec[] = {
	{"id", 		SQL_INT}
};

int sql_test()
{
	int rc = 0, i;
	char* sz_err;
	sql_db db(NULL, sql_log_cb);

	if(db.open("sql_impl_test.db"))
		return -1;

	rc = sqlite3_exec(db.handle(), sql_create, NULL, 0, &sz_err);
	if (SQLITE_OK != rc){
		fprintf(stderr, "SQL error: %s\n", sz_err);
		goto end;
	}

	{
	sql_stmt insert(db.handle(), sql_log_cb, sql_insert);
	sql_stmt select(db.handle(), sql_log_cb, sql_select);
	sql_stmt update(db.handle(), sql_log_cb, sql_update);
	sql_stmt delet(db.handle(), sql_log_cb, sql_delete);
	sql_stmt count(db.handle(), sql_log_cb, sql_count);
	std::vector<void*> result;

	insert.init(st_insert_dec, 
				sizeof(st_insert_dec)/sizeof(st_insert_dec[0]),
				NULL,
				0);

	select.init(st_select_in_dec, 
				sizeof(st_select_in_dec)/sizeof(st_select_in_dec[0]),
				st_select_out_dec,
				sizeof(st_select_out_dec)/sizeof(st_select_out_dec[0]));

	count.init(NULL, 0, st_count_out_dec, sizeof(st_count_out_dec)/sizeof(st_count_out_dec[0]));

	char name[20];
	char passwd[20];
	st_insert ins;
	ins.name.get(20);
	ins.passwd.get(20);
	st_select_in sel_in;

	// insert
	db.begin();
	for (i = 0; i < 255; ++i){
		ins.id = i;
		sprintf(name,   "user_%014d", i+1); ins.name.save(name, 20);
		sprintf(passwd, "pass_%014d", i+1); ins.passwd.save(passwd, 20);

		rc = insert.exec(&ins,  &result);
	}
	db.end();

	// select
	sel_in.id = rand() % 255;
	rc = select.exec(&sel_in, &result);
	if (!rc){
		for (i = 0; i < result.size(); ++i){
			st_select_out* pout = (st_select_out*)result[i];
			fprintf(stderr, "%8d  %-20.*s %-20.*s %-20.*s\n",
				pout->id, 
				pout->name.length(), pout->name.get(), 
				pout->passwd.length(), pout->passwd.get(), 
				pout->date.length(), pout->date.get());
		}
	}
	select.free_result(result);

	// count
	rc = count.exec(NULL, &result);
	if (!rc){
		st_count_out* pout = (st_count_out*)result[0];
		fprintf(stderr, "count: %d   size: %d\n", pout->count, result.size());
	}
	count.free_result(result);

	// count2
	rc = db.exec(NULL, NULL, sql_count, NULL, 0);
	if (!rc && db.result().size() > 0){
		std::vector<sql_data_t>& result = db.result();
		fprintf(stderr, "count2: column[%s] value[%s]\n", result[0].column, result[0].value);
	}

	// sql_twocmd
	rc = db.exec(NULL, NULL, sql_twocmd, NULL, 0);
	if (!rc && db.result().size() > 0){
		std::vector<sql_data_t>& result = db.result();
		fprintf(stderr, "twocmd: column[%s] value[%s]\n", result[0].column, result[0].value);
	}
	}
	
end:
	return rc;
}


int sql_impl_test()
{
	int error = 0;

	if (sql_test())
		error++;
	
	return error;
}

Test sql_impl_item("sql_impl", sql_impl_test, 3);
