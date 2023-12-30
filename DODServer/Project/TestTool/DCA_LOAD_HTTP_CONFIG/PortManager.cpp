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

/*
static CPortManager *g_pPortManager=NULL;
//extern BOOL g_bNotInternalTest;
void CALLBACK  UpdateChannelProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	Clog( LOG_DEBUG,_T(" CPortManager::UpdateChannelProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)!"));
	if(1>(int)(g_pPortManager->m_PortVector.size()))
	{
		return ;
	}
	CPORTVECTOR::iterator iter;

	for(iter=g_pPortManager->m_PortVector.begin();iter!=g_pPortManager->m_PortVector.end();iter++)
	{
		for(int i=0;i<(*iter)->m_ChannelNumber;i++)
		{
			CDODChannel *channel=(*iter)->m_channel[i];
			if(channel==NULL)
			{
				return ;
			}
			if (channel->m_bNeedUpdateChannel)
			{
				channel->m_bNeedUpdateChannel=FALSE;
				Clog( LOG_DEBUG,_T("[%s-%s]::UpdateChannel will start!"),channel->m_sPortName,channel->m_sChannelName);
				g_pPortManager->m_kit->UpdateCatalog((*iter)->m_nSessionID,(*iter)->m_nID,channel->m_nStreamID);			
			}
		}
	}

	//if (m_bNeedUpdateChannel ==FALSE)
	//	return;
	//	DataUpdated();
	//m_bNeedUpdateChannel=FALSE;
	// 
	//if (m_kit ==NULL)
	//	return ;
	//
	//m_kit->UpdateCatalog(m_nSessionID,m_nPortID,m_nStreamID);
}*/


DWORD WINAPI UpdateChannelProc(LPVOID lpParam) 
{
	CPortManager *g_p=(CPortManager *)lpParam;
	int  second=0;
	COleDateTime oleWriteLogTime=COleDateTime::GetCurrentTime();
//	Clog( LOG_DEBUG,_T(" CPortManager::UpdateChannelProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)!"));
	try
	{	
		while (1)
		{
			if(1>(int)(g_p->m_PortVector.size()))
			{
				if(g_bStop)
				{
					Clog( LOG_DEBUG,_T("Exit UpdateChannelProc thread :main circle POS"));
					return  0;
				}
				Sleep(3);
				continue;
			}


			CPORTVECTOR::iterator iter;

			for(iter=g_p->m_PortVector.begin();iter!=g_p->m_PortVector.end();iter++)
			{
				if(g_bStop)
				{
					Clog( LOG_DEBUG,_T("Exit UpdateChannelProc thread port circle POS"));
					return  0;
				}
				Sleep(2);

				for(int i=0;i<(*iter)->m_ChannelNumber;i++)
				{
					if(g_bStop)
					{
						Clog( LOG_DEBUG,_T("Exit UpdateChannelProc thread channel circle POS"));
						return  0;
					}
					Sleep(2);

					COleDateTimeSpan olets=COleDateTime::GetCurrentTime() - oleWriteLogTime;
					second =(int) (olets.GetTotalSeconds()); 
					if (second > (WRITEPORTMANAGERLOGTIME))
					{
						Clog( LOG_DETAIL,_T("Active !"));
						oleWriteLogTime=COleDateTime::GetCurrentTime();
					}

					CDODChannel *channel=(*iter)->m_channel[i];
					if(channel==NULL)
					{
						return 1;
					}
					if (channel->m_bNeedUpdateChannel==FALSE)
						continue;

					olets=COleDateTime::GetCurrentTime() - channel->m_LastUpdateTime;
					second =(int) (olets.GetTotalSeconds()); 
					//	Clog( LOG_DEBUG,_T(" Updatesecond = %d ,InterVal =%d"),second,g_p->m_nUpdateInterVal);
					if(second > g_p->m_nUpdateInterVal)
					{
						channel->m_bNeedUpdateChannel=FALSE;
						Clog( LOG_DEBUG,_T("[%s-%s]::UpdateChannel will start!"),channel->m_sPortName,channel->m_sChannelName);
						g_p->m_kit->UpdateCatalog((*iter)->m_nSessionID,(*iter)->m_nID,channel->m_nStreamID);			
						channel->m_LastUpdateTime=COleDateTime::GetCurrentTime();
					}
				}
			}
			Sleep(2);
		}
	}
	catch (...) 
	{
		Clog( LOG_DEBUG,_T(" UpdateChannel thread error,lasterror=%d "),GetLastError());
		return 0;
	}
	return 1;
}
void CPortManager::RunUpdateThread()
{
	DWORD IDThread;
	m_hUpdateThread=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)UpdateChannelProc,this,0 , &IDThread); 
}
CPortManager::CPortManager(CString logpath,int InterVal)
{
	m_PortVector.clear();
	m_PortNumber=0;
	m_path=logpath;
	m_Localqueuename="";
//	m_nUpdateInterval=0;
	m_kit=new CDODDevKit(logpath.GetBuffer(0));
//	g_pPortManager=this;
//	Clog( LOG_DEBUG,_T(" CPortManager::CPortManager(CString logpath,int InterVal =%d"),InterVal);
	//m_timerUpdate=(UINT)SetTimer(NULL, 0, (UINT)InterVal, UpdateChannelProc);

	m_nUpdateInterVal=InterVal;
	m_sDataTypeInitial="";
	//if ((m_timerUpdate = (UINT)SetTimer(NULL, 1, g_InterVal, UpdateChannelProc)) == 0)
	//{
	//	Clog( LOG_DEBUG,_T(" Error in create updata channel by settimer ,InterVal =%d"),InterVal);
	//}
}

