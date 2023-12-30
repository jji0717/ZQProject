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

// Branch: $Name:BcastChRequest.h$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastChRequest.h $
// 
// 6     11/05/14 10:54a Li.huang
// 
// 5     11/03/14 3:29p Li.huang
// 
// 4     10/22/14 10:56a Li.huang
// 
// 3     10/21/14 3:32p Li.huang
// 
// 2     5/30/14 2:41p Li.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     09-07-06 9:48 Li.huang
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#ifndef __ZQBroadcastPublishPoint_H__
#define __ZQBroadcastPublishPoint_H__

#include "NativeThread.h"
#include "NativeThreadPool.h"
#include <Ice/Identity.h>
#include "TsStreamer.h"
#include "TsAppBcast.h"
#include "BcastChannelEx.h"
#include "SystemUtils.h"

#define MAX_IDLE (60*60*1000) // 1hour
#define DEFAULT_IDLE (5* 60*1000) // 5sec

namespace ZQBroadCastChannel
{
#define SyncPlaylistKey "SyncPlaylist"
#define SyncPlaylistValue "true"

	class BroadCastChannelEnv;
	class AppendItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		AppendItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, 
			const TianShanIce::Application::Broadcast::ChannelItemEx& appendChnlItem);
		virtual ~AppendItemRequest();
		
	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		TianShanIce::Application::Broadcast::ChannelItemEx _appendChnlItem;

	}; // class AppendItemRequest
		
	class InsertItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		InsertItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& istPosKey,
			const TianShanIce::Application::Broadcast::ChannelItemEx& insertChnlItem);
		virtual ~InsertItemRequest();
		
	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		::std::string _istPosKey;
		TianShanIce::Application::Broadcast::ChannelItemEx _insertChnlItem;
	}; // class InsertItemRequest
		
	class ReplaceItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		ReplaceItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& oldItemKey, 
			const TianShanIce::Application::Broadcast::ChannelItemEx& replaceChnlItem);
		virtual ~ReplaceItemRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		::std::string _oldItemKey;
		TianShanIce::Application::Broadcast::ChannelItemEx _replaceChnlItem;

	}; // class ReplaceItemRequest

	class RemoveItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		RemoveItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident,
			const ::std::string& rmvItemKey);
		virtual ~RemoveItemRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		::std::string _rmvItemKey;

	}; // class RemoveItemRequest

	class OnEndOfStreamRequest : public ZQ::common::ThreadRequest
	{
	public: 
		OnEndOfStreamRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident,
			const ::std::string& playlistId);
		virtual ~OnEndOfStreamRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		::std::string _playlistId;

	}; // class OnEndOfStreamRequest

	class OnStreamExitRequest : public ZQ::common::ThreadRequest
	{
	public: 
		OnStreamExitRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident,
			const ::std::string& playlistId);
		virtual ~OnStreamExitRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		::std::string _playlistId;

	}; // class OnStreamExitRequest

	class OnEndOfItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		OnEndOfItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident,
			const ::std::string& playlistId, const int userCtrlNum);
		virtual ~OnEndOfItemRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		::std::string _playlistId;
		int _userCtrlNum;

	}; // class OnEndOfItemRequest

	class SyncPlaylistRequest : public ZQ::common::ThreadRequest
	{
	public: 
		SyncPlaylistRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident);
		virtual ~SyncPlaylistRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv& _env;
		::Ice::Identity _ident;
		::std::string _prefixBcastPP;

	}; // class SyncPlaylistRequest

	class ConnectIceStromRequest : public ZQ::common::ThreadRequest
	{
	public: 
		ConnectIceStromRequest(BroadCastChannelEnv& env);
		virtual ~ConnectIceStromRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		BroadCastChannelEnv&		_env;
		SYS::SingleObject            _hStop;
		bool			_bExit;

	}; // class ConnectIceStromRequest

	class ListChannelCmd : protected ZQ::common::ThreadRequest
	{
	public:
		/// constructor
		ListChannelCmd(const ::TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr& amdCB, BroadCastChannelEnv& env, const ::TianShanIce::StrValues& paramNames);
		virtual ~ListChannelCmd() {}
		
	public:
		
		void execute(void) { start(); }
		
	protected: // impls of ThreadRequest
		
		virtual bool init(void);
		virtual int run(void);
		bool  parseIpandPort(TianShanIce::SRM::ResourceMap& resourceRequiremet, std::string& strIp, int& nport);
		
		// no more overwrite-able
		void final(int retcode =0, bool bCancelled =false) { delete this; }
		
	protected:
		BroadCastChannelEnv&		_env;
		::TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr _amdCB;
		::TianShanIce::StrValues _paramNames;
	}; // class ListChannelCmd

	class ListFilterItemsCmd : protected ZQ::common::ThreadRequest
	{
	public:
		/// constructor
		ListFilterItemsCmd(const ::TianShanIce::Application::Broadcast::AMD_BcastPublisher_listFilterItemsPtr& amdCB, BroadCastChannelEnv& env);
		virtual ~ListFilterItemsCmd() {}

	public:

		void execute(void) { start(); }

	protected: // impls of ThreadRequest

		virtual bool init(void);
		virtual int run(void);

		// no more overwrite-able
		void final(int retcode =0, bool bCancelled =false) { delete this; }

	protected:
		BroadCastChannelEnv&		_env;
		::TianShanIce::Application::Broadcast::AMD_BcastPublisher_listFilterItemsPtr _amdCB;
		::TianShanIce::StrValues _paramNames;
	}; // class ListFilterItemsCmd

	// -----------------------------
	// class TimerCmd
	// -----------------------------
	///
	class TimerCmd : public ZQ::common::ThreadRequest
	{
	public:
		/// constructor
		TimerCmd(BroadCastChannelEnv& env, const ::Ice::Identity& bcastChIdent);

	protected:

		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected:

		BroadCastChannelEnv&		_env;
		::Ice::Identity		        _identBcastCh;
	};

	// -----------------------------
	// class TimerCmd
	// -----------------------------
	///
	class GetStatusCmd : public ZQ::common::ThreadRequest
	{
	public:
		/// constructor
		GetStatusCmd(BroadCastChannelEnv& env, const ::Ice::Identity& bcastChIdent);

	protected:

		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected:

		BroadCastChannelEnv&		_env;
		::Ice::Identity		        _identBcastCh;
	};


	// -----------------------------
	// class BcastChannelWatchDog
	// -----------------------------
	class BcastChannelWatchDog : public ZQ::common::ThreadRequest
	{
	public:
		/// constructor
		BcastChannelWatchDog(BroadCastChannelEnv& env);
		virtual ~BcastChannelWatchDog();

		///@param[in] bcastChIdent identity of bcastchannel
		///@param[in] timeout the timeout to wake up timer to check the specified session
		void watchBcastChannel(const ::Ice::Identity& bcastChIdent, long timeout);
		void removeBcastChannel(const ::Ice::Identity& bcastChIdent);

		//quit watching
		void quit();

	protected: // impls of ThreadRequest

		virtual bool init(void);
		virtual int run(void);

		// no more overwrite-able
		void final(int retcode =0, bool bCancelled =false);

		void wakeup();

	protected:

		typedef std::map <Ice::Identity, ::Ice::Long > ExpirationMap; // sessId to expiration map
		ZQ::common::Mutex   _lockExpirations;
		ExpirationMap		_expirations;
		::Ice::Long			_nextWakeup;

		BroadCastChannelEnv& _env;
		bool		  _bQuit;
#ifdef ZQ_OS_MSWIN
		HANDLE		  _hWakeupEvent;
#else
		sem_t		  _wakeupSem;
#endif
	};

	//////////////////////////////////////////////////////////////////////////
	// class NVODSupplementThread
	//////////////////////////////////////////////////////////////////////////

	class NVODSupplementThread: public ZQ::common::NativeThread
	{
	public:
		NVODSupplementThread(BroadCastChannelEnv& env, const ::Ice::Identity& ident, ::Ice::Int interval, bool bIsInit);
		~NVODSupplementThread();
		virtual int run();
		virtual void stop();
	protected:
		SYS::SingleObject            _hStop;
//		HANDLE						 _hStop;
		bool					     _stopped;
		BroadCastChannelEnv&		 _env;
		::Ice::Identity				 _ident;
		::Ice::Int                   _interval;
		bool                         _bIsInit;
	};///class NVODSupplementThread

} // namespace ZQBroadCastChannel

#endif // __ZQBroadcastPublishPoint_H__


