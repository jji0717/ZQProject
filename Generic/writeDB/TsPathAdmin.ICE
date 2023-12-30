// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: TsAdpIceEx.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/writeDB/TsPathAdmin.ICE $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 2     07-04-26 15:25 Guan.han
// 
// 21    07-04-24 16:42 Hongquan.zhang
// 
// 20    07-03-13 17:25 Hongquan.zhang
// 
// 19    07-03-13 17:17 Yonghua.deng
// 
// 18    06-12-25 12:23 Hongquan.zhang
// 
// 17    9/21/06 4:34p Hui.shao
// batch checkin on 20060921
// ===========================================================================
// build steps:
//  $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::AccreditedPath::ServiceGroupDict,long,TianShanIce::AccreditedPath::ServiceGroup" --dict-index "TianShanIce::AccreditedPath::ServiceGroupDict,id" ServiceGroupDict --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::AccreditedPath::StorageDict,string,TianShanIce::AccreditedPath::Storage" --dict-index "TianShanIce::AccreditedPath::StorageDict,netId,case-insensitive" StorageDict --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::AccreditedPath::StreamerDict,string,TianShanIce::AccreditedPath::Streamer" --dict-index "TianShanIce::AccreditedPath::StreamerDict,netId,case-insensitive" StreamerDict --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::AccreditedPath::StorageToStorageLink,TianShanIce::AccreditedPath::StorageLink,storageId" StorageToStorageLink --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::AccreditedPath::StreamerToStorageLink,TianShanIce::AccreditedPath::StorageLink,streamerId" StreamerToStorageLink --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::AccreditedPath::StreamerToStreamLink,TianShanIce::AccreditedPath::StreamLink,streamerId" StreamerToStreamLink --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::AccreditedPath::SvcGrpToStreamLink,TianShanIce::AccreditedPath::StreamLink,servicegroupId" SvcGrpToStreamLink --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::AccreditedPath::StorageLinkToTicket,TianShanIce::AccreditedPath::PathTicket,storageLinkIden" StorageLinkToTicket --output-dir .. ../$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::AccreditedPath::StreamLinkToTicket,TianShanIce::AccreditedPath::PathTicket,streamLinkIden" StreamLinkToTicket --output-dir .. ../$(InputName).ice
// outputs:
//	../$(InputName).h
//	../$(InputName).cpp
//	../ServiceGroupDict.cpp
//	../ServiceGroupDict.h
//	../StorageDict.cpp
//	../StorageDict.h
//	../StreamerDict.cpp
//	../StreamerDict.h
//	../StorageLinkIndex.cpp
//	../StorageLinkIndex.h
//	../StreamLinkIndex.cpp
//	../StreamLinkIndex.h
//	../ADPAllocIndex.cpp
//	../ADPAllocIndex.h

#ifndef __ZQ_TsAccreditedPathAdmin_ICE__
#define __ZQ_TsAccreditedPathAdmin_ICE__

#include "TsTransport.ICE"

module TianShanIce
{

module Transport
{

// -----------------------------
// class StorageLinkEx
// -----------------------------
/// extension of StorageLink for some internal access
class StorageLinkEx extends StorageLink
{
	int revision; ///< revision string for configuration validation

	/// get the revision string
	///@return the revision string
	nonmutating int getRevision();

	/// update the revision string
	///@param[in] newRevision the new revision string to set
	void updateRevision(int newRevision);

	/// update all private data on this link
	///@param[in] newValues the complete new private values
	///@return true if succeed
	bool updatePrivateData(TianShanIce::ValueMap newValues) throws InvalidParameter;
};

// -----------------------------
// class StreamLinkEx
// -----------------------------
/// extension of StreamLink for some internal access
class StreamLinkEx extends StreamLink
{
	int revision; ///< revision string for configuration validation

	/// get the revision string
	///@return the revision string
	nonmutating int getRevision();

	/// update the revision string
	///@param[in] newRevision the new revision string to set
	void updateRevision(int newRevision);

