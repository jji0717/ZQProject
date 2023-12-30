// CME.cpp : Defines the entry point for the console application.
//
#include <sys/stat.h>
#include <sys/types.h>
#include "Log.h"
#include "FileLog.h"
#include "ConfigLoader.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "CMECfg.h"
#include  "TimeUtil.h"
#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#endif

#include "CMESvc.h"
#include "CMEServiceSoapBinding.nsmap"

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
#endif

#define     MAX_PATH_LEN            260
#define		MAX_SESS_PLAY_TIME		6*3600		// 6 hours

#define     MAX_PROACITVE_LINE      1024
#define     DATETIME_NA				"N/A"
#define     NA                      "N/A"

CacheManagement::CMESvc g_server;
ZQ::common::BaseZQServiceApplication* Application = &g_server;
ZQ::common::Config::ILoader *configLoader=&CacheManagement::_gCMECfg;

#ifdef ZQ_OS_MSWIN
ZQ::common::MiniDump			_crashDump;

void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();	
	glog( ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
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


namespace CacheManagement
{
using namespace ZQ::common;

static bool isValidTimeStr(const char* timeStr)
{
	int nYear,nMon,nDay, nHour, nMin, nSec;
	if (::sscanf(timeStr, "%d-%d-%dT%d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec) <6
		&& ::sscanf(timeStr, "%4d%2d%2dT%d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec) <6

		&& ::sscanf(timeStr, "%d-%d-%d %d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec) <6
		&& ::sscanf(timeStr, "%4d%2d%2d %d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec) <6
		)
	{
		return false;
	}

	if (nYear < 1970 || nYear > 2100 || nMon < 1 || nMon > 12 || nDay < 1 || nDay > 31 || 
		nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59)
		return false;

	return true;
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

static const char* getProgramPath()
{
	std::string programPath;
	std::string programRoot;
	std::string modulesPath;
	
	char path[MAX_PATH];
#ifdef ZQ_OS_MSWIN
	if (::GetModuleFileNameA(NULL, path, MAX_PATH-2) >0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
			{
				*p='\0';
				programRoot = path;
				modulesPath = programRoot + FNSEPS "modules";
			}
			else if (NULL !=p && (0==stricmp(FNSEPS "bin64", p) || 0==stricmp(FNSEPS "exe64", p)))
			{
				*p='\0';
				programRoot = path;
				modulesPath = programRoot + FNSEPS "modules64";
			}
		}
        else
        {
            programRoot = modulesPath = path;
        }
	}
	else programRoot = modulesPath = "." ;

#else
	int res = readlink("/proc/self/exe", path, sizeof(path));
	if(res < 0 || res > (int)sizeof(path))
	{
		programRoot = modulesPath = "." ;
	}
	else
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==strcasecmp(FNSEPS "bin", p) || 0==strcasecmp(FNSEPS "exe", p)))
			{
				*p='\0';
				programRoot = path;
				modulesPath = programRoot + FNSEPS "modules";
			}
			else if (NULL !=p && (0==strcasecmp(FNSEPS "bin64", p) || 0==strcasecmp(FNSEPS "exe64", p)))
			{
				*p='\0';
				programRoot = path;
				modulesPath = programRoot + FNSEPS "modules64";
			}
		}
        else
        {
            programRoot = modulesPath = path;
        }		
	}
#endif

	return programPath.c_str();
}

static void fixupConfig(CMECfg &config)
{
    // get the program root
	std::string tsRoot = getProgramPath();
    // fixup the dump path
	if(config._crashDumpEnabled && config._crashDumpPath.empty())
    {
        // use the Root/logs as default dump folder
		config._crashDumpPath = tsRoot + FNSEPS + "logs" + FNSEPS;
    }
    else
    {
		fixupDir(config._crashDumpPath);
    }    
}




CMESvc::CMESvc()
{
#ifdef ZQ_OS_MSWIN
	strcpy(servname, "CMEV2Svc");
	strcpy(prodname, "TianShan");	
#endif
	_pCMEMain = NULL;
}

CMESvc::~CMESvc()
{
	// stop the main thread and release the object
	if(NULL != _pCMEMain)
	{
		_pCMEMain->stop();
		delete _pCMEMain;
	}
	_pCMEMain = NULL;
}

HRESULT CMESvc::OnInit()
{
	glog(Log::L_INFO, CLOGFMT(CMESvc, "Entering initialize process..."));

    // step 1: fixup the config
    fixupConfig(_gCMECfg);

    // step 2: crash dump
	if(_gCMECfg._crashDumpEnabled)
    {
#ifdef ZQ_OS_MSWIN
		if(!validatePath(_gCMECfg._crashDumpPath.c_str()))
        {
            glog(Log::L_ERROR, CLOGFMT(CMESvc, "OnInit() bad dump path [%s]"), _gCMECfg._crashDumpPath.c_str());
            return S_FALSE;
        }
        // enable crash dump
        _crashDump.setDumpPath((char*)_gCMECfg._crashDumpPath.c_str());
		_crashDump.enableFullMemoryDump(_gCMECfg._dumpFullMemory);
	    _crashDump.setExceptionCB(CrashExceptionCallBack);
#else
        DUMP_PATH = _gCMECfg._crashDumpPath.c_str();
#endif
    }   

	// parse CME endpoint since configured endpoint is <IP:Port>
	bool ret = _gCMECfg.parseCMEEndpoint();
	if(!ret)
	{
		glog(Log::L_ERROR, CLOGFMT(CMESvc, "CME endpoint is invalid, it should be either http://<IP:Port> or <IP:Port>"));
		return S_FALSE;
	}

	ret = _gCMECfg.parseLAMEndpoint();
	if(!ret)
	{
		glog(Log::L_ERROR, CLOGFMT(CMESvc, "LAM endpoint is invalid, it should be either http://<IP:Port> or <IP:Port>"));
		return S_FALSE;
	}

	// create the main thread
	_pCMEMain = new CMEService();
	_pCMEMain->start();

    _gCMECfg.snmpRegister("");
    glog(Log::L_INFO, CLOGFMT(CMESvc, "Initialized Ok."));
    return BaseZQServiceApplication::OnInit();
}

HRESULT CMESvc::OnStart()
{	
	glog(Log::L_INFO, CLOGFMT(CMESvc, "CacheManagementService Enter OnStart()"));

	glog(Log::L_INFO, CLOGFMT(CMESvc, "CacheManagementService Exit OnStart()"));

	return BaseZQServiceApplication ::OnStart();	
}

HRESULT CMESvc::OnStop()
{	
	glog(Log::L_INFO, CLOGFMT(CMESvc, "CacheManagementService Enter OnStop()"));
		
	if(NULL != _pCMEMain)
		_pCMEMain->stop();

	glog(Log::L_INFO, CLOGFMT(CMESvc, "CacheManagementService Exit OnStop()"));

	return BaseZQServiceApplication::OnStop();	
}

HRESULT CMESvc::OnUnInit()
{
	glog(Log::L_INFO, CLOGFMT(CMESvc, "CacheManagementService Enter OnUnInit()"));

	if(NULL != _pCMEMain)
		_pCMEMain->stop();

	glog(Log::L_INFO, CLOGFMT(CMESvc, "CacheManagementService Exit OnUnInit()"));
	
	return BaseZQServiceApplication ::OnUnInit();	
}

void CMESvc::OnSnmpSet( const char* varName)
{
	if(0 == strcmp(varName, "default/IceTrace/level"))
	{
	}
}

CMEService::CMEService()
{
	_cmeAlgorithm = NULL;
	_pLamSOAPClient = NULL;
	_pCMESOAPServer = NULL;
	_threadRunnng = false;
}

CMEService::~CMEService()
{
}

bool CMEService::start( )
{
	_threadRunnng = true;
	return NativeThread::start();
}

void CMEService::stop( )
{	
	if(!_threadRunnng)
		return;

	glog(Log::L_INFO, CLOGFMT(CMEService, "Stopping CMEService thread"));

	// signal the wait event, to notify the thread to exit 
	_threadRunnng = false;
	_waitEvent.signal();
	waitHandle(2000);

	glog(Log::L_INFO, CLOGFMT(CMEService, "CMEService thread stopped"));
}

bool CMEService::init(void)
{
	return NativeThread::init();
}

int CMEService::run(void)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "enter run()"));

	int i=0;
	bool ret = false;

	int retryInterval = 10; // seconds

	// 1. fetch cluster and nodes IP configuraiton, we don't know other information returned from LAM
	//
	// create objects for SOAP client(to connect LAM) and server(connect from EventGateway)
	_pLamSOAPClient = new CMELAMSOAPClient(_gCMECfg._lamEndPoint, _gCMECfg._cmeEndPoint); //this is not a thread object

	// initialize CME LAM SOAP client to get cluster information
	while(!ret && _threadRunnng)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "LamSOAPClient initialize ..."));

		// initialize to read cluster conifguration from LAM and save to _pMCGroup
		ret = _pLamSOAPClient->initialize();
		if(!ret)
		{
			glog(Log::L_WARNING, CLOGFMT(CMEService, "CME LAM SOAP Client initialize failed, retry %d seconds later"), 
				retryInterval);
			
			// to try in 10 seconds later
			SYS::SingleObject::STATE st = _waitEvent.wait(retryInterval*1000);

			if (st != SYS::SingleObject::TIMEDOUT)
			{
				// get notification to stop the thread, exit directly
				// nothing else need to be rleased addtionally
#ifdef ZQ_OS_MSWIN
				g_server.stopService();
#else
				g_server.unInit();
#endif
				return 0;
			}
			// continue for the retry
			continue;
		}
	}
	
	if(!_threadRunnng)
		return 0;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "LamSOAPClient retrieve cluster configuration ..."));

	ret = false;
	while(!ret && _threadRunnng)
	{
		// get cluster list from LAM
		ret = _pLamSOAPClient->listCluster(_lamClusters);
		if(!ret || _lamClusters.size() == 0)
		{
			if(_lamClusters.size() == 0)
			{
				ret = false;
			}

			glog(Log::L_WARNING, CLOGFMT(CMEService, "CME failed to get valid cluster list from LAM, retry %d seconds later. Please check LAM volume configuration"),
				retryInterval);

			// to try in 10 seconds later
			SYS::SingleObject::STATE st = _waitEvent.wait(retryInterval*1000);

			if (st != SYS::SingleObject::TIMEDOUT)
			{
				// get notification to stop the thread, exit directly
				// nothing else need to be rleased addtionally
#ifdef ZQ_OS_MSWIN
				g_server.stopService();
#else
				g_server.unInit();
#endif
				return 0;
			}
		}
	}

	if(!_threadRunnng)
		return 0;

	// start the Dak, which has a thread pool to deliver the received data from the connection
	uint32 threadPoolSize = _lamClusters.size() * 6;		// max 6 nodes per cluster

	// validate the data path
	size_t pos = _gCMECfg._datPath.find_last_of(FNSEPS);
	if(std::string::npos == pos)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(CMEService, "Invalid data file path %s, please check the configuration"),
			_gCMECfg._datPath.c_str());

