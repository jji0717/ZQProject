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
// Ident : $Id: TsSRM.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsSRM.ICE $
// 
// 9     2/23/16 10:34a Hui.shao
// 
// 8     5/12/14 11:47a Hui.shao
// 
// 7     7/01/13 3:07p Li.huang
// 
// 6     7/01/13 2:01p Bin.ren
// 
// 5     4/09/12 2:11p Hui.shao
// 
// 4     12/30/11 2:32p Hui.shao
// 
// 3     12/30/11 2:29p Li.huang
// 
// 2     4/20/11 3:20p Hui.shao
// added new resource type rtContentProvision
// 
// 41    10-02-09 17:31 Fei.huang
// - remove keyword "idempotent" for getId to be compatible with 1.8
// 
// 40    09-12-29 14:47 Hui.shao
// 
// 39    09-03-05 19:43 Hui.shao
// enriched the resource of physical channels
// 
// 38    09-03-02 17:31 Hui.shao
// 
// 37    08-08-14 14:30 Hui.shao
// merge from 1.7.10
// 
// 36    08-07-07 15:41 Hui.shao
// 
// 35    08-02-13 17:53 Hui.shao
// 
// 34    07-12-20 14:13 Hui.shao
// appended exception throwing
// 
// 33    07-12-07 14:28 Hui.shao
// 
// 32    07-06-21 15:02 Hui.shao
// 
// 31    07-06-20 14:30 Hui.shao
// added Session::setPrivateData2()
// 
// 30    07-03-28 16:52 Hui.shao
// added namespace Site
// 
// 30    07-03-28 16:25 Hui.shao
// added name space Site
// 
// 29    07-03-13 14:31 Hui.shao
// 
// 28    07-03-13 12:43 Hui.shao
// 
// 27    07-03-13 12:37 Hui.shao
// renamed namespace Weiwoo to SRM, AccreditPath to Transport
// 
// 26    07-03-13 12:00 Hui.shao
// 
// 23    07-01-05 17:52 Hui.shao
// 
// 22    07-01-05 16:35 Hongquan.zhang
// 
// 21    06-12-29 14:33 Hui.shao
// 
// 20    9/21/06 4:35p Hui.shao
// batch checkin on 20060921
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_SRM_ICE__
#define __ZQ_TianShanIce_SRM_ICE__

#include <TianShanIce.ICE>
#include <TsEvents.ICE>

module TianShanIce
{

// -----------------------------
// forward declarations
// -----------------------------
module Application
{
	class Purchase;
};

module Transport
{
	class PathTicket;
};

module Streamer
{
	class Stream;
};

// -----------------------------
// namespace SRM
// -----------------------------
// ///@image html logo_weiwoo.png "SRM" width=2cm
/// SRM represents the session management, resource descriptors and business management subsystem within the TianShan architecture
module SRM
{

/// The ResourceStatus is defined in DSM-CC. It represents the status of the negotiation for 
/// the resource with the SRM. 
enum ResourceStatus
{
	rsRequested,
	rsInProgress,
	rsAlternateAssigned,
	rsAssigned,
	rsFailed,
	rsUnprocessed,
	rsInvalid,
	rsReleased    
};

/// ResourceAttribute is defined in DSM-CC. It indicates the level of "requiredness" 
/// of the Resource. 
enum ResourceAttribute
{
	raMandatoryNonNegotiable,
	raMandatoryNegotiable,
	raNonMandatoryNonNegotiable,
	raNonMandatoryNegotiable     
};

/// resouce types
enum ResourceType
{
	// TianShan related resource type starts here

    /// the required resource identity of order in URI format, resource data contains fields:
	/// @li "uri" a collection of asset full URI strings such as "<proto>://<site>/<App>?order=1123"
	rtURI,

	/// the resource of storage, resource data contains fields:
	/// @li "NetworkId" the storage network ident string must be resolved by AccreditedPath to PathId for further use
	/// @li "Type" specify required storage types if necessary
	/// @li "Volume" specify the unique name of required volume in format of <NetworkId>/<VolumeOnTheContentStore>
	rtStorage,
	
