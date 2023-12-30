#include "GBVSSEnv.h"
#include "SsServiceImpl.h"
#include "GBVSSCfgLoader.h"
#include "IPathHelperObj.h"
#include "TsStreamer.h"
#include "Guid.h"
#include "GBVSSUtil.h"
#include "SystemUtils.h"
#include "ZQResource.h"
#include <TianShanIceHelper.h>
extern ZQTianShan::GBVSS::GBVSSBaseConfig::GBVSSHolder	*pGBVSSBaseConfig;
using namespace ZQ::common;

namespace ZQ {
namespace StreamService {

GBVSSEnv::GBVSSEnv(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog , 
				   ZQ::common::NativeThreadPool& rtspThpool, ZQ::common::NativeThreadPool& thrdPoolSvr)
	: SsEnvironment(mainLog, sessLog, thrdPoolSvr),
	thrdPoolRTSP(rtspThpool)
{
}

GBVSSEnv::~GBVSSEnv()
{
}

void GBVSSEnv::init()
{
    streamsmithConfig.iEnableSGScaleChangeEvent = pGBVSSBaseConfig->_postEvent.enableScaleChangeEvent;
    streamsmithConfig.iEnableSGStateChangeEvent = pGBVSSBaseConfig->_postEvent.enableStateChangeEvent;
    try
    {
        syncWatchDog = new SyncWatchDog(mainLogger);
    }
    catch (...)
    {
    }

    try
    {
        SessHisLogger.open(pGBVSSBaseConfig->_sessionHistory.path.c_str(),
            pGBVSSBaseConfig->_sessionHistory.level,
            ZQLOG_DEFAULT_FILENUM,
            pGBVSSBaseConfig->_sessionHistory.size,
            pGBVSSBaseConfig->_sessionHistory.bufferSize,
            pGBVSSBaseConfig->_sessionHistory.flushTimeout);
    }
    catch( const ZQ::common::FileLogException& ex)
    {
        glog(ZQ::common::Log::L_ERROR,
            CLOGFMT(GBVSSService,"failed to open session history log file[%s] because [%s]"),
            pGBVSSBaseConfig->_sessionHistory.path.c_str(),
            ex.what() );
    }

    int32 iMaxSessionGroupNumer = pGBVSSBaseConfig->_videoServer.SessionInterfaceMaxSessionGroup;
    int32 iMaxSessionPerGroup = pGBVSSBaseConfig->_videoServer.SessionInterfaceMaxSessionsPerGroup;;
    //initialize sessiongroup
    if (iMaxSessionGroupNumer < 1)
    {
        iMaxSessionGroupNumer = 2;
        glog(::ZQ::common::Log::L_WARNING, CLOGFMT(GBVSSEnv, "invalid MasSessionGroup number(%d), reset to 2"), iMaxSessionGroupNumer);	
    }

    std::stringstream ss;
    ss << pGBVSSBaseConfig->_videoServer.SessionInterfaceIp << ":" << pGBVSSBaseConfig->_videoServer.SessionInterfacePort;

    for (int i = 0; i < iMaxSessionGroupNumer; i++)
    {
        if (iMaxSessionPerGroup > 800)
        {
            glog(::ZQ::common::Log::L_WARNING, CLOGFMT(GBVSSEnv, "invalid MasSessionsPerGroup number(%d) exceed maximum, reset to 800"), iMaxSessionPerGroup);	
            iMaxSessionPerGroup = 800;
        }
        else if (iMaxSessionPerGroup < 200)
        {
            glog(::ZQ::common::Log::L_WARNING, CLOGFMT(GBVSSEnv,"invalid MasSessionsPerGroup number(%d) less than minimum, reset to 200"), iMaxSessionPerGroup);	
            iMaxSessionPerGroup = 200;
        }

        //		GBClient* client = new GBClient(*mainLogger, *mainThreadPool, bindAddress, std::string("rtsp://") + ss.str(), NULL, Log::L_DEBUG);
        //		GBVSSSessionGroup::addGBClient(std::string("rtsp://") + ss.str(), client);

        std::stringstream ssGroupName;
        ssGroupName << strNetId  << "." << (i+1);
        GBVSSSessionGroup* group = new GBVSSSessionGroup(*this, mainLogger, ssGroupName.str(), std::string("rtsp://") + ss.str(), iMaxSessionPerGroup, pGBVSSBaseConfig->_videoServer.streamSyncInterval*1000);
        if (syncWatchDog)
        {
            syncWatchDog->watch(group, pGBVSSBaseConfig->_videoServer.streamSyncInterval*1000);
        }
        glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(GBVSSEnv,"New SessionGroup [%s]"), ssGroupName.str().c_str());	
    }