#ifdef ZQ_OS_MSWIN
		g_server.stopService();
#else
		g_server.unInit();
#endif
		return 1;
	}

	if(_gCMECfg._datPath.size()-1 != pos)
	{
		_gCMECfg._datPath += FNSEPS;
	}

	std::string tmpDir, logDir, cacheFile;
#ifdef ZQ_OS_MSWIN
	tmpDir = std::string(g_server.m_wsLogFolder);
#else
	tmpDir = g_server._logDir;
#endif

	if(tmpDir[tmpDir.size()-1] != FNSEPC)
	{
		logDir = tmpDir + std::string(FNSEPS);
	}
	else
	{
		logDir = tmpDir;
	}

	if(_gCMECfg._proactiveEnabled && _gCMECfg._proactivePath.size() > 0)
	{
		if(_gCMECfg._proactivePath[_gCMECfg._proactivePath.size()-1] != FNSEPC)
		{
			_gCMECfg._proactivePath = _gCMECfg._proactivePath + std::string(FNSEPS);
		}
	}
	else
	{
		_gCMECfg._proactiveEnabled = false;
	}


	cacheFile = logDir + std::string("CacheEvent.log");

	_cacheFileLog.open(cacheFile.c_str(), _gCMECfg._cacheLogLevel, _gCMECfg._cacheLogCount, _gCMECfg._cacheLogSize, _gCMECfg._cacheLogBuff);
	_cacheFileLog(ZQ::common::Log::L_INFO, "===================== CMEV2Svc CMECacheAlgorithm start ======================");

	//
	// 3. create the CacheStorage & IO
	_cmeAlgorithm = new CMECacheAlgorithm(_cacheFileLog);
	_cmeAlgorithm->setContentAgingParameters(_gCMECfg._agePeriod, _gCMECfg._ageDenominator);
	_cmeAlgorithm->setStorageParameters(_gCMECfg._storageThreshold, _gCMECfg._storageCushion, _gCMECfg._playTrigger, _gCMECfg._playTrigger2);
	_cmeAlgorithm->setCacheBWParameters(_gCMECfg._bwThreshold, _gCMECfg._bwReserved);

	for(i=0; i<_lamClusters.size(); i++)
	{
		Cluster* pCluster = _lamClusters[i];

		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "Create VSISCacheIO instance for cluster %s ..."), 
			pCluster->clusterID.c_str() );

		// create VSISCacheIO
		VSISCacheIO* vsisIO = new VSISCacheIO(pCluster->clusterID, *_cmeAlgorithm);
		_cacheIOs.insert(CACHE_VSISIOS::value_type(pCluster->clusterID, vsisIO));

		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "Create CacheStorage instance for cluster %s ..."), 
			pCluster->clusterID.c_str() );

		// create CacheStorage
		CacheStorage* cachestore = new CacheStorage(pCluster->clusterID, *vsisIO, *_cmeAlgorithm);
		_cacheStorages.insert(CACHE_STORAGES::value_type(pCluster->clusterID, cachestore));

		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "VSISCacheIO instance for cluster %s initialize ..."), 
			pCluster->clusterID.c_str() );

		// initialize 
		vsisIO->setLocalIP(_gCMECfg._cmeBindIP);
		vsisIO->setConnParameters(_gCMECfg._vsisConnTimeout, _gCMECfg._vsisSendTimeout, _gCMECfg._vsisRecBuffSize);
		vsisIO->setConnScanInterval(_gCMECfg._vsisScanInterval);
		vsisIO->setContentParameter(_gCMECfg._lengthOfPAID);
		vsisIO->setReservedBW(_gCMECfg._bwReserved);
		vsisIO->setMaxRptInterval(_gCMECfg._vsisReportInterval);

		vsisIO->initialize(pCluster->nodeIPs);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "CacheStorage instance for cluster %s initialize ..."), 
			pCluster->clusterID.c_str() );

		// set cache stoage paramters and initialize
		cachestore->setSubFileCount(_gCMECfg._subFileCount);
		cachestore->setDefaultVisa(_gCMECfg._visaSeconds);
		cachestore->setTrimdays(_gCMECfg._trimDays);
		cachestore->setUsage(pCluster->freeSize, pCluster->totalSize);
		cachestore->setDataFile(_gCMECfg._datType, _gCMECfg._datPath, _gCMECfg._datFlushInterval);
		cachestore->setCushion(_gCMECfg._storageCushion);
		cachestore->setStatParams(_gCMECfg._statEnabled, logDir, _gCMECfg._statWindow, _gCMECfg._statInterval, _gCMECfg._statDelay, _gCMECfg._statCntExtraSizePer, _gCMECfg._statPrintDetails > 0);
		cachestore->setCacheRetryInterval(_gCMECfg._proactiveRetryInterval);

		cachestore->initialize(_gCMECfg._syncIOAtStart);
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "CMESOAPServer start ..."));
	//
	// start the CME SOAP Service thread to accept soap client connection
	_pCMESOAPServer = new CMESOAPServiceThread(_gCMECfg._cmeBindIP, _gCMECfg._cmeBindPort);	

	ret = _pCMESOAPServer->start();
	if(!ret)
	{
		glog(Log::L_ERROR, CLOGFMT(CMEService, "CMEService initialize failed, stopping the service and please check configuration"));

		// stop the service since it should be configuration problem
#ifdef ZQ_OS_MSWIN
		g_server.stopService();
#else
		g_server.unInit();
#endif
		return 1;
	}

	// load proactive records file
	if(_gCMECfg._proactiveEnabled)
	{
		_proactiveRecordFile = _gCMECfg._datPath + "ProactiveImports.dat";
		loadProactiveRecords(_proactiveRecordFile);
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "CME is ready to go ..."));

	uint64 timenow;
	uint64 lastTimeoutExecuted = ZQ::common::now();
	//
	// keep the thread running in case we want to do something here. 
	//
	bool runOnce = true;
	timeout_t interval = 30*1000;		// 30 seconds
	while(_threadRunnng)
	{
		SYS::SingleObject::STATE st = _waitEvent.wait(interval);  // infinite now
		switch(st)
		{
		case SYS::SingleObject::SIGNALED:
			break;

		case SYS::SingleObject::TIMEDOUT:
		default:
			if(runOnce)
			{
				// parseProactiveXml(std::string("E:\\TestCode\\CMEV2Test\\etc\\ProactiveCache.xml"));
				// proactiveImport(std::string("88538"), std::string("so.comcast.com"), std::string("AAAI2194057012805180"), 0, 600); // test
				runOnce = false;
			}
			//
			timenow = ZQ::common::now();

			if( (timenow - lastTimeoutExecuted)/1000 >= 300 )
			{
				// print hitrate
				printHitrate();
				// print C2 bandwidth
				printImportBW();
				// check sessions
				checkSessions();

				// reset the time
				lastTimeoutExecuted = timenow;
			}

			// check if there is new xml file
			// check if proative import needs to be submit
			if(_gCMECfg._proactiveEnabled)
			{
				// check the path to see if xml need to parse
				checkNewXmlFile(_gCMECfg._proactivePath);

				// check if there is proacitve records need to submit to CacheStorage
				submitProactiveImports();
			}
			break;
		}
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "exit run()"));

	return 0;
}

