#if !defined(__ZQTIANSHAN_GBSVC_CONFIG_H__)
#define __ZQTIANSHAN_GBSVC_CONFIG_H__

#include "confighelper.h"

namespace ZQTianShan {
namespace GBServerNS {

//default configuration
static std::string DefaultLayer = "default";
static std::string Slash ="/";
static std::string CrashDumpLayer = "CrashDump";

struct CrashDump
{
	std::string	 _path;
	int32	     _enabled;
	static void structure(ZQ::common::Config::Holder< CrashDump > &holder)
	{
		static std::string Layer = DefaultLayer + Slash + CrashDumpLayer;
		holder.addDetail(Layer.c_str(), "path",    &CrashDump::_path, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail(Layer.c_str(), "enabled", &CrashDump::_enabled, "", ZQ::common::Config::optReadOnly);
	}
};

struct GBsvcBase
{
	std::string  _localIp;
	std::string	 _localPort;
	int          _releaseThreadSize;
	int          _processThreadSize;
	int          _dispatchThreadSize;
	int          _enableFileDebug;
	std::string  _filesDirectory;

	std::string  _feedBackIP;
	int          _feedBackPort;
	static void structure(ZQ::common::Config::Holder< GBsvcBase > &holder)
	{
		std::string GBsvcLayer = "GBsvc";
		holder.addDetail(GBsvcLayer.c_str(), "localIp", &GBsvcBase::_localIp, "127.0.0.1", ZQ::common::Config::optReadOnly);
		holder.addDetail(GBsvcLayer.c_str(), "localPort", &GBsvcBase::_localPort, "8080", ZQ::common::Config::optReadOnly);
		holder.addDetail(GBsvcLayer.c_str(), "releaseThreadSize", &GBsvcBase::_releaseThreadSize, "10", ZQ::common::Config::optReadOnly);
		holder.addDetail(GBsvcLayer.c_str(), "processThreadSize", &GBsvcBase::_processThreadSize, "10", ZQ::common::Config::optReadOnly);
		holder.addDetail(GBsvcLayer.c_str(), "dispatchThreadSize", &GBsvcBase::_dispatchThreadSize,"10", ZQ::common::Config::optReadOnly);

		holder.addDetail(GBsvcLayer.c_str(), "enableFileDebug", &GBsvcBase::_enableFileDebug, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail(GBsvcLayer.c_str(), "filesDirectory", &GBsvcBase::_filesDirectory,"C:\\TianShan\\bin", ZQ::common::Config::optReadOnly);

		std::string GBsvcFeedBackLayer = GBsvcLayer + "/" + "FeedBack";
		holder.addDetail(GBsvcFeedBackLayer.c_str(), "port", &GBsvcBase::_feedBackPort, "4445", ZQ::common::Config::optReadOnly);
		holder.addDetail(GBsvcFeedBackLayer.c_str(), "ip", &GBsvcBase::_feedBackIP, "127.0.0.1", ZQ::common::Config::optReadOnly);
	}
};

struct GBServerConfig
{
	ZQ::common::Config::Holder< CrashDump >		_crashDump;
	ZQ::common::Config::Holder< GBsvcBase >		_gbSvcBase;

	static void structure(ZQ::common::Config::Holder<GBServerConfig> &holder)
	{
		holder.addDetail("",&GBServerConfig::readCrashDump, &GBServerConfig::registerCrashDump, ZQ::common::Config::Range(0,1));
		holder.addDetail("",&GBServerConfig::readGBsvc, &GBServerConfig::registerNothing, ::ZQ::common::Config::Range(0,1));
	};

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

	void readGBsvc(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<GBsvcBase> nvholder("");
		nvholder.read(node, hPP);
		_gbSvcBase = nvholder;
	}

	void registerNothing(const std::string&){}
};

}//GBServerNS
}//ZQTianShan 

#endif// __ZQTIANSHAN_GBSVC_CONFIG_H__