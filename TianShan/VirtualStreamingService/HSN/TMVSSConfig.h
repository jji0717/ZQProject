#ifndef __ZQTianShan_TMVSSConfig_H__
#define __ZQTianShan_TMVSSConfig_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>

namespace ZQTianShan{

namespace VSS{

namespace TM{

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

//configuration of TMVSS
static ::std::string TMVSSLayer = "TMVSS";

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
		static ::std::string Layer = TMVSSLayer + Slash + LogFileLayer;
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
		static ::std::string Layer = TMVSSLayer + Slash + BindLayer;
		holder.addDetail(Layer.c_str(), "endPoint", &stBind::endPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchSize", &stBind::dispatchSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "dispatchMax", &stBind::dispatchMax, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "evictorSize", &stBind::evictorSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "threadPoolSize", &stBind::threadPoolSize, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string RTSPPropLayer = "RTSPProp";
struct RTSPProp
{
	int32	threadPoolSize;
	int32	timeOut;
	int32	bufferMaxSize;
	static void structure(ZQ::common::Config::Holder< RTSPProp > &holder)
	{
		static ::std::string Layer =  TMVSSLayer + Slash + RTSPPropLayer;
		holder.addDetail(Layer.c_str(), "threadPoolSize", &RTSPProp::threadPoolSize, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "timeOut", &RTSPProp::timeOut, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferMaxSize", &RTSPProp::bufferMaxSize, NULL, ZQ::common::Config::optReadOnly);
	}
};

static ::std::string IceLogLayer = "IceLog";
struct TMVSSIceLog
{
	::std::string path;
	int32	level;
	int32	maxCount;
	int32	size;
	int32	bufferSize;
	int32	flushTimeout;
	static void structure(ZQ::common::Config::Holder< TMVSSIceLog > &holder)
	{
		static ::std::string Layer = TMVSSLayer + Slash + IceLogLayer;
		holder.addDetail(Layer.c_str(), "path", &TMVSSIceLog::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "level", &TMVSSIceLog::level, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "maxCount", &TMVSSIceLog::maxCount, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "size", &TMVSSIceLog::size, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "bufferSize", &TMVSSIceLog::bufferSize, "", ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "flushTimeout", &TMVSSIceLog::flushTimeout, "", ZQ::common::Config::optReadOnly);
	}
};

//LocalInfo
static ::std::string LocalInfoLayer = "SoapLocalInfo";
struct LocalInfo
{
	::std::string	ip;
	int32			port;
	static void structure(ZQ::common::Config::Holder< LocalInfo > &holder)
	{
		static ::std::string Layer = TMVSSLayer + Slash + LocalInfoLayer;
		holder.addDetail(Layer.c_str(), "ip", &LocalInfo::ip, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "port", &LocalInfo::port, "", ZQ::common::Config::optReadOnly);
	}
};

//SoapServerInfo
static ::std::string SoapServerInfoLayer = "SoapServerInfo";
struct SoapServerInfo
{
	::std::string ip;
	int32	port;

	static void structure(ZQ::common::Config::Holder< SoapServerInfo > &holder)
	{
		static ::std::string Layer = TMVSSLayer + Slash + SoapServerInfoLayer;
		holder.addDetail(Layer.c_str(), "ip", &SoapServerInfo::ip, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "port", &SoapServerInfo::port, "", ZQ::common::Config::optReadOnly);
	}
};

//for Debug
static ::std::string DebugLayer = "Debug";
static ::std::string PrivateDataLayer = "PrivateData";
static ::std::string ParamLayer = "param";
struct PrivateData
{
	struct param
	{
		std::string key;
		std::string value;

		typedef ZQ::common::Config::Holder<param> ParamHolder;

		static void structure(ZQ::common::Config::Holder<param>& holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "key", &param::key, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "value", &param::value, "", ZQ::common::Config::optReadOnly);
		}
	};
	typedef ::std::list< param::ParamHolder > params;
	params _params;

	void readParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		param::ParamHolder paramHolder("");
		paramHolder.read(node, hPP);
		_params.push_back(paramHolder);
	}

	void registerParams(const std::string &full_path)
	{
		for (params::iterator it = _params.begin(); it != _params.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	static void structure(ZQ::common::Config::Holder< PrivateData > &holder)
	{
		static ::std::string Layer = TMVSSLayer + Slash  + DebugLayer + Slash + PrivateDataLayer + Slash + ParamLayer;
		holder.addDetail(Layer.c_str(), &PrivateData::readParams, &PrivateData::registerParams);
	}
};

static ::std::string ResourceMapLayer = "ResourceMap";
struct ResourceMap
{
	struct param
	{
		std::string key;
		std::string value;

		typedef ZQ::common::Config::Holder<param> ParamHolder;

		static void structure(ZQ::common::Config::Holder<param>& holder)
		{
			static ::std::string Layer = "";
			holder.addDetail(Layer.c_str(), "key", &param::key, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "value", &param::value, "", ZQ::common::Config::optReadOnly);
		}
	};
	typedef ::std::list< param::ParamHolder > params;
	params _params;

	void readParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		param::ParamHolder paramHolder("");
		paramHolder.read(node, hPP);
		_params.push_back(paramHolder);
	}

	void registerParams(const std::string &full_path)
	{
		for (params::iterator it = _params.begin(); it != _params.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	static void structure(ZQ::common::Config::Holder< ResourceMap > &holder)
	{
		static ::std::string Layer = TMVSSLayer + Slash  + DebugLayer + Slash + ResourceMapLayer + Slash + ParamLayer;
		holder.addDetail(Layer.c_str(), &ResourceMap::readParams, &ResourceMap::registerParams);
	}
};

class TMVSSConfig
{
public:
	TMVSSConfig(const char *filepath);
	virtual ~TMVSSConfig();

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
	ZQ::common::Config::Loader< RTSPProp >			_rtspProp;
	ZQ::common::Config::Loader< TMVSSIceLog >		_tmvssIceLog;
	ZQ::common::Config::Loader< LocalInfo >			_localInfo;
	ZQ::common::Config::Loader< SoapServerInfo >	_soapServerInfo;
	ZQ::common::Config::Loader< PrivateData>		_privateData;
	ZQ::common::Config::Loader< ResourceMap>		_resourceMap;

private:
	::std::string	_strFilePath;
	ZQ::common::Log _logFile;
};

}//namespace TM

}//namespace VSS

}//namespace ZQTianShan

#endif __ZQTianShan_NSSConfig_H__