

#ifndef _CPH_CSI_CONFIG_
#define _CPH_CSI_CONFIG_

#include <ConfigHelper.h>

struct Method
{
	std::string methodName;
	int32 maxSession;
	int32 maxBandwidth;
	int32 enableFlag;
	static void structure(ZQ::common::Config::Holder<Method > &holder)
	{
		holder.addDetail("", "name", &Method::methodName, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessions", &Method::maxSession, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxBandwidth", &Method::maxBandwidth, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "enable", &Method::enableFlag, NULL, ZQ::common::Config::optReadOnly);
	}
};
struct PortRange
{
	int32  portMin;
	int32  portMax;
	int32  portEnable;
	static void structure(ZQ::common::Config::Holder<PortRange > &holder)
	{
		holder.addDetail("", "min", &PortRange::portMin, "15000",ZQ::common::Config::optReadOnly);
		holder.addDetail("", "max", &PortRange::portMax, "15200", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "enable", &PortRange::portEnable, "0", ZQ::common::Config::optReadOnly);
	}
};

struct UnifiedTrickFile
{
	int32 enable;
	static void structure(ZQ::common::Config::Holder<UnifiedTrickFile > &holder)
	{
		holder.addDetail("", "enable", &UnifiedTrickFile::enable, NULL,ZQ::common::Config::optReadOnly);
	}
	UnifiedTrickFile()
	{
		enable = true;
	};
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
class CPHCSICfg
{
public:
	CPHCSICfg ();
	~CPHCSICfg ();

    static void structure(ZQ::common::Config::Holder<CPHCSICfg> &holder);
    void registerNothing(const std::string&){}

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;
	typedef std::vector< ZQ::common::Config::Holder< PortRange > > Ranges;
	Ranges ranges;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readRange(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	typedef std::vector< ZQ::common::Config::Holder<NetInterface>> NetInterfaces;
	NetInterfaces nInterface;
	void readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);


	char	szCacheDir[256];
	int32	mediaSampleSize;
	int32   maxAllocSampleCount;
	int32	bandwidthLimitRate;
	int32	enableProgEvent;
	int32	enableStreamEvent;
	int32	streamReqSecs;
	int32	enableMD5;
	int32	enablePacingTrace;
	int32	traceIndexWrite;
	int32	enableCacheForIndex;
	int32	enableRAID1ForIndex;

	//for url encode
	char    szUrlEncode[256];
	int     decodeSourceURL;

	char	szLocalNetIf[256];	
	
	// for RTF
	int32	rtfMaxSessionNum;
	int32	rtfMaxInputBufferBytes;
	int32	rtfMaxInputBuffersPerSession;
	int32	rtfSessionFailThreshold;

	int32   timeoutInterval;

	int32   winpcapKernelBufferInMB;
	int32	winpcapMinBufferToCopyInKB;

	// for dump
	char	szDumpPath[256];
	int32	enableDump;
	int32	deleteDumpOnSuccess;

	// process ability
	//int32	maxBandwidthKBps;
	//int32	maxSessionNum;
	
	// for vstrm bandwidth management
	int32	vstrmBwClientId;

	int32	vstrmDisableBufDrvThrottle;

	// for test
	char	szNTFSOutputDir[256];	
	int32	enableTestNTFS;			//if 1 then do not write to vstrm but write to NTFS

	int32  enableCmdGetsize; 
	int32  enableFtpPassiveMode;
	int32  ftpConnectionInterval;
	int32  enableLegacyAudioOnly;

	int32  portMin;
	int32  portMax;
	
	std::string strAugmentationPids;

	UnifiedTrickFile unifiedtrickfile;
};


extern ZQ::common::Config::Loader<CPHCSICfg> _gCPHCfg;

#endif