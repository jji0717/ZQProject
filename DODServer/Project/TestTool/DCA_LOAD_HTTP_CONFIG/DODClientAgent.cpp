// DODClineAgent.cpp: implementation of the CDODClientAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DODClientAgent.h"
#include "clog.h"
#include "Markup.h"
#include "FileFindExt.h"
#include "messagemacro.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//CDODClientAgent *g_pDCAgent;

extern  BOOL	g_bStop;
//BOOL g_bNotInternalTest;
BOOL g_ConnectJBossIsOK;

#define Time_interval	5

CDODClientAgent::CDODClientAgent()
{
	m_jms=NULL;
}

CDODClientAgent::~CDODClientAgent()
{
}
BOOL CDODClientAgent::Destroy()
{
	UnInitialize();
	return TRUE;
}
BOOL CDODClientAgent::create()
{
//	g_bNotInternalTest=TRUE;
	m_Parse=NULL;
	m_pPortManager=NULL;
	Initialize();	
	return TRUE;
}

BOOL CDODClientAgent::Pause()
{
	return TRUE;
}
BOOL CDODClientAgent::Resume()
{
	return TRUE;
}

void CDODClientAgent::Initialize()
{
	Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .start") );
    
	//return;

	//CString strTmp;

	//strTmp = "d:\\work";
	//
	//DirectoryExist(strTmp.GetBuffer(0));

	//if(!DirectoryExist(strTmp))
	//	Clog( LOG_DEBUG,_T("CDODClientAgent::is not Exist ") );
	//else
	//	Clog( LOG_DEBUG,_T("CDODClientAgent:Exist") );

//	return;


	//strTmp=strTmp.TrimLeft();
	//strTmp=strTmp.TrimRight();
	//Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .start==%s ") ,strTmp);

//
//	TCHAR szRemote[MAX_PATH] = _T("\\\\");
//	_tcscat( szRemote, "192.168.80.114" );
//
//	NETRESOURCE tmpNetRes;
//	tmpNetRes.dwType = RESOURCETYPE_ANY;
//	tmpNetRes.lpLocalName = NULL;
//	tmpNetRes.lpRemoteName = szRemote;
//	tmpNetRes.lpProvider = NULL;
//
//	CString m_sUserName="ab";	CString m_sPasswd="ab";
////	CString m_sUserName="administrator";	CString m_sPasswd="ITV";
//	//if(0)
//	DWORD nRetrun;
//	nRetrun=WNetAddConnection2( &tmpNetRes, m_sUserName, m_sPasswd, CONNECT_UPDATE_PROFILE );
//	if(nRetrun!= NO_ERROR )
//	{
//		int nError=GetLastError();
//		Clog( LOG_DEBUG,_T("WNetAddConnection2::nRetrun=%d error"),nRetrun);
//		Clog( LOG_DEBUG,_T("WNetAddConnection2::GetLastError=%d error"),nError);
//		Clog( LOG_DEBUG, _T("Auto logon the remote server... IP Address:%s"),szRemote);
//		//return ;
//	}//ERROR_BAD_NET_NAME

	//	CString remotePath="\\\\192.168.80.142\\forDOD\\1126240089343";
	
	//CString m_strCachingDir="D:\\work\\DCA\\DCA\\Debug\\Port1";
/*
	CString remotePath;
	remotePath="dfsff";
	CString m_strCachingDir;
	m_strCachingDir="dsdf";
	CFileFindExt ProcessDir;
	BOOL ret=ProcessDir.DoProcess(remotePath,FileCopy,m_strCachingDir); 

	if (!ret)
	{
		Clog(LOG_DEBUG,_T("CDODChannel.Copy allfile to cacheFolder error!  ") );
		return ;
	}

	//remotePath="\\\\192.168.80.95\\forDOD\\1124429609906";
	ret=DeleteDirectory(remotePath.GetBuffer(0));
	if (!ret)
	{
		Clog(LOG_DEBUG,_T(" remotPath: del remot director error.") );
		return ;
	}
	return;*/

	::CoInitialize(NULL);
	CString        strCurDir;
	char           sModuleName[1025];
	DWORD dSize = GetModuleFileName(NULL,sModuleName,1024);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.ReverseFind('\\');
	strCurDir = strCurDir.Left(nIndex); //end with "\\"

	m_CurrentEXEPath=strCurDir;
	g_ConnectJBossIsOK=FALSE;

	CMarkup m_XmlDOM;   
	//////////////////////////////////////////////////////
//	Clog( LOG_DEBUG, _T("Load File: dca.xml"));
	strCurDir=strCurDir+"\\"+"dca.xml";
	m_XmlDOM.Load(strCurDir);

	m_XmlDOM.ResetPos();

	if(!m_XmlDOM.FindElem("DCAConfiguration"))
	{
		Clog( LOG_DEBUG,_T("dca.xml error!  CDODClientAgent::Parse() - !FindElem(DCAConfiguration).") );
		return;
	}

	m_XmlDOM.IntoElem();

	// Do parse stuff
	if( !m_XmlDOM.FindElem( "DODJBOSS" ) )
	{
		Clog( LOG_DEBUG,_T("dca.xml error! - CDODClientAgent::Parse() - !FindElem(DODJBOSS).") );
		return;
	}
	//PPsortInfo xxxinfo;
	CString sTmp = _T("");
	sTmp = m_XmlDOM.GetAttrib("IPAddress");
	//_tcscpy( xxxinfo.cAgentServerIp, sTmp.GetBuffer( 0 ) );	
	int nport= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPort" ));

	CString m_ProviderValue;
	
	m_ProviderValue.Format(":%d",nport);

	m_ProviderValue=sTmp+m_ProviderValue;

	m_Configqueue = m_XmlDOM.GetAttrib("ConfigQueueName");

	if( !m_XmlDOM.FindElem( "SRMLocation" ) )
	{
		Clog( LOG_DEBUG,_T("dca.xml error! - CDODClientAgent::Parse() - !FindElem(SRMLocation).") );
		return;
	}
	sTmp = m_XmlDOM.GetAttrib("IPAddress");
	_tcscpy( m_portInfoSRM.cSrmtIp, sTmp.GetBuffer( 0 ) );
	
	m_portInfoSRM.wSrmPort = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPort" ));

	//	Clog( LOG_DEBUG, _T("Load File: %d"),m_portInfoSRM.wAgentServerPort);


	if( !m_XmlDOM.FindElem( "UpdateInterval" ) )
	{
		Clog( LOG_DEBUG,_T("dca.xml error! - CDODClientAgent::Parse() - !FindElem(UpdateInterval).") );
		return;
	}
	sTmp = m_XmlDOM.GetAttrib("UpdateInterval");

	int Tinterval=0;
	if (sTmp.GetLength() >0)
	{
		Tinterval=_ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "UpdateInterval" ));
	}
	else
		Tinterval=Time_interval;

	Clog( LOG_DEBUG, _T("Load config:UpdateInterval is %d"),Tinterval);
