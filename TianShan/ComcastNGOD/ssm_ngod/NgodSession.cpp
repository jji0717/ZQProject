
#include <ZQ_common_conf.h>
#include <assert.h>
#include <TianShanIceHelper.h>
#include "RtspHeaderDefines.h"
#include "NgodHelper.h"
#include "NgodConfig.h"
#include "NgodEnv.h"
#include "NgodSession.h"
#include "ClientRequest.h"
#include "AnnounceRequest.h"
#include "NgodSessionManager.h"
#include "SelectionCommand.h"
#include "SOPConfig.h"

#ifdef ZQ_OS_LINUX
#define _vsnprintf		vsnprintf
#define _vsnwprintf		vswprintf
#define _snprintf		snprintf
#define _snwprintf		vswprintf

#include <ctype.h>
#endif

#if defined ZQ_OS_MSWIN
	#define	SESSREQFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"##y, request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSREQFMT(x,y) 	"[%s]%s/%s/%s/%08X/REQUEST[%s]\t"y, request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), pthread_self(),#x	
#endif	


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"[%s]/%08X/REQUEST[%s]\t"##y, mSessId.c_str(), GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"[%s]/%08X/REQUEST[%s]\t"y, mSessId.c_str(), pthread_self(),#x	
#endif	


namespace NGOD
{

NgodSessionI::NgodSessionI( NgodEnv& env, NgodSessionManager& manager )
:mEnv(env),
mSessManager(manager)
{
}

NgodSessionI::~NgodSessionI(void)
{
}

void NgodSessionI::create( const std::string& sessId, const std::string& odSessId )
{
	updateSessState( TianShanIce::stNotProvisioned );	
	mSessId			= sessId;
	mOndemandId		= odSessId;
	mIdent.name		= sessId;
	mAnnounceSeq	= 1;
	expireTime		= 0;
	expiredCount	= 0;
	mR2VerControlCode = 0;
	mC1VerControlCode = 0;
	expireTime		= 0;
}

void NgodSessionI::updateTimer(const ::Ice::Current& ) 
{
	int32 timeout = ngodConfig.rtspSession.timeout;
	{
		WLock sync(*this);
		switch( mState )
		{
		case TianShanIce::stOutOfService:
			{
				timeout = ngodConfig.rtspSession.destroyRetryInterval;
				expiredCount	= 0;
				expireTime		= ZQTianShan::now() + timeout;
				mSessManager.updateTimer( mSessId, timeout );
			}
			break;
		default:
			{
				int64 now	= ZQTianShan::now();
				int32 diff	= (int32) ( expireTime - now );
				if( expireTime <= 0 )
					diff = timeout;
				if( diff < 5000 )
				{
					mSessManager.updateTimer( mSessId, 3000 + rand() % 10000 );
				}
				else
				{
					mSessManager.updateTimer( mSessId, diff );
				}				
			}
			break;
		}		
	}
}

void NgodSessionI::prepareAnnounceSessInfo( StreamAnnounceInfo& info )
{
	WLock sync(*this);
	info.seq			= sessAnnounceSeq();
	info.sessId			= mSessId;
	info.odsessId		= mOndemandId;
	info.originalUrl	= mOriginalUrl;
	info.serverIp		= mServerIp;
	info.r2VerCode		= (NgodProtoclVersionCode)mR2VerControlCode;
	info.c1VerCode		= (NgodProtoclVersionCode)mC1VerControlCode;
	info.connectionId	= mC1ConnectionId;
	info.groupId		= mGroupId;
	info.npt			= 0;
	info.scale			= 0.0f;	
}

int32 NgodSessionI::sessExpiredCount()
{
	WLock sync(*this);
	return ++expiredCount;
}


void NgodSessionI::pingStreamSession( StreamSessionInfo& info, int64& npt)
{	
	std::string userAgent;
	{
		WLock sync(*this);		
		ZQTianShan::Util::getPropertyDataWithDefault(mProps,PROP_CLIENT_USERAGENT, "",userAgent);
	}

	//Only use ice_ping to reduce the possibility of streamsmith hang
	//if( stricmp(userAgent.c_str(), "tianshan") == 0 )
	{
		npt = 0;
		try
		{
			mStream->ice_ping();
			MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "pingStreamSession() successfully ping stream session[%s]"),
				PROXY2STR(mStream).c_str() );
		}		
		catch( const Ice::ObjectNotExistException& ex)
		{
			MLOG(ZQ::common::Log::L_WARNING,SESSFMT(NgodSessionI, "pingStreamSession() caught [%s] while ping stream[%s], destroy current session"),
				ex.ice_name().c_str(), PROXY2STR(mStream).c_str() );
			try
			{
				destroy( NULL );
			}
			catch(...){}
		}
		catch( const Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,SESSFMT(NgodSessionI, "pingStreamSession() caught [%s] while ping stream[%s]"),
				ex.ice_name().c_str(), PROXY2STR(mStream).c_str() );
		}
	}
// 	else
// 	{
// 		if(  errorcodeOK == getStreamSessionInfo( NULL, info ) )
// 		{
// 			MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "pingStreamSession() successfully ping stream for session[%s] to get stream npt"),
// 				PROXY2STR(mStream).c_str() );
// 			npt = info.npt;
// 		}
// 	}
}

void NgodSessionI::onTimer(const ::Ice::Current& )
{
	int32 timeout		= ngodConfig.rtspSession.timeout;
	int32 timeoutCount	= ngodConfig.rtspSession.timeoutCount;

	::TianShanIce::State state;
	{
		RLock sync(*this);
		state = mState;
	}
	switch ( state )
	{
	case TianShanIce::stNotProvisioned:
		//fallthrough
	case TianShanIce::stOutOfService:
		{
			try
			{
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(NgodSessionI, "onTimer() meet stOutOfService/stNotProvisioned, destroy current session"));
				destroy(NULL);
			}
			catch( const Ice::Exception&)
			{
			}
		}
		break;
	case TianShanIce::stInService:
		{			
			StreamAnnounceInfo info;
			prepareAnnounceSessInfo(info);
			Ice::Int expired = expiredCount;
			if( sessExpiredCount() >= timeoutCount )
			{
				AnnounceTerminate term(mEnv,mSessManager);
				MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "sending SessionTerminate announce for [%s]"), mSessId.c_str() );
				if( term.postAnnounce( info ) )
					MLOG(ZQ::common::Log::L_INFO,SESSFMT(NgodSessionI, "SessionTerminate announce for [%s] has been sent out"), mSessId.c_str() );
				else
					MLOG(ZQ::common::Log::L_WARNING,SESSFMT(NgodSessionI, "failed to send out SessionTerminate announce for [%s]"), mSessId.c_str() );
				try
				{
					destroy(NULL);
				}
				catch( const Ice::Exception&)
				{
				}
			}
			else
			{
				AnnounceInprogress inprogress( mEnv, mSessManager);
				
				StreamSessionInfo streamInfo;
				pingStreamSession( streamInfo, info.npt );
				MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "sending SessionInProgress announce for [%s] expiredCount [%d]"), mSessId.c_str(), expired);
				if( inprogress.postAnnounce( info ) )
				{
					MLOG(ZQ::common::Log::L_INFO,SESSFMT(NgodSessionI, "SessionInProgress announce for [%s] has been sent out"), mSessId.c_str() );
				}
				else
				{
					MLOG(ZQ::common::Log::L_INFO,SESSFMT(NgodSessionI, "failed to send out SessionInProgress announce for [%s]"), mSessId.c_str() );
				}
				
				{
					WLock sync(*this);
					expireTime = ZQTianShan::now() + timeout;
				}

				mSessManager.updateTimer( mSessId, timeout );
			}
		}
		break;
	default:
		{
			assert(false);
			//never happen
			{
				WLock sync(*this);
				expireTime = ZQTianShan::now() + timeout;
				mSessManager.updateTimer( mSessId, timeout );
			}
			updateSessState( TianShanIce::stInService ); // Is this right ?
		}
	}
}

