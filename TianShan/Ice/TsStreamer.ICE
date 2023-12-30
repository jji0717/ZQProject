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
// $Log: /ZQProjs/TianShan/Ice/TsStreamer.ICE $
// 
// 15    9/09/15 2:58p Hongquan.zhang
// 
// 14    8/25/15 12:05p Hui.shao
// 
// 13    8/25/15 11:34a Hui.shao
// new item restriction for Gehua ADS insertion
// 
// 12    2/06/15 11:46a Hui.shao
// enh#20635 added destroy2(feedback) to collect feedbacks of stream
// 
// 11    9/22/14 1:53p Hongquan.zhang
// add getPlaylistItems for retrieving items information in any stage
// 
// 10    4/23/14 11:25a Hui.shao
// rollback, move getPlaylistInfo to Application::Purchase
// 
// 8     8/13/13 2:52p Bin.ren
// 
// 6     1/17/12 12:59p Hongquan.zhang
// 
// 5     1/16/12 2:13p Hongquan.zhang
// 
// 3     12/21/11 4:27p Hongquan.zhang
// 
// 2     6/28/11 10:56a Hui.shao
// added new item flag PLISFlagOnce
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 59    10-10-19 15:23 Haoyuan.lu
// 
// 58    10-04-21 16:17 Hongquan.zhang
// merge from 1.10
// 
// 57    09-08-31 15:19 Hongquan.zhang
// 
// 56    09-02-13 14:50 Hui.shao
// added per-stream property access
// 
// 55    08-12-25 14:33 Hongquan.zhang
// add playitem
// 
// 54    08-10-28 18:01 Hongquan.zhang
// add infoSTREAMSOURCE for query Streaming Source IP port
// 
// 53    08-10-28 17:58 Hui.shao
// add replica query interface to streamservice
// 
// 52    08-09-01 16:40 Hui.shao
// 
// 51    08-09-01 13:48 Hui.shao
// 
// 50    08-08-14 14:30 Hui.shao
// merge from 1.7.10
// 
// 49    08-07-07 15:50 Hui.shao
// 
// 48    08-05-13 16:48 Hongquan.zhang
// 
// 47    08-03-27 17:58 Hui.shao
// 
// 46    08-03-27 14:40 Hui.shao
// 
// 43    08-03-27 11:57 Hui.shao
// added PlaylistItemSetupInfo::flags definitions and Stream::commit()
// 
// 42    07-09-19 12:06 Hui.shao
// added extended stream operations
// 
// 38    07-08-13 16:09 Hui.shao
// rollback the Stream::StateChanged() and SpeedChanged() definition
// 
// 35    07-05-15 15:01 Hongquan.zhang
// 
// 34    07-04-19 12:15 Hongquan.zhang
// 
// 33    07-03-13 12:37 Hui.shao
// renamed namespace Weiwoo to SRM, AccreditPath to Transport
// 
// 32    07-03-13 11:50 Hui.shao
// 
// 31    06-11-21 16:53 Hongquan.zhang
// 
// 30    06-11-14 10:54 Hongquan.zhang
// 
// 29    06-10-25 15:51 Hongquan.zhang
// 
// 28    06-10-19 18:18 Hongquan.zhang
// 
// 27    06-10-19 18:07 Hongquan.zhang
// 
// 26    10/13/06 11:59a Hui.shao
// 
// 25    06-09-19 14:56 Hui.shao
// 
// 24    06-09-19 14:52 Hongquan.zhang
// 
// 23    06-09-15 17:32 Hongquan.zhang
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_Streamer_ICE__
#define __ZQ_TianShanIce_Streamer_ICE__

#include "TianShanIce.ICE"
#include "TsEvents.ICE"
#include "TsTransport.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

// -----------------------------
// forward declarations
// -----------------------------
module Transport
{
	class PathTicket;
};

