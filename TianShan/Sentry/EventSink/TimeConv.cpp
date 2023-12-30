#include "TimeConv.h"

std::string time2utc(int64 t) {
    if(t <= 0) {
        return "";
    }
    using namespace ZQ::common;
    char buf[32] = {0};
    if(TimeUtil::TimeToUTC(t, buf, 32)) {
        return buf;
    } else {
        return "";
    }
}

#define FILELOGTIME_FORMAT    "%2d-%2d %2d:%2d:%2d.%3d" //file log time format
#define SCLOGTIME_FORMAT	  "%2d/%2d %2d:%2d:%2d:%3d" //sclog time format
#define LINUXSYSLOGTIME_FORMAT1     "%3s %2d %2d:%2d:%2d"//linux syslog time format: Jan 10 11:20:15
#define LINUXSYSLOGTIME_FORMAT2		"%4d/%2d/%2d_%2d:%2d:%2d"//format: 2010/01/13_16:32:47

#ifdef ZQ_OS_MSWIN
static int64 localTimeToUTC(const SYSTEMTIME* localTime)
{
	FILETIME ftLocal, ftUTC;
	if(!SystemTimeToFileTime(localTime, &ftLocal))
    {
        return 0;
    }
	LocalFileTimeToFileTime(&ftLocal, &ftUTC);

    SYSTEMTIME stUTC;
    FileTimeToSystemTime(&ftUTC, &stUTC);

    char buf[32];
    return (ZQ::common::TimeUtil::Time2Iso(stUTC, buf, 32) ? ZQ::common::TimeUtil::ISO8601ToTime(buf) : 0);
}

int64 getFilelogTime(const char* tmstr)
{
    if(strlen(tmstr) < 18)
    {
        return 0;
    }

	int nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0, nMSec=0;

	int nRet = sscanf(tmstr, FILELOGTIME_FORMAT, &nMonth, &nDay, &nHour, &nMin, &nSec, &nMSec);
	if (nRet != 6)
	{
        return 0;
	}

	SYSTEMTIME stLocal;
	GetLocalTime(&stLocal);
	if(nMonth > stLocal.wMonth)//last year
		stLocal.wYear -= 1;

	stLocal.wMonth = nMonth;
	stLocal.wDay = nDay;
	stLocal.wHour = nHour;
	stLocal.wMinute = nMin;
	stLocal.wSecond = nSec;
	stLocal.wMilliseconds = nMSec;

    return localTimeToUTC(&stLocal);
}

int64 getSclogTime(const char* tmstr)
{
    if(strlen(tmstr) < 17)
    {
        return 0;
    }

	int nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0, nMSec=0;

	int nRet = sscanf(tmstr, SCLOGTIME_FORMAT, &nMonth, &nDay, &nHour, &nMin, &nSec, &nMSec);
	if (nRet != 6)
	{
        return 0;
	}

	SYSTEMTIME stLocal;
	GetLocalTime(&stLocal);
	if(nMonth > stLocal.wMonth)//last year
		stLocal.wYear -= 1;

	stLocal.wMonth = nMonth;
	stLocal.wDay = nDay;
	stLocal.wHour = nHour;
	stLocal.wMinute = nMin;
	stLocal.wSecond = nSec;
	stLocal.wMilliseconds = nMSec;

    return localTimeToUTC(&stLocal);
}

#else
int64 getFilelogTime(const char* tmstr) 
{
	int nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0, nMSec=0;

	int nRet = sscanf(tmstr, FILELOGTIME_FORMAT, &nMonth, &nDay, &nHour, &nMin, &nSec, &nMSec);
	if (nRet != 6)
	{
        return 0;
	}

	time_t tm;
	struct tm tmlog;
    memset(&tmlog, 0, sizeof tmlog);
	tm = time(0);
	if(tm == (time_t)-1)
		return 0;
    struct tm tmResult;
	struct tm* ptm = localtime_r(&tm, &tmResult);
	if(!ptm)
		return 0;
	
	if(nMonth > ptm->tm_mon+1)//last year
		tmlog.tm_year = ptm->tm_year - 1;
	else 
		tmlog.tm_year = ptm->tm_year;

	tmlog.tm_mon = nMonth-1;
	tmlog.tm_mday = nDay;
	tmlog.tm_hour = nHour;
	tmlog.tm_min = nMin;
	tmlog.tm_sec = nSec;
	
	struct timeval tval;
	tval.tv_sec = mktime(&tmlog);
	if(tval.tv_sec == (time_t)-1)
		return 0;
	
	tval.tv_usec = nMSec*1000;
	char buf[50] = {0};
    return (ZQ::common::TimeUtil::Time2Iso(tval,buf,sizeof(buf)) ? ZQ::common::TimeUtil::ISO8601ToTime(buf) : 0);
}

