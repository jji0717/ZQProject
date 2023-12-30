#ifndef __TIANSHAN_SENTRY_CONFIG_H__
#define __TIANSHAN_SENTRY_CONFIG_H__
#include <ConfigHelper.h>

struct TimeServerCfg
{
	std::string timeServerAddress;
	int32 timeServerPort;
	static void structure(ZQ::common::Config::Holder<TimeServerCfg>& holder);
};
struct NTPClientCfg
{
	NTPClientCfg()
	{
		timeServerIndex = 0;
	}
	int32 ntpClientEnabled;
	int32 ntpClientAdjustTimeout;
	int32 ntpClientSyncInterval;
	int32 timeMaxOffset;

	int   timeServerIndex;

	typedef ZQ::common::Config::Holder<TimeServerCfg> TimeServerHolder;
	std::vector<TimeServerHolder> timeServerDatas;

	static void structure(ZQ::common::Config::Holder<NTPClientCfg>& holder);

	void readTimeServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerTimeServer(const std::string &full_path);
};
struct DiskMonitorConf {
    DiskMonitorConf() {
        enabled = 0;
        pollInterval = 60000;
        maxSkippedWarningCount = 0;
    }
    int32 enabled;
    int32 pollInterval;
    int32 maxSkippedWarningCount;
    std::string warningTargets;
    struct DiskConf {
        std::string path;
        std::string freeWarning;
        std::string repeatStep;
        static void structure(ZQ::common::Config::Holder<DiskConf>& holder) {
            holder.addDetail("", "name", &DiskConf::path);
            holder.addDetail("", "freeWarning", &DiskConf::freeWarning);
            holder.addDetail("", "repeatStep", &DiskConf::repeatStep);
        }
    };
    std::vector<DiskConf> pathList;
    static void structure(ZQ::common::Config::Holder<DiskMonitorConf>& holder) {
        holder.addDetail("", "enabled", &DiskMonitorConf::enabled, "1");
        holder.addDetail("", "pollInterval", &DiskMonitorConf::pollInterval, "60000");
        holder.addDetail("", "maxSkipped", &DiskMonitorConf::maxSkippedWarningCount, "10");
        holder.addDetail("", "reportBy", &DiskMonitorConf::warningTargets, "");
        holder.addDetail("Path", &DiskMonitorConf::readPath, &DiskMonitorConf::registerNothing);
    }
    void readPath(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        ZQ::common::Config::Holder<DiskConf> reader;
        reader.read(node, hPP);
        pathList.push_back(reader);
    }

    void registerNothing(const std::string &){}
};

struct DiskIOMonitorConf
{
	int32 enabled;
	int32 monitorInterval;
	std::string warningTargets;

	struct DiskDevs
	{
		std::string devName;
		std::string busyWarning;
		std::string queueSize;

		static void structure(ZQ::common::Config::Holder<DiskDevs>& holder) {
			holder.addDetail("", "name", &DiskDevs::devName, "");
			holder.addDetail("", "busyWarning", &DiskDevs::busyWarning, "0.0");
			holder.addDetail("", "queueSize", &DiskDevs::queueSize, "0.0");
        }
	};
	std::vector<struct DiskDevs> devsList;

	static void structure(ZQ::common::Config::Holder<DiskIOMonitorConf>& holder) {
        holder.addDetail("", "enabled", &DiskIOMonitorConf::enabled, "1");
        holder.addDetail("", "pollInterval", &DiskIOMonitorConf::monitorInterval, "30000");
        holder.addDetail("", "reportBy", &DiskIOMonitorConf::warningTargets, "");
        holder.addDetail("Device", &DiskIOMonitorConf::readPath, &DiskIOMonitorConf::registerNothing);
    }
    void readPath(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        ZQ::common::Config::Holder<DiskDevs> reader;
        reader.read(node, hPP);
        devsList.push_back(reader);
    }

    void registerNothing(const std::string &){}
};

