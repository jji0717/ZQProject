#ifndef __Ngod2Config_H__
#define __Ngod2Config_H__

#include "ConfigHelper.h"
#include <string>
#include <map>
#include <set>
#include "StreamSmithAdmin.h"
#include "LAMFacade.h"

namespace NGOD2
{

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
	std::string		_path;
	int32			_size;
	int32			_level;
	int32			_maxCount;
	int32			_bufferSize;

	static void structure(ZQ::common::Config::Holder<LogFile>& holder);

};

// <IceStorm endpoint="${PrimeIceStormEndpoint}" />

struct IceStorm
{
	std::string _endpoint;

	static void structure(ZQ::common::Config::Holder<IceStorm>& holder);
};

// <Bind endpoint="tcp -h ${ServerNetIf} -p 5735" dispatchSize="5" dispatchMax="30"/>

struct Bind
{
	std::string _endpoint;
	int32 _dispatchSize;
	int32 _dispatchMax;

	static void structure(ZQ::common::Config::Holder<Bind>& holder);
};

//<RTSPSession timeout="600" cacheSize="1000" monitorThreads="5" defaultServiceGroup="5"  />

struct RTSPSession
{
	int32 _timeout;
	int32 _cacheSize;
	int32 _monitorThreads;
	int32 _defaultServiceGroup;

	// add by zjm to control numbers of send "session in progress"
	int32 _timeoutCount;

	static void structure(ZQ::common::Config::Holder<RTSPSession>& holder);
};

// <Database path="${TianShanDatabaseDir}" />

struct Database
{
	std::string _path;

	static void structure(ZQ::common::Config::Holder<Database>& holder);
};

// <Announce useGlobalCSeq="0"/>

struct Announce
{
	int32 _useGlobalCSeq;
	int32 _includeTransition;

	int32 _useTianShanAnnounceCodeStateChanged;
	int32 _useTianShanAnnounceCodeScaleChanged;

	static void structure(ZQ::common::Config::Holder<Announce>& holder);
};

// <Response setupFailureWithSessId="0"/>

struct Response
{
	int32 _setupFailureWithSessId;
	std::string _streamCtrlProt;

	static void structure(ZQ::common::Config::Holder<Response>& holder);
};

//<PublishedLogs>
//	<Log path="${TianShanLogDir}\ssm_ngod2.Log" syntax="${TianShanHomeDir}\etc\ssm_ngod2.xml" />
//</PublishedLogs>

struct PublishLog
{
	std::string _path;
	std::string _syntax;
	std::string _type;
	std::string _key;

	typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;

	static void structure(ZQ::common::Config::Holder<PublishLog>& holder);
};

struct PublishLogs
{
	typedef PublishLog::PublishLogHolder PublishLogHolder;
	std::vector<PublishLogHolder> _logDatas;

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
	std::string _name;
	std::string _value;

	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;

	static void structure(IcePropertyHolder& holder);
};

struct IceProperties
{
	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;
	std::vector<IcePropertyHolder> _propDatas;

	static void structure(ZQ::common::Config::Holder<IceProperties>& holder);

    void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerIceProperty(const std::string &full_path);
};


//<SOPRestriction enableWarmup="1" retryCount="3" maxPenaltyValue="5">
//	<sop name="TEST" serviceGroup="30">
//		<streamer netId="10.15.10.250/BoardNumber0" 
//			serviceEndpoint="DummySS:tcp -h 192.168.81.102 -p 21000" 
//			volume="TEST"	 totalBW="12000" 		maxStream="50"
//			importChannel = "SEACHANGEF0022" />
//		<streamer netId="10.15.10.250/BoardNumber1" 
//			serviceEndpoint="DummySS:tcp -h 192.168.81.102 -p 21000" 
//			volume="TEST" 	totalBW="12000" 		maxStream="20"
//			importChannel = "SEACHANGEF0023"	/>
//		<streamer netId="10.15.10.250/BoardNumber2" 
//			serviceEndpoint="DummySS:tcp -h 192.168.81.102 -p 21000" 
//			volume="TEST" 	totalBW="12000" 	maxStream="10"
//			importChannel = "SEACHANGEF0022" 	/>		
//	</sop>
//</SOPRestriction>



struct SOPProp
{
	std::string fileName;
	static void structure(ZQ::common::Config::Holder<SOPProp>& holder);

};


//<PassThruStreaming enabled="1" >
//	<ImportChannel name="SEACHANGEF0022" bandwidth="100000000" maxImport="25" />
//	<ImportChannel name="SEACHANGEF0023" bandwidth="100000000" maxImport="25" />
//</PassThruStreaming>

struct ImportChannel
{
	std::string		_name;
	int32			_bandwidth; //config
	
	int64			_maxBandwidth;
	int64			_usedBandwidth;

	int64			_reportUsedBandwidth;
	int64			_reportTotalBandwidth;//total bandwidth reported from streaming service

	int32			_runningImportSessCount;//running import session count reported from streaming service

	int32			_maxImport;
	int32			_usedImport;

	bool			_bConfiged;//identify if this import channel comes from config or reported by stream service
	
