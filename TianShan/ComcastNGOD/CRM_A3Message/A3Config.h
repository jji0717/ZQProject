#ifndef __CRG_A3MESSAGE_CONFIG_H__
#define __CRG_A3MESSAGE_CONFIG_H__

#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>
using namespace ZQ::common;
struct ContentStoreMountInfo
{
	std::string exportname;
	std::string netId;
	std::string volumename;
	static void structure(::ZQ::common::Config::Holder<ContentStoreMountInfo> &holder)
	{
		holder.addDetail("", "exportname",&ContentStoreMountInfo::exportname, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "netid",&ContentStoreMountInfo::netId, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "volumename",&ContentStoreMountInfo::volumename, NULL, ZQ::common::Config::optReadOnly);
	}
};
typedef std::map<std::string, ContentStoreMountInfo> ContentStoreMountInfos;

struct ContentLib
{
	std::string endpoint;
	ContentStoreMountInfos csmounts;
	static void structure(::ZQ::common::Config::Holder<ContentLib> &holder)
	{
		holder.addDetail("", "endpoint",&ContentLib::endpoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ContentStoreMount/param",&ContentLib::readContentStores, &ContentLib::registerNothing);
	}
	ContentLib()
	{
		endpoint = "";
	}
	void readContentStores(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<ContentStoreMountInfo> csholder("");
		csholder.read(node, hPP);
		ContentStoreMountInfos::iterator itor = csmounts.find(csholder.exportname);
		if(itor != csmounts.end())
		{
			throwf<CfgException>(EXFMT(CfgException, "duplicate exportname [%s]"), csholder.exportname.c_str());
		}
		else
		{
			for(itor = csmounts.begin(); itor != csmounts.end(); itor++)
			{
				if(itor->second.netId == csholder.netId && itor->second.volumename == csholder.volumename)
				{
					throwf<CfgException>(EXFMT(CfgException, "exportname [%s] duplicate netid and volume with exportname [%s]"), 
						csholder.exportname.c_str(),  itor->second.exportname.c_str());
				}
			}
			csmounts.insert(ContentStoreMountInfos::value_type(csholder.exportname, csholder));
		}
	}
	void registerNothing(const std::string&){}
};
struct CacheStoreNode
{
	std::string endpoint;
	int32 optimize;
	static void structure(::ZQ::common::Config::Holder<CacheStoreNode> &holder)
	{
		holder.addDetail("", "endpoint",&CacheStoreNode::endpoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "optimize",&CacheStoreNode::optimize, "0", ZQ::common::Config::optReadOnly);
	}
	CacheStoreNode()
	{
		endpoint = "";
		optimize = 0;
	}
};
// config loader structure
struct A3MessageCfg
{
/*	std::string databasePath;*/

	// stores the ice properties
	std::map<std::string, std::string> icePropMap;

	int32 backStoreType;
	std::string clibEndpoint;
	ContentStoreMountInfos csmounts;

	std::string csEndpoint; //cachestore endpoint
	int32       csOptimize;
	::std::string logfilename;
	int32 lLogLevel;
	int32 lLogFileSize;
	int32 llogFileCount;
	int32 lLogBufferSize;
	int32 lLogWriteTimteout;

	::std::string Icelogfilename;
	int32 lIceLogLevel;
	int32 lIceLogFileSize;
	int32 lIcelogFileCount;
	int32 lIceLogBufferSize;
	int32 lIceLogWriteTimteout;

	std::string EventEndpoint;
	std::string ListenEventEndPoint;

	std::string cpcEndPoint;
	int registerInterval; // milli seconds
	std::string strDefaultIndexType;

	int32 defaultProvisionBW;
	std::string			strTrickSpeeds;
	std::vector<float>  trickSpeedCollection;


	std::string rootUrl;
	std::string homeContainer;
	std::string userDomain;
	int flags;
	int maxThreadPoolSize;
	int connectTimeout;
	int timeout;
	std::string mainFileExtension;
	std::string mainFilePath;
	std::string defaultIndexType;
	std::string volumeName;
	std::string contentNameFormat;
	std::string bindIp;

	int32 deleteMainFile;

//	int32 enableRaw;
	std::string exposeURL;

