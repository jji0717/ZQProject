// DODApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DODAppImpl.h"

#include "DataPublisherImpl.h"
#include "DODAppMain.h"
#include "global.h"
#include "util.h"
using namespace DataOnDemand;
#include ".\jmsdispatch\jmsdispatchdll.h"

//////////////////////////////////////////////////////////////////////////

using namespace ZQ::common;


//////////////////////////////////////////////////////////////////////////
class DODAppLogger: public Ice::Logger {
public:

	virtual void print(const ::std::string& text)
	{
		glog(Log::L_INFO, text.c_str());
	}

	virtual void trace(const ::std::string& category, const ::std::string& text)
	{
		glog(Log::L_DEBUG, "%s : %s", category.c_str(), text.c_str());
	}

	virtual void warning(const ::std::string& text)
	{
		glog(Log::L_WARNING, text.c_str());
	}

	virtual void error(const ::std::string& text)
	{
		glog(Log::L_ERROR, text.c_str());
	}
};

//////////////////////////////////////////////////////////////////////////

class PingThread: public DODAppThread {
public:
	PingThread(DataStreamServicePrx& streamer, 
		DataPublisherExPrx dataPublisher):
	  _streamer(streamer), _dataPublisher(dataPublisher)
	{
		_stopped = false;
		_disconnected = false;
	}

	virtual int run()
	{
		while(!_stopped) {

			// check it after 10 minute
			for (int i = 0; i < 10; i ++) {
				if (_stopped)
					break;

				Sleep(1000);
			}

			try {
				_streamer->ping(configSpaceName);
			} catch (Ice::Exception& e) {
				glog(Log::L_ERROR, "PingThread::run():\t ping"
					"(%s)", e.ice_name());

				_disconnected = true;
				continue;
			}

			if (_disconnected) {
				_disconnected = false;
				try {
					_dataPublisher->reconnect();
				} catch(Ice::Exception& e) {
					glog(Log::L_ERROR, "PingThread::run():\t reconnect"
						"(%s)", e.ice_name());
					_disconnected = true;
				}
			}
		}

		glog(Log::L_DEBUG, "PingThread::run():Exit!");

		return 0;
	}

	virtual void stop()
	{
		_stopped = true;
		waitHandle(INFINITE);
	}

protected:
	DataStreamServicePrx&	_streamer;
	DataPublisherExPrx		_dataPublisher;
	bool					_stopped;
	bool					_disconnected;
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
	glog(Log::L_DEBUG, "Enter DODAppServer::run ");
    

	if (!initGlobal()) {
		glog(Log::L_DEBUG, "initGlobal() failed.\n");
		return -1;
	}

	DataOnDemand::DataPublisherExPrx service;

	Ice::CommunicatorPtr ic = communicator();

	DataOnDemand::DataPublisherImpl::_ic = ic;

	Ice::PropertiesPtr properties = ic->getProperties();

	while (g_bServiceStarted)
	{
		try	{	
              glog(Log::L_DEBUG, "DODAppServer::run() get DataStream begin!");

			DataOnDemand::DataPublisherImpl::_strmSvcEndPoint = 
				properties->getProperty("DODAppService.DataStreamServiceEndPoint");
			
			DataOnDemand::DataPublisherImpl::_dataStreamSvc = 
				DataStreamServicePrx::checkedCast(createObjectWithEndPoint(ic, 
				ic->stringToIdentity("DODStreamer"), 
				DataOnDemand::DataPublisherImpl::_strmSvcEndPoint));

            glog(Log::L_DEBUG, "DODAppServer::run() get DataStream end!");

			break;

		} catch (const ::Ice::ConnectionRefusedException & ex) {

			glog(ZQ::common::Log::L_DEBUG,
				"DODAppServer::run(): Ice::Exception errorcode = %s"
				"\n\t\t\t\t endpoint = %s",ex.ice_name().c_str(),
				DataOnDemand::DataPublisherImpl::_strmSvcEndPoint);
			Sleep(2000);
			continue;

		} catch(const Ice::ObjectNotExistException& ex) {

			glog(ZQ::common::Log::L_DEBUG,
				"DODAppServer::run(): Ice::Exception errorcode = %s"
				"\n\t\t\t\t endpoint = %s",ex.ice_name().c_str(),
				DataOnDemand::DataPublisherImpl::_strmSvcEndPoint);
			Sleep(2000);
			continue;

		} catch (const ::Ice::Exception & ex) {

			glog(ZQ::common::Log::L_DEBUG,
				"DODAppServer::run(): Ice::Exception errorcode = %s",
				ex.ice_name().c_str());
			return -1;
		}		
	}

