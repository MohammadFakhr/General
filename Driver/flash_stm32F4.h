/* Internal memory of STM32

drv_MemIF.h Ver1.1

These Adresses most be in main.h defined
#define MEMIF_SECTORADDRESS 
#define MEMIF_SECTORADDRESS 
#define MEMIF_SECTORADDRESS 
#define MEMIF_SECTORADDRESS 
#define MEMIF_SECTORADDRESS 

#define MEMIF_PAGESIZE

*/ 

#ifndef _MEM_IF_F4_H
#define _MEM_IF_F4_H

#include "main.h"

#define MEMIF_OK    0
#define MEMIF_FAIL  1

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base @ of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base @ of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base @ of Sector 11, 128 Kbytes */

//The Address is based on 32 bit data
//2KB Sector has 512 address

U8 memif_ReadBuffer(U32 SectorAdd, U32 Address, U32 Length, U32 *Buffer);
U8 memif_WriteBuffer(U32 SectorAdd, U32 Length, U32 *Buffer);



#endif



