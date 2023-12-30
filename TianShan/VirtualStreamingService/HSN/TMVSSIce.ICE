// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --output-dir .. $(InputDir)\$(InputName).ice  

#ifndef __ZQ_ComcastNGOD_NSS_ICE__
#define __ZQ_ComcastNGOD_NSS_ICE__

#include "TsStreamer.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

module Streamer
{

module TMVStreamServer
{

// -----------------------------
// class TMVStream
// -----------------------------
/// represents a streaming session on a HSN TreeMachine-compatbile Streaming Server
["freeze:write"]
class TMVStream extends TianShanIce::Streamer::Playlist
{
	string sessKey; ///< should be ident id get from pathticket
	string sessId;  ///< the session id on the target TMV Streaming Server
	string pathTicketStr;///< pathTicket string to get some information
	string controlURl; ///< the url thru which the client is able to control the trick operations
	TianShanIce::Properties props;///< the properties shared with ssm
	
	///get the OnDemandSessionID, also the ident name
	///@return the OnDemandSessionID, should be ident.name
	["cpp:const", "freeze:read"] string getOnDemandSessionId();
	
	///get session id assigned by the target TMV Streaming Server
	///@return the session id assigned by the target TMV Streaming Server
	["cpp:const", "freeze:read"] string getSessionId();

	///get URL to control the stream
	///@return the stream control URL
	["cpp:const", "freeze:read"] string getCtrlURL();
	
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
// Service TMVStreamService
// -----------------------------
/// represents basic stream service as a entry to access stream sessions
interface TMVStreamService extends TianShanIce::Streamer::StreamService
{
	///get the stream statistic data
	///@param statNames expected statistic item names to return
	///@param fromUTC to specify a time window util now, empty to get all stat data available regardless time window
	///@return a collection of the statistic data
	["amd"] StreamStatCollection getStreamStat(TianShanIce::StrValues statNames, string fromUTC)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;

	///find a stream by TMV OnDemandSessionId, OnDemandSessionId equals to ident.name
	///@param onDemandSessionId specify the TMV OnDemandSessionId to look for
	///@return the access to the stream instance, NULL if not found
	///@throw InvalidParameter if the OnDemandSessionId is in an illegal format
	TMVStream* findStreamByOnDemandSession(string onDemandSessionId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
		
	///find a stream by the target TMV Streaming Server session ID
	///@param sessionId specify the TMV Streaming Server session ID to look for
	///@return the access to the stream instance, NULL if not found
	TMVStream* findStreamBySession(string sessionId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
};
 
}; // module TMVStreamServer

}; // Streamer

}; // TianShanIce
#endif // __ZQ_ComcastNGOD_NSS_ICE__
