// Dispatch.cpp 
//
#include "stdafx.h"
#include "messagemacro.h"
#include <stdlib.h>
#include <markup.h>
#include <algorithm>  

BOOL g_ConnectJBossIsOK;
BOOL g_bStop;
extern ZQ::common::Log* _logger;
#define CJMSPORTMANAGER "CJMSPortManager"
// This is the constructor of a class that has been exported.
// see JMSDispatch.h for the class definition
CJMSPortManager::CJMSPortManager(std::string datatunnelendpoint,Ice::CommunicatorPtr& ic)
 :m_strDataTunnelEndpoint(datatunnelendpoint), m_ic(ic)
{ 
	m_jms = NULL;
	m_ParsePortConfig = NULL;
	g_bStop = FALSE;
	g_ConnectJBossIsOK = FALSE;
	m_bReSetMsgListener = FALSE;
	m_channels.clear();
	m_portManager.clear();
	m_VecParser.clear();
	m_Pjmsprocthread = NULL;
	m_nUsingJBoss = 1;
	return; 
}
CJMSPortManager::~CJMSPortManager()
{
	
}
BOOL
CJMSPortManager::Create( std::string strJBossIPPort, std::string ConfigQueueName,
						int ConfigMsgTimeOut,std::string strCacheDir, int nUsingJBoss)
{
	ZQ::common::MutexGuard guard(_mutex);
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
		"Enter Create() "));

	m_strCacheDir = strCacheDir;
	m_ProviderValue = strJBossIPPort.c_str();
	m_strConfigQueueName = ConfigQueueName;
    m_nConfigTimeOut = ConfigMsgTimeOut;
	m_Pjmsprocthread = new CJmsProcThread();
	m_nUsingJBoss = nUsingJBoss;
	
	if(NULL == m_Pjmsprocthread )
	{
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
			"Create() JmsProcThread error"));
		return FALSE;
	}
	if(!m_Pjmsprocthread->init())
	{
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
			"Create() Initialize JmsProcThread::init() error"));
		return FALSE;
	}
	return TRUE;
}
void CJMSPortManager::stop()
{
    UnInitialize();
}
BOOL CJMSPortManager::Initialize()
{	
	ZQ::common::MutexGuard guard(_mutex);
	::CoInitialize(NULL);

	CString        strCurDir, strLocalConfig;
	char           sModuleName[1025];
	int            nUsingJBoss = 0;
	DWORD dSize = GetModuleFileName(NULL,sModuleName,1024);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.ReverseFind('\\');
	strCurDir = strCurDir.Left(nIndex); //end with "\\"
	nIndex = strCurDir.ReverseFind('\\');
	strCurDir = strCurDir.Left(nIndex); //end with "\\"
    strLocalConfig = strCurDir + "\\Etc\\LocalConfig.xml";
	
	if(m_nUsingJBoss)                             
	{
		(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
			"Initialize() begin connect to JBOSS service (%s)"),m_ProviderValue);
		
		m_jms = new CJMS(m_ProviderValue.GetBuffer(0));
		
		if(NULL == m_jms )
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"Initialize() fail to init JMS"));
			
			return FALSE;
		}
		
		if(!ConnectionJBoss())
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"Initialize() fail to connect to JBOSS service(%s)"),
				m_ProviderValue);
			(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
				"Initialize() fail to init jboss"));
			return FALSE;
		}
		
		m_ParsePortConfig = new  CReceiveJmsMsg(this);
		
		if(m_ParsePortConfig == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"Initialize()Create CReceiveJmsMsg Object error"));
			return FALSE;
		}
		m_ParsePortConfig->m_strQueueName = m_strConfigQueueName.c_str();
		if(m_QueueManagement.QueueIsExist(
			m_strConfigQueueName.c_str(),INT_ONLY_SEND,TRUE) == FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,(char *)m_strConfigQueueName.c_str(),			
				INT_ONLY_SEND,m_ParsePortConfig) == FALSE)
			{
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
					"Initialize()m_jms.AddOneSendQueue (%s) error"), 
					m_strConfigQueueName.c_str());
				(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
					"Initialize() fail to init JMS"));
				return FALSE;
			}
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"Initialize()The queue named (%s) is registed in JBoss"),
				m_strConfigQueueName.c_str());
		}
		else
		{
			(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()The queue named (%s) has existed in JBoss"),
				m_strConfigQueueName.c_str());
			
		}
		CJMSTxMessage tempMsg;
		while (1)
		{
			if(g_bStop)
				return  FALSE;
			CString ssaa = m_ParsePortConfig->SendGetConfig();
			
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"Initialize() send get port configuration Message: (%s)"),ssaa);
			
			if (m_jms->SyncSendMsg(TRUE,(char*)m_strConfigQueueName.c_str(),
				ssaa.GetBuffer(0),tempMsg,m_nConfigTimeOut,"MESSAGECODE",
				JMSMESSAGE_CONFIGURATION,"MESSAGECLASS","COMMAND")==FALSE)
			{
				(*_logger)(ZQ::common::Log::L_WARNING, CLOGFMT(CJMSPORTMANAGER,
				"Initialize() Send config_msg to queue(%s) by sync ,but the config_module has no response"),
				m_strConfigQueueName.c_str());				
				Sleep(50);
			}
			else
				break;
		}
		
		(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
			"Initialize()get port configuration successfully"));
		
		(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
			"Initialize()Begin parser port configuration ..."));
		
		if(m_ParsePortConfig->parseNotification(&tempMsg))
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
    
		ConventChannelName();

		if(!ConvertPortName())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}

		if(!AddChannelQueue())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		
		if(!CheckSyn())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!CreatePorts())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!CreateChannels())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!AttachChannel())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!RetryCreatPort())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!SetMessage())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
	}
	else
	{
		(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
			"Initialize() This is test mode"),m_ProviderValue);

		m_ParsePortConfig = new  CReceiveJmsMsg(this);
		
		if(m_ParsePortConfig == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to create CReceiveJmsMsg Object"));
			return FALSE;
		}
		m_ParsePortConfig->m_strQueueName = m_strConfigQueueName.c_str();
		
		CMarkup m_XmlDOM;
		
		m_XmlDOM.Load(strLocalConfig);
		CString sstr = "";		
		sstr = m_XmlDOM.GetDoc();
		
		if(!m_ParsePortConfig->ReceiverPortConfigMsg(sstr.GetBuffer(0),0,NULL))
			return FALSE;

		ConventChannelName();
		
		if(!ConvertPortName())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}

		if(!CheckSyn())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}

		if(!CreatePorts())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!CreateChannels())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!AttachChannel())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
		if(!RetryCreatPort())
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"Initialize()fail to parser port configuration"));
			return FALSE;
		}
	}

	(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
		"Initialize()parser port configuration successfully"));
	
	return TRUE;
}

