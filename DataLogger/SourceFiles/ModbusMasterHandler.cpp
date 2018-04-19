
#include <string.h>
#include "ModbusMasterHandler.h"

ModbusMasterHandler::ModbusMasterHandler() {

}

ModbusMasterHandler::~ModbusMasterHandler()
{
	for(vector<ModbusExtension*>::iterator it = RTDB::Instance()->ModbusExtensions.begin();
					it != RTDB::Instance()->ModbusExtensions.end(); ++it)
	{

		ModbusExtension *ModbusSlave = *it;
		modbus_close(ModbusSlave->ModbusMasterContext);
		modbus_free(ModbusSlave->ModbusMasterContext);
	}
}

void ModbusMasterHandler::StartThread()
{
	if(!Initialize())
	{
		fprintf(stderr, "Error initializing MODBUS handler!\n");
	}
	int err = 0;
#ifdef _WIN32
	ModbusMasterHandlerThreadID = CreateThread(NULL, 0, &ModbusMasterHandler::UpdapteValues, NULL, 0, &thID);
#else
	err = pthread_create(&ModbusMasterHandlerThreadID, NULL, &ModbusMasterHandler::UpdapteValues, NULL);
#endif
	if ( err < 0) {
		fprintf(stderr, "Error creating thread\n");
		return;
	}
}

bool ModbusMasterHandler::Initialize()
{
	for(vector<ModbusExtension*>::iterator it = RTDB::Instance()->ModbusExtensions.begin();
				it != RTDB::Instance()->ModbusExtensions.end(); ++it)
	{

		ModbusExtension *ModbusSlave = *it;

		char parity = (ModbusSlave->Parity == 0)? 'N' : (ModbusSlave->Parity == 1)? 'O' : 'E';

		if (ModbusSlave->isTCP)
		{
			ModbusSlave->ModbusMasterContext = modbus_new_tcp(ModbusSlave->TCPIP.c_str(), ModbusSlave->TCPPort);
		}
		else
		{
			ModbusSlave->ModbusMasterContext =
				modbus_new_rtu(ModbusSlave->ModbusSlavePort,
					ModbusSlave->BaudRate, parity, 8, ModbusSlave->StopBit);
		}

		modbus_set_response_timeout(ModbusSlave->ModbusMasterContext, 3, 0);

		if( modbus_connect(ModbusSlave->ModbusMasterContext) < 0 )
		{
			fprintf(stderr, "Could not connect to Modbus Slave %s ID %d!\n",
					ModbusSlave->ModbusSlaveName, ModbusSlave->ModbusSlaveID);
			return false;
		}

		if (ModbusSlave->isTCP)
		{
			fprintf(stdout, "Modbus %s ID=%d is connected successfully to IP %s, Port:%d ... \n",
				ModbusSlave->ModbusSlaveName, ModbusSlave->ModbusSlaveID, 
				ModbusSlave->TCPIP.c_str(),	ModbusSlave->TCPPort);
		}
		else
		{
			fprintf(stdout, "Modbus %s ID=%d is connected successfully to port %s, BR:%d, PR:%c, SB:%d ... \n",
				ModbusSlave->ModbusSlaveName, ModbusSlave->ModbusSlaveID, ModbusSlave->ModbusSlavePort,
				ModbusSlave->BaudRate, parity, ModbusSlave->StopBit);
		}

		modbus_set_slave(ModbusSlave->ModbusMasterContext, ModbusSlave->ModbusSlaveID);

//		modbus_set_debug(ModbusSlave->ModbusMasterContext, 1);
		fprintf(stdout , "Modbus set slave is successful... \n");

		ModbusSlave->AIs = RTDB::Instance()->GetModbusObjectsAddress(ModbusSlave->ModbusSlaveID, FunctionCode_AI);
		ModbusSlave->DIs = RTDB::Instance()->GetModbusObjectsAddress(ModbusSlave->ModbusSlaveID, FunctionCode_DI);
		ModbusSlave->DOs = RTDB::Instance()->GetModbusObjectsAddress(ModbusSlave->ModbusSlaveID, FunctionCode_DO);
	}

	return true;
}


