#ifndef _SQLITEHELPER_H_
#define _SQLITEHELPER_H_

#include <vector>
#include <string>
#include <stdio.h>

#include "sqlite3.h"

using namespace std;

class SqliteHelper
{
private:
	sqlite3* database;
public:
	SqliteHelper(string filename);
	~SqliteHelper();
	vector<vector<string> > SelectFromTable(string _tableName);
	bool ExecuteCommand(char* query);
	long int GetTableRowsCount(string _tableName);

};

#endif
