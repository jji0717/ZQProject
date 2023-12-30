#include "stdafx.h"
#include ".\jms.h"
#include "clog.h"

//#include "..\include\jmshead.h"

//extern BOOL g_ConnectJBossIsOK;

void ConnMonitor(int ErrType, void * lpData)
{
	CJMS *pMonitor = (CJMS*)lpData;
	if (pMonitor->m_bConnectionOK)
	{
		Clog( LOG_DEBUG,_T("CJMSConnector::ConnectMonitor() - Connection break down"));
		//g_ConnectJBossIsOK=FALSE;
		//pMonitor->m_pSource->ReConnectJBoss();
	}
	pMonitor->m_bConnectionOK=FALSE;
}
CJMSBaseContent::CJMSBaseContent(CJMS *jms,char *queueName,BOOL bIsSend) 
{
	m_bError=FALSE;
	strcpy(m_QueueName,queueName);

	try
	{
		if (jms==NULL)
		{
			m_bError=TRUE;
			Clog( LOG_DEBUG,_T(" jms==NULL error "));

			return;
		}

		if(jms->m_Connection.createSession(m_Session)==FALSE)
		{
			m_bError=TRUE;
			Clog( LOG_DEBUG,_T(" m_Connection.createSession error "));
			return ;
		}

		if(jms->m_JndiContext->createDestination(queueName,m_destination)==FALSE)
		{
			m_bError=TRUE;
			Clog( LOG_DEBUG,_T("  createDestination error "));
			return ;
		}
		if (bIsSend)
		{
			if(m_Session.createProducer(&m_destination,m_producer)==FALSE)
			{
				m_bError=TRUE;
				Clog( LOG_DEBUG,_T(" createProducer error "));
				return ;
			}
		}
		else
		{
			if(m_Session.createConsumer(&m_destination,m_consumer)==FALSE)
			{
				m_bError=TRUE;
				Clog( LOG_DEBUG,_T(" createConsumer error "));
				return ;
			}

		}
	}
	catch (...)
	{	
		Clog( LOG_DEBUG,_T(" CJMSBaseContent::CJMSBaseContent.exception ! "));
		m_bError=TRUE;
	}
}

CJMSBaseContent::~CJMSBaseContent() 
{

}


