// SentryService.cpp : Defines the entry point for the console application.
//
#define SOCKADDR_IN6_DEFINED //avoid redefinition

#include <boost/thread.hpp>
#include "ZQ_common_conf.h"
#include "FileLog.h"
#ifdef ZQ_OS_MSWIN
#include <minidump.h>
#else
#include <arpa/inet.h>
#endif
#include "SentryService.h"
#include "SentryConfig.h"
#include "SystemUtils.h"

using namespace ZQ::common;

SentryService			g_server;
ZQ::common::BaseZQServiceApplication	*Application	= &g_server;

ZQ::common::Config::Loader<SentryCfg> gSentryCfg("Sentry.xml");
ZQ::common::Config::ILoader *configLoader = &gSentryCfg;

extern const char* DUMP_PATH;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType =0 ;

ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);


void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
    DWORD dwThreadID = GetCurrentThreadId();
	
    glog( Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
          ExceptionCode, ExceptionAddress, dwThreadID);
	
    glog.flush();
}
#endif

SentryService::SentryService():
_pSentryEnv(0), _pSvcLog(0),_pIceLog(0),_pHttpLog(0),_pNtpClient(0),_pNtpServer(0),
_pIceLogPtr(0),_logparserman(0),_websvr(0), _pSpaceMonitor(0), _pIOMonitor(0),
_bLastRefreshModuleTable(false)
{	
    _lastRefreshTime = ZQ::common::TimeUtil::now();
}

SentryService::~SentryService()
{
}

#ifdef ZQ_OS_MSWIN
static bool validatePath( const char* szPath )
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

static bool checkConf()
{
    ZQ::common::Config::Loader<SentryCfg> &gConfig = gSentryCfg;
    // check the crash dump path
    if(gConfig.crashDumpEnabled)
    {
        if(gConfig.crashDumpPath.empty() || (!validatePath(gConfig.crashDumpPath.c_str())))
        {
            std::string defaultDumpPath = std::string(ZQTianShan::getProgramRoot()) + FNSEPS + "Logs";
            glog(Log::L_WARNING, CLOGFMT(SentryService, "checkConf() Invalid dump path [%s] found. Use the default dump path [%s] instead."), gConfig.crashDumpPath.c_str(), defaultDumpPath.c_str());

            gConfig.crashDumpPath = defaultDumpPath;
            if(!validatePath(gConfig.crashDumpPath.c_str()))
            {
                glog(Log::L_ERROR, CLOGFMT(SentryService, "checkConf() Failed to create dump directory [%s]."), gConfig.crashDumpPath.c_str());
                return false;
            }
        }
    }
    return true;
}
#endif

