#ifndef _CPH_AQUALIB_CONFIG_
#define _CPH_AQUALIB_CONFIG_

#include <ConfigHelper.h>

namespace ZQTianShan {
namespace ContentProvision{

struct Method
{
	std::string methodName;
	int32 maxSession;
	int32 maxBandwidth;
	int32 enableFlag;

    typedef ZQ::common::Config::Holder<Method> MethodHolder;
	static void structure(MethodHolder &holder)
	{
		holder.addDetail("", "name",        &Method::methodName,    NULL,   ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessions", &Method::maxSession,    NULL,   ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxBandwidth",&Method::maxBandwidth,  NULL,   ZQ::common::Config::optReadOnly);
		holder.addDetail("", "enable",      &Method::enableFlag,    NULL,   ZQ::common::Config::optReadOnly);
	}
};

struct ProvisionMethod
{
    std::vector<Method::MethodHolder> methods;

    typedef ZQ::common::Config::Holder<ProvisionMethod> ProvisionMethodHolder;

    static void structure(ProvisionMethodHolder& holder)
    {
        holder.addDetail("Method", &ProvisionMethod::readMethod, &ProvisionMethod::registerMethod);
    }

    void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        Method::MethodHolder methodHolder;
        methodHolder.read(node, hPP);
        methods.push_back(methodHolder);
    }

    void registerMethod(const std::string &full_path)
    {
        for (std::vector<Method::MethodHolder>::iterator 
            it = methods.begin(); 
            it != methods.end(); it ++)
        {
            it->snmpRegister(full_path);
        }
    }
};

struct NetInterface
{
    std::string ip;
    int32 bandwidth;

    typedef ZQ::common::Config::Holder<NetInterface> NetInterfaceHolder;
    static void structure(NetInterfaceHolder &holder)
    {
        holder.addDetail("", "ip",          &NetInterface::ip,          NULL,   ZQ::common::Config::optReadOnly);
        holder.addDetail("", "bandwidth",   &NetInterface::bandwidth,   NULL,   ZQ::common::Config::optReadOnly);
    }
};

struct Capture
{
    int32 sessionTimeout;
    int32 winpcapKernelBuffer;
    int32 winpcapMinBufferCopy;

    std::vector<NetInterface::NetInterfaceHolder> interfaces;

    typedef ZQ::common::Config::Holder<Capture> CaptureHolder;

    static void structure(CaptureHolder &holder)
    {
        holder.addDetail("", "sessionTimeout",      &Capture::sessionTimeout,       NULL,   ZQ::common::Config::optReadOnly);
        holder.addDetail("", "winpcapKernelBuffer", &Capture::winpcapKernelBuffer,  NULL,   ZQ::common::Config::optReadOnly);
        holder.addDetail("", "winpcapMinBufferCopy",&Capture::winpcapMinBufferCopy, NULL,   ZQ::common::Config::optReadOnly);

        holder.addDetail("NetworkInterface", &Capture::readNetworkInterface, &Capture::registerNetworkInterface);
    }

    void readNetworkInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        NetInterface::NetInterfaceHolder network("");
        network.read(node, hPP);
        interfaces.push_back(network);
    }
    void registerNetworkInterface(const std::string &full_path)
    {
        for (std::vector<NetInterface::NetInterfaceHolder>::iterator 
            it = interfaces.begin(); 
            it != interfaces.end(); it ++)
        {
            it->snmpRegister(full_path);
        }
    }
};

struct CaptureDumper
{
    int enable;
    std::string dumpPath;
    int deleteOnSuccess;

    typedef ZQ::common::Config::Holder<CaptureDumper> CaptureDumperHolder;
    static void structure(CaptureDumperHolder &holder)
    {
        holder.addDetail("", "enable",          &CaptureDumper::enable,         NULL,   ZQ::common::Config::optReadOnly);
        holder.addDetail("", "dumpPath",        &CaptureDumper::dumpPath,       NULL,   ZQ::common::Config::optReadOnly);
        holder.addDetail("", "deleteOnSuccess", &CaptureDumper::deleteOnSuccess,NULL,   ZQ::common::Config::optReadOnly);
    }
};

struct UnifiedTrickFile
{
	int32 enable;
    
    typedef ZQ::common::Config::Holder<UnifiedTrickFile> UnifiedTrickFileHolder;
	static void structure(UnifiedTrickFileHolder &holder)
	{
		holder.addDetail("", "enable", &UnifiedTrickFile::enable, "1", ZQ::common::Config::optReadOnly);
	}
};

