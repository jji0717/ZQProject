#define SOCKADDR_IN6_DEFINED //avoid redefinition

#include "SentrySvc.h"
#include "FileLog.h"
#include "SentryConfig.h"

#ifdef ZQ_OS_LINUX
extern "C" {
#include <sys/resource.h>
}
#endif

SentryService g_server;
ZQ::common::ZQDaemon* Application	= &g_server;

ZQ::common::Config::Loader<SentryCfg> gSentryCfg("Sentry.xml");
ZQ::common::Config::ILoader *configLoader = &gSentryCfg;

extern const char* DUMP_PATH;

SentryService::SentryService():	
_pSentryEnv(0), _pSvcLog(0),_pIceLog(0),_pHttpLog(0),
_pIceLogPtr(0),_logparserman(0),_websvr(0), _pSpaceMonitor(0), _pIOMonitor(0) 
{
}

SentryService::~SentryService() {
}

bool SentryService::OnInit() {

    ZQ::common::setGlogger(_logger);

    std::string strLogFolder = _logDir;
    if(strLogFolder.at(strLogFolder.length()-1) != '/') {
        strLogFolder += '/';
    }

//    DUMP_PATH = gSentryCfg.crashDumpPath.c_str();

    // use ZQDaemon's log object
    _pSvcLog = _logger;

    _pHttpLog = new ZQ::common::FileLog((strLogFolder+"SentryHttp.log").c_str(),
                                        gSentryCfg.lServiceLogLevel,
                                        gSentryCfg.lServiceLogCount,
                                        gSentryCfg.lServiceLogSize,
                                        gSentryCfg.lServiceLogBuffer,
                                        gSentryCfg.lServiceLogTimeout,
                                        ZQLOG_DEFAULT_EVENTLOGLEVEL,
                                        "SentryService");


    _pIceLog = new ZQ::common::FileLog( (strLogFolder+"Sentry_Icetrace.log").c_str(),
                                        gSentryCfg.lIceLogLevel,
                                        gSentryCfg.lIceLogCount,
                                        gSentryCfg.lIceLogSize,
                                        10*1024,
                                        2,
                                        ZQLOG_DEFAULT_EVENTLOGLEVEL,"SentryService");
    _pIceLogPtr = new TianShanIce::common::IceLogI(_pIceLog);

    Ice::InitializationData initData;
    // init the ice properties
    Ice::PropertiesPtr proper= Ice::createProperties();
    {
        std::map<std::string, std::string>::const_iterator itProp;
        for(itProp = gSentryCfg.iceProps.begin(); itProp != gSentryCfg.iceProps.end(); ++itProp) {
            proper->setProperty(itProp->first, itProp->second);
        }
    }
    initData.properties = proper;

    if(gSentryCfg.lIceTraceEnable)
    {
        initData.logger = _pIceLogPtr;
    }

    ic=Ice::initialize(initData);
	
    threadpool = new ZQ::common::NativeThreadPool();
    if(!threadpool) {
        glog(ZQ::common::Log::L_ERROR,"failed to create thread pool");
        return false;
    }

    try
    {
        _pSentryEnv	= new ZQTianShan::Sentry::SentryEnv(*_pSvcLog,*threadpool,ic,gSentryCfg.szServiceEndpoint);
        _logparserman =  new ZQTianShan::Sentry::LogParserManagement(*_pSvcLog, ic, gSentryCfg.getConfigFilePath());
        _pSentryEnv->_logParserManagement = _logparserman;
    }
	catch (const ZQ::common::Exception& ex)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(Sentry, "Caught ZQ exception when create SentryEnv: %s"), ex.getString());
        return false;
    }
    catch (const Ice::Exception& ex)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(Sentry, "Caught Ice exception when create SentryEnv: %s"), ex.ice_name().c_str());
        return false;
    }
    catch (...) 
    {
        glog(ZQ::common::Log::L_ERROR,"catch an unexpect error when create SentryEnv");
        return false;
    }
	
    ///initialize http service
    _pSentryEnv->_pHttpSvcLog = _pHttpLog;
    try {
        _websvr = new WebServer(*_pSentryEnv);
    }
    catch(...) {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(Sentry, "Unknown exception uring creating WebServer."));
        return false;
    }

    try {
        _adapterCollector = new ZQTianShan::Sentry::AdapterCollectorImpl(*_pSentryEnv);
    }
    catch (...) {
        glog(ZQ::common::Log::L_ERROR,"catch an unexpect error when create AdapterCollector");
        return false;
    }
    try {
        _nodeService = new ZQTianShan::Sentry::SentryServiceImpl(*_pSentryEnv);
    }
    catch (...) {
        glog(ZQ::common::Log::L_ERROR,"catch an unexpect error when create SentryService");
        return false;
    }

	// NTP Client
    if(gSentryCfg.ntpClientCfg.ntpClientEnabled)
    {
		_pNtpClient = new (std::nothrow) NTPSync::NTPClient(_pSvcLog, *threadpool, 
			gSentryCfg.ntpClientCfg.ntpClientAdjustTimeout, 
			gSentryCfg.ntpClientCfg.ntpClientSyncInterval, 
			gSentryCfg.ntpClientCfg.timeMaxOffset);
    }
    else
    {
        _pNtpClient = NULL;
    }

    // NTP Server
    if(gSentryCfg.ntpServerEnabled)
    {
        _pNtpServer = new NTPSync::NTPServer(_pSvcLog, gSentryCfg.ntpServerListenAddress, gSentryCfg.ntpServerListenPort);
    }
    else
    {
        _pNtpServer = NULL;
    }

	//disk space monitor
	if(gSentryCfg.diskMonitor.enabled) {
        _pSpaceMonitor = new DiskSpaceMonitor(*_pSentryEnv, gSentryCfg.diskMonitor.pollInterval, gSentryCfg.diskMonitor.maxSkippedWarningCount, gSentryCfg.diskMonitor.warningTargets);
        for(size_t i = 0; i < gSentryCfg.diskMonitor.pathList.size(); ++i) {
            const DiskMonitorConf::DiskConf& diskInfo = gSentryCfg.diskMonitor.pathList[i];
            double freeWarning = 0.0;
            sscanf(diskInfo.freeWarning.c_str(), "%lf%%", &freeWarning);
            double repeatStep = 0.0;
            sscanf(diskInfo.repeatStep.c_str(), "%lf%%", &repeatStep);
            _pSpaceMonitor->add(diskInfo.path, freeWarning / 100, repeatStep / 100);
        }
        // add the disk list
    } else {
        _pSpaceMonitor = NULL;
    }

	//disk io monitor
	if(gSentryCfg.diskIOMonitor.enabled)
	{
		_pIOMonitor = new DiskIOMonitor(*_pSentryEnv, gSentryCfg.diskIOMonitor.monitorInterval, gSentryCfg.diskIOMonitor.warningTargets);
		if(_pIOMonitor != NULL)
		{
			for(size_t i = 0; i < gSentryCfg.diskIOMonitor.devsList.size(); ++i) 
			{
				const DiskIOMonitorConf::DiskDevs& devInfo = gSentryCfg.diskIOMonitor.devsList[i];
				double busy = 0.0;
				sscanf(devInfo.busyWarning.c_str(), "%lf%%", &busy);
				double queueSz = 0.0;
				sscanf(devInfo.queueSize.c_str(), "%lf", &queueSz);
				_pIOMonitor->addMonitorItem(devInfo.devName, busy, queueSz);
			}
        
		}

	}
	else
		_pIOMonitor = NULL;

    gSentryCfg.snmpRegister("");
    return ZQDaemon::OnInit();

}

