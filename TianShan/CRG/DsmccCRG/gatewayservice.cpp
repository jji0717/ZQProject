#include <ZQ_common_conf.h>
#ifdef ZQ_OS_MSWIN
#include "MiniDump.h"
#else // linux
#include <sys/resource.h> // for the fnlimit
#endif

#include <TianShanIceHelper.h>
#include <IceLog.h>
#include "environment.h"
#include "gatewayservice.h"
#include "gatewayconfig.h"
#include "DsmccDefine.h"
using namespace ZQ::SNMP;

ZQ::CLIENTREQUEST::GatewayService g_GatewayService;
ZQ::common::BaseZQServiceApplication  *Application = &g_GatewayService;

namespace ZQ { namespace CLIENTREQUEST {
ZQ::common::Config::Loader<ZQ::CLIENTREQUEST::Config::Gateway> gwConfig("DsmccCRG.xml");
}}

ZQ::common::Config::ILoader *configLoader = &ZQ::CLIENTREQUEST::gwConfig;

extern const char* DUMP_PATH;

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
#endif

namespace ZQ{ namespace CLIENTREQUEST {

GatewayService::GatewayService()
:mGatewayEnv(0),
mGwCenter(0)
{
    _lastRefreshTime = ZQ::common::TimeUtil::now();
}

GatewayService::~GatewayService()
{
}

bool GatewayService::initIceRuntime()
{
	Ice::InitializationData		iceInitData;
	int i = 0;
	iceInitData.properties =Ice::createProperties( i , NULL );

	//set ice properties
	
	std :: map<std::string, std::string>::const_iterator it = gwConfig.iceProps.begin();
	for( ; it != gwConfig.iceProps.end() ; it ++ )
	{		
		iceInitData.properties->setProperty( it->first , it->second );
		glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"Set ice property [%60s] = [%s]"),
			it->first.c_str() , it->second.c_str() );
	}


	if( gwConfig.icetraceenabled >= 1 )
	{
		std::string logFolder;
#ifdef ZQ_OS_MSWIN
		logFolder = m_wsLogFolder;
#else
        logFolder = _logDir;
#endif 
		std::string path = ZQTianShan::Util::fsConcatPath(logFolder,"DsmccCRG.IceTrace.log");
		try
		{			
			mIceTraceLogger.open( path.c_str() ,gwConfig.icetracelevel,ZQLOG_DEFAULT_FILENUM,gwConfig.icetracelogsize ,	10240,	2);
		}
		catch( const ZQ::common::FileLogException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"failed to open ice trace log file[%s] because [%s]"),
				path.c_str(),ex.what() );
			return false;
		}
		iceInitData.logger = new TianShanIce::common::IceLogI( &mIceTraceLogger );
		assert( iceInitData.logger );

	}

	mIc	=	Ice::initialize( i , NULL , iceInitData );

	try
	{
		mAdapter = ZQADAPTER_CREATE(mIc,"Gateway",gwConfig.binding.c_str(),glog);
	}
	catch( const Ice::Exception &ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"initIceRuntime() caught [%s] while setup adapter with binding[%s]"),
			ex.ice_name().c_str(), gwConfig.binding.c_str() );
		return false;
	}

	return ( mAdapter != 0 );
}

HRESULT GatewayService::OnInit(void)
{
#ifdef ZQ_OS_MSWIN
	// step 2: crash dump
	if(gwConfig.crashdumpenabled)
	{
		if(!validatePath(gwConfig.crashdumppath.c_str()))
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(GatewayService, "OnInit() bad dump path [%s]"), gwConfig.crashdumppath.c_str());
			return S_FALSE;
		}
		// enable crash dump

		_crashDump.setDumpPath((char*)gwConfig.crashdumppath.c_str());
		_crashDump.enableFullMemoryDump(true);
		_crashDump.setExceptionCB(CrashExceptionCallBack);
	}
