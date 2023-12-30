#ifndef _RTSP_PROXY_CONFIG_H__
#define _RTSP_PROXY_CONFIG_H__
//#include <configloader.h>
#include <ConfigHelper.h>

#pragma warning(disable:4786)



struct MonitoredLog
{
    std::string name;
    std::string syntax;
    static void structure( ZQ::common::Config::Holder<MonitoredLog> &holder)
    {
        holder.addDetail("", "path", &MonitoredLog::name);
        holder.addDetail("", "syntax", &MonitoredLog::syntax);
    }
};

struct RtspReqeustApplication
{
	std::string	path;
	std::string handler;
	static void structure( ZQ::common::Config::Holder<RtspReqeustApplication> &holder )
	{
		holder.addDetail("","path",&RtspReqeustApplication::path);
		holder.addDetail("","handler",&RtspReqeustApplication::handler);
	}
};
struct RtspRequestSite
{
	std::string		siteName;
	std::map<std::string,std::string>	handlers;
	
	static void structure( ZQ::common::Config::Holder<RtspRequestSite> &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("",
						"name",
						&RtspRequestSite::siteName);
		holder.addDetail("Application",
						&RtspRequestSite::readApplication,
						&RtspRequestSite::registerNothing);			
	}
	void readApplication( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspReqeustApplication> applicationHolder;
		applicationHolder.read(node, hPP);			
		handlers[applicationHolder.path] = applicationHolder.handler;
	}
	void registerNothing( const std::string& )
	{//do nothing
	}
};
struct RtspDefaultSite 
{
	std::map<std::string,std::string>	handlers;
	static void structure( ZQ::common::Config::Holder<RtspDefaultSite> &holder )
	{
		holder.addDetail("Application",
						&RtspDefaultSite::readDefaultApplication,
						&RtspDefaultSite::registerNothing);
	}
	void readDefaultApplication( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspReqeustApplication> applicationHolder;
		applicationHolder.read(node, hPP);			
		handlers[applicationHolder.path] = applicationHolder.handler;
	}
	void registerNothing( const std::string& )
	{//do nothing
	}
};

struct RtspRequestPlugin 
{
	std::string		filePath;
	std::string		configPath;
	static void structure( ZQ::common::Config::Holder<RtspRequestPlugin> &holder )
	{
		holder.addDetail("",
							"file",
							&RtspRequestPlugin::filePath);
		holder.addDetail("",
							"configuration",
							&RtspRequestPlugin::configPath);
	}
};

struct RtspRequestHandler 
{

	typedef std::map<std::string , std::string> Apphandlers;
	std::map<std::string , Apphandlers >	sites;
	struct PluginAttr 
	{
		std::string			filePath;
		std::string			configFile;
	};
	std::vector<PluginAttr>						plugins;
	char										defaultHandler[512];

	static void structure(ZQ::common::Config::Holder<RtspRequestHandler>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("",
						"defaultHandler",
						(Holder<RtspRequestHandler>::PMem_CharArray)&RtspRequestHandler::defaultHandler,
						sizeof(holder.defaultHandler),
						"",
						optReadOnly);

		holder.addDetail("",
						&RtspRequestHandler::readDefaultApplication,
						&RtspRequestHandler::registerNothing);
		holder.addDetail("Site",
						&RtspRequestHandler::readSite,
						&RtspRequestHandler::registerNothing);
		holder.addDetail("Plugin/module",
						&RtspRequestHandler::readPlugin ,
						&RtspRequestHandler::registerNothing);
	}
	void readDefaultApplication( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspDefaultSite> siteHolder;
		siteHolder.read(node, hPP);			
		sites["."] = siteHolder.handlers;
	}

