
#include <ZQ_common_conf.h>
#include <stdarg.h>
#include <FileSystemOp.h>
#include <TimeUtil.h>
#include "C2StreamerEnv.h"
#include "C2SessionManager.h"
#include "C2StreamerService.h"
#include "C2EventUpdater.h"
#include "C2Session.h"
#include "C2SessionHelper.h"



#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"##y, request->requestHint.c_str(), GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"y, request->requestHint.c_str(),  (unsigned int)gettid(),#x	
#endif	

namespace C2Streamer
{


void C2Session::updateLastError( C2Streamer::RequestParamPtr request, RequestResponseParamPtr response, int errorCode, const char* fmt, ... )
{
	char szLocalBuffer[1024];
	szLocalBuffer[sizeof(szLocalBuffer)-1] = 0;
	va_list args;
	va_start( args, fmt );
	int nCount = vsnprintf( szLocalBuffer, sizeof(szLocalBuffer) - 1,fmt, args );
	if( nCount < 0 )
		szLocalBuffer[sizeof(szLocalBuffer)-1] = 0;
	else
		szLocalBuffer[nCount] = 0;
	va_end(args);
	response->setLastErr(errorCode,szLocalBuffer);
	MLOG(ZQ::common::Log::L_ERROR,SESSFMT(C2Session,"%s"),szLocalBuffer);
}


#undef SESSFMT
#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"%s/%08X/[%s]\t"##y, mSessionId.c_str(), GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"%s/%08X/[%s]\t"y, mSessionId.c_str(),  (unsigned int)gettid(),#x	
#endif	

C2Session::C2Session( C2StreamerEnv& env, C2Service& svc,const std::string& sessId)
:mEnv(env),mSvc(svc),
mSessionId(sessId),
mTimeoutInterval(500),
mStartTimeInState(0),
mSessionState(SESSION_STATE_NULL)
,mDataRunner(0),
mNextWakeUpTime(0),
mSessionType(SESSION_TYPE_NORMAL)
{
	mDataRunner = new SessionDataRunner(env,svc,this,mSvc.getCacheCenter(),*this);
	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(C2Session,"session is created"));
}

C2Session::~C2Session(void)
{
	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(C2Session,"session is deleted"));
	delete mDataRunner;
}

void C2Session::markAsConnBroken() {
	mDataRunner->markAsConnBroken();
}

void C2Session::destroy()
{
	ZQ::common::MutexGuard gd(*this);
	if( mSessionState == SESSION_STATE_DELETED )
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(C2Session,"session was already destroyed, free it"));
		return;
	}
	unregisterSession();
	mDataRunner->stopRunner();
	changeState( SESSION_STATE_DELETED, false );
	mSvc.unwatchSession(this);
	MLOG(ZQ::common::Log::L_INFO, SESSFMT(C2Session, "session destroyed, file[%s], STAT: %s"), mRequestFileName.c_str(), mDataRunner->mStatistics.toString().c_str());
}

void C2Session::changeState(const SessionState& targetState, bool bUpdateTimer )
{
	ZQ::common::MutexGuard gd(*this);
	if( mSessionState == targetState )
		return;
	MLOG(ZQ::common::Log::L_INFO, SESSFMT(C2Session, "changeState from [%s] to [%s], timeout interval[%d]"),
		 convertSessionStateToString(mSessionState),
		 convertSessionStateToString(targetState),
		 mTimeoutInterval );
		 
	mSessionState = targetState;
	mStartTimeInState = ZQ::common::now();
	
	if( bUpdateTimer )
		updateTimer( mTimeoutInterval );
	
	
	TransferStateUpdateEvent* e = new TransferStateUpdateEvent();
	e->clientTransfer 	= mClientTransferAddress;
	e->transferId		= constructResponseSessId( mSessionId );
	e->sessionState		= mSessionState;
	e->eventmethod			= METHOD_SESS_UPDATE;
	mSvc.getEventPublisher().post(e);
}

void C2Session::setRequestType( int32 type )
{
	mSessionType = type;
}

void C2Session::cancelTimer( ) 
{
	mSvc.unwatchSession(this);
}

void C2Session::updateTimer( uint64 timeInterval )
{
	mNextWakeUpTime = ZQ::common::now() + timeInterval;
	mSvc.watchSession( this, timeInterval );
}

