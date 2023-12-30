#include "stdafx.h"
#include ".\jms.h"
#include "clog.h"
//#include "BaseSource.h"

//#include "..\include\header\jmshead.h"
extern  BOOL	g_bStop;
extern BOOL g_ConnectJBossIsOK;
extern CDODClientAgent *g_pDCAagent;
void ConnectionMonitor(int ErrType, void * lpData)
{
	if(g_pDCAagent->m_jms->m_bConnectionOK == FALSE)
	{
		Clog( LOG_DEBUG,_T("DODReConnectJBossThread is connecting JBoss...!"));
		return;
	}
     g_pDCAagent->m_jms->m_bConnectionOK = FALSE;
	 ResetEvent(CJmsProcThread::m_hReConnectJBoss);

	if (!g_pDCAagent->m_jms->m_bConnectionOK)
	{
		Clog( LOG_DEBUG,_T("CJMSConnector::ConnectOperation() - JBoss Connection break down"));

		g_ConnectJBossIsOK = FALSE;
        
		Clog( LOG_DEBUG,_T("Begin ReConnection JBoss - Connection process..."));
        
		while(!g_pDCAagent->ConnectionJBoss() && !g_bStop)
		{
			Clog( LOG_DEBUG,_T(" ReConnection JBoss error!") );
		}

	    Clog( LOG_DEBUG,_T("End ReConnection JBoss - Connection OK!"));
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
			Clog( LOG_DEBUG,_T(" jms == NULL error "));

			return;
		}

		if(jms->m_Connection.createSession(m_Session) == FALSE)
		{
			m_bError = TRUE;
			Clog( LOG_DEBUG,_T(" m_Connection.createSession error "));
			return ;
		}

		if(jms->m_JndiContext->createDestination(queueName,m_destination) == FALSE)
		{
			m_bError = TRUE;
			Clog( LOG_DEBUG,_T("  createDestination error "));
			return ;
		}
		if (bIsSend)
		{
			if(m_Session.createProducer(&m_destination,m_producer) == FALSE)
			{
				m_bError = TRUE;
				Clog( LOG_DEBUG,_T(" createProducer error "));
				return ;
			}
		}
		else
		{
			if(m_Session.createConsumer(&m_destination,m_consumer) == FALSE)
			{
				m_bError = TRUE;
				Clog( LOG_DEBUG,_T(" createConsumer error "));
				return ;
			}

		}
	}
	catch (...)
	{	
		Clog( LOG_DEBUG,_T(" CJMSBaseContent::CJMSBaseContent.exception ! "));
		m_bError = TRUE;
	}
}

CJMSBaseContent::~CJMSBaseContent() 
{

}


