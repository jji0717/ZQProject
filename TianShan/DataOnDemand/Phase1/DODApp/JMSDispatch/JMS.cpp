#include "stdafx.h"
#include ".\jms.h"
extern BOOL g_ConnectJBossIsOK;
extern BOOL g_bStop;
extern CJMSPortManager *g_jmsPortManager;
void ConnectionMonitor(int ErrType, void *lpData)
{		
	if(g_jmsPortManager->m_jms->m_bConnectionOK == FALSE)
	{
		writeLog(ZQ::common::Log::L_INFO,
			"DODReConnectJBossThread is connecting JBoss...!");
		return;
	}
	g_jmsPortManager->m_jms->m_bConnectionOK = FALSE;
	ResetEvent(CJmsProcThread::m_hReConnectJBoss);
	
	if (!g_jmsPortManager->m_jms->m_bConnectionOK)
	{
		writeLog(ZQ::common::Log::L_INFO, 
			"CJMSConnector::ConnectOperation() - JBoss Connection break down!");
		
		g_ConnectJBossIsOK = FALSE;
		
		writeLog(ZQ::common::Log::L_INFO,
			"Begin ReConnection JBoss - Connection process...");
		
		while(!g_jmsPortManager->ConnectionJBoss() && !g_bStop)
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"ReConnection JBoss error!");
		}
		if(!g_bStop)
		{	
			CJMSBaseList::iterator it = g_jmsPortManager->m_jms->m_QueueRecvList.begin();
			for(int i = 0 ; it != g_jmsPortManager->m_jms->m_QueueRecvList.end(); it++)
			{
				SetEvent(g_jmsPortManager->m_VecParser[i].MsgReceive->m_hStartReceive);
				i++;
			}			
		}

		writeLog(ZQ::common::Log::L_INFO, 
			"End ReConnection JBoss - Connection OK!");
	}
}
CJMSBaseContent::CJMSBaseContent(CJMS *jms,char *queueName,BOOL bIsSend) 
{
	m_bError = FALSE;
	strcpy(m_QueueName,queueName);
	
	try
	{
		if (jms == NULL)
		{
			m_bError = TRUE;
			writeLog(ZQ::common::Log::L_DEBUG, "jms == NULL error!");
			
			return;
		}
		
		if(jms->m_Connection.createSession(m_Session) == FALSE)
		{
			m_bError = TRUE;
			writeLog(ZQ::common::Log::L_DEBUG,
				"m_Connection.createSession error!");
			
			return ;
		}
		
		if(jms->m_JndiContext->createDestination
			                (queueName,m_destination) == FALSE)
		{
			m_bError = TRUE;
			writeLog(ZQ::common::Log::L_DEBUG, "createDestination error !");
			
			return ;
		}
		if (bIsSend)
		{
			if(m_Session.createProducer(&m_destination,m_producer) == FALSE)
			{
				m_bError = TRUE;
				writeLog(ZQ::common::Log::L_DEBUG, "createProducer error !");
				
				return ;
			}
		}
		else
		{
			if(m_Session.createConsumer(&m_destination,m_consumer) == FALSE)
			{
				m_bError = TRUE;
				writeLog(ZQ::common::Log::L_DEBUG, "createConsumer error!");
				
				return ;
			}
		}
	}
	catch (...)
	{	
		writeLog(ZQ::common::Log::L_DEBUG,
			"CJMSBaseContent::CJMSBaseContent.exception !");
		
		m_bError = TRUE;
	}
}

CJMSBaseContent::~CJMSBaseContent() 
{
	
}


CJMS::CJMS(char *inStr)
{
	m_bConnectionOK = FALSE;
    m_JndiContext = NULL;
    try
	{
		m_JndiContext = new Context(inStr,
			"org.jnp.interfaces.NamingContextFactory");  
	}
	catch (...) 
	{
		writeLog(ZQ::common::Log::L_DEBUG, 
		"m_JndiContext=new Context(inStr,org.jnp.interfaces.NamingContextFactory \
		error!");		
	}
}

CJMS::~CJMS()
{
	FreeAllList();
	
	if (m_JndiContext)
	{
		delete m_JndiContext;
		m_JndiContext=NULL;
	}
}

BOOL  CJMS::ConnectStop()
{ 
	bool bReturn = m_Connection.stop();
	m_Connection.close();
	return bReturn;
}

