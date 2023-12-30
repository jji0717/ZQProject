#include "ContentLibSvc.h"
#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif
#include "Log.h"
#include "ContentLibConfig.h"

#define LOG_MODULE_NAME			"Service"


ContentLibSvc g_ContentLibService;
ZQ::common::BaseZQServiceApplication *Application = &g_ContentLibService;
ZQ::common::Config::Loader<ContentLibConfig> _config("ContentLib.xml");
ZQ::common::Config::ILoader *configLoader = &_config;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
ZQ::common::MiniDump _minidump;
static void WINAPI MiniDumpCallback(DWORD ExceptionCode, PVOID ExceptionAddress);
#else
extern const char* DUMP_PATH;
#endif

ContentLibSvc::ContentLibSvc()
: _communicator(NULL), _pContentLibEnv(NULL), _icelog(NULL), _properties(NULL), _iceFileLog(NULL)
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "ContentLibSvc");
	strcpy(prodname, "TianShan");
#endif
}

ContentLibSvc::~ContentLibSvc()
{
}

HRESULT ContentLibSvc::OnInit(void)
{
	BaseZQServiceApplication::OnInit();

	// has covered by BaseZQServiceApplication: ZQ::common::setGlogger(m_pReporter);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnInit() enter"));


	if(!_config.ContentLibEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "key [ContentLibEndPoint] must be configured"));
		return S_FALSE;
	}

	// get the endpoint of topic manager
	if(!_config.TopicMgrEndPoint.size())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "key [TopicManagerEndPoint] must be configured"));
		return S_FALSE;
	}	

	//init ice trace
#ifdef ZQ_OS_MSWIN
	std::string strLogFolder = m_wsLogFolder;
#else
	std::string strLogFolder = _logDir;
#endif

	int size = strLogFolder.size();
	if(size > 0 && strLogFolder[size -1] != FNSEPC)
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

#ifdef ZQ_OS_MSWIN
	{
		// get the crash dump path		
		if (!_minidump.setDumpPath((char *)(_config.crushDumpPath.c_str())))
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "key [CrashDumpPath] is not correct directory"));
			return S_FALSE;
		}

		_minidump.enableFullMemoryDump(_config.dumpFullMemory);
		_minidump.setExceptionCB(MiniDumpCallback);		
	}
#else
	DUMP_PATH = _config.crushDumpPath.c_str();
#endif

	_config.snmpRegister("");
	glog(ZQ::common::Log::L_INFO, "ContentLibSvc::OnInit() leave");

	return S_OK;
}

HRESULT ContentLibSvc::OnUnInit(void)
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

	return S_OK;	
}


HRESULT ContentLibSvc::OnStart(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnStart() enter"));

	BaseZQServiceApplication::OnStart();

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
			return S_FALSE;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "ContentLibEnv object initialized"));
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibSvc, "Ice initialize properties met exception with error: %s, %s @ line %d"), 
			ex.ice_name().c_str(), ex.ice_file(), ex.ice_line());
		return S_FALSE;					
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnStart() leaved"));

	return S_OK;
}

HRESULT ContentLibSvc::OnStop(void)
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

		if(_communicator != NULL)
		{
			_communicator->destroy();
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

	// Ice object are not required to release, coz they are smart pointer object

	BaseZQServiceApplication::OnStop();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibSvc, "OnStop() leave"));

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

