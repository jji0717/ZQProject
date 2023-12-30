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
// Name  : BroadCastChannleEnv.cpp
// Author: li. Huang
// Date  : 
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BroadCastChannelEnv.cpp $
// 
// 16    2/26/16 5:00p Li.huang
// fix bug 22376
// 
// 15    1/13/16 10:40a Dejian.fei
// 
// 14    12/28/15 1:56p Hui.shao
// 
// 12    7/27/15 2:15p Li.huang
// 
// 11    7/23/15 5:04p Li.huang
// 
// 10    7/22/15 2:09p Li.huang
// 
// 9     7/07/15 4:37p Li.huang
// fix bug 21518
// 
// 8     11/14/14 9:42a Li.huang
// 
// 7     11/03/14 3:29p Li.huang
// 
// 6     10/21/14 3:32p Li.huang
// 
// 5     10/17/14 3:37p Li.huang
// 
// 4     5/30/14 4:43p Li.huang
// 
// 3     5/30/14 3:55p Li.huang
// 
// 2     5/30/14 2:41p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 4     10-05-28 17:17 Li.huang
// add  ice property BtreeMinKey
// 
// 3     09-12-15 16:10 Li.huang
// 
// 3     09-12-09 18:21 Li.huang
// modify some logs
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "BroadCastChannelEnv.h"
#include "BroadcastChCfg.h"
#include "BroadcastChService.h"
#include "PlaylistEventSinkImpl.h"
#include "FileSystemOp.h"

// use thread pool to process item
#define USE_THREAD_POOL

extern ZQ::common::Config::Loader<BroadcastChCfg> gBroadcastChCfg;

MRTProxy*	_serviceInstance;

namespace ZQBroadCastChannel {

UserCtrlNumGenerator	_gUserCtrlNumGen;

	//////////////////////////////////////////////////////////////////////////
	// class BroadCastChannelEnv
	//////////////////////////////////////////////////////////////////////////
	BroadCastChannelEnv::BroadCastChannelEnv(Ice::CommunicatorPtr& communicator)
		: _communicator(communicator), _publisher(NULL), _adapter(NULL), 
		_pChannelItemDict(NULL), _factory(NULL), _watchDog(*this), _evtAdap(NULL),_mrtAdapter(NULL), _replicaUpdater(NULL)
	{
		_bInited = false;
		_sessManager = NULL;

		_nvodSupplementManager.clear();

		if(gBroadcastChCfg.renewtime < 60000)
			gBroadcastChCfg.renewtime = 60000;
		_sessionRewTime = gBroadcastChCfg.renewtime;
	}

	BroadCastChannelEnv::~BroadCastChannelEnv()
	{
		unInit();
	}

	void dumpLine(const char* line, void* pCtx)
	{
		if (line)
			glog(ZQ::common::Log::L_DEBUG, line);
	}