	void readSite( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspRequestSite> siteHolder;
		siteHolder.read(node,hPP);
		sites[siteHolder.siteName] = siteHolder.handlers;
	}
	void readPlugin( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
	{
		using namespace ZQ::common::Config;
		Holder<RtspRequestPlugin> pluginHolder;
		//PluginAttr
		pluginHolder.read( node , hPP );
		PluginAttr attr;
		attr.filePath = pluginHolder.filePath;
		attr.configFile = pluginHolder.configPath;
		plugins.push_back(attr);
	}
	void registerNothing( const std::string& )
	{//do nothing

	}
	std::vector<std::string>	GetSiteName( )
	{
		std::vector<std::string> result;
		result.clear();
		std::map<std::string , Apphandlers >::const_iterator it  = sites.begin();
		for ( ; it != sites.end() ; it ++ ) 
		{
			result.push_back(it->first);
		}
		return result;
	}
	void GetPluginPathAndInfo(std::vector<PluginAttr>& pluginInfo)
	{
		pluginInfo = plugins;
	}
	std::string GetDefaultContenHandler( )
	{
		return std::string(defaultHandler);
	}
	bool GetVSitePathFromHandler(	const std::string& siteName, 
									const std::string& handler, 
									std::vector<std::string>& path )
	{
		std::map<std::string , Apphandlers >::const_iterator itSite = sites.find(siteName);
		if( itSite == sites.end( ) )
			return false;
		Apphandlers::const_iterator itAppHanlder = itSite->second.begin();
		for( ; itAppHanlder != itSite->second.end() ; itAppHanlder ++ )
		{
			if ( itAppHanlder->second == handler )
			{
				path.push_back(itAppHanlder->first);
			}
		}
		return true;
	}
	
};

struct RtspProxyCfg 
{
	RtspProxyCfg()
	{
		lRestartViaCrash = 0;
	}
	char			szMiniDumpPath[512];		//crash dump directory
	int32			lEnableMiniDump;			//1 for enable 0 for disable


	int32			lEventSinkTimeout;
	int32			lServiceThreadCount;
	
	int32			szServiceLogName[512];
	int32			lServiceLogLevel;
	int32			lServiceLogSize;
	int32			lServiceLogBuffer;
	int32			lServiceLogTimiout;

	char			szPluginLogName[512];
	int32			lPluginLogLevel;
	int32			lPluginLogSize;
	int32			lPluginLogBuffer;
	int32			lPluginLogTimeout;	

	int32			lRestartAtBusyHang;
	int32			lRestartViaCrash;
	int32			lGetParaToPingAtBusy;
	int32			lMaxPendingRequest;
	int32			lLowPriorityRequestTimeout;
	int32			lMaxMsgSize;
	int32			lRtspParserThreadPoolSize;
	int32			lRtspParserThreadPriority;
	int32			lDummyPing;
	int32			lProcessPriority;
	int32			lEnableUseLocaltimeInDatHeader;

	int32			lRtspSocketServerPort;
	int32			lMaxClientConnection;
	int32			lServiceFrameThreadPoolSize;
	int32			lServiceFrameThreadPriority;
	int32			lServiceFrameDebugLevel;
	int32			lServiceFrameDebugDetail;
	int32			lServiceFrameIdleTimeout;
	int32			lServiceFrameIdleScanInterval;

	int32			lServiceFrameSSLEnabled;
	char			szServiceFramepublicKeyFile[512];
	char			szServiceFrameprivateKeyFile[512];
	char			szServiceFrameprivatePassword[512];
	char			szServiceFramedhParamFile[512];
	char			szServiceFramerandFile[512];

	int32			lIncomingMsgMaxLen;
	int32			lIncomingMsgRecvBufSize;
	int32			lLogHexDump;

	int32			lLscpSocketServerPort;

	int32			lServiceFrameIPv6Enabled;

	int32			lCurTimeZone;

	int32			statusCheckInterval;
	int32			lMaxSessionCount;

	//begin internal use
	int32			lLscpPort;
	char            szLscpIPv4[256];
	char            szLscpIPv6[256];
	int32			lRtspPort;
	char            szRtspIPv4[256];
	char            szRtspIPv6[256];
	int32			lListenPort;
	//end internal use

	//add by zjm
	char           szLscpSSLPort[32];
	char           szRtspSSLPort[32];
	int32          lReadBufferSize;
	int32          lEncryptBufferSize;

	int32			lUseLongSessionId;
	//license File
	char		licenseFile[512];
	int64		licenseTime;

    std::vector<MonitoredLog> monitoredLogs;
    std::map<std::string, std::string> iceProperties;
	ZQ::common::Config::Holder<RtspRequestHandler>		reqesutHanlders;

