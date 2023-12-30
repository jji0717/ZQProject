#ifndef __zq_dsmcc_gateway_configuration_header_file_h__
#define __zq_dsmcc_gateway_configuration_header_file_h__

#include <map>
#include <ConfigHelper.h>

namespace ZQ { namespace CLIENTREQUEST{ namespace Config {


struct ServerListener
{
	std::string	ip;
	std::string port;
	std::string type;
	std::string protocol;
	std::string exportAddress;

	static void structure( ZQ::common::Config::Holder<ServerListener>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","type",&ServerListener::type,"",optReadOnly);
		holder.addDetail("","ip",&ServerListener::ip,"0.0.0.0",optReadOnly);
		holder.addDetail("","port",&ServerListener::port,"",optReadOnly);
		holder.addDetail("","protocol",&ServerListener::protocol,"",optReadOnly);
		holder.addDetail("","export",&ServerListener::exportAddress,"",optReadOnly);
	}
};

struct SocketServer
{
	SocketServer()
	{
		threadcount			= 8;
		maxMsgLen			= 32*1024;
		recvBufSize			= 4*1024;
		hexDumpEnabled		= 0;
		connIdleTimeout		= 30 * 60 * 1000;
		maxConnection		= 100 * 1000;
	}

	int32		threadcount;
	int32		maxMsgLen;
	int32		recvBufSize;
	int32		hexDumpEnabled;
	int32		connIdleTimeout;
	int32		maxConnection;

	typedef ZQ::common::Config::Holder<ServerListener> ServerListenerHolder;
	std::vector<ServerListenerHolder> listeners;


	void readListener( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		ServerListenerHolder holder;
		holder.read( node , hPP );
		listeners.push_back(holder);
	}

	void registerNothing( const std::string &full_path )
	{//do nothing		
	}

	static void structure(ZQ::common::Config::Holder< SocketServer > &holder)
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","thread",&SocketServer::threadcount,"8",optReadOnly);
		holder.addDetail("","maxMessageLen",&SocketServer::maxMsgLen,"32768",optReadOnly);
		holder.addDetail("","recvBufSize",&SocketServer::recvBufSize,"4096",optReadOnly);
		holder.addDetail("","hexDump",&SocketServer::hexDumpEnabled,"0",optReadOnly);
		holder.addDetail("","connectionIdleTimeout",&SocketServer::connIdleTimeout,"300000",optReadOnly);
		holder.addDetail("","maxConnection",&SocketServer::maxConnection,"10000",optReadOnly);
		holder.addDetail("listen",&SocketServer::readListener,&SocketServer::registerNothing);		
	}
};

struct RequestHandler 
{
	std::string defaultHandler;

	struct RequestApplication 
	{
		std::string path;
		std::string handler;
		static void structure(ZQ::common::Config::Holder< RequestApplication > &holder)
		{
			using namespace ZQ::common::Config;
			holder.addDetail("","path",&RequestApplication::path,"",optReadOnly);
			holder.addDetail("","handler",&RequestApplication::handler,"",optReadOnly);
		}
	};

	struct RequestPlugin 
	{
		std::string plugingfile;
		std::string pluginconf;
		static void structure(ZQ::common::Config::Holder< RequestPlugin > &holder)
		{
			using namespace ZQ::common::Config;
			holder.addDetail("","file",&RequestPlugin::plugingfile,"",optReadOnly);
			holder.addDetail("","configuration",&RequestPlugin::pluginconf,"",optReadOnly);
		}
	};

	typedef ZQ::common::Config::Holder<RequestApplication> RequestApplicationHolder;
	std::map<std::string,std::string>	handlermap;

	typedef ZQ::common::Config::Holder<RequestPlugin> RequestPluginHolder;
	std::vector<RequestPluginHolder> plugins;


	void readRequestApplication( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		RequestApplicationHolder holder;
		holder.read(node,hPP);
		handlermap[holder.path] = holder.handler;
	}
	void readPlugin( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP  )
	{
		RequestPluginHolder holder;
		holder.read( node , hPP );
		plugins.push_back( holder );
	}
	
	void registerRequestPlugin( const std::string &full_path ){}
	void registerRequestApplication( const std::string &full_path )	{	}

