
#include "EventChannelSvc.h"
#include "EventChannelConfig.h"
#include <TianShanDefines.h>
#include <IceLog.h>
#include <boost/algorithm/string/trim.hpp>
extern "C"
{
#include <sys/stat.h>
#include <sys/types.h>
}
#include <TimeUtil.h>
using namespace ZQ::common;

EventChannelService g_server;
ZQ::common::ZQDaemon *Application = &g_server;

ZQ::common::Config::Loader<EventChannelConfig> gConfig("EventChannel.xml");
ZQ::common::Config::ILoader *configLoader = &gConfig;

extern const char* DUMP_PATH;

static std::string pathCat(const std::string &dir, const std::string& sub)
{
    std::string path;
    path.reserve(dir.size() + 1 + sub.size());
    if(!dir.empty() && dir[dir.size() - 1] != FNSEPC)
    {
        path = dir + FNSEPS;
    }
    else
    {
        path = dir;
    }

    path += boost::trim_copy_if(sub, boost::is_any_of(FNSEPS));

    return path;
}

static bool validatePath( const char* szPath )
{

	int rt = access(szPath, F_OK);
	if(rt == 0)
		return true;
	else if(errno == ENOENT || errno == ENOTDIR)
	{
		rt = mkdir(szPath, 0755);
		if(rt != 0)
			return false;

	}
	else
		return false;	

	return true;
}

static struct  
{
    std::string appName;
    std::string dbHome;
    std::string TopicMgrEndpoints;
    std::string PublishEndpoints;
    std::string libPath;
    std::string libEntry;
} gRuntimeConf; // the configuration used at runtime

class ConnControlData: public EventChannel::Sentinel::ExternalControlData {
public:
    int threshold;

    ConnControlData():threshold(300000), brokenAt_(0){}
    virtual void reportBadConnection(){
        ZQ::common::MutexGuard sync(lock_);
        if(brokenAt_ <= 0) {
            brokenAt_ = ZQ::common::now();
        } else {
            int64 brokenTime = ZQ::common::now() - brokenAt_;
            if(brokenTime >= threshold) { // expired
                glog(Log::L_NOTICE, CLOGFMT(EventChannelService, "The connection had been broken for %lld. Restarting..."), brokenTime);
                g_server.restartIceStorm();
                brokenAt_ = 0;
            }
        }
    }
    virtual void onConnectionEstablished() {
        ZQ::common::MutexGuard sync(lock_);
        brokenAt_ = 0;
    }

private:
    ZQ::common::Mutex lock_;
    int64 brokenAt_;
};
static ConnControlData gSentinelControlData; // control the Sentinel object by value

#define EC_AppName  "EventChannel"
#define IceVerMain ICE_INT_VERSION / 10000
#define IceVerSub ICE_INT_VERSION / 100 %100

static std::string int2str(int i)
{
    char buf[12] = {0};
    return itoa(i, buf, 10);
}

static bool checkConf()
{
    // set the runtime configuration
    gRuntimeConf.libPath = std::string("libIceStormService") + ".so." + int2str(IceVerMain) + int2str(IceVerSub);
    gRuntimeConf.libEntry = "createIceStorm";
    gRuntimeConf.appName = EC_AppName;
    gRuntimeConf.TopicMgrEndpoints = std::string("tcp -h ") + gConfig.bindHost + " -p " + int2str(gConfig.bindPort);
    gRuntimeConf.PublishEndpoints = std::string("tcp -h ") + gConfig.bindHost;

    // create the db directory
    gRuntimeConf.dbHome =  pathCat(gConfig.dataDir, gRuntimeConf.appName);
    if(!validatePath(gRuntimeConf.dbHome.c_str()))
    {
        glog(Log::L_ERROR, CLOGFMT(EventChannelService, "checkConf() Failed to create DB directory [%s]."), gRuntimeConf.dbHome.c_str());
        return false;
    }

    // check the crash dump path
    if(gConfig.crashDumpEnabled)
    {
        if(gConfig.crashDumpPath.empty() || (!validatePath(gConfig.crashDumpPath.c_str())))
        {
            std::string defaultDumpPath = std::string(ZQTianShan::getProgramRoot()) + FNSEPS + "Logs";
            glog(Log::L_WARNING, CLOGFMT(EventChannelService, "checkConf() Invalid dump path [%s] found. Use the default dump path [%s] instead."), gConfig.crashDumpPath.c_str(), defaultDumpPath.c_str());

            gConfig.crashDumpPath = defaultDumpPath;
            if(!validatePath(gConfig.crashDumpPath.c_str()))
            {
                glog(Log::L_ERROR, CLOGFMT(EventChannelService, "checkConf() Failed to create dump directory [%s]."), gConfig.crashDumpPath.c_str());
                return false;
            }
        }
        DUMP_PATH = gConfig.crashDumpPath.c_str();
    }

    // the self checking data
    gSentinelControlData.checkInterval = gConfig.selfcheckInterval;
    gSentinelControlData.lastCheckDelay = 0;
    gSentinelControlData.threshold = gConfig.selfcheckThreshold;
    return true;
}


