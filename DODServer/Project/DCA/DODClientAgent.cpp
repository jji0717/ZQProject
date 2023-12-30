// DODClineAgent.cpp: implementation of the CDODClientAgent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DODClientAgent.h"
#include "clog.h"
#include "Markup.h"
#include "FileFindExt.h"
#include "messagemacro.h"
#include ".\dodclientagent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern  BOOL	g_bStop;
extern  BOOL  g_ConnectJBossIsOK;
#define Time_interval	5
CDODClientAgent::CDODClientAgent()
{
	m_jms = NULL;
	m_ParsePortConfig = NULL;
	m_VecParser.clear();
	m_bReSetMsgListener = FALSE;
	m_Pjmsprocthread = NULL;
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
	m_ParsePortConfig=NULL;
	m_VecParser.clear();
	m_pPortManager=NULL;

	m_Pjmsprocthread = new CJmsProcThread();

	if(NULL == m_Pjmsprocthread )
	{
		Clog( LOG_DEBUG,_T("DCA Create JmsProcThread (m_Pjmsprocthread) error"));
		Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		return FALSE;
	}
	if(!m_Pjmsprocthread->init())
	{
		Clog( LOG_DEBUG,_T("DCA Initialize JmsProcThread::inti()  error"));
		Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		return FALSE;
	}
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

	strCurDir = strCurDir+"\\"+"dca.xml";
	m_XmlDOM.Load(strCurDir);

	m_XmlDOM.ResetPos();

	if(!m_XmlDOM.FindElem("DCAConfiguration"))
	{
		Clog( LOG_DEBUG,_T("dca.xml error!  CDODClientAgent::Parse() - !FindElem(DCAConfiguration).") );
		Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		return;
	}

	m_XmlDOM.IntoElem();

	// Do parse stuff
	if( !m_XmlDOM.FindElem( "DODJBOSS" ) )
	{
		Clog( LOG_DEBUG,_T("dca.xml error! - CDODClientAgent::Parse() - !FindElem(DODJBOSS).") );
	    Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		return;
	}
	CString sTmp = _T("");
	sTmp = m_XmlDOM.GetAttrib("IPAddress");
	int nport= _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPort" ));

	m_ProviderValue.Format(":%d",nport);

	m_ProviderValue=sTmp+m_ProviderValue;

	m_Configqueue = m_XmlDOM.GetAttrib("ConfigQueueName");

	if( !m_XmlDOM.FindElem( "SRMLocation" ) )
	{
		Clog( LOG_DEBUG,_T("dca.xml error! - CDODClientAgent::Parse() - !FindElem(SRMLocation).") );
        Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		return;
	}
	sTmp = m_XmlDOM.GetAttrib("IPAddress");
	char cSrmtIp[16];

	cSrmtIp[0] = '\0';

	if (sTmp.GetLength() < 16)
		strcpy(cSrmtIp,sTmp.GetBuffer(0));
	else
	{
		Clog( LOG_DEBUG,_T("dca.xml error! - SRM IPAddress %s error."),sTmp);
		Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		return;
	}
	
	int wSrmPort = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "IPPort" ));
    
	if( !m_XmlDOM.FindElem( "UpdateInterval" ) )
	{
		Clog( LOG_DEBUG,_T("dca.xml error! - CDODClientAgent::Parse() - !FindElem(UpdateInterval).") );
	    Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		return;
	}
	sTmp = m_XmlDOM.GetAttrib("UpdateInterval");

	int Tinterval = 0;
	if (sTmp.GetLength() >0)
	{
		Tinterval = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "UpdateInterval" ));
	}
	else
		Tinterval = Time_interval;

	Clog( LOG_DEBUG, _T("Load config:UpdateInterval is %d"),Tinterval);

	m_pPortManager=new CPortManager(m_CurrentEXEPath,Tinterval);

    if(!m_pPortManager)
	{
      Clog( LOG_DEBUG,_T("DCA Initialize CPortManager(m_pPortManager)  error"));
	   Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
      return;
	}

	BOOL IsSettingIPPort = TRUE;
	while (1)
	{		
		if(g_bStop)
			return ;

		if(m_pPortManager->m_kit->setSRMAdd(cSrmtIp,wSrmPort,IsSettingIPPort)==1)
		{
			Clog( LOG_DEBUG,_T("DODClientAgent connects SRM failure.(%s : %d)"),cSrmtIp,wSrmPort);
			Sleep(5);
		}
		else
			break;
		IsSettingIPPort = FALSE;
	}

	Clog( LOG_DEBUG,_T("CDODClientAgent::DevKit connect SRM successful (%s : %d)") ,cSrmtIp,wSrmPort);

// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月20日 10:45:19
//by the latest request,configuration can get from configuratin module or local config_file.
//Add on-off operation option in DCA.xml

	int nUseJBoss = 1;
	if(m_XmlDOM.FindElem( "RunMode" ))
	{
//		Clog( LOG_DEBUG,_T("CDODClientAgent::Read RunMode:"));
	   sTmp = m_XmlDOM.GetAttrib("LocalConfig");

		if (sTmp.CompareNoCase("1") == 0)
		{
			nUseJBoss = 0;
		}
	}
	Clog( LOG_DEBUG,_T("CDODClientAgent::RunMode is %s !"),sTmp);

 
	if(!m_XmlDOM.FindElem( "ConfigMsgTimeOut" ))
	{
	  Clog( LOG_DEBUG,_T("dca.xml error! - CDODClientAgent::Parse() - !FindElem(ConfigMsgTimeOut).") );
	  Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
	  return;
	}
    m_nConfigMsgTimeOut = _ttoi((LPCTSTR)m_XmlDOM.GetAttrib( "time" ));

	Clog( LOG_DEBUG,_T("CDODClientAgent::Read ConfigMsgTimeOut is %d !"),m_nConfigMsgTimeOut);
	// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月20日 10:56:39
	// if Local config is 0 ,denote normal running .
	if(nUseJBoss)
	{

	   m_strPortConfigFileBak = m_CurrentEXEPath + "\\" + "LocalConfigBak.txt";
	   Clog( LOG_DEBUG,_T("Read Port Configuration By JBoss!"));
	   Clog( LOG_DEBUG,_T("CDODClientAgent::Begin to Connection JBoss"));

	  if(!m_jms)
	  {		
		  m_jms = new CJMS(m_ProviderValue.GetBuffer(0));
	  }

	  if(NULL == m_jms )
	  {
		  Clog( LOG_DEBUG,_T("DCA Initialize CJMS (m_jms) error"));
		  Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		  return ;
	  }

	   if(!ConnectionJBoss())
	   {
		  Clog( LOG_DEBUG,_T("DCA Connect JBOSS error.(%s)"),m_ProviderValue);
		  Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
		  return ;
	   }


		m_ParsePortConfig = new  CReceiveJmsMsg(this);

		if(!m_ParsePortConfig)
		{
			Clog( LOG_DEBUG,_T("DCA Initialize CJMSParser(m_ParsePortConfig)  error"));
			Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
			return ;
		}
		m_ParsePortConfig->m_pm = m_pPortManager;

	//	m_Configqueue="queue/queue_channel1";

		if(m_QueueManagement.QueueIsExist(m_Configqueue,INT_ONLY_SEND,TRUE)==FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,m_Configqueue.GetBuffer(0),INT_ONLY_SEND,NULL)==FALSE)
			{
				Clog( LOG_DEBUG,_T("m_jms.AddOneSendQueue (%s) error"), m_Configqueue);
				Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
				return ;
			}
			Clog( LOG_DEBUG,_T("The queue named (%s) is registed in JBoss \n") ,m_Configqueue);
		}
		else
			Clog( LOG_DEBUG,_T("The queue named (%s) has existed in JBoss") ,m_Configqueue);


		CJMSTxMessage tempMsg;
		while (1)
		{
			if(g_bStop)
				return  ;
			CString ssaa = m_ParsePortConfig->SendGetConfig();

			Clog( LOG_DEBUG,_T("DCA Send Get Port Configuration Message: (%s)") ,ssaa);

			if (m_jms->SyncSendMsg(TRUE,m_Configqueue.GetBuffer(0),ssaa.GetBuffer(0),tempMsg,m_nConfigMsgTimeOut,"MESSAGECODE",JMSMESSAGE_CONFIGURATION,"MESSAGECLASS","COMMAND")==FALSE)
			{
				Clog( LOG_DEBUG,_T("Send config_msg to queue(%s) by sync ,but the config_module has no response"),m_Configqueue);
				Sleep(5);
			}
			else
				break;
		}

		Clog( LOG_DEBUG,_T("DCA  Get Port Configuration successful!"));

		Clog( LOG_DEBUG,_T("DCA  Begin Parser Port Configuration ..."));

		m_ParsePortConfig->parseNotification(&tempMsg);

		Clog( LOG_DEBUG,_T("DCA  Parser Port Configuration End !"));

	}
	else
	{
		Clog( LOG_DEBUG,_T("Read Port Configuration at local!"));
		m_ParsePortConfig = new  CReceiveJmsMsg(this);
		m_ParsePortConfig->m_pm = m_pPortManager;

		CMarkup m_XmlDOM;
		Clog( LOG_DEBUG, _T("Load local configuration file: Localconfig.xml"));

		strCurDir = m_CurrentEXEPath + "\\" + "Localconfig.xml";

		m_XmlDOM.Load(strCurDir);
		CString sstr = "";		
		sstr = m_XmlDOM.GetDoc();

		m_ParsePortConfig->ReceiverPortConfigMsg(sstr.GetBuffer(0),0,NULL);
		if(m_pPortManager->ApplyParameter() != 0)
		{
			Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage() Apply some Parameters of config error ") );
		    Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .error") );
			return ;
		}
		Clog( LOG_DEBUG,_T("CDODClientAgent::GetAllChannelData(): ReLoad Msg Data Begin!") );

		GetAllChannelData();

		Clog( LOG_DEBUG,_T("CDODClientAgent::GetAllChannelData(): ReLoad Msg Data End!") );
	}

	Clog( LOG_DEBUG,_T("CDODClientAgent::Initialize .End") );
}

