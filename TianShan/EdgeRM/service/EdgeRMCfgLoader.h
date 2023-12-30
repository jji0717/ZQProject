#ifndef __ZQTianShan_EdgeRMCfgLoader_H_
#define __ZQTianShan_EdgeRMCfgLoader_H_

#include <ConfigHelper.h>
namespace ZQTianShan{
namespace EdgeRM{

struct stVrepServer
{
	int32          enable;
	std::string    ipAddress;
	int32          port;
	int32          timeInterval;

	stVrepServer()
	{
		enable = 0;
		ipAddress = "";
		port = 0;
		timeInterval = 0;
	};

	static void structure(ZQ::common::Config::Holder< stVrepServer > &holder)
	{
		holder.addDetail("", "enable", &stVrepServer::enable, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "ip", &stVrepServer::ipAddress, "127.0.0.1", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "port", &stVrepServer::port, "2234", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "interval", &stVrepServer::timeInterval, "60000", ZQ::common::Config::optReadOnly);
	}
};
struct MonitoredLog
{
	std::string name;
	std::string syntax;
	std::string syntaxKey;
	std::string logType;
	static void structure(ZQ::common::Config::Holder<MonitoredLog> &holder)
	{
		holder.addDetail("", "path", &MonitoredLog::name);
		holder.addDetail("", "syntax", &MonitoredLog::syntax);
		holder.addDetail("", "key", &MonitoredLog::syntaxKey);
		holder.addDetail("", "type", &MonitoredLog::logType);
	}
};
struct ERMIClientCfg
{
	int32 enable;
	int32 sessionTimeOut;
	int32 maxSessionGroups;
	int32 GroupSyncInterval;
	int32 maxSessionCount;
	std::string bindAddress;

	static void structure(ZQ::common::Config::Holder<ERMIClientCfg> &holder)
	{
		holder.addDetail("", "enable", &ERMIClientCfg::enable, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessionCount", &ERMIClientCfg::maxSessionCount, "300", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "sessionTimeOut", &ERMIClientCfg::sessionTimeOut, "60000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "GroupSyncInterval", &ERMIClientCfg::GroupSyncInterval, "600000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "bindAddress", &ERMIClientCfg::bindAddress, "", ZQ::common::Config::optReadOnly);
	}
	ERMIClientCfg()
	{
        enable = 0;
		maxSessionCount = 3000;
		sessionTimeOut = 600000;
		GroupSyncInterval = 600000;
		bindAddress = "";
	};
};
struct R6ClientCfg
{
	int32 enable;
	int32 sessionTimeOut;
	int32 maxSessionGroups;
	int32 GroupSyncInterval;
	int32 maxSessionCount;
	std::string bindAddress;

	static void structure(ZQ::common::Config::Holder<R6ClientCfg> &holder)
	{
		holder.addDetail("", "enable", &R6ClientCfg::enable, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessionCount", &R6ClientCfg::maxSessionCount, "300", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "sessionTimeOut", &R6ClientCfg::sessionTimeOut, "60000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "GroupSyncInterval", &R6ClientCfg::GroupSyncInterval, "600000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "bindAddress", &R6ClientCfg::bindAddress, "", ZQ::common::Config::optReadOnly);
	}
	R6ClientCfg()
	{
		enable = 0;
		maxSessionCount = 3000;
		sessionTimeOut = 600000;
		GroupSyncInterval = 600000;
		bindAddress = "";
	};
};
struct TripServer
{
	int32		   enable;
	std::string    ip;
	std::string    port;
	int32          receiveThreads;
	int32		   processThreads;

	TripServer()
	{
		enable = 0;
		ip = "";
		port = "6069";
		receiveThreads = 30;
		processThreads = 30;
	};

	static void structure(ZQ::common::Config::Holder< TripServer > &holder)
	{
		holder.addDetail("", "enable", &TripServer::enable, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "ip", &TripServer::ip, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "port", &TripServer::port, "6069", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "receiveThreads", &TripServer::receiveThreads, "30", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "processThreads", &TripServer::processThreads, "30", ZQ::common::Config::optReadOnly);

	}
};
struct ERMS
{
	std::string netId;
	std::string endpoint;
	int enabled;

	static void structure(ZQ::common::Config::Holder<ERMS> &holder)
	{
		holder.addDetail("", "netId", &ERMS::netId);
		holder.addDetail("", "endpoint", &ERMS::endpoint);
		holder.addDetail("", "enabled", &ERMS::enabled);
	}
};
struct Backup
{
	std::string    mode;
	int32          syncInterval;
	int32          syncDeviceInter;
	int32          enableSync;
	int32		  exportTimeout;

    std::vector<ERMS> erms;

	Backup()
	{
		mode = "standby";
		syncInterval = 300000;
		enableSync = 0;
		syncDeviceInter = 1800;
		exportTimeout = 90000;
	};

