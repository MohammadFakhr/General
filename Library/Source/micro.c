
#include "Library/Micro.h"
#include "global.h"
#include "io.h"

double Vcc;
	
U16 micro_CalculateTemp(U16 TempAdcVal, U16 VrefAdcVal)
{
	int VrefIntCal = *CAL_VREF;
	int Temp30ACal = *CAL_TEMP30;
	int Temp110Cal = *CAL_TEMP110;

	int VrefInt = VCAL * VrefIntCal * 1000 / 4095;

	int VrefExt = VrefInt * 4095 / VrefAdcVal;
	int Vtemp = TempAdcVal * VrefExt / 4095;

//	 Vcc = ( VrefIntCal * 4095) / VrefAdcVal;

	int Vcal30 = Temp30ACal * VCAL * 1000 / 4095;// * VrefExt / VCAL;
	int Vcal110 = Temp110Cal * VCAL * 1000 / 4095;// * VrefExt / VCAL;
	int a = 30 + (Vtemp - Vcal30) * 80 / (Vcal110 - Vcal30);
	return (U16) a;
}

U16 micro_CalculateVRtc(U16 VrtcAdcVal, U16 VrefAdcVal)
{
	int cal_ref = *CAL_VREF;
	
	int VrefInt = VCAL * cal_ref * 1000 / 4095;
	
	int VrefExt = VrefInt * 4095 / VrefAdcVal;	
	int a = VrtcAdcVal * VrefExt * 2 / 409500;// ::/4095/100

	return (U16) a;	
}



