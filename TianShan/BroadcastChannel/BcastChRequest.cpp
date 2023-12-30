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

// Branch: $Name:BcastChRequest.cpp$
// Author: Huang Li
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/BroadcastChannel/BcastChRequest.cpp $
// 
// 16    2/26/16 5:00p Li.huang
// fix bug 22376
// 
// 15    12/28/15 1:52p Hui.shao
// 
// 15    12/28/15 1:50p Hui.shao
// 
// 14    11/21/14 10:35a Li.huang
// 
// 13    11/20/14 10:09a Li.huang
// 
// 12    11/19/14 10:01a Li.huang
// 
// 11    11/18/14 2:37p Li.huang
// 
// 10    11/18/14 11:06a Li.huang
// 
// 9     11/14/14 9:42a Li.huang
// 
// 8     11/05/14 10:54a Li.huang
// 
// 7     11/03/14 3:29p Li.huang
// 
// 6     10/22/14 10:56a Li.huang
// 
// 5     10/21/14 3:32p Li.huang
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
// 4     10-03-19 11:46 Li.huang
// remove evictor lock
// 
// 3     09-07-06 9:48 Li.huang
// 
// 2     09-06-15 15:39 Li.huang
// 
// 1     09-05-11 13:43 Li.huang
// ===========================================================================
#include "BcastChRequest.h"
#include "BcastChannelEx.h"
#include "BroadcastChCfg.h"
#include "TianShanDefines.h"
#include "BroadCastChannelEnv.h"
#include "soapsoapMRTProxy.h"

extern ZQ::common::Config::Loader<BroadcastChCfg> gBroadcastChCfg;