// -----------------------------
// namespace Streamer
// -----------------------------
/// Streamer represents the streaming session management subsystem within the TianShan architecture
module Streamer
{

// forward declarations
class Stream;

// -----------------------------
// enum State
// -----------------------------
/// the state of streaming
enum StreamState
{
	stsSetup,
	stsStreaming,
	stsPause,
	stsStop,
};

// -----------------------------
// struct StreamerDescriptor
// -----------------------------
/// a brief descriptor to the Streamer device inside of the StreamService, the StreamService own its explaination
struct StreamerDescriptor
{
	string deviceId; ///< a StreamService-wide unique id to indentify a streamer device, no white space is allowed
	string type;	 ///< a short type string, explained by StreamService, no white space is allowed
};

/// a collection of StreamerDescriptor
sequence <StreamerDescriptor> StreamerDescriptors;

const int InvalidCtrlNum = -2; //< Invalid control number
const int PlaylistHeadCtrlNum = -3;
const int PlaylistTailCtrlNum = -4;

// -----------------------------
// struct PlaylistItemSetupInfo
// -----------------------------
const long PLISFlagNoPause   = 0x0001; ///< PlaylistItemSetupInfo::flags(1<<0) forbid pause the item if this flag is on
const long PLISFlagNoFF      = 0x0002; ///< PlaylistItemSetupInfo::flags(1<<1) forbid fastforward the item if this flag is on, also forbid to jump over such an item by default unless PLISFlagSkipAtFF is on
const long PLISFlagNoRew     = 0x0004; ///< PlaylistItemSetupInfo::flags(1<<2) forbid rewind the item if this flag is on, also forbid to jump over such an item by default unless PLISFlagSkipAtRew is on
const long PLISFlagNoSeek    = 0x0008; ///< PlaylistItemSetupInfo::flags(1<<3) forbid seek into or out-of the item if this flag is on
const long PLISFlagPlayTimes = 0x00F0; ///< PlaylistItemSetupInfo::(flags&PLISFlagPlayTimes)>>4). count of item playtimes, would be removed from the playlist once the count is reached, >=0xE means unlimited
const long PLISFlagSkipAtFF  = 0x0100; ///< PlaylistItemSetupInfo::flags(1<<8) item would be skipped if the current speed is a fast-forward
const long PLISFlagSkipAtRew = 0x0200; ///< PlaylistItemSetupInfo::flags(1<<9) item would be skipped if the current speed is a rewind

/// defines the setup information for each item in the list
struct PlaylistItemSetupInfo
{
	string	contentName;    ///< the name of the content to put into the list
	long	inTimeOffset;   ///< the cue in offset in ms of the given media content
	long	outTimeOffset;  ///< the cue out offset in ms of the given media content
	long	criticalStart;  ///< the system time when the item must be played
	bool	spliceIn;       ///< true if splice in is enabled
	bool	spliceOut;      ///< true if splice out is enabled
	bool	forceNormal;    ///< true if the playback speed must be reset to normal once this item is being played
	long	flags;          ///< additional flags defined as PLISFLAG_XXX
	ValueMap privateData;   ///< per-session and per-content privateData. For example, the pre-encryption parameters
};
sequence <PlaylistItemSetupInfo> PlaylistItemSetupInfoS;

/// enum of stream conditional control options
enum ConditionalControlOption
{
	sccUndefined,   ///< dummy flag.
	sccApprove,     ///< directly approve this stream control without asking anyone else
	sccDeny,        ///< directly deny this stream control without asking anyone else
	sccAuthorize,   ///< ask external module to authorize this stream control via ConditionalControl::authorize()
};

/// the mask of the conditional control options
struct ConditionalControlMask
{
	ConditionalControlOption play;  ///< control mask for the Stream::Play() operation
	ConditionalControlOption changeSpeedForward;  ///< control mask for the Stream::dhangeSpeed() operation in the forward direction
	ConditionalControlOption changeSpeedBackward; ///< control mask for the Stream::dhangeSpeed() operation in the backward direction
	ConditionalControlOption pause; ///< control mask for the Stream::pause() operation
	ConditionalControlOption seek; ///< control mask for the seek operation
};

/// callback of the conditional stream control
interface ConditionalControl
{
	/// perform the authorization procedure for the stream control on the given stream instance 
	///@param[in] strmobj   the access to the stream instance on which a stream control need to be authorized
	///@param[in] ctrl      the mask value to identify what kind of control this authorization is for.
	///                     i.e. <i>ctrl.play != sccUndefined </i>means it is a play operation on the given stream
	///@return    <i>sccApprove</i> if the stream control is approved, otherwise <i>sccDeny</i>. No other return
	///           values will be expected
	ConditionalControlOption authorize(Stream* strmobj, ConditionalControlMask ctrl);
};

// -----------------------------
// struct StreamInfo
// -----------------------------
/// information about the stream instance
struct StreamInfo
{
	Ice::Identity   ident;          ///< the ident of the stream instance
	StreamState     state;          ///< the state of the stream
	TianShanIce::Properties props;  ///< properties of the stream
};

const string StreamPorpTimeOffset   = "TimeOffset";   ///< the current NPT position in msec
const string StreamPorpCurrentSpeed = "CurrentSpeed"; ///< the current speed in float value

// -----------------------------
// class Stream
// -----------------------------
/// represents a basic streaming session
["freeze:write"] 
class Stream
{
	Ice::Identity   ident;
	