int32 NgodSessionI::sessAnnounceSeq( )
{
	int32 seq = 0;
	{
		WLock sync(*this);
		if (  ngodConfig.announce.useGlobalCSeq >= 1)
		{
			seq = mEnv.incAndGetSeqNumber();
		}
		else
		{
			seq = mAnnounceSeq++;
		}
	}

	return seq;
}

::std::string NgodSessionI::getSessionId(const ::Ice::Current& ) const 
{
//	RLock gd(*this);
	return mSessId;
}
::std::string NgodSessionI::getOndemandSessId(const ::Ice::Current& ) const 
{
//	RLock gd(*this);
	return mOndemandId;
}
void NgodSessionI::updateC1ConnectionId(const ::std::string& connId, const ::Ice::Current& ) 
{
	WLock gd(*this);
	mC1ConnectionId = connId;
}

::Ice::Int NgodSessionI::processRequest(const ::NGOD::NgodClientRequestPtr& request, const ::Ice::Current& c) 
{
	try
	{
		switch( request->verb )
		{
		case requestSETUP:
			{
				NgodRequestSetupPtr req = NgodRequestSetupPtr::dynamicCast(request);
				return processSetup(req);
			}
			break;

		case requestPLAY:
			{
				NgodRequestPlayPtr req = NgodRequestPlayPtr::dynamicCast(request);
				return processPlay(req);
			}
			break;

		case requestPAUSE:
			{
				NgodRequestPausePtr req = NgodRequestPausePtr::dynamicCast(request);
				return processPause(req);
			}
			break;

		case requestTEARDOWN:
			{
				NgodRequestTeardownPtr req = NgodRequestTeardownPtr::dynamicCast(request);
				return processTeardown(req);
			}
			break;

		case requestGETPARAMETER:
			{
				NgodRequestGetParameterPtr req = NgodRequestGetParameterPtr::dynamicCast(request);
				return processGetParam(req);
			}
			break;

		case requestPING:
			updateTimer(c);
			break;

		case requestRESPONSE:
			{
				NgodRequestAnnounceResponsePtr resp = NgodRequestAnnounceResponsePtr::dynamicCast(request);
				return processAnnounceResp(resp);
			}
			break;

		default:
			assert(false);
			break;
		}
	}
	catch( const Ice::Exception& ) { }
	
	//TODO: what to be returned ?
	return 0;
}

void NgodSessionI::errlog( const NgodClientRequestIPtr& request, int errCode, const char* fmt ... )
{
	char szLocalBuffer[1024];
	
	va_list args;
	va_start(args, fmt);
	int nCount = _vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, fmt, args );
	va_end(args);

	if(nCount == -1)
	{
		szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	}
	else
	{
		szLocalBuffer[nCount] = '\0';
	}

	ServerResponsePtr response = ServerResponsePtr::dynamicCast(request->getResponse());	
	response->setLastErr( errCode, szLocalBuffer );
	if(ngodConfig.publishLogs.enabled)
	{
		ELOG(ZQ::common::Log::L_ERROR, EVENTLOGFMT(NgodSessionI, "state(%s) %s"), errorCodeTransformer(errCode), szLocalBuffer);
	}
#if defined ZQ_OS_MSWIN
	MLOG(ZQ::common::Log::L_ERROR, "[%s]%s/%s/%s/%08X/REQUEST[%s]\t%s", request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), GetCurrentThreadId(), "NgodSessionI", szLocalBuffer );
#elif defined ZQ_OS_LINUX
	MLOG(ZQ::common::Log::L_ERROR, "[%s]%s/%s/%s/%lu/REQUEST[%s]\t%s", request->sessionId.c_str(), request->cseq.c_str(), request->verbstr.c_str(), request->ondemandId.c_str(), (unsigned int)pthread_self(), "NgodSessionI", szLocalBuffer );
#endif	

}

typedef struct _ErrorDescArray 
{
	char*	errorCodeStr;
	char*	errorDescStr;
}ErrorDescArray;

static ErrorDescArray errors[]=
{
	{"", ""},
	{NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR,		NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING },
	{NGOD_ANNOUNCE_ERROR_READING_CONTENT,		NGOD_ANNOUNCE_ERROR_READING_CONTENT_STRING },
	{NGOD_ANNOUNCE_DOWNSTREAM_FAILURE,			NGOD_ANNOUNCE_DOWNSTREAM_FAILURE_STRING },
	{NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT,	NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT_STRING	},
	{NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE,		NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE_STRING }
};

void NgodSessionI::recordEvent_ItemStepped( const NGOD::StreamEventAttr& a)
{
	if( !( ngodConfig.sessionHistory.enableHistory >= 1 && a.prevCtrlNum > 0 && a.currentCtrlNum > 0 ) )
		return;
	Ice::Int oldAssetNpt = -1;
	Ice::Int newAssetNpt = -1;
	std::ostringstream ossOldNpt;
	std::ostringstream ossNewNpt;

	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "oldItemNPT", -1, oldAssetNpt );
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "newItemNPT", -1, newAssetNpt );
	
	if( oldAssetNpt >= 0 )
	{
		char szBufferOldNpt[32];
		sprintf(szBufferOldNpt, "%d.%03d",oldAssetNpt/1000,oldAssetNpt%1000);
		ossOldNpt << szBufferOldNpt;
	}
	else
	{
		ossOldNpt << "EOS";
	}

	if( newAssetNpt >= 0 )
	{
		char szBufferNewNpt[32];
		sprintf(szBufferNewNpt, "%d.%03d",newAssetNpt/1000,newAssetNpt%1000);
		ossNewNpt << szBufferNewNpt;
	}
	else
	{
		ossNewNpt << "BOS";
	}

	Ice::Int state = TianShanIce::Streamer::stsStreaming;
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "newState", TianShanIce::Streamer::stsStreaming, state );

	std::string scale;
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "scale", "1.000", scale );

	std::ostringstream ossResId;
	ossResId << a.prevCtrlNum;

	std::ostringstream ossNewResId;
	ossNewResId << a.currentCtrlNum;

	SessionEventRecord record;
	
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "echo.Time", getISOTimeString(), record.eventTime);
	record.eventType			= NGOD::Transition;
	record.NPT					= ossOldNpt.str();
	record.prop["newNPT"]		= ossNewNpt.str();
	record.streamResourceID		= ossResId.str();
	record.prop["newStreamResourcesID"] = ossNewResId.str();
	record.prop["newState"]		= ( state == TianShanIce::Streamer::stsStreaming ? "PLAY" : "PAUSE" );
	record.prop["scale"]		= scale;
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "reason", "SERVER", record.prop["reason"]);

	recordEvent( record );

	int errorCode;
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "ItemSkipErrorCode", 0, errorCode);
	
	if (errorCode > 0)
	{
		record.eventType				= NGOD::RecoverableError;
		record.prop["errorCode"]		= errors[errorCode].errorCodeStr;
		record.prop["errorDescription"] = errors[errorCode].errorDescStr;
		recordEvent( record );		
	}
}

