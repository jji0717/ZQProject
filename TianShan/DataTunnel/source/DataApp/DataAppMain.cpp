// DODApp.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "DataAppImpl.h"
#include "dataappthread.h"
#include "DataPointPublisherImpl.h"
#include "DataAppMain.h"
#include "global.h"
#include "util.h"
#include "TianShanDefines.h"
#include ".\jmsdispatch\jmsdispatchdll.h"
extern DataAppSVC g_server;
using namespace TianShanIce::Application::DataOnDemand;
using namespace ZQ::common;

//////////////////////////////////////////////////////////////////////////
class DataTunnelAppLogger: public Ice::Logger {
public:

public:
	DataTunnelAppLogger(ZQ::common::Log& log):_logger(log)
	{		
	}
	~DataTunnelAppLogger()
	{	
	}
	void print(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_DEBUG,CLOGFMT(DataTunnelAppLogger, "%s"), message.c_str());
	}
	void trace(const ::std::string& category, const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_DEBUG,CLOGFMT(DataTunnelAppLogger, "catagory %s,message %s"), category.c_str(),message.c_str());
	}
	void warning(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_WARNING,CLOGFMT(DataTunnelAppLogger, "%s"), message.c_str());
	}
	void error(const ::std::string& message)
	{
		_logger(ZQ::common::Log::L_ERROR,CLOGFMT(DataTunnelAppLogger, "%s"), message.c_str());
	}
private:
	ZQ::common::Log& _logger;
};
//////////////////////////////////////////////////////////////////////////
class PingThread: public DataTunnelAppThread {
public:
	PingThread(TianShanIce::Application::DataOnDemand::DataPointPublisherExPrx dataPrx):_datapublisPrx(dataPrx)
	{
		_stopped = false;
		_disconnected = false;
	}

	virtual int run()
	{
		glog(Log::L_INFO, CLOGFMT(PingThread, "enter datastream ping thread"));
		TianShanIce::Streamer::DataOnDemand::DataStreamPrx strmPrx;
		TianShanIce::Application::DataOnDemand::DataStreamInfos datastreams;
		TianShanIce::Application::DataOnDemand::DataStreamPrx datastreamPrx;
		TianShanIce::Application::DataOnDemand::DataStreamExPrx datastreamExPrx;

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

			glog(Log::L_INFO, CLOGFMT(PingThread, "list data streams "));

			TianShanIce::Application::DataOnDemand::DataStreamInfos::iterator it;
			datastreams = _datapublisPrx->listDataStreams("");

            glog(Log::L_INFO, CLOGFMT(PingThread,"data streams count(%d)"), datastreams.size());

			for (it = datastreams.begin(); it != datastreams.end(); it++) 
			{
				try 
				{				
                    datastreamPrx = _datapublisPrx->openDataStream(it->name);
					
                    datastreamExPrx = 
						TianShanIce::Application::DataOnDemand::DataStreamExPrx::checkedCast(datastreamPrx);

					glog(Log::L_DEBUG,CLOGFMT(PingThread,"[%s]data stream"), (*it).name.c_str() );
					
					TianShanIce::SRM::SessionPrx sessionprx = datastreamExPrx->getSession();
					
					strmPrx = TianShanIce::Streamer::DataOnDemand::DataStreamPrx::checkedCast(
						sessionprx->getStream());
					
                    strmPrx->ping();

					glog(Log::L_INFO,CLOGFMT(PingThread,"[%s]data stream ping successfully"),(*it).name.c_str());
				} 
				catch (Ice::Exception& ex) 
				{
					glog(Log::L_WARNING, CLOGFMT(PingThread,"[%s]datastream ping caught exception[%s]"),(*it).name.c_str(), ex.ice_name().c_str());
			        
					try 
					{
						for (int i = 0; i < 10; i++) 
						{
							if (_stopped)
								break;
							
							Sleep(1000);
						}
						
						SessionRenewThread *pThread = 
							TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::getSessionTrd((*it).name);
						if(pThread)
						{
							pThread->stop();
							TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_SessionTrdMap.erase((*it).name);
						}
						
						datastreamExPrx->activate();

						glog(Log::L_INFO,CLOGFMT(PingThread,"[%s]activate data stream successfully"),(*it).name.c_str());
					} 
					catch (TianShanIce::Application::DataOnDemand::StreamerException&ex) 
					{
						glog(Log::L_ERROR,CLOGFMT(PingThread, "[%s] reconnect data stream caught exceptionp[%s]"),(*it).name.c_str(), ex.ice_name().c_str());
					}
					catch(Ice::Exception& ex) 
					{
						glog(Log::L_ERROR,CLOGFMT(PingThread, "[%s] reconnect data stream caught exceptionp[%s]"),(*it).name.c_str(), ex.ice_name().c_str());
					}
				}
			}
		}
		glog(Log::L_INFO, CLOGFMT(PingThread, "leave datastream ping thread"));

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
	TianShanIce::Application::DataOnDemand::DataPointPublisherExPrx _datapublisPrx;
};

