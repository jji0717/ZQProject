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
// Name  : BroadCastChannleEnv.h
// Author: li. Huang
// Date  : 
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BroadCastChannelEnv.h $
// 
// 11    2/26/16 5:00p Li.huang
// fix bug 22376
// 
// 10    12/28/15 1:58p Hui.shao
// 
// 8     7/27/15 2:15p Li.huang
// 
// 7     7/07/15 4:37p Li.huang
// fix bug 21518
// 
// 6     11/14/14 9:42a Li.huang
// 
// 5     11/03/14 3:29p Li.huang
// 
// 4     10/21/14 3:32p Li.huang
// 
// 3     5/30/14 4:43p Li.huang
// 
// 2     5/30/14 2:41p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     10-03-19 11:46 Li.huang
// remove evictor lock
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#ifndef __BroadCastChannelEnv_h__
#define __BroadCastChannelEnv_h__

#include "Locks.h"
#include "BcastChDef.h"
#include "ChannelItemDict.h"
#include "PlaylistId2BcastPublishPoint.h"
#include "CtrlNum2ChannelItemAssoc.h"
#include "BcastPublishPoint2ItemAssoc.h"
#include "mainChName2BcastPublishPoint.h"
#include "ChItem2ItemAssoc.h"
#include "BcastChFactory.h"
#include "BcastPublisherImpl.h"
#include "BcastAppServiceImpl.h"
#include "TianShanDefines.h"
#include "EventChannel.h"
#include "BcastChRequest.h"
#include "SystemUtils.h"
#include "BcastChRequest.h"

#include "BcastMRTProxy.h"

#define		DEFAULT_SESS_CONTAINER_SIZE			20

#define		DEFAULT_CHANNELPUBLISHPOINT_EVITSIZE	60
#define		DEFAULT_CHANNEL_MAXBITRATE				4000000
#define		DEFAULT_PROTECT_TIME_MS					20000

//////////////////////////////////////////////////////////////////////////
// general member defines for environment class
//////////////////////////////////////////////////////////////////////////
#define	CHANNELITEM_KEY_SEPARATOR	"#"


#define CONTENT_NAME_CASE_SENSITIVE

#define NVODMAINCHNAME      "novdmain_chname"
#define NVODSUPPLEMENTAL    "nvod_supplemental"

#define ENDPOINT "endpoint"

#ifdef CONTENT_NAME_CASE_SENSITIVE
#define	STRTOLOWER(_S)
#else
#define	STRTOLOWER(_S)	::std::transform(_S.begin(), _S.end(), _S.begin(), (int(*)(int)) tolower)
#endif

using namespace IceUtil;

namespace ZQBroadCastChannel
{ 
#define ServiceSubDir				    "BroadCastChannel"
#define BcastChannelDataSubDir			"BcastChannels"
#define NVODChannelDataSubDir			"NVODChannels"
#define PurchaseSubDir					"Purchases"
#define FilterSubDir					"FilterItems"
#define ICE_BcastChannelItemDict		"BcastChannelItem"
#define ICE_BcastChannelPublishPoint	"BcastChannelPublishPoint"
#define ICE_NVODChannelPublishPoint	    "NVODChannelPublishPoint"
#define ICE_BcastChannelPurchase		"BcastChannelPurchase"
#define ICE_FilterItems		            "FilterItem"
#define ICE_ChannelItemAssoc	        "ChannelItemAssoc"
#define INDEXFILENAME(_IDX)				#_IDX "Idx"

#define TestSyncPlaylist 0

class NVODSupplementThread;
class BcastPublisherImpl;
class BcastAppServiceImpl;
class FilterItemsImpl;

typedef std::map<Ice::Identity, NVODSupplementThread*> NVODSupplementManager;

class BroadCastChannelEnv
{
public:
	BroadCastChannelEnv(Ice::CommunicatorPtr& communicator);
	virtual ~BroadCastChannelEnv();

	bool init();

	void unInit();

public:
	void OnEndOfStream(const std::string& playlistId);

	void OnStreamExit(const std::string& playlistId);

	void OnEndOfItem(const std::string& playlistId, int userCtrlNum);

	void appendPlaylistItem(const ::std::string& chnlName, const TianShanIce::Application::Broadcast::ChannelItemEx & appendChnlItem);

	void insertPlaylistItem(const ::std::string& chnlName, const ::std::string& istPosKey, 
		const TianShanIce::Application::Broadcast::ChannelItemEx & insertChnlItem);

	void removePlaylistItem(const ::std::string& chnlName, const ::std::string& rmvItemKey);