struct MountPath
{
    std::string entry;

    typedef ZQ::common::Config::Holder<MountPath> MountPathHolder;
    static void structure(ZQ::common::Config::Holder<MountPath > &holder)
    {
        holder.addDetail("", "entry", &MountPath::entry, "/mnt/CPESvc/", ZQ::common::Config::optReadOnly);
    }
};

struct RetryCapture
{
    int32 retrycount;

    typedef ZQ::common::Config::Holder<RetryCapture> RetryCaptureHolder;
    static void structure(RetryCaptureHolder &holder)
    {
        holder.addDetail("", "retrycount", &RetryCapture::retrycount, "", ZQ::common::Config::optReadOnly);
    }
};

struct CiscoFileExt
{
	int32 mode;

    typedef ZQ::common::Config::Holder<CiscoFileExt> CiscoFileExtHolder;

	static void structure(CiscoFileExtHolder &holder)
	{
		holder.addDetail("", "mode", &CiscoFileExt::mode, "0", ZQ::common::Config::optReadOnly);
	}
};

struct AquaServer
{
    std::string rootUrl;
    std::string homeContainer;
    std::string userDomain;
    int32       connectTimeout; //ms
    int32       timeout;        //ms
    int32       flag;
    int32       maxThreadPoolSize;
    std::string mainFileExtension;
	std::string ContentNameFormat;
    std::string bindIp;


    typedef ZQ::common::Config::Holder<AquaServer> AquaServerHolder;

    static void structure(AquaServerHolder &holder)
    {
        holder.addDetail("", "rootUrl",             &AquaServer::rootUrl,           "",     ZQ::common::Config::optReadOnly);
        holder.addDetail("", "homeContainer",       &AquaServer::homeContainer,     "",     ZQ::common::Config::optReadOnly);
        holder.addDetail("", "userDomain",          &AquaServer::userDomain,        "",     ZQ::common::Config::optReadOnly);
        holder.addDetail("", "connectTimeout",      &AquaServer::connectTimeout,    "5000", ZQ::common::Config::optReadOnly);
        holder.addDetail("", "timeout",             &AquaServer::timeout,           "10000",ZQ::common::Config::optReadOnly);
        holder.addDetail("", "flags",               &AquaServer::flag,              "0",     ZQ::common::Config::optReadOnly);
        holder.addDetail("", "maxThreadPoolSize",   &AquaServer::maxThreadPoolSize, "20",     ZQ::common::Config::optReadOnly);
        holder.addDetail("", "mainFileExtension",   &AquaServer::mainFileExtension, "",     ZQ::common::Config::optReadOnly);
		holder.addDetail("", "contentNameFormat",   &AquaServer::ContentNameFormat, "${PAID}_${PID}",     ZQ::common::Config::optReadOnly);
        holder.addDetail("", "bindIp",              &AquaServer::bindIp,            "",     ZQ::common::Config::optReadOnly);

    }
};
struct AquaCache
{

	int32       enable;
	int32       cacheBuffers;        
	int32       cacheBuffersForwrite;
	int32       cacheBuffersize;
	int32       cacheBuffersizeForwrite;
	int32       cacheLogFlag;

	int32       cacheFlushThreads;
	int32       cacheForceFlushInterval;
	int32       cacheReadAheadMax;
	int32       cacheReadAheadTrigger;
	int32       cacheReadAheadPowerBase;
	int32       cacheReadAheadRecognitions;
	int32      cacheWriteSegmentsMax;
	int32      cacheWriteLengthMax;
	int32      cacheWriteLengthMin;


	typedef ZQ::common::Config::Holder<AquaCache> AquaCacheHolder;