class DataTunnelAppServer : public Ice::Application
{
public:

	DataTunnelAppServer(const std::string& envName) :
	  _envName(envName)
	{
		_pingThread = NULL;
		_pmsgmanage = NULL;
		_adapter = NULL;
	}

	virtual ~DataTunnelAppServer() 
	{
		try
		{
			if (_pingThread)
				delete _pingThread;
			_pingThread = NULL;
            if(_pmsgmanage)
				delete _pmsgmanage;
			_pmsgmanage = NULL;
		}
		catch (...)
		{
		}
		
	}	

	virtual int run(int argc, char* argv[]);

	bool stop();

	void clear();

protected:
	bool initDispatch();

private:
	const std::string		_envName;
	ZQADAPTER_DECLTYPE      _adapter;
	::Freeze::EvictorPtr	_evictor;
	PingThread*				_pingThread;
	MessageManage*          _pmsgmanage;
};

int DataTunnelAppServer::run(int argc, char* argv[])
{
	glog(Log::L_INFO, CLOGFMT(DataTunnelAppServer, "Enter run()"));
    

	if (!initGlobal()) {
		g_server.OnStop();
		return -1;
	}

	TianShanIce::Application::DataOnDemand::DataPointPublisherExPrx service;

	Ice::CommunicatorPtr ic = communicator();

	TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_ic = ic;

	Ice::PropertiesPtr _properties = ic->getProperties();
	
	const std::map<std::string, std::string> iceConfig = gDODAppServiceConfig.icePropMap;

	std::map<std::string, std::string>::const_iterator itorCfg;
	for (itorCfg = iceConfig.begin(); itorCfg != iceConfig.end(); ++itorCfg) 
	{
		_properties->setProperty(itorCfg->first, itorCfg->second);
	}

    std::string sessEndPoint = SERVICE_NAME_SessionManager":";
    sessEndPoint = sessEndPoint + gDODAppServiceConfig.szSRMEndpoint;
	std::string contentEndPoint = gDODAppServiceConfig.szDODContentEndpoint;

	glog(Log::L_INFO,  CLOGFMT(DataTunnelAppServer, "connecting weiwoo service at endpoint[%s]"),sessEndPoint.c_str());

	while (g_bServiceStarted)
	{ 
		try	{				
			TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_sessManager =
				TianShanIce::SRM::SessionManagerPrx::checkedCast(ic->stringToProxy(sessEndPoint));

			glog(Log::L_INFO,  CLOGFMT(DataTunnelAppServer, "connect to weiwoo service successfully"));
			break;			
		} 
		catch (const ::Ice::Exception & ex)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelAppServer, "failed to connect to weiwoo service at endpoint[%s], caught exception [%s]"),
				sessEndPoint.c_str(), ex.ice_name().c_str());
			Sleep(5000);
			continue;
		}		
	}
	glog(Log::L_INFO,  CLOGFMT(DataTunnelAppServer, "connecting datacontentstore service at endpoint [%s]"),
		contentEndPoint.c_str());
	while (g_bServiceStarted)
	{ 
		try	{				
			TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_contentStroe =
				TianShanIce::Storage::ContentStorePrx::checkedCast(
										ic->stringToProxy(contentEndPoint));
			
			glog(Log::L_INFO,  CLOGFMT(DataTunnelAppServer, "connect to datacontentstore service successfully"));
			break;			
		} 
		catch (const ::Ice::Exception & ex) {
			
			glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelAppServer, 
				"failed to connect to datacontentstore service at endpoint[%s], caught exception [%s]"),contentEndPoint.c_str(), ex.ice_name().c_str());
			Sleep(5000);
			continue;
		}		
	}

	if(!g_bServiceStarted)
	{
		return -1;
	}

	_adapter = ZQADAPTER_CREATE(ic, ADAPTER_NAME_DODAPP, gDODAppServiceConfig.szDODAppEndpoint.c_str(), glog);
	if (_adapter == NULL) {
		glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to create object Adapter at endpoint [%s] "), gDODAppServiceConfig.szDODAppEndpoint.c_str());
		g_server.OnStop();
		return -1;
	}
	try
	{
#if ICE_INT_VERSION / 100 == 302
		_evictor = Freeze::createEvictor(_adapter, _envName, Servant_DataTunnel);
#else
		_evictor = Freeze::createBackgroundSaveEvictor(_adapter, _envName, Servant_DataTunnel);
#endif
		
		std::string evict;
		evict = "Freeze.Evictor." + _envName + ".evictordod.SaveSizeTrigger";
		_properties->setProperty(evict, "0");
		
		evict.clear();
		evict = "Freeze.Evictor." + _envName + ".evictordod.SavePeriod";
		_properties->setProperty(evict, "1000");
		
		Ice::Int evictorSize = _properties->getPropertyAsInt("DataTunnelAppService.EvictorSize");
		
		if(evictorSize > 0)	
		{
			_evictor->setSize(evictorSize);
		}
		
		TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_adapter = _adapter;
		TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_evictor = _evictor;
		
		Ice::ObjectFactoryPtr factory = new TianShanIce::Application::DataOnDemand::DataTunnelAppFactory();		
		ic->addObjectFactory(factory, TianShanIce::Application::DataOnDemand::DataPointPublisherEx::ice_staticId());
		ic->addObjectFactory(factory, TianShanIce::Application::DataOnDemand::DataStreamEx::ice_staticId());
		ic->addObjectFactory(factory, TianShanIce::Application::DataOnDemand::FolderEx::ice_staticId());
		ic->addObjectFactory(factory, TianShanIce::Application::DataOnDemand::MessageQueueEx::ice_staticId());
		ic->addObjectFactory(factory, TianShanIce::Application::DataOnDemand::DataTunnelPurchase::ice_staticId());
		_adapter->addServantLocator(_evictor, "");

	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to create evictor caught exception[%s]"),
			ex.ice_name().c_str());
		g_server.OnStop();
		return -1;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to create evictor caught exception [%d]"), GetLastError());
		g_server.OnStop();
		return -1;
	}

	TianShanIce::Application::DataOnDemand::DataTunnelServiceImpl* dodappservice = 
		new TianShanIce::Application::DataOnDemand::DataTunnelServiceImpl(_adapter, ic, _evictor);
	if(!dodappservice)
	{
		glog(Log::L_ERROR,   CLOGFMT(DataTunnelAppServer, "failed to create DataTunnelAppService object"));
		g_server.OnStop();
		return -1;
	}

	// add datatunnel application servant
	try
	{
		_adapter->ZQADAPTER_ADD(ic, dodappservice, DATA_ONDEMAND_APPNAME);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR,  CLOGFMT(DataTunnelAppServer, "failed to add datatunnel application servant to adapter caught exception [%s]"), ex.ice_name().c_str());
		g_server.OnStop();
		return -1;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,  CLOGFMT(DataTunnelAppServer, "failed to add datatunnel application servant to adapter caught exception [%d]"), GetLastError());
		g_server.OnStop();
		return -1;
	}
	if(!dodappservice->init())
	{
		glog(Log::L_ERROR,  CLOGFMT(DataTunnelAppServer, "failed to init DataTunnelAppService"));
		g_server.OnStop();
		return -1;
	}
	TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_dodappservice = dodappservice;

	/// initiliza messagemanage object 
	_pmsgmanage = new MessageManage(ic);
	if(!_pmsgmanage)
	{
		glog(Log::L_ERROR,  CLOGFMT(DataTunnelAppServer, "failed to create MessageManage object"));
		g_server.OnStop();
		return -1;
	}
	if(!_pmsgmanage->init())
	{
		glog(Log::L_ERROR,  CLOGFMT(DataTunnelAppServer, "failed to initiliza MessageManage object"));
		g_server.OnStop();
		return -1;
	}
	TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_messagemanage =_pmsgmanage;

	Ice::Identity rootId = ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME);

	if (!_evictor->hasObject(rootId)) {
		
		TianShanIce::Application::DataOnDemand::DataPointPublisherImpl* dataPub = 
			new TianShanIce::Application::DataOnDemand::DataPointPublisherImpl();

		if (!dataPub->init()) {
			glog(Log::L_ERROR,  CLOGFMT(DataTunnelAppServer, "failed to initiliza DataPointPublisherImpl object"));
			g_server.OnStop();
			return -1;
		}

        glog(Log::L_INFO,  CLOGFMT(DataTunnelAppServer, 
			"initiliza DataPointPublisherImpl object successfully"));

		if (_evictor->add(dataPub, 
			rootId) == NULL) {

				glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to add DataPointPublisher object to evictor"));
				g_server.OnStop();
				return -1;
		}
		glog(Log::L_INFO, CLOGFMT(DataTunnelAppServer, "add DataPointPublisher object to evictor successfully"));

		service = TianShanIce::Application::DataOnDemand::DataPointPublisherExPrx::uncheckedCast(
				_adapter->createProxy(rootId));

	} else {		
		try {		
			service = TianShanIce::Application::DataOnDemand::DataPointPublisherExPrx::checkedCast(_adapter->createProxy(rootId));

		} catch (Ice::ObjectNotExistException& ex) {

			glog(Log::L_ERROR,  CLOGFMT(DataTunnelAppServer, "failed to get datapointpublisher proxy caught exception[%s]"), ex.ice_name().c_str());
			g_server.OnStop();
			return -1;
			
		} catch (Ice::UnmarshalOutOfBoundsException& ex) {
			
			glog(Log::L_CRIT,  CLOGFMT(DataTunnelAppServer, 
				"failed to get datapointpublisher proxy caught exception[%s]"), ex.ice_name().c_str());
			g_server.OnStop();
			return -1;
			
		}
		catch(...)
		{
			glog(Log::L_CRIT,  CLOGFMT(DataTunnelAppServer, 
				"failed to get datapointpublisher proxy caught exception[%d]"), GetLastError());
			g_server.OnStop();
			return -1;
		}		      
	}
    //create Freeze DB for store messageinfo
	ignoreInterrupt();
	_adapter->activate();

	try
	{	
		service->activate();
	}
	catch (const Ice::Exception & ex)
	{
		glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to activate datapublishpoint and datastream object caught exception[%s]"), ex.ice_name().c_str());
	    g_server.OnStop();
		return -1;
	}
	// Ice::Application::shutdownOnInterrupt();

	if (!initDispatch()) {
		g_server.OnStop();
		return -1;
	}
	
	glog(Log::L_INFO, CLOGFMT(DataTunnelAppServer, "create datastream ping thread"));
	_pingThread = new PingThread(service);

	if(_pingThread)
	{		
		_pingThread->start();
	}

	ic->waitForShutdown();

	glog(Log::L_INFO, CLOGFMT(DataTunnelAppServer, "leave run()"));
	return 0;
}

