#ifndef _TIANSHAN_STREAMSMITH_CONFIG_H__
#define _TIANSHAN_STREAMSMITH_CONFIG_H__


#include <confighelper.h>
#include <tchar.h>

struct OutOfService 
{
	OutOfService()
	{
		maxPendingRequest	=	0;
		timeout				=	0;
	}
	int32			maxPendingRequest;
	int32			timeout;
	static void structure( ZQ::common::Config::Holder<OutOfService>& holder )
	{
		holder.addDetail("","maxPending",&OutOfService::maxPendingRequest,"0",ZQ::common::Config::optReadOnly);
		holder.addDetail("","timeout",&OutOfService::timeout,"7200000",ZQ::common::Config::optReadOnly);
	}
};

struct VstrmRelativeConf 
{
	VstrmRelativeConf()
	{
		sessionCallbackTimeout	= 100;
	}
	int32			sessionCallbackTimeout ;
	static void structure( ZQ::common::Config::Holder<VstrmRelativeConf>& holder )
	{
		holder.addDetail("","sessionCallbackTimeout", &VstrmRelativeConf::sessionCallbackTimeout , "100", ZQ::common::Config::optReadOnly);
	}
};


struct CSContentAttribute 
{
	int32				supportVolume;
	std::string			ignorePrefix;
	std::string			ignoreFileEventPrefix;
	std::string			ignoreInvalidCharacter;
	int32				useVsOpenAPI;
	int32				attrFromVstm;
	int32				skipZeroByteFiles;
	static void structure(  ZQ::common::Config::Holder<CSContentAttribute>& holder  )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","suportVolume",&CSContentAttribute::supportVolume , "0" ,optReadOnly );
		holder.addDetail("","useVsOpenAPI",&CSContentAttribute::useVsOpenAPI,"1",optReadOnly);
		holder.addDetail("","attrFromVstm",&CSContentAttribute::attrFromVstm,"0",optReadOnly);
		holder.addDetail("","skipZeroByteFile",&CSContentAttribute::skipZeroByteFiles,"0",optReadOnly);
		holder.addDetail("ContentNameFixup","ignorePrefix",&CSContentAttribute::ignorePrefix,"",optReadOnly);
		holder.addDetail("ContentNameFixup","ingoreHiddenSessionWithChararcter",&CSContentAttribute::ignoreInvalidCharacter,"",optReadOnly);
		holder.addDetail("ContentNameFixup","ingorefileEventPrefix",&CSContentAttribute::ignoreFileEventPrefix,"",optReadOnly);
		
	}
};

struct SpigotReplica
{
	std::string		listenerEndpoint;
	int32			defaultUpdateInterval;
	std::string		groupId;
	std::string		category;
	std::string		nodeidformat;
	static void structure( ZQ::common::Config::Holder<SpigotReplica>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("subscriber","endpoint",&SpigotReplica::listenerEndpoint,"",optReadOnly);
		holder.addDetail("subscriber","updateInterval",&SpigotReplica::defaultUpdateInterval,"",optReadOnly);
		holder.addDetail("","groupId",&SpigotReplica::groupId,"",optReadOnly);
		holder.addDetail("","category",&SpigotReplica::category,"",optReadOnly);
		holder.addDetail("","nodeidformat",&SpigotReplica::nodeidformat,"SEAC{CLUSTERID}-N{NODEINDEX}_SS_NC",optReadOnly);
	}
};

struct NatConfig
{
	int32	maxWaiting;
	int32	enable ;
	static void structure( ZQ::common::Config::Holder<NatConfig>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","maxWait",&NatConfig::maxWaiting,"2000",optReadWrite);
		holder.addDetail("","enable",&NatConfig::enable,"1",optReadWrite);
	}
};

struct SubFileExtName
{
	std::string	fileExtName;
	static void structure( ZQ::common::Config::Holder<SubFileExtName> &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("",
						"name",
						&SubFileExtName::fileExtName);		
	}
};
// struct ContentProvisonConf
// {
// 	std::string		serverEndpoint;
// 	static void structure( ZQ::common::Config::Holder<ContentProvisonConf>& holder )
// 	{
// 		using namespace ZQ::common::Config;
// 		holder.addDetail("CPE",
// 							"serverEndpoint",
// 							&ContentProvisonConf::serverEndpoint);
// 	}
// };
struct ContentProvisionCluster
{
	int32			enableCpc;
	int32			defaultProvisionBW;
	int32			sessionRegisterInterval;
	static void structure( ZQ::common::Config::Holder<ContentProvisionCluster> &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("ContentProvision","defaultProvisionBandwidth",&ContentProvisionCluster::defaultProvisionBW,"37500",optReadOnly);
		holder.addDetail("Sessions","registerInterval",&ContentProvisionCluster::sessionRegisterInterval,"15000",optReadOnly);
	}
};



struct SubFileExtNameSet 
{

	std::vector<std::string>	subFileExtNames;
	static void structure( ZQ::common::Config::Holder<SubFileExtNameSet> &holder )
	{
		using namespace ZQ::common::Config;		
		holder.addDetail("ext",
			&SubFileExtNameSet::readSubFileExtName,
			&SubFileExtNameSet::registerNothing);		
	}
	void readSubFileExtName( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<SubFileExtName> subFileName;
		subFileName.read(node, hPP);

		std::vector<std::string>::const_iterator it = subFileExtNames.begin();
		for( ; it != subFileExtNames.end() ; it ++ )
		{
			if( *it == subFileName.fileExtName )
				return;
		}
		subFileExtNames.push_back( subFileName.fileExtName);
	}
	void registerNothing( const std::string& )
	{//do nothing
	}	
};

struct ContentStoreMasterReplicaSubscriber
{
	std::string		masterReplicaSubscriber;
	static void structure( ZQ::common::Config::Holder<ContentStoreMasterReplicaSubscriber> &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","endpoint",&ContentStoreMasterReplicaSubscriber::masterReplicaSubscriber,"",optReadOnly);
	}
};

