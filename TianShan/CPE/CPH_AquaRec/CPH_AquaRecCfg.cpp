
#include "CPH_AquaRecCfg.h"

ZQ::common::Config::Loader<AquaRecConfig> _gCPHCfg("CPH_AquaRec.xml");


AquaRecConfig::AquaRecConfig()
{
	monitorInterval = 30000; //30 seconds
	leadsesslagAfterIdle = 10000; //10 seconds
	preloadTime = 5000;

	streamReqSecs = 5;	

	streamReqSecs = 5;				//5 seconds
	enableProgEvent = 1;
	enableStreamEvent = 1;
	progressSendInterval = 10000; //10 seonds
	leadsessReadInterval = 180000;
	connectTimeOut = 5000;
	requestTimeOut = 10000;
}

void AquaRecConfig::structure(ZQ::common::Config::Holder<AquaRecConfig> &holder)
{
	using namespace ZQ::common::Config;
	typedef ZQ::common::Config::Holder<AquaRecConfig>::PMem_CharArray PMem_CharArray;

	holder.addDetail("CPH_AquaRec/LeadSession", "maxSessions", &AquaRecConfig::maxLeadsessionNum, NULL, optReadOnly);
	holder.addDetail("CPH_AquaRec/LeadSession", "lagAfterIdle", &AquaRecConfig::leadsesslagAfterIdle, NULL, optReadOnly);
	holder.addDetail("CPH_AquaRec/LeadSession", "monitorInterval", &AquaRecConfig::monitorInterval, NULL, optReadOnly);
	holder.addDetail("CPH_AquaRec/LeadSession", "interval", &AquaRecConfig::leadsessReadInterval, "180000", optReadOnly);

	holder.addDetail("CPH_AquaRec/Event/Progress", "enable", &AquaRecConfig::enableProgEvent, "1", optReadOnly);
	holder.addDetail("CPH_AquaRec/Event/Progress", "interval", &AquaRecConfig::progressSendInterval, "6000", optReadOnly);
	holder.addDetail("CPH_AquaRec/Event/Streamable", "enable", &AquaRecConfig::enableStreamEvent, "1", optReadOnly);
	holder.addDetail("CPH_AquaRec/Event/Streamable", "lagAfterStart", &AquaRecConfig::streamReqSecs, NULL, optReadOnly);

	holder.addDetail("CPH_AquaRec/ProvisionMethod/Method",&AquaRecConfig::readMethod,&AquaRecConfig::registerNothing);

	holder.addDetail("CPH_AquaRec/AquaServer","rootUrl",&AquaRecConfig::aquaRootUri,NULL,optReadOnly);
	holder.addDetail("CPH_AquaRec/AquaServer","container",&AquaRecConfig::aquaContainer,NULL,optReadOnly);
	holder.addDetail("CPH_AquaRec/AquaServer","flags",&AquaRecConfig::aquaFlag,"0",optReadOnly);
	holder.addDetail("CPH_AquaRec/AquaServer","maxThreadPoolSize",&AquaRecConfig::aquaMaxThreadPoolSize,"5",optReadOnly);
	
	holder.addDetail("CPH_AquaRec/AquaServer","connectTimeOut",&AquaRecConfig::connectTimeOut,"5000",optReadOnly);
	holder.addDetail("CPH_AquaRec/AquaServer","requestTimeOut",&AquaRecConfig::requestTimeOut,"10000",optReadOnly);

	holder.addDetail("CPH_AquaRec/Subscribers","destNameExpression",&AquaRecConfig::destName,NULL,optReadOnly);

}

void AquaRecConfig::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}
