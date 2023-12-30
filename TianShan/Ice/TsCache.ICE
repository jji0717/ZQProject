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
// Ident : $Id: TsCache.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsCache.ICE $
// 
// 12    7/10/13 3:14p Hui.shao
// 
// 11    11/02/12 3:00p Hui.shao
// export the listNeighbors() from the old hidden API
// 
// 9     7/25/12 4:08p Hui.shao
// added a new api to list hot locals
// 
// 8     6/13/12 6:19p Hui.shao
// 
// 7     6/13/12 4:52p Hui.shao
// 
// 6     4/25/12 4:32p Hui.shao
// 
// 5     4/09/12 2:11p Hui.shao
// 
// 4     1/18/12 1:50p Hui.shao
// 
// 3     1/16/12 10:31p Hui.shao
// 
// 2     12/30/11 2:28p Hui.shao
// 
// 1     12/08/11 2:13p Hui.shao
// created
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_Cache_ICE__
#define __ZQ_TianShanIce_Cache_ICE__

#include "TianShanIce.ICE"
#include "TsEvents.ICE"
#include "TsStorage.ICE"
#include "TsStreamer.ICE"

#include <Ice/Identity.ice>

module TianShanIce
{

// -----------------------------
// namespace Storage
// -----------------------------
/// Storage represents a media content storage within the TianShan architecture
module Storage
{

interface CacheStore;

// -----------------------------
// struct CacheStoreDescriptor
// -----------------------------
/// struct to represent an instance of CacheStore
struct CacheStoreDescriptor
{
	string  netId;          ///< the unique netId of the CacheStore within the domain
	string  domain;         ///< the domain name of the CacheStore that belongs to
	TianShanIce::State state; ///< the state of the CacheStore

	int    loadStream;      ///< the load value of the CacheStore as of the stamp of stampAsOf.
	                        ///< this load value minimally covers the downstream load but may be not limited to cover
	                        ///< those such as disk read, cpu and memory
	int    loadImport;      ///< the load value of import usage, including that of cache and pass-thru
	int    loadCacheWrite;  ///< the load value of disk write that used for caching external contents to the server
	long   stampAsOf;       ///< the timestamp of load of the CacheStore
	
	CacheStore* theStore;  ///< the access to the CacheStore
	string      sessionInterface; ///< the session interface that can setup stream sessions of contents on the CacheStore
};

// -----------------------------
// struct CacheCandidate
// -----------------------------
/// a CacheCandidate of a given content
struct CacheCandidate
{
	CacheStoreDescriptor csd;
	
	bool isSelf;
	bool isContentEdge;
	long rawContDistance; ///< represents the raw distance for a specified content to be cached in a given
	                      ///< CacheStore up to DHT regardless load
	int  distance;        ///< represents the integraded cost for a specified content served by a given
	                      ///< CacheStore by considering both contentDistance and the load all over the
	                      ///< CacheStore within the neighborhood or domain
};

/// a collection of CacheCandidates
sequence< CacheCandidate > CacheCandidates;

/// a CacheStore neighborhood is represented as a list of cachestore, plus their common domain name separately
sequence< CacheStoreDescriptor > CacheStoreList;


// -----------------------------
// struct ContentAccess
// -----------------------------
/// a descriptor on access count of a given content
struct ContentAccess
{
	string contentName;   ///< the name of content
	int    accessCount;   ///< the access counted since stampSince, maps to METADATA_AccessCount of Content rec
	// int    countInHotWin; ///< the access count known in the hot timewindow
	long stampSince;    ///< the time since when the access was counted, maps to METADATA_AccessCountSince of Content rec
	long stampLatest;    ///< the latest timestamp  when the access recently was counted, maps to METADATA_AccessCountLatest of Content rec
};	

/// a collection of ContentAccesses
sequence< ContentAccess > ContentAccessList;

// -----------------------------
// service CacheStore
// -----------------------------
/// the CacheStore extends ContentStore with the extentions to receive the file events from the proxied filesystem
interface CacheStore extends ReplicaQuery
{
	/// a CacheStore proxies a ContentStore as an additional interface. this API provide the access to the
	/// ContentStore instance proxied
	///@return access to the ContentStore
	ContentStore* theContentStore();
	
	/// to enumerate the CacheCandidates for a given Content
	CacheCandidates getCandidatesOfContent(string contentName, bool withLoadConsidered)
		throws ClientError, ServerError;
	
	/// to export a Content as a Stream session
	///@param[in] contentName the name of Content to export
	///@param[in] subFile     the subfile extension name of Content to export in this stream.
	///                       Empty value is reserved for the main file of the content
	///                       Value "*" is reserved to allow export any subfiles of the specified Content. When
	///                       the stream source is represented as a URL, it is allowed to concatenate a
	///                       "?subfile=..." onto the URL to specify a subfile of a download
	///@param[in] cacheStoreDepth  to indicated whether this invocation is from forwarded by another CacheStore
	///                       0 - the invocation is issued originally by a ServiceEdge
	///                       non-0 the invocations may be forwarded form other CacheStores, usually it is not recommended
	///                       to forward again from the invokee
	///@param[in] idleStreamTimeout the timeout value in millisecond to indicate StreamService that the created
	///                      Stream session can terminated if idle at no downloading
	///@param[in] resources  the resources that can be employed by the stream that is being created. The stream may use or not use
	///                      the given resource but should not beyond them
	///@param[in] params     the optional parameters if appliable
	///                      wishedStreamProtocol - the expected streaming protocol to the CacheStore that the client wish to take
	///                      bitrate - the transfer bitrate wished
	///                      ...
	///@return access to the Stream if succeeded, NULL if failed
	Streamer::Stream* exportContentAsStream(string contentName, string subFile, int idleStreamTimeout, int cacheStoreDepth, TianShanIce::SRM::ResourceMap resources, TianShanIce::Properties params)
		throws ClientError, ServerError, InvalidStateOfArt;