	/// allot a reserved "Accredited Path" associate with the given ticket into the playlist
	/// the playlist may query the ticket for the parameters such as the resources information
	/// bound on this ticket
	///@param[in] the access to the ticket
	///@throw
	///   TianShanIce::InvalidStateOfArt the method is invoked at a wrong state of the playlist \n
	void allotPathTicket(Transport::PathTicket* ticket)
	throws	Transport::ExpiredTicket,
		   ServerError,
			TianShanIce::NotSupported;

	/// this entry is design to notify when the stream has been initially rendered by the application
	["amd"] void commit()
	throws ServerError;

	///	destroy the stream session
	void destroy()
		throws ServerError;

	/// get the last error message recorded
	["cpp:const", "freeze:read"] string lastError();

	/// return the identity of this stream session
	["cpp:const", "freeze:read"] idempotent Ice::Identity getIdent();

	/// Set the conditional control access for this stream instance. If a stream is set with the conditional
	/// access, the stream control operation will be authorized
	///@param[in] mask		specify whether the stream control operation should be always apporved, denied or
	///                     authorized by an external module
	///@param[in] condCtrl  callback interface to the external authorization module, non-NULL is require if 
	///                     any field of mask is sccAuthorize
	void setConditionalControl(ConditionalControlMask mask, ConditionalControl* condCtrl);

	// stream control
	// --------------------
	/// play this stream session
	///@return false if this operation failed
	["amd"] bool play()
		throws ServerError, InvalidStateOfArt;

	/// change the play speed
	///@param[in] newSpeed the new speed to apply on the stream session, the actual play speed will be rounded
	///           positive value means forward while negative means backforward, (-1, 1) means slow motion
	///@return false if this operation failed
	bool setSpeed(float newSpeed)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;

	/// pause a playing stream
	///@return false if this operation failed
	bool pause()
		throws ServerError,
			   InvalidStateOfArt;

	/// resume a paused stream
	///@return false if this operation failed
	bool resume()
		throws ServerError,
			   InvalidStateOfArt;

	///Get current stream status
	["cpp:const"] StreamState getCurrentState()
		throws ServerError;
	
	/// Get associated TianShan session instance
	///@return the access to the bound session instance
	SRM::Session* getSession()
		throws ServerError;

	///set mux rate	
	///@param nowRate current bitrate,you can pass 0 for default
	///@param maxRate max bitrate
	///@param minrate min bitrate
	void	setMuxRate(int nowRate,int maxRate,int minRate);

	///alloc dvbc resource
	///@return true if success false if fail
	///@param serviceGroupID target service group id
	///@param bandWidth needed bandwidth in kbps
	bool	allocDVBCResource(int serviceGroupID , int bandWidth)
		throws ServerError;
	
