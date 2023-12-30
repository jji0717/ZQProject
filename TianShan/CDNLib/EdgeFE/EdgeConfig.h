
#ifndef _cdn_streaming_service_config_header_file_h__
#define _cdn_streaming_service_config_header_file_h__

#include <ConfigHelper.h>
//#include <tchar.h>


namespace ZQ
{
namespace StreamService
{

struct HLSSubName2BitrateConf
{
	std::string subname;
	int32		bitrate;
	HLSSubName2BitrateConf(){
		bitrate=0;
	}

	static void structure( ZQ::common::Config::Holder<HLSSubName2BitrateConf>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","name",&HLSSubName2BitrateConf::subname,"",optReadOnly);
		holder.addDetail("","bitrate",&HLSSubName2BitrateConf::bitrate,"",optReadOnly);
	}
};


struct HLSServerConf
{
	std::string		keyfile;
	std::string		rooturl;
	std::string		homecontainer;
	std::string		serverHostUrl;
	int32			logFlag;
	int32			defaultBitrate;
	std::map<std::string,uint32> subname2bitrate;
	HLSServerConf()
	{
		logFlag = 0;
		defaultBitrate = 2048000;
	}
	void readName2Bitrate( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		ZQ::common::Config::Holder< HLSSubName2BitrateConf> conf;
		conf.read( node , hPP );
		subname2bitrate[conf.subname] = conf.bitrate;
	}
	void registerNothing(const std::string&){}

	static void structure( ZQ::common::Config::Holder<HLSServerConf>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","hostUrl",&HLSServerConf::serverHostUrl,"",optReadOnly);
		holder.addDetail("","keyfile",&HLSServerConf::keyfile,"",optReadOnly);
		holder.addDetail("","aquaRooturl",&HLSServerConf::rooturl,"",optReadOnly);
		holder.addDetail("","homecontainer",&HLSServerConf::homecontainer,"",optReadOnly);
		holder.addDetail("","logFlag",&HLSServerConf::logFlag,"",optReadOnly);
		holder.addDetail("","defaultBitrate",&HLSServerConf::defaultBitrate,"2048000",optReadOnly);
		holder.addDetail("name2bitrate",&HLSServerConf::readName2Bitrate,&HLSServerConf::registerNothing);
	}

};

struct AquaReaderConf
{
	std::string 	rootUrl;
	std::string		userDomain;
	std::string		homeContainer;
	std::string		contentNameFormat;
	std::string 	mainFileExtension;
	int32 			cdmiFlags;
	int32			ioThreadPool;
	int32			cdmiOpsThreadPool;
	int32			cacheBufferCount;
	int32           aquaReaderHlsEnableMDataQuery;
	AquaReaderConf() {
		cdmiFlags = 0x0f;
		ioThreadPool = 15;
		cdmiOpsThreadPool = 15;
		aquaReaderHlsEnableMDataQuery = 1;
	}
	static void structure( ZQ::common::Config::Holder<AquaReaderConf>& holder ) {
		using namespace ZQ::common::Config;
		holder.addDetail("", "rootUrl", &AquaReaderConf::rootUrl,"",optReadOnly);
		holder.addDetail("", "userDomain", &AquaReaderConf::userDomain,"",optReadOnly);
		holder.addDetail("", "homeContainer", &AquaReaderConf::homeContainer,"", optReadOnly);
		holder.addDetail("", "flags", &AquaReaderConf::cdmiFlags,"15", optReadOnly);
		holder.addDetail("", "ioThreadPool", &AquaReaderConf::ioThreadPool,"15", optReadOnly);
		holder.addDetail("", "cdmiOpsThreadPool", &AquaReaderConf::cdmiOpsThreadPool,"15", optReadOnly);
		holder.addDetail("", "bufferCount", &AquaReaderConf::cacheBufferCount,"1000", optReadOnly);
		holder.addDetail("", "aquaReaderHlsEnableMDataQuery", &AquaReaderConf::aquaReaderHlsEnableMDataQuery, "1", optReadOnly);
		holder.addDetail("", "contentNameFormat",&AquaReaderConf::contentNameFormat, "${PAID}_${PID}",optReadOnly);
		holder.addDetail("", "mainFileExtension",&AquaReaderConf::mainFileExtension, ".0X0000", optReadOnly);
	}
};

struct StreamerReplica
{
	std::string		listenerEndpoint;
	int32			defaultUpdateInterval;
	std::string		groupId;
	std::string		category;
	std::string		csNetId;//CDNCS's netid
	static void structure( ZQ::common::Config::Holder<StreamerReplica>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("subscriber","endpoint",&StreamerReplica::listenerEndpoint,"",optReadOnly);
		holder.addDetail("subscriber","updateInterval",&StreamerReplica::defaultUpdateInterval,"",optReadOnly);
		holder.addDetail("","groupId",&StreamerReplica::groupId,"",optReadOnly);
		holder.addDetail("","category",&StreamerReplica::category,"",optReadOnly);
		holder.addDetail("","csNetId",&StreamerReplica::csNetId,"",optReadOnly);
	}
};

struct C2StreamerPortSpeedConf
{
	std::string		portName;
	int64			speed;//KBPS
	int32			confSpeed;
	int32			natPortBase;
	int32			natPortCount;
	std::string		natIp;

	static void structure( ZQ::common::Config::Holder<C2StreamerPortSpeedConf>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","name", &C2StreamerPortSpeedConf::portName , "",optReadOnly);
		holder.addDetail("","speed",&C2StreamerPortSpeedConf::confSpeed ,"",optReadOnly);
		holder.addDetail("","natPortBase",&C2StreamerPortSpeedConf::natPortBase,"0",optReadOnly);
		holder.addDetail("","natPortCount",&C2StreamerPortSpeedConf::natPortCount,"0",optReadOnly);
	}
};

struct CacheDir
{
	std::string      homePath;
	int32      		 totalSize; //size of this dir can cached, MB
	int32            LRUSize;
	int32            readThreadCount; // count of read thread
	int32            writeThreadCount; // cout of write thread
	int32            pendingsYield;  // pendings
	int32			 maxWriteMBps;
	CacheDir()
	{
		homePath = "";
		totalSize = 0;
		LRUSize = 200;
		readThreadCount = 2;
		writeThreadCount = 2;
		pendingsYield    = 2;
		maxWriteMBps = 20;
	}

