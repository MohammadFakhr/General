/* 
	lib_Modbus.h
	Libray to handle modbus
	Project: General Library
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/

#ifndef _MODBUS_H
#define _MODBUS_H

/************************************************
This difinitions most in global.h defined
#define Delay(x)
This difinitions most in io.h defined
#define MODBUS_DIRPORT
#define MODBUS_DIRPIN
#define modbus_huart
#define modbus_dma
*************************************************/

#include "io.h"
#include "global.h"

#include "Driver/uart_dma.h"

#define MODBUS_RES_Ok					0
#define MODBUS_RES_NoResponse			1
#define MODBUS_RES_InvalidFunction		2
#define MODBUS_RES_InvalidDataAddress	3
#define MODBUS_RES_InvalidDataValue		4
#define MODBUS_RES_DeviceFailure		5
#define MODBUS_RES_Acknowledge			6
#define MODBUS_RES_SlaveBusy			7
#define MODBUS_RES_CrcError				8
#define MODBUS_RES_IPConflict			9
#define MODBUS_RES_FunctionConflict		10
#define MODBUS_RES_Nok					255
//**********************************************************************************************
#ifdef MODBUS_MASTER
// Master Modbus Functions --------------------------------------------------------------
U8 modbus_master_Read(U8 IP, U8 Function, U16 Address, U16 Length, U8 *RxBuf, U16 Delay);
U8 modbus_master_WriteSingle(U8 IP, U8 Function, U16 Address, U16 Data);
U8 modbus_master_WriteMulti(U8 IP, U8 Function, U8 Address, U16 Length, U16 *DataBuf);

void modbus_master_SetUart(U16 baud, U8 bitNo, U8 stopBit, U8 parity);

typedef struct
{
	U8 ScanIP;
	U8 Func;
	char DeviceModel[20];
	char Text[20];
	char ErrorText[20];
	U8 Wait;
}type_ModbusStatus;

#endif

#ifdef MODBUS_SLAVE
// Slave Modbus Functions ---------------------------------------------------------------
void modbus_slave_SetUart(U16 baud, U8 bitNo, U8 stopBit, U8 parity);
U8 modbus_slave_CheckCommand(U8 IP);
void modbus_slave_RxIrq(void);
void modbus_slave_TimerIrq(void);
U8 modbus_slave_GetDataReady();
U8* modbus_slave_GetCommand(void);
U8 modbus_slave_GetDataReady(void);
U8 modbus_slave_ClearDataReady(void);
U8 modbus_slave_SendReadResponse(U8 IP, U8 Function, U16 Address, U16 Length, U16 Buf[]);
U8 modbus_slave_SendWriteSingleResponse(U8 IP, U8 Function, U16 Address, U16 Length, U16 Buf[]);
U8 modbus_slave_SendWriteMultiResponse(U8 IP, U8 Function, U16 Address, U16 Length, U16 Buf[]);

#endif

// Crc ----------------------------------------------------------------------------------
U16 mbus_Crc(U8 *buf, int len);

#endif