BOOL CDODClientAgent::AddChannelQueue()
{
	if(1 > (int)(m_pPortManager->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("DCA found current PortVector.size is zero, parse config content error") );
		Clog( LOG_DEBUG,_T("CDODClientAgent::AddChannelQueue(): AddChannelQueue Error!") );
		return FALSE;
	}

	CString sTmp;
	int i = 0;
	for(i = 0; i < m_VecParser.size(); i++)
	{     
		m_VecParser[i].MsgReceive = new CReceiveJmsMsg(this);
		
		if(!m_VecParser[i].MsgReceive)
		{
			Clog( LOG_DEBUG,_T("DCA Initialize CReceiveJmsMsg(m_VecParser[%d].MsgParser) Error."),i);
			Clog( LOG_DEBUG,_T("CDODClientAgent::AddChannelQueue(): AddChannelQueue Error!") );
			return FALSE;
		}

		m_VecParser[i].MsgReceive->m_pm = m_pPortManager;
		m_VecParser[i].MsgReceive->m_strQueueName = m_VecParser[i].QueueName;

		sTmp = "queue/" + m_VecParser[i].QueueName;
		if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_RECEIVE,TRUE) == FALSE)
		{
             if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_RECEIVE,m_VecParser[i].MsgReceive) == FALSE)
			{
				Clog( LOG_DEBUG,_T("CDODClientAgent:: AddOneReceiveQueue(queuename = (%s) error."),m_VecParser[i].QueueName);
				Clog( LOG_DEBUG,_T("CDODClientAgent::AddChannelQueue(): AddChannelQueue Error!") );
				return FALSE;
			}
            if(!m_VecParser[i].MsgReceive->init())
			{
				Clog( LOG_DEBUG,_T("CDODClientAgent::Create ReceiverMsg Thread error."),m_VecParser[i].QueueName);
				Clog( LOG_DEBUG,_T("CDODClientAgent::AddChannelQueue(): AddChannelQueue Error!") );
				return FALSE;
			}
			Clog( LOG_DEBUG,_T("The queue named (%s) is registed in JBoss") ,m_VecParser[i].QueueName);
		}
		else
			Clog( LOG_DEBUG,_T("The queue named (%s) has existed in JBoss") ,m_VecParser[i].QueueName);
	}

	Sleep(2000);

	CPORTVECTOR::iterator iter;

	for(iter = m_pPortManager->m_PortVector.begin(); iter != m_pPortManager->m_PortVector.end(); iter++)
	{

		CDODPort *pPort = *iter;

		if(pPort == NULL)
		{
			return FALSE;
		}

		for(int i = 0; i < pPort->m_ChannelNumber; i++)
		{
			CDODChannel *channel = pPort->m_channel[i];
			if(channel == NULL)
			{
				return FALSE;
			}	

			if (channel->m_QueueName.GetLength() == 0)
			{
				Clog( LOG_DEBUG,_T("CDODClientAgent:: channel->m_QueueName.GetLength() == 0,portname = %s"),pPort->m_sPortName);
				continue;
			}

			if (channel->m_nType == NO_USE_JMS)
				continue;

			sTmp = "queue/" + channel->m_QueueName;
			if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_SEND,TRUE) == FALSE)
			{
				if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_SEND,NULL) == FALSE)
				{
					Clog( LOG_DEBUG,_T("[%s-%s]:: AddOneSendQueue(queuename = %s) error."),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
					Clog( LOG_DEBUG,_T("CDODClientAgent::AddChannelQueue(): AddChannelQueue Error!") );
					return FALSE;
				}
				Clog( LOG_DEBUG,_T("[%s-%s]::AddSendQueue ok,queuename = %s"),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
			}
			else
				Clog( LOG_DEBUG,_T("[%s-%s]::queuename = %s is exist"),pPort->m_sPortName,channel->m_sChannelName,channel->m_QueueName);
		}
	}

	if(!SetMessage())
		return FALSE;

	if(0 < (int)(m_pPortManager->m_PortVector.size()))
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

	try
	{
		g_bStop = TRUE;

		SetEvent(CJmsProcThread::m_hReConnectJBoss);

		while(WaitForSingleObject(CJmsProcThread::m_hReConnectJBoss,
			0) ==  WAIT_OBJECT_0)			
		{
			Sleep(1000);
		}

		if(m_pPortManager)
		{
			if (m_pPortManager->m_kit)
			{
				m_pPortManager->m_kit->DisConnectSRM();
				Sleep(2);
			}
			delete m_pPortManager;
			m_pPortManager = NULL;
		}
		Clog( LOG_DEBUG,_T("Delete all port and channel ") );

		if(m_Pjmsprocthread)
		{
			delete m_Pjmsprocthread;
			m_Pjmsprocthread = NULL;
		}

		Clog( LOG_DEBUG,_T(" CJMSPortManager::Delete Jmsprocthread success!"));

		if(m_ParsePortConfig)
		{
			delete m_ParsePortConfig;
			m_ParsePortConfig = NULL;
		}

		::CoUninitialize();
	}
	catch (...)
	{
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);

		Clog( LOG_DEBUG,
			_T("CJMSPortManager::UnInitialize() GetLastError() =%d, \
			ErrorDescription = %s"),nError, strError);

		Clog( LOG_DEBUG,
			_T(" CJMSPortManager::UnInitialize() error!"));
         return;
	}

	Clog( LOG_DEBUG,_T("CDODClientAgent UnInitialize End") );
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