    streamsmithConfig.iSupportPlaylist = LIB_SUPPORT_NORMAL_STREAM;
}

void GBVSSEnv::start()
{
    if (syncWatchDog)
    {
        syncWatchDog->start();
    }
}

void GBVSSEnv::uninit()
{
    try
    {
        if(syncWatchDog)
        {
            delete syncWatchDog;
            syncWatchDog = NULL;
        }
        GBVSSSessionGroup::clearSessionGroup();
    }
    catch (...)
    {
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//	SsServiceImpl interface
void* getVariantValue(::TianShanIce::Variant &val)
{
    switch (val.type)
    {
    case ::TianShanIce::vtStrings:
        return (void *)&(val.strs[0]);
    case ::TianShanIce::vtBin:
        return (void *)&(val.bin);
    case ::TianShanIce::vtFloats:
        return (void *)&(val.floats);
    case ::TianShanIce::vtInts:
        return (void *)&(val.ints);
    case ::TianShanIce::vtLongs:
        return (void *)&(val.lints[0]);
    default:
        return 0;
    }
}

void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal)
{
    ::TianShanIce::ValueMap::iterator iter;
    iter = pVal.find(str);
    if (iter == pVal.end())
        return 0;
    else
        return getVariantValue((*iter).second);
}

TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
    const TianShanIce::SRM::ResourceType& type,
    const std::string& strkey)
{
    TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
    if(itResMap==rcMap.end())
    {
        char szBuf[1024];
        snprintf(szBuf, sizeof(szBuf), "GetResourceMapData() type %d not found", type);

        ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("GBVSS",1001,szBuf );
    }
    TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
    if(it==itResMap->second.resourceData.end())
    {
        char szBuf[1024];
        snprintf(szBuf, sizeof(szBuf), "GetResourceMapData() value with key=%s not found", strkey.c_str());
        ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("GBVSS",1002,szBuf);
    }
    return it->second;
}

bool waitSignal(GBSession::Ptr sess, uint32 index)
{
    SYS::SingleObject::STATE st = sess->_eventHandle._pHandle[index]->wait(pGBVSSBaseConfig->_videoServer.SessionInterfaceRequestTimeout);
    if (st == SYS::SingleObject::SIGNALED)
    {
        return true;
    }
    else if (st == SYS::SingleObject::TIMEDOUT)
        return false;
    else
        return false;
}

//add for CCUR
void calcScale( float& fScale , SsContext& ctx )
{
    if( pGBVSSBaseConfig->_videoServer.FixedSpeedSetEnable <= 0 )
    {//keep current scale
        return;
    }

    //detect request scale attribute
    int requestDirection = 0 ;
    if(  ( fScale - 1.0f ) > 0.0001f )
    {//fast forward
        requestDirection = 1;
    }
    else if( fScale < 0.0f )
    {//fast rewind
        requestDirection = -1;
    }
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
    {
        scaleSet = pGBVSSBaseConfig->_videoServer.FixedSpeedSetForwardSet;
    }
    else
    {
        scaleSet = pGBVSSBaseConfig->_videoServer.FixedSpeedSetBackwardSet;
    }
    if( scaleSet.size() <= 0 )
    {
        ctx.updateContextProperty(SPEED_LASTIDX, -1);
        ctx.updateContextProperty(SPEED_LASTDIR, requestDirection);
        return ;
    }

    if(pGBVSSBaseConfig->_videoServer.EnableFixedSpeedLoop >= 1 )//EnableFixedSpeedLoop
    {
        iLastIndex = ( lastDirection * requestDirection ) < 0 ? 0 : ( (++iLastIndex) % scaleSet.size() );
    }
    else
    {
        iLastIndex = ( lastDirection * requestDirection ) < 0 ? 0 : (++iLastIndex);
        if( iLastIndex >= (int)scaleSet.size() ) 
        {
            //reach the end of the fix speed set
            ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(glog, "GBVSS", 0, "session[%s] can't loop the speed, we already reach the end of speed set", ctx.id().c_str()  );
        }
    }

    float fOldScale = fScale;

    fScale = scaleSet[iLastIndex];

    //reset last scale status
    ctx.updateContextProperty(SPEED_LASTIDX, iLastIndex);
    ctx.updateContextProperty(SPEED_LASTDIR, requestDirection);

    glog(ZQ::common::Log::L_INFO,CLOGFMT(calcScale,"session[%s] conver speed from [%f] to [%f] according to fix speed set"), ctx.id().c_str(), fOldScale, fScale);

    return;
}

int32	HandleRtspCode(GBSession::Ptr sess, std::string cmd)
{
    if (sess->_resultCode >= RTSPSink::rcOK && sess->_resultCode < 300)
    {
        return ERR_RETURN_SUCCESS;
    }

    switch (sess->_resultCode)
    {
        //withdrew case 400: // 400 was due to some ease-made NGOD pumper error, and assume our TEARDOWN request were always good 
    case RTSPSink::rcObjectNotFound:
    case RTSPSink::rcSessNotFound:
        if ("TEARDOWN" == cmd)
        {
            return ERR_RETURN_SUCCESS;
        }

        // NOTE: no "break;" here

    case RTSPSink::rcInternalError:
        if ("PLAY" == cmd || "PAUSE" == cmd)
        {
            std::string tempNotice = sess->_tianShanNotice;
            std::transform(tempNotice.begin(), tempNotice.end(), tempNotice.begin(), (int(*)(int)) toupper);

            // transfer VSTRM_GEN_CONTROL_PORT_SPEED to 503 Service Unavailable
            if (std::string::npos != tempNotice.find("VSTRM_GEN_CONTROL_PORT_SPEED"))
                sess->_resultCode = RTSPSink::rcServiceUnavail;
        }
        // be sure no 'break' here

    default:
        ZQTianShan::_IceThrow<TianShanIce::ServerError> (glog, "RtspProxying", sess->_resultCode, "doAction(%s) ODSess[%s] failed; error[%d %s]", cmd.c_str(), sess->guid(), sess->_resultCode, sess->_tianShanNotice.c_str());
    }
    return ERR_RETURN_SERVER_ERROR;
}

bool	SsServiceImpl::listAllReplicas( SsServiceImpl& ss, OUT SsReplicaInfoS& infos )
{
    SsReplicaInfo replica;
    replica.bHasPorts = false;
    replica.streamerType = "GBVSS";
    replica.bStreamReplica = true;
    replica.replicaState = TianShanIce::stInService;
    replica.replicaId = "VSOP";
    infos.insert(std::make_pair("GBVSS", replica));
    return  true;
}

bool SsServiceImpl::allocateStreamResource(	SsServiceImpl& ss, 
    IN const std::string& streamerId ,
    IN const std::string& portId ,
    IN const TianShanIce::SRM::ResourceMap&	resource )
{
    return  true;
}


bool SsServiceImpl::releaseStreamResource( SsServiceImpl& ss, SsContext& contextKey,
											IN const std::string& streamerReplicaId,
											OUT TianShanIce::Properties& feedback  )

{
    GBVSSSessionGroup* group = 0;
    GBSession::Ptr clientSession = 0;
    GBClient* client = 0;
    std::string ondemandname = contextKey.getContextProperty(ONDEMANDNAME_NAME);
    if(ondemandname.empty()) {
        glog(::ZQ::common::Log::L_WARNING, CLOGFMT(SsServiceImpl,"releaseStreamResource no OnDemandSessionId provided"));
        return false;
    }
    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"releaseStreamResource session(%s)"), ondemandname.c_str());
    group = GBVSSSessionGroup::findSessionGroup(contextKey.getContextProperty(SESSION_GROUP));
    if(group)
    {
        clientSession = group->lookupByOnDemandSessionId(ondemandname.c_str());
    } else {
        glog(::ZQ::common::Log::L_WARNING, CLOGFMT(SsServiceImpl,"releaseStreamResource session(%s) no SessionGroup provided"), ondemandname.c_str());
        return false;
    }

    if(!clientSession)
    {
        std::string sessionId = contextKey.getContextProperty(SESSION_ID);
        if(sessionId.empty()) {
            glog(::ZQ::common::Log::L_WARNING, CLOGFMT(SsServiceImpl,"releaseStreamResource session(%s) no SessionId provided"), ondemandname.c_str());
            return	false;
        }
        std::string controlUri = contextKey.getContextProperty(CONTROL_URI);
        if(controlUri.empty())
            controlUri = clientSession->getBaseURL();
        glog(::ZQ::common::Log::L_DEBUG, "fail to find session [%s, %s], need new one", ondemandname.c_str(), sessionId.c_str());
        std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
        clientSession = new GBSession((ss.env->getMainLogger()), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ondemandname.c_str());
        clientSession->setSessionId(sessionId);
        clientSession->setControlUri(controlUri);
        group->add(*clientSession);
    }

    client = group->getR2Client();
    if(!client)
    {
        glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"releaseStreamResource session(%s) failed to allocate GBClient"), ondemandname.c_str());
    }
    else
    {
        RTSPMessage::AttrMap     headers;
        headers.insert(std::make_pair("GlobalSession", contextKey.id()));
        int currentCSeq = client->sendTEARDOWN(*clientSession, NULL, headers);
        if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_WARNING, CLOGFMT(SsServiceImpl,"releaseStreamResource session(%s) failed to allocate resource for CSeq:%d"), ondemandname.c_str(), currentCSeq);
            clientSession->destroy();
            return false;
        }
        if (waitSignal(clientSession, currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doDestroy timeout"), contextKey.getContextProperty(ONDEMANDNAME_NAME).c_str(), currentCSeq);
            clientSession->_eventHandle.m_CloseEvent(currentCSeq);
            clientSession->destroy();
            return false;
        }
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        contextKey.updateContextProperty("TEARDOWN.StopNPT", clientSession->_stopNPT);
        if(pGBVSSBaseConfig->_sessionHistory.enable)
        {
            ZQ::StreamService::GBVSSEnv* pEnv = dynamic_cast<ZQ::StreamService::GBVSSEnv*>(ss.env);
            if (pEnv)
            {
                std::replace(clientSession->_sessionHistory.begin(), clientSession->_sessionHistory.end(), '\r', '.');
                std::replace(clientSession->_sessionHistory.begin(), clientSession->_sessionHistory.end(), '\n', '.');
                pEnv->SessHisLogger(Log::L_INFO, CLOGFMT(GBSession, "%s"), clientSession->_sessionHistory.c_str());
            }
        }
        if(clientSession->_resultCode == RTSPSink::rcOK || clientSession->_resultCode == RTSPSink::rcObjectNotFound || clientSession->_resultCode == RTSPSink::rcBadRequest || clientSession->_resultCode == RTSPSink::rcSessNotFound) // RTSPOK
        {
            glog(::ZQ::common::Log::L_WARNING, CLOGFMT(SsServiceImpl,"releaseStreamResource session(%s) TEARDOWN got %d"), ondemandname.c_str(), clientSession->_resultCode);
            clientSession->destroy();
        }
        return ERR_RETURN_SUCCESS == HandleRtspCode(clientSession, "TEARDOWN");
    }
    clientSession->destroy();
    return false;
}

