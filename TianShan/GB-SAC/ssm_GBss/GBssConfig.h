// File Name : GBssConfig.h

#ifndef __EVENT_IS_VODI5_CONFIGURATION_H__
#define __EVENT_IS_VODI5_CONFIGURATION_H__

#include "ConfigHelper.h"

#include <set>

/// default means value will be applied if configuration isn't provisioned in XML file

namespace GBss
{

// default : <EventChannel endpoint=""/> 
struct EventChannel
{
	std::string _endpoint;

	typedef ZQ::common::Config::Holder<EventChannel> EventChannelHolder;

	static void structure(EventChannelHolder& holder);
};
typedef EventChannel::EventChannelHolder EventChannelHolder;

// default : <IceTrace enabled="0" level="6" size="10000000" maxCount="5" />
struct IceTrace
{
	int32 _enabled;
	int32 _level;
	int32 _size;
	int32 _maxCount;

	typedef ZQ::common::Config::Holder<IceTrace> IceTraceHolder;

	static void structure(IceTraceHolder& holder);
};
typedef IceTrace::IceTraceHolder IceTraceHolder;

// default : <prop name="" value="" />
struct IceProperty
{
	std::string _name;
	std::string _value;

	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;

	static void structure(IcePropertyHolder& holder);
};
typedef IceProperty::IcePropertyHolder IcePropertyHolder;

struct IcePropertys
{
	std::vector<IcePropertyHolder> _propDatas;

	typedef ZQ::common::Config::Holder<IcePropertys> IcePropertysHolder;

	static void structure(IcePropertysHolder& holder);

	void readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIcePropertys(const std::string &full_path);
};
typedef IcePropertys::IcePropertysHolder IcePropertysHolder;

// default <Database path="" runtimePath="" />
struct Database
{
	std::string _path;
	std::string _runtimePath;

	typedef ZQ::common::Config::Holder<Database> DatabaseHolder;

	static void structure(DatabaseHolder& holder);
};
typedef Database::DatabaseHolder DatabaseHolder;

//-----------------------------------------------------------------------------------------------

// default <LogFile size="10000000" level="6" maxCount="5" bufferSize="16000" />
struct LogFile
{
	int32 _size;
	int32 _level;
	int32 _maxCount;
	int32 _bufferSize;

	typedef ZQ::common::Config::Holder<LogFile> LogFileHolder;

	static void structure(LogFileHolder& holder);
};
typedef LogFile::LogFileHolder LogFileHolder;

struct PublishLog
{
	std::string _path;
	std::string _syntax;
	std::string _key;

	typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;

	static void structure(PublishLogHolder& holder);
};
typedef PublishLog::PublishLogHolder PublishLogHolder;

struct PublishLogs
{
	std::vector<PublishLogHolder> _logDatas;

	typedef ZQ::common::Config::Holder<PublishLogs> PublishLogsHolder;
	static void structure(PublishLogsHolder& holder);

	void readPublishLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerPublishLog(const std::string &full_path);
};
typedef PublishLogs::PublishLogsHolder PublishLogsHolder;

// default <Bind serviceName="ListenEventAdapter" endpoint="" />
struct Bind
{
	std::string _serviceName;
	std::string _endpoint;

	typedef ZQ::common::Config::Holder<Bind> BindHolder;
	static void structure(BindHolder& holder);
};
typedef Bind::BindHolder BindHolder;

struct RTSPSession
{
	int32 _cacheSize;
	int32 _monitorThreads;
	int32 _timeout;
	int32 _timeoutCount;
	int32 _maxDestroyInterval;
	RTSPSession()
	{
		_maxDestroyInterval = 3600;
	}

	typedef ZQ::common::Config::Holder<RTSPSession> RTSPSessionHolder;
	static void structure(RTSPSessionHolder& holder);
};
typedef RTSPSession::RTSPSessionHolder RTSPSessionHolder;

// <playlistControl enableEOT="0" ignoreDestMac="0"/>
struct PlaylistControl 
{
	int32 _enableEOT;
	int32 _ignoreDestMac;

