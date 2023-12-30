// PortManager.cpp: implementation of the CPortManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PortManager.h"
#include "clog.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "Parser.h"
#include "Markup.h"

extern BOOL g_bNotInternalTest;

CPortManager::CPortManager()
{
	m_PortVector.clear();
	m_PortNumber=0;
	m_path="";
	m_Localqueuename="";
	m_kit=new CDODDevKit();

}

CPortManager::~CPortManager()
{
	Stop();
	Sleep(2);
	ClosePort();
	Sleep(2);
	m_PortVector.clear();

	if(m_kit)
	{
		delete m_kit;
		Sleep(2);
		m_kit=NULL;
	}
}

int CPortManager::GetState(int nID)
{/*
	CDODPort *pPort=m_pPort[index];
	if(pPort==NULL)
		{
			return -1;
		}

	return m_kit->GetState(pPort->m_nSessionID,index);*/
	return 0;
}

int  CPortManager::EnableChannel(BOOL bEnable)
{/*
	int nReturn =1;
	
	for(int h=0;h<m_PortNumber;h++)
	{

		CDODPort *pPort=m_pPort[h];

		if(pPort==NULL)
		{
			return 1;
		}
		if(pPort->m_ChannelNumber>1) 
		{
			CDODChannel *channel=pPort->m_channel[1];			
			
			if(channel==NULL)
			{
				return 1;
			}
			
			nReturn=m_kit->EnableChannel(pPort->m_nSessionID,h,channel->m_nStreamID,bEnable);
		}
	}
	*/
	return 0;
}
int CPortManager::Stop()
{
	if (m_kit==NULL)
	{
		return 1;
	}
	CPORTVECTOR::iterator iter;

	for(iter=m_PortVector.begin();iter!=m_PortVector.end();iter++)
	{
		m_kit->StopPort((*iter)->m_nSessionID,(*iter)->m_nID);		
				
	}
	return 0;
}

CString CPortManager::GetPort()
{
	CString str="";

/*	if(m_PortNumber<1)
		return "";

	int nsessionid=0;

	for(int h=0;h<m_PortNumber;h++)
	{

		CDODPort *pPort=m_pPort[h];

		if(pPort==NULL)
		{
			return "";
		}
		
		PPortInfo into;
		m_kit->GetPort(pPort->m_nSessionID,h,&into);			
		if(into.wChannelCount>0)
		{
			str=into.m_Chanel[0].szPath;
		}
	}*/
	return str;
}
int CPortManager::ClosePort()
{/*
	if(m_PortNumber<1)
		return 1;

	int nsessionid=0;

	for(int h=0;h<m_PortNumber;h++)
	{

		CDODPort *pPort=m_pPort[h];

		if(pPort==NULL)
		{
			return 1;
		}

		m_kit->ClosePort(pPort->m_nSessionID,h);			
	}*/

	CPORTVECTOR::iterator iter;

	for(iter=m_PortVector.begin();iter!=m_PortVector.end();)
	{
		m_kit->ClosePort((*iter)->m_nSessionID,(*iter)->m_nID);			

		m_PortVector.erase(iter);
		delete (*iter);
		(*iter)=NULL;
	}

	return 0;
}

