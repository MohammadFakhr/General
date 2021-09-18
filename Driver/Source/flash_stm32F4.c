#include "drv_memif_F4.h"
#include "main.h"
#include "stm32f4xx_hal_flash_ex.h"

static FLASH_EraseInitTypeDef EraseInitStruct;
static uint32_t SectorError;

static uint32_t GetSector(uint32_t Address);

//***********************************************************************************//
//read a U32 buffer from an Address of flash memory//
//the Address + length of Buffer should be less than page size //
//the Address should be multiplied by 4 for the offset because the variabe of address is 32 bits or 4 bytes// 
//**********************************************************************************//
U8 memif_ReadBuffer(U32 SectorAdd, U32 Address, U32 Length, U32 *Buffer)
{
	U32 offset;
	offset = SectorAdd + Address * 4;
	for (int j = 0; j < Length; j++)
	{
		Buffer[j] = *(__IO uint32_t*)offset;
		offset += 4;
	}
	return MEMIF_OK;
}

//***********************************************************************************//
//write U32 buffer on a Address of flash memory//
//it should be first, read the data from last writting on flash //
//then put the data on the buffer on new address "buf[Address]"//
//erase the sector of flash with "HAL_FLASHEx_Erase" // 
//write buffer on flash memory with a FOR loop. for each write, we should plus address to 4//

//**********************************************************************************//
U8 memif_WriteBuffer(U32 SectorAdd, U32 Length, U32 *Buffer)
{
	int i;
	HAL_FLASH_Unlock();

	U32 SectorNo = GetSector(SectorAdd);

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector = SectorNo;
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.Banks = FLASH_BANK_2;
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
	{
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
		{
			HAL_FLASH_Lock();
			return MEMIF_FAIL;
		}
	}
	/* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	 you have to make sure that these data are rewritten before they are accessed during code
	 execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	 DCRST and ICRST bits in the FLASH_CR register. */
	__HAL_FLASH_DATA_CACHE_DISABLE();
	__HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

	__HAL_FLASH_DATA_CACHE_RESET();
	__HAL_FLASH_INSTRUCTION_CACHE_RESET();

	__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
	__HAL_FLASH_DATA_CACHE_ENABLE();

	U32 address = SectorAdd;
	for (i = 0; i < Length; i++, address += 4)
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, Buffer[i]) != HAL_OK)
			break;

	HAL_FLASH_Lock();
	if (i < Length)
		return MEMIF_FAIL;

	return MEMIF_OK;
}

static uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if ((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
	{
		sector = FLASH_SECTOR_12;
	}
	else if ((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
	{
		sector = FLASH_SECTOR_13;
	}
	else if ((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
	{
		sector = FLASH_SECTOR_14;
	}
	else if ((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
	{
		sector = FLASH_SECTOR_15;
	}
	else if ((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
	{
		sector = FLASH_SECTOR_16;
	}
	else if ((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
	{
		sector = FLASH_SECTOR_17;
	}
	else if ((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
	{
		sector = FLASH_SECTOR_18;
	}
	else if ((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
	{
		sector = FLASH_SECTOR_19;
	}
	else if ((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
	{
		sector = FLASH_SECTOR_20;
	}
	else if ((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
	{
		sector = FLASH_SECTOR_21;
	}
	else if ((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
	{
		sector = FLASH_SECTOR_22;
	}
	else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
	{
		sector = FLASH_SECTOR_23;
	}

	return sector;
}
