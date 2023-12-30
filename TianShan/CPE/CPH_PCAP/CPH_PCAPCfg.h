

#ifndef _CPH_PCAP_CONFIG_
#define _CPH_PCAP_CONFIG_

#include <ConfigHelper.h>

struct Method
{
	std::string methodName;
	int32 maxSession;
	int32 maxBandwidth;
	int32 enableFlag;
	int32 shareFlag;
	static void structure(ZQ::common::Config::Holder<Method > &holder)
	{
		holder.addDetail("", "name", &Method::methodName, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessions", &Method::maxSession, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxBandwidth", &Method::maxBandwidth, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "enable", &Method::enableFlag, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "share", &Method::shareFlag, "0", ZQ::common::Config::optReadOnly);

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

struct DirectIO
{
	int32 enable;
	static void structure(ZQ::common::Config::Holder<DirectIO > &holder)
	{
		holder.addDetail("", "enable", &DirectIO::enable, NULL,ZQ::common::Config::optReadOnly);
	}
	DirectIO()
	{
		enable = false;
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
struct PCAPConfig
{
	PCAPConfig();

	//int32   maxSessionNum;
	int32	mediaSampleSize;
	int32   maxAllocSampleCount;

	int32	enableMD5;
	int32	enableCacheForIndex;
	
	char    szlocalIp[256];
	int32   timeoutInterval;
	
	int32   winpcapKernelBufferInMB;
	int32	winpcapMinBufferToCopyInKB;

	int32 enableProgEvent;
	int32 enableStreamEvent;
	//int32 maxBandwidthKBps;
	int32 streamReqSecs;

	// for test
	char	szNTFSOutputDir[256];	
	int32	enableTestNTFS;

	// for dump
	char	szDumpPath[256];
	int32	enableDump;
	int32	deleteDumpOnSuccess;

	//for pacing
	char   szPaceDllPath[256];
	int32   enablePacing;
	

	int32	rtfMaxSessionNum;	
	int32	rtfMaxInputBufferBytes;
	int32	rtfMaxInputBuffersPerSession;
	int32	rtfSessionFailThreshold;
	int32   deleteTargetFileCapFail;
	UnifiedTrickFile unifiedtrickfile;
	CiscoFileExt     ciscofileext;
	DirectIO         directIO;
	int32	preloadTime;			//session preload time in milliseconds 
	
	int32   retryCaptureCount;
	int32		warningDiskWriteLongLatency;
  	FileSync filesync;  
    static void structure(ZQ::common::Config::Holder<PCAPConfig> &holder);

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;
	typedef std::vector< ZQ::common::Config::Holder<NetInterface> > NetInterfaces;
	NetInterfaces nInterface;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
   
    void registerNothing(const std::string&){}
	void readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readCiscoFileExt(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readDirectIO(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readFileSync(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

};

extern ZQ::common::Config::Loader<PCAPConfig> _gCPHCfg;

#endif
