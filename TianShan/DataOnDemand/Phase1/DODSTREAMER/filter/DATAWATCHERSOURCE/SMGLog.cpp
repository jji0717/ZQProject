#include "stdafx.h"
#include <windows.h>
#include "SMGLog.h"
#include <TCHAR.h>
#include <stdlib.h>

#ifndef _NO_FIX_LOG

// added by Cary
#include <assert.h>
#include "fltinit.h"
#include <stdio.h>

ISvcLog* service_log = NULL;
int g_dwInstanceID = 0;

void SMGLog_DllMain(HANDLE hModule, DWORD  dwReason)
{

}

void glog(ISvcLog::LogLevel level, const char* fmt, ...)
{
	char outbuf[2048];
	va_list vlist;
	if (service_log == NULL) {
		printf("[DataWatcher]glog():\tservice_log is NULL\n");
		assert(false);
		return;
	}

	if (service_log->getLevel() < level)
		return;

	static const char prefix[] = "[DataWatcher]";
	static const int prefixLen = sizeof(prefix) - 1;
	strcpy(outbuf, prefix);
	va_start(vlist, fmt);
	vsprintf(&outbuf[prefixLen], fmt, vlist);
	service_log->log0(level, outbuf);
	return;
}

int smglog(int level ,unsigned long instanceId, char const * fmt, ...)
{
	char outbuf[2048];
	va_list vlist;
	if (service_log == NULL) {
		printf("smglog()\t service_log is NULL\n");
		assert(false);
		return 0;
	}

	int actualLeval;
	
	switch (level)
	{
	case SMGLOG_ERROR:
		actualLeval = ISvcLog::L_ERROR;
		break;
	case SMGLOG_WARNING:
		actualLeval = ISvcLog::L_WARNING;
		break;
	case SMGLOG_INFO:
		actualLeval = ISvcLog::L_INFO;
		break;
	case SMGLOG_DETAIL:
		actualLeval = ISvcLog::L_DEBUG;
		break;
	default:
		actualLeval = ISvcLog::L_DEBUG_DETAIL;
		break;
	}

	if (service_log->getLevel() < actualLeval)
		return 0;

	sprintf(outbuf, "[DataWatcher] (Inst: %d) ", instanceId);
	int prefixLen = strlen(outbuf);
	va_start(vlist, fmt);
	vsprintf(&outbuf[prefixLen], fmt, vlist);
	service_log->log0((ISvcLog::LogLevel )level, outbuf);
	return 0;
}

#else // #ifndef _NO_FIX_LOG

CSMGLog *g_smgLog = NULL;