BOOL CDODClientAgent::SetMessage()
{	
	CString strCurDir,sTmp;
	if(1>(int)(m_pPortManager->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("DCA found current PortVector.size is zero") );
		Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage(): SetMessage Error!") );
		return FALSE;
	} 

	if(m_pPortManager->ApplyParameter() != 0)
	{
		Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage() Apply some Parameters of config error ") );
		Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage(): SetMessage Error!") );
		return FALSE;
	}

	if(g_bStop)
		return  FALSE;

	for(int i = 0; i <m_VecParser.size(); i++)
	{
		Clog( LOG_DEBUG,_T("[CJMSParser::QueueName = %s] m_hStartReceive = TRUE!"),m_VecParser[i].QueueName );
//		m_VecParser[i].MsgParser->m_bStartReceive = TRUE;
		SetEvent(m_VecParser[i].MsgReceive->m_hStartReceive);
	}

	CPORTVECTOR::iterator iter;

	for(iter = m_pPortManager->m_PortVector.begin();iter != m_pPortManager->m_PortVector.end(); iter++)
	{

		CDODPort *pPort = *iter;

		if(pPort == NULL)
		{
			return FALSE;
		}

		if(g_bStop)
			return FALSE;
		for(int i = 0;i < pPort->m_ChannelNumber; i++)
		{
			if(g_bStop)
				return FALSE;

			CDODChannel *channel = pPort->m_channel[i];
			if(channel == NULL)
			{
				return FALSE;
			}	

// ------------------------------------------------------ Modified by zhenan_ji at 2006年3月7日 10:13:48
			if (channel->m_nType == NO_USE_JMS || channel->m_nType == DATAEXCHANGETYPE_MESSAGE_FORMAT)
				continue;

			if (channel->m_QueueName.GetLength() == 0)
			{
				Clog( LOG_DEBUG,_T("CDODClientAgent::SetMessage() channel->m_QueueName.GetLength()==0 portname=%s"),pPort->m_sPortName);
				continue;
			}

			if (m_pPortManager->m_sDataTypeInitial.GetLength()>0)
			{
				int nIndexDataType = m_pPortManager->m_sDataTypeInitial.Find(channel->m_sDataType);
		
				Clog( LOG_DEBUG,_T("m_pPortManager->m_sDataTypeInitial.Find m_sDataTypeInitial is:%s : channel->m_sDataType"),m_pPortManager->m_sDataTypeInitial,channel->m_sDataType );

				if( nIndexDataType >= 0)
				{	
					strCurDir = m_ParsePortConfig->GetDataTypeInitialMsg(channel->m_sDataType,1);

					CJMSTxMessage tempMsg;
					sTmp = "queue/" + channel->m_QueueName;
					if (m_jms->SyncSendMsg(TRUE,sTmp.GetBuffer(0),strCurDir.GetBuffer(0),tempMsg,5000,"MESSAGECODE",FIRSTDATATYPEMESSAGE,"MESSAGECLASS","COMMAND")==FALSE)
					{
						Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] SyncSendMsg :send Data Type Initial msg to queue(%s) ,error"),pPort->m_sPortName,pPort->m_nID,channel->m_sChannelName,channel->m_channelID,channel->m_QueueName);
						Sleep(5);
					}
					else
					{
						Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] SyncSendMsg :send Data Type Initial msg to queue(%s) ok"),pPort->m_sPortName,pPort->m_nID,channel->m_sChannelName,channel->m_channelID,channel->m_QueueName);
						
						int nLenDataType=channel->m_sDataType.GetLength();
						m_pPortManager->m_sDataTypeInitial.Delete(nIndexDataType,nLenDataType);
						//Clog( LOG_DEBUG,_T("current config DataTypeInitial is %s "),m_pPortManager->m_sDataTypeInitial);
					}
				}
			}
			strCurDir = m_ParsePortConfig->SendGetFullDateMsg(pPort->m_nGroupID,channel->m_sDataType,0);

			sTmp = "queue/" + channel->m_QueueName;
			if (m_jms->SendMsg(TRUE,sTmp.GetBuffer(0),strCurDir.GetBuffer(0),"MESSAGECODE",channel->m_nMessageType,"MESSAGECLASS","NOTIFICATION")==FALSE)
			{
				Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] PtopSend( SendGetFullDateMsg,(%s) error "),pPort->m_sPortName,pPort->m_nID,channel->m_sChannelName,channel->m_channelID,sTmp);
				return FALSE;
			}

			Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] PtopSend :send getfulldata msg to queue(%s) ok"),pPort->m_sPortName,pPort->m_nID,channel->m_sChannelName,channel->m_channelID,sTmp);
		}
	}
	return TRUE;
}
BOOL CDODClientAgent::ConnectionJBoss(void)
{	
	while (1)
	{
		if(g_bStop)
			return FALSE;
		int i = 0;
		//char aa[30]="192.168.80.49:1199";

		if(m_jms->StartConnect(this) == TRUE)
		{
			g_ConnectJBossIsOK = TRUE;

			if(m_bReSetMsgListener)
			{
				CJMSBaseList::iterator it = m_jms->m_QueueRecvList.begin();

				for(; it != m_jms->m_QueueRecvList.end(); ++it)
				{  
					if(m_jms->m_Connection.createSession((*it)->m_Session) == FALSE)
					{
						(*it)->m_bError = TRUE;
						Clog( LOG_ERROR,_T("ReConnect Jboss Error. m_Connection.createSession error! queuename = %s "),
							m_VecParser[i].QueueName);
						return FALSE ;
					}

					if(m_jms->m_JndiContext->createDestination((*it)->m_QueueName,(*it)->m_destination) == FALSE)
					{
						(*it)->m_bError = TRUE;
						Clog( LOG_ERROR,_T("ReConnect Jboss Error. createDestination error! queuename = %s "),
							m_VecParser[i].QueueName);
						return FALSE;
					}


					if((*it)->m_Session.createConsumer(&(*it)->m_destination,(*it)->m_consumer) == FALSE)
					{
						(*it)->m_bError = TRUE;
						Clog( LOG_DEBUG,_T(" ReConnect Jboss Error.createConsumer error! queuename = %s "),
							m_VecParser[i].QueueName);
						return FALSE;
					}

					m_VecParser[i].MsgReceive->m_Consumer = &(*it)->m_consumer;
					m_VecParser[i].MsgReceive->m_destination = &(*it)->m_destination;
					m_VecParser[i].MsgReceive->m_Session = &(*it)->m_Session;

					if (!(*it)->m_Session.textMessageCreate("", m_VecParser[i].MsgReceive->m_jmsTextMsg))
					{
						Clog( LOG_DEBUG,_T("  ReConnect Jboss Error.textMessageCreate error! queuename = %s "),
							m_VecParser[i].QueueName);
						return false;
					}

					Clog( LOG_DEBUG,_T("[%s]: Create Receive Consumer success!"),m_VecParser[i].QueueName);	 
					SetEvent(m_VecParser[i].MsgReceive->m_hStartReceive);
					i++;
				}
			}
			Clog( LOG_DEBUG,_T("DCA Connect JBOSS successful.(%s)"),m_ProviderValue);
			m_bReSetMsgListener = TRUE;
			break;
		}		
		else
		{
			Clog( LOG_DEBUG,_T("Connect JBOSS error.(%s)"),m_ProviderValue );
			Sleep(5000);
		}	
	}
	return TRUE;
}