namespace ZQBroadCastChannel
{
	AppendItemRequest::AppendItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, 
			const TianShanIce::Application::Broadcast::ChannelItemEx& appendChnlItem)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident), _appendChnlItem(appendChnlItem)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(AppendItemRequest, "[%s]BroadcastPublishPoint request append item [%s]"), 
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
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->appendPlaylistItem(_appendChnlItem);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemRequest, "[%s] BroadcastPublishPoint->appendPlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemRequest, "[%s] BroadcastPublishPoint->appendPlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(AppendItemRequest, "[%s] BroadcastPublishPoint->appendPlaylistItem() caught unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void AppendItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	InsertItemRequest::InsertItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& istPosKey, const TianShanIce::Application::Broadcast::ChannelItemEx& insertChnlItem)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident), _istPosKey(istPosKey), _insertChnlItem(insertChnlItem)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(InsertItemRequest, "[%s] BroadcastPublishPoint request insert item [%s] before [%s]"), 
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
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->insertPlaylistItem(_istPosKey, _insertChnlItem);			
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(InsertItemRequest, "[%s] BroadcastPublishPoint->insertPlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(InsertItemRequest, "[%s] BroadcastPublishPoint->insertPlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(InsertItemRequest, "[%s] BroadcastPublishPoint->insertPlaylistItem() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void InsertItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}
	///end of implement InsertItemRequest

    ///implement ReplaceItemRequest
	ReplaceItemRequest::ReplaceItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, 
			const ::std::string& oldItemKey, const TianShanIce::Application::Broadcast::ChannelItemEx& replaceChnlItem)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident), _oldItemKey(oldItemKey),
		_replaceChnlItem(replaceChnlItem)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ReplaceItemRequest, "[%s] BroadcastPublishPoint request replace item [%s] with [%s]"), 
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
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->replacePlaylistItem(_oldItemKey, _replaceChnlItem);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ReplaceItemRequest, "[%s] BroadcastPublishPoint->replacePlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ReplaceItemRequest, "[%s] BroadcastPublishPoint->replacePlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ReplaceItemRequest, "[%s] BroadcastPublishPoint->replacePlaylistItem() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void ReplaceItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}
    //// end of implement OnEndOfStreamRequest

	//// implement RemoveItemRequest
	RemoveItemRequest::RemoveItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, const ::std::string& rmvItemKey)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident), _rmvItemKey(rmvItemKey)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(RemoveItemRequest, "[%s] BroadcastPublishPoint request remove item [%s]"), 
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
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->removePlaylistItem(_rmvItemKey);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(RemoveItemRequest, "[%s] BroadcastPublishPoint->removePlaylistItem() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(RemoveItemRequest, "[%s] BroadcastPublishPoint->removePlaylistItem() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(RemoveItemRequest, "[%s] BroadcastPublishPoint->removePlaylistItem() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void RemoveItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}
   ///end of implement RemoveItemRequest


	///implement OnEndOfStreamRequest
	OnEndOfStreamRequest::OnEndOfStreamRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, 
		const ::std::string& playlistId)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident), _playlistId(playlistId)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(OnEndOfStreamRequest, "[%s] on end of stream request playlistId[%s]"), 
			_ident.name.c_str(), playlistId.c_str());
	}

	OnEndOfStreamRequest::~OnEndOfStreamRequest()
	{
	}

	bool OnEndOfStreamRequest::init()
	{
		return true;
	}

	int OnEndOfStreamRequest::run(void)
	{
		try
		{
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->OnEndOfStream(_playlistId);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnEndOfStreamRequest, "[%s] on end of stream caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnEndOfStreamRequest, "[%s] on end of stream caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnEndOfStreamRequest, "[%s] on end of stream unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void OnEndOfStreamRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}
	//// end of implement OnEndOfStreamRequest

    ///implement OnStreamExitRequest
	OnStreamExitRequest::OnStreamExitRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, const ::std::string& playlistId)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident), _playlistId(playlistId)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(OnStreamExitRequest, "[%s] on stream exit[%s]"), 
			_ident.name.c_str(), _playlistId.c_str());
	}

	OnStreamExitRequest::~OnStreamExitRequest()
	{
	}

	bool OnStreamExitRequest::init()
	{
		return true;
	}

	int OnStreamExitRequest::run(void)
	{
		try
		{
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->OnStreamExit(_playlistId);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamExitRequest, "[%s] on stream exit caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamExitRequest, "[%s] on stream exit caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnStreamExitRequest, "[%s] on stream exit unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void OnStreamExitRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}
	///end of implement OnStreamExitRequest

	///implement OnEndOfItemRequest
	OnEndOfItemRequest::OnEndOfItemRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident, const ::std::string& playlistId, const int userCtrlNum)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident), _playlistId(playlistId), _userCtrlNum(userCtrlNum)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(OnEndOfItemRequest, "[%s] On End Of Item[%s]userCtrlNum[%d]"), 
			_ident.name.c_str(), _playlistId.c_str(), _userCtrlNum);
	}

	OnEndOfItemRequest::~OnEndOfItemRequest()
	{
	}

	bool OnEndOfItemRequest::init()
	{
		return true;
	}

	int OnEndOfItemRequest::run(void)
	{
		try
		{
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->OnEndOfItem(_userCtrlNum);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnEndOfItemRequest, "[%s] On End Of Item caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnEndOfItemRequest, "[%s] On End Of Item exit caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnEndOfItemRequest, "[%s] On End Of Item unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void OnEndOfItemRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}
	///end of implement OnStreamExitRequest

	SyncPlaylistRequest::SyncPlaylistRequest(BroadCastChannelEnv& env, const ::Ice::Identity& ident)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), _ident(ident)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(SyncPlaylistRequest, "[%s] purchase request sync playlist"), 
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
			TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
			bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
			bcppPrx->syncPlaylist(ctx);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(SyncPlaylistRequest, "[%s] syncPlaylist() caught %s: %s"), 
				_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(SyncPlaylistRequest, "[%s] syncPlaylist() caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(SyncPlaylistRequest, "[%s] syncPlaylist() unexpect exception"), 
				_ident.name.c_str());
		}

		return 0;
	}

	void SyncPlaylistRequest::final(int retcode, bool bCancelled)
	{
		delete this;
	}

	ConnectIceStromRequest::ConnectIceStromRequest(BroadCastChannelEnv& env)
		: _env(env), ZQ::common::ThreadRequest(env._threadPool), 
		 _bExit(false)
	{
	}

	ConnectIceStromRequest::~ConnectIceStromRequest()
	{
	}

	bool ConnectIceStromRequest::init()
	{
		return true;
	}

	int ConnectIceStromRequest::run(void)
	{
		while (!_bExit)
		{
			if (_env.connectEventChannel())
				_bExit = true;
			else 
				_hStop.wait(2000);
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
	ListChannelCmd::ListChannelCmd(const TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr& amdCB, BroadCastChannelEnv& env, const TianShanIce::StrValues& paramNames)
		:_env(env), ZQ::common::ThreadRequest(env._threadPool), _amdCB(amdCB), _paramNames(paramNames)
	{
	}
	
	bool ListChannelCmd::init(void)
	{
		return (NULL != _amdCB);
	}
	bool  ListChannelCmd::parseIpandPort(TianShanIce::SRM::ResourceMap& resourceRequiremet, std::string& strIp, int& nport)
	{
		TianShanIce::SRM::ResourceMap::const_iterator resItor;
		resItor = resourceRequiremet.find(TianShanIce::SRM::rtEthernetInterface);
		if(resItor == resourceRequiremet.end())
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT("ListChannelCmd",
				"miss resource type='rtEthernetInterface'"));
			return false;
		}
		TianShanIce::SRM::Resource resource = resItor->second;
		TianShanIce::ValueMap resourceData = resource.resourceData;
		TianShanIce::ValueMap::iterator vmItor = resourceData.find("destPort");
		if(vmItor == resourceData.end())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "missed destport infomation"));
			return false;
		}
		else
		{
			TianShanIce::Variant vardestport = vmItor->second;

			if(vardestport.type == TianShanIce::vtInts && vardestport.ints.size() > 0)
			{
				nport = vardestport.ints[0];
			}
			else
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "missed destport infomation"));
				return false;
			}
		}

		vmItor = resourceData.find("destIP");
		if(vmItor == resourceData.end())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "missed destIP infomation"));
			return false;
		}
		else
		{
			TianShanIce::Variant vardestIp = vmItor->second;

			if(vardestIp.type == TianShanIce::vtStrings && vardestIp.strs.size() > 0)
			{
				strIp = vardestIp.strs[0];
			}
			else
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", "missed destIP infomation"));
				return false;
			}
		}
		return true;
	}

	int ListChannelCmd::run(void)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "list channel informations"));
		std::string lastError;
		ZQTianShan::IdentCollection Idents;

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "--list all channels"));
		//list all BroadCastChannel PublishPoint
		try	{
			Freeze::EvictorIteratorPtr itptr = _env._evitBcastChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
			while (itptr && itptr->hasNext())
			{
				Ice::Identity ident = itptr->next();
				try
				{	
					TianShanIce::Application::Broadcast::BcastPublishPointExPrx pointPrx = IdentityToObj(BcastPublishPointEx, ident);

					bool bIsNVODMainChannel = pointPrx->isNOVDMainCh();
					bool bIsNOVDSuppCh =  pointPrx->isNOVDSuppCh();

					if(bIsNVODMainChannel || bIsNOVDSuppCh)//not NVODMainChannel and NVODSupplementChannel
						continue;
					else
					{
						Idents.push_back(ident);
					}
				}
				catch (Ice::Exception&ex)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT("ListChannelCmd", 
						"caught exception[%s] when enumerate channels"), ex.ice_name().c_str());
				}				
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

		//list all NVODChannel PublishPoint
		try	{
			::Freeze::EvictorIteratorPtr itptr = _env._evitNOVDChannelPublishPoint->getIterator("", MAX_BATCH_ITERATOR_SIZE);
			while (itptr && itptr->hasNext())
			{
				Idents.push_back(itptr->next());
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
			
		TianShanIce::Application::PublishPointInfos results;
		try {			

			for (ZQTianShan::IdentCollection::iterator it= Idents.begin(); it != Idents.end(); it++)
			{

				try {
					TianShanIce::Application::Broadcast::BcastPublishPointPrx pointPrx= IdentityToObj(BcastPublishPoint, *it);

					TianShanIce::Application::PublishPointInfo channelInfo;
					TianShanIce::SRM::ResourceMap resource;

					std::string  supplementCount;
					std::string  Interval;
					std::string  upTime;

					channelInfo.name = pointPrx->getName();
					channelInfo.type = pointPrx->getType();
					resource = pointPrx->getResourceRequirement();

					char UpTimeTemp[128]="";
					Ice::Long    lUptime = pointPrx->getUpTime();
					sprintf(UpTimeTemp, "%llu", lUptime);
//					_ui64toa(lUptime, UpTimeTemp, 10);
					upTime = UpTimeTemp;
					channelInfo.params.insert(TianShanIce::Properties::value_type(RESKEY_UpTimpe, upTime));

					for (TianShanIce::StrValues::iterator pit= _paramNames.begin(); pit < _paramNames.end(); pit++)
					{
						if(pit->compare("desc") == 0)
						{
							channelInfo.params.insert(TianShanIce::Properties::value_type("desc", pointPrx->getDesc()));
						}
						else if(pit->compare("maxBitrate") == 0)
						{
							char buf[20];
							itoa(pointPrx->getMaxBitrate(), buf, 10);
							std::string strMaxBitrate = buf;
							channelInfo.params.insert(TianShanIce::Properties::value_type("maxBitrate", strMaxBitrate));
						}
					}
					
					std::string strIp, strIpPort;
					int nport;
					bool bParse = parseIpandPort(resource,strIp, nport);
					if(channelInfo.type == BcastChannel_Type)
					{
						supplementCount = "0";
						Interval = "0";
						channelInfo.params.insert(TianShanIce::Properties::value_type(RESKEY_Interval, Interval));
						channelInfo.params.insert(TianShanIce::Properties::value_type(RESKEY_Iterator, supplementCount));
						if(bParse)
						{
							char strTemp[10] ="";
							itoa(nport,strTemp , 10);
							strIpPort = strIp + ":" + strTemp;
							channelInfo.params.insert(TianShanIce::Properties::value_type(RESKEY_IpPort, strIpPort));
						}
					}
					else
						if(channelInfo.type == NVODChannel_Type)
						{
							try
							{
								TianShanIce::Application::Broadcast::NVODChannelPublishPointExPrx nvodEx;
								nvodEx = TianShanIce::Application::Broadcast::NVODChannelPublishPointExPrx::checkedCast(pointPrx);
								Ice::Short iteration = nvodEx->getIteration();
								Ice::Int  interval = nvodEx->getInterval();
								char temp[65]="";
								itoa(iteration, temp, 10);
								supplementCount = temp;
								memset(temp, 0, 65);
								itoa(interval, temp, 10);
								Interval = temp;
								channelInfo.params.insert(TianShanIce::Properties::value_type(RESKEY_Interval, Interval));
								channelInfo.params.insert(TianShanIce::Properties::value_type(RESKEY_Iterator, supplementCount));
								if( bParse && iteration > 0)
								{
									char strTemp[32] ="";
									itoa(nport,strTemp , 10);
									std::string strPort = strTemp;
                                    memset(strTemp, 0, 32);
                                    itoa(gBroadcastChCfg.portIncreaseBase * iteration + nport,strTemp , 10);
									strIpPort = strIp + ":" + strPort + " ~ " + strTemp;
									channelInfo.params.insert(TianShanIce::Properties::value_type(RESKEY_IpPort, strIpPort));
								}
							}
							catch (...)
							{
							}
						}

					results.push_back(channelInfo);
					glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListChannelCmd", "Results push_back(Name<%s>)"), channelInfo.name.c_str());
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
			_amdCB->ice_exception(TianShanIce::ServerError("BroadcastChannel", 501, lastError));
		}
		catch(...)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "ListChannelCmd caught unknown exception");
			lastError = buf;
			_amdCB->ice_exception(TianShanIce::ServerError("BroadcastChannel", 502, lastError));
		}
						
		return 0;
	}
	// -----------------------------
	// class ListFilterItemsCmd
	// -----------------------------
	ListFilterItemsCmd::ListFilterItemsCmd(const TianShanIce::Application::Broadcast::AMD_BcastPublisher_listFilterItemsPtr& amdCB, BroadCastChannelEnv& env)
		:_env(env), ZQ::common::ThreadRequest(env._threadPool), _amdCB(amdCB)
	{
	}

	bool ListFilterItemsCmd::init(void)
	{
		return (NULL != _amdCB);
	}

	int ListFilterItemsCmd::run(void)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListFilterItemsCmd", "list channel informations"));
		std::string lastError;
		ZQTianShan::IdentCollection Idents;

		glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListFilterItemsCmd", "--list all channels"));

		TianShanIce::Application::Broadcast::ChannnelItems results;
		try {			
			TianShanIce::StrValues itemsequence;

			itemsequence = _env._FilterItems->getFilterItemSequence();
			TianShanIce::StrValues::iterator itorSeq;

			for(itorSeq = itemsequence.begin(); itorSeq != itemsequence.end(); itorSeq++)
			{
				TianShanIce::Application::ChannelItem chitem;
				try
				{
					chitem = _env._FilterItems->findFilterItem(*itorSeq);
				}
				catch (TianShanIce::InvalidParameter& ex)
				{
					glog(ZQ::common::Log::L_WARNING, 
						CLOGFMT("ListChannelCmd", "caught tianshanIce exception[%d,%s]"),ex.errorCode, ex.message.c_str() );
					continue;
				}
				catch(::Ice::Exception&ex)
				{
					glog(ZQ::common::Log::L_WARNING, 
						CLOGFMT("ListChannelCmd", "caught ice exception[%s]"), ex.ice_name().c_str() );
					continue;
				}
                 results.push_back(chitem);
			}

			_amdCB->ice_response(results);
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT("ListFilterItemsCmd", "ice_response<%d>"), results.size());
		}
		catch(const ::Ice::Exception& ex)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "ListFilterItemsCmd caught exception[%s]", ex.ice_name().c_str());
			lastError = buf;
			_amdCB->ice_exception(TianShanIce::ServerError("BroadcastChannel", 501, lastError));
		}
		catch(...)
		{
			char buf[2048];
			snprintf(buf, sizeof(buf)-2, "ListFilterItemsCmd caught unknown exception");
			lastError = buf;
			_amdCB->ice_exception(TianShanIce::ServerError("BroadcastChannel", 502, lastError));
		}

		return 0;
	}
