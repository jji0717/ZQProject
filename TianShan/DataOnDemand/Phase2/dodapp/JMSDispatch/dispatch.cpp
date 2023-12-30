// Dispatch.cpp 
//
#include "stdafx.h"
#include "messagemacro.h"
#include <stdlib.h>
#include <markup.h>
#include <algorithm>  

BOOL g_ConnectJBossIsOK;
BOOL g_bStop;

// This is the constructor of a class that has been exported.
// see JMSDispatch.h for the class definition
CJMSPortManager::CJMSPortManager()
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
	writeLog(ZQ::common::Log::L_INFO,
		"Enter CJMSPortManager::Create() ");

	m_strCacheDir = strCacheDir;
	m_ProviderValue = strJBossIPPort.c_str();
	m_strConfigQueueName = ConfigQueueName;
    m_nConfigTimeOut = ConfigMsgTimeOut;
	m_Pjmsprocthread = new CJmsProcThread();
	m_nUsingJBoss = nUsingJBoss;
	
	if(NULL == m_Pjmsprocthread )
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::Create() JmsProcThread error");
		return FALSE;
	}
	if(!m_Pjmsprocthread->init())
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::Create() Initialize JmsProcThread::init() error");
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
		writeLog(ZQ::common::Log::L_INFO, 
			"CJMSPortManager::Initialize() Begin Connect to JBOSS service (%s)",m_ProviderValue);
		
		m_jms = new CJMS(m_ProviderValue.GetBuffer(0));
		
		if(NULL == m_jms )
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"CJMSPortManager::Initialize() init JMS  error!");
			
			return FALSE;
		}
		
		if(!ConnectionJBoss())
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"CJMSPortManager::Initialize()Connect to JBOSS service error (%s)",
				m_ProviderValue);
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::Initialize() init error");
			return FALSE;
		}
		
		m_ParsePortConfig = new  CReceiveJmsMsg(this);
		
		if(m_ParsePortConfig == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"CJMSPortManager::Initialize()Create CReceiveJmsMsg Object error");
			return FALSE;
		}
		m_ParsePortConfig->m_strQueueName = m_strConfigQueueName.c_str();
		if(m_QueueManagement.QueueIsExist(
			m_strConfigQueueName.c_str(),INT_ONLY_SEND,TRUE) == FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,(char *)m_strConfigQueueName.c_str(),			
				INT_ONLY_SEND,m_ParsePortConfig) == FALSE)
			{
				writeLog(ZQ::common::Log::L_ERROR, 
					"CJMSPortManager::Initialize()m_jms.AddOneSendQueue (%s) error", 
					m_strConfigQueueName.c_str());
				writeLog(ZQ::common::Log::L_DEBUG, 
					"CJMSPortManager::Initialize() fail to init JMS");
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_INFO, 
				"CJMSPortManager::Initialize()The queue named (%s) is registed in JBoss",
				m_strConfigQueueName.c_str());
		}
		else
		{
			writeLog(ZQ::common::Log::L_INFO,
				"CJMSPortManager::Initialize()The queue named (%s) has existed in JBoss",
				m_strConfigQueueName.c_str());
			
		}
		CJMSTxMessage tempMsg;
		while (1)
		{
			if(g_bStop)
				return  FALSE;
			CString ssaa = m_ParsePortConfig->SendGetConfig();
			
			writeLog(ZQ::common::Log::L_INFO, 
				"CJMSPortManager::Initialize() Send Get Port Configuration Message: (%s)",ssaa);
			
			if (m_jms->SyncSendMsg(TRUE,(char*)m_strConfigQueueName.c_str(),
				ssaa.GetBuffer(0),tempMsg,m_nConfigTimeOut,"MESSAGECODE",
				JMSMESSAGE_CONFIGURATION,"MESSAGECLASS","COMMAND")==FALSE)
			{
				writeLog(ZQ::common::Log::L_WARNING, 
				"CJMSPortManager::Initialize() Send config_msg to queue(%s) by sync ,but the config_module has no response",
				m_strConfigQueueName.c_str());				
				Sleep(50);
			}
			else
				break;
		}
		
		writeLog(ZQ::common::Log::L_INFO, 
			"CJMSPortManager::Initialize()Get port configuration successful");
		
		writeLog(ZQ::common::Log::L_INFO, 
			"CJMSPortManager::Initialize()Begin parser port configuration ...");
		
		if(m_ParsePortConfig->parseNotification(&tempMsg))
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
    
		ConventChannelName();

		if(!ConvertPortName())
		{
         	writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}

		if(!AddChannelQueue())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		
		if(!CheckSyn())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		if(!CreatePorts())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		if(!CreateChannels())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		if(!AttachChannel())
		{
  			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		if(!RetryCreatPort())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		if(!SetMessage())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
	}
	else
	{
		writeLog(ZQ::common::Log::L_INFO, 
			"CJMSPortManager::Initialize() This is test mode",m_ProviderValue);
		m_ParsePortConfig = new  CReceiveJmsMsg(this);
		
		if(m_ParsePortConfig == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR, "CJMSPortManager::Initialize()Create CReceiveJmsMsg Object error");
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
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}

		if(!CheckSyn())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}

		if(!CreatePorts())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize():Parser Port Configuration error ");
			return FALSE;
		}
		if(!CreateChannels())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		if(!AttachChannel())
		{
  			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
		if(!RetryCreatPort())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize()Parser Port Configuration error ");
			return FALSE;
		}
	}
	
	writeLog(ZQ::common::Log::L_INFO,
		"CJMSPortManager::Initialize()Parser Port Configuration success");
	
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
		
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::UnInitialize() caught error (%d)(%s)",nError, strError);
		return FALSE;
	}
	
	writeLog(ZQ::common::Log::L_INFO,
		" CJMSPortManager::UnInitialize() successfully");
	
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
						writeLog(ZQ::common::Log::L_ERROR,
							"CJMSPortManager::ConnectionJBoss() Connect Jboss Error. m_Connection.createSession error, queuename (%s) ",m_VecParser[i].QueueName);
						return FALSE ;
					}
					
					if(m_jms->m_JndiContext->createDestination((*iter)->m_QueueName,
						(*iter)->m_destination) == FALSE)
					{
						(*iter)->m_bError = TRUE;
						writeLog(ZQ::common::Log::L_ERROR,
							"CJMSPortManager::ConnectionJBoss()Connect Jboss Error. createDestination error ,queuename(%s)",
							m_VecParser[i].QueueName);
						return FALSE;
					}
					
					
					if((*iter)->m_Session.createConsumer(&(*iter)->m_destination,
						(*iter)->m_consumer) == FALSE)
					{
						(*iter)->m_bError = TRUE;
						writeLog(ZQ::common::Log::L_ERROR,
							"CJMSPortManager::ConnectionJBoss() Connect Jboss Error.createConsumer error, "
							"queuename (%s)",m_VecParser[i].QueueName);
						return FALSE;
					}
					
					m_VecParser[i].MsgReceive->m_Consumer = &(*iter)->m_consumer;
					m_VecParser[i].MsgReceive->m_destination = &(*iter)->m_destination;
					m_VecParser[i].MsgReceive->m_Session = &(*iter)->m_Session;
					
					if (!(*iter)->m_Session.textMessageCreate("", 
						m_VecParser[i].MsgReceive->m_jmsTextMsg))
					{
						writeLog(ZQ::common::Log::L_ERROR,
							" CJMSPortManager::ConnectionJBoss() Connect Jboss Error.textMessageCreate error, "
							"queuename (%s)",m_VecParser[i].QueueName);
						return false;
					}
					
					writeLog(ZQ::common::Log::L_ERROR,
						"CJMSPortManager::ConnectionJBoss() [%s] Create Receive Consumer success",
						m_VecParser[i].QueueName);	 
					i++;
				}				
			}
			writeLog(ZQ::common::Log::L_INFO, 
				"CJMSPortManager::ConnectionJBoss() Connect JBOSS successful.(%s)",m_ProviderValue);
			m_bReSetMsgListener = TRUE;
			break;
		}		
		else
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::ConnectionJBoss()fail to Connect JBOSS (%s)",m_ProviderValue);
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
		writeLog(ZQ::common::Log::L_ERROR, 
			"CJMSPortManager::AddChannelQueue()no Port configration info ");

		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::AddChannelQueue() AddChannelQueue Error");		
		return FALSE;
	}
	
	CString sTmp;
	UINT i = 0;
	for(i = 0; i < m_VecParser.size(); i++)
	{     
		m_VecParser[i].MsgReceive = new CReceiveJmsMsg(this);
		
		if(!m_VecParser[i].MsgReceive)
		{
			writeLog(ZQ::common::Log::L_ERROR, 
			"CJMSPortManager::AddChannelQueue()initialize CReceiveJmsMsg Error");
			writeLog(ZQ::common::Log::L_ERROR, 
				"CJMSPortManager::AddChannelQueue() fail to register queue");
			return FALSE;
		}
		
		m_VecParser[i].MsgReceive->m_strQueueName = m_VecParser[i].QueueName;
		
		sTmp = "queue/" + m_VecParser[i].QueueName;
		if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_RECEIVE,TRUE) == FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),INT_ONLY_RECEIVE,
				m_VecParser[i].MsgReceive) == FALSE)
			{
				writeLog(ZQ::common::Log::L_ERROR, 
				"CJMSPortManager::AddChannelQueue() fail to add (queuename(%s)error",m_VecParser[i].QueueName);
				writeLog(ZQ::common::Log::L_ERROR, 
					"CJMSPortManager::AddChannelQueue() fail to register queue");
				return FALSE;
			}
			if(!m_VecParser[i].MsgReceive->init())
			{
				writeLog(ZQ::common::Log::L_ERROR,
					"CJMSPortManager::AddChannelQueue() Create ReceiverMsg Thread error.",
					m_VecParser[i].QueueName);
				writeLog(ZQ::common::Log::L_ERROR,
					"CJMSPortManager::AddChannelQueue() fail to register queue" );
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_INFO, 
				"CJMSPortManager::AddChannelQueue()The queue named (%s) is registed in JBoss" ,
				m_VecParser[i].QueueName);
		}
		else
			writeLog(ZQ::common::Log::L_INFO, 
			"CJMSPortManager::AddChannelQueue() The queue named (%s) has existed in JBoss" ,
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
			writeLog(ZQ::common::Log::L_INFO, 
			"CJMSPortManager::AddChannelQueue() queue name is null");
			continue;
		}
		
		if (channel->nDataExchangeType == ::DataOnDemand::dodLocalFolder)
			continue;
		
		sTmp.Format("queue/%s",channel->strQueueName.c_str());
		if(m_QueueManagement.QueueIsExist(sTmp,INT_ONLY_SEND,TRUE) == FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,sTmp.GetBuffer(0),
				INT_ONLY_SEND,m_ParsePortConfig) == FALSE)
			{
				writeLog(ZQ::common::Log::L_ERROR, 
					"CJMSPortManager::AddChannelQueue()[channel: %s] AddOneSendQueue (%s) error.",
				iter->first.c_str(),channel->strQueueName.c_str());
				writeLog(ZQ::common::Log::L_ERROR, 
					"CJMSPortManager::AddChannelQueue() fail to register queue");
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::AddChannelQueue()[channel: %s]Add Send Queue (%s)success",
				iter->first.c_str(),channel->strQueueName.c_str());
		}
		else
			writeLog(ZQ::common::Log::L_DEBUG, 
			"CJMSPortManager::AddChannelQueue()[channel: %s]queuename (%s) is exist",
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
		writeLog(ZQ::common::Log::L_ERROR, 
			"CJMSPortManager::SetMessage()no Port configration info ");
		writeLog(ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::SetMessage() fail to set message");		
		return FALSE;
	}
	
	if(g_bStop)
		return FALSE;
	
	for(UINT i = 0; i <m_VecParser.size(); i++)
	{
		writeLog(ZQ::common::Log::L_DEBUG, 
			"CJMSPortManager::SetMessage()[CReceiveJmsMsg::QueueName = %s]set  start service flag equal to TRUE",
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
			
			if (channelinfo->nDataExchangeType == DataOnDemand::dodLocalFolder ||
				channelinfo->nDataExchangeType == DataOnDemand::dodMessage)
				continue;
			
			if (channelinfo->strQueueName.size() == 0)
			{
				writeLog(ZQ::common::Log::L_DEBUG, 
					"CJMSPortManager::SetMessage()queue name is null, portname (%s)",
					pPort->portname.c_str());
				continue;
			}
			
			if (m_sDataTypeInitial.GetLength() > 0)
			{
				int nIndexDataType = m_sDataTypeInitial.Find(
					channelinfo->strdataType.c_str());
				
				writeLog(ZQ::common::Log::L_DEBUG,
					"CJMSPortManager::SetMessage()DataTypeInitial(%s), channel dataType(%s)",
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
						writeLog(ZQ::common::Log::L_DEBUG, 
						"CJMSPortManager::SetMessage()[%s : %s] send Data Type Initial msg to queue(%s) error",
						pPort->portname.c_str(),
						it->first.c_str(),channelinfo->strQueueName.c_str());
						Sleep(5);
					}
					else
					{
						writeLog(ZQ::common::Log::L_DEBUG, 
						"CJMSPortManager::SetMessage()[%s : %s] send Data Type Initial msg to queue(%s) ok",
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
				writeLog(ZQ::common::Log::L_DEBUG,
					"CJMSPortManager::SetMessage()[%s : %s] Send get fulldata Msg (%s) error",
					pPort->portname.c_str(),it->first.c_str(),sTmp);
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::SetMessage()[%s : %s] Send  get fulldata msg to queue(%s) ok",
				pPort->portname.c_str(),it->first.c_str(),sTmp);
		}
	}
	
	return TRUE;
}

BOOL CJMSPortManager::CreateChannels()
{
	ZQ::common::MutexGuard guard(_mutex);
	::DataOnDemand::ChannelInfo channelinfo;
	::DataOnDemand::ChannelPublishPointPrx channelprx = NULL;
	::DataOnDemand::DataPublisherPrx dataPrx;
	TianShanIce::Properties props;
	if(1 > (int)m_channels.size())
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::CreateChannel() channel count is 0");		
		return FALSE;
	}
	
	CHANNELMAP::iterator iterchannel;
	for(iterchannel = m_channels.begin(); iterchannel != m_channels.end(); 
	    iterchannel++)
	{
		CHANNELINFO *pChannelInfo = iterchannel->second;
		if(pChannelInfo == NULL)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CreateChannel()fail to create channel");
			return FALSE;
		}
       
		writeLog(ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::CreateChannel() channename (%s)",
			pChannelInfo->ChannelName.c_str());
		ParserChannelType(&channelinfo.dataTypes, pChannelInfo->strdataType);

		channelinfo.name = pChannelInfo->ChannelName;
		channelinfo.encrypt = pChannelInfo->nEncrypted;
		channelinfo.streamId = pChannelInfo->nstreamId;
		channelinfo.streamType = pChannelInfo->streamType;
		channelinfo.subchannelCount = pChannelInfo->nStreamCount;
		channelinfo.tag = ConvertTag(pChannelInfo->Tag);
		channelinfo.withDestination = pChannelInfo->nSendWithDestination;
		
		pChannelInfo->strCacheDir = m_strCacheDir + pChannelInfo->ChannelName;
		
		try
		{
			dataPrx = GetDataPublisherPrx();
			if(dataPrx == NULL)
			{
				writeLog(ZQ::common::Log::L_ERROR,
					"CJMSPortManager::CreateChannel() fail to get datapublisher porxy");
				return FALSE;
			}
			try
			{ 
				channelprx = dataPrx->getChannel(pChannelInfo->ChannelName);
			}
			catch (const Ice::ObjectNotExistException)
			{
				channelprx = NULL;	
			}
			
			if(pChannelInfo->nDataExchangeType == ::DataOnDemand::dodMessage)
			{
				if(channelprx == NULL)
				{
					channelprx = dataPrx->createMsgChannel
						(pChannelInfo->ChannelName,channelinfo);
					if(channelprx == NULL)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
						"CJMSPortManager::CreateChannel() create message channel(%s) error",
						pChannelInfo->ChannelName.c_str());
						return FALSE;
					}
					
					props = channelprx->getProperties();
					props["Path"] =  pChannelInfo->strCacheDir;					
					channelprx->setProperties(props);
					writeLog(ZQ::common::Log::L_DEBUG,
						"CJMSPortManager::CreateChannel() create message channel(%s) successfully",
						pChannelInfo->ChannelName.c_str());
				}
				else
				{					
                  	writeLog(ZQ::common::Log::L_DEBUG,
						"CJMSPortManager::CreateChannel() current channel (%s) already exist",
						pChannelInfo->ChannelName.c_str());
				}
			}
			else
			{
				if(pChannelInfo->nDataExchangeType == 
					::DataOnDemand::dodLocalFolder)
				{
					pChannelInfo->strCacheDir = pChannelInfo->strQueueName;

					if (channelprx == NULL)
					{
						channelprx = dataPrx->createLocalFolderChannel(
							pChannelInfo->ChannelName,channelinfo,
							pChannelInfo->strCacheDir);
						if(channelprx == NULL)
						{
							writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::CreateChannel()create local folder channel(%s) error",
							pChannelInfo->ChannelName.c_str());
							return FALSE;
						} 	
						writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::CreateChannel()create local folder channel(%s) success",
							pChannelInfo->ChannelName.c_str());
					}
					else
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::CreateChannel()current channel (%s) already exist",
							pChannelInfo->ChannelName.c_str());
					}
				}
				else
				{
					if (channelprx == NULL)
					{
                        channelprx = dataPrx->createShareFolderChannel(
							pChannelInfo->ChannelName,channelinfo);	
						if(channelprx == NULL)
						{
							writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::CreateChannel()create share folder channel(%s) error", 
							pChannelInfo->ChannelName.c_str());
							return FALSE;
						} 
						props = channelprx->getProperties();
						
						props["Path"] =  pChannelInfo->strCacheDir;
						
						channelprx->setProperties(props);

						writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::CreateChannel()create local folder channel(%s) success",
							pChannelInfo->ChannelName.c_str());
					}
					else
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::CreateChannel()current channel (%s) already exist",
							pChannelInfo->ChannelName.c_str());
					}
				}
				
			}
		}  
		catch (const ::Ice::Exception & ex) 
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CreateChannel() caught Ice::Exception (%s)",
				ex.ice_name().c_str());
			return FALSE;
		} 
		
		if(pChannelInfo->nDataExchangeType == ::DataOnDemand::dodLocalFolder)
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::CreateChannel()This is a Local folder channel, cache directory (%s)",
				pChannelInfo->strQueueName.c_str());
			continue;
		}