//	Tinterval=Tinterval * 1000;
	m_pPortManager=new CPortManager(m_CurrentEXEPath,Tinterval); 
	//block call developeKit's connecttion SRM opertion.
	//m_pPortManager->m_nUpdateInterval=Tinterval;

	while (1)
	{		
		if(g_bStop)
			return ;

		if(m_pPortManager->m_kit->setSRMAdd(m_portInfoSRM.cSrmtIp,m_portInfoSRM.wSrmPort)==1)
		{
			Clog( LOG_DEBUG,_T("DODClientAgent connects DevKit failure.next connection will start(%s : %d)"),m_portInfoSRM.cSrmtIp,m_portInfoSRM.wSrmPort);
			Sleep(5);
		}
		else
			break;
	}

	Clog( LOG_DEBUG,_T("CDODClientAgent::DevKit connect SRM successful (%s : %d)") ,m_portInfoSRM.cSrmtIp,m_portInfoSRM.wSrmPort);

// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月20日 10:45:19
//by the latest request,configuration can get from configuratin module or local config_file.
//Add on-off operation option in DCA.xml

	int nUseJBoss=1;
	if(m_XmlDOM.FindElem( "RunMode" ) )
	{
		sTmp = m_XmlDOM.GetAttrib("LocalConfig");

		if (sTmp.CompareNoCase("1")==0)
		{
			nUseJBoss=0;
		}
	}
	// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月20日 10:56:39
	// if Local config is 0 ,denote normal running .
	if(nUseJBoss)
	{
		while (1)
		{
			if(g_bStop)
				return ;
			//char aa[30]="192.168.80.96:1099";
			m_jms=new CJMS(m_ProviderValue.GetBuffer(0));
			if(m_jms->StartConnect()==TRUE)
			{
				Clog( LOG_DEBUG,_T("DCA Connect JBOSS successful.(%s)"),m_ProviderValue);
				g_ConnectJBossIsOK = TRUE;
				break;
			}
			else
			{
				Clog( LOG_DEBUG,_T("Connect JBOSS error.(%s)"),m_ProviderValue );
				Sleep(5);
// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月23日 11:24:14
//Add delete m_JMS
				delete m_jms;
				m_jms=NULL;
			}
		}
		m_Parse=new  CJMSParser(m_jms,this);
		m_Parse->m_pm =m_pPortManager;
	//	m_Configqueue="queue/queue_channel1";
		
		if(m_QueueManagement.QueueIsExist(m_Configqueue,INT_ONLY_SEND,TRUE)==FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,m_Configqueue.GetBuffer(0),INT_ONLY_SEND,m_Parse)==FALSE)
			{
				Clog( LOG_DEBUG,_T("m_jms.AddOneSendQueue (%s) error"), m_Configqueue);
				return;
			}
			Clog( LOG_DEBUG,_T("The queue named (%s) is registed in jboss \n") ,m_Configqueue);
		}
		else
			Clog( LOG_DEBUG,_T("The queue named (%s) has existed in jboss") ,m_Configqueue);


		CJMSTxMessage tempMsg;
		while (1)
		{
			if(g_bStop)
				return ;
			CString ssaa=m_Parse->SendGetConfig();

			if (m_jms->SyncSendMsg(TRUE,m_Configqueue.GetBuffer(0),ssaa.GetBuffer(0),tempMsg,7000,"MESSAGECODE",JMSMESSAGE_CONFIGURATION,"MESSAGECLASS","COMMAND")==FALSE)
			{
				Clog( LOG_DEBUG,_T("Send config_msg to queue(%s) by sync ,but the config_module has no response"),m_Configqueue);
				Sleep(5);


				CMarkup m_XmlDOM;
				Clog( LOG_DEBUG, _T("Load local configuration file: HttpConfig.xml"));
				//	m_XmlDOM.Load( "DOD_JMS_response.xml" );

				strCurDir=m_CurrentEXEPath+"\\"+"HttpConfig.xml";

				if(m_XmlDOM.Load(strCurDir))
				{
					CString sstr="";		
					sstr = m_XmlDOM.GetDoc();
					m_Parse->ReceiverPortConfigMsg(sstr.GetBuffer(0),0,NULL);

					AddChannelQueue();
					Clog( LOG_DEBUG, _T("AddChannelQueue successful!"));
					return ;
				}
				else
				{
					Clog( LOG_DEBUG, _T("Load : HttpConfig.xml failure ,continue send command message to config_module"));
				}
			}
			else
				break;
		}
		m_Parse->parseNotification(&tempMsg);

	//	Clog( LOG_DEBUG,_T("CDODClientAgent::SyncSend config messagene") );
	}
	else
	{
		m_Parse=new  CJMSParser(NULL,NULL);
		m_Parse->m_pm =m_pPortManager;

		CMarkup m_XmlDOM;
		Clog( LOG_DEBUG, _T("Load local configuration file: Localconfig.xml"));
		//	m_XmlDOM.Load( "DOD_JMS_response.xml" );

		strCurDir=m_CurrentEXEPath+"\\"+"Localconfig.xml";

		m_XmlDOM.Load(strCurDir);
		CString sstr="";		
		sstr = m_XmlDOM.GetDoc();
		//	TRACE("sdfsdf==%d\n",sstr.GetLength());
		//	str.Format("\n%d",sstr.GetLength()); 	
		//	TRACE(str);	
		//	TRACE(sstr.GetBuffer(0)); 

		m_Parse->ReceiverPortConfigMsg(sstr.GetBuffer(0),0,NULL);
		if(m_pPortManager->ApplyParameter() !=0)
		{
			Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage() Apply some Parameters of config error ") );
			return ;
		}
	}
}

