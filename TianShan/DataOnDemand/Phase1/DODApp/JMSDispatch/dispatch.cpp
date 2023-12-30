// Dispatch.cpp 
//
#include "stdafx.h"
#include "messagemacro.h"
#include <stdlib.h>
#include <markup.h>
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
	m_portManager.clear();
	m_VecParser.clear();
	m_Pjmsprocthread = NULL;
	m_nUsingJBoss = 1;
	return; 
}
CJMSPortManager::~CJMSPortManager()
{
	
}
CJMSPortManager::Create( string strJBossIPPort, string ConfigQueueName,
						int ConfigMsgTimeOut,string strCacheDir, int nUsingJBoss)
{
	m_strCacheDir = strCacheDir;
	m_ProviderValue = strJBossIPPort.c_str();
	m_strConfigQueueName = ConfigQueueName;
    m_nConfigTimeOut = ConfigMsgTimeOut;
	m_Pjmsprocthread = new CJmsProcThread();
	m_nUsingJBoss = nUsingJBoss;
	
	if(NULL == m_Pjmsprocthread )
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager Create JmsProcThread (m_Pjmsprocthread) error");
		writeLog(ZQ::common::Log::L_ERROR,"CJMSPortManager::Create .error");
		return FALSE;
	}
	if(!m_Pjmsprocthread->init())
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager Initialize JmsProcThread::init() error");
		
		writeLog(ZQ::common::Log::L_ERROR,"CJMSPortManager::Create .error");
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
	::CoInitialize(NULL);

	CString        strCurDir;
	char           sModuleName[1025];
	int            nUsingJBoss = 0;
	DWORD dSize = GetModuleFileName(NULL,sModuleName,1024);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.ReverseFind('\\');
	strCurDir = strCurDir.Left(nIndex); //end with "\\"
    strCurDir = strCurDir + "\\LocalConfig.xml";
	
	if(m_nUsingJBoss)                             
	{
		writeLog(ZQ::common::Log::L_DEBUG, 
			"Begin Connect JBOSS (%s)",m_ProviderValue);
		
		m_jms = new CJMS(m_ProviderValue.GetBuffer(0));
		
		if(NULL == m_jms )
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"DODApp Initialize CJMS (m_jms) error!");
			
			return FALSE;
		}
		
		if(!ConnectionJBoss())
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"Connect JBOSS error.(%s)",m_ProviderValue);
			writeLog(ZQ::common::Log::L_ERROR,
				"CJMSPortManager::Initialize .error!");
			return FALSE;
		}
		
		m_ParsePortConfig = new  CReceiveJmsMsg(this);
		
		if(m_ParsePortConfig == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR, "Create JMSParser Object error!");
			return FALSE;
		}
		m_ParsePortConfig->m_strQueueName = m_strConfigQueueName.c_str();
		if(m_QueueManagement.QueueIsExist(
			m_strConfigQueueName.c_str(),INT_ONLY_SEND,TRUE) == FALSE)
		{
			if(m_jms->AddQueueOrTopic(TRUE,(char *)m_strConfigQueueName.c_str(),			
				INT_ONLY_SEND,m_ParsePortConfig) == FALSE)
			{
				writeLog(ZQ::common::Log::L_DEBUG, 
					"m_jms.AddOneSendQueue (%s) error!", 
					m_strConfigQueueName.c_str());
				writeLog(ZQ::common::Log::L_DEBUG, 
					"CJMSPortManager::Initialize .error!");
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"The queue named (%s) is registed in JBoss\n",
				m_strConfigQueueName.c_str());
		}
		else
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"The queue named (%s) has existed in JBoss\n",
				m_strConfigQueueName.c_str());
			
		}
		CJMSTxMessage tempMsg;
		while (1)
		{
			if(g_bStop)
				return  FALSE;
			CString ssaa = m_ParsePortConfig->SendGetConfig();
			
			writeLog(ZQ::common::Log::L_DEBUG, 
				"DODApp Send Get Port Configuration Message: (%s)",ssaa);
			
			if (m_jms->SyncSendMsg(TRUE,(char*)m_strConfigQueueName.c_str(),
				ssaa.GetBuffer(0),tempMsg,m_nConfigTimeOut,"MESSAGECODE",
				JMSMESSAGE_CONFIGURATION,"MESSAGECLASS","COMMAND")==FALSE)
			{
				writeLog(ZQ::common::Log::L_DEBUG, 
				"Send config_msg to queue(%s) by sync ,but the config_module"
				"has no response",
				m_strConfigQueueName.c_str());
				
				Sleep(5);
			}
			else
				break;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG, 
			"CJMSPortManager  Get Port Configuration successful!");
		
		writeLog(ZQ::common::Log::L_DEBUG, 
			"CJMSPortManager  Begin Parser Port Configuration ...!");
		
		if(m_ParsePortConfig->parseNotification(&tempMsg))
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager  Parser Port Configuration error !");
			return FALSE;
		}
		
		if(!AddChannelQueue())
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager  Parser Port Configuration error !");
			return FALSE;
		}
		
		if(!CheckSyn())
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager  Parser Port Configuration error !");
			return FALSE;
		}
		
		if(!ApplyParameter())
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager  Parser Port Configuration error !");
			return FALSE;
		}
		
		if(!SetMessage())
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager  Parser Port Configuration error !");
			return FALSE;
		}
	}
	else
	{
		m_ParsePortConfig = new  CReceiveJmsMsg(this);
		
		if(m_ParsePortConfig == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR, "Create JMSParser Object error!");
			return FALSE;
		}
		m_ParsePortConfig->m_strQueueName = m_strConfigQueueName.c_str();
		
		CMarkup m_XmlDOM;
		
		m_XmlDOM.Load(strCurDir);
		CString sstr = "";		
		sstr = m_XmlDOM.GetDoc();
		
		if(!m_ParsePortConfig->ReceiverPortConfigMsg(sstr.GetBuffer(0),0,NULL))
			return FALSE;
		
		if(!CheckSyn())
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager  Parser Port Configuration error !");
			return FALSE;
		}
		
		if(!ApplyParameter())
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager  Parser Port Configuration error !");
			return FALSE;
		}
	}
	
	writeLog(ZQ::common::Log::L_DEBUG,
		"CJMSPortManager  Parser Port Configuration success !");
	
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
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" CJMSPortManager::Delete Jmsprocthread success!");
		
		if(m_ParsePortConfig)
		{
			delete m_ParsePortConfig;
			m_ParsePortConfig = NULL;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" CJMSPortManager:: Delete ParsePortConfig success!");
		
		for(UINT i = 0 ; i < m_portManager.size(); i++)
		{
			DODPORT *pPort = m_portManager[i];
			for(UINT j = 0; j < pPort->channelvector.size(); j ++)
			{
				delete pPort->channelvector[j];
				pPort->channelvector[j] = NULL;
			}
			
			for(UINT k = 0; k < pPort->ipportvector.size(); k ++)
			{
				delete pPort->ipportvector[k];
				pPort->ipportvector[k] = NULL;
			}
			
			delete m_portManager[i];
			m_portManager[i] =  NULL;
		}	
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" CJMSPortManager::Delete PortManager success!");
		
		::CoUninitialize();
	}
	catch (...)
	{
       	int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		writeLog(ZQ::common::Log::L_DEBUG,
		"CJMSPortManager::UnInitialize() GetLastError() =%d,"
		"ErrorDescription = %s",nError, strError);
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" CJMSPortManager::UnInitialize() error!");
		return FALSE;
	}
	
	writeLog(ZQ::common::Log::L_DEBUG,
		" CJMSPortManager::UnInitialize() success!");
	
	return TRUE;
}
BOOL CJMSPortManager::ConnectionJBoss(void)
{	
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
				CJMSBaseList::iterator it = m_jms->m_QueueRecvList.begin();
				for(; it != m_jms->m_QueueRecvList.end(); it++)
				{					
					if(m_jms->m_Connection.createSession((*it)->m_Session) == FALSE)
					{
						(*it)->m_bError = TRUE;
						writeLog(ZQ::common::Log::L_ERROR,
						"ReConnect Jboss Error. m_Connection.createSession "
						"error!queuename = %s ",m_VecParser[i].QueueName);
						return FALSE ;
					}
					
					if(m_jms->m_JndiContext->createDestination((*it)->m_QueueName,
						(*it)->m_destination) == FALSE)
					{
						(*it)->m_bError = TRUE;
						writeLog(ZQ::common::Log::L_ERROR,
						"ReConnect Jboss Error. createDestination error! "
						"queuename = %s ",m_VecParser[i].QueueName);
						return FALSE;
					}
					
					
					if((*it)->m_Session.createConsumer(&(*it)->m_destination,
						(*it)->m_consumer) == FALSE)
					{
						(*it)->m_bError = TRUE;
						writeLog(ZQ::common::Log::L_ERROR,
						" ReConnect Jboss Error.createConsumer error! "
						"queuename = %s ",m_VecParser[i].QueueName);
						return FALSE;
					}
					
					m_VecParser[i].MsgReceive->m_Consumer = &(*it)->m_consumer;
					m_VecParser[i].MsgReceive->m_destination = &(*it)->m_destination;
					m_VecParser[i].MsgReceive->m_Session = &(*it)->m_Session;
					
					if (!(*it)->m_Session.textMessageCreate("", 
						m_VecParser[i].MsgReceive->m_jmsTextMsg))
					{
						writeLog(ZQ::common::Log::L_ERROR,
						"  ReConnect Jboss Error.textMessageCreate error! "
						"queuename = %s ",m_VecParser[i].QueueName);
						return false;
					}
					
					writeLog(ZQ::common::Log::L_ERROR,
						"[%s]: Create Receive Consumer success!",
						m_VecParser[i].QueueName);	 
					i++;
				}
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"DODApp Connect JBOSS successful.(%s)\n",m_ProviderValue);
			m_bReSetMsgListener = TRUE;
			break;
		}		
		else
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"DODApp Connect JBOSS error.(%s)",m_ProviderValue);
			Sleep(5000);
		}	
	}
	return TRUE;
}

