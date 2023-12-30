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
// Name  : ChODSvcEnv.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-23
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChODSvcEnv.cpp $
// 
// 2     2/06/13 11:58a Hui.shao
// 
// 2     2/06/13 11:56a Hui.shao
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 54    10-03-24 12:51 Haoyuan.lu
// set cachesize
// 
// 53    10-03-17 17:17 Haoyuan.lu
// remove lock of evictor and add DB_CONFIG
// 
// 52    09-09-23 17:07 Haoyuan.lu
// 
// 51    09-07-09 15:02 Haoyuan.lu
// 
// 50    09-07-06 15:39 Haoyuan.lu
// 
// 49    09-05-14 13:17 Haoyuan.lu
// 
// 48    09-02-06 17:24 Haoyuan.lu
// 
// 47    09-01-04 11:46 Haoyuan.lu
// 
// 46    08-12-08 15:08 Haoyuan.lu
// 
// 45    08-12-05 19:01 Haoyuan.lu
// 
// 44    08-11-28 14:36 Haoyuan.lu
// 
// 43    08-11-20 16:30 Haoyuan.lu
// 
// 42    08-10-28 15:39 Haoyuan.lu
// 
// 41    08-04-08 17:07 Haoyuan.lu
// 
// 40    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 39    08-03-12 11:54 Guan.han
// 
// 38    08-03-05 12:15 Guan.han
// 
// 39    08-03-04 18:53 Guan.han
// 
// 37    08-02-21 16:22 Guan.han
// 
// 36    08-01-15 11:07 Guan.han
// 
// 35    08-01-10 15:11 Guan.han
// 
// 33    08-01-07 18:47 Guan.han
// not exit when failed connecting topic manager
// 
// 32    07-12-24 11:53 Guan.han
// 
// 31    07-12-14 15:38 Xiaohui.chai
// 
// 30    07-08-15 14:43 Jie.zhang
// 
// 29    07-08-02 15:43 Jie.zhang
// 
// 28    07-07-13 12:43 Jie.zhang
// sync
// 
// 28    07-07-12 20:02 Jie.zhang
// * RW lock seem must not used for dictionary, normal lock is ok,  *the
// evictor must inherited from a lock, or will cause program exit
// unexcpected.
// 
// 27    07-07-12 17:26 Jie.zhang
// 
// 26    07-07-11 16:05 Jie.zhang
// sync with main tree
// 
// 27    07-07-11 15:53 Jie.zhang
// add lock to every evictor
// 
// 26    07-07-06 14:48 Jie.zhang
// add a confige: enablechannelmaxdurationcheck and add some logic to
// avoid null playlist handle
// 
// 25    07-06-26 13:30 Jie.zhang
// 
// 24    07-06-25 16:02 Jie.zhang
// add a dumpLine function
// 
// 23    07-06-23 11:00 Guan.han
// 
// 22    07-06-20 12:39 Guan.han
// 
// 21    07-05-15 17:40 Guan.han
// 
// 20    07-05-08 12:10 Guan.han
// 
// 19    07-04-29 17:22 Guan.han
// 
// 18    07-04-28 21:17 Guan.han
// 
// 17    07-04-27 18:13 Guan.han
// 
// 16    07-03-26 14:51 Jie.zhang
// 
// 15    07-02-09 14:01 Guan.han
// Modify thr format of Log, when EndOfItem or EndOfStream  Event arrives.
// 
// 13    06-12-04 17:34 Jie.zhang
// 
// 12    06-12-01 16:28 Jie.zhang
// Log changed and add thread pool to process add/insert/replace/remove
// operation
// 
// 11    06-11-17 19:38 Jie.zhang
// 
// 10    06-11-15 17:34 Jie.zhang
// 
// 9     06-10-24 18:43 Jie.zhang
// 
// 8     06-10-23 10:04 Jie.zhang
// 
// 7     06-09-26 11:18 Jie.zhang
// 
// 6     06-09-20 14:32 Jie.zhang
// 
// 5     06-09-05 11:56 Bernie.zhao
// set ChannelPublishPoint evictor size to 0
// 
// 4     06-09-04 21:07 Bernie.zhao
// added notification callbacks
// 
// 3     06-08-28 12:01 Bernie.zhao
// 1st draft done
// 
// 2     06-08-23 12:42 Bernie.zhao
// creation
// ===========================================================================

