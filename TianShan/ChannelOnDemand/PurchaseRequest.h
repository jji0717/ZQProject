#ifndef __ZQChannelOnDemand_PurchaseThread_H__
#define __ZQChannelOnDemand_PurchaseThread_H__

#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Ice/Identity.h>
#include <TsStreamer.h>
#include "ChODSvcEnv.h"

namespace ZQChannelOnDemand
{
#define SyncPlaylistKey "SyncPlaylist"
#define SyncPlaylistValue "true"

	class AppendItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		AppendItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, 
			const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& appendChnlItem);
		virtual ~AppendItemRequest();
		
	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		ChODSvcEnv& _env;
		::Ice::Identity _ident;
		NS_PREFIX(ChannelOnDemand::ChannelItemEx) _appendChnlItem;

	}; // class AppendItemRequest
		
	class InsertItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		InsertItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& istPosKey, const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& insertChnlItem);
		virtual ~InsertItemRequest();
		
	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		ChODSvcEnv& _env;
		::Ice::Identity _ident;
		::std::string _istPosKey;
		NS_PREFIX(ChannelOnDemand::ChannelItemEx) _insertChnlItem;

	}; // class InsertItemRequest
		
	class ReplaceItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		ReplaceItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& oldItemKey, const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& replaceChnlItem);
		virtual ~ReplaceItemRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		ChODSvcEnv& _env;
		::Ice::Identity _ident;
		::std::string _oldItemKey;
		NS_PREFIX(ChannelOnDemand::ChannelItemEx) _replaceChnlItem;

	}; // class ReplaceItemRequest

	class RemoveItemRequest : public ZQ::common::ThreadRequest
	{
	public: 
		RemoveItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, const ::std::string& rmvItemKey);
		virtual ~RemoveItemRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		ChODSvcEnv& _env;
		::Ice::Identity _ident;
		::std::string _rmvItemKey;

	}; // class RemoveItemRequest

	class RemovePurchaseRequest : public ZQ::common::ThreadRequest
	{
	public: 
		RemovePurchaseRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, const std::string& clientSessionId, const std::string& reason, const bool& bServerSide);
		virtual ~RemovePurchaseRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		ChODSvcEnv& _env;
		::Ice::Identity _ident;
		std::string _clientSessionId;
		std::string _reason;
		bool _bServerSide;

	}; // class RemovePurchaseRequest

	class SyncPlaylistRequest : public ZQ::common::ThreadRequest
	{
	public: 
		SyncPlaylistRequest(ChODSvcEnv& env, const ::Ice::Identity& ident);
		virtual ~SyncPlaylistRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		ChODSvcEnv& _env;
		::Ice::Identity _ident;

	}; // class SyncPlaylistRequest

	class ConnectIceStromRequest : public ZQ::common::ThreadRequest
	{
	public: 
		ConnectIceStromRequest(ChODSvcEnv& env);
		virtual ~ConnectIceStromRequest();

	protected: // impls of ScheduleTask
		virtual bool init();
		virtual int run(void);
		virtual void final(int retcode =0, bool bCancelled =false);

	protected: 
		ChODSvcEnv&		_env;
		HANDLE			_event;
		bool			_bExit;

	}; // class ConnectIceStromRequest

	class ListChannelCmd : protected ZQ::common::ThreadRequest
	{
	public:
		/// constructor
#ifdef USE_OLD_NS
		ListChannelCmd(const ::ChannelOnDemand::AMD_ChannelPublisher_listChannelInfoPtr& amdCB, ChODSvcEnv& _env, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNamesconst);
#else
		ListChannelCmd(const ::TianShanIce::Application::AMD_OnDemandPublisher_listOnDemandPointInfoPtr& amdCB, ChODSvcEnv& _env, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNamesconst);
#endif //USE_OLD_NS
		virtual ~ListChannelCmd() {}
		
	public:
		
		void execute(void) { start(); }
		
	protected: // impls of ThreadRequest
		
		virtual bool init(void);
		virtual int run(void);
		
		// no more overwrite-able
		void final(int retcode =0, bool bCancelled =false) { delete this; }
		
	protected:
		ChODSvcEnv&		_env;
#ifdef USE_OLD_NS
		::ChannelOnDemand::AMD_ChannelPublisher_listChannelInfoPtr _amdCB;
#else
		::TianShanIce::Application::AMD_OnDemandPublisher_listOnDemandPointInfoPtr _amdCB;
#endif // USE_OLD_NS
		::TianShanIce::StrValues _paramNames;
		std::string _onDemandName;
	}; // class ListProvisionRequest


} // namespace ZQChannelOnDemand

#endif // __ZQChannelOnDemand_PurchaseThread_H__


