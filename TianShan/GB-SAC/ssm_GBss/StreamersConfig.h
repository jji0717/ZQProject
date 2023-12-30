#ifndef __EVENT_IS_VODI5_STREAMERS_CONFIGURATION_H__
#define __EVENT_IS_VODI5_STREAMERS_CONFIGURATION_H__

#include "ConfigHelper.h"

namespace GBss
{

struct Streamer
{
	// configuration
	std::string _source;
	std::string _netId;
	std::string _serviceEndpoint;
	std::string	_volume; // wild card is acceptable
	int32 _totalBW; //this can be only used when read from configuration file
	int32 _maxStream; //max stream supported in this streamer
	int32 _adminEnabled;
	std::string _importChannel; //associated import channel name

	typedef ZQ::common::Config::Holder<Streamer> StreamerHolder;

	static void structure(StreamerHolder& holder);
};
typedef Streamer::StreamerHolder StreamerHolder;

struct StreamingResource
{
	int32 _enableWarmup;
	int32 _retryCount;
	int32 _maxPenaltyValue;
	int32 _replicaUpdateTimeout;
	int32 _replicaUpdateEnable;
	int32 _replicaUpdateInterval;
	std::vector<StreamerHolder> _streamerDatas;

	typedef ZQ::common::Config::Holder<StreamingResource> StreamingResourceHolder;
	static void structure(StreamingResourceHolder& holder);

	void readStreamer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerStreamer(const std::string &full_path);
};
typedef StreamingResource::StreamingResourceHolder StreamingResourceHolder;

struct StreamersConfig
{
	StreamingResourceHolder _streamingResource;
	static void structure(ZQ::common::Config::Holder<StreamersConfig> &holder);

	void readStreamingResource(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerStreamingResource(const std::string &full_path);

};

}// end GBss

extern ZQ::common::Config::Loader<GBss::StreamersConfig> _streamersConfig;

#endif