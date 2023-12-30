#ifndef __tianshan_ngod_config_header_file_h__
#define __tianshan_ngod_config_header_file_h__

#include "ConfigHelper.h"
#include <string>
#include <map>
#include <set>
#include "StreamSmithAdmin.h"
#include "LAMFacade.h"

namespace NGOD
{

struct AssetStack 
{
	AssetStack()
	{
		enable = 1;
		adjustWeight = 9900;
		startMode = 0;
	}
	int32		enable;	
	int32		adjustWeight;
	int32		startMode;
	static void structure( ZQ::common::Config::Holder<AssetStack>& holder );
};

struct SubD5Listener 
{
	std::string	d5serverIp;
	int32		d5serverPort;
	SubD5Listener()
	{
		d5serverPort = 0;
	}
	typedef ZQ::common::Config::Holder<SubD5Listener> SubListenerHolder;
	static void structure(ZQ::common::Config::Holder<SubD5Listener>& holder );
};

struct D5MessageConf
{
	int32			enable;
	std::string		d5serverIp;
	int32			d5serverPort;
	int32			holdTimer;
	int32			keepAliveInterval;
	std::string		streamZone;
	std::string		nextHopServer;
	int32			msgUpdateInterval;
	int32			diffPercent;
	typedef ZQ::common::Config::Holder<SubD5Listener> SubListenerHolder;
	std::vector<SubListenerHolder> subListener;
	static void structure( ZQ::common::Config::Holder<D5MessageConf>& holder );

	void readSubListenr(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSubListenr(const std::string &full_path){}

};
	

struct MessageFormat 
{
	MessageFormat()
	{
		rtspNptUsage = 0;
	}
	int32			rtspNptUsage;
	static void structure( ZQ::common::Config::Holder<MessageFormat>& holder );
};

// <LogFile size="50000000" level="7" maxCount="10" bufferSize="8192"/>

struct LogFile
{
	std::string		path;
	int32			size;
	int32			level;
	int32			maxCount;
	int32			bufferSize;
	LogFile()
	{
		size		= 50 * 1024*1024;
		level		= 6;
		maxCount	= 5;
		bufferSize	= 10 * 1024;
	}
	static void structure(ZQ::common::Config::Holder<LogFile>& holder);
};

struct EventLogFile
{
	std::string		path;
	int32			size;
	int32			level;
	int32			maxCount;
	int32			bufferSize;
	EventLogFile()
	{
		size		= 50 * 1024*1024;
		level		= 7;
		maxCount	= 4;
		bufferSize	= 10 * 1024;
	}
	static void structure(ZQ::common::Config::Holder<EventLogFile>& holder);
};

// <IceStorm endpoint="${PrimeIceStormEndpoint}" />

struct EventChannelConf
{
	std::string	endpoint;

	static void structure(ZQ::common::Config::Holder<EventChannelConf>& holder);
};

// <Bind endpoint="tcp -h ${ServerNetIf} -p 5735" dispatchSize="5" dispatchMax="30"/>

struct Bind
{
	std::string		endpoint;
	int32			dispatchSize;
	int32			dispatchMax;
	
	Bind()
	{
		dispatchSize	= 10;
		dispatchMax		= 15;
	}

	static void structure(ZQ::common::Config::Holder<Bind>& holder);
};

//<RTSPSession timeout="600" cacheSize="1000" monitorThreads="5" defaultServiceGroup="5"  />

struct RTSPSession
{
	int32	destroyRetryInterval;
	int32	timeout;	
	int32   cacheSize;
	int32	monitorThreads;
	int32	defaultServiceGroup;	
	int32	timeoutCount;
	int32	requestPriorityIfStreamerNA;
	int32	maxSessionDestroyTimeInterval;
	int32	timeoutParamCache;
	
	RTSPSession()
	{
		destroyRetryInterval= 60 ;
		timeout				= 20 * 60 ;	
		cacheSize           = 5000;
		monitorThreads		= 10;
		defaultServiceGroup	= 1;
		timeoutCount		= 3;
		requestPriorityIfStreamerNA = 200;
		maxSessionDestroyTimeInterval = 60 * 60;
	}
	static void structure(ZQ::common::Config::Holder<RTSPSession>& holder);
};

// <Database path="${TianShanDatabaseDir}" />

struct Database
{
	std::string path;
    std::string runtimePath;
	std::string fatalRecover;
	std::string checkpointPeriod;
	std::string saveSizeTrigger;
	std::string savePeriod;

