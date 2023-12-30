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
CDODPort::CDODPort()
{
	m_channel = NULL;
	m_castPort = NULL;
	m_ChannelNumber = 0;
	m_nID = 0;
	m_nSessionID = 0;
	m_nPmtPID = 0;
	m_nTotalRate = 0;
	m_nSessionID = 0;
	m_currentDIR = "";
	m_castcount = 0;
	m_sPortName = "";
	m_nGroupID = 0;
}

CDODPort::~CDODPort()
{
	Clog(LOG_DEBUG,_T("CDODPort::~CDODPort(%s)."),m_sPortName );

	if (m_channel)
	{
		for(int i = 0; i < m_ChannelNumber; i++)
		{
			if(m_channel[i])
			{
				delete m_channel[i];
				m_channel[i] = NULL;
			}
		}

		delete m_channel;
		m_channel = NULL;
	}
	if (m_castPort)
	{
		for(int i = 0; i < m_castcount; i++)
		{
			if(m_castPort[i])
			{
				delete m_castPort[i];
				m_castPort[i] = NULL;
			}
		}
		delete m_castPort;
		m_castPort = NULL;
	}

}
int CDODPort::CreateChannelProcess()
{
	//Clog(LOG_DEBUG,_T("CDODPort::CreateChannelProcess .") );
    CString  osTofDir;
	if(m_ChannelNumber < 1)
	{
		return 1;
	}
	CString str;
	int npos;

	for(int i = 0; i < m_ChannelNumber; i++)
	{
		CDODChannel *channel = m_channel[i];
		if(channel == NULL)
		{
			continue;
		}
		channel->m_nPortID = m_nID;
		channel->m_nSessionID = m_nSessionID;

//		str.Format("channel%d",channel->m_channelID);
		npos = channel->m_sChannelName.Find(':');
		str.Format("channel_%s",channel->m_sChannelName.Mid(npos + 1,channel->m_sChannelName.GetLength() - npos -1));
		Clog(LOG_DEBUG,_T(" CDODPort::CreateChannelProcess(): channelName = %s, channelID = %s ." ),channel->m_sChannelName, str);

		if (channel->m_nType == NO_USE_JMS)
		{
			channel->m_strCachingDir = channel->m_QueueName;
			Clog(LOG_DEBUG,_T(" CDODPort::CreateChannelProcess():channel->m_nType = NO_USE_JMS,CachingDir = %s " ), channel->m_strCachingDir);
			continue;
		}
		else
		{
			channel->m_strCachingDir = m_currentDIR+"\\" + str;
		}
    //if the (channel->m_strCachingDir)directory Exist,  delete the directory first,then Create the Directory.
		if(DirectoryExist(channel->m_strCachingDir.GetBuffer(0)))
		{
	        Clog(LOG_DEBUG,_T(" CDODPort::CreateChannelProcess():directory  CachingDir = [%s] is Exist!" ), channel->m_strCachingDir);
/*			Clog(LOG_DEBUG,_T(" CDODPort::CreateChannelProcess():first delete directory: CachingDir = [%s] !" ), channel->m_strCachingDir);

			int ret = DeleteDirectory(channel->m_strCachingDir.GetBuffer(0),false);

			if (!ret)
			{
				int nError = GetLastError();
				char strError[500];

				GetErrorDescription(nError, strError);
				Clog(LOG_DEBUG,_T(" CreateChannelProcess: delete m_strCachingDir directory error.(%s) GetLastError() = %d, ErrorDescription = %s"),channel->m_sChannelName,nError, strError);
				Clog(LOG_DEBUG,_T(" This directory is %s "),channel->m_strCachingDir);
				continue;
			}
		   Clog(LOG_DEBUG,_T(" CDODPort::CreateChannelProcess():delete directory CachingDir = [%s] success!" ), channel->m_strCachingDir);*/
		}
		else
		{
			 osTofDir = channel->m_strCachingDir;
			if (CreateDirectory((LPCTSTR)osTofDir, NULL) == FALSE)		
			{
				int nError = GetLastError();
				char strError[500];

				GetErrorDescription(nError, strError);
				Clog(LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODPort::CreateChannelProcess ,CreateDirectory is error.GetLastError() = %d, ErrorDescription = %s"),m_sPortName,m_nID,channel->m_sChannelName,channel->m_channelID,nError, strError);
				Clog(LOG_DEBUG,_T(" This directory is %s "),channel->m_strCachingDir);
				continue;
			}
		}
	   channel->Create();
	   Clog(LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::CreateChannelProcess successful, CachePath=%s\n"),m_sPortName,m_nID,channel->m_sChannelName,channel->m_channelID,osTofDir);
	}
//	Clog(LOG_DEBUG,_T("CDODPort::CreateChannelProcess .End") );
	return 0;
}