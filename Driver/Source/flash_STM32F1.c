/* 
	drv_MemIF_F1.c
	Driver to handle internal memory of Stm32F1xx
	Project: General Driver
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/

#include "drv_MemIF_F1.h"
#include "main.h"

FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t PAGEError;

//************************************************************************************//
//
//************************************************************************************//
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
//
//***********************************************************************************//
U8 memif_WriteBuffer(U32 SectorAdd, U32 Length, U32 *Buffer)
{
	uint32_t Check_Buffer[256];
	HAL_FLASH_Unlock();

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR);
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = SectorAdd; //last page is selected
	EraseInitStruct.NbPages = 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
		{
			HAL_FLASH_Lock();
			return MEMIF_FAIL;
		}
	}

	U32 address = SectorAdd;
	int i;
	for (i = 0; i < Length; i++, address += 4)
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, Buffer[i]) != HAL_OK)
			break;	
	HAL_FLASH_Lock();

	if (i < Length)
		return MEMIF_FAIL;
	return MEMIF_OK;
}