BOOL CDODClientAgent:: GetAllChannelData()
{
	if(1 > (int)(m_pPortManager->m_PortVector.size()))
	{
		Clog( LOG_DEBUG,_T("DCA found current PortVector.size is zero, parse config content error") );
		Clog( LOG_DEBUG,_T("CDODClientAgent::GetAllChannelData(): GetAllChannelData Error!") );
		return FALSE;
	}
	CPORTVECTOR::iterator iter;

	for(iter = m_pPortManager->m_PortVector.begin(); iter != m_pPortManager->m_PortVector.end(); iter++)
	{

		CDODPort *pPort = *iter;

		if(pPort == NULL)
		{
			return FALSE;
		}

		for(int i = 0; i < pPort->m_ChannelNumber; i++)
		{
			CDODChannel *channel = pPort->m_channel[i];
			if(channel == NULL)
			{
				return FALSE;
			}	

			if(channel->m_nType == DATAEXCHANGETYPE_MESSAGE_FORMAT)
			{
				Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] Reload msg file!"),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName ,channel->m_channelID);

                ReLoadMsgChannelFile(channel);

				if(channel->m_MessageList.size() > 0)
					m_pPortManager->AddUpdateChannel(channel);
			}
			else
				if(channel->m_nType == NO_USE_JMS)
				{
                   continue;
				}
				else
				{
					Clog( LOG_DEBUG,_T("This Channel is a ShareFold Mode!"));

					Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::UpdateChannel will start!"),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName,channel->m_channelID);

					m_pPortManager->m_kit->UpdateCatalog(channel->m_nSessionID,channel->m_nPortID,channel->m_nStreamID);

					Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::Update operation end!"),channel->m_sPortName,channel->m_nPortID,channel->m_sChannelName,channel->m_channelID);		
				}
		}
	}
	return TRUE;
}

