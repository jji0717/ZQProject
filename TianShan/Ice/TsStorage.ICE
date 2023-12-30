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
// Ident : $Id: TsStorage.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsStorage.ICE $
// 
// 4     3/09/15 9:34a Li.huang
// add content type ctCSI 
// 
// 3     6/13/14 10:55a Hui.shao
// 
// 2     6/23/11 10:29a Hongquan.zhang
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 53    10-02-10 12:15 Jie.zhang
// add audio type
// 
// 52    10-01-25 17:25 Jie.zhang
// add listSubFolder to folder
// 
// 51    09-12-22 17:47 Hui.shao
// abstracted folder from volume
// 
// 50    09-12-22 14:57 Hui.shao
// 
// 49    09-08-21 14:18 Hui.shao
// merged 1.10 changes as of 8/20
// 
// 49    09-08-21 14:16 Hui.shao
// 
// 48    09-06-02 16:17 Jie.zhang
// 
// 48    09-05-15 15:08 Jie.zhang
// add stamplastupdate comment
// 
// 47    09-05-13 18:50 Hui.shao
// 46    08-12-26 18:21 Hui.shao
// 
// 45    08-12-10 18:37 Jie.zhang
// error code changes
// 
// 44    08-11-18 13:58 Hui.shao
// 
// 43    08-11-06 13:17 Hui.shao
// 
// 42    08-11-03 11:25 Jie.zhang
// merge from 1.7.8 about getExportUrl
// 
// 41    08-11-03 11:23 Hui.shao
// 
// 40    08-10-07 15:52 Hui.shao
// added a layer to represent volume
// 
// 39    08-08-14 14:28 Hui.shao
// merge from 1.7.10
// 
// 42    08-08-11 18:44 Hui.shao
// added getCacheLevel
// 
// 41    08-07-31 17:21 Hui.shao	
// take off the const metadata from getExportUrl()
// 
// 40    08-07-21 10:36 Hui.shao
// check in the works of the weekend
// 
// 39    08-07-18 18:55 Hui.shao
// 
// 38    08-07-17 14:24 Hui.shao
// 
// 37    08-07-07 15:49 Hui.shao
// 
// 36    08-05-13 14:10 Fei.huang
// 
// 35    08-05-13 13:37 Jie.zhang
// 
// 34    08-04-29 13:24 Fei.huang
// 
// 33    08-04-09 12:13 Fei.huang
// 
// 32    08-04-02 13:39 Ken.qian
// add destroy2() with mandatory flag
// 
// 31    08-03-12 10:43 Fei.huang
// interface addSupportFileSize added
// 
// 32    08-03-11 16:12 Fei.huang
// 
// 31    08-03-11 14:21 Ken.qian
// add getSupportFilesize in Content interface
// 
// 30    07-12-14 18:42 Fei.huang
// 
// 29    07-12-10 12:04 Ken.qian
// had comment in cancelProvision
// 
// 28    07-08-10 15:52 Ken.qian
// 
// 27    07-06-04 16:34 Ken.qian
// 
// 26    07-05-18 11:00 Ken.qian
// Add expected transfer protocol in getExportURL function
// 
// 25    07-05-17 12:03 Ken.qian
// change the getExportURL
// 
// 24    07-05-16 18:52 Ken.qian
// Adding ContentExportInfo for support NGOD2.0 Content Export
// 
// 23    07-05-09 15:39 Fei.huang
// 
// 22    07-04-17 10:46 Ken.qian
// 
// 21    07-04-13 12:15 Ken.qian
// 
// 20    07-04-12 18:13 Ken.qian
// 
// 19    07-04-04 17:10 Ken.qian
// add interface for NGOD
// 
// 18    07-04-04 14:45 Fei.huang
// 
// 17    07-03-16 18:50 Li.huang
// 
// 16    06-09-25 16:01 Ken.qian
// 
// 15    06-09-22 9:49 Ken.qian
// 
// 14    06-09-22 9:15 Ken.qian
// 
// 13    06-09-20 13:56 Ken.qian
// 
// 12    06-09-20 12:23 Ken.qian
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_Storage_ICE__
#define __ZQ_TianShanIce_Storage_ICE__

#include "TianShanIce.ICE"
#include "TsEvents.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

