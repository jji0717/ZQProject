

#ifndef _CPH_RDSRTF_CONFIG_
#define _CPH_RDSRTF_CONFIG_

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
struct MountPath
{
	std::string entry;
	static void structure(ZQ::common::Config::Holder<MountPath > &holder)
	{
		holder.addDetail("", "entry", &MountPath::entry, NULL,ZQ::common::Config::optReadOnly);
	}
	MountPath()
	{
		entry = "/mnt/CPESvc/";
	};
};
struct FileSync
{
	int     maxSyncTimeout;
	int     bytesToSync;
	static void structure(ZQ::common::Config::Holder<FileSync > &holder)
	{
		holder.addDetail("", "maxSyncTimeout", &FileSync::maxSyncTimeout, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "bytesToSync", &FileSync::bytesToSync, NULL,ZQ::common::Config::optReadOnly);

	}
	FileSync()
	{
		maxSyncTimeout = -1;
		bytesToSync = -1;
	};
};
struct CiscoFileExt
{
	int32 mode;
	static void structure(ZQ::common::Config::Holder<CiscoFileExt > &holder)
	{
		holder.addDetail("", "mode", &CiscoFileExt::mode, NULL,ZQ::common::Config::optReadOnly);
	}
	CiscoFileExt()
	{
		mode = 0;
	};
};
struct SparseFile
{
	int32 sparseFilesize;
	static void structure(ZQ::common::Config::Holder<SparseFile > &holder)
	{
		holder.addDetail("", "sparseFilesize", &SparseFile::sparseFilesize, NULL,ZQ::common::Config::optReadOnly);

	}
	SparseFile()
	{
		sparseFilesize = 20000000; //kb 20G
	}
};

struct SleepTime
{
	int32 timeInterval;
	static void structure(ZQ::common::Config::Holder<SleepTime > &holder)
	{
		holder.addDetail("", "timeInterval", &SleepTime::timeInterval, NULL,ZQ::common::Config::optReadOnly);

	}
	SleepTime()
	{
		timeInterval = 5000; //ms
	}
};
class CPHConfig
{
public:
	CPHConfig ();
	~CPHConfig ();

    static void structure(ZQ::common::Config::Holder<CPHConfig> &holder);
    void registerNothing(const std::string&){}

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readCiscoFileExt(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readMountPath(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readsparseFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readFileSync(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readsleepTime(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	char	szCacheDir[256];
	int32	mediaSampleSize;
	int32   maxAllocSampleCount;
	int32	bandwidthLimitRate;
	int32	enableProgEvent;
	int32	enableStreamEvent;
	int32	streamReqSecs;
	int32	enableMD5;
	int32	enableCacheForIndex;
    int32   enablePacing;

	//for url encode
	char    szUrlEncode[256];
	int     decodeSourceURL;

	char	szLocalNetIf[256];	
	
	// for RTF
	int32	rtfMaxSessionNum;
	int32	rtfMaxInputBufferBytes;
	int32	rtfMaxInputBuffersPerSession;
	int32	rtfSessionFailThreshold;

	// process ability
	//int32	maxBandwidthKBps;
	//int32	maxSessionNum;
	
	// for vstrm bandwidth management
	int32	vstrmBwClientId;

	int32	vstrmDisableBufDrvThrottle;

	// for test
	char	szNTFSOutputDir[256];	
	int32	enableTestNTFS;			//if 1 then do not write to vstrm but write to NTFS

	int32  enableNSF;
	int32  enableCmdGetsize; 
	int32  timeoutForGrowing;
	int32  bandwidthCtrlInterval;
	char   szTargetDir[256];
	int32  enableResumeForDownload;
	char   szPaceDllPath[256];

	std::string strAugmentationPids;
	UnifiedTrickFile unifiedtrickfile;

	CiscoFileExt     ciscofileext;

	MountPath       mountpath;

	int32  ingressCapcaity;
	std::string bindip;
	std::string transferip;
	std::string transferIpForCache;
	int32   nspeed;
	int32   transferdelay;
	int32   transferPort;
	int32   timeout;

	int32   deleteOnFail;
	FileSync filesync;

	SparseFile   sparsefile;
	SleepTime    sleeptime;
};


extern ZQ::common::Config::Loader<CPHConfig> _gCPHCfg;

#endif

