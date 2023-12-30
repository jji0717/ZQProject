#include "SOPConfig.h"

ZQ::common::Config::Loader<NGOD::SOPConfig> sopConfig("");

namespace NGOD
{

void Streamer::structure(StreamerHolder& holder)
{
	// ATTENTION: when you change this schema, make sure to update SessionViewImpl::updateSOPXML() accordingly

	holder.addDetail("", "netId", &Streamer::netId, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "serviceEndpoint", &Streamer::serviceEndpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "volume", &Streamer::volume, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "importChannel", &Streamer::importChannel, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "totalBW", &Streamer::totalBW, "100000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxStream", &Streamer::maxStream, "100", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enabled", &Streamer::enabled, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "usedInStreamFailover", &Streamer::usedInStreamFailoverTest,"0",ZQ::common::Config::optReadOnly);
}

void Sop::structure(ZQ::common::Config::Holder<Sop>& holder)
{
	// ATTENTION: when you change this schema, make sure to update SessionViewImpl::updateSOPXML() accordingly

	holder.addDetail("", "name", &Sop::name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "serviceGroup", &Sop::serviceGroup, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("","sopGroup",&Sop::sopGroupName,"",ZQ::common::Config::optReadOnly);
	holder.addDetail("streamer", &Sop::readStreamer, &Sop::registerStreamer);
}

void Sop::readStreamer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	StreamerHolder streamer("netId");
	streamer.read(node, hPP);
	streamerDatas.push_back(streamer);
}

void Sop::registerStreamer(const std::string &full_path)
{
	for (std::vector<StreamerHolder>::iterator it = streamerDatas.begin(); it != streamerDatas.end(); ++it)
	{
		it->snmpRegister(full_path);
	}
}

void SOPRestriction::structure(ZQ::common::Config::Holder<SOPRestriction>& holder)
{
	// ATTENTION: when you change this schema, make sure to update SessionViewImpl::updateSOPXML() accordingly

	holder.addDetail("","enableReportedImportChannel",&SOPRestriction::enableReportedImportChannelBandWidth,"1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enableWarmup", &SOPRestriction::enableWarmup, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "retryCount", &SOPRestriction::retryCount, "3", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxPenaltyValue", &SOPRestriction::maxPenaltyValue, "10", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "penaltyOfTimeout", &SOPRestriction::timeoutPenalty, "2", ZQ::common::Config::optReadOnly);	
	holder.addDetail("", "penaltyMask", &SOPRestriction::penaltyEnableMask, "7", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "execMask", &SOPRestriction::execMask, "7", ZQ::common::Config::optReadOnly);
	holder.addDetail("","streamerQueryInterval",&SOPRestriction::streamerQueryInterval,"10000",ZQ::common::Config::optReadOnly);
	holder.addDetail("","replicaUpdateInterval",&SOPRestriction::replicaUpdateInterval,"60",ZQ::common::Config::optReadOnly);
	holder.addDetail("","checkReplicaUpdateInfo",&SOPRestriction::replicaUpdateEnable,"1",ZQ::common::Config::optReadOnly);
	holder.addDetail("sop", &SOPRestriction::readSop, &SOPRestriction::registerSop);
}

void SOPRestriction::readSop(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	SopHolder sop("name");
	sop.read(node, hPP);
	sopDatas[sop.name] = sop;
}

void SOPRestriction::registerSop(const std::string &full_path)
{
	for (std::map< std::string , SopHolder>::iterator it = sopDatas.begin(); it != sopDatas.end(); ++it)
	{
		it->second.snmpRegister(full_path);
	}
}

void SOPConfig::structure(ZQ::common::Config::Holder<SOPConfig> &holder)
{
	holder.addDetail("/", &SOPConfig::readSOPRestriction, &SOPConfig::registerSOPRestriction);
}

void SOPConfig::readSOPRestriction(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	sopRestrict.read(node, hPP);
}

void SOPConfig::registerSOPRestriction(const std::string &full_path)
{
	sopRestrict.snmpRegister(full_path);
}

}