// -----------------------------
// namespace Storage
// -----------------------------
/// Storage represents a media content storage within the TianShan architecture
module Storage
{

//forward declaration
interface ContentStore;
interface Volume;

// -----------------------------
// exception StoreException
// -----------------------------
/// exception thrown when meet a NO Resource exception in provision
exception NoResourceException extends BaseException
{
};

/// a collection of trick speeds
sequence < float > TrickSpeedCollection;

// two special constants of trick speeds
// const float SeamLessForward		= 1.00001; ///< the media content support seamless forward trick speed
// const float SeamLessBackward	= -1.00001; ///< the media content support seamless backward trick speed

enum ContentState
{
	csNotProvisioned,            ///< content record has been created but not yet set to provision
	csProvisioning,              ///< content is being provisioned but not available for access
	csProvisioningStreamable,    ///< the content is streamable but not complete provisionsing yet
	csInService,				 ///< the content is resident healthily for access
	csOutService,                ///< the content is resident but out of service
	csCleaning                   ///< the content is being cleaned up
};

// -----------------------------
// class Content
// -----------------------------
///@brief represents the basic access to a individual content files.
///
/// The Content is a base class, which is intended to represent the basic 
/// functionality to access a individual content files. The functions it publishes
/// for such use are equivalent to virtual functions, which must be implemented by any derived classes.
/// its base implementation is a local raw disk file
///@image html content_states.JPG "Statediagram of Content"
["freeze:write"] 
class Content
{
	// time stamps, of GMT
	long stampCreated;		///< when the content is created by calling ContentStore::openContent(create=true);
	long stampProvisioned;	///< when the content file is fully provisioned into the store
	long stampLastUpdated;	///< when the content has been newly updated

	ContentState state;     ///< the state of the content

	string provisionPrxStr;         ///< provision session proxy string
	
	TrickSpeedCollection trickSpeeds; ///< the trick speeds supported
	
	TianShanIce::Properties metaData; // some other store specific attributes may defined here

	/// get the bound content store
	///@return reference to the bound content store
	["cpp:const", "freeze:read"] idempotent ContentStore* getStore();
	
	/// get the name of the content
	///@return the name of the content
	["cpp:const", "freeze:read"] idempotent string getName();
	
	/// get all the meta data of the content
	/// some known content metadata names are described in section @sa sec_tsStContMD "Content Meta Data"
	///@return  the meta data collection
	///@sa @ref sec_tsStContMD "Content Meta Data"
	["cpp:const", "freeze:read"] TianShanIce::Properties getMetaData();
	
	/// set a user's metadata
	/// @param[in] name - the name of the metadata to set.
	/// @param[in] value - the new value of the metadata
	/// @throw InvalidParameter if the metadata specified is a system metadata
	///@sa @ref ssec_tsStUsrMD "Known User's Meta Data"
	void setUserMetaData(string name, string value)
		throws InvalidParameter, ServerError;

	/// set a collection of user's metadata
	/// @param[in] metadata - a collection of metadata map to set. Only allows to set user's metadata, will be discarded if any
	///                       system metadata are included
	///@sa @ref ssec_tsStUsrMD "Known User's Meta Data"
	void setUserMetaData2(TianShanIce::Properties metadata)
		throws ServerError;

	/// get the state of the content
	///@return the state of the content
	["cpp:const", "freeze:read"] ContentState getState();

	/// check if the content is provisioned
	///@return true if the content has been saved in the content store
	["cpp:const", "freeze:read"] bool isProvisioned();

	/// get the save time of the content
	///@return the UTC format provision time
	/// if the content has not been provisioned, the return time is empty;
	["cpp:const", "freeze:read"] string getProvisionTime();

	/// destroy the content, all related files and records will be removed
	/// @throws 1) NotImplemented: not implemented \n
	///	    2) InvalidStateOfArt: content was failed to be deleted \n
	/// @note equals destroy2(false)
	void destroy()
		throws NotImplemented, InvalidStateOfArt;
	
	///destroy the content, all related files and records will be removed
	///@param[in] mandatory: if true, the content and its related files will be removed mandatorily even it is in use 
	///                      if false, fail if the member file is in use
	///@throws 1) NotImplemented: not implemented \n
	///        2) InvalidStateOfArt: content was failed to be deleted \n
	///@note when the content-destroy is executed, the related provisioning session will also be cancelled if the content
	/// is currently being provisioned
	void destroy2(bool mandatory)
		throws NotImplemented, InvalidStateOfArt;

	/// get the type of the content
	///@return the type of the content
	["cpp:const", "freeze:read"] string getLocaltype();

