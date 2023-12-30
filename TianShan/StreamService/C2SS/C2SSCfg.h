#ifndef _C2SS_CFG_H
#define _C2SS_CFG_H
#pragma once

#include "ConfigHelper.h"
//#include "FileLog.h"

namespace ZQTianShan{
	namespace C2SS{
using namespace ZQ::common;
//publiseh log
struct PublishLog
{
	std::string _path;
	std::string _syntax;
	std::string _type;
	std::string _key;
	static void structure(ZQ::common::Config::Holder<PublishLog>& holder) ;
};

struct PublishLogs
{
	typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;
	std::vector<PublishLog> _logDatas;
	static void structure(ZQ::common::Config::Holder<PublishLogs>& holder);
	void readPublishLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) ;
	void registerPublishLog(const std::string &full_path) {}
};

//SessionHistory
struct SessionHistory
{
	int32	enable;
	::std::string	path;
	int32			level;
	int32			maxCount;
	int32			size;
	int32			bufferSize;
	int32			flushTimeout;
	typedef ZQ::common::Config::Holder<SessionHistory> SessionHistoryHolder;
	static void structure(ZQ::common::Config::Holder< SessionHistory > &holder);
};

//postEvent
struct PostEvent
{
	PostEvent()
	{
		enableScaleChangeEvent = 0;
		enableStateChangeEvent = 0;
		passScaleChangeEvent	= 1;
		passStateChangeEvent	= 1;
	}
	int32	enableScaleChangeEvent;
	int32	enableStateChangeEvent;
	int32	passScaleChangeEvent;
	int32	passStateChangeEvent;
	typedef ZQ::common::Config::Holder<PostEvent> PostEventHolder;
	static void structure(ZQ::common::Config::Holder< PostEvent > &holder);
};

//DatabasePerfTune
struct streamDatabasePerftune
{
	streamDatabasePerftune()
	{
		databaseStreamCheckpointPeriod = "120";
		databaseStreamSavePeriod="60000";
		databaseStreamSaveSizeTrigger="100";
	}
	std::string databaseStreamCheckpointPeriod;
	std::string databaseStreamSavePeriod;
	std::string databaseStreamSaveSizeTrigger;	
	typedef ZQ::common::Config::Holder<streamDatabasePerftune> streamDatabasePerftuneHolder;

	static void structure( ZQ::common::Config::Holder< streamDatabasePerftune >& holder );
};

//SessionInterface
struct SessionInterface
{
	//session interface
	std::string		SessionInterfaceIp;
	int32			SessionInterfacePort;
	std::string		SessionInterfaceBind;
	int32			SessionInterfaceMaxSessionGroup;
	int32			SessionInterfaceMaxSessionsPerGroup;
	int32			SessionInterfaceRequestTimeout;
	int32			SessionInterfaceDisconnectAtTimeout;
	int32			SessionInterfaceTryDecimalNpt;

	//FixedSpeedSet
	int32				FixedSpeedSetEnable;
	int32				EnableFixedSpeedLoop; // default is 1

	std::string			FixedSpeedSetForward;
	std::string			FixedSpeedSetBackward;

	std::vector<float>	FixedSpeedSetForwardSet;
	std::vector<float>	FixedSpeedSetBackwardSet;

	typedef ZQ::common::Config::Holder<SessionInterface> SessionInterfaceHolder;

	static void structure(SessionInterfaceHolder &holder);
};

//Vol
typedef struct Vol
{
	std::string		mount;
	std::string		targetName;
	int32			defaultVal;//1=true,0=false;
	int32			defaultBitRate;

	typedef ZQ::common::Config::Holder< Vol > VolHolder;
	static void structure(VolHolder &holder);	
}Vol;

//ContentInterface
struct ContentInterface
{
	//content interface
	std::string		ContentInterfaceIp;
	int32			ContentInterfacePort;
	std::string		ContentInterface2ndIp;
	int32			ContentInterface2ndPort;
	std::string		ContentInterfacePath;
	int32			ContentInterfaceSyncInterval;
	int32			ContentInterfaceSyncRetry;
	std::string		ContentInterfaceMode;
	int32			ContentInterfaceHttpTimeOut;
	int32			ContentInterfaceDestroyEnable;
	int32			urlPercentDecodeOnOutgoingMsg;

	//<Feedback>
	std::string		FeedbackIp;
	int32			FeedbackPort;