	typedef ZQ::common::Config::Holder<PlaylistControl> PlaylistControlHolder;
	static void structure(PlaylistControlHolder& holder);
};
typedef PlaylistControl::PlaylistControlHolder PlaylistControlHolder;


//<RequestAdjust defaultPAID="" />
struct RequestAdjust
{
	std::string _defaultPID;

	typedef ZQ::common::Config::Holder<RequestAdjust> RequestAdjustHolder;
	static void structure(RequestAdjustHolder& holder);
};
typedef RequestAdjust::RequestAdjustHolder RequestAdjustHolder;

// <Response setupFailureWithSessId="0"/>
struct Response
{
	int32 _setupFailureWithSessId;

	typedef ZQ::common::Config::Holder<Response> ResponseHolder;
	static void structure(ResponseHolder& holder);
};
typedef Response::ResponseHolder ResponseHolder;

// <Announce useGlobalCSeq="0"/>
struct Announce
{
	int32 _useGlobalCSeq;
	int32 _SRMEnabled;
	int32 _STBEnabled;

	typedef ZQ::common::Config::Holder<Announce> AnnounceHolder;
	static void structure(AnnounceHolder& holder);
};
typedef Announce::AnnounceHolder AnnounceHolder;

struct ImportChannel
{
	// configuration
	std::string _name;
	int32 _bandwidth; 
	int32 _maxImport;

	typedef ZQ::common::Config::Holder<ImportChannel> ImportChannelHolder;
	static void structure(ImportChannelHolder& holder);
};
typedef ImportChannel::ImportChannelHolder ImportChannelHolder;

//<PassThruStreaming enabled="1" >
//	<ImportChannel name="SEACHANGEF0022" bandwidth="100000000" maxImport="25" />
//	<ImportChannel name="SEACHANGEF0023" bandwidth="100000000" maxImport="25" />
//</PassThruStreaming>
struct PassThruStreaming
{
	std::vector<ImportChannelHolder> _importChannelDatas;

	typedef ZQ::common::Config::Holder<PassThruStreaming> PassThruStreamingHolder;
	static void structure(PassThruStreamingHolder& holder);

	void readImportChannel(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerImportChannel(const std::string &full_path);
};
typedef PassThruStreaming::PassThruStreamingHolder PassThruStreamingHolder;


struct SourceStreamers
{
	std::string _fileName;
	typedef ZQ::common::Config::Holder<SourceStreamers> SourceStreamersHolder;
	static void structure(SourceStreamersHolder& holder);
};
typedef SourceStreamers::SourceStreamersHolder SourceStreamersHolder;



//<LAM enableWarmup="1">
//	<TestMode enabled="0" contentName="12" playlistSize="3" enableRemote="0" remoteURL="" urlCount="3"/>
//	<LAMServer volumeName="TEST" endpoint="tcp -h 10.15.10.32 -p 20089" />
//	...
//	<LAMServer volumeName="TEST2" endpoint="tcp -h 10.15.10.32 -p 20089" />
//</LAM>
struct LAMTestModeSubItemVolume
{
	std::string _itemVolume;

	typedef ZQ::common::Config::Holder<LAMTestModeSubItemVolume> LAMTestModeSubItemVolumeHolder;
	static void structure(LAMTestModeSubItemVolumeHolder& holder);
};
typedef LAMTestModeSubItemVolume::LAMTestModeSubItemVolumeHolder LAMTestModeSubItemVolumeHolder;

struct LAMTestModeSubItemUrl
{
	std::string _itemUrl;

	typedef ZQ::common::Config::Holder<LAMTestModeSubItemUrl> LAMTestModeSubItemUrlHolder;
	static void structure(LAMTestModeSubItemUrlHolder& holder);
};
typedef LAMTestModeSubItemUrl::LAMTestModeSubItemUrlHolder LAMTestModeSubItemUrlHolder;

struct LAMSubTestSet
{
	std::string _contentName;
	int32 _bandwidth;	//KBPS
	int32 _cueIn;
	int32 _cueOut;
	std::vector<std::string> _urls;
	std::vector<std::string> _volumeList;	

	typedef ZQ::common::Config::Holder<LAMSubTestSet> LAMSubTestSetHolder;
	static void structure(LAMSubTestSetHolder& holder);

