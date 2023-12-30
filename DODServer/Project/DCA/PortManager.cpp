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
#include "messagemacro.h"

extern  BOOL	g_bStop;
#define WRITEPORTMANAGERLOGTIME 120
CDODCHANNLEQUEUE CPortManager::m_DelChannelQueue;
HANDLE CPortManager::m_hUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
CRITICAL_SECTION CPortManager::m_UpdateCriticalSection;
int CPortManager::m_nUpdateInterVal;
/*DWORD WINAPI UpdateChannelProc(LPVOID lpParam) 
{
	CPortManager *pPM = (CPortManager *)lpParam;
	int  second = 0;
	COleDateTime oleWriteLogTime = COleDateTime::GetCurrentTime();
//	Clog( LOG_DEBUG,_T(" CPortManager::UpdateChannelProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)!"));
	try
	{	
		while (1)
		{
			if(1 > (int)(pPM->m_PortVector.size()))
			{
				if(g_bStop)
				{
					Clog(LOG_DEBUG,"Exit UpdateChannelProc thread.");
					return  0;
				}
				Sleep(3);
				continue;
			}

			CPORTVECTOR::iterator iter;

			for(iter = pPM->m_PortVector.begin(); iter != pPM->m_PortVector.end(); iter++)
			{
				if(g_bStop)
				{
					Clog(LOG_DEBUG,"Exit UpdateChannelProc thread.");
					return  0;
				}
				Sleep(2);

				if ((*iter) == NULL)
				{
					Clog(LOG_ERROR,"find it_point is null in Port_vector.Error.exit thread");
                    return 0;
				}
				Clog(LOG_DETAIL,"[%s]Get current Port",(*iter)->m_sPortName);

				for(int i = 0; i < (*iter)->m_ChannelNumber; i++)
				{
					if(g_bStop)					
					{
						Clog(LOG_DEBUG,"Exit UpdateChannelProc thread.");
						return  0;
					}
					Sleep(2);

					COleDateTimeSpan olets = COleDateTime::GetCurrentTime() - oleWriteLogTime;
					second =(int) (olets.GetTotalSeconds()); 

					if (second > (WRITEPORTMANAGERLOGTIME))
					{
						Clog( LOG_DEBUG,_T("Active !"));
						oleWriteLogTime = COleDateTime::GetCurrentTime();
					}

					CDODChannel *channel = (*iter)->m_channel[i];
					if(channel == NULL)
					{
						Clog(LOG_ERROR,"find channel_point is null in channel_vector.Error");
						return 1;
					}
					Clog( LOG_DETAIL,_T("[%s,PortId:%d - %s,ChannelId:%d]Check current NeedUpdate state "),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName,channel->m_channelID);

					if (channel->m_bNeedUpdateChannel == FALSE)
						continue;

					olets = COleDateTime::GetCurrentTime() - channel->m_LastUpdateTime;
					second = (int) (olets.GetTotalSeconds()); 

					Clog( LOG_DETAIL,_T("[%s,PortId:%d - %s,ChannelId:%d]::Compare current time and Last Update Time "),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName,channel->m_channelID);

					if(second > pPM->m_nUpdateInterVal)
					{
						channel->m_bNeedUpdateChannel = FALSE;

						Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::UpdateChannel will start!"),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName,channel->m_channelID);

						pPM->m_kit->UpdateCatalog((*iter)->m_nSessionID,(*iter)->m_nID,channel->m_nStreamID);

						channel->m_LastUpdateTime = COleDateTime::GetCurrentTime();

						//Clog( LOG_DETAIL,_T("[%s,PortId:%d - %s,ChannelId:%d]::Update operation end!"),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName,channel->m_channelID);
						Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::Update operation end!"),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName,channel->m_channelID);
					}
				}
			}
			Sleep(2);
		}
	}
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog( LOG_DEBUG,_T(" UpdateChannel thread error,GetLastError() = %d, ErrorDescription = %s"),nError,strError);
		return 0;
	}
	return 1;
}*/

