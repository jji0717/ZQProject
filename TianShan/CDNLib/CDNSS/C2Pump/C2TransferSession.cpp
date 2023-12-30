
#include <ZQ_common_conf.h>
#include <sys/types.h>
#include <FileSystemOp.h>
#include <TimeUtil.h>

#include "C2StreamerEnv.h"
#include "C2StreamerService.h"

#include "C2SessionHelper.h"
#include "C2Session.h"


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	CLOGFMT(x, " REQUEST[%s]\t"##y), request->requestHint.c_str() 
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	CLOGFMT(x, " REQUEST[%s]\t"y), request->requestHint.c_str()
#endif	


namespace C2Streamer
{

int32 C2Session::processTransferSession( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response )
{
	StopWatch sw; sw.start();

	int readerType = request->getConfUrlRule()->readerType;

	if( ( readerType == CLIENT_TYPE_DISKAIO ) && ( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_SHADOW ) )
	{
		updateLastError( request , response , errorCodeNotAllowed , "processTransferSession() shadow session is not allowed to GET, reject request",
				convertSessionStateToString(mSessionState) );		
		return errorCodeNotAllowed;
	}
	else if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mCurrentDataRunner->stopRunner();// stop runner before acquire session mutex to avoid dead lock
	}
	ZQ::common::MutexGuard gd(*this); //FIXME: can I remove this locker ?
	if( mSessionState == SESSION_STATE_DELETED ) {
		updateLastError( request, response, errorCodeContentNotFound, "processTransferSession() session is deleted");
		return errorCodeContentNotFound;
	}

	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mCurrentDataRunner->reset();
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
	MLOG(ZQ::common::Log::L_INFO,SESSFMT(C2Session, "processTransferSession() param: client[%s:%d] range[%s] TransferDelay[%ld] ext[%s] viaProxy[%s]"),
		 request->clientIp.c_str() , request->clientPort , request->range.toString().c_str(),
		 request->bHasTransferDelay ? request->transferDelay : mCurrentDataRunner->getTransferDelay() ,
		 request->requestFileExt.c_str(),
		 request->viaProxy?"true":"false") ;

	if( request->bHasTransferDelay )
	{		
		mCurrentDataRunner->setTransferDelay(request->transferDelay);
	}
	
	if( request->getConfUrlRule()->mCheckClientWhileTransfer && request->clientIp != mClientTransferAddress)
	{
		updateLastError( request , response , errorCodeBadRequest , "processSessionTransfer() session reserved for client[%s], not[%s]",
						 mClientTransferAddress.c_str() , request->clientIp.c_str() );
		return errorCodeBadRequest;
	}
	
	const TransferRange& range = request->range;
	if( range.bStartValid || range.bEndValid )
		mCurrentDataRunner->setTransferRange( range );

	
	C2SessFile sf(mEnv, readerType );
	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mRequestFileName = mContentName + request->requestFileExt;
		mCurrentDataRunner->setReqFilename(mRequestFileName);
	}

	sf.process( mRequestFileName , mSessionId);
	if( !sf.isValid() )
	{
		updateLastError( request , response, errorCodeContentNotFound,"processTransferSession() failed to find file[%s]",
				  mFileName.c_str());
		return errorCodeContentNotFound;
	}

	if( !sf.checkRequestRange( mCurrentDataRunner->getTransferRange()) )
	{
		updateLastError( request, response , errorCodeBadRange,"processSessionTransfer() bad request range[%s]: file data range[%lld-%lld]",
				  mCurrentDataRunner->getTransferRange().toString().c_str(), sf.dataStartPos(), sf.dataEndPos() );
		return errorCodeBadRange;
	}
	if( !mCurrentDataRunner->getTransferRange().bStartValid  )
	{
		TransferRange tmpRange = mCurrentDataRunner->getTransferRange();
		tmpRange.bStartValid = true;
		tmpRange.startPos = sf.dataStartPos();
		mCurrentDataRunner->setTransferRange(tmpRange);
	}

	if( request->seekIFrame && mbMainFile) {
		C2IndexRecordCenter& ircenter = mSvc.getIndexRecordCenter();
		C2IndexRecordCenter::IndexRecordWrapperPtr record = ircenter.get( mIndexFilePathname );
		if(record) {
			TransferRange tmpRange = mCurrentDataRunner->getTransferRange();
			uint64 tempStart = tmpRange.startPos;
			tmpRange.startPos = (int64)record->findNextIFrame( tempStart );
			mCurrentDataRunner->setTransferRange(tmpRange);
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(processSessionTransfer,"adjust request start position from [%lu] to [%lu]"),
					tempStart, tmpRange.startPos );
		}
	}

	
	response->filename 	= mRequestFileName; // use request file
	response->fileSize	= sf.fileDataSize();
	response->range		= composeAvailRange( mCurrentDataRunner->getTransferRange(), sf );
	response->sessionType = mSessionType;
	
	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{//reset full path name of request file
		mFileName			= sf.getFileFullPath();
	}

	cancelTimer();

	mConfWriter = request->getConfWriter();
	if(mReaderType < 0 ) {
		mReaderType = request->getConfUrlRule()->readerType;
	}
	mCurrentDataRunner->setConfWriter(mConfWriter);
	mCurrentDataRunner->setReaderType(mReaderType);


	LibAsync::EventLoop* evtLoop = request->evtLoop;

	assert( evtLoop != NULL);
	mReachFileEndCb = new DataReadCallback(this, evtLoop, true);
	mDataReadCb = new DataReadCallback(this,evtLoop,true);
	mWritableCb = new HttpWritableCallback( this );
	mAsyncTimer = new C2SessionAsyncTimer(this, evtLoop);
	mAsyncStopRunner = new SessionStopRunnerAsyncWork(this, evtLoop);

	mCurrentDataRunner->updateSessProperty(request->sessProp);

	if( !mCurrentDataRunner->startRunner( response->responseHandler , this ) )
	{
		mReachFileEndCb = NULL;
		mDataReadCb = NULL;
		mWritableCb = NULL;
		mAsyncTimer = NULL;
		mAsyncStopRunner = NULL;
		updateLastError( request , response , errorCodeInternalError , "processSessionTransfer() failed to prepare data" );
		return errorCodeInternalError;
	}

	bool cacheAble = false;

	if( readerType != CLIENT_TYPE_DISKAIO && request->prop().queryIndex ) {
		// check if the file is availble ?
		// That is we should wait for the first chunk of file data to be available

		// Because request from proxy is a chunk data request
		// so I have to check the first data of the 
		int64 filePos = 0;
		int lastError = checkFileData( request, cacheAble, response, filePos);
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(processSessionTransfer,"checkFileData get lasterror[%d]."), lastError);
		if(  lastError != 0 ) {
			TransferRange* resRange = NULL;
			SessionTransferResponseParamPtr responseTransfer = SessionTransferResponseParamPtr::dynamicCast(response);
			resRange = &(responseTransfer->range);
			const TransferRange& reqRange = mCurrentDataRunner->getTransferRange();
			resRange->startPos = reqRange.bStartValid ? MAX(reqRange.startPos,0):0;
			if(filePos >=0){
				resRange->endPos = reqRange.bEndValid ? MIN(reqRange.endPos,filePos):filePos;
			}
			else{
				resRange->bEndValid = false;
			}
			MLOG(ZQ::common::Log::L_INFO,SESSFMT(processSessionTransfer,"got response filesize[%ld]"),resRange->toString().c_str());
			// turn error code into http status code
			if( errorWorkingInProcess == lastError ) {
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(processSessionTransfer,"working in progress, return EAGAIN"));
				response->errorCode = lastError;
				return lastError;
			}
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

	if( !request->viaProxy ) {
		response->errorCode = errorCodePartialContent;
	} else {
		response->errorCode = errorCodeOK;
	}
	response->cacheable = cacheAble;

	changeState( SESSION_STATE_ACTIVE );

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processSessionTransfer,"took [%lu] ms"), sw.stop()/1000);

	return response->errorCode;
}

