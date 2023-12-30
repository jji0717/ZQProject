// DODApp.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DODAppImpl.h"
#include "dodappthread.h"
#include "DataPublisherImpl.h"
#include "DODAppMain.h"
#include "global.h"
#include "util.h"
#include "TianShanDefines.h"
#include ".\jmsdispatch\jmsdispatchdll.h"
extern DODAppSVC g_server;
using namespace DataOnDemand;
//////////////////////////////////////////////////////////////////////////

using namespace ZQ::common;

//////////////////////////////////////////////////////////////////////////
class DODAppLogger: public Ice::Logger {
public:

public:
	DODAppLogger(ZQ::common::Log& log):_logger(log)
	{		
	}
	~DODAppLogger()
	{	
	}
	void print(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_DEBUG,message.c_str());
	}
	void trace(const ::std::string& category, const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_DEBUG,"catagory %s,message %s",
											category.c_str(),message.c_str());
	}
	void warning(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_WARNING,message.c_str());
	}
	void error(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_ERROR,message.c_str());
	}
private:
	ZQ::common::Log& _logger;
};
//////////////////////////////////////////////////////////////////////////
class PingThread: public DODAppThread {
public:
	PingThread(DataOnDemand::DataPublisherExPrx dataPrx):_datapublisPrx(dataPrx)
	{
		_stopped = false;
		_disconnected = false;
	}

	virtual int run()
	{
		DataOnDemand::DataStreamPrx strmPrx;
		TianShanIce::StrValues myDests;
		::DataOnDemand::DestinationPrx destPrx;
		DataOnDemand::DestinationExPrx destExPrx;

		long  streampingtime = gDODAppServiceConfig.lStreamPingtime;

		while(!_stopped)
		{
			for (int i = 0; i < streampingtime; i ++) 
			{
				if (_stopped)
					break;

				Sleep(1000);
			}
			if (_stopped)
				break;

			TianShanIce::StrValues::iterator it;
			std::string sessionID;
			myDests = _datapublisPrx->listDestinations();

            glog(Log::L_INFO, 
				"PingThread::run()[Stream Count(%d)] ping...", myDests.size());

			for (it = myDests.begin(); it != myDests.end(); it++) 
			{
				try 
				{				
                    destPrx = _datapublisPrx->getDestination(*it);
					
                    destExPrx = 
						DataOnDemand::DestinationExPrx::checkedCast(destPrx);

					glog(Log::L_INFO,
						"PingThread::run() [DestName: %s]  sessionID (%s).",
						(*it).c_str() , destExPrx->getSessionId().c_str());
					
					TianShanIce::SRM::SessionPrx sessionprx = 
						DataOnDemand::DataPublisherImpl::_sessManager->
						openSession(destExPrx->getSessionId());
					
					strmPrx = DataOnDemand::DataStreamPrx::checkedCast(
						sessionprx->getStream());
					
                    strmPrx->ping();

					glog(Log::L_INFO,
						"PingThread::run() [DestName: %s] stream ping success",
						(*it).c_str());
				} 
				catch (Ice::Exception& e) 
				{
					glog(Log::L_ERROR, "PingThread::run():[DestName: %s] ping error at (%s)",
						(*it).c_str(), e.ice_name().c_str());
			        
					try 
					{
						for (int i = 0; i < 10; i++) 
						{
							if (_stopped)
								break;
							
							Sleep(1000);
						}
						
						SessionRenewThread *pThread = 
							DataOnDemand::DataPublisherImpl::getSessionTrd(*it);
						if(pThread)
						{
							pThread->stop();
							DataOnDemand::DataPublisherImpl::_SessionTrdMap.erase(*it);
						}
						
						destExPrx->activate();

						glog(Log::L_INFO,
							"PingThread::run()[DestName: %s] destination activate success:",
						  (*it).c_str());
					} 
					catch (DataOnDemand::StreamerException&e) 
					{
						glog(Log::L_ERROR, "PingThread::run():[DestName: %s] reconnect:"
							"DataOnDemand::StreamerException (%s)",(*it).c_str(), e.ice_name().c_str());
					}
					catch(Ice::Exception& e) 
					{
						glog(Log::L_ERROR, "PingThread::run()[DestName: %s]"
							"reconnect cauht error at Ice::Exception (%s)", (*it).c_str(), e.ice_name().c_str());
					}
				}
			}
		}
		glog(Log::L_DEBUG, "Exit PingThread::run()");

		return 0;
	}