	static void structure(ZQ::common::Config::Holder<Database>& holder);
};

// <Announce useGlobalCSeq="0"/>

struct Announce
{
	int32	useGlobalCSeq;
	int32	includeTransition;

	int32   notifyTrickRestriction;
	int32   notifyItemSkip;

	int32	skipEventOfRequested;
	int32   includeTeardownRange;

	int32	useTianShanAnnounceCodeStateChanged;
	int32	useTianShanAnnounceCodeScaleChanged;

	int32	resubscribeAtIdle;
	int32	eventTTL;
	int32	announceSenderThreadCount;

	Announce()
	{
		useGlobalCSeq						= 0;
		includeTransition					= 1;
		notifyTrickRestriction              = 1;
		notifyItemSkip                      = 1;
		includeTeardownRange                = 1;
		useTianShanAnnounceCodeStateChanged	= 0;
		useTianShanAnnounceCodeScaleChanged	= 0;
		resubscribeAtIdle					= 3600 * 1000;
		eventTTL							= 30 * 60* 1000 * 1000;
		announceSenderThreadCount			= 50;
		skipEventOfRequested				= 0;
	}

	static void structure(ZQ::common::Config::Holder<Announce>& holder);
};

// <Response setupFailureWithSessId="0"/>

struct Response
{
	int32			setupFailureWithSessId;
	std::string		streamCtrlProt;
	Response()
	{
		setupFailureWithSessId	= 1;
		streamCtrlProt			= "rtsp";
	}

	static void structure(ZQ::common::Config::Holder<Response>& holder);
};

//<PublishedLogs>
//	<Log path="${TianShanLogDir}\ssm_ngod2.Log" syntax="${TianShanHomeDir}\etc\ssm_ngod2.xml" />
//</PublishedLogs>

struct PublishLog
{
	std::string		path;
	std::string		syntax;
	std::string		type;
	std::string		key;

	typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;

	static void structure(ZQ::common::Config::Holder<PublishLog>& holder);
};

struct PublishLogs
{
	int32 enabled;

	typedef PublishLog::PublishLogHolder PublishLogHolder;
	std::vector<PublishLogHolder>	logDatas;

	static void structure(ZQ::common::Config::Holder<PublishLogs>& holder);

    void readPublishLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerPublishLog(const std::string &full_path);
};


//<IceProperties>
//		<prop name="Ice.Trace.Network"                  value="1" />
//		<prop name="Ice.Trace.Protocol"                 value="0" />
// </IceProperties>

struct IceProperty
{
	std::string		name;
	std::string		value;

	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;

	static void structure(IcePropertyHolder& holder);
};

struct IceProperties
{
	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;
	std::vector<IcePropertyHolder>	propDatas;

	static void structure(ZQ::common::Config::Holder<IceProperties>& holder);

    void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerIceProperty(const std::string &full_path);
};


struct SOPProp
{
	std::string		fileName;
	static void		structure(ZQ::common::Config::Holder<SOPProp>& holder);
};


//<PassThruStreaming enabled="1" >
//	<ImportChannel name="SEACHANGEF0022" bandwidth="100000000" maxImport="25" />
//	<ImportChannel name="SEACHANGEF0023" bandwidth="100000000" maxImport="25" />
//</PassThruStreaming>

struct ImportChannel
{
	std::string		name;
	int32			bandwidth; //config
	
	int64			maxBandwidth;
	int64			usedBandwidth;

	int64			reportUsedBandwidth;
	int64			reportTotalBandwidth;//total bandwidth reported from streaming service

	int32			runningImportSessCount;//running import session count reported from streaming service

	int32			maxImport;
	int32			usedImport;	

	bool			_bConfiged;//identify if this import channel comes from config or reported by stream service
	
	ImportChannel( )
	{
		reportUsedBandwidth			= 0;
		runningImportSessCount		= 0;
		reportTotalBandwidth		= 0;	
		_bConfiged					= true;		
	}

	static void structure(ZQ::common::Config::Holder<ImportChannel>& holder);
};

struct PassThruStreaming
{
	int32					enable;	
	int32					testMode;	
	int32					muteStat;
	int32					excludeC2Down;

	PassThruStreaming()
	{
		enable	    = 1;		
		testMode	= 0;		
		muteStat	= 0;//0 means enable query import channel statistics
		excludeC2Down = 0;
	}

	typedef ZQ::common::Config::Holder<ImportChannel> ImportChannelHolder;
	std::map<std::string,ImportChannelHolder>		importChannelDatas;

