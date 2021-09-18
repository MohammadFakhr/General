/* Internal memory of STM32

drv_MemIF.h Ver1.1

These Adresses most be in main.h defined

#define MEMIF_PAGESIZE

*/ 

#ifndef FLASH_STM32G0_H
#define FLASH_STM32G0_H

#include "main.h"
#include "global.h"

#define INTERNAL_FLASH_OK    0
#define INTERNAL_FLASH_FAIL  1

U8 FLASH_STM32_ReadBuffer(U32 SectorAdd, U32 Address, U32 Length, U32 *Buffer);
U8 FLASH_STM32_WriteBuffer(U32 SectorAdd, U32 Length, U32 *Buffer);



#endif



