#include "StdAfx.h"
#include ".\basecommand.h"

extern bool m_bConnected;

#define OVERTIMECOUNT 1200 * 2
CBaseCommand::CBaseCommand()
{
	m_nCommandID=0;
	m_HWaitEvent=INVALID_HANDLE_VALUE;
	m_nOverTimeLong=5;

	m_nPortID=0;
	m_nChannelID=0;
	m_nSend1=0;
	m_nSend2=0;
	m_nSend3=0;
	m_nSessionID=0;	
	m_nRetuenCommand=0;
	m_strSend1="";
	m_strSend2="";
	m_strSend3="";
	m_strReturn1="";
	m_strReturn2="";
	m_strReturn3="";
	m_bStop=FALSE;
}

CBaseCommand::~CBaseCommand()
{
	m_bStop=TRUE;
	Sleep(2);
} 
int CBaseCommand::Execute()
{
	int noReturn=0;
	int clogcount=0;
	m_bStop=FALSE;
	while(noReturn<OVERTIMECOUNT)
	//while(1)
	{
		if (m_bStop)
		{
			//Clog( LOG_DEBUG, _T("CBasComand:~cbasecomand was called ! terminal circle"));
			return -1;
		}
		if (m_bConnected==false)
		{
			return -1;
		}
		if(m_nCommandID==m_nRetuenCommand)
		{
			return 0;
		}
		if (noReturn>70 && clogcount>25)
		{		
			//Clog( LOG_DEBUG, _T("wait response msg"));
			clogcount=0;
		}
		clogcount++;
		Sleep(1);
		noReturn++;
	}
	return -1;
}