CacheStorage* CMEService::findCacheStorage(std::string& name)
{
	CACHE_STORAGES::iterator itcs = _cacheStorages.find(name);
	if(itcs != _cacheStorages.end())
		return (CacheStorage*) itcs->second;

	return NULL;
}

void CMEService::printHitrate()
{
	CACHE_STORAGES::iterator it;
	
	uint64 local=0;
	uint64 total=0;
	uint64 localPer, totalPer;
	CacheStorage* cstorage;
	for(it=_cacheStorages.begin(); it!=_cacheStorages.end(); it++)
	{
		cstorage = (CacheStorage*)it->second;

		cstorage->getHits(localPer, totalPer);
		local += localPer;
		total += totalPer;

		_cmeAlgorithm->printHitrate(*cstorage);
	}
	// print system level hitrate
	_cmeAlgorithm->printHitrate(local, total);
}

void CMEService::printImportBW()
{
	CACHE_VSISIOS::iterator itvsis;
	for(itvsis=_cacheIOs.begin(); itvsis!=_cacheIOs.end(); itvsis++)
	{
		VSISCacheIO* vsisCacheIO = (VSISCacheIO*)itvsis->second;

		_cmeAlgorithm->printImportBW(*vsisCacheIO);
	}
}

void CMEService::checkSessions()
{
	glog(Log::L_DEBUG, CLOGFMT(CMEService, "enter checkSessions() total %d sessions in the list"), 
		_sessions.size() );

	std::vector<Session*> toDeleteSess;
	uint64 curtime = ZQ::common::now();
	uint64 diffSeconds = 0;
	SESSIONS::iterator itsess;
	for(itsess=_sessions.begin();itsess!=_sessions.end();itsess++)
	{
		Session* session = itsess->second;

		diffSeconds = curtime > session->_startTime ?  (curtime - session->_startTime)/1000 : 0;
		if(diffSeconds > MAX_SESS_PLAY_TIME)
		{
			toDeleteSess.push_back(session);
		}
	}

	int i = toDeleteSess.size()-1;
	for(; i>=0; i--)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMEService, "checkSession() session ClusterID=%s pid=%s paid=%s sid=%s timestamp=%s timeout after %d seconds"), 
			toDeleteSess[i]->_clusterID.c_str(), toDeleteSess[i]->_pid.c_str(), toDeleteSess[i]->_paid.c_str(), toDeleteSess[i]->_sid.c_str(), 
			time64ToUTCwZoneStr(toDeleteSess[i]->_startTime).c_str(), MAX_SESS_PLAY_TIME);

		_sessions.erase(toDeleteSess[i]->_sid);
		delete toDeleteSess[i];
	}
	toDeleteSess.clear();

	glog(Log::L_DEBUG, CLOGFMT(CMEService, "exit checkSessions() total %d sessions in the list"), 
		_sessions.size() );

}

