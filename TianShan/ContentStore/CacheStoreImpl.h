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
// Ident : $Id: CacheStoreImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheStoreImpl.h $
// 
// 68    4/14/17 1:59p Hui.shao
// _contentAttrCache
// 
// 67    2/28/17 2:24p Hui.shao
// 
// 66    2/23/17 3:03p Hongquan.zhang
// 
// 65    1/11/16 5:42p Dejian.fei
// 
// 64    4/03/15 2:39p Hui.shao
// thirdPartyCache for Squid
// 
// 63    3/25/15 4:12p Hui.shao
// 
// 62    3/20/15 11:57a Hongquan.zhang
// 
// 61    1/23/15 11:44a Hui.shao
// _bProxySessionBook: 1) original CDNSS w/CacheStore set to true, 2)
// StreamSegmenter and C2FE set to false
// 
// 60    7/10/13 4:03p Hui.shao
// enh#18206 - CacheServer selection policy per UML-E's local-read cost
// needs considered
// 
// 59    11/26/12 2:51p Hui.shao
// added snmp counters to export
// 
// 58    11/02/12 3:04p Hui.shao
// to include self at listNeighbor() by default
// 
// 57    11/01/12 4:06p Zonghuan.xiao
// add  export  method  for hot  and missed contents
// 
// 56    9/19/12 3:13p Hui.shao
// 
// 55    9/18/12 2:07p Hui.shao
// enh#16995 - Serve the cached copy that is catching up the PWE copy on
// source storage
// 
// 54    9/05/12 12:06p Hui.shao
// flag logging
// 
// 53    8/06/12 8:48p Hui.shao
// export _freeSpacePercent as configurable
// 
// 52    8/06/12 12:50p Hui.shao
// store::OnTimer to call ensureSpace()
// 
// 51    7/27/12 10:27a Hui.shao
// changed maxSpace from GB to MB
// 
// 50    7/26/12 7:20p Hui.shao
// enh#16577 to limit max diskspace via configuration
// 
// 49    7/26/12 6:43p Hui.shao
// firxed domain barker/probe's bind address
// 
// 47    7/25/12 4:12p Hui.shao
// added api to list hot locals
// 
// 46    7/25/12 3:56p Li.huang
// fix bug  16726 
// 
// 45    7/25/12 12:19p Hongquan.zhang
// set _groupBind as public
// 
// 44    7/24/12 2:47p Hui.shao
// stat counters on hitrate
// 
// 43    7/19/12 7:36p Hui.shao
// adjusted counter list compress logic
// 
// 42    7/19/12 5:35p Hui.shao
// added and refer to store-wide rootVolName
// 
// 41    7/18/12 1:54p Li.huang
// 
// 40    6/28/12 2:53p Hui.shao
// 
// 39    6/28/12 12:09p Li.huang
// 
// 38    6/28/12 9:59a Li.huang
// add temp directory to save index file
// 
// 37    6/26/12 5:03p Hui.shao
// FWU to faster find other folders for free space
// 
// 36    6/26/12 2:55p Hui.shao
// 
// 35    6/25/12 3:41p Hui.shao
// more logs at refreshing unpopulars
// 
// 34    6/20/12 2:13p Hui.shao
// 
// 33    6/15/12 3:05p Hui.shao
// folder space
// 
// 32    6/14/12 5:15p Hui.shao
// 
// 31    6/13/12 6:20p Hui.shao
// 
// 30    6/13/12 11:42a Li.huang
// 
// 29    6/11/12 7:23p Hui.shao
// 
// 28    6/11/12 5:22p Li.huang
// 
// 26    6/07/12 11:30a Hui.shao
// configuration schema
// 
// 25    6/06/12 4:03p Hui.shao
// to print the count result in the log
// 
// 24    5/29/12 2:21p Hui.shao
// added new portal portalBookTransferSession()
// 
// 23    5/10/12 6:50p Hui.shao
// kick off cache missed content per popularity
// 
// 22    4/27/12 6:38p Hui.shao
// CacheTask lifetime func
// 
// 21    4/26/12 6:32p Hui.shao
// the map of external source stores: reg exp of pid => c2 interface
// 
// 20    4/25/12 4:36p Hui.shao
// drafted most unpopular list of TopFolder
// 
// 19    4/23/12 2:55p Hui.shao
// configuration of up/down stream NIC, determin the destIP in
// cacheContent() automatically
// 
// 18    4/17/12 1:14p Li.huang
// 
// 17    4/11/12 10:37a Li.huang
// 
// 16    3/26/12 3:58p Li.huang
// 
// 15    3/22/12 5:13p Hui.shao
// 
// 14    1/19/12 12:09p Hui.shao
// 
// 13    1/18/12 6:24p Li.huang
// 
// 12    1/18/12 1:50p Hui.shao
// linkable
// 
// 11    1/17/12 11:35a Hui.shao
// linkable
// 
// 10    1/16/12 10:35p Hui.shao
// defined the counter and topfolder at ice level
// 
// 9     1/16/12 1:22p Hui.shao
// 
// 8     1/13/12 2:24p Hui.shao
// splitted counter and folder into separate cpp files
// 
// 7     1/12/12 7:02p Hui.shao
// 
// 6     1/06/12 2:55p Hui.shao
// 
// 5     1/06/12 2:01p Hui.shao
// 
// 4     1/06/12 11:53a Hui.shao
// 
// 3     12/30/11 2:28p Hui.shao
// 
// 2     12/29/11 8:27p Hui.shao
// cacheContent()
// 
// 1     12/22/11 1:38p Hui.shao
// created
// ===========================================================================

