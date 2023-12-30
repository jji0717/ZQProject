

#include <string>
#include "StringFuncImpl.h"
#include "windows.h"

const SFUNC_DATA	_string_func_data[] = 
{
	{"MSA_TIME", SF_MSA_TIME},
	{"DEC2HEX8", SF_DEC2HEX8},
};

int _string_func_count = sizeof(_string_func_data)/sizeof(SFUNC_DATA);


bool SF_MSA_TIME(const char* inputs, char* outputs, int size)
{
	int nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0, nMSec=0;
	
	int nRet = sscanf(inputs, "%2d/%2d %2d:%2d:%2d:%3d", &nMonth, &nDay, &nHour, &nMin, &nSec, &nMSec);
	if (nRet != 6)
		return false;

	SYSTEMTIME stLocal, stUTC;
	GetLocalTime(&stLocal);

	stLocal.wMonth = nMonth;
	stLocal.wDay = nDay;
	stLocal.wHour = nHour;
	stLocal.wMinute = nMin;
	stLocal.wSecond = nSec;
	stLocal.wMilliseconds = nMSec;
	
	FILETIME ftLocal, ftUTC;
	SystemTimeToFileTime(&stLocal, &ftLocal);
	LocalFileTimeToFileTime(&ftLocal, &ftUTC);

	FileTimeToSystemTime(&ftUTC, &stUTC);
	sprintf(outputs, "%d%02d%02d %02d%02d%02d.%03d", 
		stUTC.wYear,
		stUTC.wMonth,
		stUTC.wDay,
		stUTC.wHour,
		stUTC.wMinute,
		stUTC.wSecond,
		stUTC.wMilliseconds);

	return true;
}


bool SF_DEC2HEX8(const char* inputs, char* outputs, int size)
{	
	int nDec = atoi(inputs);

	sprintf(outputs, "%08x", nDec);
	return true;
}

