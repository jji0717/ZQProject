#ifndef __NSSCFGLOADER_H__
#define __NSSCFGLOADER_H__

#include "NSSConfig.h"
#include "A3Config.h"

namespace ZQTianShan{

namespace NSS{

struct NSSCfg
{
	//Default config
	::ZQ::common::Config::Holder< CrashDump >		_crashDump;
	::ZQ::common::Config::Holder< IceTrace	>		_iceTrace;
	::ZQ::common::Config::Holder< IceStormConfig >	_iceStorm;
	//::ZQ::common::Config::Holder< IceProperties	>	_iceProperties;
	::ZQ::common::Config::Holder< Database	>		_dataBase;
	//::ZQ::common::Config::Holder< PublishedLogs >	_publishedLogs;

	//NSS config
	//::ZQ::common::Config::Holder< LogFile >			_logFile;
	//::ZQ::common::Config::Holder< stBind >			_stBind;
	//::ZQ::common::Config::Holder< nssIceLog >		_nssIceLog;
	//::ZQ::common::Config::Holder< A3JMSEvents >		_a3JMSEvents;
	//::ZQ::common::Config::Holder< NSSVideoServer >	_nssVideoServer;
	::ZQ::common::Config::Holder< NSSBaseConfig >		_nssBaseConfig;
	

	static void structure(::ZQ::common::Config::Holder<NSSCfg> &holder)
	{
		//read default config
		holder.addDetail("",&NSSCfg::readCrashDump, &NSSCfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readIceTrace, &NSSCfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readIceStorm, &NSSCfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&NSSCfg::readIceProperties, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readDatabase, &NSSCfg::registerDatabase, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&NSSCfg::readPublishedLogs, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

		//read nss config
		//holder.addDetail("",&NSSCfg::readLogFile, &NSSCfg::registerLogFile, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&NSSCfg::readstBind, &NSSCfg::registerstBind, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&NSSCfg::readnssIceLog, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readNSS, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
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
		::ZQ::common::Config::Holder<IceStormConfig> nvholder("");
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
	void readNSS(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<NSSBaseConfig> nvholder("");
		nvholder.read(node, hPP);
		_nssBaseConfig = nvholder;
	}
	void registerNothing(const std::string&){}
};

}//namespace NSS

}//namespace ZQTianShan

#endif __NSSCFGLOADER_H__