	void readUrls(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readVolumeList(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerNothing(const std::string &full_path){}
};
typedef LAMSubTestSet::LAMSubTestSetHolder  LAMSubTestSetHolder;

struct LAMTestMode
{	
	int32 _enabled;
	typedef std::vector<LAMSubTestSetHolder> LAMSubTestSetHolderS;
	LAMSubTestSetHolderS _subTests;

	typedef ZQ::common::Config::Holder<LAMTestMode> LAMTestModeHolder;
	static void structure(LAMTestModeHolder& holder);
	void readSubTestSet(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerNothing( const std::string &full_path ){}
};
typedef LAMTestMode::LAMTestModeHolder LAMTestModeHolder;

struct ContentVolume 
{
	// configuration 
	std::string _name;
	int32 _cacheLevel;
	int32 _supportNasStreaming;

	typedef ZQ::common::Config::Holder<ContentVolume> ContentVolumeHolder;
	static void structure(ContentVolumeHolder& holder);
};
typedef ContentVolume::ContentVolumeHolder ContentVolumeHolder;


struct LAMServer
{
	typedef std::vector<ContentVolumeHolder> ContentVolumeHolderS;
	ContentVolumeHolderS _contentVolumes;

	typedef ZQ::common::Config::Holder<LAMServer> LAMServerHolder;

	static void structure(LAMServerHolder& holder);

	void readContentVolume( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP );

	void registerContentVolume( const std::string &full_path );
};
typedef LAMServer::LAMServerHolder LAMServerHolder;
typedef LAMServer::ContentVolumeHolderS ContentVolumeHolderS;

struct AssetManagement
{
	std::string _nasurlPrefix;
	LAMTestModeHolder _lamTestMode;
	LAMServerHolder _lamServer;

	typedef ZQ::common::Config::Holder<AssetManagement> AssetManagementHolder;
	static void structure(AssetManagementHolder& holder);

	void readTestMode(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerTestMode(const std::string &full_path);

	void readLAMServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerLAMServer(const std::string &full_path);
};
typedef AssetManagement::AssetManagementHolder AssetManagementHolder;


struct GBssConfig
{
	EventChannelHolder _iceStorm;
	IceTraceHolder _iceTrace;
	IcePropertysHolder _iceProps;
	DatabaseHolder _database;

	LogFileHolder _pluginLog;
	PublishLogsHolder _publishLogs;
	BindHolder _bind;
	RTSPSessionHolder _rtspSession;
	PlaylistControlHolder _PlaylistControl;
	RequestAdjustHolder _requestAdjust;
	ResponseHolder _response;
	AnnounceHolder _announce;
	SourceStreamersHolder _sourceStreamers;


	PassThruStreamingHolder _passThruStreaming;
	AssetManagementHolder _lam;

	typedef ZQ::common::Config::Holder<GBssConfig> ISVodConfigHolder;

	static void structure(ISVodConfigHolder& holder);

	void readIceStrom(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIceStrom(const std::string &full_path);

	void readIceTrace(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIceTrace(const std::string &full_path);

	void readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIcePropertys(const std::string &full_path);

	void readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerDatabase(const std::string &full_path);
	
	//------------------------------------------------------------------

	void readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerLogFile(const std::string &full_path);

	void readPublishLogs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerPublishLogs(const std::string &full_path);

	void readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerBind(const std::string &full_path);

	void readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerRTSPSession(const std::string &full_path);

	void readPlaylistControl(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerPlaylistControl(const std::string &full_path);

	void readRequestAdjust(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerRequestAdjust(const std::string &full_path);

	void readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerResponse(const std::string &full_path);

	void readAnnounce(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerAnnounce(const std::string &full_path);

	void readPassThruStreaming(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerPassThruStreaming(const std::string &full_path);

	void readSourceStreamers(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSourceStreamers(const std::string &full_path);

	void readLAM(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerLAM(const std::string &full_path);

};

} // end GBss

extern ZQ::common::Config::Loader<GBss::GBssConfig> _GBssConfig;

#endif // end __EVENT_IS_VODI5_CONFIGURATION_H__
