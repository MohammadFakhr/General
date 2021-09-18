/* 
	drv_Lcd5110.c
	Driver to handle Mono color Lcd 5110
	Project: General Driver
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/

#include "global.h"

#include "Driver/lcd_Nokia5110.h"

#include "Driver/fonts/font8.h"
#include "Driver/fonts/font8H16.h"
#include "Driver/fonts/font12.h"
#include "Driver/fonts/font16.h"
#include "Driver/fonts/font24.h"

#include "string.h"

extern SPI_HandleTypeDef lcd_nokia5110_hspi;
//*****************************************************************************************************//
//write a byte in Lcd5110 with SPI 
//****************************************************************************************************//
void WriteByte(U8 data)
{
	HAL_SPI_Transmit(&lcd_nokia5110_hspi, &data, 1, 10);
}
//******************************************************************************************
// if lcd was swap on the board, we should flip the data //
//for example if data = "0110 1001" , we flip it with this function to "1001 0110" //
//*****************************************************************************************
U8 flip (U8 in)
{
	U8 och = 0;
	if(in & 0x01)
	{
		och |= 0x80;
	}
	if(in & 0x02)
	{
		och |= 0x40;
	}
	if(in & 0x04)
	{
		och |= 0x20;
	}
	if(in & 0x08)
	{
		och |= 0x10;
	}
	if(in & 0x10)
	{
		och |= 0x08;
	}
	if(in & 0x20)
	{
		och |= 0x04;
	}
	if(in & 0x40)
	{
		och |= 0x02;
	}
	if(in & 0x80)
	{
		och |= 0x01;
	}
	return och;
}

//******************************************************************************************
//set row and column for write a char//
//if lcd was swap on the board, we should define "lcd_5110_swap" on main.h and set x on the "5 - row"
//and set y on "83 - col" ; else we set x and y on the row and col // 
// if lcd was swap on the board, we should flip the data and use this function to write it on lcd, else we write data on lcd//
//*****************************************************************************************
void writeByteXY(U8 data , U8 row, U8 col)
{
	LCD_NOKIA5110_PORT_DC_0;//command;
	if(LCD_NOKIA5110_swap)
	{
		WriteByte(0x40 | (5 - row));
	}
	else
	{
		WriteByte(0x40 | row);	
	}

	if(LCD_NOKIA5110_swap)
	{
		WriteByte(0x80 | (83 - col));
	}
	else
	{
		WriteByte(0x80 | col);
	}
	LCD_NOKIA5110_PORT_DC_1;	

	if(LCD_NOKIA5110_swap)
	{
		U8 dataSwap;
		dataSwap = flip(data);
		WriteByte(dataSwap);
	}
	else
	{
		WriteByte(data);
	}
}
//****************************************************************************************//
// Turn on or off the backlight
void LCD_NOKIA5110_Backlight(U8 state)
{
	if(state)
	{
		LCD_NOKIA5110_PORT_BL_1;    //Set back light
	}
	else
	{
		LCD_NOKIA5110_PORT_BL_0;    //Reset back light
	}
}

//*****************************************************************************************//
//General initialization for LCD
void LCD_NOKIA5110_Reset(void)
{
	LCD_NOKIA5110_Backlight(1);  //Set back light
	LCD_NOKIA5110_PORT_RST_0;    //reset_0;
	HAL_Delay(70);
	LCD_NOKIA5110_PORT_RST_1;    //set_1;
	HAL_Delay(30);
}

void LCD_NOKIA5110_Init(void)
{
	LCD_NOKIA5110_PORT_CS_1;     //ce_set;
	LCD_NOKIA5110_PORT_RST_1;    //reset_1;
	HAL_Delay(10);
	LCD_NOKIA5110_PORT_CS_0;     //ce_reset;
	LCD_NOKIA5110_PORT_DC_0;     //command;
	WriteByte(0x21);
	WriteByte(0xb8);
	WriteByte(0x07);
	WriteByte(0x14);
	WriteByte(0x20);
	WriteByte(0x0c);
	LCD_NOKIA5110_PORT_DC_1;     //data;
	for(int j = 0; j < 504; j++)
	{
		WriteByte(0x00);
	}
	LCD_NOKIA5110_PORT_DC_0;//command;
	WriteByte(0x08);
	WriteByte(0x0c);
	LCD_NOKIA5110_PORT_DC_1; //data
	LCD_NOKIA5110_PORT_CS_1;
}
//*****************************************************************************************//
//clear total contents of LCD
void LCD_NOKIA5110_Clear(U8 inv)
{
	LCD_NOKIA5110_PORT_CS_0;
	if(inv)
	{
		LCD_NOKIA5110_PORT_DC_1;     //data;
		for(U16 j = 0; j < 504; j++)
		{
		   WriteByte(0xff);
		}
	}
	else
	{
		LCD_NOKIA5110_PORT_DC_1;     //data;
		for(U16 j = 0; j < 504; j++)
		{
			WriteByte(0x00);
		}
	}
	LCD_NOKIA5110_PORT_CS_1;
}

