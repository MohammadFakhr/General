/*



	CAL_TEMP30 and CAL_TEMP110 most be defined
*/


#ifndef LIB_MICRO
#define LIB_MICRO

#include "global.h"

U16 micro_CalculateTemp(U16 TempAdcVal, U16 VrefAdcVal);
U16 micro_CalculateVRtc(U16 VrtcAdcVal, U16 VrefAdcVal);

#endif
