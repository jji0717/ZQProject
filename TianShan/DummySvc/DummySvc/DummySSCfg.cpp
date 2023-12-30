
#include "DummySSCfg.h"

DummySSCfg::DummySSCfg()
{
	pauseMaxCfg=2000;
	pauseMinCfg=500;
}
DummySSCfg::~DummySSCfg()
{

}

void DummySSCfg::structure(Config::Holder<DummySSCfg> &holder)
{
	//using namespace ZQ::common;
	//typedef Config::Holder<DummySSCfg>::PMem_CharArray PMem_CharArray;
	holder.addDetail("default/Database","path",&DummySSCfg::dataPath,NULL,Config::optReadOnly);
	holder.addDetail("default/Database","runtimePath",&DummySSCfg::runTimePath,NULL,Config::optReadOnly);
	holder.addDetail("default/IceLog","path",&DummySSCfg::IceLogPath,NULL,Config::optReadOnly);
	holder.addDetail("default/IceLog","level",&DummySSCfg::logLevel,"6",Config::optReadOnly);

	holder.addDetail("DummySvc/Bind","endpoint",&DummySSCfg::bindEndPoint,NULL,Config::optReadOnly);
	holder.addDetail("DummySvc/RandomTime","pausemax",&DummySSCfg::pauseMaxCfg,"50",Config::optReadOnly);
	holder.addDetail("DummySvc/RandomTime","pausemin",&DummySSCfg::pauseMinCfg,"10",Config::optReadOnly);
	holder.addDetail("DummySvc/Service","replicaSubscriberEndpoint",&DummySSCfg::replicaSubscriberEndpoint,NULL,Config::optReadOnly);
	holder.addDetail("DummySvc/Service","eventChannel",&DummySSCfg::eventChannel,NULL,Config::optReadOnly);

	holder.addDetail("DummySvc/NetId","nodeId",&DummySSCfg::nodeId,NULL,Config::optReadOnly);
	holder.addDetail("DummySvc/NetId/streamer", &DummySSCfg::readNetIdStreamers, &DummySSCfg::registerNothing);
	holder.addDetail("DummySvc/IceProperties/prop", &DummySSCfg::readIceProp, &DummySSCfg::registerNothing);    

	holder.addDetail("DummySvc/TimerWatch","targettime",&DummySSCfg::targetTime,"1000",Config::optReadOnly);
}

void DummySSCfg::readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<NVPair> propHolder;
	propHolder.read(node, hPP);
	iceProperties[propHolder.name] = propHolder.value;
}
void DummySSCfg::readNetIdStreamers(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<Streamer> propHolder;
	propHolder.read(node, hPP);
	spigotIds.push_back(propHolder.streamerName);
}