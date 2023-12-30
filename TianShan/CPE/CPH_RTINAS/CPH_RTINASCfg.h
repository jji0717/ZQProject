

#ifndef _CPH_RDSRTF_CONFIG_
#define _CPH_RDSRTF_CONFIG_

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

class CPHRTINasCfg
{
public:
	CPHRTINasCfg ();
	~CPHRTINasCfg ();

    static void structure(ZQ::common::Config::Holder<CPHRTINasCfg> &holder);
	void registerNothing(const std::string&){}

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	

	char	szCacheDir[256];
	int32	mediaSampleSize;
	int32	bandwidthLimitRate;
	int32	enableProgEvent;
	int32	enableStreamEvent;
	int32	streamReqSecs;
	int32	enableMD5;
	int32	enablePacingTrace;
	int32	enableCacheForIndex;

	// for RTF
	int32	rtfMaxSessionNum;
	int32	rtfMaxInputBufferBytes;
	int32	rtfMaxInputBuffersPerSession;
	int32	rtfSessionFailThreshold;

	int32   winpcapKernelBufferInMB;
	int32	winpcapMinBufferToCopyInKB;

	int32	preloadTime;			//session preload time in milliseconds 

	// for dump
	char	szDumpPath[256];
	int32	enableDump;
	int32	deleteDumpOnSuccess;

	// process ability
	//int32	maxBandwidthKBps;
	//int32	maxSessionNum;

	//for capture media
	char    szlocalIp[256];
	int32   timeoutInterval;

	char    nasRoot[256];
	
};


extern ZQ::common::Config::Loader<CPHRTINasCfg> _gCPHCfg;

#endif