	/// update all private data on this link
	///@param[in] newValues the complete new private values
	///@return true if succeed
	bool updatePrivateData(TianShanIce::ValueMap newValues) throws InvalidParameter;
};

//---------------------------
//calss PathTicketEx
//---------------------------
//class PathTicketEx extends PathTicket
//{
//	///set ticket private data
//	///@return true if success false if fail
//	bool	setPrivateData(string key , TianShanIce::Variant val);
//};



// -----------------------------
// Service PathAdmin
// -----------------------------
/// extension of PathManager to admin the path configuration and allocated path tickets
interface PathAdmin extends PathManager
{
	///add/update a service group
	///@param id	the id of service group about to update
	///@param desc	a description string of te service group
	///@return true if succeed
	bool updateServiceGroup(int id, string desc)
	   throws InvalidParameter, ServerError, NotSupported;

	///remove a service group description
	///@param id	the id of service group about to delete
	///@return true if succeed
	///@note all the links to this service group will also be destroyed
	bool removeServiceGroup(int id)
	   throws InvalidParameter, ServerError, NotSupported;

	/// add or update an existing storage
	///@param netId		the network id of storage about to update
	///@param type		the type of storage
	///@param endpoit	endpoint to access the storage
	///@param desc		a description string of the storage
	///@return true if succeed
	bool updateStorage(string netId, string type, string endpoint, string desc)
	   throws InvalidParameter, ServerError, NotSupported;

	/// remove a storage definition
	///@param netId		the network id of storage about to delete
	///@return true if succeed
	///@note all the links to this service group will also be destroyed
	bool removeStorage(string netId)
	   throws InvalidParameter, ServerError, NotSupported;

	/// set a private data of the storage
	///@param netId	the network id of storage about to set
	///@param key	the string key name of the private data
	///@param value	value of the private data
	///@return true if succeed
	bool setStoragePrivateData(string netId, string key, TianShanIce::Variant val)
	   throws InvalidParameter, ServerError, NotSupported;

	/// add or update an existing streamer
	///@param netId		the network id of streamer about to update
	///@param type		the type of streamer
	///@param endpoit	endpoint to access the streamer
	///@param desc		a description string of the streamer
	///@return true if succeed
	bool updateStreamer(string netId, string type, string endpoint, string desc)
	   throws InvalidParameter, ServerError, NotSupported;

	/// remove a streamer definition
	///@param netId		the network id of streamer about to delete
	///@return true if succeed
	///@note all the links to this service group will also be destroyed
	bool removeStreamer(string netId)
	   throws InvalidParameter, ServerError, NotSupported;

	/// set a private data of the streamer
	///@param netId	the network id of streamer about to set
	///@param key	the string key name of the private data
	///@param value	value of the private data
	///@return true if succeed
	bool setStreamerPrivateData(string netId, string key, TianShanIce::Variant value)
	   throws InvalidParameter, ServerError, NotSupported;

	/// establish a link from a storage to a streamer
	///@param storageId the storage network id to link
	///@param streamerId the streamer network id to link
	///@param type the type of this link
	///@param linkPD the private data bound on this link
	///@return the access to the newly created link
	StorageLink* linkStorage(string storageId, string streamerId, string type, TianShanIce::ValueMap linkPD)
	   throws InvalidParameter, ServerError, NotSupported;

	/// establish a link from a service group to a streamer
	///@param serviceGroupId the service group id to link
	///@param streamerId the streamer network id to link
	///@param type the type of this link
	///@param maxbandwidth the max bandwidth bound on this link
	///@return the access to the newly created link
	StreamLink*  linkStreamer(int serviceGroupId, string streamerId, string type, TianShanIce::ValueMap linkPD)
	   throws InvalidParameter, ServerError, NotSupported;

	// about the allocated paths

	/// list all the tickets that are currently allocated
	///@return a collection of all active tickets
	PathTickets listTickets()
	   throws ServerError, NotSupported;

