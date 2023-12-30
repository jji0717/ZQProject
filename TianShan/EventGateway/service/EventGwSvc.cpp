#include <TianShanDefines.h>
#include "EventGwSvc.h"
#include "EventGateway.h"
#include "PluginHelper.h"
#include "EGConfig.h"
#include <IceLog.h>

EventGateway::EventGwService g_server;
ZQ::common::ZQDaemon *Application = &g_server;

ZQ::common::Config::Loader<EventGateway::EGConfig> gConfig("EventGateway.xml");
ZQ::common::Config::ILoader *configLoader = &gConfig;

extern const char* DUMP_PATH;

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


bool EventGwService::OnInit() {

	ZQ::common::setGlogger(_logger);
    // step 1: fixup the config
    fixupConfig(gConfig);

    // step 2: crash dump
    if(gConfig.dumpEnabled)
    {
        DUMP_PATH = gConfig.dumpPath.c_str();
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
        std::string logDir = _logDir;
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
    return ZQDaemon::OnInit();
}

void EventGwService::OnUnInit() {
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
    
    try {
        _adapter = NULL;
        _communicator->destroy();
        _communicator = NULL;
    } catch (const Ice::Exception&) {
    } catch (...) {
    }

    if(_iceTrace)
    {
        try{
            delete _iceTrace;
        }catch(...)
        {
        }
        _iceTrace = NULL;
    }

	ZQ::common::setGlogger(NULL);
   return ZQDaemon::OnUnInit();
}

bool EventGateway::EventGwService::OnStart() {
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
    	return false;
    }
    return ZQDaemon::OnStart();
}

void EventGwService::OnStop() {
    try{
        _moduleMgr->uninit();
    }catch(...)
    {	
    }
    try{
        _adapter->deactivate();
    }catch(...)
    {
    }
    return ZQDaemon::OnStop();
}

} // namespace EventGateway
