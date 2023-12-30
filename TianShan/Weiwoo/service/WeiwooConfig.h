#ifndef _WEIWOO_SERVICE_CONFIG_H__
#define _WEIWOO_SERVICE_CONFIG_H__

#include <ConfigHelper.h>
#include <TianShanIceHelper.h>
#ifdef ZQ_OS_MSWIN
#include <tchar.h>
#include <io.h>
#endif

struct MonitoredLog
{
    std::string name;
    std::string syntax;
	std::string key;
	std::string type;
    static void structure(ZQ::common::Config::Holder<MonitoredLog> &holder)
    {
        holder.addDetail("", "path", &MonitoredLog::name);
        holder.addDetail("", "syntax", &MonitoredLog::syntax);
		holder.addDetail("", "key", &MonitoredLog::key,"");
		holder.addDetail("","type",&MonitoredLog::type,"");
    }
};
struct WeiwooCfg
{
	WeiwooCfg()
	{
		szIceStormEndPoint[0] = 0;
	}
    char szCrashDumpPath[512];
    int32 lCrashDumpEnable;

    char szIceStormEndPoint[512];
    int32 lIcetraceLogEnable;
    int32 lIceTraceLogLevel;
    int32 lIceTraceLogSize;

    char szIceDbFolder[512];
    char szIceDbRuntimeFolder[512];

    char szWeiwooEndpoint[512];
    char szWeiwooAdpaterThreadpool[512];
	char szWeiwooAdpaterThreadpoolMax[512];
    
    int32 lPathLogLevel;
    int32 lPathLogSize;
    int32 lPathLogBuffer;
    int32 lPathLogTimeOut;
    char szBusinessRouterEndpoint[512];

    char szPathManagerEndpoint[512];
    char szPathManagerAdapterThreadPoolSize[512];
	char szPathManagerAdapterThreadPoolSizeMax[512];
    int32 lServiceThreadPoolSize;
	int32 lMaxPendingRequestSize;
    
    int32 lEvictorWeiwooSessSize;

    int32 lEvictorPathTicketSize;
    int32 lEvictorStrmlinkSize;
    int32 lEvictorStorlinkSize;

	int32 lMaxSelectTickets;
	int32 lMaxPenaltyValue;
	int32 lStrmLinksByTicket;

	int32 lMixTeardownReasonAndTerminateReason;

	int32 lEnableReplicaSubscriber;
	int32 lReplicaUpdateInterval;

	char szWeiwooIceEnvCheckPointPeriod[512];
	char szWeiwooIceEnvDbRecoverFatal[512];

	char szPathIceEnvCheckPointPeriod[512];
	char szPathIceEnvDbRecoverFatal[512];

	char szFreezeSessionSavePeriod[512];
	char szFreezeSessionSaveSizeTrigger[512];
	
	char szFreezeStorageLinkSavePeriod[512];
	char szFreezeStorgaeLinkSaveSizeTrigger[512];
	char szFreezeStreamLinkSavePeriod[512];
	char szFreezeStreamLinkSaveSizeTrigger[512];
	char szFreezePathticketSavePeriod[512];
	char szFreezePathticketSaveSizeTrigger[512];

    std::vector<MonitoredLog> monitoredLogs;
    std::map<std::string, std::string> iceProperties;

