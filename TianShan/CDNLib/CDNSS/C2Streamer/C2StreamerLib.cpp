
#include <ZQ_common_conf.h>
#include <sstream>
#include <assert.h>
#include <Log.h>
#include "C2StreamerLib.h"
#include <strHelper.h>
#ifndef ZQ_CDN_UMG
#include "C2StreamerEnv.h"
#include "C2StreamerService.h"
#include "C2Session.h"
#include "C2SessionHelper.h"
#include "C2HttpHandler.h"
#include "C2TunnelBwmon.h"
#else
#include "HttpC2Streamer.h"
#endif

#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"##y, request->requestHint.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"y, request->requestHint.c_str() ,  (unsigned int)gettid(),#x	
#endif	

namespace C2Streamer
{

const char* convertSessionStateToString( const SessionState& state )
{
	switch( state )
	{
	case SESSION_STATE_NULL:
		return "NULL";
	case SESSION_STATE_IDLE:
		return "IDLE";
	case SESSION_STATE_ACTIVE:
		return "ACTIVE";
	case SESSION_STATE_DELETED:
		return "DELETED";
	default:
		return "UNKOWN";
	}	
}
const char* convertC2MethodToString( const C2Method& method )
{
	switch( method )
	{
	case METHOD_NULL:
		return "NULL";
	case METHOD_TRANSFER_INIT:
		return "TRANSFER_INIT";
	case METHOD_TRANSFER_TERM:
		return "TRANSFER_TERM";
	case METHOD_TRANSFER_RUN:
		return "TRANSFER_RUN";
	case METHOD_SESSION_STATUS:
		return "SESSION_STATUS";
	case METHOD_RESOURCE_STATUS:
		return "RESOURCE_STATUS";
	case METHOD_IC_UPDATE:
		return "IC_UPDATE";
	case METHOD_SESS_UPDATE:
		return "SESS_UPDATE";
	case METHOD_RES_UPDATE:
		return "RES_UPDATE";
	default:
		return "UNKOWN";
	}
}
bool		isSuccessCode( int32 errorCode )
{
	return (errorCode >= 200 && errorCode <=299);
}
void RequestResponseParam::setLastErr( int32 errCode , const char* fmt , ... )
{
	errorCode = errCode;
	char szLocalBuffer[1024];
	szLocalBuffer[ sizeof( szLocalBuffer ) - 1 ] = 0;

	va_list args;
	va_start(args, fmt);
	vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-2, fmt, args);
	va_end(args);
	errorText = szLocalBuffer;
}

void RequestResponseParam::setLastErr( const RequestParamPtr& request , int32 errCode , const char* fmt , ... )
{
	errorCode = errCode;
	char szLocalBuffer[1024];
	szLocalBuffer[ sizeof( szLocalBuffer ) - 1 ] = 0;

	va_list args;
	va_start(args, fmt);
	vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-2, fmt, args);
	va_end(args);
	errorText = szLocalBuffer;
	(*getEnvironment()->getLogger())( ZQ::common::Log::L_ERROR, SESSFMT(Response,"[%s] %s"), convertC2MethodToString(request->method) , szLocalBuffer);
}


const char* convertErrCodeToString( int32 errCode )
{
	switch ( errCode )
	{
	case errorCodeOK:					return "OK";
	case errorCodeCreated:				return "Created";
	case errorCodePartialContent:		return "Partial Content";
	case errorCodeSessionGone:			return "Gone";
	case errorCodeBadRequest:			return "Bad Request";
	case errorCodeContentNotFound:		return "Not Found";
	case errorCodeNotAllowed:			return "Method Not Allowed";
	case errorCodeNotAccpetable:		return "Not Acceptable";
	case errorCodeRequestTimeout:		return "Request Timeout";
	case errorCodeBadRange:				return "Requested Range Not Satisfiable";
	case errorCodeNotImplemented:		return "Not Implemented";
	case errorCodeServerUnavail:		return "Service Unavailable";
	case errorCodeGatewayTimeout:		return "Gateway Timeout";
	case errorCodeVersionNotSupported:	return "HTTP Version Not Supported";
	default:							return "Internal Server Error";
	}
	return "";
}