BOOL  CJMS::StartConnect()
{   
	m_Connection.close();
	m_ConnectionFactory.Destroy();

	try
	{
		if(m_JndiContext->createConnectionFactory
			("ConnectionFactory",m_ConnectionFactory)==FALSE)
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"m_JndiContext->createConnectionFactory error!");

			int nError = GetLastError();
			char strError[500];
			
			GetErrorDescription(nError, strError);
			
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMS::StartConnect() GetLastError() =%d,"
				"ErrorDescription = %s",nError, strError);
			return FALSE;
		}
		
		if(m_ConnectionFactory.createConnection(m_Connection)==FALSE)
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"m_ConnectionFactory.createConnection error!");

			int nError = GetLastError();
			char strError[500];
			
			GetErrorDescription(nError, strError);
			
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMS::StartConnect() GetLastError() =%d,"
				"ErrorDescription = %s",nError, strError);
			return FALSE;
		}
		if(!m_Connection._connection)
		{
			writeLog(ZQ::common::Log::L_DEBUG,
				"Invalid connection handle after createConnection!");
			int nError = GetLastError();
			char strError[500];
			
			GetErrorDescription(nError, strError);
			
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMS::StartConnect() GetLastError() =%d,"
				"ErrorDescription = %s",nError, strError);
			return FALSE;
		}
		m_Connection.SetConnectionCallback(ConnectionMonitor,NULL);
		
		if(m_Connection.start() == FALSE)
		{
			writeLog(ZQ::common::Log::L_DEBUG, 
				"m_Connection.start() error!");

			int nError = GetLastError();
			char strError[500];
			
			GetErrorDescription(nError, strError);
			
			writeLog(ZQ::common::Log::L_DEBUG,
				"CJMS::StartConnect() GetLastError() =%d,"
				"ErrorDescription = %s",nError, strError);

			return FALSE;
		}
	}
	catch (...)
	{		
		writeLog(ZQ::common::Log::L_DEBUG, 
			"BOOL CJMS::StartConnect.exception !");

		int nError = GetLastError();
		char strError[500];
		
		GetErrorDescription(nError, strError);
		
		writeLog(ZQ::common::Log::L_DEBUG,
			"CJMS::StartConnect() GetLastError() =%d,"
			"ErrorDescription = %s",nError, strError);
		
		return FALSE;
	}
	m_bConnectionOK = TRUE;
	return m_bConnectionOK;
}
BOOL CJMS::AssureUniquelyList(char *queueName,CJMSBaseList &listjms)
{
	std::list< CJMSBaseContent *>::iterator it;
	for( it = listjms.begin(); it != listjms.end();it++)
	{
		if (strcmpi((*it)->m_QueueName,queueName) == 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CJMS::AddQueueOrTopic(BOOL bIsQueue,char *queueName,int nFomrat,
						   CReceiveJmsMsg *Inparse)
{
	
	if (m_bConnectionOK == FALSE)	
		return FALSE;
	
	try
	{		
		if (bIsQueue)
		{
			if (nFomrat>INT_ONLY_SEND)
			{
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(
					this,queueName,FALSE);
				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				
				if(AssureUniquelyList(queueName,m_QueueRecvList) == FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				
			    Inparse->m_Consumer = &QueueTopic->m_consumer;
				Inparse->m_destination = &QueueTopic->m_destination;
				Inparse->m_Session = &QueueTopic->m_Session;

				if (!(*Inparse->m_Session).textMessageCreate("", Inparse->m_jmsTextMsg))
				{
                    return FALSE;
				}
				m_QueueRecvList.push_back(QueueTopic);
			}
			else
			{
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this,queueName,TRUE);
				
				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_QueueSendList) == FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				m_QueueSendList.push_back(QueueTopic);
			}
			
			
			if (nFomrat == INT_SENDANDRECE)
			{		
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(
					             this,queueName,TRUE);
				
				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_QueueSendList) == FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				m_QueueSendList.push_back(QueueTopic);
			}
		}
		
		else
		{
			if (nFomrat>INT_ONLY_SEND)
			{
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this,
					                    queueName,FALSE);
				
				
				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_TopicRecvList) == FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				
/*				if(QueueTopic->m_consumer.setMessageListener(Inparse) == FALSE)
				{
					return FALSE;
				}*/
				m_TopicRecvList.push_back(QueueTopic);
			}
			else
			{
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this,
					                                 queueName, TRUE);
				
				
				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName, m_TopicSendList) == FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				m_TopicSendList.push_back(QueueTopic);
			}
			
			
			if (nFomrat == INT_SENDANDRECE)
			{
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this,
					                                     queueName, TRUE);
				
				
				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_TopicSendList) == FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				m_TopicSendList.push_back(QueueTopic);
			}
		}
	}
	catch (...)
	{
		writeLog(ZQ::common::Log::L_DEBUG, 
			" BOOL CJMS::AddQueueOrTopic.exception !");
		
		return FALSE;
	}
	return TRUE;
}