    static void structure(ZQ::common::Config::Holder<WeiwooCfg> &holder)
    {
        using namespace ZQ::common::Config;
        
        /*	
        holder.addDetail("default/EventChannel",
							"endpoint", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szIceStormEndPoint, 
							sizeof(holder.szIceStormEndPoint), 
							NULL, 
							optReadOnly);
		*/

        holder.addDetail("default/CrashDump", 
							"path", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szCrashDumpPath, 
							sizeof(holder.szCrashDumpPath), 
							NULL, 
							optReadOnly);
        holder.addDetail("default/CrashDump",
							"enabled",	
							&WeiwooCfg::lCrashDumpEnable, 
							NULL, 
							optReadOnly);

        holder.addDetail("default/IceTrace", 
							"enabled", 
							&WeiwooCfg::lIcetraceLogEnable,	
							NULL, 
							optReadOnly);
        holder.addDetail("default/IceTrace", 
							"level", 
							&WeiwooCfg::lIceTraceLogLevel, 
							NULL, 
							optReadOnly);
        holder.addDetail("default/IceTrace", 
							"size", 
							&WeiwooCfg::lIceTraceLogSize, 
							NULL, 
							optReadOnly);

        holder.addDetail("default/Database", 
							"path", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szIceDbFolder, 
							sizeof(holder.szIceDbFolder), 
							NULL, 
							optReadOnly);
        holder.addDetail("default/Database", 
							"runtimePath", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szIceDbRuntimeFolder, 
							sizeof(holder.szIceDbRuntimeFolder), 
							"", 
							optReadOnly);

        holder.addDetail("default/PublishedLogs/Log", 
							&WeiwooCfg::readMonitoredLog, 
							&WeiwooCfg::registerNothing);
        holder.addDetail("default/IceProperties/prop", 
							&WeiwooCfg::readIceProperty, 
							&WeiwooCfg::registerNothing);

        holder.addDetail("Weiwoo/Bind", 
							"endpoint", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szWeiwooEndpoint, 
							sizeof(holder.szWeiwooEndpoint), 
							"15", 
							optReadOnly);
		holder.addDetail("Weiwoo/Bind", 
							"dispatchSize", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szWeiwooAdpaterThreadpool, 
							sizeof(holder.szWeiwooAdpaterThreadpool), 
							"15", 
							optReadOnly);
		holder.addDetail("Weiwoo/Bind",
							"dispatchMax",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szWeiwooAdpaterThreadpoolMax, 
							sizeof(holder.szWeiwooAdpaterThreadpoolMax), 
							NULL,
							optReadOnly);
        
        holder.addDetail("Weiwoo/ThreadPool", 
							"size", 
							&WeiwooCfg::lServiceThreadPoolSize, 
							"15", 
							optReadOnly);
		holder.addDetail("Weiwoo/ThreadPool", 
							"maxPendingSize", 
							&WeiwooCfg::lMaxPendingRequestSize, 
							"1024", 
							optReadOnly);

		holder.addDetail("Weiwoo/PathSelection",
							"maxTicketCount",
							&WeiwooCfg::lMaxSelectTickets,
							"5",
							optReadWrite);

		holder.addDetail("Weiwoo/PathSelection",
							"streamLinksByTicketCount",
							&WeiwooCfg::lStrmLinksByTicket,
							"4",
							optReadWrite);
		//lMaxPenaltyVaue
		holder.addDetail("Weiwoo/PathSelection",
							"maxPenalty",
							&WeiwooCfg::lMaxPenaltyValue,
							"10",
							optReadWrite);

		holder.addDetail("Weiwoo/PeformanceTune/IceFreezeEnviroment",
							"CheckpointPeriod",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szWeiwooIceEnvCheckPointPeriod, 
							sizeof(holder.szWeiwooIceEnvCheckPointPeriod),
							NULL,
							optReadOnly);
		holder.addDetail("Weiwoo/PeformanceTune/IceFreezeEnviroment",
							"DbRecoverFatal",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szWeiwooIceEnvDbRecoverFatal, 
							sizeof(holder.szWeiwooIceEnvDbRecoverFatal),
							NULL,
							optReadOnly);

		holder.addDetail("Weiwoo/PeformanceTune/session",
							"SavePeriod",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezeSessionSavePeriod, 
							sizeof(holder.szFreezeSessionSavePeriod),
							NULL,
							optReadOnly);
		holder.addDetail("Weiwoo/PeformanceTune/session",
							"SaveSizeTrigger",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezeSessionSaveSizeTrigger, 
							sizeof(holder.szFreezeSessionSaveSizeTrigger),
							NULL,
							optReadOnly);
		
        holder.addDetail("Weiwoo/BusinessRouter",
							"endpoint", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szBusinessRouterEndpoint, 
							sizeof(holder.szBusinessRouterEndpoint), 
							NULL, 
							optReadOnly);
        holder.addDetail("Weiwoo/BusinessRouter", 
							"mixedTeardownReason", 
							&WeiwooCfg::lMixTeardownReasonAndTerminateReason, 
							NULL, 
							optReadWrite);

        holder.addDetail("Weiwoo/DatabaseCache", 
							"sessionSize", 
							&WeiwooCfg::lEvictorWeiwooSessSize, 
							NULL, 
							optReadOnly);

        holder.addDetail("PathManager/Bind", 
							"endpoint", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szPathManagerEndpoint, 
							sizeof(holder.szPathManagerEndpoint), 
							NULL, 
							optReadOnly);
        holder.addDetail("PathManager/Bind", 
							"dispatchSize", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szPathManagerAdapterThreadPoolSize, 
							sizeof(holder.szPathManagerAdapterThreadPoolSize), 
							NULL, 
							optReadOnly);
        holder.addDetail("PathManager/Bind", 
							"dispatchMax", 
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szPathManagerAdapterThreadPoolSizeMax, 
							sizeof(holder.szPathManagerAdapterThreadPoolSizeMax), 
							NULL, 
							optReadOnly);

        holder.addDetail("PathManager/log", 
							"level", 
							&WeiwooCfg::lPathLogLevel, 
							NULL, 
							optReadOnly);
        holder.addDetail("PathManager/log", 
							"size", 
							&WeiwooCfg::lPathLogSize, 
							NULL, 
							optReadOnly);
        holder.addDetail("PathManager/log", 
							"buffer", 
							&WeiwooCfg::lPathLogBuffer, 
							NULL, 
							optReadOnly);
        holder.addDetail("PathManager/log", 
							"flushTimeout", 
							&WeiwooCfg::lPathLogTimeOut, 
							NULL, 
							optReadOnly);

		holder.addDetail("PathManager/PeformanceTune/IceFreezeEnviroment",
							"CheckpointPeriod",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szPathIceEnvCheckPointPeriod, 
							sizeof(holder.szPathIceEnvCheckPointPeriod),
							NULL,
							optReadOnly);
		holder.addDetail("PathManager/PeformanceTune/IceFreezeEnviroment",
							"DbRecoverFatal",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szPathIceEnvDbRecoverFatal, 
							sizeof(holder.szPathIceEnvDbRecoverFatal),
							NULL,
							optReadOnly);
		holder.addDetail("PathManager/PeformanceTune/storagelink",
							"SavePeriod",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezeStorageLinkSavePeriod, 
							sizeof(holder.szFreezeStorageLinkSavePeriod),
							NULL,
							optReadOnly);
		holder.addDetail("PathManager/PeformanceTune/storagelink",
							"SaveSizeTrigger",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezeStorgaeLinkSaveSizeTrigger, 
							sizeof(holder.szFreezeStorgaeLinkSaveSizeTrigger),
							NULL,
							optReadOnly);
		
		holder.addDetail("PathManager/PeformanceTune/streamlink",
							"SavePeriod",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezeStreamLinkSavePeriod, 
							sizeof(holder.szFreezeStreamLinkSavePeriod),
							NULL,
							optReadOnly);
		holder.addDetail("PathManager/PeformanceTune/streamlink",
							"SaveSizeTrigger",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezeStreamLinkSaveSizeTrigger, 
							sizeof(holder.szFreezeStreamLinkSaveSizeTrigger),
							NULL,
							optReadOnly);

		holder.addDetail("PathManager/PeformanceTune/pathticket",
							"SavePeriod",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezePathticketSavePeriod, 
							sizeof(holder.szFreezePathticketSavePeriod),
							NULL,
							optReadOnly);
		holder.addDetail("PathManager/PeformanceTune/pathticket",
							"SaveSizeTrigger",
							(Holder<WeiwooCfg>::PMem_CharArray)&WeiwooCfg::szFreezePathticketSaveSizeTrigger, 
							sizeof(holder.szFreezePathticketSaveSizeTrigger),
							NULL,
							optReadOnly);

        holder.addDetail("PathManager/DatabaseCache", 
							"storagelinkSize", 
							&WeiwooCfg::lEvictorStorlinkSize, 
							NULL, 
							optReadOnly);
        holder.addDetail("PathManager/DatabaseCache", 
							"streamlinkSize", 
							&WeiwooCfg::lEvictorStrmlinkSize, 
							NULL, 
							optReadOnly);
        holder.addDetail("PathManager/DatabaseCache", 
							"pathticketSize", 
							&WeiwooCfg::lEvictorPathTicketSize, 
							NULL, 
							optReadOnly);
		//	int32 lEnableReplicaSubscriber;
		//int32 lReplicaUpdateInterval;
		holder.addDetail("PathManager/ReplicaSubscriber",
							"enable",
							&WeiwooCfg::lEnableReplicaSubscriber,
							"0",
							optReadOnly);
		holder.addDetail("PathManager/ReplicaSubscriber",
							"updateInterval",
							&WeiwooCfg::lReplicaUpdateInterval,
							"60000",
							optReadOnly);
    }
    void readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
    {
        using namespace ZQ::common::Config;
        Holder<MonitoredLog> lmHolder;
        lmHolder.read(node, hPP);
        monitoredLogs.push_back(lmHolder);
    }