	/// get the sub type of the content
	///@return the sub type of the content
	string getSubtype();
	
	/// get the framerate of the content
	///@return the framerate of the content in frame per second, i.e. "29.97" fps
	///@throws 1) NotImplemented: not implemented
	///			2) InvalidStateOfArt: the content doesn't has frames, it maybe a non-media file
	["cpp:const", "freeze:read"] float getFramerate()
		throws InvalidStateOfArt, NotImplemented;
	
	/// get the video resolution of the content
	///@param[out]	pixelH		Horizontal resolutin in pixel
	///@param[out]	pixelV		Vertical resolutin in pixel
	///@throws 1) NotImplemented: not implemented \n
	///		   2) InvalidStateOfArt: the content doesn't support resolution , it maybe a non-media file \n
	["cpp:const", "freeze:read"] void getResolution(out int pixelHorizontal, out int pixelVertical)
		throws InvalidStateOfArt, NotImplemented;
	
	/// get the main file size in bytes
	///return    The file size in bytes
	["cpp:const", "freeze:read"] long getFilesize();

	/// get the support file size in bytes
	///return    The support file total size in bytes
	["cpp:const", "freeze:read"] long getSupportFileSize();
	
	/// get the content's playtime	
	///return    The duration in seconds
	["cpp:const", "freeze:read"] long getPlayTime();

	/// get the content's playtime	
	///return    The duration in milli-second
	["cpp:const", "freeze:read"] long getPlayTimeEx();

	/// get the content's BitRate	
	///return    The bitrate in bps
	["cpp:const", "freeze:read"] int getBitRate();

	/// get the content's MD5 checksum
	///return    The md5 checksum 
	["cpp:const", "freeze:read"] string getMD5Checksum();

	/// get the supported trick speeds of the content
	///@return	a collection of supported trick speeds. i.e. <-7.5, 7.5>. <SeamLessForward, SeamLessBackward>
	///@throws 1) NotImplemented: not implemented \n
	///		   2) InvalidStateOfArt: the content doesn't support trick speed, it maybe a non-media file \n
	["cpp:const", "freeze:read"] TrickSpeedCollection getTrickSpeedCollection()
		throws InvalidStateOfArt, NotImplemented, NotSupported;
	

	/// get the content's url
	///@param[in]	transferProtocol the expected transfer protocol to export the content, if ContentStore does not support the
	///                              expected transfer protocol, InvalidStateOfArt exception will throw. 
	///@param[in]	transferBitrate  transfer bitrate the expected transfer protocol to export the content, if ContentStore does not support the
	///@param[out]	ttl		     time to live for this export url, if url expired, exporting is not allowed
	///@param[out]	exppro		     the addition information for the export content
	///return    The url of the provisioned content in storage. This URL format: ftp://user:password@ip:port/filename 
	///
	///@throws 1) NotImplemented: not implemented \n
	///		   2) InvalidStateOfArt: the content is not ready in the storage current\n
	///                            Possible error code: csexpCodeUnsupportProto(103)
	string getExportURL(string transferProtocol, int transferBitrate, out int ttl, out Properties exppro) 
		throws InvalidStateOfArt, NotImplemented;

	///@todo get where is this content provisioned from, for audit purpose
	["cpp:const", "freeze:read"] string getSourceUrl()
		throws InvalidStateOfArt, NotImplemented, NotSupported;
	
	/// The active provision operation allows the Content's configurable data to be specified. 
	/// In particular, it tells the Content where the media file it is to manage is located (via URL), 
	/// The Content is expected to retrieve the media file from the URL between the given scheduled start time
	/// and stop time
	/// @param[in] sourceUrl - the source URL to provision this Content with
	/// @param[in] sourceContentType - the enum type code to specify the source content type, coversion may happen to 
	///                               meet the type of this destination type
	/// @param[in] overwrite         - true if need to overwrite a existing content
	/// @param[in] startTimeUTC      - the scheduled start time, values are in UTC format, NULL if need to start immediately
	/// @param[in] stopTimeUTC       - the scheduled stop time, values are in UTC format, NULL if no time-based cut off needed
	/// @param[in] maxTransferBitrate - the limitation on transfer bitrate in Kbps, the transfering
	///                       should not exceed this given max bitrate, 0 if no limitation
	/// @remarks	this method is allowed to called multiple times before the scheduled provision starts
	/// @throws 1) NotImplemented: not implemented \n
	///			2) InvalidStateOfArt: content exists but no overwriting is allowed \n
	///         3) NoResourceException: No resource to do this provision
	///         4) InvalidParameter: unsupported content type (csexpInvalidContentType) \n
	///         5) InvalidParameter: stopTime is piror to the startTime (csexpInvalidTime) \n
	///         6) InvalidParameter: unsupported source URL (csexpInvalidSourceURL) \n
	///         7) InvalidParameter: content exists but overwrite not specified as true (csexpContentIsReady) \n
	void provision(string sourceUrl,
		string sourceContentType,
		bool   overwrite,
		string startTimeUTC,
		string stopTimeUTC,
		int    maxTransferBitrate)
		throws InvalidParameter, NoResourceException, InvalidStateOfArt, NotImplemented;
	