BOOL CJMSPortManager::UnInitialize()
{
	try
	{
		g_bStop = TRUE;
		
		SetEvent(CJmsProcThread::m_hReConnectJBoss);
		
		while(WaitForSingleObject(CJmsProcThread::m_hReConnectJBoss,
			0) ==  WAIT_OBJECT_0)			
		{
			Sleep(1000);
		}
		
		if(m_Pjmsprocthread)
		{
			delete m_Pjmsprocthread;
			m_Pjmsprocthread = NULL;
		}
		
		if(m_ParsePortConfig)
		{
			delete m_ParsePortConfig;
			m_ParsePortConfig = NULL;
		}

		CHANNELMAP::iterator iter;
	
		for(iter = m_channels.begin(); iter != m_channels.end(); iter++)
		{	
			if(iter->second)
			{
				delete iter->second;
				iter->second = 0;
			}					
		}
		
		for(UINT i = 0 ; i < m_portManager.size(); i++)
		{
			DODPORT *pPort = m_portManager[i];
			CHANNELATTACHINFOMAP::iterator iterchannel;
			for(iterchannel = pPort->channelattachMap.begin();
			    iterchannel != pPort->channelattachMap.end(); 
				iterchannel++)
			{
				delete (*iterchannel).second;
				(*iterchannel).second = NULL;
			}
			
			for(UINT k = 0; k < pPort->ipportvector.size(); k ++)
			{
				delete pPort->ipportvector[k];
				pPort->ipportvector[k] = NULL;
			}
			
			delete m_portManager[i];
			m_portManager[i] =  NULL;
		}	
		
		::CoUninitialize();
	}
	catch (...)
	{
       	int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
			"UnInitialize() caught unknown exception (%d)(%s)"),nError, strError);
		return FALSE;
	}
	
	(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
		"UnInitialize() successfully"));
	
	return TRUE;
}
BOOL CJMSPortManager::ConnectionJBoss(void)
{
	ZQ::common::MutexGuard guard(_mutex);
	while (1)
	{
		if(g_bStop)
			return FALSE;
		int i = 0;
		
		if(m_jms->StartConnect() == TRUE)
		{
			g_ConnectJBossIsOK = TRUE;
			
			if(m_bReSetMsgListener)
			{
				CJMSBaseList::iterator iter = m_jms->m_QueueRecvList.begin();
				for(; iter != m_jms->m_QueueRecvList.end(); iter++)
				{
					if(m_jms->m_Connection.createSession((*iter)->m_Session) == FALSE)
					{
						(*iter)->m_bError = TRUE;
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
							"ConnectionJBoss() fail to create Jboss session, queuename (%s)"),
							m_VecParser[i].QueueName);
						return FALSE ;
					}
					
					if(m_jms->m_JndiContext->createDestination((*iter)->m_QueueName,
						(*iter)->m_destination) == FALSE)
					{
						(*iter)->m_bError = TRUE;
						(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
							"ConnectionJBoss()fail to create Jboss destination,queuename(%s)"),
							m_VecParser[i].QueueName);
						return FALSE;
					}
					
					
					if((*iter)->m_Session.createConsumer(&(*iter)->m_destination,
						(*iter)->m_consumer) == FALSE)
					{
						(*iter)->m_bError = TRUE;
						(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
							"ConnectionJBoss()fail to  create Jboss consumer,queuename (%s)"),
							m_VecParser[i].QueueName);
						return FALSE;
					}
					
					m_VecParser[i].MsgReceive->m_Consumer = &(*iter)->m_consumer;
					m_VecParser[i].MsgReceive->m_destination = &(*iter)->m_destination;
					m_VecParser[i].MsgReceive->m_Session = &(*iter)->m_Session;
					
					if (!(*iter)->m_Session.textMessageCreate("", 
						m_VecParser[i].MsgReceive->m_jmsTextMsg))
					{
						(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
							"ConnectionJBoss() fail to create Jboss textMessage,queuename (%s)"),
							m_VecParser[i].QueueName);
						return false;
					}
					
					(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
						"ConnectionJBoss() [%s] create receive consumer successfully"),
						m_VecParser[i].QueueName);	 
					i++;
				}				
			}
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"ConnectionJBoss() connect JBOSS successfully(%s)"),m_ProviderValue);
			m_bReSetMsgListener = TRUE;
			break;
		}		
		else
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"ConnectionJBoss()fail to connect JBOSS service(%s)"),m_ProviderValue);
			Sleep(5000);
		}	
	}
	return TRUE;
}