void CDODClientAgent::ReLoadMsgChannelFile(CDODChannel * pChannel)
{
	CString strDir = pChannel->m_strCachingDir +"\\*.*";
	int ncount = 0;
	CString strFileName,strMessageId;
	CString strDelTime, strCurrentTime;
	ZQCMessageInfoTINF info;
	COleDateTime oleDeletetime;
	CFileFind   finder;
	int npos;
	BOOL   bWorking  =  finder.FindFile(strDir);   
	while   (bWorking)   
	{   
		bWorking  = finder.FindNextFile(); 

		if (finder.IsDots() || finder.IsDirectory() )
			continue;

		strFileName = finder.GetFileName(); //FileName;
		npos = strFileName.Find('_');
        strMessageId = strFileName.Mid( npos + 1,strFileName.ReverseFind('_') - npos - 1);
		if(strFileName.Mid(17,3) != "DEL")
		{
			strFileName = pChannel->m_strCachingDir + "\\" + strFileName;
		    if(DeleteFile(strFileName) == FALSE)
			  Clog(LOG_DEBUG,_T("DeleteFile filename = %s error! "),strFileName );
			else
              Clog(LOG_DEBUG,_T("DeleteFile filename = %s success!"),strFileName );
		    continue;
		}

		strDelTime = strFileName.Mid(20,14); //File Delete Time; 
		strFileName = pChannel->m_strCachingDir + "\\" + strFileName;              
		strCurrentTime = (COleDateTime::GetCurrentTime()).Format("%Y%m%d%H%M%S");//Current Time;
        if(strDelTime != "00000000000000")
		{
			if( strDelTime <= strCurrentTime)
			{
				if(DeleteFile(strFileName) == FALSE)
					Clog(LOG_DEBUG,_T("DeleteFile filename = %s error! "),strFileName );
				else
					Clog(LOG_DEBUG,_T("DeleteFile filename = %s success!"),strFileName );
				continue;
			}
			info.m_fileName = strFileName;
			oleDeletetime.SetDateTime(atoi((LPCTSTR)strDelTime.Mid(0,4)),atoi((LPCTSTR)strDelTime.Mid(4,2)),atoi((LPCTSTR)strDelTime.Mid(6,2)),atoi((LPCTSTR)strDelTime.Mid(8,2)),atoi((LPCTSTR)strDelTime.Mid(10,2)),atoi((LPCTSTR)strDelTime.Mid(12,2)));	
			info.m_deleteTime = oleDeletetime;
			info.bForever = FALSE;
			info.sMessageID = strMessageId;
		}
		else
		{
			info.m_fileName = strFileName;
			info.bForever = TRUE;
			info.sMessageID = strMessageId;
		}
		pChannel->AddMsgList(&info);
	}   
}