	/// The passive provision operation allows the Content's configurable data to be specified. 
	/// In particular, it requires a push URL where the media file it is to manage is fed, 
	/// The Content is expected to receive the media file from the URL between the given scheduled start time
	/// and stop time
	/// @param[in] cType - the enum type code to specify the content type
	/// @param[in] overwrite    - true if need to overwrite a existing content
	/// @param[in] startTimeUTC - the scheduled start time, values are in UTC format, NULL if need to start immediately
	/// @param[in] stopTimeUTC - the scheduled stop time, values are in UTC format, NULL if no time-based cut off needed
	/// @param[in] maxTransferBitrate - the limitation on transfer bitrate in Kbps, the transfering
	///                       should not exceed this given max bitrate, 0 if no limitation
	/// @return		the push URL to provision this Content with
	/// @remarks	this method is allowed to called multiple times before the scheduled provision starts
	/// @throws 1) NotImplemented: not implemented \n
	///			2) InvalidStateOfArt: content exists but no overwriting is allowed \n
	///         3) NoResourceException: No resource to do this provision
	///         4) InvalidParameter: unsupported content type \n
	///         5) InvalidParameter: stopTime is piror to the startTime \n
	///         6) InvalidParameter: unsupported source URL \n
	string provisionPassive(string sourceContentType,
		bool  overwrite,
		string startTimeUTC,
		string stopTimeUTC,
		int    maxTransferBitrate)
		throws InvalidParameter, NoResourceException, InvalidStateOfArt, NotImplemented;
	
	/// The cancel provision operation will stop current content's provision task, 
	/// if the the content status is in provisioning status, the task will be cancelled, the related content file are deleted
	/// if the content has been in provsioned status, this operation will throw InvalidStateOfArt exception.
	/// @throws 1) InvalidStateOfArt: content provision can not be cancelled \n
	///            Possible error code: csexpCodeContentNotFound, csexpCodeContentIsReady, esexpCodeInternalError. 
	///         2) NotImplemented: Not implement in current version
	void cancelProvision()
		throws InvalidStateOfArt, ServerError, NotImplemented;
		
	/// get the access to the volume where this content is on
	///@return the access to the parent volume
	///@note no white space is allowed in the id string
	["cpp:const", "freeze:read"] idempotent Volume* theVolume()
		throws NotImplemented;

	/// before the provisioning gets completed, it is allowed to adjust the schedule start and stop time to meet real-time
	/// encoding scenarios
	///@param[in] startTimeUTC      - the new scheduled start time, values are in UTC format, empty if keep the original value.
	///                               it indicates to start provisioning right the way if startTimeUTC is adjusted to a moment before now
	///@param[in] stopTimeUTC       - the new scheduled stop time, values are in UTC format, empty if keep the original value
	///                               it indicates to stop provisioning right the way if stopTimeUTC is adjusted to a moment before now
	///@throws 1) NotImplemented: not implemented \n
	///        2) InvalidStateOfArt: content is not in the state of csProvisioning or csProvisioningStreamable
	///        3) InvalidParameter if the given UTC is illegal in the format of ISO8601
	///@sa provision(), provisionPassive()
	void adjustProvisionSchedule(string startTimeUTC, string stopTimeUTC)
		throws InvalidParameter, NotImplemented, InvalidStateOfArt;
};

// -----------------------------
// struct ContentInfo
// -----------------------------
/// The content information exported from the ContentStore
struct ContentInfo
{
	string name;  ///< the full name of the content
	string fullname;  ///< the short content name relative to the volume it belongs to
	ContentState state;  ///< the state of the content
	TianShanIce::Properties metaData; ///< the content metaData that are required
};

/// a collection of ContentInfo
sequence <ContentInfo> ContentInfos;

// -----------------------------
// struct FolderInfo
// -----------------------------
/// The folder information
struct FolderInfo
{
	string name;  ///< the name of the folder
	
