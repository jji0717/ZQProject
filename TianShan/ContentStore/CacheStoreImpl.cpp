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
// Ident : $Id: CacheStoreImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/CacheStoreImpl.cpp $
// 
// 136   4/25/17 5:34p Hui.shao
// 
// 135   4/14/17 1:59p Hui.shao
// _contentAttrCache
// 
// 134   4/10/17 4:00p Hui.shao
// 
// 133   2/28/17 2:24p Hui.shao
// 
// 132   8/26/16 5:54p Hui.shao
// 
// 131   8/26/16 9:52a Hui.shao
// 
// 130   8/25/16 4:00p Hui.shao
// 
// 129   6/24/15 11:11a Hui.shao
// 
// 128   4/03/15 2:39p Hui.shao
// thirdPartyCache for Squid
// 
// 127   1/23/15 11:44a Hui.shao
// _bProxySessionBook: 1) original CDNSS w/CacheStore set to true, 2)
// StreamSegmenter and C2FE set to false
// 
// 126   5/15/14 10:13a Hongquan.zhang
// 
// 125   5/14/14 3:07p Hui.shao
// for C2Streaming with index content exposed
// 
// 124   12/11/13 7:43p Hui.shao
// added hashed folder calculation
// 
// 123   9/26/13 11:23a Hui.shao
// logging
// 
// 122   7/10/13 4:03p Hui.shao
// enh#18206 - CacheServer selection policy per UML-E's local-read cost
// needs considered
// 
// 121   4/17/13 5:01p Hui.shao
// 
// 120   12/20/12 8:57a Li.huang
// 
// 119   12/19/12 5:23p Hui.shao
// 
// 118   11/26/12 2:51p Hui.shao
// added snmp counters to export
// 
// 117   9/20/12 1:37p Hui.shao
// 
// 116   9/20/12 10:26a Hui.shao
// 
// 115   9/19/12 5:30p Hui.shao
// 
// 114   9/19/12 3:13p Hui.shao
// 
// 113   9/18/12 2:07p Hui.shao
// enh#16995 - Serve the cached copy that is catching up the PWE copy on
// source storage
// 
// 112   9/12/12 12:44p Hui.shao
// 
// 111   9/12/12 11:28a Hui.shao
// forced to enable edgeMode of ContentStore if CacheStore is applied
// 
// 110   9/06/12 7:13p Hui.shao
// 
// 109   9/06/12 6:48p Hui.shao
// 
// 108   9/06/12 5:49p Hui.shao
// 
// 107   9/05/12 2:55p Hui.shao
// refreshing ContentDB that may be out of date
// 
// 106   9/05/12 12:06p Hui.shao
// flag logging
// 
// 105   9/04/12 3:05p Hui.shao
// 
// 103   9/03/12 3:24p Hui.shao
// 
// 102   8/30/12 11:46a Hui.shao
// 
// 101   8/09/12 5:15p Hui.shao
// NULL pointer test for linux
// 
// 100   8/09/12 12:21p Hui.shao
// corrected logging
// 
// 99    8/06/12 9:39p Hui.shao
// 
// 98    8/06/12 9:36p Hui.shao
// 
// 97    8/06/12 8:48p Hui.shao
// export _freeSpacePercent as configurable
// 
// 96    8/06/12 8:36p Hui.shao
// 
// 95    8/06/12 5:19p Hui.shao
// 
// 94    8/06/12 12:50p Hui.shao
// store::OnTimer to call ensureSpace()
// 
// 93    8/02/12 5:31p Hui.shao
// 
// 92    8/02/12 4:33p Hui.shao
// adjusted PWE proto per portalLocateContent() may returns c2http
// 
// 91    8/02/12 3:17p Hui.shao
// quit forwarding if any contentEdge throws ClientError
// 
// 90    7/31/12 12:28p Hui.shao
// uncount missed content unless it is confirmed availabe at source
// storage
// 
// 89    7/27/12 10:27a Hui.shao
// changed maxSpace from GB to MB
// 
// 88    7/26/12 7:20p Hui.shao
// enh#16577 to limit max diskspace via configuration
// 
// 87    7/26/12 6:43p Hui.shao
// firxed domain barker/probe's bind address
// 
// 86    7/26/12 11:44a Hui.shao
// added entry to update load
// 
// 85    7/25/12 4:19p Hui.shao
// quit creating stream if local absent and failed to locate at source
// storage
// 
// 84    7/25/12 4:12p Hui.shao
// added api to list hot locals
// 
// 83    7/25/12 3:56p Li.huang
// fix bug  16726 
// 
// 82    7/24/12 2:47p Hui.shao
// stat counters on hitrate
// 
// 81    7/19/12 7:36p Hui.shao
// adjusted counter list compress logic
// 
// 80    7/19/12 6:06p Hui.shao
// 
// 79    7/19/12 5:35p Hui.shao
// added and refer to store-wide rootVolName
// 
// 78    7/19/12 5:16p Hui.shao
// 
// 77    7/18/12 6:57p Hui.shao
// 
// 76    7/18/12 1:54p Li.huang
// 
// 75    7/18/12 10:29a Li.huang
// 
// 74    6/29/12 10:57a Li.huang
// 
// 73    6/28/12 2:53p Hui.shao
// 
// 72    6/28/12 2:26p Li.huang
// 
// 71    6/28/12 1:24p Li.huang
// 
// 70    6/28/12 12:08p Li.huang
// 
// 69    6/28/12 11:43a Li.huang
// 
// 68    6/28/12 11:26a Li.huang
// 
// 67    6/28/12 11:15a Hui.shao
// 
// 66    6/28/12 11:08a Li.huang
// 
// 65    6/28/12 11:05a Li.huang
// 
// 64    6/27/12 6:41p Li.huang
// 
// 63    6/27/12 6:03p Hui.shao
// 
// 61    6/27/12 5:24p Hui.shao
// 
// 59    6/26/12 6:20p Li.huang
// 
// 58    6/26/12 5:26p Hui.shao
// 
// 57    6/26/12 5:03p Hui.shao
// FWU to faster find other folders for free space
// 
// 56    6/26/12 2:55p Hui.shao
// 
// 55    6/20/12 7:24p Hui.shao
// 
// 54    6/20/12 4:56p Li.huang
// 
// 53    6/20/12 3:14p Hui.shao
// 
// 52    6/19/12 3:43p Li.huang
// 
// 51    6/19/12 2:58p Li.huang
// 
// 50    6/15/12 3:05p Hui.shao
// folder space
// 
// 49    6/14/12 5:15p Hui.shao
// 
// 48    6/14/12 4:36p Hui.shao
// 
// 47    6/13/12 7:50p Hui.shao
// some log printing
// 
// 46    6/13/12 6:20p Hui.shao
// 
// 45    6/13/12 11:42a Li.huang
// 
// 44    6/12/12 4:35p Li.huang
// 
// 43    6/12/12 2:48p Hui.shao
// 
// 42    6/12/12 2:12p Hui.shao
// 
// 41    6/11/12 8:16p Hui.shao
// 
// 40    6/11/12 5:20p Li.huang
// 
// 39    6/07/12 6:00p Hui.shao
// 
// 38    6/07/12 11:30a Hui.shao
// configuration schema
// 
// 37    6/06/12 4:43p Hui.shao
// 
// 36    6/06/12 4:03p Hui.shao
// to print the count result in the log
// 
// 35    5/29/12 2:21p Hui.shao
// added new portal portalBookTransferSession()
// 
// 34    5/11/12 2:50p Li.huang
// 
// 33    5/10/12 6:50p Hui.shao
// kick off cache missed content per popularity
// 
// 32    4/27/12 6:38p Hui.shao
// CacheTask lifetime func
// 
// 31    4/27/12 3:37p Build
// 
// 30    4/26/12 6:32p Hui.shao
// the map of external source stores: reg exp of pid => c2 interface
// 
// 29    4/25/12 4:36p Hui.shao
// drafted most unpopular list of TopFolder
// 
// 28    4/25/12 11:10a Hui.shao
// 
// 27    4/23/12 3:19p Hui.shao
// 
// 26    4/23/12 2:55p Hui.shao
// configuration of up/down stream NIC, determin the destIP in
// cacheContent() automatically
// 
// 25    4/18/12 6:47p Hui.shao
// 
// 24    4/17/12 11:18a Hui.shao
// for the content::provision() parameters
// 
// 23    4/16/12 5:06p Li.huang
// 
// 22    4/16/12 4:11p Hui.shao
// 
// 21    4/13/12 1:36p Hui.shao
// enabled exportContentAsStream() with external CDNSS
// 
// 20    4/11/12 10:37a Li.huang
// 
// 19    4/09/12 2:28p Hui.shao
// 
// 18    4/09/12 2:23p Hui.shao
// 
// 17    3/26/12 3:58p Li.huang
// 
// 16    3/22/12 5:13p Hui.shao
// 
// 15    1/19/12 12:09p Hui.shao
// 
// 14    1/18/12 6:24p Li.huang
// 
// 13    1/18/12 1:50p Hui.shao
// linkable
// 
// 12    1/17/12 11:35a Hui.shao
// linkable
// 
// 11    1/16/12 10:35p Hui.shao
// defined the counter and topfolder at ice level
// 
// 10    1/16/12 1:22p Hui.shao
// 
// 9     1/13/12 2:24p Hui.shao
// splitted counter and folder into separate cpp files
// 
// 8     1/12/12 7:02p Hui.shao
// 
// 7     1/06/12 2:55p Hui.shao
// 
// 6     1/06/12 2:01p Hui.shao
// 
// 5     1/06/12 11:53a Hui.shao
// 
// 4     12/30/11 2:28p Hui.shao
// 
// 3     12/29/11 8:27p Hui.shao
// cacheContent()
// 
// 2     12/22/11 3:30p Hui.shao
// the try-catches
// 
// 1     12/22/11 1:38p Hui.shao
// created
// ===========================================================================
#include "CacheStoreImpl.h"

#include "MD5CheckSumUtil.h"
#include "CacheCmds.h"

#include "ContentState.h"
#include "ContentCmds.h"
#include "urlstr.h"

#include "../CDNLib/CDNDefines.h"
#include "TianShanIceHelper.h"

#include <set> // for multiset

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#else
	#include <sys/stat.h>
#endif
	#include <math.h>
};

#ifndef min
#  define min(_X, _Y) ((_X>_Y)?_Y:_X)
#endif

#define RoundToKbps(_bps) ((_bps +999)/1000)

#define CATEGORY_CacheTask      "CacheTask"
#define CATEGORY_TopFolder      "TopFolder"
#define SUBFILE_EXTNAME_index   "index"

#define INSTANCE_DEFAULT_PWR (1.1)

#define NEIGHBOR_Timeout       (3000)  // 3sec is the default timeout between the neigbors
#define NEIGHBOR_Timeout_MAX   (8000) //  8sec is the max timeout between the neigbors

#define MEM_EVICTOR

namespace ZQTianShan {
namespace ContentStore {

#define storelog (_log)

#ifndef FUNCNAME
#define FUNCNAME(_FN) #_FN "() "
#endif // __FUNCNAME__

// -----------------------------
// CacheStoreImpl utilities
// -----------------------------
bool CacheStoreImpl::_calcHashKey(CacheStoreImpl::HashKey& key, const char* buf, uint len)
{
	if (NULL == buf)
		return false;

	if (len <=0)
		len = strlen(buf);

	memset(&key.bytes, 0x00, sizeof(key.bytes));

	ZQ::common::md5 encoder;
	encoder.Update((unsigned char*)buf, len);
	encoder.Finalize();
	memcpy(&key.bytes, encoder.Digest(), sizeof(key.bytes));

	return true;
}

std::string CacheStoreImpl::_contentHashKeyToFolder(const HashKey& contentHKey, std::string& topFolderName, std::string& leafFolderName)
{
	char buf[32] = "\0";
	uint8 t=0, l=1;
	for (int i =0; i < 8; i++)
	{
		t ^= contentHKey.bytes.b[2*i];
		l ^= contentHKey.bytes.b[2*i+1];
	}

	t &= CACHE_FOLDER_MASK;
	l &= CACHE_FOLDER_MASK;

	snprintf(buf, sizeof(buf)-2, TOP_FOLDER_FMT, t);
	topFolderName = buf;
	snprintf(buf, sizeof(buf)-2, LEAF_FOLDER_FMT, l);
	leafFolderName = buf;
	return std::string(LOGIC_FNSEPS) + topFolderName + LOGIC_FNSEPS + leafFolderName + LOGIC_FNSEPS;
}

uint64 CacheStoreImpl::_calcRawDistance(const HashKey& contentKey, const HashKey& storeKey)
{
	uint64 sqrSum =0;
	for (int i=0; i < sizeof(HashKey) / sizeof(uint16); i++)
	{
		int32 diff = contentKey.words.w[i] - storeKey.words.w[i];
		sqrSum += diff*diff;
	}

	return sqrSum;
}

// -----------------------------
// module CacheStoreImpl
// -----------------------------
CacheStoreImpl::CacheStoreImpl(ZQ::common::Log& log, ZQ::common::Log& eventlog, const char* serviceName,
				ContentStoreImpl& contentStore, ::TianShanIce::Streamer::StreamServicePrx pStreamSevice, ZQ::common::NativeThreadPool& thpool)
				: _contentStore(contentStore), _adapter(contentStore._adapter), _log(log), _eventlog(eventlog), _thpool(thpool), _prxStreamService(pStreamSevice),
				  _watchDog(log, thpool, contentStore._adapter, "CacheStore"), _probe(*this), _barker(*this), _maxCandidates(0), _flags(0),
				  _acMissed(NULL), _acHotLocals(NULL), _stampLocalCounterFlushed(0), // _acMissed("Missed"), _acHotLocals("HotLocal"),
				  _groupAddr(CACHESTORE_DEFAULT_GROUPIP), _groupPort(CACHESTORE_DEFAULT_GROUPPORT), _maxSpaceMB(0), _freeSpacePercent(10),
				  _pwrRanking(INSTANCE_DEFAULT_PWR), _prevLoadThreshold(CACHE_LOADTHRSH_DEFAULT_PREV), _successorLoadThreshold(CACHE_LOADTHRSH_DEFAULT_SUCR),
				  _maxUnpopular(CACHE_DEFAULT_UnpopularSize), _penaltyOfFwdFail(PENALTY_FORWARD_FAILED_DEFAULT),
				  _defaultProvSessionBitrateKbps(DEFAULT_PROV_SPEED_Kbps), _minBitratePercent(0), _paidLength(DEFAULT_PAID_LEN),
				  _defaultTransferServerPort(12000), _catchUpRTminPercent(50), _catchUpRT(60000), _bProxySessionBook(true), _contentAttrCache(false)
{
//    _acMissed.name="Missed", _acHotLocals.name="HotLocal";
	_serviceName = (NULL == serviceName) ? serviceName : "CacheStore";
	_acMissed    = new AccessRegistrarImpl(*this, "Missed");
	_acHotLocals = new AccessRegistrarImpl(*this, "HotLocal");
	_usedProvisionKbps = 0;
	_heatbeatInterval = 1000;
	_stampLastScanMissed =  ZQ::common::now();

	// When this CacheStore layer is enabled, the under layer ContentStore must has the following enabled
	// a) edge mode
	// b) InServiceCheck
	_contentStore._edgeMode = true;
	_contentStore._enableInServiceCheck = 1;

	resetCounters();
}

CacheStoreImpl::~CacheStoreImpl()
{
}


bool CacheStoreImpl::doInit()
{
	try
	{
		// step 1. initialize _thisDescriptor
		_thisDescriptor.desc.netId  = _contentStore._netId;
		_thisDescriptor.desc.domain = _contentStore._replicaGroupId;
		_thisDescriptor.desc.state  = ::TianShanIce::stNotProvisioned;
		_thisDescriptor.desc.loadStream = _thisDescriptor.desc.loadCacheWrite =_thisDescriptor.desc.loadImport =0;
		_thisDescriptor.desc.stampAsOf  = 0;
		//TODO: string      _thisDescriptor.desc.sessionInterface; ///< the session interface that can setup stream sessions of contents on the CacheStore

		_calcHashKey(_thisDescriptor.cskey, _thisDescriptor.desc.netId.c_str(), _thisDescriptor.desc.netId.length());
		_thisDescriptor.penalty  = 0;
		_thisDescriptor.stampLastPenalty  = 0;
		_thisDescriptor.stampStartup = ZQ::common::now();

		// step 2. call to initialize the portal layer
		initializePortal(*this);

		// step 3. to open/initialize the database
		openDB();
		_factory = new CacheFactory(*this);

/*
		{
			// step 3.1 TODO: 
#pragma message ( __TODO__ "read configuration to build up _extSourceStores by calling _extSourceStores::set() or setDefault()")
			SourceStores::StoreInfo si;
			si.sessionInterface = "c2http://10.15.10.64:10080/"; // dummy test data, should be replaced
			_extSourceStores.setDefault(si);
		}
*/
		// step 4. call to initialize the hash folders
		if (!_initFolders())
			return false;

		// step 5. to initialize the distance and load-weight tables
		if (_prevLoadThreshold <= _successorLoadThreshold)
		{
			_prevLoadThreshold      = CACHE_LOADTHRSH_DEFAULT_PREV;
			_successorLoadThreshold = CACHE_LOADTHRSH_DEFAULT_SUCR;
		}

		int i;
		for (i =0; i < CACHE_SCOPED_INSTANCE_MAX; i ++)
		{
			double t = pow((double)i+1, (double) _pwrRanking);
			_distSqrByRankTbl[i] = (float) ((t>=1.0) ? (1.0- 1/t) : 1.0);
			_distSqrByRankTbl[i] *= _distSqrByRankTbl[i];
		}

		_loadWeightTbl[0] =1;
		for (i =1; i < CACHE_SCOPED_INSTANCE_MAX; i ++)
		{
			double r = _distSqrByRankTbl[i-1] + _prevLoadThreshold * _prevLoadThreshold;
			r -= _distSqrByRankTbl[i];
			if (_successorLoadThreshold >0)
				r /= _successorLoadThreshold * _successorLoadThreshold;
			_loadWeightTbl[i] = r;
		}

		// step 6. to start the probe and barker
		_probe.start();
		_barker.start();

		// step 7. start the watch dog
		_watchDog.start();

		// step 8. start serving
		serve(true);
		proxy(); // to initialize _thisDescriptor.desc.theStore
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, doInit, "failed to initialize cotentStore"));
		return false;
	}

