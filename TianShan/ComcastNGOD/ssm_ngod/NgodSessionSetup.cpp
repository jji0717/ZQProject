
#include <ZQ_common_conf.h>
#include <TianShanIceHelper.h>
#include "NgodEnv.h"
#include "SOPConfig.h"
#include "NgodConfig.h"
#include "NgodSession.h"
#include "ClientRequest.h"
#include "SelectionCommand.h"
#include "NgodSessionManager.h"


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"##y, request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), GetCurrentThreadId(),#x
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"y, request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), pthread_self(),#x	
#endif	

namespace NGOD
{


void convertParam(const SetupParam& setup, SelectIntentionParam& selection )
{
	selection.identifier		=	setup.sopName;
	selection.groupName			=	setup.sopName;
	selection.volume			=	setup.requestVolume;
	selection.requestBW			=	setup.requestBW;
	
	const std::vector<SetupParam::PlaylistItemInfo>& items = setup.playlist;
	std::vector<SetupParam::PlaylistItemInfo>::const_iterator itItem =items.begin();
	for( ; itItem != items.end() ; itItem++ )
	{
		SelectIntentionParam::PlaylistItemInfo plItemInfo;
		const SetupParam::PlaylistItemInfo& info = *itItem;
		plItemInfo.pid		= info.pid;
		plItemInfo.paid		= info.paid;
		plItemInfo.sid		= info.sid;
		plItemInfo.cuein	= info.cuein;
		plItemInfo.cueout	= info.cueout;
		plItemInfo.restrictionFlag = info.restrictionFlag;
		plItemInfo.range = info.range;
		plItemInfo.primaryAsset = info.primaryAsset;
		plItemInfo.extAds	= info.extAds;
		selection.playlist.push_back( plItemInfo );
	}
}

void prepareResources( TianShanIce::SRM::ResourceMap& resources, const NgodRequestSetupPtr& request, int64 adjustedBW )
{
	const SetupParam& param = request->getSetupParam();
	
	ZQTianShan::Util::updateResourceData<std::string>( resources, TianShanIce::SRM::rtEthernetInterface, "destIP", param.destIp );
	ZQTianShan::Util::updateResourceData<Ice::Int>( resources, TianShanIce::SRM::rtEthernetInterface, "destPort", (Ice::Int)param.destPort );
	if (ngodConfig.playlistControl.ignoreDestMac == 0)
	{
		ZQTianShan::Util::updateResourceData<std::string>( resources, TianShanIce::SRM::rtEthernetInterface, "destMac", param.destMac );
	}
	ZQTianShan::Util::updateResourceData<Ice::Int>( resources, TianShanIce::SRM::rtEthernetInterface, "srcPort", (Ice::Int)param.serverPort );
	ZQTianShan::Util::updateResourceData<Ice::Long>( resources, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", (Ice::Long)adjustedBW );
	
	if( !param.pokeHoleSessId.empty() )
	{
		ZQTianShan::Util::updateResourceData<Ice::Int>( resources, TianShanIce::SRM::rtEthernetInterface, "natPenetrating", (Ice::Int)1 );
		ZQTianShan::Util::updateResourceData<std::string>( resources, TianShanIce::SRM::rtEthernetInterface, "pokeholeSession", param.pokeHoleSessId );
	}
}
void updateStreamerNetId( TianShanIce::SRM::ResourceMap& resources, const std::string& streamerNetId )
{
	ZQTianShan::Util::updateResourceData<std::string>( resources,TianShanIce::SRM::rtStreamer, "NetworkId", streamerNetId );
	// set streamer Type to 1, so that UdpPump(EdgeFE) can create a udp session
	ZQTianShan::Util::updateResourceData(resources, TianShanIce::SRM::rtStreamer, "Type", 1);
}

TianShanIce::Streamer::StreamPrx NgodSessionI::createStream( const NgodRequestSetupPtr& request, 
															const TianShanIce::SRM::ResourceMap& resources, 
															const StreamerSelection& streamSel)
{
	if( mStream )
	{
		try
		{
			mStream->destroy();
		}
		catch( ...)	{}
	}	
	try
	{
		TianShanIce::Streamer::PlaylistPrx stream = NULL;
		
		Ice::Context ctx;
		ctx["CLIENTSESSIONID"] = request->getSetupParam().ondemandSessId;
		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodSessionI, "trying to create stream on [%s]"), streamSel.getSelectedStreamerNetId().c_str() );
		
		stream = TianShanIce::Streamer::PlaylistPrx::uncheckedCast( streamSel.getStreamerProxy()->createStreamByResource( resources, ctx ) );
		if(!stream)
		{			
			errlog( request, errorcodeInternalError, "failed to create stream on [%s]", streamSel.getSelectedStreamerNetId().c_str() );
			return NULL;
		}		
		{
			WLock sync(*this);
			mStream			= stream;
			mStreamSessId	= mStream->getIdent().name;
		}

		MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodSessionI, "stream[%s] was created on [%s]"), PROXY2STR(mStream).c_str(), streamSel.getSelectedStreamerNetId().c_str() );
	}
	catch( const TianShanIce::BaseException& ex )
	{
		{
			WLock sync(*this);
			mStream = NULL;
		}
		errlog( request, errorcodeInternalError, "failed to create stream on [%s] due to [%s]",streamSel.getSelectedStreamerNetId().c_str(), ex.message.c_str()  );							
	}
	catch( const Ice::Exception& ex)
	{
		{
			WLock sync(*this);
			mStream = NULL;
		}
		errlog( request, errorcodeInternalError, "failed to create stream on [%s] due to [%s]",streamSel.getSelectedStreamerNetId().c_str(), ex.ice_name().c_str()  );
	}
	return mStream;
}

