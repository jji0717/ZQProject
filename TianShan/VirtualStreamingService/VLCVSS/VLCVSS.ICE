// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE -I$(ZQProjsPath)/TianShan/common --output-dir ./ $(InputDir)\$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE -I$(ZQProjsPath)/TianShan/common --index "TianShanIce::Streamer::VLCStreamServer::SessionIdx,TianShanIce::Streamer::VLCStreamServer::VLCStream,sessKey,case-sensitive" SessionIdx --output-dir ./ $(InputDir)\$(InputName).ice

#ifndef __ZQ_ComcastNGOD_VLCVSS_ICE__
#define __ZQ_ComcastNGOD_VLCVSS_ICE__

#include "TianShanUtils.ICE"
#include "TsStreamer.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

module Streamer
{

module VLCStreamServer
{

struct VLCItem
{
	string	name;
	string	path;
	int		whereUserCtrlNum;///where the item insert before
	int		userCtrlNum;	 ///current item user control number
	int		inTimeOffset;
	int		outTimeOffset;
	int		criticalStart;
	bool	spliceIn;
	bool	spliceOut;
	bool	forceNormal;
	int		timeLen;
};
sequence <VLCItem> VLCPlayList;

// -----------------------------
// class VLCStream
// -----------------------------
/// represents a streaming session on a VLC Streaming Server
["freeze:write"]
class VLCStream extends TianShanIce::Streamer::Playlist implements TianShanUtils::TimeoutObj
{
	string sessKey; ///< should be ident id get from pathticket
	string sessId;  ///< the session id on the target VLC Streaming Server, should be playlist id
	string pathTicketStr;///< pathTicket string to get some information
	TianShanIce::Properties props;///< the properties shared with ssm
	VLCPlayList vlcPL;///< the playlist
	string destIp;
	int destPort;
	
	
	///get the OnDemandSessionID, also the ident name
	///@return the OnDemandSessionID, should be ident.name
	["cpp:const", "freeze:read"] string getOnDemandSessionId();
	
	///get session id assigned by the target VLC Streaming Server
	///@return the session id assigned by the target VLC Streaming Server
	["cpp:const", "freeze:read"] string getSessionId();
	
	///get the pathTicketStr
	///@return the pathTicketStr
	["cpp:const", "freeze:read"] string getPathticketStr();
	
	///get the destIp
	///@return the destIp
	["cpp:const", "freeze:read"] string getDestIp();
	
	///get the destPort
	///@return the destPort
	["cpp:const", "freeze:read"] int getDestPort();
	
	///get  playlist assigned by the target VLC Streaming Server
	///@return the playlist assigned by the target VLC Streaming Server
	["cpp:const", "freeze:read"] VLCPlayList getPlayList();
	
	///renew the pathticket status to sync with weiwoo
	["freeze:write"]void renewPathTicket();
};

struct StreamStat
{
	string statName;
	long   windowSince;
	long   windowTil;
	
	int    count;
	int    min;
	int    max;
	int    avarage;
};

sequence <StreamStat> StreamStatCollection;

// -----------------------------
// Service VLCStreamService
// -----------------------------
/// represents basic stream service as a entry to access stream sessions
interface VLCStreamService extends TianShanIce::Streamer::StreamService
{
	///get the stream statistic data
	///@param statNames expected statistic item names to return
	///@param fromUTC to specify a time window util now, empty to get all stat data available regardless time window
	///@return a collection of the statistic data
	["amd"] StreamStatCollection getStreamStat(TianShanIce::StrValues statNames, string fromUTC)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;

	///find a stream by VLC OnDemandSessionId, OnDemandSessionId equals to ident.name
	///@param onDemandSessionId specify the VLC OnDemandSessionId to look for
	///@return the access to the stream instance, NULL if not found
	///@throw InvalidParameter if the OnDemandSessionId is in an illegal format
	VLCStream* findStreamByOnDemandSession(string onDemandSessionId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
		
	///find a stream by the target VLC Streaming Server session ID
	///@param sessionId specify the VLC Streaming Server session ID to look for
	///@return the access to the stream instance, NULL if not found
	VLCStream* findStreamBySession(string sessionId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
};
 
}; // module VLCStreamServer

}; // Streamer

}; // TianShanIce
#endif // __ZQ_ComcastNGOD_VLCVSS_ICE__