	return true;
}

void CacheStoreImpl::doUninit( )
{
	// stop the probe and barker
	_probe.stop();
	_barker.stop();

	_watchDog.quit();

	serve(false);

	_thisDescriptor.desc.state = ::TianShanIce::stOutOfService;

	uninitializePortal(*this);
	closeDB();
	_closeFolders();
}	

void CacheStoreImpl::openDB(const char* runtimeDBPath)
{
	closeDB();

	if (NULL == runtimeDBPath || strlen(runtimeDBPath) <=0)
		_dbRuntimePath = _contentStore._dbPath;
	else _dbRuntimePath = runtimeDBPath;

	if (FNSEPC != _dbRuntimePath[_dbRuntimePath.length()-1])
		_dbRuntimePath += FNSEPS;

	// step 1. about the topFolder DB
	try 
	{	
		// open the Indexes
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, openDB, "opening database at path: %s"), _dbRuntimePath.c_str());

#ifdef ZQ_OS_MSWIN
		::CreateDirectory(_dbRuntimePath.c_str(), NULL);
#else
		mkdir(_dbRuntimePath.c_str(), 0777);
#endif
		std::vector<Freeze::IndexPtr> indices;

		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, openDB, "opening " CATEGORY_TopFolder));
#ifdef MEM_EVICTOR
		_eTopFolder = new MemoryServantLocator(_adapter, CATEGORY_TopFolder);
#elif ICE_INT_VERSION / 100 >= 303
		_eTopFolder = ::Freeze::createBackgroundSaveEvictor(_adapter, cacheTaskDbPathname, CATEGORY_TopFolder, 0, indices);
#else
		_eTopFolder = Freeze::createEvictor(_adapter, cacheTaskDbPathname, CATEGORY_TopFolder, 0, indices);
#endif
		_adapter->addServantLocator(_eTopFolder, CATEGORY_TopFolder);
		_eTopFolder->setSize(CACHE_FOLDER_SIZE+10);
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 1001, "openDB() DB[topfolder] caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 1001, "openDB() DB[topfolder] caught exception"));
	}

	IdentCollection identities;

#ifndef MEM_EVICTOR
	identities.clear();
	try	{
		::Freeze::EvictorIteratorPtr itptr = _eTopFolder->getIterator("", 100);
		while (itptr->hasNext())
			identities.push_back(itptr->next());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, openDB, "failed to enumerates TopFolders"));
	}

	c=0;
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, openDB, "%d TopFolders found from DB"), identities.size());
	for (it = identities.begin(); it <identities.end(); it ++)
	{
		try {
			::TianShanIce::Storage::CacheTaskPrx task = IdentityToObj(CacheTask, *it);
			task->OnRestore();
			c++;
		}
		catch(...) {}
	}	
	_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, openDB, "%d of %d TopFolders restored"), c, identities.size());
#endif // MEM_EVICTOR for _eTopFolder

	// step 1. about the CacheTask DB
	if (_contentAttrCache)
	{
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, openDB, "ignore DB[CacheTask] as this is content-attr only cache"));
		return;
	}

	try 
	{	
		std::string cacheTaskDbPathname = createDBFolder(_log, _serviceName.c_str(), _dbRuntimePath.c_str(), CATEGORY_CacheTask, 100*1000);

		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();

		std::string dbAttrPrefix = std::string("Freeze.DbEnv.") + cacheTaskDbPathname;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");

		std::vector<Freeze::IndexPtr> indices;

		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, openDB, "opening database " CATEGORY_CacheTask));
#if ICE_INT_VERSION / 100 >= 303
		_eCacheTask = ::Freeze::createBackgroundSaveEvictor(_adapter, cacheTaskDbPathname, CATEGORY_CacheTask, 0, indices);
#else
		_eCacheTask = Freeze::createEvictor(_adapter, cacheTaskDbPathname, CATEGORY_CacheTask, 0, indices);
#endif
		proper->setProperty("Freeze.Evictor." CATEGORY_CacheTask ".PageSize",              "8192");
		proper->setProperty("Freeze.Evictor." CATEGORY_CacheTask ".$default.BtreeMinKey",  "20");

		_adapter->addServantLocator(_eCacheTask, CATEGORY_CacheTask);
		_eCacheTask->setSize(200);
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 1001, "openDB() DB[CacheTask] caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 1001, "openDB() DB[CacheTask] caught unknown exception"));
	}

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, openDB, "restoring CacheTasks"));
	try	{
		::Freeze::EvictorIteratorPtr itptr = _eCacheTask->getIterator("", 100);
		while (itptr->hasNext())
			identities.push_back(itptr->next());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, openDB, "failed to enumerates CacheTasks"));
	}

	IdentCollection::iterator it;
	int c=0;
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, openDB, "%d CacheTask(s) found from DB"), identities.size());
	for (it = identities.begin(); it <identities.end(); it ++)
	{
		try {
			::TianShanIce::Storage::CacheTaskPrx task = IdentityToObj(CacheTask, *it);
			task->OnRestore();
			c++;
		}
		catch(const Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, openDB, "restoring CacheTask[%s] caught exception[%s]"), it->name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, openDB, "restoring CacheTask[%s] caught exception"), it->name.c_str());
		}
	}	

	_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, openDB, "%d of %d CacheTasks restored"), c, identities.size());

	_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, openDB, "database ready"));
}

void CacheStoreImpl::closeDB()
{
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, closeDB, "closing local database"));
	_eCacheTask = NULL;
	_eTopFolder = NULL; // not very necessary when MEM_EVICTOR
}


::TianShanIce::Storage::CacheStorePrx CacheStoreImpl::proxy(bool collocationOptim)
{
	try {

		if (!_thisDescriptor.desc.theStore)
		{
			//_thisPrx is only modified here
			static ZQ::common::Mutex thisProxyLocker;
			ZQ::common::MutexGuard gd(thisProxyLocker);
			if (!_thisDescriptor.desc.theStore)
			{
				_thisDescriptor.desc.theStore = ::TianShanIce::Storage::CacheStorePrx::checkedCast(_adapter->createProxy(_localId));
				_thisPrxStr = _adapter->getCommunicator()->proxyToString(_thisDescriptor.desc.theStore);

				// CacheStroe should never respond slowly
					// CacheStroe should never respond slowly
				size_t posTimeout = _thisPrxStr.rfind("-t "), posEOToken =std::string::npos;
				long timeout =0;
				if (std::string::npos != posTimeout)
				{
					posEOToken = _thisPrxStr.find_first_not_of(" \t", posTimeout +2);
					if (std::string::npos != posEOToken && isdigit(_thisPrxStr.at(posEOToken)))
					{
						timeout = atol(_thisPrxStr.substr(posEOToken).c_str());
						posEOToken = _thisPrxStr.find_first_not_of("0123456789", posEOToken+1);
					}
				}

				char buf[16];
				if (timeout <= 0)
				{
					// no -t <timeout> has been specified, take the default, append it
					snprintf(buf, sizeof(buf)-2, " -t %d", NEIGHBOR_Timeout);
					_thisPrxStr += buf;
				}
				else if (timeout > NEIGHBOR_Timeout_MAX)
				{
					snprintf(buf, sizeof(buf)-2, "-t %d", NEIGHBOR_Timeout_MAX);
					if (posEOToken > posTimeout)
						_thisPrxStr.replace(posTimeout, (posEOToken - posTimeout), buf);
					else
						_thisPrxStr.replace(posTimeout, (_thisPrxStr.length() - posTimeout), buf);
				}
			}
		}

//		if (collocationOptim)
			return _thisDescriptor.desc.theStore;

//		return ::TianShanIce::Storage::CacheStorePrx::checkedCast(_thisDescriptor.desc.theStore->ice_collocationOptimized(false));
	}
	catch(const Ice::Exception& ex)
	{
		storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, proxy, "caught exception[%s]"), ex.ice_name().c_str());
	}
	catch( ...)
	{
		storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, proxy, "caught exception"));
	}

	return NULL;
}

void CacheStoreImpl::serve(bool enable)
{
	if (enable && _localId.name.empty())
	{
		try {
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, serve, "adding the interface[%s] on to the adapter[%s]"),
				SERVICE_NAME_CacheStore, _adapter->getName().c_str());
			_localId = _adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_CacheStore);
			_contentStore._adapter->ZQADAPTER_ADD(_contentStore._adapter->getCommunicator(), this, SERVICE_NAME_CacheStore);
		}
		catch(const Ice::Exception& ex)
		{
			storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, serve, "caught exception[%s]"), ex.ice_name().c_str());
		}
		catch( ...)
		{
			storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, serve, "caught exception"));
		}

		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, serve, "adding self under watching"));
		_watchDog.watch(_localId, 0);
		_thisDescriptor.desc.state = ::TianShanIce::stInService;
	}
	else if (!_localId.name.empty())
	{
		_thisDescriptor.desc.state = ::TianShanIce::stOutOfService;

		try {
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, serve, "withdrawing the interface[%s] from the adapter[%s}"), SERVICE_NAME_CacheStore, _contentStore._adapter->getName().c_str());
			_contentStore._adapter->remove(_localId);
			_localId.name = "";
		}
		catch(const Ice::Exception& ex)
		{
			storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, serve, "caught exception[%s]"), ex.ice_name().c_str());
		}
		catch( ...)
		{
			storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, serve, "caught exception"));
		}
	}
}

::std::string CacheStoreImpl::getAdminUri(const ::Ice::Current& c)
{
#pragma message ( __TODO__ "impl here")
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (_log, EXPFMT(CacheStore, 9999, "Not implemented yet"));
	return std::string("");
}

::TianShanIce::State CacheStoreImpl::getState(const ::Ice::Current& c)
{
	return _thisDescriptor.desc.state;
}
    
::TianShanIce::Storage::ContentStorePrx CacheStoreImpl::theContentStore(const ::Ice::Current& c)
{
	return _contentStore.proxy();
}

bool CacheStoreImpl::_initFolders()
{
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _initFolders, "preparing %dx%d subfolder structure"), CACHE_FOLDER_SIZE, CACHE_FOLDER_SIZE);

	// ::TianShanIce::Storage::VolumePrx root;
	try {
		_rootVol = TianShanIce::Storage::VolumeExPrx::uncheckedCast(_contentStore.proxy(true)->openVolume(DEFAULT_VOLUME_STRING));
		_rootVolName = _rootVol->getVolumeName();
	}
	catch(const Ice::Exception& ex)
	{
		storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "opening root volume caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "opening root volume caught exception"));
	}

	if (!_rootVol)
		return false;

	bool succ = true;
	::TianShanIce::Storage::FolderPrx tfolder;
	for (int t=0; succ && t < CACHE_FOLDER_SIZE; t++)
	{
		char topFolderName[64];
		snprintf(topFolderName, sizeof(topFolderName) -2, TOP_FOLDER_FMT, t);

		try {
			tfolder = _rootVol->openSubFolder(topFolderName, true, 0);
			if (!tfolder)
				succ = false;
		}
		catch(const Ice::Exception& ex)
		{
			storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "initializing tf[%s] caught exception[%s]"), topFolderName, ex.ice_name().c_str());
			succ =false;
			continue;
		}
		catch( ...)
		{
			storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "initializing tf[%s] caught exception"), topFolderName);
			succ =false;
			continue;
		}

/*
		for (int l=0; succ && l < CACHE_FOLDER_SIZE; l++)
		{
			::TianShanIce::Storage::FolderPrx lfolder;
			char folderName[64];
			snprintf(folderName, sizeof(folderName) -2, LEAF_FOLDER_FMT, l);
			try {
				lfolder = tfolder->openSubFolder(folderName, true, 0);
				if (!lfolder)
					succ = false;
			}
			catch(const Ice::Exception& ex)
			{
				storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "initializing lf[%s] caught exception[%s]"), folderName, ex.ice_name().c_str());
				succ =false;
				continue;
			}
			catch( ...)
			{
				storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "initializing lf[%s] caught exception"), folderName);
				succ =false;
				continue;
			}
		}
*/

		::Ice::Identity     identTop;
		identTop.name       = topFolderName;
		identTop.category   = CATEGORY_TopFolder;
		::TianShanIce::Storage::TopFolderPrx topFolder;

		try {
			topFolder = IdentityToObj2(TopFolder, identTop);
		}
		catch(const Ice::ObjectNotExistException&)
		{
#ifndef MEM_EVICTOR
			storelog(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _initFolders, "no DB record TF[%s] pre-exists"), topFolderName);
#endif // MEM_EVICTOR
		}
		catch(...)
		{
			storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "checking DB record TF[%s] caught exception"), topFolderName);
			succ =false;
		}

		if (succ && !topFolder)
		{
			try {
				TianShanIce::Storage::TopFolderPtr pTF = new TopFolderImpl(*this);
				if (!pTF)
				{
					storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "failed to initialize tf[%s] on allocating"), topFolderName);
					return false;
				}

				pTF->ident            = identTop;
				pTF->stampLastRefresh = 0;
				pTF->maxUnpopular     = _maxUnpopular;
				// pTF->totalSpaceMB     = pTF->_freeSpaceMB =0;
				pTF->usedSpaceMB =0;
				pTF->contentsOfleaves.resize(CACHE_FOLDER_SIZE);
				pTF->contentSubtotal  =0;

				_eTopFolder->add(pTF, pTF->ident);
			}
			catch(const Ice::Exception& ex)
			{
				storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "adding TF[%s] caught exception[%s]"), topFolderName, ex.ice_name().c_str());
				succ =false;
			}
			catch( ...)
			{
				storelog(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, _initFolders, "adding TF[%s] caught exception"), topFolderName);
				succ =false;
			}
		}

		if (succ)
			_watchDog.watch(identTop, 1000);
	}

	_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, _initFolders, "%s preparing %dx%d subfolder structure"), succ? "finished": "failed at", CACHE_FOLDER_SIZE, CACHE_FOLDER_SIZE);
	return succ;
}

void CacheStoreImpl::_closeFolders()
{
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _closeFolders, "closing folders"));
	// _topFolders.clear();
	_eTopFolder = NULL;
}


static bool lessRawDist(::TianShanIce::Storage::CacheCandidate i, ::TianShanIce::Storage::CacheCandidate j)
{
	return (i.rawContDistance < j.rawContDistance);
}

static bool lessIntegratedDist(::TianShanIce::Storage::CacheCandidate i, ::TianShanIce::Storage::CacheCandidate j)
{
	return (i.distance < j.distance);
}

