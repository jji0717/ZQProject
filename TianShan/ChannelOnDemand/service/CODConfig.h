// CODConfig.h: interface for the CODConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CODCONFIG_H__5EA57565_4B0D_4D35_B8BA_E98DD060A21C__INCLUDED_)
#define AFX_CODCONFIG_H__5EA57565_4B0D_4D35_B8BA_E98DD060A21C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ConfigHelper.h>
#include <list>
#include <map>
using namespace ZQ::common;

typedef std::map<std::string,  Config::Holder<Config::NVPair> > PARAMMAP;

struct IceProperty{
	std::string name;
	std::string value;
	static void structure(ZQ::common::Config::Holder< IceProperty > &holder)
    {
        holder.addDetail("", "name", &IceProperty::name, NULL);
        holder.addDetail("", "value", &IceProperty::value, "", ZQ::common::Config::optReadOnly);		
    }
};

struct IceProperties{
	std::string name;
	typedef std::vector< ZQ::common::Config::Holder< IceProperty > > IceProps;
	IceProps iceProps;
	static void structure(ZQ::common::Config::Holder< IceProperties > &holder)
	{
		holder.addDetail("", "name", &IceProperties::name, NULL);
		holder.addDetail("prop", &IceProperties::readIceProperty, &IceProperties::registerIceProperty);
	}
    void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<IceProperty> IcePropHolder("name");
        IcePropHolder.read(node, hPP);
        iceProps.push_back(IcePropHolder);
    }
    void registerIceProperty(const std::string &full_path)
    {
        for (IceProps::iterator it = iceProps.begin(); it != iceProps.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
};

struct JMSParam{
	std::string name;
	std::string value;
	static void structure(ZQ::common::Config::Holder< JMSParam > &holder)
	{
		holder.addDetail("", "name", &JMSParam::name, NULL);
		holder.addDetail("", "value", &JMSParam::value, "", ZQ::common::Config::optReadOnly);
	}
};

struct EventPublisher{
	std::string EPtype;
	typedef std::list< ZQ::common::Config::Holder< JMSParam > > JMSParams;
	JMSParams jmsParams;
	static void structure(ZQ::common::Config::Holder< EventPublisher > &holder)
	{
		holder.addDetail("", "type", &EventPublisher::EPtype, NULL);
		holder.addDetail("param", &EventPublisher::readJMSParam, &EventPublisher::registerJMSParam);

	}
	void readJMSParam(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder< JMSParam > JMSParamHolder("name");
		JMSParamHolder.read(node, hPP);
		jmsParams.push_back(JMSParamHolder);
	}
	void registerJMSParam(const std::string &full_path)
	{
		for(JMSParams::iterator it = jmsParams.begin(); it != jmsParams.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}
};

struct TianShanEvents{
	std::string iceStormEndPoint;
	std::string listenEndPoit;
	std::string runtimeData;
	static void structure(ZQ::common::Config::Holder< TianShanEvents > &holder)
	{
		holder.addDetail("", "EventChannelEndPoint", &TianShanEvents::iceStormEndPoint, NULL);
		holder.addDetail("", "listenEndpoint", &TianShanEvents::listenEndPoit, NULL);
		holder.addDetail("", "runtimeData", &TianShanEvents::runtimeData, NULL);
	}
};

struct AuthorizationParam{
	std::string name;
	std::string value;
	static void structure(ZQ::common::Config::Holder< AuthorizationParam > &holder)
	{
		holder.addDetail("", "name", &AuthorizationParam::name, NULL);
		holder.addDetail("", "value", &AuthorizationParam::value, NULL);
	}
};

struct Authorization{
	std::string module;
	std::string entry;
	int32 enable;
	typedef std::map<std::string, ZQ::common::Config::Holder< AuthorizationParam > > AuthorizationParams;
	AuthorizationParams authorizationParams;	
	static void structure(ZQ::common::Config::Holder< Authorization > &holder)
	{
		holder.addDetail("", "module", &Authorization::module, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "entry", &Authorization::entry, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "enable", &Authorization::enable, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("param", &Authorization::readAuthorizationParam, &Authorization::registerAuthorizationParam);
	}
	void readAuthorizationParam(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder< AuthorizationParam > AuthorizationParamHolder("name");
		AuthorizationParamHolder.read(node, hPP);
		authorizationParams[AuthorizationParamHolder.name] = AuthorizationParamHolder;
	}
	void registerAuthorizationParam(const std::string &full_path)
	{
		for(AuthorizationParams::iterator it = authorizationParams.begin(); it != authorizationParams.end(); ++it)
		{
			(it->second).snmpRegister(full_path);
		}
	}
};

struct AppDataPattern
{
	std::string param;
	std::string pattern;
	PARAMMAP appDataParammap;
	static void structure(Config::Holder<AppDataPattern>& holder)
	{
		holder.addDetail("", "param", &AppDataPattern::param, "0",Config::optReadOnly);
		holder.addDetail("", "pattern", &AppDataPattern::pattern, "0",Config::optReadOnly);
		holder.addDetail("param", &AppDataPattern::readAppDataParam, &AppDataPattern::registerAppData);
	}
	void readAppDataParam(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		appDataParammap[nvholder.name] = nvholder;
	}
	void registerAppData(const std::string &full_path)
	{
		for(PARAMMAP::iterator it = appDataParammap.begin(); it != appDataParammap.end(); ++it)
			(it->second).snmpRegister(full_path);
	}
	void registerNothing(const std::string&){}

};

typedef std::map< std::string, Config::Holder<AppDataPattern> > AppDataPatternMAP;

struct ChodConfigII{
	int32 dumpFullMemory;
	std::string crushDumpPath;

	// the directory of database
	std::string safeStorePath;
	std::string dbRuntimeDataPath;

	// defines the ChannelPublisher properties 
	std::string ChannelPubEndPoint;
	int32 ChannelPublishPointEvitSize;
	int32 InputLocalTime;
	int32 DefaultChannelMaxBitrate;
	int32 ProtectTimeInMs;
	int32 MaxItemNumber;

	//ice trace
	int32 iceTraceEnable;
	int32 iceLogLevel;
	int32 iceLogSize;
	int32 iceLogCount;

	// defines the PurchaseManagement properties 
	int32 PurchaseEvitSize;
	int32 PurchaseItemEvitSize;
	int32 purchaseTimeout;
	
	// size of ThreadPool
	int32 ThreadPoolSize;

	// size of Time Ahead
	int32 ItemStartTimeAhead;

	// defines the TianshanEvent properties
	std::string TopicMgrEndPoint;
	std::string ListenEventEndPoint;
	std::string TSEventRuntimeDataPath;

	std::string    _szEventPubType;

	Authorization authInfo;
	AppDataPatternMAP authAppDataMap;

	typedef std::vector< ZQ::common::Config::Holder< IceProperty > > IceProps;
	IceProps iceProps;

	typedef std::list< ZQ::common::Config::Holder< JMSParam > > JMSParams;
	JMSParams jmsParams;

	static void structure(ZQ::common::Config::Holder< ChodConfigII > &holder)
	{
		holder.addDetail("ChannelOnDemand/CrashDump", "fullDump", &ChodConfigII::dumpFullMemory, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/CrashDump", "path", &ChodConfigII::crushDumpPath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/Database", "path", &ChodConfigII::safeStorePath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/Database", "runtimeData", &ChodConfigII::dbRuntimeDataPath, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ChannelPublisher", "endpoint", &ChodConfigII::ChannelPubEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ChannelPublisher", "cacheSize", &ChodConfigII::ChannelPublishPointEvitSize, "60", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ChannelPublisher", "inputAsLocalTime", &ChodConfigII::InputLocalTime, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ChannelPublisher", "defaultChannelBitrate", &ChodConfigII::DefaultChannelMaxBitrate, "4000000", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ChannelPublisher", "protectionWindow", &ChodConfigII::ProtectTimeInMs, "20000", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ChannelPublisher", "maxItemNumber", &ChodConfigII::MaxItemNumber, "20", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/PurchaseManagement", "cacheSize", &ChodConfigII::PurchaseEvitSize, "200", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/PurchaseManagement", "itemCacheSize", &ChodConfigII::PurchaseItemEvitSize, "1000", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/PurchaseManagement", "timeout", &ChodConfigII::purchaseTimeout, "3600", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ThreadPool", "size", &ChodConfigII::ThreadPoolSize, "20", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/ItemStartTimeAhead", "size", &ChodConfigII::ItemStartTimeAhead, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/TianShanEvents", "EventChannelEndPoint", &ChodConfigII::TopicMgrEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/TianShanEvents", "listenEndpoint", &ChodConfigII::ListenEventEndPoint, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/TianShanEvents", "runtimeData", &ChodConfigII::TSEventRuntimeDataPath, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("ChannelOnDemand/IceTrace", "enable", &ChodConfigII::iceTraceEnable, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/IceTrace", "level", &ChodConfigII::iceLogLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/IceTrace", "size", &ChodConfigII::iceLogSize, "10240000", ZQ::common::Config::optReadOnly);
		holder.addDetail("ChannelOnDemand/IceTrace", "count", &ChodConfigII::iceLogCount, "5", ZQ::common::Config::optReadOnly);

		holder.addDetail("ChannelOnDemand/EventPublisher", "type", &ChodConfigII::_szEventPubType, NULL, ZQ::common::Config::optReadOnly);

		holder.addDetail("ChannelOnDemand/IceProperties/prop", &ChodConfigII::readIceProperty, &ChodConfigII::registerIceProperty);

		holder.addDetail("ChannelOnDemand/EventPublisher/param", &ChodConfigII::readJMSParam, &ChodConfigII::registerJMSParam);
		
		holder.addDetail("ChannelOnDemand/Authorization", &ChodConfigII::readAuthorization, &ChodConfigII::registerAuthorization);

		holder.addDetail("ChannelOnDemand/Authorization/AppDataPattern", &ChodConfigII::readauthAppData, &ChodConfigII::registerauthAppData);

	}

	void readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        ZQ::common::Config::Holder<IceProperty> IcePropHolder("name");
        IcePropHolder.read(node, hPP);
        iceProps.push_back(IcePropHolder);
    }
    void registerIceProperty(const std::string &full_path)
    {
        for (IceProps::iterator it = iceProps.begin(); it != iceProps.end(); ++it)
        {
            it->snmpRegister(full_path);
        }
    }
	void readJMSParam(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder< JMSParam > JMSParamHolder("name");
		JMSParamHolder.read(node, hPP);
		jmsParams.push_back(JMSParamHolder);
	}
	void registerJMSParam(const std::string &full_path)
	{
		for(JMSParams::iterator it = jmsParams.begin(); it != jmsParams.end(); ++it)
		{
			it->snmpRegister(full_path);
		}
	}
	void readAuthorization(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder< Authorization > AuthorizationHolder("Authorization");
		AuthorizationHolder.read(node, hPP);
		authInfo = AuthorizationHolder;
	}
	void registerAuthorization(const std::string &full_path)
	{
	}
	void readauthAppData(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<AppDataPattern> authAppholder("param");
		authAppholder.read(node, hPP);
		authAppDataMap[authAppholder.pattern]  = authAppholder;
	}
	void registerauthAppData(const std::string &full_path)
	{
		for(AppDataPatternMAP::iterator it = authAppDataMap.begin(); it != authAppDataMap.end(); ++it)
			(it->second).snmpRegister(full_path);
	}
};

static void showAppDataPattern(const AppDataPatternMAP& appdatapatmap)
{
	using namespace std;
	for(AppDataPatternMAP::const_iterator it_authappdata = appdatapatmap.begin(); it_authappdata != appdatapatmap.end(); ++it_authappdata)
	{
		AppDataPattern appdatapat = it_authappdata->second;
		cout << "\t param: " << appdatapat.param << "\n";
		cout << "\t pattern: " << appdatapat.pattern << "\n";
		cout << "\t\t params: ";
		for(PARAMMAP::const_iterator it_appdatapat= appdatapat.appDataParammap.begin(); it_appdatapat != appdatapat.appDataParammap.end(); ++it_appdatapat)
		{
			cout << "(" << it_appdatapat->first << " , " << it_appdatapat->second.value << "), ";
		}
		cout<< "\n";
	}
}
//extern ChodConfig					_config;
extern ZQ::common::Config::Loader< ChodConfigII > _config;

#endif // !defined(AFX_CODCONFIG_H__5EA57565_4B0D_4D35_B8BA_E98DD060A21C__INCLUDED_)
