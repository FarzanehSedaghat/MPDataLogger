
#include <string>
#include <stdlib.h>
#include "LoggingHandler.h"

SqliteHelper* LoggingHandler::loggerDB = NULL;

LoggingHandler::LoggingHandler() {

}

void LoggingHandler::StartThread(void )
{
	Initialize();
	int err = 0;
#ifdef _WIN32
	LoggingThreadID = CreateThread(NULL, 0, &LoggingHandler::LogValues, NULL, 0, &thID);
#else
	 err = pthread_create(&LoggingThreadID, NULL, &LoggingHandler::LogValues, NULL);
#endif
	if ( err < 0) {
		fprintf(stderr, "Error creating thread");
		return;
	}


}

LoggingHandler::~LoggingHandler()
{

}

void LoggingHandler::Initialize()
{
	//create database file
	loggerDB = new SqliteHelper( "logger.sqlite" );

	//create tables
	//char sqlcmd[] = "CREATE TABLE IF NOT EXISTS tblLOG (LOGID INTEGER PRIMARY KEY, IOA INT, VALUE FLOAT, TIMETAG BIGINT)";
	char sqlcmd[] = "CREATE TABLE IF NOT EXISTS tblLOG (LOGID INTEGER PRIMARY KEY, IOA INT, VALUE FLOAT, TIMETAG BIGINT)";

	loggerDB->ExecuteCommand(sqlcmd);
}


#ifdef _WIN32
DWORD WINAPI LoggingHandler::LogValues(LPVOID arg)
#else
static
void* LoggingHandler::LogValues(void * arg)
#endif		
{
	sleep(3);

	time_t currentTime;
	time(&currentTime);

	for(vector<int>::iterator it = RTDB::Instance()->LogIOAs.begin(); it != RTDB::Instance()->LogIOAs.end(); ++it)
	{
		LoggingHandler::InsertValues((*it), RTDB::Instance()->GetValue(*it), currentTime);
	}
	RTDB::Instance()->SetAllObjectsLastSampleTimeTag();

	while(1)
	{
		loggerDB->ExecuteCommand("BEGIN IMMEDIATE TRANSACTION");

		time(&currentTime);
		for(vector<int>::iterator it = RTDB::Instance()->LogIOAs.begin(); it != RTDB::Instance()->LogIOAs.end(); ++it)
		{
			if(currentTime - RTDB::Instance()->GetInformationObject(*it)->LastSampleTimeTag >=
					RTDB::Instance()->GetInformationObject(*it)->GetSampleInterval())
			{
				LoggingHandler::InsertValues((*it), RTDB::Instance()->GetValue(*it), currentTime);
				RTDB::Instance()->SetLastSampleTimeTag(*it);
			}
		}

		long int cnt = loggerDB->GetTableRowsCount("tblLOG");

		if(cnt > MAXLOGCOUNT)
		{
			long int dif = cnt - MAXLOGCOUNT;
			char buffer[33];
			snprintf(buffer, sizeof(buffer), "%d", dif);
			string q = "delete from tblLOG where rowid IN (select rowid from tblLOG limit " + string(buffer) + ")";
			loggerDB->ExecuteCommand((char *) q.c_str());
		}

		loggerDB->ExecuteCommand("COMMIT TRANSACTION");
		usleep(100000);
	}
	return NULL;
}

void LoggingHandler::InsertValues(int ioa, VALUE value, time_t currentTime)
{
	char* sqlcmd = new char[255];
	sprintf(sqlcmd, "INSERT INTO tblLOG (IOA, VALUE, TIMETAG) VALUES( %d, %f, %ld)",
			ioa, value.REAL, currentTime  );

	loggerDB->ExecuteCommand(sqlcmd);
}