BOOL CJMSPortManager::AddChannelQueue()
{
	ZQ::common::MutexGuard guard(_mutex);
	if(1 > (int)(m_portManager.size()))
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
			"AddChannelQueue() no port configration info "));

		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
			"AddChannelQueue() fail to add channel to queue"));		
		return FALSE;
	}
	
	CString sTmp;
	UINT i = 0;
	for(i = 0; i < m_VecParser.size(); i++)
	{     
		m_VecParser[i].MsgReceive = new CReceiveJmsMsg(this);
		
		if(!m_VecParser[i].MsgReceive)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,  CLOGFMT(CJMSPORTMANAGER,
			"AddChannelQueue()fail to initialize CReceiveJmsMsg"));

			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"AddChannelQueue() fail to add channel to queue"));		
			return FALSE;
		}
		
		m_VecParser[i].MsgReceive->m_strQueueName = m_VecParser[i].QueueName;
		
		sTmp = "queue/" + m_VecParser[i].QueueName;
		if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_RECEIVE,TRUE) == FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_RECEIVE,
				m_VecParser[i].MsgReceive) == FALSE)
			{
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"AddChannelQueue() fail to add queuename(%s)"),m_VecParser[i].QueueName);
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
					"AddChannelQueue() fail to register queue"));
				return FALSE;
			}
			if(!m_VecParser[i].MsgReceive->init())
			{
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
					"AddChannelQueue() fail to create receiverMsg thread"),
					m_VecParser[i].QueueName);
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
					"AddChannelQueue() fail to register queue"));
				return FALSE;
			}
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"AddChannelQueue()The queue named (%s) is registed in JBoss") ,
				m_VecParser[i].QueueName);
		}
		else
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
			"AddChannelQueue() The queue named (%s) has existed in JBoss"),
			m_VecParser[i].QueueName);
	}
	
	Sleep(2000);
	
	CHANNELMAP::iterator iter;
	
	for(iter = m_channels.begin(); iter != m_channels.end(); iter++)
	{				
		if(g_bStop)
			return FALSE;
        CHANNELINFO* channel = iter->second;
		
		if(channel == NULL)
		{
			return FALSE;
		}	
		
		if (channel->strQueueName.size() == 0)
		{
			(*_logger)(ZQ::common::Log::L_INFO, 
			"AddChannelQueue() queue name is null");
			continue;
		}
		
		if (channel->DataExchangeType == ZQDataApp::dataLocalFolder)
			continue;
		
		sTmp.Format("queue/%s",channel->strQueueName.c_str());
		if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_SEND,TRUE) == FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),
				INT_ONLY_SEND,m_ParsePortConfig) == FALSE)
			{
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
					"AddChannelQueue()[channel: %s] fail to add One Send Queue (%s)"),
				iter->first.c_str(),channel->strQueueName.c_str());
				(*_logger)(ZQ::common::Log::L_ERROR,  CLOGFMT(CJMSPORTMANAGER,
					"AddChannelQueue() fail to register queue"));
				return FALSE;
			}
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
				"AddChannelQueue()[channel: %s]Add Send Queue (%s)success"),
				iter->first.c_str(),channel->strQueueName.c_str());
		}
		else
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
			"AddChannelQueue()[channel: %s]queuename (%s) is exist"),
			iter->first.c_str(),channel->strQueueName.c_str());			
	}
	
	return TRUE;
}
BOOL  CJMSPortManager::SetMessage()
{
	ZQ::common::MutexGuard guard(_mutex);
	CString strCurDir,sTmp;
	if(1 > (int)(m_portManager.size()))
	{
		(*_logger)(ZQ::common::Log::L_ERROR,  CLOGFMT(CJMSPORTMANAGER,
			"SetMessage()no Port configration info "));
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
			"SetMessage() fail to set message"));		
		return FALSE;
	}
	
	if(g_bStop)
		return FALSE;
	
	for(UINT i = 0; i <m_VecParser.size(); i++)
	{
		(*_logger)(ZQ::common::Log::L_DEBUG,  CLOGFMT(CJMSPORTMANAGER,
			"SetMessage()[CReceiveJmsMsg::QueueName = %s]set start service flag equal to TRUE"),
			m_VecParser[i].QueueName);
		SetEvent(m_VecParser[i].MsgReceive->m_hStartReceive);
	}
	
	CPORTVECTOR::iterator iterport;
	
	for(iterport = m_portManager.begin(); iterport != m_portManager.end(); iterport++)
	{		
		DODPORT *pPort = *iterport;
		
		if(pPort == NULL)
		{
			return FALSE;
		}
		
		if(g_bStop)
			return FALSE;
		
		CHANNELATTACHINFOMAP::iterator iterchannel;
		for(iterchannel = pPort->channelattachMap.begin();
			iterchannel != pPort->channelattachMap.end(); 
			iterchannel++)
		{
			if(g_bStop)
				return FALSE;
			
			CHANNELATTACHINFO *channelattachinfo = iterchannel->second;
				
			CHANNELMAP::iterator it = m_channels.find(channelattachinfo->ChannelName);
			if (it == m_channels.end())
				  continue;

			CHANNELINFO* channelinfo = it->second;

			if(channelinfo == NULL)
			{
				return FALSE;
			}	
			
			if (channelinfo->DataExchangeType == ZQDataApp::dataLocalFolder ||
				channelinfo->DataExchangeType == ZQDataApp::dataMessage)
				continue;
			
			if (channelinfo->strQueueName.size() == 0)
			{
				(*_logger)(ZQ::common::Log::L_DEBUG,  CLOGFMT(CJMSPORTMANAGER,
					"SetMessage()queue name is null, portname (%s)"),
					pPort->portname.c_str());
				continue;
			}
			
			if (m_sDataTypeInitial.GetLength() > 0)
			{
				int nIndexDataType = m_sDataTypeInitial.Find(
					channelinfo->strdataType.c_str());
				
				(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
					"SetMessage()DataTypeInitial(%s), channel dataType(%s)"),
					m_sDataTypeInitial,channelinfo->strdataType.c_str());
				
				if( nIndexDataType >= 0)
				{	
					strCurDir = m_ParsePortConfig->GetDataTypeInitialMsg(
						(char *)channelinfo->strdataType.c_str(),1);
					
					CJMSTxMessage tempMsg;
					sTmp.Format("queue/%s",channelinfo->strQueueName.c_str());
					if (m_jms->SyncSendMsg(TRUE,sTmp.GetBuffer(0),
						strCurDir.GetBuffer(0),tempMsg,5000,"MESSAGECODE",
						FIRSTDATATYPEMESSAGE,"MESSAGECLASS","COMMAND")==FALSE)
					{
						(*_logger)(ZQ::common::Log::L_WARNING, CLOGFMT(CJMSPORTMANAGER,
						"SetMessage()[%s : %s] fail to send Data Type Initial msg to queue(%s)"),
						pPort->portname.c_str(),
						it->first.c_str(),channelinfo->strQueueName.c_str());
						Sleep(5);
					}
					else
					{
						(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
						"SetMessage()[%s : %s] send Data Type Initial msg to queue(%s) ok"),
						pPort->portname.c_str(),
						it->first.c_str(),channelinfo->strQueueName.c_str());				
						int nLenDataType = channelinfo->strdataType.size();
						m_sDataTypeInitial.Delete(nIndexDataType,nLenDataType);
					}
				}
			}
			strCurDir = m_ParsePortConfig->SendGetFullDateMsg(pPort->groupId,
				(char *)channelinfo->strdataType.c_str(),0);
			
			sTmp.Format("queue/%s",channelinfo->strQueueName.c_str());
			
			if (m_jms->SendMsg(TRUE,sTmp.GetBuffer(0),strCurDir.GetBuffer(0),
				"MESSAGECODE",channelinfo->nMessageCode,"MESSAGECLASS",
				"NOTIFICATION")==FALSE)
			{
				(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
					"SetMessage()[%s : %s] fail to send get fulldata Msg (%s)"),
					pPort->portname.c_str(),it->first.c_str(),sTmp);
				return FALSE;
			}
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER, 
				"SetMessage()[%s : %s] Send get fulldata msg to queue(%s)ok"),
				pPort->portname.c_str(),it->first.c_str(),sTmp);
		}
	}
	
	return TRUE;
}

BOOL CJMSPortManager::CreateChannels()
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Application::DataOnDemand::DataPublishPointInfo dataPPinfo;
	TianShanIce::Application::DataOnDemand::DataPublishPointPrx  dataPPprx = NULL;
	TianShanIce::Application::DataOnDemand::DataPointPublisherPrx dataPrx;
	TianShanIce::Properties props;
	if(1 > (int)m_channels.size())
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
			"CreateChannels() channel count is '0'"));		
		return FALSE;
	}
	
	CHANNELMAP::iterator iterchannel;
	for(iterchannel = m_channels.begin(); iterchannel != m_channels.end(); 
	    iterchannel++)
	{
		CHANNELINFO *pChannelInfo = iterchannel->second;
		if(pChannelInfo == NULL)
		{
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
				"CreateChannels() fail to create channel, no channel info"));
			return FALSE;
		}
       
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
			"CreateChannels() channel name (%s)"),
			pChannelInfo->ChannelName.c_str());
		ParserChannelType(&dataPPinfo.dataTypes, pChannelInfo->strdataType);

		dataPPinfo.name = pChannelInfo->ChannelName;
		dataPPinfo.encrypt = pChannelInfo->nEncrypted;
		dataPPinfo.streamId = pChannelInfo->nstreamId;
		dataPPinfo.streamType = pChannelInfo->streamType;
		dataPPinfo.subchannelCount = pChannelInfo->nStreamCount;
		dataPPinfo.tag = ConvertTag(pChannelInfo->Tag);
		dataPPinfo.withDestination = pChannelInfo->nSendWithDestination;
		
		pChannelInfo->strCacheDir = m_strCacheDir + pChannelInfo->ChannelName;
		
		try
		{
			dataPrx = GetDataPointPublisherPrx();
			if(dataPrx == NULL)
			{
				(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
					"CreateChannels() fail to get datapointpublisher porxy"));
				return FALSE;
			}
			try
			{ 
				dataPPprx = dataPrx->openDataPublishPoint(pChannelInfo->ChannelName);
			}
			catch (const Ice::ObjectNotExistException)
			{
				dataPPprx = NULL;	
			}
			
			if(pChannelInfo->DataExchangeType == TianShanIce::Application::DataOnDemand::dataMessage)
			{
				if(dataPPprx == NULL)
				{
					dataPPprx = dataPrx->createMessageQueue
						(pChannelInfo->ChannelName,dataPPinfo,pChannelInfo->ChannelName);
					if(dataPPprx == NULL)
					{
						(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
						"CreateChannels() fail to create message channel(%s)"),
						pChannelInfo->ChannelName.c_str());
						return FALSE;
					}
					
					props = dataPPprx->getProperties();
					props["Path"] = pChannelInfo->strCacheDir;					
					dataPPprx->setProperties(props);

					(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
						"CreateChannels() create message channel(%s) successfully"),
						pChannelInfo->ChannelName.c_str());
				}
				else
				{					
                  	(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
						"CreateChannels() current channel (%s) already exist"),
						pChannelInfo->ChannelName.c_str());
				}
			}
			else
			{
				if(pChannelInfo->DataExchangeType == 
					TianShanIce::Application::DataOnDemand::dataLocalFolder)
				{
					pChannelInfo->strCacheDir = pChannelInfo->strQueueName;

					if (dataPPprx == NULL)
					{
						dataPPprx = dataPrx->createLocalFolderPublishPoint(
							pChannelInfo->ChannelName,dataPPinfo,
							pChannelInfo->strCacheDir,pChannelInfo->ChannelName);
						if(dataPPprx == NULL)
						{
							(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"CreateChannels()fail to create local folder channel(%s)"),
							pChannelInfo->ChannelName.c_str());
							return FALSE;
						} 	
						(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"CreateChannels()create local folder channel(%s) successfully"),
							pChannelInfo->ChannelName.c_str());
					}
					else
					{
						(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"CreateChannels()current channel (%s) already exist"),
							pChannelInfo->ChannelName.c_str());
					}
				}
				else
				{
					if (dataPPprx == NULL)
					{
                        dataPPprx = dataPrx->createShareFolderPublishPoint(
							pChannelInfo->ChannelName,dataPPinfo,pChannelInfo->ChannelName);	
						if(dataPPprx == NULL)
						{
							(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"CreateChannels()fail to create share folder channel(%s)"), 
							pChannelInfo->ChannelName.c_str());
							return FALSE;
						} 
						props = dataPPprx->getProperties();
						
						props["Path"] =  pChannelInfo->strCacheDir;
						
						dataPPprx->setProperties(props);

						(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"CreateChannels()create share folder channel(%s) successfully"),
							pChannelInfo->ChannelName.c_str());
					}
					else
					{
						(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"CreateChannels()current channel (%s) already exist"),
							pChannelInfo->ChannelName.c_str());
					}
				}
				
			}
		}  
		catch (const ::Ice::Exception & ex) 
		{
			(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
				"CreateChannels() caught ice exception (%s)"),
				ex.ice_name().c_str());
			return FALSE;
		} 
		
		if(pChannelInfo->DataExchangeType == TianShanIce::Application::DataOnDemand::dataLocalFolder)
		{
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"CreateChannels()This is a Local folder channel, cache directory (%s)"),
				pChannelInfo->strQueueName.c_str());
			continue;
		}