int32	SsServiceImpl::doValidateItem(  SsServiceImpl& ss, 
    IN SsContext& ctx,
    INOUT TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
    return ERR_RETURN_SUCCESS;
}

int32 SsServiceImpl::doCommit(  SsServiceImpl& ss, 
    IN SsContext& ctx,								
    IN PlaylistItemSetupInfos& items,
    IN TianShanIce::SRM::ResourceMap& requestResources )
{
    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"enter doCommit"));
    std::string volumeName;
    std::string dest = "";
    std::string type = ""; //MP2T/DVBC/UDP as default
    std::string strClient = "";
    std::string bandwidth = "";
    std::string sop_name = "";
	std::string storageNetId;

    ::TianShanIce::Transport::PathTicketPrx _pathTicket = ss.getPathTicket(ctx.id());
    ::Ice::Identity ident;
    if(!_pathTicket)
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"Invalid path ticket proxy"));
		return ERR_RETURN_SERVER_ERROR;
	}

	//		ident = _pathTicket->getIdent();
	ident.name = ctx.id();
	// 		::TianShanIce::Transport::StorageLinkPrx sLink = _pathTicket->getStorageLink();
	// 		volumeName = sLink->getStorageInfo().netId;
	::TianShanIce::ValueMap pMap = _pathTicket->getPrivateData();
	::TianShanIce::SRM::ResourceMap resMap = _pathTicket->getResources();
	::std::string strTicket = ss.env->getCommunicator()->proxyToString(_pathTicket);
	::TianShanIce::Transport::StorageLinkPrx storageLink = _pathTicket->getStorageLink();
	std::string	storageType = storageLink->getType();
	std::string	streamLinkType = _pathTicket->getStreamLink()->getType();

	try
	{
		::std::stringstream ss;
		ZQTianShan::Util::getResourceData(requestResources, TianShanIce::SRM::rtEthernetInterface, "destMac", strClient);
		
		ZQTianShan::Util::getResourceData(requestResources, TianShanIce::SRM::rtEthernetInterface, "destIP", dest );
		if( streamLinkType.find("DVBC") == streamLinkType.npos)
			dest = "udp://@" + dest;
		else
			dest = "dvbc://@" + dest;

		Ice::Int destPort = 0;
		ZQTianShan::Util::getResourceData( requestResources, TianShanIce::SRM::rtEthernetInterface,"destPort", destPort);
		ss.str("");
		ss<<destPort;
		dest = dest + ":" + ss.str();

		Ice::Long bw = 0;
		ZQTianShan::Util::getResourceData( requestResources, TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth", bw);
		ss.str(""); ss << bw;
		bandwidth = ss.str();

		ZQTianShan::Util::getValueMapData(pMap,PathTicketPD_Field(sop_name),sop_name);

		ZQTianShan::Util::getResourceDataWithDefault(requestResources, TianShanIce::SRM::rtStorage, "NetworkId","", storageNetId );

		glog(ZQ::common::Log::L_INFO,CLOGFMT(SsServiceImpl,"get reqeust resources: dest[%s] bandwidth[%s] sop[%s] storageNetId[%s] storageType[%s]"),
			dest.c_str(), bandwidth.c_str(), sop_name.c_str(), storageNetId.c_str(), storageType.c_str() );
	}
	catch( const TianShanIce::InvalidParameter& ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(SsServiceImpl,"failed to get neccessary resource: %s"),ex.message.c_str() );
		return ERR_RETURN_INVALID_PARAMATER;
	}			
	
	if( storageType == "SeaChange.GBVSS.C2Transfer")
	{
		if (!pGBVSSBaseConfig->_videoServer.libraryVolume.empty())
		{
			volumeName = pGBVSSBaseConfig->_videoServer.libraryVolume;
		}
		else
		{
			volumeName = "library";
		}
	}
	else
	{
		if( storageNetId.empty() )
		{
			volumeName = pGBVSSBaseConfig->_videoServer.vols[0].targetName;
		}
		else
		{
			volumeName = storageNetId;
		}
	}
    
	glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"creating GBSession[%s] to dest[%s/%s] at [%s]bps"), ident.name.c_str(), dest.c_str(), strClient.c_str(), bandwidth.c_str());
	GBSession::Ptr clientSession = new GBSession(ss.env->getMainLogger(), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ident.name.c_str());

	clientSession->addTransportEx("bit_rate", bandwidth);
	if (strClient.length() >=12) // a valid MAC address must have 12+ charactors
		clientSession->addTransportEx("client", strClient);

    //char userAgent[256];
    //snprintf(userAgent, sizeof(userAgent), "TianShan/%d.%d; %s", ZQ_PRODUCT_VER_MAJOR, ZQ_PRODUCT_VER_MINOR, ZQ_INTERNAL_FILE_NAME); 
