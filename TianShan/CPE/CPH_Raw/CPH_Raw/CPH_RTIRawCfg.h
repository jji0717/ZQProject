#ifndef _CPH_RTIRAW_CONFIG_H
#define _CPH_RTIRAW_CONFIG_H

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
/*
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
*/
struct RtiRawConfig
{
	RtiRawConfig();

	int32	mediaSampleSize;
	char	szCacheDir[256];
	int32	enablePacingTrace;

 	int32	enableMD5;
 	int32	enableCacheForIndex;
 	int32	enableRAID1ForIndex;

	char    szlocalIp[256];
	int32   timeoutInterval;

	int32 enableProgEvent;
	int32 enableStreamEvent;
	int32 streamReqSecs;

	// for test
	//std::string localIp;
	// for dump
	char	szDumpPath[256];
	int32	enableDump;
	int32	deleteDumpOnSuccess;

	int32	rtfMaxSessionNum;
	int32	rtfMaxInputBufferBytes;
	int32	rtfMaxInputBuffersPerSession;
	int32	rtfSessionFailThreshold;

	int32   retryCaptureCount;

	int32		warningDiskWriteLongLatency;
	int32	preloadTime;			//session preload time in milliseconds 
	int32   deleteTargetFileCapFail;
	static void structure(ZQ::common::Config::Holder<RtiRawConfig> &holder);

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;

	//capture
	std::string captureIp;
	int totalBandwidth;
	int32   winpcapKernelBufferInMB;
	int32	winpcapMinBufferToCopyInKB;
//	typedef std::vector< ZQ::common::Config::Holder<NetInterface> > NetInterfaces;
 //	NetInterfaces nInterface;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerNothing(const std::string&){}
};

extern ZQ::common::Config::Loader<RtiRawConfig> _gCPHCfg;



#endif
