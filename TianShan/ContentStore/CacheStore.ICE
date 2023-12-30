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
// Ident : $Id: CacheStore.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheStore.ICE $
// 
// 17    11/02/12 3:01p Hui.shao
// exported the listNeighbors() API to SDK
// 
// 16    9/18/12 2:07p Hui.shao
// enh#16995 - Serve the cached copy that is catching up the PWE copy on
// source storage
// 
// 15    7/24/12 2:47p Hui.shao
// stat counters on hitrate
// 
// 14    7/19/12 7:36p Hui.shao
// adjusted counter list compress logic
// 
// 13    6/26/12 5:03p Hui.shao
// FWU to faster find other folders for free space
// 
// 12    6/15/12 3:05p Hui.shao
// folder space
// 
// 11    6/13/12 11:42a Li.huang
// 
// 10    6/06/12 4:03p Hui.shao
// to print the count result in the log
// 
// 9     5/10/12 6:50p Hui.shao
// kick off cache missed content per popularity
// 
// 8     4/27/12 5:56p Hui.shao
// 
// 7     4/25/12 4:36p Hui.shao
// drafted most unpopular list of TopFolder
// 
// 6     4/17/12 12:03p Hui.shao
// 
// 5     3/22/12 5:13p Hui.shao
// 
// 4     1/18/12 1:50p Hui.shao
// linkable
// 
// 3     1/16/12 10:35p Hui.shao
// defined the counter and topfolder at ice level
// 
// 2     12/29/11 8:27p Hui.shao
// cacheContent()
// 
// 1     12/22/11 1:38p Hui.shao
// created
// ===========================================================================
// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice --output-dir .. ../$(InputName).ice
// $(ICE_ROOT)\bin\slice2freeze.exe -I../ -I$(ICE_ROOT)/slice/ -I$(ZQProjsPath)/TianShan/Ice --index "TianShanIce::Storage::ChildVolume,TianShanIce::Storage::VolumeEx,parentVolName,case-sensitive" ChildVolume --output-dir .. ../$(InputName).ice

// outputs:
//	../$(InputName).h
//	../$(InputName).cpp

#ifndef __ZQ_TianShanIce_CacheStore_ICE__
#define __ZQ_TianShanIce_CacheStore_ICE__

#include <TsCache.ICE>
#include <ContentStore.ICE>
#include <../common/TianShanUtils.ICE>

module TianShanIce
{

module Storage
{

// -----------------------------
// struct AccessCounter
// -----------------------------
struct AccessCounter
{
	ContentAccess base;
	string folderName;
	
	long       knownSince;
	long       fileSizeMB;

	::TianShanIce::Storage::ContentState localState;
};

dictionary < string, AccessCounter > ContentCounterMap; //< map of contentName (not the localReplicaName) to ActiveContent record
sequence   < AccessCounter > ContentCounterList;

// -----------------------------
// class AccessRegistrar
// -----------------------------
class AccessRegistrar
{
	ContentAccess count(string contentName, string subfile, int countToAdd);
	void set(AccessCounter counterToOverwrite);
	void erase(string contentName);
	bool get(string contentName, out AccessCounter counter);
	int  compress(int windowSize, int flushWinSize, out int reqsInWindow, out ContentCounterList listToFlush, out ContentCounterList listEvicted);
	void sort(int windowSize, bool unpopular1st, int minCount, out ContentCounterList resultList);
	int  size();
	
	string             name;
	ContentCounterMap  counters;
}; 

// -----------------------------
// class TopFolder
// -----------------------------
["freeze:write"]
class TopFolder implements TianShanUtils::TimeoutObj
{
	Ice::Identity			ident; //< ident.name is the name of the top folder
	long                    stampLastRefresh;
	TianShanIce::StrValues  unpopularQueue;
	AccessRegistrar         unpopulars;
	int                     maxUnpopular;
	
	/// for the case that a content has been hit or destoryed per active invocations from external
	bool eraseFromUnpopular(string fullContentName);

	/// returns the content names that have been deleted
	::TianShanIce::StrValues freeForSpace(long wishedSpaceMB, out long freedMB);

	/// returns the content names that have been deleted
	bool getSpaceUsage(out long totalMB, out long freeMB, out long usedMB);

	int unpopularSize();
	void doRefreshUnpopular();

// 	long                    totalSpaceMB;
	long                    usedSpaceMB;
	IValues                 contentsOfleaves;
	int                     contentSubtotal;
};

// -----------------------------
// DB record CacheTask
// -----------------------------
["freeze:write"]
class CacheTask implements TianShanUtils::TimeoutObj
{
	Ice::Identity   ident;
	UnivContent*    localContent;
	Properties      nameFields;

	Streamer::Stream*  srcStream;
	string   urlSrcStream;
	string   srcContentType;
	bool     isSrcPWE;
	
	int      bwMin;  // min speed in bps
	int      bwMax;  // max speed in bps
	int      bwCommitted;
	// long     fileSizeMB;
	long     startOffset; // file start offset in bytes
	long     endOffset;   // file end offset in bytes
	
	// ISO8601 times about scheduled-start and end
	string   scheduledStart;
	string   scheduledEnd; 
	
	ContentProvision::ProvisionSession* provisionSess;

	void OnRestore();
	void destroy();
	
	long     stampCreated;
	long     stampCommitted;
	long     stampStopped;
};

// -----------------------------
// service CacheStoreEx
// -----------------------------
/// the ContentStoreEx extends CacheStore with the extentions to receive the file events from the proxied filesystem
interface CacheStoreEx extends CacheStore, ReplicaQuery, TianShanUtils::TimeoutObj
{
	
/*
	/// entries to receive the file events
	["ami"] void OnFileEvent(FileEvent event, string fileName, Properties params);
	["ami"] void OnProvisionEvent(ProvisionEvent event, string storeNetId, string volumeName, string contentName, Properties params);

	["ami"] void wakeupContent(string contentFullname);
	
	void enableAutoFileSystemSync(bool enable);
*/
};

};
};

#endif // __ZQ_TianShanIce_CacheStore_ICE__