	static void structure( ZQ::common::Config::Holder<CacheDir>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("", "homePath" , &CacheDir::homePath, "", optReadOnly);
		holder.addDetail("", "totalSize", &CacheDir::totalSize, "0", optReadOnly);
		holder.addDetail("", "LRUSize", &CacheDir::LRUSize, "200", optReadOnly);
		holder.addDetail("", "readThreads", &CacheDir::readThreadCount, "2", optReadOnly);
		holder.addDetail("", "writeThreads", &CacheDir::writeThreadCount, "2", optReadOnly);
		holder.addDetail("", "pendingsYield", &CacheDir::pendingsYield, "2", optReadOnly);
		holder.addDetail("", "maxWriteMBps", &CacheDir::maxWriteMBps, "20", optReadOnly);
	}
};

struct DiskCache
{
	int32                   diskCacheEnabled;
	int32                   cacheLoaderTimeout;
	std::string             cacheIgnoreFiles;
	
	typedef ZQ::common::Config::Holder<CacheDir> CACHEDIR;
	std::vector<CACHEDIR> cacheDir;
	DiskCache()
	{
		diskCacheEnabled = 0;
		cacheLoaderTimeout = 3000;
		cacheIgnoreFiles = "";
	}

	void readCacheDir( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		ZQ::common::Config::Holder<CacheDir> cachedirconf;
		cachedirconf.read(node, hPP);
		cacheDir.push_back(cachedirconf);
	}

	void registerNothing(const std::string&)
	{
	}

	static void structure( ZQ::common::Config::Holder<DiskCache>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","diskCacheEnabled", &DiskCache::diskCacheEnabled, "0", optReadOnly);
		holder.addDetail("", "loaderTimeout", &DiskCache::cacheLoaderTimeout, "3000", optReadOnly);
		holder.addDetail("", "ignoreFiles", &DiskCache::cacheIgnoreFiles, "", optReadOnly);
		holder.addDetail("CacheDir", &DiskCache::readCacheDir, &DiskCache::registerNothing);
	}			

};


struct C2StreamerWriter
{
	std::string tag;
	int32		socketPacketSize;//this is changed to MTU, it's application's resposibility to calculate the max payload size in a IP packaet
	int32		packetsPerSend;
	int32		socketWriteMonitorThreshold;
	int32		timerMonitorThreshold;
	int32		threadpoolwarningThreshold;
	int32		fsReadThreshold;
	int32		maxRoundPerRun;
	int32		minYieldInterval;
	int32		socketSndBuf;
	int32		socketSndTimeo;
	int32		useTcpCork;
	int32		firstChunkWait;
	int32       bitrateInflation;
	int32 		maxIdleCountAtEnd;
	int32 		quitPerConnectionLost;
	int32 		defaultBitrate;
	int32		udpSessMinStreamInterval;


	C2StreamerWriter()
	{
		udpSessMinStreamInterval	= 50;
		socketWriteMonitorThreshold = 50;
		timerMonitorThreshold		= 50;
		fsReadThreshold				= 50;
		socketPacketSize			= 1460;
		packetsPerSend				= 100;
		maxRoundPerRun				= 3;
		minYieldInterval			= 5;
		threadpoolwarningThreshold	= 50;
		socketSndBuf				= 1024*1024;
		socketSndTimeo				= 20 * 1000;
		firstChunkWait				= 150;
		bitrateInflation            = 100;
		maxIdleCountAtEnd			= 0;
		quitPerConnectionLost		= 0;
		defaultBitrate 				= 3750 * 1000 * 10;
	
	}