	bool BroadCastChannelEnv::connectEventChannel()
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "connectEventChannel() connecting"));

		try
		{
			_eventChannel = new TianShanIce::Events::EventChannelImpl(_evtAdap, gBroadcastChCfg.TopicMgrEndPoint.c_str());
			TianShanIce::Streamer::StreamEventSinkPtr _evtStream = new StreamEventSinkImpl(*this);
			TianShanIce::Streamer::PlaylistEventSinkPtr _evtPlaylist = new PlaylistEventSinkImpl(*this);
			TianShanIce::Properties qos;
			_eventChannel->sink(_evtStream, qos);
			_eventChannel->sink(_evtPlaylist, qos);
			_eventChannel->start();
		}
		catch (TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "connectEventChannel(%s) caught(%s: %s)")
				, gBroadcastChCfg.TopicMgrEndPoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			return false;
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "connectEventChannel(%s) caught(%s)")
				, gBroadcastChCfg.TopicMgrEndPoint.c_str(), ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "connectEventChannel(%s) caught unexpect exception")
				, gBroadcastChCfg.TopicMgrEndPoint.c_str());
			return false;
		}

		glog(ZQ::common::Log::L_NOTICE, CLOGFMT(BroadCastChannelEnv, "connectEventChannel() successfully"));
		return true;
	}

	bool BroadCastChannelEnv::connectWeiwoo()
	{
		bool bret = true;

		if(NULL == _sessManager)
		{
			try	{	
				glog(Log::L_DEBUG,  CLOGFMT(BroadCastChannelEnv, "connect to weiwoo service at endpoint[%s]"), gBroadcastChCfg.weiwooendpoint.c_str());
				std::string weiwooEndpoint = gBroadcastChCfg.weiwooendpoint;
				_sessManager = TianShanIce::SRM::SessionManagerPrx::checkedCast(_communicator->stringToProxy(weiwooEndpoint));

				glog(Log::L_INFO,  CLOGFMT(BroadCastChannelEnv, "connected to weiwoo service at endpoint[%s] successfully"),  gBroadcastChCfg.weiwooendpoint.c_str());	
			} 
			catch (const ::Ice::Exception & ex)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "fail to connect to weiwoo service, caught ice exception (%s)"),
					ex.ice_name().c_str());
				bret = false;
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "fail to connect to weiwoo service, caught unknown exception (%d)"),
					SYS::getLastErr());
				bret = false;
			}
		}

		return bret;
	}
	bool BroadCastChannelEnv::init()
	{
		if(gBroadcastChCfg.ThreadPoolSize < 20)
			gBroadcastChCfg.ThreadPoolSize = 20;
		// create native thread pool
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "resize threadpool[%d]"), gBroadcastChCfg.ThreadPoolSize);
        _threadPool.resize(gBroadcastChCfg.ThreadPoolSize);

		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "start watchdog"));
		_watchDog.start();

		_endpoint = gBroadcastChCfg.broadcastPPendpoint;	
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "open adapter %s at %s"), ADAPTER_NAME_BcastChannel, _endpoint.c_str());

		_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_BcastChannel, _endpoint.c_str(), glog);

		// create listen event adapter
		try 
		{
			_evtAdap = _communicator->createObjectAdapterWithEndpoints("BroadcastChannelEventAdapter", gBroadcastChCfg.ListenEventEndPoint);
			_evtAdap->activate();
		}
		catch(Ice::Exception& ex) 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "create adapter for listen event caught %s, endpoint is %s"), 
				ex.ice_name().c_str(), gBroadcastChCfg.ListenEventEndPoint.c_str());
			return false;
		}

		// check path
		{
			if (!gBroadcastChCfg.dbpath.size())
			{
				const char* path = ZQTianShan::getModulesPath();
				_dbPath = std::string(path) + std::string(FNSEPS "data" FNSEPS);
			}
			else _dbPath = gBroadcastChCfg.dbpath;

			if (FNSEPC != _dbPath[_dbPath.length()-1])
				_dbPath += FNSEPS;

			if(!gBroadcastChCfg.dbRuntimeDataPath.size())
			{
				_runtimePath = _dbPath;
			}
			else _runtimePath = gBroadcastChCfg.dbRuntimeDataPath;

			if (FNSEPC != _runtimePath[_runtimePath.length()-1])
				_runtimePath += FNSEPS;
		}

		try {

			glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "opening database at path: %s"), _dbPath.c_str());

#ifdef ZQ_OS_MSWIN
			// open dictionary
			::CreateDirectoryA((_dbPath + FNSEPS).c_str(), NULL);
			::CreateDirectoryA((_dbPath + ServiceSubDir FNSEPS).c_str(), NULL);
//			::CreateDirectoryA((_dbPath + ServiceSubDir FNSEPS BcastChannelDataSubDir FNSEPS).c_str(), NULL);
//			::CreateDirectoryA((_dbPath + ServiceSubDir FNSEPS NVODChannelDataSubDir FNSEPS).c_str(), NULL);
//			::CreateDirectoryA((_dbPath + ServiceSubDir FNSEPS FilterSubDir FNSEPS).c_str(), NULL);

			glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "opening runtime database at path: %s"), _runtimePath.c_str());

			::CreateDirectoryA((_runtimePath + FNSEPS).c_str(), NULL);
			::CreateDirectoryA((_runtimePath + ServiceSubDir FNSEPS).c_str(), NULL);