int CPortManager::UpdateCatalog()
{/*
	if(m_PortNumber<1)
		return 1;

	int nsessionid=0;

	CString str;

	for(int h=0;h<m_PortNumber;h++)
	{

		CDODPort *pPort=m_pPort[h];

		if(pPort==NULL)
		{
			return 1;
		}

		for(int i=0;i<pPort->m_ChannelNumber;i++)
		{
			CDODChannel *channel=pPort->m_channel[i];
			if(channel==NULL)
			{
				return 1;
			}	
			m_kit->UpdateCatalog(pPort->m_nSessionID,h,channel->m_nStreamID);			
		}
	}*/

	Clog( LOG_DEBUG,_T("CPortManager UpdateCatalog") );

	CPORTVECTOR::iterator iter;

	for(iter=m_PortVector.begin();iter!=m_PortVector.end();iter++)
	{
		for(int i=0;i<(*iter)->m_ChannelNumber;i++)
		{
			CDODChannel *channel=(*iter)->m_channel[i];
			if(channel==NULL)
			{
				return 1;
			}	
			m_kit->UpdateCatalog((*iter)->m_nSessionID,(*iter)->m_nID,channel->m_nStreamID);			
		}
	}
	return 0;
}
int CPortManager::ApplyParameter(CJMSParser *pp)
{
	if(1>int(m_PortVector.size()))
		return 1;

	Clog( LOG_DEBUG,_T("CPortManager ApplyParameter star") );

	int nsessionid=0;
	int nReturn=0;

	CString str;

	CPORTVECTOR::iterator iter;

	for(iter=m_PortVector.begin();iter!=m_PortVector.end();iter++)
	{
		nReturn=m_kit->GetSessionID(nsessionid);
		if(nReturn<0)
			return 1;

		CDODPort *pPort= (*iter);

		if(pPort==NULL)
		{
			return 1;
		}
		pPort->m_kit=m_kit;	

		pPort->m_nSessionID=nsessionid;
	
		str.Format("Port%d",pPort->m_nID);
		pPort->m_currentDIR=m_path+"\\"+str;

		if(!DirectoryExist(m_path,str))
		{
			CString  osTofDir=pPort->m_currentDIR;
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = TRUE;
			if (CreateDirectory((LPCTSTR)osTofDir, &sa) == FALSE)		
			{
				return DODcreatetempdirERROR;
			}
		}
		
		pPort->CreateChannelProcess();		
	}
	
	for(iter=m_PortVector.begin();iter!=m_PortVector.end();iter++)
	{
		CDODPort *psort=(*iter);

		if(psort==NULL)
		{
			return 1;
		}	
		PPortInfo info;

		info.nCastCount=psort->m_castcount;

		for(int h=0;h<psort->m_castcount;h++)
		{
			strcpy(info.m_castPort[h].cDestIp,psort->m_castPort[h]->cDestIp);
			info.m_castPort[h].wDestPort=psort->m_castPort[h]->wDestPort;
			info.m_castPort[h].wSendType=psort->m_castPort[h]->wSendType;
		}
		info.wPmtPID=psort->m_nPmtPID;
		info.wTotalRate=psort->m_nTotalRate;
		_tcscpy( info.szTempPath, psort->m_currentDIR.GetBuffer( 0 ) );

		for(int i=0;i<psort->m_ChannelNumber;i++)
		{
			CDODChannel *channel=psort->m_channel[i];
			if(channel==NULL)
			{
				return 1;
			}	
 
			info.m_Chanel[i].wChannelType=channel->m_nType;
			info.m_Chanel[i].nStreamType=channel->nStreamType;
			info.m_Chanel[i].wPID=channel->m_nStreamID;
			info.m_Chanel[i].wRepeatTime=channel->m_nRepeateTime;
			info.m_Chanel[i].wRate=channel->m_nRate;
		//	_tcscpy(info.m_Chanel[i].cDescriptor, channel->m_strTag);
			strcpy(info.m_Chanel[i].cDescriptor,channel->m_strTag);
			info.m_Chanel[i].bBeDetect=0;
			info.m_Chanel[i].wBeDetectInterval=5000;
			info.m_Chanel[i].wBeEncrypted=0;
			_tcscpy( info.m_Chanel[i].szPath,channel->m_strCachingDir.GetBuffer( 0 ) );
			info.m_Chanel[i].bEnable=TRUE;			
		}
		
		info.wChannelCount=psort->m_ChannelNumber;
		info.m_nSessionID=psort->m_nSessionID;
		int nReturn=0;
		nReturn=m_kit->OpenPort(psort->m_nSessionID,(DWORD)(psort->m_nID),info);
		if(nReturn)
		{
			Clog( 3, _T("OpenPort is error"));
			return 1;
		}
		Sleep(m_PortNumber);
		nReturn=m_kit->RunPort(psort->m_nSessionID,(DWORD) (psort->m_nID));
		Sleep(m_PortNumber);
		if(nReturn)
		{
			Clog( 3, _T("RunPort is error"));
			return 1;
		}		
	}
	Clog( LOG_DEBUG,_T("CPortManager ApplyParameter end") );

	return 0;
}