	static void structure(ZQ::common::Config::Holder< RequestHandler > &holder)
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","defaultHandler",&RequestHandler::defaultHandler,"",optReadOnly);
		holder.addDetail("Application",&RequestHandler::readRequestApplication,&RequestHandler::registerRequestApplication);
		holder.addDetail("Plugin/module",&RequestHandler::readPlugin,&RequestHandler::registerRequestPlugin);
	}
};

struct PerformanceTune 
{
	std::string	checkpointPeriod;
	std::string	dbRecoverFatal;
	int32		evictorSize;
	std::string savePeriod;
	std::string saveSizeTrigger;
	static void structure( ZQ::common::Config::Holder< PerformanceTune > &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("IceFreezeEnviroment","CheckpointPeriod",&PerformanceTune::checkpointPeriod,"0",optReadOnly);
		holder.addDetail("IceFreezeEnviroment","DbRecoverFatal",&PerformanceTune::dbRecoverFatal,"1",optReadOnly);
		holder.addDetail("Session","CacheSize",&PerformanceTune::evictorSize,"2000",optReadOnly);
		holder.addDetail("Session","SavePeriod",&PerformanceTune::savePeriod,"60000",optReadOnly);
		holder.addDetail("Session","SaveSizeTrigger",&PerformanceTune::saveSizeTrigger,"500",optReadOnly);
	}
};

struct Gateway
{
	Gateway()
	{
		crashdumpenabled	= 0;
		icetraceenabled		= 1;
		icetracelevel		= 7;
		icetracelogsize		= 20 * 1024 * 1024;
		processThreadCount	= 50;
		maxPendingRequest	= 0;
	}

	//crash dump
	std::string crashdumppath;
	int32		crashdumpenabled;

	///ice trace
	int32		icetraceenabled;
	int32		icetracelevel;
	int32		icetracelogsize;
	std::map<std::string, std::string> iceproperties;

	//database
	std::string	dbpath;
	std::string dbruntimepath;

	//request process
	int32		processThreadCount;
	int32		maxPendingRequest;

	std::string binding;

	typedef ZQ::common::Config::Holder<SocketServer> SocketServerHolder;
	SocketServerHolder	sockserver;

	typedef ZQ::common::Config::Holder<RequestHandler> RequestHandlerHolder;
	RequestHandlerHolder requestHandler;

	std::map<std::string,std::string>	iceProps;

	typedef ZQ::common::Config::Holder<PerformanceTune> PerformanceTuneHolder;
	PerformanceTuneHolder	perfTune;
	

	void readPerfTune( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		perfTune.read(node,hPP);
	}
	void registerNone(const std::string&){}
	void readIceProperties( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		using namespace ZQ::common::Config;
		Holder<NVPair> nvHolder;
		nvHolder.read(node, hPP);
		iceProps[nvHolder.name] = nvHolder.value;
	}
	void registerIceProperties( const std::string&  ){}
	void readSocketServer( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		sockserver.read( node , hPP );
	}
	void registerSocketServer( const std::string& path ){}

	void readRequestHandler( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
	{
		requestHandler.read( node , hPP );
	}
	void rgisterRequestHandler( const std::string& path ){ }


	static void structure( ZQ::common::Config::Holder<Gateway>& holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail("default/CrashDump","path",&Gateway::crashdumppath,"",optReadOnly);
		holder.addDetail("default/CrashDump","eanabled",&Gateway::crashdumpenabled,"0",optReadOnly);
		holder.addDetail("default/IceTrace","enabled",&Gateway::icetraceenabled,"0",optReadOnly);
		holder.addDetail("default/IceTrace","level",&Gateway::icetracelevel,"7",optReadOnly);
		holder.addDetail("default/IceTrace","size",&Gateway::icetracelogsize,"10240000",optReadOnly);
		holder.addDetail("default/Database","path",&Gateway::dbpath,"",optReadOnly);
		holder.addDetail("default/Database","runtimePath",&Gateway::dbruntimepath,"",optReadOnly);
		holder.addDetail("default/IceProperties/serviceProperty",&Gateway::readIceProperties,&Gateway::registerIceProperties);

		holder.addDetail("Dsmcc/RequestProcess","threads",&Gateway::processThreadCount,"50",optReadOnly);
		holder.addDetail("Dsmcc/RequestProcess","maxPendingRequest",&Gateway::maxPendingRequest,"0",optReadOnly);
		holder.addDetail("Dsmcc/RequestProcess","bind",&Gateway::binding,"tcp -h 127.0.0.1 -p 10501",optReadOnly);

		holder.addDetail("Dsmcc/SocketServer",&Gateway::readSocketServer,&Gateway::registerSocketServer );
		holder.addDetail("Dsmcc/RequestHandler",&Gateway::readRequestHandler,&Gateway::rgisterRequestHandler);
		holder.addDetail("Dsmcc/PeformanceTune",&Gateway::readPerfTune,&Gateway::registerNone);
	}
};

typedef ZQ::common::Config::Loader<Gateway> GateWayConfig;

}}}//namespace ZQ::DSMCC::Config

#endif//__zq_dsmcc_gateway_configuration_header_file_h__
