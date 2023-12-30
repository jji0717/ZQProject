
#include "ContentLibService.h"
#include "ContentLibConfig.h"


ContentLibService g_ContentLibService;
ZQ::common::ZQDaemon *Application = &g_ContentLibService;
ZQ::common::Config::Loader<ContentLibConfig> _config("ContentLib.xml");
ZQ::common::Config::ILoader *configLoader = &_config;

ContentLibService::ContentLibService()
	: _communicator(NULL), _pContentLibEnv(NULL), _icelog(NULL), _properties(NULL), _iceFileLog(NULL)
{
}

ContentLibService::~ContentLibService()
{
}


bool ContentLibService::OnInit(void)
{
	ZQ::common::setGlogger(&_logger);
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnInit() enter"));

	if(!_config.ContentLibEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "key [ContentLibEndPoint] must be configured"));
		return false;
	}

	// get the endpoint of topic manager
	if(!_config.TopicMgrEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "key [TopicManagerEndPoint] must be configured"));
		return false;
	}	

	//init ice trace
	std::string strLogFolder = _logDir;

	int size = strLogFolder.size();
	if(size > 0 && strLogFolder[size -1] != FNSEPC )
		strLogFolder += FNSEPS;

	_iceFileLog = new ZQ::common::FileLog( (strLogFolder+"ContentLib_IceTrace.log").c_str(),
		_config.iceLogLevel,
		_config.iceLogCount,
		_config.iceLogSize);

	//
	// Get Configuration from Application's sub Registry Level - ICE
	//	
	_properties = Ice::createProperties();

	ContentLibConfig::IceProps::iterator iter = _config.iceProps.begin();
	for (; iter != _config.iceProps.end(); ++iter) 
	{
		_properties->setProperty(iter->name, iter->value);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "ContentLibSvc::OnInit() set ICE properties <%s>--<%s>."), (iter->name).c_str(), (iter->value).c_str());
	}

	_config.snmpRegister("");
	glog(ZQ::common::Log::L_INFO, "ContentLibSvc::OnInit() leave");

	return ZQDaemon::OnInit();
}

bool ContentLibService::OnStart(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnStart() enter"));

	//
	// Initialize Ice communicator properties
	//
	int argc = 0;
	try
	{
		_icelog= new TianShanIce::common::IceLogI(_iceFileLog);

		Ice::InitializationData initData;
		if(_config.iceTraceEnable)
		{
			initData.logger = _icelog;
		}
		initData.properties = _properties;
		_communicator = Ice::initialize(argc, 0, initData);
		glog(ZQ::common::Log::L_INFO, "Ice communicator created");

		_pContentLibEnv = new ContentLibEnv(_communicator);	
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "ContentLibEnv object created"));		
		if (!_pContentLibEnv->init())
		{
			return false;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "ContentLibEnv object initialized"));
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "Ice initialize properties met exception with error: %s, %s @ line %d"), 
			ex.ice_name().c_str(), ex.ice_file(), ex.ice_line());
		return false;					
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnStart() leaved"));	

    return ZQDaemon::OnStart();
}

void ContentLibService::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnStop() enter"));
	try
	{
		if (_pContentLibEnv)
		{
			_pContentLibEnv->unInit();
			delete _pContentLibEnv;
			_pContentLibEnv = NULL;
		}

		if(_communicator)
		{
			_communicator->destroy();
			_communicator = NULL;
		}		
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "Ice destroy met exception with error: %s @ line %d"), 
			ex.ice_file(), ex.ice_line());
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "Ice destroy met unknown exception"));
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnStop() leave"));
    ZQDaemon::OnStop();
}

void ContentLibService::OnUnInit(void)
{
	if (_pContentLibEnv)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "Start to delete ContentLibEnv object"));
		delete _pContentLibEnv;
		_pContentLibEnv = NULL;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "ContentLibEnv object deleted"));
	}

	if (_iceFileLog)
	{
		delete _iceFileLog;
		_iceFileLog = NULL;
	}

    ZQDaemon::OnUnInit();
}