bool CMEService::sessionArrive(std::string& cluid, int func, std::string& pid, std::string& paid, std::string& sid, std::string& timestamp)
{
	CacheStorage* cstorage = findCacheStorage(cluid);
	if(NULL == cstorage)
	{
		glog(Log::L_ERROR, CLOGFMT(CMEService, "Unknown ClusterID %s, ignore the session %s"), 
			cluid.c_str(), sid.c_str() );

		return false;
	}

	// add lock
	ZQ::common::MutexGuard sessGd(_sessLock);
	
	if(Session::SESSION_START == func)
	{	// check if there is dumplicated session if it is start

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMEService, "%s: received session START pid=%s paid=%s sid=%s timestamp=%s"), 
			cluid.c_str(), pid.c_str(), paid.c_str(), sid.c_str(), timestamp.c_str());

		if(_sessions.find(sid) != _sessions.end())
		{	// find duplicated session, discard the new one
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMEService, "%s: drop duplicated session %s"), 
				cluid.c_str(), sid.c_str());
			return true;
		}

		// get the session time 
		uint64 sessionTime = ZQ::common::TimeUtil::ISO8601ToTime(timestamp.c_str());
		if(0 == sessionTime)
		{
			// convert fail, use current time
			sessionTime = ZQ::common::now();   
		}

		//
		// create new session object and save it
		//
		Session* session = new Session();

		session->_clusterID	= cluid;
		session->_pid		= pid;
		session->_paid		= paid;
		session->_sid		= sid;
		session->_startTime  = sessionTime;		
		
		// save this session
		_sessions.insert(SESSIONS::value_type(sid, session));

		// notify the cluster to handle the play start event
		cstorage->playStart(pid, paid, sessionTime);

	}	// end of if(Session::SESSION_START == func)

	else if(Session::SESSION_STOP == func)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMEService, "%s: received session STOP pid=%s paid=%s sid=%s timestamp=%s"), 
			cluid.c_str(), pid.c_str(), paid.c_str(), sid.c_str(), timestamp.c_str());

		// find the session from session list
		SESSIONS::iterator itsess = _sessions.find(sid);
		if(itsess == _sessions.end())
		{
			// can not find the "start" session, ignore the "stop" session
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMEService, "%s: can not find session %s in the session list, ignore this session STOP notification"), 
				cluid.c_str(), sid.c_str() );
			return true;
		}

		Session* session = (Session*) itsess->second;
						
		// get the session time 
		uint64 sessionTime = ZQ::common::TimeUtil::ISO8601ToTime(timestamp.c_str());
		if(0 == sessionTime)
		{
			// convert fail, use current time
			sessionTime = ZQ::common::now();
		}

		cstorage->playEnd(pid, paid, (sessionTime > session->_startTime) ? (sessionTime - session->_startTime)/1000 : 1);

		// delete this ession 
		_sessions.erase(itsess);
		delete session;
	}
	return true;
}