::TianShanIce::Storage::CacheCandidates CacheStoreImpl::getCandidatesOfContent(const ::std::string& contentName, bool withLoadConsidered, const ::Ice::Current& c)
{
	::TianShanIce::Storage::CacheCandidates candidates;
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, getCandidatesOfContent, "content[%s] withLoadConsidered[%c]"), contentName.c_str(), withLoadConsidered? 'T': 'F');

	// step 1. generate the hash key of content name
	HashKey contentKey;
	if (contentName.empty() || std::string::npos != contentName.find(ILLEGAL_NAME_CHARS) || !_calcHashKey(contentKey, contentName.c_str(), contentName.length()))
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (_log, EXPFMT(CacheStore, 400, "getCandidatesOfContent() invalid contentName[%s] to query"), contentName.c_str());

	// step 2. get the store list of neighborhood
	CacheStoreListInt stores; 
	if (_listNeighorsEx(stores, true, _heatbeatInterval) <=0)
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 500, "getCandidatesOfContent() content[%s] _listNeighorsEx() returns empty list"), contentName.c_str());

	// step 3. calculate the raw distances
	for (CacheStoreListInt::iterator it = stores.begin(); it < stores.end(); it++)
	{
		::TianShanIce::Storage::CacheCandidate cc;
		cc.distance = -1;
		cc.isContentEdge = false;
		cc.csd = it->desc;
		cc.isSelf = (0 ==_contentStore._netId.compare(cc.csd.netId));
		if (!cc.isSelf && it->penalty >0)
		{
			if (CacheStoreImpl::sfLoggingDomainMessages & _flags)
				_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, getCandidatesOfContent, "content[%s] skip candidate[%s] per its penalty[%d]"), contentName.c_str(), cc.csd.netId.c_str(), it->penalty);
			continue;
		}

		cc.rawContDistance = _calcRawDistance(contentKey, it->cskey);
		candidates.push_back(cc);
	}

	::std::sort(candidates.begin(), candidates.end(), lessRawDist);
	candidates[0].isContentEdge = true;

	if (!withLoadConsidered)
		return candidates;

	// step 3. apply the load into distances
	::TianShanIce::Storage::CacheCandidates tmpCandidates;
	for (size_t i = 0; i < candidates.size(); i++)
	{
		// it is unnecessary to enumlate all the known store in the neighorhood
		if (_maxCandidates >0 && i >= _maxCandidates)
			break;

#pragma message ( __TODO__ "fix units and adjust _maxCandidates here " )
		::TianShanIce::Storage::CacheCandidate& tmpCand = candidates[i];
		double d = _distSqrByRankTbl[i];

		if (tmpCand.csd.loadStream > CACHE_LOAD_MAX)
			tmpCand.csd.loadStream = CACHE_LOAD_MAX;

		if (tmpCand.csd.loadImport > CACHE_LOAD_MAX)
			tmpCand.csd.loadImport = CACHE_LOAD_MAX;

		if (tmpCand.csd.loadCacheWrite > CACHE_LOAD_MAX)
			tmpCand.csd.loadCacheWrite = CACHE_LOAD_MAX;

		float integratedLoad = max((float)tmpCand.csd.loadStream / CACHE_LOAD_MAX, (float)tmpCand.csd.loadImport / CACHE_LOAD_MAX);
		// candidate of content will not consider the write load of the node: integratedLoad = max(integratedLoad, (float)tmpCand.csd.loadCacheWrite / CACHE_LOAD_MAX);

		d += (double) integratedLoad * integratedLoad * _loadWeightTbl[i];

		tmpCand.distance = (Ice::Int) (d * CACHE_DIST_MAX);
		tmpCandidates.push_back(tmpCand);
	}

	candidates = tmpCandidates;
	::std::sort(candidates.begin(), candidates.end(), lessIntegratedDist);

	if (CacheStoreImpl::sfLoggingDomainMessages & _flags)
	{
		std::string candstr;
		char buf[100];
		int64 stampNow = ZQ::common::now();
		for (::TianShanIce::Storage::CacheCandidates::iterator itC = candidates.begin(); itC < candidates.end(); itC++)
		{
			snprintf(buf, sizeof(buf)-2, "%s[%d/%d/%d@%d]%d, ",	itC->csd.netId.c_str(),
				itC->csd.loadStream, itC->csd.loadImport, itC->csd.loadCacheWrite,
				(long) (stampNow -itC->csd.stampAsOf), itC->distance);
			candstr += buf;
		}
	
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, getCandidatesOfContent, "content[%s] candidates %s load: %s"), contentName.c_str(), withLoadConsidered? "w/": "w/o", candstr.c_str());
	}

	return candidates;
}

// typedef std::queue<TianShanIce::Storage::CacheCandidate, TianShanIce::Storage::CacheCandidates> CacheCandidateQueue;

static void cbLogLine(const char* line, void* pCtx)
{
	CacheStoreImpl* pStore = (CacheStoreImpl*) pCtx;
	if (!pStore)
		return;
	pStore->_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheStore, "%s"), line);
}

::TianShanIce::Streamer::StreamPrx CacheStoreImpl::exportContentAsStream(const ::std::string& contentName, const ::std::string& subFileIn, ::Ice::Int idleStreamTimeout, ::Ice::Int cacheStoreDepth, const ::TianShanIce::SRM::ResourceMap& resourceRequirement, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	::TianShanIce::Streamer::StreamPrx stream;
	::TianShanIce::SRM::ResourceMap resRequirement = resourceRequirement; // make a writeable copy

	std::string requestSig = std::string("sub[") +subFileIn +"]";
	::TianShanIce::Properties::const_iterator itParam = params.find(SYS_PROP("clientSession"));
	
	char buf[100];
	if (params.end() != itParam)
		requestSig += std::string("csess[") + itParam->second + "]";
	else if (c.con)
	{
		snprintf(buf, sizeof(buf)-2, "%p(%d)", c.con.get(), c.requestId); 
		requestSig += buf;
	}

	if (sfLoggingResource & _flags)
	{
		snprintf(buf, sizeof(buf)-2, "content[%s] %s resRequirement", contentName.c_str(), requestSig.c_str());
		dumpResourceMap(resRequirement, buf, cbLogLine, this);
	}

	itParam = params.find(SYS_PROP("client"));
	if (params.end() != itParam)
		requestSig += std::string("cilent[") + itParam->second + "]";

	bool bExposeIndex = false;
	itParam = params.find(SYS_PROP("exposeIndex"));
	if (params.end() != itParam && 0 != atol(itParam->second.c_str()))
		bExposeIndex = true;

#define EXPSTRMFMT(_X) FLOGFMT(CacheStore, exportContentAsStream, "content[%s] %s " _X), contentName.c_str(), requestSig.c_str()

	int64 stampStart = ZQ::common::now();
	std::string strmstr;

	std::string subFile = subFileIn;

	if (contentName.empty() || std::string::npos != contentName.find(ILLEGAL_NAME_CHARS))
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (_log, EXPFMT(CacheStore, 400, "exportContentAsStream() invalid contentName[%s] to query"), contentName.c_str());
	
	// step 1. forward the invocation to the contentEdge if needed
	bool bHandleLocally = (cacheStoreDepth >0);
	TianShanIce::Storage::CacheCandidates candidates;

	if (bHandleLocally)
		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("initialize with local exporting per depth[%d]"), cacheStoreDepth);
	else
		candidates = getCandidatesOfContent(contentName, true, c);

	int64 stampNow  = ZQ::common::now();
	int64 stampList = stampNow;
	int32 latency = 0;

	// step 1.1 select from the candidates
	size_t i =0, totalCands = candidates.size();
	for (i =0; !bHandleLocally && !candidates.empty() && (_maxCandidates <=0 || i <_maxCandidates) ; i++)
	{
		// take the first item of the list
		TianShanIce::Storage::CacheCandidate topCand = *candidates.begin();
		candidates.erase(candidates.begin());

		// quit scanning if the top pick is self
		if (topCand.isSelf)
		{
			bHandleLocally = true;
			_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("self[%s] becomes the top pick of from %d/%d candidates, quit forwarding"), _contentStore._netId.c_str(), i+1, totalCands);
			break;
		}

		int penalty =0;

		// try to forward the exportContentAsStream() to the ContentEdge
		int64 stampStartForwarding = ZQ::common::now();
		try {
			stream = NULL;

#pragma message ( __TODO__ "quit if the subtotal latency (stampStartForwarding - stampStart) has exceeded a number of msec")

			if (topCand.csd.theStore)
			{
				std::string storeEp = _adapter->getCommunicator()->proxyToString(topCand.csd.theStore);
				_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("forwarding to %d-th ContentEdge[%s]/load[%d] via endpoint[%s]"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, storeEp.c_str());
				stream = topCand.csd.theStore->exportContentAsStream(contentName, subFile, idleStreamTimeout, cacheStoreDepth+1, resRequirement, params);
				_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("forward tried to %d-th ContentEdge[%s]/load[%d] => strm %s"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, stream? "created" : "null");
			}
			else _log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("%d-th ContentEdge[%s]/[%d] connection to store not yet established"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream);

			stampNow = ZQ::common::now();
			if (stream)
			{
				countExport(ec_forward, (int32)(stampNow - stampStartForwarding));
				strmstr  = _adapter->getCommunicator()->proxyToString(stream);
				_log(ZQ::common::Log::L_INFO, EXPSTRMFMT("successfully exported as stream[%s] from %d-th ContentEdge[%s]/load[%d], latency[%lld/%lld]ms"),
					 strmstr.c_str(), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, stampNow - stampStartForwarding, stampNow - stampStart);

				return stream;
			}
		}
		catch(const ::TianShanIce::ClientError& ex)
		{
			// if any top candidate indicate errors in inputted parameter, quit the loop to try the next candidates
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("quit trying candidates per %d-th ContentEdge[%s]/load[%d] threw %s: %s"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, ex.ice_name().c_str(), ex.message.c_str());
			return NULL;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("caught exception at forwarding to %d-th ContentEdge[%s]/load[%d], %s: %s"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const Ice::SocketException& ex)
		{
			penalty  = _penaltyOfFwdFail;
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("caught exception at forwarding to %d-th ContentEdge[%s]/load[%d], %s, charge penalty[%d]"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, ex.ice_name().c_str(), penalty);
		}
		catch(const Ice::TimeoutException& ex)
		{
			penalty  = _penaltyOfFwdFail/2 +1;
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("caught exception at forwarding to %d-th ContentEdge[%s]/load[%d], %s, charge penalty[%d]"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, ex.ice_name().c_str(), penalty);
		}
		catch(const Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("caught exception at forwarding to %d-th ContentEdge[%s]/load[%d], %s"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream, ex.ice_name().c_str());
		}
		catch(...)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("caught exception at forwarding to %d-th ContentEdge[%s]/load[%d]"), i+1, topCand.csd.netId.c_str(), topCand.csd.loadStream);
		}

		countExport(ec_forward, (int32)(ZQ::common::now() - stampStartForwarding), false);
		OnForwardFailed(topCand.csd.netId, penalty);
	}

	stampNow = ZQ::common::now();
	if (!bHandleLocally && !stream)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("failed to find CacheStore to host the request after %i tries and [%llu]msec"), i, stampNow - stampList);
		return NULL;
	}

	_log(ZQ::common::Log::L_INFO, EXPSTRMFMT("finished finding external CacheStore: %i tries and [%llu]msec; creating stream locally"), i, stampNow - stampStart);

	// step 2. read the resourceRequirement, adjust if needed
	// step 2.1 prepare the local PlaylistItemSetupInfo meanwhile
	TianShanIce::Streamer::PlaylistItemSetupInfo plisi;
	plisi.inTimeOffset = plisi.outTimeOffset = 0;
	plisi.criticalStart = 0;
	plisi.spliceIn = plisi.spliceOut = plisi.forceNormal = false;

	std::string hashFolderName = getFolderNameOfContent(contentName, c);
	plisi.contentName = _rootVolName + hashFolderName + contentName;

	if (subFile.empty())
		subFile = "*";
	else
	{
		size_t pos = subFile.find_first_not_of(".");
		if (0 != pos)
			subFile = subFile.substr(pos);
	}

	TianShanIce::Properties contentNameToken;
	portalContentNameToken(*this, contentName, contentNameToken);
	// fill the name tokens into the prviate data of playlist item
	for (TianShanIce::Properties::iterator itCT =contentNameToken.begin(); itCT != contentNameToken.end(); itCT++)
		ZQTianShan::Util::updateValueMapData( plisi.privateData, itCT->first, itCT->second);

	// playlist item's privateData => plisi.privateData
	{
		::TianShanIce::Variant v;
		v.bRange = false;
		v.type = ::TianShanIce::vtStrings;

		v.strs.push_back(subFile);
		MAPSET(::TianShanIce::ValueMap, plisi.privateData, CDN_SUBTYPE, v);
		MAPSET(::TianShanIce::ValueMap, plisi.privateData, CDN_EXTENSIONNAME, v);
		
		std::string tmpstr = std::string(CDN_SUBTYPE "[") +subFile +"] ";
		// copy the attended params
		for (::TianShanIce::Properties::const_iterator itParam = params.begin(); itParam !=params.end(); itParam++)
		{
			v.strs.clear();
			v.strs.push_back(itParam->second);
			tmpstr += itParam->first + "[" + itParam->second + "] ";
			MAPSET(::TianShanIce::ValueMap, plisi.privateData, itParam->first, v);
		}

		// take the portal the chop the content name into token and metadata, such as PID, PAID and so on

		for (TianShanIce::Properties::iterator itToken = contentNameToken.begin(); itToken != contentNameToken.end(); itToken++)
		{
			tmpstr += itToken->first + "[" + itToken->second + "] ";
			v.strs.push_back(itToken->second);
			MAPSET(::TianShanIce::ValueMap, plisi.privateData, itToken->first, v);
		}

		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("=> %s"), tmpstr.c_str());
	}

	bool bStreamFromLocalContent = false;
	::TianShanIce::Storage::ContentState localReplicaState = ::TianShanIce::Storage::csNotProvisioned;
	int32  transferBitrate = -1;
	std::string extSessionInterface;
	TianShanIce::Properties strmParams;

	if (!_contentAttrCache)
	{
		// step 2.2 check if there is local content replica
		::TianShanIce::Storage::ContentPrx content;

		try {
			_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("checking local replica[%s]"), plisi.contentName.c_str());
			_contentStore.openFolderEx(_rootVolName + hashFolderName, true, 0, c); // to ensure the subfolder exists
			content = _contentStore.openContentByFullname(plisi.contentName, c);

			if (content)
			{
				TianShanIce::Properties metaData = content->getMetaData();
				TianShanIce::Properties::iterator itMD = metaData.find(SYS_PROP(State));
				if (metaData.end() != itMD)
					localReplicaState = ContentStateBase::stateId(itMD->second.c_str());

				itMD = metaData.find(METADATA_BitRate);
				if (metaData.end() != itMD)
					transferBitrate = atol(itMD->second.c_str());

				_log(ZQ::common::Log::L_INFO, EXPSTRMFMT("found local replica[%s:%s(%d)] encoded bitrate[%d]"), 
					plisi.contentName.c_str(), ContentStateBase::stateStr(localReplicaState), localReplicaState, transferBitrate);
			}
		}
		catch(const Ice::ObjectNotExistException& ex)
		{
			_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("accessing local replica[%s], caught %s"), plisi.contentName.c_str(), ex.ice_name().c_str());
			content = NULL;
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("accessing local replica[%s], caught %s: %s"), plisi.contentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("accessing local replica[%s], caught %s"), plisi.contentName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("accessing local replica[%s], caught exception"), plisi.contentName.c_str());
		}

		// step 2.3 adjust the resource[rtTsUpstreamBandwidth] per the status of local content replica
		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("adjusting resource[UpstreamBandwidth] per state[%s(%d)]"), ContentStateBase::stateStr(localReplicaState), localReplicaState);

		std::string strmUsage;
		switch (localReplicaState)
		{
		case ::TianShanIce::Storage::csProvisioningStreamable:
		case ::TianShanIce::Storage::csInService:
			// local replica is fully residented, no need for upstream resource
			_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("local replica[%s:%s(%d)] wipe resource TsUpstreamBandwidth"), 
				plisi.contentName.c_str(), ContentStateBase::stateStr(localReplicaState), localReplicaState);

			OnLocalContentRequested(contentName, subFile); // trigger the couting too

			{
				ZQ::common::MutexGuard g(_lockerNeighbors);
				if (_maxLocalStreamKbps <=0 || ( (_usedLocalStreamKbps + RoundToKbps(transferBitrate)) < _maxLocalStreamKbps  && _usedLocalStreamKbps < (_maxLocalStreamKbps*0.95)) )
					bStreamFromLocalContent = true;
				char tmp[100];
				snprintf(tmp, sizeof(tmp)-2, "%d+%d /%d", _usedLocalStreamKbps, RoundToKbps(transferBitrate), _maxLocalStreamKbps);
				strmUsage = tmp;
			}

			break;

		case ::TianShanIce::Storage::csNotProvisioned:
			// should count the missed only after the content is confirmed available on remote source storage
			// OnMissedContentRequested(contentName, subFile); // trigger the counting too
			// Note: no "break;" here

		default:
			break;
		} // switch (localReplicaState)

		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("determined StreamFromLocalContent[%c] per state[%s(%d)] and stream-usage[%s]"), bStreamFromLocalContent?'T':'F', ContentStateBase::stateStr(localReplicaState), localReplicaState, strmUsage.c_str());

	} // if _contentAttrCache

	if (bStreamFromLocalContent)
		resRequirement.erase(::TianShanIce::SRM::rtTsUpstreamBandwidth);
	else
	{
		::TianShanIce::SRM::ResourceMap::iterator itRes = resRequirement.find(::TianShanIce::SRM::rtTsUpstreamBandwidth);
		if (resRequirement.end() == itRes)
			ZQTianShan::_IceThrow<TianShanIce::ClientError> (_log, EXPFMT(CacheStore, 400, "exportContentAsStream() content[%s] local replica[%s:%s(%d)] failed at no resource TsUpstreamBandwidth specified"), 
					contentName.c_str(), plisi.contentName.c_str(), ContentStateBase::stateStr(localReplicaState), localReplicaState);

		::TianShanIce::ValueMap& upstrmdata = itRes->second.resourceData;

		if (upstrmdata.end() != upstrmdata.find("bandwidth"))
		{
			::TianShanIce::Variant& vBR = upstrmdata["bandwidth"];
			if (vBR.lints.size() >0)
				transferBitrate = (int32) vBR.lints[0];
		}

		{
			::TianShanIce::Variant v;
			v.type = ::TianShanIce::vtStrings;
			v.bRange = false;
			v.strs.push_back(_upStreamBindIP);
			MAPSET(::TianShanIce::ValueMap, itRes->second.resourceData, "bindAddress", v);
		}

		int64 stampStartExtLocate = ZQ::common::now();

		//TODO: ?? take some default "bandwidth" if the input resourceRequirement doesn't carry it

		if (upstrmdata.end() == upstrmdata.find("sessionURL")) // ELSE: the inputd resource requirement has already specified sessionURL, take it and ignore sessionInterface
		{
			// the input resource requirement has NOT specified sessionURL, associated it
			if (upstrmdata.end() != upstrmdata.find("sessionInterface"))
			{
				::TianShanIce::Variant& v = upstrmdata["sessionInterface"];
				if (v.strs.size() >0 && std::string::npos == v.strs[0].find_first_of("$"))
				{
					extSessionInterface = v.strs[0];
					_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("client requested to take sessionInterface[%s]"), extSessionInterface.c_str());
				}
			}

			if (extSessionInterface.empty())
			{
				// no sessionInterface specified by the input resource requirement
				_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("looking up the external session interface"));
				extSessionInterface = findExternalSessionInterfaceByContent(contentName, contentNameToken);
				if (!extSessionInterface.empty())
				{
					_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("found the external session interface[%s], adding it to res[TsUpstreamBandwidth]"), extSessionInterface.c_str());

					::TianShanIce::Variant v;
					v.type = ::TianShanIce::vtStrings;
					v.bRange = false;
					v.strs.push_back(extSessionInterface);
					MAPSET(::TianShanIce::ValueMap, upstrmdata, "sessionInterface", v);
				}
				else if (_bProxySessionBook)
					ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 400, "exportContentAsStream() failed to find external session interface for contentName[%s] to query"), contentName.c_str());

				//TODO: possible to map multiple sessionInterface for a PID??
			}

			if (_bProxySessionBook && !extSessionInterface.empty() && 0 != subFile.compare("*")) // this is an external source library
			{
				std::string sessionURL;
				_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("calling portal for sessionURL"));
				int nRetCode = portalBookTransferSession(*this, sessionURL, contentNameToken, subFile, extSessionInterface, transferBitrate, strmParams);
				if (0 == nRetCode)
				{
					::TianShanIce::Variant v;
					v.type = ::TianShanIce::vtStrings;
					v.bRange = false;
					v.strs.push_back(sessionURL);
					MAPSET(::TianShanIce::ValueMap, upstrmdata, "sessionURL", v);
					_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("reserved tranferSession[%s], adding it to res[TsUpstreamBandwidth]"), sessionURL.c_str());

					countExport(ec_remoteLocate, (int32)(ZQ::common::now() - stampStartExtLocate));

					if (::TianShanIce::Storage::csNotProvisioned == localReplicaState)
						OnMissedContentRequested(contentName, subFile); // trigger the counting too
				}
				else
				{
					countExport(ec_remoteLocate, (int32)(ZQ::common::now() - stampStartExtLocate), false);
					ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (_log, EXPFMT(CacheStore, nRetCode, "exportContentAsStream() content[%s] failed to locate at source storage[%s]"), contentName.c_str(), extSessionInterface.c_str());
				}
			}
		}

	} // if (!bStreamFromLocalContent)

	// step 3. call StreamService to create stream
	if (!_prxStreamService)
	{
		_log(ZQ::common::Log::L_WARNING, EXPSTRMFMT("failed at creating local stream per no StreamService attached"));
		return NULL;
	}

	int64 stampStartLocal = stampNow;
	::TianShanIce::Streamer::PlaylistPrx playlist;
	try {
		char buf[64];
		snprintf(buf, sizeof(buf)-2, "%d", idleStreamTimeout);
		MAPSET(::TianShanIce::Properties, strmParams, SYS_PROP(IdleTimeout), buf);

		if (bExposeIndex)
			MAPSET(::TianShanIce::Properties, strmParams, SYS_PROP(exposeIndex), "1");
	
		if (sfLoggingResource & _flags)
		{
			snprintf(buf, sizeof(buf)-2, "content[%s] %s localstrm res", contentName.c_str(), requestSig.c_str());
			dumpResourceMap(resRequirement, buf, cbLogLine, this);
		}

		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("creating stream on local streamer"));
		stream = _prxStreamService->createStreamByResource(resRequirement, strmParams);