//		if(!DirectoryExist(pChannelInfo->strCacheDir.c_str()))
		if(_access(pChannelInfo->strCacheDir.c_str(),0))
		{
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"CreateChannels()[%s]current Channel path (%s) not exists"),
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str());
			
			if (CreateDirectory(pChannelInfo->strCacheDir.c_str(), NULL) == FALSE)		
			{
				int nError = GetLastError();
				char strError[500];
				
				GetErrorDescription(nError, strError);
				(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
					"CreateChannels()[%s]create Directory (%s) error (%d)(%s)"),
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str(),
				nError, strError);
				
				return FALSE;
			}
			(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"CreateChannels()[%s]create Directory (%s) successfully"),
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str());
		}
		else
		{
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
				"CreateChannels()[%s]current channel path (%s) already exist"),
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str());
		}
		
	}
	return TRUE;
}
BOOL CJMSPortManager::CreatePorts()
{
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
		"Enter create ports"));
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Application::DataOnDemand::DataStreamPrx datastreamprx = NULL;
	TianShanIce::Application::DataOnDemand::DataStreamInfo datastreaminfo;
	TianShanIce::Application::DataOnDemand::DataPointPublisherPrx dataPrx;
	DODFAILPORT failport;
	char stripport[30];
	
	if(1 > (int)(m_portManager.size()))
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
			"CreatePorts() no Port configration info"));
		return FALSE;
	}

	CPORTVECTOR::iterator iter;	
	for(iter = m_portManager.begin(); iter != m_portManager.end(); iter++)
	{
		DODPORT *pPort = (*iter);
		
		if(pPort == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"CreatePorts() no port information"));
			return FALSE;
		}
		IPPORTVECTOR::iterator iteripport;
		for(iteripport = pPort->ipportvector.begin(); 
			iteripport != pPort->ipportvector.end();
			iteripport++)
		{
			(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
				"CreatePorts() create port name(%s)"),
				(*iteripport)->IpPortName.c_str());
            memset(stripport,0, 30);
			
	/*		switch((*iteripport)->nSendType)
			{
			case 0: 
					sprintf(stripport,"TCP:%s:%d",
						(*iteripport)->strIp.c_str(), (*iteripport)->nPort);
					break;
			case 2:
					sprintf(stripport,"MULITCAST:%s:%d",
						(*iteripport)->strIp.c_str(), (*iteripport)->nPort);
					break;
			default:
					sprintf(stripport,"UDP:%s:%d",
						(*iteripport)->strIp.c_str(), (*iteripport)->nPort);
					break;
			}*/

			try
			{	
				dataPrx = GetDataPointPublisherPrx();
				
				if(dataPrx == NULL)
				{
					(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
						"fail to get datapointpublisher porxy"));
					return FALSE;
				}
				
				try
				{
					datastreamprx = dataPrx->openDataStream(
						                             (*iteripport)->IpPortName);
					(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
						"CreatePorts()current port (%s) is exist"),
						(*iteripport)->IpPortName.c_str());
				}	
				catch (Ice::ObjectNotExistException& )
				{	
					TianShanIce::SRM::ResourceMap resourceRequirement;
					TianShanIce::SRM::Resource res;
					TianShanIce::ValueMap vmap;
					TianShanIce::Variant var;
					TianShanIce::Properties props;

					var.bRange = false;
					var.ints.push_back((*iteripport)->nSendType);
					vmap["protocol"] = var;
					var.ints.clear();

					var.bRange = false;	
					var.strs.push_back((*iteripport)->strIp);
					vmap["destAddr"] = var;
					var.strs.clear();

					var.bRange = false;
					var.ints.push_back((*iteripport)->nPort);
					vmap["destPort"] = var;
					var.ints.clear();	
					res.resourceData = vmap;
					res.attr = TianShanIce::SRM::ResourceAttribute::raMandatoryNegotiable;
					res.status = TianShanIce::SRM::ResourceStatus::rsRequested;
					resourceRequirement.insert(TianShanIce::SRM::ResourceMap::value_type(TianShanIce::SRM::ResourceType::rtIP, res));
					vmap.clear();
					res.resourceData.clear();

					var.bRange = false;
					var.ints.push_back(pPort->groupId);
					vmap["id"] = var;
					var.ints.clear();
					res.resourceData = vmap;
					res.attr = TianShanIce::SRM::ResourceAttribute::raMandatoryNegotiable;
					res.status = TianShanIce::SRM::ResourceStatus::rsRequested;
					resourceRequirement.insert(TianShanIce::SRM::ResourceMap::value_type(TianShanIce::SRM::ResourceType::rtServiceGroup, res));
					vmap.clear();
					res.resourceData.clear();

					var.bRange = false;
					var.ints.push_back(pPort->totalBandWidth);
					vmap["bandwidth"] = var;
					var.ints.clear();
					res.resourceData = vmap;
					res.attr = TianShanIce::SRM::ResourceAttribute::raMandatoryNegotiable;
					res.status = TianShanIce::SRM::ResourceStatus::rsRequested;
					resourceRequirement.insert(TianShanIce::SRM::ResourceMap::value_type(TianShanIce::SRM::ResourceType::rtTsDownstreamBandwidth, res));
					vmap.clear();
					res.resourceData.clear();;

					var.bRange = false;
					var.ints.push_back(pPort->pmtPid);
					vmap["PmtPid"] = var;
					var.ints.clear();
					res.resourceData = vmap;
					res.attr = TianShanIce::SRM::ResourceAttribute::raMandatoryNegotiable;
					res.status = TianShanIce::SRM::ResourceStatus::rsRequested;
					resourceRequirement.insert(TianShanIce::SRM::ResourceMap::value_type(TianShanIce::SRM::ResourceType::rtMpegProgram, res));
					vmap.clear();
					res.resourceData.clear();

					datastreamprx = dataPrx->broadcast(
											(*iteripport)->IpPortName,resourceRequirement, props, (*iteripport)->IpPortName);

					if(datastreamprx == NULL)
					{
						(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
							"CreatePorts() fail to create datastream (%s)"), 
						(*iteripport)->IpPortName.c_str());

					/*	failport.IpPortName = (*iteripport)->IpPortName;
						failport.groupId = pPort->groupId;
						failport.destAddress = pPort->destAddress;
						failport.pmtPid = pPort->pmtPid;
						failport.totalBandWidth = pPort->totalBandWidth;
						failport.channelattachMap = pPort->channelattachMap;

						m_FailPort.push_back(failport);*/
						return FALSE;
					}
					(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
						"CreatePorts() create datastream (%s) successfully "),
						(*iteripport)->IpPortName.c_str());
				}
			}
			catch (const ::TianShanIce::InvalidParameter & ex) 
			{
				(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
					"CreatePorts() caught TianShanIce InvalidParameter exception(%s)"),
					ex.message.c_str());
				/*failport.IpPortName = (*iteripport)->IpPortName;
				failport.groupId = pPort->groupId;
				failport.destAddress = pPort->destAddress;
				failport.pmtPid = pPort->pmtPid;
				failport.totalBandWidth = pPort->totalBandWidth;
				failport.channelattachMap = pPort->channelattachMap;
				
				m_FailPort.push_back(failport);*/
				return FALSE;
			}  
			catch (const ::Ice::Exception & ex) 
			{
				(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
					"CreatePorts() caught ice exception(%s)"),
					ex.ice_name().c_str());
				/*failport.IpPortName = (*iteripport)->IpPortName;
				failport.groupId = pPort->groupId;
				failport.destAddress = pPort->destAddress;
				failport.pmtPid = pPort->pmtPid;
				failport.totalBandWidth = pPort->totalBandWidth;
				failport.channelattachMap = pPort->channelattachMap;
				
				m_FailPort.push_back(failport);*/
				return FALSE;
			} 			
		}
	}
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
		"Leave create ports"));
	return TRUE;
}