// this call is for SOAP 
bool CMEService::proactiveImport(std::string& cluid, std::string& pid, std::string& paid, int increment, int lifetime, uint64 startTime)
{
	CacheStorage* cstorage = NULL;
	if(cluid == "")
	{
		cstorage = _cmeAlgorithm->candidateCacheStorage(_cacheStorages);
		if(NULL == cstorage)
		{
			// it should not happen based on candidateCacheStorage() implementation, but keep the code here
			glog(Log::L_ERROR, CLOGFMT(CMEService, "No clutser can be candidated to do proactive import for pid=%s paid=%s"), 
				pid.c_str(), paid.c_str() );

			return false;
		}
		glog(Log::L_INFO, CLOGFMT(CMEService, "Cluster %s is candidated to do proactive import for pid=%s paid=%s"), 
			cstorage->getName().c_str(), pid.c_str(), paid.c_str() );
	}
	else
	{
		cstorage = findCacheStorage(cluid);
		if(NULL == cstorage)
		{
			glog(Log::L_ERROR, CLOGFMT(CMEService, "Unknown ClusterID %s, ignore the proactive cache for pid=%s paid=%s"), 
				cluid.c_str(), pid.c_str(), paid.c_str() );

			return false;
		}
	}

	if(0 == startTime)
	{
		startTime = ZQ::common::now();
	}
	uint64 endTime = startTime + (uint64)(lifetime*1000);

	cstorage->proactiveImport(pid, paid, startTime, endTime);

	return true;
}