	static void structure(ZQ::common::Config::Holder<PassThruStreaming>& holder);

    void readImportChannel(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerImportChannel(const std::string &full_path);
};

//<LAM enableWarmup="1">
//	<TestMode enabled="0" contentName="12" playlistSize="3" enableRemote="0" remoteURL="" urlCount="3"/>
//	<LAMServer volumeName="TEST" endpoint="tcp -h 10.15.10.32 -p 20089" />
//	...
//	<LAMServer volumeName="TEST2" endpoint="tcp -h 10.15.10.32 -p 20089" />
//</LAM>

struct LAMTestModeSubItemVolume
{
	std::string			itemVolume;
	static void structure(ZQ::common::Config::Holder<LAMTestModeSubItemVolume>& holder);
};

struct LAMTestModeSubItemUrl
{
	std::string			itemUrl;
	static void structure(ZQ::common::Config::Holder<LAMTestModeSubItemUrl>& holder);
};

struct LAMSubTestContentAttribute 
{
	std::string key;
	std::string value;
	static void structure( ZQ::common::Config::Holder<LAMSubTestContentAttribute>& holder );
};

struct LAMSubTestSet
{
	std::string				contentName;
	std::string				pid;
	int32					bandwidth;	//KBPS
	int32					cueIn;
	int32					cueOut;
	std::vector<std::string>	urls;
	std::vector<std::string>	volumeList;	
	std::map<std::string,std::string> attrs;

	static void structure(ZQ::common::Config::Holder<LAMSubTestSet>& holder);