struct SentryCfg
{
    char szIceStormEndPoint[512];
    int32 lIceTraceEnable;
    char szIceLogSuffix[512];
    int32 lIceLogLevel;
    int32 lIceLogSize;
    int32 lIceLogCount;
    char szServiceEndpoint[512];
    int32 lAdapterThreadpoolSize;

    int32 lServiceLogLevel;
    int32 lServiceLogSize;
    int32 lServiceLogBuffer;
    int32 lServiceLogTimeout;
    int32 lServiceLogCount;

	ZQ::common::Config::Holder<NTPClientCfg> ntpClientCfg;

	int32 ntpServerEnabled;
	std::string ntpServerListenAddress;
	int32 ntpServerListenPort;

    std::string webBindAddr; // bind address
    std::string webPubAddr; // published address
    int32 lHttpServePort;
    char szWebRoot[512];
    char szWebLayoutConfig[512];
    int32 httpConnIdleTimeout;

    std::string neighborGroupBind;
    std::string neighborGroupAddr;
    int32 neighborGroupPort;
	int32 reverseDomainName;

    char machineType[16];

    std::string crashDumpPath;
    int32 crashDumpEnabled;

    std::map<std::string, std::string> iceProps;
    std::map<std::string, std::string> webRefs;
    std::map<std::string, std::string> neighborPref;

