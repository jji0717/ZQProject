
#ifndef _ngod_ice_definition_header_file_h__
#define _ngod_ice_definition_header_file_h__

#include <TsStreamer.ICE>
#include <Ice/Identity.ice>

module NGOD
{

sequence<TianShanIce::Streamer::PlaylistItemSetupInfo> PlaylistItemSetupInfos;

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


enum RequestVerb
{
	requestUnknown,
	requestSETUP,
	requestPLAY,
	requestPAUSE,
	requestTEARDOWN,
	requestRECORD,
	requestDESCRIBE,
	requestOPTIONS,
	requestGETPARAMETER,
	requestSETPARAMETER,
	requestPING,
	requestRESPONSE,
};

class NgodClientRequest
{	
	string						connectionId;	//connection id for current request, can be empty
	string						sessionId;		//short cut of rtsp session id
	string						ondemandId;		//short cut of ondemand session id
	string						cseq;			//short cut of cseq
	string						verbstr;		//short cut of verb string
	string						userAgent;		//short cut of user-agent
	int							protocolVerCode;//protocol versioning code
	RequestVerb					verb;
	
	//TianShanIce::StrValues		requestHeaders;
	//string						requestBody;
	string						originalUrl;
	string						originalTransport;
	
	
	void						setStartline( string startLine );
	void						setHeader( string key , string value );
	void						setBody( string body );	
	bool						post();
};

class NgodServerRequest
{
	void						setStartline( string startLine );
	void						setHeader( string key , string value );
	void						setBody( string body );	
	bool						post();
};

enum StreamEventRoutine
{
	streamEventPING,
	streamEventENDOFSTREAM,
	streamEventBEGINOFSTREAM,
	streamEventSPEEDCHANGE,
	streamEventSTATECHANGE,
	streamEventITEMSTEP,
	streamEventEXIT,
	streamEventEXIT2,
	streamReposition,
	streamEventPauseTimeout
};

struct StreamEventAttr
{
	string								proxyString;
	string								playlistString;
	float								previousSpeed;
	float								currentSpeed;
	TianShanIce::Streamer::StreamState	prevState;
	TianShanIce::Streamer::StreamState	currentState;
	int									exitCode;
	string								exitReason;
	int									currentCtrlNum;
	int									prevCtrlNum;
	TianShanIce::Properties				props;
	long								eventIndex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
///For SessionView
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
class NgodSession
{
	Ice::Identity						mIdent;				//identity of this object
	
	TianShanIce::State					mState;
	
	string								mSessId;			//be the same as mIdent.name
	string								mOndemandId;		//OnDemandSessionId use by client to identify a session
	string								mOriginalUrl;		//original url passed from client
	string								mServerIp;
	string								mClientIp;
	
	//shuold I record these information ?	
	string								mC1ConnectionId;	//c1 connection id
	
	string								mGroupId;			//which session group is this object belong to	
	
	int									mR2VerControlCode;	//
	int									mC1VerControlCode;	//
	
	//resource
	TianShanIce::Streamer::Playlist*	mStream;			//stream session proxy
	string								mStreamSessId;		//Stream session id
	string								mStreamerNetId;		//net of streamer on which the stream is created on
	string								mSopName;			//sop name
	string								mImportChannelName;	//
	long								mUsedBW;			//used bandwidth count in byte
	
	int									mAnnounceSeq;		//anounce sequence
	
	long								expireTime;			//
	int									expiredCount;
	
	PlaylistItemSetupInfos				mItemInfos;			//information of items in the playlist
	
	TianShanIce::Properties				mProps;				//properties
	
	SessionEventRecords					mSessionEvents;		//session events
	
	
	void						updateTimer( );
	
	//invoked when session is expired
	void						onTimer( );
	
	["freeze::read","cpp:const"]
	string						getSessionId( );
	
	["freeze::read","cpp:const"]
	string						getOndemandSessId( );
	
	///I am not sure if this is a must
	void						updateC1ConnectionId( string connId );
	
	//setup a session, session's state will change from init to setup if success
	
	int							processRequest( NgodClientRequest request );
	
	int							onStreamSessionEvent( StreamEventRoutine type, StreamEventAttr attr );
	
	int							destroy(  NgodClientRequest request );
	
	void						onRestore( );

	///helper function
	///translate session record to CxtData
	void						convertToCtxData( out ctxData data );

	void						rebindStreamSession( TianShanIce::Streamer::Playlist* stream, string streamSessId, string streamerNetId);
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


#endif//_ngod_ice_definition_header_file_h__