#ifndef EXTERNAL_STREAMSVC
		// this cacheStore IS embedded with a StreamSvc
		stream = ::TianShanIce::Streamer::StreamPrx::uncheckedCast(stream->ice_collocationOptimized(false));
#endif //EXTERNAL_STREAMSVC
		playlist = ::TianShanIce::Streamer::PlaylistPrx::checkedCast(stream);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		stream = NULL;
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("creating local stream caught %s: %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		stream = NULL;
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("creating local stream caught %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		stream = NULL;
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("creating local stream caught exception"));
	}

	stampNow = ZQ::common::now();
	
	if (!playlist)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("failed to create local stream, took %llu/%llumsec"),
				stampNow - stampStartLocal, stampNow - stampStart);

		countExport(ec_local, (int32)(stampNow - stampStartLocal), false);
		return NULL;
	}

	strmstr = _adapter->getCommunicator()->proxyToString(stream);

	// step 4. render and commit the stream
	bool bDirtyContentStoreDB =false;
	int64 stampCreated = stampNow;
	try {
		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("replica[%s] is being rendered onto local stream[%s]"), plisi.contentName.c_str(), strmstr.c_str());
		playlist->pushBack(1, plisi); // always take userCtrlNum =1

		int64 stampRendered = ZQ::common::now();
		
		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("committing local stream[%s]"), strmstr.c_str());
		playlist->commit();

		stampNow = ZQ::common::now();
		countExport(ec_local, (int32)(stampNow - stampStartLocal));

		_log(ZQ::common::Log::L_INFO, EXPSTRMFMT("prepared local stream[%s], took total [%d]msec: scan[%d]msec localcreate[%d]msec render[%d]msec commit[%d]msec"), strmstr.c_str(), (int)(stampNow-stampStart), 
			(int)(stampStartLocal-stampList), (int)(stampCreated -stampStartLocal), (int)(stampRendered -stampCreated), (int)(stampNow -stampRendered));
		return stream;
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("preparing stream[%s] caught %s(%d): %s"), strmstr.c_str(), ex.ice_name().c_str(), ex.errorCode, ex.message.c_str());
		if (404 == ex.errorCode && bStreamFromLocalContent)
			bDirtyContentStoreDB = true;
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("preparing stream[%s] caught %s"), strmstr.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("preparing stream[%s] caught exception"), strmstr.c_str());
	}

	// when program reach here, the stream creating has already failed, destory the stream if created
	stampNow = ZQ::common::now();
	countExport(ec_local, (int32)(stampNow - stampStartLocal), false);
	if (bDirtyContentStoreDB)
	{
		::Ice::Identity identFolder;
		identFolder.category = DBFILENAME_Volume;
		identFolder.name = _rootVolName + hashFolderName;

		// cut off the ending /
		size_t pos = identFolder.name.find_last_not_of("/\\");
		if (pos < identFolder.name.length())
			identFolder.name = identFolder.name.substr(0, pos+1);

		_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("ContentDB may be out-of-date at folder[%s]"), identFolder.name.c_str());

		try {
			// the ContentStore DB may be out of date with the file system because long time no sync
			// try to start sync it

			// step 1. read the SYS_PROP(StampLastSyncWithFileSystem)
			::TianShanIce::Storage::FolderPrx folder = _contentStore.openFolderEx(identFolder.name, true, 0, c);
			::TianShanIce::Storage::VolumeExPrx vol = ::TianShanIce::Storage::VolumeExPrx::checkedCast(folder);
#ifndef EXTERNAL_STREAMSVC
			// this cacheStore IS embedded with a StreamSvc
			vol = ::TianShanIce::Storage::VolumeExPrx::checkedCast(folder->ice_collocationOptimized(false));
#endif //EXTERNAL_STREAMSVC

			::TianShanIce::Storage::VolumeInfo vi = vol->getInfo();
			int64 stampLastSync =0;
			::TianShanIce::Properties::iterator itMD = vi.metaData.find(SYS_PROP(StampLastSyncWithFileSystem));
			if (vi.metaData.end() != itMD)
			{
				_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("ContentDB may be out-of-date at folder[%s], lastSyncFs[%s]"), identFolder.name.c_str(), itMD->second.c_str());
				stampLastSync = ZQ::common::TimeUtil::ISO8601ToTime(itMD->second.c_str());
			}

			// issue a SyncFSCmd to refresh sync
			if (stampLastSync < stampNow - 30*60*1000) // half hr
			{
				_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("refreshing ContentDB at folder[%s] that may be out-of-date"), identFolder.name.c_str());
				(new ZQTianShan::ContentStore::SyncFSCmd(_contentStore, identFolder))->execute();

				// update a dummy stamp to avoid sync too many times
				::TianShanIce::Properties newMD;
				ZQTianShan::TimeToUTC(stampNow -10*60*1000, buf, sizeof(buf) -2);
				MAPSET(::TianShanIce::Properties, newMD, SYS_PROP(StampLastSyncWithFileSystem), buf);
				vol->setMetaData(newMD);
			}
			else
			{
				::Ice::Identity identContent;
				identContent.category = DBFILENAME_Content;
				identContent.name = plisi.contentName;
				_log(ZQ::common::Log::L_DEBUG, EXPSTRMFMT("refreshing attributes of local Content[%s] that may be out-of-date"), identContent.name.c_str());
				(new ZQTianShan::ContentStore::PopulateFileAttrsCmd(_contentStore, identContent))->execute();
			}

		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("refreshing out-of-date ContentDB at folder[%s] caught %s: %s"), identFolder.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("refreshing out-of-date ContentDB at folder[%s] caught %s"), identFolder.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("refreshing out-of-date ContentDB at folder[%s] caught exception"), identFolder.name.c_str());
		}
	}

	_log(ZQ::common::Log::L_WARNING, EXPSTRMFMT("local stream[%s] failed to complete render and commit, giving it up"), strmstr.c_str());
	try {
		if (stream)
			stream->destroy();
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("destroying stream[%s] caught %s: %s"), strmstr.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("destroying stream[%s] caught %s"), strmstr.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, EXPSTRMFMT("destroying stream[%s] caught exception"), strmstr.c_str());
	}

	stream = NULL;
	return stream;
}

void CacheStoreImpl::resetCounters()
{
	ZQ::common::MutexGuard g(_lkCounters);
	memset(_exportCounters, 0x00, sizeof(_exportCounters));
	_exportCounters[ec_forward].name      = "FowardWithinDomain";
	_exportCounters[ec_remoteLocate].name = "RemoteLocate";
	_exportCounters[ec_local].name        = "LocalStream";

	_stampMesureSince = ZQ::common::now();
}

void CacheStoreImpl::countExport(ecIndex ecIdx, int32 latencyMsec, bool succ)
{
	bool resetNeeded = false;
	do
	{
		ExportCount& ec = _exportCounters[ecIdx % ec_max];
		ZQ::common::MutexGuard g(_lkCounters);
		if (++ec.count <=0)
		{
			resetNeeded = true;
			break;
		}

		if (!succ && ++ec.failCount <=0)
		{
			resetNeeded = true;
			break;
		}

		if ((ec.latencyTotal+=latencyMsec) <=0)
		{
			resetNeeded = true;
			break;
		}

		if (ec.latencyMax < latencyMsec)
			ec.latencyMax = latencyMsec;
	} while(0);

	if (resetNeeded)
		resetCounters();
}

::TianShanIce::Storage::ContentAccess CacheStoreImpl::getAccessCount(const ::std::string& contentName, const ::Ice::Current& c)
{
	::TianShanIce::Storage::AccessCounter counter;
	counter.base.contentName = contentName;
	counter.base.accessCount = 0;
	counter.base.stampLatest = counter.base.stampSince =0;

	if (_acMissed->get(contentName, counter))
	{
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, getAccessCount, "content[%s] found in the missed content list"), contentName.c_str());
		return counter.base;
	}

	if (_acHotLocals->get(contentName, counter))
	{
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, getAccessCount, "content[%s] found in the hot local list"), contentName.c_str());
		return counter.base;
	}

	counter.base.accessCount = 0;
	counter.base.stampLatest = counter.base.stampSince =0;

	try {
		std::string replicaName = _rootVolName + getFolderNameOfContent(contentName, c);
		replicaName += contentName;
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, getAccessCount, "content[%s] looking up the contentStore DB, localReplica[%s]"), contentName.c_str(), replicaName.c_str());
		::TianShanIce::Storage::ContentPrx content = _contentStore.openContentByFullname(replicaName, c);
		::TianShanIce::Properties metaData;
		if (!content)
			return counter.base;

		int64 stampNow = ZQ::common::now();

		metaData = content->getMetaData();

		::TianShanIce::Properties::iterator itMD = metaData.find(METADATA_AccessCount);
		if (metaData.end() != itMD)
			counter.base.accessCount = atoi(itMD->second.c_str());
		else counter.base.accessCount = 0;

		itMD = metaData.find(METADATA_AccessCountSince);
		if (metaData.end() != itMD)
			counter.base.stampSince = ZQ::common::TimeUtil::ISO8601ToTime(itMD->second.c_str());
		else counter.base.stampSince = stampNow;

		itMD = metaData.find(METADATA_AccessCountLatest);
		if (metaData.end() != itMD)
				counter.base.stampLatest = ZQ::common::TimeUtil::ISO8601ToTime(itMD->second.c_str());
		else counter.base.stampLatest = stampNow;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, getAccessCount, "content[%s] access ContentDB caught ObjectNotExistException"), contentName.c_str());
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, getAccessCount, "content[%s] access ContentDB caught %s: %s"), contentName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, getAccessCount, "content[%s] access ContentDB caught %s"), contentName.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, getAccessCount, "content[%s] access ContentDB caught exception"), contentName.c_str());
	}

	return counter.base;
}

void CacheStoreImpl::addAccessCount(const ::std::string& contentName, ::Ice::Int countToAdd, const ::std::string& since, const ::Ice::Current& c)
{
	if (countToAdd <=0)
		return;

#pragma message ( __TODO__ "impl here")

}

void CacheStoreImpl::listMissedContents_async(const ::TianShanIce::Storage::AMD_CacheStore_listMissedContentsPtr& amdCB, ::Ice::Int maxNum, const ::Ice::Current& c)
{
	try {
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, listMissedContents, "maxNum[%d]"), maxNum);
		(new ListMissedContentsCmd(*this, amdCB, maxNum))->exec();
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, getMissedContentList, "failed to generate ListMissedContentsCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("CacheStore", 501, "failed to generate ListMissedContentsCmd"));
	}
}