	ImportChannel( )
	{
		_reportUsedBandwidth		= 0;
		_runningImportSessCount		= 0;
		_reportTotalBandwidth		= 0;
		_bConfiged					= true;
	}

	static void structure(ZQ::common::Config::Holder<ImportChannel>& holder);
};

struct PassThruStreaming
{
	PassThruStreaming()
	{
		_testMode	= 0;
		_enabled	= 1;
		_muteStat	= 0;//0 means enable query import channel statistics
	}

	int32											_testMode;
	int32											_enabled;
	int32											_muteStat;
	typedef ZQ::common::Config::Holder<ImportChannel> ImportChannelHolder;
	std::map<std::string,ImportChannelHolder>		_importChannelDatas;

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

struct LAMSubTestSet
{
	std::string			contentName;
	int32				bandwidth;	//KBPS
	int32				cueIn;
	int32				cueOut;
	std::vector<std::string>	urls;
	std::vector<std::string>	volumeList;	

	static void structure(ZQ::common::Config::Holder<LAMSubTestSet>& holder);

	void readUrls(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readVolumeList(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerNothing( const std::string &full_path ){}
};

struct LAMTestMode
{	
	int32			_enabled;
	typedef ZQ::common::Config::Holder<LAMSubTestSet> LAMSubTestSetHolder;
	typedef std::vector< LAMSubTestSetHolder >	LAMSubTestSetHolderS;
	LAMSubTestSetHolderS	subTests;

	static void structure(ZQ::common::Config::Holder<LAMTestMode>& holder);
	void readSubTestSet(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerNothing( const std::string &full_path ){}
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
	std::string				_volumeName;
	std::string				_endpoint;
	com::izq::am::facade::servicesForIce::LAMFacadePrx	_lamPrx;

	typedef std::map<std::string , ContentVolume::ContentVolumeHolder > ContentVolumeHolderS;
	ContentVolumeHolderS	contentVolumes;

	typedef ZQ::common::Config::Holder<LAMServer> LAMServerHolder;

	static void structure(LAMServerHolder& holder);

	void readContentVolume( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void registerContentVolume( const std::string &full_path );
};


struct LAM
{
	//int32										_libMode;
	int32										_enableWarmup;
	ZQ::common::Config::Holder<LAMTestMode>		_lamTestMode;
	typedef LAMServer::LAMServerHolder			LAMServerHolder;
	//std::vector<LAMServerHolder>				_lamServerDatas;
	LAMServerHolder								_lamServer;

	static void structure(ZQ::common::Config::Holder<LAM>& holder);

    void readTestMode(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerTestMode(const std::string &full_path);

    void readLAMServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

    void registerLAMServer(const std::string &full_path);
};

//<Library staticed="1" urlTemplate=""/>

struct Library
{
	//if this value is true 
	//enable using static library url which is configured at _baseUrl
	//if this vale is false
	//discard the _baseUrl configuration value
	int32			_staticed;
	std::string		_urlTemplate;

	static void structure(ZQ::common::Config::Holder<Library>& holder);
};

struct PlaylistControl 
{
	int32		enableEOT;
	
	int32		ignoreDestMac;

	static void structure(ZQ::common::Config::Holder<PlaylistControl>& holder);
};

// <protocolVersioning enable="1" />
struct ProtocolVersioning
{
	int32 enableVersioning;

	static void structure(ZQ::common::Config::Holder<ProtocolVersioning>& holder);
};

struct SessionHistory
{
	int32 enableHistory;
	int32 enablePlayEvent;
	int32 enablePauseEvent;
	static void structure(ZQ::common::Config::Holder<SessionHistory>& holder);
};
struct Config
{
	ZQ::common::Config::Holder<LogFile> _pluginLog;
	ZQ::common::Config::Holder<IceStorm> _iceStorm;
	ZQ::common::Config::Holder<Bind> _bind;
	ZQ::common::Config::Holder<RTSPSession> _rtspSession;
	ZQ::common::Config::Holder<Database> _database;
	ZQ::common::Config::Holder<Announce> _announce;
	ZQ::common::Config::Holder<Response> _response;
	ZQ::common::Config::Holder<PublishLogs> _publishLogs;
	ZQ::common::Config::Holder<IceProperties> _iceProps;
 
	ZQ::common::Config::Holder<SOPProp> _sopProp;

	ZQ::common::Config::Holder<PassThruStreaming> _passThruStreaming;
	ZQ::common::Config::Holder<LAM> _lam;
	ZQ::common::Config::Holder<Library> _library;
	ZQ::common::Config::Holder<PlaylistControl> _PlaylistControl;
	ZQ::common::Config::Holder<ProtocolVersioning> _protocolVersioning;
	ZQ::common::Config::Holder<MessageFormat> _MessageFmt;

	ZQ::common::Config::Holder<SessionHistory> _sessionHistory;

	static void structure(ZQ::common::Config::Holder<Config>& holder);

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

	
	void readMessageFormat( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void readSOPProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSOPProp(const std::string &full_path);

	
	void regsiterNone( const std::string &full_path );

};

}; // namespace NGOD2

extern ZQ::common::Config::Loader<NGOD2::Config> _ngodConfig;

#endif // #define __Ngod2Config_H__