void NgodSessionI::recordEvent_Reposition( const NGOD::StreamEventAttr& a)
{
	if( !( ngodConfig.sessionHistory.enableHistory >= 1 ) )
		return;
	Ice::Int oldAssetNpt = -1;
	Ice::Int newAssetNpt = -1;
	std::ostringstream ossOldNpt;
	std::ostringstream ossNewNpt;

	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "oldItemNPT", -1, oldAssetNpt );
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "newItemNPT", -1, newAssetNpt );

	if( oldAssetNpt >= 0 )
	{
		char szBufferOldNpt[32];
		sprintf(szBufferOldNpt, "%d.%03d",oldAssetNpt/1000,oldAssetNpt%1000);
		ossOldNpt << szBufferOldNpt;
	}
	else
	{
		ossOldNpt << "EOS";
	}

	if( newAssetNpt >= 0 )
	{
		char szBufferNewNpt[32];
		sprintf(szBufferNewNpt, "%d.%03d",newAssetNpt/1000,newAssetNpt%1000);
		ossNewNpt << szBufferNewNpt;
	}
	else
	{
		ossNewNpt << "BOS";
	}

	Ice::Int state = TianShanIce::Streamer::stsStreaming;
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "newState", TianShanIce::Streamer::stsStreaming, state );

	std::string scale;
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "scale", "1.000", scale );

	//reposition作用的item的ctrlNum
	std::string strCtrlNum;
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "newItemIndex", "", strCtrlNum);

	SessionEventRecord record;

	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "echo.Time", getISOTimeString(), record.eventTime);
	record.eventType			= NGOD::Transition;
	record.NPT					= ossOldNpt.str();
	record.prop["newNPT"]		= ossNewNpt.str();
	record.streamResourceID		= strCtrlNum;
	record.prop["newStreamResourcesID"] = strCtrlNum;
	record.prop["newState"]		= ( state == TianShanIce::Streamer::stsStreaming ? "PLAY" : "PAUSE" );
	record.prop["scale"]		= scale;
	record.prop["reason"]		= "USER";

	recordEvent( record );
}

void NgodSessionI::prepareAnnounceStreamInfo( const ::NGOD::StreamEventAttr& a, StreamAnnounceInfo& info )
{	
	ZQTianShan::Util::getPropertyDataWithDefault( a.props, "streamNptPosition", 0, info.npt );
	info.scale	= a.currentSpeed;
	info.state	= a.currentState;
	mEnv.getStatCache().statChanged(a.playlistString, info.npt, info.scale, info.state );

	int64 nptPrimary = nptToPrimary(info.npt, true);
	char buf[40];
	snprintf(buf, sizeof(buf)-2, "%d.%03d", (int) (nptPrimary/1000), (int) (nptPrimary%1000));
	info.nptPrimary = buf;
}

//get data from a.props when the ItemStepped event.
void NgodSessionI::getEventItemStepProps(const ::NGOD::StreamEventAttr& a, StreamAnnounceInfo& info )
{
// 	info.StreamPrevItemNptPrimary = a.props.find("StreamPrevItemNptPrimary")->second;
// 	info.curPAssetId = a.props.find("currentProviderAssetId")->second;
// 	info.prevPAssetId =  a.props.find("prevProviderAssetId")->second;
// 	info.curProviderId =  a.props.find("currentProviderId")->second ;
// 	info.prevProviderId = a.props.find("prevProviderId")->second ;
// 	info.curdur = a.props.find("StreamingCurrentPlayTime")->second;
// 	info.prevdur = a.props.find("StreamingPreviousPlayTime")->second;
// 	info.curflags = a.props.find("StreamingCurrentFlag")->second;
// 	info.prevflags =  a.props.find("StreamingPreviousFlag")->second;

//	getPropertyData( const TianShanIce::Properties& props, const std::string& key, std::string& valueOut ) throw (::TianShanIce::InvalidParameter);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "StreamPrevItemNptPrimary", "", info.nptPrimary);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "currentProviderAssetId",   "", info.curPAssetId);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "prevProviderAssetId",      "", info.prevPAssetId);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "currentProviderId",        "", info.curProviderId);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "prevProviderId",           "", info.prevProviderId);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "StreamingCurrentPlayTime", "", info.curdur);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "StreamingPreviousPlayTime","", info.prevdur);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "StreamingCurrentFlag",     "", info.curflags);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "StreamingPreviousFlag",    "", info.prevflags);

	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "StreamingTotalVideoDuration","",info.totalVideodur);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props, "StreamingTotalDuration",   "",info.totalDur);
	info.ItemCurName =info.curPAssetId + info.curProviderId;
	info.ItemPrevName =  info.prevPAssetId + info.prevProviderId;
}

std::string streamEventAttrToString(const ::NGOD::StreamEventRoutine r, const ::NGOD::StreamEventAttr& a )
{
	std::ostringstream oss;
	switch (r)
	{
	case streamEventPING:
		{
			oss<< "StreamEventPING";
		}
		break;
	case streamEventENDOFSTREAM:
		{
			oss<< "EndOfStream";
		}
		break;
	case streamEventBEGINOFSTREAM:
		{
			oss<< "BeinningOfStream";
		}
		break;
	case streamEventSPEEDCHANGE:
		{
			oss<<"SpeedChange prevSpeed["<<a.previousSpeed <<"] curSpeed["<<a.currentSpeed<<"]";
		}
		break;
	case streamEventSTATECHANGE:
		{
			oss<<"StateChange prevState["<<ZQTianShan::Util::convertStreamStateToString(a.prevState)<<"] curState["<<ZQTianShan::Util::convertStreamStateToString(a.currentState)<<"]";
		}
		break;
	case streamEventITEMSTEP:
		{
			oss<<"ItemStep lastItem["<<a.prevCtrlNum<<"] curItem["<<a.currentCtrlNum<<"]";
		}
		break;
	case streamEventEXIT:
		{
			oss<<"PlaylistDestroyed";
		}
		break;
	case streamEventEXIT2:
		{
			oss<<"PlaylistDestroyed";
		}
		break;
	case streamReposition:
		{
			oss<<"StreamReposition";
		}
		break;
	case streamEventPauseTimeout:
		{
			oss<<"streamPauseTimeout";
		}
		break;
	default:		
		break;
	}
	return oss.str();
}

bool NgodSessionI::isEventIndexValid( ::NGOD::StreamEventRoutine r, const ::NGOD::StreamEventAttr& a)
{
	if( a.eventIndex < 0 )
	{
		return true; // continue to process if no event index at all
	}

	const char* indexKey = 0;
#define EVENT_INDEXKEY(x) "Sys.Prop."#x
	switch (r)
	{
	case streamEventPING:
		{
			indexKey = EVENT_INDEXKEY(streamEventPING);
		}
		break;
	case streamEventENDOFSTREAM:
		{
			indexKey = EVENT_INDEXKEY(streamEventENDOFSTREAM);
		}
		break;
	case streamEventBEGINOFSTREAM:
		{
			indexKey = EVENT_INDEXKEY(streamEventBEGINOFSTREAM);
		}
		break;
	case streamEventSPEEDCHANGE:
		{
			indexKey = EVENT_INDEXKEY(streamEventSPEEDCHANGE);
		}
		break;
	case streamEventSTATECHANGE:
		{
			indexKey = EVENT_INDEXKEY(streamEventSTATECHANGE);
		}
		break;
	case streamEventITEMSTEP:
		{
			indexKey = EVENT_INDEXKEY(streamEventITEMSTEP);
		}
		break;
	case streamEventEXIT:
		{
			indexKey = EVENT_INDEXKEY(streamEventEXIT);
		}
		break;
	case streamEventEXIT2:
		{
			indexKey = EVENT_INDEXKEY(streamEventEXIT2);
		}
		break;
	case streamReposition:
		{
			indexKey = EVENT_INDEXKEY(streamReposition);
		}
		break;
	case streamEventPauseTimeout:
		{
			indexKey = EVENT_INDEXKEY(streamEventPauseTimeout);
		}
		break;
	default:
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(NgodSessionI, "unknonw event"));
		return false;
	}
	
	Ice::Long	lastEventIndex  = -2;
	{
		WLock sync(*this);
		ZQTianShan::Util::getPropertyDataWithDefault( mProps, indexKey, -2, lastEventIndex);
	
		if( lastEventIndex >= a.eventIndex )
		{
			MLOG(ZQ::common::Log::L_WARNING,SESSFMT(NgodSessionI, "current eventIndex[%lld], lastEventIndex[%lld], reject to send announce to client"),
				a.eventIndex, lastEventIndex );
			return false;
		}
		ZQTianShan::Util::updatePropertyData( mProps, indexKey, a.eventIndex );
		return true;
	}
}