CPortManager::~CPortManager()
{	
	Clog(LOG_DEBUG,_T("CPortManager :stop .before"));
	Stop();
	Clog(LOG_DEBUG,_T("CPortManager :stop .after"));

	Sleep(2);

	ClosePort();
	Clog(LOG_DEBUG,_T("CPortManager :ClosePort .after"));

	Sleep(2);
	m_PortVector.clear();
	Clog(LOG_DEBUG,_T("delete m_kit .before"));

	if(m_kit)
	{
		delete m_kit;
		Sleep(2);
		m_kit=NULL;
	}
	Clog(LOG_DEBUG,_T("delete m_kit .after"));

}
//static void CALLBACK UpdateChannelProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


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
		Clog(LOG_DEBUG,_T("CPortManager::ClosePort(%s) .before"),(*iter)->m_sPortName);

		m_kit->ClosePort((*iter)->m_nSessionID,(*iter)->m_nID);			

		delete (*iter);
		(*iter)=NULL;
		m_PortVector.erase(iter);

		Clog(LOG_DEBUG,_T("CPortManager::ClosePort() end.") );
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
	}

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
	}*/
//	Clog( LOG_DEBUG,_T("CPortManager UpdateCatalog") );
	return 0;
}
int CPortManager::ApplyParameter()
{
	if(1>int(m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("parse config content error"));		
		return 1;
	}

	//Clog( LOG_DEBUG,_T("CPortManager ApplyParameter star") );

	int nsessionid=0;
	int nReturn=0;

	CString str;

	CPORTVECTOR::iterator iter;

	for(iter=m_PortVector.begin();iter!=m_PortVector.end();iter++)
	{
		while (1)
		{
			if(g_bStop)
				return  0;
			nReturn=m_kit->GetSessionID(nsessionid);
			if(nReturn<0)
			{
				Clog( LOG_DEBUG,_T("m_kit->GetSessionID (%s) error"),(*iter)->m_sPortName );
				Sleep(5);
			}
			else
				break;
		}

		CDODPort *pPort= (*iter);

		if(pPort==NULL)
		{
			return 1;
		}
		pPort->m_kit=m_kit;	

		pPort->m_nSessionID=nsessionid;
	
		str.Format("Port%d",pPort->m_nID);
		pPort->m_currentDIR=m_path+"\\"+str;

		if(!DirectoryExist(pPort->m_currentDIR.GetBuffer(0)))
		{
			Clog(LOG_DEBUG,"current port path is not exist path=%s",pPort->m_currentDIR);
			CString  osTofDir=pPort->m_currentDIR;
			/*SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = TRUE;*/
			if (CreateDirectory((LPCTSTR)osTofDir, NULL) == FALSE)		
			{
				Clog(LOG_DEBUG,_T("[%s]CDODPort::CreateChannelProcess ,CreateDirectory dir=(%s)is error.lasterror=%d"),pPort->m_sPortName,pPort->m_currentDIR,GetLastError());
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
		try
		{

			PPortInfo info;
			info.lstChannelInfo.clear();
			info.lstIPPortInfo.clear();

			info.nCastCount=psort->m_castcount;

			for(int h=0;h<psort->m_castcount;h++)
			{
				ZQSBFIPPORTINF TmpIPPort;
				memcpy(&TmpIPPort,(psort->m_castPort[h]),sizeof(ZQSBFIPPORTINF));
				info.lstIPPortInfo.push_back(TmpIPPort);
			}
			info.wPmtPID=psort->m_nPmtPID;
			info.wTotalRate=psort->m_nTotalRate;
			_tcscpy( info.szTempPath, "\0");

			for(int i=0;i<psort->m_ChannelNumber;i++)
			{
				CDODChannel *channel=psort->m_channel[i];
				if(channel==NULL)
				{
					return 1;
				}	
				PPChannelInfo TmpChannel;
				TmpChannel.wMultiplestream=channel->m_nMultiplestream;
				TmpChannel.wStreamCount=channel->m_nStreamCount;
				TmpChannel.wChannelType=channel->m_nType;
				TmpChannel.nStreamType=channel->nStreamType;
				TmpChannel.wPID=channel->m_nStreamID;
				TmpChannel.wRepeatTime=channel->m_nRepeateTime;
				TmpChannel.wRate=channel->m_nRate;
				// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月27日 16:27:41
				BYTE tmode=0;

				if (channel->m_nMultiplestream)
					tmode+=4;

				if (channel->m_nSendWithDestination)
					tmode+=2;
				else
					tmode+=1;

				TmpChannel.wPackagingMode=tmode;

				// ------------------------------------------------------ Modified by zhenan_ji at 2005年10月20日 16:33:31
				//if set .bBeDetect=1,there are some error,guess.but that time, at the big rate.
				//because sourcefilter did not supperted auto moniter the path,
				/*		if (channel->m_nType ==NO_USE_JMS)
				{

				}*/
				strcpy(TmpChannel.cDescriptor,channel->m_strTag);

				if (channel->m_nType ==NO_USE_JMS)
				{
					TmpChannel.bBeDetect=1;
					TmpChannel.wBeDetectInterval=DETECTINTERVAL;
				}
				else
				{
					TmpChannel.bBeDetect=0;
					TmpChannel.wBeDetectInterval=0;
				}

				TmpChannel.wBeEncrypted=channel->m_bEncrypted;

				if (channel->m_strCachingDir.GetLength() >=MAX_PATH )
				{
					Clog(LOG_DEBUG, _T("OpenPort (%s) is error ,CachingDir.GetLength() > max_path (%s)"),psort->m_sPortName,channel->m_strCachingDir);
					return 1;
				}

				_tcscpy( TmpChannel.szPath,channel->m_strCachingDir.GetBuffer( 0 ) );
				TmpChannel.bEnable=TRUE;	

				info.lstChannelInfo.push_back(TmpChannel);
			}

			info.wChannelCount=psort->m_ChannelNumber;
			info.m_nSessionID=psort->m_nSessionID;
			int nReturn=0;
			nReturn=m_kit->OpenPort(psort->m_nSessionID,(DWORD)(psort->m_nID),&info);
			if(nReturn)
			{
				Clog(LOG_DEBUG, _T("OpenPort (%s) is error"),psort->m_sPortName);
				return 1;
			}
			Sleep(m_PortNumber);
			nReturn=m_kit->RunPort(psort->m_nSessionID,(DWORD) (psort->m_nID));
			Sleep(m_PortNumber);
			if(nReturn)
			{
				Clog(LOG_DEBUG, _T("RunPort (%s) is error"),psort->m_sPortName);
				return 1;
			}	
		}
		catch (...) 
		{
			Clog( LOG_DEBUG,_T("PortManager send some commands (openport and runport) to SRM error lastError=%d !"),GetLastError());
		}
	}
	Clog( LOG_DEBUG,_T("PortManager send some commands (openport and runport) to SRM successful !") );

	return 0;
}


