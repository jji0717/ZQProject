#ifndef __TMVSSCfgLOADER_H__
#define __TMVSSCfgLOADER_H__

#include "TMVSSConfig.h"

namespace ZQTianShan{

namespace VSS{

namespace TM{

struct TMVSSCfg
{
	//Default config
	::ZQ::common::Config::Holder< CrashDump >		_crashDump;
	::ZQ::common::Config::Holder< IceTrace	>		_iceTrace;
	::ZQ::common::Config::Holder< IceStorm	>		_iceStorm;
	::ZQ::common::Config::Holder< IceProperties >	_iceProperties;
	::ZQ::common::Config::Holder< Database	>		_dataBase;
	::ZQ::common::Config::Holder< PublishedLogs >	_publishedLogs;

	//TMVSS config
	::ZQ::common::Config::Holder< LogFile >			_logFile;
	::ZQ::common::Config::Holder< stBind >			_bind;
	::ZQ::common::Config::Holder< TMVSSIceLog >		_tmvssIceLog;
	::ZQ::common::Config::Holder< RTSPProp >		_rtspProp;
	::ZQ::common::Config::Holder< LocalInfo >		_localInfo;
	::ZQ::common::Config::Holder< SoapServerInfo >	_soapServerInfo;
	::ZQ::common::Config::Holder< PrivateData >		_privateData;
	::ZQ::common::Config::Holder< ResourceMap>		_resourceMap;

	static void structure(::ZQ::common::Config::Holder<TMVSSCfg> &holder)
	{
		//read default config
		holder.addDetail("",&TMVSSCfg::readCrashDump, &TMVSSCfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readIceTrace, &TMVSSCfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readIceStorm, &TMVSSCfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readIceProperties, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readDatabase, &TMVSSCfg::registerDatabase, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readPublishedLogs, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

		//read nss config
		holder.addDetail("",&TMVSSCfg::readLogFile, &TMVSSCfg::registerLogFile, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readstBind, &TMVSSCfg::registerstBind, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readtmvssIceLog, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readRTSPProp, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readLocalInfo, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readSoapServerInfo, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readPrivateData, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&TMVSSCfg::readResourceMap, &TMVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
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
	void readstBind(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<stBind> nvholder("");
		nvholder.read(node, hPP);
		_bind = nvholder;
	}
	void registerstBind(const std::string &full_path)
	{
		_bind.snmpRegister(full_path);
	}
	void readtmvssIceLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<TMVSSIceLog> nvholder("");
		nvholder.read(node, hPP);
		_tmvssIceLog = nvholder;
	}
	void readRTSPProp(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder< RTSPProp > nvholder("");
		nvholder.read(node, hPP);
		_rtspProp = nvholder;
	}
	void registerstPTSPProp(const std::string &full_path)
	{
		_rtspProp.snmpRegister(full_path);
	}
	void readLocalInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<LocalInfo> nvholder("");
		nvholder.read(node, hPP);
		_localInfo = nvholder;
	}
	void registerstLocalInfo(const std::string &full_path)
	{
		_localInfo.snmpRegister(full_path);
	}
	void readSoapServerInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<SoapServerInfo> nvholder("");
		nvholder.read(node, hPP);
		_soapServerInfo = nvholder;
	}
	void registerstSoapServerInfo(const std::string &full_path)
	{
		_soapServerInfo.snmpRegister(full_path);
	}
	void readPrivateData(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<PrivateData> nvholder("");
		nvholder.read(node, hPP);
		_privateData = nvholder;
	}
	void registerstPrivateData(const std::string &full_path)
	{
		_privateData.snmpRegister(full_path);
	}
	void readResourceMap(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<ResourceMap> nvholder("");
		nvholder.read(node, hPP);
		_resourceMap = nvholder;
	}
	void registersResourceMap(const std::string &full_path)
	{
		_resourceMap.snmpRegister(full_path);
	}
	void registerNothing(const std::string&){}
};

}//namespace TM

}//namespace VSS

}//namespace ZQTianShan

#endif __TMVSSCfgLOADER_H__