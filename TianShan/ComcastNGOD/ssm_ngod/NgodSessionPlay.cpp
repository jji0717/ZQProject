
#include <ZQ_common_conf.h>
#include <TianShanIceHelper.h>
#include "ngod.h"
#include "NgodSession.h"
#include "NgodEnv.h"
#include "NgodSessionManager.h"
#include "SelectionCommand.h"
#include "NgodConfig.h"
#include "SOPConfig.h"


namespace NGOD
{


#if defined ZQ_OS_MSWIN
#define	SESSFMT(x, y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"##y, request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), GetCurrentThreadId(), #x	
#elif defined ZQ_OS_LINUX
#define	SESSFMT(x, y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"y, request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), pthread_self(), #x	
#endif	

#if ICE_INT_VERSION / 100 >= 306

class AsyncPlayResponseCB : public IceUtil::Shared
{
    public:
        AsyncPlayResponseCB( NGOD::NgodSessionIPtr sess ,NGOD::NgodClientRequestPtr request,  NgodEnv& env, NgodSessionManager& manager , const std::string& streamerNetId )
            :mSess(sess),
            request(request),
            mEnv(env),
            mSessManager(manager),
            mStreamerNetId(streamerNetId)
    {
    }
    private:
        void handleException(const Ice::Exception& ex){}
    public:
        void playEx(const Ice::AsyncResultPtr& r)
        {
                TianShanIce::Streamer::StreamPrx playPrx = TianShanIce::Streamer::StreamPrx::uncheckedCast(r->getProxy());
            try
            {
                playPrx->end_playEx(r);
            }
            catch(const Ice::Exception& ex)
            {
                handleException(ex);
             }

        }
    protected:
        NgodSessionIPtr                 mSess;
        NGOD::NgodClientRequestPtr      request;
        NgodEnv&                        mEnv;
        NgodSessionManager&             mSessManager;
        std::string                     mStreamerNetId;
    };
typedef IceUtil::Handle<AsyncPlayResponseCB> AsyncPlayResponseCBPtr;

class playAsyncCB : public AsyncPlayResponseCB
{
    public:
        playAsyncCB(NGOD::NgodSessionIPtr sess ,NGOD::NgodClientRequestPtr request,  NgodEnv& env, NgodSessionManager& manager , const std::string& streamerNetId )
            :AsyncPlayResponseCB(sess ,request, env, manager , streamerNetId)
        {
        }
    private:
        void handleException(const Ice::Exception& ex){}
    public:
        void playEx(const Ice::AsyncResultPtr& r)
        {
            TianShanIce::Streamer::PlaylistPrx playlsPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(r->getProxy());
            try
            {
                playlsPrx->end_playEx(r);
            }
            catch(const Ice::Exception& ex)
            {
                handleException(ex);
            }
        }

        void playItem(const Ice::AsyncResultPtr& r)
        {
            TianShanIce::Streamer::PlaylistPrx playlsPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(r->getProxy());
            try
            {
                playlsPrx->end_playItem(r);
            }
            catch(const Ice::Exception& ex)
			{
				handleException(ex);
			}
        }
    private:
};
typedef IceUtil::Handle<playAsyncCB> playAsyncCBPtr;

#else
class AsyncPlayResponse
{
public:
	AsyncPlayResponse( NGOD::NgodSessionIPtr sess, NGOD::NgodClientRequestPtr request, NgodEnv& env, NgodSessionManager& manager, const std::string& streamerNetId )
		:mSess(sess), 
		request(request), 
		mEnv(env), 
		mSessManager(manager), 
		mStreamerNetId(streamerNetId)
	{
	}
	virtual ~AsyncPlayResponse(){}

protected:
	
