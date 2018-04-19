#include <string>
#include "RTDB.h"

RTDB* RTDB::instance = NULL;

RTDB::RTDB()
{

}

RTDB* RTDB::Instance()
{
	if(instance == NULL)
		instance = new RTDB();
	return instance;
}

void RTDB::loadDB()
{
	vector<vector<string> > table_setting;
	vector<vector<string> > table_data;
	vector<vector<string> > table_modbus_extension;
	vector<vector<string> > table_modbus_group;

	SqliteHelper* configdb = new SqliteHelper("ConfigDB.conf");

	table_modbus_extension = configdb->SelectFromTable("tblModbusExtension");

	for(vector<vector<string> >::iterator it = table_modbus_extension.begin(); it != table_modbus_extension.end(); ++it)
	{
		ModbusExtension* extension = new ModbusExtension();
		extension->ModbusSlaveID = atoi((*it)[_ModbusSlaveID_PK].c_str());

		string name = (*it)[_ModbusSlaveName];
		extension->ModbusSlaveName = new char[name.size()];
		sprintf(extension->ModbusSlaveName, "%s", name.c_str());

		extension->isTCP = atoi((*it)[_ModbusTCP].c_str());

		if (extension->isTCP == 0)
		{
			string port = (*it)[_ModbusSlavePort];
			extension->ModbusSlavePort = new char[port.size()];
			sprintf(extension->ModbusSlavePort, "%s", port.c_str());

			extension->BaudRate = atoi((*it)[_ModbusBaudRate].c_str());
			extension->Parity = atoi((*it)[_ModbusParity].c_str());
			extension->StopBit = atoi((*it)[_ModbusStopBit].c_str());
		}
		else
		{
			extension->TCPIP = (*it)[_ModbusTCPIP];
			extension->TCPPort = atoi((*it)[_ModbusTCPPort].c_str());
		}

		ModbusExtensions.push_back(extension);
	}

	table_modbus_group = configdb->SelectFromTable("tblModbusGroup");

	for(vector<ModbusExtension*>::iterator extension = ModbusExtensions.begin(); extension != ModbusExtensions.end(); ++extension)
	{
		for(vector<vector<string> >::iterator git = table_modbus_group.begin(); git != table_modbus_group.end(); ++git)
		{
			int slaveid = atoi((*git)[_ModbusSlaveID_GR].c_str());
			if(slaveid == (*extension)->ModbusSlaveID)
			{
				ModbusExtensionGroup* modbusGroup = new ModbusExtensionGroup();
				modbusGroup->ModbusSlaveID = slaveid;
				modbusGroup->ModbusGroupID = atoi((*git)[_ModbusSlaveModbusGroupID_PK].c_str());
				modbusGroup->GroupFunction = atoi((*git)[_ModbusSlaveGroupFunctionID].c_str());
				modbusGroup->StartAddress = atoi((*git)[_ModbusSlaveGroupStartAddress].c_str());
				modbusGroup->Lenght = atoi((*git)[_ModbusSlaveGroupLenght].c_str());

				fprintf(stderr, "mbgrid = %d\n", modbusGroup->ModbusGroupID);

				(*extension)->DataGroups.push_back(modbusGroup);
			}
		}
	}

	table_data = configdb->SelectFromTable("tblInformationObject");

	for(vector<vector<string> >::iterator it = table_data.begin(); it != table_data.end(); ++it)
	{
		int AD = atoi((*it)[_ADType].c_str());
		int interval = atoi((*it)[_SampleInterval].c_str());
		int ModbusSlaveID = atoi((*it)[_ModbusSlaveID].c_str());
		int ModbusFunctionCode = atoi((*it)[_ModbusFunctionCode].c_str());
		int ModbusPointAddress = atoi((*it)[_ModbusPointAddress].c_str());
		int ModbusSlaveGroupID = atoi((*it)[_ModbusSlaveGroupID].c_str());

		InformationObject* obj = new InformationObject(AD, interval,
				ModbusSlaveID, ModbusFunctionCode, ModbusPointAddress, ModbusSlaveGroupID);
		objects.insert( std::pair<int, InformationObject>(ModbusPointAddress, *obj) );

		if(isInputType(ModbusFunctionCode))
		{
			UsedInputTypes.insert(ModbusFunctionCode);
		}
		else if(isOutputType(ModbusFunctionCode))
		{
			UsedOutputTypes.insert(ModbusFunctionCode);
		}


		if(ObjectTypeGroups.find(ModbusFunctionCode) == ObjectTypeGroups.end())
		{
			vector<int> tmp;
			tmp.push_back(ModbusPointAddress);
			ObjectTypeGroups.insert ( std::pair<int, vector<int> > (ModbusFunctionCode, tmp));
		}
		else //if type is already added to map, we just push back the object to its group
		{
			ObjectTypeGroups.at(ModbusFunctionCode).push_back(ModbusPointAddress);
		}

		if(interval > 0)
		{
			LogIOAs.push_back(ModbusPointAddress);
		}
	}

	for(map<int, InformationObject>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		printf("ioa = %d , type = %d\n", (*it).second.GetModbusPointAddress(), (*it).second.GetModbusFunctionCode());
	}
	printf("UsedTypes: ");
	for(set<int>::iterator it = UsedInputTypes.begin(); it != UsedInputTypes.end(); ++it)
	{
		printf( " %d, ", *it);
	}
	printf("\n");
}

