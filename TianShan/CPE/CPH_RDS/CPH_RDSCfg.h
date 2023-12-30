

#ifndef _CPH_RDS_CONFIG_
#define _CPH_RDS_CONFIG_

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

class CPHRDSCfg
{
public:
	CPHRDSCfg ();
	~CPHRDSCfg ();

    static void structure(ZQ::common::Config::Holder<CPHRDSCfg> &holder);
    void registerNothing(const std::string&){}
	
	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	char	szCacheDir[256];
	int32	mediaSampleSize;
	int32	enableProgEvent;
	int32	enableStreamEvent;
	int32	streamReqSecs;
	int32	maxCodingError;
	int32	enableMD5;
	int32	enablePacingTrace;
	int32	bandwidthLimitRate;

	//int32	maxBandwidthKBps;
	//int32	maxSessionNum;
	
	// for vstrm bandwidth management
	int32	vstrmBwClientId;
	int32	vstrmDisableBufDrvThrottle;

	// for test
	char	szNTFSOutputDir[256];	
	int32	enableTestNTFS;			//if 1 then do not write to vstrm but write to NTFS
};


extern ZQ::common::Config::Loader<CPHRDSCfg> _gCPHCfg;

#endif