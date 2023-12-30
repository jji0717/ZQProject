

#ifndef _CPH_NPVR_CONFIG_
#define _CPH_NPVR_CONFIG_

#include <ConfigHelper.h>

struct Method
{
	std::string methodName;
	int32 maxSession;
	int32 maxBandwidth;
	static void structure(ZQ::common::Config::Holder<Method > &holder)
	{
		holder.addDetail("", "name", &Method::methodName, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessions", &Method::maxSession, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxBandwidth", &Method::maxBandwidth, NULL, ZQ::common::Config::optReadOnly);
	}
};

struct NetInterface
{
	std::string strIp;
	int totalBandwidth;

	static void structure(ZQ::common::Config::Holder<NetInterface > &holder)
	{
		holder.addDetail("", "ip", &NetInterface::strIp, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "bandwidth", &NetInterface::totalBandwidth, NULL, ZQ::common::Config::optReadOnly);
	}
};

struct NPVRConfig
{
	NPVRConfig();

	char	szCacheDir[256];
	int32	enablePacingTrace;
	int32	enableMD5;
    int32	mediaSampleSize;

	int32	enableCacheForIndex;
	int32	enableRAID1ForIndex;

	char    szlocalIp[256];
	int32   timeoutInterval;
	int32 streamReqSecs;
	int32 enableProgEvent;
	int32 enableStreamEvent;

	int32   winpcapKernelBufferInMB;
	int32	winpcapMinBufferToCopyInKB;

	char	szNTFSOutputDir[256];
	char    szNTFSSource[256];
	int32	enableTestNTFS;
	// for dump
	char	szDumpPath[256];
	int32	enableDump;
	int32	deleteDumpOnSuccess;

	int32	delayDataNotify;		//dealy time in miliseconds for notify data processing thread

	int32	vstrmBwClientId;
	int32	vstrmDisableBufDrvThrottle;

	int32	rtfMaxSessionNum;
	int32	rtfMaxInputBufferBytes;
	int32	rtfMaxInputBuffersPerSession;
	int32	rtfSessionFailThreshold;
	
	int32	preloadTime;			//session preload time in milliseconds 

	int32   leadsesslagAfterIdle;
	int32   monitorInterval;
	int32   enableNtfsSource;
    int32   progressSendInterval;
	int32   maxLeadsessionNum;
    static void structure(ZQ::common::Config::Holder<NPVRConfig> &holder);

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;
	typedef std::vector< ZQ::common::Config::Holder<NetInterface>> NetInterfaces;
	NetInterfaces nInterface;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
   
    void registerNothing(const std::string&){}
};

extern ZQ::common::Config::Loader<NPVRConfig> _gCPHCfg;

#endif