	/// find the potential paths with given storages and the service group, then reserve them with a short lease term
	///@param[in] storages		a collection of the id of the source storages, Variant(vtInts, Ints)
	///@param[in] serviceGroup  the destination source group id
	///@param[in] bandwidth		the needed bandwidth to verify then reserve, 0 bandwidth is not allowed
	///@param[in] maxCost		a acceptable cost to level the paths, 0 means receive all the paths with cost 0-10000
	///@param[in] maxTickets    the max number of tickets can be received in the returning ticket collection, 0 means receive all the possible paths
	///@param[in] hintLease		a value of leaseTerm in msec to hint the service how long should reserve the path if it is possible.
	///						0 - means take the default lease of the service
	///@param[in] sess  		access to the Weiwoo session object for the allocation
	///@return				a collection of reserved ticket.
	///@note the ticket will be detroyed by the service if its expiration is met. the ticket user must renew the lease if he want to utlize
	///		 the path. Otherwise, it is recommanded to destroy the un-used tickets to leave the system at a high ablity level
	///						0 - means take the default lease of the service
	PathTickets reservePathsByStorage(TianShanIce::Variant storages, int serviceGroup,
									long bandwidth,	int maxCost, int maxTickets, int hintLease,
									SRM::Session* sess)
	   throws InvalidParameter, ServerError, NotSupported;
		
	/// find the potential paths with given streamer and the service group, then reserve them with a short lease term
	///@param[in] sourceStreamers	a collection of the id of the exection streamers, Variant(vtInts, Ints)
	///@param[in] serviceGroup  the destination source group id
	///@param[in] bandwidth		the needed bandwidth to verify then reserve, 0 bandwidth is not allowed
	///@param[in] maxCost		a acceptable cost to level the paths, 0 means receive all the paths with cost 0-10000
	///@param[in] maxTickets    the max number of tickets can be received in the returning ticket collection, 0 means receive all the possible paths
	///@param[in] hintLease		a value of leaseTerm in msec to hint the service how long should reserve the path if it is possible.
	///						0 - means take the default lease of the service
	///@param[in] sess  		access to the Weiwoo session object for the allocation
	///@return				a collection of reserved ticket.
	///@note the ticket will be detroyed by the service if its expiration is met. the ticket user must renew the lease if he want to utlize
	///		 the path. Otherwise, it is recommanded to destroy the un-used tickets to leave the system at a high ablity level
	///						0 - means take the default lease of the service
	PathTickets reservePathsByStreamer(TianShanIce::Variant sourceStreamers, int serviceGroup,
									long bandwidth,	int maxCost, int maxTickets, int hintLease,
									SRM::Session* sess)
	   throws InvalidParameter, ServerError, NotSupported;

	/// the main routin, find the potential paths with given storage, streamer and the service group, then reserve them with a short lease term
	///@param[in] maxCost		a acceptable cost to level the paths, 0 means receive all the paths with cost 0-10000
	///@param[in] maxTickets    the max number of tickets can be received in the returning ticket collection, 0 means receive all the possible paths
	///@param[in] hintLease		a value of leaseTerm in msec to hint the service how long should reserve the path if it is possible.
	///						0 - means take the default lease of the service
	///@param[in] contextIn		a map of allocation context in the case that session is not given
	///@param[in] sess  		access to the Weiwoo session object for the allocation
	///@param[out] contextOut	a map of allocation context as output, normally it is a copy of contextIn, but may adjust something
	///@return				a collection of reserved ticket.
	///@note the ticket will be detroyed by the service if its expiration is met. the ticket user must renew the lease if he want to utlize
	///		 the path. Otherwise, it is recommanded to destroy the un-used tickets to leave the system at a high ablity level
	///						0 - means take the default lease of the service
	PathTickets reservePathsEx(int maxCost, int maxTickets, int hintLease,
		                       ValueMap contextIn, SRM::Session* sess, out ValueMap contextOut)
	   throws InvalidParameter, ServerError, NotSupported;
};


};
};

#endif // __ZQ_TsAccreditedPathAdmin_ICE__
