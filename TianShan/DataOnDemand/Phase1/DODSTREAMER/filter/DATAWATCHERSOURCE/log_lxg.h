/*******************************************************************
 File Name:     log_lxg.h
 Author:        Lucky.liu
 Security:      SEACHANGE INTERNATIONAL
 Description:   Declare the CLog class
 Function Inventory: 
 Modification Log:
 When           Version        Who         What
---------------------------------------------------------------------
 2004/05/19		1.0			   Lucky.liu   Create
********************************************************************/

#ifndef LOG_LUCKYLIU_SEACHANGE_SEACHANGE_SERVER_H
#define LOG_LUCKYLIU_SEACHANGE_SEACHANGE_SERVER_H

#include <stdio.h>
#include <string.h>
#include <Windows.h>

class CLog
{
public:
	enum LOGTYPE_LXG
	{
		LOGTYPE_ERROR=0,
		LOGTYPE_INFO=1,
		LOGTYPE_UNKNOWN=2
	};

public:
	CLog();
	virtual ~CLog();
	BOOL Write(char *pLogDes,LOGTYPE_LXG logType=LOGTYPE_INFO);

protected:
	char m_pFile[MAX_PATH];
	CRITICAL_SECTION m_secFile;
};

extern CLog logFile;

#endif