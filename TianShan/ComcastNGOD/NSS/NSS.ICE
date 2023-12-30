// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: TsStreamer.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ComcastNGOD/NSS/NSS.ICE $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 3     08-07-21 15:32 Xiaoming.li
// modify nonmutating to ["cp:const", "freeze:read"]
// 
// 2     08-06-13 11:24 Xiaoming.li
// 
// 1     08-04-03 12:12 Hui.shao
// ===========================================================================
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

module NGODStreamServer
{

/*
struct NGODPlaylistItem
{
	string providerId;
	string providerAssetId;
	int    rangeFrom;
	int    rangeTo;
	string tricksAllowed;
	
	TianShanIce::Properties properties;
};

sequence <NGODPlaylistItem> NGODPlaylist;
*/

// -----------------------------
// class NGODStream
// -----------------------------
/// represents a streaming session on a NGOD-compatbile Streaming Server
["freeze:write"]
class NGODStream extends TianShanIce::Streamer::Playlist
{
	string sessKey; ///< formatted as "<sessGroup>#<sessIdNSS>"
	string sessIdNss; ///< the session id on the target NGOD Streaming Server
	string sessGroup; ///< the session group of the this stream
	string controlURl; ///< the url thru which the client is able to control the trick operations
	
	///get the NGOD OnDemandSessionID
	///@return the NGOD OnDemandSessionID, should be ident.name
	["cpp:const", "freeze:read"] string getOnDemandSessionId();

	///get session id assigned by the target NGOD Streaming Server
	///@return the session id assigned by the target NGOD Streaming Server
	["cpp:const", "freeze:read"] string getNssSessionId();

	///get session group id assigned by this NGODStreamService
	///@return the session group id assigned by this NGODStreamService
	["cpp:const", "freeze:read"] string getSessionGroup();

	///get URL to control the stream
	///@return the stream control URL
	["cpp:const", "freeze:read"] string getCtrlURL();
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
// Service NGODStreamService
// -----------------------------
/// represents basic stream service as a entry to access stream sessions
interface NGODStreamService extends TianShanIce::Streamer::StreamService
{
	///get the stream statistic data
	///@param statNames expected statistic item names to return
	///@param fromUTC to specify a time window util now, empty to get all stat data available regardless time window
	///@return a collection of the statistic data
	["amd"] StreamStatCollection getStreamStat(TianShanIce::StrValues statNames, string fromUTC)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
	
	///find a stream by NGOD OnDemandSessionId
	///@param onDemandSessionId specify the NGOD OnDemandSessionId to look for
	///@return the access to the stream instance, NULL if not found
	///@throw InvalidParameter if the OnDemandSessionId is in an illegal format
	NGODStream* findStreamByOnDemandSession(string onDemandSessionId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;

	///find a stream by the target NGOD Streaming Server session ID
	///@param sopName specify the NGOD sopName to look for
	///@param onDemandSessionId specify the NGOD Streaming Server session ID to look for
	///@return the access to the stream instance, NULL if not found
	NGODStream* findStreamByNssSession(string sopName, string nssSessionId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
};
 
}; // module NGODStreamServer

}; // Streamer

}; // TianShanIce
#endif // __ZQ_ComcastNGOD_NSS_ICE__
