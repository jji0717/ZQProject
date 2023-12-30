#include "PurchaseRequest.h"

#define AppendItemMacro "AppendItemR"
#define InsertItemMacro "InsertItemR"
#define ReplaceItemMacro "ReplaceItemR"
#define RemoveItemMacro "RemoveItemR"
#define RemovePurchaseMacro "RemovePurchaseR"
#ifdef USE_OLD_NS
#define IdentityToObj(_CLASS, _ID) ::ChannelOnDemand::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#else
#define IdentityToObj(_CLASS, _ID) ::TianShanIce::Application::ChannelOnDemand::##_CLASS##Prx::uncheckedCast(_env._adapter->createProxy(_ID))
#endif //USE_OLD_NS

namespace ZQChannelOnDemand
{
	AppendItemRequest::AppendItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, 
			const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& appendChnlItem)
		: _env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), _ident(ident), _appendChnlItem(appendChnlItem)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AppendItemMacro, "[%s] purchase request append item [%s]"), 
			_ident.name.c_str(), appendChnlItem.key.c_str());
	}

	AppendItemRequest::~AppendItemRequest()
	{
	}

	bool AppendItemRequest::init()
	{
		return true;
	}

	int AppendItemRequest::run(void)
	{
		try
		{
			NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx) purPrx = NULL;
			purPrx = NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx)::checkedCast(_env._adapter->createProxy(_ident));
			purPrx->appendPlaylistItem(_appendChnlItem);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->appendPlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->appendPlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->appendPlaylistItem() caught unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void AppendItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	InsertItemRequest::InsertItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& istPosKey, const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& insertChnlItem)
		: _env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), _ident(ident), _istPosKey(istPosKey), _insertChnlItem(insertChnlItem)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(InsertItemMacro, "[%s] purchase request insert item [%s] before [%s]"), 
			_ident.name.c_str(), _insertChnlItem.key.c_str(), _istPosKey.c_str());
	}

	InsertItemRequest::~InsertItemRequest()
	{
	}

	bool InsertItemRequest::init()
	{
		return true;
	}

	int InsertItemRequest::run(void)
	{
		try
		{
			NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx) purPrx = NULL;
			purPrx = NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx)::checkedCast(_env._adapter->createProxy(_ident));
			purPrx->insertPlaylistItem(_istPosKey, _insertChnlItem);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->insertPlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->insertPlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->insertPlaylistItem() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void InsertItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	ReplaceItemRequest::ReplaceItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& oldItemKey, const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& replaceChnlItem)
		: _env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), _ident(ident), _oldItemKey(oldItemKey), _replaceChnlItem(replaceChnlItem)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ReplaceItemMacro, "[%s] purchase request replace item [%s] with [%s]"), 
			_ident.name.c_str(), _oldItemKey.c_str(), _replaceChnlItem.key.c_str());
	}

	ReplaceItemRequest::~ReplaceItemRequest()
	{
	}

	bool ReplaceItemRequest::init()
	{
		return true;
	}

	int ReplaceItemRequest::run(void)
	{
		try
		{
			NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx) purPrx = NULL;
			purPrx = NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx)::checkedCast(_env._adapter->createProxy(_ident));
			purPrx->replacePlaylistItem(_oldItemKey, _replaceChnlItem);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->replacePlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->replacePlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->replacePlaylistItem() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void ReplaceItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	RemoveItemRequest::RemoveItemRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, const ::std::string& rmvItemKey)
		: _env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), _ident(ident), _rmvItemKey(rmvItemKey)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RemoveItemMacro, "[%s] purchase request remove item [%s]"), 
			_ident.name.c_str(), _rmvItemKey.c_str());
	}

	RemoveItemRequest::~RemoveItemRequest()
	{
	}

	bool RemoveItemRequest::init()
	{
		return true;
	}

	int RemoveItemRequest::run(void)
	{
		try
		{
			NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx) purPrx = NULL;
			purPrx = NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx)::checkedCast(_env._adapter->createProxy(_ident));
			purPrx->removePlaylistItem(_rmvItemKey);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->removePlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->removePlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->removePlaylistItem() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void RemoveItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	RemovePurchaseRequest::RemovePurchaseRequest(ChODSvcEnv& env, const ::Ice::Identity& ident, 
		const std::string& clientSessionId, const std::string& reason, const bool& bServerSide)
		: _env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), _ident(ident), 
		_clientSessionId(clientSessionId), _reason(reason), _bServerSide(bServerSide)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RemovePurchaseMacro, "[%s] purchase request remove itselt, because of [%s]"), 
			_ident.name.c_str(), _reason.c_str());
	}

	RemovePurchaseRequest::~RemovePurchaseRequest()
	{
	}

	bool RemovePurchaseRequest::init()
	{
		return true;
	}

	int RemovePurchaseRequest::run(void)
	{
		try
		{
			NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx) purPrx = NULL;
			purPrx = NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx)::checkedCast(_env._adapter->createProxy(_ident));
			if(purPrx->isInService())
				_reason = "220010 " + _reason;
			else
				_reason = "220020 idle " + _reason;

			::TianShanIce::Properties props;
			if (_bServerSide)
				props[SYS_PROP(terminateReason)] = _reason;
			else 
				props[SYS_PROP(teardownReason)] = _reason;
			purPrx->detach(_clientSessionId, props);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->detach() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->detach() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->detach() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void RemovePurchaseRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	SyncPlaylistRequest::SyncPlaylistRequest(ChODSvcEnv& env, const ::Ice::Identity& ident)
		: _env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), _ident(ident)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RemovePurchaseMacro, "[%s] purchase request sync playlist"), 
			_ident.name.c_str());
	}

	SyncPlaylistRequest::~SyncPlaylistRequest()
	{
	}

	bool SyncPlaylistRequest::init()
	{
		return true;
	}

	int SyncPlaylistRequest::run(void)
	{
		try
		{
			// notice here, I add a key-value to ice::current
			::Ice::Context ctx;
			ctx[SYS_PROP(SyncPlaylistKey)] = SyncPlaylistValue;
			NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx) purPrx = NULL;
			purPrx = NS_PREFIX(ChannelOnDemand::ChannelPurchaseExPrx)::checkedCast(_env._adapter->createProxy(_ident));
			purPrx->syncPlaylist(ctx);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->syncPlaylist() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->syncPlaylist() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemMacro, "[%s] purchase->syncPlaylist() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void SyncPlaylistRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	ConnectIceStromRequest::ConnectIceStromRequest(ChODSvcEnv& env)
		: _env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), 
		_event(NULL), _bExit(false)
	{
	}

	ConnectIceStromRequest::~ConnectIceStromRequest()
	{
	}

	bool ConnectIceStromRequest::init()
	{
		_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		return true;
	}

	int ConnectIceStromRequest::run(void)
	{
		while (!_bExit)
		{
			if (_env.ConnectIceStorm())
				_bExit = true;
			else 
				::WaitForSingleObject(_event, 2000);
		}

		return 0;
	}

	void ConnectIceStromRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	// -----------------------------
	// class ListChannelCmd
	// -----------------------------