	/// to query popularity of a given content
	///@param[in] contentName the name of Content to export
	///@param[out] since      the time, in the format of ISO8601, since when the content is counted by this CacheStore
	///@return the amount of times that the content has been accessed from this CacheStore
	ContentAccess getAccessCount(string contentName);

	/// to add access count of a given content
	///@param[in] contentName the name of Content to query
	///@param[in] accessCount the amount of times that the content has been accessed from this CacheStore to add
	///@param[in] since      the time, in the format of ISO8601, since when the content is counted by this CacheStore
	///@return true if the content is known in this CacheStore
	["ami"] void addAccessCount(string contentName, int countToAdd, string since);

	/// to query for the missed contents known by the CacheStore
	///@param[in] maxNum      the max number of ContentAccess wish to receive, default 20 if non-positive
	///@return a list of missed contents, no more than the specified maxNum, from top wished to less
	["amd"] ContentAccessList listMissedContents(int maxNum);
	
	/// to list the hot local contents in CacheStore
	///@param[in] maxNum      the max number of ContentAccess wish to receive, default 20 if non-positive
	///@return a list of hot local contents, no more than the specified maxNum, from top wished to less
	["amd"] ContentAccessList listHotLocals(int maxNum);

	/// To set the threshold of access count for missed content to be cached
	///@param[in] timeWinOfPopular to specify the float time window, in seconds, to count the access
	///@param[in] countOfPopular  in the resent time window specified by timeWinOfPopular, A content is known as popular
	///                           if its access count exceeds what countOfPopular specifies
	///@param[in] hotTimeWin      as the most recent part, in milliseconds, of timeWinOfPopular. A content is known as hot
	///                           if the access count in this window reaches round(countOfPopular *4 * hotTimeWin / timeWinOfPopular) +1
	///                           0 or negative - unnecessary to measure popularity specially for hot window
	void setAccessThreshold(int timeWinOfPopular, int countOfPopular, int hotTimeWin);

	/// get the threshold settings of access count for missed contents
	///@param[in] timeWinOfPopular to specify the float time window, in seconds, to count the access
	///@param[in] countOfPopular  in the resent time window specified by timeWinOfPopular, A content is known as popular
	///                           if its access count exceeds what countOfPopular specifies
	///@param[in] hotTimeWin      as the most recent part, in milliseconds, of timeWinOfPopular. A content is known as hot
	///                           if the access count in this window reaches round(countOfPopular *4 * hotTimeWin / timeWinOfPopular) +1
	void getAccessThreshold(out int timeWinOfPopular, out int countOfPopular, out int hotTimeWin);
	
	/// user demand to cache a content into local
	///@param[in] contentName the name of Content to cache
	///@param[in] sourceStore  the source CacheStore to propage the content from. NIL if up to the CacheStore determine
	///                       which source storage to pull the content from
	///@param[in] params     the optional parameters if appliable
	///@note this invocation does NOT guarantee the success of the cache execution. The request may be given up
	///                       by the CacheStore for reasons, such as run out of import resources, no source content
	///                       associated
	void cacheContent(string contentName, CacheStore* sourceStore, TianShanIce::Properties params)
		throws ClientError, ServerError, InvalidStateOfArt;
	
	// Utility functions
	// -----------------------------
	/// Utility function that calculate the distance between a given content and a store netId
	///@return the distance sqr
	long calculateCacheDistance(string contentName, string storeNetId);

	/// Utility function that converts the inputed content name to the path name of in the cache with hashed folders
	///@param[in] contentName the name of Content to export
	///@param[in] subFile     the subfile extension name of Content to export in this stream.
	///                       Empty value is reserved for the main file of the content
	///@return a string path name of the hashed folder under the home directory
	string getFolderNameOfContent(string contentName);

	/// Utility function that converts the inputed content name to the full path name of in the local cache with hashed folders
	///@param[in] contentName the name of Content to export
	///@param[in] subFile     the subfile extension name of Content to export in this stream.
	///                       Empty value is reserved for the main file of the content
	///@return a string full path name with hashed folders on the storage filesystem
	string getFileNameOfLocalContent(string contentName, string subfile);
	
	/// list all the stores, including self, in the same domain
	///@param[in] timeout to specify a timeout threshold in msec. The stores from which failed to hear heartbeat within this
	///                   timeout window will be execluded from the returning CacheStoreList. 0 or negative to take the configured value
	///@return a CacheStoreList collection
	CacheStoreList listNeighors(int timeout);
};

};
};

#endif // __ZQ_TianShanIce_ContProv_ICE__