	static void structure( ZQ::common::Config::Holder<C2StreamerWriter>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("", "writerTag", &C2StreamerWriter::tag,NULL,optReadOnly);
 		holder.addDetail("", "socketThreshHold", &C2StreamerWriter::socketWriteMonitorThreshold,"10",optReadOnly );
 		holder.addDetail("", "timerThreshold", &C2StreamerWriter::timerMonitorThreshold,"30",optReadOnly);
 		holder.addDetail("", "fsReadThreshold", &C2StreamerWriter::fsReadThreshold,"10",optReadOnly);
		holder.addDetail("", "packetPayload", &C2StreamerWriter::socketPacketSize,"1460",optReadOnly);
		holder.addDetail("", "packetsPerSend", &C2StreamerWriter::packetsPerSend,"100",optReadOnly);
		holder.addDetail("", "maxRoundPerRun", &C2StreamerWriter::maxRoundPerRun,"3",optReadOnly);
		holder.addDetail("", "udpSessMinStreamInterval", &C2StreamerWriter::udpSessMinStreamInterval,"50",optReadOnly);
		holder.addDetail("", "minYieldInterval", &C2StreamerWriter::minYieldInterval,"10",optReadOnly);
		holder.addDetail("", "threadpoolWarningSize", &C2StreamerWriter::threadpoolwarningThreshold,"100",optReadOnly);
		holder.addDetail("", "socketKernelSendBuf", &C2StreamerWriter::socketSndBuf,"1048576",optReadOnly);
		holder.addDetail("", "socketkernelSndTimeout", &C2StreamerWriter::socketSndTimeo,"20000",optReadOnly);
		//holder.addDetail("","useTcpCork", &C2StreamerWriter::useTcpCork,"1",optReadOnly);
		holder.addDetail("", "firstChunkWait", &C2StreamerWriter::firstChunkWait,"100",optReadOnly);
		holder.addDetail("", "bitrateInflation", &C2StreamerWriter::bitrateInflation, "0", optReadOnly);
		holder.addDetail( "" , "maxIdleAtEnd" , &C2StreamerWriter::maxIdleCountAtEnd ,"0" , optReadOnly );
		holder.addDetail( "" , "quitPerConnectionLost" ,&C2StreamerWriter::quitPerConnectionLost,"1",optReadOnly);
		holder.addDetail( "" , "defaultBitrate", &C2StreamerWriter::defaultBitrate, "37500000", optReadOnly);

	}
};

struct C2StreamerC2Client
{
	    std::string              C2ClientUpStreamIP;
		std::string              C2ClientURL;
		std::string              C2ClientTransfer;
		std::string              C2ClientHttpCRGAddr;
		std::string 			 C2ClientTransferRate;
		std::string 			 C2ClientIngressCapacity;
		std::string  			 C2ClientExclusionList;
		std::string  			 C2ClientTransferDelay;
		int32                    C2ClientHttpCRGPort;
		int32                    C2ClientDefaultGetPort;
		int32					 C2ClientWaitBufferTime;
		int32					 C2ClientIndexTimeout;
		int32					 C2ClientIndexRetryTimes;
		int32					 C2ClientMainfileTimeout;
		int32					 C2ClientMainfileRetryTimes;
		int32					 C2ClientEnableTransferDelete;
		int32					 C2ClientAlignment;	// unit is a KByte
		int32					 C2ClientMaxBufferCountPerRequest;
		int32					 C2ClientMinTransferRate;
		int32 					 C2ClientBitrateInflate;
		int32 					 C2ClientMaxBitrate;
		C2StreamerC2Client()
		{
		    C2ClientUpStreamIP="";
			C2ClientURL = "";
			C2ClientTransfer = "";
			C2ClientHttpCRGAddr = "";
			C2ClientTransferRate = "3750000";
			C2ClientIngressCapacity = "9000000000000";
			C2ClientExclusionList = "";
			C2ClientTransferDelay = "-2000";
			C2ClientHttpCRGPort = 10080;
			C2ClientDefaultGetPort = 12000;
			C2ClientWaitBufferTime = 10000;
			C2ClientIndexTimeout	= 5*1000;
			C2ClientIndexRetryTimes = 5;
			C2ClientMainfileTimeout = 15*1000;
			C2ClientMainfileRetryTimes = 5;
			C2ClientEnableTransferDelete = 0;
			C2ClientAlignment = 8;
			C2ClientMaxBufferCountPerRequest = 1;
			C2ClientMinTransferRate = 3750000;
			C2ClientBitrateInflate = 120;
			C2ClientMaxBitrate = 0;
		}
		static void structure( ZQ::common::Config::Holder<C2StreamerC2Client>& holder )
		{
			using namespace ZQ::common::Config;
			holder.addDetail("","UpStreamIP", &C2StreamerC2Client::C2ClientUpStreamIP,"",optReadOnly );
			holder.addDetail("","url", &C2StreamerC2Client::C2ClientURL,"",optReadOnly );
			holder.addDetail("","clientTransfer", &C2StreamerC2Client::C2ClientTransfer,"",optReadOnly );
			holder.addDetail("","httpCRGAddr", &C2StreamerC2Client::C2ClientHttpCRGAddr,"",optReadOnly );
			holder.addDetail("","httpCRGPort", &C2StreamerC2Client::C2ClientHttpCRGPort,"10080",optReadOnly );
			holder.addDetail("","defaultGetPort", &C2StreamerC2Client::C2ClientDefaultGetPort,"12000",optReadOnly );
			holder.addDetail("","transferRate", &C2StreamerC2Client::C2ClientTransferRate,"3750000",optReadOnly );
			holder.addDetail("","ingressCapacity", &C2StreamerC2Client::C2ClientIngressCapacity,"16512000000",optReadOnly );
			holder.addDetail("","exclusionList", &C2StreamerC2Client::C2ClientExclusionList,"",optReadOnly );
			holder.addDetail("","transferDelay", &C2StreamerC2Client::C2ClientTransferDelay,"-2000",optReadOnly );
			holder.addDetail("","waitBufferTime", &C2StreamerC2Client::C2ClientWaitBufferTime,"10000",optReadOnly );
			holder.addDetail("","indexTimeout", &C2StreamerC2Client::C2ClientIndexTimeout,"5000",optReadOnly );
			holder.addDetail("","indexRetry", &C2StreamerC2Client::C2ClientIndexRetryTimes,"5",optReadOnly );
			holder.addDetail("","mainfileTimeout", &C2StreamerC2Client::C2ClientMainfileTimeout,"15000",optReadOnly );
			holder.addDetail("","mainfileRetry", &C2StreamerC2Client::C2ClientMainfileRetryTimes,"5",optReadOnly );
			holder.addDetail("","enableTransferDelete", &C2StreamerC2Client::C2ClientEnableTransferDelete,"0",optReadOnly );
			holder.addDetail("","alignment", &C2StreamerC2Client::C2ClientAlignment,"8",optReadOnly );
			holder.addDetail("","maxBufferCountPerRequest", &C2StreamerC2Client::C2ClientMaxBufferCountPerRequest,"1",optReadOnly );
			holder.addDetail("","minTransferRate", &C2StreamerC2Client::C2ClientMinTransferRate, "3750000", optReadOnly);
			holder.addDetail("","bitrateInflate", &C2StreamerC2Client::C2ClientBitrateInflate, "120", optReadOnly);
			holder.addDetail("","maxRequestBitrate", &C2StreamerC2Client::C2ClientMaxBitrate, "0", optReadOnly);
		}
};

struct  C2StreamerHttpClient{
	  std::string          httpProxyURL;
	  std::string          segmentURL;
	  int32                 httpTimeOut;
	  int32                httpRetry;
	  C2StreamerHttpClient()
	  {
			httpProxyURL = "";
			segmentURL = "";
			httpTimeOut = 10000;
	  }
	  static void structure( ZQ::common::Config::Holder<C2StreamerHttpClient>& holder )
	  {
			using namespace ZQ::common::Config;
			holder.addDetail("","ProxyUrl", &C2StreamerHttpClient::httpProxyURL,"",optReadOnly );
			holder.addDetail("","SegmentUrl", &C2StreamerHttpClient::segmentURL,"",optReadOnly );
			holder.addDetail("","HttpTimeOut", &C2StreamerHttpClient::httpTimeOut,"10000",optReadOnly );
			holder.addDetail("","HttpRetry", &C2StreamerHttpClient::httpRetry,"0",optReadOnly );
	 }
};

struct CacheCenterConfig
{
	int32		defaultTimeOut;
	int32		fioErrTimeOut;
	int32		httpErrTimeOut;
	int32		socketErrTimeOut;
	int32		otherErrTimeOut;
	int32		cacheBufferSize;
	int32		cacheBufferCount;
	int32		cacheReadAheadCount;
	int32		useBufferIo;
	int32		indexRecordCacheSize;
	int32		attrTimeout;
	int32 		enableCRCcheck;

	  CacheCenterConfig()
	  {
		  defaultTimeOut = 24 * 3600 * 1000;
		  fioErrTimeOut = 10 * 60 *1000;
		  httpErrTimeOut = 1 * 60 * 1000;
		  socketErrTimeOut = 2 * 1000;
		  otherErrTimeOut = 5 * 1000;
		  cacheBufferSize = 2 * 1024 * 1024;
		  cacheBufferCount = 100;
		  cacheReadAheadCount = 2;
		  useBufferIo = 0;
		  indexRecordCacheSize = 4096;
		  attrTimeout = 5000;
		  enableCRCcheck = 0;
	  }