	void		aResponse( const ::TianShanIce::Streamer::StreamInfo& info );
	void		aException( const ::Ice::Exception& ex );
protected:
	NgodSessionIPtr					mSess;
	NGOD::NgodClientRequestPtr		request;
	NgodEnv&						mEnv;
	NgodSessionManager&				mSessManager;
	std::string						mStreamerNetId;
};

class AsyncPlayStreamResponse : public AsyncPlayResponse, public TianShanIce::Streamer::AMI_Stream_playEx
{
public:
	AsyncPlayStreamResponse( NGOD::NgodSessionIPtr sess, NGOD::NgodClientRequestPtr request, NgodEnv& env, NgodSessionManager& manager, const std::string& streamerNetId )
		:AsyncPlayResponse( sess, request, env, manager, streamerNetId )
	{

	}
	virtual ~AsyncPlayStreamResponse(){}
public:
	virtual void ice_response(const ::TianShanIce::Streamer::StreamInfo& info )
	{
		aResponse(info);
	}
	virtual void ice_exception(const ::Ice::Exception& ex)
	{
		aException( ex );
	}
};

class AsyncPlayItemResponse  : public AsyncPlayResponse, public TianShanIce::Streamer::AMI_Playlist_playItem
{
public:
	AsyncPlayItemResponse( NGOD::NgodSessionIPtr sess, NGOD::NgodClientRequestPtr request, NgodEnv& env, NgodSessionManager& manager, const std::string& streamerNetId )
		:AsyncPlayResponse( sess, request, env, manager, streamerNetId )
	{
	}
	virtual ~AsyncPlayItemResponse(){}
public:
	virtual void ice_response(const ::TianShanIce::Streamer::StreamInfo& info )
	{
		aResponse(info);
	}
	virtual void ice_exception(const ::Ice::Exception& ex)
	{
		aException( ex );
	}
};

void AsyncPlayResponse::aResponse( const ::TianShanIce::Streamer::StreamInfo& info )
{
	NGOD::NgodClientRequestIPtr requesti	= NGOD::NgodClientRequestIPtr::dynamicCast( request );
	ServerResponsePlayPtr response			= ServerResponsePlayPtr::dynamicCast( requesti->getResponse() );
	assert( response != NULL );

	NGOD::NgodRequestPlayPtr playRequest = NGOD::NgodRequestPlayPtr::dynamicCast(requesti);
	const PlayParam& para = playRequest->getPlayParam();

	Ice::Float scale = ZQTianShan::Util::getSpeedFromStreamInfo(info);
	char szBufScale[64];
	sprintf( szBufScale, "%f", scale );

	int oNpt = (int) ZQTianShan::Util::getStreamTimeOffset( info );
	int eNpt = (int) ZQTianShan::Util::getStreamTotalDuration( info );
	Ice::Long llStreamNpt = oNpt, llStreamEnd = eNpt;

	char szBufStart[64]	= {'\0'};
	char szBufEnd[64]	= {'\0'};
	llStreamNpt = mSess->nptToPrimary(llStreamNpt, (ngodConfig.playlistControl.nptByPrimary>0), request->sessionId.c_str());
	sprintf(szBufStart, "%lld.%03lld", llStreamNpt/1000, llStreamNpt%1000);

	if( llStreamEnd > 0 )
	{
		llStreamEnd = mSess->nptToPrimary(llStreamEnd, (ngodConfig.playlistControl.nptByPrimary>0), request->sessionId.c_str());
		sprintf(szBufEnd, "%lld.%03lld", llStreamEnd/1000, llStreamEnd%1000);
	}	
	response->setRange( szBufStart, szBufEnd );
	response->setScale( szBufScale );
	
	MLOG( ZQ::common::Log::L_INFO, SESSFMT(AsyncPlayResponse, "got stream info: scale[%s] npt[%d-%d]=>[%s-%s]"), szBufScale, oNpt, eNpt, szBufStart, szBufEnd );
	if(ngodConfig.publishLogs.enabled)
	{
		ELOG(ZQ::common::Log::L_INFO, EVENTLOGFMT(AsyncPlayResponse, "state(%s)"), errorCodeTransformer(errorcodeOK) );
	}
	
	Ice::Int iCtrlNum = ZQTianShan::Util::getUserCtrlNumFromStreamInfo(info);
	std::ostringstream ossCtrlNum;ossCtrlNum<<iCtrlNum;

	Ice::Long llItemNpt = ZQTianShan::Util::getItemTimeOffset( info );
	//llItemNpt = mSess->adjustNptToAssetBased( iCtrlNum - 1, llItemNpt ); // ctrl num is 1 based
	char szBufItemNpt[64];
	sprintf(szBufItemNpt, "%lld.%03lld", llItemNpt/1000, llItemNpt%1000);	


	std::string strNewState;
	switch(info.state)
	{
	case TianShanIce::Streamer::stsSetup:
		{
			strNewState = "INIT";
		}
		break;
	case TianShanIce::Streamer::stsStreaming: 
		{
			strNewState = "PLAY";
		}
		break;
	case TianShanIce::Streamer::stsPause: 
		{
			strNewState = "PAUSE";
		}
		break;
	case TianShanIce::Streamer::stsStop:
		{
			strNewState = "READY";
		}
		break;
	default: 
		{
			strNewState = "UNKNOWN";
		}
		break;
	}	
	MLOG(ZQ::common::Log::L_INFO, SESSFMT(AsyncPlayResponse, "add session record: item npt[%s] scale[%s] resourceId[%d]"), szBufItemNpt, szBufScale, iCtrlNum );
    // update firstplay
    Ice::Int firstPlay = 0;
	{
		NgodSessionI::WLock sync(*mSess);	
		ZQTianShan::Util::updatePropertyData( mSess->mProps, SYSKEY(SessionFirstPlay), firstPlay);
	}

	if ( ngodConfig.sessionHistory.enablePlayEvent )
	{
		std::string     strUserTime;
		ZQTianShan::Util::getPropertyDataWithDefault( info.props, "echo.Time", getISOTimeString(), strUserTime );
		mSess->recordEvent_StreamCtrl( "PLAY", strNewState, szBufItemNpt, szBufScale, ossCtrlNum.str(), strUserTime, errorcodeOK, para.reqRange, para.reqScale );
	}

	// This is a trick on accessing NgodSession::mStreamSessId directly, DO nottrying to modify this value	
	mEnv.getStatCache().statChanged( mSess->mStreamSessId, llItemNpt, scale, TianShanIce::Streamer::stsStreaming);
}

void AsyncPlayResponse::aException( const ::Ice::Exception& ex )
{
	NGOD::NgodClientRequestIPtr requesti = NGOD::NgodClientRequestIPtr::dynamicCast( request );
	assert( requesti != NULL );

	NGOD::NgodRequestPlayPtr playRequest = NGOD::NgodRequestPlayPtr::dynamicCast(requesti);
	const PlayParam& para = playRequest->getPlayParam();

	int32 errCode	= errorcodeInternalError;
	std::string errMsg;
	
	bool bSockTransmitError = false;

	try
	{
		ex.ice_throw();
	}
	catch( const Ice::ObjectNotExistException& ex )
	{
		errCode	= errorcodeObjNotFound;
		errMsg	= ex.ice_name();
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(AsyncPlayResponse, "caugh [%s], destroy current session"), ex.ice_name().c_str() );
		mSess->destroy(NULL);
	}
	catch( const Ice::TimeoutException& ex )
	{
		bSockTransmitError = true;
		errMsg	= ex.ice_name();
		errCode	= errorcodeServiceUnavail;
		if(sopConfig.sopRestrict.penaltyEnableMask & PENALTY_ENABLE_MASK_PLAY )
			mEnv.mSelManager.applyIncreasablePenalty( mSess->getSessionId(), mStreamerNetId, sopConfig.sopRestrict.timeoutPenalty, sopConfig.sopRestrict.maxPenaltyValue);
	}
	catch( const Ice::SocketException& ex)
	{
		bSockTransmitError = true;
		errCode	= errorcodeServiceUnavail;
		errMsg		= ex.ice_name();
		if(sopConfig.sopRestrict.penaltyEnableMask & PENALTY_ENABLE_MASK_PLAY )
			mEnv.mSelManager.applyPenalty( mSess->getSessionId(), mStreamerNetId, sopConfig.sopRestrict.maxPenaltyValue);
	}	
	catch( const TianShanIce::InvalidStateOfArt& ex)
	{
		errMsg	= ex.message;
		errCode = errorcodeTrickRestriction;
	}
	catch( const TianShanIce::InvalidParameter& ex)
	{
		switch(ex.errorCode )
		{
		case EXT_ERRCODE_INVALID_RANGE:			{	errCode = errorcodeInvalidRange;	}
			break;
		case EXT_ERRCODE_BANDWIDTH_EXCEEDED:	{	errCode = errorcodeNotEnoughBandwidth;		}
			break;
		default:								{	errCode = errorcodeBadParameter;	}
			break;
		}
		errMsg = ex.message;
	}
	catch( const TianShanIce::ServerError& ex )
	{
		switch(ex.errorCode )
		{
		case EXT_ERRCODE_INVALID_RANGE:			{	errCode = errorcodeInvalidRange;	}
				break;
		case EXT_ERRCODE_BANDWIDTH_EXCEEDED:	{	errCode = errorcodeNotEnoughBandwidth;		}
				break;
		case EXT_ERRCODE_SERVICEUNAVAIL:		{	errCode = errorcodeServiceUnavail; }
				break;
		default:								{	errCode = errorcodeInternalError;	}
				break;
		}
		errMsg = ex.message;
	}
	catch( const TianShanIce::BaseException& ex)
	{
		errCode	= errorcodeInternalError;
		errMsg	= ex.message;
	}
	catch( const Ice::Exception& ex )
	{
		errCode = errorcodeInternalError;
		errMsg	= ex.ice_name();
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AsyncPlayResponse, "got exception[%s]"), errMsg.c_str() );
	std::string		strAssetNpt;
	std::string		strStreamScale;
	std::string		strAssetCtrlNum;
	std::string     strNewState;
	if( !bSockTransmitError )
	{
		mSess->getPlayListInfo(strStreamScale, strAssetNpt, strAssetCtrlNum, strNewState);
	}
	else
	{
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(AsyncPlayResponse, "data transimission failed, skip getting playlist information"));
	}

	if ( ngodConfig.sessionHistory.enablePlayEvent )
	{
		mSess->recordEvent_StreamCtrl( "PLAY", strNewState, strAssetNpt, strStreamScale, strAssetCtrlNum, getISOTimeString(), errCode, para.reqRange, para.reqScale );
	}

	mSess->errlog( requesti, errCode, "failed to execute playStream/PlayItem due to [%s]", errMsg.c_str() );
}
#endif
/*
int64 NgodSessionI::fixupNpt(int64 npt, const char* sess, bool responseNpt)
{
	if (NULL ==sess)
		sess ="n/a";

	if (ngodConfig.playlistControl.nptByPrimary <=0 || npt <0)
		return npt;

	// fixup the requested npt if config[nptByPrimary] is set
	RLock sync(*this);
	int64 newNPT = npt;
	int64 primaryLeft = newNPT;
	int cAds = 0;
	for(size_t i=0; i < mItemInfos.size(); i++)
	{
		if (responseNpt && newNPT<=0)
		{
			newNPT =0;
			break;
		}

		const ::TianShanIce::Streamer::PlaylistItemSetupInfo& item = mItemInfos[i];
		int64 itemDur = item.outTimeOffset - item.inTimeOffset;
		bool isAds = (0 != ((::TianShanIce::Streamer::PLISFlagNoFF | ::TianShanIce::Streamer::PLISFlagNoSeek) & item.flags));
		if (!isAds)
		{
			TianShanIce::ValueMap::const_iterator it = item.privateData.find("extAds");
			if ((item.privateData.end() != it) && it->second.strs.size() >0 && !it->second.strs[0].empty())
				isAds = true;
		}

		if (isAds && (item.outTimeOffset <=0 || itemDur < 0))
		{
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NgodSessionI, "fixupPlayNpt() sess[%s] nptByPrimary[T] failed due to Ads item%d[%s] has no valid duration: cueIn[%d] cueOut[%d]"),
				sess, i+1, item.contentName.c_str(), (int)item.inTimeOffset, (int)item.outTimeOffset);
			newNPT = npt;
			break;
		}

		if (isAds)
		{
			if (responseNpt)
				newNPT -= itemDur; // npt to response
			else newNPT += itemDur; 
			cAds ++;
			continue;
		}

		if (itemDur < 0 || itemDur >= primaryLeft)
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSessionI, "fixupPlayNpt() sess[%s] nptByPrimary[T] requested-npt[%d] hit at primary item%d[%s] offset[%d], duration: cueIn[%d] cueOut[%d]"),
				sess, (int)npt, i+1, item.contentName.c_str(), (int)primaryLeft, (int)item.inTimeOffset, (int)item.outTimeOffset);
			break;
		}

		primaryLeft -= itemDur;
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NgodSessionI, "fixupPlayNpt() sess[%s] nptByPrimary[T] requested-npt[%d] => npt2excute[%d] by %d Ads"), 
		sess, (int)npt, (int)newNPT, cAds);

	return newNPT;
}
*/