BOOL CDODClientAgent::AddChannelQueue()
{
	if(1>(int)(m_pPortManager->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("DCA found current PortVector.size is zero, parse config content error") );
		return FALSE;
	}
	
	CString sTmp;

	sTmp="queue/"+m_pPortManager->m_Localqueuename;
	if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_RECEIVE,TRUE)==FALSE)
	{
		if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_RECEIVE,m_Parse)==FALSE)
		{
			Clog( LOG_DEBUG,_T("CDODClientAgent:: AddOneReceiveQueue(queuename= (%s) error."),m_pPortManager->m_Localqueuename);
			return FALSE;
		}
		Clog( LOG_DEBUG,_T("The queue named (%s) is registed in jboss") ,m_pPortManager->m_Localqueuename);
	}
	else
		Clog( LOG_DEBUG,_T("The queue named (%s) has existed in jboss") ,m_pPortManager->m_Localqueuename);

	Sleep(2000);

	CPORTVECTOR::iterator iter;

	for(iter=m_pPortManager->m_PortVector.begin();iter!=m_pPortManager->m_PortVector.end();iter++)
	{

		CDODPort *pPort=*iter;

		if(pPort==NULL)
		{
			return FALSE;
		}

		for(int i=0;i<pPort->m_ChannelNumber;i++)
		{
			CDODChannel *channel=pPort->m_channel[i];
			if(channel==NULL)
			{
				return FALSE;
			}	
			
			// ------------------------------------------------------ Modified by zhenan_ji at 2005年8月19日 11:35:16

			if (channel->m_QueueName.GetLength()==0)
			{
				Clog( LOG_DEBUG,_T("CDODClientAgent:: channel->m_QueueName.GetLength()==0,portname=%s"),pPort->m_sPortName);
				continue;
			}

			// ------------------------------------------------------ Modified by zhenan_ji at 2005年9月22日 10:46:37
			if (channel->m_nType ==NO_USE_JMS)
				continue;

			sTmp="queue/"+channel->m_QueueName;
			if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_SEND,TRUE)==FALSE)
			{
				if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_SEND,m_Parse)==FALSE)
				{
					Clog( LOG_DEBUG,_T("[%s-%s]:: AddOneSendQueue(queuename=%s) error."),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
					return FALSE;
				}
				Clog( LOG_DEBUG,_T("[%s-%s]::AdSendQueue ok,queuename=%s"),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
			}
			else
				Clog( LOG_DEBUG,_T("[%s-%s]::queuename=%s is exist"),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
		}
	}

	SetMessage();

	if(0<(int)(m_pPortManager->m_PortVector.size()))
	{
		m_pPortManager->RunUpdateThread();
	} 
	return TRUE;
}


