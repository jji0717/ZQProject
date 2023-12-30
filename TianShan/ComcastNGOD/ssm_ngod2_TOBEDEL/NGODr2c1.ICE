#ifndef _SSM_NGOD_R2C1_ICE_
#define _SSM_NGOD_R2C1_ICE_

#include <TsStreamer.ICE>
#include <Ice/Identity.ICE>

module NGODr2c1
{
	sequence<TianShanIce::Streamer::PlaylistItemSetupInfo> PlaylistItemSetupInfos;

    // add by zjm to support session history	
	enum SessionEventType
	{
	    UserEvent,
	    StartStreamEvent,
	    EndEvent,
	    RecoverableError,
	    Transition,
	};
	struct SessionEventRecord
	{
	    SessionEventType        eventType;
	    string                  eventTime;
	    string                  NPT;
	    string                  streamResourceID; 
	    TianShanIce::Properties prop;
	};
	sequence<SessionEventRecord> SessionEventRecords;
	
	
	/*
	struct StreamResource
	{
	    int      ctrlNum;
	    string   providerID;
	    string   assetID;
	    string   fileName;
	};
	sequence<StreamResource> StreamResources; 
	*/
	
	struct ctxData
	{
		Ice::Identity		ident;		
		string				onDemandID;
		string				streamFullID;
		string				streamShortID;
		string				normalURL;
		string				resourceURL;
		string				connectID;
		string				groupID;
		string				streamNetId;
		string				sopname; //also used for sop_group
		string				importChannelName;
		long				expiration;
		long				usedBandwidth;
		int					announceSeq;
		
		///extension for recoding setup information so that we can use these informations in other stage
		PlaylistItemSetupInfos	setupInfos;
		TianShanIce::Properties prop; 
		
		// add by zjm to support session history	
		SessionEventRecords sessionEvents; 
		//StreamResources     streamRes;
	};

	["freeze:write"] 
	class Context
	{
		ctxData data;
		string streamShortID;
		string groupID;	
		
		void addEventRecord(SessionEventRecord eventRecord);
		
		///get Context
		["cpp:const", "freeze:read"] void getState(out ctxData data);

		///renew context
		void renew(long ttl);

		void increaseAnnSeq();

		void onTimer();

		["cpp:const", "freeze:read"] TianShanIce::Streamer::Stream* getStream();
		
		void updateCtxProp(string key, string val);
		
		["cpp:const", "freeze:read"] string getCtxPropItem(string key);
	};
	
	sequence<ctxData> CtxDatas;
	
	//define sop usage structure and this can help us export runtime data out to client
	
	struct StreamerUsage
	{
		long		penaltyValue;	//penalty value
		string		streamerNetId;
		string		streamerEndpoint;
		string		attachedVolumeName;
		string		importChannelName;
		long		totalBandwidth;	//total bandwidth in bytes
		long		usedBandwidth;	//used bandwidth in bytes		
		int			maxStreamCount;
		int			usedStreamCount;				
		int			available; // 1 for available 0 for NA
		int         maintenanceEnable;
		
		// add by zjm 
		long        usedSession;
		long        failedSession;
		
		long		histCountRemoteSess;	//successfully setup
		long        histCountTotalSess;		//successfully setup
	};
	
	sequence<StreamerUsage>	StreamerUsageS;
	
	struct ImportChannelUsage
	{
		string			channelName;
		int				runningSessCount;
		long			totalImportBandwidth;
		long			usedImportBandwidth;		
	};
	sequence<ImportChannelUsage> ImportChannelUsageS;
	
	struct  SopUsage
	{
		int					servieGroupId;
		StreamerUsageS		streamerUsageInfo;
	};
	
	dictionary<string,SopUsage>	NgodUsage;

	class SessionView
	{
		// get a snapshot of all session contexts for the client who call this function
		// clientId[out], return an uniqued id for the client who will use this value for the later method call
		// which is encapsulated in ice.context.
		// return value indicates how many session contexts has been gaint in snapshot.
		// note that the SessionView keeps all snapshots for all its clients, every client has its own session
		// snapshot.
		int getAllContexts(out int clientId);
		
		// get the timeout value of session lifecycle, unit is second
		int getTimeoutValue();
		
		// get the session contexts whose groupIds equal to the input parameter sessionGroup from the corresponding
		// snapshot.
		// pre-condition, the clientId must be encapsulated in ice.context, otherwise an exception will be throw to
		// indicate this error
		// return value indicates how many session contexts has been gaint in snapshot.
		int getContextsBySG(string sessionGroup)
			throws TianShanIce::ClientError;
		
		// return the session contexts
		// pre-condition, the clientId must be encapsulated in ice.context, otherwise an exception will be throw to
		// indicate this error
		CtxDatas getRange(int from, int to)
			throws TianShanIce::ClientError;
			
		// return the session contexts whose groupIds equal to the input parameter sessionGroup from the corresponding
		// snapshot.
		// pre-condition, the clientId must be encapsulated in ice.context, otherwise an exception will be throw to
		// indicate this error
		CtxDatas getRangeBySG(int from, int to, string sessionGroup)
			throws TianShanIce::ClientError;
		
		///query current ngod resource usage
		void	 getNgodUsage( out NgodUsage usage, out string stampMeasuredSince);
		
		void	 getImportChannelUsage( out ImportChannelUsageS usage );
		
		void     enableStreamers(TianShanIce::StrValues streamerNames, bool enable);
		
		///reset counters record the setup/remote count
		void     resetCounters();
	};

};

#endif
