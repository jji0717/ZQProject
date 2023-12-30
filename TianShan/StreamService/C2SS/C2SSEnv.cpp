#include "C2SSEnv.h"
#include "TsStreamer.h"

extern ZQTianShan::C2SS::ST_C2SS::C2SSHolder	*pC2SSBaseConfig;

using namespace ZQ::common;
using namespace ZQ::StreamService;
namespace ZQTianShan{
	namespace C2SS{
// -----------------------------
// class C2SSEnv
// -----------------------------
C2SSEnv::C2SSEnv(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog, ZQ::common::NativeThreadPool& rtspThpool, ZQ::common::NativeThreadPool& ssThpool)
: _bRun(false), _rtspTraceLevel(Log::L_INFO), _sessTimeout(MAX_IDLE), _rtspThpool(rtspThpool),
SsEnvironment(mainLog,sessLog,ssThpool)
{
	_c2ContentAttrCache = new ZQTianShan::C2SS::C2ContentAttrCache(mainLog, ssThpool, pC2SSBaseConfig->_httpCRGURL);
}

C2SSEnv::~C2SSEnv()
{
	_bRun = false;
	if ( NULL != _c2ContentAttrCache)
	{
		delete _c2ContentAttrCache;
		_c2ContentAttrCache = NULL;
	}
}

void C2SSEnv::init()
{
	streamsmithConfig.iEnableSGScaleChangeEvent = pC2SSBaseConfig->_postEventHolder.enableScaleChangeEvent;
	streamsmithConfig.iEnableSGStateChangeEvent = pC2SSBaseConfig->_postEventHolder.enableStateChangeEvent;
	streamsmithConfig.iPassSGScaleChangeEvent	= pC2SSBaseConfig->_postEventHolder.passScaleChangeEvent;
	streamsmithConfig.iPassSGStateChangeEvent	= pC2SSBaseConfig->_postEventHolder.passStateChangeEvent;
	streamsmithConfig.iMaxPendingRequestSize	= pC2SSBaseConfig->_bindPendingMax;
	try
	{
		_logger.open(pC2SSBaseConfig->_sessHistoryHolder.path.c_str(),
			pC2SSBaseConfig->_sessHistoryHolder.level,
			ZQLOG_DEFAULT_FILENUM,
			pC2SSBaseConfig->_sessHistoryHolder.size,
			pC2SSBaseConfig->_sessHistoryHolder.bufferSize,
			pC2SSBaseConfig->_sessHistoryHolder.flushTimeout);
	}
	catch( const ZQ::common::FileLogException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSEnv,"failed to open session history log file[%s] because [%s]"),
			pC2SSBaseConfig->_sessHistoryHolder.path.c_str(), ex.what() );
	}

	_userAgent =  std::string("C2SS/") + __STR1__(ZQ_PRODUCT_VER_MAJOR) "." __STR1__(ZQ_PRODUCT_VER_MINOR);

	if (_rtspTraceLevel < _logger.getVerbosity())
		_rtspTraceLevel = (ZQ::common::Log::loglevel_t)_logger.getVerbosity();

	// initialize sessiongroup
	int32& maxSessionGroups = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceMaxSessionGroup;
	int32& maxSessionsPerGroup = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceMaxSessionsPerGroup;

	// step 1. adjust the configuration
	if (maxSessionGroups < 1)
	{
		maxSessionGroups = 2;
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SSEnv, "invalid max SessionGroups(%d), reset to 2"), maxSessionGroups);	
	}
	else if (maxSessionGroups > 99)
	{
		maxSessionGroups = 99;
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SSEnv, "invalid max SessionGroups(%d), reset to 99"), maxSessionGroups);	
	}

	if (maxSessionsPerGroup > 800)
	{
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SSEnv, "invalid MasSessionsPerGroup number(%d) exceed maximum, reset to 800"), maxSessionsPerGroup);	
		maxSessionsPerGroup = 800;
	}
	else if (maxSessionsPerGroup < 10)
	{
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SSEnv,"invalid MasSessionsPerGroup number(%d) less than minimum, reset to 20"), maxSessionsPerGroup);	
		maxSessionsPerGroup = 10;
	}

	// step 2. generate baseURL
	std::string baseURL = std::string("rtsp://") + pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceIp;

	if ( 554 != pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfacePort )
	{
		char buf[32];
		snprintf(buf, sizeof(buf)-2, ":%d", pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfacePort);
		baseURL += buf;
	}

	if (pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceBind.empty())
	{
		_bindAddr = InetHostAddress(pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceBind.c_str());
		glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SSEnv, "taking localAddr[%s, %s] to bind"), pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.SessionInterfaceBind.c_str(), _bindAddr.getHostAddress());	
	}

	if (pC2SSBaseConfig->_videoServerHolder.sessionRenewInterval > 10)
		_sessTimeout = pC2SSBaseConfig->_videoServerHolder.sessionRenewInterval *1000;

	glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SSEnv, "taking session timeout[%d]msec"), _sessTimeout);	

	streamsmithConfig.iSupportPlaylist = LIB_SUPPORT_NORMAL_STREAM;
}

void C2SSEnv::start()
{
	_bRun = true;
}

void C2SSEnv::uninit()
{
	_bRun = false;
}

void C2SSEnv::OnContentAttributes(const std::string& contentName, const ZQTianShan::C2SS::C2ContentQueryBind::ContentAttr& cattr)
{
	//glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SSEnv, "OnContentAttributes() enter contentName[%s]."), contentName.c_str());	
	SessionIDList  sessList ;
	{
		ZQ::common::MutexGuard g(_lkInterestMap);
		sessList = _interestMap[contentName];
	}
	SessionIDList::iterator iter = sessList.begin();
	for (; iter != sessList.end(); iter ++)
	{
		std::string sessId = *iter;
		{
			ZQ::common::MutexGuard g(_lkCAOfSessionMap);
			CAList& calist = _CAOfSessionMap[sessId];
			CAList::iterator iterCA = calist.begin();
			for ( ; iterCA != calist.end(); iterCA ++)
			{
				if (iterCA->name == contentName)
					iterCA->stampAsOf = 1;
			}
		}
		_wakeup.signal();
	}
	glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SSEnv, "OnContentAttributes() set the content[%s] stampAsOf successful."), contentName.c_str());
}

void C2SSEnv::OnError(int errCode, const std::string& errMsg)
{

}

bool C2SSEnv::c2LocateReady(const std::string& sessId)
{
	bool complete = true;
	ZQ::common::MutexGuard g(_lkCAOfSessionMap);
	CAOFSessionMAP::iterator itFind = _CAOfSessionMap.find(sessId);
	if (_CAOfSessionMap.end() != itFind)
	{
		CAList calist = itFind->second;
		CAList::iterator iterCA = calist.begin();
		for ( ; iterCA != calist.end(); iterCA ++)
		{
			if(iterCA->stampAsOf != 1)
				complete = false;
		}
	}
	return complete;
}

void C2SSEnv::deleteSessFromMap(const std::string& sessId, const std::string& contentName)
{
	ZQ::common::MutexGuard g(_lkInterestMap);
	SessionIDList& sessList = _interestMap[contentName];
	sessList.erase(std::remove(sessList.begin(), sessList.end(), sessId), sessList.end());
}
	}//namespace C2SS
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

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("C2SS",1001,szBuf );
	}

	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if (it==itResMap->second.resourceData.end())
	{
		char szBuf[1024];
		snprintf(szBuf, sizeof(szBuf), "GetResourceMapData() value with key=%s not found", strkey.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("C2SS",1002,szBuf);
	}

	return it->second;
}

