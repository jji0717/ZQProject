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
	::ZQ::common::Config::Holder< IceStorm	>		_iceStorm;
	IceProperties	_iceProperties;
	::ZQ::common::Config::Holder< Database	>		_dataBase;
	::ZQ::common::Config::Holder< PublishedLogs >	_publishedLogs;

	//NSS config
	::ZQ::common::Config::Holder< LogFile >			_logFile;
	::ZQ::common::Config::Holder< stBind >			_stBind;
	//nssLog			_nssLog;
	::ZQ::common::Config::Holder< nssIceLog >		_nssIceLog;
	::ZQ::common::Config::Holder< TimeOut >			_timeOut;
	MediaCluster	_mediaCluster;
	SessionGroup	_sessionGroup;

	//DEBUG mode
	//Debug			_debug;

	//A3CS config
	::ZQ::common::Config::Holder< ::ZQTianShan::A3::A3Log >			_a3Log;
	::ZQ::common::Config::Holder< ::ZQTianShan::A3::A3StoreInfo >	_a3StoreInfo;
	::ZQ::common::Config::Holder< ::ZQTianShan::A3::A3EventLog >	_a3EventLog;
	::ZQ::common::Config::Holder< ::ZQTianShan::A3::A3LocalInfo >	_a3LocalInfo;
	::ZQ::common::Config::Holder< ::ZQTianShan::A3::A3ServerInfo >	_a3ServerInfo;
	::ZQ::common::Config::Holder< ::ZQTianShan::A3::A3VolumeInfo >	_a3VolumeInfo;
	::ZQ::common::Config::Holder< ::ZQTianShan::A3::A3HeartBeat >	_a3HeartBeat;

	static void structure(::ZQ::common::Config::Holder<NSSCfg> &holder)
	{
		//read default config
		holder.addDetail("",&NSSCfg::readCrashDump, &NSSCfg::registerCrashDump, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readIceTrace, &NSSCfg::registerIceTrace, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readIceStorm, &NSSCfg::registerIceStorm, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readIceProperties, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readDatabase, &NSSCfg::registerDatabase, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readPublishedLogs, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

		//read nss config
		holder.addDetail("",&NSSCfg::readLogFile, &NSSCfg::registerLogFile, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readstBind, &NSSCfg::registerstBind, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&NSSCfg::readnssLog, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readnssIceLog, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readtimeOut, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readMediaCluster, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readSessionGroup, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));
		//holder.addDetail("",&NSSCfg::readDebug, &NSSCfg::registerNothing, ::ZQ::common::Config::Range(0,1));

		//read A3 config
		holder.addDetail("",&NSSCfg::readA3Log, &NSSCfg::registerA3Log, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readA3StoreInfo, &NSSCfg::registerA3StoreInfo, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readA3EventLog, &NSSCfg::registerA3EventLog, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readA3LocalInfo, &NSSCfg::registerA3LocalInfo, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readA3ServerInfo, &NSSCfg::registerA3ServerInfo, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readA3VolumeInfo, &NSSCfg::registerA3VolumeInfo, ::ZQ::common::Config::Range(0,1));
		holder.addDetail("",&NSSCfg::readA3HeartBeat, &NSSCfg::registerA3HeartBeat, ::ZQ::common::Config::Range(0,1));
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
		_stBind = nvholder;
	}
	void registerstBind(const std::string &full_path)
	{
		_stBind.snmpRegister(full_path);
	}
	//void readnssLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	//{
	//	::ZQ::common::Config::Holder<nssLog> nvholder("");
	//	nvholder.read(node, hPP);
	//	_nssLog = nvholder;
	//}
	void readnssIceLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<nssIceLog> nvholder("");
		nvholder.read(node, hPP);
		_nssIceLog = nvholder;
	}
	void readtimeOut(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<TimeOut> nvholder("");
		nvholder.read(node, hPP);
		_timeOut = nvholder;
	}
	void registersttimeOut(const std::string &full_path)
	{
		_timeOut.snmpRegister(full_path);
	}
	void readMediaCluster(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<MediaCluster> nvholder("");
		nvholder.read(node, hPP);
		_mediaCluster = nvholder;
	}
	void readSessionGroup(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<SessionGroup> nvholder("");
		nvholder.read(node, hPP);
		_sessionGroup = nvholder;
	}
	//void readDebug(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	//{
	//	::ZQ::common::Config::Holder<Debug> nvholder("");
	//	nvholder.read(node, hPP);
	//	_debug = nvholder;
	//}

	//A3Log
	void readA3Log(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<::ZQTianShan::A3::A3Log> nvholder("");
		nvholder.read(node, hPP);
		_a3Log = nvholder;
	}
	void registerA3Log(const std::string &full_path)
	{
		_a3Log.snmpRegister(full_path);
	}
	void readA3StoreInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<::ZQTianShan::A3::A3StoreInfo> nvholder("");
		nvholder.read(node, hPP);
		_a3StoreInfo = nvholder;
	}
	void registerA3StoreInfo(const std::string &full_path)
	{
		_a3StoreInfo.snmpRegister(full_path);
	}
	void readA3EventLog(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<::ZQTianShan::A3::A3EventLog> nvholder("");
		nvholder.read(node, hPP);
		_a3EventLog = nvholder;
	}
	void registerA3EventLog(const std::string &full_path)
	{
		_a3EventLog.snmpRegister(full_path);
	}
	void readA3LocalInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<::ZQTianShan::A3::A3LocalInfo> nvholder("");
		nvholder.read(node, hPP);
		_a3LocalInfo = nvholder;
	}
	void registerA3LocalInfo(const std::string &full_path)
	{
		_a3LocalInfo.snmpRegister(full_path);
	}
	void readA3ServerInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<::ZQTianShan::A3::A3ServerInfo> nvholder("");
		nvholder.read(node, hPP);
		_a3ServerInfo = nvholder;
	}
	void registerA3ServerInfo(const std::string &full_path)
	{
		_a3ServerInfo.snmpRegister(full_path);
	}
	void readA3VolumeInfo(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<::ZQTianShan::A3::A3VolumeInfo> nvholder("");
		nvholder.read(node, hPP);
		_a3VolumeInfo = nvholder;
	}
	void registerA3VolumeInfo(const std::string &full_path)
	{
		_a3VolumeInfo.snmpRegister(full_path);
	}
	void readA3HeartBeat(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<::ZQTianShan::A3::A3HeartBeat> nvholder("");
		nvholder.read(node, hPP);
		_a3HeartBeat = nvholder;
	}
	void registerA3HeartBeat(const std::string &full_path)
	{
		_a3HeartBeat.snmpRegister(full_path);
	}
	void registerNothing(const std::string&){}
};

}//namespace NSS

}//namespace ZQTianShan

#endif __NSSCFGLOADER_H__