void CacheStoreImpl::listHotLocals_async(const ::TianShanIce::Storage::AMD_CacheStore_listHotLocalsPtr& amdCB, ::Ice::Int maxNum, const ::Ice::Current& c)
{
	try {
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, listHotLocals, "maxNum[%d]"), maxNum);
		(new ListHotLocalsCmd(*this, amdCB, maxNum))->exec();
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, getMissedContentList, "failed to generate ListMissedContentsCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("CacheStore", 501, "failed to generate ListMissedContentsCmd"));
	}
}

void CacheStoreImpl::setAccessThreshold(::Ice::Int timeWinOfPopular, ::Ice::Int countOfPopular, ::Ice::Int hotTimeWin, const ::Ice::Current& c)
{
	ZQ::common::MutexGuard g(_cfgLock);
	_timeWinOfPopular = timeWinOfPopular;
	_countOfPopular   = countOfPopular;
	_hotTimeWin       = hotTimeWin;
}

void CacheStoreImpl::getAccessThreshold(::Ice::Int& timeWinOfPopular, ::Ice::Int& countOfPopular, ::Ice::Int& hotTimeWin, const ::Ice::Current& c)
{
	ZQ::common::MutexGuard g(_cfgLock);
	timeWinOfPopular = _timeWinOfPopular;
	countOfPopular   = _countOfPopular;
	hotTimeWin       = _hotTimeWin;
}

void CacheStoreImpl::_readStoreSpace(int64& freeMB, int64& totalMB)
{
	// assuming all the cache folders are under a same volume, and share the same space limitation
	do { 

		static int64 stampLastSpaceCheck =0;
		static int64 sharedFreeSpaceMB=0, sharedTotalSpaceMB=0;
		static ZQ::common::Mutex lkSpaceCheck;

		int64 stampNow = ZQ::common::now();

		if (sharedTotalSpaceMB <=0 || stampNow - stampLastSpaceCheck > 1000*10)
		{
			ZQ::common::MutexGuard g(lkSpaceCheck);
			if (sharedTotalSpaceMB >0 && stampNow - stampLastSpaceCheck <= 1000*10)
				break;

			try {
				_rootVol->getCapacity(sharedFreeSpaceMB, sharedTotalSpaceMB);
				stampLastSpaceCheck = stampNow;

				if (_freeSpacePercent <5)
					_freeSpacePercent = 5;
				else if (_freeSpacePercent >50)
					_freeSpacePercent =50;

				if (_maxSpaceMB >0)
				{
					int64 diffMB = sharedTotalSpaceMB - _maxSpaceMB;
					if (diffMB > sharedFreeSpaceMB)
					{
						sharedTotalSpaceMB = _maxSpaceMB;
						sharedFreeSpaceMB  -=diffMB;
					}
					else if (diffMB >0)
					{
						sharedTotalSpaceMB = _maxSpaceMB;
						sharedFreeSpaceMB  = 0;
					}
				}

				storelog(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _readStoreSpace, "read volume space[F%lld/T%lld]MB"), sharedFreeSpaceMB, sharedTotalSpaceMB);
			}
			catch(const Ice::Exception& ex)
			{
				storelog(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, _readStoreSpace, "read volume space caught exception[%s]"), ex.ice_name().c_str());
			}
			catch(...)
			{
				storelog(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, _readStoreSpace, "read volume space caught exception"));
			}
		}

		freeMB  = sharedFreeSpaceMB;
		totalMB = sharedTotalSpaceMB;

	} while (0);
}

bool CacheStoreImpl::_ensureSpace(long neededMB, const std::string& forTopFolderName)
{
	// check if there is free enough space for this caching
	Ice::Long totalMB=0, freeMB=0, usedMB=0;
	Ice::Long sizeToFreeMB =neededMB;

//	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _ensureSpace, "size[%d]MB under TF[%s]"), neededMB, forTopFolderName.c_str());
	::Ice::Identity     identTop;
	identTop.category   = CATEGORY_TopFolder;

	if (forTopFolderName.empty())
	{
		// no specific to folder is given, initialize the sizeToFreeMB by _maxSpaceMB
		_readStoreSpace(freeMB, totalMB);
		sizeToFreeMB = totalMB * _freeSpacePercent /100 + neededMB - freeMB;
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _ensureSpace, "read space usage: [%lld]MB <= usage[U%lld/T%lld/F%lld]MB -[%lld]MB"), 
			neededMB, usedMB, totalMB, freeMB, (sizeToFreeMB >0 ? sizeToFreeMB:0));

		if (totalMB <=0)
			return false;
	}
	else
	{
		try {
			identTop.name       = forTopFolderName;
			::TianShanIce::Storage::TopFolderPrx topFolder = IdentityToObj(TopFolder, identTop);
			if (!topFolder->getSpaceUsage(totalMB, freeMB, usedMB))
				return false;

			sizeToFreeMB = totalMB * _freeSpacePercent /100 + neededMB - freeMB;

			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _ensureSpace, "TF[%s] read space usage: [%lld]MB <= usage[U%lld/T%lld/F%lld]MB -[%lld]MB"), 
				forTopFolderName.c_str(), neededMB, usedMB, totalMB, freeMB, (sizeToFreeMB >0 ? sizeToFreeMB:0));

			if (totalMB <=0)
				return false;

			if (sizeToFreeMB <=0)
				return true;

			Ice::Long confirmedMB =0;
			if (topFolder && sizeToFreeMB >0)
				topFolder->freeForSpace(sizeToFreeMB, confirmedMB);

			sizeToFreeMB -=confirmedMB;

			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _ensureSpace, "TF[%s] confirmed to free %lldMB for needed %lldMB"), 
				forTopFolderName.c_str(), confirmedMB, neededMB);

		}
		catch(const Ice::Exception& ex)
		{
			storelog(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, _ensureSpace, "TF[%s] caught exception[%s]"), forTopFolderName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			storelog(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, _ensureSpace, "TF[%s] caught exception"), forTopFolderName.c_str());
		}
	}

	if (sizeToFreeMB <=0)
		return true;

	FWUSequence fwuseq = _sortFolderByFWU();
	if (fwuseq.size() <=0)
		_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, _ensureSpace, "empty folder list by sorting FWUs"));

	for (size_t i =0; i < fwuseq.size() && sizeToFreeMB >0; i++)
	{
		if (!forTopFolderName.empty() && 0 == forTopFolderName.compare(fwuseq[i].topFolderName))
			continue;

		try {
			identTop.name  = fwuseq[i].topFolderName;

			if (_flags & CacheStoreImpl::sfLoggingUnpopularScan)
				_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _ensureSpace, "calling TF[%s] to free space[%lld]MB"), identTop.name.c_str(), sizeToFreeMB);

			::TianShanIce::Storage::TopFolderPrx topFolder = IdentityToObj(TopFolder, identTop);

			Ice::Long confirmedMB =0;
			if (topFolder)
				topFolder->freeForSpace(sizeToFreeMB, confirmedMB);

			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, _ensureSpace, "neighbor TF[%s] confirmed to free space[%lld] for [%lld]MB"), identTop.name.c_str(), confirmedMB, sizeToFreeMB);
			sizeToFreeMB -=confirmedMB;
		}
		catch(const Ice::Exception& ex)
		{
			storelog(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, _ensureSpace, "neighbor TF[%s] caught exception[%s]"), fwuseq[i].topFolderName.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			storelog(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, _ensureSpace, "neighbor TF[%s] caught exception"), fwuseq[i].topFolderName.c_str());
		}
	}

	return (sizeToFreeMB <=0);
}

#define bpsToFileSizeMB(bps, sec) (((int64)bps * sec /8 + 1024*1024 -1) /1024/1024)

