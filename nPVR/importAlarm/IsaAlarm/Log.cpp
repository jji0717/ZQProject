// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: Log.cpp,v 1.15 2004/08/09 10:07:00 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Define Base Logger
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/nPVR/importAlarm/IsaAlarm/Log.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     06-01-10 16:05 Hongquan.zhang
// 
// 6     05-11-15 10:23 Ken.qian
// 
// 5     05-11-14 14:12 Ken.qian
// Revision 1.15  2004/08/09 10:07:00  jshen
// add wchar support to log
//
// Revision 1.14  2004/07/29 06:55:39  wli
// Add unicode support
//
// Revision 1.13  2004/07/22 06:15:19  shao
// global logger
//
// Revision 1.12  2004/06/15 01:28:33  mwang
// some bugs fixed
//
// Revision 1.11  2004/06/13 03:46:50  mwang
// no message
//
// Revision 1.10  2004/05/27 07:01:53  mwang
// no message
//
// Revision 1.9  2004/05/26 09:32:35  mwang
// no message
//
// Revision 1.8  2004/05/17 06:46:41  mwang
// no message
//
// Revision 1.7  2004/05/11 05:47:28  shao
// method to switch global logger
//
// Revision 1.6  2004/05/09 03:51:09  shao
// added classes SysLog and DebugMsg
//
// Revision 1.5  2004/04/30 05:17:46  shao
// log definition
//
// ===========================================================================

#include "Log.h"
#include "combstring.h"

#include "Locks.h"

extern "C"
{
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
}


