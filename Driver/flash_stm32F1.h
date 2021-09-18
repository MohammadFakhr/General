/* 
	drv_MemWork.h
	Libray to save
	Project: General Library
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/


#ifndef _MEMIF_H
#define _MEMIF_H

#include "global.h"
#include "io.h"

U8 memif_ReadBuffer(U32 SectorAdd, U32 Address, U32 Length, U32 *Buffer);
U8 memif_WriteBuffer(U32 SectorAdd, U32 Length, U32 *Buffer);

#define MEMIF_OK 0
#define MEMIF_FAIL 1

#endif