void fillePrivateData( const NgodRequestSetupPtr& request, const StreamerSelection& streamerSel, 
						const ElementInfo& eleInfo, TianShanIce::ValueMap& privData, int32 index )
{
	
	ZQTianShan::Util::updateValueMapData( privData, "providerId", eleInfo.pid );
	ZQTianShan::Util::updateValueMapData( privData, "providerAssetId", eleInfo.paid );
	ZQTianShan::Util::updateValueMapData( privData, "currentCtrlNum", index );

	const VolumeAttrEx& volAttr = streamerSel.getSelectedVolumeAttr();
	if( !volAttr.bLocalPlaylist )
	{			
		ZQTianShan::Util::updateValueMapData( privData, "storageLibraryUrl", eleInfo.urls, false );			
	}
	else
	{		
		privData.erase("storageLibraryUrl");
	}

	ZQTianShan::Util::updateValueMapData( privData, "extAds", eleInfo.extAds);

	const SetupParam::EncryptionDataMap& encryData = request->getSetupParam().encryptData;
	SetupParam::EncryptionDataKey key;
	key.pid		= eleInfo.pid;
	key.paid	= eleInfo.paid;
	SetupParam::EncryptionDataMap::const_iterator itData = encryData.find( key );
	if( itData == encryData.end() )
		return;//no encryption data is found, return

	const SetupParam::EncryptionData& data =itData->second;	
	if( data.vecKeyOffset.size() > 0 )
		return;	
	ZQTianShan::Util::updateValueMapData( privData, "Tianshan-ecm-data:programNumber", ( Ice::Int )data.iProgNum );
	ZQTianShan::Util::updateValueMapData( privData, "Tianshan-ecm-data:Freq_1", (Ice::Int)data.iFrq1 );
	ZQTianShan::Util::updateValueMapData( privData, "Tianshan-ecm-data:keyoffsets", data.vecKeyOffset, false  );
	ZQTianShan::Util::updateValueMapData( privData, "Tianshan-ecm-data:keys", data.vecKeys, false  );
	ZQTianShan::Util::updateValueMapData( privData, "Tianshan-ecm-data:preEncryption-Enable", (Ice::Int)1 );
	ZQTianShan::Util::updateValueMapData( privData, "TianShan-flag-pauselastuntilnext", (Ice::Int)1 );
	ZQTianShan::Util::updateValueMapData( privData, "library", streamerSel.getSelectedVolumeName() );	
}