UINT UpdateChannelProc(LPVOID lpParam) 
{
	CPortManager *pPM = (CPortManager *)lpParam;
	CDODCHANNLEQUEUE::iterator iter;
	int timeout = INFINITE;
	int second;
	try
	{
		while(!g_bStop)
		{     
			if(pPM->m_DelChannelQueue.size() > 0 )
			{
				    iter = pPM->m_DelChannelQueue.begin();
					COleDateTimeSpan olets =  (*iter)->m_UpdateTime - COleDateTime::GetCurrentTime() ;
					second = (int) (olets.GetTotalSeconds());

					if(second > 0 )
						timeout = second * 1000;
					else
						timeout = 0;
			}
			else
			{
				timeout = INFINITE;
			}
			Clog( LOG_DEBUG,_T("UpdateChannelProc Vect size:%d"),pPM->m_DelChannelQueue.size());

			if( WaitForSingleObject(pPM->m_hUpdateEvent, timeout) == WAIT_OBJECT_0)
			{
				ResetEvent(pPM->m_hUpdateEvent);
				continue;
			}

			EnterCriticalSection(&(pPM->m_UpdateCriticalSection));
			(*iter)->m_LastUpdateTime =  COleDateTime::GetCurrentTime();
			pPM->m_DelChannelQueue.pop_front();
			LeaveCriticalSection(&(pPM->m_UpdateCriticalSection));

			Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::UpdateChannel will start!"),(*iter)->m_sPortName,(*iter)->m_nPortID,(*iter)->m_sChannelName,(*iter)->m_channelID);

			pPM->m_kit->UpdateCatalog((*iter)->m_nSessionID,(*iter)->m_nPortID,(*iter)->m_nStreamID);

			Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::Update operation end!"),(*iter)->m_sPortName,(*iter)->m_nPortID,(*iter)->m_sChannelName,(*iter)->m_channelID);		
		}
	}

	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog( LOG_DEBUG,_T(" UpdateChannel thread error,GetLastError() = %d, ErrorDescription = %s"),nError,strError);
		return 0;
	}
	Clog( LOG_DEBUG,_T(" CPortManager::UpdateChannel thread Exit!!"));
	return 1;
}
void CPortManager::RunUpdateThread()
{
	DWORD IDThread;
	m_hUpdateThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)UpdateChannelProc,this,0 , &IDThread); 
//	m_hUpdateThread = AfxBeginThread(UpdateChannelProc,this,THREAD_PRIORITY_NORMAL);
}
CPortManager::CPortManager(CString logpath,int InterVal)
{
	m_PortVector.clear();
	m_DelChannelQueue.clear();
	m_PortNumber = 0;
	m_path=logpath;
	m_Localqueuename = "";
	m_kit = new CDODDevKit(logpath.GetBuffer(0));
	m_nUpdateInterVal = InterVal;
	m_sDataTypeInitial = "";
	m_hUpdateThread = NULL;
	InitializeCriticalSection(&m_UpdateCriticalSection);
}

CPortManager::~CPortManager()
{	
	if( m_hUpdateThread )	
	{
		WaitForSingleObject( m_hUpdateThread, INFINITE );
		CloseHandle(m_hUpdateThread);
		m_hUpdateThread = NULL;
		CloseHandle(m_hUpdateEvent);
	}
    DeleteCriticalSection(&m_UpdateCriticalSection);

	Clog( LOG_DEBUG,_T("Send stoping_command to all ports and channels"));
	Stop();
	Sleep(2);

	Clog( LOG_DEBUG,_T("Free all ports and channels"));
	ClosePort();
	Sleep(2);

	m_PortVector.clear();

	Clog( LOG_DEBUG,_T("Free Develop Kit"));
	if(m_kit)
	{
		delete m_kit;
		Sleep(2);
		m_kit = NULL;
	}
}

int CPortManager::GetState(int nID)
{
	/*
	CDODPort *pPort=m_pPort[index];
	if(pPort==NULL)
		{
			return -1;
		}

	return m_kit->GetState(pPort->m_nSessionID,index);*/
	return 0;
}

