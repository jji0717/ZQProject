
#include "StreamersConfig.h"

ZQ::common::Config::Loader<GBss::StreamersConfig> _streamersConfig("");

namespace GBss
{

void Streamer::structure(StreamerHolder& holder)
{
	holder.addDetail("", "source", &Streamer::_source, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "netId", &Streamer::_netId, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "serviceEndpoint", &Streamer::_serviceEndpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "volume", &Streamer::_volume, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "importChannel", &Streamer::_importChannel, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "totalBW", &Streamer::_totalBW, "100000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "adminEnabled", &Streamer::_adminEnabled, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxStream", &Streamer::_maxStream, "100", ZQ::common::Config::optReadOnly);
}

void StreamingResource::structure(StreamingResourceHolder& holder)
{
	holder.addDetail("", "enableWarmup", &StreamingResource::_enableWarmup, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "retryCount", &StreamingResource::_retryCount, "3", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxPenaltyValue", &StreamingResource::_maxPenaltyValue, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "replicaUpdateTimeout", &StreamingResource::_replicaUpdateTimeout, "60", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "checkReplicaUpdateInfo", &StreamingResource::_replicaUpdateEnable, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "replicaUpdateInterval", &StreamingResource::_replicaUpdateInterval, "60", ZQ::common::Config::optReadOnly);
	holder.addDetail("Streamer", &StreamingResource::readStreamer, &StreamingResource::registerStreamer);
}

void StreamingResource::readStreamer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	StreamerHolder streamer("netId");
	streamer.read(node, hPP);
	_streamerDatas.push_back(streamer);
}

void StreamingResource::registerStreamer(const std::string &full_path)
{
	for (std::vector<StreamerHolder>::iterator it = _streamerDatas.begin(); it != _streamerDatas.end(); ++it)
	{
		it->snmpRegister(full_path);
	}
}

void StreamersConfig::structure(ZQ::common::Config::Holder<StreamersConfig> &holder)
{
	holder.addDetail("/", &StreamersConfig::readStreamingResource, &StreamersConfig::registerStreamingResource);
}

void StreamersConfig::readStreamingResource(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_streamingResource.read(node, hPP);
}

void StreamersConfig::registerStreamingResource(const std::string &full_path)
{
	_streamingResource.snmpRegister(full_path);
}


} // end GBss