HRESULT SentryService::OnInit()
{
    ZQ::common::Config::Loader<SentryCfg> &gConfig = gSentryCfg;
    //step 1.locate the crash dump folder

#ifdef ZQ_OS_MSWIN
    // validate the configuration
    if(!checkConf())
    {
        glog(Log::L_ERROR, CLOGFMT(SentryService, "OnInit() Bad configuration detected!"));
        return S_FALSE;
    }
    // enable crash dump
    if(gConfig.crashDumpEnabled)
    {
        _crashDump.setDumpPath((char*)gConfig.crashDumpPath.c_str());
        _crashDump.enableFullMemoryDump(true);
        _crashDump.setExceptionCB(CrashExceptionCallBack);
        glog(Log::L_INFO, CLOGFMT(SentryService, "OnInit() Enable crash dump at [%s]"), gConfig.crashDumpPath.c_str());
    }
    std::string strLogFolder = m_wsLogFolder;
#else
    std::string strLogFolder = _logDir;
	DUMP_PATH = gConfig.crashDumpPath.c_str();
#endif
    if( strLogFolder[strLogFolder.length()-1]!=FNSEPC) {
        strLogFolder+=FNSEPS;
	}

    // use BaseZQServiceApplication's log object
    _pSvcLog = m_pReporter;

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
        for(itProp = gSentryCfg.iceProps.begin(); itProp != gSentryCfg.iceProps.end(); ++itProp)
            proper->setProperty(itProp->first, itProp->second);
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
        return S_FALSE;
    }

    try
    {
        _pSentryEnv	= new ::ZQTianShan::Sentry::SentryEnv(*_pSvcLog,*threadpool,ic,gSentryCfg.szServiceEndpoint);
        _logparserman =  new ::ZQTianShan::Sentry::LogParserManagement(*_pSvcLog, ic, gSentryCfg.getConfigFilePath());
        _pSentryEnv->_logParserManagement = _logparserman;
    }
    catch (const ZQ::common::Exception& ex)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(Sentry, "Caught ZQ exception when create SentryEnv: %s"), ex.getString());
        return S_FALSE;
    }
    catch (const Ice::Exception& ex)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(Sentry, "Caught Ice exception when create SentryEnv: %s"), ex.ice_name().c_str());
        return S_FALSE;
    }
    catch (...) 
    {
#ifdef ZQ_OS_MSWIN
        logEvent(Log::L_EMERG,"Unexpect error when create SentryEnv");
#endif
        return S_FALSE;
    }
	
    ///initialize http service
    _pSentryEnv->_pHttpSvcLog = _pHttpLog;
    try
    {
        _websvr = new WebServer(*_pSentryEnv);
    }
    catch(...)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(Sentry, "Unknown exception when creating WebServer."));
        return S_FALSE;
    }

    try
    {
        _adapterCollector = new ZQTianShan::Sentry::AdapterCollectorImpl(*_pSentryEnv);
    }
    catch (...) 
    {
#ifdef ZQ_OS_MSWIN
        logEvent(Log::L_ERROR,"catch an unexpect error when create AdapterCollector");
#endif
        return S_FALSE;
    }
    try
    {
        _nodeService = new ZQTianShan::Sentry::SentryServiceImpl(*_pSentryEnv);
    }
    catch (...) 
    {
#ifdef ZQ_OS_MSWIN
        logEvent(Log::L_ERROR,"catch an unexpect error when create SentryService");
#endif
        return S_FALSE;
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
    // disk monitor
    if(gSentryCfg.diskMonitor.enabled) {
        _pSpaceMonitor = new DiskSpaceMonitor(*_pSentryEnv, gSentryCfg.diskMonitor.pollInterval, gSentryCfg.diskMonitor.maxSkippedWarningCount, gSentryCfg.diskMonitor.warningTargets);
        for(size_t i = 0; i < gConfig.diskMonitor.pathList.size(); ++i) {
            const DiskMonitorConf::DiskConf& diskInfo = gConfig.diskMonitor.pathList[i];
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
			for(size_t i = 0; i < gConfig.diskIOMonitor.devsList.size(); ++i) 
			{
				const DiskIOMonitorConf::DiskDevs& devInfo = gConfig.diskIOMonitor.devsList[i];
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

    //gSentryCfg.snmpRegister("");
    return BaseZQServiceApplication::OnInit();

}
HRESULT SentryService::OnStart()
{
    if(_pSpaceMonitor) {
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
    //	_serviceFrm.begin(&_connID);
	
    return BaseZQServiceApplication::OnStart();
}
HRESULT SentryService::OnStop()
{
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
    //    _serviceFrm.end();
    if(_websvr)
    {
        _websvr->stop();
    }
    return BaseZQServiceApplication::OnStop();
}
HRESULT SentryService::OnUnInit()
{
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

        // stop log parsing
        if (_logparserman) 
        {
            delete _logparserman;
            _logparserman= NULL;
        }
        glog(ZQ::common::Log::L_INFO, "Stop log parsing.");
        // stop HTTP
        if(_websvr)
        {
            delete _websvr;
            _websvr = NULL;
        }
        glog(ZQ::common::Log::L_INFO, "Stop HTTP engine.");
        // stop ice service
        _nodeService = NULL;
        glog(ZQ::common::Log::L_INFO, "Stop node service.");

        if(threadpool) {
            delete threadpool;
            glog(ZQ::common::Log::L_INFO, "Stop Thread Pool");    
        }

        // free environment
        if(_pSentryEnv)
        {
            delete _pSentryEnv;
            _pSentryEnv = NULL;
        }
        glog(ZQ::common::Log::L_INFO, "Free environment.");

        ic->destroy();
        glog(ZQ::common::Log::L_INFO, "Destroy ice runtime.");

        // destroy log instance
        _pIceLogPtr = NULL;
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
        glog(ZQ::common::Log::L_ERROR, "Unexpected exception during the OnUninit.");
    }
    glog.flush();
    return BaseZQServiceApplication::OnUnInit();
}

// isHealth will be called during every reporting to shell
bool SentryService::isHealth(void)
{
    int64 now = ZQ::common::TimeUtil::now();
    if ((now - _lastRefreshTime) > 30*1000)
    {
        if (_bLastRefreshModuleTable)
        {
            refreshNeighborsTable();
        }else{
            refreshModulesTable();
        }
        _bLastRefreshModuleTable = _bLastRefreshModuleTable ? false : true;
        _lastRefreshTime = ZQ::common::TimeUtil::now();
    }

    if(_pNtpClient)
    {
        return _pNtpClient->keepWorking();
    }
    else
    {
        return true;
    }
}

void SentryService::doEnumSnmpExports()
{
    BaseZQServiceApplication::doEnumSnmpExports();

    refreshModulesTable();     // add tsModulesTable
    refreshNeighborsTable();   // add sentryNeighborsTable
}

void SentryService::refreshModulesTable()
{
    static ZQ::SNMP::Oid subOidTbl;
    if (subOidTbl.isNil())
        _pServiceMib->reserveTable("tsModulesTable", 7, subOidTbl);
    if (subOidTbl.isNil())
    {
        glog(ZQ::common::Log::L_WARNING, CLOGFMT(SentryService,"doEnumSnmpExports() failed to locate tsModulesTable in MIB"));
        return;
    }

    // clean up the table content
    ZQ::SNMP::Oid tmpOid(subOidTbl);
    tmpOid.append(1);
    _pServiceMib->removeSubtree(tmpOid);

    //////////////////////////////////////////////////////////////////////////
    ZQTianShan::Sentry::SentryEnv::FatNodeMap  fatNodeMap;
    if (!_pSentryEnv || !_pServiceMib)
        return;

    _pSentryEnv->gatherNodeMap(fatNodeMap);
    glog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryEnv, "create modules table end, column[%d], fatNodeMap size[%d]"), 7, fatNodeMap.size());

    int rowIndex = 1;
    for (ZQTianShan::Sentry::SentryEnv::FatNodeMap::iterator it = fatNodeMap.begin(); it != fatNodeMap.end(); it++)
    {
        ZQTianShan::Sentry::SentryEnv::FatNodeInfo& nodeinfo = it->second;
        if(nodeinfo.services.empty())
            continue;

        for (::ZqSentryIce::ServiceInfos::iterator itVector = nodeinfo.services.begin();
            itVector < nodeinfo.services.end();
            ++itVector, ++rowIndex)
        {
            char tempbuf[80];
            memset(tempbuf, 0, sizeof(tempbuf));
            std::string  interfaces (itVector->name);
            std::string  adapterId (itVector->adapterId);
            int          processId (itVector->processId);
            std::string  endpoint  (itVector->proxystr);
            //int64        lastChange = nodeinfo.baseNodeInfo.lastChange;
            std::string  lastChange(ZQTianShan::TimeToUTC(nodeinfo.baseNodeInfo.lastChange, tempbuf, sizeof(tempbuf)-2));

            int          offEndpoint =  endpoint.find("-h") + 2;
            int          countIp     = endpoint.find("-p") - offEndpoint - 1;
            std::string  ipSubEndpoint(endpoint.substr(offEndpoint, countIp));
            if( gSentryCfg.reverseDomainName >= 1 )
            {
                uint64 startReverse = ZQ::common::now();
                struct hostent* host = gethostbyname(ipSubEndpoint.c_str());
                uint64 timeDelta = ZQ::common::now() - startReverse;
                if( timeDelta > 0 )
                {
                    glog(ZQ::common::Log::L_DEBUG, CLOGFMT(SentryService,"it took us [%llu]ms to reverse domain name [%s] to ip addr"),
                        timeDelta,ipSubEndpoint.c_str() );
                }
                if(host && host->h_length > 0 && host->h_addr != 0 )
                {
                    struct in_addr in = {0};
                    memcpy((void*)&in,(void*)host->h_addr,sizeof(in));
                    ipSubEndpoint = inet_ntoa(in);
                }				
            }

            _pServiceMib->addTableCell(subOidTbl,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("hostName", nodeinfo.baseNodeInfo.name));
            _pServiceMib->addTableCell(subOidTbl,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("interface", interfaces));
            _pServiceMib->addTableCell(subOidTbl,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("adapter", adapterId));
            _pServiceMib->addTableCell(subOidTbl,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("endpoint", endpoint));
            _pServiceMib->addTableCell(subOidTbl,  5, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("pid", (int32)processId));
            _pServiceMib->addTableCell(subOidTbl,  6, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("activated", lastChange));
            _pServiceMib->addTableCell(subOidTbl,  7, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("ip", ipSubEndpoint));

        }
    }
}
void SentryService::refreshNeighborsTable()
{
    static ZQ::SNMP::Oid subOidTbl;
    if (subOidTbl.isNil())
        _pServiceMib->reserveTable("tsNeighborTable", 7, subOidTbl);
    if (subOidTbl.isNil())
    {
        glog(ZQ::common::Log::L_WARNING, CLOGFMT(SentryService,"fillNeighborsTable() failed to locate tsModulesTable in MIB"));
        return;
    }

    // clean up the table content
    ZQ::SNMP::Oid tmpOid(subOidTbl);
    tmpOid.append(1);
    _pServiceMib->removeSubtree(tmpOid);

    //////////////////////////////////////////////////////////////////////////
    ZQTianShan::Sentry::SentryEnv::FatNodeMap fatNodeMap;
    if (!_pSentryEnv || !_pServiceMib)
        return;

	_pSentryEnv->gatherNodeMap(fatNodeMap);

    int rowIndex = 1;
    for (ZQTianShan::Sentry::SentryEnv::FatNodeMap::iterator it = fatNodeMap.begin();
        it != fatNodeMap.end(); ++it, ++rowIndex)
    {
        char tempbuf[80];
        memset(tempbuf, 0, sizeof(tempbuf));
        ZQTianShan::Sentry::SentryEnv::FatNodeInfo& NodeInfoMap = it->second;
        std::ostringstream out;
        std::string    memory;
        std::string	   processor;
        std::string    nodeId(NodeInfoMap.baseNodeInfo.id );
        std::string    hostName(NodeInfoMap.baseNodeInfo.name);
        std::string    rootURL((NodeInfoMap.baseNodeInfo).adminRootUrl);
        std::string    OS(NodeInfoMap.baseNodeInfo.os); 
        //int64          startUp = NodeInfoMap.baseNodeInfo.osStartup;
        std::string    startUp(ZQTianShan::TimeToUTC(NodeInfoMap.baseNodeInfo.osStartup, tempbuf, sizeof(tempbuf)-2));

        out<< NodeInfoMap.baseNodeInfo.memTotalPhys/1024/1024 << "MB / " << NodeInfoMap.baseNodeInfo.memTotalVirtual/1024/1024 << "MB";
        memory = out.str();
        out.str("");

        out<< NodeInfoMap.baseNodeInfo.cpu << "  " << NodeInfoMap.baseNodeInfo.cpuClockMHz << "MHz x" << (NodeInfoMap.baseNodeInfo).cpuCount ;
        processor = out.str();
        out.str("");

        _pServiceMib->addTableCell(subOidTbl,  1, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("neighborName", hostName));
        _pServiceMib->addTableCell(subOidTbl,  2, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("nodeId", nodeId));
        _pServiceMib->addTableCell(subOidTbl,  3, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("rootURL", rootURL));
        _pServiceMib->addTableCell(subOidTbl,  4, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("processor", processor));
        _pServiceMib->addTableCell(subOidTbl,  5, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("memory", memory));
        _pServiceMib->addTableCell(subOidTbl,  6, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("os", OS));
        _pServiceMib->addTableCell(subOidTbl,  7, rowIndex, new ZQ::SNMP::SNMPObjectDupValue("startup", startUp));
    }
}