#include "Locks.h"
#include "ChODSvcEnv.h"
#include "ChannelPublishPointImpl.h"
#include "Log.h"
#include "ChannelOnDemandAppImpl.h"
#include "PurchaseImpl.h"
#include "PurchaseItemImpl.h"
#include "PlaylistEventSinkImpl.h"
#include "CODConfig.h"
#include "PurchaseRequest.h"
#include "TianShanDefines.h"

#include <io.h>

#define LOG_MODULE_NAME			"CodEnv"

// use thread pool to process item
#define USE_THREAD_POOL

namespace ZQChannelOnDemand {

UserCtrlNumGenerator	_gUserCtrlNumGen;

//////////////////////////////////////////////////////////////////////////
// class ChODSvcEnv
//////////////////////////////////////////////////////////////////////////
ChODSvcEnv::ChODSvcEnv(Ice::CommunicatorPtr& communicator)
	: _communicator(communicator), _publisher(NULL), _adapter(NULL), 
	_pChannelItemDict(NULL), _factory(NULL), _pWatchDog(NULL), _pThreadPool(NULL), _evtAdap(NULL)
{
	_bInited = false;
}

ChODSvcEnv::~ChODSvcEnv()
{
	unInit();
}

void dumpLine(const char* line, void* pCtx)
{
	if (line)
		glog(ZQ::common::Log::L_DEBUG, line);
}

bool ChODSvcEnv::ConnectIceStorm()
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "do connectEventChannel()"));

	try
	{
		_eventChannel = new TianShanIce::Events::EventChannelImpl(_evtAdap, _config.TopicMgrEndPoint.c_str());
		TianShanIce::Streamer::StreamEventSinkPtr _evtStream = new StreamEventSinkImpl(*this);
		TianShanIce::Streamer::PlaylistEventSinkPtr _evtPlaylist = new PlaylistEventSinkImpl(*this);
		TianShanIce::Properties qos;
		_eventChannel->sink(_evtStream, qos);
		_eventChannel->sink(_evtPlaylist, qos);
		_eventChannel->start();
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "connectEventChannel(%s) caught(%s: %s)")
			, _config.TopicMgrEndPoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "connectEventChannel(%s) caught(%s)")
			, _config.TopicMgrEndPoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "connectEventChannel(%s) caught unexpect exception")
			, _config.TopicMgrEndPoint.c_str());
		return false;
	}

	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "connectEventChannel() successfully"));
	return true;
}