// convert primary npt to executable npt
int64 NgodSessionI::primaryToNpt(int64 npt, bool byPrimary, const char* sess)
{
	if (NULL ==sess)
		sess = getSessionId().c_str();

	if (!byPrimary || npt < 0 || npt < 20) // short than a frame(20msec or 33msec at 30fps) 
		return npt;

	// fixup the requested npt if config[nptByPrimary] is set
	RLock sync(*this);
	int64 newNPT = npt;
	int64 primaryLeft = newNPT;
	int cAds = 0;
	for(size_t i=0; i < mItemInfos.size(); i++)
	{
		const ::TianShanIce::Streamer::PlaylistItemSetupInfo& item = mItemInfos[i];
		int64 itemDur = item.outTimeOffset - item.inTimeOffset;
		bool isAds = (0 != ((::TianShanIce::Streamer::PLISFlagNoFF | ::TianShanIce::Streamer::PLISFlagNoSeek) & item.flags));
		if (!isAds)
		{
			TianShanIce::ValueMap::const_iterator it = item.privateData.find("extAds");
			if ((item.privateData.end() != it) && it->second.strs.size() >0 && !it->second.strs[0].empty())
				isAds = true;
		}

		if (isAds && (item.outTimeOffset <=0 || itemDur < 0))
		{
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NgodSessionI, "primaryToNpt() sess[%s] nptByPrimary[T] failed due to Ads item%d[%s] has no valid duration: cueIn[%d] cueOut[%d]"),
				sess, i+1, item.contentName.c_str(), (int)item.inTimeOffset, (int)item.outTimeOffset);
			newNPT = npt;
			break;
		}

		if (isAds)
		{
			newNPT += itemDur;
			cAds ++;
			continue;
		}

		if (itemDur < 0 || itemDur >= primaryLeft)
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSessionI, "primaryToNpt() sess[%s] nptByPrimary[T] requested-npt[%d] hit at primary item%d[%s] offset[%d], duration: cueIn[%d] cueOut[%d]"),
				sess, (int)npt, i+1, item.contentName.c_str(), (int)primaryLeft, (int)item.inTimeOffset, (int)item.outTimeOffset);
			break;
		}

		primaryLeft -= itemDur;
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NgodSessionI, "primaryToNpt() sess[%s] nptByPrimary[T] requested-npt[%d] => npt2excute[%d] by %d Ads"), 
		sess, (int)npt, (int)newNPT, cAds);
	return newNPT;
}

