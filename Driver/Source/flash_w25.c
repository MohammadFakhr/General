/* 
	drv_MemW25.c
	Driver to handle spi flash memory W25Q32
	Project: General Driver
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
	
	Note: This driver needs 4KB global stack
*/

#include "Driver/flash_w25.h"

void writeEnable(void);
void writeDisable(void);
U8 waitBusy(void);
U8 eraseSector(U32 sectoraddr);
void reset(void);

extern SPI_HandleTypeDef flash_w25_hspi;

//******************************************************************************************//
//send 0x66 for enable reset
//send 0x99 for reset
//*******************************************************************************************//
void FLASH_W25_Reset(void)
{
	U8 buffer[2] = {0x66, 0x99};
	FLASH_W25_PORT_CS_0;
	HAL_SPI_Transmit(&flash_w25_hspi, &buffer[0], 1, 100);
	FLASH_W25_PORT_CS_1;
	HAL_Delay(1);
	FLASH_W25_PORT_CS_0;
	HAL_SPI_Transmit(&flash_w25_hspi, &buffer[1], 1, 100);
	FLASH_W25_PORT_CS_1;	
	
	FLASH_W25_PORT_WP_1;
}
//**********************************************************************************************//
//read a U8 data from a U32 Address of memory
//**********************************************************************************************//
U8 FLASH_W25_ReadByte(U32 ByteAddr)
{
	U8 data = 0;
	U8 cmd[4] = {0x03, (ByteAddr & 0xff0000) >> 16, (ByteAddr & 0xff00) >> 8, (ByteAddr & 0xff)};
	waitBusy();
	FLASH_W25_PORT_CS_0;
	HAL_SPI_Transmit(&flash_w25_hspi, cmd, 4, 100); 
    HAL_SPI_Receive(&flash_w25_hspi, &data, 1, 100);
	FLASH_W25_PORT_CS_1;
	return data;
}
//**************************************************************************************************//
//Read 256byte from SPI flash memory to RAM
//The BlockAddr should be from 0 to 32768 
//*************************************************************************************************//
U8 FLASH_W25_ReadBlock256(U8 *buf, U32 Block256Addr)
{
	if(Block256Addr < 32768)
	{
		U32 ByteAddr = Block256Addr * 256;
		U8 cmd[4] = {0x03, (ByteAddr & 0xff0000) >> 16, (ByteAddr & 0xff00) >> 8, (ByteAddr & 0xff)};
		waitBusy();
		FLASH_W25_PORT_CS_0;
		HAL_SPI_Transmit(&flash_w25_hspi, cmd, 4, 100); 
		HAL_SPI_Receive(&flash_w25_hspi, buf, 256, 100);
		FLASH_W25_PORT_CS_1;
		return FLASH_W25_OK;
	}
	return FLASH_W25_FAIL;
}
//*********************************************************************************//
// Write a 4K sector from Ram to SPI flash memory
// The SectorAddr : 0 to 2048
// buf: buffer to write
// For each write should it be first enable writing
//*********************************************************************************//
U8 FLASH_W25_WriteSector4K(U8 *buf, U32 Sector4KAddr)
{
	if(Sector4KAddr < 2048)
	{
		eraseSector(Sector4KAddr);
		U32 ByteAddr = Sector4KAddr * 4096;
		waitBusy();
		HAL_Delay(1);
		writeEnable();
		for(int i = 0; i < 16; i++)
		{
			waitBusy();
			U8 cmd[4] = {0x02, (ByteAddr & 0xff0000) >> 16, (ByteAddr & 0xff00) >> 8, (ByteAddr & 0xff)};
			writeEnable();
			FLASH_W25_PORT_CS_0;
			HAL_SPI_Transmit(&flash_w25_hspi, cmd, 4, 100); 
			HAL_SPI_Transmit(&flash_w25_hspi, &buf[256 * i], 256, 100);
			FLASH_W25_PORT_CS_1;
			ByteAddr += 256;
		}
		return FLASH_W25_OK;
	}
	return FLASH_W25_FAIL;
}