int32 C2Session::processUdpSession(const SessionUdpRequestParamPtr request , SessionUdpResponseParamPtr response)
{
	StopWatch sw; sw.start();

	int readerType = request->getConfUrlRule()->readerType;

	if( ( readerType == CLIENT_TYPE_DISKAIO ) && ( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_SHADOW ) )
	{
		updateLastError( request , response , errorCodeNotAllowed , "processUdpSession() shadow session is not allowed to GET, reject request",
				convertSessionStateToString(mSessionState) );		
		return errorCodeNotAllowed;
	}
	else if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mCurrentDataRunner->stopRunner();// stop runner before acquire session mutex to avoid dead lock
	}
	ZQ::common::MutexGuard gd(*this); //FIXME: can I remove this locker ?

	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mCurrentDataRunner->reset();
	}
	else
	{
		if( mSessionState == SESSION_STATE_ACTIVE )
		{
			updateLastError( request , response , errorCodeNotAllowed , "processUdpSession() session is in [%s] state, reject transfer request",
				convertSessionStateToString(mSessionState) );		
			return errorCodeNotAllowed;
		}
	}
	MLOG(ZQ::common::Log::L_INFO,SESSFMT(C2Session, "processUdpSession() param: client[%s:%d] range[%s]  ext[%s]"),
		 request->udpClientAddress.c_str() , request->udpClientPort , request->range.toString().c_str(),request->requestFileExt.c_str());

	/*
	if( request->bHasTransferDelay )
	{		
		mCurrentDataRunner->mSessTransferDelay = request->transferDelay;
	}
	*/
	if( request->getConfUrlRule()->mCheckClientWhileTransfer && request->udpClientAddress != mClientTransferAddress)
	{
		updateLastError( request , response , errorCodeBadRequest , "processUdpSession() session reserved for client[%s], not[%s]",
						 mClientTransferAddress.c_str() , request->udpClientAddress.c_str() );
		return errorCodeBadRequest;
	}
	
	const TransferRange& range = request->range;
	if( range.bStartValid || range.bEndValid )
		mCurrentDataRunner->setTransferRange(range);

	
	C2SessFile sf(mEnv, readerType );
	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{
		mRequestFileName = mContentName + request->requestFileExt;
	}

	sf.process( mRequestFileName , mSessionId);
	if( !sf.isValid() )
	{
		updateLastError( request , response, errorCodeContentNotFound,"processUdpSession() failed to find file[%s]",
				  mFileName.c_str());
		return errorCodeContentNotFound;
	}

	if( !sf.checkRequestRange( mCurrentDataRunner->getTransferRange() ) )
	{
		updateLastError( request, response , errorCodeBadRange,"processUdpSession() bad request range[%s]: file data range[%lld-%lld]",
				  mCurrentDataRunner->getTransferRange().toString().c_str() , sf.dataStartPos() ,sf.dataEndPos() );
		return errorCodeBadRange;
	}
	if( !mCurrentDataRunner->getTransferRange().bStartValid  )
	{
		TransferRange tmpRange = mCurrentDataRunner->getTransferRange();
		tmpRange.bStartValid = true;
		tmpRange.startPos = sf.dataStartPos();
		mCurrentDataRunner->setTransferRange(tmpRange);
	}
	/*
	if( request->seekIFrame && mbMainFile) {
		C2IndexRecordCenter& ircenter = mSvc.getIndexRecordCenter();
		C2IndexRecordCenter::IndexRecordWrapperPtr record = ircenter.get( mIndexFilePathname );
		if(record) {
			int64 tempStart = mCurrentDataRunner->mRequestRange.startPos;
			mCurrentDataRunner->mRequestRange.startPos = record->findNextIFrame( tempStart );
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(processSessionTransfer,"adjust request start position from [%lu] to [%lu]"),
					tempStart, mCurrentDataRunner->mRequestRange.startPos );
		}
	}
	*/
	
	response->filename 	= mRequestFileName; // use request file
	response->fileSize	= sf.fileDataSize();
	response->range		= composeAvailRange( mCurrentDataRunner->getTransferRange(), sf );
	//response->sessionType = mSessionType;
	
	if( BASE_SESSION_TYPE(mSessionType) == SESSION_TYPE_COMPLEX )
	{//reset full path name of request file
		mFileName			= sf.getFileFullPath();
	}

	cancelTimer();
	mConfWriter = request->getConfWriter();
	if(mReaderType < 0 ) {
		mReaderType = request->getConfUrlRule()->readerType;
	}
	mCurrentDataRunner->setConfWriter(mConfWriter);
	mCurrentDataRunner->setReaderType(mReaderType);
	
	//LibAsync::EventLoop* evtLoop = request->evtLoop;
	LibAsync::EventLoop* evtLoop = response->responseHandler->getLoop();
	assert( evtLoop != NULL);
	mReachFileEndCb = new DataReadCallback(this, evtLoop, true);
	mDataReadCb = new DataReadCallback(this,evtLoop,true);
	mWritableCb = new HttpWritableCallback( this );
	mAsyncTimer = new C2SessionAsyncTimer(this, evtLoop);
	mAsyncStopRunner = new SessionStopRunnerAsyncWork(this, evtLoop);

	mCurrentDataRunner->updateSessProperty(request->sessProp);
	response->responseHandler->setLocalAddr(mServerTransferAddress, 0);

	if( !mCurrentDataRunner->startRunner( response->responseHandler , this ) )
	{
		mReachFileEndCb = NULL;
		mDataReadCb = NULL;
		mWritableCb = NULL;
		mAsyncTimer = NULL;
		mAsyncStopRunner = NULL;
		updateLastError( request , response , errorCodeInternalError , "processUdpSession() failed to prepare data" );
		return errorCodeInternalError;
	}

	bool cacheAble = false;
	/*
	if( readerType != CLIENT_TYPE_DISKAIO && request->prop().queryIndex ) {
		// check if the file is availble ?
		// That is we should wait for the first chunk of file data to be available

		// Because request from proxy is a chunk data request
		// so I have to check the first data of the 
		int lastError = checkFileData( request, cacheAble, response);
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(processUdpSession,"checkFileData get lasterror[%d]."), lastError);
		if(  lastError != 0 ) {
			// turn error code into http status code
			if( errorWorkingInProcess == lastError ) {
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(processUdpSession,"working in progress, return EAGAIN"));
				response->errorCode = lastError;
				return lastError;
			}
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
			updateLastError( request, response, lastError,"processUdpSession() failed to get file data");
			return lastError;
		}
	}
	*/

	/*
	if( !request->viaProxy ) {
		response->errorCode = errorCodePartialContent;
	} else {
		response->errorCode = errorCodeOK;
	}
	*/
	response->errorCode = errorCodeOK;
	//response->cacheable = cacheAble;

	changeState( SESSION_STATE_ACTIVE );

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processSessionTransfer,"took [%lu] ms"), sw.stop()/1000);


	return response->errorCode;
}


