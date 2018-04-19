#ifndef _MODBUS_EXTENSION_GROUP_
#define _MODBUS_EXTENSION_GROUP_

class ModbusExtensionGroup
{
public:
	int ModbusSlaveID;
	int GroupFunction;
	int ModbusGroupID;
	int StartAddress;
	int Lenght;

	ModbusExtensionGroup()
	{
		ModbusSlaveID = 0;
		GroupFunction = 0;
		ModbusGroupID = 0;
		StartAddress = 0;
		Lenght = 0;
	}

};

#endif