	void replacePlaylistItem(const ::std::string& chnlName, const ::std::string& oldItemKey, 
		const TianShanIce::Application::Broadcast::ChannelItemEx & replaceChnlItem);

	bool connectEventChannel();

	bool connectWeiwoo();

	bool addNVODSupplMgr(const ::Ice::Identity& ident, ::Ice::Int interval, bool bIsInit);
	bool removeNVODSupplMgr(const ::Ice::Identity& ident);

public:
	IceUtil::AbstractMutexI<IceUtil::RecMutex>	_dictLock;
	ZQBroadCastChannel::ChannelItemDict*	_pChannelItemDict;
 
	// BcastChannelPublishPoint
	Freeze::EvictorPtr					_evitBcastChannelPublishPoint;

	//NVODChannelPublishPoint
	Freeze::EvictorPtr					_evitNOVDChannelPublishPoint;

	// Purcharse
	Freeze::EvictorPtr					_evitPurchase;

	// filter items
	Freeze::EvictorPtr					_evitFilterItems;

	// ChannelItemAssoc
	Freeze::EvictorPtr					_evitChannelItemAssoc;

	ZQBroadCastChannel::PlaylistId2BcastPublishPointPtr  _idxPlaylistId2BcastPublishPoint;
	ZQBroadCastChannel::CtrlNum2ChannelItemAssocPtr      _idxCtrlNumber2ChannelItemAssoc;
	ZQBroadCastChannel::BcastPublishPoint2ItemAssocPtr   _idxBcastPublishPoint2ItemAssoc;
    ZQBroadCastChannel::ChItem2ItemAssocPtr              _idxChItem2ItemAssoc;
	ZQBroadCastChannel::mainChName2BcastPublishPointPtr  _idxmainChName2BcastPublishPoint;

	std::string								_dbPath;
	std::string                             _runtimePath;
	std::string								_endpoint;

	Ice::CommunicatorPtr					_communicator;

	ZQADAPTER_DECLTYPE                      _adapter;
	::Ice::ObjectAdapterPtr					_evtAdap;
	Freeze::ConnectionPtr					_connCh;

	BcastChFactory::Ptr						_factory;
	BcastPublisherImpl::Ptr	                _publisher;
	BcastAppServiceImpl::Ptr	            _BcastAppSvc;
	TianShanIce::Application::Broadcast::FilterItemsPrx   _FilterItems;

	TianShanIce::SRM::SessionManagerPrx     _sessManager;

    
	TianShanIce::Events::EventChannelImpl::Ptr		_eventChannel;
	// native thread pool
	ZQ::common::NativeThreadPool _threadPool;

	NVODSupplementManager                     _nvodSupplementManager;

	BcastChannelWatchDog		_watchDog;
	int                         _sessionRewTime;

	TianShanIce::Streamer::StreamServicePtr _service;
	Ice::ObjectAdapterPtr  _mrtAdapter;
	ReplicaUpdater*        _replicaUpdater;

protected:
	bool					_bInited;
};
#define IdentityToObj(_CLASS, _ID) TianShanIce::Application::Broadcast::_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#define IdentityToObj2(_CLASS, _ID) TianShanIce::Application::Broadcast::_CLASS##Prx::checkedCast(_env._adapter->createProxy(_ID))

class UserCtrlNumGenerator
{
public:
#ifdef ZQ_OS_LINUX
	ZQ::common::Mutex _lockBaseNum;
#endif

	enum {
		BASE_ADJUST_VALUE = 1160000000
	};

	UserCtrlNumGenerator()
	{
		_nBaseNum = time(0) - BASE_ADJUST_VALUE;		
	}

	int Generate()
	{

#ifdef ZQ_OS_MSWIN
		return InterlockedIncrement(&_nBaseNum);
#else
		ZQ::common::MutexGuard gd(_lockBaseNum);
		_nBaseNum++;
		return _nBaseNum;
#endif	
	}

	void SetBaseValue(int nBase)
	{	
#ifdef ZQ_OS_MSWIN
		InterlockedExchange(&_nBaseNum, nBase);
#else
		ZQ::common::MutexGuard gd(_lockBaseNum);
		nBase = _nBaseNum;
#endif	
	}

	long GetBaseValue()
	{
		return _nBaseNum;
	}

private:
	volatile long		_nBaseNum;
};

extern UserCtrlNumGenerator	_gUserCtrlNumGen;

void dumpLine(const char* line, void* pCtx = NULL);

}
#endif ///end define __BroadCastChannelEnv_h__ 