int  CPortManager::EnableChannel(BOOL bEnable)
{
	/*
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
	SetEvent(m_hUpdateEvent);
	if (m_kit == NULL)
	{
		Clog(LOG_ERROR,"PortManager::Stop:m_kit is null .Error");
		return 1;
	}
	CPORTVECTOR::iterator iter;

	for(iter = m_PortVector.begin(); iter != m_PortVector.end(); iter++)
	{
		if (*iter == NULL)
		{
			Clog(LOG_ERROR,"PortManager::Stop:find port is null .Error");
			continue;
		}
		for(int i = 0; i < (*iter)->m_ChannelNumber; i++)
		{	
			CDODChannel *channel = (*iter)->m_channel[i];
			if(channel == NULL)
			{
				Clog(LOG_ERROR,"Stop:find channel is null in channel_array.Error");
				continue;
			}
			channel->Stop();			
		}	
	}

	for(iter = m_PortVector.begin(); iter != m_PortVector.end(); iter++)
	{		
		if (*iter == NULL)
		{
			Clog(LOG_ERROR,"PortManager::Stop:find port is null .Error");
			continue;
		}

		Clog(LOG_ERROR,"[%s]Send Stop_command to srm",(*iter)->m_sPortName);
		m_kit->StopPort((*iter)->m_nSessionID,(*iter)->m_nID);	
	}
	return 0;
}

CString CPortManager::GetPort()
{
	CString str = "";

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
{
	CPORTVECTOR::iterator iter;

	for(iter = m_PortVector.begin(); iter != m_PortVector.end();)
	{
		Clog(LOG_DEBUG,_T("CPortManager::ClosePort(%s) .before"),(*iter)->m_sPortName);

		m_kit->ClosePort((*iter)->m_nSessionID,(*iter)->m_nID);			

		delete (*iter);
		(*iter) = NULL;
		m_PortVector.erase(iter);

		Clog(LOG_DEBUG,_T("CPortManager::ClosePort() end.") );
	}

	return 0;
}

int CPortManager::UpdateCatalog()
{
	/* test code.
	if(m_PortNumber < 1)
		return 1;

	int nsessionid = 0;

	CString str;

	for(int h = 0; h < m_PortNumber; h++)
	{

		CDODPort *pPort = m_pPort[h];

		if(pPort == NULL)
		{
			return 1;
		}

		for(int i = 0; i < pPort->m_ChannelNumber; i++)
		{
			CDODChannel *channel = pPort->m_channel[i];
			if(channel == NULL)
			{
				return 1;
			}	
			m_kit->UpdateCatalog(pPort->m_nSessionID,h,channel->m_nStreamID);			
		}
	}

	CPORTVECTOR::iterator iter;

	for(iter = m_PortVector.begin(); iter != m_PortVector.end(); iter++)
	{
		for(int i = 0; i < (*iter)->m_ChannelNumber; i++)
		{
			CDODChannel *channel = (*iter)->m_channel[i];
			if(channel == NULL)
			{
				return 1;
			}	
			m_kit->UpdateCatalog((*iter)->m_nSessionID,(*iter)->m_nID,channel->m_nStreamID);			
		}
	}*/
//	Clog( LOG_DEBUG,_T("CPortManager UpdateCatalog") );
	return 0;
}
int CPortManager::ApplyParameter()
{	
	Clog( LOG_DEBUG,_T("CPortManager ApplyParameter Begin!") );

	if(1>int(m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("parse config content error"));	
		Clog( LOG_DEBUG,_T("CPortManager::ApplyParameter() error: PortNumber is zero!"));	
		return 1;
	}

	int nsessionid = 0;
	int nReturn = 0;

	CString str, strPortName;

	CPORTVECTOR::iterator iter;

	for(iter = m_PortVector.begin();iter != m_PortVector.end();iter++)
	{
		CDODPort *pPort = (*iter);

		if(pPort == NULL)
		{
			Clog( LOG_DEBUG,_T("CPortManager::ApplyParameter() error:Port is NULL!"));
			return 1;
		}

		while (1)
		{
			if(g_bStop)
				return  0;
			nReturn = m_kit->GetSessionID(nsessionid);
			if(nReturn < 0)
			{
				Clog( LOG_DEBUG,_T("m_kit->GetSessionID (%s) error"),(*iter)->m_sPortName );
				Sleep(5);
			}
			else
				break;
		}

		pPort->m_nSessionID = nsessionid;
//		str.Format("Port%d",pPort->m_nPmtPID);       
		str = pPort->m_sPortName;
		int npos = str.ReverseFind(':');
		strPortName.Format("Port_%s",str.Mid(npos + 1,str.GetLength() - npos -1));

		pPort->m_currentDIR = m_path + "\\" + strPortName ;

	   Clog( LOG_DEBUG,_T(" CPortManager::ApplyParameter():current PortName = %s ,PortId = %d!"),pPort->m_sPortName ,pPort->m_nID);

		if(!DirectoryExist(pPort->m_currentDIR.GetBuffer(0)))
		{
			Clog(LOG_DEBUG,"current port path is not exist[ path = %s ]",pPort->m_currentDIR);
			CString  osTofDir = pPort->m_currentDIR;
			if (CreateDirectory((LPCTSTR)osTofDir, NULL) == FALSE)		
			{
				int nError = GetLastError();
				char strError[500];

				GetErrorDescription(nError, strError);
				Clog(LOG_DEBUG,_T("[%s]CDODPort::CreatePortDirectory[ path = %s ]is error.GetLastError() = %d, ErrorDescription = %s"),pPort->m_sPortName,pPort->m_currentDIR,nError, strError);
				return DODcreatetempdirERROR;
			}
			Clog(LOG_DEBUG,_T("[%s]CDODPort::CreatePortDirectory[ path = %s ]  success !"),pPort->m_sPortName,pPort->m_currentDIR);
		}
		else
		{
           Clog(LOG_DEBUG,"current port path is  exist[ path = %s ]",pPort->m_currentDIR);
		}

		pPort->CreateChannelProcess();	
	}

	for(iter = m_PortVector.begin(); iter != m_PortVector.end(); iter++)
	{
		CDODPort *psort = (*iter);

		if(psort == NULL)
		{
			return 1;
		}	
		try
		{
			PPortInfo info;
			info.lstChannelInfo.clear();
			info.lstIPPortInfo.clear();

			info.nCastCount = psort->m_castcount;

			for(int h = 0; h < psort->m_castcount; h++)
			{
				ZQSBFIPPORTINF TmpIPPort;
				memcpy(&TmpIPPort,(psort->m_castPort[h]),sizeof(ZQSBFIPPORTINF));
				info.lstIPPortInfo.push_back(TmpIPPort);
			}
			info.wPmtPID = psort->m_nPmtPID;
			info.wTotalRate = psort->m_nTotalRate;
			_tcscpy( info.szTempPath, "\0");

			for(int i = 0; i < psort->m_ChannelNumber; i++)
			{
				CDODChannel *channel = psort->m_channel[i];
				if(channel == NULL)
				{
					return 1;
				}	
				PPChannelInfo TmpChannel;
				TmpChannel.wMultiplestream = channel->m_nMultiplestream;
				TmpChannel.wStreamCount = channel->m_nStreamCount;
				TmpChannel.wChannelType = channel->m_nType;
				TmpChannel.nStreamType = channel->nStreamType;
				TmpChannel.wPID = channel->m_nStreamID;
				TmpChannel.wRepeatTime = channel->m_nRepeateTime;
				TmpChannel.wRate=channel->m_nRate;
				// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê12ÔÂ27ÈÕ 16:27:41
				// by document,deliver new parameter to srm for datawrapper 
				BYTE tmode = 0;

				if (channel->m_nMultiplestream)
					tmode += 4;

				if (channel->m_nSendWithDestination)
					tmode += 2;
				else
					tmode += 1;

				TmpChannel.wPackagingMode = tmode;
				strcpy(TmpChannel.cDescriptor,channel->m_strTag);

				if (channel->m_nType == NO_USE_JMS)
				{
					TmpChannel.bBeDetect = 1;
					TmpChannel.wBeDetectInterval = DETECTINTERVAL;
				}
				else
				{
					TmpChannel.bBeDetect = 0;
					TmpChannel.wBeDetectInterval = 0;
				}

				TmpChannel.wBeEncrypted = channel->m_bEncrypted;

				if (channel->m_strCachingDir.GetLength() >= MAX_PATH )
				{
					Clog(LOG_DEBUG, _T("OpenPort (%s) is error ,CachingDir.GetLength() > max_path (%s)"),psort->m_sPortName,channel->m_strCachingDir);
					return 1;
				}

				_tcscpy( TmpChannel.szPath,channel->m_strCachingDir.GetBuffer( 0 ) );
				TmpChannel.bEnable = TRUE;	

				info.lstChannelInfo.push_back(TmpChannel);
			}

			info.wChannelCount = psort->m_ChannelNumber;
			info.m_nSessionID = psort->m_nSessionID;
			int nReturn = 0;

			nReturn = m_kit->OpenPort(psort->m_nSessionID,(DWORD)(psort->m_nID),&info);
			if(nReturn)
			{
				Clog(LOG_DEBUG, _T("OpenPort portname =(%s), portid = %d,SessionID = %d, nReturn = %d error"),psort->m_sPortName, (DWORD)(psort->m_nID),psort->m_nSessionID,nReturn);
				return 1;
			}

			Sleep(m_PortNumber);
			nReturn = m_kit->RunPort(psort->m_nSessionID,(DWORD) (psort->m_nID));

			Sleep(m_PortNumber);
			if(nReturn)
			{
				Clog(LOG_DEBUG, _T("RunPort portname =(%s), portid = %d,SessionID = %d ,nReturn = %d error"),psort->m_sPortName, (DWORD)(psort->m_nID),psort->m_nSessionID,nReturn);
				return 1;
			}	
		}
		catch (...) 
		{
			int nError = GetLastError();
			char strError[500];

			GetErrorDescription(nError, strError);
			Clog( LOG_DEBUG,_T("PortManager send some commands (openport and runport) to SRM error GetLastError() =%d ,ErrorDescription = %s !"),nError,strError);
		}
	}
	Clog( LOG_DEBUG,_T("PortManager send some commands (openport and runport) to SRM successful !") );
	Clog( LOG_DEBUG,_T("CPortManager ApplyParameter End!") );
	return 0;
}

