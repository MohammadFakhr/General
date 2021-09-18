/* 
	drv_MemW25.?
	Driver to handle spi flash memory W25Q32
	Project: General Driver
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
	
	Note: This driver needs 4KB global stack
*/

#ifndef FLASH_W25_H
#define FLASH_W25_H

#include "main.h"
#include "global.h"
#include "io.h"

// Definitions ********************************************
#define FLASH_W25_OK		1
#define FLASH_W25_FAIL		0

//********************************************************//
//Main functions
//********************************************************//
void FLASH_W25_Reset(void);

// Extended functions
U8 FLASH_W25_ReadBlock512(U8 *buf, U32 Block512Addr);
U8 FLASH_W25_WriteBlock512(U8 *buf, U32 Block512Addr);

// Main functions
U8 FLASH_W25_ReadBlock256(U8 *buf, U32 BlockAddr);
U8 FLASH_W25_ReadByte(U32 ByteAddr);
U8 FLASH_W25_WriteSector4K(U8 *buf, U32 sectorAddr);


#endif


