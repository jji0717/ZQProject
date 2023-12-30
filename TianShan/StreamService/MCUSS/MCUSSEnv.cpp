#include "MCUSSEnv.h"
#include "TsStreamer.h"

extern ZQTianShan::MCUSS::ST_MCUSS::MCUSSHolder	*pMCUSSBaseConfig;

using namespace ZQ::common;
using namespace ZQ::StreamService;
namespace ZQTianShan{
	namespace MCUSS{
		// -----------------------------
		// class MCUSSEnv
		// -----------------------------
		MCUSSEnv::MCUSSEnv(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog, ZQ::common::NativeThreadPool& rtspThpool, ZQ::common::NativeThreadPool& ssThpool)
			: _bRun(false), _rtspTraceLevel(Log::L_INFO), _sessTimeout(MAX_IDLE), _rtspThpool(rtspThpool),
			SsEnvironment(mainLog,sessLog,ssThpool)
		{
			
		}

		MCUSSEnv::~MCUSSEnv()
		{
			_bRun = false;
		}

		void MCUSSEnv::init()
		{
			streamsmithConfig.iEnableSGScaleChangeEvent = pMCUSSBaseConfig->_postEventHolder.enableScaleChangeEvent;
			streamsmithConfig.iEnableSGStateChangeEvent = pMCUSSBaseConfig->_postEventHolder.enableStateChangeEvent;
			streamsmithConfig.iPassSGScaleChangeEvent	= pMCUSSBaseConfig->_postEventHolder.passScaleChangeEvent;
			streamsmithConfig.iPassSGStateChangeEvent	= pMCUSSBaseConfig->_postEventHolder.passStateChangeEvent;
			streamsmithConfig.iMaxPendingRequestSize	= pMCUSSBaseConfig->_bindPendingMax;
			try
			{
				_logger.open(pMCUSSBaseConfig->_sessHistoryHolder.path.c_str(),
					pMCUSSBaseConfig->_sessHistoryHolder.level,
					ZQLOG_DEFAULT_FILENUM,
					pMCUSSBaseConfig->_sessHistoryHolder.size,
					pMCUSSBaseConfig->_sessHistoryHolder.bufferSize,
					pMCUSSBaseConfig->_sessHistoryHolder.flushTimeout);
			}
			catch( const ZQ::common::FileLogException& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSEnv,"failed to open session history log file[%s] because [%s]"),
					pMCUSSBaseConfig->_sessHistoryHolder.path.c_str(), ex.what() );
			}

			_userAgent =  std::string("MCUSS/") + __STR1__(ZQ_PRODUCT_VER_MAJOR) "." __STR1__(ZQ_PRODUCT_VER_MINOR);

			if (_rtspTraceLevel < _logger.getVerbosity())
				_rtspTraceLevel = (ZQ::common::Log::loglevel_t)_logger.getVerbosity();

			// initialize sessiongroup
			int32& maxSessionGroups = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceMaxSessionGroup;
			int32& maxSessionsPerGroup = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceMaxSessionsPerGroup;

			// step 1. adjust the configuration
			if (maxSessionGroups < 1)
			{
				maxSessionGroups = 2;
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSSEnv, "invalid max SessionGroups(%d), reset to 2"), maxSessionGroups);	
			}
			else if (maxSessionGroups > 99)
			{
				maxSessionGroups = 99;
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSSEnv, "invalid max SessionGroups(%d), reset to 99"), maxSessionGroups);	
			}

			if (maxSessionsPerGroup > 800)
			{
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSSEnv, "invalid MasSessionsPerGroup number(%d) exceed maximum, reset to 800"), maxSessionsPerGroup);	
				maxSessionsPerGroup = 800;
			}
			else if (maxSessionsPerGroup < 10)
			{
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSSEnv,"invalid MasSessionsPerGroup number(%d) less than minimum, reset to 20"), maxSessionsPerGroup);	
				maxSessionsPerGroup = 10;
			}

			// step 2. generate baseURL
			std::string baseURL = std::string("rtsp://") + pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceIp;

			if ( 554 != pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfacePort )
			{
				char buf[32];
				snprintf(buf, sizeof(buf)-2, ":%d", pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfacePort);
				baseURL += buf;
			}

			if (pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceBind.empty())
			{
				_bindAddr = InetHostAddress(pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceBind.c_str());
				glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(MCUSSEnv, "taking localAddr[%s, %s] to bind"), pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceBind.c_str(), _bindAddr.getHostAddress());	
			}

			if (pMCUSSBaseConfig->_videoServerHolder.sessionRenewInterval > 10)
				_sessTimeout = pMCUSSBaseConfig->_videoServerHolder.sessionRenewInterval *1000;

			glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(MCUSSEnv, "taking session timeout[%d]msec"), _sessTimeout);	

			streamsmithConfig.iSupportPlaylist = LIB_SUPPORT_NORMAL_STREAM;
		}

		void MCUSSEnv::start()
		{
			_bRun = true;
		}

		void MCUSSEnv::uninit()
		{
			_bRun = false;
		}

		void MCUSSEnv::deleteSessFromMap(const std::string& sessId, const std::string& contentName)
		{
			ZQ::common::MutexGuard g(_lkInterestMap);
			SessionIDList& sessList = _interestMap[contentName];
			sessList.erase(std::remove(sessList.begin(), sessList.end(), sessId), sessList.end());
		}
	}//namespace MCUSS
}//namespace ZQTianShan

namespace ZQ {
	namespace StreamService {

		using namespace ZQ::common;

		TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
			const TianShanIce::SRM::ResourceType& type,
			const std::string& strkey)
		{
			TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
			if (itResMap==rcMap.end())
			{
				char szBuf[1024];
				snprintf(szBuf, sizeof(szBuf), "GetResourceMapData() type %d not found", type);

				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("MCUSS",1001,szBuf );
			}

			TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
			if (it==itResMap->second.resourceData.end())
			{
				char szBuf[1024];
				snprintf(szBuf, sizeof(szBuf), "GetResourceMapData() value with key=%s not found", strkey.c_str());
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("MCUSS",1002,szBuf);
			}

			return it->second;
		}

		//add for CCUR
		void calcScale(float& fScale, SsContext& ctx )
		{
			if( pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetEnable <= 0 )
				return; // keep current scale

			//detect request scale attribute
			int requestDirection = 0 ;
			if ((fScale - 1.0f) > 0.0001f )
				requestDirection = 1; //fast forward
			else if( fScale < 0.0f )
				requestDirection = -1; //fast rewind
			else
			{
				//reset last scale status
				ctx.updateContextProperty(SPEED_LASTIDX, -1);
				ctx.updateContextProperty(SPEED_LASTDIR, 1);
				return ;
			}

			int iLastIndex = atoi(ctx.getContextProperty(SPEED_LASTIDX).c_str());

			int lastDirection = atoi(ctx.getContextProperty(SPEED_LASTDIR).c_str());

			bool bUseForwardSet = requestDirection > 0;

			std::vector<float> scaleSet;
			if( bUseForwardSet )
				scaleSet = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetForwardSet;
			else
				scaleSet = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetBackwardSet;

			if (scaleSet.size() <= 0 )
			{
				ctx.updateContextProperty(SPEED_LASTIDX, -1);
				ctx.updateContextProperty(SPEED_LASTDIR, requestDirection);
				return ;
			}

			if (pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.EnableFixedSpeedLoop >= 1 )//EnableFixedSpeedLoop
			{
				iLastIndex = ( lastDirection * requestDirection ) < 0 ? 0 : ( (++iLastIndex) % (int) scaleSet.size() );
			}
			else
			{
				iLastIndex = ( lastDirection * requestDirection ) < 0 ? 0 : (++iLastIndex);
				if( iLastIndex >= (int) scaleSet.size() ) 
				{
					//reach the end of the fix speed set
					ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(glog, "MCUSS", 0, "session[%s] can't loop the speed, we already reach the end of speed set", ctx.id().c_str()  );
				}
			}

			float fOldScale = fScale;

			fScale = scaleSet[iLastIndex];

			//reset last scale status
			ctx.updateContextProperty(SPEED_LASTIDX, iLastIndex);
			ctx.updateContextProperty(SPEED_LASTDIR, requestDirection);

			glog(ZQ::common::Log::L_DEBUG, CLOGFMT(MCUSS, "calcScale() session[%s] convert speed from [%f] to [%f] according to fix speed set"), ctx.id().c_str(), fOldScale, fScale);

			return;
		}