	static void structure(ZQ::common::Config::Holder< Backup > &holder)
	{
		holder.addDetail("", "mode", &Backup::mode, "standby", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "enableSync", &Backup::enableSync, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "interval", &Backup::syncInterval, "120000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "exportTimeout", &Backup::exportTimeout, "90000", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "syncDeviceInter", &Backup::syncDeviceInter, "90000", ZQ::common::Config::optReadOnly);

		holder.addDetail("ERM", &Backup::readERMs, &Backup::registerNothing);
	}

	void readERMs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
	{
		using namespace ZQ::common::Config;
		Holder<ERMS> ermHolder;
		ermHolder.read(node, hPP);
		if(ermHolder.enabled)
			erms.push_back(ermHolder);
		if(stricmp(mode.c_str(), "standby") && enableSync && erms.size() > 1 )
		{
			ZQ::common::throwf<ZQ::common::CfgException>(EXFMT(ZQ::common::CfgException, "Backup[standby mode] allowed to have only one ERM"));
		}
	}
	void registerNothing(const std::string&){}
};
struct EdgeRMCfgLoader
{
	//default crashdump
	std::string crashdumpPath;
	int32       enableCrashdump;

	//default icetrace
	int32	iceTraceLevel;
	int32	iceTraceSize;
	int32   iceTraceCount;
	int32   iceTraceBuffersize;
	int32   iceTraceFlashtimeout;
    //default eventchannel
	std::string eventchanneEndpoint;

    //default ice properties
	std::map<std::string, std::string> iceProperties;
	//default publishlogs
     std::vector<MonitoredLog> monitoredLogs;
	//default database
	std::string databasePath;
	std::string databaseRuntimepath;

	// EdgeRM Bind 
	std::string  edgeRMEndpoint;
	int32		 dispatchSize;
	int32		 dispatchMax;
	int32		 evictorSize;
	int32		 threadPoolSize;
	int32		 clientPoolSize;
	int32		 autoLink;
	std::string  netId;

    //EdgeRM Alloction
	int32			allocationLease;
	int32           retryInterval;
	int32           retrytimes;

    //EdgeRM RtspEngine
	::std::string ipv4;
	::std::string ipv6;
	::std::string tcpPort;
	::std::string udpPort;
	::std::string sslPort;
	int32 receiveThreads;
	int32 processTheads;
	::std::string logfilename;
	int32 lLogLevel;
	int32 lLogFileSize;
	int32 llogFileCount;
	int32 lLogBufferSize;
	int32 lLogWriteTimteout;

	//EdgeRM SSL
	int32 enabledSSL;
	::std::string publicKeyFile;
	::std::string privateKeyFile;
	::std::string privatePassword;
	::std::string dhParamFile;
	::std::string randFile;


	ERMIClientCfg ermiClient;
	R6ClientCfg   r6Client;
	::ZQ::common::Config::Holder< stVrepServer >	_stVrepServer;

	TripServer _tripServer;

	Backup  _backup;