BOOL CJMSPortManager::AddChannelQueue()
{
	if(1 > (int)(m_portManager.size()))
	{
		writeLog(ZQ::common::Log::L_ERROR, 
		"DODApp found current PortVector.size is zero, "
		"parse config content error!");
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::AddChannelQueue(): AddChannelQueue Error!\n!");
		
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
			"DODApp Initialize CReceiveJmsMsg(m_VecParser[%d].MsgParser)"
			"Error!",i);
			writeLog(ZQ::common::Log::L_ERROR, 
				"CJMSPortManager::AddChannelQueue(): AddChannelQueue Error!");
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
				"CJMSPortManager:: AddOneReceiveQueue(queuename = (%s) "
				"error!",m_VecParser[i].QueueName);
				writeLog(ZQ::common::Log::L_ERROR, 
					"CJMSPortManager:: AddChannelQueue(): AddChannelQueue Error!");
				return FALSE;
			}
			if(!m_VecParser[i].MsgReceive->init())
			{
				writeLog(ZQ::common::Log::L_ERROR,
					"CJMSPortManager::Create ReceiverMsg Thread error.",
					m_VecParser[i].QueueName);
				writeLog(ZQ::common::Log::L_ERROR,
					"CJMSPortManager::AddChannelQueue(): AddChannelQueue Error!" );
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"The queue named (%s) is registed in JBoss\n" ,
				m_VecParser[i].QueueName);
		}
		else
			writeLog(ZQ::common::Log::L_DEBUG, 
			"The queue named (%s) has existed in JBoss\n" ,
			m_VecParser[i].QueueName);
	}
	
	Sleep(2000);
	
	CPORTVECTOR::iterator iter;
	
	for(iter = m_portManager.begin(); iter != m_portManager.end(); iter++)
	{
		
		DODPORT *pPort = *iter;
		
		if(pPort == NULL)
		{
			return FALSE;
		}
		
		for(UINT i = 0; i < pPort->channelvector.size(); i++)
		{
			CHANNELINFO *channel = pPort->channelvector[i];
			if(channel == NULL)
			{
				return FALSE;
			}	
			
			if (channel->strQueueName.size() == 0)
			{
				writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager:: channel->m_QueueName.GetLength() == 0,"
				"portname = %s",pPort->portname.c_str());
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
					writeLog(ZQ::common::Log::L_DEBUG, 
						"[%s-%s]:: AddOneSendQueue(queuename = %s) error.",
						pPort->portname.c_str(),channel->ChanneName.c_str(),
						channel->strQueueName.c_str());
					writeLog(ZQ::common::Log::L_DEBUG, 
						"CJMSPortManager::AddChannelQueue(): AddChannelQueue Error!");
					return FALSE;
				}
				writeLog(ZQ::common::Log::L_DEBUG, 
					"[%s-%s]::AddSendQueue ok,queuename = %s",pPort->portname.c_str(),
					channel->ChanneName.c_str(),channel->strQueueName.c_str());
			}
			else
				writeLog(ZQ::common::Log::L_DEBUG, 
				"[%s-%s]::queuename = %s is exist",pPort->portname.c_str(),
				channel->ChanneName.c_str(),channel->strQueueName.c_str());			
		}
	}	
	return TRUE;
}
BOOL  CJMSPortManager::SetMessage()
{
	CString strCurDir,sTmp;
	if(1 > (int)(m_portManager.size()))
	{
		writeLog(ZQ::common::Log::L_DEBUG, 
		"DODApp found current PortVector.size is zero, parse config "
		"content error!");
		writeLog(ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::SetMessage(): SetMessage Error!");		
		return FALSE;
	}
	
	if(g_bStop)
		return FALSE;
	
	for(UINT i = 0; i <m_VecParser.size(); i++)
	{
		writeLog(ZQ::common::Log::L_DEBUG, 
			"[CReceiveJmsMsg::QueueName = %s] m_hStartReceive = TRUE!",
			m_VecParser[i].QueueName);
		SetEvent(m_VecParser[i].MsgReceive->m_hStartReceive);
	}
	
	CPORTVECTOR::iterator iter;
	
	for(iter = m_portManager.begin(); iter != m_portManager.end(); iter++)
	{		
		DODPORT *pPort = *iter;
		
		if(pPort == NULL)
		{
			return FALSE;
		}
		
		if(g_bStop)
			return FALSE;
		for(UINT i = 0; i < pPort->channelvector.size(); i++)
		{
			if(g_bStop)
				return FALSE;
			
			CHANNELINFO *channel = pPort->channelvector[i];
			if(channel == NULL)
			{
				return FALSE;
			}	
			
			if (channel->nDataExchangeType == DataOnDemand::dodLocalFolder ||
				channel->nDataExchangeType == DataOnDemand::dodMessage)
				continue;
			
			if (channel->strQueueName.size() == 0)
			{
				writeLog(ZQ::common::Log::L_DEBUG, 
					"SetMessage() channel->strQueueName.GetLength()==0 portname=%s",
					pPort->portname.c_str());
				continue;
			}
			
			if (m_sDataTypeInitial.GetLength() > 0)
			{
				int nIndexDataType = m_sDataTypeInitial.Find(
					channel->strdataType.c_str());
				
				writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::m_sDataTypeInitial.Find m_sDataTypeInitial "
				"is:%s :channel->strdataType = %s\n",
				m_sDataTypeInitial,channel->strdataType.c_str());
				
				if( nIndexDataType >= 0)
				{	
					strCurDir = m_ParsePortConfig->GetDataTypeInitialMsg(
						(char *)channel->strdataType.c_str(),1);
					
					CJMSTxMessage tempMsg;
					sTmp.Format("queue/%s",channel->strQueueName.c_str());
					if (m_jms->SyncSendMsg(TRUE,sTmp.GetBuffer(0),
						strCurDir.GetBuffer(0),tempMsg,5000,"MESSAGECODE",
						FIRSTDATATYPEMESSAGE,"MESSAGECLASS","COMMAND")==FALSE)
					{
						writeLog(ZQ::common::Log::L_DEBUG, 
						"[%s- %s] SyncSendMsg :send Data Type Initial msg to "
						"queue(%s),error",pPort->portname.c_str(),
						channel->ChanneName.c_str(),channel->strQueueName.c_str());
						Sleep(5);
					}
					else
					{
						writeLog(ZQ::common::Log::L_DEBUG, 
						"[%s - %s] SyncSendMsg :send Data Type Initial msg "
						"to queue(%s) ok",pPort->portname.c_str(),
						channel->ChanneName.c_str(),channel->strQueueName.c_str());				
						int nLenDataType = channel->strdataType.size();
						m_sDataTypeInitial.Delete(nIndexDataType,nLenDataType);
					}
				}
			}
			strCurDir = m_ParsePortConfig->SendGetFullDateMsg(pPort->groupId,
				(char *)channel->strdataType.c_str(),0);
			
			sTmp.Format("queue/%s",channel->strQueueName.c_str());
			
			if (m_jms->SendMsg(TRUE,sTmp.GetBuffer(0),strCurDir.GetBuffer(0),
				"MESSAGECODE",channel->nMessageCode,"MESSAGECLASS",
				"NOTIFICATION")==FALSE)
			{
				writeLog(ZQ::common::Log::L_DEBUG,
					"[%s - %s] PtopSend( SendGetFullDateMsg,(%s) error",
					pPort->portname.c_str(),channel->ChanneName.c_str(),sTmp);
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"[%s - %s] PtopSend :send getfulldata msg to queue(%s) ok",
				pPort->portname.c_str(),channel->ChanneName.c_str(),sTmp);
		}
	}
	
	return TRUE;
}
BOOL CJMSPortManager::ApplyParameter()
{
	::DataOnDemand::DestinationPrx destinationprx = NULL;
	::DataOnDemand::DestInfo destinfo;
	::DataOnDemand::DataPublisherPrx dataPrx;
	
	if(1 > (int)(m_portManager.size()))
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"DODApp found current PortVector.size is zero,"
			"parse config content error!");
		
		writeLog(ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::ApplyParameter(): SetMessage Error!");
		
		return FALSE;
	}
	CPORTVECTOR::iterator iter;	
	for(iter = m_portManager.begin(); iter != m_portManager.end(); iter++)
	{
		DODPORT *pPort = (*iter);
		
		if(pPort == NULL)
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CJMSPortManager::ApplyParameter() error:Port is NULL!");
			return FALSE;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" ApplyParameter():current PortName = %s",pPort->portname.c_str());
		
		if(!ParserIPandPort(pPort))
			return FALSE;
		
		destinfo.name = pPort->portname;
		destinfo.groupId = pPort->groupId;
		destinfo.destAddress = pPort->destAddress;
		destinfo.pmtPid = pPort->pmtPid;
		destinfo.totalBandwidth = pPort->totalBandWidth;
		
		try
		{	
            dataPrx = GetDataPublisherPrx();
			
			if(dataPrx == NULL)
			{
				writeLog(ZQ::common::Log::L_ERROR,"Get dataPubliserPrx Error.");
				return FALSE;
			}
			
			try
			{
				destinationprx = dataPrx->getDestination(pPort->portname);	    			
			}	
			catch (Ice::ObjectNotExistException& )
			{				
				destinationprx = dataPrx->createDestination(pPort->portname,destinfo);
				if(destinationprx == NULL)
				{
					writeLog(ZQ::common::Log::L_ERROR, 
						"createDestination error, portname = %s!", 
						pPort->portname.c_str());
					return FALSE;
				}
			}
			
			if(!CreateChannel(pPort,destinationprx))
				return FALSE;
			
			try
			{
				destinationprx->serve();
			}
			//			catch(DataOnDemand::StreamInvalidState)
			catch (DataOnDemand::StreamerException* e)
			{
				writeLog(ZQ::common::Log::L_ERROR, 
					"Serve error, portname = %s!, StreamerException errorcode = %s,",
					pPort->portname.c_str(), e->ice_name().c_str());
				return FALSE;
			}
			
		}
		catch (const ::TianShanIce::InvalidParameter & ex) 
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"ApplyParameter:TianShanIce::InvalidParameter errorcode = %s",
				ex.message);
			return FALSE;
		}  
		catch (const ::Ice::Exception & ex) 
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"ApplyParameter: Ice::Exception errorcode = %s",
				ex.ice_name().c_str());
			return FALSE;
		} 
	}
	
	return TRUE;
}
BOOL CJMSPortManager::CreateChannel(DODPORT* pPort, 
									DataOnDemand::DestinationPrx destinationprx)
{
	::DataOnDemand::ChannelInfo channelinfo;
	::DataOnDemand::ChannelPublishPointPrx channelprx = NULL;
	::DataOnDemand::DataPublisherPrx dataPrx;
	TianShanIce::Properties props;
	char PackMode[10];
	if(1 > (int)pPort->channelvector.size())
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"DODApp found current PortName = %s:channelvector.size is zero!",
			pPort->portname.c_str());
		writeLog(ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::CreateChannel(): CreateChannel Error!");		
		return FALSE;
	}
	
	CHANNELVECTOR::iterator iter;
	for(iter = pPort->channelvector.begin(); 
	iter != pPort->channelvector.end(); iter++)
	{
		CHANNELINFO *pChannelInfo = (*iter);
		if(pChannelInfo == NULL)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CreateChannel() error:Channel is NULL!");
			return FALSE;
		}
		ParserChannelType(&channelinfo.dataTypes, pChannelInfo->strdataType);
		channelinfo.name = pChannelInfo->ChanneName;
		channelinfo.encrypt = pChannelInfo->nEncrypted;
		channelinfo.streamId = pChannelInfo->nstreamId;
		channelinfo.streamType = pChannelInfo->streamType;
		channelinfo.subchannelCount = pChannelInfo->nStreamCount;
		channelinfo.tag = ConvertTag(pChannelInfo->Tag);
		
		pChannelInfo->strCacheDir = m_strCacheDir + pChannelInfo->ChanneName;
		
		BYTE tmode = 0;
		
		if (pChannelInfo->nChannelType)
			tmode += 4;
		
		if (pChannelInfo->nSendWithDestination)
			tmode += 2;
		else
			tmode += 1;
		sprintf(PackMode, "%d", tmode);
		
		try
		{
			dataPrx = GetDataPublisherPrx();
			if(dataPrx == NULL)
			{
				writeLog(ZQ::common::Log::L_ERROR,"Get dataPubliserPrx Error.");
				return FALSE;
			}
			try
			{ 
				channelprx = dataPrx->getChannel(pChannelInfo->ChanneName);
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
						(pChannelInfo->ChanneName,channelinfo);
					if(channelprx == NULL)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
						"CreateChannel():CreateMsgChannel error,portname = "
						"%s, channelname = %s !",pPort->portname.c_str(),
						pChannelInfo->ChanneName.c_str());
						return FALSE;
					}
					
					props = channelprx->getProperties();
					
					props["Detect"] = "0";
					props["DetectInterval"] = "0";
					props["DataExchangeType"] = "1" ;
					props["PackagingMode"] = PackMode;
					props["Path"] =  pChannelInfo->strCacheDir;
					
					channelprx->setProperties(props);				
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
							pChannelInfo->ChanneName,channelinfo,
							pChannelInfo->strCacheDir);
						if(channelprx == NULL)
						{
							writeLog(ZQ::common::Log::L_DEBUG,
							"CreateChannel():createLocalFolderChannel error,"
							"portname = %s,channelname = %s !",
							pPort->portname.c_str(),
							pChannelInfo->ChanneName.c_str());
							return FALSE;
						} 
						
						props = channelprx->getProperties();
						
						props["Detect"] = "1";
						props["DetectInterval"] = "5000";
						props["DataExchangeType"] = "2" ;
						props["PackagingMode"] = PackMode ;
						props["Path"] =  pChannelInfo->strCacheDir;
						
						channelprx->setProperties(props);						
					}					
				}
				else
				{
					if (channelprx == NULL)
					{
                        channelprx = dataPrx->createShareFolderChannel(
							pChannelInfo->ChanneName,channelinfo);	
						if(channelprx == NULL)
						{
							writeLog(ZQ::common::Log::L_DEBUG,
							"CreateChannel():createShareFolderChannel error,portname = %s, "
							"channelname = %s !",pPort->portname.c_str(), 
							pChannelInfo->ChanneName.c_str());
							return FALSE;
						} 
						props = channelprx->getProperties();
						
						props["Detect"] = "0";
						props["DetectInterval"] = "0";
						props["DataExchangeType"] = "0" ;
						props["PackagingMode"] = PackMode;
						props["Path"] =  pChannelInfo->strCacheDir;
						
						channelprx->setProperties(props);
					}
				}				
			}
			
			try
			{	
				destinationprx->attachChannel(pChannelInfo->ChanneName,
					pChannelInfo->nChannelRate, pChannelInfo->nRepeatetime);
			}
			catch (const DataOnDemand::ObjectExistException &e)
			{
				writeLog(ZQ::common::Log::L_DEBUG,
					"CreateChannel:attachChannel Object exist! errorcode  = %s",
					e.ice_name().c_str());
			}
		}  
		
		catch (const ::TianShanIce::InvalidParameter & ex) 
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CreateChannel:TianShanIce::InvalidParameter errorcode = %s",
				ex.message);
			return FALSE;
		} 
		catch( const DataOnDemand::StreamerException& ex)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CreateChannel: DataOnDemand::StreamerException errorcode = %s",
				ex.ice_name().c_str());
			return FALSE;
		}
		catch (const ::Ice::Exception & ex) 
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CreateChannel: Ice::Exception errorcode = %s",
				ex.ice_name().c_str());
			return FALSE;
		} 
		
		if(pChannelInfo->nDataExchangeType == ::DataOnDemand::dodLocalFolder)
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"CreateChannel():This is a LocalFolder CachingDir = %s.",
				pChannelInfo->strQueueName.c_str());
			continue;
		}
		if(!DirectoryExist(pChannelInfo->strCacheDir.c_str()))
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"current Channel path is not exist[ path = %s ]",
				pChannelInfo->strCacheDir.c_str());
			
			if (CreateDirectory(pChannelInfo->strCacheDir.c_str(), NULL) == FALSE)		
			{
				int nError = GetLastError();
				char strError[500];
				
				GetErrorDescription(nError, strError);
				writeLog(ZQ::common::Log::L_DEBUG, 
				"[%s]CJMSPortManager::CreateChannelDirectory[ path = %s ]is "
				"error.GetLastError() = %d, ErrorDescription = %s",
				pChannelInfo->ChanneName.c_str(),pChannelInfo->strCacheDir.c_str(),
				nError, strError);
				
				return FALSE;
			}
			writeLog(ZQ::common::Log::L_DEBUG, 
				"[%s]CJMSPortManager::CreateChannelDirectory[ path = %s ]  success !",
				pChannelInfo->ChanneName.c_str(),pChannelInfo->strCacheDir.c_str());
		}
		else
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"current channel path is exist[path = %s]",
				pChannelInfo->strCacheDir.c_str());
		}
		
	}
	return TRUE;
}
BOOL CJMSPortManager::ParserIPandPort(DODPORT* pPort)
{
	char strIpPort[30]="\0";
	string strtemp = "";
    if(1 > (int)pPort->ipportvector.size())
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"DODApp found current ipportvector.size is zero!");
		writeLog(ZQ::common::Log::L_DEBUG, 
			"CJMSPortManager::ParserIPandPort(): ParserIPandPort Error!");		
		return FALSE;
	}
	
	IPPORTVECTOR::iterator iter;
	for(iter = pPort->ipportvector.begin(); 
	iter != pPort->ipportvector.end(); iter++)
	{
		IPPORT *pIpPort = (*iter);
		if(pIpPort == NULL)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::ParserIPandPort() error:IpPort is NULL!");
			return FALSE;
		}
		switch(pIpPort->nSendType)
		{
		case 0: 
            sprintf(strIpPort,"TCP:%s:%d;",pIpPort->strIp.c_str(),pIpPort->nPort);
			break;
		case 2:
			sprintf(strIpPort,"MULITCAST:%s:%d;",pIpPort->strIp.c_str(),pIpPort->nPort);
			break;
		default:
			sprintf(strIpPort,"UDP:%s:%d;",pIpPort->strIp.c_str(),pIpPort->nPort);
			break;
		}
		strtemp += strIpPort;
	}   
	pPort->destAddress = strtemp;
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
	if(1 > (int)(m_portManager.size()))
	{
		writeLog(ZQ::common::Log::L_ERROR, 
		"DODApp found current PortVector.size is zero,"
		"parse config content error!");
		writeLog(ZQ::common::Log::L_ERROR,
			"CJMSPortManager::CheckSyn(): CheckSyn Error!");		
		return FALSE;
	}
	
	::DataOnDemand::DataPublisherPrx dataPrx;
	::DataOnDemand::DestinationPrx destprx;
	::DataOnDemand::ChannelPublishPointPrx channelprx;
	
	::TianShanIce::StrValues dests;
	::TianShanIce::StrValues channels;
	
	::TianShanIce::StrValues::iterator destsitor;
	::TianShanIce::StrValues::iterator channelsitor;
	
	dataPrx = GetDataPublisherPrx();
	
	if(dataPrx == NULL)
	{
		writeLog(ZQ::common::Log::L_ERROR,"Get dataPubliserPrx Error.");
		return FALSE;
	}

	try
	{		
		dests = dataPrx->listDestinations();

		writeLog( ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::checkSync():destination size = %d",
			dests.size());
		
		for(destsitor = dests.begin();destsitor != dests.end(); destsitor++) 
		{
			writeLog( ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::checkSync():destination name  = %s",
				(*destsitor).c_str());
		}
		
		channels = dataPrx->listChannels();

		writeLog( ZQ::common::Log::L_DEBUG,
			"CJMSPortManager::checkSync():channnel size = %d",
			channels.size());
		
		for(channelsitor = channels.begin();channelsitor != channels.end(); 
		                   channelsitor++) 
		{
			writeLog( ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::checkSync():Channnel name  = %s",
				(*channelsitor).c_str());
		}
						   
		CPORTVECTOR::iterator iterport;	
		for(iterport = m_portManager.begin(); iterport != m_portManager.end(); 
		iterport++)
		{
			writeLog( ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::checkSync():Port name  = %s",
				(*iterport)->portname.c_str());

			destsitor = std::find(dests.begin(), 
				dests.end(), (*iterport)->portname);
			
			if( destsitor == dests.end())
				continue;
			
			dests.erase(destsitor);
			
			CHANNELVECTOR::iterator iterchannel;
			for(iterchannel = (*iterport)->channelvector.begin(); 
			iterchannel != (*iterport)->channelvector.end(); iterchannel++)
			{
				writeLog( ZQ::common::Log::L_DEBUG,
					"CJMSPortManager::checkSync():channel name  = %s",
					(*iterchannel)->ChanneName.c_str());

				channelsitor = std::find(channels.begin(), 
					channels.end(), (*iterchannel)->ChanneName);
				
				if( channelsitor == channels.end())
					continue;
				
				channels.erase(channelsitor);
			}
		}
		try
		{
			if(!channels.empty())
			{
				for(channelsitor = channels.begin(); 
				channelsitor!= channels.end(); channelsitor++)
				{
					writeLog(ZQ::common::Log::L_DEBUG, 
						"checkSync:[Name = %s]destroy channel!",
						(*channelsitor).c_str());
					
					channelprx = dataPrx->getChannel(*channelsitor);
					channelprx->destroy();
					
					writeLog(ZQ::common::Log::L_DEBUG, 
						"checkSync:[Name = %s]destroy channel success!",
						(*channelsitor).c_str());
				}
			}
		}
		catch (const Ice::ObjectNotExistException &)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CheckSyn getChannel = %s"
				"Ice::ObjectNotExistException",(*channelsitor).c_str());
			return FALSE;
		}
		catch (const Ice::Exception& ex) 
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CheckSyn Ice::Exception errorMsg = %s",
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
					writeLog(ZQ::common::Log::L_DEBUG, 
						"checkSync:[Name = %s]destroy destinaton!",
						(*destsitor).c_str());
					
					destprx = dataPrx->getDestination(*destsitor);
					destprx->stop();
					destprx->destroy();
					
					writeLog(ZQ::common::Log::L_DEBUG, 
						"checkSync:[Name = %s]destroy destinaton success!",
						(*destsitor).c_str());
				}
			}
		}
		catch(const Ice::ObjectNotExistException &)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CheckSyn getDestination = %s"
				"Ice::ObjectNotExistException",(*destsitor).c_str());
			return FALSE;
		}
		catch (const Ice::Exception& ex) 
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMSPortManager::CheckSyn Ice::Exception errorMsg = %s",
				ex.ice_name().c_str());
			return FALSE;
		}		
	}
	catch(...)
	{
		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		writeLog(ZQ::common::Log::L_DEBUG,
		"CJMSPortManager::CheckSyn() GetLastError() =%d,"
		"ErrorDescription = %s",nError, strError);
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" CJMSPortManager::CheckSyn() error!");
		return FALSE;
	}
	
	writeLog(ZQ::common::Log::L_DEBUG, 
		"CJMSPortManager::checkSync  success!");
	
	return TRUE;
}