bool SentryService::OnStart() {

    if(_pSpaceMonitor) 
	{
        _pSpaceMonitor->start();
    }
	if(_pIOMonitor)
	{
		_pIOMonitor->start();
	}
	if(_pNtpServer)
    {
        _pNtpServer->start();
    }
    if(_pNtpClient)
    {
        _pNtpClient->start();
    }

    if (_pSentryEnv) 
    {
        _pSentryEnv->_adapter->activate();
        _pSentryEnv->_loopbackAdapter->activate();
    }

    if(_websvr)
    {
        _websvr->start();
    }
    
#ifdef ZQ_OS_LINUX // an enh according to bug#15985
	struct rlimit rlim;
	rlim.rlim_cur = 8*1024;
	rlim.rlim_max = 64*1024;
	::setrlimit(RLIMIT_NOFILE, &rlim);
#endif
	
    return ZQDaemon::OnStart();
}

void SentryService::OnStop() {
    if(_pSpaceMonitor) 
	{
        _pSpaceMonitor->stop();
    }
	if(_pIOMonitor)
	{
		_pIOMonitor->stop();
	}
	if(_pNtpServer)
    {
        // stop the server
        _pNtpServer->stopNTPServer();
    }
    if(_pNtpClient)
    {
        _pNtpClient->stop();
    }

    if(_pSentryEnv)
    {
        _pSentryEnv->_adapter->deactivate();
        _pSentryEnv->_loopbackAdapter->deactivate();
    }
    if(_websvr)
    {
        _websvr->stop();
    }
    return ZQDaemon::OnStop();
}

void SentryService::OnUnInit() {

    glog(ZQ::common::Log::L_DEBUG, "Enter OnUnInit().");
    try
    {
		 // disk monitor
        if(_pSpaceMonitor) 
		{
            delete _pSpaceMonitor;
            _pSpaceMonitor = NULL;
        }
		if(_pIOMonitor)
		{
			delete _pIOMonitor;
			_pIOMonitor = NULL;
		}
        // ntp server
        if(_pNtpServer)
        {
            delete _pNtpServer;
            _pNtpServer = NULL;
        }
        // ntp client
        if(_pNtpClient)
        {
            delete _pNtpClient;
            _pNtpClient = NULL;
        }

        if (_logparserman) 
        {
            delete _logparserman;
            _logparserman= NULL;
        }
        glog(ZQ::common::Log::L_INFO, "Stop log parsing.");

        if(_websvr)
        {
            delete _websvr;
            _websvr = NULL;
        }
        glog(ZQ::common::Log::L_INFO, "Stop HTTP engine.");

        _nodeService = NULL;
        glog(ZQ::common::Log::L_INFO, "Stop node service.");

        if(threadpool) {
            delete threadpool;
            glog(ZQ::common::Log::L_INFO, "Stop Thread Pool");    
        }

        if(_pSentryEnv)
        {
            delete _pSentryEnv;
            _pSentryEnv = NULL;
        }
        glog(ZQ::common::Log::L_INFO, "Free environment.");

        _pIceLogPtr = NULL;

        ic->destroy();
        glog(ZQ::common::Log::L_INFO, "Destroy ICE Runtime");


        if (_pIceLog) 
        {
            delete _pIceLog;
            _pIceLog = NULL;
        }
        _pSvcLog = NULL;

        glog(ZQ::common::Log::L_INFO, "destroy log instance.");
    }
    catch (...) 
    {
        glog(ZQ::common::Log::L_ERROR, "Unexpect exception in OnUninit");
    }
    glog.flush();

    return ZQDaemon::OnUnInit();
}

// isHealth will be called during every reporting to shell
bool SentryService::isHealth(void)
{
	if(_pNtpClient)
    {
        return _pNtpClient->keepWorking();
    }
    return true;
}


