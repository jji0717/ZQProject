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
// Ident : $Id: TsRepository.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsRepository.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 5     10-06-22 15:14 Haoyuan.lu
// 
// 4     10-03-04 15:44 Haoyuan.lu
// 
// 3     09-09-04 11:41 Hui.shao
// 
// 2     09-08-07 12:29 Hui.shao
// added ContentLib and ContentPropagator
// 
// 1     09-04-01 16:52 Hui.shao
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_Repository_ICE__
#define __ZQ_TianShanIce_Repository_ICE__

#include "TianShanIce.ICE"
#include "TsSRM.ICE"
#include "TsStorage.ICE"
#include "TsEvents.ICE"
//#include <../common/TianShanUtils.ICE>

module TianShanIce
{

// -----------------------------
// namespace Repository
// -----------------------------
/// Repository represents the content distributions and metadata within a local site
module Repository
{

/// defines a metadata structure.
struct MetaDataValue
{
	string value;	///< the value of a metadata will always be stored in string format
	ValueType hintedType;  ///< this field indicate how to covert the value date
};

/// defines a key to value map of metadata.
dictionary<string, MetaDataValue> MetaDataMap;

// -----------------------------
// class MetaObject
// -----------------------------
class MetaObject
{
	Ice::Identity	ident; ///< ident.name = objectId and ident.name = objectType

	["cpp:const", "freeze:read"] idempotent Ice::Identity getObjId();
	["cpp:const", "freeze:read"] idempotent string getType();
};

/// defines the return value data type of an meta object
struct MetaObjectInfo
{
	string id;		///< the unique id of the object/type
	string type;	///< the type of the meta-object

	MetaDataMap metaDatas; ///< the collection of the expected metadata of this meta-object
};

/// a collection of MetaObjectInfo
sequence <MetaObjectInfo> MetaObjectInfos;


// ----------------------------------------------
// interface CachedMetaObjectInfos to extends MetaObject
// ----------------------------------------------
// interface CachedMetaObjectInfos extends TianShanUtils::TimeoutObj
// {
// 	["amd", "ami"]
// 	MetaObjectInfos next(string startName, int maxCount, int renewMs)
// 		throws ServerError, NotSupported;
// 
// 	["cpp:const"] int size();
// 
// 	["cpp:const"] int left();
// 
// 	void destory();
// };

// -----------------------------
// interface MetaDataLibrary
// -----------------------------
/// this interface represents the common query interface for the repository modules
interface MetaDataLibrary
{

	///
	MetaObjectInfo openObject(string id, StrValues expectedMetaDataNames)
	throws
		EntityNotFound,
		NotSupported,
		InvalidParameter;

	string createObject(string type, long timeout)
	throws
		ServerError;

	["amd"] MetaObjectInfos lookup(string type, Properties searchForMetaData, StrValues expectedMetaDataNames)
	throws
		NotSupported,
		InvalidParameter,
		ServerError;
};

// ----------------------------------------------
// interface ContentReplica to extends MetaObject
// ----------------------------------------------
/// the MetaContent is an entity class of ContentLib extends MetaDataLibrary to cover extended options. This interface usually will not
/// be exported externally.
/// its name of identify is formatted in "ShortContentName@NetId$VolName" and category = "ContentReplica"
interface ContentReplica
{
//    ::TianShanIce::Storage::Content* content; // access to the content object on the ContentStore
//    ::TianShanIce::Properties metaData; // the additional metadata of A3Content
//    ::TianShanIce::Storage::ContentState state;
    
    /// get the unique replica id of the content
    ///@return the content replicaId in the format of "ShortContentName@NetId$VolName"
    ["cpp:const", "freeze:read"] idempotent Ice::Identity getReplicatId();

    /// get the asset identification of the content
    ///@param[out] providerId the provider Id of the content
    ///@param[out] providerAssetId the provider asset Id of the content
    ///@note see CableLab's asset ID definiton
    ["cpp:const", "freeze:read"] idempotent
    void getAssetId(out string providerId, out string providerAssetId);
    
    /// get the location information of this replica of content
    ///@param[out] contentStoreNetId the net Id of the ContentStore
    ///@param[out] volumeName the volume name on the ContentStore that this replica is saved in
    ["cpp:const", "freeze:read"] idempotent
    void getReplicaLocation(out string contentStoreNetId, out string volumeName);
    