void CDODClientAgent::Stop()
{	
	Clog( LOG_DEBUG,_T("CDODClientAgent Stop ") );

	if (m_pPortManager)
	{
		if(m_pPortManager)
		{
			m_pPortManager->Stop();
		}
	}
}

void CDODClientAgent::UnInitialize()
{

	Clog( LOG_DEBUG,_T("CDODClientAgent UnInitialize ") );

	if(m_pPortManager)
	{
		if (m_pPortManager->m_kit)
		{
			Clog( LOG_DEBUG,_T("CDODClientAgent m_kit->DisConnectSRM before ") );
			m_pPortManager->m_kit->DisConnectSRM();
			Clog( LOG_DEBUG,_T("CDODClientAgent m_kit->DisConnectSRM after ") );
			Sleep(2);
		}
		delete m_pPortManager;
		m_pPortManager=NULL;
	}

	if(m_Parse)
	{
		delete m_Parse;
		m_Parse=NULL;
	}

	if(m_jms)
	{
		delete m_jms;
		m_jms=NULL;
	}
	Clog( LOG_DEBUG,_T("CDODClientAgent UnInitialize end") );
	::CoUninitialize();
}
void CDODClientAgent::Updatecatelog()
{
	Clog( LOG_DEBUG,_T("CDODClientAgent Updatecatelog ") );

	m_pPortManager->UpdateCatalog();	
}

int  CDODClientAgent::EnableChannel(BOOL bEnable)
{
	return 	m_pPortManager->EnableChannel(bEnable);
}

CString CDODClientAgent::GetPort()
{
	return m_pPortManager->GetPort();
}
void CDODClientAgent::ClosePort()
{
	Clog( LOG_DEBUG,_T("CDODClientAgent ClosePort ") );

	m_pPortManager->ClosePort();
}

int CDODClientAgent::GetState(int index)
{
	return m_pPortManager->GetState(index);
}