	  static void structure( ZQ::common::Config::Holder<CacheCenterConfig>& holder )
	  {
			using namespace ZQ::common::Config;
			holder.addDetail("","DefaultTimeout", &CacheCenterConfig::defaultTimeOut,"86400000",optReadOnly );
			holder.addDetail("","FioErrTimeOut", &CacheCenterConfig::fioErrTimeOut,"600000",optReadOnly );
			holder.addDetail("","HttpErrTimeOut", &CacheCenterConfig::httpErrTimeOut,"60000",optReadOnly );
			holder.addDetail("","SocketErrTimeOut", &CacheCenterConfig::socketErrTimeOut,"2000",optReadOnly );
			holder.addDetail("","OtherErrTimeout", &CacheCenterConfig::otherErrTimeOut,"5000",optReadOnly );
			holder.addDetail("","cacheBufferSize",&CacheCenterConfig::cacheBufferSize,"2097152",optReadOnly);
			holder.addDetail("","processBuffers",&CacheCenterConfig::cacheBufferCount,"100",optReadOnly);
			holder.addDetail("","cacheReadAheadCount",&CacheCenterConfig::cacheReadAheadCount,"2",optReadOnly);
			holder.addDetail("","useBufferIo",&CacheCenterConfig::useBufferIo,"0",optReadOnly);
			holder.addDetail("","indexRecordCacheSize",&CacheCenterConfig::indexRecordCacheSize,"4096",optReadOnly);
			holder.addDetail("","attrTimeout", &CacheCenterConfig::attrTimeout,"5000",optReadOnly);
			holder.addDetail("","enableCRC", &CacheCenterConfig::enableCRCcheck,"0",optReadOnly);
	  }
};


struct C2StreamerUrlRule {
	std::string urlPrefix;
	std::string writerTag;
	std::string readerTag;
	std::string fsPath;

	static void structure( ZQ::common::Config::Holder<C2StreamerUrlRule>& holder ) {
		using namespace ZQ::common::Config;
		holder.addDetail("", "prefix", &C2StreamerUrlRule::urlPrefix, NULL, optReadOnly);
		holder.addDetail("", "writer", &C2StreamerUrlRule::writerTag, NULL, optReadOnly);
		holder.addDetail("", "reader", &C2StreamerUrlRule::readerTag, NULL, optReadOnly);
		holder.addDetail("", "fsPath", &C2StreamerUrlRule::fsPath, "", optReadOnly);
	}
};



struct C2StreamerSvcConfig
{
	//doc path for c2streamer should be the same as ContentStore configuration
	std::string				httpBindIp;
	std::string				httpBindPort;
	int32					serviceThreadCount;
	int32					locateThreadCount;
	int32					locateMaxPending;
	std::string				cpuCores;
	int32 					defaultReaderType;
	int32					enableC2ClientHybrid;
	int32					c2HybridMaxHitCount;
	int32 					c2HybridMaxTimeDuration;
#ifdef ZQ_CDN_UMG
    std::string statusReportSvrIp;
    std::string statusReportSvrPort;
    int32 statusReportSvrThreadCount;
#endif

	typedef ZQ::common::Config::Holder<C2StreamerWriter> C2StreamerWriterHolder;

	std::map<std::string,C2StreamerWriterHolder>		writers;
	ZQ::common::Config::Holder<C2StreamerC2Client> c2client;
	ZQ::common::Config::Holder<C2StreamerHttpClient> httpclient;
	ZQ::common::Config::Holder<CacheCenterConfig> c2CacheCenter;
	ZQ::common::Config::Holder<AquaReaderConf>				aquaReaderConfig;
	typedef ZQ::common::Config::Holder<C2StreamerPortSpeedConf> PORTCONF;
	std::vector<PORTCONF> portConf;
	ZQ::common::Config::Holder<DiskCache>       diskcache;
	typedef ZQ::common::Config::Holder<C2StreamerUrlRule> C2StreamerUrlRuleHolder;
	std::vector<C2StreamerUrlRuleHolder>				urlRules;

	void readUrlRulesConfig( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP ) {
		C2StreamerUrlRuleHolder rule;
		rule.read(node, hPP);
		urlRules.push_back(rule);
	}

	void readAquaReaderConfig( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		aquaReaderConfig.read(node,hPP);
	}

	void readPortSpeed( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		ZQ::common::Config::Holder<C2StreamerPortSpeedConf> speedConf;
		speedConf.read(node,hPP);
		speedConf.speed = ((int64)speedConf.confSpeed) * 1000;
		portConf.push_back(speedConf);
	}

	void readwriterConf( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		C2StreamerWriterHolder writer;
		writer.read( node, hPP );
		writers[writer.tag] = writer;
	}
	void readC2Client(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hpp)
	{
		c2client.read( node, hpp);
	}

	void readDiskCache(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hpp)
	{
		diskcache.read(node, hpp);
	}

	void readHttpClient(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hpp)
	{
		httpclient.read( node, hpp);
	}

