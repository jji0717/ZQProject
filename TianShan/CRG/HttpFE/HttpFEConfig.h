#ifndef __TIANSHAN_HttpFECONFIG_H__
#define __TIANSHAN_HttpFECONFIG_H__
#include <ConfigHelper.h>
#include <set>

namespace ZQTianShan{
namespace HttpFE{

static ::std::string DefaultLayer = "default";
static ::std::string Slash = "/";

static ::std::string CrashDumpLayer = "CrashDump";
struct CrashDump
{
	::std::string path;
	int32         enabled;
	static void structure(ZQ::common::Config::Holder<CrashDump> &holder)
	{
		static ::std::string Layer = DefaultLayer + Slash + CrashDumpLayer;
		holder.addDetail(Layer.c_str(), "path", &CrashDump::path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "enabled", &CrashDump::enabled, "", ZQ::common::Config::optReadOnly);
	}
};


static ::std::string HttpFELayer = "HttpFE";

//ThreadPool
static ::std::string ThreadCountLayer = "ThreadCount";
struct ThreadCountConfig
{
	int32			size;
    int32 maxPendingRequest;
	static void structure(ZQ::common::Config::Holder< ThreadCountConfig > &holder)
	{
		static ::std::string Layer = HttpFELayer + Slash + ThreadCountLayer;
        holder.addDetail(Layer.c_str(), "size", &ThreadCountConfig::size, "", ZQ::common::Config::optReadOnly);
        holder.addDetail(Layer.c_str(), "maxPendingRequest", &ThreadCountConfig::maxPendingRequest, "-1", ZQ::common::Config::optReadOnly);
        
	}
};

static ::std::string BindLayer="Bind";
struct Bind
{
	::std::string ip;
	int32         port;
	static void structure(ZQ::common::Config::Holder< Bind > &holder)
	{
		static ::std::string Layer = HttpFELayer + Slash + BindLayer;
		holder.addDetail(Layer.c_str(), "ip", &Bind::ip, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "port", &Bind::port, "", ZQ::common::Config::optReadOnly);
	}
};

static ::std::string PlunginsLayer = "Plugins";
static ::std::string PlugingsModuleLayer = "Module";
struct PluginsConfig
{
    std::string populatePath;
    std::string configDir;
    std::string logDir;

	struct ModuleConfig
	{
		typedef ZQ::common::Config::Holder<ModuleConfig> ModuleConfigHolder;
		std::string image;
		static void structure(ModuleConfigHolder &holder)
		{
			holder.addDetail("", "image", &ModuleConfig::image, NULL, ZQ::common::Config::optReadOnly);
		}
	};

	typedef ModuleConfig::ModuleConfigHolder ModuleConfigHolder;
	std::vector<ModuleConfigHolder> _modules;
   
	void readPlugins(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ModuleConfigHolder logHolder("Module");
		logHolder.read(node, hPP);
		_modules.push_back(logHolder);
	}

	void registerPlugins(const std::string &full_path)
	{
		for (std::vector<ModuleConfigHolder>::iterator it = _modules.begin(); it != _modules.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}

	static void structure(ZQ::common::Config::Holder< PluginsConfig > &holder)
	{
		static ::std::string Layer = HttpFELayer + Slash + PlunginsLayer;
		holder.addDetail(Layer.c_str(), "populatePath", &PluginsConfig::populatePath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "configDir", &PluginsConfig::configDir, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "logDir", &PluginsConfig::logDir, NULL, ZQ::common::Config::optReadOnly);

		Layer += Slash + PlugingsModuleLayer;
		holder.addDetail(Layer.c_str(), &PluginsConfig::readPlugins, &PluginsConfig::registerPlugins);
	}

	std::set<std::string> modules;
	void populate(const std::string &filespec);
};

struct HttpFEConfig
{
	//Default config
	::ZQ::common::Config::Holder< CrashDump >		_crashDump;
	//::ZQ::common::Config::Holder< EventChannel	>	_eventChannel;
	//::ZQ::common::Config::Holder< PublishedLogs >	_publishedLogs;

	//HttpFE config
	::ZQ::common::Config::Holder< ThreadCountConfig >_ThreadCountConfig;
//	::ZQ::common::Config::Holder< LogFile >			_logFile;
	::ZQ::common::Config::Holder< Bind >            _bind;
	::ZQ::common::Config::Holder< PluginsConfig >   _pluginsConfig;
    int32 hexDumpMode;
    int32 fnLimit;
	static void structure(::ZQ::common::Config::Holder<HttpFEConfig> &holder)
	{
		//read default config
        holder.addDetail("HttpFE", "hexDump", &HttpFEConfig::hexDumpMode, "0");
        holder.addDetail("HttpFE", "fnLimit", &HttpFEConfig::fnLimit, "1024");

		holder.addDetail("",&HttpFEConfig::readCrashDump, &HttpFEConfig::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&HttpFEConfig::readEventChannel, &HttpFEConfig::registerEventChannel, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&HttpFEConfig::readPublishedLogs, &HttpFEConfig::registerNothing, ::ZQ::common::Config::Range(0,1));

		//read A3 Server config
		holder.addDetail("",&HttpFEConfig::readThreadCount, &HttpFEConfig::registerThreadCount, ::ZQ::common::Config::Range(0,1));
//		holder.addDetail("",&HttpFEConfig::readLogFile, &HttpFEConfig::registerLogFile, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&HttpFEConfig::readBind, &HttpFEConfig::registerBind, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&HttpFEConfig::readPluginsConfig, &HttpFEConfig::registerPluginsConfig, ::ZQ::common::Config::Range(0,1));
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
	void readThreadCount(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<ThreadCountConfig> nvholder("");
		nvholder.read(node, hPP);
		_ThreadCountConfig = nvholder;
	}
	void registerThreadCount(const std::string &full_path)
	{
		_ThreadCountConfig.snmpRegister(full_path);
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

	void readPluginsConfig(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<PluginsConfig> nvholder("");
		nvholder.read(node, hPP);
		_pluginsConfig = nvholder;
	}
	void registerPluginsConfig(const std::string &full_path)
	{
		_pluginsConfig.snmpRegister(full_path);
	}

	void registerNothing(const std::string &full_path)
	{
	}
};

} // namespace HttpFE
} // namespace ZQTianShan 

//extern ZQ::common::Config::Loader<::ZQTianShan::HttpFE::HttpFEConfig> gConfig;
#endif