namespace ZQ {
namespace common {

static char* _levelstrs[8]={"EMERG", "ALERT", "CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

const char* Log::getVerbosityStr(int level)
{
	return _levelstrs[level % (sizeof(_levelstrs)/sizeof(char*))];
}

int Log::getVerbosityInt(const char *levelstr)
{
	for (int i=0; i <8; i ++)
		if (stricmp(_levelstrs[i], levelstr) == 0) return i;
	return -1;
}

const char* Log::getTimeStampStr(char* str, const int size/*=-1*/, bool compacted/*=false*/)
{
	static const char *monthstr[]= {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};

	if (str == NULL)
		return NULL;

	struct tm *timeData;
	time_t     longTime;

	time(&longTime);
	timeData = localtime(&longTime);

	if (compacted)
	{
		uint32 dsec= (timeData->tm_hour *60 +timeData->tm_min)*60 +timeData->tm_sec;
		uint16 dday= (timeData->tm_mon+1) *0x100 +timeData->tm_mday;

		if (size<0)
			sprintf(str, "%03x%05x", dday, dsec);
		else
			snprintf(str, size, "%03x%05x", dday, dsec);

	}
	else
	{
		if (size<0)
			sprintf(str, "%3s %02d %02d:%02d:%02d",
			monthstr[timeData->tm_mon],
			timeData->tm_mday,
			timeData->tm_hour,
			timeData->tm_min,
			timeData->tm_sec);
		else
			snprintf(str, size, "%3s %02d %02d:%02d:%02d",
			monthstr[timeData->tm_mon],
			timeData->tm_mday,
			timeData->tm_hour,
			timeData->tm_min,
			timeData->tm_sec);
	}

	return str;
}

int Log::getVerbosity()
{
	return _verbosity;
}

void Log::setVerbosity(int newlevel)
{
	if (newlevel < L_EMERG)
		_verbosity = L_EMERG;
	else if (newlevel > L_DEBUG)
		_verbosity = L_DEBUG;
	else _verbosity = newlevel;
}

const char* Log::getVerbosityStr()
{
	return getVerbosityStr(_verbosity);
}

Log& Log::operator()(int level, const char *fmt, ...)
{
	if ((level & 0xff) > _verbosity)
		return *this;

	char msg[2048];
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	writeMessage(msg, level & 0xff);

	return *this;
}

Log& Log::operator()(int level, const wchar_t *fmt, ...)
{
	if ((level & 0xff) > _verbosity)
		return *this;

	wchar_t msg[2048];
	va_list args;

	va_start(args, fmt);
	vswprintf(msg, fmt, args);
	va_end(args);

	writeMessage(msg, level & 0xff);

	return *this;
}

Log& Log::operator()(const char *fmt, ...)
{
	char msg[2048];
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

	writeMessage(msg, _verbosity);

	return *this;
}

Log& Log::operator()(const wchar_t *fmt, ...)
{
	wchar_t msg[2048];
	va_list args;

	va_start(args, fmt);
	vswprintf(msg, fmt, args);
	va_end(args);

	writeMessage(msg, _verbosity);

	return *this;
}

Log& Log::operator()(void)
{
	return *this;
}

//Log NullLogger;
#ifdef _DEBUG
Log NullLogger(Log::L_DEBUG);
#else
Log NullLogger;
#endif

Log* pGlog = &NullLogger;

void setGlogger(Log* pLogger /*=NULL*/)
{
	pGlog = (pLogger!=NULL) ? pLogger : &NullLogger;
}


// -----------------------------
// class SysLog
// -----------------------------
#ifdef WIN32
SysLog::SysLog(const char* applicationName, const int verbosity/*=L_ERROR*/, const char* machine /*=NULL*/)
       : _hEventSource(NULL), _machine(NULL), _appname(NULL)
{
	if (machine != NULL && strlen(machine)>0)
	{
		_machine = _names;
		strcpy(_machine, machine);
	}
	
	if (applicationName != NULL && strlen(applicationName)>0)
	{
		_appname = (_machine==NULL) ? _names : (_machine + strlen(_machine) +2);
		strcpy(_appname, applicationName);
	}
}
#endif // WIN32

SysLog::~SysLog()
{
#ifdef WIN32
	if (_hEventSource)
		::DeregisterEventSource(_hEventSource);
#endif // WIN32
}

void SysLog::writeMessage(const char *msg, int level /*=-1*/)
{
#ifdef WIN32
	WORD wType = EVENTLOG_INFORMATION_TYPE;
	
	// convert level to EventLog type
	switch(level)
	{
	case L_EMERG:
	case L_ALERT:
	case L_CRIT:
	case L_ERROR:
		wType = EVENTLOG_ERROR_TYPE;
		break;
		
	case L_WARNING:
		wType = EVENTLOG_WARNING_TYPE;
		break;
		
	case L_NOTICE:
	case L_INFO:
	case L_DEBUG:
	default:
		wType = EVENTLOG_INFORMATION_TYPE;
		break;
	}
#ifdef UNICODE
	const wchar_t* ps[2];
	CombString str = msg;
	ps[0] = str.c_str();
	ps[1] = NULL;

	// Check the event source has been registered and if not then register it now
	CombString sMachine = _machine;
	CombString sAppname = _appname;
	if(!_hEventSource)
		_hEventSource = ::RegisterEventSource(sMachine.c_str(),sAppname.c_str());

	if (_hEventSource)
	{
		// put the event
		::ReportEvent(_hEventSource,
			wType,
			0,
			(WORD) level,
			NULL, // sid
			(WORD)((ps[0]==NULL) ? 0:1),
			0,
			ps,
			NULL);
	}
#else
	const char* ps[2];
    ps[0] = msg;
    ps[1] = NULL;
	
    // Check the event source has been registered and if not then register it now
    if (!_hEventSource)
        _hEventSource = ::RegisterEventSource(_machine,  // local machine
		_appname); // source name
	
    if (_hEventSource)
	{
		// put the event
        ::ReportEvent(_hEventSource,
			wType,
			0,
			(WORD) level,
			NULL, // sid
			(WORD)((ps[0]==NULL) ? 0:1),
			0,
			ps,
			NULL);
    }
#endif // UNICODE
#endif // WIN32
}

// 
void xlog(Log &logger, int level, const char *fmt, ...)
{
//	if (logger == NULL) return;

	char msg[2048];
	va_list args;
	va_start(args, fmt);
	_snprintf(msg, 2048, fmt, args);
	va_end(args);

	logger(level, msg);
}

void xlog(Log &logger, int level, const wchar_t *fmt, ...)
{
	//	if (logger == NULL) return;

	wchar_t msg[2048];
	va_list args;
	va_start(args, fmt);
	_snwprintf(msg, 2048, fmt, args);
	va_end(args);

	logger(level, msg);
}
} // namespace common
} // namespace ZQ