	virtual void stop()
	{
		_stopped = true;
		waitHandle(INFINITE);
	}

protected:
	bool					_stopped;
	bool					_disconnected;	
	DataOnDemand::DataPublisherExPrx _datapublisPrx;
};

class DODAppServer : public Ice::Application
{
public:

	DODAppServer(const std::string& envName) :
	  _envName(envName)
	{
		_pingThread = NULL;
		_adapter = NULL;
	}

	virtual ~DODAppServer() 
	{
		if (_pingThread)
			delete _pingThread;
	}	

	virtual int run(int argc, char* argv[]);

	bool stop();

	void clear();

protected:
	bool initDispatch();

private:
	const std::string		_envName;
	::Ice::ObjectAdapterPtr _adapter;
	::Freeze::EvictorPtr	_evictor;
	PingThread*				_pingThread;
};

int DODAppServer::run(int argc, char* argv[])
{
	glog(Log::L_INFO, "Enter DODAppServer::run() ");
    

	if (!initGlobal()) {
		glog(Log::L_ERROR, "init global parameter failed.");
		g_server.OnStop();
		return -1;
	}

	DataOnDemand::DataPublisherExPrx service;

	Ice::CommunicatorPtr ic = communicator();

	DataOnDemand::DataPublisherImpl::_ic = ic;

	Ice::PropertiesPtr _properties = ic->getProperties();
	
	const std::map<std::string, std::string> iceConfig = 
								gDODAppServiceConfig.icePropMap;

	std::map<std::string, std::string>::const_iterator iter = iceConfig.begin();
	for (int i = 0; iter != iceConfig.end(); ++iter) 
	{
		_properties->setProperty(iter->first, iter->second);
		i++;
	}

	std::string        _sessEndPoint;
	std::string		   _contentEndPoint;
    
    _sessEndPoint = SERVICE_NAME_SessionManager":";
    _sessEndPoint = _sessEndPoint + gDODAppServiceConfig.szSRMEndpoint;
	_contentEndPoint = gDODAppServiceConfig.szDODContentEndpoint;

	glog(Log::L_INFO, 
		"DODAppServer::run() Connect to WeiWoo service at endpoint [%s]",
		_sessEndPoint.c_str());

	while (g_bServiceStarted)
	{ 
		try	{				
			DataOnDemand::DataPublisherImpl::_sessManager =
				TianShanIce::SRM::SessionManagerPrx::checkedCast(
				ic->stringToProxy(_sessEndPoint));
			
			glog(Log::L_INFO, "DODAppServer::run() Connect to WeiWoo service success");
			break;			
		} 
		catch (const ::Ice::Exception & ex)
		{
			glog(ZQ::common::Log::L_INFO,
				"DODAppServer::run() Connect to  WeiWoo service error, Ice::Exception (%s)",
				ex.ice_name().c_str());
			Sleep(5000);
			continue;
		}		
	}
	glog(Log::L_INFO, 
		"DODAppServer::run() Connect to DODContentStore service at endpoint [%s]",
		_contentEndPoint.c_str());
	while (g_bServiceStarted)
	{ 
		try	{				
			DataOnDemand::DataPublisherImpl::_contentStroe =
				TianShanIce::Storage::ContentStorePrx::checkedCast(
										ic->stringToProxy(_contentEndPoint));
			
			glog(Log::L_INFO, "DODAppServer::run() Connect to DODContentStore service success");
			break;			
		} 
		catch (const ::Ice::Exception & ex) {
			
			glog(ZQ::common::Log::L_INFO,
				"DODAppServer::run() Connect to DODContentStore service error, Ice::Exception (%s)",ex.ice_name().c_str());
			Sleep(5000);
			continue;
		}		
	}

	if(!g_bServiceStarted)
	{
		return -1;
	}
	_adapter = ic->createObjectAdapterWithEndpoints(ADAPTER_NAME_DODAPP,
										gDODAppServiceConfig.szDODAppEndpoint);	

	
	if (_adapter == NULL) {
		glog(Log::L_DEBUG,
			"DODAppServer::run() createObjectAdapter() at enpoint [%s] failed",
			gDODAppServiceConfig.szDODAppEndpoint.c_str());
		return -1;
	}

    DataOnDemand::DODAppServiceImpl* dodappservice = 
		              new DataOnDemand::DODAppServiceImpl();
	if(!dodappservice)
	{
		glog(Log::L_ERROR, "DODAppServer::run() creat DODAppService object failed");
		g_server.OnStop();
		return -1;
	}
	dodappservice->_adapter = _adapter;
	if(!dodappservice->init())
	{
		glog(Log::L_ERROR, "DODAppServer::run() DODAppService init failed");
		g_server.OnStop();
		return -1;

	}
	_adapter->add(dodappservice, ic->stringToIdentity(DATA_ONDEMAND_APPNAME));
	
	DataOnDemand::DataPublisherImpl::_dodappservice = dodappservice;

	try
	{
#if ICE_INT_VERSION / 100 == 302
		_evictor = Freeze::createEvictor(_adapter, _envName, "evictordod");
#else
		_evictor = Freeze::createBackgroundSaveEvictor(_adapter, _envName, "evictordod");
#endif
		
		std::string evict;
		evict = "Freeze.Evictor." + _envName + ".evictordod.SaveSizeTrigger";
		_properties->setProperty(evict, "0");
		
		evict.clear();
		evict = "Freeze.Evictor." + _envName + ".evictordod.SavePeriod";
		_properties->setProperty(evict, "1000");
		
		Ice::Int evictorSize = _properties->getPropertyAsInt("DODAppService.EvictorSize");
		
		if(evictorSize > 0)	
		{
			_evictor->setSize(evictorSize);
		}
		
		DataOnDemand::DataPublisherImpl::_adapter = _adapter;
		DataOnDemand::DataPublisherImpl::_evictor = _evictor;
		
		Ice::ObjectFactoryPtr factory = new DataOnDemand::DODAppFactory();
		
		ic->addObjectFactory(factory, "::DataOnDemand::DataPublisherEx");
		ic->addObjectFactory(factory, "::DataOnDemand::DestinationEx");
		ic->addObjectFactory(factory, "::DataOnDemand::FolderChannelEx");
		ic->addObjectFactory(factory, "::DataOnDemand::MessageChannelEx");
		
		_adapter->addServantLocator(_evictor, "");

	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DODAppServer::run()createEvictor error, Ice::Exception (%s)",
			ex.ice_name().c_str());
		g_server.OnStop();
		return -1;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,
			"DODAppServer::run()create Evictor error (%d)", GetLastError());
		g_server.OnStop();
			return -1;
	}


	Ice::Identity rootId = ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME);
	
	if (!_evictor->hasObject(rootId)) {
		
		DataOnDemand::DataPublisherImpl* dataPub = 
			new DataOnDemand::DataPublisherImpl();

		if (!dataPub->init()) {
			glog(Log::L_CRIT, "DataPublisherImpl object initilization failed.");
			g_server.OnStop();
			return -1;
		}

        glog(Log::L_INFO, "DataPublisherImpl object initilization success.");

		if (_evictor->add(dataPub, 
			ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME)) == NULL) {

				glog(Log::L_CRIT, "DataPublisherImpl object ObjectAdapter::add() failed.");
				g_server.OnStop();
				return -1;
		}
		glog(Log::L_INFO, "DataPublisherImpl object ObjectAdapter::add() success.");

		service = DataOnDemand::DataPublisherExPrx::uncheckedCast(
				_adapter->createProxy(ic->stringToIdentity(
				DATA_ONDEMAND_DODAPPNAME)));

	} else {
		
		try {
			
			service = DataOnDemand::DataPublisherExPrx::checkedCast(
				_adapter->createProxy(ic->stringToIdentity(
				DATA_ONDEMAND_DODAPPNAME)));
			
		} catch (Ice::ObjectNotExistException& e) {
			
			glog(Log::L_CRIT, "%s:\tget DataStreamer failed:%s", 
				__FUNCTION__, e.ice_name());
			g_server.OnStop();
			return -1;
			
		} catch (Ice::UnmarshalOutOfBoundsException& e) {
			
			glog(Log::L_CRIT, "%s:\tget DataStreamer failed:%s", 
				__FUNCTION__, e.ice_name());
			g_server.OnStop();
			return -1;
			
		}
		
       	glog(Log::L_DEBUG, "get DataPublisherImpl object proxy success.");
		      
	}
    
	ignoreInterrupt();
	_adapter->activate();

	try
	{	
		service->activate();
	}
	catch (const Ice::Exception & ex)
	{
		glog(Log::L_ERROR, "Active object error, err: %s", 
			ex.ice_name().c_str());
	    g_server.OnStop();
		return -1;
	}
	// Ice::Application::shutdownOnInterrupt();

	if (!initDispatch()) {
		glog(Log::L_ERROR, "DODAppServer::run()initDispatch() failed");
		g_server.OnStop();
		return -1;
	}
	
	glog(Log::L_INFO, "DODAppServer::run() Create PingThread...");
	_pingThread = new PingThread(service);

	if(_pingThread)
	{		
		_pingThread->start();
	}

