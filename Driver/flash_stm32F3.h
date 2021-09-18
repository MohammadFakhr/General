/* Internal memory of STM32

drv_MemIF.h Ver1.1

These Adresses most be in main.h defined

#define MEMIF_PAGESIZE

*/ 

#ifndef _MEM_IF_F3_H
#define _MEM_IF_F3_H

#include "main.h"
#include "global.h"

#define MEMIF_OK    0
#define MEMIF_FAIL  1

U8 memif_ReadBuffer(U32 SectorAdd, U32 Address, U32 Length, U32 *Buffer);
U8 memif_WriteBuffer(U32 SectorAdd, U32 Length, U32 *Buffer);



#endif