//			::CreateDirectoryA((_runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS).c_str(), NULL);
#else
			FS::createDirectory((std::string)(_dbPath + FNSEPS), true);
			FS::createDirectory((std::string)(_dbPath + ServiceSubDir FNSEPS), true);

	//		FS::createDirectory((std::string)(_dbPath + ServiceSubDir FNSEPS BcastChannelDataSubDir FNSEPS), true);
	//		FS::createDirectory((std::string)(_dbPath + ServiceSubDir FNSEPS NVODChannelDataSubDir FNSEPS), true);
	//		FS::createDirectory((std::string)(_dbPath + ServiceSubDir FNSEPS FilterSubDir FNSEPS), true);
			glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "opening runtime database at path: %s"), _runtimePath.c_str());

			FS::createDirectory((std::string)(_runtimePath + FNSEPS), true);
			FS::createDirectory((std::string)(_runtimePath + ServiceSubDir FNSEPS), true);
//			FS::createDirectory((std::string)(_runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS)), true);
#endif
			std::string BcastChannelsDbPathname    = ZQTianShan::createDBFolder(glog, "BroadCastChannel", _dbPath.c_str(), BcastChannelDataSubDir);
			std::string NVODChannelsDbPathname = ZQTianShan::createDBFolder(glog, "BroadCastChannel", _dbPath.c_str(), NVODChannelDataSubDir);
			std::string FilterItemsDbPathname    = ZQTianShan::createDBFolder(glog, "BroadCastChannel", _dbPath.c_str(), FilterSubDir);
			std::string PurchasesDbPathname = ZQTianShan::createDBFolder(glog, "BroadCastChannel", _runtimePath.c_str(), PurchaseSubDir);

             glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "Create indices..."));
			_idxPlaylistId2BcastPublishPoint = new ZQBroadCastChannel::PlaylistId2BcastPublishPoint(INDEXFILENAME(PlaylistId2BcastPublishPoint));
			_idxCtrlNumber2ChannelItemAssoc = new ZQBroadCastChannel::CtrlNum2ChannelItemAssoc(INDEXFILENAME(CtrlNum2ChannelItemAssoc));
			_idxBcastPublishPoint2ItemAssoc = new ZQBroadCastChannel::BcastPublishPoint2ItemAssoc(INDEXFILENAME(BcastPublishPoint2ItemAssoc));
			_idxChItem2ItemAssoc = new ZQBroadCastChannel::ChItem2ItemAssoc(INDEXFILENAME(ChItem2ItemAssoc));
			_idxmainChName2BcastPublishPoint = new ZQBroadCastChannel::mainChName2BcastPublishPoint(INDEXFILENAME(mainChName2BcastPublishPoint));

			_connCh = Freeze::createConnection(_adapter->getCommunicator(), (_dbPath + ServiceSubDir FNSEPS BcastChannelDataSubDir FNSEPS));

			_pChannelItemDict = new ZQBroadCastChannel::ChannelItemDict(_connCh, ICE_BcastChannelItemDict);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "Dictionary of ChannelItem created"));

			// open BroadcastChannelPublishPoint evictor
			Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();
			std::string evictorAttrPrefix = std::string("Freeze.Evictor.");
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_BcastChannelPublishPoint+ ".$default.BtreeMinKey",      "16");
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_BcastChannelPublishPoint ".PageSize",      "8192");

			{
				std::vector<Freeze::IndexPtr> indices;
				indices.push_back(_idxPlaylistId2BcastPublishPoint);
				indices.push_back(_idxmainChName2BcastPublishPoint);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "create evictor %s"), ICE_BcastChannelPublishPoint);
#if ICE_INT_VERSION/100 >302
				_evitBcastChannelPublishPoint = Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + ServiceSubDir FNSEPS BcastChannelDataSubDir FNSEPS, ICE_BcastChannelPublishPoint, 0, indices);