int C2Session::checkFileData( const RequestParamPtr request,bool& cacheAble, RequestResponseParamPtr response, int64& filePos ) {
	int readerType = request->getConfUrlRule()->readerType;
    AioFile* f = mSvc.getCacheCenter().open( mRequestFileName, readerType, mSessionId);
    int64 offset = 0;
    if( mCurrentDataRunner->getTransferRange().bStartValid) {
        offset = mCurrentDataRunner->getTransferRange().startPos;
    }
	//TODO: how about removing file reading here ?
	BufferUser  bu = f->read(offset);
	TransferSessionCallbackForBuf::Ptr  sinker = new TransferSessionCallbackForBuf(mEnv, this, request->evtLoop, request, response, bu);
	if( bu.asyncWait(sinker, 1) ) {
		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processSessionTransfer,"wait buffer[%ld], for[%s:%ld]"), bu->reqId(), bu->filename().c_str(), bu->offsetInFile());
		return errorWorkingInProcess;
	}
	return checkFileAttr(request, cacheAble, response, bu, filePos);
}

int  C2Session::checkFileAttr(const RequestParamPtr request, bool& cacheAble, RequestResponseParamPtr response, const BufferUser& bu,int64& filePos )
{
	TransferRange*  resRange = NULL;
	if( METHOD_TRANSFER_RUN == request->method )
	{
		//SessionTransferParamPtr requestTransfer = SessionTransferParamPtr::dynamicCast(request);
		SessionTransferResponseParamPtr responseTransfer = SessionTransferResponseParamPtr::dynamicCast(response);
		//assert(requestTransfer);
		assert(responseTransfer);
		resRange = &(responseTransfer->range);
	}
	else if( METHOD_UDP_RUN == request->method ) 
	{
		//SessionUdpRequestParamPtr t  = SessionUdpRequestParamPtr::dynamicCast(request);
		SessionUdpResponseParamPtr s = SessionUdpResponseParamPtr::dynamicCast(response);
		//assert(t);
		assert(s);
		resRange = &(s->range);
	}
	else
	{
		return -1;
	}
	//TransferRange& resRange = response->range;
	TransferRange reqRange;
	if(mCurrentDataRunner){
		//const TransferRange& reqRange = mCurrentDataRunner->getTransferRange();//DO NOT use range in request, it may be empty
		reqRange = mCurrentDataRunner->getTransferRange();//DO NOT use range in request, it may be empty
	}

	resRange->startPos = reqRange.bStartValid ? MAX(reqRange.startPos, 0): 0;
	int64 fileEndPos = bu.fileSize() - 1;
	if( fileEndPos >= 0 ) {
		resRange->endPos = reqRange.bEndValid ? MIN(reqRange.endPos, fileEndPos): fileEndPos;
		filePos = resRange->endPos;
	} else {
		resRange->bEndValid = false;
	}
	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processSessionTransfer,"got fileSize[%ld] requestRange[%s] responseRange[%s]"),
			bu.fileSize(), reqRange.toString().c_str(), resRange->toString().c_str());

	int lastError = 0;
	int readerType = request->getConfUrlRule()->readerType;
	AssetAttribute::Ptr attr = mSvc.getSessManager().getAssetAttribute( mSessionId, mRequestFileName, readerType );
	if( attr ) {
		int64 timeStart = ZQ::common::now();
		//attr->wait();
		TransferSessionCallbackForAtt::Ptr sinker = new TransferSessionCallbackForAtt(mEnv, this, request->evtLoop, request, response, attr);
		if( attr->asyncWait(sinker) ){
			MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processSessionTransfer, "checkFileAttr() async wait for attr[%ld]."), attr->reqId());
			return errorWorkingInProcess;
		}
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

