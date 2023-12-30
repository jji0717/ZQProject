
#include <ZQ_common_conf.h>
#include "NgodEnv.h"
#include "AnnounceRequest.h"
#include "NgodSessionManager.h"
#include "RtspHeaderDefines.h"
#include <assert.h>
#include "NgodConfig.h"
#include <TimeUtil.h>

#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"[%s]%d/%s/%08X/REQUEST[%s]\t"##y, info.sessId.c_str() ,  info.seq, info.odsessId.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
#define	SESSFMT(x,y) 	"[%s]%d/%s/%08X/REQUEST[%s]\t"y, info.sessId.c_str(),info.seq,info.odsessId.c_str(),pthread_self(),#x	
#endif	

namespace NGOD
{


AnnounceRequest::AnnounceRequest( NgodEnv& env  , NgodSessionManager& manager )
:mEnv(env),
mSessManager(manager),
mServerRequest(NULL)
{
}

AnnounceRequest::~AnnounceRequest(void)
{
	if( mServerRequest)
	{
		mServerRequest->release();
		mServerRequest = NULL;
	}
}

bool AnnounceRequest::init( const std::string& sessId ,const std::string& connId )
{
	if( mServerRequest)
	{
		mServerRequest->release();
		mServerRequest = NULL;
	}
	mServerRequest = mSessManager.getServerRequest( sessId , connId );
	{
		if(mServerRequest == NULL)
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(AnnounceRequest,"failed to get server request for session[%s] with connectionId[%s]"),
				sessId.c_str() , connId.c_str() );
		}
	}
	return mServerRequest != NULL; 
}
void AnnounceRequest::setStartline( const std::string& startLine )
{
	assert( mServerRequest != NULL );
	mServerRequest->printCmdLine( startLine.c_str() );
}
void AnnounceRequest::setHeader( const std::string& key , const std::string& value )
{
	assert( mServerRequest != NULL );
	mServerRequest->printHeader( (char*)key.c_str() , (char*)value.c_str() );
}

void AnnounceRequest::setHeader( const std::string& key , int32 value )
{
	char szBuf[32];
	sprintf(szBuf,"%d",value);
	setHeader( key , std::string(szBuf));
}

void AnnounceRequest::setHeader( const std::string& key , int64 value )
{
	char szBuf[64];
	sprintf(szBuf,"%lld",value);
	setHeader( key , std::string(szBuf));
}

void AnnounceRequest::setHeader( const std::string& key , float value )
{
	char szBuf[32];
	sprintf(szBuf,"%f",value);
	setHeader( key , std::string(szBuf));
}
void AnnounceRequest::setBody( const std::string& body )
{
	assert( mServerRequest != NULL );
	mServerRequest->printMsgBody( (char*)body.c_str() );
}
bool AnnounceRequest::post( )
{	
	if( mServerRequest )
	{
		int64 startTime = ZQ::common::now();

		bool bOK = mServerRequest->post() > 0 ;

		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(AnnounceRequest,"post() it take [%lld]ms to send out announce, result[%s], notice string :%s "),
			ZQ::common::now() - startTime, bOK?"succ":"fail",mNoticeStr.c_str() );
		return bOK;
	}
	else
		return false;
}
void AnnounceRequest::attach( IServerRequest* serverRequest )
{
	if( mServerRequest)
	{
		mServerRequest->release();
	}
	mServerRequest = serverRequest;
}
void AnnounceRequest::updateStartline( const std::string& url )
{
	//std::string startline = "ANNOUNCE ";
	std::string startline = "ANNOUNCE rtsp://";//fixed according to NGOD-608
	startline += url;
	startline +=" RTSP/1.0";
	setStartline( startline );
}