#else
	DUMP_PATH = gwConfig.crashdumppath.c_str();

    // set ulimit
    int maxSession = (gwConfig.sockserver.maxConnection*1.5 >= 10000) ? gwConfig.sockserver.maxConnection*1.5 : 10000;
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = maxSession;
    int rc = setrlimit(RLIMIT_NOFILE, &rt);
#endif

	if( !initIceRuntime() )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"OnInit() failed to initialize ice runtime environment"));
		return S_FALSE;
	}

	mGatewayEnv = new Environment(*m_pReporter,mAdapter);
	mGwCenter	= new GatewayCenter(*mGatewayEnv);
	//if (NULL != mGatewayEnv)
	//    mGatewayEnv->registerSnmp(mGwCenter);

	glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"OnInit() environment[%s], center[%s]"), (NULL != mGatewayEnv ? "Succeed" : "NULL"), (NULL != mGwCenter ? "Succeed" : "NULL"));
	return BaseZQServiceApplication::OnInit();
}

HRESULT GatewayService::OnStart(void)
{
	if(!mGwCenter)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"OnStart() failed to initialize service, refuse to start"));
		return S_FALSE;
	}

	std::string logFolder;
#ifdef ZQ_OS_MSWIN
	logFolder = m_wsLogFolder;
#else
	logFolder = _logDir;
#endif 

	if(!mGwCenter->start(logFolder.c_str(),gwConfig.dbruntimepath.c_str() ))
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"OnStart() failed to start gateway service"));
		return S_FALSE;
	}
	mAdapter->activate();
	return BaseZQServiceApplication::OnStart();
}

HRESULT GatewayService::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"OnStop() gateway is stopping"));
	if( mAdapter)
	{
		mAdapter->deactivate();
	}
	if( mAdapter)
	{
		mAdapter = 0;
	}

	if(mGwCenter)	
		mGwCenter->stop();
	return BaseZQServiceApplication::OnStop();
}

HRESULT GatewayService::OnUnInit(void)
{	
	if( mGwCenter )
		delete mGwCenter;
	if( mGatewayEnv)
		delete mGatewayEnv;
	
	if( mIc )
	{
		try
		{
			mIc->destroy();
		}
		catch(...)
		{
		}
		mIc = NULL;		
	}
	glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"OnUnInit() gateway stopped"));
	return BaseZQServiceApplication::OnUnInit();
}

std::string fixString(const std::string& key, int num)
{
	char buf[32] = "";
	memset(buf, 0, sizeof(buf));
	itoa(num, buf, 10);
	return std::string(key + std::string(buf));
}

bool GatewayService::isHealth()
{
    int64 now = ZQ::common::TimeUtil::now();
    if ((now - _lastRefreshTime) > 30*1000)
    {
        refreshCrStatTable();
        _lastRefreshTime = ZQ::common::TimeUtil::now();
    }

    return true;
}

