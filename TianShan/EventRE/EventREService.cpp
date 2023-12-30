
#include "EventREService.h"
#include "EventREConfig.h"

#define LOG_MODULE_NAME			"Service"


EventREService	 g_EventRESvc;
ZQ::common::ZQDaemon  *Application = &g_EventRESvc;

ZQ::common::Config::Loader< EventREConfig > _config("EventRuleEngine.xml");
ZQ::common::Config::ILoader *configLoader = &_config;


EventREService::EventREService()
:_communicator(NULL), _icelog(NULL), _properties(NULL), 
	_pRuleEngine(NULL), _pThreadPool(NULL)
{
}

EventREService::~EventREService()
{
}

bool EventREService::OnInit(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventREService, "OnInit() enter"));

	// get the endpoint of topic manager
	if(!_config.TopicMgrEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventREService, "key [TopicManagerEndPoint] must be configured"));
		return false;
	}	

	//
	// Get Configuration from Application's sub Registry Level - ICE
	//	
	_properties = Ice::createProperties();

	EventREConfig::IceProps::iterator iter = _config.iceProps.begin();
	for (; iter != _config.iceProps.end(); ++iter) 
	{
		_properties->setProperty(iter->name, iter->value);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(EventREService, "OnInit() set ICE properties <%s>--<%s>."), (iter->name).c_str(), (iter->value).c_str());
	}

	_config.snmpRegister("");
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventREService,  "OnInit() leave"));

	return ZQDaemon::OnInit();
}

bool EventREService::OnStart(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventREService, "OnStart() enter"));

	//
	// Initialize Ice communicator properties
	//
	int argc = 0;
	try
	{
		_icelog= new TianShanIce::common::IceLogI(_logger);
		Ice::InitializationData initData;
		initData.logger = _icelog;
		initData.properties = _properties;
		_communicator = Ice::initialize(argc, 0, initData);
		glog(ZQ::common::Log::L_INFO, "Ice communicator created");

		_pThreadPool = new ZQ::common::NativeThreadPool(_config.ThreadPoolSize);

		_pRuleEngine = new EventRuleEngine(glog, *_pThreadPool, _config.RuleDepth, _communicator);

		_pRuleEngine->populate(_config.actionPath.c_str());

		_pRuleEngine->loadConfig(_config.configPath.c_str(), false);

		if (!_pRuleEngine->init())
		{
			return false;
		}
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventREService, "Ice initialize properties met exception with error: %s, %s @ line %d"), 
			ex.ice_name().c_str(), ex.ice_file(), ex.ice_line());
		return false;					
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventREService, "OnStart() leaved"));

	return ZQDaemon::OnStart();
}

void EventREService::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventREService, "OnStop() enter"));

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
			_pThreadPool = 0;
		}

		if(_communicator)
		{
			_communicator->destroy();
			_communicator = 0;
		}		
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventREService, "Ice destroy met exception with error: %s @ line %d"), 
			ex.ice_file(), ex.ice_line());
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(EventREService, "Ice destroy met unknown exception"));
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(EventREService, "OnStop() leave"));

	ZQDaemon::OnStop();
}

void EventREService::OnUnInit(void)
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
	ZQDaemon::OnUnInit();
}


