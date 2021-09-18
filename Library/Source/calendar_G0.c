/* 
	lib_TimeDate.c
	Libray to handle modbus
	Project: General Library
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/

#include "Library/calendar.h"

extern RTC_HandleTypeDef hrtc;

void GeorgianToShamsi(type_Calendar georgian, type_Calendar *shamsi);
void ShamsiToGeorgian(type_Calendar shamsi, type_Calendar *georgian);

void calendar_Calibrate(U32 freq1Hz, U16 Temp)
{
	U32 add,sub, ppm;
	int t = Temp;
	t -= 25;
	t *= t;
	int ppmTemp = 32 * t / 1000;

	freq1Hz -= ppmTemp;
	if(freq1Hz > 1000000)
	{
		add = RTC_SMOOTHCALIB_PLUSPULSES_RESET;
		ppm = freq1Hz - 1000000;
		sub = ppm * 1048576 / 1000000;
	}
	else if(freq1Hz < 1000000)
	{
		add = RTC_SMOOTHCALIB_PLUSPULSES_SET;
		ppm = 1000000 - freq1Hz;
		sub = 512 - (ppm * 1048576 / 1000000);
	}
	else
	{
		add = RTC_SMOOTHCALIB_PLUSPULSES_RESET;
		sub = 0;
	}
	HAL_RTCEx_SetSmoothCalib(&hrtc,RTC_SMOOTHCALIB_PERIOD_32SEC, add, sub);
}

// Calendar object --------------------------------------------------------------
U8 calendar_Serialize(const type_Calendar *dt, U32 *buffer, U8 index)
{
	buffer[index++] = dt->Year;
	buffer[index++] = dt->Month;
	buffer[index++] = dt->Day;
	buffer[index++] = dt->Weekday;	
	buffer[index++] = dt->Hour;
	buffer[index++] = dt->Minute;
	buffer[index++] = dt->Second;
	buffer[index++] = dt->ShamsiFlag;

	return index;
}

U8 calendar_Unserialize(type_Calendar *dt,U32 *buffer, U8 index)
{
	dt->Year = buffer[index++];
	dt->Month = buffer[index++];
	dt->Day = buffer[index++];
	dt->Weekday = buffer[index++];
	dt->Hour = buffer[index++];
	dt->Minute = buffer[index++];
	dt->Second = buffer[index++];
	dt->ShamsiFlag = buffer[index++];
	
	return index;
}

// Day Ligth Saving functions --------------------------
//void HAL_RTC_DST_Add1Hour(RTC_HandleTypeDef *hrtc)
//{
//  UNUSED(hrtc);
//  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
//  SET_BIT(RTC->CR, RTC_CR_ADD1H);
//  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
//}
//void HAL_RTC_DST_Sub1Hour(RTC_HandleTypeDef *hrtc)
//{
//  UNUSED(hrtc);
//  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
//  SET_BIT(RTC->CR, RTC_CR_SUB1H);
//  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
//}
//uint32_t HAL_RTC_DST_ReadStoreOperation(RTC_HandleTypeDef *hrtc)
//{
//  UNUSED(hrtc);
//  return READ_BIT(RTC->CR, RTC_CR_BKP);
//}
//void HAL_RTC_DST_SetStoreOperation(RTC_HandleTypeDef *hrtc)
//{
//  UNUSED(hrtc);
//  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
//  SET_BIT(RTC->CR, RTC_CR_BKP);
//  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
//}
//void HAL_RTC_DST_ClearStoreOperation(RTC_HandleTypeDef *hrtc)
//{
//  UNUSED(hrtc);
//  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
//  CLEAR_BIT(RTC->CR, RTC_CR_BKP);
//  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
//}

// Read current date/time
RTC_DateTypeDef date;
RTC_TimeTypeDef time;
void calendar_Get(type_Calendar *calendar)
{
	type_Calendar tempDT;

	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	tempDT.Year = date.Year;
	tempDT.Month = date.Month;
	tempDT.Day = date.Date;
	tempDT.Weekday = date.WeekDay;

	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	tempDT.Hour = time.Hours;
	tempDT.Minute = time.Minutes;
	tempDT.Second = time.Seconds;

	if(calendar->ShamsiFlag)
	{
		GeorgianToShamsi(tempDT, calendar);	
		if(calendar->DayLightSaving)
		{
			// Daylight handle
			if((((calendar->Month > 6) || ((calendar->Month == 6) && (calendar->Day > 30))) ||
				((calendar->Month == 1) && (calendar->Day == 1))))
			{
				if(HAL_RTC_DST_ReadStoreOperation(&hrtc) != 0)
				{
					if(calendar->Hour > 0)
					{
						HAL_RTC_DST_Sub1Hour(&hrtc);
						HAL_RTC_DST_ClearStoreOperation(&hrtc);
					}
				}
			}
			else
			{
				if(HAL_RTC_DST_ReadStoreOperation(&hrtc) == 0)
				{
					HAL_RTC_DST_Add1Hour(&hrtc);
					HAL_RTC_DST_SetStoreOperation(&hrtc);
				}
			}
		}
	}
	else
	{
		if(tempDT.Year > 50)
		{
			calendar->Year = tempDT.Year + 1900;
		}
		else
		{
			calendar->Year = tempDT.Year + 2000;
		}
		calendar->Month= tempDT.Month;
		calendar->Day = tempDT.Day;
		calendar->Weekday = tempDT.Weekday;
		calendar->Hour = tempDT.Hour;
		calendar->Minute = tempDT.Minute;
		calendar->Second = tempDT.Second;
	}
}

// Update date/time
void calendar_Set(type_Calendar calendar)
{
	type_Calendar tempDT;

	// Convert if date is shamsi
	if(calendar.ShamsiFlag)
	{
		ShamsiToGeorgian(calendar, &tempDT);
	}
	else
	{
		tempDT = calendar;
	}

	// Set date
	date.Year = tempDT.Year % 100;
	date.Month = tempDT.Month;
	date.Date = tempDT.Day;
	date.WeekDay = tempDT.Weekday;
	HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);

	// Set time
	time.Hours = tempDT.Hour;
	time.Minutes = tempDT.Minute;
	time.Seconds = tempDT.Second;
	time.TimeFormat = 0;
	HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);

	if(calendar.DayLightSaving && calendar.ShamsiFlag)
	{
		if( (calendar.Month > 6) ||
			((calendar.Month == 6) && (calendar.Day > 30)) ||
			((calendar.Month == 1) && (calendar.Day == 1)))
		{
			HAL_RTC_DST_ClearStoreOperation(&hrtc);
		}
		else
		{
			HAL_RTC_DST_SetStoreOperation(&hrtc);
		}
	}
}

static U8 shmtable[6][12]=
{
	{11, 10, 10, 9, 9, 9, 8, 9, 9, 10, 11, 9},
	{20, 20, 21, 21 ,22, 22, 22, 22, 21, 21, 20, 19},
	{11, 10, 10, 9, 9, 9, 8, 9, 9, 10, 11, 10},
	{20, 20, 21, 21, 22, 22, 22, 22, 21, 21, 20, 19},
	{12, 11, 11, 10, 10, 10, 9, 10, 10, 11, 12, 10},
	{19, 19, 20, 20, 21, 21, 21, 21, 20, 20, 19, 18}
};

static U8 miltable[6][12]=
{
	{20, 19, 19, 19, 20, 20, 21, 21, 21, 21, 20, 20},
	{10, 11, 10, 12, 11, 11, 10, 10, 10, 9, 10, 10},
	{19, 18, 20, 20, 21, 21, 22, 22, 22, 22, 21, 21},
	{11, 12, 10, 11, 10, 10, 9, 9, 9, 8, 9, 9},
	{20, 19, 20, 20, 21, 21, 22, 22, 22, 22, 21, 21},
	{10, 11, 9, 11, 10, 10, 9, 9, 9, 8, 9, 9}
};

void ShamsiToGeorgian(type_Calendar shamsi, type_Calendar *georgian)
{
	U8 k,t1,t2;

	shamsi.Year %= 100;
    k = shamsi.Year % 4;
    if(k == 0)
	{
		k = 2;
	}
    else
	{
		k = k + k;
	}
    t1 = shmtable[k - 2][shamsi.Month-1];
    t2 = shmtable[k-1][shamsi.Month-1];
    if((shamsi.Month < 10) || ((shamsi.Month == 10) && (shamsi.Day <= shmtable[k - 2][shamsi.Month - 1])))
    {
		if(shamsi.Year >= 79)
		{
			georgian->Year = shamsi.Year - 79;
		}
		else
		{
			georgian->Year = shamsi.Year + 21;
		}
	}
    else
    {
		if(shamsi.Year >= 78)
		{
			georgian->Year = shamsi.Year - 78;
		}
		else  
		{
			georgian->Year = shamsi.Year + 22;
		}
	}

    if(shamsi.Day <= t1)
    {
       georgian->Day = shamsi.Day + t2;
       georgian->Month = (shamsi.Month + 1 ) % 12 + 1;
    }
    else
    {
       georgian->Day = shamsi.Day - t1;
       georgian->Month = (shamsi.Month + 2) %12 + 1;
    }

	if(georgian->Year < 70)
	{
		georgian->Year += 2000;
	}
	else
	{
		georgian->Year += 1900;
	}
	
	georgian->ShamsiFlag = 0;
	
	georgian->Weekday = shamsi.Weekday;
	georgian->Hour = shamsi.Hour;
	georgian->Minute = shamsi.Minute;
	georgian->Second = shamsi.Second;
}

void GeorgianToShamsi(type_Calendar georgian, type_Calendar *shamsi)
{
	U8 k,t1,t2;

	georgian.Year %= 100;

    k = georgian.Year % 4;
    if(k == 3)
	{
		k = 2;
	}
    k *= 2;
    t1 = miltable[k][georgian.Month - 1];
    t2 = miltable[k + 1][georgian.Month - 1];

	if((georgian.Month < 3) || ((georgian.Month == 3) && (georgian.Day <= miltable[k][georgian.Month - 1])))
	{
		shamsi->Year = georgian.Year + 78;
	}
    else
	{
		shamsi->Year = georgian.Year + 79;
	}
	
    if(georgian.Day <= t1)
    {
       shamsi->Day = georgian.Day + t2;
       shamsi->Month = (georgian.Month + 8) % 12 + 1;
    }
    else
    {
       shamsi->Day = georgian.Day - t1;
       shamsi->Month = (georgian.Month + 9) % 12 + 1;
    }

	if(shamsi->Year < 50)
	{
		shamsi->Year += 1400;
	}
	else
	{
		shamsi->Year += 1300;
	}
	
	shamsi->Weekday = georgian.Weekday;
	shamsi->Hour = georgian.Hour;
	shamsi->Minute = georgian.Minute;
	shamsi->Second = georgian.Second;
}