void GatewayService::refreshCrStatTable()
{
    static ZQ::SNMP::Oid subOidTbl;
    if (subOidTbl.isNil())
        _pServiceMib->reserveTable("dsmccCrStat", 15, subOidTbl);
    if (subOidTbl.isNil())
    {
        glog(ZQ::common::Log::L_WARNING, CLOGFMT(SentryService,"doEnumSnmpExports() failed to locate dsmccCrStat in MIB"));
        return;
    }

    // clean up the table content
    ZQ::SNMP::Oid tmpOid(subOidTbl);
    tmpOid.append(1);
    _pServiceMib->removeSubtree(tmpOid);

    const char* rowName[] = {
        "null"  ,
        "setup"	,
        "GetParameter"	,
        "play",
        "TearDown",
        "Colunm end"
    };

#define GET_VALUE_FROM_MAP_BY_INDEX(index)   (it = erroeToCountMapRef.find(index), (it != erroeToCountMapRef.end()) ? it->second : 0 )

    uint32 idxRow = 1;
    for(int idxRow = 1; idxRow <= STATISTICS_COUNT; idxRow++)
    {
        Oid subOid = subOidTbl; 
        subOid.append(1), subOid.append(idxRow);
        GatewayStatisics* pStats = &(mGatewayEnv->getCollector().getStatistics()[idxRow-1]);
        _pServiceMib->addObject(new SNMPObjectDupValue(fixString("rowName#", idxRow), rowName[idxRow]), subOid);

        ErrorToCountMap::iterator it;
        ErrorToCountMap& erroeToCountMapRef = pStats->errorToCountMap;
        int countOK = GET_VALUE_FROM_MAP_BY_INDEX(ZQ::DSMCC::RsnOK);
        int countNoSess = GET_VALUE_FROM_MAP_BY_INDEX(ZQ::DSMCC::RspNeNoSession);
        int countBadReqest = GET_VALUE_FROM_MAP_BY_INDEX(ZQ::DSMCC::lscErr_BadRequest);
        int countInvalidMethod = GET_VALUE_FROM_MAP_BY_INDEX(ZQ::DSMCC::lscErr_InvalidMethod);
        int countServerError = GET_VALUE_FROM_MAP_BY_INDEX(ZQ::DSMCC::lscErr_ServerErr);

        subOid = subOidTbl, subOid.append(2), subOid.append(idxRow);
        SvcMIB_ExportReadOnlyVar(fixString("countSubTotal#", idxRow), (uint32&)pStats->totalCount, subOid);

        subOid = subOidTbl, subOid.append(3), subOid.append(idxRow);
        SvcMIB_ExportReadOnlyVar(fixString("latencyMax#", idxRow), (uint32&)pStats->timeCostMax, subOid);

        subOid = subOidTbl, subOid.append(4), subOid.append(idxRow);
        SvcMIB_ExportReadOnlyVar(fixString("latencyAvg#", idxRow), (uint32&)pStats->timeCostAvg, subOid);

        subOid = subOidTbl, subOid.append(5), subOid.append(idxRow);
        _pServiceMib->addObject(new SNMPObjectDupValue(fixString("countOK#", idxRow), (int32)countOK), subOid);

        subOid = subOidTbl, subOid.append(6), subOid.append(idxRow);
        _pServiceMib->addObject(new SNMPObjectDupValue(fixString("countNoSess#", idxRow), (int32)countNoSess), subOid);

        subOid = subOidTbl, subOid.append(7), subOid.append(idxRow);
        _pServiceMib->addObject(new SNMPObjectDupValue(fixString("countBadRequest#", idxRow), (int32)countBadReqest), subOid);

        subOid = subOidTbl, subOid.append(8), subOid.append(idxRow);
        _pServiceMib->addObject(new SNMPObjectDupValue(fixString("countInvalidMethod#", idxRow), (int32)countInvalidMethod), subOid);

        subOid = subOidTbl, subOid.append(9), subOid.append(idxRow);
        _pServiceMib->addObject(new SNMPObjectDupValue(fixString("countServerError#", idxRow), (int32)countServerError), subOid);
    }
}

void GatewayService::doEnumSnmpExports()
{
	BaseZQServiceApplication::doEnumSnmpExports();

	_pServiceMib->addObject(new SNMPObjectByAPI<GatewayCenter, uint32>("dsmccStat-SessionCount", *mGwCenter, AsnType_Int32, &GatewayCenter::sessionCount));
	_pServiceMib->addObject(new SNMPObjectByAPI<GatewayStatisticsCollector, uint32>("dsmccStat-Measure-Reset", mGatewayEnv->getCollector(), AsnType_Int32, &GatewayStatisticsCollector::dummyGet, &GatewayStatisticsCollector::reset));
	_pServiceMib->addObject(new SNMPObjectByAPI<GatewayStatisticsCollector, uint64>("dsmccStat-Measured-Since", mGatewayEnv->getCollector(), AsnType_Int64, &GatewayStatisticsCollector::measuredSince));
	_pServiceMib->addObject(new SNMPObjectByAPI<GatewayCenter, uint32>("dsmccStat-PendingSize",  *mGwCenter, AsnType_Int32, &GatewayCenter::pendingRequestSize));
	_pServiceMib->addObject(new SNMPObjectByAPI<GatewayCenter, uint32>("dsmccStat-BusyThreads", *mGwCenter, AsnType_Int32,  &GatewayCenter::activeCount));
	_pServiceMib->addObject(new SNMPObjectByAPI<GatewayCenter, uint32>("dsmccStat-ThreadPoolSize", *mGwCenter, AsnType_Int32, &GatewayCenter::size));
	
    refreshCrStatTable();
}

