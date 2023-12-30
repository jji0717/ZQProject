#ifndef __NGOD2_SOP_CONFIG_H__
#define __NGOD2_SOP_CONFIG_H__

#include "ConfigHelper.h"
#include "StreamSmithAdmin.h"

namespace NGOD2
{
struct Streamer
{
	Streamer()
	{
		_bReplicaStatus			= false;
		_bAllVolumeAvailble		= false;
		_totalBW				= 0;
		_totalBandwidth			= 0;
		_usedBandwidth			= 0;
		_maxStream				= 0;
		_usedStream				= 0;
		_streamServicePrx		= NULL;
		_lastReplicaUpdate		= 0;
		_setupCountersForRemote = 0;
		_setupCountersTotal		= 0;

		_usedSession            = 0;
		_failedSession          = 0;
	}
	//this is not a configuration
	//if _bEnabled == true , the streamer is available
	//if _bEnabled == false, the streamer is unavailable
	bool						_bReplicaStatus;

	int64						_lastReplicaUpdate;

	int32						_penalty;	


	std::vector<std::string>	volumes;
	bool						_bAllVolumeAvailble;
	std::string					_storageNetId;

	//streamer net id
	std::string					_netId;

	//streamer endpoint
	std::string					_serviceEndpoint;

	//associated volume name , and wild card is acceptable
	std::string					_volume;


	//this can be only used when read from configuration file
	int32						_totalBW;

	//this is the real bandwidth used at runtime
	int64						_totalBandwidth;
	int64						_usedBandwidth;

	///max stream supported in this streamer
	int32						_maxStream;
	int32						_usedStream;


	int32						_setupCountersForRemote;//
	int32						_setupCountersTotal;

	///associated import channel name
	std::string					_importChannel;

	int32                       _enabled;

	
	// add by zjm 
	int64                       _usedSession;
	int64                       _failedSession;

	TianShanIce::Streamer::StreamSmithAdminPrx	_streamServicePrx;
	typedef ZQ::common::Config::Holder<Streamer> StreamerHolder;

	static void structure(StreamerHolder& holder);
};

struct Sop
{
	std::string _name;
	int32 _serviceGroup;
	typedef ZQ::common::Config::Holder<Streamer> StreamerHolder;
	std::vector<StreamerHolder> _streamerDatas;

	static void structure(ZQ::common::Config::Holder<Sop>& holder);

	void readStreamer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerStreamer(const std::string &full_path);
};

struct SOPRestriction
{
	SOPRestriction()
	{
		_enableReportedImportChannelBandWidth	= 0;
		_penaltyEnableMask						= 1;
	}
	int32 _enableReportedImportChannelBandWidth;
	int32 _enableWarmup;
	int32 _retryCount;
	int32 _maxPenaltyValue;
	int32 _streamerQueryInterval;
	int32 _replicaUpdateInterval;
	int32 _replicaUpdateEnable;

	//add by hongquan to support penalty mask
	int32						_penaltyEnableMask;


	typedef ZQ::common::Config::Holder<Sop> SopHolder;
	std::map< std::string , SopHolder > _sopDatas;

	static void structure(ZQ::common::Config::Holder<SOPRestriction>& holder);

	void readSop(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSop(const std::string &full_path);
};

struct SOPConfig
{
	ZQ::common::Config::Holder<SOPRestriction> _sopRestrict;
	static void structure(ZQ::common::Config::Holder<SOPConfig>& holder);

	void readSOPRestriction(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerSOPRestriction(const std::string &full_path);
};

}

extern ZQ::common::Config::Loader<NGOD2::SOPConfig> _sopConfig;

#endif // end for __NGOD2_SOP_CONFIG_H__