::Ice::Int NgodSessionI::onStreamSessionEvent(::NGOD::StreamEventRoutine r, const ::NGOD::StreamEventAttr& a, const ::Ice::Current& )
{
	MLOG(ZQ::common::Log::L_INFO,SESSFMT( NgodSessionI, "onStreamSessionEvent: %s"), streamEventAttrToString( r, a ).c_str() );

	if(!isEventIndexValid(r,a))
		return 0;

	StreamAnnounceInfo info;
	prepareAnnounceSessInfo( info );
	if (r == streamEventITEMSTEP)
	{
		char tmp[100];
		std::string strProps;
		getEventItemStepProps(a, info);
		TianShanIce::Properties::const_iterator it;
		for(it = a.props.begin();it!=a.props.end();++it) 
		{
			snprintf(tmp, sizeof(tmp)-2, "%s[%s],", (it->first).c_str(), (it->second).c_str());
			strProps += tmp;
		}

		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(NgodSessionI, "%s"), strProps.c_str());
	}

	prepareAnnounceStreamInfo( a, info );	
	{
		RLock sync(*this);
		ZQTianShan::Util::getPropertyDataWithDefault( mProps, "sys.primaryItemNPT", "", info.primaryItemNPT);
	}

	bool bOk = true;
	switch( r )
	{
	case streamEventENDOFSTREAM:
		{
			if(ngodConfig.announce.notifyTrickRestriction > 0)
			{
				// get scaleOf2204
				std::string scaleOf2204;
				{
					RLock sync(*this);
					ZQTianShan::Util::getPropertyDataWithDefault( mProps, "scaleOf2204", "", scaleOf2204);
				}

				if(scaleOf2204 != "")
				{					
					// send c1 announce 2201
					AnnounceTrickNoConstrained announce( mEnv, mSessManager );
					TransitionAnnounceInfo transitionInfo;
					transitionInfo.prevCtrlNum		= a.prevCtrlNum;
					transitionInfo.currentCtrlNum	= a.currentCtrlNum;
					bOk = announce.postAnnounce( info, transitionInfo );
 					// reset scaleOf2204
					{
						WLock sync(*this);
						ZQTianShan::Util::updatePropertyData( mProps, "scaleOf2204", "");
					}
				}
			}

			{
				//ticket#18642, force to take the last ctrlnum of the playlist, which equals to the playlist size
				std::string tmp;
				ZQTianShan::Util::getPropertyDataWithDefault(a.props, "ItemIndexOnEndOfStream", "", tmp);
				int ctrlNum= atol(tmp.c_str());

				if (ctrlNum <=0)
				{
					ZQTianShan::Util::getPropertyDataWithDefault(mProps, "PlayListTotalSize", "", tmp);
					ctrlNum= atol(tmp.c_str());
				}

				char buf[10];
				snprintf(buf, sizeof(buf)-2, "%d", ctrlNum);
				recordEvent_StreamEnd("SERVER", "EOS", buf);
			}

			AnnounceEndOfStream announce( mEnv, mSessManager );
			bOk = announce.postAnnounce( info );
		}
		break;

	case streamEventBEGINOFSTREAM:
		{
			AnnounceBeginOfStream announce( mEnv, mSessManager );
			bOk = announce.postAnnounce( info );
		}
		break;

	case streamEventSPEEDCHANGE:
		{
			bOk = true;
			if( ngodConfig.announce.useTianShanAnnounceCodeScaleChanged >= 1)
			{
				{
					char scale[20] = {0};
					snprintf(scale, sizeof(scale), "%f", a.currentSpeed);
					WLock sync(*this);
					ZQTianShan::Util::updatePropertyData( mProps, "lastScale", scale);
				}
				AnnounceScaleChange announce( mEnv, mSessManager);
				std::string perRequest;
				ZQTianShan::Util::getPropertyData (a.props, "perRequested",perRequest);
				if(ngodConfig.announce.skipEventOfRequested == 0 || perRequest == "0" )
					bOk = announce.postAnnounce( info );
 				MLOG(ZQ::common::Log::L_INFO,SESSFMT(NgodSessionI, 
 					"streamEventSPEEDCHANGE skipEventOfRequested [%d] in ssm_ngod2.xml perRequest [%s]"),ngodConfig.announce.skipEventOfRequested , perRequest.c_str());

			}
		}
		break;

	case streamEventSTATECHANGE:
		{
			bOk = true;
			if( ngodConfig.announce.useTianShanAnnounceCodeStateChanged >= 1)
			{
				AnnounceStateChange announce( mEnv, mSessManager );
				std::string perRequest;
				ZQTianShan::Util::getPropertyData (a.props, "perRequested",perRequest);
				if(ngodConfig.announce.skipEventOfRequested == 0 || perRequest == "0" )
				bOk = announce.postAnnounce( info );
 				MLOG(ZQ::common::Log::L_INFO,SESSFMT(NgodSessionI, 
 					"streamEventSTATECHANGE skipEventOfRequested [%d] in ssm_ngod2.xml perRequest [%s]"),ngodConfig.announce.skipEventOfRequested , perRequest.c_str());
			}
		}
		break;

	case streamEventPauseTimeout:
		{
			AnnouncePauseTimeout announce( mEnv, mSessManager );
			bOk = announce.postAnnounce( info );
		}
		break;

	case streamEventITEMSTEP:
		{
			bool bPerUserRequest = false;
			{
				std::string tmpstr;
				ZQTianShan::Util::getPropertyDataWithDefault(a.props, "ItemExitReason", "", tmpstr);
				std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), tolower);
				if (std::string::npos != tmpstr.find("userreq"))
					bPerUserRequest = true;
			}

			bool needToIssue2205 = false;
			bool needOdrmTransition  = true;

			int currentUserCtrlNum = a.currentCtrlNum;
			int prevUserCtrlNum = a.prevCtrlNum;

			int newStreamResourcesID = currentUserCtrlNum;
			int currentIndex = currentUserCtrlNum - 1;
			int streamResourcesID = prevUserCtrlNum;

			::Ice::Int renderedFirst = 0;
			::Ice::Int renderedLast = 0;
			int totalSize = 0;
			{
				RLock sync(*this);
				ZQTianShan::Util::getPropertyDataWithDefault( mProps, "PlayListTotalSize", 0, totalSize);
			}

			int listSize = (int)mItemInfos.size();
			if (currentUserCtrlNum < 0)
			{
				// find out the ctrlNums of the beginning and end of the true playlist of
				if( listSize > 0 )
				{
					if(mItemInfos[0].privateData.find("currentCtrlNum") != mItemInfos[0].privateData.end() && !mItemInfos[0].privateData["currentCtrlNum"].ints.empty())
					{
						renderedFirst = mItemInfos[0].privateData["currentCtrlNum"].ints[0];
					}
					if(mItemInfos[listSize-1].privateData.find("currentCtrlNum") != mItemInfos[listSize-1].privateData.end() && !mItemInfos[listSize-1].privateData["currentCtrlNum"].ints.empty())
					{
						renderedLast = mItemInfos[listSize-1].privateData["currentCtrlNum"].ints[0];
					}
				}
			}
			else if (currentIndex>=0 && currentIndex < listSize)
			{
				::TianShanIce::ValueMap& itemPrivData = mItemInfos[currentIndex].privateData;

				if (itemPrivData.find("extAds") != itemPrivData.end() && itemPrivData["extAds"].strs.size() >0)
					info.extItemInfo = itemPrivData["extAds"].strs[0];
			}

			if(ngodConfig.announce.notifyItemSkip > 0)
			{
				if (abs(currentUserCtrlNum - prevUserCtrlNum) >1)
				{
					needToIssue2205 = true;
					if (TianShanIce::Streamer::PlaylistHeadCtrlNum == currentUserCtrlNum)
					{
						// reached the begining of list
						// adjust the X-current-playlist-element-index to renderedFirst-1;
						// adjust the newStreamResourcesID to renderedFirst
						currentIndex = renderedFirst - 1;
						newStreamResourcesID = renderedFirst;
					}

					if (TianShanIce::Streamer::PlaylistTailCtrlNum == currentUserCtrlNum)
					{
						// reached the end of list
						newStreamResourcesID = renderedLast;

						// withdraw 2205: needToIssue2205 =false 
						//if (prevUserCtrlNum == totalSize)
							needToIssue2205 =false;
					}

					if (prevUserCtrlNum == TianShanIce::Streamer::PlaylistHeadCtrlNum && currentUserCtrlNum !=1)
					{
						streamResourcesID = 1;
					}

					if (prevUserCtrlNum == TianShanIce::Streamer::PlaylistTailCtrlNum && currentUserCtrlNum != totalSize)
					{
						streamResourcesID = renderedLast;
					}
				}

				needToIssue2205 = needToIssue2205 && (newStreamResourcesID != prevUserCtrlNum);

				if (prevUserCtrlNum == TianShanIce::Streamer::PlaylistHeadCtrlNum && currentUserCtrlNum !=1)
					needToIssue2205 = true;

				if(prevUserCtrlNum == TianShanIce::Streamer::PlaylistHeadCtrlNum && currentUserCtrlNum ==1)
					needToIssue2205 = false;

				if (prevUserCtrlNum == TianShanIce::Streamer::PlaylistTailCtrlNum && currentUserCtrlNum !=totalSize)
					needToIssue2205 = true;

				if (prevUserCtrlNum == TianShanIce::Streamer::PlaylistTailCtrlNum && currentUserCtrlNum ==totalSize)
					needToIssue2205 = false;

				//sending announce to C1 2205
				if(!bPerUserRequest && needToIssue2205)
				{
					AnnounceSkipItem announce( mEnv, mSessManager );
					TransitionAnnounceInfo transitionInfo;
					transitionInfo.prevCtrlNum		= prevUserCtrlNum;
					transitionInfo.currentCtrlNum	= currentIndex;
					bOk = announce.postAnnounce( info, transitionInfo );
				}
			}

			if(ngodConfig.announce.notifyTrickRestriction > 0 && currentUserCtrlNum > 0 && prevUserCtrlNum > 0)
			{
				// get scaleOf2204
				std::string scaleOf2204;
				{
					RLock sync(*this);
					ZQTianShan::Util::getPropertyDataWithDefault( mProps, "scaleOf2204", "", scaleOf2204);
				}
				if(scaleOf2204 != "")
				{
					// get current scale
					Ice::Float fScale;
					sscanf(scaleOf2204.c_str(), "%f", &fScale);
					// get current tricks
					::Ice::Long tricks;
					NGOD::PlaylistItemSetupInfos::iterator resIter = mItemInfos.begin();
					for (;  resIter != mItemInfos.end(); resIter++)
					{
						if(resIter->privateData.find("currentCtrlNum") != resIter->privateData.end() && !resIter->privateData["currentCtrlNum"].ints.empty())
						{
							if(resIter->privateData["currentCtrlNum"].ints[0] == currentUserCtrlNum)
							{
								tricks = resIter->flags;
								break;
							}
						}
					}
					bool bAllowed = true;
					if(fScale > 1.000 && (tricks & TianShanIce::Streamer::PLISFlagNoFF) == TianShanIce::Streamer::PLISFlagNoFF)
					{
						bAllowed = false;
					}
					if(fScale < 0.000 && (tricks & TianShanIce::Streamer::PLISFlagNoRew) ==  TianShanIce::Streamer::PLISFlagNoRew)
					{
						bAllowed = false;
					}
					if (bAllowed)
					{
						// send c1 announce 2201
						AnnounceTrickNoConstrained announce( mEnv, mSessManager );
						TransitionAnnounceInfo transitionInfo;
						transitionInfo.prevCtrlNum		= prevUserCtrlNum;
						transitionInfo.currentCtrlNum	= currentUserCtrlNum;
						bOk = announce.postAnnounce( info, transitionInfo );
						// reset scaleOf2204
						{
							WLock sync(*this);
							ZQTianShan::Util::updatePropertyData( mProps, "scaleOf2204", "");
						}
					}
				}

				// get lastScale
				std::string lastScale;
				{
					RLock sync(*this);
					ZQTianShan::Util::getPropertyDataWithDefault( mProps, "lastScale", "", lastScale);
				}

				if(lastScale != "")
				{
					// get current scale
					Ice::Float fScale;
					sscanf(lastScale.c_str(), "%f", &fScale);
					// get current tricks
					::Ice::Long tricks;

					NGOD::PlaylistItemSetupInfos::iterator resIter = mItemInfos.begin();
					for (;  resIter != mItemInfos.end(); resIter++)
					{
						if(resIter->privateData.find("currentCtrlNum") != resIter->privateData.end() && !resIter->privateData["currentCtrlNum"].ints.empty())
						{
							if(resIter->privateData["currentCtrlNum"].ints[0] == currentUserCtrlNum)
							{
								tricks = resIter->flags;
								break;
							}
						}
					}
					bool bAllowed = true;
					if(fScale > 1.000 && (tricks & TianShanIce::Streamer::PLISFlagNoFF) == TianShanIce::Streamer::PLISFlagNoFF)
					{
						bAllowed = false;
					}

					if(fScale < 0.000 && (tricks & TianShanIce::Streamer::PLISFlagNoRew) == TianShanIce::Streamer::PLISFlagNoRew)
					{
						bAllowed = false;
					}

					if (!bAllowed)
					{
						// send c1 announce 2204
						AnnounceTrickConstrained announce( mEnv, mSessManager );
						TransitionAnnounceInfo transitionInfo;
						transitionInfo.prevCtrlNum		= prevUserCtrlNum;
						transitionInfo.currentCtrlNum	= currentUserCtrlNum;
						bOk = announce.postAnnounce( info, transitionInfo );						
						// reset scaleOf2204
						{
							WLock sync(*this);
							ZQTianShan::Util::updatePropertyData( mProps, "scaleOf2204", lastScale);
						}
					}
				}
			}
			
			if(prevUserCtrlNum == TianShanIce::Streamer::PlaylistHeadCtrlNum)
				needOdrmTransition = false;
			if (prevUserCtrlNum == TianShanIce::Streamer::PlaylistTailCtrlNum && currentUserCtrlNum ==totalSize)
				needOdrmTransition = false;
			needOdrmTransition = needOdrmTransition && (newStreamResourcesID != prevUserCtrlNum);
			if (ngodConfig.sessionHistory.enableHistory > 0 && needOdrmTransition)
			{
				recordEvent_ItemStepped( a );
			}

			if( a.prevCtrlNum > 0 && a.currentCtrlNum > 0 )
			{
				AnnounceTransition announce( mEnv, mSessManager );
				TransitionAnnounceInfo transitionInfo;
				transitionInfo.prevCtrlNum		= a.prevCtrlNum;
				transitionInfo.currentCtrlNum	= a.currentCtrlNum;
				bOk = announce.postAnnounce( info, transitionInfo );
//				recordEvent_ItemStepped( a );
			}

			{
				std::string scale;
				WLock sync(*this);
				ZQTianShan::Util::getPropertyDataWithDefault( a.props, "scale", "1.000", scale );				
				ZQTianShan::Util::updatePropertyData( mProps, "lastScale", scale);
			}

			Ice::Int errorCode = 0;
			ZQTianShan::Util::getPropertyDataWithDefault( a.props, "ItemSkipErrorCode", 0, errorCode );
			if( errorCode > 0 && errorCode < (sizeof(errors)/sizeof(errors[0])) )
			{
				AnnounceError announce( mEnv, mSessManager );
				std::string errorDetail;
				ZQTianShan::Util::getPropertyDataWithDefault( a.props, "errorDetail", "", errorDetail);
				announce.setErrorReason( errorDetail );

				std::string errorDesc;
				ZQTianShan::Util::getPropertyDataWithDefault( a.props, "ItemSkipErrorDescription", "", errorDesc );
				announce.postAnnounce( info, errors[errorCode].errorCodeStr, errors[errorCode].errorDescStr );				
			}
		}
		break;

	case streamReposition:
		recordEvent_Reposition( a );		
		break;

	default:		
		break;
	}

	return 0;
}