/*
void GatewayService::snmp_refreshSessionStatus(const uint32& iDummy)
{
	const char* rowName[] = {
		"null"  ,
		"setup"	,
		"GetParameter"	,
		"play",
		"TearDown",
		"Colunm end"
	};

	Oid subOidTable;
	_pServiceMib->reserveTable("reqStatRow", 9, subOidTable);

	GatewayStatisics statisticsState[STATISTICS_COUNT];
	const int ROW_END = sizeof(statisticsState) / sizeof(GatewayStatisics) + 1;
	mGatewayEnv->getCollector().getStatistics(statisticsState);

	uint32 idxRow =1;
	for (idxRow = 1; idxRow < ROW_END; ++idxRow)
	{
		try
		{
			struct GatewayStatisics * rowState = (struct GatewayStatisics *)statisticsState + (idxRow - 1);
			ErrorToCountMap& erroeToCountMapRef = rowState->errorToCountMap;
			std::string rowInstance(rowName[idxRow]);
			int totalCount(rowState->totalCount) ;	 
			int timeCostMax(rowState->timeCostMax); 
			int timeCostMin(rowState->timeCostMin);
			int timeCostAvg(rowState->timeCostAvg);
			ErrorToCountMap::iterator it;

#define GET_VALUE_FROM_MAP_BY_INDEX(index)   (it = erroeToCountMapRef.find(index), (it != erroeToCountMapRef.end()) ? it->second : 0 )

			int countOK            = GET_VALUE_FROM_MAP_BY_INDEX( ZQ::DSMCC::RsnOK );
			int countNoSess        = GET_VALUE_FROM_MAP_BY_INDEX( ZQ::DSMCC::RspNeNoSession );
			int countBadReqest     = GET_VALUE_FROM_MAP_BY_INDEX( ZQ::DSMCC::lscErr_BadRequest );
			int countInvalidMethod = GET_VALUE_FROM_MAP_BY_INDEX( ZQ::DSMCC::lscErr_InvalidMethod );
			int countServerError   = GET_VALUE_FROM_MAP_BY_INDEX( ZQ::DSMCC::lscErr_ServerErr );

			_pServiceMib->addTableCell(subOidTable, 1, idxRow, new SNMPObjectDupValue("rowName", rowInstance));
			_pServiceMib->addTableCell(subOidTable, 2, idxRow, new SNMPObjectDupValue("countSubTotal",  (int32)totalCount));
			_pServiceMib->addTableCell(subOidTable, 3, idxRow, new SNMPObjectDupValue("latencyMax", (int32)timeCostMax));
			_pServiceMib->addTableCell(subOidTable, 4, idxRow, new SNMPObjectDupValue("latencyAvg", (int32)timeCostAvg));
			_pServiceMib->addTableCell(subOidTable, 5, idxRow, new SNMPObjectDupValue("countOK", (int32)countOK));
			_pServiceMib->addTableCell(subOidTable, 6, idxRow, new SNMPObjectDupValue("countNoSess", (int32)countNoSess));
			_pServiceMib->addTableCell(subOidTable, 7, idxRow, new SNMPObjectDupValue("countBadRequest", (int32)countBadReqest));
			_pServiceMib->addTableCell(subOidTable, 8, idxRow, new SNMPObjectDupValue("countInvalidMethod", (int32)countInvalidMethod));
			_pServiceMib->addTableCell(subOidTable, 9, idxRow, new SNMPObjectDupValue("countServerError", (int32)countServerError));
		}
		catch (...) 
		{
		}
	}
}
*/

}}