bool DataTunnelAppServer::stop()
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DataTunnelAppServer, "enter stop()"));
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
      glog(Log::L_INFO, CLOGFMT(DataTunnelAppServer,  "stop DataTunnelAppServer caught exception [%s]"), ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_INFO,  CLOGFMT(DataTunnelAppServer, "leave stop()"));
	return true;
}

void DataTunnelAppServer::clear()
{
	TianShanIce::Application::DataOnDemand::DataPointPublisherExPrx service;
    Ice::CommunicatorPtr ic = _adapter->getCommunicator();
	try {		
		service = TianShanIce::Application::DataOnDemand::DataPointPublisherExPrx::checkedCast(
			_adapter->createProxy(ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME)));
		
	} catch (Ice::ObjectNotExistException& ex) {
		
		glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to get DataPointPublisher proxy caught exception[%s]"), ex.ice_name().c_str());
		return;
		
	} catch (Ice::UnmarshalOutOfBoundsException& ex) {
		
		glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to get DataPointPublisher proxy caught exception[%s]"), ex.ice_name().c_str());
		return;		
	}
	catch(Ice::Exception& ex)
	{
		glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to get DataPointPublisher proxy caught exception[%s]"), ex.ice_name().c_str());
		return;
	}
	
	{
		TianShanIce::Application::DataOnDemand::DataStreamInfos datastreams= service->listDataStreams("");
		TianShanIce::Application::DataOnDemand::DataStreamInfos::iterator  it;
		TianShanIce::Application::DataOnDemand::DataStreamPrx datastreamPrx;
		try {
			for (it = datastreams.begin(); it != datastreams.end(); it ++) {
				datastreamPrx = service->openDataStream(it->name);
				//destPrx->stop();
				datastreamPrx->destroy();
			}
		} catch(Ice::Exception& ex) {
			glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to get destination caught exception[%s]"), ex.ice_name().c_str());
			return;

		} catch(...){
			glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to get destination caught exception[%d]"), GetLastError());
			return;
		}
	}


	::TianShanIce::Application::DataOnDemand::DataPublishPointInfos  DataPPs = service->listDataPublishPoints("");
	::TianShanIce::Application::DataOnDemand::DataPublishPointInfos::iterator it;
	TianShanIce::Application::DataOnDemand::DataPublishPointPrx DataPPPrx;

	try {

		for (it = DataPPs.begin(); it != DataPPs.end(); it ++) {
			DataPPPrx = service->openDataPublishPoint(it->name);
			DataPPPrx->destroy();
		}
	} catch(Ice::Exception& ex) {
		glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to get DataPublishPoint caught exception[%s]"), ex.ice_name().c_str());
		return;

	} catch(...){
		glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to get DataPublishPoint caught exception[%d]"), GetLastError());
		return;
	}
	_evictor->remove(ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME));
}
bool DataTunnelAppServer::initDispatch()
{
	JMSDISPATCHPARAMETER jmsinfo;

	jmsinfo.JbossIpport = gDODAppServiceConfig.szJBossIpPort;
	jmsinfo.ConfigQueueName = gDODAppServiceConfig.szConfigQueueName;
	jmsinfo.CacheFolder = sourceCachePath;
	jmsinfo.comfigTimeOut = gDODAppServiceConfig.lConfigTimeOut;
	jmsinfo.UsingJboss = gDODAppServiceConfig.lUsingJboss;
//	jmsinfo.UsingJboss = 0;
	jmsinfo.DataTunnelEndPoint = gDODAppServiceConfig.szDODAppEndpoint;

	if(!JmsPortInitialize(jmsinfo,
						  TianShanIce::Application::DataOnDemand::DataPointPublisherImpl::_ic,
						  &glog)) 
	{
        glog(Log::L_ERROR, CLOGFMT(DataTunnelAppServer, "failed to initialize JMS at IPPort[%s]"), jmsinfo.JbossIpport.c_str());
		return false;
	}
	glog(Log::L_DEBUG, CLOGFMT(DataTunnelAppServer, "Initialize JMS successfully"));
	return true;
}