	if(!g_bServiceStarted)
	{
		return -1;
	}
	
	glog(ZQ::common::Log::L_DEBUG, 
		"DODAppServer::run()\t connect to streamer success!endpoint = %s",
		DataOnDemand::DataPublisherImpl::_strmSvcEndPoint);
	
	_adapter = ic->createObjectAdapter(ADAPTER_NAME_DODAPP);	

	
	if (_adapter == NULL) {
		glog(Log::L_DEBUG, "Communicator::createObjectAdapter() failed");
		return -1;
	}
	
	/*
 	DataOnDemand::ChannelTypeIndexPtr chTypeIndex = 
		new DataOnDemand::ChannelTypeIndex("myType");

	std::vector<Freeze::IndexPtr> indices;
	indices.push_back(chTypeIndex);
	
	_evictor = Freeze::createEvictor(_adapter, _envName, "evictordod", 
		0, indices);	
	*/
	
	_evictor = Freeze::createEvictor(_adapter, _envName, "evictordod");	

	Ice::Int evictorSize = properties->getPropertyAsInt(
		"DODAppService.EvictorSize");
	
	if(evictorSize > 0)	{
		_evictor->setSize(evictorSize);
	}

	DataOnDemand::DataPublisherImpl::_adapter = _adapter;
	DataOnDemand::DataPublisherImpl::_evictor = _evictor;
	// DataOnDemand::DataPublisherImpl::_channelTypeIndex = chTypeIndex;

	Ice::ObjectFactoryPtr factory = new DataOnDemand::DODAppFactory();

	ic->addObjectFactory(factory, "::DataOnDemand::DataPublisherEx");
	ic->addObjectFactory(factory, "::DataOnDemand::DestinationEx");
	ic->addObjectFactory(factory, "::DataOnDemand::FolderChannelEx");
	ic->addObjectFactory(factory, "::DataOnDemand::MessageChannelEx");

	_adapter->addServantLocator(_evictor, "");

	sourceCachePath = properties->getProperty(
		"DODAppService.SourceCachePath");
	if(sourceCachePath.empty())
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

	if(sourceCachePath[length - 1] != '\\') {
		sourceCachePath += FNSEPS;
	}

	properties->setProperty("DODAppService.SourceCachePath", 
		sourceCachePath);
	
	Ice::Identity rootId = ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME);

	if (!_evictor->hasObject(rootId)) {
		
		DataOnDemand::DataPublisherImpl* dataPub = 
			new DataOnDemand::DataPublisherImpl();

		if (!dataPub->init()) {
			glog(Log::L_CRIT, "DataPublisher initilization failed.");
			return -1;
		}

        glog(Log::L_CRIT, "DataPublisher initilization success.");

		if (_evictor->add(dataPub, 
			ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME)) == NULL) {

				glog(Log::L_CRIT, "ObjectAdapter::add() failed.");
				return -1;
		}
		glog(Log::L_DEBUG, "ObjectAdapter::add() success.");

		service = DataOnDemand::DataPublisherExPrx::uncheckedCast(
				_adapter->createProxy(ic->stringToIdentity(
				DATA_ONDEMAND_DODAPPNAME)));

		::DataOnDemand::DataPublisherImpl::_dataStreamSvc->clear(
			   configSpaceName);

	} else {
		
		try {

			service = DataOnDemand::DataPublisherExPrx::checkedCast(
				_adapter->createProxy(ic->stringToIdentity(
				DATA_ONDEMAND_DODAPPNAME)));

		} catch (Ice::ObjectNotExistException& e) {

			glog(Log::L_CRIT, "%s:\tget DataStreamer failed:%s", 
				__FUNCTION__, e.ice_name());
			return -1;

		} catch (Ice::UnmarshalOutOfBoundsException& e) {

			glog(Log::L_CRIT, "%s:\tget DataStreamer failed:%s", 
				__FUNCTION__, e.ice_name());
			return -1;

		}
		
       	glog(Log::L_DEBUG, "get DataStreamer success.");
		      
	}

	// _evictor->keep(ic->stringToIdentity(DATA_ONDEMAND_DODAPPNAME));
	try
	{	
		service->activate();
	}
	catch (const Ice::Exception & ex)
	{
		glog(Log::L_DEBUG, "Active object error, err: %s", 
			ex.ice_name());
		
	}

	_adapter->activate();
	// Ice::Application::shutdownOnInterrupt();

	if (!initDispatch()) {
		glog(Log::L_DEBUG, "initDispatch() failed");
		return -1;
	}
									
	_pingThread = new PingThread(
		DataOnDemand::DataPublisherImpl::_dataStreamSvc, 
		service);
		
	_pingThread->start();

	ic->waitForShutdown();
	glog(Log::L_DEBUG, "leaving DODAppServer::run ");
	return 0;
}