#ifndef __ZQTianShan_CacheStoreImpl_H__
#define __ZQTianShan_CacheStoreImpl_H__

#include "../common/TianShanDefines.h"
#include "Locks.h"
#include "Log.h"
#include "NativeThreadPool.h"

#include "CacheStore.h"
#include "ContentImpl.h"
#include "CacheDomain.h"
#include "CacheFactory.h"

#include "ContentUser.h" // for some known metadatas

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace ZQTianShan {
namespace ContentStore {

class CacheStoreImpl;
#define CACHESTORE_DEFAULT_GROUPIP      "225.23.23.23"
#define CACHESTORE_DEFAULT_GROUPPORT    (11122)

#define CACHE_FOLDER_SIZE               (1<<6)
#define CACHE_FOLDER_MASK               ((uint32)CACHE_FOLDER_SIZE-1)
//#define CACHE_FOLDER_MASK               (0x3f)
#define CACHE_DEFAULT_UnpopularSize     (1<<6)
#define CACHE_MIN_UnpopularSize         (20)

#define CACHE_SCOPED_INSTANCE_MAX      (8)
#define CACHE_LOAD_MAX                 (10000)
#define CACHE_DIST_MAX                 (10000)
#define CACHE_LOADTHRSH_DEFAULT_PREV   (8000)
#define CACHE_LOADTHRSH_DEFAULT_SUCR   (2000)

#define DOCACHE_TIMEOUT                (6*3600*1000) // give up if a content can't be cached within 6hr
#define PROVISION_PING_INTERVAL        (30*1000)     // 30sec

#define PENALTY_FORWARD_FAILED_DEFAULT (20)
#define PENALTY_FORWARD_MAX            (50)
#define DEFAULT_PROV_SPEED_Kbps        (4000) // 4000kbps

#define DEFAULT_PAID_LEN              (20)

#define METADATA_AccessCount           SYS_PROP(AccessCount)
#define METADATA_AccessCountSince      SYS_PROP(AccessCountSince)
#define METADATA_AccessCountLatest     SYS_PROP(AccessCountLatest)

#define CACHE_FOLDER_PREFIX "cache."
#define TOP_FOLDER_FMT       CACHE_FOLDER_PREFIX "T%02X"
#define LEAF_FOLDER_FMT      CACHE_FOLDER_PREFIX "L%02X"

#define MAX_LOAD (10000)

// -----------------------------
// class TopFolderImpl
// -----------------------------
//class TopFolderImpl : public TianShanIce::Storage::TopFolder, virtual public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class TopFolderImpl : public TianShanIce::Storage::TopFolder, virtual public ICEAbstractMutexRLock
{
	friend class RefreshUnpopularListCmd;

public:
	TopFolderImpl(CacheStoreImpl& store);
	// TopFolderImpl(CacheStoreImpl& store, const std::string& folderName, uint maxUnpopular=0);
	virtual ~TopFolderImpl();

protected: // impl of TopFolder

	/// for the case that a content has been hit or destoryed per active invocations from external
    virtual bool eraseFromUnpopular(const ::std::string& fullContentName, const ::Ice::Current& c);
	/// returns the content names that have been deleted
    virtual ::TianShanIce::StrValues freeForSpace(::Ice::Long, ::Ice::Long& freedMB, const ::Ice::Current& c);
    virtual ::Ice::Int unpopularSize(const ::Ice::Current& c);
//    virtual void checkUnpopular(const ::Ice::Current& c);
    virtual void doRefreshUnpopular(const ::Ice::Current& c);
    bool getSpaceUsage(::Ice::Long& totalMB, ::Ice::Long& freeMB, ::Ice::Long& usedMB, const ::Ice::Current& c);

protected: // impl of TimeoutObj
    virtual void OnTimer(const ::Ice::Current& c);

public:
	int64 _freeSpaceMB, _totalSpaceMB;

private:
	TianShanIce::Storage::FolderPrx _openFolder(const ::Ice::Current& c =::Ice::Current());

protected:

	CacheStoreImpl& _store;
	TianShanIce::Storage::FolderPrx _folder;
};

// -----------------------------
// entity CacheTaskImpl
// -----------------------------
//class CacheTaskImpl : public ::TianShanIce::Storage::CacheTask, virtual public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class CacheTaskImpl : public ::TianShanIce::Storage::CacheTask, virtual public ICEAbstractMutexRLock
{
public:
	typedef ::IceInternal::Handle< CacheTaskImpl > Ptr;

	CacheTaskImpl(CacheStoreImpl& store) : _store(store) {}
	virtual ~CacheTaskImpl() {}

public: // impl of CacheTask
    virtual void OnRestore(const ::Ice::Current& c);
    virtual void destroy(const ::Ice::Current& c);

protected: // impl of TimeoutObj
    virtual void OnTimer(const ::Ice::Current& c);

protected:
	CacheStoreImpl& _store;
};

// -----------------------------
// module CacheStoreImpl
// -----------------------------
class CacheStoreImpl : public ::TianShanIce::Storage::CacheStoreEx
{
	friend class CacheCmd;
	friend class CacheStoreProbe;
	friend class CacheStoreBarker;
	friend class ListMissedContentsCmd;
	friend class ListHotLocalsCmd;
	friend class TopFolderImpl;
	friend class CacheTaskImpl;

public:
	typedef ::IceInternal::Handle< CacheStoreImpl > Ptr;

	CacheStoreImpl(ZQ::common::Log& log, ZQ::common::Log& eventlog, const char* serviceName,
				ContentStoreImpl& contentStore, ::TianShanIce::Streamer::StreamServicePrx pStreamSevice, ZQ::common::NativeThreadPool& thpool);
	
	virtual ~CacheStoreImpl();

	bool doInit();
	void doUninit( );

	void serve(bool enable=true);

	::TianShanIce::Storage::CacheStorePrx proxy(bool collocationOptim =false);

public:

	ContentStoreImpl&		_contentStore;
	::TianShanIce::Streamer::StreamServicePrx		_prxStreamService;

	ZQ::common::Log&		_log;
	ZQ::common::Log&		_eventlog;
	ZQADAPTER_DECLTYPE&		_adapter;
	ZQTianShan::TimerWatchDog	_watchDog;

	TianShanIce::Storage::VolumeExPrx _rootVol;
	std::string                       _rootVolName;

public: // the section of configuration variables

	// configuration schema
	// ---------------------------------------------------------
	// <CacheStore heatbeatInterval="10000" flags="0" sessionInterface="" groupAddr="225.23.23.23" groupPort="11122" maxSpaceMB="0" freeSpacePercent="10" thirdPartyCache="0" >
	//   <CacheFactor maxCandidates="3" pwrRanking="1100" prevLoadThreshold="8000" successorLoadThreshold="2000" maxUnpopular="100" />
	//   <ContentPopularity timeWindow="300000" countOfPopular="4" hotWindow="30000" />
	//   <CachePenalty forwardFail="20" max="60" />
	//   <UpStream bindIP="" defaultProvSessionBitrateKbps="4000" defaultTransferServerPort="10080" minBitratePercent="0" totalProvisionBWKbps="1000000" 
	//             catchUpRT="600000" catchUpRTminPercent="50" />
	//   <DownStream bindIP="" />
	//   <ContentDomains paidLength="20" defaultSessionInterface="c2http://172.12.23.54:8099/" natPort="28790">
    //      <Provider name="hbo.com" sessionInterface="c2http://10.122.5.34:8080/" natPort="28791" />
    //      <Provider name="news[1-3].cnn.com" sessionInterface="c2http://172.12.23.54:8080/" natPort="28792" />
    //      <Provider name="news[4-9].cnn.com" sessionInterface="c2http://172.12.23.55:8080/" natPort="28793" />
    //   </ContentDomains>
	// </CacheStore>
	// 	call _extSourceStores.set() for each <Provider>, and call setDefault(const StoreInfo& storeInfo) for the default ProviderEdge


	// the configuration that can be changed on fly. protected via a locker
	ZQ::common::Mutex        _cfgLock;
	uint32                   _timeWinOfPopular, _countOfPopular, _hotTimeWin; // in msec, <ContentPopularity timeWindow="3" countOfPopular="1100" hotWindow="8000" />
	uint32                   _heatbeatInterval; // in msec, <CacheStore heatbeatInterval="10000" >
	uint32					 _maxCandidates; // <CacheFactor maxCandidates="3">
	float					 _pwrRanking; // takes that of <CacheFactor pwrRanking="1100"> divided by 1000
	uint					 _prevLoadThreshold, _successorLoadThreshold; // <CacheFactor prevLoadThreshold="8000" successorLoadThreshold="2000">
	uint32                   _maxUnpopular; // <CacheFactor maxUnpopular="100" />
	uint32					 _penaltyOfFwdFail, _penaltyMax; // <CachePenalty forwardFail="20" max="60" />
	uint32					 _defaultProvSessionBitrateKbps, _minBitratePercent, _defaultTransferServerPort; // <UpStream defaultProvSessionBitrateKbps="4000" defaultTransferServerPort="10080" minBitratePercent="0" totalProvisionBWKbps="1000000" />
	uint64		             _totalProvisionKbps;
	uint32					 _paidLength; // <ContentDomains paidLength="20" >
	uint32                   _catchUpRT, _catchUpRTminPercent; // <UpStream catchUpRT="600000" _catchUpRTminPercent="50" />

	uint32                   _maxSpaceMB; // optional configuration <CacheStore maxSpaceMB="0">
	uint32                   _freeSpacePercent;  // optional configuration <CacheStore freeSpacePercent="10">, acceptable range [5,50]

	std::string              _downStreamBindIP, _upStreamBindIP; // <DownStream bindIP=""> and <UpStream bindIP=""> 
	std::string              _cacheDir; // temp directory to save index file
	bool                     _bProxySessionBook; // 1) original CDNSS w/CacheStore set to true, 2) StreamSegmenter and C2FE set to false

	bool                     _contentAttrCache; // <CacheStore contentAttrCache="0" >, true if CacheStore is only wrapper for content attributes and will not counts or kick off CacheTasks

	typedef enum _StoreFlags
	{
		sfEnableNeighborhoodPropagation   = FLAG(0),
		sfLoggingDomainMessages           = FLAG(1),
		sfEnableDomainMessageDump         = FLAG(2),
		sfLoggingUnpopularScan            = FLAG(3),
		sfLoggingResource                 = FLAG(4),
	} StoreFlags;

	uint32					 _flags; // <CacheStore flags="0" > flags consists of StoreFlags

	void setFlags(uint32 flags) { _flags |= flags; 	}
	void resetFlags(uint32 flags) { _flags &= ~(uint32)flags; }

public: // the section of statistic variables, will be modified by the core, readonly to the clients
	int32 _sizeMissed, _sizeHotLocals;  // content-oriented hit rate can be learned from _sizeHotLocals/(_sizeHotLocals+_sizeMissed)*100%
	int32 _reqsMissedInWin, _reqsHotLocalsInWin; // request-oriented hit rate can be learned from _reqsHotLocalsInWin/(_reqsHotLocalsInWin+_reqsMissedInWin)*100%

	int64 _stampMesureSince;
	ZQ::common::Mutex _lkCounters;

	typedef struct _ExportCount
	{
		const char* name;
		int32 count;
		int32 failCount;
		int64 latencyTotal;
		int32 latencyMax;
	} ExportCount;

	typedef enum _ecIndex {	ec_forward, ec_remoteLocate, ec_local, ec_max } ecIndex;
	ExportCount _exportCounters[ec_max]; // index 0-forward, 1-remoteLocate, 2-localContent

	void resetCounters();
	void countExport(ecIndex ecIdx, int32 latencyMsec, bool succ=true);

protected:

	ZQ::common::NativeThreadPool&         _thpool;
	Ice::Identity                         _localId;
//	::TianShanIce::State	              _serviceState;
	std::string							  _serviceName;
	std::string							  _dbRuntimePath;

	float  _distSqrByRankTbl[CACHE_SCOPED_INSTANCE_MAX];
	double _loadWeightTbl[CACHE_SCOPED_INSTANCE_MAX];

	DECLARE_CONTAINER(CacheTask);
	DECLARE_CONTAINER(TopFolder);
	CacheFactory::Ptr                     _factory;

	void openDB(const char* runtimeDBPath = NULL);
	void closeDB();

public:

	typedef union _HashKey
	{
		struct { uint32 dw[4];  } dwords;
		struct { uint16  w[8]; }   words;
		struct { uint8  b[16]; }  bytes;
	} HashKey;

	typedef struct _FWU
	{
		std::string topFolderName;
		int minAccessCount;
		int64 unpopularSize;
		int64 usedMB;
	} FWU;

	static bool   _calcHashKey(HashKey& key, const char* str, uint len=0);
	static uint64 _calcRawDistance(const HashKey& contentKey, const HashKey& storeKey);
	static ::std::string _content2FolderName(const ::std::string& contentName, std::string& topFolderName, std::string& leafFolderName);
	
	// data structures about the neighborhood
	typedef struct _CacheStoreDsptr
	{
		::TianShanIce::Storage::CacheStoreDescriptor desc;
		HashKey cskey;
		int     penalty;
		int64   stampLastPenalty;
		int64   stampStartup;
	} CacheStoreDsptr;

	void penalize(CacheStoreDsptr& store, int penaltyToCharge =1);

	typedef std::map <std::string, CacheStoreDsptr> CacheStoreMap;
	typedef std::vector<CacheStoreDsptr> CacheStoreListInt;

	int _listNeighorsEx(CacheStoreListInt& list, bool includeSelf=true, uint32 heatbeatInterval =0);
	virtual void OnForwardFailed(std::string storeNetId, int penaltyToCharge=1);
	
	virtual std::string findExternalSessionInterfaceByContent(const std::string& contentName, const TianShanIce::Properties& nameFields);
	virtual std::string findExternalSessionInterfaceByProvider(const std::string& providerId);

    virtual ::TianShanIce::Storage::CacheStoreList listNeighors(::Ice::Int heatbeatInterval, const ::Ice::Current& c);
	virtual void updateStreamLoad(uint32 usedBwLocalStreaming, uint32 maxBwLocalStreaming, uint32 usedBwPassThru, uint32 maxBwPassThru);

	TianShanIce::Storage::AccessRegistrarPtr & getContentMissed(void)
	{ return _acMissed;	}
	
	TianShanIce::Storage::AccessRegistrarPtr & getContentHotLocals(void)
	{ return _acHotLocals;	}

public:

	SourceStores _extSourceStores;
	CacheStoreDsptr   _thisDescriptor; // .sessionInterface maps configure <CacheStore sessionInterface="" >
	ZQ::common::InetHostAddress _groupBind;
	ZQ::common::InetMcastAddress _groupAddr; // optional config <CacheStore groupAddr="225.23.23.23" >, default value: CACHESTORE_DEFAULT_GROUPIP
	int              _groupPort; // optional config <CacheStore groupPort="225.23.23.23" >, default value: CACHESTORE_DEFAULT_GROUPPORT

		
protected:

	CacheStoreMap     _neighbors;
	std::string       _thisPrxStr;
	ZQ::common::Mutex _lockerNeighbors;

	CacheStoreProbe   _probe;
	CacheStoreBarker  _barker;

	// load info collected by entry updateStreamLoad()
	uint32  _usedLocalStreamKbps, _maxLocalStreamKbps; 
	uint32  _usedPassThruKbps,    _maxPassThruKbps;

protected:  // impl of ReplicaQuery
    virtual void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly, const ::Ice::Current& c);

protected: // about the content access and folder abstraction

	typedef std::map <std::string, FWU> FWUMap;
	typedef std::vector <FWU> FWUSequence;
	FWUMap _fwuMap;
	ZQ::common::Mutex _lkFWU;

	void _updateFWU(const CacheStoreImpl::FWU& fwu);
	FWUSequence _sortFolderByFWU();
	bool _ensureSpace(long neededMB, const std::string& forTopFolderName= std::string(""));

	static std::string _contentHashKeyToFolder(const HashKey& contentHKey, std::string& topFolderName, std::string& leafFolderName);
	void _readStoreSpace(int64& freeMB, int64& totalMB);

	bool _initFolders();
	void _closeFolders();

	// the access counters of missedContents and hot local contents.
	//  - missed-contents means the contents that haven't been at InService
	//  - _hotLocals means the local contents that has been accessed recently
	::TianShanIce::Storage::AccessRegistrarPtr _acMissed, _acHotLocals;
	int64 _stampLocalCounterFlushed, _stampLastScanMissed;

	virtual void OnLocalContentRequested(const std::string& contentName, const std::string& subfile);
	virtual void OnMissedContentRequested(const std::string& contentName, const std::string& subfile);

public: // impls of CacheStoreEx

protected: // about CacheTask
	typedef std::map <std::string, CacheTaskImpl::Ptr > CacheTaskMap;
	CacheTaskMap _taskMap;
	int64		 _usedProvisionKbps; // the provision usage limited by  
	ZQ::common::Mutex _taskLocker;

	CacheTaskImpl::Ptr newCacheTask(const std::string& contentName);
	void _watchCacheTask(CacheTaskImpl::Ptr& pTask); // no lock protection in this func
	::TianShanIce::Storage::CacheTaskPrx commitCacheTask(CacheTaskImpl::Ptr& pTask);
	void withdrawCacheTask(CacheTaskImpl::Ptr& pTask);
	void scanCacheTasks();

protected: // impl of TimeoutObj
    virtual void OnTimer(const ::Ice::Current& c);

public: // impls of CacheStore

    virtual ::TianShanIce::Storage::ContentStorePrx theContentStore(const ::Ice::Current& c);
    virtual ::TianShanIce::Storage::CacheCandidates getCandidatesOfContent(const ::std::string& contentName, bool withLoadConsidered, const ::Ice::Current& c);

	virtual ::TianShanIce::Streamer::StreamPrx exportContentAsStream(const ::std::string& contentName, const ::std::string& subFile, ::Ice::Int idleStreamTimeout, ::Ice::Int cacheStoreDepth, const ::TianShanIce::SRM::ResourceMap& resourceRequirement, const ::TianShanIce::Properties& params, const ::Ice::Current& c);

	virtual void addAccessCount(const ::std::string& contentName, ::Ice::Int countToAdd, const ::std::string& since, const ::Ice::Current& c);
	virtual ::TianShanIce::Storage::ContentAccess getAccessCount(const ::std::string& contentName, const ::Ice::Current& c);
	virtual void listMissedContents_async(const ::TianShanIce::Storage::AMD_CacheStore_listMissedContentsPtr& amd, ::Ice::Int maxNum, const ::Ice::Current& c);
	virtual void listHotLocals_async(const ::TianShanIce::Storage::AMD_CacheStore_listHotLocalsPtr& amd, ::Ice::Int maxNum, const ::Ice::Current& c);

	virtual void setAccessThreshold(::Ice::Int timeWinOfPopular, ::Ice::Int countOfPopular, ::Ice::Int hotTimeWin, const ::Ice::Current& c);
    virtual void getAccessThreshold(::Ice::Int& timeWinOfPopular, ::Ice::Int& countOfPopular, ::Ice::Int& hotTimeWin, const ::Ice::Current& c);

	virtual void cacheContent(const ::std::string& contentName, const ::TianShanIce::Storage::CacheStorePrx& sourceCS, const ::TianShanIce::Properties& params, const ::Ice::Current& c);

    virtual ::Ice::Long calculateCacheDistance(const ::std::string& contentName, const ::std::string& storeNetId, const ::Ice::Current& c);

	virtual ::std::string getFolderNameOfContent(const ::std::string& contentName, const ::Ice::Current& c);
    virtual ::std::string getFileNameOfLocalContent(const ::std::string& contentName, const ::std::string& subfile, const ::Ice::Current& c);

protected: // impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

protected: // EventSink of a content record has just been created, deleted or state changed
	       // The child class derives from ContentStore should redirect ContentStore::OnContentCreated(), OnContentDestroyed(), OnContentStateChanged() to CacheStore
	virtual void OnContentCreated(const std::string& contentReplicaName);
	virtual void OnContentDestroyed(const std::string& contentReplicaName);
	virtual void OnContentStateChanged(const std::string& contentReplicaName, const ::TianShanIce::Storage::ContentState previousState, const ::TianShanIce::Storage::ContentState newState);

protected: // defines the portal entires

	// CacheStore Portal Entries
	// -----------------------------

	void* _ctxPortal;  ///< the context for the portal layer to customize

	/// Portal entry, called during ContentStore initialization, for the portal implementation to initialize.
	/// If necessary, the portal should initialize the portal context _ctxPortal
	///@param[in] store reference to the ContentStore
	static void initializePortal(CacheStoreImpl& store);

	/// Portal entry, called during ContentStore uninitialization, for the portal implementation to cleanup its context
	/// if necessary.
	///@param[in] store reference to the ContentStore
	static void uninitializePortal(CacheStoreImpl& store);

public: // defines the entires 

	/// portal function to map the subfile to the file extension name
	///@return the extension filename, normally with a leading "."
	static std::string portalSubfileToFileExtname(const CacheStoreImpl& store, const std::string& subfile);

	/// portal function to chop a content name into tokens, such as convert the content name into provider Id, provider Asset Id
	/// some known meta data name are
	/// contentNameFields[METADATA_PID] - the provider Id
	/// contentNameFields[METADATA_PAID] - the provider's asset Id
	///@return the size of token recoganized
	static int portalContentNameToken(const CacheStoreImpl& store, const std::string& contentName, TianShanIce::Properties& contentNameField);

	/// portal function to book a transfer session on external source library thru the given interface
	///@param[in] store the CacheStore that the portal may reference
	///@param[out] sessionURL - the url to the transfer session
	///@param[in] contentNameToken - the content name tokens that output from portalContentNameToken() invoked previously
	///@param[in] subFile - the subfile that to transfer
	///@param[in] extSessionInterface the LocateContent server interface
	///@param[in] transferBitrate - the speed to transfer the session
	///@param[out] params - the additional parameter the portal can collect from the response from external CDN. some known parameters are
	//              cdn.transferId, cdn.transferPort, cdn.transferTimeout, cdn.availableRange, cdn.openForWrite
	/// @return 0 if book succeeded
	static int portalBookTransferSession(const CacheStoreImpl& store, std::string& sessionURL, const TianShanIce::Properties& contentNameToken, const std::string& subFile, const std::string& extSessionInterface, long transferBitrate, ::TianShanIce::Properties& params);

	/// portal function to locate a content thru the given interface
	///@param[in] store the CacheStore that the portal may reference
	///@param[in] extSessionInterface the LocateContent server interface
	///@param[in, out] cacheTask - the task structure
	///            some input parameters that may be used in the portal are
	///               @li ident.name - content name
	///               @li bwMin, bwMax - the speed limited determined by the CacheStore for the portal to reference
	///               @li nameFields[providerId] - the provider id of the content
	///               @li nameFields[providerAssetId] - the provider asset id of the content
	///            some output parameters expected to be filled in by the portal are
	///               @li isSrcPWE - true if the source storage has the content being provisioned
	///               @li bwCommitted - the download speed in bps determined by the portal, should not exceed the range of [bwMin, bwMax]
	/// @return 0 if locate succeeded
	static int portalLocateContent(const CacheStoreImpl& store, const std::string& extSessionInterface, CacheTaskImpl& cacheTask);

};

// -----------------------------
// module AccessRegistrarImpl
// -----------------------------
//class AccessRegistrarImpl : public ::TianShanIce::Storage::AccessRegistrar, virtual public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class AccessRegistrarImpl : public ::TianShanIce::Storage::AccessRegistrar, virtual public ICEAbstractMutexRLock
{
public:
	AccessRegistrarImpl(CacheStoreImpl& store, const std::string& counterName ="") : _store(store) { if (!counterName.empty()) name=counterName; }
	virtual ~AccessRegistrarImpl() {}

	/// utility function to merge access counter into one
	///@param acA  input access count A
	///@param acB  input access count B
	///@param newWindowFrom  the timestamp that the merged return access count will count since
	///@return the new merged access count that from specified newWindowFrom
	///@note. for example, m(acA(10, [20120510T120000.000-20120510T120040.000]) + acB(20, [20120510T120010.000-20120510T120100.000]), since(20120510T120030.000))
	///                 = (10* (40-30)/(40-0) + 20*(60-30)/(60-10))/2 + 20*(60-40)/(60-10) = ac(round(15.25), [20120510T120030.000-20120510T120100.000])
	/// where the acA could be the access counter that has been flushed into ContentStore DB and acB is the one newly counted in the memory
	/// when the metadata of the content is being updated into the DB, the merged count values should be the result goes into DB
	static ::TianShanIce::Storage::AccessCounter mergeAccessCount(const ::TianShanIce::Storage::AccessCounter& acA, const ::TianShanIce::Storage::AccessCounter& acB, int64 newWindowFrom);
	static const char* ContentAccessStr(const TianShanIce::Storage::ContentAccess& ac, char* buf, int maxLen);

protected:

	virtual ::TianShanIce::Storage::ContentAccess count(const ::std::string& contentName, const ::std::string& subfile, ::Ice::Int countToAdd, const ::Ice::Current& c);
    virtual void set(const ::TianShanIce::Storage::AccessCounter& counterToOverwrite, const ::Ice::Current& c);
    virtual void erase(const ::std::string& contentName, const ::Ice::Current& c);
    virtual bool get(const ::std::string& contentName, ::TianShanIce::Storage::AccessCounter& counter, const ::Ice::Current& c);
	virtual ::Ice::Int compress(::Ice::Int windowSize, ::Ice::Int flushWinSize, ::Ice::Int& reqsInWindow, ::TianShanIce::Storage::ContentCounterList& listToFlush, ::TianShanIce::Storage::ContentCounterList& listEvicted, const ::Ice::Current& c);
	virtual void sort(::Ice::Int windowSize, bool unpopular1st, int minCount, ::TianShanIce::Storage::ContentCounterList& result, const ::Ice::Current& c);
    virtual ::Ice::Int size(const ::Ice::Current& c);

protected:
	CacheStoreImpl& _store;
};

}} // namespace

#endif // __ZQTianShan_CacheStoreImpl_H__