	static void structure(::ZQ::common::Config::Holder<EdgeRMCfgLoader> &holder)
	{
		//read default crash
		holder.addDetail("default/CrashDump", "path", &EdgeRMCfgLoader::crashdumpPath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("default/CrashDump", "enabled", &EdgeRMCfgLoader::enableCrashdump, "1", ZQ::common::Config::optReadOnly);
		//read default icetrace
		holder.addDetail("default/IceTrace", "level", &EdgeRMCfgLoader::iceTraceLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/IceTrace", "size", &EdgeRMCfgLoader::iceTraceSize, "10240000", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/IceTrace", "count", &EdgeRMCfgLoader::iceTraceCount, "5", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/IceTrace", "buffer", &EdgeRMCfgLoader::iceTraceBuffersize, "204800", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/IceTrace", "flushtimeout", &EdgeRMCfgLoader::iceTraceFlashtimeout, "2", ZQ::common::Config::optReadOnly);
        //read default eventchannel
		holder.addDetail("default/EventChannel", "endPoint", &EdgeRMCfgLoader::eventchanneEndpoint, NULL, ZQ::common::Config::optReadOnly);
		//read default IceProp
		holder.addDetail("default/IceProperties/prop", &EdgeRMCfgLoader::readIceProperties, &EdgeRMCfgLoader::registerNothing);    
		//read default publishlogs
		holder.addDetail("default/PublishedLogs/Log", &EdgeRMCfgLoader::readMonitoredLog, &EdgeRMCfgLoader::registerNothing);
		//read default database
		holder.addDetail("default/Database", "path", &EdgeRMCfgLoader::databasePath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("default/Database", "runtimePath", &EdgeRMCfgLoader::databaseRuntimepath, NULL, ZQ::common::Config::optReadOnly);
        
		//read EdgeRM bind 	
		holder.addDetail("EdgeRM", "netId", &EdgeRMCfgLoader::netId, "netId", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Bind", "endpoint", &EdgeRMCfgLoader::edgeRMEndpoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Bind", "dispatchSize", &EdgeRMCfgLoader::dispatchSize, "30", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Bind", "dispatchMax", &EdgeRMCfgLoader::dispatchMax, "50", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Bind", "evictorSize", &EdgeRMCfgLoader::evictorSize, "1000", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Bind", "threadPoolSize", &EdgeRMCfgLoader::threadPoolSize, "30", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Bind", "clientPoolSize", &EdgeRMCfgLoader::clientPoolSize, "20", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Bind", "autoLink", &EdgeRMCfgLoader::autoLink, "0", ZQ::common::Config::optReadOnly);
		//holder.addDetail("EdgeRM/Bind", "netId", &EdgeRMCfgLoader::netId, "netId", ZQ::common::Config::optReadOnly);

        //read Allocation
		holder.addDetail("EdgeRM/Allocation", "allocationLease", &EdgeRMCfgLoader::allocationLease, "600000", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Allocation", "retryTimes", &EdgeRMCfgLoader::retrytimes, "3", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/Allocation", "retryInterval", &EdgeRMCfgLoader::retryInterval, "600000", ZQ::common::Config::optReadOnly);
        
		//read RtspEngine
		holder.addDetail("EdgeRM/RtspEngine", "rtspIPv4", &EdgeRMCfgLoader::ipv4, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "rtspIPv6", &EdgeRMCfgLoader::ipv6, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "rtspTCPPort", &EdgeRMCfgLoader::tcpPort, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "rtspSSLPort", &EdgeRMCfgLoader::sslPort, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "rtspUDPPort", &EdgeRMCfgLoader::udpPort, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "receiveThreads", &EdgeRMCfgLoader::receiveThreads, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "processThreads", &EdgeRMCfgLoader::processTheads, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "logfilename", &EdgeRMCfgLoader::logfilename, "RtspEngine.log", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "level", &EdgeRMCfgLoader::lLogLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "size", &EdgeRMCfgLoader::lLogFileSize, "40960000", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "count", &EdgeRMCfgLoader::llogFileCount, "5", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "buffer", &EdgeRMCfgLoader::lLogBufferSize, "10240", ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/RtspEngine", "flushtimeout", &EdgeRMCfgLoader::lLogWriteTimteout, "2", ZQ::common::Config::optReadOnly);

		//read SSL
		holder.addDetail("EdgeRM/SSL", "enabled", &EdgeRMCfgLoader::enabledSSL, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/SSL", "publicKeyFile", &EdgeRMCfgLoader::publicKeyFile, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/SSL", "privateKeyFile", &EdgeRMCfgLoader::privateKeyFile, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/SSL", "privatePassword", &EdgeRMCfgLoader::privatePassword, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/SSL", "dhParamFile", &EdgeRMCfgLoader::dhParamFile, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("EdgeRM/SSL", "randFile", &EdgeRMCfgLoader::randFile, NULL, ZQ::common::Config::optReadOnly);

		//read VerpServer config
		holder.addDetail("EdgeRM/VrepServer",&EdgeRMCfgLoader::readstVrepServer, &EdgeRMCfgLoader::registerNothing, ::ZQ::common::Config::Range(0,1));
        //read ermiclient config
		holder.addDetail("EdgeRM/ERMIClient",&EdgeRMCfgLoader::readERMIClient, &EdgeRMCfgLoader::registerNothing, ::ZQ::common::Config::Range(0,1));
		//read r6client config
		holder.addDetail("EdgeRM/R6Client",&EdgeRMCfgLoader::readR6Client, &EdgeRMCfgLoader::registerNothing, ::ZQ::common::Config::Range(0,1));
       //read trip server config
		holder.addDetail("EdgeRM/TripServer",&EdgeRMCfgLoader::readTripServer, &EdgeRMCfgLoader::registerNothing, ::ZQ::common::Config::Range(0,1));
	   //read Backup config
		holder.addDetail("EdgeRM/Backup",&EdgeRMCfgLoader::readBackup, &EdgeRMCfgLoader::registerNothing, ::ZQ::common::Config::Range(0,1));

	};

	void readIceProperties(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<ZQ::common::Config::NVPair> propHolder;
		propHolder.read(node, hPP);
		iceProperties[propHolder.name] = propHolder.value;
	}

	void readERMIClient(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<ERMIClientCfg> nvholder("");
		nvholder.read(node, hPP);
		ermiClient = nvholder;
	}	
	void readR6Client(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<R6ClientCfg> nvholder("");
		nvholder.read(node, hPP);
		r6Client = nvholder;
	}
	
	void readTripServer(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<TripServer> nvholder("");
		nvholder.read(node, hPP);
		_tripServer = nvholder;
	}
	void readstVrepServer(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<stVrepServer> nvholder("");
		nvholder.read(node, hPP);
		_stVrepServer = nvholder;
	}
	void readBackup(::ZQ::common::XMLUtil::XmlNode node, const ::ZQ::common::Preprocessor* hPP)
	{
		::ZQ::common::Config::Holder<Backup> nvholder("");
		nvholder.read(node, hPP);
		_backup = nvholder;
	}
	
	void readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
	{
		using namespace ZQ::common::Config;
		Holder<MonitoredLog> lmHolder;
		lmHolder.read(node, hPP);
		monitoredLogs.push_back(lmHolder);
	}
	void registerNothing(const std::string&){}
};

}//namespace EdgeRM

}//namespace ZQTianShan

#endif // __ZQTianShan_EdgeRMCfgLoader_H_