    // should be covered by getInfo()
    // ["cpp:const", "freeze:read"] idempotent 
    // ::TianShanIce::Properties getMetaData();
    
    /// get the content state of this replica
    ///@return the content state of the replica
    ["cpp:const", "freeze:read"] idempotent
    ::TianShanIce::Storage::ContentState getState();
    
    /// convert to the access of the content
    ///@return the access of the content
    ["cpp:const", "freeze:read"] idempotent
    ::TianShanIce::Storage::Content* theContent();
    
    /// get the replica object information by specifying the expected metadata names
    ///@param[in] expectedMetaData to specify the metadata names that wish to receive
    ///@return the object information
    ["cpp:const", "freeze:read"] idempotent
    ::TianShanIce::StatedObjInfo getInfo(::TianShanIce::StrValues expectedMetaData);
    
    /// handles the content event
    ///@param[in] eventName the name of the content event
    ///@param[in] params    the parameters of the content event
    ["freeze:write"]
    void OnEvent(string eventName, ::TianShanIce::Properties params); // process the content event

    /// destroy a content replica, the Content that this replica refer to will also be destroyed
	["freeze:write"]
	void destroy(string reason)
		throws NotSupported;
};

/// a collection of ContentReplica
sequence <ContentReplica* > ContentReplicas;

// ----------------------------------------------
// interface ContentStoreReplica to extends MetaObject
// ----------------------------------------------
/// the MetaContent is an entity class of ContentLib extends MetaDataLibrary to cover extended options. This interface usually will not
/// be exported externally.
/// its name of identify is formatted in "NetId" and category = "ContentStoreReplica"
interface ContentStoreReplica
{
    ["cpp:const", "freeze:read"] idempotent
    Ice::Identity getNetId();
    
    ["cpp:const", "freeze:read"] idempotent
    ::TianShanIce::Storage::ContentStore* theStore();
    
    ["cpp:const", "freeze:read"] idempotent
    ::TianShanIce::ObjectInfo getInfo(::TianShanIce::StrValues expectedMetaData);
    
    ["freeze:write"]
    void OnEvent(string eventName, ::TianShanIce::Properties params); // process the store event
};

// ----------------------------------------------
// interface MetaVolume to extends MetaObject
// ----------------------------------------------
/// the MetaContent is an entity class of ContentLib extends MetaDataLibrary to cover extended options. This interface usually will not
/// be exported externally.
/// its name of identify is formatted in "NetId$VolName" and category = "MetaVolume"
interface MetaVolume
{
    ["cpp:const", "freeze:read"] idempotent
    Ice::Identity getNetId();
    
    ["cpp:const", "freeze:read"] idempotent
    ::TianShanIce::Storage::Volume* theVolume();

    ["cpp:const", "freeze:read"] idempotent
    ::TianShanIce::StatedObjInfo getInfo(::TianShanIce::StrValues expectedMetaData);
    
    ["freeze:write"]
    void OnEvent(string eventName, ::TianShanIce::Properties params); // process the volume event
};

// ----------------------------------------------
// service ContentLib
// ----------------------------------------------
/// ContentLib to extend MetaDataLibrary as its meta data query interface for local contents, volumes and ContentStores.
/// @li lookup(type="Content", ...) to query for local contents
/// @li lookup(type="Volume", ...) to query for local managed volumes
/// @li lookup(type="ContentStore", ...) to query for local managed ContentStore instances
/// ContentLib extends with the entries to cover content locating
interface ContentLib extends ::TianShanIce::BaseService, ::TianShanIce::Events::GenericEventSink
{
	/// convert the ContentStore netId to the access to te ContentStoreReplica object
	///@param[in] netId the netId of ContentStore to convert
	///@return the access to the ContentStore, NULL if the ContentStore is unreachable
	///@throw EntityNotFound if the specified ContentStore is unknown locally
	ContentStoreReplica* toStoreReplica(string netId)
		throws EntityNotFound, InvalidParameter;

	/// convert the volumn id to the access to te MetaVolume object
	///@param[in] volumeId the volumeId in the format of "NetId$VolName"  to convert
	///@return the access to the Volume, NULL if the Volume is unreachable
	///@throw EntityNotFound if the specified volume is unknown locally
	MetaVolume* toVolume(string volumeId)
		throws EntityNotFound, InvalidParameter;