bool ChODSvcEnv::init()
{
	if (_config.ChannelPubEndPoint.size() == 0)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "End point not configurated"));
		return false;
	}

	// create native thread pool
	_pThreadPool = new ZQ::common::NativeThreadPool(_config.ThreadPoolSize);

	_endpoint = _config.ChannelPubEndPoint;	
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "open adapter %s at %s"), SERVICE_NAME_ChannelPublisher, _endpoint.c_str());

    _adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_ChOD, _endpoint.c_str(), glog);

	// create listen event adapter
	try 
	{
		_evtAdap = _communicator->createObjectAdapterWithEndpoints("CODEventAdapter", _config.ListenEventEndPoint);
		_evtAdap->activate();
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "create adapter for listen event caught %s, endpoint is %s"), 
			ex.ice_name().c_str(), _config.ListenEventEndPoint.c_str());
		return false;
	}
	
	// check path
	{
		if (!_config.safeStorePath.size())
		{
			char path[MAX_PATH];
			if (::GetModuleFileNameA(NULL, path, MAX_PATH-1)>0)
			{
				char* p = strrchr(path, FNSEPC);
				if (NULL !=p)
				{
					*p='\0';
					p = strrchr(path, FNSEPC);
					if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
						*p='\0';
				}
				
				strcat(path, FNSEPS "data" FNSEPS);
				_dbPath = path;
			}
		}
		else _dbPath = _config.safeStorePath;
		
		if (FNSEPC != _dbPath[_dbPath.length()-1])
			_dbPath += FNSEPS;

		if(!_config.dbRuntimeDataPath.size())
		{
			_runtimePath = _dbPath;
		}
		else _runtimePath = _config.dbRuntimeDataPath;

		if (FNSEPC != _runtimePath[_runtimePath.length()-1])
			_runtimePath += FNSEPS;
	}
	
	try {
		
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "opening database at path: %s"), _dbPath.c_str());

		// open dictionary
		::CreateDirectoryA((_dbPath + FNSEPS).c_str(), NULL);
		::CreateDirectoryA((_dbPath + ServiceSubDir FNSEPS).c_str(), NULL);
		::CreateDirectoryA((_dbPath + ServiceSubDir FNSEPS ChannelDataSubDir FNSEPS).c_str(), NULL);
		//		::CreateDirectoryA((_dbPath + PurchaseSubDir FNSEPS).c_str(), NULL);
		//		::CreateDirectoryA((_dbPath + PurchaseItemSubDir FNSEPS).c_str(), NULL);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "opening runtime database at path: %s"), _runtimePath.c_str());

		::CreateDirectoryA((_runtimePath + FNSEPS).c_str(), NULL);
		::CreateDirectoryA((_runtimePath + ServiceSubDir FNSEPS).c_str(), NULL);
		::CreateDirectoryA((_runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS).c_str(), NULL);

		// generator db_config file
		std::string channelDbPathname = ZQTianShan::createDBFolder(glog, ServiceSubDir, _dbPath.c_str(), ChannelDataSubDir);
		std::string purchaseDbPathname = ZQTianShan::createDBFolder(glog, ServiceSubDir, _runtimePath.c_str(), PurchaseSubDir);

		_connCh = Freeze::createConnection(_adapter->getCommunicator(), channelDbPathname);

		_pChannelItemDict = new ::ChannelOnDemand::ChannelItemDict(_connCh, ICE_ChannelItemDict);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Dictionary of ChannelItem created"));

		// open PurchaseItemAssoc evictor and all indices
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Create indices..."));
		_idxChannelNameIndex = new ::ChannelOnDemand::ChannelNameIndex(INDEXFILENAME(ChannelNameIndex));
		_idxChannel2Purchase = new ::ChannelOnDemand::Channel2Purchase(INDEXFILENAME(Channel2Purchase));
		_idxCtrlNum2ItemAssoc = new ::ChannelOnDemand::CtrlNum2ItemAssoc(INDEXFILENAME(CtrlNum2ItemAssoc));
		_idxPurchase2ItemAssoc = new ::ChannelOnDemand::Purchase2ItemAssoc(INDEXFILENAME(Purchase2ItemAssoc));
		_idxPlaylistId2Purchase = new ::ChannelOnDemand::PlaylistId2Purchase(INDEXFILENAME(PlaylistId2ItemAssoc));

		// open ChannelPublishPoint evictor
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxChannelNameIndex);

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "create evictor %s"), ICE_ChannelPublishPoint);
#if ICE_INT_VERSION/100 >302
			_evitChannelPublishPoint = Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + ServiceSubDir FNSEPS ChannelDataSubDir FNSEPS, ICE_ChannelPublishPoint, 0, indices);
#else	
			_evitChannelPublishPoint = Freeze::createEvictor(_adapter, channelDbPathname, ICE_ChannelPublishPoint, 0, indices);
#endif
			_evitChannelPublishPoint->setSize(_config.ChannelPublishPointEvitSize);
			_adapter->addServantLocator(_evitChannelPublishPoint, ICE_ChannelPublishPoint);

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "ChannelPublishPoint evictor created"));
		}

		// open Purchase evictor
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxChannel2Purchase);
			indices.push_back(_idxPlaylistId2Purchase);

#if ICE_INT_VERSION/100 >302
			_evitPurchase = Freeze::createBackgroundSaveEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS, ICE_ChannelPurchase, 0, indices);
#else			
			_evitPurchase = Freeze::createEvictor(_adapter, purchaseDbPathname, ICE_ChannelPurchase, 0, indices);
#endif
			_evitPurchase->setSize(_config.PurchaseEvitSize);
			_adapter->addServantLocator(_evitPurchase, ICE_ChannelPurchase);
			_adapter->addServantLocator(_evitPurchase, ICE_ChannelPurchaseEx);

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "ChannelPurchase evictor created"));
		}

		// open PurchaseItemAssoc evictor
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxCtrlNum2ItemAssoc);
			indices.push_back(_idxPurchase2ItemAssoc);