	///seek to a specified offset from the beginning of s stream	
	///@return the exact offset that the service seek to ,in millisecond
	///@param offset millisecond from beginning of a stream	
	///@param startPos the start position for seeking  1==begining of the stream  2==end of stream
	///NOTE if startPos == 1, the offset must be positive
	///		if startPos == 2, the offset must be negative
	["amd"] long	seekStream(long offset, int startPos)
		throws ServerError,InvalidParameter;

	// extended stream control operations
	// --------------------
	/// extended-play() play this stream session
	///@param[in] newSpeed the new speed to apply on the stream session, the actual play speed will be rounded
	///           positive value means forward while negative means backforward, (-1, 1) means slow motion
	///           for the reason of a float value, abs(newSpeed) < 0.01 would be considered as 0.0, which mean keep the current speed
	///@param[in] offset millisecond from beginning of a stream	
	///@param[in] from the start position for seeking 
	///            0 - from the current streaming location
	///            1 - from the begining of the stream, the offset must be positive
	///            2 - from the end of stream, the offset must be negative
	///@param[in] expectedProps the property names that wish to receive in the returning StreamInfo::props, @sa the StreamPorpXXX consts
	///@return the stream information record after the operation is executed
	///@note newSpeed = 0.0 means no change on speed instead of PAUSE
	///@note from=0 and offset=0 means no change on location, continue with the current position
	["ami","amd"] StreamInfo playEx(float newSpeed, long offset, short from, StrValues expectedProps)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;

	///  extended-pause() pause this stream session
	///@param[in] expectedProps the property names that wish to receive in the returning StreamInfo::props, @sa the StreamPorpXXX consts
	///@return the stream information record after the operation is executed
	["ami"] StreamInfo pauseEx(StrValues expectedProps)
		throws ServerError,
			   InvalidStateOfArt;
			   
	/// query for the properties of the stream
	///@return the map of properties
	["cpp:const", "freeze:read"] TianShanIce::Properties getProperties()
		throws ServerError;
		
	/// update properties into the stream, will overwrite those already in stream if the keys are matched
	/// @param props a map of properties to set
	void setProperties(TianShanIce::Properties props)
		throws ServerError;
			   
	/// query for the resources that have been employed by the stream
	/// @return the resources that have been employed by the stream
	["cpp:const", "freeze:read"] TianShanIce::SRM::ResourceMap getResources()
		throws ServerError;

	///	the extended destroy() with properties returned as feedback
	void destroy2(out TianShanIce::Properties props)
		throws ServerError;
};

// -----------------------------
// interface StreamProgressSink
// -----------------------------
/// topic of stream progress notifcation, see @sa StreamProgressSink
const string TopicOfStreamProgress			= "TianShan/Event/Streamer/StreamProgress";

///@brief interface to subscribe the progress updates fired with TopicOfStreamProgress
/// during streaming execution
interface StreamProgressSink extends TianShanIce::Events::BaseEventSink
{
	/// event fired when the end of the stream has some progress
	///@param[in] proxy  the proxy string to access the stream object
	///@param[in] id	 the value of ident.name to identify the stream instance in a string format
	///@param[in] done	 work with "total" to give a progress description: done/total
	///@param[in] total	 work with "done" to give a progress description: done/total
	///@param[in] step	 work with "totalsteps" to give a progress description in a multiple steps' stream: "step <step> of <totalsteps> steps"
	///@param[in] totalsteps work with "step" to give a progress description in a multiple steps' stream: "step <step> of <totalsteps> steps"
	["cpp:const"] void OnProgress(string proxy, string id, int done, int total, int step, int totalsteps, string comment, Properties props  );
};

// -----------------------------
// interface StreamEventSink
// -----------------------------
/// topic of stream events, see @sa StreamEventSink
const string TopicOfStream				="TianShan/Event/Streamer/Stream";
enum StreamErrors
{
	seSuccess,
	seServerError,
	seReadingContentData,
	seDownstreamFailure,
	seBandwidthExceeded,
	seDownstreamDestUnreachable
};

///@brief interface to subscribe the events fired with topic TopicOfStream during streaming execution
interface StreamEventSink extends TianShanIce::Events::BaseEventSink
{
	/// event fired when the end of the stream session has been reached, the resource bound on this
	/// stream session will be released after this event.
	///@param[in] proxy  the proxy string to access the stream object
	///@param[in] id	 the value of ident.name to identify the stream instance in a string format
	///@note for a playlist with multiple items, this event means the end of playlist has been reached
	///      if you wish to sink the end of an item in a playlist, please use PlaylistEventSink
	///@sa   PlaylistEventBind::OnEndOfItem()
	["cpp:const"] void OnEndOfStream(string proxy, string id , Properties props );