// convert executable npt to primary npt
int64 NgodSessionI::nptToPrimary(int64 npt, bool byPrimary, const char* sess)
{
	if (NULL ==sess)
		sess = getSessionId().c_str();

	if (!byPrimary || npt < 0)
		return npt;

	// fixup the requested npt if config[nptByPrimary] is set
	RLock sync(*this);
	int64 newNPT = npt;
	int64 nptLeft = npt;
	int cAds = 0;
	for(size_t i=0; nptLeft >0 && i < mItemInfos.size(); i++)
	{
		const ::TianShanIce::Streamer::PlaylistItemSetupInfo& item = mItemInfos[i];
		int64 itemDur = item.outTimeOffset - item.inTimeOffset;
		bool isAds = (0 != ((::TianShanIce::Streamer::PLISFlagNoFF | ::TianShanIce::Streamer::PLISFlagNoSeek) & item.flags));
		if (!isAds)
		{
			TianShanIce::ValueMap::const_iterator it = item.privateData.find("extAds");
			if ((item.privateData.end() != it) && it->second.strs.size() >0 && !it->second.strs[0].empty())
				isAds = true;
		}

		if (isAds && (item.outTimeOffset <=0 || itemDur < 0))
		{
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(NgodSessionI, "nptToPrimary() sess[%s] nptByPrimary[T] failed due to Ads item%d[%s] has no valid duration: cueIn[%d] cueOut[%d]"),
				sess, i+1, item.contentName.c_str(), (int)item.inTimeOffset, (int)item.outTimeOffset);
			newNPT = npt;
			break;
		}

		if (isAds)
		{
			newNPT  -= itemDur;
			nptLeft -= itemDur;
			cAds ++;
			continue;
		}

		if (itemDur < 0)
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSessionI, "nptToPrimary() sess[%s] nptByPrimary[T] requested-npt[%d] hit at primary item%d[%s] offset[%d], duration: cueIn[%d] cueOut[%d]"),
				sess, (int)npt, i+1, item.contentName.c_str(), (int)nptLeft, (int)item.inTimeOffset, (int)item.outTimeOffset);
			break;
		}

		nptLeft -= itemDur;
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(NgodSessionI, "nptToPrimary() sess[%s] nptByPrimary[T] requested-npt[%d] => npt2excute[%d] by %d Ads"), 
		sess, (int)npt, (int)newNPT, cAds);
	return newNPT;
}

