#include "main.h"
#include "Driver/flash_STM32G0.h"

#include "stm32g0xx_hal_flash_ex.h"

static FLASH_EraseInitTypeDef EraseInitStruct;
static uint32_t PageError;

static uint32_t getPageNo(uint32_t memoryAddress)
{
	return (memoryAddress - FLASH_BASE) / FLASH_PAGE_SIZE;
}
//***********************************************************************************//
//read a uint32_t buffer from an Address of flash memory//
//the Address + length of Buffer should be less than page size //
//the Address should be multiplied by 4 for the offset because the variabe of address is 32 bits or 4 bytes// 
//**********************************************************************************//
uint8_t FLASH_STM32_ReadBuffer(uint32_t SectorAdd, uint32_t Address, uint32_t Length, uint32_t *Buffer)
{
	uint32_t offset;
	offset = SectorAdd + Address * 4;
	for (int j = 0; j < Length; j++)
	{
		Buffer[j] = *(__IO uint32_t*)offset;
		offset += 4;
	}
	return INTERNAL_FLASH_OK;
}

//***********************************************************************************//
//write uint32_t buffer on a Address of flash memory//
//it should be first, read the data from last writting on flash //
//then put the data on the buffer on new address "buf[Address]"//
//erase the sector of flash with "HAL_FLASHEx_Erase" // 
//write buffer on flash memory with a FOR loop. for each write, we should plus address to 4//
//**********************************************************************************//
uint8_t FLASH_STM32_WriteBuffer(uint32_t PageAdd, uint32_t Length, uint32_t *Buffer)
{
	int i;
	uint32_t PageNo;

	HAL_FLASH_Unlock();

	PageNo = getPageNo(PageAdd);

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Page = PageNo;
	EraseInitStruct.NbPages = 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
	{
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
		{
			HAL_FLASH_Lock();
			return INTERNAL_FLASH_FAIL;
		}
	}

	uint64_t data;
	uint32_t address = PageAdd;
	for (i = 0; i < Length; i += 2, address += 8)
	{
		data = ((uint64_t)Buffer[i + 1] << 32) + Buffer[i];
		
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data) != HAL_OK)
		{
			break;
		}
	}
	HAL_FLASH_Lock();
	if (i < Length)
	{
		return INTERNAL_FLASH_FAIL;
	}
	return INTERNAL_FLASH_OK;
}