//****************************************************************************************************//
//Write a char in specified position with row and col using font8//
U8 LCD_NOKIA5110_Putchar8(char ch, U8 row, U8 col, U8 inv)
{
	U8 i;
	U8 width = Font8[ch - 32][0];
	U8 bytesNo = width + 1;
	U8 endCol = col + width;
	if(endCol > 84)
	{
		return 84;
	}
	LCD_NOKIA5110_PORT_CS_0;

	if(inv)
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			writeByteXY(~Font8[ch - 32][i], row, col++); 
		}
	}
	else
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			writeByteXY(Font8[ch - 32][i], row, col++);
		}
	}
	LCD_NOKIA5110_PORT_CS_1;
	return endCol;
}

//***********************************************************************************************//
//Write a string in specified position using font8
// lastCol == 0 had no effect
U8 LCD_NOKIA5110_Puts8(const char *str, U16 row, U16 col, U8 lastCol, U8 inv)
{
	U8 i = 0;

	while(str[i] != 0)
	{
		col = LCD_NOKIA5110_Putchar8(str[i], row, col, inv);
		i++;
	}
	// Draw blank space after text if lastCol > col
	if(lastCol > 83)
	{
		lastCol = 83;
	}
	while(col < lastCol + 1) 
	{
		col = LCD_NOKIA5110_Putchar8('|', row, col, inv);
	}
	return col;
}

//****************************************************************************************************//
//Write a char in specified position with row and col using font 8H16//
U8 LCD_NOKIA5110_Putchar8H16(char ch, U8 row, U8 col, U8 inv)
{
	U8 i;
	U8 width = Font8H16[ch - 32][0];
	U8 bytesNo = width * 2 + 1;
	U8 endCol = col + width;
	if(endCol > 84)
	{
		return 84;
	}
	LCD_NOKIA5110_PORT_CS_0;

	if(inv)
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(i & 0x01)
			{
				 writeByteXY(~Font8H16[ch - 32][i], row, col);
			}
			else
			{
				writeByteXY(~Font8H16[ch - 32][i], row+1, col++);
			}
		}
	}
	else
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(i & 0x01)
			{
				writeByteXY(Font8H16[ch - 32][i], row, col); 
			}
			else
			{
				writeByteXY(Font8H16[ch - 32][i], row+1, col++);
			}
		}
	}
	LCD_NOKIA5110_PORT_CS_1;
	return endCol;
}

//***********************************************************************************************//
//Write a string in specified position using font 8H16
U8 LCD_NOKIA5110_Puts8H16(const char *str, U16 row, U16 col, U8 lastCol, U8 inv)
{
	U8 i = 0;

	while(str[i] != 0)
	{
		col = LCD_NOKIA5110_Putchar8H16(str[i], row, col, inv);
		i++;
	}
	// Draw blank space after text if lastCol > col
	if(lastCol > 83)
	{
		lastCol = 83;
	}
	while(col < lastCol + 1) 
	{
		col = LCD_NOKIA5110_Putchar16('|', row, col, inv);
	}
	return col;
}

//****************************************************************************************************//
U8 LCD_NOKIA5110_Putchar16(char ch, U8 row, U8 col, U8 inv)
{
	U8 i;
	U8 width = Font16[ch - 32][0];
	U8 bytesNo = width * 2 + 1;
	U8 endCol = col + width;

	LCD_NOKIA5110_PORT_CS_0;
	if(inv)
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(i & 0x01)
			{
				 writeByteXY(~Font16[ch - 32][i], row, col); 
			}
			else
			{
				writeByteXY(~Font16[ch - 32][i], row + 1, col++); 
			}
		}
	}
	else
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(i & 0x01)
			{
				writeByteXY(Font16[ch - 32][i], row, col); 
			}
			else
			{
				writeByteXY(Font16[ch - 32][i], row + 1, col++); 
			}
		}
	}
	LCD_NOKIA5110_PORT_CS_1;
	return endCol;
}
//***********************************************************************************************//
//Write a string in specified position using font16
//***********************************************************************************************//
U8 LCD_NOKIA5110_Puts16(const char *str, U16 row, U16 col, U8 lastCol, U8 inv)
{
	U8 i = 0;

	while(str[i] != 0 && col < 84)
	{
		col = LCD_NOKIA5110_Putchar16(str[i], row, col, inv);
		i++;
	}
	// Draw blank space after text if lastCol > col
	if(lastCol > 83)
	{
		lastCol = 83;
	}
	while(col < lastCol + 1) 
	{
		col = LCD_NOKIA5110_Putchar16('|', row, col, inv);
	}
	return col;
}

