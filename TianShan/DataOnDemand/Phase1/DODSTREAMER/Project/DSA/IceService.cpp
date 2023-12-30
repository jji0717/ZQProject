#include "IceService.h"
#include <Freeze/Freeze.h>
#include "svclog.h"
#include <assert.h>

extern CSvcLog service_log;
void glog(ISvcLog::LogLevel level, const char* fmt, ...);


class DODLogger: public Ice::Logger {
public:

	virtual void print(const ::std::string& text)
	{
		service_log.log0(ISvcLog::L_INFO, text.c_str());
	}

	virtual void trace(const ::std::string& category, const ::std::string& text)
	{
		service_log.log(ISvcLog::L_DEBUG, "%s : %s", category.c_str(), text.c_str());
	}

	virtual void warning(const ::std::string& text)
	{
		service_log.log0(ISvcLog::L_WARNING, text.c_str());
	}

	virtual void error(const ::std::string& text)
	{
		service_log.log0(ISvcLog::L_ERROR, text.c_str());
	}
};
//////////////////////////////////////////////////////////////////////////

class DODStreamerServer : public Ice::Application
{
public:
	friend DWORD __stdcall __stopThreadProc(void* p);

	DODStreamerServer(const std::string& envName) :
	  _envName(envName)
	{
		_chkTimeoutStopped = true;		  
	}

	virtual int run(int argc, char* argv[]);

	bool stop();

protected:
	bool startCheckTimeout();
	virtual DWORD checkTimeoutRun();
	void stopCheckTimeout();

    static DWORD __stdcall __checkTimeoutThreadProc(void* param);

protected:
	const std::string		_envName;
	::Ice::ObjectAdapterPtr _adapter;
	::Freeze::EvictorPtr	_evictor;

	HANDLE					_chkTimeoutThread;
	bool					_chkTimeoutStopped;

	
};

DWORD __stdcall __stopThreadProc(void* p)
{
	DODStreamerServer* svr = (DODStreamerServer* )p;
	Sleep(20000);
	svr->_evictor->deactivate("");
	svr->_adapter->deactivate();
	svr->stop();

	return 0;
}
int DODStreamerServer::run(int argc, char* argv[])
{
	try {
		Ice::CommunicatorPtr ic = communicator();

		_adapter = ic->createObjectAdapter("DODStreamer");	
		if (_adapter == NULL) {
			glog(ISvcLog::L_CRIT, "Communicator::createObjectAdapter() failed");
			return -1;
		}

		_evictor = Freeze::createEvictor(_adapter, _envName, "evictordod");

		Ice::PropertiesPtr properties = communicator()->getProperties();

		Ice::Int evictorSize = properties->getPropertyAsInt("DODStreamer.EvictorSize");
		if(evictorSize > 0)	{
			_evictor->setSize(evictorSize);
		}

		DataOnDemand::DataStreamServiceImpl::_adapter = _adapter;
		DataOnDemand::DataStreamServiceImpl::_evictor = _evictor;

		Ice::ObjectFactoryPtr factory = new DataOnDemand::StreamerFactory();

		ic->addObjectFactory(factory, "::DataOnDemand::DataStreamEx");
		ic->addObjectFactory(factory, "::DataOnDemand::MuxItemEx");
		ic->addObjectFactory(factory, "::DataOnDemand::DataStreamServiceEx");

		_adapter->addServantLocator(_evictor, "");

		Ice::Identity rootId = ic->stringToIdentity("DODStreamer");

		if (!_evictor->hasObject(rootId)) {
			DataOnDemand::DataStreamServiceImpl* dataStreamer = 
				new DataOnDemand::DataStreamServiceImpl();

			if (!dataStreamer->init()) {
				glog(ISvcLog::L_CRIT, "dataStreamer initilization failed.\n");
				return -1;
			}

			if (_evictor->add(dataStreamer, 
				ic->stringToIdentity("DODStreamer")) == NULL) {

					glog(ISvcLog::L_CRIT, "ObjectAdapter::add() failed");
					return -1;
			}

			dataStreamer->myProps["NextSessionId"] = "0";

		} else {
			DataOnDemand::DataStreamServiceExPrx service;
			try {
				service = DataOnDemand::DataStreamServiceExPrx::checkedCast(
					_adapter->createProxy(ic->stringToIdentity("DODStreamer")));
			} catch (Ice::ObjectNotExistException& e) {

				glog(ISvcLog::L_CRIT, "%s:\tget DataStreamer failed:%s", 
					__FUNCTION__, e.ice_name().c_str());
				return -1;
			} catch (Ice::UnmarshalOutOfBoundsException& e) {

				glog(ISvcLog::L_CRIT, "%s:\tget DataStreamer failed:%s", 
					__FUNCTION__, e.ice_name().c_str());
				return -1;
			}

			service->activate();
		}

		// CreateThread(NULL, 0, __stopThreadProc, this, 0, NULL);

		_adapter->activate();
		startCheckTimeout();

		// Ice::Application::shutdownOnInterrupt();
		ic->waitForShutdown();
	} catch (Ice::Exception& e) {
		glog(ISvcLog::L_CRIT, "%s:\t%s:%d (occurred a Ice exception)", 
			__FUNCTION__, e.ice_file(), e.ice_line());
		return -1;
	}

	return 0;
}