#else	
				_evitBcastChannelPublishPoint = Freeze::createEvictor(_adapter, _dbPath + ServiceSubDir FNSEPS BcastChannelDataSubDir FNSEPS, ICE_BcastChannelPublishPoint, 0, indices);
#endif
				_evitBcastChannelPublishPoint->setSize(gBroadcastChCfg.BroadcastPPEvitSize);
				_adapter->addServantLocator(_evitBcastChannelPublishPoint, ICE_BcastChannelPublishPoint);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "BcastChannelPublishPoint evictor created"));
			}

		    // open NOVDChannelPublishPoint evictor
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_NVODChannelPublishPoint+ ".$default.BtreeMinKey",      "16");
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_NVODChannelPublishPoint ".PageSize",      "8192");

			{
				std::vector<Freeze::IndexPtr> indices;

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "create evictor %s"), ICE_NVODChannelPublishPoint);
#if ICE_INT_VERSION/100 >302
				//_evitNOVDChannelPublishPoint = Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + ServiceSubDir FNSEPS ChannelDataSubDir FNSEPS, ICE_ChannelPublishPoint, 0, indices);
				_evitNOVDChannelPublishPoint = Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + ServiceSubDir FNSEPS NVODChannelDataSubDir FNSEPS, ICE_NVODChannelPublishPoint, 0, indices);
#else	
				_evitNOVDChannelPublishPoint = Freeze::createEvictor(_adapter, _dbPath + ServiceSubDir FNSEPS NVODChannelDataSubDir FNSEPS, ICE_NVODChannelPublishPoint, 0, indices);
#endif
				_evitNOVDChannelPublishPoint->setSize(gBroadcastChCfg.BroadcastPPEvitSize);
				_adapter->addServantLocator(_evitNOVDChannelPublishPoint, ICE_NVODChannelPublishPoint);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "NOVDChannelPublishPoint evictor created"));
			}

			// open  Purchase evictor
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_BcastChannelPurchase+ ".$default.BtreeMinKey",      "16");
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_BcastChannelPurchase ".PageSize",      "8192");

			{
				std::vector<Freeze::IndexPtr> indices;
#if ICE_INT_VERSION/100 >302
				_evitPurchase = Freeze::createBackgroundSaveEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS, ICE_BcastChannelPurchase, 0, indices);
#else			
				_evitPurchase = Freeze::createEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS, ICE_BcastChannelPurchase, 0, indices);
#endif
				_evitPurchase->setSize(20);
				_adapter->addServantLocator(_evitPurchase, ICE_BcastChannelPurchase);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "ChannelPurchase evictor created"));
			}

			// open  FilterItems evictor
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_FilterItems+ ".$default.BtreeMinKey",      "16");
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_FilterItems ".PageSize",      "8192");

			{
				std::vector<Freeze::IndexPtr> indices;
#if ICE_INT_VERSION/100 >302
				_evitFilterItems = Freeze::createBackgroundSaveEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS FilterSubDir FNSEPS, ICE_FilterItems, 0, indices);
#else			
				_evitFilterItems = Freeze::createEvictor(_adapter, _dbPath + ServiceSubDir FNSEPS FilterSubDir FNSEPS, ICE_FilterItems, 0, indices);
#endif
				_evitFilterItems->setSize(20);
				_adapter->addServantLocator(_evitFilterItems, ICE_FilterItems);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "FilterItems evictor created"));
			}

			// open ChannelItemAssoc evictor
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_ChannelItemAssoc+ ".$default.BtreeMinKey",      "16");
			proper->setProperty(evictorAttrPrefix + (std::string)ICE_ChannelItemAssoc ".PageSize",      "8192");

			{
				std::vector<Freeze::IndexPtr> indices;
				indices.push_back(_idxCtrlNumber2ChannelItemAssoc);
				indices.push_back(_idxBcastPublishPoint2ItemAssoc);
				indices.push_back(_idxChItem2ItemAssoc);

#if ICE_INT_VERSION/100 >302
				//_evitChannelItemAssoc = Freeze::createBackgroundSaveEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS, ICE_PurchaseItemAssoc, 0, indices);
				_evitChannelItemAssoc = Freeze::createBackgroundSaveEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS, ICE_ChannelItemAssoc, 0, indices);