// -----------------------------
// class TimerCmd
// -----------------------------
TimerCmd::TimerCmd(BroadCastChannelEnv& env, const ::Ice::Identity& bcastIdent)
: ThreadRequest(env._threadPool), _env(env), _identBcastCh(bcastIdent)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerCmd, "[%s] OnTimer"), _identBcastCh.name.c_str());
}

int TimerCmd::run(void)
{
	int _ltime = gBroadcastChCfg.renewtime - 10000;
	TianShanIce::SRM::SessionPrx sessionprx;
	Ice::Long expiration = 0;
	bool isPersistent = true;
    bool bInService = true;

	TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
	try
	{	
		bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::checkedCast(_env._adapter->createProxy(_identBcastCh));
		//检查BcastChannel是否是永久保留Channel, 是否到达过期时间
		//如果是非永久保留Channel,并且到期则需要Destroy();
		expiration = bcppPrx->getExpiration();
		isPersistent = bcppPrx->isPersistent();

		if(!isPersistent && ZQ::common::now() >= expiration)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(TimerCmd, "[%s] BcastPublishPoint expired, destroy it"), _identBcastCh.name.c_str());
			bcppPrx->destroy();
			return 0;
		}

		//检查BcastChannel是否是InService状态, 如果不是InService状态,说明没有流在播,忽略这次OnTimer
		bInService = bcppPrx->isInService();
		if(!bInService)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(TimerCmd, "[%s] BcastPublishPoint is not inService Status"), _identBcastCh.name.c_str());
		}
		else
		{
			try
			{		
				sessionprx = bcppPrx->getSession();
				sessionprx->renew(gBroadcastChCfg.renewtime);
				bcppPrx->pingStream();
				glog(ZQ::common::Log::L_INFO, CLOGFMT(TimerCmd, "[%s] weiwoo session renew (%d)ms"), _identBcastCh.name.c_str(), gBroadcastChCfg.renewtime);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "[%s] session renew caught %s: %s"), 
					_identBcastCh.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			}
			catch (const Ice::ObjectNotExistException&ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "[%s] session renew caught %s"), 
					_identBcastCh.name.c_str(), ex.ice_name().c_str());
				bcppPrx->OnStreamExit("000000");				
			}
			catch (const Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "[%s] session renew caught %s"), 
					_identBcastCh.name.c_str(), ex.ice_name().c_str());
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "[%s] session renew caught unexpect exception"), 
					_identBcastCh.name.c_str());
			}

		}
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "[%s] failed to get bcastChannel proxy caught %s: %s"), 
			_identBcastCh.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::ObjectNotExistException&ex)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(TimerCmd, "[%s] bcastChannel not exist"), _identBcastCh.name.c_str());	
		return 0;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(TimerCmd, "[%s] failed to get bcastChannel proxy caught %s"), 
			_identBcastCh.name.c_str(), ex.ice_name().c_str());
	}

