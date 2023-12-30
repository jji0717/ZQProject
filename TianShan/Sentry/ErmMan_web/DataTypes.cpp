#include "DataTypes.h"

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
		result = "QAM 128";
		break;
	case 0x0d:
	case 0x0e:
	case 0x0f:
	case 0x10:
		result = "QAM 256";
		break;
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

/*bool localTime2TianShanTime(const char* szTime, int64& lTime)
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

	lTime = ZQ::common::TimeUtil::ISO8601ToTime(szTime);
	if (lTime == 0)
		return false;
	return true;
}

std::string TianShanTime2String(int64 lTime)
{
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));

	FILETIME ft_local, ft_utc;

	lTime = lTime * 10000;  
	memcpy(&ft_utc, &lTime, sizeof(ft_utc));

	FileTimeToLocalFileTime(&ft_utc, &ft_local);
	FileTimeToSystemTime(&ft_local, &st);

	char result[20] = {0};
	memset(result, 0, 20);
	sprintf(result, "%04d-%02d-%02dT%02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	char UTCTimeBuff[128];
	memset(UTCTimeBuff,'\0',128);
	if(NULL == ZQ::common::TimeUtil::TimeToUTC(lTime,UTCTimeBuff,sizeof(UTCTimeBuff)))
		return "";
	char LocalTimeBuff[128];
	memset(LocalTimeBuff,'\0',128);
	if(!ZQ::common::TimeUtil::Iso2Local(UTCTimeBuff,LocalTimeBuff,sizeof(LocalTimeBuff)))
		return "";
	return LocalTimeBuff;
}*/