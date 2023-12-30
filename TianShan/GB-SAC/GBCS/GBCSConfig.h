#ifndef __ZQTianShan_GBCSConfig_H__
#define __ZQTianShan_GBCSConfig_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>
#include <set>

namespace ZQTianShan{

namespace GBCS{

//root configuration

//global configuration

//default configuration
static std::string DefaultLayer = "default";
static std::string Slash ="/";
static std::string CrashDumpLayer = "CrashDump";

struct CrashDump
{
	std::string	path;
	int32			enabled;
	static void structure(ZQ::common::Config::Holder< CrashDump > &holder)
    {
		static std::string Layer = DefaultLayer + Slash + CrashDumpLayer;
		holder.addDetail(Layer.c_str(), "path", &CrashDump::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "enabled", &CrashDump::enabled, "", ZQ::common::Config::optReadOnly);
	}
};

static std::string IceTraceLayer = "IceTrace";
struct IceTrace
{
	int32	enabled;
	int32	level;
	int32	size;
	static void structure(ZQ::common::Config::Holder< IceTrace > &holder)
    {
		static std::string Layer = DefaultLayer + Slash + IceTraceLayer;
		holder.addDetail(Layer.c_str(), "enabled", &IceTrace::enabled, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &IceTrace::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &IceTrace::size, "", ZQ::common::Config::optReadOnly);
	}
};

static std::string IceStormLayer = "EventChannel";
struct IceStorm
{
	std::string	endPoint;
	static void structure(ZQ::common::Config::Holder< IceStorm > &holder)
	{
		static std::string Layer = DefaultLayer + Slash + IceStormLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &IceStorm::endPoint, NULL, ZQ::common::Config::optReadOnly);
	}
};

static std::string IcePropertiesLayer = "IceProperties";
static std::string PropLayer = std::string("prop");
struct IceProperties
{
	struct prop
	{
		std::string	name;
		std::string	value;
		static void structure(ZQ::common::Config::Holder< prop > &holder)
		{
			static std::string Layer = "";
			holder.addDetail(Layer.c_str(), "name", &prop::name, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "value", &prop::value,NULL, ZQ::common::Config::optReadOnly);
		}
	};
	typedef std::list< ZQ::common::Config::Holder<prop> > props;
	props _props;
	void readProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<prop> dptholder("");
        dptholder.read(node, hPP);
        _props.push_front(dptholder);
    }
    void registerProp(const std::string &full_path)
    {
        for(props::iterator it = _props.begin(); it != _props.end(); ++it)
            it->snmpRegister(full_path);
    }
    static void structure(ZQ::common::Config::Holder<IceProperties> &holder)
    {
		static std::string Layer = IcePropertiesLayer + Slash + PropLayer;
        holder.addDetail(Layer.c_str(), &IceProperties::readProp, &IceProperties::registerProp);
    }
};

static std::string DatabaseLayer = "Database";
struct Database
{
	std::string	path;
	std::string	runtimePath;
	static void structure(ZQ::common::Config::Holder< Database > &holder)
	{
		static std::string Layer = DefaultLayer + Slash + DatabaseLayer;
		holder.addDetail(Layer.c_str(), "path", &Database::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "runtimePath", &Database::runtimePath, NULL, ZQ::common::Config::optReadOnly);
	}
};

struct PublishLog
{
    std::string _path;
    std::string _syntax;
    std::string _type;
    std::string _key;

    static void structure(ZQ::common::Config::Holder<PublishLog>& holder) {
        holder.addDetail("", "path", &PublishLog::_path, "", ZQ::common::Config::optReadOnly);
        holder.addDetail("", "syntax", &PublishLog::_syntax, "", ZQ::common::Config::optReadOnly);
        holder.addDetail("","key",&PublishLog::_key,"");
        holder.addDetail("","type",&PublishLog::_type,"");
    }
};

struct PublishLogs
{
    typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;
    std::vector<PublishLog> _logDatas;

    static void structure(ZQ::common::Config::Holder<PublishLogs>& holder) {
        holder.addDetail("Log", &PublishLogs::readPublishLog, &PublishLogs::registerPublishLog);
    }