#define N2S2(x) #x
#define N2S(x) N2S2(x)

#define GB_USER_AGENT "TianShan/" N2S(ZQ_PRODUCT_VER_MAJOR) "." N2S(ZQ_PRODUCT_VER_MINOR) "; " ZQ_INTERNAL_FILE_NAME

	MAPSET(TianShanIce::Properties, clientSession->_props, "User-Agent", GB_USER_AGENT);
	// GlobalSession?
	MAPSET(TianShanIce::Properties, clientSession->_props, "GlobalSession", ident.name);
	// x-userID?
	MAPSET(TianShanIce::Properties, clientSession->_props, "x-userID", "a1b2c3");

	// bug#18135 to include Content-Type header in out-going request
	// need ssm_GBstb to add into res[rtServiceGroup]["descriptor"]
	try
	{
		TianShanIce::Variant sgDescptr = GetResourceMapData(requestResources, TianShanIce::SRM::rtServiceGroup, "descriptor");
		if ( sgDescptr.type == TianShanIce::vtStrings || sgDescptr.strs.size () > 0 )
		{
			std::string& strServiceGroupIP = sgDescptr.strs[0];
			if (!strServiceGroupIP.empty())
				MAPSET(TianShanIce::Properties, clientSession->_props, "Service-Group-IP", strServiceGroupIP);
		}
	}
	catch (...) {}

	try
	{
		TianShanIce::Variant sgUser = GetResourceMapData(requestResources, TianShanIce::SRM::rtServiceGroup, "user");
		if ( sgUser.type == TianShanIce::vtStrings || sgUser.strs.size () > 0 )
		{
			std::string& strV = sgUser.strs[0];
			if (!strV.empty())
				MAPSET(TianShanIce::Properties, clientSession->_props, "x-userID", strV);
		}
	}
	catch (...) {}

	ctx.updateContextProperty(SESSION_GROUP, clientSession->_groupName);
    ctx.updateContextProperty(VOLUME_NAME, clientSession->_volumeName);
    ctx.updateContextProperty(ONDEMANDNAME_NAME, ident.name);
    ctx.updateContextProperty(DESTINATION_NAME, dest);
    ctx.updateContextProperty(CONTROL_URI, clientSession->getBaseURL());

    // a=x-playlist-item: <contentID> <type> [bitrate] [ <range>][ tricks/[F][R][P] ]
    for (PlaylistItemSetupInfos::const_iterator iter = items.begin();
        iter != items.end(); iter++)
    {
        std::string contentType = "vod"; // vod/ad/npvr
        // no bitrate
        // range data
        char rangeBuf[64];
        if (iter->outTimeOffset > 0)
            snprintf(rangeBuf, sizeof(rangeBuf)-2, "%llx-%llx ", iter->inTimeOffset, iter->outTimeOffset);
        else
            snprintf(rangeBuf, sizeof(rangeBuf)-2, "%llx- ", iter->inTimeOffset);

        // restriction: tricks/FRP
        std::string restriction = "tricks/";
        if(iter->flags & TianShanIce::Streamer::PLISFlagNoFF)
            restriction += "F";
        if(iter->flags & TianShanIce::Streamer::PLISFlagNoRew)
            restriction += "R";
        if(iter->flags & TianShanIce::Streamer::PLISFlagNoPause)
            restriction += "P";
        std::stringstream ss;
// a=x-playlist-item: <contentID> <type> [bitrate] [ <range>][ tricks/[F][R][P] ]
        ss << "x-playlist-item: " << iter->contentName << " " << contentType << " " << rangeBuf;
        if(restriction.size() > 7)
            ss << " " << restriction;

        std::string _tmp = ss.str();
        clientSession->addSDPValue_a(_tmp);
    }
	std::string zero("0");
	clientSession->setSDPValue_v(zero);
	std::string userName("-"), sessVersion(""), sessNetType("IN"), sessAddrType("IP4"), sessAddr("0.0.0.0");
	clientSession->setSDPValue_o(userName, ident.name, sessVersion, sessNetType, sessAddrType, sessAddr);
	clientSession->setSDPValue_t(zero, zero);
	clientSession->setSDPValue_c(sessNetType, sessAddrType,  sessAddr, sessVersion);

	ZQ::common::RTSPSession::MediaPropertyData mpd;
	mpd.mediaType = "video";
	mpd.mediaPort = 0;
	mpd.mediaTransport = "udp";
	mpd.mediaFmt = "MP2T";
	clientSession->addSDPValue_m(mpd);
	
    GBClient* client = clientSession->getR2Client(); 
    if(client)
    {
        int currentCSeq = client->sendSETUP(*clientSession, NULL, NULL, clientSession->_props);
        if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
            return ERR_RETURN_SERVER_ERROR;
        if (waitSignal(clientSession, currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doCommit timeout"), ident.name.c_str(), currentCSeq);
            clientSession->_eventHandle.m_CloseEvent(currentCSeq);
            return ERR_RETURN_SERVER_ERROR;
        }
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        ctx.updateContextProperty(SESSION_ID, clientSession->getSessionId());	// update session id from server
        ctx.updateContextProperty(CONTROL_URI, clientSession->controlUri());
        ctx.updateContextProperty("SETUP.ControlSession", clientSession->_controlSession);
        ctx.updateContextProperty("SETUP.GlobalSession", ident.name);
        if(!clientSession->_primartItemNPT.empty())
            ctx.updateContextProperty("sys.primaryItemNPT", clientSession->_primartItemNPT);
        if (clientSession->_resultCode == RTSPSink::rcOK)
        {
            ctx.updateContextProperty(SETUP_TIMESTAMP, clientSession->getStampSetup());
        }
        return HandleRtspCode(clientSession, "SETUP");
    }
    else
    {
        glog(::ZQ::common::Log::L_ERROR, "SsServiceImpl::doCommit() failed to allocate GBClient");
        return ERR_RETURN_INVALID_PARAMATER;
    }

    return ERR_RETURN_INVALID_PARAMATER;
}


