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
// Name  : ChODSvcEnv.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2006-8-21
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ChannelOnDemand/ChODSvcEnv.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 28    10-03-17 17:17 Haoyuan.lu
// remove lock of evictor and add DB_CONFIG
// 
// 27    09-02-06 17:24 Haoyuan.lu
// 
// 26    09-01-04 11:11 Haoyuan.lu
// 
// 25    08-12-08 15:08 Haoyuan.lu
// 
// 24    08-03-24 17:31 Guan.han
// merge from 1.7.5
// 
// 24    08-03-12 11:54 Guan.han
// 
// 23    08-03-05 12:15 Guan.han
// 
// 23    08-03-04 18:53 Guan.han
// 
// 22    08-02-21 16:22 Guan.han
// 
// 21    07-12-24 11:53 Guan.han
// 
// 20    07-12-14 15:38 Xiaohui.chai
// 
// 19    07-07-13 12:43 Jie.zhang
// sync
// 
// 18    07-07-12 20:02 Jie.zhang
// * RW lock seem must not used for dictionary, normal lock is ok,  *the
// evictor must inherited from a lock, or will cause program exit
// unexcpected.
// 
// 17    07-07-11 16:05 Jie.zhang
// sync with main tree
// 
// 18    07-07-11 15:53 Jie.zhang
// add lock to every evictor
// 
// 17    07-07-06 14:48 Jie.zhang
// add a confige: enablechannelmaxdurationcheck and add some logic to
// avoid null playlist handle
// 
// 16    07-06-25 16:02 Jie.zhang
// add a dumpLine function
// 
// 15    07-06-23 11:00 Guan.han
// 
// 14    07-04-28 21:17 Guan.han
// 
// 13    07-04-27 18:13 Guan.han
// 
// 12    07-03-26 14:51 Jie.zhang
// 
// 11    06-12-01 16:28 Jie.zhang
// Log changed and add thread pool to process add/insert/replace/remove
// operation
// 
// 10    06-11-17 19:38 Jie.zhang
// 
// 9     06-11-15 17:34 Jie.zhang
// 
// 8     06-10-24 18:43 Jie.zhang
// 
// 7     06-10-23 10:04 Jie.zhang
// 
// 6     06-09-27 10:34 Jie.zhang
// 
// 5     06-09-26 11:18 Jie.zhang
// 
// 4     06-09-20 14:32 Jie.zhang
// ===========================================================================

#ifndef __CHODSVCENV_H__
#define __CHODSVCENV_H__

#include "ChODDefines.h"
#include "ChannelItemDict.h"
#include "ChannelNameIndex.h"
#include "Channel2Purchase.h"
#include "CtrlNum2ItemAssoc.h"
#include "Purchase2ItemAssoc.h"
#include "PlaylistId2Purchase.h"
#include "ChODFactory.h"
#include "ChannelPublisherImpl.h"
#include "time.h"
#include "Locks.h"
#include "ChannelOnDemandAppImpl.h"
#include "EventChannel.h"
#include "todas.h"
#include "./WatchDog.h"
#include "TianShanDefines.h"

#define		DEFAULT_SESS_CONTAINER_SIZE			20

#define		DEFAULT_CHANNELPUBLISHPOINT_EVITSIZE	60
#define		DEFAULT_PURCHASE_EVITSIZE				200
#define		DEFAULT_PURCHASEITEM_EVITSIZE			1000
#define		DEFAULT_CHANNEL_MAXBITRATE				4000000
#define		DEFAULT_PROTECT_TIME_MS					20000



//////////////////////////////////////////////////////////////////////////
// general member defines for environment class
//////////////////////////////////////////////////////////////////////////
#define	CHANNELITEM_KEY_SEPARATOR	"#"


#define CONTENT_NAME_CASE_SENSITIVE

#define ENDPOINT "endpoint"

#ifdef CONTENT_NAME_CASE_SENSITIVE
#define	STRTOLOWER(_S)
#else
#define	STRTOLOWER(_S)	::std::transform(_S.begin(), _S.end(), _S.begin(), (int(*)(int)) tolower)
#endif

using namespace IceUtil;

// #ifndef USE_OLD_NS
// namespace TianShanIce {
// 	namespace Application {
// #endif //!USE_OLD_NS

