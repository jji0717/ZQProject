#ifndef __CRG_C3DSERVER_CONFIG_H__
#define __CRG_C3DSERVER_CONFIG_H__

#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>
using namespace ZQ::common;

struct C3dServerCfg
{
	int32 contentNameWithProviderId;
	std::string providerId;
	// stores the ice properties
	std::map<std::string, std::string> icePropMap;

	std::string logfilename;
	int32 lLogLevel;
	int32 lLogFileSize;
	int32 llogFileCount;
	int32 lLogBufferSize;
	int32 lLogWriteTimteout;

	std::string Icelogfilename;
	int32 lIceLogLevel;
	int32 lIceLogFileSize;
	int32 lIcelogFileCount;
	int32 lIceLogBufferSize;
	int32 lIceLogWriteTimteout;

	//CPE
	std::string EndpointOfCPE;
	//ContentStore
	std::string CSId;
	std::string CSVolume;
	//AquaServer
	std::string rootUrl;
	std::string homeContainer;
	std::string sourceFolder;
	std::string contentFolder;
	int32	flags;
	int32	maxThreadPoolSize;
	int32	timeZoneOfFileName;
	int32	connectTimeout;
	int32   timeout;


	//TestMode
	struct TestMode 
	{
		int32 enable;
		int32 duration;
		int32 delay;
		static void structure(ZQ::common::Config::Holder<TestMode > &holder)
		{
			holder.addDetail("", "enable", &TestMode::enable, "0",ZQ::common::Config::optReadOnly);
			holder.addDetail("", "duration", &TestMode::duration, "300",ZQ::common::Config::optReadOnly);
			holder.addDetail("", "delay", &TestMode::delay, "30",ZQ::common::Config::optReadOnly);
		}
		TestMode()
		{
			enable = 0;
			duration = 300;
			delay = 30;
		};
	};

	TestMode testMode;


	C3dServerCfg()
	{

	};
	static void structure(ZQ::common::Config::Holder<C3dServerCfg> &holder)
	{
		holder.addDetail("default/IceProperties/prop",&C3dServerCfg::readIceProperties, &C3dServerCfg::registerNothing);

		holder.addDetail("OpenVBO3dServer/Log", "logfilename", &C3dServerCfg::logfilename, "CRM_3dServer.log", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/Log", "level", &C3dServerCfg::lLogLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/Log", "size", &C3dServerCfg::lLogFileSize, "40960000", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/Log", "count", &C3dServerCfg::llogFileCount, "5", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/Log", "buffer", &C3dServerCfg::lLogBufferSize, "10240", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/Log", "flushtimeout", &C3dServerCfg::lLogWriteTimteout, "2", ZQ::common::Config::optReadOnly);


		holder.addDetail("OpenVBO3dServer/IceTrace", "logfilename", &C3dServerCfg::Icelogfilename, "CRM_3dServerIce.log", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/IceTrace", "level", &C3dServerCfg::lIceLogLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/IceTrace", "size", &C3dServerCfg::lIceLogFileSize, "40960000", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/IceTrace", "count", &C3dServerCfg::lIcelogFileCount, "5", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/IceTrace", "buffer", &C3dServerCfg::lIceLogBufferSize, "10240", ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/IceTrace", "flushtimeout", &C3dServerCfg::lIceLogWriteTimteout, "2", ZQ::common::Config::optReadOnly);

		holder.addDetail("OpenVBO3dServer/CPE","endpoint",&C3dServerCfg::EndpointOfCPE,"",ZQ::common::Config::optReadOnly);

		holder.addDetail("OpenVBO3dServer","netId",&C3dServerCfg::CSId,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer","volume",&C3dServerCfg::CSVolume,"$",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer","contentNameWithProviderId",&C3dServerCfg::contentNameWithProviderId,"0",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer","defaultProviderId",&C3dServerCfg::providerId,"",ZQ::common::Config::optReadOnly);

		holder.addDetail("OpenVBO3dServer/AquaServer","rootUrl",&C3dServerCfg::rootUrl,NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","homeContainer",&C3dServerCfg::homeContainer,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","sourceFolder",&C3dServerCfg::sourceFolder,"/npvr/sources/",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","contentFolder",&C3dServerCfg::contentFolder,"/npvr/contents/",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","flags",&C3dServerCfg::flags,"0",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","maxThreadPoolSize",&C3dServerCfg::maxThreadPoolSize,"5",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","timeZoneOfFileName",&C3dServerCfg::timeZoneOfFileName,"1",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","connectTimeout",&C3dServerCfg::connectTimeout,"5000",ZQ::common::Config::optReadOnly);
		holder.addDetail("OpenVBO3dServer/AquaServer","timeout",&C3dServerCfg::timeout,"10000",ZQ::common::Config::optReadOnly);

		holder.addDetail("OpenVBO3dServer/TestMode", &C3dServerCfg::readTestMode, &C3dServerCfg::registerNothing, ZQ::common::Config::Range(0,1));
	}

	void readTestMode(XMLUtil::XmlNode node, const Preprocessor* hpp)
	{
		ZQ::common::Config::Holder<TestMode> testModeHolder("");
		testModeHolder.read(node, hpp);
		testMode = testModeHolder;
	}

	void readIceProperties(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		icePropMap[nvholder.name] = nvholder.value;
	}

	void registerNothing(const std::string&){}
};

#endif