	/// event fired when the beginning of the stream session has been reached, especially for rewind operations
	/// the resource bound on this stream session will be released after this event.
	///@param[in] proxy  the proxy string to access the stream object
	///@param[in] id	 the value of ident.name to identify the stream instance in a string format
	///@note for a playlist with multiple items, this event means the end of playlist has been reached
	///      if you wish to sink the end of an item in a playlist, please use PlaylistEventSink
	///@note this event doesn't equal to the stream start event, please use StreamEventSink::OnStateChanged()
	///      and StreamProgressSink::OnProgress() instead if you are insterested at stream start
	///@sa   PlaylistEventBind::OnEndOfItem()
	["cpp:const"] void OnBeginningOfStream(string proxy, string id , Properties props);

	/// event fired when a stream's state has been changed
	///@param[in] proxy the proxy string to access the stream object
	///@param[in] id	the value of ident.name to identify the stream instance in a string format
	///@param[in] prevState the previous state of the stream
	///@param[in] currentState the current state of the stream
	["cpp:const"] void OnStateChanged(string proxy, string id, StreamState prevState, StreamState currentState , Properties props );

	/// event fired when a stream's state has been changed
	///@param[in] proxy the proxy string to access the stream object
	///@param[in] id	the value of ident.name to identify the stream instance in a string format
	///@param[in] prevSpeed the previous play speed of the stream
	///@param[in] currentSpeed the current play speed of the stream
	["cpp:const"] void OnSpeedChanged(string proxy, string id, float prevSpeed, float currentSpeed , Properties props );

	/// event fired when a stream exit
	///@param[in] proxy	   the proxy string to access the stream object
	///@param[in] id	   the value of ident.name to identify the stream instance in a string format
	///@param[in] exitCode the exist code of the stream
	///@param[in] reason   a description of the exist reason
	["cpp:const"] void OnExit(string proxy, string id, int exitCode, string reason);
	
	
	///another  stream exit event call ,and properties are added
	///@param[in] proxy	   the proxy string to access the stream object
	///@param[in] id	   the value of ident.name to identify the stream instance in a string format
	///@param[in] exitCode the exist code of the stream
	///@param[in] reason   a description of the exist reason
	///@param[in] props		property of current event
	["cpp:const"] void OnExit2(string proxy, string id, int exitCode, string reason ,Properties props );
};

// -----------------------------
// Service StreamService
// -----------------------------
/// represents basic stream service as a entry to access stream sessions
interface StreamService extends TianShanIce::BaseService, TianShanIce::ReplicaQuery
{
	/// create a stream instance by a given path
	///@param[in] pathTicket access to the path ticket that the new stream to use
	///@return	  access to a stream instance that is newly created
	Stream* createStream(Transport::PathTicket* pathTicket)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;
		
	///create playlist by serviceGroup ID and bandwith
	///@return the stream instance if success,false if fail
	///@param res the needed resource	
	Stream*	createStreamByResource(TianShanIce::SRM::ResourceMap res, TianShanIce::Properties props )
			throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;

	/// list the managed Streamer devices in this service
	///@return a collection of descriptors of the streamers
	StreamerDescriptors listStreamers()
		throws ServerError;

