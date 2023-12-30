// stdafx.cpp : source file that includes just the standard includes
//	ZQBroadcastFilter.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
/*
void LogToFile(int nErrorLevel, char* szMsg)
{
	char szLog[MAX_PATH*2];
	wsprintf(szLog,":%s",szMsg);
	DbgLog((LOG_ERROR, 0, TEXT(szLog)));

}
*/

#ifndef _NO_FIX_LOG
#include <assert.h>
#include "fltinit.h"

ISvcLog* service_log = NULL;

void glog(ISvcLog::LogLevel level, const char* fmt, ...)
{
	char outbuf[2048];
	va_list vlist;	

	if (service_log == NULL) {
		printf("[ZQBrodcast]log():\tservice_log is NULL\n");
		assert(false);
		return;
	}

	if (service_log->getLevel() < level)
		return;

	static const char prefix[] = "[ZQBrodcast]";
	static const int prefixLen = sizeof(prefix) - 1;
	strcpy(outbuf, prefix);
	va_start(vlist, fmt);
	vsprintf(&outbuf[prefixLen], fmt, vlist);
	service_log->log0(level, outbuf);
	return;
}

void LogToSystemEvent(int nErrorLevel,char* szMsg)
{
	char    szLog[MAX_PATH*2];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[1];
	sprintf(szLog,"ZQ Broadcast:%s",szMsg);
	lpszStrings[0] = szLog;

	ISvcLog::LogLevel level;
	WORD eventType;
	switch(nErrorLevel) {
	case 0:
		level = ISvcLog::L_DEBUG;
		eventType = EVENTLOG_SUCCESS;
		break;
	case 1:
		level = ISvcLog::L_ERROR;
		eventType = EVENTLOG_ERROR_TYPE;
		break;
	case 2:
		level = ISvcLog::L_WARNING;
		eventType = EVENTLOG_WARNING_TYPE;
		break;
	case 3:
		level = ISvcLog::L_INFO;
		eventType = EVENTLOG_INFORMATION_TYPE;
		break;
	case 4:
		level = ISvcLog::L_INFO;
		eventType = EVENTLOG_AUDIT_SUCCESS;
		break;
	case 5:
		level = ISvcLog::L_CRIT;
		eventType = EVENTLOG_AUDIT_FAILURE;
		break;
	default:
		level = ISvcLog::L_DEBUG;
		eventType = EVENTLOG_INFORMATION_TYPE;
		break;
	}

	hEventSource = RegisterEventSource(NULL, "Jizhenan");

	if (hEventSource) {
		// Write to event log.
		ReportEvent(hEventSource, eventType, 0, 111, NULL, 1, 0, (LPCSTR* )&lpszStrings, NULL);
		DeregisterEventSource(hEventSource);
	}

	LogMyEvent(level, 0, szMsg);
}

void LogMyEvent(int errorLevel,int errorcode,char* errorStr)
{
	char outbuf[2048];
	if (service_log == NULL) {
		printf("[ZQBrodcast]LogMyEvent():\tservice_log is NULL\n");
		assert(false);
		return;
	}

	if (service_log->getLevel() < errorLevel)
		return;

	static const char prefix[] = "[ZQBroadcast]";
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

void LogToSystemEvent(int nErrorLevel,char* szMsg)
{
	char    szLog[MAX_PATH*2];
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[1];
	sprintf(szLog,"ZQ Broadcast:%s",szMsg);
	lpszStrings[0] = szLog;

	hEventSource = RegisterEventSource(NULL, "Jizhenan");
	if (hEventSource != NULL)
	{
		// Write to event log.
		if(nErrorLevel==0)
			ReportEvent(hEventSource, EVENTLOG_SUCCESS,			0, 111, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(nErrorLevel==1)
			ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE,		0, 111, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(nErrorLevel==2)
			ReportEvent(hEventSource, EVENTLOG_WARNING_TYPE,	0, 111, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(nErrorLevel==3)
			ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE,0, 111, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(nErrorLevel==4)
			ReportEvent(hEventSource, EVENTLOG_AUDIT_SUCCESS,	0, 111, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(nErrorLevel==5)
			ReportEvent(hEventSource, EVENTLOG_AUDIT_FAILURE,	0, 111, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);

		DeregisterEventSource(hEventSource);
	}
}

void LogMyEvent(int errorLevel,int errorcode,char* errorStr)
{
	char    chMsg[1024];
	SYSTEMTIME		St;
	GetLocalTime(&St);
	wsprintf(chMsg,"%02d-%02d %02d:%02d:%02d %3d %s",St.wMonth,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds,errorStr);
	if (errorLevel==1)
		DbgLog((LOG_ERROR, 0, TEXT(chMsg)));
	else
		DbgLog((LOG_TRACE, 3, TEXT(chMsg)));
	//In regedit, set TRACE's value is 0x10, LOG_TRACE's content will be output.
	return;

//	HANDLE  hEventSource;
//	LPTSTR  lpszStrings[1];
//	sprintf(chMsg,"Object=ZQ Broadcast,Code=%d,Spec=%s",errorcode,errorStr);
//	lpszStrings[0] = chMsg;
//	DWORD lastError;
//#ifdef NEED_LOGINFORMATION
//	DbgLog((LOG_ERROR, 0, TEXT(errorStr)));
//#endif
//	// Get a handle to use with ReportEvent().
//	hEventSource = RegisterEventSource(NULL, "DOD Server");
//	if (hEventSource != NULL)
//	{
//		// Write to event log.
//		if(errorLevel==0)
//			ReportEvent(hEventSource, EVENTLOG_SUCCESS, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
//		else if(errorLevel==1)
//			ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
//		else if(errorLevel==2)
//			ReportEvent(hEventSource, EVENTLOG_WARNING_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
//		else if(errorLevel==3)
//			ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
//		else if(errorLevel==4)
//			ReportEvent(hEventSource, EVENTLOG_AUDIT_SUCCESS, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
//		else if(errorLevel==5)
//			ReportEvent(hEventSource, EVENTLOG_AUDIT_FAILURE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
//
//		DeregisterEventSource(hEventSource);
//	}
//	else
//	{
//		lastError=GetLastError();
//		DbgLog((LOG_ERROR, 0, TEXT("LogMyEvent: fail to Register Event Source.")));
//	}

}

#endif // #ifndef _NO_FIX_LOG