BOOL CJMS::SyncSendMsg(BOOL IsQueue,char *queueName,char * MsgData,
					   CJMSTxMessage &responseMsg,int nTimeOut,char *intkey,
					   int nPara1,char *strkey,char *cPara2)
{
	if (m_bConnectionOK == FALSE)	
		return FALSE;
	
	try
	{		
		std::list< CJMSBaseContent *>::iterator it;
		CJMSTxMessage tempmsg;
		
		CJMSBaseList *baselist = NULL;
		if (IsQueue)
			baselist = &m_QueueSendList;
		else
			baselist = &m_TopicSendList;
		
		
		for( it = baselist->begin(); it != baselist->end();it++)
		{
			if (strcmpi((*it)->m_QueueName,queueName) == 0)
			{
				if((*it)->m_Session.textMessageCreate(MsgData,tempmsg) == FALSE)
				{
					writeLog(ZQ::common::Log::L_DEBUG, 
						"BOOL  CJMS::SendMsg.textMessageCreate error ! ");
					
					return FALSE;
				}
				
				if (intkey != NULL)
				{
					if(tempmsg.setIntProperty(intkey,nPara1) == FALSE)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"tempmsg.setIntProperty(intkey,nPara1) error !");
						
						return FALSE;
					}
				}
				
				if (strkey != NULL  && cPara2 != NULL)
				{
					if(tempmsg.setStringProperty(strkey,cPara2) == FALSE)
					{
						writeLog(ZQ::common::Log::L_DEBUG, 
							"tempmsg.setStringProperty(intkey,nPara1) error ! ");
						
						return FALSE;
					}
				}
				if (m_bConnectionOK == FALSE)	
					return FALSE;
				
				Requestor rr(&((*it)->m_Session),&((*it)->m_destination));
				
				if(rr.request(&tempmsg,responseMsg,nTimeOut,nTimeOut) == FALSE)
				{
					writeLog(ZQ::common::Log::L_DEBUG, 
						"rr.request(&tempmsg,responseMsg, error !");
					
					return FALSE;
				}		
				if (responseMsg._message == NULL)
				{
					
					writeLog(ZQ::common::Log::L_DEBUG, 
						"Send msg to (%s) by sync,no response,timeout,error ! ",
						queueName);
					
					return FALSE;
				}
				return TRUE;
			}
		}
	}
	catch (...)
	{			
		writeLog(ZQ::common::Log::L_DEBUG, "BOOL CJMS::SyncSendMsg.exception !");		
		return FALSE;
	}
	return FALSE;
}

BOOL CJMS::SendMsg(BOOL IsQueue,char *queueName,char * MsgData,char *intkey,
				   int nPara1,char *strkey,char *cPara2)
{
	if (m_bConnectionOK == FALSE)	
		return FALSE;
	
	try
	{		
		std::list< CJMSBaseContent *>::iterator it;
		
		CJMSTxMessage tempmsg;
		
		CJMSBaseList *baselist = NULL;
		if (IsQueue)
			baselist = &m_QueueSendList;
		else
			baselist = &m_TopicSendList;
		
		for( it = baselist->begin(); it != baselist->end();it++)
		{
			if (strcmpi((*it)->m_QueueName,queueName) == 0)
			{
				if((*it)->m_Session.textMessageCreate(MsgData,tempmsg) == FALSE)
				{
					writeLog(ZQ::common::Log::L_DEBUG, 
						"BOOL  CJMS::SendMsg.textMessageCreate error !");
					
					return FALSE;
				}
				
				if (intkey != NULL)
				{
					if(tempmsg.setIntProperty(intkey,nPara1) == FALSE)
					{
						writeLog(ZQ::common::Log::L_DEBUG,
							"tempmsg.setIntProperty(intkey,nPara1) error !");
						
						return FALSE;
					}
				}
				
				if (strkey != NULL  && cPara2 != NULL)
				{
					if(tempmsg.setStringProperty(strkey,cPara2) == FALSE)
					{
						
						writeLog(ZQ::common::Log::L_DEBUG, 
							"empmsg.setStringProperty(intkey,nPara1) error !");
						
						return FALSE;
					}
				}
				
				if((*it)->m_producer.send(&tempmsg) == FALSE)
				{
					
					writeLog(ZQ::common::Log::L_DEBUG,
						"m_producer.send error !");
					
					
					return FALSE;
				}
				return TRUE;
			}
		}				
	}
	catch (...)
	{	
		
		writeLog(ZQ::common::Log::L_DEBUG, " BOOL CJMS::SendMsg.exception ! ");
		
		return FALSE;
	}
	return FALSE;
}

//free all resources
void CJMS::FreeAllList()
{
	std::list< CJMSBaseContent *>::iterator it;
	while(m_QueueSendList.size())
	{
		it = m_QueueSendList.begin();
		if (*it)
		{
			delete (*it);
			(*it) = NULL;
		}
		m_QueueSendList.erase(it);
	}
	while(m_QueueRecvList.size())
	{
		it = m_QueueRecvList.begin();
		if (*it)
		{
			delete (*it);
			(*it) = NULL;
		}
		m_QueueRecvList.erase(it);
	}
	while(m_TopicSendList.size())
	{
		it = m_TopicSendList.begin();
		if (*it)
		{
			delete (*it);
			(*it) = NULL;
		}
		m_TopicSendList.erase(it);
	}
	while(m_TopicRecvList.size())
	{
		it = m_TopicRecvList.begin();
		if (*it)
		{
			delete (*it);
			(*it) = NULL;
		}
		m_TopicRecvList.erase(it);
	}
}