#ifdef _WIN32
DWORD WINAPI ModbusMasterHandler::UpdapteValues(LPVOID arg)
#else
void *ModbusMasterHandler::UpdapteValues(void* args)
#endif
{
	int rc = -1;
	bool microRead = false;
	int n = 1;

	//Reading time from micro to set a valid time before synchronizing with IEC master station

	while(1)
	{
		//Update from Modbus Links
		for(vector<ModbusExtension*>::iterator it = RTDB::Instance()->ModbusExtensions.begin();
					it != RTDB::Instance()->ModbusExtensions.end(); ++it)
		{

			ModbusExtension *ModbusSlave = *it;

//			fprintf(stderr, "\nreading modbus slave %s registers \n", ModbusSlave->ModbusSlaveName);

			string val;

			usleep(400000);

			for(vector<ModbusExtensionGroup*>::iterator git = ModbusSlave->DataGroups.begin();
					git != ModbusSlave->DataGroups.end(); ++git)
			{
				ModbusExtensionGroup* group = *git;
				int addr = group->StartAddress;
				int nb = group->Lenght;
				uint8_t req[12];
				switch(group->GroupFunction)
				{
				case MODBUS_FC_READ_COILS                    :
				{
					uint8_t *rc_reg = new uint8_t(nb);
					rc = modbus_send_request_bits(ModbusSlave->ModbusMasterContext, addr, nb, rc_reg, req);
					if (rc > 0)
					{
						rc = modbus_read_bits(ModbusSlave->ModbusMasterContext, addr, nb, rc_reg, req);
					}
				}
					break;
				case MODBUS_FC_READ_DISCRETE_INPUTS          :
					//rc = modbus_read_input_bits(ModbusSlave->ModbusMasterContext, addr, nb, tab_reg);
					break;
				case MODBUS_FC_READ_HOLDING_REGISTERS        :
				{
					uint16_t *tab_reg = new uint16_t(nb);
					rc = modbus_send_request_registers(ModbusSlave->ModbusMasterContext, addr, nb, tab_reg, req);
					if (rc > 0)
					{
						rc = modbus_read_registers(ModbusSlave->ModbusMasterContext, addr, nb, tab_reg, req);
					}


					//printf("\nMODBUS: %d bytes are read!\n", rc);
					if (rc > 0)
					{
						//printf("group%d, FC%d: ", group->ModbusGroupID, group->GroupFunction);
						for (int i = 0; i < rc; i++) {
							//printf("reg[%d]=%d (0x%X)\n", ModbusSlave->AIs[i], tab_reg[i], tab_reg[i]);
							//RTDB update
							VALUE val;
							val.REAL = tab_reg[i];
							RTDB::Instance()->SetValue(ModbusSlave->AIs[i], val);
						}
					}
					else if (rc == -1)
					{
						fprintf(stderr, "error in reading modbus registers \n");
					}


				}
					break;
				case MODBUS_FC_READ_INPUT_REGISTERS          :
					//rc = modbus_read_input_registers(ModbusSlave->ModbusMasterContext, addr, nb, tab_reg);
					break;
				case MODBUS_FC_WRITE_SINGLE_COIL             :
				case MODBUS_FC_WRITE_SINGLE_REGISTER         :
				case MODBUS_FC_READ_EXCEPTION_STATUS         :
				case MODBUS_FC_WRITE_MULTIPLE_COILS          :
				case MODBUS_FC_WRITE_MULTIPLE_REGISTERS      :
				case MODBUS_FC_REPORT_SLAVE_ID               :
				case MODBUS_FC_MASK_WRITE_REGISTER           :
				case MODBUS_FC_WRITE_AND_READ_REGISTERS      :
				default:
					break;
				}


			}

			usleep(1000000);
		}
	}

	fprintf(stderr, "MODBUS THREAD EXITED DUE TO AN ERROR... \nError: [Errno: %d] %s\n", rc, strerror(rc));

	return NULL;
}