#ifdef USE_OLD_NS
	ListChannelCmd::ListChannelCmd(const ::ChannelOnDemand::AMD_ChannelPublisher_listChannelInfoPtr& amdCB, ChODSvcEnv& env, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNames)
#else
	ListChannelCmd::ListChannelCmd(const ::TianShanIce::Application::AMD_OnDemandPublisher_listOnDemandPointInfoPtr& amdCB, ChODSvcEnv& env, const ::std::string& onDemandName, const ::TianShanIce::StrValues& paramNames)
#endif
		:_env(env), ZQ::common::ThreadRequest(*(env._pThreadPool)), _amdCB(amdCB), _onDemandName(onDemandName), _paramNames(paramNames)
	{
	}
	
	bool ListChannelCmd::init(void)
	{
		return (NULL != _amdCB);
	}
	
	int ListChannelCmd::run(void)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "list channel informations"));
		std::string lastError;
		IdentCollection Idents;
		if(_onDemandName.size()==0 || _onDemandName.compare("*") ==0)
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "OnDemandName: <%s>--list all channels"), _onDemandName.c_str());
			//list all channel point
			try	{
				::Freeze::EvictorIteratorPtr itptr = _env._evitChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
				while (itptr && itptr->hasNext())
					Idents.push_back(itptr->next());
			}
			catch (const ::Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "caught exception[%s] when enumerate channels"), ex.ice_name().c_str());
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "caught unknown exception when enumerate channels"));
			}
		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "OnDemandName: <%s>"), _onDemandName.c_str());
			try	{
				::Freeze::EvictorIteratorPtr its;
				its = _env._evitChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
				while(its->hasNext())
				{
					::Ice::Identity ident = its->next();
					NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) pointPrx = IdentityToObj(ChannelPublishPoint, ident);
					::std::string tmpName = pointPrx->getOnDemandName();
					if(tmpName.compare(_onDemandName) == 0)
						Idents.push_back(ident);
				}
			}
			catch (const ::Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "caught exception[%s] when enumerate channels"), ex.ice_name().c_str());
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "caught unknown exception when enumerate channels"));
			}			
		}

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "found %d matched transactions"), Idents.size());
		
