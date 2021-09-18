/*
	lib_Sim800Gprs.h
	Libray to Read from and Write to via Gprs
	Project: GprsModem ML11
	Author : Mohammad Fakhr
	Start Date : 1399.4.16
*/

/* Most be defined in io.h:
	#define gprs_dma
	#define gprs_huart

	#define GPRS_PWRKEY_0
	#define GPRS_PWRKEY_1
*/

#ifndef SIM800GPRS
#define SIM800GPRS

#include "global.h"

#define SIM800_OK 0

#define SIM800_EVENT_POWERON 1
#define SIM800_EVENT_POWEROFF 2

#define SIM800_EVENT_SIMCARD 3
#define SIM800_EVENT_ANTENNA 4

#define SIM800_GPRS_EVENT_INIT_CONTYPE 5
#define SIM800_GPRS_EVENT_INIT_APN 6
#define SIM800_GPRS_EVENT_INIT_OPEN 7
#define SIM800_GPRS_EVENT_INIT_QUERY 8
#define SIM800_GPRS_EVENT_END 9

#define SIM800_GPRS_EVENT_GPRSCONNECTION 10

#define SIM800_GPRS_EVENT_NTP 11
#define SIM800_GPRS_EVENT_INITSERVER 12

#define SIM800_GPRS_EVENT_MAKEDIR 13

#define SIM800_GPRS_EVENT_READOPEN 14
#define SIM800_GPRS_EVENT_READDATA 15
#define SIM800_GPRS_EVENT_LITTLEDATA 16

#define SIM800_GPRS_EVENT_WRITEOPEN 17
#define SIM800_GPRS_EVENT_WRITEDATA 18
#define SIM800_GPRS_EVENT_WRITECLOSE 19

#define SIM800_GPRS_EVENT_DELETEFILE 20

#define SIM800_GPRS_EVENT_CHARGE 21

#define SIM800_SMS_EVENT_SEND 22
#define SIM800_SMS_EVENT_RECIEVE 23
#define SIM800_SMS_EVENT_DELETE 24

static char gprs_ErrorText[24][20] =
{
	"OK"
	, "Poweron"
	, "Poweroff"
	, "Simcard"
	, "Antenna"
	, "ConType"
	, "APN"
	, "InitOpen"
	, "InitQuery"
	, "GprsEnd"
	, "GprsConnect"
	, "NTP"
	, "InitServer"
	, "MakeDir"
	, "ReadOpen"
	, "ReadData"
	, "LittleData"	
	, "WriteOpen"
	, "WriteData"
	, "WriteClose"
	, "Delete"
	, "Charge"
	, "SmsSend"
	, "SmsRecieve"
};

// Gprs Status -----------------------------
typedef struct
{
	U8 Event;
	char Text[20];
	U16 Antena;
	char Gprs[10];
	U32 Charge;
	U8 Wait;
}type_GprsStatus;

void sim800_SetDelayFunction(void (*fun_ptr)(U32));
U8 sim800_PowerOn(void);
U8 sim800_PowerOff(void);

U8 sim800_InitGprs(char apn[]);
U8 sim800_EndGprs(void);

U8 sim800_CheckSim(void);
U8 sim800_CheckAntenna(void);

U8 sim800_SendSms(const char *text, const char *number);
U8 sim800_ReadSms(char *text, char *number, int SmsIndex);
U8 sim800_DeleteSms(int SmsIndex);
U8 sim800_DeleteAllSms();

// Gprs -------------------------------------------------------------------------------------------
U8 sim800_FtpInitServer(char serverName[], char userName[], char passWord[]);

U8 sim800_CheckConnection(void);
U8 sim800_UpdateCalendar(U8 daylight);

U8 sim800_FtpMakeDir(char FilePath[]);

U8 sim800_FtpOpenRead(char FileName[],char FilePath[]);
U8 sim800_FtpReadData(U8 data[], U16 len);
U8 sim800_FtpReadFile(char FileName[],char FilePath[], U8 data[], U16 len);
U8 sim800_FtpCloseRead(void);
U8 sim800_FtpSetBroken(U32 Point);

U8 sim800_FtpOpenWrite(char FileName[],char FilePath[], U8 AppendFlag);
U8 sim800_FtpWriteData(U8 data[], U16 len);
U8 sim800_FtpCloseWrite(void);
U8 sim800_FtpWriteFile(char FileName[],char FilePath[], U8 data[], U16 len,U8 AppendFlag);

U8 sim800_FtpDeleteFile(char FileName[], char FilePath[]);

U8 sim800_CheckCharge(U8 Operator);
#endif






