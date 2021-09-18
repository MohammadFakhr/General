/* 
	drv_UartDma.c
	Driver to handle Uart of Stm32 via DMA
	Project: General Driver
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/

#include "Driver/uart_dma.h"

#include "string.h"
//*****************************************************************************//
void uartdma_SetBaudRate(UART_HandleTypeDef *huart,uint32_t baudrate)
{
	huart->Init.BaudRate = baudrate;
	if (HAL_UART_Init(huart) != HAL_OK)
	{
		Error_Handler();
	}
}

//*****************************************************************************//
//return 1 if Expected Response is finded else return 0
U8 UART_DMA_Transfer(UART_DMA_HandleTypeDef huartDma)
{
	HAL_DMA_StateTypeDef dmaState;
	float byteWidth;// Time width of 1 byte transmition
	int rxDelay = 0;
	U8 result = 0;

	byteWidth  = 10000.0f / huartDma.huart->Init.BaudRate;
	
	HAL_UART_DMAStop(huartDma.huart);
	HAL_UART_Receive_DMA(huartDma.huart, huartDma.rxData, huartDma.RxLength);
	HAL_UART_Transmit_DMA(huartDma.huart, huartDma.txData, huartDma.TxLength);

	// Wait until trasmition is compelte
	huartDma.Delay_func((uint16_t)(byteWidth * huartDma.TxLength) + 1);
	// Wait if Tx is busy
	dmaState = HAL_DMA_STATE_BUSY;
	while(dmaState == HAL_DMA_STATE_BUSY)
	{
		huartDma.Delay_func(1);
		dmaState = HAL_DMA_GetState(huartDma.hdma);
	}
	while(HAL_UART_GetState(huartDma.huart) == HAL_UART_STATE_BUSY_TX);
	// Wait untill data come
	while(rxDelay < huartDma.RxWaitTime)
	{
		huartDma.Delay_func(100);
		rxDelay += 100;
		huartDma.rxData[huartDma.RxLength - 1] = 0;
		if(strlen(huartDma.expectedAnswer) > 0)
		{
			// Check Valid Response from start of String
			if(strstr((char*)huartDma.rxData, huartDma.expectedAnswer) != 0)
			{
				huartDma.Delay_func(huartDma.DelayAfterAnswer);
				rxDelay = huartDma.RxWaitTime;// break loop when answer come
				result = 1;
			}
		}
		else
		{
			result = 1;
		}
	}
	HAL_UART_DMAStop(huartDma.huart);
	
	return result;
}

//*****************************************************************************//
void UART_DMA_Transfer485(UART_DMA_HandleTypeDef huartDma, GPIO_TypeDef *DirPort,uint16_t DirPin)
{
	HAL_DMA_StateTypeDef stat;
	float ByteWidth; // Time width of 1 byte transmition

	ByteWidth  = 10000.0f / huartDma.huart->Init.BaudRate;
	HAL_UART_Receive_DMA(huartDma.huart, huartDma.rxData, huartDma.RxLength);
	HAL_GPIO_WritePin(DirPort, DirPin, GPIO_PIN_SET); // DIR = HIGH: Transmit mode
	HAL_UART_Transmit_DMA(huartDma.huart, huartDma.txData, huartDma.TxLength);
	// Wait until trasmition is compelte
	huartDma.Delay_func((uint16_t)(ByteWidth * huartDma.TxLength));
	// Wait if Tx is busy
	stat = HAL_DMA_STATE_BUSY;
	while(stat == HAL_DMA_STATE_BUSY)
	{
		huartDma.Delay_func(1);
		stat = HAL_DMA_GetState(huartDma.hdma);
	}
	while(HAL_UART_GetState(huartDma.huart) == HAL_UART_STATE_BUSY_TX)
	{
		huartDma.Delay_func(1);
	}
	huartDma.Delay_func(1);
	HAL_GPIO_WritePin(DirPort, DirPin, GPIO_PIN_RESET);// DIR = LOW: Recieve mode
	// Wait untill data come
	huartDma.Delay_func(huartDma.RxWaitTime);
	HAL_UART_DMAStop(huartDma.huart);
}



