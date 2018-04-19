#ifndef _MODBUS_MASTER_HANDLER_H_
#define _MODBUS_MASTER_HANDLER_H_

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#endif

#include "modbus.h"
#include "RTDB.h"

class ModbusMasterHandler
{
private:

	bool Initialize();
	
#ifdef _WIN32
	static DWORD WINAPI UpdapteValues(LPVOID arg);
#else
	static void* UpdapteValues(void*);
#endif
	
public:
#ifdef _WIN32
	DWORD thID;
	HANDLE ModbusMasterHandlerThreadID;
#else
	pthread_t ModbusMasterHandlerThreadID;
#endif	

	ModbusMasterHandler();
	~ModbusMasterHandler();

	void StartThread();
};


#endif
