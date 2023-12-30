#include "SOPConfig.h"

ZQ::common::Config::Loader<NGOD2::SOPConfig> _sopConfig("");

namespace NGOD2
{

void Streamer::structure(StreamerHolder& holder)
{
	holder.addDetail("", "netId", &Streamer::_netId, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "serviceEndpoint", &Streamer::_serviceEndpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "volume", &Streamer::_volume, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "importChannel", &Streamer::_importChannel, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "totalBW", &Streamer::_totalBW, "100000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxStream", &Streamer::_maxStream, "100", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enabled", &Streamer::_enabled, "1", ZQ::common::Config::optReadOnly);
}

void Sop::structure(ZQ::common::Config::Holder<Sop>& holder)
{
	holder.addDetail("", "name", &Sop::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "serviceGroup", &Sop::_serviceGroup, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("streamer", &Sop::readStreamer, &Sop::registerStreamer);
}

void Sop::readStreamer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	StreamerHolder streamer("netId");
	streamer.read(node, hPP);
	_streamerDatas.push_back(streamer);
}

void Sop::registerStreamer(const std::string &full_path)
{
	for (std::vector<StreamerHolder>::iterator it = _streamerDatas.begin(); it != _streamerDatas.end(); ++it)
	{
		it->snmpRegister(full_path);
	}
}

void SOPRestriction::structure(ZQ::common::Config::Holder<SOPRestriction>& holder)
{
	holder.addDetail("","enableReportedImportChannel",&SOPRestriction::_enableReportedImportChannelBandWidth,"1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enableWarmup", &SOPRestriction::_enableWarmup, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "retryCount", &SOPRestriction::_retryCount, "3", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxPenaltyValue", &SOPRestriction::_maxPenaltyValue, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "penaltyMask", &SOPRestriction::_penaltyEnableMask, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("","streamerQueryInterval",&SOPRestriction::_streamerQueryInterval,"10000",ZQ::common::Config::optReadOnly);
	holder.addDetail("","replicaUpdateInterval",&SOPRestriction::_replicaUpdateInterval,"60",ZQ::common::Config::optReadOnly);
	holder.addDetail("","checkReplicaUpdateInfo",&SOPRestriction::_replicaUpdateEnable,"1",ZQ::common::Config::optReadOnly);
	holder.addDetail("sop", &SOPRestriction::readSop, &SOPRestriction::registerSop);
}

void SOPRestriction::readSop(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	SopHolder sop("name");
	sop.read(node, hPP);
	_sopDatas[sop._name] = sop;
}

void SOPRestriction::registerSop(const std::string &full_path)
{
	for (std::map< std::string , SopHolder>::iterator it = _sopDatas.begin(); it != _sopDatas.end(); ++it)
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
	_sopRestrict.read(node, hPP);
}

void SOPConfig::registerSOPRestriction(const std::string &full_path)
{
	_sopRestrict.snmpRegister(full_path);
}

}