void CacheStoreImpl::cacheContent(const ::std::string& contentName, const ::TianShanIce::Storage::CacheStorePrx& sourceCS, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	if (_contentAttrCache)
		return;

	::TianShanIce::Storage::CacheStorePrx sourceStore = sourceCS;

	//step 0. validate the input parameters
	if (contentName.empty() || std::string::npos != contentName.find(ILLEGAL_NAME_CHARS))
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (_log, EXPFMT(CacheStore, 400, "cacheContent() invalid contentName[%s]"), contentName.c_str());

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] sig[%s]"), contentName.c_str(), invokeSignature(c).c_str());
	int64 stampNow = ZQ::common::now();

	/*
	std::string startTime = startTimeStr;

	if (startTimeStr.empty() || stampNow > ZQ::common::TimeUtil::ISO8601ToTime(startTimeStr.c_str()))
	{
		char buf[40];
		if (ZQ::common::TimeUtil::TimeToUTC(stampNow, buf, sizeof(buf) -2))
			startTime = buf;

		_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, cacheContent, "content[%s] adjusted startTime[%s] to [%s]"), contentName.c_str(), startTimeStr.c_str(), startTime.c_str());
	}
*/

	// step 1. start with a CacheTask
	CacheTaskImpl::Ptr pTask = newCacheTask(contentName); // booked the bw according to available resource
	if (!pTask)
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 500, "cacheContent() contentName[%s] failed to allocate CacheTask"), contentName.c_str());

	if (pTask->bwMax <=0)
	{
	    pTask->destroy(c);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 500, "cacheContent() contentName[%s] server run out of propagation bandwidth"), contentName.c_str());
	}

	// step 2 validate if the content has or is being cached locally, otherwise create a new content
	// take the portal the chop the content name into token and metadata, such as PID, PAID and so on
	portalContentNameToken(*this, contentName, pTask->nameFields);
	std::string replicaName;
	std::string topFolderName, leafFolderName;

	do {
		std::string folderName;
		::TianShanIce::Storage::FolderPrx folder;

		try {
			folderName = _content2FolderName(contentName, topFolderName, leafFolderName);
//			::TianShanIce::Storage::VolumePrx vol = _contentStore.proxy(true)->openVolume(DEFAULT_VOLUME_STRING);
//			folder = vol->openSubFolder(top, true, 0);
//			folder = folder->openSubFolder(leaf, true, 0);
			folder = _contentStore.openFolderEx(topFolderName + LOGIC_FNSEPS + leafFolderName, true, 0, ::Ice::Current());
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, cacheContent, "content[%s] opening folder[%s] caught %s: %s"),
				contentName.c_str(), folderName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, cacheContent, "content[%s] opening folder[%s] caught %s"),
				contentName.c_str(), folderName.c_str(), ex.ice_name().c_str());
		}
		catch( ...)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, cacheContent, "content[%s] opening folder[%s] caught exception"),
				contentName.c_str(), folderName.c_str());
		}

		if (!folder)
		{
			pTask->destroy(c);
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 500, "cacheContent() content[%s] failed to access folder[%s]"), contentName.c_str(), folderName.c_str());
		}
		
		replicaName = folderName + contentName;

		::TianShanIce::Storage::ContentState state = TianShanIce::Storage::csNotProvisioned;

		try {
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] checking if localreplica[%s] exists"), contentName.c_str(), replicaName.c_str());
			pTask->localContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(folder->openContent(contentName, "", false));
			if (pTask->localContent)
				state = pTask->localContent->getState();
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] checking if localreplica[%s] exists caught %s: %s"),
				contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::ObjectNotExistException&)
		{
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] localreplica[%s] doesn't exist"), contentName.c_str(), replicaName.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] checking if localreplica[%s] exists caught %s"),
				contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str());
		}
		catch (...) 
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] checking if localreplica[%s] exists caught exception"),
				contentName.c_str(), replicaName.c_str());
		}

		if (pTask->localContent) // replica already exists
		{
			if (TianShanIce::Storage::csNotProvisioned == state)
				break; // good to call provision() again, otherwise give up with exception

		    pTask->destroy(c);
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_log, EXPFMT(CacheStore, 500, "cacheContent() content[%s] localReplica[%s] already exist with state[%s(%d)]"), contentName.c_str(), replicaName.c_str(), ContentStateBase::stateStr(state), state);
		}

		// no local replica if reaches here, create one
		try {
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] creating replica under hash folder[%s]"), contentName.c_str(), folderName.c_str());

			pTask->localContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(folder->openContent(contentName, "", true));
			if (pTask->localContent)
				_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, cacheContent, "content[%s] a replica has been created under hash folder[%s]"), contentName.c_str(), folderName.c_str());
			
			pTask->localContent->setUserMetaData2(pTask->nameFields); // adding the name fields as user metadata of the content
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, cacheContent, "content[%s] creating localreplica[%s] caught %s: %s"),
				contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, cacheContent, "content[%s] creating localreplica[%s] caught %s"),
				contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str());
		}
		catch (...) 
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheFolder, cacheContent, "content[%s] creating localreplica[%s] caught exception"),
				contentName.c_str(), replicaName.c_str());
		}

	} while (0);

	// double check if a replica has been opened to call provision()
	if (!pTask->localContent)
	{
		pTask->destroy(c);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 500, "cacheContent() content[%s] failed to open localReplica[%s] to cache"), contentName.c_str(), replicaName.c_str());
	}


	// step 3. build up the resource for exportAsStream from the source CacheStore
	int64 stampLast = stampNow;
	stampNow = ZQ::common::now();
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] read localReplica[%s], took %lldmsec, preparing resources"), contentName.c_str(), replicaName.c_str(), stampNow - stampLast);
	::TianShanIce::SRM::Resource resDownstreamBandwidth, resUpstreamBandwidth, resEthernetInterface;
	resDownstreamBandwidth.status = TianShanIce::SRM::rsRequested;
	resUpstreamBandwidth.status   = TianShanIce::SRM::rsRequested;

	// step 3.1 set the propagation speed
	{
		::TianShanIce::Variant v;
		v.type = ::TianShanIce::vtLongs;
		if (pTask->bwMin < pTask->bwMax)
		{
			v.bRange = true;
			v.lints.push_back(pTask->bwMin);
			v.lints.push_back(pTask->bwMax);
		}
		else
		{
			v.bRange = false;
			v.lints.push_back(pTask->bwMin);
		}

		MAPSET(::TianShanIce::ValueMap, resDownstreamBandwidth.resourceData, "bandwidth", v);
		MAPSET(::TianShanIce::ValueMap, resUpstreamBandwidth.resourceData, "bandwidth", v);
	}

	// step 3.2 determin the sessionInterface of the external storage

	// for the resource of rtTsUpstreamBandwidth
	std::string extSessionInterface;
	do {
		if (sourceStore)
			break; // no necessary to have rtTsUpstreamBandwidth if propagate from another CacheStore

		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore,  cacheContent, "content[%s] looking up the external session interface"), contentName.c_str());
		extSessionInterface = findExternalSessionInterfaceByContent(contentName, pTask->nameFields);
		if (extSessionInterface.empty())
			_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, cacheContent, "content[%s] failed to find external session interface"), contentName.c_str());
		else
		{
			::TianShanIce::Variant v;
			v.type = ::TianShanIce::vtStrings;
			v.bRange = false;
			v.strs.push_back(extSessionInterface);
			MAPSET(::TianShanIce::ValueMap, resUpstreamBandwidth.resourceData, "sessionInterface", v);
		}

	} while(0); // end of rtTsUpstreamBandwidth

	// step 3.3 By default assume this caching is from the external domain,
	// set the upstream interface as the destIP of resEthernetInterface for SS::exportContentAsStream()
	{
		::TianShanIce::Variant v;
		v.type = ::TianShanIce::vtStrings;
		v.bRange = false;
		v.strs.push_back(_upStreamBindIP);
		MAPSET(::TianShanIce::ValueMap, resEthernetInterface.resourceData, "destIP", v);
	}

	// step 4. try to create the source stream exported from other CacheStore
	std::string  srcStreamStr;

	// case 4.1 if the input parameter specified a source CacheStore
	if (sourceStore)
	{
		std::string tmpstr = _adapter->getCommunicator()->proxyToString(sourceStore);
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] asking specified source CacheStore[%s] to export content"), contentName.c_str(), tmpstr.c_str());

		try {
			std::string srcStoreNetId = sourceStore->theContentStore()->getNetId();

			bool bSourceStoreInNeighborhood = false;
			{
				::ZQ::common::MutexGuard gd(_lockerNeighbors);
				bSourceStoreInNeighborhood = (_neighbors.end() != _neighbors.find(srcStoreNetId));
			}

			if (bSourceStoreInNeighborhood)
			{
				_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] the source CacheStore[%s][%s] is in neighborhood, taking the downStreamIp[%s] to duplicate content"), contentName.c_str(), srcStoreNetId.c_str(), tmpstr.c_str(), _downStreamBindIP.c_str());

				::TianShanIce::Variant& v = resEthernetInterface.resourceData["destIP"];
				v.strs.clear();
				v.strs.push_back(_downStreamBindIP);
			}
		}
		catch(const ::Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, cacheContent, "content[%s] communicate source CacheStore[%s] caught %s"),
				contentName.c_str(), tmpstr.c_str(), ex.ice_name().c_str());
		}
		catch (...) 
		{
			_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, cacheContent, "content[%s] communicate source CacheStore[%s] caught exception"),
				contentName.c_str(), tmpstr.c_str());
		}

		::TianShanIce::SRM::ResourceMap exportResources;
		MAPSET(::TianShanIce::SRM::ResourceMap, exportResources, TianShanIce::SRM::rtTsDownstreamBandwidth, resDownstreamBandwidth);
		MAPSET(::TianShanIce::SRM::ResourceMap, exportResources, TianShanIce::SRM::rtEthernetInterface, resEthernetInterface);

		try {
			stampNow = ZQ::common::now();
			pTask->srcStream = sourceStore->exportContentAsStream(contentName, "", 20000, 1, exportResources, params);
			srcStreamStr = _adapter->getCommunicator()->proxyToString(pTask->srcStream);

			stampLast = stampNow;
			stampNow = ZQ::common::now();
			_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, cacheContent, "content[%s] source CacheStore[%s] exported content as stream[%s], took %lldmsec"), contentName.c_str(), tmpstr.c_str(), srcStreamStr.c_str(), stampNow - stampLast);
		}
		catch(const ::TianShanIce::BaseException& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] export from CacheStore[%s] caught %s: %s"),
				contentName.c_str(), tmpstr.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch(const ::Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] export from CacheStore[%s] caught %s"),
				contentName.c_str(), tmpstr.c_str(), ex.ice_name().c_str());
		}
		catch (...) 
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] export from CacheStore[%s] caught exception"),
				contentName.c_str(), tmpstr.c_str());
		}

		if (!pTask->srcStream) // when the sourceStore is specified, it must be a propagation between the CacheStore instances
		{ 
			pTask->destroy(c);
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (_log, EXPFMT(CacheStore, 400, "cacheContent() content[%s] failed to export from specified CacheStore[%s]"), contentName.c_str(), tmpstr.c_str());
		}

	}
	else if (sfEnableNeighborhoodPropagation & _flags)
	{
		// case 4.2 if the configuration prioritize propagation among neighborhood
		::TianShanIce::Variant& v = resEthernetInterface.resourceData["destIP"];
		v.strs.clear();
		v.strs.push_back(_downStreamBindIP);
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] trying to propagate among neighborhood, adjusted to take downStreamIp[%s] to duplicate content"), contentName.c_str(), _downStreamBindIP.c_str());

		::TianShanIce::SRM::ResourceMap exportResources;
		MAPSET(::TianShanIce::SRM::ResourceMap, exportResources, TianShanIce::SRM::rtTsDownstreamBandwidth, resDownstreamBandwidth);
		MAPSET(::TianShanIce::SRM::ResourceMap, exportResources, TianShanIce::SRM::rtEthernetInterface, resEthernetInterface);

		bool importFromExtBySelf = false;
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] sourceStore not specified, determining one from the neighborhood"), contentName.c_str());

		stampNow = ZQ::common::now();
		TianShanIce::Storage::CacheCandidates candidates = getCandidatesOfContent(contentName, true, c);
		for (size_t i=0; !importFromExtBySelf && i < candidates.size(); i++)
		{
			if (candidates[i].isSelf)
			{
				importFromExtBySelf = true;
				break;
			}

			if (!candidates[i].csd.theStore && candidates[i].csd.loadStream >= CACHE_LOAD_MAX)
				continue;

			std::string tmpstr = _adapter->getCommunicator()->proxyToString(candidates[i].csd.theStore);

			try {
				_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] asking candidate CacheStore[%s] w/ load[%d] to export the content"), contentName.c_str(), tmpstr.c_str(), candidates[i].csd.loadStream);
				pTask->srcStream = candidates[i].csd.theStore->exportContentAsStream(contentName, "", 20000, 1, exportResources, params);

				stampNow = ZQ::common::now();
				if (pTask->srcStream)
				{
					sourceStore = candidates[i].csd.theStore;
					srcStreamStr = _adapter->getCommunicator()->proxyToString(pTask->srcStream);
					stampLast = stampNow;
					stampNow = ZQ::common::now();
					_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, cacheContent, "content[%s] candidate CacheStore[%s] w/ load[%d] exported as stream[%s], took %lldmsec after [%d] tries"), contentName.c_str(), tmpstr.c_str(), candidates[i].csd.loadStream, srcStreamStr.c_str(), stampNow - stampLast, i);
					break;
				}
			}
			catch(const ::TianShanIce::BaseException& ex)
			{
				_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] export from candidate CacheStore[%s] caught %s: %s"),
					contentName.c_str(), tmpstr.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch(const ::Ice::Exception& ex)
			{
				_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] export from candidate CacheStore[%s] caught %s"),
					contentName.c_str(), tmpstr.c_str(), ex.ice_name().c_str());
			}
			catch (...) 
			{
				_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] export from candidate CacheStore[%s] caught exception"),
					contentName.c_str(), tmpstr.c_str());
			}
		}
	}
 
	// step 5 prepare input parameters for Content::provision()
	stampLast = stampNow; stampNow = ZQ::common::now();

	if (pTask->srcStream)
	{
		// step 1. read the parameters of the stream
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] reading parameters of src stream[%s]"), contentName.c_str(), srcStreamStr.c_str());

		::TianShanIce::SRM::ResourceMap resOfStrm = pTask->srcStream->getResources();
		::TianShanIce::SRM::ResourceMap::iterator itRes = resOfStrm.find(TianShanIce::SRM::rtTsDownstreamBandwidth);
		
		if (resOfStrm.end() != itRes)
		{
			::TianShanIce::ValueMap& resData = itRes->second.resourceData;
			::TianShanIce::ValueMap::iterator itV = resData.find("sessionURL");
			if (resData.end() != itV && itV->second.strs.size() >0)
				pTask->urlSrcStream = itV->second.strs[0];

			itV = resData.find("bandwidth");
			if (resData.end() != itV && itV->second.lints.size() >0)
				pTask->bwCommitted = (Ice::Int) itV->second.lints[0];
		}

		// no necessary to check if the src stream is PWE because we didn't give the resource of UpstreamBandwidth.
		// the source CacheStore should reject exportContentAsStream() if it has not had the content InService

		// convert the urlSrcStream to c2http://
		if (pTask->urlSrcStream.empty())
		{
			pTask->destroy(c);
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 400, "cacheContent() content[%s] failed to read URL from source stream[%s]"), contentName.c_str(), srcStreamStr.c_str());
		}

		std::string tmpstr = pTask->urlSrcStream;
		ZQ::common::URLStr url(tmpstr.c_str());
		std::string proto = url.getProtocol();
		transform(proto.begin(), proto.end(), proto.begin(), tolower);
		// replace the http:// with c2http
		if (0 == proto.compare("http"))
			url.setProtocol("c2http");

		// append with a flag saying the source stream can cover multiple member files
		url.setVar("fileset", "true"); 
		pTask->urlSrcStream = url.generate();

		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, cacheContent, "content[%s] src stream[%s] converted the url[%s] to [%s]"), contentName.c_str(), srcStreamStr.c_str(), tmpstr.c_str(), pTask->urlSrcStream.c_str());
	}
	else
	{
		// step 1 call portal to SETUP a C2 session from the external storage
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] locating via external interface[%s]"), contentName.c_str(), extSessionInterface.c_str());

		int nRetCode = portalLocateContent(*this, extSessionInterface, *pTask);
		if (nRetCode > 0)
		{
			pTask->destroy(c);
			ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, nRetCode, "cacheContent() content[%s] failed to locate at external source storage[%s]"), contentName.c_str(), extSessionInterface.c_str());
		}

		std::string tmpstr = pTask->urlSrcStream;
		ZQ::common::URLStr url(tmpstr.c_str());

		// replace the http:// with c2http:// or c2pull://
		std::string proto = url.getProtocol();
		transform(proto.begin(), proto.end(), proto.begin(), tolower);
		if (0 == proto.compare("http") || 0 == proto.compare("c2http"))
			url.setProtocol(pTask->isSrcPWE ? "c2pull" : "c2http");

		pTask->urlSrcStream = url.generate();
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, cacheContent, "content[%s] converted the source url[%s] to [%s] per PWE[%c] range[%lld~%lld]"), contentName.c_str(), tmpstr.c_str(), pTask->urlSrcStream.c_str(), pTask->isSrcPWE?'T':'F', pTask->startOffset, pTask->endOffset);
	}

	// validate the paramters gathered in CacheTask
	if (pTask->bwMax> 0 && pTask->bwCommitted > pTask->bwMax)
	{
		pTask->destroy(c); // cancel the CacheTask, old: withdrawCacheTask(pTask);
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 400, "cacheContent() content[%s] CacheTask is withdrawn per bitrate[%d] of src stream beyonded max[%d], please adjust defaultProvSessionBitrateKbps and/or minBitratePercent"), contentName.c_str(), pTask->bwCommitted, pTask->bwMax);
	}

	// read the fileSize in MB
	long fileSizeMB =0;
	if (pTask->startOffset >=0 && pTask->endOffset > pTask->startOffset)
		fileSizeMB = (long) ((pTask->endOffset - pTask->startOffset) >> 20);

	if (pTask->isSrcPWE && pTask->bwMax > pTask->bwCommitted)
	{
		// play magic here for enh#16995 Serve the cached copy that is catching up the PWE copy on source storage
		int64 bpsCatchUp = 0;
		if (_catchUpRT > 5000) // 5 sec in minimal
			bpsCatchUp = (pTask->endOffset -pTask->startOffset) *8 / _catchUpRT *1000;

		long bpsAhead = pTask->bwCommitted * _catchUpRTminPercent /100;
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] replic[%s] source PWE available range[%lld~%lld] mpeg[%d]bps, bpsCatchUp[%d]bps per catchUpRT[%d]msec, bpsAhead[%d]bps per %d%%"),
			contentName.c_str(), replicaName.c_str(), pTask->startOffset, pTask->endOffset, pTask->bwCommitted, bpsCatchUp, _catchUpRT, bpsAhead, _catchUpRTminPercent);

		if (bpsCatchUp < bpsAhead)
			bpsCatchUp = bpsAhead;

		if (bpsCatchUp >0)
		{
			if (bpsCatchUp > pTask->bwMax - pTask->bwCommitted)
				bpsCatchUp = pTask->bwMax - pTask->bwCommitted;

			// determin the streamable playtime
			char streamablePlaytime[16];
			snprintf(streamablePlaytime, sizeof(streamablePlaytime)-2, "%lld", (int64)((pTask->endOffset -pTask->startOffset) *8 / bpsCatchUp *1000));
			::TianShanIce::Properties metaData;
			MAPSET(::TianShanIce::Properties, metaData, METADATA_EstimatedStreamable, streamablePlaytime);
			pTask->localContent->setUserMetaData2(metaData);

			// adjust the bwCommitted
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] replic[%s] source PWE available[%d]MB, adding overahead[%d]bps to transferRate[%d]bps per minCatchup[%d]%% catchUpTime[%d]msec, estimated streamable [%s]ms"),
				                                   contentName.c_str(), replicaName.c_str(), fileSizeMB, bpsCatchUp, pTask->bwCommitted, _catchUpRTminPercent, _catchUpRT, streamablePlaytime);
			pTask->bwCommitted += bpsCatchUp;
		}
	}
	
	if (fileSizeMB <=0 && pTask->bwMin >0) // guess the file has 1/2hr if no fileSizeMB is specified
		fileSizeMB = bpsToFileSizeMB(pTask->bwMin, 60*30);

	// check if there is free enough space for this caching
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] replic[%s] ensuring for filesize[%d]MB @[%d~%d]bps"), contentName.c_str(), replicaName.c_str(), fileSizeMB, pTask->bwMin, pTask->bwCommitted);
	_ensureSpace(fileSizeMB, topFolderName);

	// step 6 prepare input parameters for Content::provision()
	try {
		stampNow = ZQ::common::now();
		char buf[64];
		ZQ::common::TimeUtil::TimeToUTC(stampNow+500, buf, sizeof(buf)-2);     pTask->scheduledStart = buf;
		ZQ::common::TimeUtil::TimeToUTC(stampNow+2*3600*1000, buf, sizeof(buf)-2); pTask->scheduledEnd = buf;

		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, cacheContent, "content[%s] provisioning replica[%s] via src[%s] srcContentType[%s] @[%d]bps, %s~%s"), contentName.c_str(), replicaName.c_str(), pTask->urlSrcStream.c_str(), pTask->srcContentType.c_str(), pTask->bwCommitted, pTask->scheduledStart.c_str(), pTask->scheduledEnd.c_str());
		pTask->localContent->provision(pTask->urlSrcStream, pTask->srcContentType, false, pTask->scheduledStart, pTask->scheduledEnd, pTask->bwCommitted);
		::TianShanIce::Storage::CacheTaskPrx task = commitCacheTask(pTask);
//		stampLast = stampNow; stampNow = ZQ::common::now();
//		pTask->stampCommitted = stampNow;
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, cacheContent, "content[%s] provision replica[%s] via src[%s]/type[%s] @[%d]bps, took %lldmsec"), contentName.c_str(), replicaName.c_str(), pTask->urlSrcStream.c_str(), pTask->srcContentType.c_str(), pTask->bwCommitted, stampNow-stampLast);
		if (task)
			return; // all set
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] replica[%s]::provision() caught %s: %s"),
			contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] replica[%s]::provision() caught %s"),
			contentName.c_str(), replicaName.c_str(), ex.ice_name().c_str());
	}
	catch (...) 
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, cacheContent, "content[%s] replica[%s]::provision() caught exception"),
			contentName.c_str(), replicaName.c_str());
	}

	// provision() failed if reach here
	pTask->destroy(c); // cancel the CacheTask, old: withdrawCacheTask(pTask);
	ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log, EXPFMT(CacheStore, 400, "cacheContent() content[%s] replica[%s]::provision() failed"), contentName.c_str(), replicaName.c_str());
}

::Ice::Long CacheStoreImpl::calculateCacheDistance(const ::std::string& contentName, const ::std::string& storeNetId, const ::Ice::Current& c)
{
	HashKey contentKey, storeKey;
	if (!_calcHashKey(contentKey, contentName.c_str(), contentName.length()))
		return -1;
	if (!_calcHashKey(storeKey, storeNetId.c_str(), storeNetId.length()))
		return -1;

	return _calcRawDistance(contentKey, storeKey);
}

::std::string CacheStoreImpl::_content2FolderName(const ::std::string& contentName, std::string& topFolderName, std::string& leafFolderName)
{
	topFolderName = leafFolderName = "";

	HashKey contentKey;
	if (!_calcHashKey(contentKey, contentName.c_str(), contentName.length()))
		return "";
	
	return _contentHashKeyToFolder(contentKey, topFolderName, leafFolderName);
}

::std::string CacheStoreImpl::getFolderNameOfContent(const ::std::string& contentName, const ::Ice::Current& c)
{
	std::string top, leaf;
	return _content2FolderName(contentName, top, leaf);
}

::std::string CacheStoreImpl::getFileNameOfLocalContent(const ::std::string& contentName, const ::std::string& subfile, const ::Ice::Current& c)
{
	std::string hashedPathName = _rootVolName + getFolderNameOfContent(contentName, c);
	hashedPathName += contentName;

	std::string filepathname;

	try {
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, getFileNameOfLocalContent, "content[%s] reading LocalContent[%s]"), contentName.c_str(), hashedPathName.c_str());

		::TianShanIce::Storage::UnivContentPrx content = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_contentStore.openContentByFullname(hashedPathName, c));
		if (!content)
			ZQTianShan::_IceThrow<TianShanIce::EntityNotFound> (_log, EXPFMT(CacheStore, 404, "getFileNameOfLocalContent() content[%s] not found"), hashedPathName.c_str());

		filepathname = content->getMainFilePathname();
		if (!subfile.empty() && std::string::npos == subfile.find('*'))
			filepathname += portalSubfileToFileExtname(*this, subfile);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore,  getFileNameOfLocalContent, "content[%s] failed to access LocalContent[%s], caught %s: %s"), contentName.c_str(), hashedPathName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore,  getFileNameOfLocalContent, "content[%s] failed to access LocalContent[%s], caught %s"), contentName.c_str(), hashedPathName.c_str(), ex.ice_name().c_str());
	}
	catch (...) 
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore,  getFileNameOfLocalContent, "content[%s] failed to access LocalContent[%s], caught exception"), contentName.c_str(), hashedPathName.c_str());
	}

	return filepathname;
}

