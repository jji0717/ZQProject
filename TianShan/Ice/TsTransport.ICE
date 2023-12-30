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
// Ident : $Id: TsAccreditedPath.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsTransport.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 51    10-06-18 15:27 Hui.shao
// 
// 50    08-08-05 13:37 Hui.shao
// 
// 50    08-08-01 14:22 Hui.shao
// added penalty on ticket
// 
// 49    08-07-07 15:50 Hui.shao
// 
// 48    08-01-03 15:47 Hui.shao
// 
// 47    07-03-22 17:26 Hui.shao
// 
// 46    07-03-21 16:05 Hui.shao
// added entry to commit path ticket
// 
// 45    07-03-13 16:35 Hui.shao
// 
// 44    07-03-13 15:43 Hui.shao
// 
// 43    07-03-13 12:37 Hui.shao
// renamed namespace Weiwoo to SRM, AccreditPath to Transport
// 
// 40    07-01-05 12:58 Hui.shao
// 
// 39    06-12-20 16:41 Hui.shao
// 
// 38    11/06/06 6:11p Hui.shao
// 
// 37    9/21/06 4:35p Hui.shao
// batch checkin on 20060921
// ===========================================================================
// build steps:
//  $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice  
// outputs:
//	$(InputName).h
//	$(InputName).cpp

#ifndef __ZQ_TsAccreditedPath_ICE__
#define __ZQ_TsAccreditedPath_ICE__

#include "TianShanIce.ICE"
#include "TsSRM.ICE"

#include <Ice/Identity.ice>

module TianShanIce
{

// -----------------------------
// forward declarations
// -----------------------------
module SRM
{
	class Session;
};


/// Transport represents an abstract layer of network path allocation
module Transport
{

// -----------------------------
// exception ExpiredTicket
// -----------------------------
/// exception thrown when the operated ticket has already out of life
exception ExpiredTicket extends TianShanIce::BaseException
{
};

// -----------------------------
// struct ServiceGroup
// -----------------------------
/// describes a service group information in AccreditedPath
struct ServiceGroup
{
	int		id;		///< integer id of a group of servic consumers
	string	desc;   ///< description string
	string	type;	///< reserved, must be "ServiceGroup"
};

// -----------------------------
// struct Storage
// -----------------------------
/// describes a storage information in AccreditedPath
struct Storage
{
	string  netId; ///< equals to the return value of ContentStore::getNetId()
	string	type;
	string  desc; ///< description string
	string  ifep; ///< the string value of "<interface>:<endpoint>" to access the ContentStore
	TianShanIce::ValueMap privateData;
};

// -----------------------------
// struct Streamer
// -----------------------------
/// describes a streamer information in AccreditedPath
struct Streamer
{
	string  netId;	///< a combination of StreamService network id and deviceId of StreamerDescriptor
					///< in the format of "<StreamService netId>/<deviceId>"
	string	type;	///< equals to the type of StreamerDescriptor
	string  desc;   ///< description string
	string  ifep;	///< the string value of "<interface>:<endpoint>" to access the StreamService
	TianShanIce::ValueMap privateData;
};

// -----------------------------
// class StorageLink
// -----------------------------
/// describes a link between a storage and a streamer in AccreditedPath
["freeze:write"] 
class StorageLink
{
	Ice::Identity ident;

	string		storageId;
	string		streamerId;

	string	 type; // transport type, i.e. ftp protocol
	TianShanIce::ValueMap privateData;

	///	get the identity of this link
	///@return the identity
	["cpp:const", "freeze:read"] idempotent Ice::Identity getIdent();
	
	/// get the type of this link
	///@return the type of the link
	["cpp:const", "freeze:read"] idempotent string getType();

	///@return the storage id bound
	["cpp:const", "freeze:read"] idempotent string getStorageId();

	///@return the streamer id bound
	["cpp:const", "freeze:read"] idempotent string getStreamerId();

	/// get the storage information bound on this link
	///@return the storage info structure
	["cpp:const", "freeze:read"] Storage  getStorageInfo();

	/// get the streamer information bound on this link
	///@return the streamer info structure
	["cpp:const", "freeze:read"] Streamer getStreamerInfo();

	/// set a private data on this link
	///@return true if succeed
	bool setPrivateData(string key, TianShanIce::Variant val);

	/// get the private data map bound on this link
	///@return a map of private data bound
	["cpp:const", "freeze:read"] TianShanIce::ValueMap getPrivateData();

