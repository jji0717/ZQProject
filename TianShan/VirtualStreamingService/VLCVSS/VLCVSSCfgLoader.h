#ifndef __VLCVSSCfgLOADER_H__
#define __VLCVSSCfgLOADER_H__

#include "VLCVSSConfig.h"

namespace ZQTianShan{

namespace VSS{

namespace VLC{

struct VLCVSSCfg
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
	::ZQ::common::Config::Holder< VLCVSSIceLog >	_VLCVSSIceLog;
	::ZQ::common::Config::Holder< TelnetProp >		_telnetProp;
	::ZQ::common::Config::Holder<StreamServiceProp> _streamServiceProp;
	::ZQ::common::Config::Holder< VLCServiceProp>   _VLCServiceProp;
	::ZQ::common::Config::Holder< TelnetServerInfo >_telnetServerInfo;
	::ZQ::common::Config::Holder< VolumnInfo >		_volumnInfo;
	::ZQ::common::Config::Holder< StoreInfo >		_storeInfo;

	static void structure(::ZQ::common::Config::Holder<VLCVSSCfg> &holder)
	{
		//read default config
		holder.addDetail("",&VLCVSSCfg::readCrashDump, &VLCVSSCfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readIceTrace, &VLCVSSCfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readIceStorm, &VLCVSSCfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readIceProperties, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readDatabase, &VLCVSSCfg::registerDatabase, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readPublishedLogs, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

		//read nss config
		holder.addDetail("",&VLCVSSCfg::readLogFile, &VLCVSSCfg::registerLogFile, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readstBind, &VLCVSSCfg::registerstBind, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readVLCVSSIceLog, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readTelnetProp, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readStreamServiceProp, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readVLCServiceProp, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readTelnetServerInfo, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readVolumnInfo, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&VLCVSSCfg::readStoreInfo, &VLCVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
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
	void readVLCVSSIceLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<VLCVSSIceLog> nvholder("");
		nvholder.read(node, hPP);
		_VLCVSSIceLog = nvholder;
	}
	void readTelnetProp(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder< TelnetProp > nvholder("");
		nvholder.read(node, hPP);
		_telnetProp = nvholder;
	}
	void registerstTelnetProp(const std::string &full_path)
	{
		_telnetProp.snmpRegister(full_path);
	}
	
	void readStreamServiceProp(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder< StreamServiceProp> nvholder("");
		nvholder.read(node, hPP);
		_streamServiceProp = nvholder;
	}

	void readVLCServiceProp(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder< VLCServiceProp> nvholder("");
		nvholder.read(node, hPP);
		_VLCServiceProp = nvholder;
	}


	void registerstStreamServiceProp(const std::string &full_path)
	{
		_streamServiceProp.snmpRegister(full_path);
	}

	void readTelnetServerInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder< TelnetServerInfo > nvholder("");
		nvholder.read(node, hPP);
		_telnetServerInfo = nvholder;
	}
	void registerstTelnetServerInfo(const std::string &full_path)
	{
		_telnetServerInfo.snmpRegister(full_path);
	}
	
	void readVolumnInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder< VolumnInfo > nvholder("");
		nvholder.read(node, hPP);
		_volumnInfo = nvholder;
	}
	void registerstVolumnInfo(const std::string &full_path)
	{
		_volumnInfo.snmpRegister(full_path);
	}

	void readStoreInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder< StoreInfo > nvholder("");
		nvholder.read(node, hPP);
		_storeInfo = nvholder;
	}
	void registerstStoreInfo(const std::string &full_path)
	{
		_storeInfo.snmpRegister(full_path);
	}
	
	void registerNothing(const std::string&){}
};

}//namespace VLC

}//namespace VSS

}//namespace ZQTianShan

#endif __VLCVSSCfgLOADER_H__