/*	glog(Log::L_INFO, "DODAppServer::run(): Create CreatDestionTrd!");

	DataOnDemand::DataPublisherImpl::_pCreatDestTrd = new CreatDestionTrd();

	if(DataOnDemand::DataPublisherImpl::_pCreatDestTrd)
	{
       DataOnDemand::DataPublisherImpl::_pCreatDestTrd->start();
	}*/

	ic->waitForShutdown();

	glog(Log::L_INFO, "Leave DODAppServer::run");

	return 0;
}

bool DODAppServer::stop()
{
	glog(ZQ::common::Log::L_INFO, 
			"enter DODAppServer::stop()");
	try
	{
		if(_adapter)
		{
			Ice::CommunicatorPtr ic = _adapter->getCommunicator();
			_adapter->deactivate();
			_adapter->waitForDeactivate();	
			ic->shutdown();
		}	
	}
	catch(const Ice::Exception &ex)
	{
      glog(Log::L_INFO, "DODAppServer::stop() errorcode = %s ",
		  ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_INFO, 
			"Leave DODAppServer::stop()!");
	return true;
}

void DODAppServer::clear()
{
	DataOnDemand::DataPublisherExPrx service;
    Ice::CommunicatorPtr ic = _adapter->getCommunicator();
	try {		
		service = DataOnDemand::DataPublisherExPrx::checkedCast(
			_adapter->createProxy(ic->
			         stringToIdentity(DATA_ONDEMAND_DODAPPNAME)));
		
	} catch (Ice::ObjectNotExistException& e) {
		
		glog(Log::L_CRIT, "%s:\tget DataStreamer failed:%s", 
			__FUNCTION__, e.ice_name().c_str());
		return;
		
	} catch (Ice::UnmarshalOutOfBoundsException& e) {
		
		glog(Log::L_CRIT, "%s:\tget DataStreamer failed:%s", 
			__FUNCTION__, e.ice_name().c_str());
		return;		
	}
	
	{
		::TianShanIce::StrValues  destions= service->listDestinations();
		::TianShanIce::StrValues::iterator it;
		::DataOnDemand::DestinationPrx destPrx;
		try {
			for (it = destions.begin(); it != destions.end(); it ++) {
				destPrx = service->getDestination(*it);
				//destPrx->stop();
				destPrx->destroy();
			}
		} catch(Ice::Exception& e) {
			
			glog(Log::L_CRIT, "%s:\tget Destination failed:%s", 
				__FUNCTION__, e.ice_name().c_str());
			return;

		} catch(...) {

			// LPEXCEPTION_POINTERS excep = GetExceptionInformation();

			glog(Log::L_CRIT, "%s:\tget Destination failed. GetLastError: %x"
				"GetExceptionCode: %x", __FUNCTION__, GetLastError(), 0);
			return;
		}
	}

	{
		::TianShanIce::StrValues  channels = service->listChannels();
		::TianShanIce::StrValues::iterator it;
		::DataOnDemand::ChannelPublishPointPrx channel;
		
		try {

			for (it = channels.begin(); it != channels.end(); it ++) {
				channel = service->getChannel(*it);
				channel->destroy();
			}
		} catch(Ice::Exception& e) {

			glog(Log::L_CRIT, "%s:\tget Channel failed:%s", 
				__FUNCTION__, e.ice_name().c_str());
			return;

		} catch(...) {
			glog(Log::L_CRIT, "%s:\tget Destination failed. GetLastError: %x"
				"GetExceptionCode: %x", __FUNCTION__, GetLastError(), 
				0);
			return;
		}
	}

	// ::DataOnDemand::DataPublisherImpl::_dataStreamSvc->clear(
	//	configSpaceName);

	_evictor->remove(ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME));
}
bool DODAppServer::initDispatch()
{	
	if(!JmsPortInitialize(gDODAppServiceConfig.szJBossIpPort,
						  gDODAppServiceConfig.szConfigQueueName,
						  sourceCachePath,
						  gDODAppServiceConfig.lConfigTimeOut,
						  gDODAppServiceConfig.lUsingJboss,
					      gDODAppServiceConfig.szDODAppEndpoint,
						  DataOnDemand::DataPublisherImpl::_ic,
						  &glog)) 
	{
        glog(Log::L_ERROR, "DODAppServer::initDispatch() Initialize JMS Error");
		return false;
	}

	glog(Log::L_DEBUG, "DODAppServer::initDispatch() Initialize JMS success");

	return true;
}

