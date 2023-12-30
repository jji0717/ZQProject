// DODPort.cpp: implementation of the CDODPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DODPort.h"
#include "messagemacro.h"
#include "clog.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//extern BOOL g_bNotInternalTest;
CDODPort::CDODPort()
{
	m_channel=NULL;
	m_castPort=NULL;

	m_ChannelNumber=0;

	m_kit=NULL;
	m_nID=0;
	m_cDSADstIp[0]='\0';
	m_nSessionID=0;
	m_nPmtPID=0;
	m_nTotalRate=0;
	m_nSessionID=0;
	m_currentDIR="";
	m_castcount=0;
	m_sPortName="";
	m_nGroupID=0;
}

CDODPort::~CDODPort()
{
	Clog(LOG_DEBUG,_T("CDODPort::~CDODPort(%s)."),m_sPortName );

	if (m_channel)
	{
		for(int i=0;i<m_ChannelNumber;i++)
		{
			if(m_channel[i])
			{
				delete m_channel[i];
				m_channel[i]=NULL;
			}

		}

		delete m_channel;
		m_channel=NULL;
	}
	if (m_castPort)
	{
		for(int i=0;i<m_castcount;i++)
		{
			if(m_castPort[i])
			{
				delete m_castPort[i];
				m_castPort[i]=NULL;
			}
		}

		delete m_castPort;
		m_castPort=NULL;
	}

}
int CDODPort::CreateChannelProcess()
{
	//Clog(LOG_DEBUG,_T("CDODPort::CreateChannelProcess .") );

	if(m_ChannelNumber<1)
	{
		return 1;
	}
	CString str;

	for(int i=0;i<m_ChannelNumber;i++)
	{
		CDODChannel *channel=m_channel[i];
		if(channel==NULL)
		{
			continue;
		}
		channel->m_nPortID=m_nID;
		channel->m_nSessionID=m_nSessionID;
		channel->m_kit=m_kit;

		str.Format("channel%d",channel->m_channelID);

		if (channel->m_nType ==NO_USE_JMS)
		{
			channel->m_strCachingDir=channel->m_QueueName;		
			continue;
		}
		else
			channel->m_strCachingDir=m_currentDIR+"\\"+str;

		if(DirectoryExist(channel->m_strCachingDir.GetBuffer(0)))
		{
			int ret=DeleteDirectory(channel->m_strCachingDir.GetBuffer(0));
			if (!ret)
			{
				Clog(LOG_DEBUG,_T(" CreateChannelProcess: delete m_strCachingDir directory error.(%s)lasterror=%d"),channel->m_sChannelName,GetLastError());
				Clog(LOG_DEBUG,_T(" This directory is %s "),channel->m_strCachingDir);
				continue;
			}
		}

		CString  osTofDir=channel->m_strCachingDir;
// ------------------------------------------------------ Modified by zhenan_ji at 2006Äê1ÔÂ20ÈÕ 19:31:34
// old 	if (CreateDirectory((LPCTSTR)osTofDir,&sa) == FALSE)		

		if (CreateDirectory((LPCTSTR)osTofDir, NULL) == FALSE)		
		{
			Clog(LOG_DEBUG,_T("[%s-%s]CDODPort::CreateChannelProcess ,CreateDirectory is error.lasterror=%d"),m_sPortName,channel->m_sChannelName,GetLastError());
			Clog(LOG_DEBUG,_T(" This directory is %s "),channel->m_strCachingDir);
			continue;
		}

		channel->Create();
		Clog(LOG_DEBUG,_T("[%s-%s]::CreateChannelProcess successful, CachePath=%s"),m_sPortName,channel->m_sChannelName,osTofDir);
	}
//	Clog(LOG_DEBUG,_T("CDODPort::CreateChannelProcess .end") );
	return 0;
}