int32 SsServiceImpl::doLoad(	SsServiceImpl& ss,
    IN SsContext& contextKey,								
    IN const TianShanIce::Streamer::PlaylistItemSetupInfo& itemInfo, 
    IN int64 timeoffset,
    IN float scale,								
    INOUT StreamParams& ctrlParams,
    OUT std::string&	streamId )
{
    streamId = contextKey.getContextProperty(ONDEMANDNAME_NAME);
    if(streamId.empty())
        return ERR_RETURN_INVALID_PARAMATER;
    else
        return ERR_RETURN_SUCCESS;
}

int32 SsServiceImpl::doPlay(	SsServiceImpl& ss ,
    SsContext& contextKey,							   
    IN const std::string& streamId,
    IN int64 timeOffset,
    IN float scale,							   
    INOUT StreamParams& ctrlParams )
{	
    GBVSSSessionGroup* group = 0;
    GBSession::Ptr clientSession = 0;
    GBClient* client = 0;
    std::string ondemandname = contextKey.getContextProperty(ONDEMANDNAME_NAME);
    if(ondemandname.empty())
        return ERR_RETURN_INVALID_PARAMATER;
    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"doPlay session(%s) stream(%s) range(%lld) speed(%f)"), ondemandname.c_str(), streamId.c_str(), timeOffset, scale);
    group = GBVSSSessionGroup::findSessionGroup(contextKey.getContextProperty(SESSION_GROUP));
    if(group)
    {
        clientSession = group->lookupByOnDemandSessionId(ondemandname.c_str());
    }
    else
        return ERR_RETURN_INVALID_PARAMATER;

    if(!clientSession)
    {
        std::string sessionId = contextKey.getContextProperty(SESSION_ID);
        if(sessionId.empty())
            return	ERR_RETURN_INVALID_PARAMATER;
        std::string controlUri = contextKey.getContextProperty(CONTROL_URI);
        if(controlUri.empty())
            controlUri = clientSession->getBaseURL();
        glog(::ZQ::common::Log::L_DEBUG, "fail to find session [%s, %s], need new one", ondemandname.c_str(), sessionId.c_str());
        std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
        clientSession = new GBSession(ss.env->getMainLogger(), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ondemandname.c_str());
        clientSession->setSessionId(sessionId);
        clientSession->setControlUri(controlUri);
        group->add(*clientSession);
    }

    client = group->getC1Client(clientSession->controlUri()); 
    if(!client)
    {
        glog(::ZQ::common::Log::L_ERROR, "failed to allocate GBClient to Server %s", clientSession->controlUri());
    }
    else
    {
        RTSPMessage::AttrMap props;
        props.insert(std::make_pair("GlobalSession", contextKey.id()));
        calcScale(scale, contextKey);
        double start = (ctrlParams.mask & MASK_TIMEOFFSET) ? static_cast<double>(timeOffset) / 1000.0f : -1.0f; // if the mask of timeoffset is not set, play from now(-1.0)
        int currentCSeq = client->sendPLAY(*clientSession, start, -1.0f, scale, NULL, props);
        if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
            return ERR_RETURN_SERVER_ERROR;
        if (waitSignal(clientSession, currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doPlay timeout"), contextKey.getContextProperty(ONDEMANDNAME_NAME).c_str(), currentCSeq);
            clientSession->_eventHandle.m_CloseEvent(currentCSeq);
            return ERR_RETURN_SERVER_ERROR;
        }
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        if(clientSession->_resultCode == RTSPSink::rcOK) // RTSPOK
        {
            GBSession::StreamInfos info = clientSession->getInfos();
            ctrlParams.timeoffset = info.timeoffset;
            ctrlParams.duration = info.duration;
            ctrlParams.scale = info.scale;
            ctrlParams.streamState = TianShanIce::Streamer::stsStreaming;
        }
        return HandleRtspCode(clientSession, "PLAY");
    }
    return ERR_RETURN_INVALID_PARAMATER;
}