	TianShanIce::Properties metaData; ///< metadata of the folder
};

/// a collection of FolderInfo
sequence <FolderInfo> FolderInfos;

// -----------------------------
// interface Folder
// -----------------------------
///@brief represents a folder of the storage on the ContentStore.
/// A folder may be physical or virtual, no content can be store across folders
interface Folder
{
	/// get a unique folder name of this ContentStore instance hosts
	///@return the value of this folder
	///@note no white space is allowed in the id string
	["cpp:const"] string getName()
		throws NotSupported; // throw when default folder of CS is not available
		
	/// list the content based on the wildcard
	///@param[in] condition the seaching condition, support wildcard(* and ?)
	///@return  the content list
	///@throws InvalidStateOfArt the Folder is not ready to respond the operation
	["cpp:const"] StrValues listContent(string condition)
	    throws InvalidStateOfArt, ServerError, NotSupported;

	/// the enhanced API to list contents.
	///@param[in] metaDataNames specify the name of returning meta data of each content \n
	///@param[in] startName the start content name. all the content name listed in the result will be no smaller than it. empty if 
	///                    wish to list from the beginning
	///@param[in] maxCount the max count of the contents in the list result, 0 or negative if wish not to specify the limitation
	///@return a collection of ContentInfo
	///@sa @ref sec_tsStContMD "Content Meta Data" describes some known content metadata names
	///@note some expected metadata may not be presented in the returning data if they are unavailalbe in the content
	///@throw ServerError
	["cpp:const", "amd", "ami"]
	ContentInfos listContents(TianShanIce::StrValues metaDataNames, string startName, int maxCount)
		throws ServerError, NotSupported;

	/// open a content of the current folder, it also covers the method to create a new content
	///@param[in] name the short name of the content about to open, be aware that this short name is not a full name but a 
	///           relative name based on the current folder
	///@param[in] destinationContentType the type of content about to create as if createIfNotExist=true. This input parameter
	///           would be ignored if createIfNotExist=false or the content has already existed
	///@param[in] createIfNotExist true if to create a new content if no matched
	///@return pointer to the content object, NULL if the content doesn't exist
	///@throws 1) InvalidParameter: the destinationContentType is not supported
	///        2) NotImplemented: not implemented \n
	Content* openContent (string name, string destinationContentType, bool createIfNotExist)
		throws InvalidParameter, NotImplemented;

	/// open or create a sub-folder within this folder
	///@param[in] subname sub-name of the virtual folder to create, the fullname of this new folder would be "<ThisFolderName>/<subname>"
	///@param[in] createIfNotExist true if need to create the new virtual folder when it is not exists
	///            physical storage
	///@param[in] quotaSpaceMB the quota space assigned to this virtual folder, 0 or negative if to specify unlimited and up to the
	///            physical storage. effective only when createIfNotExist=true. 
	///@return the newly created folder, NULL if the folder doesn't exist
	///@note no white space is allowed in the subname
	Folder* openSubFolder(string subname, bool createIfNotExist, long quotaSpaceMB)
		throws InvalidParameter, NotSupported, NotImplemented;

	/// get the access to the parent folder
	["cpp:const"] idempotent Folder* parent()
		throws NotImplemented;

	/// list the sub folders in the this folder
	///@return a collection of FolderInfo that matches the list conditions
	["cpp:const"] FolderInfos listSubFolders()
		throws NotImplemented;
		
	/// destroy the folder, the user must make sure that the folder is empty
	/// @throws 1) NotSupported, persistent folder is not supported to delete \n
	///			2) InvalidStateOfArt, folder is not allowed to delete if it is not empty
	void destroy()
		throws NotSupported, InvalidStateOfArt;

};

// -----------------------------
// interface Volume
// -----------------------------
///@brief represents a volume of the storage on the ContentStore.
/// A volume may be physical or virtual, no content can be store across volumes
interface Volume extends Folder
{
	/// get a unique volume name of this ContentStore instance hosts
	///@return the value of this volume
	///@note no white space is allowed in the id string
	["cpp:const"] string getVolumeName()
		throws NotSupported; // throw when default volume of CS is not available
		