void C2Session::unregisterSession()
{
	if( !mClientTransferAddress.empty() )
	{
		mSvc.getClientManager().unregisterSession( mClientTransferAddress, mSessionId );
		mClientTransferAddress = "";
	}
	
	if( !mServerTransferAddress.empty() )
	{
		mSvc.getPortManager().unregisterSession( mServerTransferAddress, mSessionId );
	}
	mSvc.getSessManager().removeSession( mSessionId );
}

#ifdef ABS
#undef ABS
#endif
#define ABS(x) (x>0?x:-x)

void C2Session::onTimer()
{

	switch( mSessionState )
	{
	case SESSION_STATE_NULL:		
	case SESSION_STATE_DELETED:		
	case SESSION_STATE_IDLE:
		{
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(C2Session,"state[%s] onTimer()"),
				convertSessionStateToString(mSessionState));
			destroy();
		}
		break;

	case SESSION_STATE_ACTIVE:
		{
			int64 cur = ZQ::common::now();
			int64 delta = cur - (int64)mNextWakeUpTime;			
			if( delta > mEnv.getConfig().mTimerThreadHold )
			{
				MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(C2Session,"onTimer() target wakeup time[%lu] now[%ld] delta[%ld]"),
				 mNextWakeUpTime, cur, delta );
			}
			runSession( delta );
		}
		break;
	
	default:
		{
			assert(false);
		}
		break;
	}
}

const std::string& C2Session::getSessId( ) const
{
	return mSessionId;
}
	
int64 C2Session::getReservedBandwidth( ) const
{
	return mDataRunner->mSessTransferRate;
}

SessionState C2Session::getState( ) const
{
	return mSessionState;
}

std::string C2Session::getFileName( ) const
{
	return mRequestFileName;
}

int64 C2Session::getBytesTransfered( ) const
{
	return mDataRunner->mBytesTransfered;
}

std::string C2Session::getTransferServerAddress( ) const
{
	return mServerTransferAddress;
}

std::string C2Session::getTransferClientAddress( ) const
{
	return mClientTransferAddress;
}

std::string C2Session::getTransferPortName( ) const
{
	return mServerTransferPortName;
}

int64 C2Session::getTimeInState( ) const
{
	int64 cur = ZQ::common::now();
	return (cur - mStartTimeInState);
}

int64 C2Session::getTransferRate( ) const
{
	return mDataRunner->mSessTransferRate;
}

int32 C2Session::processRequest( const RequestParamPtr request, RequestResponseParamPtr response )
{
	if( mSessionState == SESSION_STATE_DELETED )
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(C2Session,"processRequest() session is destroyed"));
		return errorCodeSessionGone;
	}
	int32 retCode = errorCodeNotImplemented;
	switch( request->method )
	{
	case METHOD_TRANSFER_INIT:
	{
		TransferInitRequestParamPtr t 	= TransferInitRequestParamPtr::dynamicCast(request);
		TransferInitResponseParamPtr s	= TransferInitResponseParamPtr::dynamicCast(response);
		assert(t);assert(s);
		retCode = processTransferInit(t,s);
		changeState( SESSION_STATE_IDLE );
	}
	break;
	case METHOD_TRANSFER_TERM:
	{
		TransferTermRequestParamPtr t = TransferTermRequestParamPtr::dynamicCast( request );
		TransferTermResponseParamPtr s = TransferTermResponseParamPtr::dynamicCast( response );
		assert(t);assert(s);
		retCode = processTransferTerm( t, s );
	}
	break;
	case METHOD_TRANSFER_RUN:
	{
		cancelTimer();
		SessionTransferParamPtr t 	= SessionTransferParamPtr::dynamicCast(request);
		SessionTransferResponseParamPtr s	= SessionTransferResponseParamPtr::dynamicCast(response);
		assert(t);assert(s);
		retCode = processTransferSession(t,s);
		if( isSuccessCode(retCode) ) {
			cancelTimer();
		} else {
			updateTimer(mTimeoutInterval);
		}

	}
	break;
	case METHOD_SESSION_STATUS:
	{
	}
	break;	
	case METHOD_RESOURCE_STATUS:
	{
	}
	break;	
	default:	
		return errorCodeBadRequest;
	
	}
	return retCode;
}

}//namespace C2Streamer