CacheTaskImpl::Ptr CacheStoreImpl::newCacheTask(const std::string& contentName)
{
	ZQ::common::MutexGuard g(_taskLocker);
	CacheTaskMap::iterator it = _taskMap.find(contentName);
	if (_taskMap.end() != _taskMap.find(contentName))
	{
		char buf[60];
		ZQ::common::TimeUtil::TimeToUTC(it->second->stampCreated, buf, sizeof(buf));
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore,  newCacheTask, "content[%s] a previous CacheTask already created at %s"), contentName.c_str(), buf);
		return NULL;
	}

	CacheTaskImpl::Ptr pTask = new CacheTaskImpl(*this);
	pTask->ident.name = contentName;
	pTask->ident.category = CATEGORY_CacheTask;
	pTask->srcContentType = ::TianShanIce::Storage::ctMPEG2TS;
	pTask->isSrcPWE = false;
	pTask->bwMin = _defaultProvSessionBitrateKbps *1000;
	pTask->bwMax = 0;
	pTask->bwCommitted = 0;
	pTask->stampCreated = ZQ::common::now();
	pTask->stampCommitted = 0;
	pTask->stampStopped = 0;
	pTask->startOffset = pTask->endOffset =0;

	int64 freeProvisionbps = _totalProvisionKbps - _usedProvisionKbps;
	freeProvisionbps *= 1000; // from Kbps to bps

	if (pTask->bwMin > freeProvisionbps)
	{
		_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore,  newCacheTask, "content[%s] runs out of propagation bandwidth"), contentName.c_str());
		return NULL;
	}

	if (_minBitratePercent <=0)
		pTask->bwMax = pTask->bwMin;
	else
	{
		if (_minBitratePercent > 20)
			_minBitratePercent = 20;

		pTask->bwMax = (Ice::Int) (freeProvisionbps * _minBitratePercent /100);
		if (pTask->bwMax < pTask->bwMin)
			pTask->bwMax = pTask->bwMin;

		// adjust bwMax to be no greater than bwMin*10
		if (pTask->bwMin >0 && pTask->bwMax > pTask->bwMin *10)
			pTask->bwMax = pTask->bwMin *10;
	}

	if (pTask->bwMax > freeProvisionbps)
		pTask->bwMax = (Ice::Int) freeProvisionbps;

	_usedProvisionKbps += RoundToKbps(pTask->bwMax);
	if (_totalProvisionKbps >0)
		_thisDescriptor.desc.loadCacheWrite =(long) (MAX_LOAD * _usedProvisionKbps /_totalProvisionKbps);

	MAPSET(CacheTaskMap, _taskMap, pTask->ident.name, pTask);
	_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, newCacheTask, "content[%s] reserved cacheTask[%p] w/ bw[%d~%d]; usage now[%lld/%lld]Kbps"), contentName.c_str(), pTask.get(), pTask->bwMin, pTask->bwMax, _usedProvisionKbps, _totalProvisionKbps);
	return pTask;
}

::TianShanIce::Storage::CacheTaskPrx CacheStoreImpl::commitCacheTask(CacheTaskImpl::Ptr& pTask)
{
	if (_contentAttrCache)
		return NULL;

	::TianShanIce::Storage::CacheTaskPrx task;
	if (!pTask->localContent)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (_log, EXPFMT(CacheTask, 401, "commit() content[%s] NULL localContent"), pTask->ident.name.c_str());

	pTask->provisionSess = pTask->localContent->getProvisionSession();

	if (!pTask->provisionSess)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_log, EXPFMT(CacheTask, 402, "commit() content[%s] NULL ProvisionSession"), pTask->ident.name.c_str());

	std::string provSessStr = _adapter->getCommunicator()->proxyToString(pTask->provisionSess);
	pTask->stampCommitted = ZQ::common::now();

	int64 oldUsed =0;

	{
		ZQ::common::MutexGuard g(_taskLocker);
		oldUsed = _usedProvisionKbps;
		_usedProvisionKbps -= RoundToKbps(pTask->bwMax);
		_usedProvisionKbps += RoundToKbps(pTask->bwCommitted);
		if (_totalProvisionKbps >0)
			_thisDescriptor.desc.loadCacheWrite = (long) (MAX_LOAD * _usedProvisionKbps /_totalProvisionKbps);
	}

	try {
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheTask, commit, "adding task[%s] to DB"), pTask->ident.name.c_str());
		_eCacheTask->add(pTask, pTask->ident);
		task = IdentityToObj(CacheTask, pTask->ident);

		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, commit, "content[%s] task[%p] committed [%d]bps, provision session[%s] BWUsage[%lld=>U%lld/T%lld]Kbps"), pTask->ident.name.c_str(), pTask.get(), pTask->bwCommitted, provSessStr.c_str(), oldUsed, _usedProvisionKbps, _totalProvisionKbps);
	}
	catch(const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheTask, commit, "adding task[%s] caught %s"), pTask->ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheTask, commit, "adding task[%s] caught exception"), pTask->ident.name.c_str());
	}

	if (!task)
	{
		ZQ::common::MutexGuard g(_taskLocker);
		_taskMap.erase(pTask->ident.name);
		_usedProvisionKbps -= RoundToKbps(pTask->bwCommitted);
		if (_totalProvisionKbps >0)
			_thisDescriptor.desc.loadCacheWrite = (long) (MAX_LOAD * _usedProvisionKbps /_totalProvisionKbps);

		_log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheStore, commit, "content[%s] task[%p] committed failed, provision session[%s] BWUsage[%lld=>U%lld/T%lld]Kbps"), pTask->ident.name.c_str(), pTask.get(), provSessStr.c_str(), oldUsed, _usedProvisionKbps, _totalProvisionKbps);

		return NULL;
	}

	_watchDog.watch(pTask->ident, PROVISION_PING_INTERVAL);
	return task;
}

void CacheStoreImpl::withdrawCacheTask(CacheTaskImpl::Ptr& pTask)
{
	if (_contentAttrCache)
		return;

	if (!pTask)
		return;

/*
	if (pTask->stampCommitted >0)
	{
		try
		{
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, withdrawCacheTask, "content[%s] removing task from DB"), pTask->ident.name.c_str());
			_eCacheTask->remove(pTask->ident);
		}
		catch(const ::Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, withdrawCacheTask, "content[%s] removing task from DB caught[%s]"), pTask->ident.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, withdrawCacheTask, "content[%s] removing task from DB caught exception"), pTask->ident.name.c_str());
		}
	}

	ZQ::common::MutexGuard g(_taskLocker);
	_taskMap.erase(pTask->ident.name);

	if (pTask->stampCommitted >0)
		_freeProvisionBW += pTask->bwCommitted;
	else
		_freeProvisionBW += pTask->bwMax;

	_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, withdrawCacheTask, "content[%s] removing task withdrawn"), pTask->ident.name.c_str());
*/
}

/*
void CacheStoreImpl::_countAccess(AccessCounters& caCounters, const std::string& contentName, const std::string& subfile)
{
	if (0 == subfile.compare(SUBFILE_EXTNAME_INDEX))
		return;

	ZQ::common::MutexGuard g(caCounters.lkMap);
	int64 stampNow = ZQ::common::now();
	ContentAccessMap::iterator it = caCounters.caMap.find(contentName);
	if (caCounters.caMap.end() == it)
	{
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, FUNCNAME "content[%s] not in %s, adding it in CacheCounter[%s]"), contentName.c_str(), caCounters.name.c_str());
		ActiveContent ac;
		ac.contentName = contentName;
		ac.folderName = getFolderNameOfContent(contentName, Ice::Current());
		ac.countSince = ac.knownSince = stampNow;
		ac.accessCount = ac.fileSizeMB = 0;
		ac.localState = TianShanIce::Storage::csNotProvisioned;
		ac.countLatest = stampNow;
		MAPSET(ContentAccessMap, caCounters.caMap, contentName, ac);
		it = caCounters.caMap.find(contentName);
	}

	if (caCounters.caMap.end() == it)
		return;

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, FUNCNAME "content[%s] subfile[%s] increasing %s[%d]"), contentName.c_str(), subfile.c_str(), caCounters.name.c_str());
	it->second.accessCount++;
	it->second.countLatest = stampNow;
}
*/

void CacheStoreImpl::OnLocalContentRequested(const std::string& contentName, const std::string& subfile)
{
//	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnLocalContentRequested, "content[%s] subfile[%s]"), contentName.c_str(), subfile.c_str());
	if (0 == subfile.compare(SUBFILE_EXTNAME_index))
		return;

	TianShanIce::Storage::ContentAccess ac = _acHotLocals->count(contentName, subfile, 1);
	char buf[128];
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnLocalContentRequested, "subfile[%s] %s"), subfile.c_str(), AccessRegistrarImpl::ContentAccessStr(ac, buf, sizeof(buf)-1));
}

void CacheStoreImpl::OnMissedContentRequested(const std::string& contentName, const std::string& subfile)
{
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnMissedContentRequested, "content[%s] subfile[%s]"), contentName.c_str(), subfile.c_str());

	if (0 == subfile.compare(SUBFILE_EXTNAME_index))
		return;

	TianShanIce::Storage::ContentAccess ac = _acMissed->count(contentName, subfile, 1);
	char buf[128];
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnMissedContentRequested, "subfile[%s] %s"), subfile.c_str(), AccessRegistrarImpl::ContentAccessStr(ac, buf, sizeof(buf)-1));
/*
	ZQ::common::MutexGuard g(_acMissed.lkMap);
	int64 stampNow = ZQ::common::now();
	ContentAccessMap::iterator it = _acMissed.acMap.find(contentName);
	if (_acMissed.acMap.end() == it)
	{
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, FUNCNAME "content[%s] not in _acMissed.acMap, adding it in"), contentName.c_str());
		ActiveContent ac;
		ac.contentName = contentName;
		ac.folderName = getFolderNameOfContent(contentName, Ice::Current());
		ac.countSince = ac.knownSince = ZQ::common::now();
		ac.accessCount = 1;
		ac.fileSizeMB = 0;
		ac.localState = TianShanIce::Storage::csNotProvisioned;
		ac.countLatest = stampNow;
		MAPSET(ContentAccessMap, _acMissed.acMap, contentName, ac);
		it = _acMissed.acMap.find(contentName);
	}

	if (_acMissed.acMap.end() == it)
		return;

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, FUNCNAME "content[%s] subfile[%s] increasing counter[%d]"), contentName.c_str(), subfile.c_str(), it->second.accessCount);
	it->second.accessCount++;
	it->second.countLatest = ZQ::common::now();
*/
}


void CacheStoreImpl::OnContentCreated(const std::string& contentReplicaName)
{
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnContentCreated, "contentReplica[%s]"), contentReplicaName.c_str());
#pragma message ( __TODO__ "impl here")
}

void CacheStoreImpl::OnContentDestroyed(const std::string& contentReplicaName)
{
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnContentDestroyed, "removing contentReplica[%s] from unpopular list"), contentReplicaName.c_str());
	size_t pos = contentReplicaName.find(CACHE_FOLDER_PREFIX "T");
	if (std::string::npos == pos)
		return;

	Ice::Identity identTF;
	identTF.category   = CATEGORY_TopFolder;
	identTF.name = contentReplicaName.substr(pos, strlen(CACHE_FOLDER_PREFIX "T00"));

	try {
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnContentDestroyed, "removing contentReplica[%s] from TF[%s]'s list"), contentReplicaName.c_str(), identTF.name.c_str());
		::TianShanIce::Storage::TopFolderPrx topFolder = IdentityToObj(TopFolder, identTF);
		topFolder->eraseFromUnpopular(contentReplicaName);
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_WARNING,FLOGFMT(CacheStore, OnContentDestroyed, "removing contentReplica[%s] from TF[%s]'s list caught exception[%s]"), contentReplicaName.c_str(), identTF.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_WARNING,FLOGFMT(CacheStore, OnContentDestroyed, "removing contentReplica[%s] from TF[%s]'s list caught exception"), contentReplicaName.c_str(), identTF.name.c_str());
	}
}

void CacheStoreImpl::OnContentStateChanged(const std::string& contentReplicaName, const ::TianShanIce::Storage::ContentState previousState, const ::TianShanIce::Storage::ContentState newState)
{
	size_t posCN = contentReplicaName.find_last_not_of("/");
	std::string contentName = contentReplicaName.substr(posCN+1);
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnContentStateChanged, "content[%s] localReplica[%s] state(%d=>%d)"), contentName.c_str(), contentReplicaName.c_str(), previousState, newState);

	::TianShanIce::Storage::AccessCounter counter;
	bool bInMissed = _acMissed->get(contentName, counter);
	if (bInMissed)
	{
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnContentStateChanged, "content[%s] pre-existed in _acMissed state(%d)"), contentName.c_str(), newState);
		counter.localState = newState;
	}

/*
	ZQ::common::MutexGuard g(_acMissed.lkMap);
	ContentAccessMap::iterator it = _acMissed.caMap.find(contentName);
	if (_acMissed.caMap.end() != it)
	{
		_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, FUNCNAME "content[%s] updating _acMissed.acMap state(%d)"), contentName.c_str(), contentReplicaName.c_str(), newState);
		it ->second.localState = newState;
	}
*/

	int64 stampNow = ZQ::common::now();

	switch (newState)
	{
	case ::TianShanIce::Storage::csNotProvisioned:
	case ::TianShanIce::Storage::csProvisioning:
	case ::TianShanIce::Storage::csProvisioningStreamable:
		if (!bInMissed)
		{
			_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, OnContentStateChanged, "content[%s] localReplica[%s] state(%d) not in _acMissed, adding it in"), contentName.c_str(), contentReplicaName.c_str(), newState);

			::TianShanIce::Storage::AccessCounter ac;
			ac.base.contentName = contentName;
			ac.base.accessCount = 1;
			ac.base.stampSince = stampNow;
			ac.base.stampLatest = stampNow;

			std::string top, leaf;
			ac.folderName = CacheStoreImpl::_content2FolderName(contentName, top, leaf);
			ac.knownSince = stampNow;
			ac.fileSizeMB = 0;
			ac.localState = newState;

			_acMissed->set(counter);
		}
		break;

	case ::TianShanIce::Storage::csInService:
		if (bInMissed)
		{
			_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, OnContentStateChanged, "content[%s] localReplica[%s] state(%d) is in _acMissed, removing it"), contentName.c_str(), contentReplicaName.c_str(), newState);
			_acMissed->erase(contentName);
		}
		break;

	case ::TianShanIce::Storage::csOutService:
		break;
	case ::TianShanIce::Storage::csCleaning:
		break;
	default:
		break;
	}
}

#define DEFAULT_STORE_ONTIMER_INTERVAL (1000*60) // start with 1min

