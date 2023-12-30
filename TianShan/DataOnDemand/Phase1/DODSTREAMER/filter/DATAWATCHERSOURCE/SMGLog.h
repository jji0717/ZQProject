//SMGLog.h
#ifndef ZQ_SMGLOG_H__
#define ZQ_SMGLOG_H__

#ifndef _NO_FIX_LOG
//////////////////////////////////////////////////////////////////////////
#include "fltinit.h"

#define SMGLOG_ERROR 	ISvcLog::L_ERROR
#define SMGLOG_WARNING	ISvcLog::L_WARNING
#define SMGLOG_INFO		ISvcLog::L_INFO
#define SMGLOG_DETAIL	ISvcLog::L_DEBUG

void SMGLog_DllMain(HANDLE hModule, DWORD  dwReason);
extern int g_dwInstanceID;
extern ISvcLog* service_log;
int smglog(int,unsigned long,char const *, ...);

//////////////////////////////////////////////////////////////////////////
#else // #ifndef _NO_FIX_LOG

#include <atlconv.h>
#include <stdio.h>

//Event level
#define SMGLOG_ERROR 	0
#define SMGLOG_WARNING	10
#define SMGLOG_INFO		20
#define SMGLOG_DETAIL	30

//#define SMGLOG_FILTER_NEED_WRITE_DBGLOG 

#ifdef SMGLOG_FILTER_NEED_WRITE_DBGLOG
#include <streams.h>
	#include <olectl.h>
	#include <initguid.h>
#endif 

BOOL InitializeLog(char * pLogFileName="C:\\log\\SMGLog.log",char * strInstanceName="SMGLOG",int nLogLevel=SMGLOG_INFO,BOOL bNeedDbgLog=FALSE,BOOL bNeedEventView=FALSE);
BOOL UnInitializeLog();
BOOL smglog(int errorLevel,DWORD DInstanceNumber,LPCTSTR format, ...);

extern int g_dwInstanceID;

void GetFilterAxFile(HANDLE hModule, char* szAxFile, char* szInstance);
BOOL GetAxFilterRegInfo(HANDLE hModule, char* szLogFile, char* szInstance, DWORD* pdwLogLevel, DWORD* pdwLogToSysEvent);

void SMGLog_DllMain(HANDLE hModule, DWORD  dwReason);
BOOL SMGLog_Init(HANDLE hModule);
void SMGLog_Uninit();

class CSMGLog
{
public: 
	CSMGLog();
	~CSMGLog();

public:
	
	int UnInitialize();

	int Initialize(char * pLogFileName,char * strInstanceName,int nLogLevel,BOOL bNeedDbgLog,BOOL bNeedEventView);
	
	//Description:  Output log string to log file
	int AddEvent(int errorLevel,char *strDescription);	

	char m_strFileName[MAX_PATH];
	char m_InstanceName[MAX_PATH];
	int m_nSettingLogLevel;
	BOOL m_bNeeEvnetView;
	BOOL m_bNeedDbgLog;
	FILE * m_pStream;
private:	
	//Add message to windows event views.
	void LogToSystemEvent(int errorLevel, int nErrorcode, char* szMsg);

};

#endif // #ifndef _NO_FIX_LOG

#endif