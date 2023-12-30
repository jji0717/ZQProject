#include "TianShanDefines.h"
#include "EventGwService.h"
#include "EventGateway.h"
#include "PluginHelper.h"
#include "EGConfig.h"
#include <IceLog.h>
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif

EventGateway::EventGwService g_server;
ZQ::common::BaseZQServiceApplication	*Application	= &g_server;

ZQ::common::Config::Loader<EventGateway::EGConfig> gConfig("EventGateway.xml");
ZQ::common::Config::ILoader *configLoader = &gConfig;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1 ;
DWORD gdwServiceInstance = 1;
// crash dump
ZQ::common::MiniDump _crashDump;
static void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
    DWORD dwThreadID = GetCurrentThreadId();	
    glog( ZQ::common::Log::L_ERROR,  "Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
    glog.flush();
}

static bool validatePath( const char *     szPath )
{
    if (-1 != ::GetFileAttributesA(szPath))
        return true;
    
    DWORD dwErr = ::GetLastError();
    if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
    {
        if (!::CreateDirectoryA(szPath, NULL))
        {
            dwErr = ::GetLastError();
            if ( dwErr != ERROR_ALREADY_EXISTS)
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    
    return true;
}
#else
extern const char* DUMP_PATH;
#endif

namespace EventGateway
{
using namespace ZQ::common;
/// constructor
EventGwService::EventGwService()
:_gw(NULL),_iceTrace(NULL)
{
}
/// destructor
EventGwService::~EventGwService()
{
}

static void setProperties(
                          Ice::PropertiesPtr proper, 
                          const std::map<std::string, std::string> &properties
                          )
{
    if(!proper)
        return;

    std::map<std::string, std::string>::const_iterator it;
    for(it = properties.begin(); it != properties.end(); ++it)
        proper->setProperty(it->first, it->second);
}
static void fixupDir(std::string &path)
{
    if(path.empty())
    {
        return;
    }
    else
    {
        if(path[path.size() - 1] != FNSEPC)
            path.push_back(FNSEPC);
    }
}
static void fixupConfig(EGConfig &config)
{
    // get the program root
    std::string tsRoot = ZQTianShan::getProgramRoot();
    // fixup the dump path
    if(gConfig.dumpEnabled && gConfig.dumpPath.empty())
    {
        // use the Root/logs as default dump folder
        gConfig.dumpPath = tsRoot + FNSEPS + "logs" + FNSEPS;
    }
    else
    {
        fixupDir(gConfig.dumpPath);
    }
    
    // fixup the plug-in's config path
    if(gConfig.plugins.configDir.empty())
    {
        // use the Root/etc as default config folder
        gConfig.plugins.configDir = tsRoot + FNSEPS + "etc" + FNSEPS;
    }
    else
    {
        fixupDir(gConfig.plugins.configDir);
    }
    
    // fixup the plug-in's log path
    if(gConfig.plugins.logDir.empty())
    {
        // get the program root
        std::string tsRoot = ZQTianShan::getProgramRoot();
        // use the Root/logs as default log folder
        gConfig.plugins.logDir = tsRoot + FNSEPS + "logs" + FNSEPS;
    }
    else
    {
        fixupDir(gConfig.plugins.logDir);
    }
    
    // expand the plug-in's populate path
    if(!gConfig.plugins.populatePath.empty())
    {
        gConfig.plugins.populate(gConfig.plugins.populatePath);
    }

    // fixup IceStorm endpoint
    if(gConfig.iceStormEndpoint.empty())
    {
        gConfig.iceStormEndpoint = DEFAULT_ENDPOINT_TopicManager;
    }
}


HRESULT EventGwService::OnInit(void)
{
	// covered by basesvc: ZQ::common::setGlogger(m_pReporter);
    glog(Log::L_DEBUG, CLOGFMT(EventGwService, "Enter initialization process..."));
    // step 1: fixup the config
    fixupConfig(gConfig);

    // step 2: crash dump
    if(gConfig.dumpEnabled)
    {
#ifdef ZQ_OS_MSWIN
        if(!validatePath(gConfig.dumpPath.c_str()))
        {
            glog(Log::L_ERROR, CLOGFMT(EventGwService, "OnInit() bad dump path [%s]"), gConfig.dumpPath.c_str());
            return S_FALSE;
        }
        // enable crash dump

        _crashDump.setDumpPath((char*)gConfig.dumpPath.c_str());
        _crashDump.enableFullMemoryDump(gConfig.fullMemoryDumpEnabled);
	    _crashDump.setExceptionCB(CrashExceptionCallBack);
#else
        DUMP_PATH = gConfig.dumpPath.c_str();
#endif
    }
    // step 3: init log & ice
    
    Ice::InitializationData initData;

    // init ice properties
    Ice::PropertiesPtr proper = Ice::createProperties();
    setProperties(proper, gConfig.iceProperties);
    initData.properties = proper;

    // create Ice trace
    if(gConfig.iceTraceEnabled)
    {
#ifdef ZQ_OS_MSWIN
        std::string logDir = m_wsLogFolder;
#else
        std::string logDir = _logDir;
#endif
        if(logDir.empty())
        {
            // get the program root
            std::string tsRoot = ZQTianShan::getProgramRoot();
            // use the Root/logs as default log folder
            logDir = tsRoot + FNSEPS + "logs" + FNSEPS;
        }
        else
        {
            fixupDir(logDir);
        }

        std::string iceTraceFile = logDir + "EventGwIceTrace.log";
        // create ice trace log
        try
        {
            _iceTrace = new FileLog(
                iceTraceFile.c_str(),
                gConfig.iceTraceLevel,
                gConfig.iceTraceCount,
                gConfig.iceTraceFileSize
                );
        }
        catch (const FileLogException& e)
        {
            glog(Log::L_ERROR, CLOGFMT(EventGwService, "caught [%s] during create file log [%s]."), e.getString(), iceTraceFile.c_str());
        }
        catch (...)
        {
            glog(Log::L_ERROR, CLOGFMT(EventGwService, "caught unknown exception during create file log [%s]."), iceTraceFile.c_str());
        }
    }

    // create Ice communicator
    if(_iceTrace)
    {
        TianShanIce::common::IceLogIPtr iceLogger = new TianShanIce::common::IceLogI(_iceTrace);
        initData.logger = iceLogger;
    }
    _communicator = Ice::initialize(initData);

    // create adapter for subscribers
    _adapter = _communicator->createObjectAdapterWithEndpoints("EventGateway", "tcp");
    // step 4: create servents
    std::string iceStormProxy = std::string(SERVICE_NAME_TopicManager":") + gConfig.iceStormEndpoint;
    _gw = new EventGw(
        _communicator,
        _adapter,
        glog,
        iceStormProxy,
        gConfig.plugins.configDir,
        gConfig.plugins.logDir,
        gConfig.connCheckIntervalMSec
        );

    _moduleMgr = new ModuleManager(_gw, glog);
    gConfig.snmpRegister("");
    glog(Log::L_INFO, CLOGFMT(EventGwService, "Initialized Ok."));
    return BaseZQServiceApplication::OnInit();
}

HRESULT EventGwService::OnUnInit(void)
{
    glog(Log::L_DEBUG, CLOGFMT(EventGwService, "Enter uninit process..."));
    if(_moduleMgr)
    {
        try
        {
            delete _moduleMgr;
        }
        catch (...)
        {
        }
        _moduleMgr = NULL;
    }

    if(_gw)
    {
        try{
            delete _gw;
        }catch(...)
        {
        }
        _gw = NULL;
    }
    _adapter = NULL;
    _communicator->destroy();
    _communicator = NULL;

    if(_iceTrace)
    {
        try{
            delete _iceTrace;
        }catch(...)
        {
        }
        _iceTrace = NULL;
    }
    glog(Log::L_INFO, CLOGFMT(EventGwService, "Uninitialized Ok."));
    return BaseZQServiceApplication::OnUnInit();
}

HRESULT EventGateway::EventGwService::OnStart(void)
{
    glog(Log::L_DEBUG, CLOGFMT(EventGwService, "Starting..."));
    try{
        // load modules
        std::set<std::string>::iterator it_module;
        for (it_module = gConfig.plugins.modules.begin(); it_module != gConfig.plugins.modules.end(); ++it_module)
        {
            _moduleMgr->add(*it_module);
        }

        _adapter->activate();
    }
    catch (...)
    {
        glog(Log::L_ERROR, CLOGFMT(EventGwService, "Unexpected exception during starting the service."));
    	return S_FALSE;
    }
    glog(Log::L_INFO, CLOGFMT(EventGwService, "Started Ok."));
    return BaseZQServiceApplication::OnStart();
}

HRESULT EventGwService::OnStop(void)
{
    glog(Log::L_DEBUG, CLOGFMT(EventGwService, "Stopping..."));
    if(_moduleMgr)
    {
        try{
            _moduleMgr->uninit();
        }catch(...)
        {
            glog(Log::L_ERROR, CLOGFMT(EventGwService, "Unexpected exception during uninit the modules."));
        }
    }
    else
    {
        glog(Log::L_WARNING, CLOGFMT(EventGwService, "OnStop() module manager not exist!"));
    }
    if(_adapter)
    {
        try{
            _adapter->deactivate();
        }catch(...)
        {
            glog(Log::L_ERROR, CLOGFMT(EventGwService, "Unexpected exception during deactivating the adapter."));
            return S_FALSE;
        }
    }
    else
    {
        glog(Log::L_WARNING, CLOGFMT(EventGwService, "OnStop() adapter not exist!"));
    }
    glog(Log::L_INFO, CLOGFMT(EventGwService, "Stopped Ok."));
    return BaseZQServiceApplication::OnStop();
}

void EventGwService::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();
	ServiceMIB_ExportByAPI(_pServiceMib, "eventGw-llevelIce", EventGwService, *this, uint32, AsnType_Int32, &EventGwService::getLogLevel_Ice, &EventGwService::setLogLevel_Ice, "");
}

uint32 EventGwService::getLogLevel_Ice()
{
	if (NULL == _iceTrace)
		return 0;

	return _iceTrace->getVerbosity();
}

void   EventGwService::setLogLevel_Ice(const uint32& newLevel)
{
	if (_iceTrace)
		_iceTrace->setVerbosity(newLevel);
}


} // namespace EventGateway