CJMS::CJMS(char *inStr)
{
	m_bConnectionOK = FALSE;
	m_pSource = NULL;
	m_JndiContext = NULL;
	strcpy(m_JbossIp,inStr);
	try
	{
		m_JndiContext = new Context(inStr,"org.jnp.interfaces.NamingContextFactory");  
	}
	catch (...) 
	{
		Clog( LOG_DEBUG,_T("m_JndiContext=new Context(inStr,org.jnp.interfaces.NamingContextFactory error "));
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

BOOL  CJMS::StartConnect(CDODClientAgent *pSource)
{ 
	CJMSBaseList::iterator it = m_QueueRecvList.begin();

	for(; it != m_QueueRecvList.end(); ++it)
	{  
		(*it)->m_destination.destroy();
		(*it)->m_Session.close();
		(*it)->m_consumer.close();
	}
    m_Connection.close();
    m_ConnectionFactory.Destroy();

	if (pSource)
	{
		m_pSource=pSource;
	}

	try
	{
		if(m_JndiContext->createConnectionFactory("ConnectionFactory",m_ConnectionFactory)==FALSE)
		{
			Clog( LOG_DEBUG,_T(" m_JndiContext->createConnectionFactory error "));
			return FALSE;
		}

		if(m_ConnectionFactory.createConnection(m_Connection)==FALSE)
		{
			Clog( LOG_DEBUG,_T(" m_ConnectionFactory.createConnection error "));
			return FALSE;
		}
		if(!m_Connection._connection)
		{
			Clog(LOG_DEBUG,_T("Invalid connection handle after createConnection"));
			return FALSE;
		}
		m_Connection.SetConnectionCallback(ConnectionMonitor,this);

		if(m_Connection.start() == FALSE)
		{
			Clog( LOG_DEBUG,_T(" m_Connection.start() error "));
			return FALSE;
		}
	}
	catch (...)
	{		
		Clog( LOG_DEBUG,_T(" BOOL CJMS::StartConnect.exception ! "));
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

BOOL CJMS::AddQueueOrTopic(BOOL bIsQueue,char *queueName,int nFomrat,CReceiveJmsMsg *Inparse)
{

	if (m_bConnectionOK == FALSE)	
		return FALSE;

	try
	{		
		if (bIsQueue)
		{
			if (nFomrat>INT_ONLY_SEND)
			{
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this,queueName,FALSE);
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
		}

		else
		{
			if (nFomrat>INT_ONLY_SEND)
			{
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this,queueName,FALSE);


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
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this, queueName, TRUE);


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
				CJMSBaseContent *QueueTopic = new CJMSBaseContent(this, queueName, TRUE);


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
		Clog( LOG_DEBUG,_T(" BOOL CJMS::AddQueueOrTopic.exception ! "));
		return FALSE;
	}
	return TRUE;
}

BOOL CJMS::SyncSendMsg(BOOL IsQueue,char *queueName,char * MsgData,CJMSTxMessage &responseMsg,int nTimeOut,char *intkey,int nPara1,char *strkey,char *cPara2)
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
					Clog( LOG_DEBUG,_T(" BOOL  CJMS::SendMsg.textMessageCreate error ! "));
					return FALSE;
				}

				if (intkey != NULL)
				{
					if(tempmsg.setIntProperty(intkey,nPara1) == FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setIntProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}

				if (strkey != NULL  && cPara2 != NULL)
				{
					if(tempmsg.setStringProperty(strkey,cPara2) == FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setStringProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}
				if (m_bConnectionOK == FALSE)	
					return FALSE;

				Requestor rr(&((*it)->m_Session),&((*it)->m_destination));

				if(rr.request(&tempmsg,responseMsg,nTimeOut,nTimeOut) == FALSE)
				{
					Clog( LOG_DEBUG,_T(" rr.request(&tempmsg,responseMsg, error ! "));
					return FALSE;
				}		
				if (responseMsg._message == NULL)
				{
					Clog( LOG_DEBUG,_T("Send msg to (%s) by sync,no response,timeout,error ! "),queueName);
					return FALSE;
				}
				return TRUE;
			}
		}
	}
	catch (...)
	{	
		Clog( LOG_DEBUG,_T(" BOOL CJMS::SyncSendMsg.exception ! "));
		return FALSE;
	}
	return FALSE;
}

BOOL CJMS::SendMsg(BOOL IsQueue,char *queueName,char * MsgData,char *intkey,int nPara1,char *strkey,char *cPara2)
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
					Clog( LOG_DEBUG,_T(" BOOL  CJMS::SendMsg.textMessageCreate error ! "));
					return FALSE;
				}

				if (intkey != NULL)
				{
					if(tempmsg.setIntProperty(intkey,nPara1) == FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setIntProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}

				if (strkey != NULL  && cPara2 != NULL)
				{
					if(tempmsg.setStringProperty(strkey,cPara2) == FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setStringProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}

				if((*it)->m_producer.send(&tempmsg) == FALSE)
				{
					Clog( LOG_DEBUG,_T(" m_producer.send error "));

					return FALSE;
				}
				return TRUE;
			}
		}				
	}
	catch (...)
	{	
		Clog( LOG_DEBUG,_T(" BOOL CJMS::SendMsg.exception ! "));
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


