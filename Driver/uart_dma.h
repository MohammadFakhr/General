#ifndef _UART_DMA_H
#define _UART_DMA_H


#include "main.h"
#include "global.h"
#include "io.h"

typedef struct
{
	UART_HandleTypeDef *huart;
	DMA_HandleTypeDef *hdma;
	uint8_t *txData;
	uint8_t *rxData;
	char *expectedAnswer;
	uint16_t TxLength; 
	uint16_t RxLength;
	uint32_t RxWaitTime;
	uint32_t DelayAfterAnswer;
	void (*Delay_func)(U32);
}UART_DMA_HandleTypeDef;

void UART_DMA_SetBaudRate(UART_HandleTypeDef *huart,uint32_t baudrate);
U8 UART_DMA_Transfer(UART_DMA_HandleTypeDef huartDma);
void UART_DMA_Transfer485(UART_DMA_HandleTypeDef huartDma, GPIO_TypeDef *DirPort,uint16_t DirPin);

#endif


