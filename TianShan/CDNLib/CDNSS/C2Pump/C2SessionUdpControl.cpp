#include <ZQ_common_conf.h>
#include <sys/types.h>
#include <FileSystemOp.h>
#include <TimeUtil.h>

#include "C2StreamerEnv.h"
#include "C2StreamerService.h"

#include "C2SessionHelper.h"
#include "C2Session.h"


#if defined ZQ_OS_MSWIN
	#define	REQFMT(x,y) 	CLOGFMT(x, " REQUEST[%s]\t"##y), request->requestHint.c_str() 
#elif defined ZQ_OS_LINUX
	#define	REQFMT(x,y) 	CLOGFMT(x, " REQUEST[%s]\t"y), request->requestHint.c_str()
#endif	

namespace C2Streamer {

int32 C2Session::processSessionUdpControl( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response ) {
	StopWatch sw;sw.start();
	ZQ::common::MutexGuard gd(*this);
	switch( request->ctrlCode ) {
	case SessionUdpControlRequestParam::CONTROL_LOAD:
		{
			return udpLoad( request, response );
		}
		break;
	case SessionUdpControlRequestParam::CONTROL_PLAY:
		{
			return udpPlay( request, response );
		}
		break;
	case SessionUdpControlRequestParam::CONTROL_PAUSE:
		{
			return udpPause( request, response );
		}
		break;
	case SessionUdpControlRequestParam::CONTROL_UNLOAD:
		{
			return udpUnload( request, response );
		}
		break;
	case SessionUdpControlRequestParam::CONTROL_INFOQUERY:
		{
			return udpInfoQuery( request, response );
		}
		break;
	default:
		{
			updateLastError(request, response, errorCodeBadRequest, "unknow control code: %d", request->ctrlCode);
			return errorCodeBadRequest;
		}		
	}
	return errorCodeBadRequest;
}


int32 C2Session::parseUdpRequest( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response , bool bToBeLoad) {
	MLOG.debug(REQFMT(parseUdpRequest, "parseUdpRequest enter"));
	if(request->ctrlCode == SessionUdpControlRequestParam::CONTROL_PAUSE ) {
		if( mCurrentDataRunner == NULL ) {
			return errorCodeBadRequest;
		}
		// 这里的response->filename不会再被校正，
		// 所以这里直接使用mCurrentDataRunner->getReqFilename() 
		// 而不是getReqAssetName()
		response->filename = mCurrentDataRunner->getReqFilename();
		response->scale = mCurrentDataRunner->getUdpSessionRunnerScale();
		response->timeOffset = mCurrentDataRunner->getUdpSessionRunnerNpt(&response->dataOffset);
		return errorCodeOK;
	} else if( request->ctrlCode == SessionUdpControlRequestParam::CONTROL_PLAY && mUdpSessionState == UDPSTATE_PAUSE ) {
		if( floatEqual(0.0f, request->scale) && request->timeOffset == 0 ) {
			if(!mCurrentDataRunner) {
				return errorCodeInternalError;
			}
			//response->filename会被校正， 所以这里使用getReqAssetName
			response->filename = mCurrentDataRunner->getReqAssetName();
			response->scale = mCurrentDataRunner->getUdpSessionRunnerScale();
			response->timeOffset = mCurrentDataRunner->getUdpSessionRunnerNpt(&response->dataOffset);
		}
	}
	int readerType = request->getConfUrlRule()->readerType;
	int32 result = errorCodeOK;
	MLOG.debug(REQFMT(parseUdpRequest, "mIndexDataReader parse begin sess[%s]"),mSessionId.c_str());
	if(mIndexDataReader == NULL) {
		mIndexDataReader =  getC2StreamerService()->getUdpInfoQuerier()->parse(response->filename, readerType, mSessionId);
	}

	if( mIndexDataReader == NULL || mIndexDataReader->getLastErrCode() != errorCodeOK) {
		return errorCodeBadRequest;
	}
	std::string ext;
	mIndexDataReader->findSubfileInfo( response->scale, ext, response->duration );

	if( bToBeLoad ) {
		if( request->timeOffset < 0 ) {
			if( response->scale < -0.001f ) {
				response->timeOffset = response->duration;//如果是快退，且不指定timeOffset，那么从文件最尾部开始播放
			} else {
				response->timeOffset = 0;
			}
		} 
		if( floatEqual(request->scale, 0.0f) ) {
			response->scale = mCurrentDataRunner->getUdpSessionRunnerScale();
		}
	} else if(mCurrentDataRunner != NULL) {
		if( request->timeOffset < 0 ) {
			response->timeOffset = mCurrentDataRunner->getUdpSessionRunnerNpt();
		}
		if( floatEqual( request->scale, 0.0f) ) {
			response->scale = mCurrentDataRunner->getUdpSessionRunnerScale();
			//refresh scale and ext
		}
	} else {
		response->timeOffset = 0;
		response->scale = 1.0f;
		//refresh scale and ext
	}
	response->bitrate = mIndexDataReader->getDataBitrate();
	if(response->bitrate <= 0) {
		MLOG.error(REQFMT(parseUdpRequest, "bad bitrate parsed from index file [%ld]"), response->bitrate );
		return errorCodeBadRequest;
	}

	mIndexDataReader->findSubfileInfo( response->scale, ext, response->duration );
	MLOG.debug(REQFMT (parseUdpRequest, "before adjusting timeOffset/inTimeOffset: %ld/%ld, outTimeOffset: %ld "), 
			response->timeOffset, response->inTimeOffset, response->outTimeOffset );
	response->timeOffset += response->inTimeOffset;
	int64  pretimeOffset = response->timeOffset, predataOffset = 0;
	float prescale = 0;
	if (mCurrentDataRunner)
	{
		mCurrentDataRunner->getUdpSessionRunnerNpt(&predataOffset) ;//response->dataOffset;
		prescale = mCurrentDataRunner->getUdpSessionRunnerScale();
	}
	response->dataOffset = mIndexDataReader->findDataOffset( response->timeOffset, response->scale );
	MLOG.info(REQFMT (parseUdpRequest, "file[%s] change scale, requested timeOffset[%ld] dataOffset[%ld] former scale[%.3f] adjusted to timeOffset[%ld] dataOffset[%ld] hexdataOffset[%02X] scale[%.3f] subfile[%s]"),
			response->filename.c_str(), pretimeOffset, predataOffset, prescale , response->timeOffset, response->dataOffset, (response->dataOffset/188),response->scale, ext.c_str()); 
	if( response->duration < response->timeOffset ) {
		response->timeOffset = response->duration;
	}
	response->inTimeOffset = response->timeOffset;
	if(response->outTimeOffset > 0 ) {
		response->outDataOffset = mIndexDataReader->findDataOffset( response->outTimeOffset, response->scale ) - 1;
		if( response->outTimeOffset > response->duration ) {
			response->outTimeOffset = response->duration;
		}
		if( response->outTimeOffset <= response->timeOffset ) {
			MLOG.debug(REQFMT(parseUdpRequest, "invalid request due to: inTimeOffset[%ld] outTimeOffset[%ld]"), response->timeOffset, response->outTimeOffset);
			return errorCodeBadRequest;
		}
		if( response->outDataOffset <= response->timeOffset ) {
			MLOG.debug(REQFMT(parseUdpRequest, "invalid request due to: inDataOffset[%ld] outDataOffset[%ld]"), response->dataOffset, response->outDataOffset);
			return errorCodeBadRequest;
		}
	}
	response->filename = response->filename + ext;

	MLOG.debug( REQFMT(parseUdpRequest, "get sub file info: request scale[%f] timeOffset[%ld] inTimeOffset[%ld] outTimeOffset[%ld]  -> response ext[%s] scale[%f] inTimeOffset[%ld] outTimeOffset[%ld] inDataOffset[%ld] outDataOffset[%ld] hexdataOffset[%02X] duration[%ld] dataBitrate[%ld] filename[%s]"),
			request->scale, request->timeOffset, request->inTimeOffset, request->outTimeOffset,
			ext.c_str(), response->scale, response->timeOffset, response->outTimeOffset,
			response->dataOffset, response->outDataOffset, (response->outDataOffset/188),response->duration, response->bitrate,response->filename.c_str());

	return result;
}

int32 C2Session::udpLoad( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response) {

	response->filename = request->assetName;
	response->timeOffset = request->timeOffset;
	response->scale = request->scale;
	response->inTimeOffset = request->inTimeOffset;
	response->outTimeOffset = request->outTimeOffset;
	if( response->timeOffset == 0 && floatEqual(response->scale, 0.0f) ) {
		if(mCurrentDataRunner) {
			float currentScale = mCurrentDataRunner->getUdpSessionRunnerScale();
			if(currentScale < -0.01f) {
				response->timeOffset = -1;
			}
			response->scale = currentScale;
		}
	}

	if( parseUdpRequest( request, response, true)  != errorCodeOK ) {
		updateLastError( request, response, errorCodeBadRequest, "udpLoad, failed to parse request param");
		return errorCodeBadRequest;
	}
	if(floatEqual(response->scale, 0.0f) ) {
		response->scale = 1.0f;
	}

	MLOG.debug(REQFMT(udpLoad,"trying to load item[%s] scale[%f] timeOffset[%ld] dataOffset[%ld]"), 
			response->filename.c_str(), response->scale, response->timeOffset, response->dataOffset);

	SessionDataRunner::Ptr runner = new SessionDataRunner( mEnv, mSvc, this, mSvc.getCacheCenter(), *this, mSvc.getSessManager().genSubSessionId());

	TransferRange range;
	range.bStartValid = true;
	range.startPos = response->dataOffset;
	if( response->outDataOffset > 0 ) {
		range.endPos = response->outDataOffset;
		range.bEndValid = true;
	}
	runner->setTransferRange( range );
	runner->setReqAssetName( request->assetName );
	runner->setUdpSessionRunnerStartNpt( response->timeOffset );
	int64 targetBitrate = response->bitrate;
	static int64 brInflation = request->getConfWriter()->mBitrateInflationPercent;
	brInflation = MIN(150, MAX(80, brInflation));
	targetBitrate *= brInflation;
	targetBitrate /= 100;
	runner->setTransferRate( targetBitrate);
	runner->setResponseHandler( mResponseHandler );
	runner->setReqFilename( response->filename );
	runner->setConfWriter( mConfWriter );
	runner->setReaderType( mReaderType );
	runner->setUdpSessionRunnerScale( response->scale );
	runner->setUdpSessionRunnerState( UDPSTATE_PLAY );

	if(!runner->prepareUdpRunner()) {
		updateLastError(request, response, errorCodeInternalError,  "udpLoad, failed to start data runner");
		return errorCodeInternalError;
	}
	
	switch(mUdpSessionState) {
	case UDPSTATE_NONE://fallthrough
	case UDPSTATE_DONE: // 由于mCurrentDataRunner在C2Session实例化的时候就会被创建，所以需要这个判断
		{
			mCurrentDataRunner = runner;
			mUdpSessionState = UDPSTATE_INIT;
		}
		break;
	default:
		{
			if(mCurrentDataRunner == NULL) {
				mCurrentDataRunner = runner;
			}  else {
				mBackupRunners.push_back(runner);
			}
		}
		break;
	}
	response->subSessionId = runner->getRunnerId();
	MLOG.info(REQFMT(udpLoad,"successfully load[%s] at[%ld]ms scale[%f]: %s/%f/%ld/%ld, runner[%ld-%d]"),
			request->assetName.c_str(), request->timeOffset, request->scale,
			response->filename.c_str(), response->scale, response->timeOffset, response->dataOffset,
			runner->getRunnerId(), runner->getRunnerVersion());
	return errorCodeOK;
}


SessionDataRunner::Ptr C2Session::makeShadowUdpSessionRunner( SessionDataRunner::Ptr from, SessionUdpControlResponseParamPtr response, bool bPauseRunner ) {
	//FIXME: not implemented
	SessionDataRunner::Ptr runner;
	if(!bPauseRunner) {
		runner = new SessionDataRunner( mEnv, mSvc, this, mSvc.getCacheCenter(), *this, from->getRunnerId());
	} else {
		UdpPauseFrameRunner* pauseRunner = new UdpPauseFrameRunner( mEnv, mSvc, this, mSvc.getCacheCenter(), *this, from->getRunnerId());
		runner = pauseRunner;
		uint16 BFrameSize = 0 ;
		const unsigned char* bFrame = mIndexDataReader->getZeroMotionBFrame(BFrameSize);
		uint16 PFrameSize = 0;
		const unsigned char* pFrame = mIndexDataReader->getZeroMotionPFrame(PFrameSize);

		pauseRunner->setBZeroMotionFrame((const char*)bFrame, BFrameSize);
		pauseRunner->setPZeroMotionFrame((const char*)pFrame, PFrameSize);
	}
	//transferRange will be changed, so do not fill it
	runner->setRunnerVersion( from->getRunnerVersion() + 1 );
	runner->setTransferRate( from->getTransferRate() );
	runner->setResponseHandler( mResponseHandler );
	runner->setReqFilename( response->filename );
	runner->setReqAssetName( from->getReqAssetName() );
	runner->setConfWriter( mConfWriter );
	runner->setReaderType(mReaderType);
	runner->setUdpSessionRunnerStartNpt( response->timeOffset );
	runner->setUdpSessionRunnerState( bPauseRunner ? UDPSTATE_PAUSE : UDPSTATE_PLAY );
	runner->setUdpSessionRunnerScale( response->scale );

	TransferRange range;
	range.bStartValid = true;
	range.startPos = response->dataOffset;
	if(response->outDataOffset > 0) {
		range.bEndValid = true;
		range.endPos = response->outDataOffset;
	}

	runner->setTransferRange( range );
	return runner;
}

int C2Session::detectUdpRequestVerb( const SessionUdpControlRequestParamPtr request, const SessionUdpControlResponseParamPtr response ) const {
	const int64& timeOffset = response->timeOffset;
	const float& scale = response->scale;
	int verb = UDP_CHANGE_NONE;

	if( !(floatEqual(scale, mCurrentDataRunner->getUdpSessionRunnerScale()) ||
				floatEqual(scale, 0.0f)) ) {
		verb |= UDP_CHANGE_SCALE;
	}

	if( request->timeOffset >= 0 && timeOffset >= 0 && abs( timeOffset - mCurrentDataRunner->getUdpSessionRunnerNpt() ) >= 1000) {
		verb |= UDP_CHANGE_OFFSET;
	}
	if( (request->ctrlCode == SessionUdpControlRequestParam::CONTROL_PLAY && mUdpSessionState == UDPSTATE_PAUSE ) || 
		(request->ctrlCode == SessionUdpControlRequestParam::CONTROL_PAUSE && mUdpSessionState == UDPSTATE_PLAY ) ) {
		verb |= UDP_CHANGE_STATE;
	}
	MLOG.debug(REQFMT(detectUdpRequestVerb,"req[%f/%ld] vs current[%f/%ld]"),
			response->scale, request->timeOffset,
			mCurrentDataRunner->getUdpSessionRunnerScale(),
			mCurrentDataRunner->getUdpSessionRunnerNpt());

	return verb;
}

int32 C2Session::udpPlay( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response) {
	if( mCurrentDataRunner == NULL ) {
		updateLastError( request, response, errorCodeBadRequest, "udpPlay, must load item before play");
		return errorCodeBadRequest;
	}

	if( mCurrentDataRunner->getRunnerId() != request->subSessionId ) {
		SessionDataRunner::Ptr runner = NULL;
		std::vector<SessionDataRunner::Ptr>::const_iterator it = mBackupRunners.begin();
		for( ; it != mBackupRunners.end(); it ++ ) {
			if( (*it)->getRunnerId() == request->subSessionId ) {
				runner = *it;
				break;
			}
		}
		if(!runner) {
			updateLastError( request, response, errorCodeBadRequest, "udpPlay, failed to find runner[%ld]", request->subSessionId);
			return errorCodeBadRequest;
		}
		response->filename = runner->getReqFilename();
		response->timeOffset = runner->getUdpSessionRunnerNpt(&response->dataOffset, &response->outDataOffset);
		response->scale = runner->getUdpSessionRunnerScale();
		MLOG.info(REQFMT(udpPlay, "trying to play an init session[%ld], return as successfully"), request->subSessionId);
		return errorCodeOK;
	} else {
		response->filename = request->assetName;
		if( response->filename.empty() ) {
			// 使用当前的assetName作为返回文件名
			// 在parseUdpRequest的时候会使用response->filename
			// 并对其做出校正
			response->filename = mCurrentDataRunner->getReqAssetName();
		}
		response->scale = request->scale;
		response->timeOffset = request->timeOffset;
		response->inTimeOffset = request->inTimeOffset;
		response->outTimeOffset = request->outTimeOffset;
	}

	if( parseUdpRequest( request, response, false ) != errorCodeOK ) {
		updateLastError(request, response, errorCodeBadRequest, "udpPlay, failed to parse request param");
		return errorCodeBadRequest;
	}

	// response->filename = mCurrentDataRunner->getReqAssetName() + response->filename;

	MLOG.debug(REQFMT(udpPlay,"trying to play session[%ld], adjust req[%f/%ld]  to scale[%f] timeOffset[%ld] "),
			request->subSessionId,
			request->scale, request->timeOffset,
			response->scale, response->timeOffset);

	int verb = detectUdpRequestVerb( request , response);
	if( verb != UDP_CHANGE_NONE ) {
		if( mShadowDataRunner != NULL ) {
			// last change is not complete, reject new change request
			updateLastError(request, response, errorCodeRequestConflict, "udpPlay, request conflict, last change is not complete");
			return errorCodeRequestConflict;
		}
	}
	switch( mUdpSessionState ) {
	case UDPSTATE_NONE://fallthrough
	case UDPSTATE_DONE:
		{
			updateLastError( request, response, errorCodeBadRequest, "udpPlay, invalid state[%d]", mUdpSessionState );
			return errorCodeBadRequest;
		}
		break;
	case UDPSTATE_INIT:
		{
			if( verb == UDP_CHANGE_NONE ) {
				MLOG.debug(REQFMT(udpPlay, "nothing changed since item load, run session data runner[%ld-%d]"), 
						mCurrentDataRunner->getRunnerId(), mCurrentDataRunner->getRunnerVersion());
				runCurrentSessionRunner( );
				break;
			} else {//about to change
				MLOG.debug(REQFMT(udpPlay, "params changed since item loaded, start a shadown sesion, and stop current runner[%ld-%d]"), 
						mCurrentDataRunner->getRunnerId(), mCurrentDataRunner->getRunnerVersion());
				mShadowDataRunner = makeShadowUdpSessionRunner( mCurrentDataRunner, response );
				if(!mShadowDataRunner->prepareUdpRunner()) {
					MLOG.error(REQFMT(udpPlay, "failed to prepare shadow session"));
					return errorCodeBadRequest;
				}
				stopUdpSessionRunner( mCurrentDataRunner );
			}
			mUdpSessionState = UDPSTATE_PLAY;
		}
		break;
	case UDPSTATE_PLAY:
		{
			if( verb == UDP_CHANGE_NONE ) {
				MLOG.info(REQFMT(udpPlay, "request is same as running session, return as successfully"));
				break;
			}
			MLOG.debug(REQFMT(udpPlay, "params changed while runner running, start a shadown sesion, and stop current runner[%ld-%d]"), 
					mCurrentDataRunner->getRunnerId(), mCurrentDataRunner->getRunnerVersion());
			mShadowDataRunner = makeShadowUdpSessionRunner( mCurrentDataRunner, response );
			if(!mShadowDataRunner->prepareUdpRunner() ) {
				MLOG.error(REQFMT(udpPlay, "failed to prepare shadow session"));
			}
			stopUdpSessionRunner(mCurrentDataRunner);
		}
		break;
	case UDPSTATE_PAUSE:
		{
			if(verb == UDP_CHANGE_NONE ) {
				MLOG.info(REQFMT(udpPlay, "request is same as running session, return as successfully"));
				break;
			}
			MLOG.debug(REQFMT(udpPlay, "params changed while runner running, start a shadown sesion, and stop current runner[%ld-%d]"),
					mCurrentDataRunner->getRunnerId(), mCurrentDataRunner->getRunnerVersion());
			mShadowDataRunner = makeShadowUdpSessionRunner( mCurrentDataRunner, response );
			if(!mShadowDataRunner->prepareUdpRunner() ) {
				MLOG.error(REQFMT(udpPlay, "failed to prepare shadow session"));
			}
			stopUdpSessionRunner(mCurrentDataRunner);

		}
		break;
	default:
		{
			updateLastError(request, response, errorCodeInternalError, "udpPlay, logic error, invalid udp state[%d]", mUdpSessionState);
			return errorCodeInternalError;
		}
		break;
	}

	mUdpSessionState = UDPSTATE_PLAY;

	response->subSessionId = mCurrentDataRunner->getRunnerId();

	MLOG.info(REQFMT(udpPlay, "run session at[%s/%f/%ld/%ld]"), 
			response->filename.c_str(), response->scale, response->timeOffset, response->dataOffset);
	return errorCodeOK;
}

int32 C2Session::udpPause( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response) {
	//FIXME: not implemented
	int verb = detectUdpRequestVerb( request, response );
	if( verb == UDP_CHANGE_NONE ) {
		return errorCodeOK;
	}
	if( !(verb & UDP_CHANGE_STATE)) {
		updateLastError(request, response, errorCodeBadRequest, REQFMT(udpPause,"bad request"));
		return errorCodeBadRequest;
	}

	if( mShadowDataRunner != NULL ) {
		// last change is not complete, reject new change request
		updateLastError(request, response, errorCodeRequestConflict, "udpPause, request conflict, last change is not complete");
		return errorCodeRequestConflict;
	}
	if( !mCurrentDataRunner) {
		updateLastError(request, response, errorCodeBadRequest, "udpPause, can not pause an un-streaming session");
		return errorCodeBadRequest;
	}
	if( mCurrentDataRunner->getUdpSessionRunnerState() == UDPSTATE_PAUSE ) {
		MLOG.info(REQFMT(udpPause, "current session is in pause state, skip request"));
		return errorCodeOK;
	}
	response->filename = mCurrentDataRunner->getReqFilename();
	response->scale = mCurrentDataRunner->getUdpSessionRunnerScale();
	response->timeOffset = mCurrentDataRunner->getUdpSessionRunnerNpt(&response->dataOffset, &response->outDataOffset);
	mShadowDataRunner = makeShadowUdpSessionRunner(mCurrentDataRunner, response, true);
	if(!mShadowDataRunner->prepareUdpRunner()) {
		updateLastError(request, response, errorCodeBadRequest, "udpPause, failed to parepare udp runner");
		return errorCodeBadRequest;
	}
	response->subSessionId = mShadowDataRunner->getRunnerId();
	mUdpSessionState = UDPSTATE_PAUSE;
	stopUdpSessionRunner(mCurrentDataRunner);
	MLOG.info( REQFMT(udpPause,"paused at file[%s] timeOffset[%ld] scale[%f]"),
			response->filename.c_str(), response->timeOffset, response->scale);
	return errorCodeOK;
}

int32 C2Session::udpInfoQuery( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response ) {
	MLOG.debug(REQFMT(infoQuery, "query info for session[%ld]"), request->subSessionId);
	if( request->subSessionId == 0 ) {
		MLOG.debug(REQFMT(infoQuery, "query an unloaded item [%s]"), request->assetName.c_str());
		if( request->assetName.empty()) {
			MLOG.error(REQFMT(infoQuery,"query unloaded item but no assetName is provided, reject"));
			return errorCodeBadRequest;
		}
		int readerType = request->getConfUrlRule()->readerType;
		int32 result = errorCodeOK;
		response->filename = request->assetName;
		if(mIndexDataReader == NULL) {
			mIndexDataReader =  getC2StreamerService()->getUdpInfoQuerier()->parse(response->filename, readerType, mSessionId);
		}
		if( mIndexDataReader == NULL) {
			return errorCodeBadRequest;
		}
		std::string ext;
		response->scale = 1.0;
		mIndexDataReader->findSubfileInfo( response->scale, ext, response->duration );
		MLOG.info(REQFMT(udpInfoQuery, "get info for unloaded item: filename[%s] scale[%f] duration[%ld]"),
			response->filename.c_str(), 
			response->scale, response->duration);
		return errorCodeOK;
	}

	if( !mCurrentDataRunner ) {
		updateLastError(request, response, errorCodeBadRequest, REQFMT(udpInfoQuery, "udpInfoQuery, no running session"), request->subSessionId);
		return errorCodeBadRequest;
	}
	
	SessionDataRunner::Ptr runner = NULL;
	if( request->subSessionId == mCurrentDataRunner->getRunnerId() ) {
		if( mShadowDataRunner != NULL ) {
			runner = mShadowDataRunner; //如果当前有shadow runner， 说明当前正在做状态转换，所以直接使用状态转换的目标值
		} else {
			runner = mCurrentDataRunner;
		}
	} else {
		std::vector<SessionDataRunner::Ptr>::const_iterator it = mBackupRunners.begin();
		for( ;it != mBackupRunners.end(); it ++ ) {
			if( (*it)->getRunnerId() == request->subSessionId ) {
				runner = *it;
				break;
			}
		}
	}
	if(!runner) {
		updateLastError(request, response, errorCodeBadRequest, REQFMT(udpInfoQuery, "udpInfoQuery, session[%ld] is not found"), request->subSessionId);
		return errorCodeBadRequest;
	}

	response->subSessionId = runner->getRunnerId();
	response->timeOffset = runner->getUdpSessionRunnerNpt( &response->dataOffset, &response->outDataOffset );
	response->scale = runner->getUdpSessionRunnerScale();
	response->filename = runner->getReqFilename();
	std::string ext; //temp
	mIndexDataReader->findSubfileInfo( response->scale, ext, response->duration );
	MLOG.info(REQFMT(udpInfoQuery, "get info for subSess[%ld]: filename[%s] timeOffset[%ld] dataOffset[%ld] scale[%f] duration[%ld]"),
			request->subSessionId, response->filename.c_str(), 
			response->timeOffset, response->dataOffset,
			response->scale, response->duration);
	return errorCodeOK;
}

class DataRunnerAsyncStopper : public LibAsync::AsyncWork {
public:
	DataRunnerAsyncStopper( SessionDataRunner::Ptr runner, LibAsync::EventLoop* loop )
	:LibAsync::AsyncWork(*loop),
	mRunner(runner) {
	}
	virtual ~DataRunnerAsyncStopper() {
	}
private:
	
