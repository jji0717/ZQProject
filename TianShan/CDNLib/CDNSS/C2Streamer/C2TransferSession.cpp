
#include <ZQ_common_conf.h>
#include <sys/types.h>
#include <FileSystemOp.h>
#include <TimeUtil.h>

#include "C2StreamerEnv.h"
#include "C2StreamerService.h"

#include "C2SessionHelper.h"
#include "C2Session.h"


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"##y, request->requestHint.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"y, request->requestHint.c_str() ,  (unsigned int)gettid(),#x	
#endif	


namespace C2Streamer
{

int32 C2Session::processTransferSession( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response )
{
	StopWatch sw; sw.start();

	if( ( mEnv.getConfig().clientType != 2 ) && ( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_SHADOW ) )
	{
		updateLastError( request , response , errorCodeNotAllowed , "processTransferSession() shadow session is not allowed to GET, reject request",
				convertSessionStateToString(mSessionState) );		
		return errorCodeNotAllowed;
	}
	else if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mDataRunner->stopRunner();// stop runner before acquire session mutex to avoid dead lock
	}
	ZQ::common::MutexGuard gd(*this); //FIXME: can I remove this locker ?

	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mDataRunner->reset();
	}
	else
	{
		if( mSessionState == SESSION_STATE_ACTIVE )
		{
			updateLastError( request , response , errorCodeNotAllowed , "processTransferSession() session is in [%s] state, reject transfer request",
				convertSessionStateToString(mSessionState) );		
			return errorCodeNotAllowed;
		}
	}
	MLOG(ZQ::common::Log::L_INFO,SESSFMT(C2Session, "processTransferSession() param: client[%s:%d] range[%s] TransferDelay[%lld] ext[%s] viaProxy[%s]"),
		 request->clientIp.c_str() , request->clientPort , request->range.toString().c_str(),
		 request->bHasTransferDelay ? request->transferDelay : mDataRunner->mSessTransferDelay ,
		 request->requestFileExt.c_str(),
		 request->viaProxy?"true":"false") ;

	if( request->bHasTransferDelay )
	{		
		mDataRunner->mSessTransferDelay = request->transferDelay;
	}
	
	if( mEnv.getConfig().mCheckClientWhileTransfer && request->clientIp != mClientTransferAddress)
	{
		updateLastError( request , response , errorCodeBadRequest , "processSessionTransfer() session reserved for client[%s], not[%s]",
						 mClientTransferAddress.c_str() , request->clientIp.c_str() );
		return errorCodeBadRequest;
	}
	
	const TransferRange& range = request->range;
	if( range.bStartValid || range.bEndValid )
		mDataRunner->mRequestRange = range;

	
	C2SessFile sf(mEnv);
	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mRequestFileName = mContentName + request->requestFileExt;
	}

	sf.process( mRequestFileName , mSessionId);
	if( !sf.isValid() )
	{
		updateLastError( request , response, errorCodeContentNotFound,"processTransferSession() failed to find file[%s]",
				  mFileName.c_str());
		return errorCodeContentNotFound;
	}

	if( !sf.checkRequestRange( mDataRunner->mRequestRange) )
	{
		updateLastError( request, response , errorCodeBadRange,"processSessionTransfer() bad request range[%s]: file data range[%lld-%lld]",
				  mDataRunner->mRequestRange.toString().c_str() , sf.dataStartPos() ,sf.dataEndPos() );
		return errorCodeBadRange;
	}
	if( !mDataRunner->mRequestRange.bStartValid  )
	{
		mDataRunner->mRequestRange.bStartValid = true;
		mDataRunner->mRequestRange.startPos = sf.dataStartPos();
	}

	if( request->seekIFrame && mbMainFile) {
		C2IndexRecordCenter& ircenter = mSvc.getIndexRecordCenter();
		C2IndexRecordCenter::IndexRecordWrapperPtr record = ircenter.get( mIndexFilePathname );
		if(record) {
			uint64 tempStart = mDataRunner->mRequestRange.startPos;
			mDataRunner->mRequestRange.startPos = record->findNextIFrame( tempStart );
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(processSessionTransfer,"adjust request start position from [%lu] to [%lu]"),
					tempStart, mDataRunner->mRequestRange.startPos );
		}
	}

	
	response->filename 	= mRequestFileName; // use request file
	response->fileSize	= sf.fileDataSize();
	response->range		= composeAvailRange( mDataRunner->mRequestRange , sf );
	response->sessionType = mSessionType;
	
	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{//reset full path name of request file
		mFileName			= sf.getFileFullPath();
	}

	cancelTimer();

	if( !mDataRunner->startRunner( response->responseHandler , mFileName , this ) )
	{
		updateLastError( request , response , errorCodeInternalError , "processSessionTransfer() failed to prepare data" );
		return errorCodeInternalError;
	}

	bool cacheAble = false;

	if( mEnv.getConfig().clientType != 0 ) {
		// check if the file is availble ?
		// That is we should wait for the first chunk of file data to be available

		// Because request from proxy is a chunk data request
		// so I have to check the first data of the 
		int lastError = checkFileData( request, cacheAble );
		if( lastError != 0 ) {
			// turn error code into http status code
			int category = lastError >> 28;
			lastError = lastError & 0x0FFFFFFF;
			switch( category ) {
				case Buffer::ECATE_FILEIO:
					lastError = errorCodeInternalError;
					break;
				case Buffer::ECATE_HTTP:
					break;
				case Buffer::ECATE_TIMEOUT:
					lastError = errorCodeGatewayTimeout;
					break;
				default:
					lastError = errorCodeInternalError;
					break;
			}
			updateLastError( request, response, lastError,"processSessionTransfer() failed to get file data");
			return lastError;
		}
	}

	if( !request->viaProxy) {
		response->errorCode = mDataRunner->mRequestRange.bStartValid ? 
			(mDataRunner->mRequestRange.startPos != 0 ? errorCodePartialContent : errorCodeOK ) 
			: errorCodeOK;
	} else {
		response->errorCode = errorCodeOK;
		response->cacheable = cacheAble;
	}

	changeState( SESSION_STATE_ACTIVE, false );

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processSessionTransfer,"took [%lu] ms"), sw.stop()/1000);

	return response->errorCode;
}