int64 getSclogTime(const char* tmstr)
{
	int nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0, nMSec=0;

	int nRet = sscanf(tmstr, SCLOGTIME_FORMAT, &nMonth, &nDay, &nHour, &nMin, &nSec, &nMSec);
	if (nRet != 6)
	{
        return 0;
	}

	time_t tm;
	struct tm tmlog;
    memset(&tmlog, 0, sizeof tmlog);
	tm = time(0);
	if(tm == (time_t)-1)
		return 0;
    struct tm tmResult;
	struct tm* ptm = localtime_r(&tm, &tmResult);
	if(!ptm)
		return 0;
	
	if(nMonth > ptm->tm_mon+1)//last year
		tmlog.tm_year = ptm->tm_year - 1;
	else
		tmlog.tm_year = ptm->tm_year;

	tmlog.tm_mon = nMonth-1;
	tmlog.tm_mday = nDay;
	tmlog.tm_hour = nHour;
	tmlog.tm_min = nMin;
	tmlog.tm_sec = nSec;
	
	struct timeval tval;
	tval.tv_sec = mktime(&tmlog);
	if(tval.tv_sec == (time_t)-1)
		return 0;
	
	tval.tv_usec = nMSec*1000;
	char buf[50] = {0};
//	ZQ::common::TimeUtil::Time2Iso(tval,buf,sizeof(buf));

    return (ZQ::common::TimeUtil::Time2Iso(tval,buf,sizeof(buf)) ? ZQ::common::TimeUtil::ISO8601ToTime(buf) : 0);

}


#endif
static char chMonAr[13][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" ""};
int64 getSyslogTime1(const char* tmstr)
{
	char chMon[5] = {0};
	int nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0;

	int nRet = sscanf(tmstr, LINUXSYSLOGTIME_FORMAT1, chMon, &nDay, &nHour, &nMin, &nSec);
	if (nRet != 5)
	{
        return 0;
	}

	for(nMonth = 0; nMonth < 13; nMonth++)
		if(strcmp(chMonAr[nMonth], chMon) == 0)
			break; 
	if(nMonth ==  13)
		return 0;

	time_t tm;
	struct tm tmlog;
    memset(&tmlog, 0, sizeof tmlog);
	tm = time(0);
	if(tm == (time_t)-1)
		return 0;
#ifdef ZQ_OS_MSWIN
	struct tm* ptm = localtime(&tm);
#else
    struct tm tmResult;
    struct tm* ptm = localtime_r(&tm, &tmResult);
#endif
	if(!ptm)
		return 0;

	
	if(nMonth > ptm->tm_mon)//last year
		tmlog.tm_year = ptm->tm_year - 1;
	else
		tmlog.tm_year = ptm->tm_year;

	tmlog.tm_mon = nMonth;
	tmlog.tm_mday = nDay;
	tmlog.tm_hour = nHour;
	tmlog.tm_min = nMin;
	tmlog.tm_sec = nSec;
	
	time_t tutc = mktime(&tmlog);
	if(tutc == (time_t)-1)
		return 0;
	char buf[50] = {0};

    return (ZQ::common::TimeUtil::Time2Iso(tutc, buf, sizeof(buf)) ? ZQ::common::TimeUtil::ISO8601ToTime(buf) : 0);
}

//vonvert format like: 2010/01/13_16:29:55
int64 getSyslogTime2(const char* tmstr)
{
	int nYear=0, nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0;

	int nRet = sscanf(tmstr, LINUXSYSLOGTIME_FORMAT2, &nYear, &nMonth, &nDay, &nHour, &nMin, &nSec);
	if (nRet != 6)
	{
        return 0;
	}

	struct tm tmlog;
	memset(&tmlog, 0, sizeof (struct tm));
	tmlog.tm_year = nYear-1900;
	tmlog.tm_mon = nMonth-1;
	tmlog.tm_mday = nDay;
	tmlog.tm_hour = nHour;
	tmlog.tm_min = nMin;
	tmlog.tm_sec = nSec;
		
	time_t tutc = mktime(&tmlog);
	if(tutc == (time_t)-1)
		return 0;

	char buf[50] = {0};
    return (ZQ::common::TimeUtil::Time2Iso(tutc, buf, sizeof(buf)) ? ZQ::common::TimeUtil::ISO8601ToTime(buf) : 0);
}