/*
 * This function must be called from MODBUS thread to fill the real time data
 */
bool RTDB::SetValue(int ioa, VALUE value, bool isCommand)
{
	bool succeed = false;
	if(objects.find(ioa) != objects.end())
	{
		if(isCommand)
		{
			printf("Going to apply change to database object addr:%u \n", ioa);
#ifdef _WIN32
			EnterCriticalSection(&RTDBLock);
#else
			pthread_mutex_lock(&RTDBLock);
#endif
			objects.at(ioa).ApplyCommand(value);
#ifdef _WIN32
			LeaveCriticalSection(&RTDBLock);
#else
			pthread_mutex_unlock(&RTDBLock);
#endif
			succeed = true;
		}
		else if( value != objects.at(ioa).GetValue() )
		{
#ifdef _WIN32
			EnterCriticalSection(&RTDBLock);
#else
			pthread_mutex_lock(&RTDBLock);
#endif
			objects.at(ioa).SetValue(value);
#ifdef _WIN32
			LeaveCriticalSection(&RTDBLock);
#else
			pthread_mutex_unlock(&RTDBLock);
#endif
			succeed = true;
		}

	}
	return succeed;
}

/*
 * This function must be called from LOGGER thread to read the real time data
 */
VALUE RTDB::GetValue(int ioa)
{
	VALUE val;
	val.REAL = 0;
	if(objects.find(ioa) != objects.end())
	{
#ifdef _WIN32
		EnterCriticalSection(&RTDBLock);
#else
		pthread_mutex_lock(&RTDBLock);
#endif
		val = objects.at(ioa).GetValue();
#ifdef _WIN32
		LeaveCriticalSection(&RTDBLock);
#else
		pthread_mutex_unlock(&RTDBLock);
#endif
	}

	return val;
}

void RTDB::SetLastSampleTimeTag(int ioa)
{
	if(objects.find(ioa) != objects.end())
		time(&objects[ioa].LastSampleTimeTag);
}

void RTDB::SetAllObjectsLastSampleTimeTag()
{
	for(map<int, InformationObject>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		time(&(*it).second.LastSampleTimeTag);
	}

}

void RTDB::SetAllIECObjectsTimeStamp()
{
	for(map<int, InformationObject>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		(*it).second.GetValue();
	}
}

InformationObject* RTDB::GetInformationObject(int ioa)
{
	if(objects.find(ioa) != objects.end())
		return &objects[ioa];
	return NULL;
}

InformationObject* RTDB::GetCommandedInformationObject(int _command_ioa, int _command_type)
{	
	return NULL;
}

set<int> RTDB::GetUsedInputTypes()
{
	return UsedInputTypes;
}

set<int> RTDB::GetUsedOutputTypes()
{
	return UsedOutputTypes;
}

bool RTDB::ContainsObjectAddress(int ioa)
{
	if(objects.find(ioa) != objects.end())
		return true;
	return false;
}

vector<int> RTDB::GetModbusObjectsAddress(int SlaveID, int FunctionCode)
{
	vector<int> objectList;
	for(map<int, InformationObject>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		if((*it).second.GetModbusSlaveID() == SlaveID && (*it).second.GetModbusFunctionCode() == FunctionCode)
		{
			objectList.push_back((*it).first);
		}
	}
	return objectList;
}

bool RTDB::isInputType(int type)
{	
	return false;
}

bool RTDB::isOutputType(int type)
{
	return false;
}

vector<int> RTDB::GetListOfObjectAddressesWithModbusAddress(int mbaddress)
{
	vector<int> objectList;
	for(map<int, InformationObject>::iterator it = objects.begin(); it != objects.end(); ++it)
	{
		if((*it).second.GetModbusPointAddress() == mbaddress)
		{
			objectList.push_back((*it).first);
		}
	}
	return objectList;
}