BOOL InitializeLog(char * pLogFileName,char *strInstanceName,int nLogLevel,BOOL bNeedDbgLog,BOOL bNeedEventView)
{
	if (pLogFileName == NULL)
		return FALSE;
	g_smgLog=new CSMGLog;

	char strTInstanceName[MAX_PATH];

	if (strInstanceName != NULL || (strlen(strInstanceName)<=0))
		strncpy(strTInstanceName,strInstanceName,MAX_PATH);
	else
		strcpy(strTInstanceName,"SMG_InstanceName");

	if (g_smgLog->Initialize(pLogFileName,strTInstanceName,nLogLevel,bNeedDbgLog,bNeedEventView))
	{
		g_smgLog->UnInitialize();
		delete g_smgLog;
		g_smgLog=NULL;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
BOOL UnInitializeLog()
{
	if (g_smgLog)
	{	
		g_smgLog->UnInitialize();
		delete g_smgLog;
		g_smgLog=NULL;
	}
	return TRUE;
}
BOOL smglog(int errorLevel,DWORD DInstanceNumber,LPCTSTR format, ...)
{
	if (g_smgLog==NULL)
		return FALSE;

	if (g_smgLog->m_pStream==NULL)
		return FALSE;

	int nEventLevel=errorLevel- errorLevel%10;	
	
	if (g_smgLog->m_nSettingLogLevel < nEventLevel)
		return 1;

	char chMsg[MAX_PATH]; 

	SYSTEMTIME		St;
	GetLocalTime(&St);
	wsprintf(chMsg,"%02d-%02d-%02d %02d:%02d:%02d:%03d %s 0x%08x ",
				St.wYear,
				St.wMonth,
				St.wDay,
				St.wHour,
				St.wMinute,
				St.wSecond,
				St.wMilliseconds,
				g_smgLog->m_InstanceName,DInstanceNumber);

	switch (nEventLevel)
	{
	case SMGLOG_ERROR:
		strcat(chMsg,"ERROR:  ");
		break;
	case SMGLOG_WARNING:
		strcat(chMsg,"WARN:   ");
		break;
	case SMGLOG_INFO:
		strcat(chMsg,"INFO:   ");
		break;
	case SMGLOG_DETAIL:
		strcat(chMsg,"DETAIL: ");
		break;
	default:
		strcat(chMsg,"OTHERS: ");
		break;
	}
	try
	{	
		va_list args;
		va_start(args, format);

		TCHAR buf[1024*4];
		strcpy(buf,chMsg);
		int nLength =(int) strlen(chMsg);

		try
		{
			_vstprintf((buf+nLength), format, args);
		}
		catch (...)
		{
			buf[nLength]='\0';
			strcat(buf," log exception!");
		}

		if (g_smgLog->m_pStream != NULL)
		{
			fseek(g_smgLog->m_pStream,0,SEEK_END);
			_ftprintf(g_smgLog->m_pStream,"\n%s",buf);	
		}
		if(g_smgLog->m_bNeedDbgLog || g_smgLog->m_bNeeEvnetView)
		{
 			g_smgLog->AddEvent(nEventLevel,buf);
		}
		va_end(args);
	}
	catch(...)
	{
		
	}
	return TRUE;
}

CSMGLog::CSMGLog()
{
	strcpy(m_strFileName,"\0");
	strcpy(m_InstanceName,"\0");
	m_nSettingLogLevel=0;
	m_pStream=NULL;
	m_bNeeEvnetView=FALSE;
}

CSMGLog::~CSMGLog()
{
	UnInitialize();
}
int CSMGLog::UnInitialize()
{
	if (m_pStream !=NULL)
	{
		fclose(m_pStream);
		m_pStream=NULL;
	}
	return 0;
}
int CSMGLog::Initialize(char * pLogFileName,char * strInstanceName,int nLogLevel,BOOL bNeedDbgLog,BOOL bNeedEventView)
{
	if(m_pStream !=NULL )
		return 1;
	strcpy(m_strFileName,pLogFileName);
	strcpy(m_InstanceName,strInstanceName);

	m_nSettingLogLevel = nLogLevel;
	m_bNeeEvnetView	   = bNeedEventView;
	m_bNeedDbgLog	   = bNeedDbgLog;

	try
	{	
		m_pStream = fopen(m_strFileName, "at");
	}
	catch(...)
	{
		m_pStream=NULL;
	}

	return 0;
}

int CSMGLog::AddEvent(int errorLevel,char *strDescription)
{
#ifdef SMGLOG_FILTER_NEED_WRITE_DBGLOG
	if (m_bNeedDbgLog)
		if (errorLevel <=LOG_ERROR)
			DbgLog((LOG_ERROR,0, TEXT(strDescription)));

#endif 

	if (m_bNeeEvnetView)
	{		
		LogToSystemEvent(errorLevel,0,strDescription);
	}

	return 0;
}

void CSMGLog::LogToSystemEvent(int errorLevel, int nErrorcode, char* szMsg)
{
	HANDLE  hEventSource;
	LPTSTR  lpszStrings[1];
	lpszStrings[0] = szMsg;
	// Get a handle to use with ReportEvent().
	hEventSource = RegisterEventSource(NULL,m_InstanceName);
	if (hEventSource != NULL)
	{
		// Write to event log.
		if(errorLevel==SMGLOG_DETAIL)
			ReportEvent(hEventSource, EVENTLOG_SUCCESS, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==SMGLOG_ERROR)
			ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==SMGLOG_WARNING)
			ReportEvent(hEventSource, EVENTLOG_WARNING_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==SMGLOG_INFO)
			ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==SMGLOG_DETAIL)
			ReportEvent(hEventSource, EVENTLOG_AUDIT_SUCCESS, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==SMGLOG_DETAIL)
			ReportEvent(hEventSource, EVENTLOG_AUDIT_FAILURE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);

		DeregisterEventSource(hEventSource);
	}
	
}