void CPortManager::AddUpdateChannel(CDODChannel* pChannelInfo)
{
	COleDateTimeSpan olets;
	int  second = 0;
	CDODCHANNLEQUEUE::iterator iter;
     
	if(pChannelInfo->m_nType != DATAEXCHANGETYPE_MESSAGE_FORMAT)
	{
		return;
	}

	olets = COleDateTime::GetCurrentTime() - pChannelInfo->m_LastUpdateTime;
	second = (int) (olets.GetTotalSeconds()); //get update time interval

	if(second > m_nUpdateInterVal)
	{
		second = m_nUpdateInterVal;
	}

	EnterCriticalSection(&m_UpdateCriticalSection);
	if (m_DelChannelQueue.size() >= 1)
	{
		for(iter = m_DelChannelQueue.begin();iter != m_DelChannelQueue.end();++iter)
		{		
			if(*iter == pChannelInfo)
			{
				Clog(LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]This Channel is Exist in UpdateChannel!"),pChannelInfo->m_sPortName,pChannelInfo->m_nPortID,pChannelInfo->m_sChannelName,pChannelInfo->m_channelID);
				LeaveCriticalSection(&m_UpdateCriticalSection);
				return ;
			}	
		}
	}

  	olets.SetDateTimeSpan(0, 0, 0, m_nUpdateInterVal - second); 
	pChannelInfo->m_UpdateTime = COleDateTime::GetCurrentTime() + olets;

	for(iter = m_DelChannelQueue.begin();iter != m_DelChannelQueue.end();)
	{	
		olets = pChannelInfo->m_UpdateTime - (*iter)->m_UpdateTime;
		second = olets.GetTotalSeconds();
		if(second > 0 )	
			iter++;
		else
			break;
	}
	if(iter == m_DelChannelQueue.begin())
	{
		Clog(LOG_DEBUG,_T("This Channel is Push_front!"));
		m_DelChannelQueue.push_front(pChannelInfo);
		SetEvent(m_hUpdateEvent);
	}
	else
		if(iter != m_DelChannelQueue.end())
		{
			Clog(LOG_DEBUG,_T("This Channel is Push_back!"));
			m_DelChannelQueue.insert(iter, pChannelInfo);
		}
		else
		{
			Clog(LOG_DEBUG,_T("This Channel is Push_middle!"));
			m_DelChannelQueue.push_back(pChannelInfo);
		}	
    Clog(LOG_DEBUG, _T("CPortManager::AddUpdateChannel QueueSize is %d"), m_DelChannelQueue.size());
	LeaveCriticalSection(&m_UpdateCriticalSection);
}