	/// convert the contentReplicaId to the access to te ContentReplica object
	///@param[in] volumeId the contentReplicaId in the format of "NetId$VolName/ContentName"  to convert
	///@return the access to the Content, NULL if the Content is unreachable
	///@throw EntityNotFound if the specified content is unknown locally
	ContentReplica* toContentReplica(string contentReplicaId)
		throws EntityNotFound, InvalidParameter;

	/// locate a content by metadata
	///@return a collection of content replica metaobject
	["ami", "amd"]
	MetaObjectInfos locateContent(Properties searchForMetaData, StrValues expectedMetaDataNames, int maxHop, bool pingOnly)
		throws
			NotSupported,
			InvalidParameter;
			
	/// locate a content by providerId and providerAssetID
	///@return a collection of content replica metaobject
	["ami", "amd"]
	MetaObjectInfos locateContentByPIDAndPAID(string netId, string volumeId, string providerId, string providerAssetId, StrValues expectedMetaDataNames)
		throws
			NotSupported,
			InvalidParameter;
			
	/// locate  a content by  NetID and Volume name
	///@return a collection of content replica metaobject
	["cpp:const", "ami", "amd", "freeze:read"] 
	MetaObjectInfos locateContentByNetIDAndVolume(string netId, string volumeId, StrValues expectedMetaDataNames, int startCount, int maxCount, out int totalCount)
		throws
			NotSupported,
			InvalidParameter;
	
    ///list volumes by NetID
	///@return a collection of volumes replica metaobject
	["ami", "amd"]
	MetaObjectInfos locateVolumesByNetID(string netId, StrValues expectedMetaDataNames)
		throws
			NotSupported,
			InvalidParameter;		
	
};

// ----------------------------------------------
// service ContentPropagator
// ----------------------------------------------
/// ContentPropagator extends ContentLib to cover content publishing and exchanging
interface ContentPropagator extends ContentLib
{
	/// ingest a content into the local storages managed by the library. The ContentPropagator takes the given propagation rule name to
	/// select the candidate ContentStore instance(s) and create content instance to host the ingestion
	///@param[in] contentName the name of the Content to create
	///@param[in] sourceURL the source URL that the ingestion can be from
	///@param[in] contentMetadata the user-defined metadata of the content that is being created
	///@param[in] maxBW the maximal ingestion speed in bps
	///@param[in] ruleName the string name of the propagation rule defined in the ContentPropagator to apply to the ingestion
	///@param[in] expiration the expiration of this ingest request. the ingest will be terminated if it can not finish piror to the expiration
	///@param[in] params other user-defined parameters recoganized by the ContentPropagator implementation
	///@return a collection of ContentReplica that the ContentPropagator selected to host the ingestions. Usually a single instance in the
	///        collection for the edge storage purposes, but may be multiple if the ContentPropagator wishes to perform concurrently in one batch
	["ami", "amd"]
	ContentReplicas ingestContent(string contentName, string sourceURL, Properties contentMetadata, int maxBW, string ruleName, string expiration)
		throws NotSupported, InvalidParameter;

	/// export a content from a selected storage of local ContentStore managed by the library. The ContentPropagator takes the given
	/// propagation rule name to select one candidate ContentStore instance to host the exporting
	///@param[in] contentName the name of the Content to export
	///@param[in] requirements the resource requirement to perform the exporting
	///@param[in] ruleName the name of the propagation rule that the exporting should follow
	///@return the access to the picked content, the user may call Content::expose() to follow up the further exporting steps
	["amd"]
	ContentReplica* exportContent(string contentName, ::TianShanIce::SRM::ResourceMap requirements, string ruleName)
		throws NotSupported, InvalidParameter;

	/// export a content from local ContentStores via a streaming session. The ContentPropagator would prepare a streaming session on 
	/// SRM to perform the content exporting
	///@param[in] contentName the name of the Content to export
	///@param[in] requirements the resource requirement to perform the exporting
	///@param[in] ruleName the name of the propagation rule that the exporting should follow
	///@return the access to the streaming session that has been InService, the user may query the session data for the expected parameters
	///        or call session::stream::play() to make the ContentStore pump the content to the destination
	["amd"]
	SRM::Session* exportContentViaStream(string contentName, ::TianShanIce::SRM::ResourceMap requirements, string ruleName)
		throws NotSupported, InvalidParameter;
};

};
};

#endif // __ZQ_TianShanIce_Storage_ICE__