int32 SsServiceImpl::doPause(	SsServiceImpl& ss , SsContext& contextKey, IN const std::string& streamId , INOUT StreamParams& ctrlParams )
{
    GBVSSSessionGroup* group = 0;
    GBSession::Ptr clientSession = 0;
    GBClient* client = 0;
    std::string ondemandname = contextKey.getContextProperty(ONDEMANDNAME_NAME);
    if(ondemandname.empty())
        return ERR_RETURN_INVALID_PARAMATER;
    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"doPause session(%s) stream(%s)"), ondemandname.c_str(), streamId.c_str());
    group = GBVSSSessionGroup::findSessionGroup(contextKey.getContextProperty(SESSION_GROUP));
    if(group)
    {
        clientSession = group->lookupByOnDemandSessionId(ondemandname.c_str());
    }
    else
        return ERR_RETURN_INVALID_PARAMATER;

    if(!clientSession)
    {
        std::string sessionId = contextKey.getContextProperty(SESSION_ID);
        if(sessionId.empty())
            return	ERR_RETURN_INVALID_PARAMATER;
        std::string controlUri = contextKey.getContextProperty(CONTROL_URI);
        if(controlUri.empty())
            controlUri = clientSession->getBaseURL();
        glog(::ZQ::common::Log::L_DEBUG, "fail to find session [%s, %s], need new one", ondemandname.c_str(), sessionId.c_str());
        std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
        clientSession = new GBSession(ss.env->getMainLogger(), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ondemandname.c_str());
        clientSession->setSessionId(sessionId);
        clientSession->setControlUri(controlUri);
        group->add(*clientSession);
    }

    client = group->getC1Client(clientSession->controlUri()); 
    if(!client)
    {
        glog(::ZQ::common::Log::L_ERROR, "failed to allocate GBClient to Server %s", clientSession->controlUri());
    }
    else
    {
        RTSPMessage::AttrMap props;
        props.insert(std::make_pair("GlobalSession", contextKey.id()));
        int currentCSeq = client->sendPAUSE(*clientSession, NULL, props);
        if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
            return ERR_RETURN_SERVER_ERROR;
        if (waitSignal(clientSession, currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doPause timeout"), contextKey.getContextProperty(ONDEMANDNAME_NAME).c_str(), currentCSeq);
            clientSession->_eventHandle.m_CloseEvent(currentCSeq);
            return ERR_RETURN_SERVER_ERROR;
        }
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        if(clientSession->_resultCode == RTSPSink::rcOK) // RTSPOK
        {
            GBSession::StreamInfos info = clientSession->getInfos();
            ctrlParams.timeoffset = info.timeoffset;
            ctrlParams.duration = info.duration;
            ctrlParams.scale = info.scale;
            ctrlParams.streamState = TianShanIce::Streamer::stsPause;
        }
        return HandleRtspCode(clientSession, "PAUSE");
    }
    return ERR_RETURN_INVALID_PARAMATER;
}


int32 SsServiceImpl::doResume(  SsServiceImpl& ss , SsContext& contextKey , IN const std::string& streamId ,INOUT StreamParams& ctrlParams )
{
    GBVSSSessionGroup* group = 0;
    GBSession::Ptr clientSession = 0;
    GBClient* client = 0;
    std::string ondemandname = contextKey.getContextProperty(ONDEMANDNAME_NAME);
    if(ondemandname.empty())
        return ERR_RETURN_INVALID_PARAMATER;
    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"doResume session(%s) stream(%s)"), ondemandname.c_str(), streamId.c_str());
    group = GBVSSSessionGroup::findSessionGroup(contextKey.getContextProperty(SESSION_GROUP));
    if(group)
    {
        clientSession = group->lookupByOnDemandSessionId(ondemandname.c_str());
    }
    else
        return ERR_RETURN_INVALID_PARAMATER;

    if(!clientSession)
    {
        std::string sessionId = contextKey.getContextProperty(SESSION_ID);
        if(sessionId.empty())
            return	ERR_RETURN_INVALID_PARAMATER;
        std::string controlUri = contextKey.getContextProperty(CONTROL_URI);
        if(controlUri.empty())
            controlUri = clientSession->getBaseURL();
        glog(::ZQ::common::Log::L_DEBUG, "fail to find session [%s, %s], need new one", ondemandname.c_str(), sessionId.c_str());
        std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
        clientSession = new GBSession(ss.env->getMainLogger(), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ondemandname.c_str());
        clientSession->setSessionId(sessionId);
        clientSession->setControlUri(controlUri);
        group->add(*clientSession);
    }

    client = group->getC1Client(clientSession->controlUri()); 
    if(!client)
    {
        glog(::ZQ::common::Log::L_ERROR, "failed to allocate GBClient to Server %s", clientSession->controlUri());
    }
    else
    {
        RTSPMessage::AttrMap props;
        props.insert(std::make_pair("GlobalSession", contextKey.id()));
        int currentCSeq = client->sendPLAY(*clientSession, -1.0f, -1.0f, 1.0f, NULL, props);
        if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
            return ERR_RETURN_SERVER_ERROR;
        if (waitSignal(clientSession, currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doPause timeout"), contextKey.getContextProperty(ONDEMANDNAME_NAME).c_str(), currentCSeq);
            clientSession->_eventHandle.m_CloseEvent(currentCSeq);
            return ERR_RETURN_SERVER_ERROR;
        }
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        if(clientSession->_resultCode == RTSPSink::rcOK) // RTSPOK
        {
            GBSession::StreamInfos info = clientSession->getInfos();
            ctrlParams.timeoffset = info.timeoffset;
            ctrlParams.duration = info.duration;
            ctrlParams.scale = info.scale;
            ctrlParams.streamState = TianShanIce::Streamer::stsStreaming;
        }
        return HandleRtspCode(clientSession, "PLAY");
    }
    return ERR_RETURN_INVALID_PARAMATER;
}