#if ICE_INT_VERSION/100 >302
			_evitPurchaseItemAssoc = Freeze::createBackgroundSaveEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS, ICE_PurchaseItemAssoc, 0, indices);
#else	
			_evitPurchaseItemAssoc = Freeze::createEvictor(_adapter, purchaseDbPathname, ICE_PurchaseItemAssoc, 0, indices);
			#endif
			_evitPurchaseItemAssoc->setSize(_config.PurchaseItemEvitSize);
			_adapter->addServantLocator(_evitPurchaseItemAssoc, ICE_PurchaseItemAssoc);

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "PurchaseItemAssoc evictor created"));
		}

		// ChannelOnDemand application service
		_chOdSvc = new ChannelOnDemandAppImpl(*this);
        //_adapter->add(_chOdSvc, _communicator->stringToIdentity(CHANNEL_ONDEMAND_APPNAME));
        _adapter->ZQADAPTER_ADD(_communicator, _chOdSvc, CHANNEL_ONDEMAND_APPNAME);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "ChannelOnDemandApp interface added to adatper"));

		// ChannelPublisher
		_publisher = new ::ZQChannelOnDemand::ChannelPublisherImpl(*this);

#ifdef ENABLE_CHANNEL_EXPIRE_MANAGER
		_publisher->setMonitorTraceFlag(true);	
#endif

        //_adapter->add(_publisher, _communicator->stringToIdentity(SERVICE_NAME_ChannelPublisher));
        _adapter->ZQADAPTER_ADD(_communicator, _publisher, SERVICE_NAME_ChannelPublisher);
        //_adapter->add(_publisher, _communicator->stringToIdentity(SERVICE_NAME_ChannelPublisherEx));
        //_adapter->ZQADAPTER_ADD(_communicator, _publisher, SERVICE_NAME_ChannelPublisherEx);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "ChannelPublisher interface added to adatper"));
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught ice exception: %s"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught unknown exception"));
		return false;
	}
	glog.flush();

	// init the object factory for objects
	_factory = new ChODFactory(*this);

	try
	{
		_adapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught ice exception while active adapter: %s"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught unknown exception while active adapter"));
		return false;
	}
	glog.flush();

	ConnectIceStromRequest* pConnIceStorm = new ConnectIceStromRequest(*this);
	pConnIceStorm->start();

	//
	// connect todas
	//
	if (_config.authInfo.enable)
	{
		map<std::string, ZQ::common::Config::Holder< AuthorizationParam > >::const_iterator iter;
		iter = _config.authInfo.authorizationParams.find("endpoint");
		if(iter == _config.authInfo.authorizationParams.end())
			return false;
		try
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Connecting to TODAS[%s]"), (iter->second.value).c_str());

			_todasPrx = ::com::izq::todas::integration::cod::TodasForCodPrx::uncheckedCast(_communicator->stringToProxy(iter->second.value));
		}
		catch(const::Ice::ProxyParseException& e)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught ProxyParseException, %s"), e.str.c_str());
			return false;
		}
		catch(const::Ice::NoEndpointException& e)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught NoEndpointException, %s"), e.ice_name().c_str());
			return false;
		}
		catch(const::Ice::ObjectNotFoundException& e)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught ObjectNotFoundException, %s"), e.ice_name().c_str());
			return false;
		}
		catch(const::Ice::ObjectNotExistException& e)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught ObjectNotExistException, %s"), e.ice_name().c_str());
			return false;
		}
		catch (const::Ice::Exception& e)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught ice Exception, %s"), e.ice_name().c_str());
			return false;
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Caught unknown exception"));
			return false;
		}