std::string TransferRange::toString() const
{
	std::ostringstream oss;
	if( bStartValid )
	{
		oss << startPos ;
	}
	
	if( bStartValid || bEndValid )
		oss << "-";
	
	if( bEndValid )
	{
		oss << endPos;
	}
	return oss.str();
}
bool TransferRange::parse(const std::string& r)
{
    std::string::size_type pos = r.find('-');
    if( pos != std::string::npos )
    {
		std::string::size_type pos2 = r.find("-",pos+1);
		if( pos2 != std::string::npos)
			pos = pos2;
        std::string start = r.substr( 0 , pos );
        std::string stop = r.substr( pos + 1 );
        ZQ::common::stringHelper::TrimExtra( start, " \t\v\r\n");
        ZQ::common::stringHelper::TrimExtra( stop , " \t\v\r\n-");
        int64 t = 0;
        if( !start.empty() )
        {
            sscanf(start.c_str(),FMT64,&t);
            bStartValid = true;
            startPos = t;
        }
        if( !stop.empty() )
        {
            sscanf(stop.c_str() , FMT64 , &t );
            bEndValid = true;
            endPos = t;
        }
    }
    return (bStartValid || bEndValid);
}

#ifndef ZQ_CDN_UMG
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///C2HttpResponseHanlder
C2HttpResponseHanlder::C2HttpResponseHanlder( ZQHttp::IResponse* handler , ZQHttp::IConnection* conn)
:mResponseConnection(conn),
mResponseHandler(handler),
mbComplete(false),
mbFlushed(false),
mbConnBroken(false),
mConnId(-1)
{
	assert( mResponseHandler != NULL );
}

C2HttpResponseHanlder::~C2HttpResponseHanlder()
{
	if( !mbComplete)
		mResponseHandler->complete();	
}

void C2HttpResponseHanlder::updateStartline(  int statusCode , const std::string& desc )
{	
	if( !mbComplete)
		mResponseHandler->setStatus( statusCode , desc.c_str() );
}

void C2HttpResponseHanlder::updateHeader( const std::string& key , const std::string& value )
{	
	if( !mbComplete)
		mResponseHandler->setHeader( key.c_str() , value.c_str() );	
}

bool C2HttpResponseHanlder::flushHeader( ssize_t bodySize )
{
	ZQ::common::MutexGuard gd(mLocker);

	if(mbFlushed)
		return false;

	mbFlushed = true;	
	{
		if(bodySize >= 0 )
			updateContentLength( (size_t)bodySize );
		else
			mResponseHandler->setHeader("Transfer-Encoding","chunked");

		bool bRet = mResponseHandler->headerPrepared( );

		if( mbComplete )
		{
			return mResponseHandler->complete();
		}
		else
		{
			return bRet;
		}
	}	
}

bool C2HttpResponseHanlder::complete()
{	
	ZQ::common::MutexGuard gd(mLocker);
	if( !mbComplete && mResponseHandler != NULL )
	{
		mbComplete = true;
		if(!mbFlushed)
		{
			return true;//header had not been flushed yet
		}

		bool bOK = mResponseHandler->complete();
		if( !bOK)
		{
			//should log some thing here
		}
		return bOK;
	}
	return false;
}
bool C2HttpResponseHanlder::updateContentLength( size_t length)
{	
	if( !mbComplete )
	{
		char szLocalBuf[128];
		mResponseHandler->setHeader( "Content-Length" , itoa(length,szLocalBuf,10) );		
	}
	return true;
}

bool C2HttpResponseHanlder::addBodyContent( const char* data , size_t size )
{	
	if( !mbComplete )
	{	
		return mResponseHandler->addContent( data, size );
	}
	return false;
}
std::string C2HttpResponseHanlder::lastError() const
{
	if( !mbComplete )
	{
		const char* p = mResponseHandler->getLastError();
		if(p && p[0])
			return std::string(p);		
	}
	return std::string("");
}

bool C2HttpResponseHanlder::isConnectionBroken() const
{
	return mbConnBroken;
}
	
void C2HttpResponseHanlder::setConnectionBroken( bool bBroken)
{
	mbConnBroken = bBroken;
}