    void readPublishLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
        PublishLogHolder logHolder("path");
        logHolder.read(node, hPP);
        _logDatas.push_back(logHolder);
    }

    void registerPublishLog(const std::string &full_path) {}
};

//configuration of ngod_GBCS
static std::string GBCSLayer = "GBCS";

//LogFile
static std::string LogFileLayer = "LogFile";
struct LogFile
{
	std::string	path;
	int32			level;
	int32			maxCount;
	int32			size;
	int32			bufferSize;
	int32			flushTimeout;
	static void structure(ZQ::common::Config::Holder< LogFile > &holder)
	{
		static std::string Layer = GBCSLayer + Slash + LogFileLayer;
		holder.addDetail(Layer.c_str(), "path", &LogFile::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &LogFile::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &LogFile::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &LogFile::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &LogFile::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &LogFile::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//bind
static std::string gbcsBindLayer = "Bind";
struct stBind
{
	std::string	endPoint;
	int32			dispatchSize;
	int32			dispatchMax;	
	int32			evictorSize;
	int32			threadPoolSize;
	int32			pendingMax;
	int32			contentStoreThreadPoolSize;
	static void structure(ZQ::common::Config::Holder< stBind > &holder)
	{
		static std::string Layer = gbcsBindLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &stBind::endPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchSize", &stBind::dispatchSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchMax", &stBind::dispatchMax, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "pendingMax", &stBind::pendingMax, "100", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "evictorSize", &stBind::evictorSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "threadPoolSize", &stBind::threadPoolSize, "10", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "contentstoreThreadPoolSize", &stBind::contentStoreThreadPoolSize, "10", ZQ::common::Config::optReadOnly);
	}
};

static std::string gbcsIceLogLayer = "IceLog";
struct gbcsIceLog
{
	std::string path;
	int32	level;
	int32	maxCount;
	int32	size;
	int32	bufferSize;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< gbcsIceLog > &holder)
	{
		static std::string Layer = gbcsIceLogLayer;
		holder.addDetail(Layer.c_str(), "path", &gbcsIceLog::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &gbcsIceLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &gbcsIceLog::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &gbcsIceLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &gbcsIceLog::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &gbcsIceLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

static std::string ContentInterfaceLayer = "ContentInterface";
static std::string FeedbackLayer = ContentInterfaceLayer + Slash + "Feedback";
static std::string StoreReplicaLayer = ContentInterfaceLayer + Slash + "StoreReplica";
static std::string DatabaseCacheLayer = ContentInterfaceLayer + Slash + "DatabaseCache";
static std::string LogLayer = ContentInterfaceLayer + Slash + "Log";
static std::string EventLogLayer = ContentInterfaceLayer + Slash + "EventLog";
static std::string VolumesLayer = ContentInterfaceLayer + Slash + "Volumes";
static std::string VolLayer = VolumesLayer + Slash + "Vol";
static std::string VideoServer = ContentInterfaceLayer + Slash + "VideoServer";

struct ContentInterface
{
	//content interface
	std::string	ContentInterfaceIp;
	int32			ContentInterfacePort;
	std::string	ContentInterfacePath;
	int32			ContentInterfaceSyncInterval;
	int32			ContentInterfaceSyncRetry;
	std::string	ContentInterfaceMode;
	int32			ContentInterfaceHttpTimeOut;
	int32			ContentInterfaceDestroyEnable;
	int32			ContentInterfaceTestEnable;
	std::string   ContentInterfaceTestFolder;
	int32			urlPercentDecodeOnOutgoingMsg;

	//<Feedback>
	std::string	FeedbackIp;
	int32			FeedbackPort;

	//<StoreReplica>
	std::string	StoreReplicaGroupId;
	std::string	StoreReplicaReplicaId;
	int32			StoreReplicaReplicaPriority;
	int32			StoreReplicaTimeout;

	//<DatabaseCache>
	int32	DatabaseCacheVolumeSize;
	int32	DatabaseCacheContentSize;
	int32	DatabaseCacheContentSavePeriod;
	int32	DatabaseCacheContentSaveSizeTrigger;

	// <VideoServer>
	std::string	VideoServerId;
	std::string VideoOnDemandVersion;

	typedef struct Vol
	{
		std::string	mount;
		std::string	targetName;
		int32			defaultVal;//1=true,0=false;
		int32			defaultBitRate;

		typedef ZQ::common::Config::Holder< Vol > VolHolder;
		static void structure(VolHolder &holder)
		{
			static std::string Layer = "";
			holder.addDetail(Layer.c_str(), "mount", &Vol::mount, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "targetName", &Vol::targetName, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "default", &Vol::defaultVal, "1", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "defaultBitRate", &Vol::defaultBitRate, "37500000", ZQ::common::Config::optReadOnly);
		}		
	}Vol;

	typedef std::vector<Vol::VolHolder> VolList;
	VolList vols;

	typedef ZQ::common::Config::Holder<ContentInterface> VideoServerHolder;
	void readVol(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		Vol::VolHolder volHolder("");
		volHolder.read(node, hPP);
		vols.push_back(volHolder);
	}
	void registerVol(const std::string &full_path)
	{
		for (VolList::iterator it = vols.begin(); it != vols.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}


	struct GbcsStreamer 
	{
		std::string			streamerName;
		static void structure( ZQ::common::Config::Holder<GbcsStreamer>& holder )
		{
			holder.addDetail( "" , "name",&GbcsStreamer::streamerName,"",ZQ::common::Config::optReadOnly );
		}
	};
	typedef ZQ::common::Config::Holder<GbcsStreamer>		GbcsStreamerHolder;

	typedef std::vector< GbcsStreamerHolder >		GbcsStreamerHolderSet;
	GbcsStreamerHolderSet	streamerSet;

	static void structure(VideoServerHolder &holder)
	{
		holder.addDetail(ContentInterfaceLayer.c_str(), "ip", &ContentInterface::ContentInterfaceIp, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "port", &ContentInterface::ContentInterfacePort, "8080", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "path", &ContentInterface::ContentInterfacePath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "syncInterval", &ContentInterface::ContentInterfaceSyncInterval, "200000", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "syncRetry", &ContentInterface::ContentInterfaceSyncRetry, "3", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "mode", &ContentInterface::ContentInterfaceMode, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "httpTimeOut", &ContentInterface::ContentInterfaceHttpTimeOut, "200000", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "destroyEnable", &ContentInterface::ContentInterfaceDestroyEnable, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "testEnable", &ContentInterface::ContentInterfaceTestEnable, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "testFolder", &ContentInterface::ContentInterfaceTestFolder, "c:/tianshan/bin/testfile_content/", ZQ::common::Config::optReadOnly);
		holder.addDetail(ContentInterfaceLayer.c_str(), "urlPercentDecodeOnOutgoingMsg", &ContentInterface::urlPercentDecodeOnOutgoingMsg, "0", ZQ::common::Config::optReadOnly);

		//load Feedback attribute
		holder.addDetail(FeedbackLayer.c_str(), "ip", &ContentInterface::FeedbackIp, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(FeedbackLayer.c_str(), "port", &ContentInterface::FeedbackPort, "", ZQ::common::Config::optReadOnly);

		//load StoreReplica attribute
		holder.addDetail(StoreReplicaLayer.c_str(), "groupId", &ContentInterface::StoreReplicaGroupId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(StoreReplicaLayer.c_str(), "replicaId", &ContentInterface::StoreReplicaReplicaId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(StoreReplicaLayer.c_str(), "replicaPriority", &ContentInterface::StoreReplicaReplicaPriority, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(StoreReplicaLayer.c_str(), "timeout", &ContentInterface::StoreReplicaTimeout, "", ZQ::common::Config::optReadOnly);

		//load DatabaseCache attribute
		holder.addDetail(DatabaseCacheLayer.c_str(), "volumeSize", &ContentInterface::DatabaseCacheVolumeSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(DatabaseCacheLayer.c_str(), "contentSize", &ContentInterface::DatabaseCacheContentSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(DatabaseCacheLayer.c_str(), "contentSavePeriod", &ContentInterface::DatabaseCacheContentSavePeriod, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(DatabaseCacheLayer.c_str(), "contentSaveSizeTrigger", &ContentInterface::DatabaseCacheContentSaveSizeTrigger, "", ZQ::common::Config::optReadOnly);

		//load Volumes attribute
		holder.addDetail(VolLayer.c_str(), &ContentInterface::readVol, &ContentInterface::registerVol);

		// VideoServer
		holder.addDetail(VideoServer.c_str(), "id", &ContentInterface::VideoServerId, "Video server ID", ZQ::common::Config::optReadOnly);
		holder.addDetail(VideoServer.c_str(), "videoOnDemandVersion", &ContentInterface::VideoOnDemandVersion, "Technical specification for video on demand system of NGB", ZQ::common::Config::optReadOnly);
	}
};//struct ContentInterface

struct GBCSBaseConfig
{
	struct GBCS
	{
		typedef ::ZQ::common::Config::Holder< IceProperties >	IcePropertiesHolder;
		typedef ::ZQ::common::Config::Holder< PublishLogs >	PublishedLogHolder;
		typedef ::ZQ::common::Config::Holder< stBind >			BindHodler;
		typedef ::ZQ::common::Config::Holder< ContentInterface >		VideoServerHolder;

		std::string			netId;
		int32					_ForceRefOnly;
		IcePropertiesHolder		_iceProperty;
		PublishedLogHolder		_publishedLog;
		BindHodler				_bind;
		VideoServerHolder		_videoServer;

		static void structure(::ZQ::common::Config::Holder< GBCS > &holder)
		{
			//load GBCS
			holder.addDetail("", "netId", &GBCS::netId, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("","forceRefOnly",&GBCS::_ForceRefOnly,"0",ZQ::common::Config::optReadOnly);
			holder.addDetail("", &GBCS::readIceProperty, &GBCS::registerIceProperty);
			holder.addDetail("PublishedLogs", &GBCS::readPublishedLog, &GBCS::registerPublishedLog);
			holder.addDetail("", &GBCS::readBind, &GBCS::registerBind);
			holder.addDetail("", &GBCS::readVideoServer, &GBCS::registerVideoServer);
		}

		void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			IcePropertiesHolder holder("");
			holder.read(node, hPP);
			_iceProperty = holder;
		}
		void registerIceProperty(const std::string &full_path)
		{
			_iceProperty.snmpRegister(full_path);
		}

		void readPublishedLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			PublishedLogHolder holder("");
			holder.read(node, hPP);
			_publishedLog = holder;
		}
		void registerPublishedLog(const std::string &full_path)
		{
			_publishedLog.snmpRegister(full_path);
		}

		void readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			BindHodler holder("");
			holder.read(node, hPP);
			_bind = holder;
		}
		void registerBind(const std::string &full_path)
		{
			_bind.snmpRegister(full_path);
		}

		void readVideoServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			VideoServerHolder holder("");
			holder.read(node, hPP);
			_videoServer = holder;
		}
		void registerVideoServer(const std::string &full_path)
		{
			_videoServer.snmpRegister(full_path);
		}
	};

	typedef ::ZQ::common::Config::Holder< GBCS > GBCSHolder;
	typedef std::vector< GBCSHolder > GBCSHolderVec;
	GBCSHolderVec GBCSVec;

	static void structure(::ZQ::common::Config::Holder< GBCSBaseConfig > &holder)
	{
		//load GBCS
		holder.addDetail(GBCSLayer.c_str(), &GBCSBaseConfig::readGBCS, &GBCSBaseConfig::registerGBCS);
	}

	void readGBCS(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		GBCSHolder holder("");
		holder.read(node, hPP);
		GBCSVec.push_back(holder);
	}
	void registerGBCS(const std::string &full_path)
	{
		for (GBCSHolderVec::iterator it = GBCSVec.begin(); it != GBCSVec.end(); it++)
			it->snmpRegister(full_path);
	}
};

}//namespace GBCS

}//namespace ZQTianShan

#endif //__ZQTianShan_GBCSConfig_H__
