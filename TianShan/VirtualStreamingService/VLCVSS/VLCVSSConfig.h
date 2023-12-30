#ifndef __ZQTianShan_VLCVSSConfig_H__
#define __ZQTianShan_VLCVSSConfig_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>

namespace ZQTianShan{

namespace VSS{

namespace VLC{

//root configuration

//global configuration

//default configuration
static ::std::string DefaultLayer = "default";
static ::std::string Slash ="/";
static ::std::string CrashDumpLayer = "CrashDump";

struct CrashDump
{
	::std::string	path;
	int32			enabled;
	static void structure(ZQ::common::Config::Holder< CrashDump > &holder)
    {
		static ::std::string Layer = DefaultLayer + Slash + CrashDumpLayer;
		holder.addDetail(Layer.c_str(), "path", &CrashDump::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "enabled", &CrashDump::enabled, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IceTraceLayer = "IceTrace";
struct IceTrace
{
	int32	enabled;
	int32	level;
	int32	size;
	static void structure(ZQ::common::Config::Holder< IceTrace > &holder)
    {
		static ::std::string Layer = DefaultLayer + Slash + IceTraceLayer;
		holder.addDetail(Layer.c_str(), "enabled", &IceTrace::enabled, "", ZQ::common::Config::optReadOnly);
		//holder.addDetail(Layer.c_str(), "logfilesuffix", &IceTrace::logfilesuffix, NULL);
		holder.addDetail(Layer.c_str(), "level", &IceTrace::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &IceTrace::size, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IceStormLayer = "EventChannel";
struct IceStorm
{
	::std::string	endPoint;
	static void structure(ZQ::common::Config::Holder< IceStorm > &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + IceStormLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &IceStorm::endPoint, NULL, ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IcePropertiesLayer = "IceProperties";
static ::std::string PropLayer = ::std::string("prop");
struct IceProperties
{
	struct prop
	{
		::std::string	name;
		::std::string	value;
		static void structure(ZQ::common::Config::Holder< prop > &holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "name", &prop::name, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "value", &prop::value,NULL, ZQ::common::Config::optReadOnly);
		}
	};
	typedef ::std::list< ZQ::common::Config::Holder<prop> > props;
	props _props;
	void readProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<prop> dptholder("");
        dptholder.read(node, hPP);
        _props.push_back(dptholder);
    }
    void registerProp(const std::string &full_path)
    {
        for(props::iterator it = _props.begin(); it != _props.end(); ++it)
            it->snmpRegister(full_path);
    }
    static void structure(ZQ::common::Config::Holder<IceProperties> &holder)
    {
		static ::std::string Layer = DefaultLayer + Slash + IcePropertiesLayer + Slash + PropLayer;
        holder.addDetail(Layer.c_str(), &IceProperties::readProp, &IceProperties::registerProp);
    }
};

static ::std::string DatabaseLayer = "Database";
struct Database
{
	::std::string	path;
	::std::string	runtimePath;
	static void structure(ZQ::common::Config::Holder< Database > &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + DatabaseLayer;
		holder.addDetail(Layer.c_str(), "path", &Database::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "runtimePath", &Database::runtimePath, NULL, ZQ::common::Config::optReadOnly);
	}
};

static ::std::string PublishedLogsLayer = "PublishedLogs";
static ::std::string PublishLogLayer = "Log";
struct PublishedLogs
{
	struct PublishLog
	{
		std::string _path;
		std::string _syntax;
		std::string _key;
		std::string _type;

		typedef ZQ::common::Config::Holder<PublishLog> PublishLogHolder;

		static void structure(ZQ::common::Config::Holder<PublishLog>& holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "path", &PublishLog::_path, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "syntax", &PublishLog::_syntax, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "key", &PublishLog::_key, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "type", &PublishLog::_type, "", ZQ::common::Config::optReadOnly);
		}
	};
	typedef PublishLog::PublishLogHolder PublishLogHolder;
	std::vector<PublishLogHolder> _logDatas;

	void readPublishedLogs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		PublishLogHolder logHolder("path");
		logHolder.read(node, hPP);
		_logDatas.push_back(logHolder);
	}

	void registerPublishedLogs(const std::string &full_path)
	{
		for (std::vector<PublishLogHolder>::iterator it = _logDatas.begin(); it != _logDatas.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	static void structure(ZQ::common::Config::Holder< PublishedLogs > &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + PublishedLogsLayer + Slash + PublishLogLayer;
		holder.addDetail(Layer.c_str(), &PublishedLogs::readPublishedLogs, &PublishedLogs::registerPublishedLogs);
	}
};

//configuration of VLCVSS
static ::std::string VLCVSSLayer = "VLCVSS";

//LogFile
static ::std::string LogFileLayer = "LogFile";
struct LogFile
{
	::std::string	path;
	int32			level;
	int32			maxCount;
	int32			size;
	int32			bufferSize;
	int32			flushTimeout;
	static void structure(ZQ::common::Config::Holder< LogFile > &holder)
	{
		static ::std::string Layer = VLCVSSLayer + Slash + LogFileLayer;
		holder.addDetail(Layer.c_str(), "path", &LogFile::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &LogFile::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &LogFile::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &LogFile::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &LogFile::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &LogFile::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//bind
static ::std::string BindLayer = "Bind";
struct stBind
{
	::std::string	endPoint;
	int32			dispatchSize;
	int32			dispatchMax;
	int32			evictorSize;
	int32			threadPoolSize;
	static void structure(ZQ::common::Config::Holder< stBind > &holder)
	{
		static ::std::string Layer = VLCVSSLayer + Slash + BindLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &stBind::endPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchSize", &stBind::dispatchSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchMax", &stBind::dispatchMax, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "evictorSize", &stBind::evictorSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "threadPoolSize", &stBind::threadPoolSize, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string TelnetPropLayer = "TelnetProp";
struct TelnetProp
{
	int32	threadPoolSize;
	int32	timeOut;
	int32	bufferMaxSize;
	static void structure(ZQ::common::Config::Holder< TelnetProp > &holder)
	{
		static ::std::string Layer =  VLCVSSLayer + Slash + TelnetPropLayer;
		holder.addDetail(Layer.c_str(), "threadPoolSize", &TelnetProp::threadPoolSize, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "timeOut", &TelnetProp::timeOut, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferMaxSize", &TelnetProp::bufferMaxSize, NULL, ZQ::common::Config::optReadOnly);
	}
};

static std::string StreamServicePropLayer = "StreamServiceProp";
struct StreamServiceProp
{
	int32   synInterval;
	int32   retry;
	static void structure(ZQ::common::Config::Holder< StreamServiceProp > &holder)
	{
		static ::std::string Layer =  VLCVSSLayer + Slash + StreamServicePropLayer;
		holder.addDetail(Layer.c_str(), "synInterval", &StreamServiceProp::synInterval, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "retry", &StreamServiceProp::retry, NULL, ZQ::common::Config::optReadOnly);
	}
};

static std::string VLCServicePropLayer = "VLCServiceProp";
struct VLCServiceProp
{
	int32 synInterval;
	std::string szServiceName;

	static void structure(ZQ::common::Config::Holder< VLCServiceProp > &holder)
	{
		static ::std::string Layer =  VLCVSSLayer + Slash + VLCServicePropLayer;
		holder.addDetail(Layer.c_str(), "synInterval", &VLCServiceProp::synInterval, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "serviceName", &VLCServiceProp::szServiceName, NULL, ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IceLogLayer = "IceLog";
struct VLCVSSIceLog
{
	::std::string path;
	int32	level;
	int32	maxCount;
	int32	size;
	int32	bufferSize;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< VLCVSSIceLog > &holder)
	{
		static ::std::string Layer = VLCVSSLayer + Slash + IceLogLayer;
		holder.addDetail(Layer.c_str(), "path", &VLCVSSIceLog::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &VLCVSSIceLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &VLCVSSIceLog::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &VLCVSSIceLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &VLCVSSIceLog::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &VLCVSSIceLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//SoapServerInfo
static ::std::string TelnetServerInfoLayer = "TelnetServerInfo";
struct TelnetServerInfo
{
	::std::string ip;
	int32	port;
	::std::string password;

	static void structure(ZQ::common::Config::Holder< TelnetServerInfo > &holder)
	{
		static ::std::string Layer = VLCVSSLayer + Slash + TelnetServerInfoLayer;
		holder.addDetail(Layer.c_str(), "ip", &TelnetServerInfo::ip, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "port", &TelnetServerInfo::port, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "password", &TelnetServerInfo::password, NULL, ZQ::common::Config::optReadOnly);
	}
};

static ::std::string VolumnInfoLayer = "VolumnInfo";
static ::std::string VolumnLayer = "Volumn";
struct VolumnInfo
{
	struct Volumn
	{
		::std::string	name;
		::std::string	path;
		static void structure(ZQ::common::Config::Holder< Volumn > &holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "name", &Volumn::name, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "path", &Volumn::path,NULL, ZQ::common::Config::optReadOnly);
		}
	};
	typedef ::std::list< ZQ::common::Config::Holder<Volumn> > Volumns;
	Volumns _volumns;
	void readProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<Volumn> dptholder("");
		dptholder.read(node, hPP);
		_volumns.push_back(dptholder);
	}
	void registerProp(const std::string &full_path)
	{
		for(Volumns::iterator it = _volumns.begin(); it != _volumns.end(); ++it)
			it->snmpRegister(full_path);
	}
	static void structure(ZQ::common::Config::Holder<VolumnInfo> &holder)
	{
		static ::std::string Layer = VLCVSSLayer + Slash + VolumnInfoLayer + Slash + VolumnLayer;
		holder.addDetail(Layer.c_str(), &VolumnInfo::readProp, &VolumnInfo::registerProp);
	}
};

static std::string StoreInfoLayer = "StoreInfo";
struct StoreInfo
{
	std::string netId;
	std::string type;
	std::string streamableLength;

	std::string groupId;
	std::string replicaId;
	int			replicaPriority;
	int			timeout;
	int			contentSize;
	int			volumeSize;

	static void structure(ZQ::common::Config::Holder< StoreInfo > &holder)
	{
		static ::std::string Layer = VLCVSSLayer + Slash + StoreInfoLayer;
		holder.addDetail(Layer.c_str(), "netId", &StoreInfo::netId, NULL);
		holder.addDetail(Layer.c_str(), "type", &StoreInfo::type, NULL);
		holder.addDetail(Layer.c_str(), "streamableLength", &StoreInfo::streamableLength,NULL);


		Layer += Slash + "StoreReplica";
		holder.addDetail(Layer.c_str(), "groupId", &StoreInfo::groupId, NULL);
		holder.addDetail(Layer.c_str(), "replicaId", &StoreInfo::replicaId, NULL);
		holder.addDetail(Layer.c_str(), "replicaPriority", &StoreInfo::replicaPriority, NULL);
		holder.addDetail(Layer.c_str(), "timeout", &StoreInfo::timeout, NULL);
		holder.addDetail(Layer.c_str(), "contentSize", &StoreInfo::contentSize, NULL);
		holder.addDetail(Layer.c_str(), "volumeSize", &StoreInfo::volumeSize, NULL);
	}
};

class VLCVSSConfig
{
public:
	VLCVSSConfig(const char *filepath);
	virtual ~VLCVSSConfig();

	void ConfigLoader();
	
	//default config
	ZQ::common::Config::Loader< CrashDump >			_crashDump;
	ZQ::common::Config::Loader< IceTrace >			_iceTrace;
	ZQ::common::Config::Loader< IceStorm >			_iceStorm;
	ZQ::common::Config::Loader< IceProperties >		_iceProperties;
	ZQ::common::Config::Loader< Database >			_dataBase;
	ZQ::common::Config::Loader< PublishedLogs >		_publishedLogs;

	//TMVSS config
	ZQ::common::Config::Loader< LogFile >			_cLogFile;
	ZQ::common::Config::Loader< stBind >			_bind;
	ZQ::common::Config::Loader< TelnetProp >		_telnetProp;
	ZQ::common::Config::Loader< StreamServiceProp > _streamServiceProp;
	ZQ::common::Config::Loader< VLCServiceProp >    _VLCServiceProp;
	ZQ::common::Config::Loader< VLCVSSIceLog >		_VLCVSSIceLog;
	ZQ::common::Config::Loader< TelnetServerInfo >	_telnetServerInfo;
	ZQ::common::Config::Loader< VolumnInfo >		_volumnInfo;
	ZQ::common::Config::Loader< StoreInfo >			_storeInfo;

private:
	::std::string	_strFilePath;
	ZQ::common::Log _logFile;
};

}//namespace VLC

}//namespace VSS

}//namespace ZQTianShan

#endif __ZQTianShan_VLCVSSConfig_H__