bool CMEService::parseProactiveXml(std::string& xmlfile)
{
#define XML_PROV_ROOT_NAME		"Proactive"
#define XML_PROV_CONTENT        "Content"
#define XML_PROV_PID            "pid"
#define XML_PROV_PAID           "paid"
#define XML_PROV_CLUSTER        "cluster"
#define XML_PROV_STARTTIME      "startTime"
#define XML_PROV_LIFETIME       "lifeTimeInHours"

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	
	try 
	{
		if( !xmlDoc.open(xmlfile.c_str()) )
		{
			glog(Log::L_ERROR, CLOGFMT(CMEService, "Failed to open proactive xml file: %s"), 
				xmlfile.c_str());
			return false;
		}
	}
	catch(ZQ::common::XMLException e)
	{
		glog(Log::L_ERROR, CLOGFMT(CMEService, "Failed to open proactive xml file: %s because: %s"), 
			xmlfile.c_str(), e.getString());
		return false;
	}

	glog(Log::L_INFO, CLOGFMT(CMEService, "Succeeded to open proactive xml file: %s"), 
		xmlfile.c_str());
	
	ZQ::common::XMLPreferenceEx* rootNode = xmlDoc.getRootPreference();

	char tempBuff[100]={0};
	rootNode->name(tempBuff, 100);
	if(0 != stricmp(tempBuff, XML_PROV_ROOT_NAME))
	{
		glog(Log::L_ERROR, CLOGFMT(CMEService, "invalid xml root name %s, it should be %s"), 
			tempBuff, XML_PROV_ROOT_NAME);
		
		rootNode->free();
		xmlDoc.clear();
		return false;
	}
	
	ZQ::common::XMLPreferenceEx* contentNode = rootNode->firstChild(XML_PROV_CONTENT);
	if(NULL == contentNode)
	{
		glog(Log::L_ERROR, CLOGFMT(CMEService, "There is no xml node <%s/> under the xml root <%s/>"), 
			XML_PROV_CONTENT, XML_PROV_ROOT_NAME);

		rootNode->free();
		xmlDoc.clear();
		return false;
	}

	char utcstr[100];
	std::string pid, paid, cluster;
	uint64 startTime, endTime;
	uint32 lifeTimeHours;
	bool bExist;
	bool bCorrectSection = true;

	do
	{
		bCorrectSection = true;
		// get pid
		memset(tempBuff, 0x0, 100);
		bExist = contentNode->getAttributeValue(XML_PROV_PID, tempBuff);
		if( !bExist || (0==strcmp(tempBuff, "")) )
		{
			glog(Log::L_ERROR, CLOGFMT(CMEService, "Attribute %s does not exist or empty value"), 
				XML_PROV_PID);

			bCorrectSection = false;
		}
		else
		{
			pid = tempBuff;
		}

		// get paid
		memset(tempBuff, 0x0, 100);
		bExist = contentNode->getAttributeValue(XML_PROV_PAID, tempBuff);
		if( !bExist || (0==strcmp(tempBuff, "")) )
		{
			glog(Log::L_ERROR, CLOGFMT(CMEService, "Attribute %s does not exist or empty value"), 
				XML_PROV_PAID);

			bCorrectSection = false;
		}
		else
		{
			paid = tempBuff;
		}

		// get cluster
		memset(tempBuff, 0x0, 100);
		bExist = contentNode->getAttributeValue(XML_PROV_CLUSTER, tempBuff);
		if(!bExist)
		{
			glog(Log::L_ERROR, CLOGFMT(CMEService, "Attribute %s does not exist or empty value"), 
				XML_PROV_CLUSTER);

			bCorrectSection = false;
		}
		else
		{
			cluster = tempBuff;
		}

		// get lifeTimeInHours
		memset(tempBuff, 0x0, 100);
		bExist = contentNode->getAttributeValue(XML_PROV_LIFETIME, tempBuff);
		if( !bExist || (0==strcmp(tempBuff, "")) )
		{
			glog(Log::L_ERROR, CLOGFMT(CMEService, "Attribute %s does not exist or empty value"), 
				XML_PROV_LIFETIME);

			bCorrectSection = false;
		}
		else
		{
			lifeTimeHours = atoi(tempBuff);
		}
		
		// get startTime
		memset(tempBuff, 0x0, 100);
		bExist = contentNode->getAttributeValue(XML_PROV_STARTTIME, tempBuff);
		if(!bExist || ( 0!=strcmp(tempBuff, "") && !isValidTimeStr(&tempBuff[0]) ) )
		{
			glog(Log::L_ERROR, CLOGFMT(CMEService, "Attribute %s does not exist or it is invalid time format"), 
				XML_PROV_STARTTIME);

			bCorrectSection = false;
		}
		else
		{
			if( 0==strcmp(tempBuff, "") )
			{
				startTime = ZQ::common::now();
			}
			else
			{
				memset(utcstr, 0x0, 100);
				ZQ::common::TimeUtil::Local2Iso(tempBuff, utcstr, 100);
				startTime = ZQ::common::TimeUtil::ISO8601ToTime(utcstr);
			}
			
			endTime =  startTime + (uint64)(lifeTimeHours*3600*1000);

			if(endTime < ZQ::common::now())
			{
				glog(Log::L_ERROR, CLOGFMT(CMEService, "startTime + lifeTimeHours is %s, less than now"), 
					time64ToUTCwZoneStr(endTime).c_str() );

				bCorrectSection = false;
			}
		}

		if(bCorrectSection)
		{
			PROACTIVE_RECORDS::iterator itrec = _proactiveRecords.find(PAID_PID(paid,pid));
			if(_proactiveRecords.end() == itrec)
			{
				ProactiveRecord* provRecord = new ProactiveRecord();

				provRecord->_pid = pid;
				provRecord->_paid = paid;
				provRecord->_clusterID = cluster;
				provRecord->_startTime = startTime;
				provRecord->_liftime = (uint64)(lifeTimeHours);
		
				_proactiveRecords.insert(PROACTIVE_RECORDS::value_type(PAID_PID(paid,pid), provRecord));
			}
			else
			{
				ProactiveRecord* provRecord = (ProactiveRecord*)itrec->second;

				provRecord->_clusterID = cluster;
				provRecord->_startTime = startTime;
				provRecord->_liftime = (uint64)(lifeTimeHours);
			}

			glog(Log::L_DEBUG, CLOGFMT(CMEService, "Proactive request: pid=%s paid=%s cluster=%s startTime=%s lifeTimeInHours=%d"), 
				pid.c_str(), paid.c_str(), cluster.c_str(), 
				time64ToUTCwZoneStr(startTime).c_str(), lifeTimeHours);
		}

		// free the content node
		contentNode->free();

		// check if there is other xml node
		if( rootNode->hasNextChild() )
			contentNode = rootNode->nextChild();
		else
			break;

	} while(true);

	rootNode->free();
	xmlDoc.clear();
	
	if(bCorrectSection)
	{
		std::string newName = xmlfile + ".parsed.txt";

		if (0 == ::access(xmlfile.c_str(), 0) && 0 !=::rename(xmlfile.c_str(), newName.c_str()))
		{
			glog(Log::L_WARNING, CLOGFMT(CMEService, "Failed to rename %s to %s"), 
				xmlfile.c_str(), newName.c_str());
		}
	}
	
	return bCorrectSection;
}