	//Default config
	static void structure(::ZQ::common::Config::Holder<A3MessageCfg> &holder)
	{
		//database folder
// 		holder.addDetail("default/Database", "path", &A3MessageCfg::databasePath, NULL, ZQ::common::Config::optReadOnly);

		//ice property
		holder.addDetail("default/IceProperties/prop",&A3MessageCfg::readIceProperties, &A3MessageCfg::registerNothing);
		holder.addDetail("A3Message", "backStoreType", &A3MessageCfg::backStoreType, "0", ZQ::common::Config::optReadOnly);

		holder.addDetail("A3Message/Log", "logfilename", &A3MessageCfg::logfilename, "RtspEngine.log", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/Log", "level", &A3MessageCfg::lLogLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/Log", "size", &A3MessageCfg::lLogFileSize, "40960000", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/Log", "count", &A3MessageCfg::llogFileCount, "5", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/Log", "buffer", &A3MessageCfg::lLogBufferSize, "10240", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/Log", "flushtimeout", &A3MessageCfg::lLogWriteTimteout, "2", ZQ::common::Config::optReadOnly);


		holder.addDetail("A3Message/IceTrace", "logfilename", &A3MessageCfg::Icelogfilename, "RtspEngine.log", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/IceTrace", "level", &A3MessageCfg::lIceLogLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/IceTrace", "size", &A3MessageCfg::lIceLogFileSize, "40960000", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/IceTrace", "count", &A3MessageCfg::lIcelogFileCount, "5", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/IceTrace", "buffer", &A3MessageCfg::lIceLogBufferSize, "10240", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/IceTrace", "flushtimeout", &A3MessageCfg::lIceLogWriteTimteout, "2", ZQ::common::Config::optReadOnly);

		holder.addDetail("A3Message/TianShanEvents", "EventChannelEndPoint", &A3MessageCfg::EventEndpoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/TianShanEvents", "listenEndpoint", &A3MessageCfg::ListenEventEndPoint, NULL, ZQ::common::Config::optReadOnly);
	
		holder.addDetail("A3Message/ContentLib", &A3MessageCfg::readContentLib, &A3MessageCfg::registerNothing, ZQ::common::Config::Range(0, 1));
		holder.addDetail("A3Message/CacheStore", &A3MessageCfg::readCacheStore, &A3MessageCfg::registerNothing, ZQ::common::Config::Range(0, 1));

		holder.addDetail("A3Message/AquaServer","rootUrl",&A3MessageCfg::rootUrl,NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","homeContainer",&A3MessageCfg::homeContainer,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","userDomain",&A3MessageCfg::userDomain,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","flags",&A3MessageCfg::flags,"0",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","maxThreadPoolSize",&A3MessageCfg::maxThreadPoolSize,"5",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","connectTimeout",&A3MessageCfg::connectTimeout,"5000",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","timeout",&A3MessageCfg::timeout,"10000",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","mainFileExtension",&A3MessageCfg::mainFileExtension,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","mainFilePath",&A3MessageCfg::mainFilePath,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","defaultIndexType",&A3MessageCfg::defaultIndexType,"VVC",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","volumeName",&A3MessageCfg::volumeName,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","contentNameFormat",&A3MessageCfg::contentNameFormat,"${PAID}_${PID}", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","bindIp",&A3MessageCfg::bindIp,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","deleteMainFile",&A3MessageCfg::deleteMainFile,"1", ZQ::common::Config::optReadOnly);
//		holder.addDetail("A3Message/AquaServer","enableRaw",&A3MessageCfg::enableRaw,"0", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer","exposeURL",&A3MessageCfg::exposeURL,"", ZQ::common::Config::optReadOnly);


		/* CPC items */
		holder.addDetail("A3Message/AquaServer/CPC/Bind", "endpoint", &A3MessageCfg::cpcEndPoint, 0, ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer/CPC/Sessions", "registerInterval", &A3MessageCfg::registerInterval, "15000", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer/CPC/Sessions", "DefaultIndexType", &A3MessageCfg::strDefaultIndexType,"");

		holder.addDetail("A3Message/AquaServer/Provision", "trickSpeeds", &A3MessageCfg::strTrickSpeeds, "7.5", ZQ::common::Config::optReadOnly);
		holder.addDetail("A3Message/AquaServer/Provision", "defaultBandwidth", &A3MessageCfg::defaultProvisionBW, "3750000", ZQ::common::Config::optReadOnly);
	}
	void readIceProperties(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		icePropMap[nvholder.name] = nvholder.value;
	}
	void readContentLib(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<ContentLib> clbholder("");
		clbholder.read(node, hPP);
		clibEndpoint = clbholder.endpoint;
		csmounts = clbholder.csmounts;
	}
	void readCacheStore(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<CacheStoreNode> csholder("");
		csholder.read(node, hPP);
		csEndpoint = csholder.endpoint;
		csOptimize = csholder.optimize;
	}

	void registerNothing(const std::string&){}
};

#endif