//	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerCmd, "[%s] expiration[%lld] now[%lld] next expiration time[%d]"), 
//		_identBcastCh.name.c_str(), (int64)expiration, ZQ::common::now(), (int)(expiration - ZQ::common::now()));

	//如果BcastChannel是非永久保留性的Channel，则需要比较Channel的过期时间与WeiwooSession的RewTime
	if(expiration > 0 && !isPersistent)
	{
		int64 lTemp = expiration - ZQ::common::now();
		if(lTemp < 0)
			lTemp = 0;
		if(lTemp >= 0 &&  lTemp < _env._sessionRewTime - 10000)
		{
			_ltime = lTemp;
			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerCmd, "[%s] OnTimer adjust next expiration to %dms"),_identBcastCh.name.c_str(), _ltime);
		}
	} 

	_env._watchDog.watchBcastChannel(_identBcastCh, _ltime);

	return 0;
}

void TimerCmd::final(int retcode, bool bCancelled)
{
	if (bCancelled)
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerCmd, "sess[%s] user canceled timer activity"),_identBcastCh.name.c_str());

	delete this;
}


// -----------------------------
// class GetStatusCmd
// -----------------------------
GetStatusCmd::GetStatusCmd(BroadCastChannelEnv& env, const ::Ice::Identity& bcastIdent)
: ThreadRequest(env._threadPool), _env(env), _identBcastCh(bcastIdent)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(GetStatusCmd, "[%s] get status"), _identBcastCh.name.c_str());

}
void logSoapErrorMsg(const soapMRT& soapMRTClient)
{
	if(soapMRTClient.soap->error)
	{ 
		std::string errMsg ="";
		const char **s;
		if (!*soap_faultcode(soapMRTClient.soap))
			soap_set_fault(soapMRTClient.soap);

		s = soap_faultdetail(soapMRTClient.soap);
		if (s && *s)
		{	
			errMsg = *s;
		}
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GetStatusCmd, "SOAP FAULT: SocketErrNo=%d ErrorCode=%d  FaultCode=<%s> FaultString=<%s> Detail Error: %s"), 
			soapMRTClient.soap->errnum, soapMRTClient.soap->error, *soap_faultcode(soapMRTClient.soap), *soap_faultstring(soapMRTClient.soap), errMsg.c_str());
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GetStatusCmd, "unknown soap error"));
	}
	glog.flush();
}
int GetStatusCmd::run(void)
{
	TianShanIce::Application::Broadcast::BcastPublishPointExPrx bcppPrx = NULL;
	try
	{	
/*		bcppPrx = TianShanIce::Application::Broadcast::BcastPublishPointExPrx::uncheckedCast(_env._adapter->createProxy(_identBcastCh));
		TianShanIce::Properties properties = bcppPrx->getProperties();

		soapMRT soapMRTClient;
		std::string endpoint;

		if(properties.find(SOAP_EndPoint) == properties.end())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(GetStatusCmd, "[%s]getStatusSoapStream() failed to find soap endpoint parameter"),_identBcastCh.name.c_str());
			return 0;
		}
//		strncpy(soapMRTClient.endpoint, (char*)properties[SOAP_EndPoint].c_str(), SOAP_TAGLEN);
		soapMRTClient.endpoint = (char*)properties[SOAP_EndPoint].c_str();

		std::string sessionId;
		if(properties.find(SOAP_SessionID) == properties.end())
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(GetStatusCmd, "[%s]getStatusSoapStream() failed to find sessionId parameter"),_identBcastCh.name.c_str());
			return 0;
		}
		sessionId = properties[SOAP_SessionID];

		soapMRTClient.soap->connect_timeout = gBroadcastChCfg.soapMRTCfg.connectTimeout;
		soapMRTClient.soap->send_timeout = gBroadcastChCfg.soapMRTCfg.sendTimeout;
		soapMRTClient.soap->recv_timeout = gBroadcastChCfg.soapMRTCfg.receiverTimeout;

		glog(ZQ::common::Log::L_INFO, CLOGFMT(GetStatusCmd, "[%s]getStatus with url[%s],sessionId[%s]"), 
			                   _identBcastCh.name.c_str(), soapMRTClient.endpoint, sessionId.c_str());
		bool bret = false;
		ZQ2__getStatusResponse getStatusRes;
		if(soapMRTClient.ZQ2__getStatus((char*)sessionId.c_str(), getStatusRes) != SOAP_OK)
		{
			// soap level error
			logSoapErrorMsg(soapMRTClient);
			return 0;
		}

		if(getStatusRes.ret != true)
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(GetStatusCmd, "[%s]getStatus soap session doesn't exist, stop weiwoo session"),_identBcastCh.name.c_str());
			bcppPrx->stop();
		}
*/
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GetStatusCmd, "[%s] failed to get bcastChannel proxy caught %s: %s"), 
			_identBcastCh.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::ObjectNotExistException&ex)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(GetStatusCmd, "[%s] bcastChannel not exist"), _identBcastCh.name.c_str());	
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(GetStatusCmd, "[%s] failed to get bcastChannel proxy caught %s"), 
			_identBcastCh.name.c_str(), ex.ice_name().c_str());
	}
	return 0;
}

