

#ifndef _CPH_RNC_CONFIG_
#define _CPH_RNC_CONFIG_

#include <ConfigHelper.h>

struct Method
{
	std::string methodName;
	int32 maxSession;
	int32 maxBandwidth;
	static void structure(ZQ::common::Config::Holder<Method > &holder)
	{
		holder.addDetail("", "name", &Method::methodName, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessions", &Method::maxSession, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxBandwidth", &Method::maxBandwidth, NULL, ZQ::common::Config::optReadOnly);
	}
};

struct NasCopyConfig
{
	NasCopyConfig();

	//int32   maxSessionNum;
	
	int32 enableProgEvent;
	int32 enableStreamEvent;
	//int32 maxBandwidthKBps;
	int32 streamReqSecs;

	// for vstrm bandwidth management
	int32	vstrmBwClientId;
	int32	bDisableBitrateLimit;

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;

	void registerNothing(const std::string&){}

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
	{
		ZQ::common::Config::Holder<Method> methodholder("name");
		methodholder.read(node, hPP);
		methods.push_back(methodholder);
	}
	
	 
    static void structure(ZQ::common::Config::Holder<NasCopyConfig> &holder)
    {
        using namespace ZQ::common::Config;
        typedef ZQ::common::Config::Holder<NasCopyConfig>::PMem_CharArray PMem_CharArray;
		//holder.addDetail("CPH_NasCopy", "maxSessions", &NasCopyConfig::maxSessionNum, NULL, optReadOnly);
		//holder.addDetail("CPH_NasCopy", "maxBandwidth", &NasCopyConfig::maxBandwidthKBps, NULL, optReadOnly);
        holder.addDetail("CPH_NasCopy/Event/Progress", "enable", &NasCopyConfig::enableProgEvent, NULL, optReadOnly);
		holder.addDetail("CPH_NasCopy/Event/Streamable", "enable", &NasCopyConfig::enableStreamEvent, NULL, optReadOnly);
		holder.addDetail("CPH_NasCopy/Event/Streamable", "lagAfterStart", &NasCopyConfig::streamReqSecs, NULL, optReadOnly);
       	holder.addDetail("CPH_NasCopy/Vstream", "BWMgrClientId", &NasCopyConfig::vstrmBwClientId, NULL, optReadOnly); 
		holder.addDetail("CPH_NasCopy/Session","disableBitrateLimit",&NasCopyConfig::bDisableBitrateLimit, "0",optReadOnly);
		holder.addDetail("CPH_NasCopy/ProvisionMethod/Method",&NasCopyConfig::readMethod,&NasCopyConfig::registerNothing);
	}
};


extern ZQ::common::Config::Loader<NasCopyConfig> _gCPHCfg;

#endif