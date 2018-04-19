#ifndef _RTDB_H_
#define _RTDB_H_

#ifndef _WIN32
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <set>


#include "SqliteHelper.h"
#include "InformationObject.h"
#include "ModbusExtension.h"
#include "Value.h"



#ifndef NULL
#define NULL 0
#endif

enum _tblModbusExtensionGroup
{
	_ModbusSlaveID_GR = 0,
	_ModbusSlaveGroupFunctionID,
	_ModbusSlaveModbusGroupID_PK,
	_ModbusSlaveGroupStartAddress,
	_ModbusSlaveGroupLenght
};

enum _tblModbusExtension
{
	_ModbusSlaveName =0,
	_ModbusSlaveID_PK,
	_ModbusTCP,
	_ModbusStopBit,
	_ModbusParity,
	_ModbusBaudRate,
	_ModbusSlavePort,
	_ModbusTCPIP,
	_ModbusTCPPort
};

enum _tblProtocolSetting
{
	_SlaveID = 0,
	_SlaveIPAddress,
	_MasterIPAddress,
	_Port,
	_CommonAddress,
	_Frequency
};

enum _tblInformationObject
{
	_IOID = 0,
	_Name,
	_ModbusSlaveGroupID,
	_ModbusSlaveID,
	_ModbusFunctionCode,
	_ModbusPointAddress,
	_SampleInterval,
	_ADType
};

class RTDB
{
private:
	static RTDB* instance;
	RTDB();

	map<int, InformationObject> objects;
	map<int, vector<int> > ObjectTypeGroups;
	set<int> UsedInputTypes;
	set<int> UsedOutputTypes;

#ifdef _WIN32
	CRITICAL_SECTION RTDBLock;
#else
	pthread_mutex_t RTDBLock;
#endif 

public:
	vector<int> LogIOAs;
	vector<ModbusExtension*> ModbusExtensions;

	static RTDB* Instance();
	~RTDB();

	void loadDB();

	bool SetValue(int ioa, VALUE value, bool isCommand = false);
	VALUE GetValue(int ioa);
	InformationObject* GetInformationObject(int ioa);
	InformationObject* GetCommandedInformationObject(int _command_ioa, int _command_type);

	vector<int> GetModbusObjectsAddress(int SlaveID, int FunctionCode);

	set<int> GetUsedInputTypes();
	set<int> GetUsedOutputTypes();

	vector<int> GetListOfObjectAddressesWithModbusAddress(int mbaddress);

	void SetAllObjectsLastSampleTimeTag();
	void SetLastSampleTimeTag(int ioa);

	void SetAllIECObjectsTimeStamp();

	bool ContainsObjectAddress(int ioa);
	bool isInputType(int type);
	bool isOutputType(int type);
};



#endif