	/// query the capacity of the volume
	///@param[out] freeMB the free space in MegaByte
	///@param[out] totalMB the total space in MegaByte
	///@throws InvalidStateOfArt the Volume is not ready to respond the operation
	///@throws ServerError API error failed to get the capacity info
    void getCapacity (out long freeMB, out long totalMB)
		throws InvalidStateOfArt, ServerError, NotSupported;
};

// -----------------------------
// struct VolumeInfo
// -----------------------------
/// The content information exported from the ContentStore
struct VolumeInfo
{
	string name;  ///< the full name of the volume
	bool   isVirtual; ///< true if this is a virtual volume created thru Volume::openSubVolume()
	long   quotaSpaceMB;  ///< the quota space size assigned for the volume
	
	TianShanIce::Properties metaData; ///< metadata of the volume
};

/// a collection of ContentInfo
sequence <VolumeInfo> VolumeInfos;

// -----------------------------
// Service ContentStore
// -----------------------------
///@brief represents the basic access to manage the storage and delivery of Content files.
///
/// The ContentStore is a service intends to represent the basic 
/// functionality to manage the storage and delivery of Content files. The functions it publishes
/// for such use are equivalent to virtual functions, which must be implemented by any derived classes.\n
/// ContentStore will redirect the invocations of the interface Volume it inherited to the default volume if there is one
interface ContentStore extends TianShanIce::BaseService, Volume
{
	/// get a network-wide unique id of this ContentStore instance,
	/// normally this value could be read from the configuration of the ContentStore
	///@return a network-wide unique id
	///@note no white space is allowed in the id string
	["cpp:const"] string getNetId();

	/// get the type of this content store
	///@return the typeof the store
	["cpp:const"] string type();
	
	/// check if the content store is functinal
	///@return true if the store is healthy and ready to access
	["cpp:const"] bool isValid();
	
	/// get the access to a specific volume in the ContentStore
	///@param[in] name full name of the volume to open
	/// normally this value could be read from the configuration of the ContentStore
	///@return a network-wide unique id, NULL if the volume doesn't exist
	///@note no white space is allowed in the id string
	["cpp:const"] Volume* openVolume(string name)
		throws InvalidParameter, InvalidStateOfArt, NotImplemented;

	/// list the volumes on the ContentStore
	///@param[in] listFrom the name of the parent volume name to list, "/", "*" or empty means list from the store root
	///@param[in] includingFolders true if to include the folders in the returning result
	///@return a collection of VolumeInfo that matches the list conditions
	///@throws InvalidStateOfArt the ContentStore is not ready to respond the operation
	["cpp:const", "amd", "ami"]
	VolumeInfos listVolumes(string listFrom, bool includingFolders)
		throws InvalidStateOfArt, NotImplemented;
		
	/// open an existing content in the store by its full content name including the volume name specified
	///@param[in] fullname full content name of the existing content about to open
	///@return pointer to the content object
	///@throws InvalidStateOfArt the ContentStore is not ready to respond the operation, or content is at an inaccessiable state
    ["cpp:const"] Content* openContentByFullname (string fullname)
		throws InvalidStateOfArt;

	/// query the cache level for the content store
	/// Some storage itself may be built with stream server, those storage may be used as a cache
	/// to buffer some content for streaming from local. The cache should have a level to prioritize
	///@return the cache level of this storage, 0 - highest
	///@throws NotSupported if the content store wont run as cache mode
	["cpp:const"] byte getCacheLevel()
		throws NotSupported;

