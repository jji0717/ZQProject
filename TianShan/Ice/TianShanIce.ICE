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
// Ident : $Id: TianShanIce.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TianShanIce.ICE $
// 
// 3     1/11/16 6:04p Dejian.fei
// 
// 2     10/09/13 11:04a Hongquan.zhang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 40    09-05-13 18:47 Hui.shao
// 
// 39    09-03-09 15:18 Hui.shao
// changed the confusing attribute names in Replica
// 
// 38    09-03-02 17:29 Hui.shao
// 
// 37    08-11-06 13:13 Hui.shao
// 
// 36    08-10-27 17:14 Hui.shao
// 
// 35    08-10-17 10:50 Hui.shao
// 
// 34    08-08-14 14:28 Hui.shao
// merge from 1.7.10
// 
// 34    08-08-11 18:45 Hui.shao
// added replica defintion
// 
// 33    08-04-29 16:24 Hui.shao
// include the zq ice version definiton
// 
// 32    08-02-19 15:14 Hui.shao
// 
// 30    08-01-03 15:47 Hui.shao
// 
// 29    07-08-07 16:43 Hui.shao
// 
// 28    07-08-07 16:41 Hui.shao
// 
// 27    07-05-23 13:26 Hui.shao
// added fields category and errcode into exception
// 
// 26    07-04-18 15:40 Ken.qian
// Add float to Variant
// 
// 25    07-03-28 16:52 Hui.shao
// added namespace Site
// 
// 25    07-03-28 16:25 Hui.shao
// added name space Site
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_ICE__
#define __ZQ_TianShanIce_ICE__

#include <ZQIceVersion.ICE>
#include <Ice/Identity.ice>

/// TianShan represents an open architecture of VOD integration
module TianShanIce
{

// -----------------------------
// version definition
// -----------------------------
// the following constants are temporary, will be updated by the build procedure automatically
const int MajorVersion = 1;
const int MinorVersion = 7;
const int PatchNumber  = 0;
const int buildNumber  = 0;
const string buildTime = "20070806T120000";

// -----------------------------
// exception BaseException
// -----------------------------
/// the base exception in TianShan
exception BaseException
{
	string category;  ///< the source component that throw the exception
	int	   errorCode; ///< the error code of the category
	string message;   ///< exception description message
};

// -----------------------------
// exception NotSupported
// -----------------------------
/// exception thrown when the application/object or operation is not supported
exception NotSupported extends BaseException
{
};

// -----------------------------
// exception EntityNotFound
// -----------------------------
/// exception thrown when an refer-ed entity doesn't not exist
exception EntityNotFound extends BaseException
{
};

// -----------------------------
// exception ServerError
// -----------------------------
/// exception thrown when an internal server error occurs
exception ServerError extends BaseException
{
};

// -----------------------------
// exception ClientError
// -----------------------------
/// exception thrown when an error caused by client's invocation
exception ClientError extends BaseException
{
};

// -----------------------------
// exception NotImplemented
// -----------------------------
/// exception thrown when the application/object or operation is not implemented
exception NotImplemented extends NotSupported
{
};

// -----------------------------
// exception InvalidStateOfArt
// -----------------------------
/// exception thrown when the application/object is not at the state-of-the-art for the operation
exception InvalidStateOfArt extends NotSupported
{
};


// -----------------------------
// exception InvalidParameter
// -----------------------------
/// exception thrown when the given parameters are not as expected
exception InvalidParameter extends ClientError
{
};
// error codes belong to InvalidParameter
const int errcodeAssetNotfound = -10;



// -----------------------------
// enum State
// -----------------------------
/// the state of an object or service
enum State
{
	stNotProvisioned,   ///< the object is created but hasn't been provisioned
	stProvisioned,      ///< The serivce or object has been provisioned, but is not presently 100% intended to perform its function.
    stInService,        ///< The serivce or object has been provisioned, and is 100% intended to provide its function.
	stOutOfService,     ///< The serivce or object has been provisioned, but has been invalid to perform its function.
};

// -----------------------------
// enum AdminState
// -----------------------------
/// the state of an object or service
enum AdminState
{
	asUp,			  ///< The serivce or object is up for using if it becomes available
	asDown,			  ///< The serivce or object is forced to be down or reserved not to utilize in the current system even if it becomes available
};

// -----------------------------
// interface BaseService
// -----------------------------
/// the base service definition
interface BaseService
{
//	/// returns the production information
//	///@param[out] category the service name, case-insensive, same as the class name
//	///@param[out] vendor a short name of the vendor
//	///@param[out] proddesc a short desc of the service
//	///@param[out] version the vendor's version string
//	void getProductInfo(out string category, out string vendor, out string proddesc, out string version)
//		throws NotSupported;

