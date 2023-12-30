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
// $Log: /ZQProjs/TianShan/ContentStore/ContentStore.ICE $
// 
// 9     12/11/13 7:43p Hui.shao
// added hashed folder calculation
// 
// 8     3/06/13 4:47p Hui.shao
// export the indexing from mountpath to volname thru ICE
// 
// 7     4/27/12 5:56p Hui.shao
// added Content::getProvisionSession()
// 
// 6     3/26/12 4:01p Hui.shao
// added openFolderEx() to create subfolder recursively
// 
// 5     6/23/11 10:26a Hongquan.zhang
// 
// 4     6/22/11 7:36p Build
// a temporary solution to cover getSubtype() internally calls
// freeze::write funcs
// 
// 3     10-12-08 12:19 Hui.shao
// 
// 2     10-11-17 18:17 Hui.shao
// 
// 18    10-04-27 13:41 Haoyuan.lu
// 
// 17    10-04-26 15:05 Hui.shao
// added cached content list
// 
// 16    09-09-11 11:17 Hui.shao
// made the auto sync with filesystem optional
// 
// 15    09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 16    09-06-29 18:22 Jie.zhang
// add wakeupContent interface to internal contentstore interface
// 
// 15    09-05-26 13:45 Hongquan.zhang
// add OnFileCreated
// 
// 14    09-01-23 15:44 Hongquan.zhang
// 
// 13    08-12-11 12:31 Hui.shao
// 
// 12    08-11-15 15:20 Hui.shao
// 
// 11    08-11-15 14:00 Hui.shao
// 
// 10    08-11-15 13:45 Hui.shao
// added file residential status
// 
// 9     08-11-10 11:49 Hui.shao
// removed getTimeStamps()
// 
// 8     08-11-06 18:19 Hui.shao
// added a Content::isInUse() query for clusterCS to query node
// 
// 7     08-11-06 13:22 Hui.shao
// 
// 6     08-10-28 17:57 Hui.shao
// test the last write if it is worthy to populate attributes
// 
// 5     08-10-23 18:31 Hui.shao
// moved watchdog out to common
// 
// 4     08-10-14 11:33 Hui.shao
// 
// 3     08-10-07 19:55 Hui.shao
// added volume layer
// 
// 2     08-08-14 18:01 Hui.shao
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// 
// 6     08-08-11 18:42 Hui.shao
// added store replica handling
// 
// 5     08-07-31 18:43 Hui.shao
// restrict on state for provision-related operation
// 
// 4     08-07-21 11:51 Hui.shao
// check in the works of last weekend
// 
// 3     08-07-18 15:12 Hui.shao
// 
// 2     08-07-15 14:19 Hui.shao
// 
// 1     08-07-10 19:29 Hui.shao
// ===========================================================================
// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice --output-dir .. ../$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice --index "TianShanIce::Storage::ChildVolume,TianShanIce::Storage::VolumeEx,parentVolName,case-sensitive" ChildVolume --output-dir .. ../$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice --index "TianShanIce::Storage::FileOfVol,TianShanIce::Storage::UnivContent,identVolume" FileOfVol --output-dir .. ../$(InputName).ice
// outputs:
//	../$(InputName).h
//	../$(InputName).cpp
//	../ChildVolume.h
//	../ChildVolume.cpp
//	../FileOfVol.h
//	../FileOfVol.cpp

#ifndef __ZQ_TianShanIce_ContentStore_ICE__
#define __ZQ_TianShanIce_ContentStore_ICE__

#include <TsStorage.ICE>
#include <TsContentProv.ICE>
#include <../common/TianShanUtils.ICE>

#include <Ice/Identity.ice>

module TianShanIce
{

module Storage
{

enum FileEvent
{
	fseFileCreated,		///< the event of file creating
	fseFileDeleted,		///< the event of file deleting
	fseFileRenamed,		///< the event of file name changing
	fseFileModified,	///< the event of file size or attributes changing
	fseSecurity,		///< the event of access security settings changing
};

enum ProvisionEvent
{
	peProvisionStarted,	 ///< the event of provision starts
	peProvisionStopped,	 ///< the event of provision stops
	peProvisionProgress, ///< the event of provision progress
	peProvisionStreamable, ///< the event when the provision think its output can be streamable
};

enum FileResidentialFlag
{
	frfResidential,   ///< flag (1 <<frfResidential) equals 1 if at least one of the content member files is residental on the filesystem
	frfReading,   ///< flag (1 <<frfReading) equals 1 if at least one of the content member files is being read
	frfWriting,   ///< flag (1 <<frfWriting) equals 1 if at least one of the content member files is being written
	frfAbsence,   ///< flag (1 <<frfAbsence)   equals 1 if at least one of the content member files appears unavailable
	frfCorrupt,   ///< flag (1 <<frfCorrupt)   equals 1 if at least one of the content member files appears corrupt in its content
	frfDirectory, ///< flag (1 <<frfDirectory) equals 1 if at least one of the content member files appears as a subdirectory
};

/// defines a universal content structure
["freeze:write"] 
class UnivContent extends Content implements TianShanUtils::TimeoutObj
{
	Ice::Identity ident;	///< identity
	bool fromFsOrphan;		///< true if this content is only populated from the file system instead a client create thru ContentStore
	long expiration;
	long stampProvisionSetup;
	bool bDirtyAttrs;
	