	/// the resource of streamer, resource data contains fields:
	/// @li "NetworkId" the streamer network ident string must be resolved by AccreditedPath to PathId for further use
	/// @li "Type" specify required streamer types if necessary
	/// @li "sessionInterface" the URL to the stream server where can setup session to stream from
    rtStreamer,

	/// the resource of destination service group, resource data contains fields:
	/// @li "id" the integer id of the destination service group
	/// @li "routeNames" the string route names of the destination
	/// @li "deliveryId" the unique id that hint streamer to take as stream id, usually assigned by SRM when session provision
	rtServiceGroup,

	/// the resource to indicates the desire to allocate a quantity of bandwidth to provision a content for streaming
	/// @li "bandwidth" the long integer bandwidth in bps
    rtProvisionBandwidth,

    // DSMCC compatible resource definitions

    /// the resource of MPEG, resource data contains fields:
	/// @li "Id" the long id of MPEG program id
	/// @li "PmtPid" the int id of MPEG PMT PID. This field is reserved for future use
	/// @li "CaPid" the int id of MPEG CA PID. This field is reserved for future use
	/// @li "PidCount" the int count of MPEG PIDs. This field is reserved for future use
	rtMpegProgram,
	
	/// the resource to indicates the desire to allocate a quantity of bandwidth on a given transport stream
	/// @li "bandwidth" the long integer bandwidth in bps
	/// @li "tsid" the tsid of the transport stream
	/// @li "sessionURL" the URL to the already created session for the receiver to catch from
    rtTsDownstreamBandwidth,
	
	/// the resource to to convey IP address for stream pumping or service access
	/// @li "protocol" long constant, value equals to C such as UDP=>SOCK_DGRAM, TCP=>SOCK_STREAM
	/// @li "destAddr" string value(s) of the destination address such as "192.168.80.121"
	/// @li "destPort" integer of the destination port number
    rtIP,
	
	/// the resource to to convey NIC address for ARP stream pumping
	/// @li "srcMac"	long interger value(s) of source mac address
	/// @li "srcIP"		string value(s) of source IP address
	/// @li "srcPort"   interger value(s) of source port number
	/// @li "destMac"   long interger value(s) of destination mac address
	/// @li "destIP"    string value(s) of destination IP address
	/// @li "destPort"  interger value(s) of destination port number
	/// @li "natPenetrating" the first integer value is non-zero if need enable the support of NAT penetrating tech
	/// @li "pokeholeSession" string value(s) of the pokehole sessions, normally assigned by the StreamService after create the session
    rtEthernetInterface,
	
	/// Describes the channel used for tuning sessions. The channelId field is the QAM frequency in Khertz. Its direction field is "Downstream" 	
	/// @li "channelId" the long of channel id
	/// @li "edgeDeviceName" the name of the edge device
	/// @li "edgeDeviceIP" the input IP of the edge device, 1-to-1 sequentially map to edgeDeviceName items if they exist
	/// @li "edgeDeviceZone" the zone of the edge device
    rtPhysicalChannel,
	
	/// the resource to reserve to convey information to the set top about tuning information
	/// @li "transmissionSystem" the byte defines the type of system used to deliver the service
	///      0x00 - Unknown transmission system
	///      0x01 - SADVB transmission system
	///      0x02 - GI transmission system
	///      0x03-0xff reserved
	/// @li "modulationFormat" the byte to sepcify to modulation used for this channel.
	///      0x00 - Unknown modulation format
	///      0x01-0x05 - reserved
	///      0x06 - QAM 16 modulation format
	///      0x07 - QAM 32 modulation format
	///      0x08 - QAM 64 modulation format
	///      0x09-0x0b - reserved
	///      0x0c - QAM 128 modulation format
	///      0x0d-0x0f - reserved
	///      0x10 - QAM 256 modulation format
	///      0x11-0xff - reserved
	/// @li "symbolRate" the integer to sepcify to the symbol rate
	/// @li "interleaveDepth" the byte to sepcify the used interleaving, reserved
	/// @li "modulationMode" the byte to sepcify the used modulation mode, reserved for now
	///      0x00 - no modulation
	///      0x01-0x08 - reserved
	///      0x09 - QAM 4 modulation mode
	///      0x0a-0xff - reserved
	/// @li "FEC" the byte to sepcify the type of forward error correction
	///      0x00 - FEC transmission system
	///      0x01 - FEC DAVIC
	///      0x02-0xff - reserved
    rtAtscModulationMode,

