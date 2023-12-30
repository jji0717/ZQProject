// stdafx.cpp : source file that includes just the standard includes
// DataWrapper.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifndef _NO_FIX_LOG
#include <assert.h>
#include "fltinit.h"
#include <stdio.h>

ISvcLog* service_log = NULL;

void glog(ISvcLog::LogLevel level, const char* fmt, ...)
{
	char outbuf[2048];
	va_list vlist;
	if (service_log == NULL) {
		printf("[DataWrapper]glog():\tservice_log is NULL\n");
		assert(false);
		return;
	}

	if (service_log->getLevel() < level)
		return;

	static const char prefix[] = "[DataWrapper]";
	static const int prefixLen = sizeof(prefix) - 1;
	strcpy(outbuf, prefix);
	va_start(vlist, fmt);
	vsprintf(&outbuf[prefixLen], fmt, vlist);
	service_log->log0(level, outbuf);
	return;
}

void LogMyEvent(int errorLevel,int errorcode,char* errorStr)
{
	char outbuf[2048];
	if (service_log == NULL) {
		printf("[DataWrapper]LogMyEvent():\tservice_log is NULL\n");
		assert(false);
		return;
	}

	if (service_log->getLevel() < errorLevel)
		return;

	static const char prefix[] = "[DataWrapper]";
	static int prefixLen = sizeof(prefix) - 1; // exclude the zero
	strcpy(outbuf, prefix);
	sprintf(&outbuf[prefixLen], "ErrCode: 0x%x, ErrMsg: %s", 
		errorcode, errorStr);
	ISvcLog::LogLevel level;
	switch(errorLevel) {
	case 0:
		level = ISvcLog::L_ERROR;
		break;
	case 1:
		level = ISvcLog::L_INFO;
		break;
	case 2:
		level = ISvcLog::L_DEBUG;
		break;
	default:
		level = ISvcLog::L_DEBUG_DETAIL;
		break;
	}

	service_log->log0(level, outbuf);
}


#else // #ifndef _NO_FIX_LOG

#define Log_level 5
void LogMyEvent(int errorLevel,int errorcode,char* errorStr)
{
	if (errorLevel > Log_level)
		return;

	char    chMsg[1024];
	SYSTEMTIME		St;
	GetLocalTime(&St);
	wsprintf(chMsg,"%02d-%02d %02d:%02d:%02d %03d %s",St.wMonth,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds,errorStr);
	DbgLog((LOG_ERROR, 0, TEXT(chMsg)));
	return;
}

#endif // #ifndef _NO_FIX_LOG

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
