/* 
	drv_Lcd5110.c
	Driver to handle Mono color Lcd 5110
	Project: General Driver
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/
/* Most be defined in global.h:

	#define LCD5110_SWAP => to swap LCD

*/
/* Most be defined in io.h:
	#define LCD_NOKIA5110_hspi : spi handle

	#define LCD5110_PORT_DC_0
	#define LCD5110_PORT_DC_1
	#define LCD5110_PORT_BL_0
	#define LCD5110_PORT_BL_1
	#define LCD5110_PORT_CS_0
	#define LCD5110_PORT_CS_1
	#define LCD5110_PORT_RST_0
	#define LCD5110_PORT_RST_1
*/

#ifndef _LCD5110_H
#define _LCD5110_H

#include "main.h"
#include "io.h"
#include "global.h"

//*******************************************************************************//
void LCD_NOKIA5110_Backlight(U8 state);
void LCD_NOKIA5110_Reset(void);
void LCD_NOKIA5110_Init(void);
void LCD_NOKIA5110_Clear(U8 inv);

U8 LCD_NOKIA5110_Putchar8(char ch, U8 row, U8 col, U8 inv);
U8 LCD_NOKIA5110_Puts8(const char *str, U16 row, U16 col, U8 lastCol, U8 inv);

U8 LCD_NOKIA5110_Putchar8H16(char ch, U8 row, U8 col, U8 inv);
U8 LCD_NOKIA5110_Puts8H16(const char *str, U16 row, U16 col, U8 lastCol, U8 inv);

U8 LCD_NOKIA5110_Putchar9(char ch, U8 row, U8 col,U8 inv);
U8 LCD_NOKIA5110_Puts9(const char *str, U16 row, U16 col, U8 lastCol, U8 inv);

U8 LCD_NOKIA5110_Putchar12(char ch, U8 row, U8 col,U8 inv);
U8 LCD_NOKIA5110_Puts12(const char *str, U16 row, U16 col, U8 lastCol, U8 inv);

U8 LCD_NOKIA5110_Putchar16(char ch, U8 row, U8 col, U8 inv);
U8 LCD_NOKIA5110_Puts16(const char *str, U16 row, U16 col, U8 lastCol, U8 inv);

U8 LCD_NOKIA5110_Putchar24(char ch, U16 row, U16 col, U8 inv);
U8 LCD_NOKIA5110_Puts24(const char *str, U16 row, U16 col, U8 lastCol, U8 inv);

#endif