	/// get a network-wide unique id of this StreamService instance,
	/// normally this value could be read from the configuration of the StreamSerivce
	///@return a network-wide unique id
	///@note no white space is allowed in the id string
	["cpp:const"] string getNetId()
		throws ServerError;

	/// open stream proxy by stream stream session id
	/// @param[in] the stream session's id
	/// @return the instance proxy the opened stream
	Stream* openStream( string id )
		throws InvalidParameter,ServerError;
};

enum plInfoMask
{
	infoDVBCRESOURCE,
	infoPLAYPOSITION,
	infoSTREAMNPTPOS,
	infoSTREAMSOURCE
};

// -----------------------------
// class Playlist
// -----------------------------
/// extension of stream to represents a server-side playlist based streaming session
["freeze:write"] 
class Playlist extends Stream
{
	/// return the GUID string ID of the playlist
	["cpp:const", "freeze:read"] idempotent string getId();

	/// get the information about the playlist
	///@return true if success false if fail
	///@param mask which information do you want to get
	///refer to enum structure plInfoMask
	///@param var the return value returned from streamer	
	bool getInfo(int mask, out ValueMap var);
		
	
	// list operations
	// --------------------

	/// insert an media item into the list
	///@param[in] userCtrlNum  the user control number assigned for the this item, must be unique within the playlist
	///@param[in] newItemInfo  the item setup information structure about the new item
	///@param[in] whereUserCtrlNum the user control number to specify where the new item is being inserted before
	///@return confirm with the user control number if successful
	///@throw InvalidParameter will be thrown when 1) the user ctrlnum of the new item has already exists, 2) the "where" does't exist
	int insert(int userCtrlNum, PlaylistItemSetupInfo newItemInfo, int whereUserCtrlNum)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;

	/// append an media item into the end of the playlist
	///@param[in] userCtrlNum  the user control number assigned for the this item, must be unique within the playlist
	///@param[in] newItemInfo  the item setup information structure about the new item
	///@return confirm with the user control number if successful
	///@throw InvalidParameter will be thrown when 1) the user ctrlnum of the new item has already exists, 2) the "where" does't exist
	int pushBack(int userCtrlNum, PlaylistItemSetupInfo newItemInfo)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;

	/// get the full size of the list
	["cpp:const", "freeze:read"] int size();

	/// get the un-played item count
	///@return the item left from current playing to the end()
	["cpp:const", "freeze:read"] int left();

	/// test if the list is empty
	///@return  true if the playlist is empty
	["cpp:const", "freeze:read"] bool empty()
		throws ServerError,
			   InvalidStateOfArt;

	/// get the user control number of the current playing item
	///@return the user control number of the current playing item
	["cpp:const", "freeze:read"] int current()
		throws ServerError,
			   InvalidStateOfArt;

	///delete a specific item from the list
	///@param[in] whereUserCtrlNum the user control number to identify which item to be deleted
	void erase(int whereUserCtrlNum)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;

	///flush those item before the current playing item
	///@return the user control number of the first item in the list remained
	int flushExpired()
		throws ServerError,
			   InvalidStateOfArt;


	///clear the un-played items
	///@param[in] includeInitedNext true - teardown the next item even if it has already been loaded
	///@return true if successful
	bool clearPending(bool includeInitedNext)
		throws ServerError,
			   InvalidStateOfArt;

	// play list state checking
	// ----------------------------------

	/// test if the playlist has reached its end
	///@return true if it is the end of the playlist
	bool isCompleted()
		throws ServerError,
			   InvalidStateOfArt;

	/// get item sequence in the playlist, the sequence is represented via a list of user control number
	///@return a list of user control number
	["cpp:const", "freeze:read"] TianShanIce::IValues getSequence();

	/// search by userCtrlNum, default from the begin of the list
	///@param userCtrlNum the user control number to look for
	///@param from the user control number where to start finding, InvalidCtrlNum means from the begin
	///@return confirm the given user control number if successful, InvalidCtrlNum if not found
	int findItem(int userCtrlNum, int from)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;
	
