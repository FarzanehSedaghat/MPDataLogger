#ifndef _MODBUS_EXTENSION_
#define _MODBUS_EXTENSION_

#include "ModbusExtensionGroup.h"
#include "modbus.h"

class ModbusExtension
{
public:
	char* ModbusSlaveName;
	char* ModbusSlavePort;
	bool isTCP;
	int ModbusSlaveID;
	int BaudRate;
	int Parity;
	int StopBit;
	string TCPIP;
	int TCPPort;
	modbus_t *ModbusMasterContext;

	vector<int> AIs;
	vector<int> DIs;
	vector<int> DOs;

	vector<ModbusExtensionGroup*> DataGroups;

	ModbusExtension()
	{

	}

};

#endif