	void readCacheCenter(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hpp)
	{
		c2CacheCenter.read(node, hpp);
	}
	void registerNothing(const std::string&){}
	static void structure( ZQ::common::Config::Holder<C2StreamerSvcConfig>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","HttpBindIp", &C2StreamerSvcConfig::httpBindIp,"0.0.0.0",optReadOnly );
		holder.addDetail("","httpBindPort", &C2StreamerSvcConfig::httpBindPort, "5150", optReadOnly );
		holder.addDetail("","servicethreadCount",&C2StreamerSvcConfig::serviceThreadCount,"30",optReadOnly);
		holder.addDetail("","locatethreadCount",&C2StreamerSvcConfig::locateThreadCount,"20",optReadOnly);
		holder.addDetail("","locateMaxPending",&C2StreamerSvcConfig::locateMaxPending,"80",optReadOnly);
		holder.addDetail("","cpuCores", &C2StreamerSvcConfig::cpuCores,"",optReadOnly);
		//holder.addDetail("", "diskCacheEnabled", &C2StreamerSvcConfig::diskCacheEnabled, "0", optReadOnly);
		holder.addDetail("", "assetIndexDefaultReaderType", &C2StreamerSvcConfig::defaultReaderType, "2", optReadOnly );
		holder.addDetail("", "enableC2ClientHybrid", &C2StreamerSvcConfig::enableC2ClientHybrid, "1", optReadOnly);	
		holder.addDetail("", "c2HybridMaxHitCount", &C2StreamerSvcConfig::c2HybridMaxHitCount, "10", optReadOnly);
		holder.addDetail("", "c2HybridMaxTimeDuration", &C2StreamerSvcConfig::c2HybridMaxTimeDuration, "10000", optReadOnly);
#ifdef ZQ_CDN_UMG
        holder.addDetail("StatusReportServer","bindIp",&C2StreamerSvcConfig::statusReportSvrIp);
        holder.addDetail("StatusReportServer","bindPort",&C2StreamerSvcConfig::statusReportSvrPort);
        holder.addDetail("StatusReportServer","threadCount",&C2StreamerSvcConfig::statusReportSvrThreadCount);
#endif
		holder.addDetail("StreamingPort",&C2StreamerSvcConfig::readPortSpeed, &C2StreamerSvcConfig::registerNothing);
		holder.addDetail("DiskCache", &C2StreamerSvcConfig::readDiskCache, &C2StreamerSvcConfig::registerNothing);
		
		holder.addDetail("Writers/writer",&C2StreamerSvcConfig::readwriterConf, &C2StreamerSvcConfig::registerNothing);
		holder.addDetail("C2Client",&C2StreamerSvcConfig::readC2Client, &C2StreamerSvcConfig::registerNothing);
		holder.addDetail("CacheCenter",&C2StreamerSvcConfig::readCacheCenter, &C2StreamerSvcConfig::registerNothing);
		holder.addDetail("SegmentFetcher",&C2StreamerSvcConfig::readHttpClient, &C2StreamerSvcConfig::registerNothing);
		holder.addDetail("AquaReader",&C2StreamerSvcConfig::readAquaReaderConfig,&C2StreamerSvcConfig::registerNothing);
		holder.addDetail("UrlRules/urlRule", &C2StreamerSvcConfig::readUrlRulesConfig, &C2StreamerSvcConfig::registerNothing);
	}
};

struct MonitoredLog
{
	std::string name;
	std::string syntax;
	std::string type;
	std::string	key;
	static void structure( ZQ::common::Config::Holder<MonitoredLog> &holder)
	{
		holder.addDetail("", "path", &MonitoredLog::name,"");
		holder.addDetail("", "syntax", &MonitoredLog::syntax,"");
		holder.addDetail("","key",&MonitoredLog::key,"");
		holder.addDetail("","type",&MonitoredLog::type,"");
	}
};

struct VolumeConfig
{
	std::string name;
	std::string path;
	std::string fstype;
	int32 isDefault;

	static void structure(ZQ::common::Config::Holder<VolumeConfig>& holder)
	{
		holder.addDetail("", "name", &VolumeConfig::name);
		holder.addDetail("", "path", &VolumeConfig::path);
		holder.addDetail("", "fstype", &VolumeConfig::fstype,"enfs");
		holder.addDetail("", "default", &VolumeConfig::isDefault, "0");
	}
};

struct CacheProvider
{
	std::string		name;
	std::string		sessInterface;
	static void structure( ZQ::common::Config::Holder<CacheProvider>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","name",&CacheProvider::name,"",optReadOnly);
		holder.addDetail("","sessionInterface",&CacheProvider::sessInterface,"",optReadOnly);
	}
};

struct CacheStoreConfig
{
	int32	enable;
	int32	heartbeatInterval;

	int32 	proxySessionBook;

	int32	maxCandidates;
	int32	pwrRanking;
	int32	prevLoadThreshold;
	int32	successorLoadThreshold;
	int32	maxUnpopular;

	int32	popTimeWindow;
	int32	countOfPopular;
	int32	hotWindow;

	int32	forwardFailPenalty;
	int32	penaltyMax;

	std::string upStreamBindip;
	int32	defaultProvSessBitrate;
	int32	defaultTransferServerPort;
	int32	minBitratePercent;
	int32	totalProvBitrate;

	std::string	downStreamBindIp;

	int32	paidLength;
	std::string defaultSessInterface;
	std::string	thisSessInterface;

	int32	flags;

	std::string groupAddr;

	std::string downloadIndexTmpFilePath;
	std::string forwardURL;


	CacheStoreConfig()
	{
		paidLength = 20;
		defaultProvSessBitrate = 4000;
		defaultTransferServerPort = 12000;
		minBitratePercent = 0;
		totalProvBitrate = 1000000;
		enable = 1;
		heartbeatInterval = 10000;
		maxCandidates = 3;
		pwrRanking = 1100;
		prevLoadThreshold = 8000;
		successorLoadThreshold = 2000;
		maxUnpopular = 100;
		popTimeWindow = 300000;
		countOfPopular = 4;
		hotWindow = 30000;
		forwardFailPenalty = 20;
		penaltyMax = 60;
		flags  = 0;
		forwardURL = "";
		proxySessionBook = 1;
	}

	typedef ZQ::common::Config::Holder<CacheProvider> CacheProviderHolder;

	std::vector<CacheProviderHolder>  cacheProviders;

	void registerNothing(const std::string&)
	{
	}
	void readCacheProvider( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		CacheProviderHolder holder;
		holder.read(node,hPP);
		if( !(holder.name.empty() && holder.sessInterface.empty() ))
			cacheProviders.push_back( holder );
	}