	/// If the store has attached StreamServices to work as a cache-able streaming service. This
	/// method can provide the access to the attached StreamServices interfaces. It is pretty much like
	/// the interface ReplicaQuery
	/// @ return a collection of Replica information. In which, Replica::category equals StreamService and 
	///          Replica::priority equals the cacheLevel as getCacheLevel.
	///@throws NotSupported if the content store is storage only or is not allowed to expose the StreamServices
	["cpp:const"] Replicas getStreamServices()
		throws NotSupported;
};

/*!
@section sec_tsStContMD Content Meta Data
@subsection ssec_tsStSysMD System Reserved Meta Data
The client of ContentStore can query but are not allowed to modify the system meta data
<table>
<tr><th align=left>Name</th>     <th align=left>Description</th></tr>
<tr><td>sys.SourceUrl</td>       <td>where the content was provisioned from</td></tr>
<tr><td>sys.ParentName</td>      <td>asset or package that this Content belongs to if available</td></tr>
<tr><td>sys.Comment</td>         <td>string comment of the content</td></tr>
<tr><td>sys.FileSize</td>        <td>file size in bytes</td></tr>
<tr><td>sys.SupportFileSize</td> <td>file size subtotal, in bytes, of supplemental files excluding the main file</td></tr>
<tr><td>sys.PixelHorizontal</td> <td>picture resoultion</td></tr>
<tr><td>sys.PixelVertical</td>   <td>picture resoultion</td></tr>
<tr><td>sys.BitRate</td>         <td>the bitrate of the Content</td></tr>
<tr><td>sys.PlayTime</td>        <td>play time in msec</td></tr>
<tr><td>sys.FrameRate</td>         <td>framerate in fps</td></tr>
<tr><td>sys.SourceType</td>         <td>source encode type when the content is provisioned from</td></tr>
<tr><td>sys.LocalType</td>         <td>local encode type after save the content into local storage</td></tr>
<tr><td>sys.SubType</td>         <td>sub type in addition to the local encode type</td></tr>
<tr><td>sys.MD5CheckSum</td>         <td>the MD5 checksum of the main file</td></tr>
<tr><td>sys.ScheduledProvisonStart</td>         <td>the schduled start time, in IS8601, of the content provisioning</td></tr>
<tr><td>sys.ScheduledProvisonEnd</td>         <td>the schduled start time, in IS8601, of the content provisioning</td></tr>
<tr><td>sys.MaxProvisonBitRate</td>         <td>the maximal bitrate limitation, in bps, set for the provisioing procedure</td></tr>
<tr><td>sys.nPVRLeadCopy</td>   <td>For nPVR copy only, to indicate the fullpath of the lead nPVR copy on the filesystem, no index file extname will be included</td></tr>
<tr><td>sys.StampLastUpdated</td>   <td>when the content has been newly updated</td></tr>
</table>

@subsection ssec_tsStUsrMD Known User's Meta Data
The client of ContentStore can query and modify the user's meta data
<table>
<tr><th align=left>Name</th>       <th align=left>Description</th></tr>
<tr><td>[user.]ProviderId</td>     <td>the Provider ID, defined by CableLabs, of the content</td></tr>
<tr><td>[user.]ProviderAssetId</td><td>the Provider-Asset-ID, defined by CableLabs, of the content</td></tr>
<tr><td>[user.]nPVRCopy</td>       <td>non-zero if the content is a private nPVR copy</td></tr>
<tr><td>[user.]SubscriberId</td>   <td>The owner's subscriberID if the Content is a private nPVR copy</td></tr>
</table>

*/


// ****************************************************************************************
// REFERENCE CONSTANTS
// The following are some reference constants exists in the kown SeaChange implementations
// ****************************************************************************************

// -----------------------------
//  types of ContentStore
//  SubContent type could follow the ContentType, seperated by colon(:) to specify
//  the ContentType and its sub type, if no sub type, just use ContentType
// -----------------------------
const string csNTFS             = "NTFS";                      ///< raw NTFS disk filesystem
const string csNAS				= "NAS";					   ///< NAS store
const string csSCMediaClusterCache = "SeaChange.MediaCluster.cache"; ///< cache mode
const string csSCMediaCluster   = "SeaChange.MediaCluster";    ///< SeaChange's MediaCluster with RAID5^2 technology
const string csSCAxiomVOD       = "SeaChange.Axiom";           ///< SeaChange's full deployment, the combination of Axiom and MediaClusters
const string csSCBMLe           = "SeaChange.BMLe";            ///< SeaChange's BMLe
const string csIPSAN            = "IPSAN";                     ///< Generic IP SAN storage
const string csUnknown          = "Unknown";                   ///< Unknown type of the content store

// -----------------------------
//	ContentType
//  SubContent type could follow the ContentType, seperated by colon(:) to specify
//  the ContentType and its sub type, if no sub type, just use ContentType
// -----------------------------
const string ctRawFile              = "RawFile";               ///< raw disk file
const string ctMPEG2TS              = "MPEG2TS";               ///< MPEG2 TS file
const string ctAudioTS              = "AudioTS";               ///< Audio TS file
const string ctES                   = "ES";                    ///< ES streaming file
const string ctH264                 = "H264";                  ///< H.264 media file
const string ctAVS                  = "AVS";                   ///< AVS media file
const string ctWMV                  = "WMV";                   ///< Microsoft WMV media file
const string ctVodCastArchive       = "VodCastArchive";        ///< Vodcast archive file
const string ctDODTS                = "DODTS";                 ///< DataOnDemand Ts file
const string ctUnknown              = "Unknown";               ///< Unknow type of content
const string ctCSI					= "CSI";                   ///< auto detect type of content


const string ctMPEG2TSVVX           = "MPEG2TS:VVX";           ///< SeaChange spcification: MPEG2 TS with VVX index File
const string ctMPEG2TSVV2           = "MPEG2TS:VV2";           ///< SeaChange spcification: MPEG2 TS with VV2 index File

//-----------------------------
//	Content Sub Type
//-----------------------------
const string subctVVX               = "VVX";                   ///< SeaChange VVX type
const string subctVV2               = "VV2";                   ///< SeaChange VV2 type


// -----------------------------
//	Content Import and Export Protocol
// -----------------------------
const string potoFTP                = "ftp";                   ///< FTP protocol
const string potoNFS                = "nfs";                   ///< Network File Share(NFS) protocol
const string potoCIFS               = "cifs";                  ///< Common Internet File System(CIFS) protocol
const string potoPGM                = "pgm";                   ///< Pragmatic General Multicast(PGM) protocol

// -----------------------------
//	Property of Content Export
// -----------------------------
const string expFileCount           = "FileCount";             ///< FileCount for the export
const string expFileName            = "FileName";              ///< The real property name is FileName with number, the number is depends on FileCount, index is from 1
const string expUserName            = "UserName";              ///< User name to export the content
const string expPassword            = "Password";              ///< Password to export the content
const string expTTL                 = "TTL";                   ///< Number of seconds that the client has to open the FTP connection or transferring the file.
const string expTransferBitrate     = "TransferBitrate";       ///< The max tranfer bitrate to export the content
const string expTimeWindowStart     = "TimeWindowStart";       ///< Start time of the expose time window, in ISO8601 format
const string expTimeWindowEnd       = "TimeWindowEnd";         ///< End time of the expose time window, in ISO8601 format


// -----------------------------
//	Exception Code of ContentStore
// -----------------------------

///< 200 OK, no error happen
const int cs200OK     		= 200;                     

///< No specified error code for the exception
const int csexpNotSpecified     = 300;                     

///< Invalid parameter in the request
// invalid parameter might be empty name, invalid time
const int csexpInvalidParam     = 1000;                     

///< Unauthorized account, bad username or passowrd
const int csexpUnauthorized     = 1001;                     

///< Can not find the source content from the specified URL
const int csexpSourcNotFound    = 1002;                     

///< The speified protocol is not supported
const int csexpUnsupportProto   = 1003;                     

///< The speified content can not be found in contentstore
const int csexpContentNotFound  = 1004;                     

///< The cancel failed since the content has been ready
// content already provsioned, overwrite or cancel not allowed 
const int csexpContentIsReady   = 1005;                     

///< The exported content is not ready
const int csexpContentNotReady  = 1006;                     

///< The content file is in use
// content is being streamed, read, or written
const int csexpContentInUse     = 1007;                     

///< There is no resource to take the request 
// The module that supposed to complete this request can't be reached.
const int csexpNoResource       = 1008;                     

///< Interal server error 
// ICE internal error, Network error, Runtime Error
const int csexpInternalError    = 1009;       

///< invalid provision time 
const int csexpInvalidTime      = 1010;

///< invalid content type
const int csexpInvalidContentType = 1011;

///< invalid source url
const int csexpInvalidSourceURL   = 1012;               

//-----------------------------
//	Error Code of Content Provision
//-----------------------------
///< vstream disk full
const int csexpVstrmDiskFull   = 2001;               

///< vstream bandwidth exceeded
const int csexpVstrmBwExceeded   = 2002;               

///< vstream not ready
const int csexpVstrmNotReady   = 2003;               

///< user canceled provision
const int csexpUserCanceled   = 2004;               

/* need to clean up
enum ContentProvisionStatus
{
	cpsSetup,                    /// the provision has been setup
	cpsStart,                    /// the provision has been started
	cpsStreamable,               /// the content is streamable
	cpsProvisioned,              /// the content has been provisioned
	cpsFailed,                   /// the provision failed
	cpsDestroyed                 /// the content is destroyed
};
*/

};
};

#endif // __ZQ_TianShanIce_Storage_ICE__