void NgodSessionI::fixupPlayParams(NgodRequestPlayPtr& request)
{
	PlayParam playParams = request->getPlayParam();
	playParams.nptStart = primaryToNpt(playParams.nptStart, request->sessionId.c_str());

	request->setPlayParam(playParams);
}

int NgodSessionI::processPlay( const NgodRequestPlayPtr& oldrequest )
{
	NgodRequestPlayPtr request = oldrequest; 

	if( 0 == ( sopConfig.sopRestrict.execMask & PENALTY_ENABLE_MASK_PLAY) ) {
		int penalty = 0;
		if( !mEnv.getSelResManager().isStreamHealthy(mStreamerNetId, &penalty) )
		{
			errlog(request, errorcodeServiceUnavail, "streamer[%s] has penalty[%d], skip PLAY per execMask[%x]", 
				mStreamerNetId.c_str(), penalty, sopConfig.sopRestrict.execMask);
			return errorcodeServiceUnavail;
		}
	}

	ServerResponsePlayPtr response = ServerResponsePlayPtr::dynamicCast( request->getResponse() );
	assert( response != NULL );

	updateConnectionAndVerCode( request );

	const PlayParam& para = request->getPlayParam();

	TianShanIce::StrValues expectValues;
	expectValues.push_back("ITEM_CURRENTPOS");
	expectValues.push_back("ITEM_TOTALPOS");	
	expectValues.push_back("CURRENTPOS");
	expectValues.push_back("TOTALPOS");
	expectValues.push_back("SPEED");
	expectValues.push_back("STATE");
	expectValues.push_back("USERCTRLNUM");

	try
	{
		::Ice::Context ctx;
		ctx["echo.Time"] = getISOTimeString();

		fixupPlayParams(request);

		if( !para.bFromNow )
		{
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodSessionI, "play from: now[%s] npt[%lld] scale[%f]"), para.bFromNow ? "true":"false", para.nptStart, para.scale );		
#if ICE_INT_VERSION / 100 >= 306
			AsyncPlayResponseCBPtr onPlayCbPtr = new AsyncPlayResponseCB(this,request , mEnv , mSessManager , mStreamerNetId);
			Ice::CallbackPtr genericCB = Ice::newCallback(onPlayCbPtr, &AsyncPlayResponseCB::playEx);
			mStream->begin_playEx( para.scale, para.nptStart, 1, expectValues, ctx , genericCB);
#else
			mStream->playEx_async(new AsyncPlayStreamResponse(this, request, mEnv, mSessManager, mStreamerNetId), para.scale, para.nptStart, 1, expectValues, ctx );
#endif
			return errorcodeOK;
		}

		Ice::Int	spIndex = -1;
		Ice::Long	spOffset = 0;

		if(!getSetupStartPoint(spIndex, spOffset) )
		{
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodSessionI, "play from now, scale[%f]"), para.scale );		
#if ICE_INT_VERSION / 100 >= 306
			AsyncPlayResponseCBPtr onPlayCbPtr = new AsyncPlayResponseCB(this,request , mEnv , mSessManager , mStreamerNetId);
			Ice::CallbackPtr genericCB = Ice::newCallback(onPlayCbPtr, &AsyncPlayResponseCB::playEx);
			mStream->begin_playEx(para.scale, 0, 0, expectValues,ctx, genericCB);