	//NOTE: the following resource descriptors are not used !!

	/// the resource to reserve to allow for a single service server to serve Sessions in multiple headends
	/// @li "id" the string value of headend id(s)
	/// @li "flag" long integer as a 64 bit flags of the headend attributes reserved for future
	/// @li "tsid" the tsid of the headend
    rtHeadendId,

    /// the resource to reserve for client conditional access
    rtClientConditionalAccess,

    /// the resource to reserve for server conditional access
	rtServerConditionalAccess,
	
	/// the required resource identity of order in URI format, resource data contains fields:
	/// @li "sourceUri"	   StrValues contains the uri to fetch the source media content
	/// @li "expectedPassiveProto"	   exclusive with sourceUri, StrValues contains wished transfer protocols (from highest priority to lowest) to push the source media in
	/// @li "sourceType"   StrValues contains type of the source media content
	/// @li "scheduledStart" StrValues[0] indicates the scheduled start time in the format of ISO8601, 0 or eariler than now to start immediately
	/// @li "scheduledEnd" StrValues[0] indicates the scheduled end time in the format of ISO8601, 0 to stop per source feed
	/// @li "bandwidth"    the bitrate to download the source in bps
	/// @li "tricks"       FValues contains the trick speeds
	rtContentProvision,

	/// In a cached streaming architecture, the upstream defines the resource where the content can be cached from
	/// @li "bandwidth" the long integer bandwidth in bps, can be a range [min, max] to allow streamer to utilize according to its capacity and need
	/// @li "sessionInterface" the URL to the uplink server where can setup session to cache the content from
	/// @li "sessionURL" the URL to the already created session on uplink server for edge to cache the content from
    rtTsUpstreamBandwidth,

};

// -----------------------------
// exception InvalidResource
// -----------------------------
/// exception throw when an invalid resource is being procedued
exception InvalidResource extends TianShanIce::BaseException
{
};

// -----------------------------
// exception OutOfResource
// -----------------------------
/// exception throw when the system is out of resource
exception OutOfResource extends TianShanIce::BaseException
{
};

// -----------------------------
// struct Resource
// -----------------------------
/// The Resource is the basic class that is allocated in response to the addResource operation. 
struct Resource
{
	ResourceStatus		status;
	ResourceAttribute	attr;

	TianShanIce::ValueMap resourceData;
};

/// a collection of resource
dictionary <ResourceType, Resource> ResourceMap;

// -----------------------------
// class Session
// -----------------------------
///@brief The Session is the Weiwoo representation of the server-to-server session
/// The Session object is created by the SessionManager. It is designed in response to a Server
/// Session Setup Indication as of today. 
///@image html sess_states.JPG "Statediagram of session"
["freeze:write"] 
class Session
{
	ResourceMap				resources;
	TianShanIce::ValueMap	privdata;
	TianShanIce::State		state;
	string					sessId;
	string					purchasePrxStr;
	Transport::PathTicket* ticket;
	string					streamPrxStr;
	
	/// The expiration of this session in msec and GMT, can be renew-ed thru renew().
	/// It always starts with 0 when session is created until renew() is invoked. Once the expiration is
	/// a non-zero, the session becomes volatilizable. Whenever the expiration is met, the session will
	/// enter the state of TianShanIce::stOutOfService
	long					expiration;

	/// get the session id
	///@ return the session id in a compact string format
    ///@ remove "idempotent" keyword to be compatible with 1.8
	["cpp:const", "freeze:read"] string getId(); 

	/// get the session state
	///@ return the session id in a compact string format
	["cpp:const", "freeze:read"] TianShanIce::State getState(); 