	static void structure(ZQ::common::Config::Holder<RtspProxyCfg> &holder)
	{
		using namespace ZQ::common::Config;
		holder.addDetail("default/CrashDump", 
							"path", 
							(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szMiniDumpPath, 
							sizeof(holder.szMiniDumpPath), 
							NULL, 
							optReadOnly);
        holder.addDetail("default/CrashDump",
							"enabled",	
							&RtspProxyCfg::lEnableMiniDump, 
							"0", 
							optReadOnly);
		holder.addDetail("RtspProxy/PriorityOfProcess",
							"priority",
							&RtspProxyCfg::lProcessPriority,
							"1",
							optReadOnly);
		holder.addDetail("RtspProxy/RequestProcess",
							"threads",
							&RtspProxyCfg::lServiceThreadCount,
							"25",
							optReadOnly);
		//lGetParaToPingAtBusy
		holder.addDetail("RtspProxy/RequestProcess",
							"GP2PingAtBusy",
							&RtspProxyCfg::lGetParaToPingAtBusy,
							"0",
							optReadOnly);
		holder.addDetail("RtspProxy/RequestProcess",
							"maxPendingRequest",
							&RtspProxyCfg::lMaxPendingRequest,
							"0",
							optReadWrite);
		holder.addDetail("RtspProxy/RequestProcess",
							"statusCheckInterval",
							&RtspProxyCfg::statusCheckInterval,
							"300000",
							optReadWrite);
		
		holder.addDetail("RtspProxy/RequestProcess",
							"lowPriorityRequestTimeout",
							&RtspProxyCfg::lLowPriorityRequestTimeout,
							"10000",
							optReadWrite);
		
		holder.addDetail("RtspProxy/RequestProcess",
							"restarAtBusyHang",
							&RtspProxyCfg::lRestartAtBusyHang,
							"10000",
							optReadWrite);
		holder.addDetail("RtspProxy/RequestProcess",
							"restartViaCrash",
							&RtspProxyCfg::lRestartViaCrash,
							"0",
							optReadWrite);

		holder.addDetail("RtspProxy/RequestParse",
							"threads",
							&RtspProxyCfg::lRtspParserThreadPoolSize,
							"24",
							optReadOnly);
		holder.addDetail("RtspProxy/RequestParse",
							"priority",
							&RtspProxyCfg::lRtspParserThreadPriority,
							"2",
							optReadOnly);

		holder.addDetail("RtspProxy/RequestParse",
			"dummyPing",	
			&RtspProxyCfg::lDummyPing, 
			"0", 
			optReadOnly);

		// add by zjm
		holder.addDetail("RtspProxy/RequestParse",
			"readBufferSize",
			&RtspProxyCfg::lReadBufferSize,
			"4096",
			optReadOnly);
		holder.addDetail("RtspProxy/RequestParse",
			"encryptBufferSize",
			&RtspProxyCfg::lEncryptBufferSize,
			"4096",
			optReadOnly);

		holder.addDetail("RtspProxy/EventPublisher",
							"timeout",
							&RtspProxyCfg::lEventSinkTimeout,
							"15000",
							optReadWrite);
		holder.addDetail("RtspProxy/Response/RtspHeader",
							"useLocaltime",
							&RtspProxyCfg::lEnableUseLocaltimeInDatHeader,
							"0",
							optReadWrite);

		// add by zjm
		holder.addDetail("RtspProxy/SocketServer",
			"rtspIPv4",
			(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szRtspIPv4,
			sizeof(holder.szRtspIPv4),
			"0.0.0.0",
			optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
			"rtspIPv6",
			(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szRtspIPv6,
			sizeof(holder.szRtspIPv6),
			"::",
			optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
			"rtspSSLPort",
			(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szRtspSSLPort,
			sizeof(holder.szRtspSSLPort),
			"5540",
			optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
			"lscpIPv4",
			(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szLscpIPv4,
			sizeof(holder.szLscpIPv4),
			"0.0.0.0",
			optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
			"lscpIPv6",
			(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szLscpIPv6,
			sizeof(holder.szLscpIPv6),
			"::",
			optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
			"lscpSSLPort",
			(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szLscpSSLPort,
			sizeof(holder.szLscpSSLPort),
			"5550",
			optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
							"rtspPort",
							&RtspProxyCfg::lRtspSocketServerPort,
							"554",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
							"lscpPort",
							&RtspProxyCfg::lLscpSocketServerPort,
							"5542",
							optReadOnly);
 		holder.addDetail("RtspProxy/SocketServer",
 							"maxConnections",
 							&RtspProxyCfg::lMaxClientConnection,
 							"850",
 							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
							"threads",
							&RtspProxyCfg::lServiceFrameThreadPoolSize,
							"20",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
							"threadPriority",
							&RtspProxyCfg::lServiceFrameThreadPriority,
							"2",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
							"debugLevel",
							&RtspProxyCfg::lServiceFrameDebugLevel,
							"4",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
							"debugDetail",
							&RtspProxyCfg::lServiceFrameDebugDetail,
							"3",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
							"idleTimeout",
							&RtspProxyCfg::lServiceFrameIdleTimeout,
							"300000",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer",
							"idleScanInterval",
							&RtspProxyCfg::lServiceFrameIdleScanInterval,
							"500",
							optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
 							"maxSessions",
 							&RtspProxyCfg::lMaxSessionCount,
 							"10000",
							optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
							"license",
							(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::licenseFile,
							sizeof(holder.licenseFile),
							"",
							optReadOnly);

		holder.addDetail("RtspProxy/SocketServer",
							"enableIPv6",
							&RtspProxyCfg::lServiceFrameIPv6Enabled,
							"0",
							optReadOnly);

		//lUseLongSessionId
		holder.addDetail("RtspProxy/SocketServer",
							"useLongSessionId",
							&RtspProxyCfg::lUseLongSessionId,
							"0",
							optReadOnly);

		holder.addDetail("RtspProxy/SocketServer/SSL",
							"enabled",
							&RtspProxyCfg::lServiceFrameSSLEnabled,
							"0",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer/SSL",
							"publicKeyFile",
							(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szServiceFramepublicKeyFile, 
							sizeof(holder.szServiceFramepublicKeyFile), 
							"",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer/SSL",
							"privateKeyFile",
							(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szServiceFrameprivateKeyFile,   
							sizeof(holder.szServiceFrameprivateKeyFile),
							"",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer/SSL",
							"privatePassword",							
							(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szServiceFrameprivatePassword,   
							sizeof(holder.szServiceFrameprivatePassword),
							"",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer/SSL",
							"dhParamFile",
							(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szServiceFramedhParamFile,
							sizeof(holder.szServiceFramedhParamFile),
							"",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer/SSL",
							"randFile",
							(Holder<RtspProxyCfg>::PMem_CharArray)&RtspProxyCfg::szServiceFramerandFile,
							sizeof(holder.szServiceFramerandFile),
							"",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer/IncomingMessage",
							"maxLen",
							&RtspProxyCfg::lIncomingMsgMaxLen,
							"32768",
							optReadOnly);
		holder.addDetail("RtspProxy/SocketServer/IncomingMessage",
							"recvBufSize",
							&RtspProxyCfg::lIncomingMsgRecvBufSize,
							"65536",
							optReadOnly);

		holder.addDetail("RtspProxy/SocketServer/IncomingMessage",
			"hexDump",
			&RtspProxyCfg::lLogHexDump,
			"0",
			optReadWrite);

		holder.addDetail("RtspProxy/RequestHandler",
							&RtspProxyCfg::readReqesutHandler,
							&RtspProxyCfg::registerNothing)	;	
	}
	void readReqesutHandler(  ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP  )
	{
		using namespace ZQ::common::Config;		
		reqesutHanlders.read(node,hPP);
	}
	void registerNothing( const std::string&)
	{
	}
	//return a bundle of site
};

extern ZQ::common::Config::Loader<RtspProxyCfg>		gRtspProxyConfig;

#define GAPPLICATIONCONFIGURATION	gRtspProxyConfig



#endif//_RTSP_PROXY_CONFIG_H__