/*		
		if(!_todasPrx)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Failed to get todas object with endpoint %s"), (iter->second.value).c_str());
			return false;
		}
*/
		glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "TODAS connected"));
	}
	// init session watch dog
	_pWatchDog = new WatchDog(*this);
	if (NULL == _pWatchDog)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "watch dog create failed"));
		return false;
	}
	_pWatchDog->start();

	// watch purchases under watchdog
	if (_config.purchaseTimeout < 1800)
	{
		_config.purchaseTimeout = 1800;
		glog(ZQ::common::Log::L_NOTICE, CLOGFMT(LOG_MODULE_NAME, "tune purchase timeout value to 1800s"));
	}
	int nPurchaseNum = 0;
	::Freeze::EvictorIteratorPtr tItor = _evitPurchase->getIterator("", 20000);
	while (tItor->hasNext())
	{
		__int64 rn_value = _config.purchaseTimeout * 1000 + (50 * nPurchaseNum ++);
		Ice::Identity ident = tItor->next();
		_pWatchDog->watch(ident.name, rn_value);
#if TestSyncPlaylist
		SyncPlaylistRequest* pRequest = new SyncPlaylistRequest(*this, ident);
		if (pRequest)
			pRequest->start();
#endif
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "ChannelOnDemand initialized successfully"));

	_bInited = true;
	return true;
}

void ChODSvcEnv::unInit()
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "unInit ChODSvcEnv object"));

	// destroy watch dog
	if (NULL != _pWatchDog)
		try {delete _pWatchDog;} catch (...){}
	_pWatchDog = NULL;

	// destroy native thread pool
	if (NULL != _pThreadPool)
		try {delete _pThreadPool;} catch (...){}
	_pThreadPool = NULL;

	if (NULL != _pChannelItemDict)
		try {delete _pChannelItemDict;} catch (...){}
	_pChannelItemDict=NULL;

	_evitChannelPublishPoint	= NULL;
	_evitPurchase               = NULL;
	_evitPurchaseItemAssoc		= NULL;

	_idxChannelNameIndex		= NULL;
	_idxChannel2Purchase		= NULL;
	_idxCtrlNum2ItemAssoc		= NULL;
	_idxPurchase2ItemAssoc		= NULL;
	_idxPlaylistId2Purchase		= NULL;	

	_connCh                     = NULL;
	
	_bInited = false;
}

// available error code [100, 200)
// throw exception list ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void ChODSvcEnv::appendPlaylistItem(const ::std::string& chnlName, const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& appendChnlItem)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "append [%s] to all purchases of channel [%s]"), 
		appendChnlItem.key.c_str(), chnlName.c_str());

	// find all purchases of channel
	std::vector<Ice::Identity> idents;
	try
	{
		idents = _idxChannel2Purchase->find(chnlName);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 100, "find all purchases of channel[%s] caught %s:%s", 
			chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 101, "find all purchases of channel[%s] caught %s", 
			chnlName.c_str(), ex.ice_name().c_str());
	}

	// iterator all purchase
	for (unsigned int i = 0; i < idents.size(); i ++)
	{
		AppendItemRequest* pRequest = new AppendItemRequest(*this, idents[i], appendChnlItem);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "[%s] create purchase request for append item failed"), 
				idents[i].name.c_str());
		}
	}
}

// available error code [200, 300)
void ChODSvcEnv::insertPlaylistItem(const ::std::string& chnlName, const ::std::string& istPosKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& insertChnlItem)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "insert [%s] before [%s] to all purchases of channel [%s]"), 
		insertChnlItem.key.c_str(), istPosKey.c_str(), chnlName.c_str());

	// find all purchases of channel
	std::vector<Ice::Identity> idents;
	try
	{
		idents = _idxChannel2Purchase->find(chnlName);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 200, "find all purchases of channel[%s] caught %s:%s", 
			chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 201, "find all purchases of channel[%s] caught %s", 
			chnlName.c_str(), ex.ice_name().c_str());
	}

	for (unsigned int i = 0; i < idents.size(); i ++)
	{
		InsertItemRequest* pRequest = new InsertItemRequest(*this, idents[i], istPosKey, insertChnlItem);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "[%s] create purchase request for insert item failed"), 
				idents[i].name.c_str());
		}
	}
}