//add for CCUR
void calcScale(float& fScale, SsContext& ctx )
{
	if( pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetEnable <= 0 )
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
		scaleSet = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetForwardSet;
	else
		scaleSet = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetBackwardSet;

	if (scaleSet.size() <= 0 )
	{
		ctx.updateContextProperty(SPEED_LASTIDX, -1);
		ctx.updateContextProperty(SPEED_LASTDIR, requestDirection);
		return ;
	}

	if (pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.EnableFixedSpeedLoop >= 1 )//EnableFixedSpeedLoop
	{
		iLastIndex = ( lastDirection * requestDirection ) < 0 ? 0 : ( (++iLastIndex) % (int) scaleSet.size() );
	}
	else
	{
		iLastIndex = ( lastDirection * requestDirection ) < 0 ? 0 : (++iLastIndex);
		if( iLastIndex >= (int) scaleSet.size() ) 
		{
			//reach the end of the fix speed set
			ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(glog, "C2SS", 0, "session[%s] can't loop the speed, we already reach the end of speed set", ctx.id().c_str()  );
		}
	}

	float fOldScale = fScale;

	fScale = scaleSet[iLastIndex];

	//reset last scale status
	ctx.updateContextProperty(SPEED_LASTIDX, iLastIndex);
	ctx.updateContextProperty(SPEED_LASTDIR, requestDirection);

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2SS, "calcScale() session[%s] convert speed from [%f] to [%f] according to fix speed set"), ctx.id().c_str(), fOldScale, fScale);

	return;
}

bool	SsServiceImpl::listAllReplicas( SsServiceImpl& ss, OUT SsReplicaInfoS& infos )
{
	SsReplicaInfo replica;
	replica.bHasPorts = false;
	replica.streamerType = "C2SS";
	replica.bStreamReplica = true;
	replica.replicaState = TianShanIce::stInService;
	replica.replicaId = "VSOP";
	infos.insert(std::make_pair("C2SS", replica));

	return  true;
}

bool SsServiceImpl::allocateStreamResource(	SsServiceImpl& ss, 
										   IN const std::string& streamerId,
										   IN const std::string& portId,
										   IN const TianShanIce::SRM::ResourceMap&	resource )
{
	return  true;
}

bool SsServiceImpl::releaseStreamResource( SsServiceImpl& ss, SsContext& sessCtx, IN const std::string& streamerReplicaId, OUT TianShanIce::Properties& feedback)
{
	return true;
}