//////////////////////////////////////////////////////////////////////////

extern DataTunnelAppMainThread* mainThread;

DataTunnelAppThread::DataTunnelAppThread()
{
	mainThread->addSubthread(this);
}

DataTunnelAppThread::~DataTunnelAppThread()
{
	mainThread->removeSubthread(this);
}

//////////////////////////////////////////////////////////////////////////

DataTunnelAppMainThread::DataTunnelAppMainThread()
{
	_appSvr = NULL;
}

DataTunnelAppMainThread::~DataTunnelAppMainThread()
{
	if (_appSvr)
		delete _appSvr;
}

bool DataTunnelAppMainThread::init()
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
	strCurDir = strCurDir + "DataTunnelAppService";
	
    glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DataTunnelAppMainThread, "DataTunnel database directory [%s]"),strCurDir.c_str());

	if(_access((strCurDir + FNSEPS).c_str(),0))
	{
		// open dictionary
		bool ret = ::CreateDirectoryA((strCurDir + FNSEPS).c_str(), NULL);
		if(!ret)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelAppMainThread, "failed to create database directory error[%d]"),GetLastError());
			g_server.OnStop();
			return false;
		}
	}

	assert(!_appSvr);
	_appSvr = new DataTunnelAppServer(strCurDir.c_str());
	return true;
}

