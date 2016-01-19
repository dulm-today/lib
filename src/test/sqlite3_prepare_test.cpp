#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for(i=0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int main(int argc, char **argv){
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	if( argc!=3 ){
		fprintf(stdout, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
		return(1);
	}
	rc = sqlite3_open(argv[1], &db);
	if( rc ){
		fprintf(stdout, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(1);
	}
	
	const char* create_str = "create table if not exists user"
				"(id UNSIGNED BIG INT, name CHARACTER(20), password CHARACTER(20), date text,"
				"PRIMARY KEY(id))";
	const char* insert_str = "insert into user values(?,?,?,datetime('now'))";
	const char* select_str = "select * from user where id >= ?";

	rc = sqlite3_exec(db, create_str, NULL, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		fprintf(stdout, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	
	sqlite3_stmt* stmt_insert = NULL;
	sqlite3_stmt* stmt_select = NULL;
	const char* szTail = NULL;
	
	rc = sqlite3_prepare_v2(db, insert_str, strlen(insert_str), &stmt_insert, &szTail);
	if (rc != SQLITE_OK){
		fprintf(stdout, "SQL error: sqlite3_prepare_v2(%s) fail at %s\n", insert_str, szTail);
		return -1;
	}
	
	rc = sqlite3_prepare_v2(db, select_str, strlen(insert_str), &stmt_select, &szTail);
	if (rc != SQLITE_OK){
		fprintf(stdout, "SQL error: sqlite3_prepare_v2(%s) fail at %s\n", select_str, szTail);
		return -1;
	}
	
	int index = 0;
	char username[30];
	char password[30];
	
	fprintf(stdout, "begin to insert: %d\n", clock());
	
	/*sqlite 在每次commit后,都会更新文件, 造成插入性能低下. 
	  这里自定义事务,能使得只执行一次文件更新 */
	sqlite3_exec(db, "begin", NULL, 0, &zErrMsg); 
	for (;index < 100; ++index){
		sprintf(username, "user_%015d", index);
		sprintf(password, "pass_%015d", index);
		
		sqlite3_bind_int(stmt_insert, 1, index);
		sqlite3_bind_text(stmt_insert, 2, username, 20, SQLITE_STATIC);
		sqlite3_bind_text(stmt_insert, 3, password, 20, SQLITE_STATIC);

		rc = sqlite3_step(stmt_insert);
		if (rc != SQLITE_DONE){
			fprintf(stdout, "SQL error: sqlite3_step insert return %d: %s\n", rc,
					sqlite3_errmsg(db));
			return -1;
		}
		
		sqlite3_reset(stmt_insert);
	}
	sqlite3_exec(db, "commit", NULL, 0, &zErrMsg);
	
	fprintf(stdout, "end insert: %d\n", clock());
	
	fprintf(stdout, "\nbegin to select: %d\n", clock());
	for (int i = 0; i < 5; ++i){
		index = rand() % 200;
		
		sqlite3_bind_int(stmt_select, 1, index);
		
		rc = sqlite3_step(stmt_select);
		if (rc == SQLITE_ROW){
			fprintf(stdout, "select id >= %d result:\n", index);
			do{
				int col = sqlite3_column_count(stmt_select);
				if (col == 0){
					fprintf(stdout, "column == 0\n", index);
					break;
				}
				
				int id = sqlite3_column_int(stmt_select, 0);
				const unsigned char* user = sqlite3_column_text(stmt_select, 1);
				const unsigned char* passwd = sqlite3_column_text(stmt_select, 2);
				const unsigned char* date = sqlite3_column_text(stmt_select, 3);
				
				fprintf(stdout, "    %-4d %-20s %-20s %-20s\n", id, user, passwd, date);
			}while(SQLITE_ROW == (rc = sqlite3_step(stmt_select)));
			
			fprintf(stdout, "  at last sqlite3_step return %d\n", rc);
		}
		else{
			fprintf(stdout, "select id >= %d return %d, SQLITE_DONE[%d]\n", index, rc, SQLITE_DONE);
		}
		sqlite3_reset(stmt_select);
	}
	fprintf(stdout, "end select: %d\n", clock());
	
	sqlite3_finalize(stmt_insert);
	sqlite3_finalize(stmt_select);
	sqlite3_close(db);
	return 0;
}