struct VsisEventConf 
{
	VsisEventConf()
	{
		enable	= 0;
	}
	int32			enable ;
	std::string		localIp;
	std::string		groupIp;
	int32			groupPort;
	static void structure( ZQ::common::Config::Holder<VsisEventConf>& holder )
	{
		holder.addDetail("multicast","enable", &VsisEventConf::enable , "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("multicast","localIp", &VsisEventConf::localIp , "", ZQ::common::Config::optReadOnly);
		holder.addDetail("multicast","groupIp", &VsisEventConf::groupIp , "", ZQ::common::Config::optReadOnly);
		holder.addDetail("multicast","groupPort", &VsisEventConf::groupPort , "0", ZQ::common::Config::optReadOnly);
	}
};


struct EmbededContentStore
{
	std::string			csMainLogPath;
	int32				csMainLogSize;
	int32				csMainLogCount;
	int32				csMainLogLevel;

	std::string			csEventLogPath;
	int32				csEventLogSize;
	int32				csEventLogCount;
	int32				csEventLogLevel;

	int32				csEnableCpc;
	std::string			csCpcEndpoint;

	//std::string			csStrMasterListenerEndpoint;
	std::string			csStrReplicaGroupId;
	std::string			csStrStoreType;
	std::string			csStrNetId;
	std::string			csStrReplicaId;
	int32				csIReplicaPriority;
	int32				csIDefaultReplicaUpdateInterval;
	int32				csCacheMode;
	int32				csCacheLevel;
	int32				timeoutNotProvisioned;
	int32				timeoutIdleProvisoning;
	int32				timeoutOfPlaytimeAtProvisioning;

	int32				syncAtStart;
	int32				streamableLength;

	int32				csEvictorContentSize;
	int32				csEvictorVolumeSize;
	int32				csServiceThreadCount;

	//int32				csSupportNpVr;

	ZQ::common::Config::Holder<ContentStoreMasterReplicaSubscriber> csMasterReplicaSubscriber;

	ZQ::common::Config::Holder<SubFileExtNameSet>	subFileExtNameSet;
	ZQ::common::Config::Holder<ContentProvisionCluster> CpcConfig;
	ZQ::common::Config::Holder<CSContentAttribute>	ctntAttr;
	ZQ::common::Config::Holder<VsisEventConf>	vsisEventConf;

	static void structure( ZQ::common::Config::Holder<EmbededContentStore> &holder )
	{
		using namespace ZQ::common::Config;
		
		holder.addDetail("",
							"netId",
							&EmbededContentStore::csStrNetId,
							"");
		holder.addDetail("",
							"type",
							&EmbededContentStore::csStrStoreType,
							"MediaServerCS");
// 		holder.addDetail("",
// 							"nPVRMode",
// 							&EmbededContentStore::csSupportNpVr,
// 							"0");

		holder.addDetail("",
							"syncAtStart",
							&EmbededContentStore::syncAtStart,
							"0",
							optReadOnly);		
		holder.addDetail("",
							"serviceThread",
							&EmbededContentStore::csServiceThreadCount,
							"10",
							optReadOnly);
		holder.addDetail("",
							"cacheMode",
							&EmbededContentStore::csCacheMode,
							"0",
							optReadOnly);
		holder.addDetail("",
							"cacheLevel",
							&EmbededContentStore::csCacheLevel,
							"1",
							optReadOnly);
		holder.addDetail("",
							"timeoutNotProvisioned",
							&EmbededContentStore::timeoutNotProvisioned,
							"600000",
							optReadOnly);
		holder.addDetail("",
							"timeoutIdleProvisioning",
							&EmbededContentStore::timeoutIdleProvisoning,
							"7200000",
							optReadOnly);
		holder.addDetail("",
							"timeoutOfPlaytimeAtProvisioning",
							&EmbededContentStore::timeoutOfPlaytimeAtProvisioning,
							"1000",
							optReadOnly);
		holder.addDetail("",
							"streamableLength",
							&EmbededContentStore::streamableLength,
							"0",
							optReadOnly);

		holder.addDetail("DatabaseCache",
							"volumeSize",
							&EmbededContentStore::csEvictorVolumeSize,
							"100",
							optReadOnly);
		holder.addDetail("DatabaseCache",
							"contentSize",
							&EmbededContentStore::csEvictorContentSize,
							"1000",
							optReadOnly);

		holder.addDetail("StoreReplica",
							"groupId",
							&EmbededContentStore::csStrReplicaGroupId,
							"");

		holder.addDetail("StoreReplica",
							"replicaId",
							&EmbededContentStore::csStrReplicaId,
							"");

		holder.addDetail("StoreReplica",
							"replicaPriority",
							&EmbededContentStore::csIReplicaPriority,
							"0");

		holder.addDetail("StoreReplica",
							"timeout",
							&EmbededContentStore::csIDefaultReplicaUpdateInterval,
							"60");

// 		holder.addDetail("StoreReplica/MasterReplica",
// 							"endpoint",
// 							&EmbededContentStore::csStrMasterListenerEndpoint,
// 							"");		
	
		holder.addDetail("Log/CsMainLog",
							"path",
							&EmbededContentStore::csMainLogPath);
		holder.addDetail("Log/CsMainLog",
							"filesize",
							&EmbededContentStore::csMainLogSize,
							"102400000");
		holder.addDetail("Log/CsMainLog",
							"count",
							&EmbededContentStore::csMainLogCount,
							"5");
		holder.addDetail("Log/CsMainLog",
							"level",
							&EmbededContentStore::csMainLogLevel,
							"7");


		holder.addDetail("Log/CsEventLog",
							"path",
							&EmbededContentStore::csEventLogPath);
		holder.addDetail("Log/CsEventLog",
							"filesize",
							&EmbededContentStore::csEventLogSize,
							"102400000");
		holder.addDetail("Log/CsEventLog",
							"count",
							&EmbededContentStore::csEventLogCount,
							"5");
		holder.addDetail("Log/CsEventLog",
							"level",
							&EmbededContentStore::csEventLogLevel,
							"7");
	
		holder.addDetail("subFileExtName",						
							&EmbededContentStore::readSubFileExtName,
							&EmbededContentStore::registerNothing);

		holder.addDetail("CPC",
							&EmbededContentStore::readCPCCfg,
							&EmbededContentStore::registerNothing);

		holder.addDetail("CPC","enable",&EmbededContentStore::csEnableCpc,"0",optReadOnly);
		holder.addDetail("CPC","endpoint",&EmbededContentStore::csCpcEndpoint,"",optReadOnly);


		holder.addDetail("ContentAttribute",
							&EmbededContentStore::readContentAttribute,
							&EmbededContentStore::registerNothing);
		holder.addDetail("StoreReplica/MasterReplica",
							&EmbededContentStore::readContentStoreMasterSubscriber,
							&EmbededContentStore::registerNothing);
		holder.addDetail("VsisEvent",
							&EmbededContentStore::readVsisEventMulticast,
							&EmbededContentStore::registerNothing);

	}

