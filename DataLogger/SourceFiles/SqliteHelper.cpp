#include <stdlib.h>
#include "SqliteHelper.h"

SqliteHelper::SqliteHelper(string filename)
{
	database = NULL;
	if( sqlite3_open(filename.c_str(), &database) != SQLITE_OK )
		fprintf(stderr, "Error opening database...\n");
}

SqliteHelper::~SqliteHelper()
{
	if(database != NULL)
	{
		if ( sqlite3_close(database) != SQLITE_OK )
			fprintf(stderr, "Error closing database...\n");
	}
}

vector<vector<string> > SqliteHelper::SelectFromTable(string _tableName)
{
	string select_query = "SELECT * from " + _tableName;
	vector<vector<string> > table_data;
	sqlite3_stmt *statement;

	int result = 0;
	result = sqlite3_prepare_v2(database, (char*) select_query.c_str(), -1, &statement, 0);

	if (result == SQLITE_OK) {
		int NumberOfColumns = sqlite3_column_count(statement);

		while (true) {
			result = sqlite3_step(statement);
			if (result == SQLITE_ROW) {
				vector<string> tempRow;
				for (int col = 0; col < NumberOfColumns; col++) {
					 char* valstr = (char*) sqlite3_column_text(statement, col);
					 tempRow.push_back((valstr == NULL)? "": valstr);
				}
				table_data.push_back(tempRow);
			} else if (result == SQLITE_DONE) {
				break;
			} else {
				fprintf(stderr, "%s %s\n", (char*) select_query.c_str(), sqlite3_errmsg(database));
				break;
			}
		}
		sqlite3_finalize(statement);
	}
	else
	{
		printf("ERROR: cannot select from table %s\n", _tableName.c_str());
	}

	return table_data;
}


bool SqliteHelper::ExecuteCommand(char* query)
{
	bool res = false;
	sqlite3_stmt* statement;
	if(sqlite3_prepare_v2(database, query, -1, &statement, 0) == SQLITE_OK)
	{
		int result = sqlite3_step(statement);
		if(result == SQLITE_DONE)
		{
			res = true;
		}
		sqlite3_finalize(statement);
	}

	return res;
}

static int callback(void *count, int argc, char **argv, char **azColName) {
    long int *c = (long int*) count;
    *c = atol(argv[0]);
    return 0;
}

long int SqliteHelper::GetTableRowsCount(string _tableName)
{
	char *zErrMsg = 0;
	long int cnt = 0;
	string select_query = "SELECT count(*) from " + _tableName;
	sqlite3_exec(database, (char*) select_query.c_str(), callback, &cnt, &zErrMsg);
	return cnt;
}