namespace ZQChannelOnDemand {

	

//////////////////////////////////////////////////////////////////////////
// Database Names
//////////////////////////////////////////////////////////////////////////

#define ServiceSubDir				    "ChOD"
#define ChannelDataSubDir				"Channels"
#define PurchaseSubDir					"Purchases"
#define PurchaseItemSubDir				"PurchaseItem"
#define ICE_ChannelItemDict				"ChannelItem"
#define ICE_ChannelPublishPoint			"ChannelPublishPoint"
#define ICE_PurchaseItemAssoc			"PurchaseItemAssoc"
#define ICE_ChannelPurchase				"ChannelPurchase"
#define ICE_ChannelPurchaseEx			"ChannelPurchaseEx"


#define INDEXFILENAME(_IDX)				#_IDX "Idx"

#define TestSyncPlaylist 0


//////////////////////////////////////////////////////////////////////////
// class ChODSvcEnv
//////////////////////////////////////////////////////////////////////////

class ChannelPublisherImpl;
class ChannelOnDemandAppImpl;

class ChODSvcEnv
{
public:
    ChODSvcEnv(Ice::CommunicatorPtr& communicator);
	virtual ~ChODSvcEnv();

	bool init();

	void unInit();

public:

	void appendPlaylistItem(const ::std::string& chnlName, const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& appendChnlItem);

	void insertPlaylistItem(const ::std::string& chnlName, const ::std::string& istPosKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& insertChnlItem);

	void removePlaylistItem(const ::std::string& chnlName, const ::std::string& rmvItemKey);

	void removePurchases(const ::std::string& chnlName);

	void replacePlaylistItem(const ::std::string& chnlName, const ::std::string& oldItemKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& replaceChnlItem);

	void OnEndOfStream(const std::string& playlistId);

	void OnStreamExit(const std::string& playlistId);

	void OnEndOfItem(const std::string& playlistId, int userCtrlNum);

	bool ConnectIceStorm();

	IceUtil::AbstractMutexI<IceUtil::RecMutex>	_dictLock;
	::ChannelOnDemand::ChannelItemDict*	_pChannelItemDict;

	// ChannelPublishPoint
//	IceUtil::AbstractMutexI<IceUtil::RecMutex>	_evitCPPLock;
	Freeze::EvictorPtr					_evitChannelPublishPoint;

	// Purcharse
//	IceUtil::AbstractMutexI<IceUtil::RecMutex>	_evitPurLock;
	Freeze::EvictorPtr					_evitPurchase;

	// PurcharseItem
//	IceUtil::AbstractMutexI<IceUtil::RecMutex>	_evitPITLock;
	Freeze::EvictorPtr					_evitPurchaseItemAssoc;

	::ChannelOnDemand::ChannelNameIndexPtr _idxChannelNameIndex;
	::ChannelOnDemand::Channel2PurchasePtr _idxChannel2Purchase;
	::ChannelOnDemand::CtrlNum2ItemAssocPtr _idxCtrlNum2ItemAssoc;
	::ChannelOnDemand::Purchase2ItemAssocPtr _idxPurchase2ItemAssoc;
	::ChannelOnDemand::PlaylistId2PurchasePtr _idxPlaylistId2Purchase;
	
	std::string								_dbPath;
	std::string                             _runtimePath;
	std::string								_endpoint;
	
    Ice::CommunicatorPtr					_communicator;

    ZQADAPTER_DECLTYPE                      _adapter;
	::Ice::ObjectAdapterPtr					_evtAdap;
	Freeze::ConnectionPtr					_connCh;
	
	ChODFactory::Ptr						_factory;
	ChannelPublisherImpl::Ptr				_publisher;
	ChannelOnDemandAppImpl::Ptr				_chOdSvc;
	WatchDog*								_pWatchDog; // session watch dog

	::com::izq::todas::integration::cod::TodasForCodPrx		_todasPrx;

	TianShanIce::Events::EventChannelImpl::Ptr		_eventChannel;

	// native thread pool
	ZQ::common::NativeThreadPool* _pThreadPool;

protected:
	bool					_bInited;
};

void dumpLine(const char* line, void* pCtx = NULL);

class UserCtrlNumGenerator
{
public:
	enum {
		BASE_ADJUST_VALUE = 1160000000
	};

	UserCtrlNumGenerator()
	{
		_nBaseNum = time(0) - BASE_ADJUST_VALUE;		
	}

	int Generate()
	{	
		return InterlockedIncrement(&_nBaseNum);
	}

	void SetBaseValue(int nBase)
	{	
		InterlockedExchange(&_nBaseNum, nBase);
	}

	long GetBaseValue()
	{
		return _nBaseNum;
	}

private:
	volatile long		_nBaseNum;
};

extern UserCtrlNumGenerator	_gUserCtrlNumGen;
	
} // namespace

// #ifndef USE_OLD_NS
// 	}; // namespace TianShanIce
// }; // namespace Application
// #endif //!USE_OLD_NS

#endif