int C2Session::checkFileData( const SessionTransferParamPtr request,bool& cacheAble ) {
    AioFile* f = mSvc.getCacheCenter().open( mRequestFileName );
    uint64 offset = 0;
    if( mDataRunner->mRequestRange.bStartValid) {
        offset = mDataRunner->mRequestRange.startPos;
    }
    AssetAttribute::Ptr attr = mSvc.getSessManager().getAssetAttribute( mRequestFileName );
	//test code
    BufferUser bu = f->read( offset, 64 *1024, true);
    //if( !bu.valid())
    //    return 404;//is this OK
	int lastError = 0;
    if( attr ) {
		int64 timeStart = ZQ::common::now();
        attr->wait();
        cacheAble = !attr->pwe();
		if(!cacheAble && (lastError >> 28) == Buffer::ECATE_TIMEOUT ) {
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(processSessionTransfer,"timed out while get data for[%s], but it's in pwe state, treat it as OK"),
				mRequestFileName.c_str() );
			lastError = 0;
		}
		MLOG(ZQ::common::Log::L_INFO, SESSFMT(processSessionTransfer,"took [%ld]ms to get asset atribute for[%s]: pwe[%s] error[%d]"),
			ZQ::common::now()-timeStart, mRequestFileName.c_str(), attr->pwe()?"true":"false", attr->lastError() );
    }
    return lastError;
}

bool C2Session::startTransfer(  )
{
	int32 waitInterval = rand()%mEnv.getConfig().mFirstChunkWait + 5;
	updateTimer( waitInterval );
	return true;
}
void C2Session::runSession(uint64 timeshift)
{
	switch ( mSessionState )
	{
	case SESSION_STATE_ACTIVE:
		{
			if( !mDataRunner->run(timeshift) )
			{
				//DO NOT UPDATE TIMER
				return;
			}
			
		}
		break;;
	default:
		{
			//do nothing ?
		}
		break;
	}
}

}//namespace C2Streamer