bool CMEService::checkNewXmlFile(std::string& path)
{
	std::string findname = path + std::string("*.xml");
	std::vector<std::string> xmlfiles;

#ifdef ZQ_OS_MSWIN

	WIN32_FIND_DATA data;
	HANDLE findHandle = FindFirstFile(findname.c_str(), &data);

	if(findHandle == INVALID_HANDLE_VALUE) 
	{
		return false;
	}

	do{
		if( (data.cFileName[0] == '.') || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
		{
			continue;
		}

		xmlfiles.push_back(std::string(data.cFileName));

	} while(FindNextFile(findHandle, &data));

	FindClose(findHandle);

#else
    struct dirent **namelist;
    int n, len;

    n = scandir(path.c_str(), &namelist, 0, alphasort);
    if (n < 0)
	{        
		glog(Log::L_ERROR, CLOGFMT(CMEService, "Failed to scan path %s"), 
			path.c_str());
	}
    else 
	{
        while(n--) 
		{
            len = strlen(namelist[n]->d_name);
            if(len>4
               && namelist[n]->d_name[len-4]=='.'
               && tolower(namelist[n]->d_name[len-3])=='x'
               && tolower(namelist[n]->d_name[len-2])=='m'
               && tolower(namelist[n]->d_name[len-1])=='l' )
			{
				xmlfiles.push_back(std::string(namelist[n]->d_name));
			}
            free(namelist[n]);
        }
        free(namelist);
    }

#endif

	bool bFlush = false;
	size_t i=0;
	std::string xmlfullname;
	for(; i<xmlfiles.size(); i++)
	{
		xmlfullname = path+xmlfiles[i];
		bFlush = bFlush | parseProactiveXml(xmlfullname);
	}

	// flush the data to DB
	if(xmlfiles.size() > 0 && bFlush)
	{
		flushProactiveRecords(_proactiveRecordFile);
	}
	return true;
}

void CMEService::submitProactiveImports()
{
	if(_proactiveRecords.size() == 0)
		return ;

	bool bChanged = false;
	uint64 tnow;
	ProactiveRecord* proRecord = NULL;
	PROACTIVE_RECORDS::iterator it;
	PROACTIVE_ARRAY tobeRemoved;

	for(it=_proactiveRecords.begin(); it!=_proactiveRecords.end(); it++)
	{
		proRecord = (ProactiveRecord*)it->second;
	
		tnow = ZQ::common::now();
		if( (proRecord->_startTime + 60000) < tnow)	// submit 1 minute ahead
		{
			bChanged = true;

			proactiveImport(proRecord->_clusterID, proRecord->_pid, proRecord->_paid, 0, (proRecord->_liftime*3600), proRecord->_startTime);

			tobeRemoved.push_back(proRecord);
		}
	}

	// flush the data to DB
	if(bChanged)
	{
		for(size_t i=0; i<tobeRemoved.size(); i++)
		{
			proRecord = tobeRemoved[i];
			
			_proactiveRecords.erase(PAID_PID(proRecord->_paid,proRecord->_pid));
			delete proRecord;
		}
		tobeRemoved.clear();

		// flush data since data changed
		flushProactiveRecords(_proactiveRecordFile);
	}
}

bool CMEService::loadProactiveRecords(std::string& filename)
{
	std::ifstream datRd(filename.c_str());
	 
	if(!datRd.is_open())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "Failed to open proactive record file %s, no data will be read"),
			filename.c_str());
		return false;
	}
	
	int narg = 0;
	int len = 0;
	uint32 recordsCount = 0;
	char lineBuff[MAX_PROACITVE_LINE];

	char szPid[100], szPaid[100], szCluster[100];
	char szStartTime[100];
	int32 lifeTimeHours;
	
	datRd.getline(lineBuff, MAX_PROACITVE_LINE); // eofbit
	while( !datRd.eof() && datRd.good() )
	{
		memset(szPid,0x00, 100); memset(szPaid,0x00, 100); memset(szCluster,0x00, 100); memset(szStartTime, 0x00, 100);
		lifeTimeHours=0;

		narg = sscanf(lineBuff, "%20s %36s %10s %36s %4d", 
								&szPid[0], &szPaid[0], &szCluster[0], &szStartTime[0], &lifeTimeHours);

		if(narg < 5)
		{
			len = strlen(lineBuff);
			if(len>3)
			{
#ifdef ZQ_OS_MSWIN
				lineBuff[len-3] = '\0';
#else 
				lineBuff[len-2] = '\0';	
#endif
			}
			glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "Failed to parse from data line: %s"),
				lineBuff);

			continue;
		}

		ProactiveRecord* provRecord = new ProactiveRecord();
		provRecord->_pid = szPid;
		provRecord->_paid = szPaid;
		provRecord->_clusterID = strcmp(szCluster, NA) == 0 ? "" : szCluster;
		provRecord->_startTime = strcmp(szStartTime, DATETIME_NA) == 0 ? ZQ::common::now() : ZQ::common::TimeUtil::ISO8601ToTime(szStartTime);;
		provRecord->_liftime = (uint32)lifeTimeHours;

		_proactiveRecords.insert(PROACTIVE_RECORDS::value_type(PAID_PID(provRecord->_paid,provRecord->_pid), provRecord));
	
		// print log of this population	
		recordsCount++;
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMEService, "No.%04d proactive import content pid=%s paid=%s cluster=%s startTime=%s lifeTime=%d hours is populated from data file"),
			recordsCount, szPid, szPaid, szCluster, szStartTime, lifeTimeHours);

		// get a new line
		datRd.getline(lineBuff, MAX_PROACITVE_LINE);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "Total %d valid content records were populated from data file %s"),
		recordsCount, filename.c_str());

	// close file
	datRd.close();

	return true;
}