void CacheStoreImpl::OnTimer(const ::Ice::Current& c)
{
//	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnTimer, ""));
	int64 stampNow = ZQ::common::now();

	 // put a dummy long watching and overwrite later, just to make sure that the store is always under watching
	_watchDog.watch(_localId, DEFAULT_STORE_ONTIMER_INTERVAL *8);
	
	uint32 nextSleep = DEFAULT_STORE_ONTIMER_INTERVAL;

	try {

		int32 flushedTime =  (int32) (stampNow - _stampLocalCounterFlushed);

		if (flushedTime >= (int32) max(DEFAULT_STORE_ONTIMER_INTERVAL, _timeWinOfPopular/4))
		{
			_stampLocalCounterFlushed = stampNow;
			::TianShanIce::Storage::ContentCounterList listToFlush, listEvicted;

			// step 1. for the missed list: 
			//    a) evict those long-time no order contents from watching list 
			//    b) ignore the list to flush
			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnTimer, "refreshing the counters of missed contents"));
			_sizeMissed = _acMissed->compress(_timeWinOfPopular, _timeWinOfPopular, _reqsMissedInWin, listToFlush, listEvicted, c);
			_log((listEvicted.size() >0 ? ZQ::common::Log::L_INFO : ZQ::common::Log::L_DEBUG), FLOGFMT(CacheStore, OnTimer, "refreshed the counters of missed contents, %d evicted, %d left"), listEvicted.size(), _sizeMissed);
			listToFlush.clear(); listEvicted.clear();

			// step 2. for the hot local list: 
			//    a) evict those long-time no order contents from watching list 
			//    b) compress the counts piror to this monitor time window and flush counter into content DB
			_sizeHotLocals = _acHotLocals->compress(_timeWinOfPopular, flushedTime, _reqsHotLocalsInWin, listToFlush, listEvicted, c);
			if ((listEvicted.size()+listToFlush.size()) >0)
				_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, OnTimer, "flushed the cached counter of local hot contents, %d to flush, %d evicted, %d left"), listToFlush.size(), listEvicted.size(), _sizeHotLocals);

			for (::TianShanIce::Storage::ContentCounterList::iterator it = listToFlush.begin(); it < listToFlush.end(); it++)
			{
				FlushAccessCounterCmd* pCmd = FlushAccessCounterCmd::newCmd(*this, *it);
				if (pCmd)
					pCmd->exec();
			}
		}

		if (_countOfPopular <1) // _countOfPopular must be greater than 0
			_countOfPopular =1;

		// #pragma message ( __TODO__ "step 3. check if the top populars in the missed list are necessary to cache into local")
		do {
			if (stampNow - _stampLastScanMissed < min(_hotTimeWin, _timeWinOfPopular/2))
				break; // no necessary to scan the missed

			_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnTimer, "scanning for popular contents to cache, missed size[%d]"), _acMissed->size());
			::TianShanIce::Storage::ContentCounterList popularMissedList;
			try {
				stampNow = ZQ::common::now();
				_acMissed->sort(_timeWinOfPopular, false, _countOfPopular, popularMissedList, c);
				_stampLastScanMissed = ZQ::common::now();
				_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, OnTimer, "%d missed contents under watching, sort took %lldmsec"), popularMissedList.size(), _stampLastScanMissed - stampNow);
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheStore, OnTimer, "sort missed contents caught exception"));
			}

			for (::TianShanIce::Storage::ContentCounterList::iterator it =popularMissedList.begin(); it < popularMissedList.end(); it++)
			{
				if (it->base.accessCount < _countOfPopular)
					continue; // can be "break" here in theory

				// create a command to cache this content to local
				std::string strSince, strLatest;
				char buf[128];
				if (ZQ::common::TimeUtil::TimeToUTC(it->base.stampSince, buf, sizeof(buf)-2))
					strSince = buf;
				if (ZQ::common::TimeUtil::TimeToUTC(it->base.stampLatest, buf, sizeof(buf)-2))
					strLatest = buf;

				_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnTimer, "content[%s] counted %d during[%s~%s] met threshold %d, issuing command to cache it"), it->base.contentName.c_str(), it->base.accessCount, strSince.c_str(), strLatest.c_str(), _countOfPopular);
				try {
					ZQ::common::MutexGuard g(_taskLocker);
					if (_taskMap.end() != _taskMap.find(it->base.contentName))
						continue; // the cache task has been previously created, skip

					(new ImportContentCmd(*this, it->base.contentName))->exec();
					_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnTimer, "content[%s] counted %d during[%s~%s] met threshold %d, acquired to cache to local"), it->base.contentName.c_str(), it->base.accessCount, strSince.c_str(), strLatest.c_str(), _countOfPopular);
				} catch(...) {}
			}

		} while (0);
	}
	catch (...) {}

	int64 freeMB=0, totalMB=0;
	_readStoreSpace(freeMB, totalMB);

	// adjust the interval of OnTimer()
	if (totalMB >0 && freeMB < totalMB *2/10)
		nextSleep = min(nextSleep, 10*1000);

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnTimer, "checking space[F%lld/T%lld]MB"), freeMB, totalMB);
	_ensureSpace(0);

	_watchDog.watch(_localId, nextSleep);
}

void CacheStoreImpl::penalize(CacheStoreDsptr& store, int penaltyToCharge)
{
	int64 stampNow = ZQ::common::now();
	int old = store.penalty;
	store.penalty = old + penaltyToCharge; // - (stampNow - store.desc.stampAsOf) / _heatbeatInterval;
	if (store.penalty <0)
		store.penalty =0;
	else if (_penaltyMax >0)
	{
		if (store.penalty > (int)_penaltyMax)
			store.penalty = _penaltyMax;
		
		if (store.penalty > (int)(_penaltyMax*3/4 +1))
			store.desc.theStore = NULL; // force to re-connect
	}

	if (penaltyToCharge >0)
		store.stampLastPenalty = stampNow;

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, penalize, "neighor[%s] pen[%d] to [%d]"), store.desc.netId.c_str(), old, store.penalty);
}

void CacheStoreImpl::OnForwardFailed(std::string storeNetId, int penaltyToCharge)
{
	::ZQ::common::MutexGuard gd(_lockerNeighbors);
	CacheStoreImpl::CacheStoreMap::iterator it = _neighbors.find(storeNetId); 
	if (_neighbors.end() == it)
		return;

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, OnForwardFailed, "charging neighor[%s] by penalty[%d]"), storeNetId.c_str(), penaltyToCharge);
	penalize(it->second, penaltyToCharge);
}

std::string CacheStoreImpl::findExternalSessionInterfaceByContent(const std::string& contentName, const TianShanIce::Properties& nameFields)
{
	std::string providerId;

	TianShanIce::Properties::const_iterator itMD = nameFields.find(METADATA_ProviderId);
	if (nameFields.end() != itMD)
		providerId = itMD->second;

	return findExternalSessionInterfaceByProvider(providerId);
}

std::string CacheStoreImpl::findExternalSessionInterfaceByProvider(const std::string& providerId)
{
	SourceStores::StoreInfo si;
	_extSourceStores.find(providerId, si);
	return si.sessionInterface;
}

::TianShanIce::Storage::CacheStoreList CacheStoreImpl::listNeighors(::Ice::Int heatbeatInterval, const ::Ice::Current& c)
{
	::TianShanIce::Storage::CacheStoreList result;
	CacheStoreListInt list;

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, listNeighors, ""));

	_listNeighorsEx(list, heatbeatInterval);

	for (CacheStoreListInt::iterator it = list.begin(); it < list.end(); it++)
		result.push_back(it->desc);

	_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, listNeighors, "%d neighbors found"), result.size());
	return result;
}

int CacheStoreImpl::_listNeighorsEx(CacheStoreListInt& list, bool includeSelf, uint32 heatbeatInterval)
{
	if (heatbeatInterval < _heatbeatInterval)
		heatbeatInterval = _heatbeatInterval;

	list.clear();
	int64 stampExp = ZQ::common::now() - (long) (heatbeatInterval * 1.5);
	::ZQ::common::MutexGuard gd(_lockerNeighbors);
	for (CacheStoreImpl::CacheStoreMap::iterator it = _neighbors.begin(); it != _neighbors.end(); it++)
	{
		if (it->second.desc.stampAsOf < stampExp)
			continue;

		list.push_back(it->second);
	}

	if (includeSelf)
		list.push_back(_thisDescriptor);

	return list.size();
}

void CacheStoreImpl::queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly, const ::Ice::Current& c)
{
	// redirect to ContentStore
	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, queryReplicas, "redirecting to local ContentStore"));
	_contentStore.queryReplicas_async(amdCB, category, groupId, localOnly, c);
}

struct LessFWU
{
	bool operator() (const CacheStoreImpl::FWU& A, const CacheStoreImpl::FWU& B)
     {
		 return (A.minAccessCount < B.minAccessCount);
     }
};

typedef std::multiset<CacheStoreImpl::FWU, LessFWU> SortedFWU;

CacheStoreImpl::FWUSequence CacheStoreImpl::_sortFolderByFWU()
{
	SortedFWU sortedFWU;
	FWUSequence fwuseq;
	{
		ZQ::common::MutexGuard g(_lkFWU);
		for (FWUMap::iterator it = _fwuMap.begin(); it != _fwuMap.end(); it++)
			sortedFWU.insert(it->second);
	}

	for (SortedFWU::iterator it = sortedFWU.begin(); it != sortedFWU.end(); it++)
		fwuseq.push_back(*it);

	return fwuseq;
}

void CacheStoreImpl::_updateFWU(const CacheStoreImpl::FWU& fwu)
{
	ZQ::common::MutexGuard g(_lkFWU);
	if (fwu.unpopularSize >0)
		MAPSET(CacheStoreImpl::FWUMap, _fwuMap, fwu.topFolderName, fwu);
	else _fwuMap.erase(fwu.topFolderName);
}

void CacheStoreImpl::updateStreamLoad(uint32 usedBwLocalStreamKbps, uint32 maxBwLocalStreamKbps, uint32 usedBwPassThruKbps, uint32 maxBwPassThruKbps)
{
	if (usedBwLocalStreamKbps <=0)
		usedBwLocalStreamKbps =0;

	if (usedBwPassThruKbps <=0)
		usedBwPassThruKbps =0;

	// step 1. determine loadStream for downstream interface
	uint32 usedDownBw = usedBwLocalStreamKbps + usedBwPassThruKbps;
	uint32 totalDownBw = MAX(maxBwLocalStreamKbps, maxBwPassThruKbps);
//	if (maxBwLocalStreamKbps >0 && maxBwPassThruKbps>0)
//		totalDownBw = maxBwLocalStreamKbps + maxBwPassThruKbps;

	ZQ::common::MutexGuard g(_lockerNeighbors);

	if (totalDownBw <= 0)
		_thisDescriptor.desc.loadStream =0;
	else if (usedDownBw >= totalDownBw)
		_thisDescriptor.desc.loadStream = MAX_LOAD;
	else
		_thisDescriptor.desc.loadStream = MAX_LOAD * usedDownBw / totalDownBw;

	// step 2. deterime the load of uplink interface
	uint32 usedImportKbps  = usedBwPassThruKbps + _usedProvisionKbps;
	uint32 totalImportKbps = MAX(maxBwPassThruKbps, _totalProvisionKbps);

	if (totalImportKbps <=0)
		_thisDescriptor.desc.loadImport =0;
	else if (usedImportKbps >= totalImportKbps)
		_thisDescriptor.desc.loadImport = MAX_LOAD;
	else _thisDescriptor.desc.loadImport = MAX_LOAD * usedImportKbps / totalImportKbps;

	_usedLocalStreamKbps = usedBwLocalStreamKbps;
	_maxLocalStreamKbps  = maxBwLocalStreamKbps; 
	_usedPassThruKbps    = usedBwPassThruKbps;
	_maxPassThruKbps     = maxBwPassThruKbps;

	_log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheStore, updateStreamLoad, "local-stream[%d/%d] pass-thru[%d/%d]=> streamLoad[%d] importLoad[%d]"),
		  usedBwLocalStreamKbps, maxBwLocalStreamKbps, usedBwPassThruKbps, maxBwPassThruKbps, _thisDescriptor.desc.loadStream, _thisDescriptor.desc.loadImport);
}

// -----------------------------
// class CacheTaskImpl
// -----------------------------
/*
::TianShanIce::Storage::CacheTaskPrx CacheTaskImpl::commit()
{
	if (!localContent)
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(CacheTask, 401, "commit() content[%s] NULL localContent"), ident.name.c_str());

	provisionSess = localContent->getProvisionSession();

	if (!provisionSess)
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(CacheTask, 402, "commit() content[%s] NULL ProvisionSession"), ident.name.c_str());

	std::string provSessStr = _store->_adapter->getCommunicator()->proxyToString(provisionSess);
	stampCommitted = ZQ::common::now();

	try {
		_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheTask, commit, "adding task[%s] to DB"), ident.name.c_str());
		_eCacheTask->add(pTask, pTask->ident);

		ZQ::common::MutexGuard g(_taskLocker);
		_freeProvisionBW += pTask->bwMax;
		_freeProvisionBW -= pTask->bwCommitted;
		_log(ZQ::common::Log::L_INFO, FLOGFMT(CacheStore, commit, "task [%s] committed, provision session[%s]"), ident.name.c_str(), provSessStr.c_str());

		return IdentityToObj(CacheTask, ident);
	}
	catch (...) {}


}
*/

void CacheTaskImpl::OnRestore(const ::Ice::Current& c)
{
	{
		RLock sync(*this);
		_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheTask, OnRestore, "task[%s]"), ident.name.c_str());

		ZQ::common::MutexGuard g(_store._taskLocker);
		if (_store._taskMap.end() == _store._taskMap.find(ident.name))
		{
			int64 bwToUsed = (stampCommitted >0) ? bwCommitted : bwMax;
			_store._usedProvisionKbps += RoundToKbps(bwToUsed);
			if (_store._totalProvisionKbps >0)
				_store._thisDescriptor.desc.loadCacheWrite = (long) (MAX_LOAD * _store._usedProvisionKbps /_store._totalProvisionKbps);

			MAPSET(CacheStoreImpl::CacheTaskMap, _store._taskMap, ident.name, this);
		}
	}

	::Ice::Current newc =c;
	MAPSET(::Ice::Context, newc.ctx, "signature", "OnRestore()");
	OnTimer(newc);
}

void CacheTaskImpl::destroy(const ::Ice::Current& c)
{
	if (!_store._eCacheTask)
		return;

	try
	{
		WLock sync(*this);
		_store._eCacheTask->remove(ident);
		_store._log(ZQ::common::Log::L_INFO, FLOGFMT(CacheTask, destroy, "content[%s] task removed from DB"), ident.name.c_str());
	}
	catch(const Ice::ObjectNotExistException&)
	{
		// do nothing
	}
	catch(const Ice::NotRegisteredException&)
	{
		// do nothing
	}
	catch(const ::Ice::Exception& ex)
	{
		_store._log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheTask, destroy, "content[%s] removing task from DB caught[%s]"), ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		_store._log(ZQ::common::Log::L_ERROR, FLOGFMT(CacheTask, destroy, "content[%s] removing task from DB caught exception"), ident.name.c_str());
	}

	ZQ::common::MutexGuard g(_store._taskLocker);
	_store._taskMap.erase(ident.name);

	int64 bwToFree = (stampCommitted >0) ? bwCommitted : bwMax;

	_store._usedProvisionKbps -= RoundToKbps(bwToFree);
	if (_store._totalProvisionKbps >0)
		_store._thisDescriptor.desc.loadCacheWrite = (long) (MAX_LOAD * _store._usedProvisionKbps / _store._totalProvisionKbps);

	_store._log(ZQ::common::Log::L_INFO, FLOGFMT(CacheTask, destroy, "content[%s] freed task[%p] w/ [%lld]bps, now BW usage[U%lld/T%lld]Kbps"), ident.name.c_str(), this, bwToFree, _store._usedProvisionKbps, _store._totalProvisionKbps);
}

void CacheTaskImpl::OnTimer(const ::Ice::Current& c)
{
	int64 stampNow = ZQ::common::now();
	bool bDestroyNeeded  = false;

	do {
		RLock sync(*this);

//		_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheTask, OnTimer, "content[%s] task[%p]"), ident.name.c_str(), this);
		_store._watchDog.watch(ident, PROVISION_PING_INTERVAL);

		if (!provisionSess)
		{
			if (stampCreated < (stampNow - 2*60*1000))
			{
				_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheTask, OnTimer, "content[%s] task[%p] destroying per no provision session attached"), ident.name.c_str(), this);
				bDestroyNeeded = true;
			}

			break;
		}

		std::string strPS;
		try {
			strPS = _store._adapter->getCommunicator()->proxyToString(provisionSess);
			provisionSess->ice_ping();
			_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheTask, OnTimer, "task[%p] ProvisionSession[%s] still running"), this, strPS.c_str());
		}
		catch(const ::Freeze::DatabaseException& ex)
		{
			_store._log(ZQ::common::Log::L_INFO, FLOGFMT(CacheTask, OnTimer, "task[%p] ping ProvisionSession[%s] caught %s"), this, strPS.c_str(), ex.ice_name().c_str());
		}
		catch(const ::Ice::ObjectNotExistException&)
		{
			_store._log(ZQ::common::Log::L_INFO, FLOGFMT(CacheTask, OnTimer, "task[%p] ProvisionSession[%s] gone, cleaning task"), this, strPS.c_str());
			bDestroyNeeded = true;
		}
		catch(const ::Ice::Exception& ex)
		{
			_store._log(ZQ::common::Log::L_INFO, FLOGFMT(CacheTask, OnTimer, "task[%p] ping ProvisionSession[%s] caught %s"), this, strPS.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			_store._log(ZQ::common::Log::L_INFO, FLOGFMT(CacheTask, OnTimer, "task[%p] ping ProvisionSession[%s] caught exception"), this, strPS.c_str());
		}

	} while(0);

	if (localContent)
	{
		::Ice::Context::const_iterator itCtx = c.ctx.find("signature");
		if (bDestroyNeeded || (c.ctx.end() != itCtx && 0 == itCtx->second.compare("OnRestore()")))
		{
			try {
				_store._log(ZQ::common::Log::L_DEBUG, FLOGFMT(CacheTask, OnTimer, "task[%p] force content[%s] to populate attributes per cache completion or restored"), this, ident.name.c_str());
				localContent->populateAttrsFromFilesystem();
			}
			catch(...)
			{
				_store._log(ZQ::common::Log::L_WARNING, FLOGFMT(CacheTask, OnTimer, "task[%p] force content[%s] to populate attributes caught exception"), this, ident.name.c_str());
			}
		}
	}

	if (bDestroyNeeded)
		return destroy(c);
}

}} // namespace