BOOL CJMSPortManager::AttachChannel()
{
	ZQ::common::MutexGuard guard(_mutex);
	TianShanIce::Application::DataOnDemand::DataStreamPrx datastreamprx = NULL;
	TianShanIce::Application::DataOnDemand::DataPointPublisherPrx dataPrx;
	
	CPORTVECTOR::iterator iter;	
	for(iter = m_portManager.begin(); iter != m_portManager.end(); iter++)
	{
		DODPORT *pPort = (*iter);
		
		if(pPort == NULL)
		{
			(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
				"AttachChannel()fail to get port info"));
			return FALSE;
		}
		IPPORTVECTOR::iterator iteripport;
		for(iteripport = pPort->ipportvector.begin(); 
			iteripport != pPort->ipportvector.end();
			iteripport++)
		{
			(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
				"AttachChannel() port name(%s)"),
				(*iteripport)->IpPortName.c_str());			
			
			dataPrx = GetDataPointPublisherPrx();
			
			if(dataPrx == NULL)
			{
				(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
					"AttachChannel() fail to get datapointpublisher porxy"));
				return FALSE;
			}
			
			try
			{
				datastreamprx = dataPrx->openDataStream((*iteripport)->IpPortName);
			}
			catch (Ice::ObjectNotExistException&)
			{
				(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
					"AttachChannel()fail to get datastream (%s) info,ice object not exist exception"), 
					(*iteripport)->IpPortName.c_str());
				return FALSE;
			}
			
			CHANNELATTACHINFOMAP::iterator iterchannelAttach;
			for(iterchannelAttach = pPort->channelattachMap.begin();
				iterchannelAttach != pPort->channelattachMap.end();
				iterchannelAttach++)
				{
					CHANNELATTACHINFO *channelattachinfo = 
						iterchannelAttach->second;
					try
					{	
						TianShanIce::Application::DataOnDemand::DataAttachInfo  attachinfo;
						attachinfo.dataPublishPointName = iterchannelAttach->first;
						attachinfo.minBitRate = channelattachinfo->nChannelRate;
						attachinfo.repeatTime = channelattachinfo->nRepeatetime;
						datastreamprx->attachDataPublishPoint(
							iterchannelAttach->first, attachinfo);

						(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
							"AttachChannel() [port: %s , channel: %s] attach channel successfully"),
							(*iteripport)->IpPortName.c_str(),iterchannelAttach->first.c_str());
					}
					catch (const TianShanIce::Application::DataOnDemand::ObjectExistException &e)
					{
						(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"AttachChannel() [port: %s , channel: %s] Object exist errorcode(%s)"),
							(*iteripport)->IpPortName.c_str(),iterchannelAttach->first.c_str(),e.ice_name().c_str());
					}
					catch (const TianShanIce::Application::DataOnDemand::StreamerException &e)
					{
						(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"AttachChannel()[port: %s , channel: %s] caught error at DataOnDemand::StreamerException (%s)"),
							(*iteripport)->IpPortName.c_str(),iterchannelAttach->first.c_str(),e.ice_name().c_str());
						return FALSE;
					}
					catch (const Ice::Exception &e)
					{
						(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
							"AttachChannel()[port: %s , channel: %s] caught error at ice exception(%s)"),
							(*iteripport)->IpPortName.c_str(),iterchannelAttach->first.c_str(),e.ice_name().c_str());
						return FALSE;
					}
										
					if(channelattachinfo->DataExchangeType ==
						TianShanIce::Application::DataOnDemand::dataSharedFolder )
					{
						char portcachefold[255];
						std::string channelCache = m_strCacheDir + iterchannelAttach->first;
						std::string portfolder;
				    	sprintf(portcachefold,"%s\\%d",
							channelCache.c_str(), pPort->groupId);

						portfolder = portcachefold;
//						if(!DirectoryExist(portfolder.c_str()))
						if(_access(portfolder.c_str(),0))
						{
							(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
								"AttachChannel() current channel (%s) path (%s)is not exist"),
								iterchannelAttach->first.c_str(),portfolder.c_str());
							
							if(CreateDirectory(portfolder.c_str(), NULL) == FALSE)		
							{
								int nError = GetLastError();
								char strError[500];
								
								GetErrorDescription(nError, strError);
								(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
									"AttachChannel() fail to create directory (%s) (%d)(%s)"),
									portfolder.c_str(),nError, strError);
								
								return FALSE;
							}
							(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
								"AttachChannel() create directory (%s) successfully"),
								portfolder.c_str());
						}
						else
						{
							(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(CJMSPORTMANAGER,
								"AttachChannel()current channel path (%s) already exist"),
								portfolder.c_str());
						}
						
					}					
				}
					
				try
				{
					datastreamprx->start();
					datastreamprx = NULL;
					(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
						"AttachChannel()(%s) start stream successfully"),
						(*iteripport)->IpPortName.c_str());
				}
				catch (TianShanIce::Application::DataOnDemand::StreamerException& ex)
				{
					(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
						"AttachChannel()(%s) fail to start stream ,caught Streamer Exception (%s)"),
						(*iteripport)->IpPortName.c_str(), ex.ice_name().c_str());
				}
				catch(TianShanIce::Application::DataOnDemand::StreamInvalidState& ex)
				{
					(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
						"AttachChannel()(%s) fail to start stream ,caught Stream InvalidState(%s)"),
						(*iteripport)->IpPortName.c_str(), ex.ice_name().c_str());
				}
				catch (Ice::Exception& ex)
				{
					(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
						"AttachChannel()(%s) fail to start stream ,caught ice exception(%s)"),
						(*iteripport)->IpPortName.c_str(), ex.ice_name().c_str());
				}
				catch (...)
				{
					int nError = GetLastError();
					char strError[500];
					
					GetErrorDescription(nError, strError);

					(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
						"AttachChannel()(%s) fail to start stream (%d)(%s)"),
						(*iteripport)->IpPortName.c_str(), nError, strError);
					return FALSE;
				}
			}
	}	
	return TRUE;

}
BOOL CJMSPortManager::DirectoryExist(LPCTSTR lpszpathame)
{
	TCHAR szWildpath[MAX_PATH];		//260
	_tcscpy( szWildpath, lpszpathame );
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile = FindFirstFile( szWildpath, &FindFileData );
	if( hFindFile == INVALID_HANDLE_VALUE )
	{
		//Clog(LOG_DEBUG,"find first file error path=%s errorcode=%d",
		//szWildpath,GetLastError());
		return FALSE;
	}
	
	int iCount = 0;
	//	TCHAR szTempPath[MAX_PATH];	
	do{
		if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )       //всд©б╪?
		{
			FindClose( hFindFile );
			return TRUE;
		}
	}while( FindNextFile( hFindFile, &FindFileData ) );
	FindClose( hFindFile );
	return FALSE;
}

