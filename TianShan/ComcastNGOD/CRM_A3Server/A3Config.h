// FileName : A3Config.h
// Author   : Zheng Junming
// Date     : 2009-05

#ifndef __CRG_PLUGIN_A3SERVER_CONFIG_H__
#define __CRG_PLUGIN_A3SERVER_CONFIG_H__

#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>

namespace CRG
{
namespace Plugin
{
namespace A3Server
{
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
		int32   maxCount;
		static void structure(ZQ::common::Config::Holder< IceTrace > &holder)
		{
			static ::std::string Layer = DefaultLayer + Slash + IceTraceLayer;
			holder.addDetail(Layer.c_str(), "enabled", &IceTrace::enabled, "", ZQ::common::Config::optReadOnly);
			//holder.addDetail(Layer.c_str(), "logfilesuffix", &IceTrace::logfilesuffix, NULL);
			holder.addDetail(Layer.c_str(), "level", &IceTrace::level, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "size", &IceTrace::size, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "maxCount", &IceTrace::maxCount, "", ZQ::common::Config::optReadOnly);
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
	static ::std::string HttpA3ServerLayer = "Http_A3Server";

	//LogFile
	static ::std::string LogFileLayer = "LogFile";
	struct LogFile
	{
		int32			level;
		int32			maxCount;
		int32			size;
		int32			bufferSize;
		int32			flushTimeout;
		static void structure(ZQ::common::Config::Holder< LogFile > &holder)
		{
			static ::std::string Layer = HttpA3ServerLayer + Slash + LogFileLayer;
			holder.addDetail(Layer.c_str(), "level", &LogFile::level, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "maxCount", &LogFile::maxCount, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "size", &LogFile::size, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "bufferSize", &LogFile::bufferSize, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "flushTimeout", &LogFile::flushTimeout, "", ZQ::common::Config::optReadOnly);
		}
	};

	static ::std::string ContentStoreListLayer = "ContentStoreList";
	static ::std::string ContentStoreLayer = "ContentStore";
	struct ContentStoreList
	{
		struct ContentStore
		{
			std::string endpoint;
			static void structure(ZQ::common::Config::Holder< ContentStore > &holder)
			{
				static ::std::string Layer = "";
				holder.addDetail(Layer.c_str(), "endpoint", &ContentStore::endpoint, NULL, ZQ::common::Config::optReadOnly);
			}
		};
		typedef ::std::list< ZQ::common::Config::Holder<ContentStore> > contentStores;
		contentStores _contentStores;
		void readProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			ZQ::common::Config::Holder<ContentStore> dptholder("");
			dptholder.read(node, hPP);
			_contentStores.push_back(dptholder);
		}
		void registerProp(const std::string &full_path)
		{
			for(contentStores::iterator it = _contentStores.begin(); it != _contentStores.end(); ++it)
				it->snmpRegister(full_path);
		}
		static void structure(ZQ::common::Config::Holder<ContentStoreList> &holder)
		{
			static ::std::string Layer = HttpA3ServerLayer + Slash + ContentStoreListLayer + Slash + ContentStoreLayer;
			holder.addDetail(Layer.c_str(), &ContentStoreList::readProp, &ContentStoreList::registerProp);
		}
	};

	static ::std::string bindLayers="Bind";
	struct Bind
	{
		::std::string serviceName;
		::std::string endpoint;
		int32 evictorSize;
		static void structure(ZQ::common::Config::Holder< Bind > &holder)
		{
			static ::std::string Layer = HttpA3ServerLayer + Slash + bindLayers;
			holder.addDetail(Layer.c_str(), "serviceName", &Bind::serviceName, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "endpoint", &Bind::endpoint, "", ZQ::common::Config::optReadOnly);
			holder.addDetail(Layer.c_str(), "evictorSize", &Bind::evictorSize, "", ZQ::common::Config::optReadOnly);
		}
	};

	// config loader structure
	struct HttpA3Cfg
	{
		//Default config
		::ZQ::common::Config::Holder< CrashDump >		_crashDump;
		::ZQ::common::Config::Holder< IceTrace	>		_iceTrace;
		::ZQ::common::Config::Holder< IceStorm	>		_iceStorm;
		::ZQ::common::Config::Holder< IceProperties >	_iceProperties;
		::ZQ::common::Config::Holder< Database	>		_dataBase;
		::ZQ::common::Config::Holder< PublishedLogs >	_publishedLogs;

