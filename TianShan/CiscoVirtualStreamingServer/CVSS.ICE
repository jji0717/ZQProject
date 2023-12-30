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
// Ident : $Id: CVSS.ICE $
// Branch: $Name:  $
// Author: LXM
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CiscoVirtualStreamingServer/CVSS.ICE $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 2     09-01-20 9:48 Xiaoming.li
// add some interface to heartbeat and response announce
// 
// 1     08-12-15 9:07 Xiaoming.li
// initial checkin
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --output-dir .. $(InputDir)\$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --index "TianShanIce::Streamer::CiscoVirtualStreamServer::SessionIdx,TianShanIce::Streamer::CiscoVirtualStreamServer::CVStream,sessKey,case-sensitive" SessionIdx --output-dir .. $(InputDir)\$(InputName).ice

#ifndef __ZQ_CVSS_ICE__
#define __ZQ_CVSS_ICE__

#include "TsStreamer.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

module Streamer
{

module CiscoVirtualStreamServer
{

struct stItem
{
	string	contentName;	///< the name of the content to put into the list
	long	userCtrlNum;	///< the item list index
	long	inTimeOffset;	///< the cue in offset in ms of the given media content
	long	outTimeOffset;  ///< the cue out offset in ms of the given media content
	long	criticalStart;  ///< the system time when the item must be played
	bool	spliceIn;		///< true if splice in is enabled
	bool	spliceOut;		///< true if splice out is enabled
	bool	forceNormal;	///< true if the playback speed must be reset to normal once this item is being played
	long	flags;			///< additional flags defined as PLISFLAG_XXX
	long	contentLen;		///< item play time length
	//string	destination;	///< destination client ip address
	//string	clientPort;		///< destination client port
};
sequence <stItem> ItemList;

// -----------------------------
// class CVStream
// -----------------------------
/// represents a streaming session on a Cisco-compatbile Streaming Server(CDS)
["freeze:write"]
class CVStream extends TianShanIce::Streamer::Playlist
{
	string		sessKey;			///< the index of this session, socket number >
	string		sessId;				///< the session id on the target Cisco Virtual Streaming Server
	string		controlURl;			///< the url thru which the client is able to control the trick operations
	long		curPlayListIdx;		///< current playlist item number
	ItemList	playList;			///< play list
	string		pathTicketStr;		///< pathTicket string to get some information
	string		destination;		///< destination client ip address
	long		clientPort;			///< destination client port

	///get session id assigned by the target Cisco Streaming Server
	///@return the session id assigned by the target Cisco Streaming Server
	["cpp:const", "freeze:read"] string getSessionId();

	///get URL to control the stream
	///@return the stream control URL
	["cpp:const", "freeze:read"] string getCtrlURL();
	
	///get the ItemList list
	///@return ItemList list
	["cpp:const", "freeze:read"] ItemList getItemList();

	///get the current playlist index
	///@return the current playlist index
	["cpp:const", "freeze:read"] long getCurPlayListIdx();
	
	///get the PathTicket proxy string
	///@return the PathTicket proxy string
	["cpp:const", "freeze:read"] string getPathTicketStr();
	
	///get the client destiantion ip address
	///@return the ip address
	["cpp:const", "freeze:read"] string getDestination();
	
	///get the client port
	///@return the client port
	["cpp:const", "freeze:read"] long getClientPort();
	
	///do heartbeat with RTSP server
	["cpp:const", "freeze:read"] void doHeartBeat();
	
	///play with RTSP server use the next item content id
	["freeze:write"] void playNextItem();
	
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
// Service CVStreamService
// -----------------------------
/// represents basic stream service as a entry to access stream sessions
interface CVStreamService extends TianShanIce::Streamer::StreamService
{
	///get the stream statistic data
	///@param statNames expected statistic item names to return
	///@param fromUTC to specify a time window util now, empty to get all stat data available regardless time window
	///@return a collection of the statistic data
	["amd"] StreamStatCollection getStreamStat(TianShanIce::StrValues statNames, string fromUTC)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
	
	///find a stream by the TCP connection socket ID
	///@param sockId specify the TCP connection socket ID to look for
	///@return the access to the stream instance, NULL if not found
	CVStream* findStreamBySocket(long sockId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;

	///find a stream by the target Cisco Virtual Streaming Server session ID
	///@param sessionId specify the Cisco Virtual Streaming Server session ID to look for
	///@return the access to the stream instance, NULL if not found
	CVStream* findStreamBySession(string sessionId)
	throws
		TianShanIce::InvalidParameter,
		TianShanIce::ServerError;
};
 
}; // module NGODStreamServer

}; // Streamer

}; // TianShanIce
#endif // __ZQ_CVSS_ICE__