void fillPlaylistItemSetupInfo( const ElementInfo& eleInfo, const std::string& volumeName, TianShanIce::Streamer::PlaylistItemSetupInfo& setupInfo )
{
	setupInfo.contentName			=	eleInfo.fullContentName(volumeName );
	setupInfo.criticalStart			=	0;
	setupInfo.inTimeOffset			=	eleInfo.cueIn;
	setupInfo.outTimeOffset			=	eleInfo.cueOut;
	setupInfo.flags					=	eleInfo.flags;
	setupInfo.spliceIn				=	false;
	setupInfo.spliceOut				=	false;
	setupInfo.forceNormal			=	false;
	ZQTianShan::Util::updateValueMapData( setupInfo.privateData, "range", eleInfo.range );
}

bool NgodSessionI::getStreamSourceAddressInfo( const NgodRequestSetupPtr& request, SetupResponsePara& para )
{
	ServerResponseSetupPtr response = ServerResponseSetupPtr::dynamicCast( request->getResponse() );
	assert( response != NULL );

	TianShanIce::ValueMap value;
	try
	{
		if( !mStream->getInfo( TianShanIce::Streamer::infoSTREAMSOURCE, value) )
		{
			errlog( request, errorcodeInternalError, "NgodSessionI", "failed to get stream source ip/port");
			return false;
		}	
	}
	catch( const TianShanIce::BaseException& ex )
	{
		errlog( request, errorcodeInternalError, "NgodSessionI", "failed to get stream source ip/port due to [%s]", ex.message.c_str() );
		return false;
	}
	catch( const Ice::Exception& ex )
	{
		errlog( request, errorcodeInternalError, "NgodSessionI", "failed to get stream source ip/port due to [%s]", ex.ice_name().c_str() );
		return false;
	}

	ZQTianShan::Util::getValueMapDataWithDefault( value, "StreamingSourceIp", "", para.streamerSourceIp );
	ZQTianShan::Util::getValueMapDataWithDefault( value, "StreamingSourcePort", 0, para.streamerSourcePort );	

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodSessionI, "get source ip[%s] port[%d]"), para.streamerSourceIp.c_str(), para.streamerSourcePort );

	return true;
}

std::string flag2str( int64 flag )
{
	std::string str;
	if( flag & TianShanIce::Streamer::PLISFlagNoPause )
		str = str + "NoPause(P) ";
	
	if( flag & TianShanIce::Streamer::PLISFlagNoFF )
		str = str + "NoFF(F) ";

	if( flag & TianShanIce::Streamer::PLISFlagNoRew )
		str = str + "NoRew(R) ";

	if( flag & TianShanIce::Streamer::PLISFlagNoSeek )
		str = str + "NoSeek(S) ";

// 	if( flag & TianShanIce::Streamer::PLISFlagOnce )
// 		str = str + "PlayOnce ";
//fdj
	if( flag & TianShanIce::Streamer::PLISFlagSkipAtFF )
		str = str + "SkipAtFF(K) ";

	if( flag & TianShanIce::Streamer::PLISFlagSkipAtRew )
		str = str + "SkipAtRew(W) ";

	long playtimes = (long) (flag & TianShanIce::Streamer::PLISFlagPlayTimes)>>4;
	if( playtimes > 0 && playtimes < 10)
	{
		char Cplaytimes[2] = {0};
		sprintf(Cplaytimes, "%ld",playtimes);
		str = str + "PlayTimes(" + Cplaytimes + ") ";
	}
	return str;
}