		//A3 Server config
		::ZQ::common::Config::Holder< LogFile >			_logFile;
		::ZQ::common::Config::Holder< ContentStoreList > _contentStoreList;
		::ZQ::common::Config::Holder< Bind >            _bind;

		static void structure(::ZQ::common::Config::Holder<HttpA3Cfg> &holder)
		{
			//read default config
			holder.addDetail("",&HttpA3Cfg::readCrashDump, &HttpA3Cfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
			holder.addDetail("",&HttpA3Cfg::readIceTrace, &HttpA3Cfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
			holder.addDetail("",&HttpA3Cfg::readIceStorm, &HttpA3Cfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
			holder.addDetail("",&HttpA3Cfg::readIceProperties, &HttpA3Cfg::registerNothing, ::ZQ::common::Config::Range(0,1));
			holder.addDetail("",&HttpA3Cfg::readDatabase, &HttpA3Cfg::registerDatabase, ::ZQ::common::Config::Range(0,1));
			holder.addDetail("",&HttpA3Cfg::readPublishedLogs, &HttpA3Cfg::registerNothing, ::ZQ::common::Config::Range(0,1));

			//read A3 Server config
			holder.addDetail("",&HttpA3Cfg::readLogFile, &HttpA3Cfg::registerLogFile, ::ZQ::common::Config::Range(0,1));
			holder.addDetail("",&HttpA3Cfg::readContentStoreList, &HttpA3Cfg::registerNothing, ::ZQ::common::Config::Range(0,1));
			holder.addDetail("",&HttpA3Cfg::readBind, &HttpA3Cfg::registerBind, ::ZQ::common::Config::Range(0,1));
		}
		void readCrashDump(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<CrashDump> nvholder("");
			nvholder.read(node, hPP);
			_crashDump = nvholder;
		}
		void registerCrashDump(const std::string &full_path)
		{
			_crashDump.snmpRegister(full_path);
		}
		void readIceTrace(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<IceTrace> nvholder("");
			nvholder.read(node, hPP);
			_iceTrace = nvholder;
		}
		void registerIceTrace(const std::string &full_path)
		{
			_iceTrace.snmpRegister(full_path);
		}
		void readIceStorm(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<IceStorm> nvholder("");
			nvholder.read(node, hPP);
			_iceStorm = nvholder;
		}
		void registerIceStorm(const std::string &full_path)
		{
			_iceStorm.snmpRegister(full_path);
		}
		void readIceProperties(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<IceProperties> nvholder("");
			nvholder.read(node, hPP);
			_iceProperties = nvholder;
		}
		void readDatabase(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<Database> nvholder("");
			nvholder.read(node, hPP);
			_dataBase = nvholder;
		}
		void registerDatabase(const std::string &full_path)
		{
			_dataBase.snmpRegister(full_path);
		}
		void readPublishedLogs(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<PublishedLogs> nvholder("");
			nvholder.read(node, hPP);
			_publishedLogs = nvholder;
		}
		void registerPublishedLogs(const std::string &full_path)
		{
			_publishedLogs.snmpRegister(full_path);
		}
		void readLogFile(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<LogFile> nvholder("");
			nvholder.read(node, hPP);
			_logFile = nvholder;
		}
		void registerLogFile(const std::string &full_path)
		{
			_logFile.snmpRegister(full_path);
		}
		void readContentStoreList(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<ContentStoreList> nvholder("");
			nvholder.read(node, hPP);
			_contentStoreList = nvholder;
		}

		void readBind(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
		{
			::ZQ::common::Config::Holder<Bind> nvholder("");
			nvholder.read(node, hPP);
			_bind = nvholder;
		}
		void registerBind(const std::string &full_path)
		{
			_bind.snmpRegister(full_path);
		}

		void registerNothing(const std::string &full_path)
		{
		}
	};

} // end for A3Server
} // end for Plugin
} // end for CRG

extern  ZQ::common::Config::Loader<CRG::Plugin::A3Server::HttpA3Cfg> _httpA3Config;

#endif