BOOL CJMSPortManager::ParserChannelType(
						::TianShanIce::IValues *dataTypes, string strDataType)
{
	ZQ::common::MutexGuard guard(_mutex);
	char strtemp[30] = "\0";
	char seps[] = "; ";
	char *strtoken;
	
	strcpy(strtemp, strDataType.c_str()); 
	
   	dataTypes->clear();
	for( strtoken = strtok(strtemp,seps);strtoken != NULL; 
	strtoken = strtok(NULL,seps))
	{
		dataTypes->push_back(atoi(strtoken));
	}
	return TRUE;
}
int  CJMSPortManager::ConvertTag(string strTag)
{
	ZQ::common::MutexGuard guard(_mutex);
	int tag = 0;
	unsigned char strTemp;
	for(int i = 0; i<3; i++)
	{	    
		strTemp = strTag[i];
		tag |= strTemp; 
		tag = tag << 8;
	}
	return tag;
}
BOOL CJMSPortManager::CheckSyn()
{
	ZQ::common::MutexGuard guard(_mutex);
	(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
		"CheckSyn()Enter sync port and channel"));

	if(1 > (int)(m_portManager.size()))
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
			"CheckSyn()no Port configration info "));
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
			"CheckSyn() fail to  sync port and channel"));		
		return FALSE;
	}
	
	TianShanIce::Application::DataOnDemand::DataPointPublisherPrx dataPrx;
	TianShanIce::Application::DataOnDemand::DataStreamPrx datastreamprx;
	TianShanIce::Application::DataOnDemand::DataPublishPointPrx datappprx =NULL;
	
	::TianShanIce::Application::DataOnDemand::DataStreamInfos datastreaminfos;
	::TianShanIce::StrValues portchannels;

	::TianShanIce::Application::DataOnDemand::DataPublishPointInfos datappinfos;
	::TianShanIce::Application::DataOnDemand::DataAttachInfos attachinfos;
	::TianShanIce::Application::DataOnDemand::DataAttachInfos::iterator attachitor;
	
	::TianShanIce::Application::DataOnDemand::DataStreamInfos::iterator datastreamitor;
	::TianShanIce::Application::DataOnDemand::DataPublishPointInfos::iterator datappitor;

	TianShanIce::Properties props;
	
	dataPrx = GetDataPointPublisherPrx();
	
	if(dataPrx == NULL)
	{
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
			"CheckSyn() fail to get datapointpublisher porxy"));
		return FALSE;
	}

	try
	{		
		datastreaminfos = dataPrx->listDataStreams("");// list all Destinations

		(*_logger)( ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
			"CheckSync() list datastreams, count(%d)"),datastreaminfos.size());
		
		for(datastreamitor = datastreaminfos.begin();datastreamitor != datastreaminfos.end(); datastreamitor++) 
		{
			(*_logger)( ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
				"CheckSync()datastream name(%s)"),
				(*datastreamitor).name.c_str());
		}
		
		datappinfos = dataPrx->listDataPublishPoints("");// list all channels

		(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
			"CheckSync()list datapublishpoint, count(%d)"),
			datappinfos.size());
		
		for(datappitor = datappinfos.begin(); datappitor != datappinfos.end(); 
		                   datappitor++) 
		{
			(*_logger)( ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
				"CheckSync()datapublishpoint name (%s)"),
				(*datappitor).name.c_str());
		}

	  //Sync Channel			
	   CHANNELMAP::iterator iter;
	   CHANNELINFO* pChannelinfo;
	   if(datappinfos.size() != 0)
	   {
		   for(iter = m_channels.begin(); iter!= m_channels.end();iter++)
		   {
			   pChannelinfo = iter->second;
			   
			   for(datappitor = datappinfos.begin(); datappitor != datappinfos.end(); datappitor++)
			   {
				   if(datappitor->name == pChannelinfo->ChannelName)
					   break;
			   }

			   if( datappitor == datappinfos.end())
				   continue;

			   datappprx = dataPrx->openDataPublishPoint(pChannelinfo->ChannelName);
			   if(datappprx == NULL)
				   continue;
			 
			   props = datappprx->getProperties(); 
			
			   string filepath = props["Path"];
               pChannelinfo->strCacheDir = m_strCacheDir + pChannelinfo->ChannelName;
               
			   std::string  type = datappprx->getType();

			   (*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				  "CheckSyn()channel name(%s),filepath(%s), cache directory(%s)"),
				   (*datappitor).name.c_str(), filepath.c_str(),pChannelinfo->strCacheDir.c_str());

			   if(filepath != pChannelinfo->strCacheDir && type != TianShanIce::Application::DataOnDemand::dataLocalFolder)
			   {
				   datappprx = NULL;
				   continue;
			   }
			   datappinfos.erase(datappitor);
		   }
		   
		   try
		   {
			   (*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				   "CheckSyn()different channel size (%d)"),
				   datappinfos.size());

			   if(!datappinfos.empty())
			   {
				   for(datappitor = datappinfos.begin(); 
				   datappitor!= datappinfos.end(); datappitor++)
				   {
					   (*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
						   "CheckSyn()destroy datapublishpoint name(%s)"),
						   (*datappitor).name.c_str());
					   
					   datappprx = dataPrx->openDataPublishPoint(datappitor->name);
					   datappprx->destroy();
					   
					   (*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
						   "CheckSyn()destroy datapublishpoint name(%s) successfully"),
						   (*datappitor).name.c_str());
				   }
			   }
		   }
		   catch (const Ice::ObjectNotExistException &)
		   {
			   (*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				   "CheckSyn()get channel(%s) caught error at Ice::ObjectNotExistException"),
				   (*datappitor).name.c_str());
			   (*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				   "CheckSyn() fail to  sync port and channel"));	
			   return FALSE;
		   }
		   catch (const Ice::Exception& ex) 
		   {
			   (*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				   "CheckSyn()get channel caught error at Ice::Exception (%s)"),
				   ex.ice_name().c_str());
			   (*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				   "CheckSyn() fail to  sync port and channel"));	
			   return FALSE;
		   }
		   catch (...) 
		   {
			   int nError = GetLastError();
			   char strError[500];
			   
			   GetErrorDescription(nError, strError);
			   
			   (*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				   "CheckSyn()destory channel error (%d)(%s)"),nError, strError);
			   
			   (*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				   "CheckSyn() fail to  sync port and channel"));	
			   return FALSE;
		   }		   
	   }

	   //Sync Destination
	   if(datastreaminfos.size() == 0)
	   {
          	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn()sync port successfully"));
			return TRUE;
	   }
		CPORTVECTOR::iterator iterport;	
		std::string destname;
		try
		{
			for(iterport = m_portManager.begin(); iterport != m_portManager.end(); 
			iterport++)
			{	
				portchannels.clear();
				portchannels = listPortChannels(*iterport);
				
				IPPORTVECTOR::iterator iterIpPort;
				for(iterIpPort = (*iterport)->ipportvector.begin();
				iterIpPort != (*iterport)->ipportvector.end();
				iterIpPort++)
				{
					(*_logger)(ZQ::common::Log::L_INFO,
						"CheckSync()Port name (%s)",
						(*iterIpPort)->IpPortName.c_str());

					for(datastreamitor = datastreaminfos.begin();datastreamitor !=datastreaminfos.end();datastreamitor++)
					{
                         if(datastreamitor->name == (*iterIpPort)->IpPortName)
						 {
							 break;
						 }
					}

					if( datastreamitor == datastreaminfos.end())
						continue;
					destname = datastreamitor->name;
					datastreaminfos.erase(datastreamitor);
					
					datastreamprx = dataPrx->openDataStream((*iterIpPort)->IpPortName);

					attachinfos.clear();
					attachinfos = datastreamprx->listAttachments("");//dodapp destination channel list
					
					if(portchannels.size() != attachinfos.size())
					{
						datastreamprx->stop();
						datastreamprx->destroy();
						
						(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
							"CheckSyn()destroy destinaton (%s) success"),
							destname.c_str());
						continue;
					}
					::TianShanIce::StrValues::iterator iterchannel;
					for(iterchannel = portchannels.begin(); 
							iterchannel != portchannels.end(); 
							iterchannel++)
					{
						(*_logger)( ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
							"CheckSyn()channel name (%s)"),
							(*iterchannel).c_str());

						for(attachitor = attachinfos.begin(); attachitor != attachinfos.end(); attachitor++)
						{
							if(attachitor->dataPublishPointName == *iterchannel)
							{
								break;
							}
						}
						
						if( attachitor == attachinfos.end())
						{
							datastreamprx->stop();
							datastreamprx->destroy();
							
							(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
								"CheckSyn()destroy destinaton (%s) success"),
								destname.c_str());
							break;
						}
						else
						{
							// here modify
							int minBitRate = 0;
							int repeatTime = 0;
							CHANNELATTACHINFO *channelattachinfo;
							TianShanIce::Application::DataOnDemand::DataAttachInfos oneattachinfo;
                							oneattachinfo = datastreamprx->listAttachments(*iterchannel);
							channelattachinfo = 
								(*iterport)->channelattachMap[*iterchannel];
							if(oneattachinfo.size() < 1)
							{
								(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
									"CheckSyn() %s, fail to get attachment info (%s)"),
									destname.c_str(), (*iterchannel).c_str());
								return FALSE;
							}
							if(channelattachinfo->nRepeatetime != oneattachinfo[0].repeatTime ||
								channelattachinfo->nChannelRate != oneattachinfo[0].minBitRate)
							{
								datastreamprx->stop();
								datastreamprx->destroy();
								
								(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
									"CheckSyn()destroy destinaton (%s) success"),
									destname.c_str());
								break;
							}							
							attachinfos.erase(attachitor);
						}
					}
				}
			}			
		}
		catch(::TianShanIce::InvalidParameter&ex)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() destination (%s)caught error at ::TianShanIce::InvalidParameter(%s)"),
				destname.c_str(),ex.message.c_str());
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() fail to  sync port and channel"));	
			return FALSE;
		}
		catch(const Ice::ObjectNotExistException &)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() get destination (%s) caught error at Ice::ObjectNotExistException"),
				destname.c_str());
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() fail to  sync port and channel"));	
			return FALSE;
		}
		catch (const Ice::Exception& ex) 
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() get destination caught error at Ice::Exception(%s)"),
				ex.ice_name().c_str());
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() fail to  sync port and channel"));	
			return FALSE;
		}
		
		try
		{
			if(!datastreaminfos.empty())
			{
				for(datastreamitor = datastreaminfos.begin(); 
				datastreamitor!= datastreaminfos.end(); datastreamitor++)
				{
					(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
						"CheckSyn()destroy destinaton (%s)"),
						(*datastreamitor).name.c_str());
							
					datastreamprx = dataPrx->openDataStream(datastreamitor->name);
					datastreamprx->stop();
					datastreamprx->destroy();
					
					(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
						"CheckSyn()destroy destinaton (%s) success"),
						(*datastreamitor).name.c_str());
				}
			}
		}
		catch(const Ice::ObjectNotExistException &)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn()get destination(%s) caught error at Ice::ObjectNotExistException"),
				(*datastreamitor).name.c_str());
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() fail to  sync port and channel"));	
			return FALSE;
		}
		catch (const Ice::Exception& ex) 
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn()get destination caught error at Ice::Exception (%s)"),
				ex.ice_name().c_str());
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"CheckSyn() fail to  sync port and channel"));	
			return FALSE;
		}		
	}
	catch(...)
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
			"CheckSyn() sync destination  error (%d)(%s)"),
			nError, strError);
		
		(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
			"CheckSyn() sync destination error"));
		return FALSE;
	}
	
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(CJMSPORTMANAGER,
		"CheckSyn() sync destination success"));

	(*_logger)(ZQ::common::Log::L_INFO,CLOGFMT(CJMSPORTMANAGER,
		"CheckSyn()Levae sync port and channel"));
	return TRUE;
}