	static void structure( ZQ::common::Config::Holder<CacheStoreConfig>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","enable",&CacheStoreConfig::enable,"0");
		holder.addDetail("", "proxySessionBook", &CacheStoreConfig::proxySessionBook, "1", optReadOnly);
		holder.addDetail("","heartbeatInterval",&CacheStoreConfig::heartbeatInterval,"10000",optReadOnly);
		holder.addDetail("","flags",&CacheStoreConfig::flags,"0",optReadOnly);
		holder.addDetail("","groupAddr", &CacheStoreConfig::groupAddr, "225.23.23.23", optReadOnly);
		holder.addDetail("","tempFilePath",&CacheStoreConfig::downloadIndexTmpFilePath,"",optReadOnly);
		holder.addDetail("","sessionInterface",&CacheStoreConfig::thisSessInterface,"",optReadOnly);
		holder.addDetail("","forwardURL",&CacheStoreConfig::forwardURL,"",optReadOnly);

		holder.addDetail("CacheFactor","maxCandidates",&CacheStoreConfig::maxCandidates,"3",optReadOnly);

		holder.addDetail("CacheFactor","pwrRanking",&CacheStoreConfig::pwrRanking,"1100",optReadOnly);
		holder.addDetail("CacheFactor","prevLoadThreshold",&CacheStoreConfig::prevLoadThreshold,"8000",optReadOnly);
		holder.addDetail("CacheFactor","successorLoadThreshold",&CacheStoreConfig::successorLoadThreshold,"2000",optReadOnly);
		holder.addDetail("CacheFactor","maxUnpopular",&CacheStoreConfig::maxUnpopular,"100",optReadOnly);

		holder.addDetail("ContentPopularity","timeWindow",&CacheStoreConfig::popTimeWindow,"300000",optReadOnly);
		holder.addDetail("ContentPopularity","countOfPopular",&CacheStoreConfig::countOfPopular,"4",optReadOnly);
		holder.addDetail("ContentPopularity","hotWindow",&CacheStoreConfig::hotWindow,"30000",optReadOnly);

		holder.addDetail("CachePenalty","forwardFail",&CacheStoreConfig::forwardFailPenalty,"20",optReadOnly);
		holder.addDetail("CachePenalty","max",&CacheStoreConfig::penaltyMax,"60",optReadOnly);

		holder.addDetail("UpStream","bindIP",&CacheStoreConfig::upStreamBindip,"",optReadOnly);
		holder.addDetail("UpStream","defaultProvSessionBitrateKbps",&CacheStoreConfig::defaultProvSessBitrate,"4000",optReadOnly);
		holder.addDetail("UpStream","defaultTransferServerPort",&CacheStoreConfig::defaultTransferServerPort,"12000",optReadOnly);
		holder.addDetail("UpStream","minBitratePercent",&CacheStoreConfig::minBitratePercent,"0",optReadOnly);
		holder.addDetail("UpStream","totalProvisionBWKbps",&CacheStoreConfig::totalProvBitrate,"1000000",optReadOnly);

		holder.addDetail("DownStream","bindIP",&CacheStoreConfig::downStreamBindIp,"",optReadOnly);

		holder.addDetail("ContentDomains","paidLength",&CacheStoreConfig::paidLength,"20",optReadOnly);
		holder.addDetail("ContentDomains","defaultSessionInterface",&CacheStoreConfig::defaultSessInterface,"",optReadOnly);
		holder.addDetail("ContentDomains/Provider",&CacheStoreConfig::readCacheProvider,&CacheStoreConfig::registerNothing);
	}
};

struct EdgeFEConfig
{
	std::string				eventChannelEndpoint;	//event channel endpoint , can be empty

	std::string				crashDumpPath;			//crash dump path , can not be empty if crash dump is enabled
	int32					enableCrashDump;		// >= 1 means enable crash dump

	int32					enableIceTrace;			// >= 1 means enable ice trace
	int32					iceTraceLevel;			// 7 -> debug
	int32					iceTraceLogSize;

	std::map<std::string , std::string >	iceProperties;

	std::string				dbPath;					//database path

	std::string				netId;					//service's net id

	std::string				serviceEndpoint;		//service endpoint , no servant name is re

	std::string				dispatchThreadCount;
	std::string				dispatchThreadCountMax;
	std::string             sesslogfilename;
	std::string             sesslogfiledirectory;
	int32                   maxsesslognum;
	int32                   sesslogmgrloopInterval;
	int32                   sesslogmgrsessExpire;
	int32                   sessStatReportInterval;
	int32                   stateSwitchInterval;
	int32					LastFramePos; 

	int32					mainThreadCount;		//count of thread in main thread pool
	int32 					mainThreadMaxPending;

	std::vector<MonitoredLog>	monitoredLogs;

	int32					sessionTimeout;			//session timeout interval in milliseconds
	int32                   perRequestedInterval;   
	int32					sessionScanInterval;
	int32					resourceScanInterval;
	int32					ticketRenewInterval;
	int32					ticketRenewThreadpoolSize;

	ZQ::common::Config::Holder<StreamerReplica>		streamerReplicaConfig;


	int32					perfSessionCacheSize;	//performance tune , evictor size
	std::string				perfCheckpointPeriod;
	std::string				perfDbRecoverFatal;
	std::string				perfSavePeriod;
	std::string				perfSaveSizeTrigger;
	int32					perfUseMemoryDB;


	int32					sessionLogFileSize;
	int32					sessionLogLevel;
	int32					sessionLogBufferSize;
	int32					sessionLogTimeout;


    // <ContentStore>
	int32 					isCacheMode;
	int32 					cacheLevel;
	int32					isAutoSync;

	int32					mainLogFileSize;
	int32					mainLogLevel;
	int32					mainLogBufferSize;
	int32					mainLogTimeout;

	int32					eventLogFileSize;
	int32					eventLogLevel;
	int32					eventLogBufferSize;
	int32					eventLogTimeout;

	int32 					contentEvictorSize;
	int32 					volumeEvictorSize;

	int32					csTimeoutIdleProvisioning;
	int32					csTimeoutNotProvisioned;
	int32					csMinimalMainfileSize;
	int32 					defaultProvisionBW;
	std::string				strTrickSpeeds;
	std::vector<float> 		trickSpeedCollection;

    std::string 			cpcEndpoint;
    int32 					cpcRegisterInterval;
	std::string             cpcDefaultIndexType;

	std::string				csStrReplicaGroupId;
//	std::string				csStrReplicaId;
	int32					csReplicaPriority;
	int32					csReplicaTimeout;		//in milisecond
    std::string        		csMasterEndpoint;
	int32					csEnableEventMulticast;
	std::string				csEventMulticastGroupIp;
	int32					csEventMulticastGroupPort;
	std::string				csEventMulticastLocalIp;
	int32					csNeMappingSendInterval;
	int32					csEventShowAll;
	int32					csThreadPoolSize;

	int32					csVolCheckTimeout;


	typedef std::vector< ZQ::common::Config::Holder<VolumeConfig> > Volumes;
	Volumes volumes;

	ZQ::common::Config::Holder<C2StreamerSvcConfig> 		c2StreamerConfig;
	ZQ::common::Config::Holder<CacheStoreConfig>			cacheStoreConfig;
	ZQ::common::Config::Holder<HLSServerConf>				hlsServerConfig;


