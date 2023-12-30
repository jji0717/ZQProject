// File Name : SsmGBss.ICE

#ifndef __SSM_GBss_ICE__
#define __SSM_GBss_ICE__

// $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice  -I$(ZQPROJSPATH)/tianshan/ice -I$(ZQPROJSPATH)/tianshan/common --output-dir . .\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/tianshan/ice -I$(ZQPROJSPATH)/tianshan/common --index "SsmGBss::StreamIdx,SsmGBss::CRGSession,streamId" StreamIdx --output-dir . .\$(InputName).ice


#include <TianShanIce.ICE>
#include <TsStreamer.ICE>

#include <TianShanUtils.ICE>

#include <Ice/Identity.ice>

module SsmGBss
{
    sequence<TianShanIce::Streamer::PlaylistItemSetupInfo> PlaylistItemSetupInfos;
    
    /// Session in Client-Request-Gateway
    ["freeze:write"]
    class CRGSession implements TianShanUtils::TimeoutObj
    {
        /// the unique id of the CRGSession
		Ice::Identity ident;
		
		string requestURL;
		
		// 
		string streamerSource;
		
		// streamer Net Id
		string streamerNetId;
		
		/// the unique ID of the stream in the StreamService
		string streamId;
		
		/// the expiration timestamp of the session
		long expiration;
		
		/// connection ID between STB and Streaming Server
		/// if stream control proxy model is used, STBConnectionID may equal to SRMConnectionID
		string STBConnectionID;

		string globalSessionId;
		
		/// 
		int announceSeq;
		
		/// access the stream instance on StreamService
		TianShanIce::Streamer::Stream* stream; 
		
		PlaylistItemSetupInfos playlist;
		
		TianShanIce::Properties metadata;
		
		///renew session context
		void renew(long ttl);

        string getAnnounceSeq();

		["freeze::read","cpp:const"]
		string	getGlobalSessId( );
        
        ///get all metadata of the CRGSession
		["cpp:const", "freeze:read"] TianShanIce::Properties getMetaData();
		
		["cpp:const", "freeze:read"] TianShanIce::Streamer::Stream* getStream(out string strStreamerNetId, out string strStreamId);
		
		
		/// set a list of metadata: overwrite the existing if the CRGSession::metadata has it, 
		/// otherwise add as new metadata
		void setMetaData(TianShanIce::Properties newMetadata);
		
		void attachStream(TianShanIce::Streamer::Stream* newstream);

        /// destroy stream instance in real streaming server
		bool destroy();
    };
    
    /// streamer's statistics infos
    struct StreamerStatistics
    {
		long penaltyValue;
		string streamerSource;
		string streamerNetId;
		string streamerEndpoint;
		string attachedVolumeName;
		string importChannelName;
		long totalBandwidth;
		long usedBandwidth;
		int maxStreamCount;
		int usedStreamCount;
		int available;
		int adminEnabled;
		
		// usedSessions = remoteSessions + localSessions
		long usedSessions;
		long failedSessions;
		
		long remoteSessions;
		long localSessions;
		
		TianShanIce::Properties extendProps;
    };
    sequence<StreamerStatistics> StreamersStatistics;
    struct StreamersStatisticsWithStamp
    {
		string stampMeasuredSince;
		StreamersStatistics streamersInfos;
    };
  
    /// importChannel's statistics infos  
    struct ImportChannelStatistics
	{
		string channelName;
		int runningSessCount;
		long totalImportBandwidth;
		long usedImportBandwidth;
		TianShanIce::Properties extendProps;	
	};
	sequence<ImportChannelStatistics> ImportChannelsStatistics;
	struct ImportChannelsStatisticsWithStamp
	{
		string stampMeasuredSince;
		ImportChannelsStatistics importChannelsInfos;
	};
  
    interface GBssServant extends TianShanIce::ReplicaSubscriber
    {
		///query the current streamer statictics
		///@return the StreamerStat
		/// InvalidStateOfArt, //< if this API gets called while service initialization or uninitialization
		/// ServerError; //< if have some internal server errors
		StreamersStatisticsWithStamp getStreamerInfos() throws TianShanIce::InvalidStateOfArt, TianShanIce::ServerError;
		
		/// query for the statictics of ImportChannels
		ImportChannelsStatisticsWithStamp getImportChannelStat();
			
		/// for administration purpose, to enable/disable a streamer
		void enableStreamers(TianShanIce::StrValues streamerNetIds, bool enable);
			
		///reset counters record the setup/remote count
		void resetCounters();
     };
    
}; // end SsmGBss

#endif // end __SSM_GBss_ICE__
