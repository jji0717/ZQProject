#ifndef _TIANSHAN_NGOD_PHO_CONFIGURATION_HEADER_FILE_H__
#define _TIANSHAN_NGOD_PHO_CONFIGURATION_HEADER_FILE_H__

#include <ConfigHelper.h>

struct phoVssConfForWeiwoo
{
	std::string			phoVssConfPath;
	
	static void structure(ZQ::common::Config::Holder<phoVssConfForWeiwoo> &holder)
	{
		using namespace ZQ::common::Config;
		holder.addDetail("","phoConfigurationPath",&phoVssConfForWeiwoo::phoVssConfPath,"",optReadOnly);
	}
};

struct PHOConfig
{
	char		szPHOLogFileName[512];
	int32		lPHOLogLevel;
	int32		lPHOLogFileSize;
	int32		lPHOLogBufferSize;
	int32		lPHOLogWriteTimteout;

	ZQ::common::Config::Holder<phoVssConfForWeiwoo> phoConf;	

	PHOConfig()
	{
		strcpy(szPHOLogFileName,"pho_VSS.log");
		lPHOLogLevel			= 7;
		lPHOLogFileSize			= 1024 * 1000 * 10;
		lPHOLogBufferSize		= 10240;
		lPHOLogWriteTimteout	= 2;
	}

	void registerNothing(const std::string&){}
	void readPhoConf(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		phoConf.read( node , hPP );
	}
	static void structure(ZQ::common::Config::Holder<PHOConfig> &holder)
	{
		using namespace ZQ::common::Config;
		holder.addDetail("PHO/log", "level", &PHOConfig::lPHOLogLevel, "7", optReadOnly);
		holder.addDetail("PHO/log", "size", &PHOConfig::lPHOLogFileSize, "10240000", optReadOnly);
		holder.addDetail("PHO/log", "buffer", &PHOConfig::lPHOLogBufferSize, "10240", optReadOnly);
		holder.addDetail("PHO/log", "flushtimeout", &PHOConfig::lPHOLogWriteTimteout, "2", optReadOnly);
		holder.addDetail( "PHO/confPath", &PHOConfig::readPhoConf, &PHOConfig::registerNothing);
	}
};

struct IceProperty
{
	std::string		_name;
	std::string		_value;

	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;

	static void structure(IcePropertyHolder& holder)
	{
		holder.addDetail("", "name", &IceProperty::_name, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("", "value", &IceProperty::_value, "", ZQ::common::Config::optReadOnly);
	}
};

struct IceProperties
{
	typedef ZQ::common::Config::Holder<IceProperty> IcePropertyHolder;
	std::vector<IcePropertyHolder> _propDatas;

	static void structure(ZQ::common::Config::Holder<IceProperties>& holder)
	{

	}

	void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerIceProperty(const std::string &full_path);
};

struct PhoVssConf 
{

	ZQ::common::Config::Holder<IceProperties>	iceProps;
	std::string									phoReplicaSubscriberEndpoint;
	int32										replicaUpdaterInterval;
	PhoVssConf()
	{
		replicaUpdaterInterval	= 60 * 1000;
	}

	void readIceProperties(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		iceProps.read(node, hPP);
	}

	void registerIceProperties(const std::string &full_path){}

	static void structure( ZQ::common::Config::Holder<PhoVssConf> &holder )
	{
		using namespace ZQ::common::Config;
		holder.addDetail( "PHO/HsnTree","subscriberBind",&PhoVssConf::phoReplicaSubscriberEndpoint,"",optReadOnly);
		holder.addDetail( "PHO/HsnTree","updateInterval",&PhoVssConf::replicaUpdaterInterval,"60000",optReadOnly);
		holder.addDetail( "default/IceProperties", &PhoVssConf::readIceProperties, &PhoVssConf::registerIceProperties);
	}
};

#endif //_TIANSHAN_NGOD_PHO_CONFIGURATION_HEADER_FILE_H__