void DataTunnelAppMainThread::final()
{	
	return;
}

int DataTunnelAppMainThread::run()
{
	if(NULL == _appSvr)	{

     	glog(Log::L_ERROR, CLOGFMT(DataTunnelAppMainThread,"failed to create DODAppService"));
		return -1;
	}

	m_iceLogger = new DataTunnelAppLogger(*m_pIceLog);

	if(!m_iceLogger)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelAppMainThread,"failed to create DataTunnelAppLogger"));
	}
     Ice::InitializationData initdata;
	 initdata.logger = m_iceLogger;
	int result = _appSvr->main(__argc , __argv, initdata);
	return result;
}

void DataTunnelAppMainThread::stop()
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
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(DataTunnelAppMainThread," stop DataTunnelAppMainThread caught excepiton[%s]"),ex.ice_name().c_str());
		
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(DataTunnelAppMainThread,"leave stop DataTunnelAppMainThread"));
}

void DataTunnelAppMainThread::destory()
{
	
}

void DataTunnelAppMainThread::addSubthread(DataTunnelAppThread* thread)
{
	assert(thread);

	std::pair<ThreadSet::iterator, bool> r;
	r = _subthreads.insert(thread);
	if (!r.second) {
		throw ::TianShanIce::InvalidParameter();
	}
}

void DataTunnelAppMainThread::removeSubthread(DataTunnelAppThread* thread)
{
	assert(thread);

	std::pair<ThreadSet::iterator, bool> r;
	_subthreads.erase(thread);
}

void DataTunnelAppMainThread::stopSubthreads()
{
	ThreadSet backupSet = _subthreads;
	ThreadSet::iterator it;
	for (it = backupSet.begin(); it != backupSet.end(); it ++) {
		(*it)->stop();
	}
}