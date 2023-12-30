

#ifndef _CPH_AquaRec_CONFIG_
#define _CPH_AquaRec_CONFIG_

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

struct AquaRecConfig
{
	AquaRecConfig();

	int32	preloadTime;			//session preload time in milliseconds 

	int32   leadsesslagAfterIdle;
	int32   monitorInterval;
	int32    leadsessReadInterval;
	int32   maxLeadsessionNum;

	int32   progressSendInterval;

	int32	enableProgEvent;
	int32	enableStreamEvent;
	int32	streamReqSecs;

	std::string destName;
	std::string aquaRootUri;
	std::string aquaContainer;
	int32 connectTimeOut;
	int32 requestTimeOut;
	int32 aquaFlag;
	int32 aquaMaxThreadPoolSize;
    static void structure(ZQ::common::Config::Holder<AquaRecConfig> &holder);

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
   
    void registerNothing(const std::string&){}
};

extern ZQ::common::Config::Loader<AquaRecConfig> _gCPHCfg;

#endif