	/// access to the owner who created and maintainance the session
	["cpp:const", "freeze:read"] Object* getOwner(); 

	///	destroy the session
	void destroy()
		throws ServerError;

	/// setPrivateData allows an object to add private data to the session response.
	///@param key the keyname of the private data
	///@param val the value of the private data
	void setPrivateData(string key, TianShanIce::Variant val);

	/// getPrivateData allows a proxy object to get private data of the session.
	///@return the private data of the session
	["cpp:const", "freeze:read"] TianShanIce::ValueMap getPrivateData();

	/// setPrivateData2 to set a collection of private data.
	///@param  privateData the private data map to set
	///@return the number of private data that have been successfully set
	///@note   it will be overwritten if the same private data key exists in the session.
	///@note   all the privateData keys in the session will be kept untrouched if they do not exist in the param privateData
	int setPrivateData2(TianShanIce::ValueMap privateData);

	/// addResource
    /// The addResource operation allows the guts of a Resource to be sent to a Session. 
	/// The Session then instantiates a Resource object and adds it to its list. When the 
	/// negotiateResources operation is invoked, these will be the Resources that are negotiated.
    /// Each Resource is assigned a unique number by the Session.
	///@param type the enum value of ResouceType to identify which resource to be added
	///@param attr the enum value of ResourceAttribute to identify the attribute of the resource
	///@param resData a collection of resource fields, see the bullet of each ResourceType
    long addResource (ResourceType type, 
					  ResourceAttribute attr,
					  ValueMap	resData)
	throws
      InvalidResource,
	  NotSupported,
      InvalidParameter;

	/// addResourceEx
    /// The addResourceEx operation allows the guts of a Resource to be sent to a Session. 
	/// The Session then instantiates a Resource object and adds it to its list. When the 
	/// negotiateResources operation is invoked, these will be the Resources that are negotiated.
    /// Each Resource is assigned a unique number by the Session.
	///@param type the enum value of ResouceType to identify which resource to be added
	///@param res the prepared Resource structure
    long addResourceEx(ResourceType type, Resource res)
	throws
      InvalidResource,
	  NotSupported,
      InvalidParameter;

	/// get all the resource collection bound in the session
	///@return the resource collection
	["cpp:const", "freeze:read"] ResourceMap getReources();

    /// The removeResource operation allows a previously defined Resource to be retired.
	///@param type the enum value of ResouceType to identify which resource to be removed
    void removeResource (ResourceType type);

    /// perform the provision for the session. At this step, the session will be associated with
	/// a purchase of application, if necessary resouce requirement or private data have not been
	/// collected before provision() is invoked, this operation will be rejected
	///@note resource rtURI is a must to associate the purchase, all other necessary resource or
	///      runtime parameters are up to the application logic
	///@note the destination state of the session will be turned to stProvisioned if succeeded
	///@throw InvalidStateOfArt if try to provsion from a illeagal state
	///@throw InvalidResource if a required resource has not been collected piror to this operation
	///@throw InvalidParameter if a required parameter has not been collected piror to this operation
	///@throw InvalidStateOfArt if the current state of session does not perform this operation
    ["amd"] void provision()
	throws
      InvalidResource,
	  InvalidStateOfArt,
      InvalidParameter,
      NotSupported,
	  ServerError;

    /// prepare the session to be ready for serving client. At this step, the session will
	///@li certain all the negociateable resource requirement and perform resource selection.
	///@li create a stream session on a picked StreamService and apply the selected Transport path
	///@li ask application to render the purchase on to the stream session
	///@note the destination state of the session will be turned to stInService if succeeded
	///@note error occurs after any of certain resource allocation will lead the session to a 
	///      stOutOfService state
	///@note no resource requirement collection thru addResource() will be allowed after this
	///@throw InvalidResource if a required resource has not been collected piror to this operation
	///@throw InvalidStateOfArt if try to serve from a illeagal state
	///@throw InvalidParameter if a required parameter has not been collected piror to this operation
	///@throw InvalidStateOfArt if the current state of session does not perform this operation
	///@throw OutOfResource if the system is running out of resource to serve this session
    ["amd"] void serve()
	throws
	  InvalidStateOfArt,
      InvalidResource,
	  OutOfResource,
      InvalidParameter,
      NotSupported,
	  ServerError;