void CDODClientAgent::SetMessage()
{
	//
	//TCHAR szRemote[MAX_PATH] = _T("\\\\");
	//_tcscat( szRemote, "192.168.80.142" );
	//
	//NETRESOURCE tmpNetRes;
	//tmpNetRes.dwType = RESOURCETYPE_ANY;
	//tmpNetRes.lpLocalName = NULL;
	//tmpNetRes.lpRemoteName = szRemote;
	//tmpNetRes.lpProvider = NULL;
	//
	//CString m_sUserName="administrator";
	//CString m_sPasswd="cdci";
	//if(0)
	////if( WNetAddConnection2( &tmpNetRes, m_sUserName, m_sPasswd, CONNECT_UPDATE_PROFILE ) != NO_ERROR )
	//{
	//	Clog( LOG_DEBUG, _T("Auto logon the remote server... IP Address: 192.168.80.81"));
	//	return ;
	//}
	//
	
//	CJMSParser pp;



	CString strCurDir,sTmp;
	if(1>(int)(m_pPortManager->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("DCA found current PortVector.size is zero") );
		return;
	} 

	
 //m_logFileLocation=strCurDir + "TaskEngine.log";
//	strCurDir="D:\\work\\DODDevKit";
	
	if(m_pPortManager->ApplyParameter() !=0)
	{
		Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage() Apply some Parameters of config error ") );
		return ;
	}

	if(g_bStop)
		return  ;

	m_Parse->m_bStartReceive = TRUE;

	CPORTVECTOR::iterator iter;

	for(iter=m_pPortManager->m_PortVector.begin();iter!=m_pPortManager->m_PortVector.end();iter++)
	{

		CDODPort *pPort=*iter;

		if(pPort==NULL)
		{
			return ;
		}

		if(g_bStop)
			return ;
		for(int i=0;i<pPort->m_ChannelNumber;i++)
		{
			if(g_bStop)
				return;

			CDODChannel *channel=pPort->m_channel[i];
			if(channel==NULL)
			{
				return ;
			}	

// ------------------------------------------------------ Modified by zhenan_ji at 2006年3月7日 10:13:48
			if (channel->m_nType ==NO_USE_JMS || channel->m_nType ==DATAEXCHANGETYPE_MESSAGE_FORMAT)
				continue;

			if (channel->m_QueueName.GetLength()==0)
			{
				Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage() channel->m_QueueName.GetLength()==0 portname=%s"),pPort->m_sPortName);
				continue;
			}

			if (m_pPortManager->m_sDataTypeInitial.GetLength()>0)
			{
				int nIndexDataType=m_pPortManager->m_sDataTypeInitial.Find(channel->m_sDataType);
		
				Clog( LOG_DEBUG,_T("m_pPortManager->m_sDataTypeInitial.Find m_sDataTypeInitial is:%s : channel->m_sDataType"),m_pPortManager->m_sDataTypeInitial,channel->m_sDataType );

				if( nIndexDataType>=0)
				{	
					strCurDir=m_Parse->GetDataTypeInitialMsg(channel->m_sDataType,1);

					CJMSTxMessage tempMsg;
					sTmp="queue/"+channel->m_QueueName;
					if (m_jms->SyncSendMsg(TRUE,sTmp.GetBuffer(0),strCurDir.GetBuffer(0),tempMsg,5000,"MESSAGECODE",FIRSTDATATYPEMESSAGE,"MESSAGECLASS","COMMAND")==FALSE)
					{
						Clog( LOG_DEBUG,_T("[%s-%s] SyncSendMsg :send Data Type Initial msg to queue(%s) ,error"),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
						Sleep(5);
					}
					else
					{
						Clog( LOG_DEBUG,_T("[%s-%s] SyncSendMsg :send Data Type Initial msg to queue(%s) ok"),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
						
						int nLenDataType=channel->m_sDataType.GetLength();
						m_pPortManager->m_sDataTypeInitial.Delete(nIndexDataType,nLenDataType);
						//Clog( LOG_DEBUG,_T("current config DataTypeInitial is %s "),m_pPortManager->m_sDataTypeInitial);
					}
				}
			}
			strCurDir=m_Parse->SendGetFullDateMsg(pPort->m_nGroupID,channel->m_sDataType,0);

			sTmp="queue/"+channel->m_QueueName;
			if (m_jms->SendMsg(TRUE,sTmp.GetBuffer(0),strCurDir.GetBuffer(0),"MESSAGECODE",channel->m_nMessageType,"MESSAGECLASS","NOTIFICATION")==FALSE)
			{
				Clog( LOG_DEBUG,_T("[%s-%s] PtopSend( SendGetFullDateMsg,(%s) error "),pPort->m_sPortName,channel->m_sChannelName,sTmp);
				return ;
			}

			Clog( LOG_DEBUG,_T("[%s-%s] PtopSend :send getfulldata msg to queue(%s) ok"),pPort->m_sPortName,channel->m_sChannelName,sTmp);

		}
	}


/*
	sTmp="queue/"+m_pPortManager->m_Localqueuename;
	if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_RECEIVE,m_Parse)==FALSE)
	{
		Clog( LOG_DEBUG,_T("CDODClientAgent:: AddOneReceiveQueue(sTmp.GetBuffer error.") );
		return ;
	}*/
}