	static void structure(AquaCacheHolder &holder)
	{
		holder.addDetail("", "enable",						&AquaCache::enable,			  		"0",     ZQ::common::Config::optReadOnly);
		//holder.addDetail("", "cacheBuffers",              &AquaCache::cacheBuffers,           "2",     ZQ::common::Config::optReadOnly);
		holder.addDetail("", "cacheBuffersForwrite",        &AquaCache::cacheBuffersForwrite,    "12800",     ZQ::common::Config::optReadOnly);
		//holder.addDetail("", "cacheBuffersize",		        &AquaCache::cacheBuffersize,         "65536",     ZQ::common::Config::optReadOnly);

		holder.addDetail("", "cacheBuffersizeForwrite",		&AquaCache::cacheBuffersizeForwrite, "65536",     ZQ::common::Config::optReadOnly);

		holder.addDetail("", "cacheLogFlag",				&AquaCache::cacheLogFlag,            "10",     ZQ::common::Config::optReadOnly);
		holder.addDetail("", "cacheFlushThreads",           &AquaCache::cacheFlushThreads,        "10",     ZQ::common::Config::optReadOnly);
		holder.addDetail("", "cacheForceFlushInterval",     &AquaCache::cacheForceFlushInterval,  "1000",     ZQ::common::Config::optReadOnly);
		//holder.addDetail("", "cacheReadAheadMax",           &AquaCache::cacheReadAheadMax,         "8",     ZQ::common::Config::optReadOnly);
		//holder.addDetail("", "cacheReadAheadTrigger",       &AquaCache::cacheReadAheadTrigger,     "0",     ZQ::common::Config::optReadOnly);
		//holder.addDetail("", "cacheReadAheadPowerBase",     &AquaCache::cacheReadAheadPowerBase,    "8",     ZQ::common::Config::optReadOnly);
		//holder.addDetail("", "cacheReadAheadRecognitions",  &AquaCache::cacheReadAheadRecognitions,  "100",     ZQ::common::Config::optReadOnly);

		holder.addDetail("", "cacheWriteSegmentsMax",       &AquaCache::cacheWriteSegmentsMax,       "10",     ZQ::common::Config::optReadOnly);
		holder.addDetail("", "cacheWriteLengthMax",       &AquaCache::cacheWriteLengthMax,       "4194304",     ZQ::common::Config::optReadOnly);
		holder.addDetail("", "cacheWriteLengthMin",       &AquaCache::cacheWriteLengthMin,       "524288",     ZQ::common::Config::optReadOnly);	
	}
	AquaCache()
	{
		cacheBuffers  = 2;
		cacheBuffersize = 65536;	
		cacheReadAheadMax = 8;	
		cacheReadAheadTrigger = 0;
		cacheReadAheadPowerBase = 8;
		cacheReadAheadRecognitions  = 100;
	}
};

class CPH_AquaLibConfig
{
public:
	CPH_AquaLibConfig ();
	~CPH_AquaLibConfig ();

    static void structure(ZQ::common::Config::Holder<CPH_AquaLibConfig> &holder);

    Capture::CaptureHolder                      capture;
    CaptureDumper::CaptureDumperHolder          captureDumper;
    UnifiedTrickFile::UnifiedTrickFileHolder    unifiedtrickfile;
    CiscoFileExt::CiscoFileExtHolder            ciscofileext;
    MountPath::MountPathHolder                  mountpath;
    RetryCapture::RetryCaptureHolder            retryCapture;
    ProvisionMethod::ProvisionMethodHolder      provisionMethod;
    AquaServer::AquaServerHolder                aquaServer;
	AquaCache::AquaCacheHolder                  aquaCache;

    void readCapture(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterCapture(const std::string& full_path );

    void readCaptureDumper(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterCaptureDumper(const std::string& full_path );

	void readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterUnifiedTrickFile(const std::string& full_path );

	void readCiscoFileExt(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterCiscoFileExt(const std::string& full_path );

	void readMountPath(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterMountPath(const std::string& full_path );

    void readRetryCapture(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterRetryCapture(const std::string& full_path );

    void readProvisionMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterProvisionMethod(const std::string& full_path );

    void readAquaServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void regsiterAquaServer(const std::string& full_path );

	void readAquaCache(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void regsiterAquaCache(const std::string& full_path );


	char	szCacheDir[256];
	int32	mediaSampleSize;
	int32   maxAllocSampleCount;
	int32	bandwidthLimitRate;
	int32	enableProgEvent;
	int32   minIntervalMs;
	int32   maxPecentageStep;
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


	int32  enableCmdGetsize; 
	int32  timeoutForGrowing;
	int32  bandwidthCtrlInterval;
	char   szTargetDir[256];
	int32  enableResumeForDownload;
	char   szPaceDllPath[256];

	std::string strAugmentationPids;

	int32   deleteOnFail;
};

extern ZQ::common::Config::Loader<CPH_AquaLibConfig> _gCPHCfg;

}}
#endif