	void registerNothing(const std::string&)
	{
	}
	void readHLSServerConfig( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		hlsServerConfig.read(node,hPP);
	}
	void readCacheStoreConfig( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		cacheStoreConfig.read(node,hPP);
	}
	void readC2StreamerConf( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		c2StreamerConfig.read( node , hPP );
	}
	void readIceProperty( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<NVPair> propHolder;
		propHolder.read(node, hPP);
		iceProperties[propHolder.name] = propHolder.value;
	}
	void readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
	{
		using namespace ZQ::common::Config;
		Holder<MonitoredLog> lmHolder;
		lmHolder.read(node, hPP);
		monitoredLogs.push_back(lmHolder);
	}
	void readStreamerReplicaConfig(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		streamerReplicaConfig.read(node,hPP);
	}

    void readVolumes(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
		using namespace ZQ::common::Config;
        Holder<VolumeConfig> volumeHolder;
        volumeHolder.read(node, hPP);

        volumes.push_back(volumeHolder);
    }

	static void structure( ZQ::common::Config::Holder<EdgeFEConfig>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("default/EventChannel","endpoint",&EdgeFEConfig::eventChannelEndpoint,"",optReadOnly);
		holder.addDetail("default/CrashDump","path",&EdgeFEConfig::crashDumpPath,"",optReadOnly);
		holder.addDetail("default/CrashDump","enabled",&EdgeFEConfig::enableCrashDump,"0",optReadOnly);
		holder.addDetail("default/IceTrace","enabled",&EdgeFEConfig::enableIceTrace,"0",optReadOnly);
		holder.addDetail("default/IceTrace","level",&EdgeFEConfig::iceTraceLevel,"7",optReadOnly);
		holder.addDetail("default/IceTrace","size",&EdgeFEConfig::iceTraceLogSize,"10240000",optReadOnly);
		holder.addDetail("default/IceProperties/serviceProperty",&EdgeFEConfig::readIceProperty,&EdgeFEConfig::registerNothing);
		holder.addDetail("default/Database","path",&EdgeFEConfig::dbPath,"",optReadOnly);

		//C2StreamerSvcConfig

		//service
		holder.addDetail("EdgeFE","netId",&EdgeFEConfig::netId,"",optReadOnly);
		holder.addDetail("EdgeFE/Bind","endpoint",&EdgeFEConfig::serviceEndpoint,"",optReadOnly);
		holder.addDetail("EdgeFE/Bind","dispatchSize",&EdgeFEConfig::dispatchThreadCount,"5",optReadOnly);
		holder.addDetail("EdgeFE/Bind","dispatchMax",&EdgeFEConfig::dispatchThreadCountMax,"10",optReadOnly);
		holder.addDetail("EdgeFE/RequestProcess","threads",&EdgeFEConfig::mainThreadCount,"10",optReadOnly);
		holder.addDetail("EdgeFE/RequestProcess","maxPending",&EdgeFEConfig::mainThreadMaxPending,"20",optReadOnly);

		holder.addDetail("EdgeFE/C2Streamer",&EdgeFEConfig::readC2StreamerConf,&EdgeFEConfig::registerNothing);
		holder.addDetail("EdgeFE/HLSServer",&EdgeFEConfig::readHLSServerConfig,&EdgeFEConfig::registerNothing);

		//publish log
		holder.addDetail("EdgeFE/PublishedLogs/Log",&EdgeFEConfig::readMonitoredLog,&EdgeFEConfig::registerNothing);

		//session
		holder.addDetail("EdgeFE/StreamSession","timeout",&EdgeFEConfig::sessionTimeout,"600000",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","perRequestInterval",&EdgeFEConfig::perRequestedInterval,"1000",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","sesslogfilename",&EdgeFEConfig::sesslogfilename,"sess",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","sesslogfiledirectory",&EdgeFEConfig::sesslogfiledirectory,"/opt/TianShan/logs",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","maxsesslogfilenum",&EdgeFEConfig::maxsesslognum,"5",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","sesslogmgrloopInterval",&EdgeFEConfig::sesslogmgrloopInterval,"3",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","sesslogmgrsessExpire",&EdgeFEConfig::sesslogmgrsessExpire,"3000",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","sessStatReportInterval",&EdgeFEConfig::sessStatReportInterval,"2000",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","stateSwitchInterval",&EdgeFEConfig::stateSwitchInterval,"1000",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","LastFramePos",&EdgeFEConfig::LastFramePos,"1",optReadOnly);
		//ticketRenewInterval
		holder.addDetail("EdgeFE/StreamSession","sessionScanInterval",&EdgeFEConfig::sessionScanInterval,"5000",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","ticketRenewInterval",&EdgeFEConfig::ticketRenewInterval,"60000",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","ticketRenewThreadpoolSize",&EdgeFEConfig::ticketRenewThreadpoolSize,"5",optReadOnly);
		holder.addDetail("EdgeFE/StreamSession","resourceScanInterval",&EdgeFEConfig::resourceScanInterval,"100000",optReadOnly);
		//resourceScanInterval

		//replica
		holder.addDetail("EdgeFE/Replica",&EdgeFEConfig::readStreamerReplicaConfig, &EdgeFEConfig::registerNothing);

		//performance tune
		holder.addDetail("EdgeFE/PeformanceTune/IceFreezeEnviroment","CheckpointPeriod",&EdgeFEConfig::perfCheckpointPeriod,"240",optReadOnly);
		holder.addDetail("EdgeFE/PeformanceTune/IceFreezeEnviroment","DbRecoverFatal",&EdgeFEConfig::perfDbRecoverFatal,"1",optReadOnly);
		holder.addDetail("EdgeFE/PeformanceTune/session","sessionSize",&EdgeFEConfig::perfSessionCacheSize,"200",optReadOnly);
		holder.addDetail("EdgeFE/PeformanceTune/session","SavePeriod",&EdgeFEConfig::perfSavePeriod,"60000",optReadOnly);
		holder.addDetail("EdgeFE/PeformanceTune/session","SaveSizeTrigger",&EdgeFEConfig::perfSaveSizeTrigger,"30",optReadOnly);
		holder.addDetail("EdgeFE/PeformanceTune/session","useMemoryDB",&EdgeFEConfig::perfUseMemoryDB,"0",optReadOnly);

		//assistant log
		holder.addDetail("EdgeFE/AssistantLog/SessionLog","level",&EdgeFEConfig::sessionLogLevel,"7",optReadOnly);
		holder.addDetail("EdgeFE/AssistantLog/SessionLog","fileSize",&EdgeFEConfig::sessionLogFileSize,"10240000",optReadOnly);
		holder.addDetail("EdgeFE/AssistantLog/SessionLog","buffer",&EdgeFEConfig::sessionLogBufferSize,"10240",optReadOnly);
		holder.addDetail("EdgeFE/AssistantLog/SessionLog","flushTimeout",&EdgeFEConfig::sessionLogTimeout,"2",optReadOnly);

		//CacheStore
		holder.addDetail("CacheStore",&EdgeFEConfig::readCacheStoreConfig,&EdgeFEConfig::registerNothing);

        // <ContentStore>
		holder.addDetail("ContentStore/Log/MainLog","level",&EdgeFEConfig::mainLogLevel,"7",optReadOnly);
		holder.addDetail("ContentStore/Log/MainLog","fileSize",&EdgeFEConfig::mainLogFileSize,"10240000",optReadOnly);
		holder.addDetail("ContentStore/Log/MainLog","buffer",&EdgeFEConfig::mainLogBufferSize,"10240",optReadOnly);
		holder.addDetail("ContentStore/Log/MainLog","flushTimeout",&EdgeFEConfig::mainLogTimeout,"2",optReadOnly);
		holder.addDetail("ContentStore/Log/EventLog","level",&EdgeFEConfig::eventLogLevel,"7",optReadOnly);
		holder.addDetail("ContentStore/Log/EventLog","fileSize",&EdgeFEConfig::eventLogFileSize,"10240000",optReadOnly);
		holder.addDetail("ContentStore/Log/EventLog","buffer",&EdgeFEConfig::eventLogBufferSize,"10240",optReadOnly);
		holder.addDetail("ContentStore/Log/EventLog","flushTimeout",&EdgeFEConfig::eventLogTimeout,"2",optReadOnly);

		holder.addDetail("ContentStore/Volumes","volCheckTimeout",&EdgeFEConfig::csVolCheckTimeout,"30000",optReadOnly);
        holder.addDetail("ContentStore/Volumes/Volume", &EdgeFEConfig::readVolumes, &EdgeFEConfig::registerNothing);
        //holder.addDetail("ContentStore", "netId", &EdgeFEConfig::netId, 0, optReadOnly);
        holder.addDetail("ContentStore", "cacheMode", &EdgeFEConfig::isCacheMode, "0", optReadOnly);
        holder.addDetail("ContentStore", "cacheLevel", &EdgeFEConfig::cacheLevel, "10", optReadOnly);
        holder.addDetail("ContentStore", "autoFileSync", &EdgeFEConfig::isAutoSync, "0", optReadOnly);
        holder.addDetail("ContentStore","ThreadPoolSize",&EdgeFEConfig::csThreadPoolSize,"20",optReadOnly);

        holder.addDetail("ContentStore/Provision", "defaultBandwidth", &EdgeFEConfig::defaultProvisionBW, "3750000", optReadOnly);
        holder.addDetail("ContentStore/Provision", "trickSpeeds", &EdgeFEConfig::strTrickSpeeds, "7.5", optReadOnly);
        holder.addDetail("ContentStore/Provision", "timeoutIdleProvisioning", &EdgeFEConfig::csTimeoutIdleProvisioning, "172800000", optReadOnly);
        holder.addDetail("ContentStore/Provision", "timeoutNotProvisioned", &EdgeFEConfig::csTimeoutNotProvisioned, "600000", optReadOnly);
		holder.addDetail("ContentStore/Provision","minimalMainfileSize",&EdgeFEConfig::csMinimalMainfileSize,"2457600",optReadOnly);

        holder.addDetail("ContentStore/DatabaseCache", "volumeSize", &EdgeFEConfig::volumeEvictorSize, "20", optReadOnly);
        holder.addDetail("ContentStore/DatabaseCache", "contentSize", &EdgeFEConfig::contentEvictorSize, "500", optReadOnly);

        holder.addDetail("ContentStore/CPC", "endpoint", &EdgeFEConfig::cpcEndpoint, 0, optReadOnly);
        holder.addDetail("ContentStore/CPC/Sessions", "registerInterval", &EdgeFEConfig::cpcRegisterInterval, "15000", optReadOnly);
		holder.addDetail("ContentStore/CPC/Sessions", "DefaultIndexType", &EdgeFEConfig::cpcDefaultIndexType, "VVC", optReadOnly);

        holder.addDetail("ContentStore/StoreReplica", "groupId", &EdgeFEConfig::csStrReplicaGroupId, "");
//        holder.addDetail("ContentStore/StoreReplica", "replicaId", &EdgeFEConfig::csStrReplicaId, "");
        holder.addDetail("ContentStore/StoreReplica", "replicaPriority", &EdgeFEConfig::csReplicaPriority, "0");
        holder.addDetail("ContentStore/StoreReplica", "timeout", &EdgeFEConfig::csReplicaTimeout, "60");
        holder.addDetail("ContentStore/StoreReplica/MasterReplica", "endpoint", &EdgeFEConfig::csMasterEndpoint, "");

		holder.addDetail("ContentStore/FileEvent","enableMulticast",&EdgeFEConfig::csEnableEventMulticast ,"0",optReadOnly);
		holder.addDetail("ContentStore/FileEvent","groupAddress",&EdgeFEConfig::csEventMulticastGroupIp,"",optReadOnly);
		holder.addDetail("ContentStore/FileEvent","groupPort",&EdgeFEConfig::csEventMulticastGroupPort,"65002",optReadOnly);
		holder.addDetail("ContentStore/FileEvent","groupBind",&EdgeFEConfig::csEventMulticastLocalIp,"",optReadOnly);
		holder.addDetail("ContentStore/FileEvent","neMappingSendInterval",&EdgeFEConfig::csNeMappingSendInterval,"5000",optReadOnly);
		holder.addDetail("ContentStore/FileEvent","showAllMulticastEvent",&EdgeFEConfig::csEventShowAll,"0",optReadOnly);
	}
};

}}

extern ZQ::common::Config::Loader<ZQ::StreamService::EdgeFEConfig>	 gEdgeFEConfig;


#endif//_cdn_streaming_service_config_header_file_h__
