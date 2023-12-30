#ifndef _SITE_ADMIN_SERVICE_CONIG_H__
#define _SITE_ADMIN_SERVICE_CONIG_H__
#include <ConfigHelper.h>
#include <configloader.h>


struct PerformanceTuneCfg
{
	std::string		strCheckpointPeriod;
	std::string		strDbRecoverFatal;

	std::string		strLivTxnSavePeriod;
	std::string		strLivTxnSaveSizeTrigger;

	static void structure( ZQ::common::Config::Holder<PerformanceTuneCfg> &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("IceFreezeEnviroment",
						"CheckpointPeriod",
						&PerformanceTuneCfg::strCheckpointPeriod,
						"240",
						optReadOnly);

		holder.addDetail("IceFreezeEnviroment",
						"DbRecoverFatal",
						&PerformanceTuneCfg::strDbRecoverFatal,
						"1",
						optReadOnly);

		holder.addDetail("LiveTxn",
						"SavePeriod",
						&PerformanceTuneCfg::strLivTxnSavePeriod,
						"240",
						optReadOnly);

		holder.addDetail("LiveTxn",
						"SaveSizeTrigger",
						&PerformanceTuneCfg::strLivTxnSaveSizeTrigger,
						"10000",
						optReadOnly);
	}
};

struct EventSinkModule
{
    std::string file;
    std::string type;
    std::string config;
    int32 enable;
    static void structure(ZQ::common::Config::Holder<EventSinkModule> &holder)
    {
        holder.addDetail("", "file", &EventSinkModule::file);
        holder.addDetail("", "type", &EventSinkModule::type);
        holder.addDetail("", "config", &EventSinkModule::config);
        holder.addDetail("", "enable", &EventSinkModule::enable);
    }
};
struct SAConfig
{
    char			szIceStormEndpoint[512];
    char			szCrashDumpPath[512];
    int32			lCrashDumpEnable;

    char			szDatabaseFolder[1024];
	char			szRuntimeDbFolder[1024];
    
    int32			lIceTraceEnable;
    char			szIceTraceLogName[512];
    int32			lIceTraceLogLevel;
    int32			lIceTraceLogSize;
    int32			lIceTraceCount;

	char			szTxnDataDest[512];
	char			szTxnDataTemplate[512];
	int32			lTxnDataSize;
	int32			lTxnDataNumber;
    int32           lTxnDataEnabled;

    char			szServiceEndpoint[512];
    int32			lIceAdapterThreadpool;
    int32			lSvcThreadPoolSize;
    std::string     urlMode;
    

    typedef std::map<std::string, std::string> IceProps;
    IceProps iceProps;
    typedef std::vector<EventSinkModule> EventSinkModules;
    EventSinkModules eventSinkModules;
	ZQ::common::Config::Holder<PerformanceTuneCfg> performanceTune;
    static void structure(ZQ::common::Config::Holder<SAConfig> &holder)
    {
        using namespace ZQ::common::Config;
        typedef ZQ::common::Config::Holder<SAConfig>::PMem_CharArray PMem_CharArray;
        holder.addDetail("default/EventChannel", "endpoint", (PMem_CharArray)&SAConfig::szIceStormEndpoint, sizeof(holder.szIceStormEndpoint), "", optReadOnly);
        holder.addDetail("default/CrashDump", "path", (PMem_CharArray)&SAConfig::szCrashDumpPath, sizeof(holder.szCrashDumpPath), NULL, optReadOnly);
        holder.addDetail("default/CrashDump", "enableDump", &SAConfig::lCrashDumpEnable, NULL, optReadOnly);
        holder.addDetail("default/DatabaseFolder", "path", (PMem_CharArray)&SAConfig::szDatabaseFolder, sizeof(holder.szDatabaseFolder), NULL, optReadOnly);
		//szRuntimeDbFolder
		holder.addDetail("default/DatabaseFolder", "runtimeDbPath", (PMem_CharArray)&SAConfig::szRuntimeDbFolder, sizeof(holder.szRuntimeDbFolder), NULL, optReadOnly);
        
        holder.addDetail("default/IceTrace", "enabled", &SAConfig::lIceTraceEnable, "1", optReadOnly);
        holder.addDetail("default/IceTrace", "logfileSuffix", (PMem_CharArray)&SAConfig::szIceTraceLogName,	sizeof(holder.szIceTraceLogName), "SiteAdmin.ice.log", optReadOnly);
        holder.addDetail("default/IceTrace", "level", &SAConfig::lIceTraceLogLevel, "7", optReadWrite);
        holder.addDetail("default/IceTrace", "size", &SAConfig::lIceTraceLogSize,	"10240000", optReadOnly);
        holder.addDetail("default/IceTrace", "count", &SAConfig::lIceTraceCount, "5", optReadOnly);

//		<TxnData destination="C:\TianShan\data\TxnData_template.mdb" 
//			template="C:\TianShan\data\TxnData_template.mdb" size="20000000" number="5"/>
		holder.addDetail("default/TxnData", "destination", (PMem_CharArray)&SAConfig::szTxnDataDest, sizeof(holder.szTxnDataDest), "", optReadOnly);
		holder.addDetail("default/TxnData", "template", (PMem_CharArray)&SAConfig::szTxnDataTemplate, sizeof(holder.szTxnDataTemplate), "", optReadOnly);
		holder.addDetail("default/TxnData", "size", &SAConfig::lTxnDataSize, "20000000", optReadOnly);
		holder.addDetail("default/TxnData", "number", &SAConfig::lTxnDataNumber, "5", optReadOnly);
        holder.addDetail("default/TxnData", "enabled", &SAConfig::lTxnDataEnabled, "1", optReadOnly);

        holder.addDetail("SiteAdmin/Service/SiteAdminService", "endPoint", (PMem_CharArray)&SAConfig::szServiceEndpoint, sizeof(holder.szServiceEndpoint), NULL, optReadOnly, "SiteAdminEndpoint");
        holder.addDetail("SiteAdmin/Service/SiteAdminService", "urlmode", &SAConfig::urlMode, "DNS", optReadOnly, "URLMode");
        holder.addDetail("SiteAdmin/Service/ServiceThread", "count", &SAConfig::lSvcThreadPoolSize, "10", optReadOnly, "ServiceThreadPoolSize");

        holder.addDetail("default/IceProperties/prop", &SAConfig::readIceProp, &SAConfig::registerNothing);
        holder.addDetail("SiteAdmin/EventSinkPlugin/EventSinkModule", &SAConfig::readEventSinkModule, &SAConfig::registerNothing);

		holder.addDetail("SiteAdmin/Service/SiteAdminService/PeformanceTune",&SAConfig::readPerformanceTune, &SAConfig::registerNothing );
    }

    void readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<Config::NVPair> propHolder("");
        propHolder.read(node, hPP);
        iceProps[propHolder.name] = propHolder.value;
    }
    void readEventSinkModule(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<EventSinkModule> esmHolder("");
        esmHolder.read(node, hPP);
        eventSinkModules.push_back(esmHolder);
    }

	void readPerformanceTune( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		performanceTune.read( node , hPP );
	}

    void registerNothing(const std::string&){}
};


extern ZQ::common::Config::Loader<SAConfig> gSiteAdminConfig;
#endif//_SITE_ADMIN_SERVICE_CONIG_H__