	/// calculate the distance between from and to,return false if error
	/// @param[in] to the control number of destination
	/// @param[in] from the control number of start, InvalidCtrlNum means from current
	/// @param[out] dist the distance of from and to 
	bool distance(int to, int from, out int dist)
		throws InvalidParameter,
		       ServerError,
			   InvalidStateOfArt;


	// extended stream control
	// --------------------

	/// skip to a specific item in the list
	///@param[in] where the user control number to identify the item in the list
	///@param[in] bPlay true - play immediately even if the playlist's state is not stsStreaming
	///@return false if this operation failed
	["amd"] bool skipToItem(int where, bool bPlay)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;
	
	///Seek to a specify position
	///@param[in] UserCtrlNum user control number to identify which item to be seeked
	///@param[in] timeOffset time offse in milliseconds
	///@param[in] startPos the startposition: 0 - current position;  1 - begining of the stream;  2 - end of stream
	["amd"] bool seekToPosition(int UserCtrlNum , int timeOffset,int startPos)
		throws InvalidParameter,
			   ServerError,
			   InvalidStateOfArt;
	
	/// urn on or off the EdgeOfTail (EoT). EdgeOfTail was designed for real-time ingestion, it will protect the streaming
	/// not to reach the real-ingestion position.
	///@param[in] enable true if to turn on the EoT protection, false to disable the protection
	///@note By default, the playlist will always have EoT turned on if no enableEoT() has been invoked
	void enableEoT(bool enable)
		throws ServerError, InvalidStateOfArt;
		
	/// Play from a specified item and position
	///@param[in] UserCtrlNum user control number to identify which item to be seeked
	///@param[in] timeOffset time offse in milliseconds
	///@param[in] from the type of start position: 0 - current position;  1 - begining of the stream;  2 - end of stream
	///@param[in] newSpeed the new speed to apply on the stream session, the actual play speed will be rounded
	///           positive value means forward while negative means backforward, (-1, 1) means slow motion
	///           for the reason of a float value, abs(newSpeed) < 0.01 would be considered as 0.0, which mean keep the current speed
	///@return the stream information record after the operation is executed
	///@note newSpeed = 0.0 means no change on speed instead of PAUSE
	///@note from=0 and offset=0 means no change on location, continue with the current position
	["ami","amd"] StreamInfo playItem(int UserCtrlNum , int timeOffset, short from, float newSpeed, StrValues expectedProps)
			throws ServerError, InvalidStateOfArt,InvalidParameter;
	
	["cpp:const", "freeze:read"] PlaylistItemSetupInfoS getPlaylistItems();
};

// -----------------------------
// interface PlaylistEventSink
// -----------------------------
/// topic of playlist events, see @sa PlaylistEventSink
const string TopicOfPlaylist				= "TianShan/Event/Streamer/Playlist";

///@brief extension of stream events to cover playlist based events
interface PlaylistEventSink extends TianShanIce::Events::BaseEventSink
{
	/// event fired when the end of an item within a playlist has been stepped in execution, the resource bound on this
	/// stream session will be held till StreamEventSink::OnEndOfStream(  ).
	///@param[in] proxy  the proxy string to access the playlist object
	///@param[in] playlistId the playlist id
	///@param[in] currentUserCtrlNum the user's control number of the current item in the playlist after stepping
	///@param[in] prevUserCtrlNum the user's control number of the previous item in the playlist before stepping
	///                           invalid user's control number such as -1, -2 may be given when there is no previous item
	///@note if you wish to sink the end of an whole playlist, please use StreamEventSink::OnEndOfStream()
	///@sa   StreamEventSink::OnEndOfStream()
	["cpp:const"] void OnItemStepped(string proxy, string playlistId, int currentUserCtrlNum, int prevUserCtrlNum, Properties ItemProps);
};

// -----------------------------
// Service StreamSmith
// -----------------------------
/// extends basic StreameService to represent a server-side playlist based streamer service
interface StreamSmith extends StreamService
{
};

};
};

#endif // __ZQ_TianShanIce_Streamer_ICE__