//////////////////////////////////////////////////////////////////////////

extern DODAppMainThread* mainThread;

DODAppThread::DODAppThread()
{
	mainThread->addSubthread(this);
}

DODAppThread::~DODAppThread()
{
	mainThread->removeSubthread(this);
}

//////////////////////////////////////////////////////////////////////////

DODAppMainThread::DODAppMainThread()
{
	_appSvr = NULL;
}

DODAppMainThread::~DODAppMainThread()
{
	if (_appSvr)
		delete _appSvr;
}

bool DODAppMainThread::init()
{
	::std::string  strCurDir;
	char           sModuleName[1025];
	
    strCurDir = gDODAppServiceConfig.szDODAppDBFolder;
	
	if(strCurDir.size() < 1)
	{
		DWORD dSize = GetModuleFileNameA(NULL,sModuleName,1024);
		sModuleName[dSize] = '\0';
		strCurDir = sModuleName;
		int nIndex = strCurDir.rfind('\\');
		strCurDir = strCurDir.substr(0,nIndex);
		nIndex = strCurDir.rfind('\\');
		strCurDir = strCurDir.substr(0,nIndex); //end with "\\"	
		strCurDir = strCurDir + "\\data\\";
	}
	
	int length = strCurDir.size();
	
	if(strCurDir[length -1] != '\\')
	{
		strCurDir += FNSEPS;
	}
	strCurDir = strCurDir + "DODAppService";
	
    glog(ZQ::common::Log::L_DEBUG, "DODApp database directory (%s)",
		strCurDir.c_str());

	if(_access((strCurDir + FNSEPS).c_str(),0))
	{
		// open dictionary
		bool ret = ::CreateDirectoryA((strCurDir + FNSEPS).c_str(), NULL);
		if(!ret)
		{
			glog(ZQ::common::Log::L_ERROR, "Create database directory error (%d)",GetLastError());
			g_server.OnStop();
			return false;
		}
	}

	assert(!_appSvr);
	_appSvr = new DODAppServer(strCurDir.c_str());
	return true;
}