	//<StoreReplica>
	std::string		StoreReplicaGroupId;
	std::string		StoreReplicaReplicaId;
	int32			StoreReplicaReplicaPriority;
	int32			StoreReplicaTimeout;

	//<DatabaseCache>
	int32	DatabaseCacheVolumeSize;
	int32	DatabaseCacheContentSize;
	int32	DatabaseCacheContentSavePeriod;
	int32	DatabaseCacheContentSaveSizeTrigger;

	typedef ZQ::common::Config::Holder<ContentInterface> ContentInterfaceHolder;
	typedef ::std::vector<Vol::VolHolder> VolList;
	VolList vols;

	static void structure(ContentInterfaceHolder &holder);
	void readVol(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerVol(const std::string &full_path);

};

//VideoServer
struct VideoServer
{
	//VideoServer
	std::string		vendor;
	std::string		model;
	int32			enableMessageBinaryDump;
	int32			streamSyncInterval;
	int32			sessionRenewInterval;
	int32			bSeperateIPStreaming;
	std::string		libraryVolume;

	typedef ZQ::common::Config::Holder<VideoServer> VideoServerHolder;

	streamDatabasePerftune::streamDatabasePerftuneHolder   streamDbPerfTune;
	SessionInterface::SessionInterfaceHolder					sessionInterfaceHolder;
	ContentInterface::ContentInterfaceHolder					contentInterfaceHolder;

	static void structure(VideoServerHolder &holder);

	void readStreamDatabasePerftune(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerStreamDatabasePerftune(const std::string &full_path);
	void readSessionInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerSessionInterface(const std::string &full_path);
	void readContentInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerContentInterface(const std::string &full_path);
}; //struct VideoServer

//SC2SS
struct ST_C2SS  
{
	ST_C2SS()
	{
		_syncCall =0;
		_syncTimeOut = 1000;
	}
	//ST_C2SS 
	typedef ::ZQ::common::Config::Holder< PublishLogs >	PublishedLogHolder;

	std::string	_netId;
	std::map<std::string, std::string> _icePropertiesMap;
	PublishedLogHolder		_publishedLog;
	std::string					_bindEndpoint;	
	int32			_bindDispatchSize;
	int32			_bindDispatchMax;	
	int32			_bindEvictorSize;
	int32			_bindThreadPoolSize;
	int32			_bindPendingMax;
	int32			_bindContentStoreThreadPoolSize;
	int32			_c2ContentTimeout;
	int32			_c2ContentManagerSize;
	int32			_c2ContentThreadSize;
	int32			_c2ContentCacheSize;

	std::string     _httpCRGUpStreamIP;
	std::string		_httpCRGAddr;
	int32			_httpCRGPort;
	std::string		_httpCRGURL;
	std::string		_httpCRGServerIP;
	int32            _httpCRGDefaultGetPort;
	int32			_syncCall;
	int32			_syncTimeOut;
	SessionHistory::SessionHistoryHolder			_sessHistoryHolder;
	PostEvent::PostEventHolder					_postEventHolder;
	VideoServer::VideoServerHolder				_videoServerHolder;

	typedef ZQ::common::Config::Holder<ST_C2SS> C2SSHolder;

	static void structure(C2SSHolder &holder);

	void registerNothing(const std::string&){}
	void readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readPublishedLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerPublishedLog(const std::string &full_path);
	void readSessionHistory( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
	void registerSessionHistory(const std::string& full_path );
	void readPostEvent( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
	void registerPostEvent(const std::string& full_path );
	void readVideoServer( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
	void registerVideoServer(const std::string& full_path );

};

//C2SSCfg
class C2SSCfg
{
public:
	C2SSCfg();
	~C2SSCfg(void);

	static void structure(Config::Holder<C2SSCfg> &holder);
	void registerNothing(const std::string&){}
	void readC2SS( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP );
	void registerC2SS(const std::string& full_path );
public:
	//default in xml 
	char	_szCrashDumpPath[256];
	int32   _crashDumpEnabled; 
	int32	_iceTraceLogEnabled;
	int32	_iceTraceLogLevel;
	int32	_iceTraceLogSize;
	char	_eventChannelEndpoint[256];
	char	_szIceDbFolder[256];
	char     _szIceRuntimeDbFolder[256];

	typedef ::std::vector<ST_C2SS::C2SSHolder> C2SSList;
	C2SSList _c2ssList;
};

	}//namespace C2SS
} // namespace ZQTianShan

#endif 