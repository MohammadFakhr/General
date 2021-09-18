/* 
	lib_Modbus.h
	Libray to handle modbus
	Project: General Library
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/

#ifndef _DATETIME_H
#define _DATETIME_H

#include "global.h"
#include "io.h"

// Date & Time ----------------------------------
static char strWeekDay[7][8] = {"Mon|", "Tue|", "Wed|", "Thu|", "Fri|", "Sat|", "Sun|"};
static char strWeekDayLong[7][15] = {"Monday|", "Tuesday|", "Wednsday|", "Thursday|", "Friday|", "Saturday|", "Sunday|"};
typedef struct
{
	U16 Year;
	U16 Month;
	U16 Day;
	
	U16 Weekday;//0~6
	
	U16 Hour;
	U16 Minute;
	U16 Second;
	
	U8 ShamsiFlag;// 1: Shamsi, 0: Georgian
	U8 DayLightSaving;
}type_Calendar;

void calendar_Get(type_Calendar *calendar);
void calendar_Set(type_Calendar calendar);

void calendar_Calibrate(U32 freq1Hz, U16 Temp);

U8 calendar_Serialize(const type_Calendar *dt,U32 *buffer, U8 index);
U8 calendar_Unserialize(type_Calendar *dt,U32 *buffer, U8 index);


//U8 calendar_Test();
#endif