void AnnounceRequest::updateNotice( const std::string code , const std::string& reason , const std::string& offset )
{	
	mNoticeStr = generatorNoticeString( code , reason , offset );
	setHeader( HeaderNotice , mNoticeStr );
}
void AnnounceRequest::updateRequire( bool bR2 , NgodProtoclVersionCode verCode )
{
	if( !bR2 )
	{
		if( verCode == NgodVerCode_C1 )
		{
			setHeader( HeaderRequire , "com.comcast.ngod.c1");
		}
		else
		{
			setHeader( HeaderRequire , "com.comcast.ngod.c1,com.comcast.ngod.c1.decimal_npts");
		}
	}
	else
	{
		if( verCode == NgodVerCode_R2 )
		{
			setHeader( HeaderRequire , "com.comcast.ngod.r2");
		}
		else
		{
			setHeader( HeaderRequire , "com.comcast.ngod.r2,com.comcast.ngod.r2.decimal_npts");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceEndOfStream
AnnounceEndOfStream::AnnounceEndOfStream( NgodEnv& env , NgodSessionManager& manager  )
:AnnounceRequest( env ,manager )
{
}
AnnounceEndOfStream::~AnnounceEndOfStream()
{

}

bool AnnounceEndOfStream::postAnnounce(  const StreamAnnounceInfo&  info )
{
	ZQ::common::Log& mainLog = *mEnv.getMainLogger();
	if(!init( info.sessId , info.connectionId))
	{
		mainLog(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceEndOfStream,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );		
	setHeader( HeaderSequence , info.seq );
	char szTemp[64];
	snprintf(szTemp, sizeof(szTemp) - 1, "%lld.%03lld", info.npt/1000, info.npt%1000 );
	std::stringstream oss;
	oss << "npt=" << szTemp << ";scale=" << info.scale; 
	if(ngodConfig.adsReplacment.provideLeadingAdsPlaytime)
		oss << ";primaryItemNPT=" << info.primaryItemNPT;
	setHeader( HeaderTianShanNoticeParam, oss.str() );
	updateNotice( NGOD_ANNOUNCE_ENDOFSTREAM , NGOD_ANNOUNCE_ENDOFSTREAM_STRING , generatorOffsetString(info.npt , info.c1VerCode ) );
	updateRequire( false , info.c1VerCode );
	
	if(!post())
	{
		mainLog(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceEndOfStream,"failed to send out anounce"));
		return false;
	}
	else
	{
		mainLog(ZQ::common::Log::L_INFO,SESSFMT(AnnounceEndOfStream,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceBeginOfStream
AnnounceBeginOfStream::AnnounceBeginOfStream( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest(env,manager)
{

}
AnnounceBeginOfStream::~AnnounceBeginOfStream()
{

}

bool AnnounceBeginOfStream::postAnnounce( const StreamAnnounceInfo& info )
{
	if(!init( info.sessId , info.connectionId))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceBeginOfStream,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	char szTemp[64];
	snprintf(szTemp, sizeof(szTemp) - 1, "%lld.%03lld", info.npt/1000, info.npt%1000 );
	std::stringstream oss;
	oss << "npt=" << szTemp << ";scale=" << info.scale;
	if(ngodConfig.adsReplacment.provideLeadingAdsPlaytime)
		oss << ";primaryItemNPT=" << info.primaryItemNPT;
	setHeader( HeaderTianShanNoticeParam, oss.str() );

	updateNotice( NGOD_ANNOUNCE_BEGINOFSTREAM , NGOD_ANNOUNCE_BEGINOFSTREAM_STRING , generatorOffsetString(info.npt , info.c1VerCode ) );

	updateRequire( false , info.c1VerCode );


	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceBeginOfStream,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceBeginOfStream,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceScaleChange
AnnounceScaleChange::AnnounceScaleChange( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest(env,manager)
{
}
AnnounceScaleChange::~AnnounceScaleChange()
{

}

bool AnnounceScaleChange::postAnnounce( const StreamAnnounceInfo& info )
{
	if(!init( info.sessId , info.connectionId))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceScaleChange,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	
	std::string noticeStr = generatorNoticeString( NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE , NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE_STRING , generatorOffsetString(info.npt , info.c1VerCode ) );
	{
		char scaleBuf[32];
		sprintf( scaleBuf , " scale=%f ", info.scale );
		noticeStr += scaleBuf;
	}
	setHeader( HeaderNotice , noticeStr );
	char szTemp[64];
	snprintf(szTemp, sizeof(szTemp) - 1, "%lld.%03lld", info.npt/1000, info.npt%1000 );
	std::stringstream oss;
	oss << "npt=" << szTemp << ";scale=" << info.scale;
	if(ngodConfig.adsReplacment.provideLeadingAdsPlaytime)
		oss << ";primaryItemNPT=" << info.primaryItemNPT;
	setHeader( HeaderTianShanNoticeParam, oss.str() );

	updateRequire( false , info.c1VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceScaleChange,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceScaleChange,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceStateChange
AnnounceStateChange::AnnounceStateChange( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest( env , manager )
{
}
AnnounceStateChange::~AnnounceStateChange()
{

}
bool AnnounceStateChange::postAnnounce( const StreamAnnounceInfo& info )
{
	if(!init( info.sessId , info.connectionId))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceStateChange,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	

	std::string noticeStr = generatorNoticeString( NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE , NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE_STRING , generatorOffsetString(info.npt , info.c1VerCode ) );
	{
	std::string state_str;
		switch (info.state)
		{
		case TianShanIce::Streamer::stsSetup:
			state_str = "init";
			break;
		case TianShanIce::Streamer::stsStreaming:
			state_str = "play";
			break;
		case TianShanIce::Streamer::stsPause:
			state_str = "pause";
			break;
		case TianShanIce::Streamer::stsStop:
			state_str = "ready";
			break;
		default:
			state_str = "unknown";
			break;
		}
		noticeStr += " presentation_state=" + state_str;
	}
	setHeader( HeaderNotice , noticeStr );
	char szTemp[64];
	snprintf(szTemp, sizeof(szTemp) - 1, "%lld.%03lld", info.npt/1000, info.npt%1000 );
	std::stringstream oss;
	oss << "npt=" << szTemp << ";scale=" << info.scale;
	if(ngodConfig.adsReplacment.provideLeadingAdsPlaytime)
		oss << ";primaryItemNPT=" << info.primaryItemNPT;
	setHeader( HeaderTianShanNoticeParam, oss.str() );

	updateRequire( false , info.c1VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceStateChange,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceStateChange,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceTransition
AnnounceTransition::AnnounceTransition( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest( env , manager )
{
}
AnnounceTransition::~AnnounceTransition()
{

}

#define LINETERM "\r\n"

void AnnounceTransition::outputTransition( const StreamAnnounceInfo& info , const TransitionAnnounceInfo& transition )
{	
	std::ostringstream oss;
	oss << "v=0" << LINETERM;
	oss << "o=- " << info.sessId << " " << getGMTString() << " IN IP4 " << info.serverIp << LINETERM;
	oss << "s=" << LINETERM;
	oss << "c=IN IP4 0.0.0.0" << LINETERM;
	oss << "a=X-previous-playlist-element-index: " << transition.prevCtrlNum - 1 << LINETERM;
	oss << "a=X-current-playlist-element-index: " << transition.currentCtrlNum - 1 << LINETERM;
	setBody( oss.str() );
}

bool AnnounceTransition::postAnnounce( const StreamAnnounceInfo& info , const TransitionAnnounceInfo& transition )
{
	if(!init( info.sessId , info.connectionId))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceTransition,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	updateNotice( NGOD_ANNOUNCE_TRANSITION , NGOD_ANNOUNCE_TRANSITION_STRING , generatorOffsetString(info.npt, info.c1VerCode ) );
	setHeader(HeaderContentType,"application/sdp");

	std::stringstream oss;
	oss << "prevItem=" << info.ItemPrevName << ";currentItem=" << info.ItemCurName
		<< ";prevCtrlNum="<< transition.prevCtrlNum - 1 << ";currentCtrlNum=" << transition.currentCtrlNum - 1 
		<< ";prevPAID=" << info.prevPAssetId << ";prevPID=" << info.prevProviderId 
		<< ";currentPAID=" << info.curPAssetId << ";currentPID=" << info.curProviderId 
		<< ";nptPrimary="<< info.nptPrimary 
		<< ";prevDur=" << info.prevdur << ";currentDur=" << info.curdur 
		<< ";prevFlags="<< info.prevflags << ";currentFlags=" << info.curflags
		<< ";totalVideodur=" << info.totalVideodur << ";totalDur=" << info.totalDur	;
	if (!info.extItemInfo.empty())
	{
		std::string stupidAttr = info.extItemInfo;
		std::replace(stupidAttr.begin(), stupidAttr.end(), '/', '=');
		oss << ";" << stupidAttr;
	}
				
	setHeader( HeaderTianShanNoticeParam, oss.str());

	outputTransition( info, transition );

	updateRequire( false, info.c1VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceTransition,"failed to send out anounce"));
		return false;
	}

	MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceTransition,"announce sent out"));
		return true;
}

//////////////////////////////////////////////////////////////////////////
///AnnounceSkipItem
AnnounceSkipItem::AnnounceSkipItem( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest( env , manager )
{
}
AnnounceSkipItem::~AnnounceSkipItem()
{

}
bool AnnounceSkipItem::postAnnounce( const StreamAnnounceInfo& info, const TransitionAnnounceInfo& transition )
{
	if(!init( info.sessId , info.connectionId))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceStateChange,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	setHeader( HeaderScale, info.scale);

	std::string noticeStr = generatorNoticeString( NGOD_ANNOUNCE_SKIP_ITEM , NGOD_ANNOUNCE_SKIP_ITEM_STRING , generatorOffsetString(info.npt , info.c1VerCode ) );

	setHeader( HeaderNotice , noticeStr );

	setHeader(HeaderContentType,"application/sdp");

	std::stringstream ss;
	ss << "v=0" << LINETERM;
	ss << "o=- " << info.sessId << " " << getGMTString() << " IN IP4 " << info.serverIp << LINETERM;
	ss << "s=" << LINETERM;
	ss << "c=IN IP4 0.0.0.0" << LINETERM;
	if(transition.prevCtrlNum - 1 > 0)
		ss << "a=X-previous-playlist-element-index: " << transition.prevCtrlNum - 1 << LINETERM;
	else
		ss << "a=X-previous-playlist-element-index: " << 0 << LINETERM;
	ss << "a=X-current-playlist-element-index: " << transition.currentCtrlNum << LINETERM;
	setBody( ss.str() );

	updateRequire( false , info.c1VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceSkipItem,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceSkipItem,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceTrickNoConstrained
AnnounceTrickNoConstrained::AnnounceTrickNoConstrained( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest( env , manager )
{
}
AnnounceTrickNoConstrained::~AnnounceTrickNoConstrained()
{

}
bool AnnounceTrickNoConstrained::postAnnounce( const StreamAnnounceInfo& info, const TransitionAnnounceInfo& transition )
{
	if(!init( info.sessId , info.connectionId))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceTrickNoConstrained,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	setHeader( HeaderScale, info.scale);

	std::string noticeStr = generatorNoticeString( NGOD_ANNOUNCE_TRICK_NO_CONSTRAINED , NGOD_ANNOUNCE_TRICK_NO_CONSTRAINED_STRING , generatorOffsetString(info.npt , info.c1VerCode ) );

	setHeader( HeaderNotice , noticeStr );

	setHeader(HeaderContentType,"application/sdp");

	std::stringstream ss;
	ss << "v=0" << LINETERM;
	ss << "o=- " << info.sessId << " " << getGMTString() << " IN IP4 " << info.serverIp << LINETERM;
	ss << "s=" << LINETERM;
	ss << "c=IN IP4 0.0.0.0" << LINETERM;
	ss << "a=X-previous-playlist-element-index: " << transition.prevCtrlNum - 1 << LINETERM;
	ss << "a=X-current-playlist-element-index: " << transition.currentCtrlNum - 1 << LINETERM;
	setBody( ss.str() );

	updateRequire( false , info.c1VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceTrickNoConstrained,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceTrickNoConstrained,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceTrickConstrained
AnnounceTrickConstrained::AnnounceTrickConstrained( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest( env , manager )
{
}
AnnounceTrickConstrained::~AnnounceTrickConstrained()
{

}
bool AnnounceTrickConstrained::postAnnounce( const StreamAnnounceInfo& info, const TransitionAnnounceInfo& transition )
{
	if(!init( info.sessId , info.connectionId))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceTrickConstrained,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	setHeader( HeaderScale, info.scale);

	std::string noticeStr = generatorNoticeString( NGOD_ANNOUNCE_TRICK_CONSTRAINED , NGOD_ANNOUNCE_TRICK_CONSTRAINED_STRING , generatorOffsetString(info.npt , info.c1VerCode ) );

	setHeader( HeaderNotice , noticeStr );

	setHeader(HeaderContentType,"application/sdp");

	std::stringstream ss;
	ss << "v=0" << LINETERM;
	ss << "o=- " << info.sessId << " " << getGMTString() << " IN IP4 " << info.serverIp << LINETERM;
	ss << "s=" << LINETERM;
	ss << "c=IN IP4 0.0.0.0" << LINETERM;
	ss << "a=X-previous-playlist-element-index: " << transition.prevCtrlNum - 1 << LINETERM;
	ss << "a=X-current-playlist-element-index: " << transition.currentCtrlNum - 1 << LINETERM;
	setBody( ss.str() );

	updateRequire( false , info.c1VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceTrickConstrained,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceTrickConstrained,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceError
AnnounceError::AnnounceError( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest(env,manager)
{
}
AnnounceError::~AnnounceError()
{
}
void AnnounceError::setErrorReason( const std::string& reason )
{
	mErrorReason = reason;
}

bool AnnounceError::postAnnounce( const StreamAnnounceInfo& info , const std::string& noticeCode , const std::string& noticeReason  )
{
	if(!init( info.sessId ,  mSessManager.getR2ConnectionId(info.groupId)))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceError,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	setHeader( HeaderTianShanNotice , mErrorReason );
	updateNotice( noticeCode , noticeReason , generatorOffsetString(info.npt , info.r2VerCode ) );
	
	updateRequire( true , info.r2VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceError,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceError,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceInprogress
AnnounceInprogress::AnnounceInprogress( NgodEnv& env  , NgodSessionManager& manager )
:AnnounceRequest(env,manager)
{
}

AnnounceInprogress::~AnnounceInprogress()
{

}

bool AnnounceInprogress::postAnnounce( const StreamAnnounceInfo& info )
{
	if(!init( info.sessId , mSessManager.getR2ConnectionId(info.groupId)))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceInprogress,"failed to initialize announce with connection"));
		return false;
	}

	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	updateNotice( NGOD_ANNOUNCE_SESSIONINPROGRESS , NGOD_ANNOUNCE_SESSIONINPROGRESS_STRING , generatorOffsetString(info.npt , info.r2VerCode ) );

	updateRequire( true , info.r2VerCode );
	

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceInprogress,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceInprogress,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnouncePauseTimeout
AnnouncePauseTimeout::AnnouncePauseTimeout( NgodEnv& env , NgodSessionManager& manager )
:AnnounceRequest(env,manager)
{
}

AnnouncePauseTimeout::~AnnouncePauseTimeout()
{
}

bool AnnouncePauseTimeout::postAnnounce( const StreamAnnounceInfo& info )
{
	if(!init( info.sessId , mSessManager.getR2ConnectionId(info.groupId)))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnouncePauseTimeout,"failed to initialize announce with connection"));
		return false;
	}
	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	updateNotice( NGOD_ANNOUNCE_PAUSETIMEOUT , NGOD_ANNOUNCE_PAUSETIMEOUT_STRING , generatorOffsetString(info.npt , info.r2VerCode ) );

	updateRequire( true , info.r2VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnouncePauseTimeout,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnouncePauseTimeout,"announce sent out"));
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
///AnnounceTerminate
AnnounceTerminate::AnnounceTerminate( NgodEnv& env , NgodSessionManager& manager  )
:AnnounceRequest(env,manager)
{

}
AnnounceTerminate::~AnnounceTerminate()
{

}
bool AnnounceTerminate::postAnnounce( const StreamAnnounceInfo& info )
{
	if(!init( info.sessId , mSessManager.getR2ConnectionId(info.groupId)))
	{
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(AnnounceTerminate,"failed to initialize announce with connection"));
		return false;
	}
	setHeader( HeaderServer , mEnv.moduleName() );
	updateStartline( info.originalUrl );
	setHeader( HeaderSession , info.sessId );
	setHeader( HeaderOnDemandSessId , info.odsessId );	
	setHeader( HeaderSequence , info.seq );	
	updateNotice( NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED , NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED_STRING , generatorOffsetString(info.npt , info.r2VerCode ) );

	updateRequire( true , info.r2VerCode );

	if(!post())
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(AnnounceTerminate,"failed to send out anounce"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(AnnounceTerminate,"announce sent out"));
		return true;
	}
}

}//namespace NGOD