		bool	SsServiceImpl::listAllReplicas( SsServiceImpl& ss, OUT SsReplicaInfoS& infos )
		{
			SsReplicaInfo replica;
			replica.bHasPorts = false;
			replica.streamerType = "MCUSS";
			replica.bStreamReplica = true;
			replica.replicaState = TianShanIce::stInService;
			replica.replicaId = "VSOP";
			infos.insert(std::make_pair("MCUSS", replica));

			return  true;
		}

		bool SsServiceImpl::allocateStreamResource(	SsServiceImpl& ss, 
			IN const std::string& streamerId,
			IN const std::string& portId,
			IN const TianShanIce::SRM::ResourceMap&	resource )
		{
			return  true;
		}

		bool SsServiceImpl::releaseStreamResource( SsServiceImpl& ss, SsContext& sessCtx, IN const std::string& streamerReplicaId  )
		{
			return true;
		}

		int32	SsServiceImpl::doValidateItem(  SsServiceImpl& ss, 
			IN SsContext& ctx,
			INOUT TianShanIce::Streamer::PlaylistItemSetupInfo& info )
		{
			glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(MCUSS, "doValidateItem() sess[%s]  content [%s] enter."), ctx.id().c_str(), info.contentName.c_str());
			//set content as InService, so that no more GET_PARAMETER will be used to get the content play duration
			Ice::Int state = 1;
			ZQTianShan::Util::updateValueMapData( info.privateData , "StreamService.ContentInService" , state );
			ZQTianShan::MCUSS::MCUSSEnv* c2Env = dynamic_cast<ZQTianShan::MCUSS::MCUSSEnv*> (ss.env);

			//c2Env->_c2ContentAttrCache->setUri(/*ss.getAdminUri()*/ss.strAdminUrl);

			if ( !c2Env)
			{
				glog(::ZQ::common::Log::L_ERROR, CLOGFMT(MCUSS, "doValidateItem() sess[%s] the env is null ."), ctx.id().c_str());
				return ERR_RETURN_SERVER_ERROR;
			}
			//if (1 == pMCUSSBaseConfig->_syncCall)
			//{

			//	ZQTianShan::MCUSS::C2ContentQueryBind::ContentAttr contentAttr;
			//	//	c2Env->_c2ContentAttrCache->lookupContent_sync(contentAttr, info.contentName, pMCUSSBaseConfig->_syncTimeOut);
			//	if( !c2Env->_c2ContentAttrCache->lookupContent_sync(contentAttr, info.contentName, pMCUSSBaseConfig->_syncTimeOut))
			//		return ERR_RETURN_SERVER_ERROR;
			//}
			//else
			//{
			//	std::string sessionId = ctx.id();
			//	ZQTianShan::MCUSS::MCUSSEnv::SessionIDList idList;
			//	{
			//		ZQ::common::MutexGuard g(c2Env->_lkInterestMap);
			//		ZQTianShan::MCUSS::MCUSSEnv::InterestMAP::iterator  iterC = c2Env->_interestMap.find(info.contentName);
			//		if (c2Env->_interestMap.end() != iterC)
			//			iterC->second.push_back(sessionId);
			//		else
			//		{
			//			idList.push_back(sessionId);
			//			c2Env->_interestMap[info.contentName] = idList;
			//		}
			//	}