int32 SsServiceImpl::doReposition( SsServiceImpl& ss , SsContext& contextKey,IN const std::string& streamId ,
    IN int64 timeOffset, IN const float& scale, INOUT StreamParams& ctrlParams )
{
    GBVSSSessionGroup* group = 0;
    GBSession::Ptr clientSession = 0;
    GBClient* client = 0;
    std::string ondemandname = contextKey.getContextProperty(ONDEMANDNAME_NAME);
    if(ondemandname.empty())
        return ERR_RETURN_INVALID_PARAMATER;
    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"doReposition session(%s) stream(%s) range(%lld) speed(%f)"), ondemandname.c_str(), streamId.c_str(), timeOffset, scale);
    group = GBVSSSessionGroup::findSessionGroup(contextKey.getContextProperty(SESSION_GROUP));
    if(group)
    {
        clientSession = group->lookupByOnDemandSessionId(ondemandname.c_str());
    }
    else
        return ERR_RETURN_INVALID_PARAMATER;

    if(!clientSession)
    {
        std::string sessionId = contextKey.getContextProperty(SESSION_ID);
        if(sessionId.empty())
            return	ERR_RETURN_INVALID_PARAMATER;
        std::string controlUri = contextKey.getContextProperty(CONTROL_URI);
        if(controlUri.empty())
            controlUri = clientSession->getBaseURL();
        glog(::ZQ::common::Log::L_DEBUG, "fail to find session [%s, %s], need new one", ondemandname.c_str(), sessionId.c_str());
        std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
        clientSession = new GBSession(ss.env->getMainLogger(), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ondemandname.c_str());
        clientSession->setSessionId(sessionId);
        clientSession->setControlUri(controlUri);
        group->add(*clientSession);
    }

    client = group->getC1Client(clientSession->controlUri()); 
    if(!client)
    {
        glog(::ZQ::common::Log::L_ERROR, "failed to allocate GBClient to Server %s", clientSession->controlUri());
    }
    else
    {
        RTSPMessage::AttrMap props;
        props.insert(std::make_pair("GlobalSession", contextKey.id()));
        int currentCSeq = client->sendPLAY(*clientSession, ((float)timeOffset)/1000, -1.0f, scale, NULL, props);
        if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
            return ERR_RETURN_SERVER_ERROR;
        if (waitSignal(clientSession, currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doReposition timeout"), contextKey.getContextProperty(ONDEMANDNAME_NAME).c_str(), currentCSeq);
            clientSession->_eventHandle.m_CloseEvent(currentCSeq);
            return ERR_RETURN_SERVER_ERROR;
        }
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        if(clientSession->_resultCode == RTSPSink::rcOK) // RTSPOK
        {
            GBSession::StreamInfos info = clientSession->getInfos();
            ctrlParams.timeoffset = info.timeoffset;
            ctrlParams.duration = info.duration;
            ctrlParams.scale = info.scale;
            ctrlParams.streamState = TianShanIce::Streamer::stsStreaming;
        }
        return HandleRtspCode(clientSession, "PLAY");
    }
    return ERR_RETURN_INVALID_PARAMATER;
}

int32 SsServiceImpl::doChangeScale(	 SsServiceImpl& ss , SsContext& contextKey, IN const std::string& streamId ,								
    IN float newScale, INOUT StreamParams& ctrlParams )
{
    GBVSSSessionGroup* group = 0;
    GBSession::Ptr clientSession = 0;
    GBClient* client = 0;
    std::string ondemandname = contextKey.getContextProperty(ONDEMANDNAME_NAME);
    if(ondemandname.empty())
        return ERR_RETURN_INVALID_PARAMATER;
    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"doChangeScale session(%s) stream(%s) speed(%f)"), ondemandname.c_str(), streamId.c_str(), newScale);
    group = GBVSSSessionGroup::findSessionGroup(contextKey.getContextProperty(SESSION_GROUP));
    if(group)
    {
        clientSession = group->lookupByOnDemandSessionId(ondemandname.c_str());
    }
    else
        return ERR_RETURN_INVALID_PARAMATER;

    if(!clientSession)
    {
        std::string sessionId = contextKey.getContextProperty(SESSION_ID);
        if(sessionId.empty())
            return	ERR_RETURN_INVALID_PARAMATER;
        std::string controlUri = contextKey.getContextProperty(CONTROL_URI);
        if(controlUri.empty())
            controlUri = clientSession->getBaseURL();
        glog(::ZQ::common::Log::L_DEBUG, "fail to find session [%s, %s], need new one", ondemandname.c_str(), sessionId.c_str());
        std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
        clientSession = new GBSession(ss.env->getMainLogger(), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ondemandname.c_str());
        clientSession->setSessionId(sessionId);
        clientSession->setControlUri(controlUri);
        group->add(*clientSession);
    }

    client = group->getC1Client(clientSession->controlUri()); 
    if(!client)
    {
        glog(::ZQ::common::Log::L_ERROR, "failed to allocate GBClient to Server %s", clientSession->controlUri());
    }
    else
    {
        RTSPMessage::AttrMap props;
        props.insert(std::make_pair("GlobalSession", contextKey.id()));
        calcScale(newScale, contextKey);
        int currentCSeq = client->sendPLAY(*clientSession, -1.0f, -1.0f, newScale, NULL, props);
        if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
            return ERR_RETURN_SERVER_ERROR;
        if (waitSignal(clientSession, currentCSeq) == false)
        {
            glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doChangeScale timeout"), contextKey.getContextProperty(ONDEMANDNAME_NAME).c_str(), currentCSeq);
            clientSession->_eventHandle.m_CloseEvent(currentCSeq);
            return ERR_RETURN_SERVER_ERROR;
        }
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        if(clientSession->_resultCode == RTSPSink::rcOK) // RTSPOK
        {
            GBSession::StreamInfos info = clientSession->getInfos();
            ctrlParams.timeoffset = info.timeoffset;
            ctrlParams.duration = info.duration;
            ctrlParams.scale = info.scale;
            ctrlParams.streamState = TianShanIce::Streamer::stsStreaming;
        }
        return HandleRtspCode(clientSession, "PLAY");
    }
    return ERR_RETURN_INVALID_PARAMATER;
}