// void prepareRemoteAssetsInfo( const NGOD::PlaylistItemSetupInfos& itemInfos, RemoteAssetStack::RemoteAssetS& assets )
// {
// 	NGOD::PlaylistItemSetupInfos::const_iterator itElement = itemInfos.begin();
// 	for( ; itElement != itemInfos.end() ; itElement++ )
// 	{
// 		RemoteAssetStack::AssetKey k;
// 		ZQTianShan::Util::getValueMapDataWithDefault( itElement->privateData, "providerId", "", k.pid );
// 		ZQTianShan::Util::getValueMapDataWithDefault( itElement->privateData, "providerAssetId", "", k.paid );
// 		assets.insert( k );
// 	}
// }

void NgodSessionI::onRestore(const ::Ice::Current& )
{
	mSessManager.restoreRtspSession(mSessId, mOriginalUrl);
	StreamerResourcePara resPara;
	{
		RLock sync(*this);
		//	allocateResource( const StreamerResourcePara& para, const std::string& streamerNetId );		
		resPara.bNeedImportChannel		= !mImportChannelName.empty();
		resPara.cseq					= "0";
		resPara.identifier				= mSopName;
		resPara.method					= "RESTORE";
		resPara.requestBW				= (int32)mUsedBW;
		resPara.sessionId				= mSessId;
		resPara.bRestore				= true;
	}

	if( !mEnv.getSelResManager().allocateResource( resPara, mStreamerNetId) )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(NgodSessionI, "failed to restore session[%s], go into outOfService State"), mSessId.c_str() );
		mStreamerNetId = "";//prevent destroy routine to release the resource again
		updateSessState(TianShanIce::stOutOfService);
		mSessManager.updateTimer( mSessId, 2000 + rand() % 10000 );
	}
	else
	{
		//update asset stack information
// 		RemoteAssetStack::RemoteAssetS assets;
// 		prepareRemoteAssetsInfo( mItemInfos, assets);
//		mEnv.getSelEnv().getAssetStackManager().registerSession( assets, mStreamerNetId );
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(NgodSessionI, "session[%s] was restored from database"), mSessId.c_str());
		mSessManager.updateTimer( mSessId, 3000 + rand() % 10000 );
	}
}

