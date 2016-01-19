#include "sqlreg.h"
#include "sqlapi.h"
#include "sqlapi_sqlite3.h"
#include "test_core.h"
#include "buffer.h"
#include <stdio.h>

using namespace sqlapi;

static LOG_V_MODULE(_log_cb_v, "sqlapi_test", g_global_log_output, 0);


static std::shared_ptr<sql_db> g_test_db;
static sql_db_config_sqlite3 g_test_db_config = {"sqlapi_test.db"};


// user create
const char* table_user_create = "create table if not exists user"
				"(id UNSIGNED BIG INT, name CHARACTER(20), password CHARACTER(20), date text,"
				"PRIMARY KEY(id))";

static std::shared_ptr<sql_exec> g_exec_user_create;

// user insert
const char* table_user_insert = "insert into user values(?,?,?,datetime('now'))";
static std::shared_ptr<sql_exec> g_exec_user_insert;

struct st_user_insert{
	int				id;
	buffer_t		name;
	buffer_t		passwd;
};

SQL_DEC_BEGIN(user, insert)
SQL_DEC(id, SQL_INT)
SQL_DEC(name, SQL_TEXT)
SQL_DEC(passwd, SQL_TEXT)
SQL_DEC_END(user, insert)


// user select
const char* table_user_search = "select * from user where id > ?";
static std::shared_ptr<sql_exec> g_exec_user_search;

struct st_user_search_in
{
	int id;
};

SQL_DEC_BEGIN(user, search_in)
SQL_DEC(id, SQL_INT)
SQL_DEC_END(user, search_in)

struct st_user_search{
	int				id;
	buffer_t		name;
	buffer_t		passwd;
	buffer_t		date;
};

SQL_DEC_BEGIN(user, search)
SQL_DEC(id, SQL_INT)
SQL_DEC(name, SQL_TEXT)
SQL_DEC(passwd, SQL_TEXT)
SQL_DEC(date, SQL_TEXT)
SQL_DEC_END(user, search)


// db_id_reg
sql_db_id_reg_t	db_id_list[] = {
	{SQL_REG_ID8("sqlite3\0"), SQL_REG_ID8_TYPE("sqlapi\0\0"), &g_test_db_config, g_test_db},
};


// stmt_type
sql_stmt_type_reg_t stmt_type_list[] = {
	{SQL_REG_ID16_TYPE("lite_user_insert000000000"), SQL_DEC_OBJ(user, insert), NULL, 0},
	{SQL_REG_ID16_TYPE("lite_user_select000000000"), SQL_DEC_OBJ(user, search_in), SQL_DEC_OBJ(user, search)},
};

// exec_id
sql_exec_id_reg_t exec_id_list[] = {
	{SQL_REG_ID8("sqlapi\0\0"), SQL_EXEC_TYPE_CMD | SQL_EXEC_TYPE_STARTUP, SQL_REG_ID16_TYPE("user_create00000"), 
			table_user_create, {"user"}, 1, g_exec_user_create },

	{SQL_REG_ID8("sqlapi\0\0"), SQL_REG_ID16("lite_user_insert000000000"), SQL_REG_ID16_TYPE("user_insert00000"), 
			table_user_insert, {"user"}, 1, g_exec_user_insert },

	{SQL_REG_ID8("sqlapi\0\0"), SQL_REG_ID16("lite_user_select000000000"), SQL_REG_ID16_TYPE("user_search00000"), 
			table_user_search, {"user"}, 1, g_exec_user_search },
};

