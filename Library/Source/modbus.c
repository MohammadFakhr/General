
#include "io.h"
#include "global.h"
#include "Library/modbus.h"

U16 CrcCalc(U8 *buf, int len);
extern void modbusDelay(U32);

#ifdef MODBUS_MASTER

extern UART_HandleTypeDef modbus_master_huart;
extern DMA_HandleTypeDef modbus_master_dma;

// Master funtions ********************************************************************************
type_ModbusStatus ModbusStatus = {0, 0, "", "", "", 0};

// Set uart settings --------------------------------------------------------------------
void modbus_master_SetUart(U16 baud, U8 bitNo, U8 stopBit, U8 parity)
{
	modbus_master_huart.Init.BaudRate = baud;
	modbus_master_huart.Init.StopBits = (stopBit == 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;
	modbus_master_huart.Init.WordLength = (bitNo == 9) ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B;

	if(modbus_master_huart.Init.Parity == 0)
	{
		modbus_master_huart.Init.Parity = UART_PARITY_NONE;
	}
	else if(modbus_master_huart.Init.Parity == 1)
	{
		modbus_master_huart.Init.Parity = UART_PARITY_ODD;
	}
	else if(modbus_master_huart.Init.Parity == 2)
	{
		modbus_master_huart.Init.Parity = UART_PARITY_EVEN;
	}
}

//Read Coils and Discrete and Holding Registers And Input Registers ---------------------
U8 modbus_master_Read(U8 IP, U8 Function, U16 Address, U16 Length, U8 *RxBuf, U16 RxDelay)
{
	UART_DMA_HandleTypeDef huart_dma;

	if(Function < 1 || Function > 4 || Length > 127)
	{
		return MODBUS_RES_Nok;
	}
	ModbusStatus.Wait = 1;
	strcpy(ModbusStatus.ErrorText, ">>......<<<");
	ModbusStatus.ScanIP = IP;
	ModbusStatus.Func = Function;
	// Making Request packet
	U8 TxBuf[10];
	TxBuf[0] = IP;
	TxBuf[1] = Function;
	TxBuf[2] = Address >> 8;
	TxBuf[3] = Address & 0x00FF;
	TxBuf[4] = Length >> 8;
	TxBuf[5] = Length & 0x00FF;
	U16 Crc = CrcCalc (TxBuf, 6);
	TxBuf[6] = Crc & 0x00ff;
	TxBuf[7] = Crc >> 8;
	// Calculating length of recieved data 
	U16 RxMaxLen = (Function > 2) ? ((Length * 2) + 5): ((Length / 8) + 6);
	// Reset last data
	for(int i = 0; i < RxMaxLen; i++)
	{
		RxBuf[i] = 0xFF;
	}
	// Send Request and recieve Response via Dma and Uart
	huart_dma.huart = &modbus_master_huart;
	huart_dma.hdma = &modbus_master_dma;
	huart_dma.txData = TxBuf;
	huart_dma.rxData = RxBuf;
	huart_dma.TxLength = 8;
	huart_dma.RxLength = RxMaxLen;
	huart_dma.RxWaitTime = RxDelay;
	huart_dma.expectedAnswer = "";
	huart_dma.DelayAfterAnswer = 0;
	huart_dma.Delay_func = modbusDelay;
	UART_DMA_Transfer485(huart_dma, MODBUS_MASTER_DIRPORT, MODBUS_MASTER_DIRPIN);
	// Calculate length of packet
	U16 len = RxBuf[2] + 3;
	Crc = RxBuf[len]  + RxBuf[len + 1] * 256;
	// Check if errors exist --------------------------------------------------
	// No Response
	ModbusStatus.Wait = 0;
	if((RxBuf[0] == 0xFF) && (RxBuf[1] == 0xFF) &&(RxBuf[2] == 0xFF))
	{
		strcpy(ModbusStatus.ErrorText, "No Resp.");
		return MODBUS_RES_NoResponse;
	}
	// Recieved IP is not as same as request ip
	if(RxBuf[0] != IP)
	{
		strcpy(ModbusStatus.ErrorText, "IP Invalid");
		return MODBUS_RES_IPConflict;
	}
	// Execption error
	if(RxBuf[1] == (Function | 0x80) )
	{
		strcpy(ModbusStatus.ErrorText, "Resp Error");
		return RxBuf[2];
	}
	// Recieved Function is not as same as request ip
	if(RxBuf[1] != Function)
	{
		strcpy(ModbusStatus.ErrorText, "Func Invalid");
		return MODBUS_RES_FunctionConflict;
	}
	// Crc Error
	if(CrcCalc(RxBuf, len) != Crc)
	{
		strcpy(ModbusStatus.ErrorText, "Crc Fail");
		return MODBUS_RES_CrcError;
	}
	// return ok if no error
	strcpy(ModbusStatus.ErrorText, "OK");
	return MODBUS_RES_Ok;
}

// Write Single data --------------------------------------------------------------------
U8 modbus_master_WriteSingle(U8 IP, U8 Function, U16 Address, U16 Data)
{
	UART_DMA_HandleTypeDef huart_dma;

	if(Function < 5 || Function > 6)
	{
		return MODBUS_RES_Nok;
	}
	U8 TxBuf[10];
	U8 RxBuf[10];
	TxBuf[0] = IP;
	TxBuf[1] = Function;
	TxBuf[2] = Address >> 8;
	TxBuf[3] = Address & 0x00FF;
	TxBuf[4] = Data >> 8;
	TxBuf[5] = Data & 0x00FF;
	U16 Crc = CrcCalc (TxBuf, 6);
	TxBuf[6] = Crc & 0x00ff;
	TxBuf[7] = Crc >> 8;

	huart_dma.huart = &modbus_master_huart;
	huart_dma.hdma = &modbus_master_dma;
	huart_dma.txData = TxBuf;
	huart_dma.rxData = RxBuf;
	huart_dma.TxLength = 8;
	huart_dma.RxLength = 10;
	huart_dma.RxWaitTime = 200;
	huart_dma.expectedAnswer = "";
	huart_dma.DelayAfterAnswer = 0;
	UART_DMA_Transfer485(huart_dma, MODBUS_MASTER_DIRPORT, MODBUS_MASTER_DIRPIN);

	U16 len = RxBuf[2] + 3;
	Crc = RxBuf[len]  + RxBuf[len + 1] * 256;

	// Check if errors exist --------------------------------------------------
	// Recieved IP is not as same as request ip
	if(RxBuf[0] != IP)
	{
		return MODBUS_RES_IPConflict;
	}
	// Execption error
	if(RxBuf[1] == (Function | 0x80) )
	{
		return RxBuf[2];
	}
	// Recieved Function is not as same as request ip
	if(RxBuf[1] != Function)
	{
		return MODBUS_RES_FunctionConflict;
	}
	// Crc Error
	if(CrcCalc(RxBuf, 6) != Crc)
	{
		return MODBUS_RES_CrcError;
	}
	// return ok if no error
	return MODBUS_RES_Ok;
}

//Write Register ------------------------------------------------------------------------
U8 modbus_master_WriteMulti(U8 IP, U8 Function, U8 Address, U16 Length, U16 *DataBuf)
{
	UART_DMA_HandleTypeDef huart_dma;

	if(Function < 15 || Function > 16 || Length > 127)
	{
		return MODBUS_RES_Nok;
	}
	U8 TxBuf[260];
	U8 RxBuf[10];

	TxBuf[0] = IP;
	TxBuf[1] = Function;
	TxBuf[2] = Address >> 8;
	TxBuf[3] = Address & 0x00FF;
	TxBuf[4] = Length >> 8;
	TxBuf[5] = Length & 0x00FF;

	U8 len = Function == 15 ? (Length % 8 ? Length / 8 + 1: Length / 8): Length * 2;
	TxBuf[6] = len;

	U16 k = 7;
	for(int i = 0; i< len; i++)
	{
		TxBuf[k++] = DataBuf[i] >> 8;
		TxBuf[k++] = DataBuf[i] & 0x00FF;
	}
	U16 Crc = CrcCalc (TxBuf, 6);
	TxBuf[k++] = Crc & 0x00ff;
	TxBuf[k++] = Crc >> 8;

	huart_dma.huart = &modbus_master_huart;
	huart_dma.hdma = &modbus_master_dma;
	huart_dma.txData = TxBuf;
	huart_dma.rxData = RxBuf;
	huart_dma.TxLength = k;
	huart_dma.RxLength = 8;
	huart_dma.RxWaitTime = 200;
	huart_dma.expectedAnswer = "";
	huart_dma.DelayAfterAnswer = 0;
	UART_DMA_Transfer485(huart_dma, MODBUS_MASTER_DIRPORT, MODBUS_MASTER_DIRPIN);

	len = RxBuf[2] + 5;
	Crc = RxBuf[len - 1] * 256 + RxBuf[len - 2];
	// Check if errors exist --------------------------------------------------
	// Recieved IP is not as same as request ip
	if(RxBuf[0] != IP)
	{
		return MODBUS_RES_IPConflict;
	}
	// Recieved Function is not as same as request ip
	if(RxBuf[1] != Function)
	{
		return MODBUS_RES_FunctionConflict;
	}
	// Crc Error
	if(CrcCalc(RxBuf, 6) != Crc)
	{
		return MODBUS_RES_CrcError;
	}
	// Execption error
	if(RxBuf[1] == (Function | 0x80) )
	{
		return RxBuf[2];
	}
	// return ok if no error
	return MODBUS_RES_Ok;
}

#endif

#ifdef MODBUS_SLAVE
extern UART_HandleTypeDef modbus_slave_huart;
extern DMA_HandleTypeDef modbus_slave_TxDma;

U8 SlaveBuf[128] = {0};
U8 RxTimeout = 0;
typedef enum { Rx_State_Wait, Rx_State_Fill, Rx_State_Ready } RxState;
RxState slave_RxState = Rx_State_Wait;


// Slave funtions *********************************************************************************
// Set uart settings --------------------------------------------------------------------
void modbus_slave_SetUart(U16 baud, U8 bitNo, U8 stopBit, U8 parity)
{
	modbus_slave_huart.Init.BaudRate = baud;
	modbus_slave_huart.Init.StopBits = (stopBit == 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;
	modbus_slave_huart.Init.WordLength = (bitNo == 9) ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B;

	if(modbus_slave_huart.Init.Parity == 0)
	{
		modbus_slave_huart.Init.Parity = UART_PARITY_NONE;
	}
	else if(modbus_slave_huart.Init.Parity == 1)
	{
		modbus_slave_huart.Init.Parity = UART_PARITY_ODD;
	}
	else if(modbus_slave_huart.Init.Parity == 2)
	{
		modbus_slave_huart.Init.Parity = UART_PARITY_EVEN;
	}
}

U8 modbus_slave_CheckCommand(U8 IP)
{
	U16 len;

	if(SlaveBuf[1] < 7)
	{
		len = 6;
	}
	else
	{
		len = SlaveBuf[6] + 6;
	}
	U16 Crc = SlaveBuf[len + 1] * 256 + SlaveBuf[len];
	// Check if errors exist --------------------------------------------------
	if(SlaveBuf[0] != IP)
	{
		return MODBUS_RES_IPConflict;
	}
	// Crc Error
	if(CrcCalc(SlaveBuf, len) != Crc)
	{
		return MODBUS_RES_CrcError;
	}
	// return ok if no error
	return MODBUS_RES_Ok;
}

U8 SlaveIndex = 0;
void modbus_slave_RxIrq(void)
{
	if(slave_RxState != Rx_State_Ready)
	{
		slave_RxState = Rx_State_Fill;
		//HAL_UART_Receive(&modbus_slave_huart, &SlaveRxBuf[SlaveRxIndex], 1, 0);//modbus_slave_huart.Instance->RDR;
		SlaveBuf[SlaveIndex] = modbus_slave_huart.Instance->RDR;
		if(SlaveIndex < 127)
		{
			SlaveIndex++;
		}
		RxTimeout = 0;
	}
}

void modbus_slave_TimerIrq(void)
{
	if(slave_RxState == Rx_State_Fill)
	{
		RxTimeout++;
		if(RxTimeout > 10)
		{
			SlaveIndex = 0;
			slave_RxState = Rx_State_Ready;
		}
	}
}

U8 modbus_slave_GetDataReady(void)
{
	return (slave_RxState == Rx_State_Ready) ? 1 : 0;
}

U8 modbus_slave_ClearDataReady(void)
{
	slave_RxState = Rx_State_Wait;
}

U8* modbus_slave_GetCommand(void)
{
	return SlaveBuf;
}

U8 modbus_slave_SendReadResponse(U8 IP, U8 Function, U16 Address, U16 Length, U16 Data[])
{
	UART_DMA_HandleTypeDef huart_dma;
	U16 Crc;
	int index = 0;

	if ( (Function == 3) || (Function == 4) )
	{
		if(Length > 127)
		{
			return MODBUS_RES_Nok;
		}
	}

	// Making Request packet
	SlaveBuf[index++] = IP;
	SlaveBuf[index++] = Function;
	if(Function < 3)
	{
		SlaveBuf[index++] = (Length % 8 == 0) ? (Length / 8) : (Length / 8) + 1;
		U8 shift = 0x01;
		for(int i = 0; i < Length; i++)
		{
			SlaveBuf[index] |= (Data[i + Address]) ? shift : 0;
			shift = (shift << 1);
			if(shift == 0)
			{
				shift = 0x01;
				index++;
			}
		}
	}
	else
	{
		SlaveBuf[index++] = Length * 2;		
		for(int i = 0; i < Length; i++)
		{
			SlaveBuf[index++] = Data[i + Address] >> 8;
			SlaveBuf[index++] = Data[i + Address] & 0x00FF;
		}
	}

	Crc = CrcCalc(SlaveBuf, index);
	SlaveBuf[index++] = Crc & 0x00FF;
	SlaveBuf[index++] = Crc >> 8;
	
	U8 RxBuf[2]; // not used
	// Send Request and recieve Response via Dma and Uart
	huart_dma.huart = &modbus_slave_huart;
	huart_dma.hdma = &modbus_slave_TxDma;
	huart_dma.txData = SlaveBuf;
	huart_dma.rxData = RxBuf;
	huart_dma.TxLength = index;
	huart_dma.RxLength = 0;
	huart_dma.RxWaitTime = 0;
	huart_dma.expectedAnswer = "";
	huart_dma.DelayAfterAnswer = 10;
	huart_dma.Delay_func = modbusDelay;
	UART_DMA_Transfer485(huart_dma, MODBUS_SLAVE_DIRPORT, MODBUS_SLAVE_DIRPIN);
}
#endif

// Compute the MODBUS RTU CRC *********************************************************************
U16 CrcCalc(U8 *buf, int len)
{
	U16 crc = 0xFFFF;
	int pos,i;
	for(pos = 0; pos < len; pos++) 
	{
		crc ^= (U16)buf[pos];          // XOR byte into least sig. byte of crc
		for (i = 8; i != 0; i--) 
		{    // Loop over each bit
			if ((crc & 0x0001) != 0)
			{      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}