BOOL CJMSPortManager::ConventChannelName()
{
	ZQ::common::MutexGuard guard(_mutex);
	CHANNELMAP::iterator iter;
	CHANNELINFO* pChannelinfo;
    char channelname[200];
	for(iter = m_channels.begin(); iter!= m_channels.end();iter++)
	{
		std::string name = iter->first;
		pChannelinfo = iter->second;

		sprintf(channelname,"%s_%d_%d_%s_%d_%d_%s_%d",
		pChannelinfo->ChannelName.c_str(),
		pChannelinfo->nStreamCount,
		pChannelinfo->nstreamId,
		pChannelinfo->strdataType.c_str(),
		pChannelinfo->streamType,
		pChannelinfo->nEncrypted,
		pChannelinfo->Tag.c_str(),
		pChannelinfo->nSendWithDestination);
	   pChannelinfo->ChannelName = channelname;
	   replace(pChannelinfo->ChannelName.begin(),pChannelinfo->ChannelName.end(), ';', '_' ); 
	}
	return TRUE;
}

BOOL CJMSPortManager::ConvertPortName()
{
	ZQ::common::MutexGuard guard(_mutex);
	CPORTVECTOR::iterator iterport;	
	CHANNELATTACHINFOMAP channelattachmap;
	char portname[100];
	
	for(iterport = m_portManager.begin(); iterport != m_portManager.end(); 
	    iterport++)
	{
		IPPORTVECTOR::iterator iterIpPort;
		for(iterIpPort = (*iterport)->ipportvector.begin();
		iterIpPort != (*iterport)->ipportvector.end();
		iterIpPort++)
		{	
			sprintf(portname,"%s_%d_%d_%s_%d",(*iterport)->portname.c_str(),
				(*iterport)->pmtPid,(*iterport)->groupId,
				(*iterIpPort)->strIp.c_str(), (*iterIpPort)->nPort);			
			(*iterIpPort)->IpPortName = portname;			
		}

        channelattachmap.clear();
		
		CHANNELATTACHINFOMAP::iterator iterchannel;
		for(iterchannel =(*iterport)->channelattachMap.begin();
			iterchannel != (*iterport)->channelattachMap.end(); 
			iterchannel ++)
		{					
			CHANNELMAP::iterator it = m_channels.find(iterchannel->first);
			std::string channelname = iterchannel->first;
			if (it == m_channels.end())
			{
				(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
					"find channel error,channelname='%s', please Check PortConfigration"),
					iterchannel->first.c_str());
				return FALSE;
			}

			iterchannel->second->DataExchangeType = it->second->DataExchangeType;

			channelattachmap[it->second->ChannelName] = iterchannel->second;
		}

		(*iterport)->channelattachMap.clear();
    	(*iterport)->channelattachMap = channelattachmap;
	}
	return TRUE;
}
TianShanIce::StrValues  CJMSPortManager::listPortChannels(DODPORT * pPort)
{
	ZQ::common::MutexGuard guard(_mutex);
    TianShanIce::StrValues channels;
	CHANNELATTACHINFOMAP::iterator iterchannel;
	for(iterchannel = pPort->channelattachMap.begin();
		iterchannel != pPort->channelattachMap.end(); 
		iterchannel++)
	{
         channels.push_back(iterchannel->first);
	}
	return channels;
}
BOOL CJMSPortManager::RetryCreatPort()
{
/*	(*_logger)(ZQ::common::Log::L_DEBUG, 
		RetryCreatPort(): Retry Port size = %s",
		m_FailPort.size());

	if(m_FailPort.size() == 0)
		return true;

	CFAILPORTVECTOR::iterator itor = m_FailPort.begin();
	
	while(itor != m_FailPort.end() && !g_bStop)
	{
       if(CreateFailPort(itor))
	   {
		   m_FailPort.erase(itor);
	   }
	   Sleep(1000);
	   itor = m_FailPort.begin();
	}*/
	return true;
}
TianShanIce::Application::DataOnDemand::DataPointPublisherPrx CJMSPortManager::GetDataPointPublisherPrx()
{
	TianShanIce::Application::DataOnDemand::DataPointPublisherPrx dataPrx = NULL;
	try
	{
		Ice::ObjectPrx base = m_ic->stringToProxy(m_strDataTunnelEndpoint);

		if(!base)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"GetDataPointPublisherPrx()fail to convert string to proxy error"));
			return NULL;
		}

		dataPrx = TianShanIce::Application::DataOnDemand::DataPointPublisherPrx::checkedCast(base);

		if (!dataPrx)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,CLOGFMT(CJMSPORTMANAGER,
				"GetDataPointPublisherPrx() fail to checkedCast object"));
			return NULL;
		}   		
	}
	catch (const ::Ice::Exception & ex) 
	{
		(*_logger)(ZQ::common::Log::L_DEBUG,CLOGFMT(CJMSPORTMANAGER,
			"GetDataPointPublisherPrx() caught Ice Exception (%s)"),
			ex.ice_name().c_str());
		return NULL;
	}

	return dataPrx;
}
BOOL CJMSPortManager::CreateFailPort(DODFAILPORT* pfailport)
{
/*	::DataOnDemand::DestInfo destinfo;
	DataOnDemand::DestinationPrx destinationprx;
	destinfo.name = pfailport->IpPortName;
	destinfo.groupId = pfailport->groupId;
	destinfo.destAddress = pfailport->destAddress;
	destinfo.pmtPid = pfailport->pmtPid;
	destinfo.totalBandwidth = pfailport->totalBandWidth;
	
	try
	{	
		dataPrx = GetDataPointPublisherPrx();
		
		if(dataPrx == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR,"fail to get datapointpublisher porxy");
			return FALSE;
		}
		try
		{
			destinationprx = dataPrx->getDestination((*iteripport)->IpPortName);
		}
		catch (Ice::ObjectNotExistException&)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, 
			"get Destination error, portname = %s", 
			(*iteripport)->IpPortName.c_str());
			
			destinationprx = dataPrx->createDestination(
									pfailport->IpPortName,destinfo);
		
			if(destinationprx == NULL)
			{
				(*_logger)(ZQ::common::Log::L_ERROR, 
				"createDestination error, portname = %s", 
				pfailport->IpPortName.c_str());
				return FALSE;
			}
		}

		(*_logger)(ZQ::common::Log::L_DEBUG, 
			"[portname = %s ]CJMSPortManager::CreatePorts() success ",
			pfailport->IpPortName.c_str());

			CHANNELATTACHINFOMAP::iterator iterchannelAttach;
			for(iterchannelAttach = pPort->channelattachMap.begin();
				iterchannelAttach != pPort->channelattachMap.end();
				iterchannelAttach++)
				{					
					CHANNELATTACHINFO *channelattachinfo = 
						iterchannelAttach->second;
					try
					{		
						destinationprx->attachChannel(
							iterchannelAttach->first,	
							channelattachinfo->nChannelRate,
							channelattachinfo->nRepeatetime);

						(*_logger)(ZQ::common::Log::L_DEBUG, 
							"[portname = %s , channelname = %s]attachChannel  success ",
							(*iteripport)->IpPortName.c_str(),iterchannelAttach->first.c_str());
					}
					catch (const DataOnDemand::ObjectExistException &e)
					{
						(*_logger)(ZQ::common::Log::L_DEBUG,
							"AttachChannel():channelname = %s  Object exist"
							"\t\t\t\t\t\t\t\t\t\t errorcode  = %s",
							iterchannelAttach->first.c_str(),e.ice_name().c_str());
					}
					catch (const DataOnDemand::StreamerException &e)
					{
						(*_logger)(ZQ::common::Log::L_DEBUG,
							"AttachChannel():[channelname = %s ] "
							"DataOnDemand::StreamerException errorcode  = %s",
							iterchannelAttach->first.c_str(),e.ice_name().c_str());
						return FALSE;
					}
										
					if(channelattachinfo->DataExchangeType ==
						::DataOnDemand::dodSharedFolder )
					{
						char portcachefold[255];
						std::string channelCache = m_strCacheDir + iterchannelAttach->first;
						std::string portfolder;
				    	sprintf(portcachefold,"%s\\%d",
							channelCache.c_str(), pPort->groupId);

						portfolder = portcachefold;
						if(!DirectoryExist(portfolder.c_str()))
						{
							(*_logger)(ZQ::common::Log::L_DEBUG, 
								"current Channel path is not exist[ path = %s ]",
								portfolder.c_str());
							
							if(CreateDirectory(portfolder.c_str(), NULL) == FALSE)		
							{
								int nError = GetLastError();
								char strError[500];
								
								GetErrorDescription(nError, strError);
								(*_logger)(ZQ::common::Log::L_DEBUG, 
									CreateChannelDirectory[ path = %s ]is "
									"error.GetLastError() = %d, ErrorDescription = %s",
									portfolder.c_str(),nError, strError);
								
								return FALSE;
							}
							(*_logger)(ZQ::common::Log::L_DEBUG, 
								CreateChannelDirectory[ path = %s ]  success ",
								portfolder.c_str());
						}
						else
						{
							(*_logger)(ZQ::common::Log::L_DEBUG, 
								"current channel path is exist[path = %s]",
								portfolder.c_str());
						}						
					}					
				}       
	}*/
    return true;
}