bool NgodSessionI::recordStreamDestroyStatusAndUpdateTimer( bool update )
{
	Ice::Long destroyTime = 0 ;
	{
#define STREAM_SESSION_DESTROY_TIME SYS_PROP(TimeStampForFirstStreamDestroy)
		{
			RLock sync(*this);
			ZQTianShan::Util::getPropertyDataWithDefault(mProps,STREAM_SESSION_DESTROY_TIME,0,destroyTime);
		}

		if(destroyTime <= 0)
		{
			destroyTime = ZQ::common::now();
			WLock sync(*this);			
			ZQTianShan::Util::updatePropertyData(mProps,STREAM_SESSION_DESTROY_TIME, destroyTime );			
		}
	}

	int64 timeDelta = ZQ::common::now() - destroyTime;
	if( update || (timeDelta <  ngodConfig.rtspSession.maxSessionDestroyTimeInterval) )
	{
		updateTimer();
		return true;
	}

	MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(NgodSessionI, "it took us [%lld]ms to destroy session[%s]'s stream[%s] but failed, destroy action will be terminated. mark as destroy successfully"),
		timeDelta, mSessId.c_str(), mStreamSessId.c_str() );
	return false;
}

int NgodSessionI::destroy( const ::NGOD::NgodClientRequestPtr& requesti, const ::Ice::Current& )
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionI, "destroy() trying to destroy session[%s]"), mSessId.c_str() );

	NgodClientRequestIPtr request = NULL;
	if( requesti)
	{
		request = NgodClientRequestIPtr::dynamicCast(requesti);
	}
	
	int errCode = errorcodeOK;
	std::string errMsg;
	TianShanIce::Streamer::PlaylistPrx stream = NULL;
	{
		RLock sync(*this);
		stream = mStream;
	}

	if( stream)
	{
		try
		{
			stream->destroy();
		}
		catch( const Ice::ObjectNotExistException& )
		{
		}
		catch( const Ice::TimeoutException& ex)
		{
			errCode = errorcodeServiceUnavail;
			errMsg = ex.ice_name();
		}
		catch( const Ice::SocketException& ex)
		{
			errCode = errorcodeServiceUnavail;
			errMsg = ex.ice_name();
		}
		catch( const TianShanIce::BaseException& ex)
		{
			errCode = errorcodeInternalError;
			errMsg = ex.message;
		}
		catch( const Ice::Exception& ex)
		{
			errCode = errorcodeInternalError;
			errMsg = ex.ice_name();
		}

		if( errCode != errorcodeOK )
		{
			if(request)
			{
				errlog( request, errCode, "failed to destroy stream[%s] due to [%s]", mStreamSessId.c_str(), errMsg.c_str() );
			}
			else
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSessionI, "failed to destroy stream session[%s] due to [%s]"), 
					mSessId.c_str(), errMsg.c_str() );
			}

			if(recordStreamDestroyStatusAndUpdateTimer())
				return errCode;
		}		
	}	
	
	{
		WLock sync(*this);		
		if( !mStreamerNetId.empty() )
		{
			//release resource
			StreamerResourcePara resPara;
			resPara.bNeedImportChannel	= !mImportChannelName.empty();
			resPara.cseq				= request ? request->cseq : "9999";
			resPara.identifier			= mSopName;
			resPara.method				= "destroy";
			resPara.requestBW			= (int32)mUsedBW;
			resPara.sessionId			= mSessId;
			mEnv.getSelResManager().releaseResource( resPara, mStreamerNetId );

// 			RemoteAssetStack::RemoteAssetS assets;
// 			prepareRemoteAssetsInfo( mItemInfos, assets );
// 			mEnv.getSelEnv().getAssetStackManager().unregisterSession( assets, mStreamerNetId );

			mStreamerNetId = "";//clear streamer net id
		}

		if( !mSessId.empty() )
		{
			mSessManager.destroyRtspSession( mSessId );
			mEnv.getStatCache().removeStat( mSessId );
			mSessId = ""; // clear session id
		}
	}

	mSessManager.cancelTimer( mIdent.name );
	mSessManager.getDatabase().removeSession( mIdent.name );
	
	
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionI, "session[%s] destroyed"), mIdent.name.c_str() );

	return errorcodeOK;
}
void NgodSessionI::updateSessState( const TianShanIce::State& state )
{
	WLock sync(*this);
	mState = state;
}