void DODAppMainThread::final()
{	
	return;
}

int DODAppMainThread::run()
{
	if(NULL == _appSvr)	{

     	glog(Log::L_CRIT, "DODAppMainThread::run() create DODAppService failed!");
		return -1;
	}

    glog(ZQ::common::Log::L_DEBUG, "Enter appserver main()");		

	m_iceLogger = new DODAppLogger(*m_pIceLog);

	if(!m_iceLogger)
	{
		glog(ZQ::common::Log::L_ERROR, "DODAppMainThread::run() Create DODAppLogger error");
	}
     Ice::InitializationData initdata;
	 initdata.logger = m_iceLogger;
	int result = _appSvr->main(__argc , __argv, initdata);

	glog(ZQ::common::Log::L_DEBUG, "Leave appserver main()");
	return result;
}

void DODAppMainThread::stop()
{
/*	if (_appSvr) {
		_appSvr->clear();
	}*/
	try
	{	
		JmsPortUnInitialize();
		
		stopSubthreads();
		
		if (_appSvr) {
			
			_appSvr->stop();
			delete _appSvr;
			_appSvr = NULL;
		}
	}
	catch (const Ice::Exception &ex)
	{
		glog(ZQ::common::Log::L_DEBUG, 
			"DODAppMainThread::stop() caught ICE excepiton (%s)",
			ex.ice_name().c_str());
		
	}

	glog(ZQ::common::Log::L_DEBUG, 
			"Leave DODAppMainThread::stop()");
}

void DODAppMainThread::destory()
{
	
}

void DODAppMainThread::addSubthread(DODAppThread* thread)
{
	assert(thread);

	std::pair<ThreadSet::iterator, bool> r;
	r = _subthreads.insert(thread);
	if (!r.second) {
		throw ::TianShanIce::InvalidParameter();
	}
}

void DODAppMainThread::removeSubthread(DODAppThread* thread)
{
	assert(thread);

	std::pair<ThreadSet::iterator, bool> r;
	_subthreads.erase(thread);
}

void DODAppMainThread::stopSubthreads()
{
	ThreadSet backupSet = _subthreads;
	ThreadSet::iterator it;
	for (it = backupSet.begin(); it != backupSet.end(); it ++) {
		(*it)->stop();
	}
}