// available error code [300, 400)
// throw exception list ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void ChODSvcEnv::removePurchases(const ::std::string& chnlName)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "remove all purchases of channel [%s]"), chnlName.c_str());

	// find all purchases of channel
	std::vector<Ice::Identity> idents;
	try
	{
		idents = _idxChannel2Purchase->find(chnlName);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 300, "find all purchases of channel[%s] caught %s:%s", 
			chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 301, "find all purchases of channel[%s] caught %s", 
			chnlName.c_str(), ex.ice_name().c_str());
	}

	char buffRsn[MAX_PATH];
	memset(buffRsn, 0, sizeof(buffRsn));
	_snprintf(buffRsn, sizeof(buffRsn) - 1, "purchase terminated per channel[%s] deletion", chnlName.c_str());
	for (unsigned int i = 0; i < idents.size(); i ++)
	{
		RemovePurchaseRequest* pRequest = new RemovePurchaseRequest(*this, idents[i], "", buffRsn, true);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "[%s] create purchase request for remove purchase failed"), 
				idents[i].name.c_str());
		}
	}
}

// available error code [400, 500)
void ChODSvcEnv::removePlaylistItem(const ::std::string& chnlName, const ::std::string& rmvItemKey)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "remove [%s] from all purchases of channel [%s]"), 
		rmvItemKey.c_str(), chnlName.c_str());

	std::vector<Ice::Identity> idents;
	try
	{
		idents = _idxChannel2Purchase->find(chnlName);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 400, "find all purchases of channel[%s] caught %s:%s", 
			chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 401, "find all purchases of channel[%s] caught %s", 
			chnlName.c_str(), ex.ice_name().c_str());
	}

	for (unsigned int i = 0; i < idents.size(); i ++)
	{
		RemoveItemRequest* pRequest = new RemoveItemRequest(*this, idents[i], rmvItemKey);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "[%s] create purchase request for remove item failed"), 
				idents[i].name.c_str());
		}
	}

}

// available error code [500, 600)
// throw exception list ServerError;
// if the inside function call throw an exception which outside the above list, 
// you must transfer it.
void ChODSvcEnv::replacePlaylistItem(const ::std::string& chnlName, const ::std::string& oldItemKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& replaceChnlItem)
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "replace [%s] with [%s] on all purchases of channel [%s]"), 
		oldItemKey.c_str(), replaceChnlItem.key.c_str(), chnlName.c_str());

	std::vector<Ice::Identity> idents;
	try
	{
		idents = _idxChannel2Purchase->find(chnlName);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 500, "find all purchases of channel[%s] caught %s:%s", 
			chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, LOG_MODULE_NAME, 501, "find all purchases of channel[%s] caught %s", 
			chnlName.c_str(), ex.ice_name().c_str());
	}

	for(unsigned int i = 0; i < idents.size(); i ++)
	{
		ReplaceItemRequest* pRequest = new ReplaceItemRequest(*this, idents[i], oldItemKey, replaceChnlItem);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "[%s] create purchase request for replace item failed"), 
				idents[i].name.c_str());
		}
	}
}

// using this event to remove the corresponding purchase
void ChODSvcEnv::OnStreamExit(const std::string& playlistId)
{
	glog(ZQ::common::Log::L_DEBUG, "Stream Exit [%s]", playlistId.c_str());
#pragma message(__MSGLOC__"TODO: here we can catch this event to destroy the corresponding purchase")
}

void ChODSvcEnv::OnEndOfStream(const std::string& playlistId)
{
	glog(ZQ::common::Log::L_DEBUG, "End-of-Stream [%s]", playlistId.c_str());
}

void ChODSvcEnv::OnEndOfItem(const std::string& playlistId, int userCtrlNum)
{
	glog(ZQ::common::Log::L_DEBUG, "End-of-Item [%s]", playlistId.c_str());

	std::vector<Ice::Identity> idents;
	try
	{
		idents = _idxPlaylistId2Purchase->find(playlistId);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "stream [%s] to purchase caught %s: %s"), 
			playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "stream [%s] to purchase caught %s"), 
			playlistId.c_str(), ex.ice_name().c_str());
		return;
	}

	if (idents.size() == 0)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "no purchase associated with stream [%s]"), 
			playlistId.c_str());
		return;
	}

	SyncPlaylistRequest* pRequest = new SyncPlaylistRequest(*this, idents[0]);
	if (pRequest)
	{
		pRequest->start();
	}
	else 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "[%s] create purchase request for sync playlist failed"), 
			idents[0].name.c_str());
	}
}

}	// namespace ZQChannelOnDemand