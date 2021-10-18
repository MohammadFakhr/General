
#include "Library/sim800.h"
#include "Library/calendar.h"
#include "Driver/uart_dma.h"

#include "string.h"
#include "stdio.h"

#define TIMEOUT 200

extern UART_HandleTypeDef sim800_huart;
extern DMA_HandleTypeDef sim800_dma;

type_Sim800Status Sim800Status = {0, "", 0,"...", 0, 0};

extern void sim800Delay(U32);
// Array of bytes to holding recieved responses from SIM800
const int responseLen = 150;
char AtResponse[responseLen]={0};
/**
Send At Command to sim800 and recieve Module
 * @Command	AT command to send
 * @ValidResponse Valid AtResponse to recieve
 * @ResponseLen maximum AtResponse len
 * @RetryNo Number of retries
 * @RetryDelay Delay between each retry

 * @retval = 0 if ValidResoponse is in the recieved text and 1 if not
*/
U8 AtSendRecieve(U8 TxBuf[], U16 TxLen, U8 RxBuf[], U16 RxLen, char ValidRx[], int Timeout,  U8 RetryNo, U32 RetryDelay, U16 DelayAfterRx)
{
	UART_DMA_HandleTypeDef huart_dma;
	U8 result = 0;

	Sim800Status.Wait = 1;
	for(int i = 0; i < RetryNo; i++)
	{
		for(int i = 0; i < RxLen - 1; i++)
		{
			RxBuf[i] = 0xFF;
		}
		RxBuf[RxLen - 1] = 0;

		huart_dma.huart = &sim800_huart;
		huart_dma.hdma = &sim800_dma;
		huart_dma.txData = TxBuf;
		huart_dma.rxData = RxBuf;
		huart_dma.TxLength = TxLen;
		huart_dma.RxLength = RxLen;
		huart_dma.RxWaitTime = Timeout;
		huart_dma.expectedAnswer = ValidRx;
		huart_dma.DelayAfterAnswer = DelayAfterRx;
		huart_dma.Delay_func = sim800Delay;
//		if(strstr((char*)RxBuf, ValidRx) == 0)
		if(UART_DMA_Transfer(huart_dma) == 0)// Response not found
		{
			result = 1;
			sim800Delay(RetryDelay);
		}
		else
		{
			result = 0;
			break;
		}
	}
	Sim800Status.Wait = 0;
	return result;
}
/**
Send At Data to and from sim800
 * @writeData data to send
 * @Len	Length of data
 * @ValidResponse Valid AtResponse to recieve
 * @ResponseLen maximum AtResponse len
 * @RetryNo Number of retries
 * @RetryDelay Delay between each retry

 * @retval = 0 if ValidResoponse is in the recieved text and 1 if not
*/
U8 AtCommand(char command[], char validResponse[], int timeout, U8 retryNo, U32 retryDelay, U16 DelayAfterRx)
{
	return AtSendRecieve((U8*)command, strlen(command), (U8*)AtResponse, responseLen, validResponse, timeout, retryNo, retryDelay, DelayAfterRx);
}

// Power on module --------------------------------------------------------------------------------
U8 sim800_PowerOn(void)
{
	SIM800_ON;

	// Turn on module with PoweKey if it is off
	if(AtCommand("AT\r\n", "OK", 1000, 2, 2000, 100))
	{
		SIM800_PWRKEY_1;
		sim800Delay(500);
		SIM800_PWRKEY_0;
		sim800Delay(2000);
		SIM800_PWRKEY_1;
		sim800Delay(5000);
		if(AtCommand("AT\r\n", "OK", 2000, 5, 3000, 100))
		{
			return SIM800_EVENT_POWERON;
		}
	}
	// Turn off Echo
	if(AtCommand("ATE0\r\n", "OK", 3000, 2, 3000, 100))
	{
		return SIM800_EVENT_POWERON;
	}
	if(AtCommand("AT+CMGF=1\r\n", "OK", 3000, 2, 3000, 100))
	{
		return SIM800_EVENT_POWERON;
	}

	if(AtCommand("AT+CSCS=\"GSM\"\r\n", "OK", 3000, 2, 3000, 100))
	{
		return SIM800_EVENT_POWERON;
	}
//	if(AtCommand("AT+CSMP=17,167,0,0\r\n", "OK", 3000, 2, 3000, 100))
//	{
//		return SIM800_EVENT_POWERON;
//	}
	if(AtCommand("AT+CSAS=0\r\n", "OK", 5000, 2, 5000, 100))
	{
		return SIM800_EVENT_POWERON;
	}
	
	return SIM800_OK;
}