int64 C2HttpResponseHanlder::getConnectionId() const
{
	return mConnId;
}
void C2HttpResponseHanlder::setConnectionId( int64 id )
{
	mConnId = id;
}
void C2HttpResponseHanlder::setCommOption(int opt , int value )
{
	if(!mResponseConnection)
		return;
	mResponseConnection->setCommOption(opt,value);
}

////////////////////////////////////////////////////////////////////////////
C2EventSinkerPtr updateEventReceiver( C2EventSinkerPtr& newEventReceiver , C2Method mask )
{
	getC2StreamerService()->getEventPublisher().setSinker( newEventReceiver , mask );
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
int32		cTransferInit( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response )
{
	C2SessionPtr sess =  getC2StreamerService()->getSessManager().createSession(request->sessionId);
	if( !sess )
	{
		response->setLastErr( request, errorCodeInternalError , "failed to create session");
		return errorCodeInternalError;
	}
	request->sessionId = sess->getSessId();
	{
		std::ostringstream oss;
		oss<<"Sess["<<request->sessionId<<"] ";
		request->requestHint = oss.str();
	}
	return sess->processRequest( request , response );
}

int32		cTransferRun( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response )
{
	std::string sessId = getSessionIdFromCompoundString(request->sessionId);
	C2SessionPtr sess = getC2StreamerService()->getSessManager().findSession( sessId );
	if(!sess)
	{
		response->setLastErr( request , errorCodeContentNotFound , "can't find session [%s] [%s]", request->sessionId.c_str() ,sessId.c_str() );
		return errorCodeContentNotFound;
	}
	int32 retCode = sess->processRequest( request , response );
	if( isSuccessCode( retCode ) )
	{
		if( sess->startTransfer() )
		{
			return retCode;
		}
		else
		{
			return errorCodeInternalError;
		}
	}
	else
	{
		return retCode;
	}
}
int32		cTransferTerm( const TransferTermRequestParamPtr request , TransferTermResponseParamPtr response )
{
	std::string sessId = getSessionIdFromCompoundString(request->sessionId);
	C2SessionPtr sess = getC2StreamerService()->getSessManager().findSession( sessId );
	if(!sess)
	{
		//response->setLastErr( request , errorCodeContentNotFound , "can't find session [%s] [%s]", request->sessionId.c_str() , sessId.c_str() );
		return errorCodeContentNotFound;
	}
	return sess->processRequest( request , response );
}

int32		cSessionStatus( const SessionStatusRequestParamPtr request , SessionStatusResponseParamPtr response )
{
	return getC2StreamerService()->getSessionStatus(request, response) ? errorCodeOK : errorCodeInternalError;
}

int32		cResourceStatus( const ResourceStatusRequestParamPtr request , ResourceStatusResponseParamPtr response )
{
	return getC2StreamerService()->getResourceStatus( request , response ) ? errorCodeOK : errorCodeInternalError;
}
#endif

void updateSessionTimer( const std::string& sessId , int64 interval )
{
	C2SessionPtr sess = getC2StreamerService()->getSessManager().findSession( sessId );
	if(!sess)
	{
		return ;
	}
	sess->updateTimer(interval);
}

bool getC2Load( const std::string& upStreamIp, C2ServiceLoad& load )
{
	load.localStreamBWMax = 0;
	load.localStreamBW = 0;
	load.natStreamBWMax = 0;
	load.natStreamBW = 0;

	C2Service* svc = getC2StreamerService();
	PortManager& pm = svc->getPortManager();
	C2IpConnTackMon& tb = svc->getC2TunnelMon();

	load.natStreamBW = tb.queryBW("");
	load.localStreamBW = pm.queryBW("", &load.localStreamBWMax);
	PortManager::PortAttr attr;
	AddrInfoHelper aih;
	if( !upStreamIp.empty() && aih.getinfo( upStreamIp,"1") )
	{
		if( pm.getPortAttr( aih.ip(), attr ) )
		{
			load.natStreamBWMax = attr.physicalCapability;
		}
	}
	else
	{
		return upStreamIp.empty() ? true:false;
	}


	return true;
}

}//namespace C2Streamer