    /// unlink
	void destroy() throws ServerError;
};

// -----------------------------
// class StreamLink
// -----------------------------
/// describes a link between a service group and a streamer in AccreditedPath
["freeze:write"] 
class StreamLink
{
	Ice::Identity ident;

	string		streamerId;
	int			servicegroupId;

	string	 type; // transport type, i.e. IP or DVBC connection
	TianShanIce::ValueMap privateData;

	/// get the Identity of the StreamLink
	///@return the identity
	["cpp:const", "freeze:read"] idempotent Ice::Identity getIdent();

	/// get the type of the StreamLink
	///@return the type of the link
	["cpp:const", "freeze:read"] idempotent string getType();

	///@return the steamer id bound
	["cpp:const", "freeze:read"] idempotent string getStreamerId();

	///@return the service group id bound
	["cpp:const", "freeze:read"] idempotent int getServiceGroupId();

	/// get the streamer information bound on this link
	///@return the streamer info structure
	["cpp:const", "freeze:read"] Streamer getStreamerInfo();

	/// get the service group information bound on this link
	///@return the service group info structure
	["cpp:const", "freeze:read"] ServiceGroup getServiceGroupInfo();

	/// set a private data on this link
	///@return true if succeed
	bool setPrivateData(string key, TianShanIce::Variant val);

	/// get the private data map bound on this link
	///@return a map of private data bound
	["cpp:const", "freeze:read"] TianShanIce::ValueMap getPrivateData();

    /// unlink
    void destroy() throws ServerError;

};

const int MaxCost					= 10000;
const int OutOfServiceCost			= 10001;
const string PathTicketPDPrefix		= "PathTicket.";

// -----------------------------
// class PathTicket
// -----------------------------
/// represents a ticket on a reserved delivery path in the network
["freeze:write"] 
class PathTicket
{
	Ice::Identity	ident;

	Ice::Identity	storageLinkIden;
	Ice::Identity	streamLinkIden;

	long			stampAllocated;	   ///< the timestamp when this ticket is allocated during cost evaluation
	long			stampNarrowed;     ///< the timestamp when this ticket is negociated to narrow the resources bound
	long			stampComitted;     ///< the timestamp when this ticket is committed with narrowed resources
	long			expiration;		   ///< the expiration of this ticket, the AccreditedPath will detroy the ticket and free the allocation once it is met

	int				costWhenAllocated; ///< a value for evaluation in the range of 0-MaxCost

	TianShanIce::SRM::ResourceMap resources;
	TianShanIce::ValueMap privateData;

	TianShanIce::State state;

	/// get the identity of this ticket
	///@return the identity of this ticket
	["cpp:const", "freeze:read"] idempotent Ice::Identity getIdent();

	/// get the state of this ticket
	///@return the state of this ticket
	["cpp:const", "freeze:read"] TianShanIce::State	getState();

	/// get all the resource collection bound on this ticket
	///@return the resource collection
	["cpp:const", "freeze:read"] TianShanIce::SRM::ResourceMap getResources();

	/// get the private data map bound on this ticket
	///@return a map of private data bound
	["cpp:const", "freeze:read"] TianShanIce::ValueMap getPrivateData();

	/// renew the lease term of this ticket, the new lease term will always overwrite any existing remaining
	/// lease term will always be counted from the moment this method is called
	///@param leaseTerm the new lease term in msec, range 0 - 21600000 (up to 6 hour)
	///@note  InvalidParameter will be thrown if not in the range of 0- 6hr
	void	renew(int  leaseTerm) throws InvalidParameter;

	/// access to the storage link bound on this reserved path
	["cpp:const", "freeze:read"] StorageLink*	getStorageLink();

	/// access to the stream link bound on this reserved path
	["cpp:const", "freeze:read"] StreamLink*		getStreamLink();

	["cpp:const", "freeze:read"] int				getCost();

	/// get the remaining lease time in msec
	["cpp:const", "freeze:read"] int				getLeaseLeft();

	/// narrow the ticket to a specific path
	///@param[in] sess access to the session that this ticket will be narrowed by
	///@note context in the session may be also updated
	["amd"] void		narrow(SRM::Session* sess) throws InvalidStateOfArt;

	/// commit to use the the ticket after a stream instance has been successfully created
	///@param[in] sess access to the session that this ticket is used for
	///@note context in the session may be also updated
	["amd"] void		commit(SRM::Session* sess) throws InvalidStateOfArt, ServerError;