bool NgodSessionI::renderStream( const NgodRequestSetupPtr& request, const StreamerSelection& streamerSel, bool& bSkipVolume )
{
	const SetupParam& setupPara = request->getSetupParam();
	ServerResponseSetupPtr response = ServerResponseSetupPtr::dynamicCast( request->getResponse() );
	assert( response != NULL );

	const ElementInfoS& elements = streamerSel.getElements();
	int32			iTotalElementCount =(int32) elements.size();
	int32			iCurrentElementIndex = 1;		//used as CtrlNum based on 1

	{
		WLock sync(*this);	
		ZQTianShan::Util::updatePropertyData( mProps, "PlayListTotalSize", elements.size());
	}

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodSessionI, "trying to render stream [%s]"), PROXY2STR(mStream).c_str() );
	//push every single element into StreamService
	ElementInfoS::const_iterator itElement = elements.begin();
	for ( ; itElement != elements.end() ; itElement ++ )
	{
		const ElementInfo& ei = *itElement;
		static int64 shortSlipDuration = ngodConfig.playlistControl.minClipDuration;
		if( ei.cueOut > 0 && 
			(ei.cueOut > ei.cueIn) && 
			(ei.cueOut - ei.cueIn) <= shortSlipDuration ) {
			MLOG(ZQ::common::Log::L_WARNING, SESSFMT(NgodSessionI, "clip[%s]'s duration [%lld] is short than [%lld], skip"),
				ei.name.c_str(), ei.cueOut - ei.cueIn, shortSlipDuration);
			continue;
		}

		TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
		setupInfo.privateData.clear();		
		
		fillePrivateData( request, streamerSel, ei, setupInfo.privateData, iCurrentElementIndex );
		fillPlaylistItemSetupInfo( ei, streamerSel.getSelectedVolumeName(), setupInfo );		
		
		if ( iTotalElementCount == iCurrentElementIndex + 1 )
		{//if this is the last item, erase TianShan-flag-pauselastuntilnext flag
			setupInfo.privateData.erase("TianShan-flag-pauselastuntilnext");
		}

		if( setupPara.startPoint.index == iCurrentElementIndex )
		{//update start point index and offset, we must subtract inTimeOffset because start point offset is asset relative but our system treat offset as segment relative

			updateSetupStartPoint( iCurrentElementIndex, setupPara.startPoint.offset - setupInfo.inTimeOffset );
		}

		try
		{
			mStream->pushBack( iCurrentElementIndex, setupInfo );
			{
				WLock sync(*this);
				mItemInfos.push_back( setupInfo );
			}
			MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodSessionI, "push item[%s] flag[%s] onto stream"), 
				setupInfo.contentName.c_str(), flag2str(setupInfo.flags).c_str() );
		}
		catch( const TianShanIce::InvalidParameter& ex )
		{
			// Change per NGOD-506          
			if ( ngodConfig.playlistControl.ignoreAbsentItems <= 0)
			{
				bSkipVolume = true;
				errlog( request, errorcodeAssetNotFound, "failed to render stream due to [%s]", ex.message.c_str() );
				return false;
			}			
			MLOG(ZQ::common::Log::L_WARNING, SESSFMT(NgodSessionI, "caught [%s] while push item [%s] into stream serivce"), 
								ex.ice_name().c_str(), setupInfo.contentName.c_str() );
		}
		catch( const TianShanIce::BaseException& ex )
		{
			errlog(request, errorcodeInternalError, "failed to render stream due to [%s]", ex.message.c_str() );
			return false;
		}
		catch( const Ice::Exception& ex )
		{
			errlog(request, errorcodeInternalError, "failed to render stream due to [%s]", ex.ice_name().c_str() );
			return false;
		}
		++iCurrentElementIndex;		
	}

	// Change per NGOD-506          
	if (mItemInfos.size() <=0)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NgodSessionI, "playlist[%s] non of %d items has been successfuly rendered"), PROXY2STR(mStream).c_str(), elements.size());
		errlog(request, errorcodeAssetNotFound, "playlist is empty" );
		return false;
	}

	try
	{
		mStream->enableEoT( ngodConfig.playlistControl.enableEOT >= 1 );		
		mStream->commit( );		
	}
	catch( const TianShanIce::BaseException& ex )
	{
		errlog( request, errorcodeInternalError, "failed to commit stream due to [%s]",ex.message.c_str() );
		return false;
	}
	catch( const Ice::Exception& ex)
	{
		errlog( request, errorcodeInternalError, "failed to commit stream due to [%s]",ex.ice_name().c_str() );
		return false;
	}

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodSessionI, "successfully rendered stream[%s]"), PROXY2STR(mStream).c_str() );
	return true;
}


