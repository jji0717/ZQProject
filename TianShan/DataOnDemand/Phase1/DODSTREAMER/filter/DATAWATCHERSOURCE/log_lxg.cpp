/*******************************************************************
 File Name:     log_lxg.cpp
 Author:        Lucky.liu
 Security:      SEACHANGE INTERNATIONAL
 Description:   Implement the CLog class
 Function Inventory: 
 Modification Log:
 When           Version        Who         What
---------------------------------------------------------------------
 2004/05/19		1.0			   Lucky.liu   Create
********************************************************************/

#include "stdafx.h"
#include "log_lxg.h"

CLog logFile;

//constructor
CLog::CLog()
{ 
	memset(m_pFile,0,sizeof(m_pFile));
	FILE *pFile;
	pFile=fopen("D:\\Async\\logFile.log","a+");
	if(pFile)
	{
		strcpy(m_pFile,"D:\\Async\\logFile.log");
		fclose(pFile);
	}
	::InitializeCriticalSection(&m_secFile);
	Write("start ...............");
}

//desconstructor
CLog::~CLog()
{ 
	::DeleteCriticalSection(&m_secFile);
	memset(m_pFile,0,sizeof(m_pFile)); 
}

//write a log to logfile
BOOL CLog::Write(char *pLogDes,LOGTYPE_LXG logType)
{
	static int index=0;
	FILE *pFile;
	SYSTEMTIME tSys;

	EnterCriticalSection(&m_secFile);
	::GetLocalTime(&tSys);
	pFile=fopen(m_pFile,"a+");
	if(pFile)
	{
		//fprintf(pFile,"[%.4d-%.2d-%.2d]",tSys.wYear,tSys.wMonth,tSys.wDay);
		//fprintf(pFile,"[%.2d:%.2d:%.2d:%.4d]",tSys.wHour,tSys.wMinute,tSys.wSecond,tSys.wMilliseconds);
		fprintf(pFile,"[Index=%.8d]",index++);
		if(logType==LOGTYPE_ERROR)
		   fprintf(pFile,"%12s","Error");
		else if(logType==LOGTYPE_INFO)
		   fprintf(pFile,"%12s","Information");
		else 
		   fprintf(pFile,"%12s","Unknown");
		fprintf(pFile," :%s\n",pLogDes);
		fclose(pFile);
		LeaveCriticalSection(&m_secFile);
		return TRUE;
	}
	LeaveCriticalSection(&m_secFile);
	return FALSE;
}

