#ifndef _LOGGINGHANDLER_H_
#define _LOGGINGHANDLER_H_

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#endif

#include <time.h>
#include <stdlib.h>
#include "RTDB.h"

#define MAXLOGCOUNT 5000000

class LoggingHandler
{
private:
	static SqliteHelper* loggerDB;
public:

#ifdef _WIN32
	DWORD thID;
	HANDLE LoggingThreadID;
#else
	pthread_t LoggingThreadID;
#endif

	LoggingHandler();
	~LoggingHandler();

	void StartThread();

	void Initialize();

#ifdef _WIN32
	static DWORD WINAPI LogValues(LPVOID arg);
#else
	static void* LogValues(void*);
#endif

	static void InsertValues(int ioa, VALUE value, time_t currentTime);
};

#endif