	virtual void	onAsyncWork() {
		mRunner->stopRunner();
	}
private:
	SessionDataRunner::Ptr 	mRunner;
};

int32 C2Session::udpUnload( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response) {
	SessionDataRunner::Ptr runner = NULL;
	ZQ::common::MutexGuard gd(*this);
	if( mCurrentDataRunner && mCurrentDataRunner->getRunnerId() == request->subSessionId ) {
		runner = mCurrentDataRunner;
		mShadowDataRunner = NULL;//置shadow为空， 这样onDataRunnerStopped()的时候会直接跳过进入backupRunners
	} else {
		std::vector<SessionDataRunner::Ptr>::iterator it = mBackupRunners.begin();
		for(; it != mBackupRunners.end(); it ++ ) {
			if( (*it)->getRunnerId() == request->subSessionId ) {
				runner = *it;
				mBackupRunners.erase(it);
				break;
			}
		}
	}
	stopUdpSessionRunner( runner );

	return errorCodeOK;
}

#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	CLOGFMT(x, " Session[%s]\t"##y), mSessionId.c_str() 
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	CLOGFMT(x, " Session[%s]\t"y), mSessionId.c_str()
#endif	


void C2Session::stopUdpSessionRunner( SessionDataRunner::Ptr runner) {
	MLOG.debug(SESSFMT(stopUdpSessionRunner, "stopping runner[%ld-%d]"), runner->getRunnerId(), runner->getRunnerVersion());
	(new DataRunnerAsyncStopper(runner, mResponseHandler->getLoop()))->queueWork();
}

void C2Session::runCurrentSessionRunner( ) {
	assert( mCurrentDataRunner != NULL );
	updateAsyncTimer(mEnv.getConfig().stateSwitchInterval);//test 0
	MLOG.debug(SESSFMT(C2Session, "runCurrentSessionRunner() and updateAsyncTimer[%d]"),mEnv.getConfig().stateSwitchInterval);
}

void C2Session::onDataRunnerStopped( int64 id, int version ) {
	ZQ::common::MutexGuard gd(*this);
	if( id != mCurrentDataRunner->getRunnerId() ) {
		MLOG.warning( SESSFMT(onDataRunnerStopped, "subSess[%ld-%d] is done, but is not current running subsession"), 
				id, version);
		return;
	}
	if( mShadowDataRunner == NULL && mBackupRunners.size() == 0) {
		reportUdpSessionDone( mCurrentDataRunner->getRunnerId() );
		destroy();
		mCurrentDataRunner = NULL;
		return;
	}
	if( mShadowDataRunner != NULL ) {
		MLOG.debug(SESSFMT(onDataRunnerStopped, "subSess[%ld-%d] is done, trying to run next shadow runner"), mCurrentDataRunner->getRunnerId(), mCurrentDataRunner->getRunnerVersion());
		if( !floatEqual( mCurrentDataRunner->getUdpSessionRunnerScale(),
					mShadowDataRunner->getUdpSessionRunnerScale() ) ) {

			reportUdpSessionScaleChange( mCurrentDataRunner->getRunnerId(),
					mCurrentDataRunner->getUdpSessionRunnerScale(),
					mShadowDataRunner->getUdpSessionRunnerScale(),
					mCurrentDataRunner->getUdpSessionRunnerNpt() );
		} else if( mCurrentDataRunner->getUdpSessionRunnerState() != mShadowDataRunner->getUdpSessionRunnerState() ) {
			reportUdpSessionStateChange( mCurrentDataRunner->getRunnerId(),
						mCurrentDataRunner->getUdpSessionRunnerState(),
						mShadowDataRunner->getUdpSessionRunnerState(),
						mCurrentDataRunner->getUdpSessionRunnerNpt() );
		}

		mCurrentDataRunner = mShadowDataRunner;
		mShadowDataRunner = NULL;
	} else {
		MLOG.debug(SESSFMT(onDataRunnerStopped, "subSess[%ld-%d] is done, trying to run next runner[%ld-%d]"), 
				mCurrentDataRunner->getRunnerId(), mCurrentDataRunner->getRunnerVersion(),
				mBackupRunners[0]->getRunnerId(), mBackupRunners[0]->getRunnerVersion() );
		reportUdpSessionDone( mCurrentDataRunner->getRunnerId() );
		mCurrentDataRunner = mBackupRunners[0];
		mBackupRunners.erase(mBackupRunners.begin());
		mCurrentDataRunner->setResponseHandler(mResponseHandler);
	}
	runCurrentSessionRunner();
}
}//namespace C2Streamer