CJMS::CJMS(char *inStr)
{
	m_bConnectionOK=FALSE;
	m_pSource=NULL;
	try
	{
		m_JndiContext=new Context(inStr,"org.jnp.interfaces.NamingContextFactory");  
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


BOOL  CJMS::StartConnect(CBaseSource *pSource)
{ 
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
		//m_Connection.SetConnectionCallback(ConnMonitor,this);
		if(m_Connection.start()==FALSE)
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
	m_bConnectionOK=TRUE;
	return m_bConnectionOK;
}
BOOL CJMS::AssureUniquelyList(char *queueName,CJMSBaseList &listjms)
{
	std::list< CJMSBaseContent *>::iterator it;
	for( it = listjms.begin(); it != listjms.end();it++)
	{
		if (strcmpi((*it)->m_QueueName,queueName)==0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CJMS::AddQueueOrTopic(BOOL bIsQueue,char *queueName,int nFomrat,CJMSParser *Inparse)
{
	if (m_bConnectionOK==FALSE)	
		return FALSE;

	try
	{		
		if (bIsQueue)
		{
			if (nFomrat>INT_ONLY_SEND)
			{
				CJMSBaseContent *QueueTopic=new CJMSBaseContent(this,queueName,FALSE);
				if (QueueTopic->m_bError)
				{
					return FALSE;
				}

				if(AssureUniquelyList(queueName,m_QueueRecvList)==FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}

				if(QueueTopic->m_consumer.setMessageListener(Inparse)==FALSE)
				{
					return FALSE;
				}
				m_QueueRecvList.push_back(QueueTopic);
			}
			else
			{
				CJMSBaseContent *QueueTopic=new CJMSBaseContent(this,queueName,TRUE);

				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_QueueSendList)==FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				m_QueueSendList.push_back(QueueTopic);
			}


			if (nFomrat==INT_SENDANDRECE)
			{		
				CJMSBaseContent *QueueTopic=new CJMSBaseContent(this,queueName,TRUE);

				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_QueueSendList)==FALSE)
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
				CJMSBaseContent *QueueTopic=new CJMSBaseContent(this,queueName,FALSE);


				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_TopicRecvList)==FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}

				if(QueueTopic->m_consumer.setMessageListener(Inparse)==FALSE)
				{
					return FALSE;
				}
				m_TopicRecvList.push_back(QueueTopic);
			}
			else
			{
				CJMSBaseContent *QueueTopic=new CJMSBaseContent(this,queueName,TRUE);


				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_TopicSendList)==FALSE)
				{
					//the queue was exist in queueList
					return FALSE;
				}
				m_TopicSendList.push_back(QueueTopic);
			}


			if (nFomrat==INT_SENDANDRECE)
			{
				CJMSBaseContent *QueueTopic=new CJMSBaseContent(this,queueName,TRUE);


				if (QueueTopic->m_bError)
				{
					return FALSE;
				}
				if(AssureUniquelyList(queueName,m_TopicSendList)==FALSE)
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
	if (m_bConnectionOK==FALSE)	
		return FALSE;

	try
	{		
		std::list< CJMSBaseContent *>::iterator it;
		CJMSTxMessage tempmsg;

		CJMSBaseList *baselist=NULL;
		if (IsQueue)
			baselist=&m_QueueSendList;
		else
			baselist=&m_TopicSendList;
		
		for( it = baselist->begin(); it != baselist->end();it++)
		{
			if (strcmpi((*it)->m_QueueName,queueName)==0)
			{
				if((*it)->m_Session.textMessageCreate(MsgData,tempmsg)==FALSE)
				{
					Clog( LOG_DEBUG,_T(" BOOL  CJMS::SendMsg.textMessageCreate error ! "));
					return FALSE;
				}
				if (intkey !=NULL)
				{
					if(tempmsg.setIntProperty(intkey,nPara1)==FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setIntProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}
				
				if (strkey !=NULL  && cPara2 !=NULL)
				{
					if(tempmsg.setStringProperty(strkey,cPara2)==FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setStringProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}
				if (m_bConnectionOK==FALSE)	
					return FALSE;
				Requestor rr(&((*it)->m_Session),&((*it)->m_destination));

				if(rr.request(&tempmsg,responseMsg,nTimeOut)==FALSE)
				{
					Clog( LOG_DEBUG,_T(" rr.request(&tempmsg,responseMsg, error ! "));
					return FALSE;
				}		

				if (responseMsg._message==NULL)
				{
// ------------------------------------------------------ Modified by zhenan_ji at 2006Äê1ÔÂ16ÈÕ 18:02:48
//					Clog( LOG_DEBUG,_T(" responseMsg ,may tiemout, error ! "));
// spelling mistake
					Clog( LOG_DEBUG,_T(" responseMsg ,may timeout, error ! "));
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
	if (m_bConnectionOK==FALSE)	
		return FALSE;

	try
	{		
		std::list< CJMSBaseContent *>::iterator it;

		CJMSTxMessage tempmsg;

		CJMSBaseList *baselist=NULL;
		if (IsQueue)
			baselist=&m_QueueSendList;
		else
			baselist=&m_TopicSendList;

		for( it = baselist->begin(); it != baselist->end();it++)
		{
			if (strcmpi((*it)->m_QueueName,queueName)==0)
			{
				if((*it)->m_Session.textMessageCreate(MsgData,tempmsg)==FALSE)
				{
					Clog( LOG_DEBUG,_T(" BOOL  CJMS::SendMsg.textMessageCreate error ! "));
					return FALSE;
				}

				if (intkey !=NULL)
				{
					if(tempmsg.setIntProperty(intkey,nPara1)==FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setIntProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}

				if (strkey !=NULL  && cPara2 !=NULL)
				{
					if(tempmsg.setStringProperty(strkey,cPara2)==FALSE)
					{
						Clog( LOG_DEBUG,_T(" tempmsg.setStringProperty(intkey,nPara1) error ! "));
						return FALSE;
					}
				}

				if((*it)->m_producer.send(&tempmsg)==FALSE)
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
		it=m_QueueSendList.begin();
		if (*it)
		{
			delete (*it);
			(*it)=NULL;
		}
		m_QueueSendList.erase(it);
	}
	while(m_QueueRecvList.size())
	{
		it=m_QueueRecvList.begin();
		if (*it)
		{
			delete (*it);
			(*it)=NULL;
		}
		m_QueueRecvList.erase(it);
	}
	while(m_TopicSendList.size())
	{
		it=m_TopicSendList.begin();
		if (*it)
		{
			delete (*it);
			(*it)=NULL;
		}
		m_TopicSendList.erase(it);
	}
	while(m_TopicRecvList.size())
	{
		it=m_TopicRecvList.begin();
		if (*it)
		{
			delete (*it);
			(*it)=NULL;
		}
		m_TopicRecvList.erase(it);
	}
}


