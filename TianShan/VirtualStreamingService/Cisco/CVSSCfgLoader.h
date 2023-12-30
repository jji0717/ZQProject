#ifndef __CVSSCFGLOADER_H__
#define __CVSSCFGLOADER_H__

#include "CVSSConfig.h"
#include "A3Config.h"

namespace ZQTianShan{

namespace CVSS{

struct CVSSCfg
{
	//Default config
	::ZQ::common::Config::Holder< CrashDump >		_crashDump;
	::ZQ::common::Config::Holder< IceTrace	>		_iceTrace;
	::ZQ::common::Config::Holder< IceStorm	>		_iceStorm;
	IceProperties	_iceProperties;
	::ZQ::common::Config::Holder< Database	>		_dataBase;
	::ZQ::common::Config::Holder< PublishedLogs >	_publishedLogs;

	//CVSS config
	::ZQ::common::Config::Holder< LogFile >			_logFile;
	::ZQ::common::Config::Holder< stBind >			_bind;
	::ZQ::common::Config::Holder< IceLog >			_iceLog;
	::ZQ::common::Config::Holder< RTSPProp >		_rtspProp;
	::ZQ::common::Config::Holder< StreamingServer > _streamingServer;
	::ZQ::common::Config::Holder< SoapLog >			_soapLog;
	::ZQ::common::Config::Holder<StoreInfo >		_storeInfo;

	static void structure(::ZQ::common::Config::Holder<CVSSCfg> &holder)
	{
		//read default config
		holder.addDetail("",&CVSSCfg::readCrashDump, &CVSSCfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readIceTrace, &CVSSCfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readIceStorm, &CVSSCfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readIceProperties, &CVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readDatabase, &CVSSCfg::registerDatabase, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readPublishedLogs, &CVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

		//read CVSS config
		holder.addDetail("",&CVSSCfg::readLogFile, &CVSSCfg::registerLogFile, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readBind, &CVSSCfg::registerstBind, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readIceLog, &CVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readRTSPProp, &CVSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readStreamingServer, &CVSSCfg::registerstStreamingServer, ::ZQ::common::Config::Range(0,1));
		
		//read A3 config
		holder.addDetail("",&CVSSCfg::readSoapLog, &CVSSCfg::registerstSoapLog, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&CVSSCfg::readStoreInfo, &CVSSCfg::registerstStoreInfo, ::ZQ::common::Config::Range(0,1));
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
	void readBind(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<stBind> nvholder("");
		nvholder.read(node, hPP);
		_bind = nvholder;
	}
	void registerstBind(const std::string &full_path)
	{
		_bind.snmpRegister(full_path);
	}
	void readIceLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<IceLog> nvholder("");
		nvholder.read(node, hPP);
		_iceLog = nvholder;
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
	void readStreamingServer(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<StreamingServer> nvholder("");
		nvholder.read(node, hPP);
		_streamingServer = nvholder;
	}
	void registerstStreamingServer(const std::string &full_path)
	{
		_streamingServer.snmpRegister(full_path);
	}
	void readSoapLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<SoapLog> nvholder("");
		nvholder.read(node, hPP);
		_soapLog = nvholder;
	}
	void registerstSoapLog(const std::string &full_path)
	{
		_soapLog.snmpRegister(full_path);
	}
	void readStoreInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<StoreInfo> nvholder("");
		nvholder.read(node, hPP);
		_storeInfo = nvholder;
	}
	void registerstStoreInfo(const std::string &full_path)
	{
		_storeInfo.snmpRegister(full_path);
	}
	void registerNothing(const std::string&){}
};

}//namespace CVSS

}//namespace ZQTianShan

#endif __CVSSCFGLOADER_H__