int NgodSessionI::processSetup( const NgodRequestSetupPtr& request )
{
	//DO NOT LOCK THESE CODE, we haven't add this session into database so no other thread can operator this session
	ServerResponseSetupPtr response = ServerResponseSetupPtr::dynamicCast( request->getResponse() );
	assert( response != NULL );

	SelectionIntention selIntention( mEnv.getSelEnv(), request->sessionId, request->cseq, request->verbstr );	
	convertParam( request->getSetupParam(), selIntention.getParameter() );

	SelectIntentionParam& intentionPara = selIntention.getParameter( );

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodSessionI, "setup with sop[%s] volume[%s] bandwidth[%lld]"),
					intentionPara.identifier.c_str(), intentionPara.volume.c_str(), intentionPara.requestBW );

	StreamerSelection streamerSel( mEnv.getSelEnv(), mEnv.getSelResManager(), selIntention );
	
	//initialize selection
	if( !streamerSel.findFirstStreamer() )
	{
		errlog( request, streamerSel.getLastError(), streamerSel.getErrorMsg().c_str() );
		return streamerSel.getLastError();
	}

	//prepare resources exclude streamer net id, we will do it in next step after we select a available streamer
	TianShanIce::SRM::ResourceMap			resources;	
	prepareResources( resources, request, streamerSel.getAdjustedBandwidth() );

	bool	bSkipVolume = false;
	bool	bAddPenalty = false;
	bool	bStreamSuccess = false;
	int32	retryCount = sopConfig.sopRestrict.retryCount;
	bool	bFirstTry = true;

	for( int i = 0 ; i <= retryCount ; i++ )
	{
		if( !streamerSel.findNextStreamer( bSkipVolume, bAddPenalty ) )
		{
			if (bFirstTry)
			{
				errlog( request, streamerSel.getLastError(), (std::string("first selection err, ") + streamerSel.getErrorMsg()).c_str()	);
				return streamerSel.getLastError();
			}

			MLOG(ZQ::common::Log::L_WARNING, SESSFMT(NgodSessionI, "no more valid streamers"));
			return request->getResponse()->getErrorCode();
		}

		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodSessionI, "trying streamer[%s]"), streamerSel.getSelectedStreamerNetId().c_str() );
		bFirstTry = false;

		bSkipVolume = false;
		bAddPenalty = false;
		
		updateStreamerNetId( resources, streamerSel.getSelectedStreamerNetId() );

		if(!createStream( request, resources, streamerSel ))
		{
			errlog( request, errorcodeServiceUnavail, "failed to create stream on [%s]",streamerSel.getSelectedStreamerNetId().c_str() );
			bAddPenalty = true;
			continue;
		}

		response->setLastErr(errorcodeOK, "");
		if ( !renderStream( request, streamerSel, bSkipVolume ) )
		{
			if (errorcodeOK == request->getResponse()->getErrorCode())
				errlog(request, errorcodeServiceUnavail, "failed to render stream[%s] on [%s]", PROXY2STR(mStream).c_str(),	streamerSel.getSelectedStreamerNetId().c_str() );

			continue;
		}

		SetupResponsePara resPara;
		if(!getStreamSourceAddressInfo(request,resPara ))
		{
			errlog( request, errorcodeServiceUnavail, "failed to get source address information for stream[%s]",
				PROXY2STR(mStream).c_str() );
			continue;
		}
		
		response->setPara( resPara );
		bStreamSuccess = true;
		break;
	}

	if(!bStreamSuccess)
	{
		if (errorcodeOK == request->getResponse()->getErrorCode())
			errlog( request, errorcodeServiceUnavail, "no stream is created");

		return request->getResponse()->getErrorCode();
	}


	SessionStat*	cacheStat = new SessionStat();
	cacheStat->resources = resources;
	cacheStat->bitrate = (uint32)streamerSel.getAdjustedBandwidth();
	cacheStat->items = mItemInfos;
	cacheStat->properties["CLIENTSESSIONID"] = request->ondemandId;
	cacheStat->localPlaylist = streamerSel.isLocalPlaylist();
	cacheStat->selectionResource = streamerSel.getStreamerResourcePara();

	cacheStat->streamerNetId = streamerSel.getSelectedStreamerNetId();	
	cacheStat->streamSessId = mStreamSessId;
	cacheStat->rtspSessId = request->sessionId;
	cacheStat->ondemandSessId = request->ondemandId;
	// do not change lastTouch time stamp, timeOffset, scale, state due to no stream is really being pumped
	// leave state in stsSetup

	mEnv.getStatCache().addStat(cacheStat);

	streamerSel.commit();
	{		
		WLock sync(*this);
		mGroupId				= request->getSetupParam().sessionGroupName;
		mOndemandId				= request->ondemandId;
		mStreamerNetId			= streamerSel.getSelectedStreamerNetId();
		mSopName				= request->getSetupParam().sopName;
		mImportChannelName		= streamerSel.getSelectedImportChannelName();
		mUsedBW					= streamerSel.getAdjustedBandwidth();
		mOriginalUrl			= request->originalUrl;
		mServerIp				= request->mRtspServerIp;

		ZQTianShan::Util::updatePropertyData( mProps, SYSKEY(SetupTime), getISOTimeString() );
		char temp[256];
		if (request->mPrimaryItemNPT > 0.1f)
		{
			snprintf(temp, sizeof(temp), "%.3f", request->mPrimaryItemNPT);
			ZQTianShan::Util::updatePropertyData(mProps, "sys.primaryItemNPT", temp);
		}
		else 
		{
			ZQTianShan::Util::updatePropertyData(mProps, "sys.primaryItemNPT", "0.0");
		}


		if (request->mPrimaryItemEnd > 0.1f)
		{
			snprintf(temp, sizeof(temp), "%.3f", request->mPrimaryItemEnd);
			ZQTianShan::Util::updatePropertyData(mProps, "sys.primaryItemEnd", temp);
		}

		updateConnectionAndVerCode( request );
		mSessManager.updateR2ConnectionId( mGroupId, request->connectionId );

		//stream created successfully, fill some data into session's record and 
		updateSessState( TianShanIce::stInService );

		//record user-agent into session's property
		ZQTianShan::Util::updatePropertyData( mProps,PROP_CLIENT_USERAGENT,request->userAgent);
	}

	response->setLastErr( errorcodeOK, "")	;
	if(ngodConfig.publishLogs.enabled)
	{
		std::string strPAID="";
		if(!request->getSetupParam().playlist.empty())
		{
			strPAID = request->getSetupParam().playlist[0].paid;
		}
		ELOG(ZQ::common::Log::L_INFO, EVENTLOGFMT(NgodSessionI, "SessionHistoryLog Streamer(%s)Bandwidth(%lld)PAID(%s)"), mStreamerNetId.c_str(), mUsedBW, strPAID.c_str());
		ELOG(ZQ::common::Log::L_INFO, EVENTLOGFMT(NgodSessionI, "state(%s)"), errorCodeTransformer(errorcodeOK) );
	}	

	return errorcodeOK;
}

}//namespace NGOD