	/// The negotiateResources operation is intended to initiate a DSM-CC Session resource 
    /// negotiation. If the Session is being established, only a single round of negotiation 
    /// is allowed. After the Session is established other rounds of negotiation are allowed 
    /// by the spec, but not by the present implementation of the SRM (we believe.) We may need 
    /// a SessionState variable to indicate where we are in the establishment process.
    ///
	/// The adminState is the intended state if no exceptions are raised during this operation. 
    /// If the operation does raise exceptions, the adminState will remain as it was previous to 
    /// the negotiateResources operation.
	///@param state the goal state of this negotiation
	///@throw InvalidStateOfArt if try to negotiate resources from a illeagal state
	void negotiateResources (TianShanIce::State state)
	throws
	  InvalidStateOfArt,
      InvalidResource,
	  OutOfResource,
      InvalidParameter,
      NotSupported,
	  ServerError;

	/// renew the session's lifecycle if this session is volatilizable
	/// @param TTL time-to-live for this session in msec from now
	///@throw InvalidStateOfArt if try to renew from a illeagal state
	void renew (long TTL);

	///	access to the attached purchase instance that also is associated with business logic
	///@return the purchanse instance
	///@note only supported in stProvisioned and stInService
	///@throw InvalidStateOfArt if try to get the access to the purchase when it isn't at a Provisioned or InService state
	["cpp:const", "freeze:read"] Application::Purchase* getPurchase()
	throws
      NotSupported,
      ServerError;

	/// get the access to the stream instance
	///@return the acces to the stream instance
	///@note only supported in stInService
	///@throw InvalidStateOfArt if try to get the access to the purchase when it isn't at an InService state
	["cpp:const", "freeze:read"] Streamer::Stream* getStream()
	throws
      NotSupported,
      ServerError;
};

sequence <Session* > Sessions; ///< a collection of session instances

// -----------------------------
// Service SessionManager
// -----------------------------
/// represents a manager of the sessions within SRM
interface SessionManager extends TianShanIce::BaseService
{
	/// create a session with a specific asset
	///@param[in] assetUri	the asset URI required in the session
	///@return    a session instance, NULL if failed
	///@note	  resouce requirement must be specified by calling Session::addResource()
	Session* createSession(Resource assetUri)
	throws ServerError;

	/// open an existing session with a session id
	///@param[in] sessId	the session id
	///@return    a session instance, NULL if failed
	Session* openSession(string sessId);

	/// create a session by a "Server Session Setup Indication"
	///@param[in] assetUri	the required asset uri in the session
	///@param[in] resSSSI the resources recommanded in the original "Server Session Setup Indication"
	///@return    a session instance, NULL if failed
	Session* createSessionBySSSI(Resource assetUri, ResourceMap resSSSI)
	throws ServerError;
};

// -----------------------------
// interface SessionEventSink
// -----------------------------
/// topic of session events, see @sa SessionEventSink
const string TopicOfSession				= "TianShan/Event/SRM/Session";

///@brief interface to subscribe the session events fired in session lifecycles
/// with topic TopicOfSession
///@note all the session events are one-way
interface SessionEventSink extends TianShanIce::Events::BaseEventSink
{
	/// event fired when a new session is created
	///@param[in] sessId the session id of the newly created session
	///@param[in] proxy the proxy string to access this session
	void OnNewSession(string sessId, string proxy);

	/// event fired when a session is destroyed
	///@param[in] sessId the session id of the session
	void OnDestroySession(string sessId);

	/// event fired when a session's state has been changed
	///@param[in] sessId the session id of the newly created session
	///@param[in] proxy the proxy string to access this session
	///@param[in] prevState the previous state of the session
	///@param[in] currentState the current state of the session
	void OnStateChanged(string sessId, string proxy, State prevState, State currentState);
};

};
};

#endif // __ZQ_TianShanIce_SRM_ICE__