int32	SsServiceImpl::doValidateItem(  SsServiceImpl& ss, 
									  IN SsContext& ctx,
									  INOUT TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SS, "doValidateItem() sess[%s]  content [%s] enter."), ctx.id().c_str(), info.contentName.c_str());
	//set content as InService, so that no more GET_PARAMETER will be used to get the content play duration
	Ice::Int state = 1;
	ZQTianShan::Util::updateValueMapData( info.privateData , "StreamService.ContentInService" , state );
	ZQTianShan::C2SS::C2SSEnv* c2Env = dynamic_cast<ZQTianShan::C2SS::C2SSEnv*> (ss.env);
	
	//c2Env->_c2ContentAttrCache->setUri(/*ss.getAdminUri()*/ss.strAdminUrl);

	if ( !c2Env)
	{
		  glog(::ZQ::common::Log::L_ERROR, CLOGFMT(C2SS, "doValidateItem() sess[%s] the env is null ."), ctx.id().c_str());
		  return ERR_RETURN_SERVER_ERROR;
	}
	/*
	//the sync call not needed
	if (1 == pC2SSBaseConfig->_syncCall)
	{

	ZQTianShan::C2SS::C2ContentQueryBind::ContentAttr contentAttr;
	//	c2Env->_c2ContentAttrCache->lookupContent_sync(contentAttr, info.contentName, pC2SSBaseConfig->_syncTimeOut);
	if( !c2Env->_c2ContentAttrCache->lookupContent_sync(contentAttr, info.contentName, pC2SSBaseConfig->_syncTimeOut))
	return ERR_RETURN_SERVER_ERROR;
	}
	else
	{
	*/
	std::string sessionId = ctx.id();
	ZQTianShan::C2SS::C2SSEnv::SessionIDList idList;
	{
		  ZQ::common::MutexGuard g(c2Env->_lkInterestMap);
		  ZQTianShan::C2SS::C2SSEnv::InterestMAP::iterator  iterC = c2Env->_interestMap.find(info.contentName);
		  if (c2Env->_interestMap.end() != iterC)
				iterC->second.push_back(sessionId);
		  else
		  {
				idList.push_back(sessionId);
				c2Env->_interestMap[info.contentName] = idList;
		  }
	}

	ZQTianShan::C2SS::C2SSEnv::CA	ca;
	ca.name = info.contentName;
	ca.stampAsOf = -1;
	ZQTianShan::C2SS::C2SSEnv::CAList caList;
	{
		  ZQ::common::MutexGuard g(c2Env->_lkCAOfSessionMap);
		  ZQTianShan::C2SS::C2SSEnv::CAOFSessionMAP::iterator iterS = c2Env->_CAOfSessionMap.find(sessionId);
		  if (c2Env->_CAOfSessionMap.end() != iterS)
		  {
				iterS->second.push_back(ca);
		  }
		  else
		  {
				caList.push_back(ca);
				c2Env->_CAOfSessionMap[sessionId] = caList;
		  }
	}
	c2Env->_c2ContentAttrCache->lookupContent_async(*c2Env, info.contentName);
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

	glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SS, "doCommit() stream[%s]"), ident.name.c_str());

	::TianShanIce::Transport::PathTicketPrx _pathTicket = ss.getPathTicket(ident.name);
	if (!_pathTicket)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(C2SS, "stream[%s] null PathTicket bound"), ident.name.c_str());
		return ERR_RETURN_SERVER_ERROR;
	}

	// read the PathTicket
	::std::string strTicket = ss.env->getCommunicator()->proxyToString(_pathTicket);
	glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SS, "doCommit() stream[%s] reading PathTicket[%s]"), ident.name.c_str(), strTicket.c_str());

	::TianShanIce::ValueMap pMap = _pathTicket->getPrivateData();
	::TianShanIce::SRM::ResourceMap resMap = _pathTicket->getResources();
	::TianShanIce::Transport::StorageLinkPrx storageLink = _pathTicket->getStorageLink();
	std::string	storageLinkType = storageLink->getType();
	std::string	streamLinkType = _pathTicket->getStreamLink()->getType();
	dest = "dvbc://@";
	if (pC2SSBaseConfig->_videoServerHolder.bSeperateIPStreaming && std::string::npos == streamLinkType.find("DVBC")) // ip streaming
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
			glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid dest mac type or no data"), ident.name.c_str(), strTicket.c_str());
			return ERR_RETURN_INVALID_PARAMATER;
		}

		strClient = valDestMac.strs[0];
	}
	catch (TianShanIce::InvalidParameter&)
	{
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] no dest mac information"), ident.name.c_str(), strTicket.c_str());
		return ERR_RETURN_INVALID_PARAMATER;
	}

	// 2. destination IP
	try
	{
		valDestAddr = GetResourceMapData (requestResources,TianShanIce::SRM::rtEthernetInterface, "destIP");
		if (valDestAddr.type != TianShanIce::vtStrings || valDestAddr.strs.size () == 0)
		{
			glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid destAddress type or no data"), ident.name.c_str(), strTicket.c_str());
			return ERR_RETURN_INVALID_PARAMATER;
		}

		dest += valDestAddr.strs[0];
	}
	catch (TianShanIce::InvalidParameter&)
	{
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid destAddress type or no data"), ident.name.c_str(), strTicket.c_str());
		return ERR_RETURN_INVALID_PARAMATER;
	}

	//get destination port
	try
	{
		valDestPort = GetResourceMapData(requestResources,TianShanIce::SRM::rtEthernetInterface,"destPort");
		if ( valDestPort.type != TianShanIce::vtInts || valDestPort.ints.size() == 0 )
		{
			glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid destPort type or no data"), ident.name.c_str(), strTicket.c_str());
			return ERR_RETURN_INVALID_PARAMATER;
		}

		::std::stringstream ss;
		ss << valDestPort.ints[0];
		dest = dest + ":" + ss.str();
	}
	catch (TianShanIce::InvalidParameter&)
	{
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid destPort type or no data"), ident.name.c_str(), strTicket.c_str());
		return ERR_RETURN_INVALID_PARAMATER;
	}

	//get bandwidth
	try
	{
		valBandwidth = GetResourceMapData(requestResources, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth");
		if (valBandwidth.type != TianShanIce::vtLongs || valBandwidth.lints.size () == 0 )
		{
			glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid bandwidth type or no data"), ident.name.c_str(), strTicket.c_str());
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
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid bandwidth type or no data"), ident.name.c_str(), strTicket.c_str());
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
		glog(::ZQ::common::Log::L_WARNING, CLOGFMT(C2SS, "doCommit() stream[%s]ticket[%s] invalid SOP"), ident.name.c_str(), strTicket.c_str());
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
		if (!pC2SSBaseConfig->_videoServerHolder.libraryVolume.empty())
			volumeName = pC2SSBaseConfig->_videoServerHolder.libraryVolume;
		else
			volumeName = "library";
	}
	else
	{
		if( storageNetId.empty() )
			volumeName = pC2SSBaseConfig->_videoServerHolder.contentInterfaceHolder.vols[0].targetName;
		else
			volumeName = storageNetId;
	}

	glog(::ZQ::common::Log::L_INFO, CLOGFMT(C2SS, "Stream[%s] read ticket[%s]: dest[%s], destMAC[%s], bandwidth[%s], SOP[%s], volume[%s]"), 
		ident.name.c_str(), strTicket.c_str(), dest.c_str(), strClient.c_str(), bandwidth.c_str(), sop_name.c_str(), volumeName.c_str());
	
	bool loopWhile = true;
	ZQTianShan::C2SS::C2SSEnv* c2Env = dynamic_cast<ZQTianShan::C2SS::C2SSEnv*> (ss.env);
	//glog(::ZQ::common::Log::L_INFO, CLOGFMT(C2SS, "doCommit() sess[%s] try to get Atrr."), ctx.id().c_str());
	/*
	if (1 == pC2SSBaseConfig->_syncCall)
	{
	for (PlaylistItemSetupInfos::iterator iter = items.begin(); iter != items.end(); iter++)
	{
	ZQTianShan::C2SS::C2ContentQueryBind::ContentAttr c2Attr;
	if( !(c2Env->_c2ContentAttrCache->getAttr(iter->contentName, c2Attr)))
	{
	glog(::ZQ::common::Log::L_ERROR, CLOGFMT(C2SS, "doCommit() can not find attr from cache with  content[%s]."), iter->contentName.c_str());
	return ERR_RETURN_SERVER_ERROR;
	}
	TianShanIce::ValueMap privateData;
	ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_AssetID, c2Attr.assetId.c_str());
	ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ProviderID, c2Attr.providerId.c_str());
	ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ExtName, c2Attr.mainFileExtName.c_str());
	ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_MuxBitrate, c2Attr.muxBitrate);
	ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_PlayTime, c2Attr.playTime_msec);
	ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_OpenForWrite, c2Attr.isPWE);
	ZQTianShan::C2SS::C2ContentQueryBind::AttrMap::iterator iterAttr = c2Attr.attrs.begin();
	for (; iterAttr != c2Attr.attrs.end(); iterAttr ++)
	ZQTianShan::Util::updateValueMapData(privateData, iterAttr->first, iterAttr->second.c_str());
	iter->privateData = privateData;
	}
	}
	else
	{
	*/
	while(loopWhile)
	{
		  // TEST IF the CA of this session has been ready
		  // commit the SS if true, otherwise continue to wait 
		  if(c2Env->c2LocateReady(ctx.id()))
		  {
				glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(C2SS, "doCommit() all of the content in sess[%s] is ready."), ctx.id().c_str());
				ZQTianShan::C2SS::C2SSEnv::CAList cAlist ;
				{
					  ZQ::common::MutexGuard g(c2Env->_lkCAOfSessionMap);
					  ZQTianShan::C2SS::C2SSEnv::CAOFSessionMAP::iterator iterSess = c2Env->_CAOfSessionMap.find(ctx.id());
					  if (c2Env->_CAOfSessionMap.end() != iterSess)
					  {
							cAlist = iterSess->second;
							c2Env->_CAOfSessionMap.erase(iterSess);
					  }
				}
				//没有写在下面循环中是因为：下面循环如果出错可能直接退出了。
				for(ZQTianShan::C2SS::C2SSEnv::CAList::iterator iterDe = cAlist.begin(); iterDe != cAlist.end(); iterDe++)
					  c2Env->deleteSessFromMap(ctx.id(), iterDe->name);

				for(ZQTianShan::C2SS::C2SSEnv::CAList::iterator iterca = cAlist.begin(); iterca != cAlist.end(); iterca++)
				{
					  for (PlaylistItemSetupInfos::iterator iter = items.begin(); iter != items.end(); iter++)
					  {
							if (iterca->name == iter->contentName)
							{
								  ZQTianShan::C2SS::C2ContentQueryBind::ContentAttr c2Attr;
								  if( !(c2Env->_c2ContentAttrCache->getAttr(iter->contentName, c2Attr)))
								  {
										glog(::ZQ::common::Log::L_ERROR, CLOGFMT(C2SS, "doCommit() can not find attr from cache with  content[%s]."), iter->contentName.c_str());
										return ERR_RETURN_SERVER_ERROR;
								  }
								  TianShanIce::ValueMap privateData;
								  ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_AssetID, c2Attr.assetId.c_str());
								  ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ProviderID, c2Attr.providerId.c_str());
								  ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_ExtName, c2Attr.mainFileExtName.c_str());
								  ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_MuxBitrate, c2Attr.muxBitrate);
								  ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_PlayTime, c2Attr.playTime_msec);
								  ZQTianShan::Util::updateValueMapData(privateData, C2CLIENT_OpenForWrite, c2Attr.isPWE);
								  ZQTianShan::C2SS::C2ContentQueryBind::AttrMap::iterator iterAttr = c2Attr.attrs.begin();
								  for (; iterAttr != c2Attr.attrs.end(); iterAttr ++)
										ZQTianShan::Util::updateValueMapData(privateData, iterAttr->first, iterAttr->second.c_str());
								  iter->privateData = privateData;
							}//if
					  }//for
				}//for
				loopWhile = false;
		  }//if (c2Env->c2LocateReady(ctx.id()))
		  else //
		  {
				SYS::SingleObject::STATE sigState = c2Env->_wakeup.wait(pC2SSBaseConfig->_c2ContentTimeout);
				if (SYS::SingleObject::SIGNALED == sigState)
				{
					  continue;
				}
				else
				{
					  glog(::ZQ::common::Log::L_ERROR, CLOGFMT(C2SS, "doCommit() sess[%s] failed with sess timeout."), ctx.id().c_str());
					  return ERR_RETURN_SERVER_ERROR;
				}
		  }
	}//while
	//}
	glog(::ZQ::common::Log::L_INFO, CLOGFMT(C2SS, "doCommit() sess[%s] successful."), ctx.id().c_str());
	
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

	if (timeOffset <1 && abs(scale) >1 && 0 != pC2SSBaseConfig->_videoServerHolder.vendor.compare("SeaChange"))
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