#ifdef USE_OLD_NS
		::ChannelOnDemand::ChannelInfos results;
#else
		::TianShanIce::Application::PublishPointInfos results;
#endif // USE_OLD_NS
		
		try {			
			
			for (IdentCollection::iterator it= Idents.begin(); it != Idents.end(); it++)
			{
				
				try {
					NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) pointPrx = IdentityToObj(ChannelPublishPoint, *it);
					::std::string OnDemandName;

#ifdef USE_OLD_NS
					::ChannelOnDemand::ChannelInfo channelInfo;
#else
					::TianShanIce::Application::PublishPointInfo channelInfo;
#endif // USE_OLD_NS

					channelInfo.name = pointPrx->getName();

#ifdef USE_OLD_NS
					channelInfo.onDemandName = pointPrx->getOnDemandName();
					OnDemandName = channelInfo.onDemandName;
#else
					channelInfo.type = pointPrx->getOnDemandName();
					OnDemandName = channelInfo.type;
#endif //USE_OLD_NS
					
					for (::TianShanIce::StrValues::iterator pit= _paramNames.begin(); pit < _paramNames.end(); pit++)
					{
						if(pit->compare("desc") == 0)
						{
							channelInfo.params.insert(::TianShanIce::Properties::value_type("desc", pointPrx->getDesc()));
						}
						else if(pit->compare("maxBitrate") == 0)
						{
							char buf[20];
							itoa(pointPrx->getMaxBitrate(), buf, 10);
							std::string strMaxBitrate = buf;
							channelInfo.params.insert(::TianShanIce::Properties::value_type("maxBitrate", strMaxBitrate));
						}
					}
					
					results.push_back(channelInfo);
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "Results push_back(Name<%s>, OnDemandName<%s>)"), channelInfo.name.c_str(), OnDemandName.c_str());
					std::map<std::string, std::string>::const_iterator iter = channelInfo.params.begin();
					for (; iter != channelInfo.params.end(); ++iter) 
					{
						glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "<%s>--<%s>"), (iter->first).c_str(), (iter->second).c_str());
					}
				}
				catch (...) {}
			}
			
			_amdCB->ice_response(results);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "ice_response<%d>"), results.size());
		}
		catch(const ::Ice::Exception& ex)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "ListChannelCmd caught exception[%s]", ex.ice_name().c_str());
			lastError = buf;
			_amdCB->ice_exception(::TianShanIce::ServerError("COD", 501, lastError));
		}
		catch(...)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "ListChannelCmd caught unknown exception");
			lastError = buf;
			_amdCB->ice_exception(::TianShanIce::ServerError("COD", 502, lastError));
		}
						
		return 0;
	}

} // namespace ZQChannelOnDemand