	Ice::Identity identVolume;

	["cpp:const", "freeze:read"] idempotent Ice::Identity getIdent();

	void populateAttrsFromFilesystem();
	
	void setMetaData(TianShanIce::Properties metaData);

	["cpp:const", "freeze:read"] bool isDirty();
	["cpp:const", "freeze:read"] bool isCorrupt();

	["cpp:const", "freeze:read"] idempotent string getMainFilePathname();

	/// entry of record restore during service restart
	void OnRestore(string stampLastFileWrite);
	
	["ami"] void OnFileModified();

	["ami"] void OnFileRenamed(string newName);
	
	void OnFileCreated( string memberFile );

// 	bool isInUse() throws ServerError;

	ContentState enterState(ContentState targetState)
		throws InvalidStateOfArt;
		
	/// test the the file residential status of this content
	///@param FRFlagsToTest the flags that this query is interested, (~0) if test all. @sa FileResidentialFlag
	///@return the flags of the member files of this content
	["cpp:const", "freeze:read"] long checkResidentialStatus(long FRFlagsToTest)
		throws InvalidStateOfArt, ServerError, InvalidParameter;

	["cpp:const", "freeze:read"] ContentProvision::ProvisionSession* getProvisionSession();
};

interface CachedContentList extends TianShanUtils::TimeoutObj
{
	["amd", "ami"]
	ContentInfos next(TianShanIce::StrValues metaDataNames, string startName, int maxCount, int renewMs)
		throws ServerError, NotSupported;

	["cpp:const"] int size();

	["cpp:const"] int left();

	void destory();
};

/// defines a universal content structure
["freeze:write"] 
class VolumeEx implements Volume, TianShanUtils::TimeoutObj
{
	Ice::Identity ident;	///< identity to find meta object. @sa LibMetaObject::ident
	bool   isVirtual;		///< true if this content is only populated from the file system instead a client create thru ContentStore
	string mountPath;
	long quotaSpaceMB;
	long stampCreated;
	
	string parentVolName;
	
	TianShanIce::Properties metaData;

	["cpp:const", "freeze:read"] idempotent Ice::Identity getIdent();
	["cpp:const", "freeze:read"] idempotent string getMountPath();
	["cpp:const", "freeze:read"] VolumeInfo getInfo();
	["cpp:const", "ami", "amd", "freeze:read"] CachedContentList* cachedListContents(int timeout)
		throws ServerError;

	UnivContent* createContent(Ice::Identity identContent, bool fromFsOrphan, string stampLastFileWrite);
	bool deleteContent(::Ice::Identity identContent);
	
	void setMetaData(TianShanIce::Properties metaData);
	
	///@throw ServerError if the portal file system is not available
	["ami"] void syncWithFileSystem() throws ServerError;
};

// -----------------------------
// service ContentStoreEx
// -----------------------------
/// the ContentStoreEx extends ContentStore with the extentions to receive the file events from the proxied filesystem
interface ContentStoreEx extends ContentStore, ReplicaSubscriber, ReplicaQuery, TianShanUtils::TimeoutObj
{
	/// entries to receive the file events
	["ami"] void OnFileEvent(FileEvent event, string fileName, Properties params);
	["ami"] void OnProvisionEvent(ProvisionEvent event, string storeNetId, string volumeName, string contentName, Properties params);

	["ami"] void wakeupContent(string contentFullname);
	
	void enableAutoFileSystemSync(bool enable);
	
	/// openFolder by full name, create if necessary, designed for private use to create subfolder recursively especially by the CacheStore
	///@param fullFolderName the full folder name, such as "/<volName>/<Paths of folder>/"
	///	            if not start with a root "/", the inputted folder name will be recoganized to be under the default volume
	///@param createIfNotExist true if want to create the folders recursively
	///@param quotaSpaceMB     quotaSpace limitation in MB, 0 or negative means unlimitted
	///@return the access to the folder object, NULL if can not open
	Folder* openFolderEx(string fullFolderName, bool createIfNotExist, long quotaSpaceMB)
		throws InvalidParameter, NotImplemented;
		
	// indecing the mounted path to volume name reversly that has been registered via mountStoreVolume()
	//@return volumeName, empty if not found
	string mountPathToVolName(string mountPath);
	
	/// Utility function that converts the inputed content name to the path name of in the cache with hashed folders
	///@param[in] contentName the name of Content to export
	///@return a string path name of the hashed folder including the leading volumeName
	string hashFolderNameByContentName(string contentName);

};

};
};

#endif // __ZQ_TianShanIce_BaseCS_ICE__