    void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
    {
        using namespace ZQ::common::Config;
        Holder<NVPair> propHolder;
        propHolder.read(node, hPP);
        iceProperties[propHolder.name] = propHolder.value;
    }
    void registerNothing(const std::string&){}
};

extern ZQ::common::Config::Loader<WeiwooCfg> gWeiwooServiceConfig;

#include <sstream>
class BerkeleyDBConfig
{
public:
	BerkeleyDBConfig()
	{
	}
	virtual ~BerkeleyDBConfig(){}
public:

	bool		generateConfig( const std::string& dbEnvPath , 
		int32	cacheSize	= 64*1024*1024, 
		int		cacheBlock	= 1,
		int32	maxlocks	= 100 * 1000,
		int32	maxObjs		= 100 * 1000, 
		int32	maxLockers	= 100 * 1000 )
	{
		std::ostringstream oss;
		oss<<"set_lk_max_locks " << maxlocks <<"\n";
		oss<<"set_lk_max_objects " << maxObjs <<"\n";
		oss<<"set_lk_max_lockers " << maxLockers << "\n";
		oss<<"set_cachesize 0 " << cacheSize << " "<< cacheBlock << "\n";
		std::string str = oss.str();
		std::string confPath = ZQTianShan::Util::fsConcatPath( dbEnvPath , "DB_CONFIG" );
		if( -1 == ::access( confPath.c_str(), 0 ) )
		{
			FILE * fConf = fopen( confPath.c_str() , "w+b");
			if( !fConf) return false;
			fwrite( str.c_str() , 1, str.length() ,fConf );
			fclose( fConf );
		}
		return true;
	}

private:

};

#endif//_WEIWOO_SERVICE_CONFIG_H__
