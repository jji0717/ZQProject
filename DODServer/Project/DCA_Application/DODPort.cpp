// DODPort.cpp: implementation of the CDODPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DODPort.h"

#include "clog.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern BOOL g_bNotInternalTest;
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
	//m_strName="";
	m_nGroupID=0;
}

CDODPort::~CDODPort()
{
	Clog( 1,_T("CDODPort::~CDODPort().") );

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
	Clog( 1,_T("CDODPort::CreateChannelProcess .") );

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
			return 1;
		}
		channel->m_nPortID=m_nID;
		channel->m_nSessionID=m_nSessionID;
		channel->m_kit=m_kit;

		str.Format("channel%d",channel->m_channelID);

		channel->m_strCachingDir=m_currentDIR+"\\"+str;

		if(DirectoryExist(m_currentDIR,str))
		{
			int ret=DeleteDirectory(channel->m_strCachingDir.GetBuffer(0));
			if (!ret)
			{
				Clog( 1,_T(" remotePath: delete m_strCachingDir directory error.") );
				return 1;
			}
		}

		CString  osTofDir=channel->m_strCachingDir;
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		if (CreateDirectory((LPCTSTR)osTofDir, &sa) == FALSE)		
		{
			Clog( 1,_T("CDODPort::CreateChannelProcess ,CreateDirectory is error.") );
			return 1;
		}

		channel->Create();
	}
	Clog( 1,_T("CDODPort::CreateChannelProcess .end") );
	return 0;
}