#else	
				_evitChannelItemAssoc = Freeze::createEvictor(_adapter, _runtimePath + ServiceSubDir FNSEPS PurchaseSubDir FNSEPS, ICE_ChannelItemAssoc, 0, indices);
#endif
				_evitChannelItemAssoc->setSize(gBroadcastChCfg.ChannelItemEvitSize);
				_adapter->addServantLocator(_evitChannelItemAssoc, ICE_ChannelItemAssoc);

				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "ChannelItemAssoc evictor created"));
			}


			// init the object factory for objects
			_factory = new ZQBroadCastChannel::BcastChFactory(*this);

			// BcastChannel application service
			_BcastAppSvc = new BcastAppServiceImpl(*this);
			if(!_BcastAppSvc->init())
				return false;
			_adapter->ZQADAPTER_ADD(_communicator, _BcastAppSvc, Broadcast_APPNAME);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "BcastAppService interface added to adatper"));

			// ChannelPublisher
			_publisher = new BcastPublisherImpl(*this);
			_adapter->ZQADAPTER_ADD(_communicator, _publisher, SERVICE_NAME_BcastchannelPublisher);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "ChannelPublisher interface added to adatper"));

            //create filteritems object
			{
				Ice::ObjectPrx objprx;
				Ice::Identity ident;
				ident.category = ICE_FilterItems;
				ident.name = "FilterItems";		
				if(_evitFilterItems->hasObject(ident) == NULL)
				{	
					FilterItemsImpl* pfilteritems = new FilterItemsImpl(*this);

					objprx = _evitFilterItems->add(pfilteritems, ident);
					_FilterItems = TianShanIce::Application::Broadcast::FilterItemsPrx::uncheckedCast(objprx);
				}  
				else
				{
					objprx = _adapter->createProxy(ident);	
					_FilterItems = TianShanIce::Application::Broadcast::FilterItemsPrx::checkedCast(objprx);
				}		
				if(_FilterItems == NULL)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv,
						"init() Invaild FilterItems proxy"));
					return false;
				}
			}

			glog.flush();

			ConnectIceStromRequest* pConnIceStorm = new ConnectIceStromRequest(*this);
			pConnIceStorm->start();

			connectWeiwoo();

			//init broadcast session renew thread

			{
				try
				{
					::Freeze::EvictorIteratorPtr tItor = _evitBcastChannelPublishPoint->getIterator("", 20000);
					while (tItor->hasNext())
					{
						Ice::Identity ident = tItor->next();
						TianShanIce::Application::Broadcast::BcastPublishPointExPrx pointPrx = 
							TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(_adapter->createProxy(ident));
						pointPrx->activate();					
					}
				}
				catch(const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught ice exception: %s"), ex.ice_name().c_str());
					return false;
				}
				catch(...)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught unknown exception: %d"), SYS::getLastErr());
					return false;
				}
			}

			//init NVOD supplement Manager
			{
				try
				{
					::Freeze::EvictorIteratorPtr tItor = _evitNOVDChannelPublishPoint->getIterator("", 20000);
					while (tItor->hasNext())
					{
						Ice::Identity ident = tItor->next();
						TianShanIce::Application::Broadcast::NVODChannelPublishPointExPrx pointPrx = 
							TianShanIce::Application::Broadcast::NVODChannelPublishPointExPrx::uncheckedCast(_adapter->createProxy(ident));
						pointPrx->activate();
					}
				}
				catch(const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught ice exception: %s"), ex.ice_name().c_str());
					return false;
				}
				catch(...)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught unknown exception: %d"), SYS::getLastErr());
					return false;
				}
			}

			try
			{
				_adapter->activate();
			}
			catch(const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught ice exception while active adapter: %s"), ex.ice_name().c_str());
				return false;
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught unknown exception while active adapter"));
				return false;
			}


			if(gBroadcastChCfg.mrtStreamServiceCfg.enable)
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "initialize StreamService"));

				try
				{
					glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "create StreamService Adapter at endpoint[%s]"), gBroadcastChCfg.mrtStreamServiceCfg.bindEndPoint.c_str());

					_mrtAdapter=_communicator->createObjectAdapterWithEndpoints( "MRTSS" , gBroadcastChCfg.mrtStreamServiceCfg.bindEndPoint);
				}
				catch ( const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnStart,"catch Ice Exception when create object adapter\n"));
					return S_FALSE;			 
				}
				_serviceInstance = new BcastMRTProxy(*this, _mrtAdapter,_runtimePath + ServiceSubDir FNSEPS, gBroadcastChCfg.mrtStreamServiceCfg.nodeId, gBroadcastChCfg.mrtStreamServiceCfg.spigotIds, gBroadcastChCfg.mrtStreamServiceCfg.streamToMRTEndpointInfos, glog, gBroadcastChCfg.mrtStreamServiceCfg.targetTime);
				_serviceInstance->listenerEndpoint = gBroadcastChCfg.mrtStreamServiceCfg.replicaSubscriberEndpoint;
				_serviceInstance->updateInterval = 60*1000;
				_serviceInstance->setStreamPenalty(gBroadcastChCfg.mrtStreamServiceCfg.penalty, gBroadcastChCfg.mrtStreamServiceCfg.maxPenalty);
				_service= _serviceInstance ;

				if( !gBroadcastChCfg.mrtStreamServiceCfg.eventChannel.empty() ) {
					_serviceInstance->connectToEventChannel(gBroadcastChCfg.mrtStreamServiceCfg.eventChannel);
				}
				try
				{
					_mrtAdapter->add(_service,_communicator->stringToIdentity("StreamService"));	
					_mrtAdapter->activate();

					if( !gBroadcastChCfg.mrtStreamServiceCfg.replicaSubscriberEndpoint.empty() )
					{
						_replicaUpdater = new ReplicaUpdater(*_serviceInstance);
						if(_replicaUpdater)
							_replicaUpdater->start();
					}

					glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "StreamService interface added to adatper"));
				}
				catch(const Ice::Exception& ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnStart,"catch Ice Exception when create object adapter\n"));
					return S_FALSE;			 
				}
			}
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught ice exception: %s"), ex.ice_name().c_str());
			return false;
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "Caught unknown exception"));
			return false;
		}

		glog.flush();
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "BroadCastChannel initialized successfully"));

		_bInited = true;
		return true;
	}

	void BroadCastChannelEnv::unInit()
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "unInit BroadCastChannelEnv object"));

        /// stop all of session renew thread
		try
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "quit watch dog"));
			_watchDog.quit();

			{
				NVODSupplementManager::iterator itor = _nvodSupplementManager.begin();
				while(itor != _nvodSupplementManager.end())
				{
					itor->second->stop();
					itor++;
				}
				_nvodSupplementManager.clear();
			}
			if(_evtAdap)
				_evtAdap->deactivate();

			if(_replicaUpdater)
			{
				_replicaUpdater->stop();
				delete _replicaUpdater;
				_replicaUpdater = NULL;
			}
			if(_mrtAdapter)
				_mrtAdapter->deactivate();

			if(_adapter)
				_adapter->deactivate();

		}
		catch(...){}	
		

		if (NULL != _pChannelItemDict)
			try {delete _pChannelItemDict;} catch (...){}
		_pChannelItemDict=NULL;

		_evitBcastChannelPublishPoint	= NULL;
		_evitPurchase                   = NULL;
		_evitNOVDChannelPublishPoint    = NULL;
		_evitChannelItemAssoc           = NULL;

		_idxPlaylistId2BcastPublishPoint	= NULL;
		_idxCtrlNumber2ChannelItemAssoc	    = NULL;
		_idxBcastPublishPoint2ItemAssoc	    = NULL;
		_idxChItem2ItemAssoc	            = NULL;
		_idxmainChName2BcastPublishPoint	= NULL;

		_connCh                         = NULL;
		_bInited = false;
		_evtAdap = NULL;
		_serviceInstance = NULL;
		_service = NULL;
		_mrtAdapter = NULL;
		_adapter = NULL;
	}

	// using this event to remove the corresponding purchase
	void BroadCastChannelEnv::OnStreamExit(const std::string& playlistId)
	{
//		glog(ZQ::common::Log::L_DEBUG, "Stream Exit [%s]", playlistId.c_str());

		std::vector<Ice::Identity> idents;
		try
		{
			idents = _idxPlaylistId2BcastPublishPoint->find(playlistId);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "stream [%s] to BcastPublishPoint caught %s: %s"), 
				playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			return;
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "stream [%s] to BcastPublishPoint caught %s"), 
				playlistId.c_str(), ex.ice_name().c_str());
			return;
		}

		if (idents.size() == 0)
		{
//			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "no BcastPublishPoint associated with stream [%s]"), 
//				playlistId.c_str());
			return;
		}

		OnStreamExitRequest* pRequest = new OnStreamExitRequest(*this, idents[0], playlistId);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "[%s] create On Stream Exit request failed"), idents[0].name.c_str());
		}
	}

	void BroadCastChannelEnv::OnEndOfStream(const std::string& playlistId)
	{
		glog(ZQ::common::Log::L_DEBUG, "End-of-Stream [%s]", playlistId.c_str());
		std::vector<Ice::Identity> idents;
		try
		{
			idents = _idxPlaylistId2BcastPublishPoint->find(playlistId);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "stream [%s] to BcastPublishPoint caught %s: %s"), 
				playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			return;
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "stream [%s] to BcastPublishPoint caught %s"), 
				playlistId.c_str(), ex.ice_name().c_str());
			return;
		}

		if (idents.size() == 0)
		{
//			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "no BcastPublishPoint associated with stream [%s]"), 
//				playlistId.c_str());
			return;
		}

		OnEndOfStreamRequest* pRequest = new OnEndOfStreamRequest(*this, idents[0], playlistId);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, 
				"[%s] create On End Of Stream request failed"), 
				idents[0].name.c_str());
		}
	}

	void BroadCastChannelEnv::OnEndOfItem(const std::string& playlistId, int userCtrlNum)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv,"End-of-Item [%s, %d]"), playlistId.c_str(), userCtrlNum);

		std::vector<Ice::Identity> idents;
		try
		{
			idents = _idxPlaylistId2BcastPublishPoint->find(playlistId);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "stream [%s] to purchase caught %s: %s"), 
				playlistId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			return;
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "stream [%s] to purchase caught %s"), 
				playlistId.c_str(), ex.ice_name().c_str());
			return;
		}

		if (idents.size() == 0)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, "no purchase associated with stream [%s]"), 
				playlistId.c_str());
			return;
		}

		OnEndOfItemRequest* pRequest = new OnEndOfItemRequest(*this, idents[0], playlistId, userCtrlNum);
		if (pRequest)
		{
			pRequest->start();
		}
		else 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "[%s] create on End Of Item request failed"), 
				idents[0].name.c_str());
		}
	}

	bool BroadCastChannelEnv::addNVODSupplMgr(const ::Ice::Identity& ident, ::Ice::Int interval, bool isInit)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv,
			"[%s] add NVOD Supplement Channel thread"), ident.name.c_str());

		NVODSupplementManager::iterator itor = _nvodSupplementManager.find(ident);
		if(itor != _nvodSupplementManager.end())
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, 
				"[%s]NVOD Supplement Channel thread already exist"), ident.name.c_str());
			itor->second->stop();
			_nvodSupplementManager.erase(itor);
		}

		NVODSupplementThread* pThread = NULL;
		pThread = new NVODSupplementThread(*this, ident, interval, isInit);
		if(NULL == pThread)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv,
				"fail to create NVOD Supplement Channel thread [%s]"), ident.name.c_str());
			return false;
		}

		pThread->start();

		_nvodSupplementManager.insert(NVODSupplementManager::value_type(ident, pThread));

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, 
			"[%s] add NVOD Supplement Channel thread successfully"), ident.name.c_str());
		return true;
	}
	bool BroadCastChannelEnv::removeNVODSupplMgr(const ::Ice::Identity& ident)
	{
		NVODSupplementManager::iterator itor = _nvodSupplementManager.find(ident);
		if(itor == _nvodSupplementManager.end())
		{
			return false;
		}

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, 
			"[%s] remove NVOD Supplement Channel thread "), ident.name.c_str());

		itor->second->stop();
		_nvodSupplementManager.erase(itor);	 
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BroadCastChannelEnv, 
			"[%s] remove NVOD Supplement Channel thread successfully"), ident.name.c_str());
		return true;
	}

	void BroadCastChannelEnv::appendPlaylistItem(const ::std::string& chnlName, const TianShanIce::Application::Broadcast::ChannelItemEx & appendChnlItem)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "append [%s] to all of channel [%s]"), 
			appendChnlItem.key.c_str(), chnlName.c_str());

		// find all purchases of channel
		std::vector<Ice::Identity> idents;
		try
		{
			idents = _idxmainChName2BcastPublishPoint->find(chnlName);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 100, "find all of channel[%s] caught %s:%s", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 101, "find all of channel[%s] caught %s", 
				chnlName.c_str(), ex.ice_name().c_str());
		}

		// iterator all broadcastpublishpoint
		for (unsigned int i = 0; i < idents.size(); i ++)
		{
			AppendItemRequest* pRequest = new AppendItemRequest(*this, idents[i], appendChnlItem);
			if (pRequest)
			{
				pRequest->start();
			}
			else 
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "[%s] create broadcastpublishpoint request for append item failed"), 
					idents[i].name.c_str());
			}
		}
	}

	void BroadCastChannelEnv::insertPlaylistItem(const ::std::string& chnlName, const ::std::string& istPosKey, 
		const TianShanIce::Application::Broadcast::ChannelItemEx & insertChnlItem)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "insert [%s] before [%s] to all of channel [%s]"), 
			insertChnlItem.key.c_str(), istPosKey.c_str(), chnlName.c_str());

		// find all broadcastpublishpoint of channel
		std::vector<Ice::Identity> idents;
		try
		{
			idents = _idxmainChName2BcastPublishPoint->find(chnlName);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 200, "find all of channel[%s] caught %s:%s", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 201, "find all of channel[%s] caught %s", 
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
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "[%s] create broadcastpublishpoint request for insert item failed"), 
					idents[i].name.c_str());
			}
		}
	}

	void BroadCastChannelEnv::removePlaylistItem(const ::std::string& chnlName, const ::std::string& rmvItemKey)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "remove [%s] from all purchases of channel [%s]"), 
			rmvItemKey.c_str(), chnlName.c_str());

		std::vector<Ice::Identity> idents;
		try
		{
			idents = _idxmainChName2BcastPublishPoint->find(chnlName);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 400, "find all of channel[%s] caught %s:%s", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 401, "find all of channel[%s] caught %s", 
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
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "[%s] create broadcastpublishpoint request for remove item failed"), 
					idents[i].name.c_str());
			}
		}
	}

	void BroadCastChannelEnv::replacePlaylistItem(const ::std::string& chnlName, const ::std::string& oldItemKey, 
		const TianShanIce::Application::Broadcast::ChannelItemEx & replaceChnlItem)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(BroadCastChannelEnv, "replace [%s] with [%s] on all purchases of channel [%s]"), 
			oldItemKey.c_str(), replaceChnlItem.key.c_str(), chnlName.c_str());

		std::vector<Ice::Identity> idents;
		try
		{
			idents = _idxmainChName2BcastPublishPoint->find(chnlName);
		}
		catch (const Freeze::DatabaseException& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 500, "find all of channel[%s] caught %s:%s", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "BroadCastChannelEnv", 501, "find all of channel[%s] caught %s", 
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
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(BroadCastChannelEnv, "[%s] create broadcastpublishpoint request for replace item failed"), 
					idents[i].name.c_str());
			}
		}
	}
}	// namespace ZQChannelOnDemand