bool DODStreamerServer::stop()
{
	stopCheckTimeout();
	Ice::CommunicatorPtr ic = _adapter->getCommunicator();
	_adapter->deactivate();
	_adapter->waitForDeactivate();	
	ic->shutdown();
	ic->waitForShutdown();	
	return true;
}

bool DODStreamerServer::startCheckTimeout()
{
	DWORD threadId;
	_chkTimeoutThread = CreateThread(NULL, 0, __checkTimeoutThreadProc, 
		this, 0, &threadId);

	return _chkTimeoutThread != NULL;
}

DWORD DODStreamerServer::checkTimeoutRun()
{
	_chkTimeoutStopped = false;
	DataOnDemand::DataStreamServiceExPrx service;

	try {
		
		Ice::Identity rootId = _adapter->getCommunicator()->
			stringToIdentity("DODStreamer");

		service = DataOnDemand::DataStreamServiceExPrx::checkedCast(
			_adapter->createProxy(rootId));
	} catch (Ice::Exception& e) {
		glog(ISvcLog::L_DEBUG, "%s:\tfailed to get service object(%s).", 
			__FUNCTION__, e.ice_name().c_str());
		return -1;
	}
    
	while (!_chkTimeoutStopped) {
		try {

			// service->checkTimeout(120 * 60 * 1000); // 2 hour
			service->checkTimeout(70 * 60 * 1000); // 70 minutes

		} catch(Ice::Exception& e) {
			glog(ISvcLog::L_ERROR, "%s:\tcheckTimeout() occurred a exception(%s).", 
				__FUNCTION__, e.ice_name().c_str());
		}

		// check it after 10 minute
		for (int i = 0; i < 10; i ++) {
			if (_chkTimeoutStopped)
				break;

			Sleep(1000);
		}		
	}

	return 0;
}

void DODStreamerServer::stopCheckTimeout()
{
	_chkTimeoutStopped = true;
	WaitForSingleObject(_chkTimeoutThread, INFINITE);
}

DWORD __stdcall DODStreamerServer::__checkTimeoutThreadProc(void* param)
{
	DODStreamerServer* thisPtr = (DODStreamerServer* )param;
	return thisPtr->checkTimeoutRun();
}


//////////////////////////////////////////////////////////////////////////

IceService::IceService()
{
	_exitEvent = ::CreateEvent(NULL,TRUE, FALSE, NULL);
}

IceService::~IceService()
{
	CloseHandle(_exitEvent);
}

bool IceService::start()
{
	_threadHandle = ::CreateThread(NULL, 0, win32ThreadProc, this, 0, 
		&_threadId );

	if (_threadHandle == NULL)
		return false;

	return true;
}

bool IceService::stop()
{
	bool r = _server->stop();
	WaitForSingleObject(_exitEvent, INFINITE);
	return r;
}

DWORD __stdcall IceService::win32ThreadProc(void* param)
{
	IceService* me = (IceService* )param;
	assert(me);
	if (!me->init()) {
		glog(ISvcLog::L_DEBUG, "IceService::init() result is false.");
		return -1;
	}

	DWORD r = me->run();
    me->final();
	return r;
}

bool IceService::init()
{
	return true;
}

unsigned long  IceService::run()
{
	_server = new DODStreamerServer("DODStreamer");
	Ice::LoggerPtr logger = new DODLogger();
	return _server->main(__argc, __argv, "config", logger);
}

void IceService::final()
{
	delete _server;
	SetEvent(_exitEvent);
}