	void readUrls(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readVolumeList(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readAttributes(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerNothing( const std::string &full_path ){}
};

struct LAMTestMode
{	
	int32					enabled;
	typedef ZQ::common::Config::Holder<LAMSubTestSet> LAMSubTestSetHolder;
	typedef std::vector< LAMSubTestSetHolder >	LAMSubTestSetHolderS;
	LAMSubTestSetHolderS	subTests;	

	static void structure(ZQ::common::Config::Holder<LAMTestMode>& holder);
	void readSubTestSet(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerNothing( const std::string &full_path ){}
};

struct ContentLibMode
{	
	int32					enabled;
	std::string             name;
	std::string             endpoint;

	static void structure(ZQ::common::Config::Holder<ContentLibMode>& holder);
};

struct ContentVolume 
{
	std::string		name;
	int32			cache;	//if this value is 1, current volume can be selected when rtsp message 's volume== library
	int32			cacheLevel;
	int32			supportNasStreaming;

	std::string		netId;
	std::set<std::string> volumeName;
	bool			bAllVolumeAvailable;

	std::string		endpoint;//ContentVolume ICE Endpoint
	typedef ZQ::common::Config::Holder<ContentVolume> ContentVolumeHolder;
	static void structure( ContentVolumeHolder& holder );
};

struct LAMServer
{
	std::string				volumeName;
	std::string				endpoint;
	com::izq::am::facade::servicesForIce::LAMFacadePrx	lamPrx;

	typedef std::map<std::string , ContentVolume::ContentVolumeHolder > ContentVolumeHolderS;
	ContentVolumeHolderS	contentVolumes;

	typedef ZQ::common::Config::Holder<LAMServer> LAMServerHolder;

	static void structure(LAMServerHolder& holder);

	void readContentVolume( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void registerContentVolume( const std::string &full_path );
};


struct LAM
{
	
	int32										enableWarmup;
	std::string                                 contentNameFmt;
	ZQ::common::Config::Holder<LAMTestMode>		lamTestMode;
	ZQ::common::Config::Holder<ContentLibMode>  contentLibMode;
	typedef LAMServer::LAMServerHolder			LAMServerHolder;	
	LAMServerHolder								lamServer;

	static void structure(ZQ::common::Config::Holder<LAM>& holder);

    void readTestMode(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerTestMode(const std::string &full_path);

	void readContentLibMode(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerContentLibMode(const std::string &full_path);

    void readLAMServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerLAMServer(const std::string &full_path);
};


// struct Library
// {
// 	//if this value is true 
// 	//enable using static library url which is configured at _baseUrl
// 	//if this vale is false
// 	//discard the _baseUrl configuration value
// 	int32			_staticed;
// 	std::string		_urlTemplate;
// 
// 	static void structure(ZQ::common::Config::Holder<Library>& holder);
// };

struct PlaylistControl 
{
	int32		enableEOT;
	int32       ignoreAbsentItems;
	
	int32		ignoreDestMac;

	int32		minClipDuration;
	int32		nptByPrimary;

	static void structure(ZQ::common::Config::Holder<PlaylistControl>& holder);
};

// <protocolVersioning enable="1" />
struct ProtocolVersioning
{
	int32		enableVersioning;
	std::string customer;

	static void structure(ZQ::common::Config::Holder<ProtocolVersioning>& holder);
};

struct SessionHistory
{
	int32 enableHistory;
	int32 enablePlayEvent;
	int32 enablePauseEvent;
	static void structure(ZQ::common::Config::Holder<SessionHistory>& holder);
};

struct AdsProvider 
{
	std::string		pid;

	typedef ZQ::common::Config::Holder<AdsProvider> AdsProviderHolder;
	static void structure( AdsProviderHolder& holder );
};

struct AdsReplacment
{
	int32 enabled;
	int32 provideLeadingAdsPlaytime;
	std::string defaultTrickRestriction;
	int32 adsRestricts;
	int32 playOnce;
	AdsReplacment()
		:adsRestricts(0)
	{
	}

	typedef std::vector< AdsProvider::AdsProviderHolder > AdsProviderHolderS;
	AdsProviderHolderS	providers;

	static void structure(ZQ::common::Config::Holder<AdsReplacment>& holder);

	void readProvider( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void registerProvider( const std::string &full_path );
};

struct Config
{
	ZQ::common::Config::Holder<LogFile>					pluginLog;
	ZQ::common::Config::Holder<EventLogFile>			eventLog;
	ZQ::common::Config::Holder<EventChannelConf>		iceStorm;
	ZQ::common::Config::Holder<Bind>					bind;
	ZQ::common::Config::Holder<RTSPSession>				rtspSession;
	ZQ::common::Config::Holder<Database>				database;
	ZQ::common::Config::Holder<Announce>				announce;
	ZQ::common::Config::Holder<Response>				response;
	ZQ::common::Config::Holder<PublishLogs>				publishLogs;
	ZQ::common::Config::Holder<IceProperties>			iceProps;
 
	ZQ::common::Config::Holder<SOPProp>					sopProp;

	ZQ::common::Config::Holder<PassThruStreaming>		passThruStreaming;
	ZQ::common::Config::Holder<LAM>						lam;	
	ZQ::common::Config::Holder<PlaylistControl>			playlistControl;
	ZQ::common::Config::Holder<ProtocolVersioning>		protocolVersioning;
	ZQ::common::Config::Holder<MessageFormat>			messageFmt;

	ZQ::common::Config::Holder<SessionHistory>			sessionHistory;
	ZQ::common::Config::Holder<D5MessageConf>			d5messsage;
	ZQ::common::Config::Holder<AssetStack>				assetStack;
	ZQ::common::Config::Holder<AdsReplacment>			adsReplacment;

	static void structure(ZQ::common::Config::Holder<Config>& holder);

	void readEventLogFile( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void readAssetStack( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void registerAssetStack( const std::string &full_path );

	void readD5Message( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void readLibrary(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerLibrary(const std::string &full_path);

	void readLAM(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerLAM(const std::string &full_path);

	void readPassThruStreaming(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerPassThruStreaming(const std::string &full_path);

	void readIceProperties(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIceProperties(const std::string &full_path);

	void readPublishLogs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerPublishLogs(const std::string &full_path);

	void readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerResponse(const std::string &full_path);

	void readAnnounce(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerAnnounce(const std::string &full_path);

	void readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerDatabase(const std::string &full_path);

	void readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerRTSPSession(const std::string &full_path);

	void readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerBind(const std::string &full_path);

	void readIceStorm(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIceStorm(const std::string &full_path);

	void readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerLogFile(const std::string &full_path);
	
	void readPlaylistControl(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerPlaylistControl(const std::string &full_path);

	void readProtocolVersioning(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerProtocolVersioning(const std::string &full_path);

	void readSessionHistory(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSessionHistory(const std::string &full_path);

	void readAdsReplacment(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerAdsReplacment(const std::string &full_path);
	
	void readMessageFormat( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void readSOPProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSOPProp(const std::string &full_path);

	
	void regsiterNone( const std::string &full_path );

};

}; // namespace NGOD2

extern ZQ::common::Config::Loader<NGOD::Config> ngodConfig;

#endif //__tianshan_ngod_config_header_file_h__