    DiskMonitorConf diskMonitor;
	DiskIOMonitorConf diskIOMonitor;
    static void structure(ZQ::common::Config::Holder<SentryCfg> &holder)
    {
        using namespace ZQ::common::Config;
        holder.addDetail("default/EventChannel", "endpoint", (Holder<SentryCfg>::PMem_CharArray)&SentryCfg::szIceStormEndPoint, sizeof(holder.szIceStormEndPoint), NULL, optReadOnly);
        holder.addDetail("default/CrashDump", "path", &SentryCfg::crashDumpPath, NULL, optReadOnly);
        holder.addDetail("default/CrashDump", "enabled", &SentryCfg::crashDumpEnabled, "1", optReadOnly);

        holder.addDetail("default/IceTrace", "enabled", &SentryCfg::lIceTraceEnable, "1", optReadOnly);
        holder.addDetail("default/IceTrace", "level", &SentryCfg::lIceLogLevel, "7", optReadOnly);
        holder.addDetail("default/IceTrace", "size", &SentryCfg::lIceLogSize, "10240000", optReadOnly);
        holder.addDetail("default/IceTrace", "count", &SentryCfg::lIceLogCount, "5", optReadOnly);

        holder.addDetail("default/IceProperties/prop", &SentryCfg::readIceProp, &SentryCfg::registerNothing);

        holder.addDetail("Sentry/Bind", "endpoint", (Holder<SentryCfg>::PMem_CharArray)&SentryCfg::szServiceEndpoint, sizeof(holder.szServiceEndpoint), NULL, optReadOnly);
        holder.addDetail("Sentry/Bind", "threadpool", &SentryCfg::lAdapterThreadpoolSize, NULL, optReadOnly);

        holder.addDetail("Sentry/HttpLog", "level", &SentryCfg::lServiceLogLevel, "7", optReadOnly);
        holder.addDetail("Sentry/HttpLog", "size", &SentryCfg::lServiceLogSize, "10240000", optReadOnly);
        holder.addDetail("Sentry/HttpLog", "buffer", &SentryCfg::lServiceLogBuffer, "10240", optReadOnly);
        holder.addDetail("Sentry/HttpLog", "flushTimeout", &SentryCfg::lServiceLogTimeout, "2", optReadOnly);
        holder.addDetail("Sentry/HttpLog", "count", &SentryCfg::lServiceLogCount, "5", optReadOnly);

        holder.addDetail("Sentry/Neighbor", "groupBind", &SentryCfg::neighborGroupBind);
        holder.addDetail("Sentry/Neighbor", "groupAddress", &SentryCfg::neighborGroupAddr);
        holder.addDetail("Sentry/Neighbor", "groupPort", &SentryCfg::neighborGroupPort);
		holder.addDetail("Sentry/Neighbor", "reverseDomainName", &SentryCfg::reverseDomainName,"0");

        holder.addDetail("Sentry/http", "bindAddr", &SentryCfg::webBindAddr);
        holder.addDetail("Sentry/http", "pubAddr", &SentryCfg::webPubAddr, "");
        holder.addDetail("Sentry/http", "ServePort", &SentryCfg::lHttpServePort, "80", optReadOnly);
        holder.addDetail("Sentry/http", "webRoot", (Holder<SentryCfg>::PMem_CharArray)&SentryCfg::szWebRoot, sizeof(holder.szWebRoot), NULL, optReadOnly);
        holder.addDetail("Sentry/http", "weblayoutConfig", (Holder<SentryCfg>::PMem_CharArray)&SentryCfg::szWebLayoutConfig, sizeof(holder.szWebLayoutConfig), NULL, optReadOnly);
        holder.addDetail("Sentry/http", "idleTimeout", &SentryCfg::httpConnIdleTimeout, "300000", optReadOnly);

		holder.addDetail("Sentry/NTP", &SentryCfg::readNTPClient, &SentryCfg::registerNothing);
		/*holder.addDetail("Sentry/NTP", "enabled", &SentryCfg::ntpClientEnabled, "1", optReadOnly);
		holder.addDetail("Sentry/NTP", "adjustTimeout", &SentryCfg::ntpClientAdjustTimeout, "", optReadOnly);
        holder.addDetail("Sentry/NTP", "syncInterval", &SentryCfg::ntpClientSyncInterval, "", optReadOnly);
		holder.addDetail("Sentry/NTP", "timeMaxOffset", &SentryCfg::timeMaxOffset, "", optReadOnly);
        holder.addDetail("Sentry/NTP/TimeServer", "address", &SentryCfg::timeServerAddress, "", optReadOnly);
        holder.addDetail("Sentry/NTP/TimeServer", "port", &SentryCfg::timeServerPort, "123", optReadOnly);*/
		holder.addDetail("Sentry/NTPServer", "enabled", &SentryCfg::ntpServerEnabled, "1", optReadOnly);
		holder.addDetail("Sentry/NTPServer", "address", &SentryCfg::ntpServerListenAddress, "0.0.0.0", optReadOnly);
		holder.addDetail("Sentry/NTPServer", "port", &SentryCfg::ntpServerListenPort, "", optReadOnly);

        holder.addDetail("Sentry/WebReference/reference", &SentryCfg::readWebRef, &SentryCfg::registerNothing);
        holder.addDetail("Sentry/DiskSpaceMonitor", &SentryCfg::readDiskMonitor, &SentryCfg::registerNothing, ZQ::common::Config::Range(0, 1));
		holder.addDetail("Sentry/DiskIOMonitor", &SentryCfg::readDiskIOMonitor, &SentryCfg::registerNothing, ZQ::common::Config::Range(0, 1));
        
        holder.addDetail("Sentry", "type", (Holder<SentryCfg>::PMem_CharArray)&SentryCfg::machineType, sizeof(holder.machineType), "", optReadOnly);
    }

    void readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common::Config;
        Holder<NVPair> nvHolder;
        nvHolder.read(node, hPP);
        iceProps[nvHolder.name] = nvHolder.value;
    }
    void readWebRef(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common::Config;
        Holder<NVPair> nvHolder;
        nvHolder.read(node, hPP);
        webRefs[nvHolder.name] = nvHolder.value;
        
    }
	void readNTPClient(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ntpClientCfg.read(node, hPP);
	}

    void readDiskMonitor(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<DiskMonitorConf> holder;
        holder.read(node, hPP);
        diskMonitor = holder;
    }
	
	 void readDiskIOMonitor(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<DiskIOMonitorConf> holder;
        holder.read(node, hPP);
        diskIOMonitor = holder;
    }

    void registerNothing(const std::string &){}
};

extern ZQ::common::Config::Loader<SentryCfg> gSentryCfg;

#endif