//		if(!DirectoryExist(pChannelInfo->strCacheDir.c_str()))
		if(_access(pChannelInfo->strCacheDir.c_str(),0))
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::CreateChannel()[%s]current Channel path (%s) not exists",
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str());
			
			if (CreateDirectory(pChannelInfo->strCacheDir.c_str(), NULL) == FALSE)		
			{
				int nError = GetLastError();
				char strError[500];
				
				GetErrorDescription(nError, strError);
				writeLog(ZQ::common::Log::L_DEBUG, 
					"CJMSPortManager::CreateChannel()[%s]create Directory (%s) error (%d)(%s)",
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str(),
				nError, strError);
				
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::CreateChannel()[%s]create Directory (%s) successfully",
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str());
		}
		else
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::CreateChannel()[%s]current channel path (%s) exist",
				pChannelInfo->ChannelName.c_str(),pChannelInfo->strCacheDir.c_str());
		}
		
	}
	return TRUE;
}
BOOL CJMSPortManager::CreatePorts()
{
	ZQ::common::MutexGuard guard(_mutex);
	::DataOnDemand::DestinationPrx destinationprx = NULL;
	::DataOnDemand::DestInfo destinfo;
	::DataOnDemand::DataPublisherPrx dataPrx;
	DODFAILPORT failport;
	char stripport[30];
	
	if(1 > (int)(m_portManager.size()))
	{
		writeLog(ZQ::common::Log::L_ERROR, 
			"CJMSPortManager::CreatePorts() no Port configration info ");
		
		writeLog(ZQ::common::Log::L_INFO,
			"CJMSPortManager::CreatePorts() fail to creat port");		
		return FALSE;
	}

	CPORTVECTOR::iterator iter;	
	for(iter = m_portManager.begin(); iter != m_portManager.end(); iter++)
	{
		DODPORT *pPort = (*iter);
		
		if(pPort == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"CJMSPortManager::CreatePorts() no port information");
			return FALSE;
		}
		IPPORTVECTOR::iterator iteripport;
		for(iteripport = pPort->ipportvector.begin(); 
			iteripport != pPort->ipportvector.end();
			iteripport++)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CreatePorts() PortName(%s)",
				(*iteripport)->IpPortName.c_str());
            memset(stripport,0, 30);
			
			switch((*iteripport)->nSendType)
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
			}
			pPort->destAddress = stripport;
			destinfo.name = (*iteripport)->IpPortName;
			destinfo.groupId = pPort->groupId;
			destinfo.destAddress = pPort->destAddress;
			destinfo.pmtPid = pPort->pmtPid;
			destinfo.totalBandwidth = pPort->totalBandWidth;
			
			try
			{	
				dataPrx = GetDataPublisherPrx();
				
				if(dataPrx == NULL)
				{
					writeLog(ZQ::common::Log::L_ERROR,"fail to get datapublisher porxy");
					return FALSE;
				}
				
				try
				{
					destinationprx = dataPrx->getDestination(
						                             (*iteripport)->IpPortName);
					writeLog(ZQ::common::Log::L_DEBUG, 
						"CJMSPortManager::CreatePorts()current port (%s) is exist",
						(*iteripport)->IpPortName.c_str());
				}	
				catch (Ice::ObjectNotExistException& )
				{				
					destinationprx = dataPrx->createDestination(
											(*iteripport)->IpPortName,destinfo);

					if(destinationprx == NULL)
					{
						writeLog(ZQ::common::Log::L_ERROR, 
							"CJMSPortManager::CreatePorts() create destination (%s)error", 
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
					writeLog(ZQ::common::Log::L_INFO, 
						"CJMSPortManager::CreatePorts() create destination (%s) success ",
						(*iteripport)->IpPortName.c_str());
				}
			}
			catch (const ::TianShanIce::InvalidParameter & ex) 
			{
				writeLog(ZQ::common::Log::L_ERROR,
					"CJMSPortManager::CreatePorts() caught TianShanIce::InvalidParameter(%s)",
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
				writeLog(ZQ::common::Log::L_DEBUG,
					"CJMSPortManager::CreatePorts() caught Ice::Exception (%s)",
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
	return TRUE;
}

BOOL CJMSPortManager::AttachChannel()
{
	ZQ::common::MutexGuard guard(_mutex);
	::DataOnDemand::DestinationPrx destinationprx = NULL;
	::DataOnDemand::DataPublisherPrx dataPrx;
	
	CPORTVECTOR::iterator iter;	
	for(iter = m_portManager.begin(); iter != m_portManager.end(); iter++)
	{
		DODPORT *pPort = (*iter);
		
		if(pPort == NULL)
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::AttachChannel()fail to get port info");
			return FALSE;
		}
		IPPORTVECTOR::iterator iteripport;
		for(iteripport = pPort->ipportvector.begin(); 
			iteripport != pPort->ipportvector.end();
			iteripport++)
		{
			writeLog(ZQ::common::Log::L_INFO,
				"CJMSPortManager::AttachChannel() PortName(%s)",
				(*iteripport)->IpPortName.c_str());			
			
			dataPrx = GetDataPublisherPrx();
			
			if(dataPrx == NULL)
			{
				writeLog(ZQ::common::Log::L_ERROR,
					"CJMSPortManager::AttachChannel() fail to get datapublisher porxy");
				return FALSE;
			}
			
			try
			{
				destinationprx = dataPrx->getDestination((*iteripport)->IpPortName);
			}
			catch (Ice::ObjectNotExistException&)
			{
				writeLog(ZQ::common::Log::L_ERROR, 
					"CJMSPortManager::AttachChannel()get Destination (%s) info error (Ice::ObjectNotExistException)", 
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
						destinationprx->attachChannel(
							iterchannelAttach->first,	
							channelattachinfo->nChannelRate,
							channelattachinfo->nRepeatetime);

						writeLog(ZQ::common::Log::L_DEBUG, 
							"CJMSPortManager::AttachChannel() [port: %s , channel: %s] attach channel  success",
							(*iteripport)->IpPortName.c_str(),iterchannelAttach->first.c_str());
					}
					catch (const DataOnDemand::ObjectExistException &e)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::AttachChannel()channel(%s)  Object exist"
							"\t\t\t\t\t\t\t\t\t\t errorcode(%s)",
							iterchannelAttach->first.c_str(),e.ice_name().c_str());
					}
					catch (const DataOnDemand::StreamerException &e)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::AttachChannel() channel(%s) caught error at "
							"DataOnDemand::StreamerException (%s)",
							iterchannelAttach->first.c_str(),e.ice_name().c_str());
						return FALSE;
					}
					catch (const Ice::Exception &e)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"CJMSPortManager::AttachChannel() channel(%s) caught error at "
							"DataOnDemand::Ice::Exception(%s)",
							iterchannelAttach->first.c_str(),e.ice_name().c_str());
						return FALSE;
					}
										
					if(channelattachinfo->nDataExchangeType ==
						::DataOnDemand::dodSharedFolder )
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
							writeLog(ZQ::common::Log::L_DEBUG, 
								"CJMSPortManager::AttachChannel() current channel (%s) path (%s)is not exist",
								iterchannelAttach->first.c_str(),portfolder.c_str());
							
							if(CreateDirectory(portfolder.c_str(), NULL) == FALSE)		
							{
								int nError = GetLastError();
								char strError[500];
								
								GetErrorDescription(nError, strError);
								writeLog(ZQ::common::Log::L_DEBUG, 
									"CJMSPortManager::AttachChannel() create directory (%s) error(%d)(%s)",
									portfolder.c_str(),nError, strError);
								
								return FALSE;
							}
							writeLog(ZQ::common::Log::L_DEBUG, 
								"CJMSPortManager::AttachChannel() create directory (%s) success",
								portfolder.c_str());
						}
						else
						{
							writeLog(ZQ::common::Log::L_DEBUG, 
								"CJMSPortManager::AttachChannel()current channel path (%s) already exist",
								portfolder.c_str());
						}
						
					}					
				}
					
				try
				{
					destinationprx->serve();
					destinationprx = NULL;
					writeLog(ZQ::common::Log::L_INFO, 
						"CJMSPortManager::AttachChannel()portname (%s) Serve success",
						(*iteripport)->IpPortName.c_str());
				}
				catch (DataOnDemand::StreamerException& ex)
				{
					writeLog(ZQ::common::Log::L_ERROR, 
						"CJMSPortManager::AttachChannel() portname (%s) Serve error, StreamerException (%s)",
						(*iteripport)->IpPortName.c_str(), ex.ice_name().c_str());
				}
				catch(DataOnDemand::StreamInvalidState& ex)
				{
					writeLog(ZQ::common::Log::L_ERROR, 
						"CJMSPortManager::AttachChannel()portname (%s) Serve error,"
						" DataOnDemand::StreamInvalidState(%s)",
						(*iteripport)->IpPortName.c_str(), ex.ice_name().c_str());
				}
				catch (Ice::Exception& ex)
				{
					writeLog(ZQ::common::Log::L_ERROR, 
						"CJMSPortManager::AttachChannel()portname (%s) Serve error,",
						(*iteripport)->IpPortName.c_str(), ex.ice_name().c_str());
				}
				catch (...)
				{
					int nError = GetLastError();
					char strError[500];
					
					GetErrorDescription(nError, strError);

					writeLog(ZQ::common::Log::L_ERROR, 
						"CJMSPortManager::AttachChannel()portname (%s) Serve error(%d)(%s)",
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
	writeLog(ZQ::common::Log::L_INFO,
		"CJMSPortManager::CheckSyn()Enter sync port and channel");

	if(1 > (int)(m_portManager.size()))
	{
		writeLog(ZQ::common::Log::L_ERROR, 
			"CJMSPortManager::CheckSyn()no Port configration info ");
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::CheckSyn() CheckSyn error");		
		return FALSE;
	}
	
	::DataOnDemand::DataPublisherPrx dataPrx;
	::DataOnDemand::DestinationPrx destprx;
	::DataOnDemand::ChannelPublishPointPrx channelprx =NULL;
	
	::TianShanIce::StrValues dests;
	::TianShanIce::StrValues channels, portchannels,destchannels;
	
	::TianShanIce::StrValues::iterator destsitor;
	::TianShanIce::StrValues::iterator channelsitor;

	TianShanIce::Properties props;
	
	dataPrx = GetDataPublisherPrx();
	
	if(dataPrx == NULL)
	{
		writeLog(ZQ::common::Log::L_ERROR,"CJMSPortManager::CheckSyn()fail to get datapublisher porxy");
		return FALSE;
	}

	try
	{		
		dests = dataPrx->listDestinations();// list all Destinations

		writeLog( ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::checkSync()DODApp destination count(%d)",
			dests.size());
		
		for(destsitor = dests.begin();destsitor != dests.end(); destsitor++) 
		{
			writeLog( ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::checkSync()DODApp destination name(%s)",
				(*destsitor).c_str());
		}
		
		channels = dataPrx->listChannels();// list all channels

		writeLog( ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::checkSync()DODApp channnel count (%d)",
			channels.size());
		
		for(channelsitor = channels.begin();channelsitor != channels.end(); 
		                   channelsitor++) 
		{
			writeLog( ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::checkSync()DODApp Channnel name (%s)",
				(*channelsitor).c_str());
		}

	  //Sync Channel			
	   CHANNELMAP::iterator iter;
	   CHANNELINFO* pChannelinfo;
	   if(channels.size() != 0)
	   {
		   for(iter = m_channels.begin(); iter!= m_channels.end();iter++)
		   {
			   pChannelinfo = iter->second;
			   
			   channelsitor = std::find(channels.begin(), 
				   channels.end(), pChannelinfo->ChannelName);
			   
			   if( channelsitor == channels.end())
				   continue;

			   channelprx = dataPrx->getChannel(pChannelinfo->ChannelName);
			   if(channelprx == NULL)
				   continue;
			 
			   props = channelprx->getProperties(); 
			
			   string filepath = props["Path"];
               pChannelinfo->strCacheDir = m_strCacheDir + pChannelinfo->ChannelName;
               
			   DataOnDemand::ChannelType type = channelprx->getType();

			   writeLog(ZQ::common::Log::L_INFO, 
				   "CJMSPortManager::CheckSyn()channel name(%s),filepath(%s), cache directory(%s)",
				   (*channelsitor).c_str(), filepath.c_str(),pChannelinfo->strCacheDir.c_str());

			   if(filepath != pChannelinfo->strCacheDir && type != DataOnDemand::dodLocalFolder )
			   {
				   channelprx = NULL;
				   continue;
			   }
			   channels.erase(channelsitor);
		   }
		   
		   try
		   {
			   writeLog(ZQ::common::Log::L_INFO, 
				   "CJMSPortManager::CheckSyn():different channel size (%d)",
				   channels.size());

			   if(!channels.empty())
			   {
				   for(channelsitor = channels.begin(); 
				   channelsitor!= channels.end(); channelsitor++)
				   {
					   writeLog(ZQ::common::Log::L_INFO, 
						   "CJMSPortManager::CheckSyn()destroy channel name(%s)",
						   (*channelsitor).c_str());
					   
					   channelprx = dataPrx->getChannel(*channelsitor);
					   channelprx->destroy();
					   
					   writeLog(ZQ::common::Log::L_INFO, 
						   "CJMSPortManager::CheckSyn()destroy channel name(%s) success",
						   (*channelsitor).c_str());
				   }
			   }
		   }
		   catch (const Ice::ObjectNotExistException &)
		   {
			   writeLog(ZQ::common::Log::L_ERROR,
				   " CJMSPortManager::CheckSyn()get channel(%s) caught error at Ice::ObjectNotExistException",(*channelsitor).c_str());
			   return FALSE;
		   }
		   catch (const Ice::Exception& ex) 
		   {
			   writeLog(ZQ::common::Log::L_ERROR,
				   "CJMSPortManager::CheckSyn()get channel caught error at Ice::Exception (%s)",
				   ex.ice_name().c_str());
			   return FALSE;
		   }
		   catch (...) 
		   {
			   int nError = GetLastError();
			   char strError[500];
			   
			   GetErrorDescription(nError, strError);
			   
			   writeLog(ZQ::common::Log::L_ERROR,
				   "CJMSPortManager::CheckSyn()destory channel error (%d)(%s)",nError, strError);
			   
			   writeLog(ZQ::common::Log::L_ERROR,
				   "CJMSPortManager::CheckSyn()sync port error!");
			   return FALSE;
		   }		   
	   }

	   //Sync Destination
	   if(dests.size() == 0)
	   {
          	writeLog(ZQ::common::Log::L_INFO, 
				"CJMSPortManager::CheckSyn()sync port successfully");
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
					writeLog( ZQ::common::Log::L_INFO,
						"CJMSPortManager::checkSync()Port name (%s)",
						(*iterIpPort)->IpPortName.c_str());

					destsitor = std::find(dests.begin(), 
						dests.end(), (*iterIpPort)->IpPortName);
					
					if( destsitor == dests.end())
						continue;
					destname = *destsitor;
					dests.erase(destsitor);
					
					destprx = dataPrx->getDestination((*iterIpPort)->IpPortName);
					
					destchannels.clear();
					destchannels = destprx->listChannels();//dodapp destination channel list
					
					if(portchannels.size() != destchannels.size())
					{
						destprx->stop();
						destprx->destroy();
						
						writeLog(ZQ::common::Log::L_INFO, 
							"CJMSPortManager::checkSync()destroy destinaton (%s) success",
							destname.c_str());
						continue;
					}
					::TianShanIce::StrValues::iterator iterchannel;
					for(iterchannel = portchannels.begin(); 
							iterchannel != portchannels.end(); 
							iterchannel++)
					{
						writeLog( ZQ::common::Log::L_INFO,
							"CJMSPortManager::checkSync()channel name (%s)",
							(*iterchannel).c_str());
						
						channelsitor = std::find(destchannels.begin(), 
							destchannels.end(), *iterchannel);
						
						if( channelsitor == destchannels.end())
						{
							destprx->stop();
							destprx->destroy();
							
							writeLog(ZQ::common::Log::L_INFO, 
								"CJMSPortManager::checkSync()destroy destinaton (%s) success",
								destname.c_str());
							break;
						}
						else
						{
							int minBitRate = 0;
							int repeatTime = 0;
							CHANNELATTACHINFO *channelattachinfo;
							destprx->getChannelAttachedInfo(*channelsitor,
														minBitRate,repeatTime);
							channelattachinfo = 
								(*iterport)->channelattachMap[*channelsitor];
							if(channelattachinfo->nRepeatetime != repeatTime ||
								channelattachinfo->nChannelRate != minBitRate)
							{
								destprx->stop();
								destprx->destroy();
								
								writeLog(ZQ::common::Log::L_INFO, 
									"CJMSPortManager::checkSync()destroy destinaton (%s) success",
									destname.c_str());
								break;
							}							
							destchannels.erase(channelsitor);
						}
					}
				}
			}			
		}
		catch(::TianShanIce::InvalidParameter&ex)
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::checkSync() destination (%s) channel(%s)caught error at ::TianShanIce::InvalidParameter(%s)",
				destname.c_str(), (*channelsitor).c_str()),ex.message.c_str();
			return FALSE;
		}
		catch(const Ice::ObjectNotExistException &)
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::checkSync() get destination (%s) caught error at Ice::ObjectNotExistException",
				destname.c_str());
			return FALSE;
		}
		catch (const Ice::Exception& ex) 
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::checkSync() get destination caught error at Ice::Exception(%s)",
				ex.ice_name().c_str());
			return FALSE;
		}
		
		try
		{
			if(!dests.empty())
			{
				for(destsitor = dests.begin(); 
				destsitor!= dests.end(); destsitor++)
				{
					writeLog(ZQ::common::Log::L_INFO, 
						"CJMSPortManager::checkSync()destroy destinaton (%s)",
						(*destsitor).c_str());
							
					destprx = dataPrx->getDestination(*destsitor);
					destprx->stop();
					destprx->destroy();
					
					writeLog(ZQ::common::Log::L_INFO, 
						"CJMSPortManager::checkSync()destroy destinaton (%s) success",
						(*destsitor).c_str());
				}
			}
		}
		catch(const Ice::ObjectNotExistException &)
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::checkSync()get destination(%s) caught error at Ice::ObjectNotExistException",
				(*destsitor).c_str());
			return FALSE;
		}
		catch (const Ice::Exception& ex) 
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::checkSync()get destination caught error at Ice::Exception (%s)",
				ex.ice_name().c_str());
			return FALSE;
		}		
	}
	catch(...)
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::checkSync() sync destination  error (%d)(%s)",
			nError, strError);
		
		writeLog(ZQ::common::Log::L_ERROR,
			" CJMSPortManager::checkSync() sync destination error");
		return FALSE;
	}
	
	writeLog(ZQ::common::Log::L_INFO, 
		"CJMSPortManager::checkSync() sync destination success");

	writeLog(ZQ::common::Log::L_INFO,
		"CJMSPortManager::CheckSyn()Levae sync port and channel");
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
				writeLog(ZQ::common::Log::L_ERROR,
					"find channel error,channelname='%s, please Check PortConfigration",
					iterchannel->first.c_str());
				return FALSE;
			}

			iterchannel->second->nDataExchangeType = it->second->nDataExchangeType;

			channelattachmap[it->second->ChannelName] = iterchannel->second;
		}

		(*iterport)->channelattachMap.clear();
    	(*iterport)->channelattachMap = channelattachmap;
	}
	return TRUE;
}
::TianShanIce::StrValues  CJMSPortManager::listPortChannels(DODPORT * pPort)
{
	ZQ::common::MutexGuard guard(_mutex);
    ::TianShanIce::StrValues channels;
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
/*	writeLog(ZQ::common::Log::L_DEBUG, 
		"CJMSPortManager::RetryCreatPort(): Retry Port size = %s",
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
		dataPrx = GetDataPublisherPrx();
		
		if(dataPrx == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR,"fail to get datapublisher porxy");
			return FALSE;
		}
		try
		{
			destinationprx = dataPrx->getDestination((*iteripport)->IpPortName);
		}
		catch (Ice::ObjectNotExistException&)
		{
			writeLog(ZQ::common::Log::L_ERROR, 
			"get Destination error, portname = %s", 
			(*iteripport)->IpPortName.c_str());
			
			destinationprx = dataPrx->createDestination(
									pfailport->IpPortName,destinfo);
		
			if(destinationprx == NULL)
			{
				writeLog(ZQ::common::Log::L_ERROR, 
				"createDestination error, portname = %s", 
				pfailport->IpPortName.c_str());
				return FALSE;
			}
		}

		writeLog(ZQ::common::Log::L_DEBUG, 
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

						writeLog(ZQ::common::Log::L_DEBUG, 
							"[portname = %s , channelname = %s]attachChannel  success ",
							(*iteripport)->IpPortName.c_str(),iterchannelAttach->first.c_str());
					}
					catch (const DataOnDemand::ObjectExistException &e)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"AttachChannel():channelname = %s  Object exist"
							"\t\t\t\t\t\t\t\t\t\t errorcode  = %s",
							iterchannelAttach->first.c_str(),e.ice_name().c_str());
					}
					catch (const DataOnDemand::StreamerException &e)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"AttachChannel():[channelname = %s ] "
							"DataOnDemand::StreamerException errorcode  = %s",
							iterchannelAttach->first.c_str(),e.ice_name().c_str());
						return FALSE;
					}
										
					if(channelattachinfo->nDataExchangeType ==
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
							writeLog(ZQ::common::Log::L_DEBUG, 
								"current Channel path is not exist[ path = %s ]",
								portfolder.c_str());
							
							if(CreateDirectory(portfolder.c_str(), NULL) == FALSE)		
							{
								int nError = GetLastError();
								char strError[500];
								
								GetErrorDescription(nError, strError);
								writeLog(ZQ::common::Log::L_DEBUG, 
									"CJMSPortManager::CreateChannelDirectory[ path = %s ]is "
									"error.GetLastError() = %d, ErrorDescription = %s",
									portfolder.c_str(),nError, strError);
								
								return FALSE;
							}
							writeLog(ZQ::common::Log::L_DEBUG, 
								"CJMSPortManager::CreateChannelDirectory[ path = %s ]  success ",
								portfolder.c_str());
						}
						else
						{
							writeLog(ZQ::common::Log::L_DEBUG, 
								"current channel path is exist[path = %s]",
								portfolder.c_str());
						}						
					}					
				}       
	}*/
    return true;
}
