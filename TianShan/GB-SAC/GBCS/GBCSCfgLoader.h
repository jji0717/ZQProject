#ifndef __GBCSCFGLOADER_H__
#define __GBCSCFGLOADER_H__

#include "GBCSConfig.h"

namespace ZQTianShan{

namespace GBCS{

struct GBCSCfg
{
	//Default config
	::ZQ::common::Config::Holder< CrashDump >		_crashDump;
	::ZQ::common::Config::Holder< IceTrace	>		_iceTrace;
	::ZQ::common::Config::Holder< IceStorm	>		_iceStorm;
	::ZQ::common::Config::Holder< Database	>		_dataBase;

	::ZQ::common::Config::Holder< GBCSBaseConfig >		_gbcsBaseConfig;

	static void structure(::ZQ::common::Config::Holder<GBCSCfg> &holder)
	{
		//read default config
		holder.addDetail("",&GBCSCfg::readCrashDump, &GBCSCfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&GBCSCfg::readIceTrace, &GBCSCfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&GBCSCfg::readIceStorm, &GBCSCfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&GBCSCfg::readDatabase, &GBCSCfg::registerDatabase, ::ZQ::common::Config::Range(0,1));

		//read gbcs config
		holder.addDetail("",&GBCSCfg::readGBCS, &GBCSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

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

	void readGBCS(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<GBCSBaseConfig> nvholder("");
		nvholder.read(node, hPP);
		_gbcsBaseConfig = nvholder;
	}

	void registerNothing(const std::string&){}
};

}//namespace GBCS

}//namespace ZQTianShan

extern ZQ::common::Config::Loader<ZQTianShan::GBCS::GBCSCfg> gGBCSConfig;

#endif //__GBCSCFGLOADER_H__