	void readVsisEventMulticast(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		vsisEventConf.read( node , hPP );
	}
	void readContentStoreMasterSubscriber( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		csMasterReplicaSubscriber.read(node,hPP);
	}

	void readContentAttribute( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		ctntAttr.read( node , hPP );
	}
	void readSubFileExtName( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		
		subFileExtNameSet.read(node, hPP);
	}
	void readCPCCfg(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP  )
	{
		CpcConfig.read( node,hPP );
	}

	void registerNothing( const std::string& )
	{//do nothing
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

struct RtspReqeustApplication
{
	std::string	path;
	std::string handler;
	static void structure( ZQ::common::Config::Holder<RtspReqeustApplication> &holder )
	{
		holder.addDetail("","path",&RtspReqeustApplication::path);
		holder.addDetail("","handler",&RtspReqeustApplication::handler);
	}
};
struct RtspRequestSite
{
	std::string		siteName;
	std::map<std::string,std::string>	handlers;
	
	static void structure( ZQ::common::Config::Holder<RtspRequestSite> &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("",
						"name",
						&RtspRequestSite::siteName);
		holder.addDetail("Application",
						&RtspRequestSite::readApplication,
						&RtspRequestSite::registerNothing);			
	}
	void readApplication( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspReqeustApplication> applicationHolder;
		applicationHolder.read(node, hPP);			
		handlers[applicationHolder.path] = applicationHolder.handler;
	}
	void registerNothing( const std::string& )
	{//do nothing
	}
};
struct RtspDefaultSite 
{
	std::map<std::string,std::string>	handlers;
	static void structure( ZQ::common::Config::Holder<RtspDefaultSite> &holder )
	{
		holder.addDetail("Application",
						&RtspDefaultSite::readDefaultApplication,
						&RtspDefaultSite::registerNothing);
	}
	void readDefaultApplication( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspReqeustApplication> applicationHolder;
		applicationHolder.read(node, hPP);			
		handlers[applicationHolder.path] = applicationHolder.handler;
	}
	void registerNothing( const std::string& )
	{//do nothing
	}
};

struct RtspRequestPlugin 
{
	std::string		filePath;
	std::string		configPath;
	static void structure( ZQ::common::Config::Holder<RtspRequestPlugin> &holder )
	{
		holder.addDetail("module",
							"file",
							&RtspRequestPlugin::filePath);
		holder.addDetail("module",
							"configuration",
							&RtspRequestPlugin::configPath);
	}
};

struct RtspRequestHandler 
{

	typedef std::map<std::string , std::string> Apphandlers;
	std::map<std::string , Apphandlers >	sites;
	struct PluginAttr 
	{
		std::string			filePath;
		std::string			configFile;
	};
	std::vector<PluginAttr>						plugins;
	char										defaultHandler[512];
	static void structure(ZQ::common::Config::Holder<RtspRequestHandler>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("",
						"defaultHandler",
						(Holder<RtspRequestHandler>::PMem_CharArray)&RtspRequestHandler::defaultHandler,
						sizeof(holder.defaultHandler),
						"",
						optReadOnly);
		holder.addDetail("",
						&RtspRequestHandler::readDefaultApplication,
						&RtspRequestHandler::registerNothing);
		holder.addDetail("Site",
						&RtspRequestHandler::readSite,
						&RtspRequestHandler::registerNothing);
		holder.addDetail("Plugin",
						&RtspRequestHandler::readPlugin ,
						&RtspRequestHandler::registerNothing);
	}
	void readDefaultApplication( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspDefaultSite> siteHolder;
		siteHolder.read(node, hPP);			
		sites["."] = siteHolder.handlers;
	}

	void readSite( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspRequestSite> siteHolder;
		siteHolder.read(node,hPP);
		sites[siteHolder.siteName] = siteHolder.handlers;
	}
	void readPlugin( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspRequestPlugin> pluginHolder;
		//PluginAttr
		pluginHolder.read( node , hPP );
		PluginAttr attr;
		attr.filePath = pluginHolder.filePath;
		attr.configFile = pluginHolder.configPath;
		plugins.push_back(attr);
	}
	void registerNothing( const std::string& )
	{//do nothing

	}
	std::vector<std::string>	GetSiteName( )
	{
		std::vector<std::string> result;
		result.clear();
		std::map<std::string , Apphandlers >::const_iterator it  = sites.begin();
		for ( ; it != sites.end() ; it ++ ) 
		{
			result.push_back(it->first);
		}
		return result;
	}
	void GetPluginPathAndInfo(std::vector<PluginAttr>& pluginInfo)
	{
		pluginInfo = plugins;
	}
	std::string GetDefaultContenHandler( )
	{
		return std::string(defaultHandler);
	}
	bool GetVSitePathFromHandler(	const std::string& siteName, 
									const std::string& handler, 
									std::vector<std::string>& path )
	{
		std::map<std::string , Apphandlers >::const_iterator itSite = sites.find(siteName);
		if( itSite == sites.end( ) )
			return false;
		Apphandlers::const_iterator itAppHanlder = itSite->second.begin();
		for( ; itAppHanlder != itSite->second.end() ; itAppHanlder ++ )
		{
			if ( itAppHanlder->second == handler )
			{
				path.push_back(itAppHanlder->first);
			}
		}
		return true;
	}
	
};
struct PlaylistItemConfig 
{
	PlaylistItemConfig()
	{
		lGetContentAttributeFromRemoteThreshold = 20 * 1000;
		lGetContentAttributeFromRemoteForPWE	= 10 * 1000;
	}
	int32			lGetContentAttributeFromRemoteThreshold;
	int32			lGetContentAttributeFromRemoteForPWE;
	static void structure(ZQ::common::Config::Holder<PlaylistItemConfig>& holder )	
	{
		holder.addDetail("","cachedAttributeTimeout",&PlaylistItemConfig::lGetContentAttributeFromRemoteThreshold,"20000",ZQ::common::Config::optReadOnly);
		holder.addDetail("","pweAttributeTimeout",&PlaylistItemConfig::lGetContentAttributeFromRemoteForPWE,"10000",ZQ::common::Config::optReadOnly);
	}
};

struct StreamSmithCfg 
{
	//to get the cluster Id of current node using VstrmClassGetClusterDataEx
	//And this configuration will be set when vstrm class is initialized
	int32			mediaClusterId;				


	char			szMiniDumpPath[512];		//crash dump directory
	int32			lEnableMiniDump;			//1 for enable 0 for disable
	
	int32			lEnableIceTrace;			//1 for enable 0 for disable	
	int32			lIceTraceStandAlone;
	char			szIceLogSuffix[512];
	int32			lIceLogLevel;
	int32			lIceLogSize;
	int32			lIceLogBuffer;
	int32			lIceLogTimeout;
	
	char			szSvcLogFileName[512];
	int32			lSvcLogLevel;
	int32			lSvcLogSize;	
	int32			lSvcLogbuffer;
	int32			lSvcLogTimeout;
	
	char			szSvcEndpoint[512];
	
	char			szPluginLogName[512];
	int32			lPluginLogLevel;
	int32			lPluginLogSize;
	int32			lPluginLogBuffer;
	int32			lPluginLogTimeout;

	int32			lEventSinkTimeout;
	int32			lServiceThreadCount;
	int32			lServiceAmdThreadCount;

	char			szSessMonLogName[512];
	int32			lSessMonLogLevel;
	int32			lSessMonLogSize;
	int32			lSessMonLogBuffer;
	int32			lSessMonLogTimeout;

	char			szSuperPluginPath[1024];
	int32			lEnableSuperPlugin;
	char			szEventChannelEndpoint[512];
	char			szServiceID[512];
	char			szDefaultSpigotID[512];

	int32			lForceNormalTimeBeforeEnd;

	char			szFailOverDatabasePath[512];
	char			szResourceManagerFileName[512];

	int32			lIsSvcFrameWork;
	int32			lKillPlInterval;
	int32			lPlaylistTimeout;
	int32			lKeepOnPause;
	int32			lQueryLastItemPlayTime;
	int32			lPauseWhenUnloadItem;
	int32			lSpigotStatusScanInterval;
	int32			lLoadItemWithoutNpt;
	
	int32			lUseRandomVstrmPortAssignStrategy;
	int32			lProgressEventSendoutInterval;	


	int32			lUseLocalVvxParser;						/*This is a temporary configuration for debug use*/
	int32			lCheckContentStoreInterval;				/*interval for check availability of contentstore*/
	int32			lEnableQueryFromContentStore;			/*if this value is set to 1, StreamSmith will query item' information from content Store*/
	int32			lUseLocalParserValue;

	char			szPrimaryContentStoreEndpoint[1024];
	char			szSecondaryContentStoreEndpoint[1024];


	int32			lEnableShowEventSinkDetail;				/*turn this flag on if you want to see the detail of sending out event in file log*/


	int32			lEncryptionCycle1;
	int32			lEncryptionCycle2;
	int32			lEncryptionFreq1;
	int32			lEncryptionFreq2;


	int32			lEvictorPlaylistSize;
	int32			lEvictorItemSize;
	int32			lMaxPendingRequest;
	int32			lmaxWaitTimeToStartFirstItem;
	int32			lEnablemaxWaitTimeToStartFirstItem;
	int32			lprogressScanInterval;

	int32			lEnablePartialSelectVstrmPort;
	int32			lPartialSelectLow;
	int32			lPartialSelectHigh;

	char			szAdapterThreadpoolSize[32];
	char			szAdapterThreadpoolSizeMax[32];

	int32			lSessScanCoef;
	int32			lMaxMsgSize;
	int32			lEnableUseLocaltimeInDatHeader;
	int32			lEOTProtectionTime;
	int32			lPreloadTime;
	int32			lProtectionWindow;
	int32			lVstrmSessionScanInterval;
	int32			lRepositioInaccuracy;
	int32			lUseDeviceIoScan;
	int32			lUseRepositionWhenUnload;
	int32			lRetryCountAtBusyChangeSpeed;
	int32			lRetryIntervalAtUnload;
	int32			lPortCooldownTime;
	int32			lApplyRestrictionWhenPrimenext;
	int32           lperRequestInterval;
	


	int32			lRtspParserThreadPoolSize;
	int32			lRtspParserThreadPriority;
	int32			lProcessPriority;


	int32			lRtspSocketServerPort;
	int32			lMaxClientConnection;
	int32			lServiceFrameThreadPoolSize;
	int32			lServiceFrameThreadPriority;
	int32			lServiceFrameDebugLevel;
	int32			lServiceFrameDebugDetail;
	int32			lServiceFrameIdleTimeout;
	int32			lServiceFrameIdleScanInterval;

	int32			lServiceFrameSSLEnabled;
	char			szServiceFramepublicKeyFile[512];
	char			szServiceFrameprivateKeyFile[512];
	char			szServiceFrameprivatePassword[512];
	char			szServiceFramedhParamFile[512];
	char			szServiceFramerandFile[512];

	int32			lIncomingMsgMaxLen;
	int32			lIncomingMsgRecvBufSize;

	int32			lLscpSocketServerPort;

	char			szIceEnvCheckPointPeriod[512];
	char			szIceEnvDbRecoverFatal[512];
	char			szFreezePlaylistSavePeriod[512];
	char			szFreezePlaylistSaveSizeTrigger[512];
	char			szFreezeItemSavePeriod[512];
	char			szFreezeItemSaveSizeTrigger[512];

	int32			lCurTimeZone;	

	int32			lBandwidthUsageScanInterval;

	//begin internal use
	int32			lLscpPort;
	int32			lRtspPort;
	int32			lListenPort;
	//end internal use

	int32			serverMode;
	//	0 for normal server
	//	1 for Npvr server
	//	2 for edge server
	std::string		strServerMode;



	int32			lDbHealththreshold;
	

	
	std::vector<MonitoredLog> monitoredLogs;
    std::map<std::string, std::string> iceProperties;
	ZQ::common::Config::Holder<RtspRequestHandler>		reqesutHanlders;
	
	ZQ::common::Config::Holder<NatConfig>		natConfiguration;

	ZQ::common::Config::Holder<EmbededContentStore>		embededContenStoreCfg;

	ZQ::common::Config::Holder<SpigotReplica>			spigotReplicaConfig;

	ZQ::common::Config::Holder<OutOfService>			outOfServiceConf;

	ZQ::common::Config::Holder<VstrmRelativeConf>		vstrmRelativeConf;

	ZQ::common::Config::Holder<PlaylistItemConfig>		playlistItemConf;

	static void structure(ZQ::common::Config::Holder<StreamSmithCfg> &holder)
	{
		using namespace ZQ::common::Config;		

		holder.addDetail("default/EventChannel",
							"endpoint",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szEventChannelEndpoint, 
							sizeof(holder.szEventChannelEndpoint),
							NULL,
							optReadOnly);
		
		holder.addDetail("default/CrashDump", 
							"path", 
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szMiniDumpPath, 
							sizeof(holder.szMiniDumpPath), 
							NULL, 
							optReadOnly);
		holder.addDetail("default/CrashDump",
							"enabled",	
							&StreamSmithCfg::lEnableMiniDump, 
							"0", 
							optReadOnly);

		holder.addDetail("default/IceTrace",
							"standalone",
							&StreamSmithCfg::lIceTraceStandAlone,
							"0",
							optReadOnly);

		holder.addDetail("default/IceTrace",
							"enabled",
							&StreamSmithCfg::lEnableIceTrace,
							"0",
							optReadOnly);
		holder.addDetail("default/IceTrace",
							"level",
							&StreamSmithCfg::lIceLogLevel,
							"7",
							optReadOnly);
		holder.addDetail("default/IceTrace",
							"size",
							&StreamSmithCfg::lIceLogSize,
							"10240000",
							optReadOnly);
		holder.addDetail("default/IceProperties/serviceProperty",
							&StreamSmithCfg::readIceProperty,
							&StreamSmithCfg::registerNothing);
		holder.addDetail("default/Database",
							"path",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szFailOverDatabasePath, 
							sizeof(holder.szFailOverDatabasePath),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith",
							"netId",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szServiceID,
							sizeof(holder.szServiceID),
							"",
							optReadOnly);

		holder.addDetail("StreamSmith",
							"mode",
							&StreamSmithCfg::strServerMode,
							"",
							optReadOnly);

		holder.addDetail("StreamSmith/SuperPlugin",
							"path",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szSuperPluginPath,
							sizeof(holder.szSuperPluginPath),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/SuperPlugin",
							"enable",
							&StreamSmithCfg::lEnableSuperPlugin,
							"0",
							optReadOnly);
		holder.addDetail("StreamSmith/SuperPlugin",
							"eventSinkTimeout",
							&StreamSmithCfg::lEventSinkTimeout,
							"15000",
							optReadWrite);
		holder.addDetail("StreamSmith/SuperPlugin",
							"enableShowDetail",
							&StreamSmithCfg::lEnableShowEventSinkDetail,
							"1",
							optReadWrite);
		holder.addDetail("StreamSmith/Bind",
							"endpoint",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szSvcEndpoint,
							sizeof(holder.szSvcEndpoint),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/Bind",
							"dispatchSize",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szAdapterThreadpoolSize,
							sizeof(holder.szAdapterThreadpoolSize),
							"",
							optReadOnly);
		
		holder.addDetail("StreamSmith/Bind",
							"dispatchMax",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szAdapterThreadpoolSizeMax,
							sizeof(holder.szAdapterThreadpoolSizeMax),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/RequestProcess",
							"threads",
							&StreamSmithCfg::lServiceThreadCount,
							"20",
							optReadOnly);		
		holder.addDetail("StreamSmith/RequestProcess",
							"amdThreads",
							&StreamSmithCfg::lServiceAmdThreadCount,
							"20",
							optReadOnly);
		
		holder.addDetail("StreamSmith/LocalResource",
							"configuration",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szResourceManagerFileName,
							sizeof(holder.szResourceManagerFileName),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/LocalResource/Streamer",
							"defaultSpigotId",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szDefaultSpigotID,
							sizeof(holder.szDefaultSpigotID),
							"-1",
							optReadOnly);
		holder.addDetail("StreamSmith/LocalResource/Streamer",
							"partialSelectEnable",
							&StreamSmithCfg::lEnablePartialSelectVstrmPort,
							"0",
							optReadOnly);
		holder.addDetail("StreamSmith/LocalResource/Streamer",
							"PartialSelectionLow",
							&StreamSmithCfg::lPartialSelectLow,
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/LocalResource/Streamer",
							"PartialSelectionHigh",
							&StreamSmithCfg::lPartialSelectHigh,
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/PublishedLogs/Log",
							&StreamSmithCfg::readMonitoredLog,
							&StreamSmithCfg::registerNothing);
		holder.addDetail("StreamSmith/DatabaseCache",
							"playlistSize",
							&StreamSmithCfg::lEvictorPlaylistSize,
							"1000",
							optReadOnly);
		holder.addDetail("StreamSmith/DatabaseCache",
							"dbHealthCheckThreshold",
							&StreamSmithCfg::lDbHealththreshold,
							"1800000",
							optReadOnly);
		holder.addDetail("StreamSmith/DatabaseCache",
							"itemSize",
							&StreamSmithCfg::lEvictorItemSize,
							"2000",
							optReadOnly);
		holder.addDetail("StreamSmith/Playlist",
							"pauseWhenUnload",
							&StreamSmithCfg::lPauseWhenUnloadItem,
							"0",
							optReadOnly);
		holder.addDetail("StreamSmith/Playlist",			
							"loadItemWithoutNpt",
							&StreamSmithCfg::lLoadItemWithoutNpt,
							"0",
							optReadOnly);
		holder.addDetail("StreamSmith/Playlist",
							"sessionScanInterval",
							&StreamSmithCfg::lVstrmSessionScanInterval,
							"66",
							optReadOnly);
		
		holder.addDetail("StreamSmith/Playlist",
							"portCooldownTime",
							&StreamSmithCfg::lPortCooldownTime,
							"6000",
							optReadOnly);
		
		holder.addDetail("StreamSmith/Playlist",
							"applyRestrictionWhenPrimenext",
							&StreamSmithCfg::lApplyRestrictionWhenPrimenext,
							"0",
							optReadOnly);


		holder.addDetail("StreamSmith/Playlist",
						"bandwidthUsageScanInterval",
						&StreamSmithCfg::lBandwidthUsageScanInterval,
						"10000",
						optReadWrite);
		
		holder.addDetail("StreamSmith/Playlist",
						"UseDeviceIoScan",
						&StreamSmithCfg::lUseDeviceIoScan,
						"0",
						optReadWrite);
		
		holder.addDetail("StreamSmith/Playlist",
							"UseRepositionWhenUnload",
							&StreamSmithCfg::lUseRepositionWhenUnload,
							"1",
							optReadWrite);
		holder.addDetail("StreamSmith/Playlist",
							"RetryCountAtBusyChangeSpeed",
							&StreamSmithCfg::lRetryCountAtBusyChangeSpeed,
							"5",
							optReadWrite);

		//lRetryIntervalAtUnload
		holder.addDetail("StreamSmith/Playlist",
							"RetryIntervalAtUnload",
							&StreamSmithCfg::lRetryIntervalAtUnload,
							"1",
							optReadWrite);
							
		holder.addDetail("StreamSmith/Playlist",
							"timeout",
							&StreamSmithCfg::lPlaylistTimeout,
							"100000",
							optReadWrite);
		holder.addDetail("StreamSmith/Playlist",
							"keepOnPause",
							&StreamSmithCfg::lKeepOnPause,
							"0",
							optReadOnly);
		holder.addDetail("StreamSmith/Playlist",
							"QueryLastItemPlayTime",
							&StreamSmithCfg::lQueryLastItemPlayTime,
							"1",
							optReadOnly);
		//lSpigotStatusScanInterval
		holder.addDetail("StreamSmith/Playlist",
							"SpigotStatusScanInterval",
							&StreamSmithCfg::lSpigotStatusScanInterval,
							"3600000",
							optReadOnly);

		holder.addDetail("StreamSmith/Playlist",
							"progressInterval",
							&StreamSmithCfg::lProgressEventSendoutInterval,
							"30000",
							optReadOnly);
		holder.addDetail("StreamSmith/Playlist",
							"EOTsize",
							&StreamSmithCfg::lEOTProtectionTime,
							"15000",
							optReadWrite);		
		holder.addDetail("StreamSmith/Playlist",
							"PreLoadTime",
							&StreamSmithCfg::lPreloadTime,
							"5000",
							optReadWrite);
		holder.addDetail("StreamSmith/Playlist",
							"protectionArea",
							&StreamSmithCfg::lProtectionWindow,
							"1000",
							optReadWrite);
		holder.addDetail("StreamSmith/Playlist",
							"EOTsize",
							&StreamSmithCfg::lForceNormalTimeBeforeEnd,
							"15000",
							optReadWrite,
							"StreamSmith/Playlist/protectionArea");
		holder.addDetail("StreamSmith/Playlist",
							"delayedCleanup",
							&StreamSmithCfg::lKillPlInterval,
							"150000",
							optReadWrite);
		holder.addDetail( "StreamSmith/Playlist",
							"repositionInaccuracy",
							&StreamSmithCfg::lRepositioInaccuracy,
							"5000",
							optReadOnly);
		holder.addDetail( "StreamSmith/Playlist",
			                "perRequestInterval",
							&StreamSmithCfg::lperRequestInterval,
							"1000",
							optReadOnly);

		holder.addDetail("StreamSmith/CriticalStartPlayWait",
							"enable",
							&StreamSmithCfg::lEnablemaxWaitTimeToStartFirstItem,
							"1",
							optReadWrite);
		holder.addDetail("StreamSmith/CriticalStartPlayWait",
							"timeout",
							&StreamSmithCfg::lmaxWaitTimeToStartFirstItem,
							"3000",
							optReadWrite);
		holder.addDetail("StreamSmith/MotoPreEncryption",
							"cycle1",
							&StreamSmithCfg::lEncryptionCycle1,
							"0",
							optReadWrite);
		holder.addDetail("StreamSmith/MotoPreEncryption",
							"freq1",
							&StreamSmithCfg::lEncryptionFreq1,
							"8",
							optReadWrite);
		holder.addDetail("StreamSmith/MotoPreEncryption",
							"cycle2",
							&StreamSmithCfg::lEncryptionCycle2,
							"15000",
							optReadWrite);
		holder.addDetail("StreamSmith/MotoPreEncryption",
							"freq2",
							&StreamSmithCfg::lEncryptionFreq2,
							"8",
							optReadWrite);
		holder.addDetail("StreamSmith/Plugin/log",
							"level",
							&StreamSmithCfg::lPluginLogLevel,
							"7",
							optReadOnly);
		holder.addDetail("StreamSmith/Plugin/log",
							"fileSize",
							&StreamSmithCfg::lPluginLogSize,
							"10240000",
							optReadOnly);
		holder.addDetail("StreamSmith/Plugin/log",
							"buffer",
							&StreamSmithCfg::lPluginLogBuffer,
							"10240",
							optReadOnly);
		holder.addDetail("StreamSmith/Plugin/log",
							"flushTimeout",
							&StreamSmithCfg::lPluginLogTimeout,
							"2",
							optReadOnly);
		holder.addDetail("StreamSmith/SessionMonitor/log",
							"level",
							&StreamSmithCfg::lSessMonLogLevel,
							"7",
							optReadOnly);
		holder.addDetail("StreamSmith/SessionMonitor/log",
							"fileSize",
							&StreamSmithCfg::lSessMonLogSize,
							"10240000",
							optReadOnly);
		holder.addDetail("StreamSmith/SessionMonitor/log",
							"buffer",
							&StreamSmithCfg::lSessMonLogBuffer,
							"10240",
							optReadOnly);
		holder.addDetail("StreamSmith/SessionMonitor/log",
							"flushTimeout",
							&StreamSmithCfg::lSessMonLogTimeout,
							"2",
							optReadOnly);

		holder.addDetail("StreamSmith/PeformanceTune/IceFreezeEnviroment",
							"CheckpointPeriod",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szIceEnvCheckPointPeriod,
							sizeof(holder.szIceEnvCheckPointPeriod),
							"240",
							optReadOnly);

		holder.addDetail("StreamSmith/PeformanceTune/IceFreezeEnviroment",
							"DbRecoverFatal",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szIceEnvDbRecoverFatal,
							sizeof(holder.szIceEnvDbRecoverFatal),
							"1",
							optReadOnly);
		holder.addDetail("StreamSmith/PeformanceTune/playlist",
							"SavePeriod",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szFreezePlaylistSavePeriod,
							sizeof(holder.szFreezePlaylistSavePeriod),
							"240",
							optReadOnly);
		holder.addDetail("StreamSmith/PeformanceTune/playlist",
							"SaveSizeTrigger",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szFreezePlaylistSaveSizeTrigger,
							sizeof(holder.szFreezePlaylistSaveSizeTrigger),
							"1000",
							optReadOnly);

		holder.addDetail("StreamSmith/PeformanceTune/item",
							"SavePeriod",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szFreezeItemSavePeriod,
							sizeof(holder.szFreezeItemSavePeriod),
							"240",
							optReadOnly);
		holder.addDetail("StreamSmith/PeformanceTune/item",
							"SaveSizeTrigger",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szFreezeItemSaveSizeTrigger,
							sizeof(holder.szFreezeItemSaveSizeTrigger),
							"1000",
							optReadOnly);


		holder.addDetail("StreamSmith/SocketServer",
							"enable",
							&StreamSmithCfg::lIsSvcFrameWork,
							"0",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"rtspPort",
							&StreamSmithCfg::lRtspSocketServerPort,
							"554",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"lscpPort",
							&StreamSmithCfg::lLscpSocketServerPort,
							"5542",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"maxConnections",
							&StreamSmithCfg::lMaxClientConnection,
							"850",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"threads",
							&StreamSmithCfg::lServiceFrameThreadPoolSize,
							"20",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"threadPriority",
							&StreamSmithCfg::lServiceFrameThreadPriority,
							"2",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"debugLevel",
							&StreamSmithCfg::lServiceFrameDebugLevel,
							"4",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"debugDetail",
							&StreamSmithCfg::lServiceFrameDebugDetail,
							"3",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"idleTimeout",
							&StreamSmithCfg::lServiceFrameIdleTimeout,
							"300000",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer",
							"idleScanInterval",
							&StreamSmithCfg::lServiceFrameIdleScanInterval,
							"500",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/SSL",
							"enabled",
							&StreamSmithCfg::lServiceFrameSSLEnabled,
							"0",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/SSL",
							"publicKeyFile",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szServiceFramepublicKeyFile, 
							sizeof(holder.szServiceFramepublicKeyFile), 
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/SSL",
							"privateKeyFile",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szServiceFrameprivateKeyFile,   
							sizeof(holder.szServiceFrameprivateKeyFile),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/SSL",
							"privatePassword",							
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szServiceFrameprivatePassword,   
							sizeof(holder.szServiceFrameprivatePassword),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/SSL",
							"dhParamFile",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szServiceFramedhParamFile,
							sizeof(holder.szServiceFramedhParamFile),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/SSL",
							"randFile",
							(Holder<StreamSmithCfg>::PMem_CharArray)&StreamSmithCfg::szServiceFramerandFile,
							sizeof(holder.szServiceFramerandFile),
							"",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/IncomingMessage",
							"maxLen",
							&StreamSmithCfg::lIncomingMsgMaxLen,
							"32768",
							optReadOnly);
		holder.addDetail("StreamSmith/SocketServer/IncomingMessage",
							"recvBufSize",
							&StreamSmithCfg::lIncomingMsgRecvBufSize,
							"65536",
							optReadOnly);

		holder.addDetail("StreamSmith/RequestHandler",
							&StreamSmithCfg::readReqesutHandler,
							&StreamSmithCfg::registerNothing)	;	

		holder.addDetail("ContentStore",						
						&StreamSmithCfg::readEmbededContenStore,
						&StreamSmithCfg::registerNothing);	
		
		holder.addDetail("StreamSmith/NAT",
						&StreamSmithCfg::readNatConfig,
						&StreamSmithCfg::registerNothing);
		holder.addDetail("StreamSmith/Replica",
						&StreamSmithCfg::readSpigotReplicaConfig,
						&StreamSmithCfg::registerNothing);

		holder.addDetail("StreamSmith/OutOfService",
						&StreamSmithCfg::readOutOfServiceConf,
						&StreamSmithCfg::registerNothing);
		
		holder.addDetail("StreamSmith/VstrmConfig",
						&StreamSmithCfg::readVstrmRelativeConfig,
						&StreamSmithCfg::registerNothing);
		
		holder.addDetail("StreamSmith/Playlist/PlaylistItem",
						&StreamSmithCfg::readPlaylistItemConfig,
						&StreamSmithCfg::registerNothing);

	}

	void readPlaylistItemConfig( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		playlistItemConf.read( node , hPP );
	}
	void readVstrmRelativeConfig(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		vstrmRelativeConf.read( node , hPP );
	}

	void readOutOfServiceConf( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		outOfServiceConf.read( node , hPP );
	}

	void readSpigotReplicaConfig(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP   )
	{
		spigotReplicaConfig.read(node,hPP);
	}

	void readNatConfig(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP  )
	{
		natConfiguration.read(node,hPP);
	}
	
	void readReqesutHandler(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP  )
	{
		using namespace ZQ::common::Config;		
		reqesutHanlders.read(node,hPP);
	}

	void readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
    {
        using namespace ZQ::common::Config;
        Holder<MonitoredLog> lmHolder;
        lmHolder.read(node, hPP);
        monitoredLogs.push_back(lmHolder);
    }

	void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
    {
        using namespace ZQ::common::Config;
        Holder<NVPair> propHolder;
        propHolder.read(node, hPP);
        iceProperties[propHolder.name] = propHolder.value;
    }
	void readEmbededContenStore( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		//Holder<EmbededContentStore> ncsConfig;
		embededContenStoreCfg.read(node, hPP);	
		//embededContenStoreCfg = ncsConfig;
	}
    void registerNothing(const std::string&){}


	StreamSmithCfg()
	{
		ZeroMemory(szMiniDumpPath,sizeof(szMiniDumpPath));
		lEnableMiniDump=1;
		
		lEnableIceTrace=1;
		ZeroMemory(szIceLogSuffix,sizeof(szIceLogSuffix));
		lIceLogLevel=7;
		lIceLogSize=10*1024*1024;
		lIceLogBuffer=10*1024;
		lIceLogTimeout=2;
		
		ZeroMemory(szSvcLogFileName,sizeof(szSvcLogFileName));
		lSvcLogLevel=7;
		lSvcLogSize=10*10224*1024;
		lSvcLogbuffer=10*1024;
		lSvcLogTimeout=2;
		
		ZeroMemory(szSvcEndpoint,sizeof(szSvcEndpoint));	
		
		ZeroMemory(szPluginLogName,sizeof(szPluginLogName));
		lPluginLogLevel=7;
		lPluginLogSize=10*1024*1024;
		lPluginLogBuffer=10*1024;;
		lPluginLogTimeout=2;
		
		lEventSinkTimeout=500*1000;
		lServiceThreadCount=5;
		
		ZeroMemory(szSessMonLogName,sizeof(szSessMonLogName));
		lSessMonLogLevel=7;
		lSessMonLogSize=10*1024*1024;
		lSessMonLogBuffer=10*1024;
		lSessMonLogTimeout=2;
		
		ZeroMemory(szSuperPluginPath,sizeof(szSuperPluginPath));
		lEnableSuperPlugin=0;
		ZeroMemory(szEventChannelEndpoint,sizeof(szEventChannelEndpoint));
		ZeroMemory(szServiceID,sizeof(szServiceID));
		ZeroMemory(szDefaultSpigotID,sizeof(szDefaultSpigotID));
		
		lForceNormalTimeBeforeEnd=15*1000;
		
		ZeroMemory(szFailOverDatabasePath,sizeof(szFailOverDatabasePath));
		
		lKillPlInterval=100*1000;
		lPlaylistTimeout=1000*1000;
		lKeepOnPause=1;
		lUseDeviceIoScan = 0;

		lIsSvcFrameWork=1;
		
		lUseRandomVstrmPortAssignStrategy = 0;	//default to use random vstrm port assign strategy
		lProgressEventSendoutInterval	= 30000;	//default send out progress event 1 piece per second
		
		lUseLocalVvxParser				= 0;		//this configuration is only used to debug,so set it to 0 as default
		lCheckContentStoreInterval		=5000;
		lEnableQueryFromContentStore	=	1;
		lUseLocalParserValue			= 0;
		
		ZeroMemory( szPrimaryContentStoreEndpoint,sizeof(szPrimaryContentStoreEndpoint) );
		ZeroMemory( szSecondaryContentStoreEndpoint,sizeof(szSecondaryContentStoreEndpoint) );
		
		lEnableShowEventSinkDetail		=	0;
		
		lEncryptionCycle1				= 0;
		lEncryptionCycle2				= 15000;
		lEncryptionFreq1				= 8;
		lEncryptionFreq2				= 8;
		
		lEvictorPlaylistSize			= 150;
		lEvictorItemSize				= 300;
		lMaxPendingRequest				= 0;
		
		lmaxWaitTimeToStartFirstItem	= 3000;//milliseconds
		lprogressScanInterval			= 5000;//progress scan interval
		
		lEnablePartialSelectVstrmPort	= 0;
		lPartialSelectLow				= 0;
		lPartialSelectHigh				= 0;
		lSessScanCoef					= -1; //Default value,will be changed in vstrm class initialize
		
		strcpy(szAdapterThreadpoolSize,"10");
		
		lMaxMsgSize						= 16*1024;
		
		lCurTimeZone = 0;
		lEnableUseLocaltimeInDatHeader = 0;
		
		lEOTProtectionTime				= 10000;
		
		lPreloadTime					= 5000;
		
		lBandwidthUsageScanInterval		= 10*1000;
	}
};

extern ZQ::common::Config::Loader<StreamSmithCfg> gStreamSmithConfig;
#define GAPPLICATIONCONFIGURATION gStreamSmithConfig



#endif//_TIANSHAN_STREAMSMITH_CONFIG_H__