			//	ZQTianShan::MCUSS::MCUSSEnv::CA	ca;
			//	ca.name = info.contentName;
			//	ca.stampAsOf = -1;
			//	ZQTianShan::MCUSS::MCUSSEnv::CAList caList;
			//	{
			//		ZQ::common::MutexGuard g(c2Env->_lkCAOfSessionMap);
			//		ZQTianShan::MCUSS::MCUSSEnv::CAOFSessionMAP::iterator iterS = c2Env->_CAOfSessionMap.find(sessionId);
			//		if (c2Env->_CAOfSessionMap.end() != iterS)
			//		{
			//			iterS->second.push_back(ca);
			//		}
			//		else
			//		{
			//			caList.push_back(ca);
			//			c2Env->_CAOfSessionMap[sessionId] = caList;
			//		}
			//	}
			//	c2Env->_c2ContentAttrCache->lookupContent_async(*c2Env, info.contentName);
			//}
			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doCommit(SsServiceImpl& ss, 
			IN SsContext& ctx,								
			IN PlaylistItemSetupInfos& items,
			IN TianShanIce::SRM::ResourceMap& requestResources )
		{
			std::string volumeName, dest, type; //MP2T/DVBC/UDP as default
			std::string strClient, bandwidth, sop_name, sop_group;

			::Ice::Identity ident;
			ident.name = ctx.id();

			glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(MCUSS, "doCommit() stream[%s]"), ident.name.c_str());

			::TianShanIce::Transport::PathTicketPrx _pathTicket = ss.getPathTicket(ident.name);
			if (!_pathTicket)
			{
				glog(::ZQ::common::Log::L_ERROR, CLOGFMT(MCUSS, "stream[%s] null PathTicket bound"), ident.name.c_str());
				return ERR_RETURN_SERVER_ERROR;
			}