bool DODAppServer::stop()
{
	try
	{
		if(_adapter)
		{
			Ice::CommunicatorPtr ic = _adapter->getCommunicator();
			_adapter->deactivate();
			_adapter->waitForDeactivate();	
			ic->shutdown();
			ic->waitForShutdown();
		}	
	}
	catch(const Ice::Exception &ex)
	{
      glog(Log::L_DEBUG, "DODAppServer::stop() errorcode = %s ",
		  ex.ice_name().c_str());
	}

	glog(ZQ::common::Log::L_DEBUG, 
			"DODAppServer::stop() end!");
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
			__FUNCTION__, e.ice_name());
		return;
		
	} catch (Ice::UnmarshalOutOfBoundsException& e) {
		
		glog(Log::L_CRIT, "%s:\tget DataStreamer failed:%s", 
			__FUNCTION__, e.ice_name());
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
				__FUNCTION__, e.ice_name());
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
				__FUNCTION__, e.ice_name());
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
	if(!JmsPortInitialize(DataOnDemand::DataPublisherImpl::_ic, &glog)) 
	{
        glog(Log::L_ERROR, L"JmsPortInitialize Error!");
		return false;
	}

	glog(Log::L_DEBUG, L"JmsPortInitialize Finished!");

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
	int            nUsingJBoss = 0;
	DWORD dSize = GetModuleFileNameA(NULL,sModuleName,1024);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.rfind('\\');
	strCurDir = strCurDir.substr(0,nIndex); //end with "\\"
    strCurDir = strCurDir + "\\DODAppService";

    glog(ZQ::common::Log::L_DEBUG, "DODApp DB dir = %s",
		strCurDir.c_str());

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

     	glog(Log::L_CRIT, "create DODAppService failed!");
		return -1;
	}

	::std::string  strCurDir;
	char           sModuleName[1025];
	int            nUsingJBoss = 0;
	DWORD dSize = GetModuleFileNameA(NULL,sModuleName,1024);
	sModuleName[dSize] = '\0';
	strCurDir = sModuleName;
	int nIndex = strCurDir.rfind('\\');
	strCurDir = strCurDir.substr(0,nIndex); //end with "\\"
    strCurDir = strCurDir + "\\DODAppConfig";

    glog(ZQ::common::Log::L_DEBUG, "DODAppConfig  dir = %s",
		strCurDir.c_str());

	glog(ZQ::common::Log::L_DEBUG, "before _appSvr->main()");

	Ice::LoggerPtr logger = new DODAppLogger();

	int result = _appSvr->main(__argc , __argv, strCurDir.c_str(), logger);

	logger = NULL;

/*	int result = _appSvr->main(__argc , __argv, strCurDir.c_str());*/
	
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
			"DODAppMainThread::stop() errorcode = %s",
			ex.ice_name().c_str());
		
	}

	glog(ZQ::common::Log::L_DEBUG, 
			"DODAppMainThread::stop() end!");
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