// Power off module -------------------------------------------------------------------------------
U8 sim800_PowerOff(void)
{
	Sim800Status.Antena = 0;
	

	// Set Baud Rate
	AtCommand("AT\r\n", "OK", 2000, 2, 1000, 100);
	// Power down Module
	if(AtCommand("AT+CPOWD=1\r\n", "NORMAL", 1000, 1, 0, 100))
	{
		// Hard shutdown if neccessary
		AtCommand("AT+CPOWD=0\r\n", "", 2000, 1, 0, 100);
	}
	SIM800_OFF;
	SIM800_PWRKEY_1;
	sim800Delay(3000);

	return SIM800_OK;
}

// Initialize gprs conection ----------------------------------------------------------------------
U8 sim800_InitGprs(char apn[])
{
	char str[100];
	
	if(AtCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", "OK", 5000, 2, 5000, 1000))
	{
		return SIM800_GPRS_EVENT_INIT_CONTYPE;
	}
	sprintf(str, "AT+SAPBR=3,1,\"APN\",\"%s\"\r\n", apn);
	if(AtCommand(str, "OK", 5000, 2, 5000, 1000))
	{
		return SIM800_GPRS_EVENT_INIT_APN;
	}
	if(AtCommand("AT+SAPBR=1,1\r\n", "OK", 60000, 2, 10000, 1000))
	{
		return SIM800_GPRS_EVENT_INIT_OPEN;
	}
	if(AtCommand("AT+SAPBR=2,1\r\n", "+SAPBR: 1,1,", 10000, 2, 10000, 1000))
	{
		return SIM800_GPRS_EVENT_INIT_QUERY;
	}
	return SIM800_OK;
}
// Ending gprs conection --------------------------------------------------------------------------
U8 sim800_EndGprs(void)
{
	if(AtCommand("AT+SAPBR=0,1\r\n", "OK", 3000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_END;
	}
	return SIM800_OK;
}

U8 sim800_CheckSim(void)
{
	if(AtCommand("AT+CSMINS?\r\n", "+CSMINS: 0,1", 5000, 2, 30000, 100))
	{
		return SIM800_EVENT_SIMCARD;
	}
	return SIM800_OK;
}

U8 sim800_CheckAntenna(void)
{
	U8 ant = 0;
	if(AtCommand("AT+CSQ\r\n", "+CSQ:", 3000, 2, 10000, 100))
	{
		return SIM800_EVENT_ANTENNA;
	}
	if( strstr( AtResponse, "+CSQ: 99,1" ) || 
		strstr( AtResponse, "+CSQ: 0,1" ) || 
		strstr( AtResponse, "+CSQ: 1,1" ) )
	{
		Sim800Status.Antena = 0;
		return SIM800_EVENT_ANTENNA;
	}
	// Calculate Antenna percent
	char *sTemp;
	sTemp = strstr(AtResponse, ": ");
	sTemp += 2;
	if(sTemp[1] == ',')
	{
		ant = sTemp[0] - '0';
	}
	else
	{
		ant = (sTemp[0] - '0') * 10;
		ant += (sTemp[1] - '0');
	}
	if((ant > 1) &&(ant < 31))
	{
		Sim800Status.Antena = (ant - 2) * 100 / 28;
	}
	else if(ant == 31)
	{
		Sim800Status.Antena = 100;
	}
	else
	{
		Sim800Status.Antena = 0;
	}
	return SIM800_OK;
}

// Send sms ---------------------------------------------------------------------------------------
U8 sim800_SendSms(const char *text, const char *number)
{
	char str[50];
	if(AtCommand("AT+CMGF=1\r\n", "OK", 5000, 2, 5000, 100))
	{
		return SIM800_SMS_EVENT_SEND;
	}
	sprintf(str, "AT+CMGS=\"%s\"\r\n", number);
	AtCommand(str, "", 3000, 1, 5000, 100);
	
	sim800Delay(500);
	AtCommand((char*)text, "", 3000, 1, 5000, 100);
	strcpy(str, "\r\nx");
	str[2] = 0x1a;
	AtCommand(str, "", 3000, 1, 5000, 100);

	return SIM800_OK;
}
// Read new sms -----------------------------------------------------------------------------------
U8 sim800_ReadSms(char *text, char *number, int SmsIndex)
{
	char strCmd[50];
	char* str;
	int i = 0;
	
	sprintf(strCmd, "AT+CMGR=%d\r\n", SmsIndex);
	if(AtCommand(strCmd, "+CMGR", 10000, 2, 5000, 1000))
	{
		return SIM800_SMS_EVENT_RECIEVE;
	}
	
	// Finde number
	str = strstr(AtResponse, "+");
	str = strstr(str + 1, "+");
	
	while((*str != '\"') && ( i < 25))
	{
		number[i++] = *str;
		str++;
	}
	number[i] = 0;
	
	// FInding Sms text
	str = strstr(str, "\r\n");
	str += 2;
	i = 0;
	while((*str != '\r') && ( i < 160))
	{
		text[i++] = *str;
		str++;
	}
	text[i] = 0;
	
	sim800Delay(1000);

	return SIM800_OK;
}

U8 sim800_DeleteSms(int SmsIndex)
{
	char str[50];
	sprintf(str, "AT+CMGD=%d\r\n", SmsIndex);
	if(AtCommand(str, "OK", 10000, 2, 5000, 1000))
	{
		return SIM800_SMS_EVENT_RECIEVE;
	}
	sim800Delay(1000);

	return SIM800_OK;
}

U8 sim800_DeleteAllSms()
{
	char str[50];
	if(AtCommand("AT+CMGDA=\"DEL ALL\"\r\n", "OK", 10000, 2, 5000, 1000))
	{
		return SIM800_SMS_EVENT_DELETE;
	}
	sim800Delay(1000);

	return 0;	
}

// Initila server ---------------------------------------------------------------------------------
U8 sim800_FtpInitServer(char serverName[], char userName[], char passWord[])
{
	char strCmd[100];
	if(AtCommand("AT+FTPCID=1\r\n", "OK", 2000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_INITSERVER;
	}
	if(AtCommand("AT+FTPTYPE=\"I\"\r\n", "OK", 2000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_INITSERVER;
	}
	// Server -----------------------------------
	sprintf(strCmd, "AT+FTPSERV=\"%s\"\r\n", serverName);
	if(AtCommand(strCmd, "OK", 2000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_INITSERVER;
	}
	// User Name --------------------------------
	sprintf(strCmd, "AT+FTPUN=\"%s\"\r\n", userName);
	if(AtCommand(strCmd, "OK", 2000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_INITSERVER;
	}
	// Password ---------------------------------
	sprintf(strCmd, "AT+FTPPW=\"%s\"\r\n", passWord);
	if(AtCommand(strCmd, "OK", 2000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_INITSERVER;
	}
	return SIM800_OK;
}
// Check Gprs Connection --------------------------------------------------------------------------
U8 sim800_CheckConnection()
{
	if(AtCommand("AT+SAPBR=2,1\r\n", "+SAPBR: 1,1,", 5000, 2, 3000, 100))
	{
		strcpy(Sim800Status.Gprs, "fail");
		return SIM800_GPRS_EVENT_GPRSCONNECTION;
	}
	strcpy(Sim800Status.Gprs, "OK");
	return SIM800_OK;
}
// Update time and date from net
char ntpList[][20]=
{
	"ntp.day.ir",
	"0.asia.pool.ntp.org",
	"1.asia.pool.ntp.org",
	"2.asia.pool.ntp.org",
	"3.asia.pool.ntp.org"
};


U8 sim800_UpdateCalendar(U8 daylight)
{
	U8 result;
	char str[50];
	type_Calendar calendar;
	for(int i = 0; i < 5; i++)
	{
		AtCommand("AT+CNTPCID=1\r\n", "", 3000, 1, 3000, 100);
		sprintf(str, "AT+CNTP=\"%s\",14\r\n", ntpList[i]);
		AtCommand(str, "", 3000, 1, 3000, 0);
		if(AtCommand("AT+CNTP\r\n", "+CNTP: 1", 3000, 1, 3000, 100))
		{
			result = SIM800_GPRS_EVENT_NTP;
			sim800Delay(1000);
		}
		else
		{
			if(AtCommand("AT+CCLK?\r\n", "+CCLK:", 3000, 1, 3000, 500))
			{
				result = SIM800_GPRS_EVENT_NTP;
				sim800Delay(1000);
			}
			else
			{
				char *cal;
				cal = strstr(AtResponse, "+CCLK:") + 8;
				calendar.Weekday = 1;
				calendar.DayLightSaving = daylight;
				
				calendar.Year = (cal[0] - '0') * 10 + (cal[1] - '0');
				calendar.Month = (cal[3] - '0') * 10 + (cal[4] - '0');
				calendar.Day = (cal[6] - '0') * 10 + (cal[7] - '0');
				calendar.Hour = (cal[9] - '0') * 10 + (cal[10] - '0');
				calendar.Minute = (cal[12] - '0') * 10 + (cal[13] - '0');
				calendar.Second = (cal[15] - '0') * 10 + (cal[16] - '0');
				
				calendar.ShamsiFlag = 0;

				// Correct time if day light is active
				//if(calendar.Hour < 23)
				{
					calendar_Set(calendar);
					//result = SIM800_GPRS_EVENT_NTP;
//				}
//				else
//				{
					result = SIM800_OK;
				}
				break;
			}
		}
	}
	return result;
}

U8 sim800_FtpMakeDir(char filePath[])
{
	if((filePath[0] != '/') && (filePath[strlen(filePath) - 1] != '/'))
	{
		return SIM800_GPRS_EVENT_MAKEDIR;
	}

	char strPath[100];
	char strCmd[100];
	strcpy(strPath, filePath);
	for(int i = 1; i < strlen(filePath); i++)
	{
		if(strPath[i] == '/')
		{
			strPath[i] = 0;
			sprintf(strCmd, "AT+FTPGETPATH=\"%s\"\r\n", strPath);
			if(AtCommand(strCmd, "OK", 3000, 2, 0, 100))
			{
				return SIM800_GPRS_EVENT_MAKEDIR;
			}
			if(AtCommand("AT+FTPMKD\r\n", "+FTPMKD: 1,", 5000, 2, 0, 200))
			{
				return SIM800_GPRS_EVENT_MAKEDIR;
			}
			// return error if there is error
			if((strstr(AtResponse, "+FTPMKD: 1,0") == 0) &&
			   (strstr(AtResponse, "+FTPMKD: 1,77") == 0))
			{
				return SIM800_GPRS_EVENT_MAKEDIR;
			}
			strPath[i] = '/';
		}
	}
	return SIM800_OK;
}

// Open file to read ------------------------------------------------------------------------------
U8 sim800_FtpOpenRead( char fileName[], char filePath[])
{
	char str[100];
	// File name --------------------------------
	sprintf(str, "AT+FTPGETNAME=\"%s\"\r\n", fileName);
	if(AtCommand(str, "OK", 5000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_READOPEN;
	}
	// Path -------------------------------------
	sprintf(str, "AT+FTPGETPATH=\"%s\"\r\n", filePath);
	if(AtCommand(str, "OK", 5000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_READOPEN;
	}
	// Open connection
	if(AtCommand("AT+FTPGET=1\r\n", "+FTPGET: 1,", 30000, 2, 30000, 100))
	{
		return SIM800_GPRS_EVENT_READOPEN;
	}
	if(strstr(AtResponse, "+FTPGET: 1,1") == 0)
	{
		return SIM800_GPRS_EVENT_READOPEN;
	}
	return SIM800_OK;
}
// Open file to write -----------------------------------------------------------------------------
U8 sim800_FtpOpenWrite( char fileName[], char filePath[], U8 appendFlag)
{
	char str[100];
	// Close File if it is open
	AtCommand("AT+FTPPUT=2,0\r\n", "+FTPPUT: 1,0", 5000, 1, 0, 100);
	// File name --------------------------------
	sprintf(str, "AT+FTPPUTNAME=\"%s\"\r\n", fileName);
	if(AtCommand(str, "OK", 5000, 2, 3000, 200))
	{
		return SIM800_GPRS_EVENT_WRITEOPEN;
	}
	// Path -------------------------------------
	sprintf(str, "AT+FTPPUTPATH=\"%s\"\r\n", filePath);
	if(AtCommand(str, "OK", 5000, 2, 3000, 200))
	{
		return SIM800_GPRS_EVENT_WRITEOPEN;
	}
	// Append -------------------------------------
	if(appendFlag)
	{
		sprintf(str, "AT+FTPPUTOPT=\"APPE\"\r\n");
	}
	else
	{
		sprintf(str, "AT+FTPPUTOPT=\"STOR\"\r\n");
	}
	if(AtCommand(str, "OK", 3000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_WRITEOPEN;
	}
	// Open connection
	if(AtCommand("AT+FTPPUT=1\r\n", "+FTPPUT: 1,1,1360", 20000, 2, 10000, 100))
	{
		return SIM800_GPRS_EVENT_WRITEOPEN;
	}
	return SIM800_OK;
}

// Open Read data -----------------------------------------------------------------------------
// *note: size of 'data' array most be larger than len + 30
U8 sim800_FtpReadData(U8 readData[], U16 len)
{
	char strCmd[50];
	char strRes[50];
	char *str;
	// Send write command
	sprintf(strCmd, "AT+FTPGET=2,%d\r\n", len);
	sprintf(strRes, "+FTPGET: 2,");
	// Read data
	if(AtSendRecieve((U8*)strCmd, strlen(strCmd), readData, len + 30, strRes, 10000, 2, 10000, len * 2))
	{
		return SIM800_GPRS_EVENT_READDATA;
	}
	str = strstr((char*)readData, "+FTPGET:2,0");
	if(str != 0)
	{
		// Retry
		if(AtSendRecieve((U8*)strCmd, strlen(strCmd), readData, len + 30, strRes, 10000, 2, 10000, len * 2))
		{
			return SIM800_GPRS_EVENT_READDATA;
		}
	}
	
	sprintf(strRes, "+FTPGET: 2,%d", len);
	str = strstr((char*)readData, strRes);
	if(str == 0)
	{
		return SIM800_GPRS_EVENT_LITTLEDATA;
	}
	for(int i = 0; i < len; i++)
	{
		readData[i] = str[strlen(strRes) + 2 + i];
	}
	return SIM800_OK;
}
// Set Broken Point  -----------------------------------------------------------------------------
U8 sim800_FtpSetBroken(U32 Point)
{
	char strCmd[50];
	// Send write command
	sprintf(strCmd, "AT+FTPREST=%d\r\n", Point);
	if(AtCommand(strCmd, "OK", 5000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_READOPEN;
	}
	if(AtCommand("AT+FTPGET=1\r\n", "+FTPGET: 1,1", 20000, 2, 10000, 100))
	{
		return SIM800_GPRS_EVENT_READOPEN;
	}
	return SIM800_OK;
}

// Open file to write -----------------------------------------------------------------------------
U8 sim800_FtpWriteData(U8 writeData[], U16 len)
{
	char strCmd[50];
	char strRes[50];
	// Send write command
	sprintf(strCmd, "AT+FTPPUT=2,%d\r\n", len);
	sprintf(strRes, "+FTPPUT: 2,%d", len);
	if(AtCommand(strCmd, strRes, 5000, 2, 10000, 100))
	{
		return SIM800_GPRS_EVENT_WRITEDATA;
	}
	// Write data
	if(AtSendRecieve(writeData, len, (U8*)AtResponse, responseLen, "+FTPPUT: 1,1,1360", 10000, 2, 5000, 400))
	{
		return SIM800_GPRS_EVENT_WRITEDATA;
	}
	return SIM800_OK;
}

U8 sim800_FtpCloseWrite(void)
{
	if(AtCommand("AT+FTPPUT=2,0\r\n", "+FTPPUT: 1,0", 10000, 2, 5000, 100))
	{
		return SIM800_GPRS_EVENT_WRITECLOSE;
	}
	return SIM800_OK;
}
U8 sim800_FtpCloseRead(void)
{
	if(AtCommand("", "+FTPGET: 1,64", 30000, 2, 5000, 100))
	{
		return SIM800_GPRS_EVENT_WRITECLOSE;
	}
	return SIM800_OK;
}
U8 sim800_FtpReadFile(char fileName[],char filePath[], U8 data[], U16 len)
{
	if(sim800_FtpOpenRead(fileName, filePath))
	{
		return SIM800_GPRS_EVENT_READOPEN;
	}
	if(sim800_FtpReadData(data, len))
	{
		return SIM800_GPRS_EVENT_READDATA;
	}
	sim800Delay(5000);
	return SIM800_OK;
}

U8 sim800_FtpWriteFile(char fileName[],char filePath[], U8 data[], U16 len,U8 AppendFlag)
{
	if(sim800_FtpOpenWrite(fileName, filePath, 0))
	{
		return SIM800_GPRS_EVENT_WRITEOPEN;
	}
	if(sim800_FtpWriteData(data, len))
	{
		return SIM800_GPRS_EVENT_WRITEDATA;
	}
	if(sim800_FtpCloseWrite())
	{
		return SIM800_GPRS_EVENT_WRITECLOSE;
	}
	return SIM800_OK;
}

// Open file to read ------------------------------------------------------------------------------
U8 sim800_FtpDeleteFile( char fileName[], char filePath[])
{
	char str[100];
	// File name --------------------------------
	sprintf(str, "AT+FTPGETNAME=\"%s\"\r\n", fileName);
	if(AtCommand(str, "OK", 5000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_DELETEFILE;
	}
	// Path -------------------------------------
	sprintf(str, "AT+FTPGETPATH=\"%s\"\r\n", filePath);
	if(AtCommand(str, "OK", 5000, 2, 3000, 100))
	{
		return SIM800_GPRS_EVENT_DELETEFILE;
	}
	// Open connection
	if(AtCommand("AT+FTPDELE\r\n", "+FTPDELE: 1,0", 10000, 2, 20000, 100))
	{
		return SIM800_GPRS_EVENT_DELETEFILE;
	}
	return SIM800_OK;
}

U8 sim800_CheckCharge(U8 Operator)//0:Irancell, 1: Aval, 2:Rightel
{
	char strCharge[15];
	char strPersian[15];
	char str[50];
	char validRes[10];
	char *strTemp;
	switch(Operator)
	{
	case 0:
		strcpy(strCharge, "*141*1#");
		strcpy(strPersian, "*555*4*3*2#");
		break;

	case 1:
		strcpy(strCharge, "*140*11#");
		strcpy(strPersian, "*198*2#");
		break;

	case 2:
		strcpy(strCharge, "*140#");
		strcpy(strPersian, "*720*7*1*3#");
		break;
	
	default:
		strcpy(strCharge, "");
		strcpy(strPersian, "");
		break;
	}
	Sim800Status.Charge = 0;

	AtCommand("AT\r\n", "OK", 2000, 2, 1000, 100);
	sim800Delay(1000);

	if(AtCommand("AT+CUSD=1\r\n", "OK", 20000, 2, 20000, 100))
	{
		return SIM800_GPRS_EVENT_CHARGE;
	}
	sim800Delay(5000);
	AtCommand("AT\r\n", "OK", 2000, 2, 1000, 100);
	sim800Delay(1000);
	AtCommand("AT+CMGF=1\r\n", "OK", 2000, 2, 1000, 100);
	sim800Delay(1000);

	sprintf(str, "AT+CUSD=1,\"%s\"\r\n", strPersian);
	// Open connection
	if(AtCommand(str, "language", 20000, 2, 20000, 500))
	{
		return SIM800_GPRS_EVENT_CHARGE;
	}
	sim800Delay(5000);
	sprintf(str, "AT+CUSD=1,\"%s\"\r\n", strCharge);
	
	if(Operator == 0)
	{
		strcpy(validRes, "IRR");
	}
	else
	{
		strcpy(validRes, "Rial");
	}
	
	if(AtCommand(str, validRes, 20000, 2, 20000, 500))
	{
		return SIM800_GPRS_EVENT_CHARGE;
	}
	strTemp = strstr(AtResponse, validRes);
	strTemp -= 2;
	int charge = 0;
	int dec = 1;
	while(((*strTemp) >= '0') && ((*strTemp) <= '9'))
	{
		charge += (*strTemp - '0') * dec;
		dec *= 10;
		strTemp--;
	}
	Sim800Status.Charge = charge;
	return SIM800_OK;
}

U8 sim800_ReadSimNumber(char simNumber[])
{
	if(AtCommand("AT+CCID\r\n", "OK", 5000, 2, 5000, 100))
	{
		
	}
	
	return SIM800_OK;
}