			// read the PathTicket
			::std::string strTicket = ss.env->getCommunicator()->proxyToString(_pathTicket);
			glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(MCUSS, "doCommit() stream[%s] reading PathTicket[%s]"), ident.name.c_str(), strTicket.c_str());

			::TianShanIce::ValueMap pMap = _pathTicket->getPrivateData();
			::TianShanIce::SRM::ResourceMap resMap = _pathTicket->getResources();
			::TianShanIce::Transport::StorageLinkPrx storageLink = _pathTicket->getStorageLink();
			std::string	storageLinkType = storageLink->getType();
			std::string	streamLinkType = _pathTicket->getStreamLink()->getType();
			dest = "dvbc://@";
			if (pMCUSSBaseConfig->_videoServerHolder.bSeperateIPStreaming && std::string::npos == streamLinkType.find("DVBC")) // ip streaming
				dest = "udp://@";

			TianShanIce::Variant	valBandwidth;
			TianShanIce::Variant	valDestAddr;
			TianShanIce::Variant	valDestPort;
			TianShanIce::Variant	valDestMac;

			// read the resource of ticket
			// 1. client mac(client_id), and mac may not available
			try
			{
				valDestMac = GetResourceMapData(requestResources, TianShanIce::SRM::rtEthernetInterface, "destMac");
				if (valDestMac.type != TianShanIce::vtStrings || valDestMac.strs.size () == 0 )
				{
					glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid dest mac type or no data"), ident.name.c_str(), strTicket.c_str());
					return ERR_RETURN_INVALID_PARAMATER;
				}

				strClient = valDestMac.strs[0];
			}
			catch (TianShanIce::InvalidParameter&)
			{
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] no dest mac information"), ident.name.c_str(), strTicket.c_str());
				return ERR_RETURN_INVALID_PARAMATER;
			}

			// 2. destination IP
			try
			{
				valDestAddr = GetResourceMapData (requestResources,TianShanIce::SRM::rtEthernetInterface, "destIP");
				if (valDestAddr.type != TianShanIce::vtStrings || valDestAddr.strs.size () == 0)
				{
					glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid destAddress type or no data"), ident.name.c_str(), strTicket.c_str());
					return ERR_RETURN_INVALID_PARAMATER;
				}

				dest += valDestAddr.strs[0];
			}
			catch (TianShanIce::InvalidParameter&)
			{
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid destAddress type or no data"), ident.name.c_str(), strTicket.c_str());
				return ERR_RETURN_INVALID_PARAMATER;
			}

			//get destination port
			try
			{
				valDestPort = GetResourceMapData(requestResources,TianShanIce::SRM::rtEthernetInterface,"destPort");
				if ( valDestPort.type != TianShanIce::vtInts || valDestPort.ints.size() == 0 )
				{
					glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid destPort type or no data"), ident.name.c_str(), strTicket.c_str());
					return ERR_RETURN_INVALID_PARAMATER;
				}

				::std::stringstream ss;
				ss << valDestPort.ints[0];
				dest = dest + ":" + ss.str();
			}
			catch (TianShanIce::InvalidParameter&)
			{
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid destPort type or no data"), ident.name.c_str(), strTicket.c_str());
				return ERR_RETURN_INVALID_PARAMATER;
			}

			//get bandwidth
			try
			{
				valBandwidth = GetResourceMapData(requestResources, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth");
				if (valBandwidth.type != TianShanIce::vtLongs || valBandwidth.lints.size () == 0 )
				{
					glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid bandwidth type or no data"), ident.name.c_str(), strTicket.c_str());
					return ERR_RETURN_INVALID_PARAMATER;
				}

				long lBandwidth =0;
				if (::TianShanIce::vtLongs == valBandwidth.type)
					lBandwidth = (long) valBandwidth.lints[0];
				else if (::TianShanIce::vtInts == valBandwidth.type)
					lBandwidth = valBandwidth.ints[0];

				::std::stringstream ss;
				ss << lBandwidth;
				bandwidth = ss.str();
			}
			catch (TianShanIce::InvalidParameter&)
			{
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid bandwidth type or no data"), ident.name.c_str(), strTicket.c_str());
				return ERR_RETURN_INVALID_PARAMATER;
			}

			try
			{
				::TianShanIce::ValueMap::iterator itPV = pMap.find(PathTicketPD_Field(sop_name));
				if (pMap.end() != itPV && itPV->second.strs.size() >0)
					sop_name = itPV->second.strs[0];

				itPV = pMap.find(PathTicketPD_Field(sop_group));
				if (pMap.end() != itPV && itPV->second.strs.size() >0)
					sop_group = itPV->second.strs[0];
			}
			catch (...)
			{
				glog(::ZQ::common::Log::L_WARNING, CLOGFMT(MCUSS, "doCommit() stream[%s]ticket[%s] invalid SOP"), ident.name.c_str(), strTicket.c_str());
				return ERR_RETURN_SERVER_ERROR;
			}

			std::string storageNetId;
			//resMap
			TianShanIce::SRM::ResourceMap::const_iterator itStorageNetId = resMap.find( TianShanIce::SRM::rtStorage );
			if( itStorageNetId != resMap.end() )
			{
				const TianShanIce::SRM::Resource& resStorageNetId = itStorageNetId->second;
				const TianShanIce::ValueMap::const_iterator itNetId = resStorageNetId.resourceData.find("NetworkId");
				if (itNetId != resStorageNetId.resourceData.end() && itNetId->second.type == TianShanIce::vtStrings && itNetId->second.strs.size() > 0 )
					storageNetId = itNetId->second.strs[0];
			}

			if (storageLinkType == "SeaChange.NSS.C2Transfer")
			{
				if (!pMCUSSBaseConfig->_videoServerHolder.libraryVolume.empty())
					volumeName = pMCUSSBaseConfig->_videoServerHolder.libraryVolume;
				else
					volumeName = "library";
			}
			else
			{
				if( storageNetId.empty() )
					volumeName = pMCUSSBaseConfig->_videoServerHolder.contentInterfaceHolder.vols[0].targetName;
				else
					volumeName = storageNetId;
			}

			glog(::ZQ::common::Log::L_INFO, CLOGFMT(MCUSS, "Stream[%s] read ticket[%s]: dest[%s], destMAC[%s], bandwidth[%s], SOP[%s], volume[%s]"), 
				ident.name.c_str(), strTicket.c_str(), dest.c_str(), strClient.c_str(), bandwidth.c_str(), sop_name.c_str(), volumeName.c_str());

			bool loopWhile = true;
			ZQTianShan::MCUSS::MCUSSEnv* c2Env = dynamic_cast<ZQTianShan::MCUSS::MCUSSEnv*> (ss.env);
			//glog(::ZQ::common::Log::L_INFO, CLOGFMT(MCUSS, "doCommit() sess[%s] try to get Atrr."), ctx.id().c_str());
			//if (1 == pMCUSSBaseConfig->_syncCall)
			//{
			//	for (PlaylistItemSetupInfos::iterator iter = items.begin(); iter != items.end(); iter++)
			//	{
			//		ZQTianShan::MCUSS::C2ContentQueryBind::ContentAttr c2Attr;
			//		if( !(c2Env->_c2ContentAttrCache->getAttr(iter->contentName, c2Attr)))
			//		{
			//			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(MCUSS, "doCommit() can not find attr from cache with  content[%s]."), iter->contentName.c_str());
			//			return ERR_RETURN_SERVER_ERROR;
			//		}
			//		TianShanIce::ValueMap privateData;
			//		ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_AssetID, c2Attr.assetId.c_str());
			//		ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ProviderID, c2Attr.providerId.c_str());
			//		ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ExtName, c2Attr.mainFileExtName.c_str());
			//		ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_MuxBitrate, c2Attr.muxBitrate);
			//		ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_PlayTime, c2Attr.playTime_msec);
			//		ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_OpenForWrite, c2Attr.isPWE);
			//		ZQTianShan::MCUSS::C2ContentQueryBind::AttrMap::iterator iterAttr = c2Attr.attrs.begin();
			//		for (; iterAttr != c2Attr.attrs.end(); iterAttr ++)
			//			ZQTianShan::Util::updateValueMapData(privateData, iterAttr->first, iterAttr->second.c_str());
			//		iter->privateData = privateData;
			//	}
			//}
			//else
			//{
			//	while(loopWhile)
			//	{
			//		// TEST IF the CA of this session has been ready
			//		// commit the SS if true, otherwise continue to wait 
			//		if(c2Env->c2LocateReady(ctx.id()))
			//		{
			//			glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(MCUSS, "doCommit() all of the content in sess[%s] is ready."), ctx.id().c_str());
			//			ZQTianShan::MCUSS::MCUSSEnv::CAList cAlist ;
			//			{
			//				ZQ::common::MutexGuard g(c2Env->_lkCAOfSessionMap);
			//				ZQTianShan::MCUSS::MCUSSEnv::CAOFSessionMAP::iterator iterSess = c2Env->_CAOfSessionMap.find(ctx.id());
			//				if (c2Env->_CAOfSessionMap.end() != iterSess)
			//				{
			//					cAlist = iterSess->second;
			//					c2Env->_CAOfSessionMap.erase(iterSess);
			//				}
			//			}
			//			//没有写在下面循环中是因为：下面循环如果出错可能直接退出了。
			//			for(ZQTianShan::MCUSS::MCUSSEnv::CAList::iterator iterDe = cAlist.begin(); iterDe != cAlist.end(); iterDe++)
			//				c2Env->deleteSessFromMap(ctx.id(), iterDe->name);

			//			for(ZQTianShan::MCUSS::MCUSSEnv::CAList::iterator iterca = cAlist.begin(); iterca != cAlist.end(); iterca++)
			//			{
			//				for (PlaylistItemSetupInfos::iterator iter = items.begin(); iter != items.end(); iter++)
			//				{
			//					if (iterca->name == iter->contentName)
			//					{
			//						ZQTianShan::MCUSS::C2ContentQueryBind::ContentAttr c2Attr;
			//						if( !(c2Env->_c2ContentAttrCache->getAttr(iter->contentName, c2Attr)))
			//						{
			//							glog(::ZQ::common::Log::L_ERROR, CLOGFMT(MCUSS, "doCommit() can not find attr from cache with  content[%s]."), iter->contentName.c_str());
			//							return ERR_RETURN_SERVER_ERROR;
			//						}
			//						TianShanIce::ValueMap privateData;
			//						ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_AssetID, c2Attr.assetId.c_str());
			//						ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ProviderID, c2Attr.providerId.c_str());
			//						ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ExtName, c2Attr.mainFileExtName.c_str());
			//						ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_MuxBitrate, c2Attr.muxBitrate);
			//						ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_PlayTime, c2Attr.playTime_msec);
			//						ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_OpenForWrite, c2Attr.isPWE);
			//						ZQTianShan::MCUSS::C2ContentQueryBind::AttrMap::iterator iterAttr = c2Attr.attrs.begin();
			//						for (; iterAttr != c2Attr.attrs.end(); iterAttr ++)
			//							ZQTianShan::Util::updateValueMapData(privateData, iterAttr->first, iterAttr->second.c_str());
			//						iter->privateData = privateData;
			//					}//if
			//				}//for
			//			}//for
			//			loopWhile = false;
			//		}//if (c2Env->c2LocateReady(ctx.id()))
			//		else //
			//		{
			//			SYS::SingleObject::STATE sigState = c2Env->_wakeup.wait(pMCUSSBaseConfig->_c2ContentTimeout);
			//			if (SYS::SingleObject::SIGNALED == sigState)
			//			{
			//				continue;
			//			}
			//			else
			//			{
			//				glog(::ZQ::common::Log::L_ERROR, CLOGFMT(MCUSS, "doCommit() sess[%s] failed with sess timeout."), ctx.id().c_str());
			//				return ERR_RETURN_SERVER_ERROR;
			//			}
			//		}
			//	}//while
			//}
			glog(::ZQ::common::Log::L_INFO, CLOGFMT(MCUSS, "doCommit() sess[%s] successful."), ctx.id().c_str());

			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doDestroy(SsServiceImpl& ss, SsContext&  contextKey,IN const std::string& streamId )
		{
			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doLoad(	SsServiceImpl& ss,
			IN SsContext& contextKey,								
			IN const TianShanIce::Streamer::PlaylistItemSetupInfo& itemInfo, 
			IN int64 timeoffset,
			IN float scale,								
			INOUT StreamParams& ctrlParams,
			OUT std::string&	streamId )
		{
			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doPlay(SsServiceImpl& ss,
			SsContext& contextKey,							   
			IN const std::string& streamId,
			IN int64 timeOffset,
			IN float scale,							   
			INOUT StreamParams& ctrlParams )
		{	
			calcScale(scale, contextKey);
			double start = (ctrlParams.mask & MASK_TIMEOFFSET) ? static_cast<double>(timeOffset) / 1000.0f : -1.0f; // if the mask of timeoffset is not set, play from now(-1.0)

			if (timeOffset <1 && abs(scale) >1 && 0 != pMCUSSBaseConfig->_videoServerHolder.vendor.compare("SeaChange"))
				start =-1.0f;

			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doPause(	SsServiceImpl& ss, SsContext& contextKey, IN const std::string& streamId, INOUT StreamParams& ctrlParams )
		{
			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doResume(  SsServiceImpl& ss, SsContext& contextKey, IN const std::string& streamId,INOUT StreamParams& ctrlParams )
		{
			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doReposition( SsServiceImpl& ss, SsContext& contextKey,IN const std::string& streamId,
			IN int64 timeOffset, IN const float& scale, INOUT StreamParams& ctrlParams )
		{
			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doChangeScale(	 SsServiceImpl& ss, SsContext& contextKey, IN const std::string& streamId,								
			IN float newScale, INOUT StreamParams& ctrlParams )
		{
			return ERR_RETURN_SUCCESS;
		}

		int32 SsServiceImpl::doGetStreamAttr(	 SsServiceImpl& ss, SsContext& contextKey, IN	const std::string& streamId,
			IN  const TianShanIce::Streamer::PlaylistItemSetupInfo& info, OUT StreamParams& ctrlParams )
		{
			return ERR_RETURN_SUCCESS;
		}

	} //namespace ZQ
} //namespace StreamService