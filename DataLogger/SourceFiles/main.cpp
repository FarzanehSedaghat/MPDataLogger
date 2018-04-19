
#include "RTDB.h"
#include "LoggingHandler.h"
#include "ModbusMasterHandler.h"

int main(int argc, char **argv) {

	RTDB::Instance()->loadDB();

	LoggingHandler loggingHandler;
	loggingHandler.StartThread();

	ModbusMasterHandler* modbusHandler = new ModbusMasterHandler();
	modbusHandler->StartThread();

#ifdef _WIN32
	WaitForSingleObject(loggingHandler.LoggingThreadID, INFINITE);
	WaitForSingleObject(modbusHandler->ModbusMasterHandlerThreadID, INFINITE);
	CloseHandle(loggingHandler.LoggingThreadID);
	CloseHandle(modbusHandler->ModbusMasterHandlerThreadID);
#else
	pthread_join(loggingHandler.LoggingThreadID, NULL);
	pthread_join(modbusHandler->ModbusMasterHandlerThreadID, NULL);
#endif
	return 0;

}