/////////////////////// class EventChannelService /////////////////
EventChannelService::EventChannelService()
{
    _pIceStorm = NULL;
    _pIceTraceLog = NULL;
    _pSentinel = NULL;
    _bInService = false;
}

EventChannelService::~EventChannelService()
{
}

bool EventChannelService::OnInit()
{
    // validate the configuration
    if(!checkConf())
    {
        glog(Log::L_ERROR, CLOGFMT(EventChannelService, "OnInit() Bad configuration detected!"));
        return false;
    }

    Ice::InitializationData initData;
    // create ICE trace logger
    if(gConfig.iceTraceEnabled)
    {
        std::string iceTraceFileName = _logDir;
        if((*iceTraceFileName.rbegin()) != FNSEPC)
        {
            iceTraceFileName += FNSEPS "EventChannelIceTrace.log";
        }
        else
        {
            iceTraceFileName += "EventChannelIceTrace.log";
        }

        try
        {
            _pIceTraceLog = new FileLog(iceTraceFileName.c_str(), gConfig.iceTraceLevel, gConfig.iceTraceCount, gConfig.iceTraceSize);
        }
        catch(const FileLogException& e)
        {
            glog(Log::L_ERROR, CLOGFMT(EventChannelService, "OnInit() Caught FileLogException [%s] during creating Ice trace logger at [%s]."), e.getString(), iceTraceFileName.c_str());
        }

        if(NULL == _pIceTraceLog)
        {
            glog(Log::L_ERROR, CLOGFMT(EventChannelService, "OnInit() Failed to create Ice trace logger at [%s]."), iceTraceFileName.c_str());
            return false;
        }

        initData.logger = new TianShanIce::common::IceLogI(_pIceTraceLog);
        glog(Log::L_INFO, CLOGFMT(EventChannelService, "OnInit() Enable Ice trace logger at [%s]."), iceTraceFileName.c_str());
    }

    // initialize embedded IceStorm service
    { // ICE configuration
        Ice::PropertiesPtr props = Ice::createProperties();
        // setup the internal properties
        {
            // This property defines the endpoints on which the IceStorm
            // TopicManager listens.
            props->setProperty(gRuntimeConf.appName + ".TopicManager.Endpoints", gRuntimeConf.TopicMgrEndpoints);

            // This property is used by the administrative client to connect to IceStorm.
            //props->setProperty("EventChannel.TopicManager.Proxy", std::string("TianShanEvents/TopicManager:") + gRuntimeConf.TopicMgrEndpoints);

            // The IceStorm service instance name.
            props->setProperty(gRuntimeConf.appName + ".InstanceName", "TianShanEvents");

            // This property defines the home directory of the Freeze 
            // database environment for the IceStorm service.
            props->setProperty(std::string("Freeze.DbEnv.") + gRuntimeConf.appName + ".DbHome", gRuntimeConf.dbHome);

            // This property defines the endpoints on which the topic
            // publisher objects listen.
            props->setProperty(gRuntimeConf.appName + ".Publish.Endpoints", gRuntimeConf.PublishEndpoints);

            // the trace setting
            props->setProperty(gRuntimeConf.appName + ".Trace.TopicManager", (gConfig.iceTraceVerbose ? "2" : "1"));
            props->setProperty(gRuntimeConf.appName + ".Trace.Topic", "1"); // Topic Tracing
            props->setProperty(gRuntimeConf.appName + ".Trace.Subscriber", "1"); // Subscriber Tracing
            props->setProperty(gRuntimeConf.appName + ".Trace.Flush", "1");// Flush Tracing (for batch mode transfer flushing)
            // Amount of time in milliseconds between flushes for batch mode
            // transfer. The minimum allowable value is 100ms.
            props->setProperty(gRuntimeConf.appName + ".Flush.Timeout", "2000");
        }
        EventChannelConfig::Properties::const_iterator itProp;
        for (itProp = gConfig.properties.begin(); itProp != gConfig.properties.end(); ++itProp)
        {
            props->setProperty(itProp->first, itProp->second);
        }
        initData.properties = props;
    }

    _pIceStorm = new EmbeddedIceStorm(glog, gRuntimeConf.appName);

    if(NULL == _pIceStorm)
    {
        glog(Log::L_ERROR, CLOGFMT(EventChannelService, "OnInit() Failed to create EventChannel instance!"));
        return false;
    }

    if(!_pIceStorm->setup(gRuntimeConf.libPath, gRuntimeConf.libEntry, initData))
    {
        glog(Log::L_ERROR, CLOGFMT(EventChannelService, "OnInit() Failed to setup EventChannel service."));
        return false;
    }

    _pSentinel = new EventChannel::Sentinel(glog, std::string("TianShanEvents/TopicManager:") + gRuntimeConf.TopicMgrEndpoints, &gSentinelControlData);

    // register the Sentinel data
    SNMPManageVariable("Self Checking Interval", &(gSentinelControlData.checkInterval), ZQSNMP_VARTYPE_INT32, false);
    SNMPManageVariable("Last Self Check Delay", &(gSentinelControlData.lastCheckDelay), ZQSNMP_VARTYPE_INT32, true);
    gConfig.snmpRegister("");

    return ZQDaemon::OnInit();
}

