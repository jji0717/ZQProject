#include "stdafx.h"
#include "dispatchmain.h"
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
			writeLog(ZQ::common::Log::L_ERROR,"stringtoproxy error.");
			return NULL;
		}
		
		dataPrx = DataOnDemand::DataPublisherPrx::checkedCast(base);
		
		if (!dataPrx)
		{
			writeLog(ZQ::common::Log::L_ERROR,"checkedCast error.");
			return NULL;
		}   		
	}
	catch (const ::Ice::Exception & ex) 
	{
		writeLog(ZQ::common::Log::L_DEBUG,
			"GetDataPublisherPrx:Ice::Exception errorcode = %s",
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
void  DODAppThread::run()
{
    try 
	{	
//		Ice::CommunicatorPtr ic = Ice::Application::communicator();
		Ice::PropertiesPtr properties = g_ic->getProperties();

		const char* refProperty = "DODAppService.Endpoints";
		
		std::string ref = properties->getProperty(refProperty);
		std::string endpoint;
		if(ref.empty())
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"property: %s not set !", refProperty);
			return ;
		}		
		g_strDODAppPorxy = "DODApp:"+ ref;
		
	    
		Ice::Int nUsingJBoss = properties->getPropertyAsInt(
			"DODAppService.UsingJboss");

		writeLog(ZQ::common::Log::L_DEBUG,
			"DODAppService.UsingJboss = %d!", nUsingJBoss);

		g_jmsPortManager = new CJMSPortManager();
		
		if(g_jmsPortManager == NULL)
		{
			writeLog(ZQ::common::Log::L_ERROR,
				"Init JmsPortManager error!");
			return ;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG, 
			"Initialize JMS and ConnectJBoss...!");
				
		string strJbossIPaddress = properties->getProperty(	
			                      "DODAppService.JBossIpAddress");
		if(strJbossIPaddress.empty()) 
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"Read DODAppService.JbossIpAddress Error!");
			return ;
		}
		
		string strQueueCf = properties->getProperty(
			"DODAppService.ConfigQueueName");
		if(strQueueCf.empty())
		{
			writeLog(ZQ::common::Log::L_ERROR, 
				"Read DODAppService.ConfigQueueName Error!");
			return ;
		}
		
		int  ConfigMsgTimeOut = properties->getPropertyAsInt(
			"DODAppService.ConfigMsgTimeOut");
		if(ConfigMsgTimeOut == 0) 
		{
			ConfigMsgTimeOut = 7000;
		}
		
		string sourceCachePath = properties->getProperty(
			"DODAppService.SourceCachePath");
/*		if(sourceCachePath.empty())
		{
			char path[MAX_PATH];
			if (::GetModuleFileNameA(NULL, path, MAX_PATH-1)>0)
			{
				char* p = strrchr(path, FNSEPC);
				if (NULL !=p)
				{
					*p='\0';
				}		
				
				strcat(path, FNSEPS "DODAppCachedir" FNSEPS);
				sourceCachePath = path;
			}
		}
		// open dictionary
		::CreateDirectoryA((sourceCachePath + FNSEPS).c_str(), NULL);

         int length = sourceCachePath.size();
         if(sourceCachePath[length - 1] != '\\')
		 {
			 sourceCachePath += FNSEPS;
		 }
		properties->setProperty("DODAppService.SourceCachePath",sourceCachePath);*/

		if(!g_jmsPortManager->Create(strJbossIPaddress, strQueueCf,
			ConfigMsgTimeOut, sourceCachePath,nUsingJBoss))
		{
			writeLog(ZQ::common::Log::L_ERROR,
				" Initialize JMS and ConnectJBoss Error!");
			return ;
		}
		
		writeLog(ZQ::common::Log::L_DEBUG,
			" Initialize JMS and ConnectJBoss finish!");
		   
    } 
	catch (const ::TianShanIce::InvalidParameter & ex) 
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"InitDataPublisher: TianShanIce::InvalidParameter errorcode = %s",
			ex.message);
		return ;
    } 
	catch (const ::Ice::Exception & ex) 
	{
		writeLog(ZQ::common::Log::L_ERROR,
			"InitDataPublisher: Ice::Exception errorcode = %s",
			ex.ice_name().c_str());
		return ;
    } 
}

void DODAppThread::stop()
{
	g_jmsPortManager->stop();

   if(g_jmsPortManager)
   {
	   delete g_jmsPortManager;
	   g_jmsPortManager = NULL;
   }
}

