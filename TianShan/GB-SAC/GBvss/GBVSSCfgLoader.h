#ifndef __GBVSSCFGLOADER_H__
#define __GBVSSCFGLOADER_H__

#include "GBVSSConfig.h"

namespace ZQTianShan{

namespace GBVSS{

struct GBVSSCfg
{
	//Default config
	::ZQ::common::Config::Holder< CrashDump >		_crashDump;
	::ZQ::common::Config::Holder< IceTrace	>		_iceTrace;
	::ZQ::common::Config::Holder< IceStorm	>		_iceStorm;
	::ZQ::common::Config::Holder< Database	>		_dataBase;

	::ZQ::common::Config::Holder< GBVSSBaseConfig >		_GBVSSBaseConfig;

	static void structure(::ZQ::common::Config::Holder<GBVSSCfg> &holder)
	{
		//read default config
		holder.addDetail("",&GBVSSCfg::readCrashDump, &GBVSSCfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&GBVSSCfg::readIceTrace, &GBVSSCfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&GBVSSCfg::readIceStorm, &GBVSSCfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&GBVSSCfg::readDatabase, &GBVSSCfg::registerDatabase, ::ZQ::common::Config::Range(0,1));

		//read GBVSS config
		holder.addDetail("",&GBVSSCfg::readGBVSS, &GBVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

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

	void readGBVSS(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<GBVSSBaseConfig> nvholder("");
		nvholder.read(node, hPP);
		_GBVSSBaseConfig = nvholder;
	}

	void registerNothing(const std::string&){}
};

}//namespace GBVSS

}//namespace ZQTianShan

extern ZQ::common::Config::Loader<ZQTianShan::GBVSS::GBVSSCfg> gGBVSSConfig;

#endif //__GBVSSCFGLOADER_H__