//*********************************************************************************//
// Write a 512 byte block from Ram to SPI flash memory
// The BlockAddr : 0 to 2048
// buf: buffer to write
// For each write should it be first enable writing
//*********************************************************************************//
U8 FLASH_W25_ReadBlock512(U8 *buf, U32 Block512Addr)
{
	U32 block256Addr = Block512Addr * 2;

	if(FLASH_W25_ReadBlock256(buf, block256Addr) == FLASH_W25_FAIL)
		if(FLASH_W25_ReadBlock256(buf, block256Addr) == FLASH_W25_FAIL)// Retry if fail
			return FLASH_W25_FAIL;

	if(FLASH_W25_ReadBlock256(buf + 256, block256Addr + 1) == FLASH_W25_FAIL)
		if(FLASH_W25_ReadBlock256(buf + 256, block256Addr + 1) == FLASH_W25_FAIL)// Retry if fail
			return FLASH_W25_FAIL;

	return FLASH_W25_OK;
}

U8 buf4K[4096];
U8 FLASH_W25_WriteBlock512(U8 *buf, U32 Block512Addr)
{
	U32 sector4KAddr = Block512Addr / 8;
	U32 block256Addr = sector4KAddr * 16;

	// First saving compelte sector into buf4K
	for(int i = 0;i < 16; i++)
	{
		if(FLASH_W25_ReadBlock256(buf4K + i * 256, block256Addr + i) == FLASH_W25_FAIL)
			if(FLASH_W25_ReadBlock256(buf4K + i * 256, block256Addr + i) == FLASH_W25_FAIL)// Retry if fail
				return FLASH_W25_FAIL;
	}
	
	// Fill 512 byte of buffer
	U32 remain4K = Block512Addr % 8;
	U32 bufOffset = remain4K * 512;
	for(int i = 0; i < 512; i++)
		buf4K[bufOffset + i] = buf[i];

	if(FLASH_W25_WriteSector4K(buf4K, sector4KAddr) ==  FLASH_W25_FAIL)
		if(FLASH_W25_WriteSector4K(buf4K, sector4KAddr) ==  FLASH_W25_FAIL)// Retry if fail
			return FLASH_W25_FAIL;

	return FLASH_W25_OK;
}

//***************************************************************************************************//
//Internal functions
//********************************************************************************//
//send 0x06 for write enable
//********************************************************************************//
void writeEnable(void)
{
	U8 data = 0x06;
	FLASH_W25_PORT_CS_0;
	HAL_SPI_Transmit(&flash_w25_hspi, &data, 1, 100);
	FLASH_W25_PORT_CS_1;
	HAL_Delay(1);
}
//*********************************************************************************//
////send 0x04 for write disable
//*********************************************************************************//
void writeDisable(void)
{
	U8 data = 0x04;
	FLASH_W25_PORT_CS_0;
	HAL_SPI_Transmit(&flash_w25_hspi, &data, 1, 100);
	FLASH_W25_PORT_CS_1;
	HAL_Delay(1);
}
//**********************************************************************************//
//wait until memory is busy
//***************************************************************************************//
U8 waitBusy(void)
{
	U8 status = 1, Command = 0x05, tdata = 0xA5;
	U32 count;
	FLASH_W25_PORT_CS_0;
	HAL_SPI_Transmit(&flash_w25_hspi, &Command, 1, 100);
	while((status & 0x01) == 0x01)
	{
		HAL_SPI_TransmitReceive(&flash_w25_hspi, &tdata, &status, 1, 100);
		count++;
		if(count > 1000000)
		{
			FLASH_W25_PORT_CS_1;
			return FLASH_W25_FAIL;
		}
	}
	FLASH_W25_PORT_CS_1;
	return FLASH_W25_OK;
}
//******************************************************************************************//
//send 0x20 for erase sector
//sector address can be for example 1,2....5 to 2048
//******************************************************************************************//
U8 eraseSector(U32 sectorAddr)
{
	if(sectorAddr < 2048)
	{		
		U32 ByteAddr = sectorAddr * 4096;
		U8 buffer[4] = {0x20, (ByteAddr >> 16) & 0xff, (ByteAddr >> 8) & 0xff, ByteAddr & 0xff};
		waitBusy();
		writeEnable();
		FLASH_W25_PORT_CS_0;
		HAL_SPI_Transmit(&flash_w25_hspi, buffer, 4, 100);
		FLASH_W25_PORT_CS_1;
		waitBusy();
		return FLASH_W25_OK;
	}
	return FLASH_W25_FAIL;
}