int g_dwInstanceID = 0;

void GetFilterAxFile(HANDLE hModule, char* szAxFile, char* szInstance)
{
	//get module path name.
	char szModule[MAX_PATH];
	char szExt[16];
	DWORD dwSize = GetModuleFileName((HMODULE)hModule, szModule, MAX_PATH-1);
	szModule[dwSize] = '\0';

	_splitpath(szModule, NULL, NULL, szAxFile, szExt);
	strcpy(szInstance, szAxFile);
	_strupr(szInstance);
	strcat(szAxFile, szExt);
}

BOOL GetAxFilterRegInfo(HANDLE hModule, char* szLogFile, char* szInstance, DWORD* pdwLogLevel, DWORD* pdwLogToSysEvent)
{
	char szAxFile[MAX_PATH];
	char szReg[MAX_PATH];
	HKEY hKey;
	DWORD dwSize;
	DWORD dwType;

	//get ax file name, such as MXFReader.ax
	GetFilterAxFile(hModule, szAxFile, szInstance);

	//construct key item, such as HKEY_LOCAL_MACHINE\SOFTWARE\Debug\MXFReader.ax
	sprintf(szReg, "SOFTWARE\\Debug\\%s", szAxFile);

	//open key item
	LONG lRet = RegOpenKey(HKEY_LOCAL_MACHINE, szReg, &hKey);
	if( lRet!=ERROR_SUCCESS )
	{
		return FALSE;
	}

	//get SMGLogFile
	dwSize = MAX_PATH-1;
	dwType = REG_SZ;
	lRet = RegQueryValueEx(hKey, "SMGLogFile", NULL, &dwType, (LPBYTE)szLogFile, &dwSize);
	if( lRet!=ERROR_SUCCESS )
	{
		RegCloseKey(hKey);
		return FALSE;
	}
	szLogFile[dwSize] = '\0';

	//get SMGLogLevel
	dwSize = sizeof(DWORD);
	dwType = REG_DWORD;
	lRet = RegQueryValueEx(hKey, "SMGLogLevel", NULL, &dwType, (LPBYTE)pdwLogLevel, &dwSize);
	if( lRet!=ERROR_SUCCESS )
	{
		RegCloseKey(hKey);
		return FALSE;
	}

	//get SMGLogToSystemEvent
	dwSize = sizeof(DWORD);
	dwType = REG_DWORD;
	lRet = RegQueryValueEx(hKey, "SMGLogToSystemEvent", NULL, &dwType, (LPBYTE)pdwLogToSysEvent, &dwSize);
	if( lRet!=ERROR_SUCCESS )
	{
		RegCloseKey(hKey);
		return FALSE;
	}

	RegCloseKey(hKey);
	return TRUE;
}

BOOL SMGLog_Init(HANDLE hModule)
{
	char szLogFile[MAX_PATH];
	char szInstance[MAX_PATH];
	DWORD dwLogLevel;
	DWORD dwLogToSysEvent;

	if( !GetAxFilterRegInfo(hModule, szLogFile, szInstance, &dwLogLevel, &dwLogToSysEvent) )
	{
		return FALSE;
	}

	if( !InitializeLog(szLogFile, szInstance, dwLogLevel, FALSE, dwLogToSysEvent) )
	{
		return FALSE;
	}

	return TRUE;
}

void SMGLog_Uninit()
{
	UnInitializeLog();
}

void SMGLog_DllMain(HANDLE hModule, DWORD  dwReason)
{
   switch (dwReason)
   {
    case DLL_PROCESS_ATTACH:
		SMGLog_Init(hModule);
		break;
 
    case DLL_PROCESS_DETACH:
		SMGLog_Uninit();
		break;
    }
}

#endif // #ifndef _NO_FIX_LOG
