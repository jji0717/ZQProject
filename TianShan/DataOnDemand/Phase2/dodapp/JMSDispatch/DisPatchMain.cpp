#include "stdafx.h"
#include "dispatchmain.h"
using namespace ZQ::common;
CJMSPortManager *g_jmsPortManager = NULL;
std::string g_strDODAppPorxy;
extern Ice::CommunicatorPtr g_ic;
//////////////////////////////////////////////////////////////////////////
DataOnDemand::DataPublisherPrx GetDataPublisherPrx()
{
	DataOnDemand::DataPublisherPrx dataPrx = NULL;
	try
	{
		Ice::ObjectPrx base = g_ic->stringToProxy(g_strDODAppPorxy);
		
		if(!base)
		{
			writeLog(ZQ::common::Log::L_ERROR," GetDataPublisherPrx() string to proxy error");
			return NULL;
		}
		
		dataPrx = DataOnDemand::DataPublisherPrx::checkedCast(base);
		
		if (!dataPrx)
		{
			writeLog(ZQ::common::Log::L_ERROR," GetDataPublisherPrx() checked Cast error");
			return NULL;
		}   		
	}
	catch (const ::Ice::Exception & ex) 
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"GetDataPublisherPrx() Ice::Exception (%s)",
			ex.ice_name().c_str());
		return NULL;
	}

	return dataPrx;
}
DODAppThread::DODAppThread()
{

}

DODAppThread::~DODAppThread()
{
	
}
int  DODAppThread::run()
{
    try 
	{			
	    g_strDODAppPorxy = "DODApp:"+ m_jmspar.DODEndPoint;
		//g_strDODAppPorxy = "DataOnDemandApp:"+ m_jmspar.DODEndPoint;
		
		writeLog(ZQ::common::Log::L_DEBUG,
			"DODAppThread::run() DODAppService.UsingJboss = %d", m_jmspar.UsingJboss);

		g_jmsPortManager = new CJMSPortManager();
		
		if(g_jmsPortManager == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"DODAppThread::run()Init JmsPortManager error");
			return -1;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG, 
			"DODAppThread::run() Initialize JMS and ConnectJBoss...");


		if(!g_jmsPortManager->Create(m_jmspar.JbossIpport,
									 m_jmspar.ConfigQueueName,
									 m_jmspar.comfigTimeOut,
		 							 m_jmspar.CacheFolder,
									 m_jmspar.UsingJboss))
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"DODAppThread::run()Initialize JMS and ConnectJBoss Error");
			return -1 ;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" DODAppThread::run()Initialize JMS and ConnectJBoss finish");
		   
    } 
	catch (const ::TianShanIce::InvalidParameter & ex) 
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"DODAppThread::run() caught  TianShanIce::InvalidParameter(%s)",
			ex.message.c_str());
		return -1;
    } 
	catch (const ::Ice::Exception & ex) 
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"DODAppThread::run()caught Ice::Exception(%s)",
			ex.ice_name().c_str());
		return -1;
    } 
	return 1;
}

void DODAppThread::stop()
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

