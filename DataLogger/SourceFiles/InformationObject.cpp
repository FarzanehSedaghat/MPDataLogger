
#include "InformationObject.h"

InformationObject::InformationObject() {
	SampleInterval = 0;
	ModbusSlaveID = 0;
	ModbusFunctionCode = 0;
	ModbusPointAddress = 0;
	ModbusSlaveGroupID = 0;
}

InformationObject::InformationObject(
		int _ad, int _sampleTime,
		int _modbusSlaveID, int _modbusFunctionCode, int _modbusPointAddress, int _modbusSlaveGroupID) {
	AD = _ad;
	SampleInterval = _sampleTime;
	ModbusSlaveID = _modbusSlaveID;
	ModbusFunctionCode = _modbusFunctionCode;
	ModbusPointAddress = _modbusPointAddress;
	ModbusSlaveGroupID = _modbusSlaveGroupID;
	VALUE init;
	init.UDINT = 0;
	SetValue(init);
}

void InformationObject::ApplyCommand(VALUE _value)
{
	SetValue(_value);
}

void InformationObject::SetValue(VALUE _value)
{
	value = _value;
}

VALUE InformationObject::GetValue() 
{
	return value;
}

int InformationObject::GetSampleInterval() {
	return SampleInterval;
}

int InformationObject::GetModbusSlaveID()
{
	return ModbusSlaveID;
}

int InformationObject::GetModbusPointAddress()
{
	return ModbusPointAddress;
}

int InformationObject::GetModbusFunctionCode()
{
	return ModbusFunctionCode;
}

int InformationObject::GetModbusSlaveGroupID()
{
	return ModbusSlaveGroupID;
}

bool InformationObject::isAnalog()
{
	return AD > 0;
}