int sqlapi_reg_init_test()
{
	sql_db_id_reg_do(db_id_list, sizeof(db_id_list)/sizeof(db_id_list[0]));
	sql_stmt_type_reg_do(stmt_type_list, sizeof(stmt_type_list)/sizeof(stmt_type_list[0]));
	sql_exec_id_reg_do(exec_id_list, sizeof(exec_id_list)/sizeof(exec_id_list[0]));

	int num, i;

	// db_type
	num = sql_db_type_reg::num();
	fprintf(stderr, "sql_db_type_reg: %d\n", num);
	for (i = 0; i < num; ++i){
		sql_db_type_reg** reg = sql_db_type_reg::get(i);
		if (NULL != reg){
			fprintf(stderr, "  %2d  0x%08x  %-32s\n",
				i, (*reg)->id(), (*reg)->type());
		}
	}

	// db_id
	num = sql_db_id_reg::num();
	fprintf(stderr, "sql_db_id_reg: %d\n", num);
	for (i = 0; i < num; ++i){
		sql_db_id_reg** reg = sql_db_id_reg::get(i);
		if (NULL != reg){
			fprintf(stderr, "  %2d  0x%08x  %-32s %-32s\n",
				i, (*reg)->id(), (*reg)->name(), (*reg)->type());
		}
	}

	// stmt_type
	num = sql_stmt_type_reg::num();
	fprintf(stderr, "sql_stmt_type_reg: %d\n", num);
	for (i = 0; i < num; ++i){
		sql_stmt_type_reg** reg = sql_stmt_type_reg::get(i);
		if (NULL != reg){
			fprintf(stderr, "  %2d  0x%08x  %-32s\n",
				i, (*reg)->id(), (*reg)->type());
		}
	}


	// exec_id
	num = sql_exec_id_reg::num();
	fprintf(stderr, "sql_exec_id_reg: %d\n", num);
	for (i = 0; i < num; ++i){
		sql_exec_id_reg** reg = sql_exec_id_reg::get(i);
		if (NULL != reg){
			fprintf(stderr, "  %2d  0x%08x  %-32s  %-32s  %-32s\n",
				i, (*reg)->id(), (*reg)->name(), (*reg)->db_type(), (*reg)->stmt_type());
		}
	}

	return 0;
}

int sqlapi_reg_release_test()
{
	sql_exec_id_reg_undo(exec_id_list, sizeof(exec_id_list)/sizeof(exec_id_list[0]));
	sql_stmt_type_reg_undo(stmt_type_list, sizeof(stmt_type_list)/sizeof(stmt_type_list[0]));
	sql_db_id_reg_undo(db_id_list, sizeof(db_id_list)/sizeof(db_id_list[0]));

	return 0;
}

int sqlapi_test()
{
	int iret;
	
	sqlapi_reg_init_test();

	sql_db_id_reg::init(_log_cb_v);
	sql_exec_id_reg::init(_log_cb_v);

	fprintf(stderr, "init finish, do sql test\n");

	// sql test
	if (g_exec_user_create.get())
		iret = g_exec_user_create->exec();

	// insert
	struct st_user_insert ins;
	g_test_db->begin();
	for (int i = 0; i < 256; ++i){
		ins.id = i+1;
		sprintf((char*)ins.name.get(20), "user_%014d", i+1);
		ins.name.save(20);

		sprintf((char*)ins.passwd.get(20), "passwd_%012d", i+1);
		ins.passwd.save(20);

		g_exec_user_insert->exec(&ins);
	}
	g_test_db->end();

	// search
	struct st_user_search_in sear;
	sear.id = rand() % 256;
	g_exec_user_search->exec(&sear);

	stmt_data_list& result = std::dynamic_pointer_cast<sql_stmt>(g_exec_user_search)->result();
	for (size_t i = 0; i < result.size(); ++i){
		st_user_search* re = (st_user_search*)result[i];
		fprintf(stderr, "    %4d  %20s  %20s %20s\n", re->id, re->name.get(), re->passwd.get(), re->date.get());
	}

	fprintf(stderr, "db and exec release starting\n");
	sql_exec_id_reg::release();
	sql_db_id_reg::release();

	fprintf(stderr, "db and exec release finish\n");
	sqlapi_reg_release_test();
	
	return 0;
}

Test _sqlapi_test_item("sqlapi", sqlapi_test, 1);