int32 SsServiceImpl::doDestroy(	 SsServiceImpl& ss , SsContext&  contextKey ,IN const std::string& streamId )
{
    return ERR_RETURN_SUCCESS;
}

int32 SsServiceImpl::doGetStreamAttr(	 SsServiceImpl& ss , SsContext& contextKey, IN	const std::string& streamId,
    IN  const TianShanIce::Streamer::PlaylistItemSetupInfo& info, OUT StreamParams& ctrlParams )
{
	bool bIsOnRestore = ( 0 != (ctrlParams.mask & MASK_SESSION_RESTORE) );

	const std::string& ondemandname = contextKey.id();

	std::string ODSessId = contextKey.getContextProperty(ONDEMANDNAME_NAME);
	std::string groupName = contextKey.getContextProperty(SESSION_GROUP);
	if (ODSessId.empty() || groupName.empty())
	{
		glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl, "stream[%s] doGetStreamAttr() invalid OnDemandSessId[%s] or SessGroup[%s]"), streamId.c_str(), ODSessId.c_str(), groupName.c_str());
		return bIsOnRestore ? ERR_RETURN_OBJECT_NOT_FOUND :ERR_RETURN_INVALID_PARAMATER;
	}

    GBVSSSessionGroup* group = 0;
    GBSession::Ptr clientSession = 0;
    GBClient* client = 0;

    glog(::ZQ::common::Log::L_DEBUG, CLOGFMT(SsServiceImpl,"doGetStreamAttr session(%s) stream(%s)"), ondemandname.c_str(), streamId.c_str());
    group = GBVSSSessionGroup::findSessionGroup(contextKey.getContextProperty(SESSION_GROUP));
    if(group)
    {
        clientSession = group->lookupByOnDemandSessionId(ondemandname.c_str());
    }
    else
        return ERR_RETURN_INVALID_PARAMATER;

    if(!clientSession)
    {
        std::string sessionId = contextKey.getContextProperty(SESSION_ID);
        if(sessionId.empty())
            return	ERR_RETURN_INVALID_PARAMATER;
        std::string controlUri = contextKey.getContextProperty(CONTROL_URI);
        if(controlUri.empty())
            controlUri = clientSession->getBaseURL();
        glog(::ZQ::common::Log::L_DEBUG, "fail to find session [%s, %s], need new one", ondemandname.c_str(), sessionId.c_str());
        std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
        clientSession = new GBSession(ss.env->getMainLogger(), ss.env->getMainThreadPool(), dest.c_str(), NULL, Log::L_DEBUG, pGBVSSBaseConfig->_videoServer.sessionRenewInterval*1000, ondemandname.c_str());
        clientSession->setSessionId(sessionId);
        clientSession->setControlUri(controlUri);
        group->add(*clientSession);
    }

	if (bIsOnRestore)
	{
		std::string sessionId = contextKey.getContextProperty(SESSION_ID);
		std::string dest = contextKey.getContextProperty(DESTINATION_NAME);
		std::string controlUri = contextKey.getContextProperty(CONTROL_URI);

		if(controlUri.empty())
			controlUri = clientSession->getBaseURL();

		clientSession->setSessionId(sessionId);
		clientSession->setControlUri(controlUri);
		clientSession->setStreamDestination(dest.c_str());

		return ERR_RETURN_SUCCESS;
	}

    client = group->getC1Client(clientSession->controlUri()); 
    if(!client)
    {
        glog(::ZQ::common::Log::L_ERROR, "failed to allocate GBClient to Server %s", clientSession->controlUri());
		return ERR_RETURN_INVALID_PARAMATER;
    }
    
    RTSPMessage::AttrMap     headers;
    headers.insert(std::make_pair("User-Agent", GB_USER_AGENT));
    RTSPRequest::AttrList parameterNames;

	// stupid NGB spelling, see 258 C4.4.4.3
    parameterNames.push_back("Stream_State");
    parameterNames.push_back("Position");
    parameterNames.push_back("Scale");
    parameterNames.push_back("StreamSource");
    RTSPMessage::AttrMap props;
    int currentCSeq = client->sendGET_PARAMETER(*clientSession, parameterNames, NULL , props);
    if (currentCSeq <= 0 || clientSession->_eventHandle.m_Init(currentCSeq) == false)
        return ERR_RETURN_SERVER_ERROR;
    if (waitSignal(clientSession, currentCSeq) == false)
    {
        glog(::ZQ::common::Log::L_ERROR, CLOGFMT(SsServiceImpl,"session(%s) cseq(%d) doGetStreamAttr timeout"), contextKey.getContextProperty(ONDEMANDNAME_NAME).c_str(), currentCSeq);
        clientSession->_eventHandle.m_CloseEvent(currentCSeq);
        return ERR_RETURN_SERVER_ERROR;
    }
    clientSession->_eventHandle.m_CloseEvent(currentCSeq);
    if(clientSession->_resultCode == RTSPSink::rcOK) // RTSPOK
    {
        GBSession::StreamInfos info = clientSession->getInfos();
        ctrlParams.timeoffset = info.timeoffset;
        ctrlParams.scale = info.scale;
        ctrlParams.streamState = convertState(info.state);
    }
    return HandleRtspCode(clientSession, "GETPARAMETER");

}

}
}