#else
			mStream->playEx_async(new AsyncPlayStreamResponse(this, request, mEnv, mSessManager, mStreamerNetId), para.scale, 0, 0, expectValues, ctx );
#endif
			return errorcodeOK;
		}		

		Ice::Int firstPlay = 1;
		{
			RLock sync(*this);
			ZQTianShan::Util::getPropertyDataWithDefault( mProps, SYSKEY(SessionFirstPlay), 1, firstPlay );
		}

		if( isSessionStreaming() || !firstPlay)
		{
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodSessionI, "play[now-] ignore startPoint[%d %lld] due to streaming or firstPlay[%d]"), spIndex, spOffset, firstPlay);
#if ICE_INT_VERSION / 100 >= 306
			AsyncPlayResponseCBPtr onPlayCbPtr = new AsyncPlayResponseCB(this,request , mEnv , mSessManager , mStreamerNetId);
			Ice::CallbackPtr genericCB = Ice::newCallback(onPlayCbPtr, &AsyncPlayResponseCB::playEx);
			mStream->begin_playEx( para.scale, 0, 0, expectValues, ctx, genericCB);
#else
			mStream->playEx_async( new AsyncPlayStreamResponse(this, request, mEnv, mSessManager, mStreamerNetId), para.scale, 0, 0, expectValues, ctx);
#endif
		}
		else
		{
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(NgodSessionI, "play[now-] takes startPoint[%d %lld]"), spIndex, spOffset );		
#if ICE_INT_VERSION / 100 >= 306
			playAsyncCBPtr onPlayCbPtr = new playAsyncCB(this,request , mEnv , mSessManager , mStreamerNetId);
			Ice::CallbackPtr genericCB = Ice::newCallback(onPlayCbPtr, &playAsyncCB::playItem);
			mStream->begin_playItem(spIndex, (Ice::Int)spOffset, 1, para.scale, expectValues, ctx,genericCB);
#else
			mStream->playItem_async( (new AsyncPlayItemResponse(this, request, mEnv, mSessManager, mStreamerNetId) ), spIndex, (Ice::Int)spOffset, 1, para.scale, expectValues, ctx );
#endif
		}	

		return errorcodeOK;
	}
	catch( const Ice::Exception& ex)
	{
		errlog(request, errorcodeInternalError, "failed to perform play command due to[%s]", ex.ice_name().c_str() );
	}

	return errorcodeInternalError;
}

}//namespace NGOD