	/// get the access URI to the administrative page of the derived service
	///@return the URI to the admin page
	string getAdminUri() throws NotSupported;

	/// get the state of the serivce
	///@return the current state of the service
	State getState() throws NotSupported;
};

// -----------------------------
// class Variant
// -----------------------------
sequence <byte>		BValues; ///< container of bytes in Variant
sequence <long>		LValues; ///< container of long integers in Variant
sequence <int>		IValues; ///< container of integers in Variant
sequence <string>	StrValues; ///< container of strings in Variant
sequence <float>	FValues;   ///< container of floats in Variant

/// enum value type
enum ValueType
{
//	vtNil,
	vtBin,
	vtInts,
	vtLongs,
	vtStrings,
	vtFloats,
//	vtMap,
	vtUnknown
};

/// represent a Variant
struct Variant
{
	ValueType		type; ///< enum value of ValueType
	bool			bRange; ///< true if the value is a range, only the value 0 and 1 in the value container are valid then
	BValues			bin;  ///< where contains the binary values when type=vtBin
	LValues			lints;  ///< where contains the long values when type=vtLongs
	IValues			ints;  ///< where contains the integer values when type=vtInts
	StrValues		strs;  ///< where contains the string values when type=vtStrings
	FValues			floats; ///< where contains the string values when type=vtFloats
};

/// a key to variant map
dictionary <string, Variant> ValueMap;

/// represent a property map as a key-to-string map
dictionary <string, string> Properties;

/// supported configurate items with default values
struct PDElement
{
	string			keyname;
	//bool			optional;
	bool			optional2;
	Variant			defaultvalue;
};

/// the schema definition of the private data set
sequence <PDElement> PDSchema;

// -----------------------------
// struct StatedObjInfo
// -----------------------------
/// The StatedObjInfo information exported from the entities with states
struct StatedObjInfo
{
	Ice::Identity ident;
	TianShanIce::State state;
	TianShanIce::Properties props;
};

// -----------------------------
// struct ObjectInfo
// -----------------------------
/// The information about an object exported from the entities
struct ObjectInfo
{
	Ice::Identity ident;
	TianShanIce::Properties props;
};

// -----------------------------
// struct Replica
// -----------------------------
/// represent a Replica
struct Replica
{
	string category;   ///< the category of the replica, usually the name of class/interface defined in TianShan SDK
	string groupId;	   ///< the group id assisting the category name
	string replicaId;  ///< the unique id of the replica instance with in the category and group

	byte priority;	   ///< the pirority of this replica, 0 - highest
//	bool master;	   ///< true if the replica is known as the master replica in this category and group
	State replicaState;///< state of the replica
	
	byte maxPrioritySeenInGroup; ///< the maximal priority in the group that this replica reporter has ever seen
	
	Object* obj;       ///< access to the replica

	long stampBorn;    ///< The timestamp when the owner of this replica realizes the existence of the replica
	long stampChanged; ///< The timestamp when the owner detected the latest change on the replica

	TianShanIce::Properties props;
};

/// a collection of replicas
sequence< Replica > Replicas;

// -----------------------------
// interface ReplicaQuery
// -----------------------------
/// the interface to query for replicas
interface ReplicaQuery
{
	/// entries to query for the replica of a given category and group of objects
	///@param[in] category the category name of the objects to query, must specify and non-empty
	///@param[in] groupId the groupId of the objects to query, empty or "*" means all the objects of the given category regardless
	///           group
	///@return    a collection of replica information that matches the query
	///@throw InvalidParameter illegal input in parameters category or groupId
	///@throw NotSupported the specified category or groupId is not support in this query interface
	["ami", "amd"] Replicas queryReplicas(string category, string groupId, bool localOnly)
		throws InvalidParameter, NotImplemented, NotSupported;
};

// -----------------------------
// interface ReplicaSubscriber
// -----------------------------
/// the ContentStoreEx extends ContentStore with the extentions to receive the file events from the proxied filesystem
interface ReplicaSubscriber
{
	/// entries to receive the object replica reports
	///@param[in] stores a collection of the ContentStore replica information to update
	///@return    timeout in second that this ContentStore expects the replica reporter to report again next time
	///            0 or negative - INFINITE, no timeout, and will be completely up to the reporter to invoke
	["ami", "amd"] int updateReplica(Replicas replicaInfos);
};

};

#endif // __ZQ_TianShanIce_ICE__
