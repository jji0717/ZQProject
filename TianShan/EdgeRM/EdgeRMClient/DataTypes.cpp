#include "DataTypes.h"
#include "TimeUtil.h"
std::string switchModulation(short  modulationFormat)
{
	std::string result = "";
	switch (modulationFormat)
	{
	case 0x00:
		result = "Unknown";
		break;
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
		result = "QAM 16";
		break;
	case 0x07:
		result = "QAM 32";
		break;
	case 0x08:
		result = "QAM 64";
		break;
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
		break;
		result = "QAM 128";
	case 0x0d:
	case 0x0e:
	case 0x0f:
	case 0x10:
		break;
		result = "QAM 256";
	default:
		break;			
	}
	return result;
}

std::string switchLevel(short  interleaverMode)
{
	std::string result = "";
	switch (interleaverMode)
	{
	case 0x01:
		result = "level 1";
		break;
	case 0x02:
		result = "level 2";
		break;
	default:
		break;
	}
	return result;
}

std::string switchMode(short  interleaverLevel)
{
	std::string result = "";
	switch (interleaverLevel)
	{
	case 0x01:
		result = "fecI128J1";
		break;
	case 0x02:
		result = "fecI128J2";
		break;
	case 0x03:
		result = "fecI64J2";
		break;
	case 0x04:
		result = "fecI128J3";
		break;
	case 0x05:
		result = "fecI32J4";
		break;
	case 0x06:
		result = "fecI128J4";
		break;
	case 0x07:
		result = "fecI16J8";
		break;
	case 0x08:
		result = "fecI128J5";
		break;
	case 0x09:
		result = "fecI8J16";
		break;
	case 0x0a:
		result = "fecI128J6";
		break;
	case 0x0b:
		result = "fecI128J7";
		break;
	case 0x0c:
		break;
		result = "fecI128J8";
	default:
		break;
	}
	return result;
}

std::string switchState(TianShanIce::State state)
{
	std::string result = "";
	switch (state)
	{	
	case TianShanIce::stNotProvisioned:
		result = "NotProvisioned";
		break;
	case TianShanIce::stProvisioned:
		result = "Provisioned";
		break;
	case TianShanIce::stInService:
		result = "InService";
		break;
	case TianShanIce::stOutOfService:
		result = "OutOfService";
		break;
	default:
		break;
	}
	return result;
}

const char * GetLevelText(int eAction)
{
	static const char* LeveText[] = { "level 1", "level 2" };

	return LeveText[eAction];
}

const char * GetModeText(int eAction)
{
	static const char* ModeText[] = { "fecI128J1", "fecI128J2", "fecI64J2", "fecI128J3", "fecI32J4", "fecI128J4",
									  "fecI16J8", "fecI128J5", "fecI8J16", "fecI128J6", "fecI128J7", "fecI128J8" };
	return ModeText[eAction];
}

const char * GetModulationText(int eAction)
{
	static const char* ModeText[] = { "Unknown", "reserved", "reserved", "reserved", "reserved", "reserved",
									  "QAM 16", "QAM 32", "QAM 64", "reserved", "reserved", "reserved",
									  "QAM 128", "reserved", "reserved", "reserved", "QAM 256", "reserved"};
	return ModeText[eAction];
}

int
getLevelCode(std::string ActionText)
{
	for (int i = 1; i < eLevelMax; i++)
	{
		if (!stricmp(ActionText.c_str(), GetLevelText(i)))
			return i;
	}
	return 0;
}

int
getModeCode(std::string ActionText)
{
	for (int i = 1; i < eModeMax; i++)
	{
		if (!stricmp(ActionText.c_str(), GetModeText(i)))
			return i;
	}
	return 0;
}

int
getModulationCode(std::string ActionText)
{
	for (int i = 1; i < eModulationMax; i++)
	{
		if (!stricmp(ActionText.c_str(), GetModulationText(i)))
			return i;
	}
	return 0;
}

std::string switchFec(short  fec)
{
	std::string result = "";
	switch (fec)
	{
	case 0x00:
		result = "FEC transmission system";
		break;
	case 0x01:
		result = "FEC DAVIC";
		break;
	default:
		break;
	}
	return result;
}
/*
bool localTime2TianShanTime(const char* szTime, int64& lTime)
{
	int nYear,nMon,nDay, nHour, nMin, nSec;
	if (sscanf(szTime, "%d-%d-%dT%d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec)<6)
		return false;

	if (nYear < 1970 || nYear > 2100 || nMon < 1 || nMon > 12 || nDay < 1 || nDay > 31 || 
		nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59)
		return false;

	// convert to system time
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	st.wYear = nYear;
	st.wMonth = nMon;
	st.wDay = nDay;
	st.wHour = nHour;
	st.wMinute = nMin;
	st.wSecond = nSec;

	FILETIME ft_local, ft_utc;
	SystemTimeToFileTime(&st, &ft_local);
	LocalFileTimeToFileTime(&ft_local, &ft_utc);

	memcpy(&lTime, &ft_utc, sizeof(lTime));
	lTime = lTime / 10000;  

	return true;
}
*/
std::string TianShanTime2String(int64 lTime)
{
/*	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));

	FILETIME ft_local, ft_utc;

	lTime = lTime * 10000;  
	memcpy(&ft_utc, &lTime, sizeof(ft_utc));

	FileTimeToLocalFileTime(&ft_utc, &ft_local);
	FileTimeToSystemTime(&ft_local, &st);

	char result[20] = {0};
	memset(result, 0, 20);
	sprintf(result, "%d-%d-%dT%d:%d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);*/

	// show Last Updated
	char timeBuffer[128];
	memset(timeBuffer,'\0',sizeof(timeBuffer));
	ZQ::common::TimeUtil::TimeToUTC(lTime,timeBuffer,sizeof(timeBuffer),true);

	return timeBuffer;
}
