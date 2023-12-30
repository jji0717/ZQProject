#ifndef __NGOD2_SOP_CONFIG_H__
#define __NGOD2_SOP_CONFIG_H__

#include "ConfigHelper.h"
#include "StreamSmithAdmin.h"

namespace NGOD
{
struct Streamer
{
	Streamer()
	{
		bReplicaStatus			= false;
		bAllVolumeAvailble		= false;
		totalBW					= 0;
		totalBandwidth			= 0;
		usedBandwidth			= 0;
		maxStream				= 0;
		usedStream				= 0;
		streamServicePrx		= NULL;
		lastReplicaUpdate		= 0;
		setupCountersForRemote	= 0;
		setupCountersTotal		= 0;
		penalty					= 0;
		usedSession				= 0;
		failedSession			= 0;
		usedInStreamFailoverTest = 0;
	}
	//this is not a configuration
	//if _bEnabled == true , the streamer is available
	//if _bEnabled == false, the streamer is unavailable
	bool						bReplicaStatus;
	int64						lastReplicaUpdate;
	int32						penalty;	
	std::string					sourceIp;

	std::vector<std::string>	volumes;
	bool						bAllVolumeAvailble;
	std::string					storageNetId;

	//streamer net id
	std::string					netId;

	//streamer endpoint
	std::string					serviceEndpoint;

	//associated volume name , and wild card is acceptable
	std::string					volume;


	//this can be only used when read from configuration file
	int32						totalBW;

	//this is the real bandwidth used at runtime
	int64						totalBandwidth;
	int64						usedBandwidth;

	///max stream supported in this streamer
	int32						maxStream;
	int32						usedStream;


	int32						setupCountersForRemote;//
	int32						setupCountersTotal;

	///associated import channel name
	std::string					importChannel;

	int32                       enabled;//maintainence enable

	
	// add by zjm 
	int64                       usedSession;
	int64                       failedSession;


	/// FailOver TEST
	int32						usedInStreamFailoverTest;

	TianShanIce::Streamer::StreamSmithAdminPrx	streamServicePrx;
	typedef ZQ::common::Config::Holder<NGOD::Streamer> StreamerHolder;

	static void structure(StreamerHolder& holder);
};

struct Sop
{
	std::string		name;
	int32			serviceGroup;
	std::string		sopGroupName;
	typedef ZQ::common::Config::Holder<NGOD::Streamer> StreamerHolder;
	std::vector<StreamerHolder>		streamerDatas;

	static void structure(ZQ::common::Config::Holder<Sop>& holder);

	void readStreamer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerStreamer(const std::string &full_path);
};

struct SOPRestriction
{
	SOPRestriction()
	{
		enableReportedImportChannelBandWidth	= 0;
		penaltyEnableMask						= 1;
		execMask								= 1;
		timeoutPenalty							= 5;
		maxPenaltyValue							= 10;
	}
	int32 enableReportedImportChannelBandWidth;
	int32 enableWarmup;
	int32 retryCount;
	int32 maxPenaltyValue;
	int32 timeoutPenalty;
	int32 streamerQueryInterval;
	int32 replicaUpdateInterval;
	int32 replicaUpdateEnable;

	int32 penaltyEnableMask;
	int32 execMask;


	typedef ZQ::common::Config::Holder<Sop> SopHolder;
	std::map< std::string , SopHolder > sopDatas;

	static void structure(ZQ::common::Config::Holder<SOPRestriction>& holder);

	void readSop(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	void registerSop(const std::string &full_path);
};

struct SOPConfig
{
	ZQ::common::Config::Holder<SOPRestriction> sopRestrict;
	static void structure(ZQ::common::Config::Holder<SOPConfig>& holder);

	void readSOPRestriction(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void registerSOPRestriction(const std::string &full_path);
};

}

extern ZQ::common::Config::Loader<NGOD::SOPConfig> sopConfig;

#endif // end for __NGOD2_SOP_CONFIG_H__