void GetStatusCmd::final(int retcode, bool bCancelled)
{
	if (bCancelled)
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(TimerCmd, "sess[%s] user canceled timer activity"),_identBcastCh.name.c_str());

	delete this;
}

	// -----------------------------
	// class BcastChannelWatchDog
	// -----------------------------
#ifdef ZQ_OS_MSWIN
	BcastChannelWatchDog::BcastChannelWatchDog(BroadCastChannelEnv& env)
		:ThreadRequest(env._threadPool), _bQuit(false), _hWakeupEvent(NULL), _env(env), _nextWakeup(now() + MAX_IDLE)
	{
		_hWakeupEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	BcastChannelWatchDog::~BcastChannelWatchDog()
	{
		_bQuit = true;
		wakeup();

		::Sleep(1);
		if(_hWakeupEvent)
			::CloseHandle(_hWakeupEvent);
	}

	void BcastChannelWatchDog::wakeup()
	{
		::SetEvent(_hWakeupEvent);
	}

	bool BcastChannelWatchDog::init(void)
	{	
		return (NULL != _hWakeupEvent && !_bQuit);
	}

#else
	BcastChannelWatchDog::BcastChannelWatchDog(BroadCastChannelEnv& env)
		:ThreadRequest(env._threadPool), _nextWakeup(now() + MAX_IDLE), _env(env), _bQuit(false)
	{
		sem_init(&_wakeupSem,0,0);
	}

	BcastChannelWatchDog::~BcastChannelWatchDog()
	{
		_bQuit = true;
		wakeup();

		usleep(1000);
		try
		{
			sem_destroy(&_wakeupSem);
		}
		catch(...){}
	}

	void BcastChannelWatchDog::wakeup()
	{
		sem_post(&_wakeupSem);
	}

	bool BcastChannelWatchDog::init(void)
	{
		int nVal;
		bool re = sem_getvalue(&_wakeupSem,&nVal);
		return (re ==0 && !_bQuit);	
	}
#endif

#define MIN_YIELD	(100)  // 100 msec

	int BcastChannelWatchDog::run()
	{
		glog(ZQ::common::Log::L_DEBUG, "BcastChannelWatchDog() BcastChannel WatchDog is running");
		while(!_bQuit)
		{
			::Ice::Long timeOfNow = now();

			ZQTianShan::IdentCollection bcastChIdentsToExecute;

			{
				ZQ::common::MutexGuard gd(_lockExpirations);
				glog(ZQ::common::Log::L_INFO, CLOGFMT(BcastChannelWatchDog, "%d BcastChannel(s) under watching"), _expirations.size());
				_nextWakeup = timeOfNow + MAX_IDLE;

				for(ExpirationMap::iterator it = _expirations.begin(); it != _expirations.end(); it ++)
				{
					if (it->second <= timeOfNow)
						bcastChIdentsToExecute.push_back(it->first);
					else
						_nextWakeup = (_nextWakeup > it->second) ? it->second : _nextWakeup;
				}

				for (ZQTianShan::IdentCollection::iterator it2 = bcastChIdentsToExecute.begin(); it2 < bcastChIdentsToExecute.end(); it2++)
					_expirations.erase(*it2);
			}

			if(_bQuit)
				break;	// should quit polling

			for (ZQTianShan::IdentCollection::iterator it = bcastChIdentsToExecute.begin(); it < bcastChIdentsToExecute.end(); it++)
			{
				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChannelWatchDog, "run() timer strart bcastChannel[%s] ,[%d]bcastChannel(s) should be Timer start."), it->name.c_str(), bcastChIdentsToExecute.size());
				try
				{
					(new TimerCmd(_env, *it))->start();
				}
				catch (...)
				{
					glog(ZQ::common::Log::L_ERROR, CLOGFMT(BcastChannelWatchDog, "run() timer strart bcastChannel[%s] caught unknown exception"), it->name.c_str());
				}	
			}
			bcastChIdentsToExecute.clear();

			if(_bQuit)
				break;	// should quit polling

			long sleepTime = (long) (_nextWakeup - now());

			if (sleepTime < MIN_YIELD)
				sleepTime = MIN_YIELD;
#ifdef ZQ_OS_MSWIN
			::WaitForSingleObject(_hWakeupEvent, sleepTime);
#else
			struct timespec ts;
			struct timeval tmval;
			gettimeofday(&tmval,(struct timezone*)NULL);
			int64 nMicro = sleepTime*1000ll + tmval.tv_usec;
			ts.tv_sec = tmval.tv_sec + nMicro/1000000;
			ts.tv_nsec = (nMicro%1000000) * 1000;
			sem_timedwait(&_wakeupSem,&ts);
#endif
		} // while

		glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastChannelWatchDog, "end of BcastChannel WatchDog"));

		return 0;
	}

	void BcastChannelWatchDog::quit()
	{
		_bQuit=true;
		wakeup();
	}
	void BcastChannelWatchDog::removeBcastChannel(const ::Ice::Identity& bcastChIdent)
	{
		ZQ::common::MutexGuard gd(_lockExpirations);
		ExpirationMap::iterator iter = _expirations.find(bcastChIdent);
		if(iter != _expirations.end())
			_expirations.erase(iter);
	}
	void BcastChannelWatchDog::watchBcastChannel(const ::Ice::Identity& bcastChIdent, long timeout)
	{
		int originalExpirationsSize = 0;
		int newExpirationsSize = 0;
		std::string strOriginal = "";
		std::string strNew="";
		{
			ZQ::common::MutexGuard gd(_lockExpirations);
			ExpirationMap::iterator iter = _expirations.begin();
			for (; iter != _expirations.end(); iter++)
			{
				strOriginal += iter->first.name;
				strOriginal += ";";
			}
			originalExpirationsSize = _expirations.size();
			if (timeout > 0)
			{
				::Ice::Long newExp = now() + timeout;
				//ZQ::common::MutexGuard gd(_lockExpirations);
				MAPSET(ExpirationMap , _expirations, bcastChIdent, newExp);
				if (newExp < _nextWakeup)
				{
					wakeup();
//					glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChannelWatchDog, "watchBcastChannel() wake up the bcastChannel[%s] event."), bcastChIdent.name.c_str());
				}
			}
			else
			{
				//ZQ::common::MutexGuard gd(_lockExpirations);
				ExpirationMap::iterator it = _expirations.find(bcastChIdent);
				if (it!=_expirations.end())
				{
					_expirations.erase(it);
				}
				(new TimerCmd(_env, bcastChIdent))->start();
			}
			newExpirationsSize = _expirations.size();
			ExpirationMap::iterator iterNew = _expirations.begin();
			for (; iterNew != _expirations.end(); iterNew++)
			{
				strNew += iterNew->first.name;
				strNew += ";";
			}
		}
//		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChannelWatchDog, "watchBcastChannel() bcastChannel[%s] timeout[%d] ,expirations size [%d =>%d] , expirations[%s==>>%s]"), bcastChIdent.name.c_str(), timeout, originalExpirationsSize, newExpirationsSize, strOriginal.c_str() ,strNew.c_str());
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(BcastChannelWatchDog, "watchBcastChannel() bcastChannel[%s] timeout[%d] ,expirations size [%d =>%d]"), bcastChIdent.name.c_str(), timeout, originalExpirationsSize, newExpirationsSize);

	}

	void BcastChannelWatchDog::final(int retcode, bool bCancelled)
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(BcastChannelWatchDog, "bcastChannel watch dog stopped"));
	}

	//////////////////////////////////////////////////////////////////////////
	// class NVODSupplementManager
	//////////////////////////////////////////////////////////////////////////

	NVODSupplementThread::NVODSupplementThread(BroadCastChannelEnv& env, const ::Ice::Identity& ident, ::Ice::Int interval, bool bIsInit):
	_env(env), _ident(ident),_interval(interval),_bIsInit(bIsInit)

	{
		_stopped = false;
	}
	NVODSupplementThread::~NVODSupplementThread()
	{

	}
	int NVODSupplementThread::run()
	{
		::Ice::Int _ltime = _interval;
		int        iterator = 0; //indicate the total number of supplement channel
        TianShanIce::Application::Broadcast::NVODSupplementalChannels suplChannels;
		try
		{  			
			TianShanIce::Application::Broadcast::NVODChannelPublishPointExPrx pintPrx = NULL;
			pintPrx = TianShanIce::Application::Broadcast::NVODChannelPublishPointExPrx::checkedCast(_env._adapter->createProxy(_ident));
            suplChannels = pintPrx->getSupplementalChannels();
		}
		catch (Ice::Exception&ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(NVODSupplementThread, "[%s] get supplement channel caught %s"), 
				_ident.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(NVODSupplementThread, "[%s] get supplement channel caught unknown exception %d"), 
				_ident.name.c_str(), SYS::getLastErr());
		}

		while(!_stopped)
		{			
			// check it after _time ms

			_hStop.wait(_ltime);
			if (_stopped)
				break;
			try
			{
				if(iterator >= suplChannels.size())
				{
					_ltime = _interval * 2;
                    ///check stream;
				}
				else
				{
					TianShanIce::Application::Broadcast::NVODSupplementalChannels::iterator suplitor;
					if(suplChannels.size() > 0)
					{
						try
						{
							TianShanIce::Application::BroadcastPublishPointPrx bcastPrx = suplChannels[iterator];
							bcastPrx->start();
							iterator++;
						}
						catch (TianShanIce::ServerError&ex)
						{
							glog(ZQ::common::Log::L_ERROR, CLOGFMT(NVODSupplementThread, "[%s] start NVOD supplement channel caught %s, %s"), 
								_ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str()); 
						}						
					}
				}             
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(NVODSupplementThread, "[%s]caught unknown exception %d"), 
					_ident.name.c_str(), SYS::getLastErr());
			}
		}

		glog(ZQ::common::Log::L_INFO, CLOGFMT(NVODSupplementMgrMacro,
			          "[%s]exit NVODSupplement Channel Manager thread"), _ident.name.c_str());

		return 0;
	}

	void NVODSupplementThread::stop()
	{
		_stopped = true;
		_hStop.signal();
	}
} // namespace ZQChannelOnDemand