void NgodSessionI::convertToCtxData(::NGOD::ctxData& data, const ::Ice::Current& )
{
	RLock sync(*this);

	data.announceSeq	= mAnnounceSeq;
	data.connectID		= mC1ConnectionId;
	data.expiration		= expireTime;
	data.groupID		= mGroupId;
	data.ident			= mIdent;
	data.importChannelName = mImportChannelName;
	data.normalURL		= mOriginalUrl;
	data.onDemandID		= mOndemandId;
	data.sopname		= mSopName;
	data.streamFullID	= PROXY2STR(mStream);
	data.streamNetId	= mStreamerNetId;
	data.streamShortID	= mStreamSessId;
	data.usedBandwidth	= mUsedBW;
}

bool isR2Request( NGOD::NgodClientRequestPtr request )
{
	switch ( request->verb )
	{
	case NGOD::requestSETUP:
	case NGOD::requestTEARDOWN:
		return true;
	default:
		return false;
	}
}

void NgodSessionI::updateConnectionAndVerCode( NGOD::NgodClientRequestPtr request )
{
	{
		WLock sync(*this);
		if( !isR2Request(request) )
		{
			updateC1ConnectionId( request->connectionId );
			mC1VerControlCode = request->protocolVerCode;
		}
		else
		{			
			mR2VerControlCode = request->protocolVerCode;			
		}
	}

	updateTimer();
}

bool NgodSessionI::isSessionStreaming()
{
	TianShanIce::Streamer::StreamState state = TianShanIce::Streamer::stsSetup;
	if( getStreamState(state))
		return ( state == TianShanIce::Streamer::stsStreaming || state == TianShanIce::Streamer::stsPause );

	return false;
}

bool NgodSessionI::getStreamState( TianShanIce::Streamer::StreamState& state )
{
	try
	{
		state = mStream->getCurrentState();
	}
	catch( const Ice::TimeoutException& )
	{
		if(sopConfig.sopRestrict.penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR && !mSopName.empty() && !mStreamerNetId.empty())
			mEnv.mSelManager.applyIncreasablePenalty( mSessId, mStreamerNetId, sopConfig.sopRestrict.timeoutPenalty,sopConfig.sopRestrict.maxPenaltyValue);
		return false;
	}
	catch( const Ice::SocketException& )
	{
		if(sopConfig.sopRestrict.penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR && !mSopName.empty() && !mStreamerNetId.empty())
			mEnv.mSelManager.applyPenalty( mSessId, mStreamerNetId, sopConfig.sopRestrict.maxPenaltyValue);
		return false;
	}
	catch( const Ice::Exception&)
	{
		return false;
	}

	return true;
}

void NgodSessionI::storeCurrentSsInfo( const StreamSessionInfo& info )
{
	WLock sync(*this);
	ZQTianShan::Util::updatePropertyData(mProps, "streaminfo_cache_playposition", info.npt);
	ZQTianShan::Util::updatePropertyData(mProps, "streaminfo_cache_totalplaytime", info.playTime);
	ZQTianShan::Util::updatePropertyData(mProps, "streaminfo_cache_scale", info.scale);
	ZQTianShan::Util::updatePropertyData(mProps, "streaminfo_cache_itemoffset", info.assetNpt);
	ZQTianShan::Util::updatePropertyData(mProps, "streaminfo_cache_index",info.assetIndex);
	ZQTianShan::Util::updatePropertyData(mProps, "streaminfo_cache_timestamp", ZQ::common::now());
}

bool NgodSessionI::getCachedSsInfo( StreamSessionInfo& info, int64& stampAsOf)
{
	RLock sync(*this);
	ZQTianShan::Util::getPropertyDataWithDefault(mProps, "streaminfo_cache_timestamp", -1, stampAsOf);
	if( stampAsOf < 0 )
		return false;

	int64 timeDelta = ZQ::common::now() - stampAsOf;
	ZQTianShan::Util::getPropertyDataWithDefault(mProps, "streaminfo_cache_playposition",	-1, info.npt);
	ZQTianShan::Util::getPropertyDataWithDefault(mProps, "streaminfo_cache_totalplaytime",	-1, info.playTime);
	ZQTianShan::Util::getPropertyDataWithDefault(mProps, "streaminfo_cache_itemoffset",		-1, info.assetNpt);
	ZQTianShan::Util::getPropertyDataWithDefault(mProps, "streaminfo_cache_index",			-1, info.assetIndex);
	ZQTianShan::Util::getPropertyDataWithDefault(mProps, "streaminfo_cache_scale",			"", info.scale);
	if( !info.scale.empty())
	{
		double scale = atof(info.scale.c_str());
		if( ( scale - 0.001f) > 0.001f )
		{
			info.npt += (Ice::Int)(timeDelta * scale);
			info.assetNpt += (Ice::Int)(timeDelta * scale);
			if( info.npt >= info.playTime)
				info.npt = info.playTime;
		}
	}

	return true;
}

int NgodSessionI::getStreamSessionInfo( const NgodClientRequestIPtr& request, StreamSessionInfo& info )
{
	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "getStreamSessionInfo() entering "));
	int32 errCode = errorcodeOK;
	std::string errMsg;
	try
	{
		TianShanIce::ValueMap valMap;
		mStream->getInfo( ::TianShanIce::Streamer::infoSTREAMNPTPOS, valMap);	
		
		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "playposition", 0, info.npt );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "totalplaytime", 0, info.playTime );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "scale", "", info.scale );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "itemOffset", -1, info.assetNpt );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "index", 0, info.assetIndex  );
		storeCurrentSsInfo(info);
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "getStreamSessionInfo() information got: npt[%d] playtime[%d] scale[%s] assetNpt[%d] assetIndex[%d] "),
			info.npt, info.playTime, info.scale.c_str(), info.assetNpt, info.assetIndex );

		return errCode;
	}
	catch( const Ice::ObjectNotExistException& ex)
	{
		errCode	= errorcodeObjNotFound;
		errMsg	= ex.ice_name();
		MLOG(ZQ::common::Log::L_WARNING,SESSFMT(NgodSessionI, "getStreamSessionInfo() session[%s] failed to get stream instance, destroy current session"), mIdent.name.c_str() );
		destroy(NULL);
	}
	catch( const Ice::TimeoutException & ex)
	{
		errCode	= errorcodeServiceUnavail;
		errMsg	= ex.ice_name();
		if(sopConfig.sopRestrict.penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR && !mSopName.empty() && !mStreamerNetId.empty())
			mEnv.mSelManager.applyIncreasablePenalty( mSessId, mStreamerNetId, sopConfig.sopRestrict.timeoutPenalty,sopConfig.sopRestrict.maxPenaltyValue);
	}
	catch( const Ice::SocketException& ex)
	{
		errCode	= errorcodeServiceUnavail;
		errMsg	= ex.ice_name();
		if(sopConfig.sopRestrict.penaltyEnableMask & PENALTY_ENABLE_MASK_GETPAR && !mSopName.empty() && !mStreamerNetId.empty())
			mEnv.mSelManager.applyPenalty( mSessId, mStreamerNetId, sopConfig.sopRestrict.maxPenaltyValue);
	}
	catch( TianShanIce::BaseException& ex)
	{
		errCode = errorcodeInternalError;
		errMsg	= ex.message;
	}
	catch( const Ice::Exception& ex)
	{
		errCode = errorcodeInternalError;
		errMsg	= ex.ice_name();
	}

	if( request)
		errlog(request,errCode, "getStreamSessionInfo() failed to get stream session[%s]'s info due to [%s]", mSessId.c_str(), errMsg.c_str() );
	
	return errCode;
}