static void applyPreConf(Ice::CommunicatorPtr comm);
bool EventChannelService::OnStart()
{
    if(!_pIceStorm->start())
    {
        glog(Log::L_ERROR, CLOGFMT(EventChannelService, "OnStart() Failed to start EventChannel service."));
        return false;
    }
    _pSentinel->start();

    applyPreConf(_pIceStorm->communicator());
    _bInService = true;
    return ZQDaemon::OnStart();
}

void EventChannelService::OnStop()
{
    _bInService = false;
    _pSentinel->stop();
    _pIceStorm->stop();

    ZQDaemon::OnStop();
}

void EventChannelService::OnUnInit()
{
    _bInService = false;
	if(_pSentinel)
	{
   		delete _pSentinel;
    	_pSentinel = NULL;
	}
	
	if(_pIceStorm)
	{
    	delete _pIceStorm;
    	_pIceStorm = NULL;
	}

	if(_pIceTraceLog)
	{
    	delete _pIceTraceLog;
    	_pIceTraceLog = NULL;
	}

    ZQDaemon::OnUnInit();
}
void EventChannelService::restartIceStorm() {
    if(_bInService && _pIceStorm) {
        _pIceStorm->stop();
        _pIceStorm->start();
        glog(Log::L_NOTICE, CLOGFMT(EventChannelService, "The EventChannel restarted."));
    } else {
        glog(Log::L_WARNING, CLOGFMT(EventChannelService, "The EventChannel is not in service now. Ignore the restart command."));
    }
}
// pre-configure the EventChannel
static void applyPreConf(Ice::CommunicatorPtr comm) {
    try {
        Ice::ObjectPrx prx = comm->stringToProxy("TianShanEvents/TopicManager:" + gRuntimeConf.TopicMgrEndpoints);
        IceStorm::TopicManagerPrx topicMgr = IceStorm::TopicManagerPrx::uncheckedCast(prx);

#define CREATE_TOPIC(NAME) try { topicMgr->create(NAME); } catch (const IceStorm::TopicExists&){}

        const TopicConf::Topics& topics = gConfig.preConf.topics;
        for(size_t i = 0; i < topics.size(); ++i) {
            CREATE_TOPIC(topics[i]);
            glog(Log::L_INFO, CLOGFMT(PreConf, "Created topic(%s)"), topics[i].c_str());
        }

        // create links
        const TopicConf::TopicLinks& links = gConfig.preConf.links;
        for(size_t i = 0; i < links.size(); ++i) {
            const TopicConf::TopicLink& link = links[i];
            CREATE_TOPIC(link.from);
            IceStorm::TopicPrx linkFrom = topicMgr->retrieve(link.from);
            CREATE_TOPIC(link.to);
            IceStorm::TopicPrx linkTo = topicMgr->retrieve(link.to);

            // unlink first to apply the COST 
            try { linkFrom->unlink(linkTo); } catch (const IceStorm::NoSuchLink&) {}
            try { linkFrom->link(linkTo, link.cost); } catch(const IceStorm::LinkExists&) {}
            glog(Log::L_INFO, CLOGFMT(PreConf, "Linked topic(%s -> %s) with cost(%d)"), link.from.c_str(), link.to.c_str(), link.cost);
        }
    } catch (const Ice::Exception& e) {
        glog(Log::L_WARNING, CLOGFMT(PreConf, "Got %s when configure the EventChannel. desc(%s)"), e.ice_name().c_str(), e.what());
    } catch (...) {
        glog(Log::L_WARNING, CLOGFMT(PreConf, "Got unexpected exception when configure the EventChannel."));
    }
}
