/**********************************************************
 $Id: LogCtrl.cpp,v 1.7 2003/06/18 20:43:11 shao Exp $
 $Author: Admin $
 $Revision: 1 $
 $Date: 10-11-12 16:00 $

Copyright (C) 2002-2003 Hui Shao.
All Rights Reserved, Hui Shao.
**********************************************************/

#include "LogCtrl.h"
#define _LOG_ECHO

#include <string>
#include <iostream>

extern "C"
{
#ifndef WIN32
	#include <utime.h>
#endif //WIN32
	#include <time.h>
	#include <stdio.h>
	#include <stdarg.h>
};

ENTRYDB_NAMESPACE_BEGIN

LogCtrl log;

LogCtrl::LogCtrl(void)
        :mLevel(LC_DEBUG), bScrEcho(false)
{
}

LogCtrl::~LogCtrl(void)
{
	close();
}

void LogCtrl::close(void)
{
#ifdef _MT
	lock.enter();
#endif // _MT

	if (mLog.is_open())
		mLog.close();

#ifdef _MT
	lock.leave();
#endif // _MT
}

void LogCtrl::open(const char *ident, log_level_t level)
{
	std::string id = (ident !=NULL) ? ident : "unknown";

	int pos = id.find_last_not_of(" \t\r\n");
	pos = (pos>=0) ? id.find_last_of('.', pos) : pos;

	id = (pos>0) ? id.substr(0, pos+1) + "log": "unkonwn.log";

#ifdef _MT
	lock.enter();
#endif // _MT

	if (mLog.is_open())
		mLog.close();
	mLog.open(id.c_str(), std::ofstream::out | std::ofstream::app);

#ifdef _MT
	lock.leave();
#endif // _MT

	mLevel = level;
}

void LogCtrl::writemsg(log_level_t level, const char* msg)
{
	if (mLevel<=0 || level >mLevel || msg==NULL)
		return;

#ifdef _MT
	lock.enter();
#endif // _MT
	try {
		if (bScrEcho)
 			std::cout << msg << std::endl;
		mLog << timestampstr() << msg << endl;
		mLog.flush();
	}
	catch(...) {}

#ifdef _MT
	lock.leave();
#endif // _MT
}

LogCtrl& LogCtrl::operator()(log_level_t level, const char *fmt, ...)
{
	char msg[1024];
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	writemsg(level, msg);

	return *this;
}

LogCtrl& LogCtrl::operator()(const char *fmt, ...)
{
	char msg[1024];
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	writemsg(mLevel, msg);

	return *this;
}

LogCtrl& LogCtrl::operator()(void)
{
	return *this;
}	

log_level_t LogCtrl::level(log_level_t level)
{
	mLevel = level;
	return mLevel;
}

bool LogCtrl::isopened()
{
	return mLog.is_open();
}

const char* LogCtrl::timestampstr(bool digits)
{
	const char *monthstr[]= {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};
 
#ifdef _WIN32_WCE
	SYSTEMTIME timeData;
	
	GetLocalTime(&timeData);
	
	if (digits)
		sprintf(timestamp, "%02d-%02d %02d:%02d:%02d  ",
		        timeData.wMonth+1, timeData.wDay, timeData.wHour, timeData.wMinute, timeData.wSecond);
	else
		sprintf(timestamp, "%3s %02d %02d:%02d:%02d  ",
		        monthstr[timeData.wMonth], timeData.wDay,	timeData.wHour,	timeData.wMinute, timeData.wSecond);
#else
	struct tm *timeData;
	time_t     longTime;
	
	time(&longTime);
	timeData = localtime(&longTime);
	
	if (digits)
		sprintf(timestamp, "%02d-%02d %02d:%02d:%02d  ",
		        timeData->tm_mon+1, timeData->tm_mday, timeData->tm_hour, timeData->tm_min, timeData->tm_sec);
	else
		sprintf(timestamp, "%3s %02d %02d:%02d:%02d  ",
		        monthstr[timeData->tm_mon], timeData->tm_mday, timeData->tm_hour, timeData->tm_min, timeData->tm_sec);
#endif
 
	return timestamp;
}

#ifdef SMARKIN_LG_SET
const char* LogCtrl::tLevelStr[] =
{
	"NONE", "FATAL", "CRITICAL", "SERIOUS", "CAUTION", "WARNING", "INFO", "INFO", "VERBOSE"
};
#else
const char* LogCtrl::tLevelStr[] =
{
	NULL, "EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"
};
#endif // SMARKIN_LG_SET

const char* LogCtrl::level2string(log_level_t level)
{
	return tLevelStr[level];
}

int LogCtrl::string2level(const char* levelstr)
{
	for (int i =1; i< sizeof(tLevelStr)/sizeof(const char*); i++)
		if (stricmp(levelstr, tLevelStr[i]) ==0)
			return i;
	return -1;
}

ENTRYDB_NAMESPACE_END
