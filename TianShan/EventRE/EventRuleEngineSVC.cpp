#include "EventRuleEngineSVC.h"
#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif
#include "Log.h"
#include "ConfigHelper.h"
#include "EventREConfig.h"

#define LOG_MODULE_NAME			"Service"


EventRuleEngineSVC g_ContentLibService;
ZQ::common::BaseZQServiceApplication *Application = &g_ContentLibService;
ZQ::common::Config::Loader<EventREConfig> _config("EventRuleEngine.xml");
ZQ::common::Config::ILoader *configLoader = &_config;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

ZQ::common::MiniDump _minidump;
static void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress);
#else
extern const char* DUMP_PATH;
#endif

EventRuleEngineSVC::EventRuleEngineSVC()
:_communicator(NULL), _icelog(NULL), _properties(NULL), 
	_pRuleEngine(NULL), _pThreadPool(NULL)
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "EventRuleEngineSvc");
	strcpy(prodname, "TianShan");
#endif
}

EventRuleEngineSVC::~EventRuleEngineSVC()
{
}

HRESULT EventRuleEngineSVC::OnInit(void)
{
	BaseZQServiceApplication::OnInit();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventRuleEngineSVC, "OnInit() enter"));

	// get the endpoint of topic manager
	if(!_config.TopicMgrEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngineSVC, "key [TopicManagerEndPoint] must be configured"));
		return S_FALSE;
	}	

	//
	// Get Configuration from Application's sub Registry Level - ICE
	//	
	_properties = Ice::createProperties();

	EventREConfig::IceProps::iterator iter = _config.iceProps.begin();
	for (; iter != _config.iceProps.end(); ++iter) 
	{
		_properties->setProperty(iter->name, iter->value);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(EventRuleEngineSVC, "EventRuleEngineSVC::OnInit() set ICE properties <%s>--<%s>."), (iter->name).c_str(), (iter->value).c_str());
	}

#ifdef ZQ_OS_MSWIN
	{
		// get the crash dump path		
		if (!_minidump.setDumpPath((char *)(_config.crushDumpPath.c_str())))
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngineSVC, "key [CrashDumpPath] is not correct directory"));
			return S_FALSE;
		}

		_minidump.enableFullMemoryDump(_config.dumpFullMemory);
		_minidump.setExceptionCB(MiniDumpCallback);		
	}
#else
	DUMP_PATH = _config.crushDumpPath.c_str();
#endif

	_config.snmpRegister("");
	glog(ZQ::common::Log::L_INFO, "EventRuleEngineSVC::OnInit() leave");

	return S_OK;
}

HRESULT EventRuleEngineSVC::OnUnInit(void)
{
	if(_pRuleEngine)
	{
		delete _pRuleEngine;
		_pRuleEngine = NULL;
	}

	if(_pThreadPool)
	{
		delete _pThreadPool;
		_pThreadPool = NULL;
	}
	return S_OK;	
}


HRESULT EventRuleEngineSVC::OnStart(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventRuleEngineSVC, "OnStart() enter"));

	BaseZQServiceApplication::OnStart();

	//
	// Initialize Ice communicator properties
	//
	int argc = 0;
	try
	{
		_icelog= new TianShanIce::common::IceLogI(m_pReporter);
		Ice::InitializationData initData;
		initData.logger = _icelog;
		initData.properties = _properties;
		_communicator = Ice::initialize(argc, 0, initData);
		glog(ZQ::common::Log::L_INFO, "Ice communicator created");

		_pThreadPool = new ZQ::common::NativeThreadPool(_config.ThreadPoolSize);

		_pRuleEngine = new EventRuleEngine(*m_pReporter, *_pThreadPool, _config.RuleDepth, _communicator);

		_pRuleEngine->populate(_config.actionPath.c_str());

		_pRuleEngine->loadConfig(_config.configPath.c_str(), false);

		if (!_pRuleEngine->init())
		{
			return S_FALSE;
		}
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngineSVC, "Ice initialize properties met exception with error: %s, %s @ line %d"), 
			ex.ice_name().c_str(), ex.ice_file(), ex.ice_line());
		return S_FALSE;					
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventRuleEngineSVC, "OnStart() leaved"));

	return S_OK;
}

HRESULT EventRuleEngineSVC::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventRuleEngineSVC, "OnStop() enter"));

	try
	{
		if (_pRuleEngine)
		{
			_pRuleEngine->unInit();
			delete _pRuleEngine;
			_pRuleEngine = NULL;
		}

		if(_pThreadPool)
		{
			delete _pThreadPool;
			_pThreadPool = NULL;
		}

		if(_communicator)
		{
			_communicator->destroy();
		}		
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngineSVC, "Ice destroy met exception with error: %s @ line %d"), 
			ex.ice_file(), ex.ice_line());
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventRuleEngineSVC, "Ice destroy met unknown exception"));
	}

	// Ice object are not required to release, coz they are smart pointer object

	BaseZQServiceApplication::OnStop();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventRuleEngineSVC, "OnStop() leave"));

	return S_OK;
}

#ifdef ZQ_OS_MSWIN
void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog(ZQ::common::Log::L_ERROR,  "Application crashed, ExceptionCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);

	glog.flush();
}
#endif