void   C2Session::setResponse(int lastError, RequestResponseParamPtr response, const RequestParamPtr request, bool cacheable)
{
	  if ( lastError != 0 )
	  {
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
			updateLastError( request, response, lastError, "processSessionTransfer() failed to get file data");
			response->response();
			return;
	  }
	  if( METHOD_TRANSFER_RUN == request->method )
	  {
		  SessionTransferParamPtr t   = SessionTransferParamPtr::dynamicCast(request);
		  SessionTransferResponseParamPtr s   = SessionTransferResponseParamPtr::dynamicCast(response);
		  assert(t);assert(s);
		  if( !t->viaProxy ) {
			  s->errorCode = errorCodePartialContent;
		  } else {
			  s->errorCode = errorCodeOK;
			  s->cacheable = cacheable;
		  }
	  }
	  else
	  {
		  response->errorCode = errorCodeOK;
	  }

	  changeState( SESSION_STATE_ACTIVE );
	  response->response();
	  return ;
}

bool C2Session::startTransfer(  )
{
	//int32 waitInterval = rand() % mEnv.getConfig().mFirstChunkWait + 5;
	updateAsyncTimer( 0 );
	return true;
}

void C2Session::runSession(int64 timeshift)
{
	switch ( mSessionState )
	{
	case SESSION_STATE_ACTIVE:
		{
			if( !mCurrentDataRunner->run(timeshift) )
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


