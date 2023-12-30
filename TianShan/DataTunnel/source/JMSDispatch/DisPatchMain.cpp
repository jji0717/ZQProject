#include "stdafx.h"
#include "dispatchmain.h"
using namespace ZQ::common;
extern ZQ::common::Log* _logger;
CJMSPortManager *g_jmsPortManager = NULL;
#define JMSDISPACTCHTHREAD "JMSdispatchThread"
//////////////////////////////////////////////////////////////////////////
JMSdispatchThread::JMSdispatchThread(Ice::CommunicatorPtr& ic):m_ic(ic)
{
  
}

JMSdispatchThread::~JMSdispatchThread()
{
	
}
int  JMSdispatchThread::run()
{
    try 
	{			
		std::string datatunnelEP = "DataPointPublisher:"+ m_jmspar.DataTunnelEndPoint;

		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(JMSDISPACTCHTHREAD,
			"run() using jboss flag '%d'"), m_jmspar.UsingJboss);

		g_jmsPortManager = new CJMSPortManager(datatunnelEP, m_ic);
		
		if(g_jmsPortManager == NULL)
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(JMSDISPACTCHTHREAD,
				"run()fail to create JmsPortManager object"));
			return -1;
		}
		
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(JMSDISPACTCHTHREAD,
			"run() Initialize JMS..."));


		if(!g_jmsPortManager->Create(m_jmspar.JbossIpport,
									 m_jmspar.ConfigQueueName,
									 m_jmspar.comfigTimeOut,
		 							 m_jmspar.CacheFolder,
									 m_jmspar.UsingJboss))
		{
			(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(JMSDISPACTCHTHREAD,
				"run()fail to initialize JMS"));
			return -1 ;
		}
		
		(*_logger)(ZQ::common::Log::L_DEBUG, CLOGFMT(JMSDISPACTCHTHREAD,
			"Initialize JMS sucessfully"));
		   
    } 
	catch (const ::TianShanIce::InvalidParameter & ex) 
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(JMSDISPACTCHTHREAD,
			"caught  TianShanIce InvalidParameter(%s)"),
			ex.message.c_str());
		return -1;
    } 
	catch (const ::Ice::Exception & ex) 
	{
		(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(JMSDISPACTCHTHREAD,
			"caught Ice exception (%s)"),
			ex.ice_name().c_str());
		return -1;
    } 
	return 1;
}

void JMSdispatchThread::stop()
{
	try
	{
		g_jmsPortManager->stop();

		if(g_jmsPortManager)
		{
			delete g_jmsPortManager;
			g_jmsPortManager = NULL;
		}
	}
	catch(...)
	{

	}
}