bool NgodSessionI::getSetupStartPoint( Ice::Int& index, Ice::Long& offset )
{
	RLock sync(*this);
	ZQTianShan::Util::getPropertyDataWithDefault( mProps, SYSKEY(SetupStartPointIndex), -1,index);
	ZQTianShan::Util::getPropertyDataWithDefault( mProps, SYSKEY(SetupStartPointOffset), 0,offset);
	return index > 0 ;
}

void NgodSessionI::updateSetupStartPoint( Ice::Int index, Ice::Long offset )
{
	WLock sync(*this);	
	ZQTianShan::Util::updatePropertyData( mProps, SYSKEY(SetupStartPointIndex),index);
	ZQTianShan::Util::updatePropertyData( mProps, SYSKEY(SetupStartPointOffset),offset);
}

void NgodSessionI::recordEvent_StreamEnd( const std::string& reason, const std::string& strNpt, const std::string& stopIdx )
{
	if ( ngodConfig.sessionHistory.enableHistory <= 0 )
		return;

	ZQTianShan::Util::TimeSpan span;
	span.start();

	{
		WLock sync(*this);
		NGOD::SessionEventRecord sessionEvent;
		sessionEvent.eventType		  = NGOD::EndEvent;
		sessionEvent.eventTime		  = getISOTimeString();
		sessionEvent.prop["reason"]   = reason;
		sessionEvent.NPT			  = strNpt;
		sessionEvent.streamResourceID = stopIdx;

		recordEvent( sessionEvent );
	}

	if( span.stop() > 100 )
	{
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "recordEvent_StreamEnd() use [%lld] to add EndEvent record"),
			span.span() );
	}
}

void NgodSessionI::recordEvent_StreamCtrl( const std::string& targetState, const std::string& newState,
									const std::string& strNptStart, const std::string& strScale,
									const std::string& streamResourceId, const std::string& userTime,
									int status, const std::string& reqRange, const std::string& reqScale )
{
	if ( ngodConfig.sessionHistory.enableHistory <= 0 )
		return;

	Ice::Int firstPlay = 1;
	
	ZQTianShan::Util::TimeSpan span;
	span.start();
	{
		WLock sync(*this);	

		ZQTianShan::Util::getPropertyDataWithDefault( mProps, SYSKEY(SessionFirstPlay), 1, firstPlay );

		NGOD::SessionEventRecord sessionEvent;
		sessionEvent.eventTime			= userTime;
		sessionEvent.streamResourceID	= streamResourceId;
		sessionEvent.NPT				= strNptStart;
		sessionEvent.prop["scale"]		= strScale;
		sessionEvent.prop["newState"]	= newState;
		ZQTianShan::Util::updatePropertyData( sessionEvent.prop, "status", status );
		sessionEvent.prop["reqState"]	= targetState;
		sessionEvent.prop["reqRange"]	= reqRange;
		sessionEvent.prop["reqScale"]	= reqScale;

		if ( firstPlay )
		{
			//firstPlay = 0;
			//ZQTianShan::Util::updatePropertyData( mProps, SYSKEY(SessionFirstPlay), firstPlay );
			ZQTianShan::Util::updatePropertyData( mProps, "lastScale", strScale );
			sessionEvent.eventType = NGOD::StartStreamEvent;		
		}
		else
		{
			sessionEvent.eventType = NGOD::UserEvent;		
		}

		recordEvent( sessionEvent );
	}

	if( span.stop() > 100 )
	{
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "recordEvent_StreamCtrl() use [%lld] to add stream/user event record"),
			span.span() );
	}
}

void NgodSessionI::recordEvent( const SessionEventRecord& record )
{
	WLock sync(*this);
	mSessionEvents.push_back(record);
}

int64 NgodSessionI::adjustNptToAssetBased( int index, int64 npt )
{
	RLock sync(*this);
	if( index < 0 || index >= (int)mItemInfos.size() )
		return npt;

	return npt + mItemInfos[index].inTimeOffset;
}

bool NgodSessionI::getPlayListInfo(std::string& strSpeed, std::string& strNPT, std::string& strAssetCtrlNum, std::string& strNewState)
{
	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "getPlayListInfo() entering"));
	strSpeed = "0.000";
	strNPT = "";
	strAssetCtrlNum = "";
	strNewState = "";

	TianShanIce::ValueMap valMap;
	TianShanIce::Streamer::StreamState strmState;
	try
	{
		// get scale npt ctrlNumber
		mStream->getInfo(::TianShanIce::Streamer::infoSTREAMNPTPOS, valMap);
		
		Ice::Int			assetNpt = 0;
		Ice::Int			assetCtrlNum = 0;

		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "scale", "0.000", strSpeed );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "itemOffset", 0, assetNpt );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap, "index", 0, assetCtrlNum );

		char buffer[256] = { 0 };
		sprintf(buffer, "%d.%03d", assetNpt/1000, assetNpt%1000);
		strNPT = buffer;

		std::ostringstream oss;
		oss.str("");oss<<assetCtrlNum; strAssetCtrlNum = oss.str();

		// get new State
		strmState = mStream->getCurrentState();
		switch(strmState)
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

		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(NgodSessionI, "getPlayListInfo() information got: speed[%s], npt[%s] assetCtrlNum[%s] state[%s] "),
			strSpeed.c_str(), strNPT.c_str(), strAssetCtrlNum.c_str(), strNewState.c_str() );
	}
	catch( const Ice::SocketException& ex )
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(NgodSessionI, "caught [%s] when perform getinfo for session[%s]"),
			ex.ice_name().c_str(), mSessId.c_str() );
		return false;
	}
	catch( const Ice::TimeoutException& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(NgodSessionI, "caught [%s] when perform getinfo"),ex.ice_name().c_str() );
		return false;
	}	
	catch(const ::TianShanIce::BaseException& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, SESSFMT(NgodSessionI, "perform getInfo() on session: [%s] caught [%s]:[%s]"), mSessId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}	
	catch(const ::Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, SESSFMT(NgodSessionI, "perform getInfo() on stream: [%s] caught [%s]"), mSessId.c_str(), ex.ice_name().c_str());
		return false;
	}

	return true;
}

void NgodSessionI::rebindStreamSession( const TianShanIce::Streamer::PlaylistPrx& stream, const std::string& streamSessId, const std::string& streamerNetId, const ::Ice::Current& )
{
	WLock sync(*this);
	mStream = stream;
	mStreamSessId = streamSessId;
	mStreamerNetId = streamerNetId;
}

//////////////////////////////////////////////////////////////////////////
///processAnnounceResp( const NgodRequestAnnounceResponsePtr& request );
int NgodSessionI::processAnnounceResp( const NgodRequestAnnounceResponsePtr& request )
{
	switch( request->getRetCode() )
	{
	case 404:
	case 454:
		{
			MLOG(ZQ::common::Log::L_INFO,SESSREQFMT(NgodSessionI, "client response announce with code[%d], destroy the session"), request->getRetCode() );
			destroy( NULL);
		}
		break;

	default:
		{
			{
				WLock sync(*this);
				expiredCount	= 0;//reset expired count
				MLOG(ZQ::common::Log::L_INFO,SESSREQFMT(NgodSessionI, "reset expiredcount to 0 due to the response of session-in-progress announce"));
				//expireTime		= ZQTianShan::now() + timeout;				
			}
			updateC1ConnectionId( request->connectionId )	;
			updateTimer();
		}
		break;
	}

	return errorcodeOK;
}

}//namespace NGOD