    void		destroy() throws ServerError;

	/// set the penalty on this ticket, if the session manager failed to create a stream session with this ticket,
	/// it will set a penalty on the ticket, and wish the path manager considers such a failure for the next session
	/// setup. PHO may take this penalty value to yield using the same resource for a timeout window
	///@param[in] penalty the penalty value to set, 0 - means clear the penalty
	void setPenalty(byte penalty);
};


/// a collection of service group info
sequence <ServiceGroup> ServiceGroups;

/// a collection of storage info
sequence <Storage> Storages;

/// a collection of streamer info
sequence <Streamer> Streamers;

/// a collection of storage link
sequence <StorageLink*> StorageLinks;

/// a collection of stream link
sequence <StreamLink*>  StreamLinks;

/// a collection of allocated tickets
sequence <PathTicket*> PathTickets;

// -----------------------------
// Service PathManager
// -----------------------------
/// represent the service to reserve stream path in the network
interface PathManager extends TianShanIce::BaseService
{
	/// @name Path schema accesses
	/// Access to the supported link type and their schema for configuration purposes
	///@{

	/// list all currently supported StorageLink types
	///@return a string list of all currently supported StorageLink types
	["cpp:const"] StrValues listSupportedStorageLinkTypes() throws InvalidParameter;

	/// list all currently supported StreamLink types
	///@return a string list of all currently supported StreamLink types
	["cpp:const"] StrValues listSupportedStreamLinkTypes() throws InvalidParameter;

	/// get the private data schema of a specific StorageLink type
	///@param type	the string type of StorageLink
	///@return the schema of the acceptable private data bound with this type of StorageLink
	["cpp:const"] PDSchema getStorageLinkSchema(string type) throws InvalidParameter;

	/// get the private data schema of a specific StreamLink type
	///@param type	the string type of StreamLink
	///@return the schema of the acceptable private data bound with this type of StreamLink
	["cpp:const"] PDSchema getStreamLinkSchema(string type) throws InvalidParameter;

	///@}

	/// @name Record accesses
	/// Access to the entities in the path management
	///@{

	/// list all defined service groups
	ServiceGroups listServiceGroups();
	
	/// list all defined storages
	Storages listStorages();

	/// get all the private values of the storage
	TianShanIce::ValueMap getStoragePrivateData(string id);
	
	/// list all defined streamers
	Streamers listStreamers();

	/// get all the private values of the streamer
	TianShanIce::ValueMap getStreamerPrivateData(string id);

	/// list the storage links by specified storage
	///@param[in] storageId the network id of the storage
	///@return a collection of the matched StorageLink
	StorageLinks listStorageLinksByStorage(string storageId);

	/// list the storage links by specified streamer
	///@param[in] streamerId the network id of the streamer
	///@return a collection of the matched StorageLink
	StorageLinks listStorageLinksByStreamer(string streamerId);

	/// list the stream links by specified storage
	///@param[in] streamerId the network id of the streamer
	///@return a collection of the matched StreamLink
	StreamLinks  listStreamLinksByStreamer(string streamerId);

	/// list the stream links by specified service group
	///@param[in] servicegroup the id of the destination service group
	///@return a collection of the matched StreamLink
	StreamLinks  listStreamLinksByServiceGroup(int servicegroup);

	///@}

	///reserve the potential paths by Session at the state of Provision
	///@param[in] maxCost		a acceptable cost to level the paths, 0 means receive all the paths with cost 0-10000
	///@param[in] maxTickets    the max number of tickets can be received in the returning ticket collection, 0 means receive all the possible paths
	///@param[in] hintLease		a value of leaseTerm in msec to hint the service how long should reserve the path if it is possible.
	///						0 - means take the default lease of the service
	///@param[in] sess			access to the session that this allocation is kicked off for
	///@return				a collection of reserved ticket.
	///@note the ticket will be detroyed by the service if its expiration is met. the ticket user must renew the lease if he want to utlize
	///		 the path. Otherwise, it is recommanded to destroy the un-used tickets to leave the system at a high ablity level
	///@note the sess maybe updated during allocation, but the reserved path tickets will not add into the session
	///      until you call Session::addResource(rtPathTicket)
	["amd"] PathTickets reservePaths(int maxCost, int maxTickets, int hintLease, SRM::Session* sess)
	   throws InvalidParameter, ServerError, NotSupported;
};

};
};

#endif // __ZQ_TsAccreditedPath_ICE__