bool CMEService::flushProactiveRecords(std::string& filename)
{
	std::ofstream datWt(filename.c_str(), std::ofstream::out|std::ofstream::trunc);
	
	if(!datWt.is_open())
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMEService, "Failed to open proactive record file %s, no data will be writen"),
			filename.c_str());
		return false;
	}
	
	int len;
	char lineBuff[MAX_PROACITVE_LINE];

	// write the version informaiton first
	//
	// don't add \r\n etc at the end of the line here
	int recordsCount=0;
	ProactiveRecord* proRecord = NULL;
	PROACTIVE_RECORDS::iterator itrec;
	for(itrec=_proactiveRecords.begin(); itrec!=_proactiveRecords.end(); itrec++)
	{
		recordsCount++;

		proRecord = (ProactiveRecord*)itrec->second;

		len = snprintf(lineBuff, MAX_PROACITVE_LINE-3, "%s\t%s\t\t%s\t%s\t%d",
														 proRecord->_pid.c_str(), proRecord->_paid.c_str(), 
														 "" == proRecord->_clusterID ? NA : proRecord->_clusterID.c_str(),
														 time64ToUTCwZoneStr(proRecord->_startTime).c_str(), 
														 proRecord->_liftime);

#ifdef ZQ_OS_MSWIN
			lineBuff[len] = '\r';
			lineBuff[len+1] = '\n';
			lineBuff[len+2] = '\0';

			len += 2;
#else 
			lineBuff[len] = '\n';
			lineBuff[len+1] = '\0';

			len += 1;
#endif
		datWt.write(lineBuff, len);
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheStorage, "Total %d proactive records were write into data file %s"),
		recordsCount, filename.c_str());


	// close file
	datWt.close();

	return true;
}

bool CMEService::proactiveDelete(std::string& cluid, std::string& pid, std::string& paid)
{
	//readdir(
	return true;
}

void CMEService::final(void)
{
	glog(Log::L_INFO, CLOGFMT(CMEService, "enter final()"));

	glog(Log::L_INFO, CLOGFMT(CMEService, "Stopping CMESOAPSever thread"));
	_pCMESOAPServer->stop();

	CACHE_VSISIOS::iterator itio;
	for(itio=_cacheIOs.begin(); itio!=_cacheIOs.end(); itio++)
	{
		VSISCacheIO* vsisIO = (VSISCacheIO*) itio->second;
		
		glog(Log::L_INFO, CLOGFMT(CMEService, "Stopping VSIS IO %s thread thread"), 
			vsisIO->getClusterID().c_str());

		vsisIO->unInitialize();
	}

	CACHE_STORAGES::iterator itcs;
	for(itcs=_cacheStorages.begin(); itcs!=_cacheStorages.end(); itcs++)
	{
		CacheStorage* cstorage = (CacheStorage*) itcs->second;

		glog(Log::L_INFO, CLOGFMT(CMEService, "Stopping CacheStorage %s thread thread"), 
			cstorage->getName().c_str() );

		cstorage->uninitialize();
	}

	glog(Log::L_INFO, CLOGFMT(CMEService, "Release LAM Cluster objects"));
	for(int i=0; i<_lamClusters.size(); i++)
	{
		Cluster* pCluster = _lamClusters[i];
		delete pCluster;
	}
	_lamClusters.clear();

	glog(Log::L_INFO, CLOGFMT(CMEService, "Release VSISCacheIO objects"));
//	CACHE_VSISIOS::iterator itio;
	while(!_cacheIOs.empty())
	{
		itio = _cacheIOs.begin();

		VSISCacheIO* csIO = (VSISCacheIO*) itio->second;

		_cacheIOs.erase(itio);
		delete csIO;
	}

	glog(Log::L_INFO, CLOGFMT(CMEService, "Release CacheStorage objects"));
//	CACHE_STORAGES::iterator itcs;
	while(!_cacheStorages.empty())
	{
		itcs = _cacheStorages.begin();

		CacheStorage* cstorage = (CacheStorage*) itcs->second;

		_cacheStorages.erase(itcs);
		delete cstorage;
	}

	if(NULL != _cmeAlgorithm)
		delete _cmeAlgorithm;
	_cmeAlgorithm = NULL;

	glog(Log::L_INFO, CLOGFMT(CMEService, "Release SOAP objects"));

	if(NULL != _pLamSOAPClient)
		delete _pLamSOAPClient;
	_pLamSOAPClient = NULL;

	if(NULL != _pCMESOAPServer)
		delete _pCMESOAPServer;
	_pCMESOAPServer = NULL;

	glog(Log::L_INFO, CLOGFMT(CMEService, "Release Session objects"));

	ZQ::common::MutexGuard sessGd(_sessLock);
	SESSIONS::iterator itsess;
	while(!_sessions.empty())
	{
		itsess = _sessions.begin();

		Session* session = (Session*) itsess->second;

		_sessions.erase(itsess);
		delete session;
	}
	glog(Log::L_INFO, CLOGFMT(CMEService, "exit final()"));
}


}  // namespace CacheManagement