//****************************************************************************************************//
//Write a char in specified position with row and col using font12//
U8 LCD_NOKIA5110_Putchar12(char ch, U8 row, U8 col, U8 inv)
{
	U8 i;
	U8 width = Font12[ch - 32][0];
	U8 bytesNo = width * 2 + 1;
	U8 endCol = col + width;
	LCD_NOKIA5110_PORT_CS_0;

	if(inv)
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(i & 0x01)
			{
				writeByteXY((~Font12[ch - 32][i]) & 0xFC, row, col); 
			}
			else
			{
				writeByteXY((~Font12[ch - 32][i]) & 0x7F, row + 1, col++); 
			}
		}
	}
	else
	{
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(i & 0x01)
			{
				writeByteXY(Font12[ch - 32][i], row, col); 
			}
			else
			{
				writeByteXY(Font12[ch - 32][i], row + 1, col++); 
			}
		}
	}
	LCD_NOKIA5110_PORT_CS_1;
	return endCol;
}

//***********************************************************************************************//
//Write a string in specified position using font12
U8 LCD_NOKIA5110_Puts12(const char *str, U16 row, U16 col, U8 lastCol, U8 inv)
{
	U8 i = 0;

	while(str[i] != 0)
	{
		col = LCD_NOKIA5110_Putchar12(str[i], row, col, inv);
		i++;
	}
	// Draw blank space after text if lastCol > col
	if(lastCol > 83)
	{
		lastCol = 83;
	}
	while(col < lastCol + 1) 
	{
		col = LCD_NOKIA5110_Putchar12('|', row, col, inv);
	}
	return col;
}

//*************************************************************************************************//
//Write a char in in specified position using font24
U8 LCD_NOKIA5110_Putchar24(char ch, U16 row, U16 col, U8 inv)
{
	U8 i,k;
	U8 width = Font24[ch - 32][0];
	U8 bytesNo = width	* 3 + 1;
	U8 endCol = col + width;
	LCD_NOKIA5110_PORT_CS_0;
	if(inv)
	{
		k = 1;
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(k == 1)
			{
				 writeByteXY(~Font24[ch - 32][i], row, col);
			}
			else if(k == 2)
			{
				writeByteXY(~Font24[ch - 32][i], row+1, col);
			}
			else if(k == 3)
			{
				writeByteXY(~Font24[ch - 32][i], row+2, col++); 
			}
			if(k < 3)
			{
				k++;
			}
			else
			{
				k = 1;
			}
		}
	}
	else
	{
		k = 1;
		for(i = 1; i < bytesNo && col < 84; i++)
		{
			if(k == 1)
			{
				 writeByteXY(Font24[ch - 32][i], row, col); 
			}
			else if(k == 2)
			{
				writeByteXY(Font24[ch - 32][i], row + 1, col); 
			}
			else
			{
				writeByteXY(Font24[ch - 32][i], row + 2, col++); 
			}
			if(k < 3)
			{
				k++;
			}
			else
			{
				k = 1;
			}
		}
	}
	LCD_NOKIA5110_PORT_CS_1;
	return endCol ;	
}

//****************************************************************************************************//
//Write a string in in specified position using font24
U8 LCD_NOKIA5110_Puts24(const char *str, U16 row, U16 col, U8 lastCol, U8 inv)
{
	U8 i = 0;
	while(str[i] != 0)
	{
		col = LCD_NOKIA5110_Putchar24(str[i], row, col, inv);
		i++;
	}
	// Draw blank space after text if lastCol > col
	if(lastCol > 83)
	{
		lastCol = 83;
	}
	while(col < lastCol + 1) 
	{
		col = LCD_NOKIA5110_Putchar24('|', row, col, inv);
	}	
	return col;
}




