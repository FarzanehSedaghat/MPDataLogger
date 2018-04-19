#ifndef _INFORMATIONOBJECT_H_
#define _INFORMATIONOBJECT_H_

#include <time.h>
#include <stdio.h>
#include "Value.h"

class InformationObject
{
private:
	VALUE value;
	int SampleInterval;
	int ModbusSlaveID;
	int ModbusFunctionCode;
	int ModbusPointAddress;
	int ModbusSlaveGroupID;
	int AD;	//Indicates Analog or Digital Object

public:
	time_t LastSampleTimeTag;

	InformationObject();
	InformationObject(int _ad, int _sampleTime,
			int _modbusSlaveID, int _modbusFunctionCode, int _modbusPointAddress, int _modbusSlaveGroupID);
	void ApplyCommand(VALUE _value);
	void SetValue(VALUE _value);
	VALUE GetValue();

	int GetSampleInterval();
	int GetModbusSlaveID();
	int GetModbusPointAddress();
	int GetModbusFunctionCode();
	int GetModbusSlaveGroupID();
	bool isAnalog();
};

#endif
