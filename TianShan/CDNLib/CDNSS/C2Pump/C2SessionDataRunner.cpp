#include <sstream>
#include <ZQ_common_conf.h>
#include <TimeUtil.h>
#include "AioFile.h"
#include "C2Session.h"
#include "C2StreamerEnv.h"
#include "C2StreamerService.h"
#include "C2SessionHelper.h"
#include "HttpEngineInterface.h"
#include "C2SessionStatDump.h"
#include <libasync/eventloop.h>

#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	CLOGFMT(x, "[%s/%ld-%d]\t"##y), mSessId.c_str(), mSubSessionRunnerId, mSubSessionRunnerVersion
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	CLOGFMT(x, "[%s/%ld-%d]\t" y), mSessId.c_str(), mSubSessionRunnerId, mSubSessionRunnerVersion
#endif	
#include <sys/times.h>
namespace C2Streamer
{
//////////////////////////////////////////////////////////////////////////
DataRunnerStatistic::DataRunnerStatistic()
:mRequestedBitrate(0),
mTotalDataSendCount(0),
mMaxDataSendCount(0),
mMinDataSendCount(0xFFFFFFFF),
mDataSendRecordCount(0),
mTotalSendTime(0),
mMaxSendTime(0),
mMinSendTime(0xFFFFFFFF),
mSendTimeRecordCount(0),
mTotalFsWaitTime(0),
mMaxFsWaitTime(0),
mMinFsWaitTime(0xFFFFFFFF),
mFsWaitTimeRecordCount(0),
mTotalRunTime(0),
mMaxRunTime(0),
mMinRunTime(0xFFFFFFFF),
mRunTimeRecordCount(0),
mFirstChunkWait(0),
mTotalTimerShift(0),
mMaxTimerShift(0),
mMinTimerShift(0xFFFFFFFF),
mTimerShiftCount(0),
mSentBytes(0),
mRangeStart(0),
mRangeEnd(0),
mDuration(0),
mIoCommitTimeMax(0),
mIoCommitTimeMin(0xFFFFFFFF),
mIoCommitTimeTotal(0),
mIoCommitCounts(0)
{
}

DataRunnerStatistic::~DataRunnerStatistic()
{
}

void DataRunnerStatistic::reset()
{
	mTotalDataSendCount = 0;
	mMaxDataSendCount = 0;
	mMinDataSendCount = 0xFFFFFFFF;
	mDataSendRecordCount = 0;
	mTotalSendTime = 0;
	mMaxSendTime = 0;
	mMinSendTime  = 0xFFFFFFFF;
	mSendTimeRecordCount = 0;
	mTotalFsWaitTime = 0;
	mMaxFsWaitTime = 0;
	mMinFsWaitTime = 0xFFFFFFFF;
	mFsWaitTimeRecordCount = 0;
	mTotalRunTime = 0;
	mMaxRunTime = 0;
	mMinRunTime  = 0xFFFFFFFF;
	mRunTimeRecordCount = 0;
	mFirstChunkWait = 0;
	mTotalTimerShift = 0;
	mMaxTimerShift = 0;
	mMinTimerShift = 0xFFFFFFFF;
	mTimerShiftCount = 0;
	mSentBytes = 0;
	mRangeStart = 0;
	mRangeEnd = 0;
	mDuration = 0;	
	mIoCommitTimeMax = 0;	
	mIoCommitTimeMin = 0xFFFFFFFF;	
	mIoCommitTimeTotal = 0;
	mIoCommitCounts = 0;
}

#define SEPMARK <<"/"<<
std::string	DataRunnerStatistic::toString() const
{
//	std::ostringstream oss;
// 	oss <<"SocketSend["<<mTotalSendTime<<"/"<<mMaxSendTime<<"/"<<mMinSendTime<<"/"<<(int64)(mSendTimeRecordCount>0 ? mTotalSendTime/mSendTimeRecordCount:0)<<"] "
// 		<<"DataSendCount["<<mTotalDataSendCount<<"/"<<mMaxDataSendCount<<"/"<<mMinDataSendCount<<"/"<<int64(mDataSendRecordCount>0 ? mTotalDataSendCount/mDataSendRecordCount : 0 )<<"] "
// 		<<"OneRuntimeCost["<<mTotalRunTime<<"/"<<mMaxRunTime<<"/"<<mMinRunTime<<"/"<<int64( mRunTimeRecordCount > 0 ? mTotalRunTime/mRunTimeRecordCount : 0 )<<"] "
// 		<<"FsWait["<<mTotalFsWaitTime<<"/"<<mMaxFsWaitTime<<"/"<<mMinFsWaitTime<<"/"<<int64( mFsWaitTimeRecordCount > 0 ? mTotalFsWaitTime/mFsWaitTimeRecordCount : 0 )<<"] "
// 		<<"TimerShift["<<mTotalTimerShift<<"/"<<mMaxTimerShift<<"/"<<mMinTimerShift<<"/"<<int64( mTimerShiftCount > 0 ? mTotalTimerShift/mTimerShiftCount : 0 )<<"] "
// 		<<"FirstChunkWait["<<mFirstChunkWait<<"] ";

	char statbuf[1024], *p = statbuf, *t= statbuf + sizeof(statbuf) -2;

#define VALID_WAIT_USEC(_VAR) ((_VAR <0 || _VAR > (mDuration*1000)) ? -1: _VAR)

	p += snprintf(p, t -p, "req[%ld]bps ", mRequestedBitrate);
	p += snprintf(p, t -p, "range[%lu +%lu]@%lubps ", mRangeStart, mSentBytes, (int64 (mDuration>0?mSentBytes*8000/mDuration:0)) );
	p += snprintf(p, t -p, "duration[%lu]ms@%.2fppm ", mDuration, (mDuration > 0 ? ((float) VALID_WAIT_USEC(mTotalSendTime)*1000 / mDuration): 0.0));
	p += snprintf(p, t -p, "send-latency[%lu,a%lu,m%lu]us ", VALID_WAIT_USEC(mMinSendTime), (int64)(mSendTimeRecordCount>0 ? VALID_WAIT_USEC(mTotalSendTime)/mSendTimeRecordCount:0), VALID_WAIT_USEC(mMaxSendTime));
	p += snprintf(p, t -p, "buf-per-send[%lu,a%lu,m%lu] ", mMinDataSendCount, (int64)(mDataSendRecordCount>0 ? mTotalDataSendCount/mDataSendRecordCount :0), mMaxDataSendCount);
	p += snprintf(p, t -p, "read-latency[%lu,a%lu,m%lu]us ", VALID_WAIT_USEC(mMinFsWaitTime), (int64)( mFsWaitTimeRecordCount > 0 ? VALID_WAIT_USEC(mTotalFsWaitTime)/mFsWaitTimeRecordCount :0), VALID_WAIT_USEC(mMaxFsWaitTime));
//	p += snprintf(p, t -p, "read-speed[%lld]bps ", (int64)( mFsTotalReadTime>0 ? mSentBytes*8000/mFsTotalReadTime:0);
	p += snprintf(p, t -p, "filtered-timer-lag[1st%lu,%lu,a%lu,m%lu]us x%d ", VALID_WAIT_USEC(mFirstChunkWait), VALID_WAIT_USEC(mMinTimerShift), (int64( mTimerShiftCount > 0 ? VALID_WAIT_USEC(mTotalTimerShift)/mTimerShiftCount :0)), VALID_WAIT_USEC(mMaxTimerShift), (int)mTimerShiftCount);

	return statbuf;
//	return oss.str();
}

void DataRunnerStatistic::updateRequestedBitrate(int64 reqBitrate) {
	mRequestedBitrate = reqBitrate;
}

void DataRunnerStatistic::updateSessionStatus( int64 sentBytes , int64 rangeStart, int64 rangeEnd , int64 duration )
{
	mSentBytes	= sentBytes;
	mRangeStart	= rangeStart;
	mRangeEnd	= rangeEnd;
	mDuration	= duration;
}

void DataRunnerStatistic::updateSendDataStat( int64 sendTime )
{
	mTotalSendTime += sendTime;
	mMinSendTime	= MIN( mMinSendTime , sendTime);
	mMaxSendTime	= MAX( mMaxSendTime , sendTime );
	++mSendTimeRecordCount;
}

void DataRunnerStatistic::updateOneProcessCountData( int64 sendCount )
{
	
	mTotalDataSendCount		+= sendCount;
	mMaxDataSendCount		= MAX( mMaxDataSendCount , sendCount );
	mMinDataSendCount		= MIN( mMinDataSendCount , sendCount );
	++mDataSendRecordCount;
}

void DataRunnerStatistic::updateOneRunStat( int64 runTime)
{
	mTotalRunTime 	+= runTime;
	mMaxRunTime		= MAX( mMaxRunTime , runTime );
	mMinRunTime		= MIN( mMinRunTime , runTime );
	++mRunTimeRecordCount;
}

void DataRunnerStatistic::updateFsWaitStat( int64 waitTime )
{	
	mTotalFsWaitTime	+= waitTime;
	mMaxFsWaitTime		= MAX( mMaxFsWaitTime , waitTime );
	mMinFsWaitTime		= MIN( mMinFsWaitTime , waitTime );
	++mFsWaitTimeRecordCount;
}

void DataRunnerStatistic::updateTimerShift( int64 waitTime )
{
	if(mFirstChunkWait == 0 )
	{
		mFirstChunkWait = waitTime;
	}
	if( waitTime < 10 )
		return;
	mTotalTimerShift += waitTime;
	mMaxTimerShift	= MAX( mMaxTimerShift , waitTime );
	mMinTimerShift	= MIN( mMinTimerShift , waitTime );
	++mTimerShiftCount;
}

void DataRunnerStatistic::updateIoCommit( int64 commitTime )
{
	mIoCommitCounts++;
	mIoCommitTimeTotal += commitTime;
	mIoCommitTimeMax = MAX( mIoCommitTimeMax , commitTime );
	mIoCommitTimeMin = MIN( mIoCommitTimeMin , commitTime );
}

SessionDataRunner::SessionDataRunner( C2StreamerEnv& env , C2Service& svc , C2SessionPtr sess, CacheCenter& cc, ZQ::common::Mutex& m, int64 subSessId )
:mbRunning(false),
mEnv(env),
mSvc(svc),
mCc(cc),
mSessId(sess->getSessId()),
mSess(sess),
mMutex(m),
mAioFile(0),
mbFileReopened(false),
mBufUser(mCc,NULL),
mBytesTransfered(0),
mDataTransferStartTime(0),
mRealDataTransferStartTime(0),
mBytesSentSinceStart(0),
mOffsetInFile(0),
mDataFileState(DATA_FILE_STATE_NULL),
mOldDataFileState(DATA_FILE_STATE_NULL),
mbFileReverseWritten(false),
mbGetFileWritenDirection(false),
mTransferDelay(0),
mTransferRate(0),
mNextWakeupInterval(0),
mMaxDataSendCount(0),
mRequestByteCount(0),
mLastContentStateLogTime(0),
mDataRunnerStartTime(0),
mDataRunnerStopTime(0),
mbDataTransferComplete(true),
mbFirstChunkWait(true),
mFsWaitCount(0),
mEthMtu(1500),
mTransferState(STATE_RUNNER_IDLE),
mbConnBroken(false),
mConfWriter(NULL),
mReaderType(-1),
mbUdpSession(false),
mSubSessionRunnerId(subSessId),
mSubSessionRunnerVersion(0),
mUdpSessionRunnerScale(0.0f),
mUdpSessionRunnerStartNpt(0),
mUdpSessionRunnerStartTimeStamp(0),
mbInUdpStateTransition(false),
mbRunnerComplete(false)
{
}

SessionDataRunner::~SessionDataRunner()
{
}

void SessionDataRunner::reset( )
{
	mbConnBroken = false;
	mbRunning	= false;
	mBytesTransfered.store(0);
	mDataTransferStartTime = 0;
	mBytesSentSinceStart = 0;
	mDataFileState = DATA_FILE_STATE_NULL;
	mOldDataFileState = DATA_FILE_STATE_NULL;
	mbFileReverseWritten = false;
	mbGetFileWritenDirection = false;
	mTransferDelay = 0;
	mNextWakeupInterval = 0;
	mMaxDataSendCount = 0;
	mRequestByteCount = 0;
	mLastContentStateLogTime = 0;
	mRealDataTransferStartTime = 0;
	mDataRunnerStartTime = 0 ; 
	mDataRunnerStopTime = 0;
	mbDataTransferComplete = false;
	mbFirstChunkWait = true;
	mResponseHandler = 0;
	mFsWaitCount = 0;
	mbFileReopened = false;
	mBufUser = BufferUser( mCc, NULL);
	mTransferState = STATE_RUNNER_IDLE;
	mbRunnerComplete = false;
}

int64 SessionDataRunner::getUdpSessionRunnerNpt( int64 *dataOffset, int64 *outDataOffset) const{
	if( mUdpSessionRunnerStartTimeStamp == 0 ) {
		return mUdpSessionRunnerStartNpt;
	} 
	if( mUdpSessionRunnerState == UDPSTATE_PAUSE ) {
		return mUdpSessionRunnerStartNpt;
	} else if( mUdpSessionRunnerState == UDPSTATE_PLAY ) {
		//int64 delta = ZQ::common::now() - mUdpSessionRunnerStartTimeStamp;
		int64 delta = mBytesTransfered.load();
		if(dataOffset) {
			*dataOffset = mRequestRange.startPos + delta;
		}

		if( outDataOffset != NULL ) {
			if(!mRequestRange.bEndValid) {
				*outDataOffset = 0;
			} else {
				*outDataOffset = mRequestRange.endPos;
			}
		}

		assert(!floatEqual(mUdpSessionRunnerScale, 0.0f) );
		delta = delta * 8000 / mTransferRate * mUdpSessionRunnerScale;// npt is not relative to scale
		delta = mUdpSessionRunnerStartNpt + delta;

		MLOG.info(SESSFMT(SessionDataRunner,"getUdpSessionRunnerNpt delta [%ld]"), delta);
		return delta;
	} else {
		return -1;
	}
}

void SessionDataRunner::setTransferRate(int64 rate) {
	mTransferRate = rate;
	if(rate <= 0) {
		assert(false && "invalid bitrate");
	}
	//MLOG.info(SESSFMT(SessionDataRunner,"set transfer rate to [%ld]bps"), rate);
}

int64 SessionDataRunner::bytesToSend( int64 maxBytes ) const
{
	if( maxBytes > 0 )
		return maxBytes;
	if( mConfWriter->mPacketsPerSend > 0 )
		return mConfWriter->mPacketsPerSend * ( mConfWriter->mSendPacketSize );
	return mConfWriter->ioBlockSize * mConfWriter->readBufferCount;
}

class processTimeRecord
{
public:
	processTimeRecord(DataRunnerStatistic& statistics)
	:mStatistics(statistics)
	{
		mWatch.start();
	}
	~processTimeRecord()
	{
		mStatistics.updateOneRunStat( mWatch.stop() );		
	}
private:
	DataRunnerStatistic& mStatistics;
	StopWatch			mWatch;
};


bool SessionDataRunner::prepareRunnerBuffer( BufferUser& bufUser, int64& bytesPrepared )
{
	StopWatch sw;sw.start();
	bufUser = mAioFile->read(mOffsetInFile);
	if( !bufUser.valid() )
	{
		MLOG(ZQ::common::Log::L_ERROR, SESSFMT(SessionDataRunner,"SessionDataRunner(), got empty bufferUser"));
		return false;
	}

	MLOG.debug( SESSFMT(SessionDataRunner,"prepareRunnerBuffer() read at [%s:%ld/%zu] mSize[%zu] dataleft[%zu] bufReqId[%ld], cost[%ld]us"),
			mRequestFileName.c_str(), mOffsetInFile, bufUser.tell(), bufUser.dataSize(), bufUser.dataLeft(),bufUser.bufReqId(), sw.stop() );

	return true;
}

bool SessionDataRunner::dataProcessing(int32& dataSendCount)
{
	if( mRealDataTransferStartTime == 0 ) {
		mDataTransferStartTime	= (int64)((int64)ZQ::common::now() + (int64)mTransferDelay ); 
		mRealDataTransferStartTime = mDataTransferStartTime;
	}

	if( mbUdpSession &&  mUdpSessionRunnerStartTimeStamp == 0 ) {
		mUdpSessionRunnerStartTimeStamp = ZQ::common::now();
	}

	dataSendCount = 0;
	StopWatch watch(false);

	StopWatch watchTotal;
	
	int64 sendingBytes = 0;

	watchTotal.start();	
	StopWatch s1,s2,s3,s4;
	size_t runcount = 0 ;

	//BufferUser		bufUser(mBufUser);
	//MLOG.debug(SESSFMT(SessionDataRunner,"start pumping data"));

	while( mbRunning)
	{
		//first we need know how many bytes we should send so far
		sendingBytes = bytesToSend( );

				
		if( ( mRequestByteCount > 0 ) && ( sendingBytes > (mRequestByteCount - (int64)mBytesTransfered.load() ) ) )
		{
			sendingBytes = mRequestByteCount - (int64)mBytesTransfered.load();
		}

		while( sendingBytes > 0 && mbRunning )
		{
			if( !mBufUser.valid() || mBufUser.dataLeft() <= 0 || !dataEnoughToRun(mBufUser) )
			{
				int64 bytesPrepared = 0;
				s1.start();
				//now bytesPrepared is not available
				bool bOK = prepareRunnerBuffer( mBufUser, bytesPrepared );
				s1.stop();
				if( !bOK )
				{
					if( mEnv.getConfig().diskCache.diskCacheEnabled ) {
						MLOG(ZQ::common::Log::L_ERROR, SESSFMT(SessionDataRunner,"run(), empty bufferUser returned with DiskCache enabled, session aborted"));
						return false;
					}

					mFsWaitCount ++;
					if( mFsWaitCount > mConfWriter->mFsDataWaitCount )
					{
						MLOG(ZQ::common::Log::L_ERROR,SESSFMT(SessionDataRunner,"run() no more data read from file[%s] retried [%d], quit reading"),
							mRequestFileName.c_str(), mFsWaitCount);
						return false;
					}
					int64 interval = 50 + rand() % 50;
					MLOG.info(SESSFMT(SessionDataRunner,"run() no data read from file[%s] retried [%d], updateTimer[%ld]"),
						mRequestFileName.c_str(), mFsWaitCount, interval);
					mTransferState = STATE_RUNNER_TRANSFER;
					mSess->updateAsyncTimer(interval);
					goto EXITLOOP;
				}
				assert(mBufUser.valid());

				size_t waitingByte=0;
				if( mRequestFileName.find("index") != std::string::npos ) {
					waitingByte = 100;
				} else {
					waitingByte = sendingBytes * 3;
				}
				if( mBufUser.asyncWait( mSess->getDataReadCb(), waitingByte) ) {
					MLOG.debug( SESSFMT(SessionDataRunner,"run() data not available, waiting for it"));
					mLastReadWaitTimeStamp = ZQ::common::now();
					mTransferState = STATE_RUNNER_WAITTOREAD;
					goto EXITLOOP;
				}

				bytesPrepared = mBufUser.dataLeft();
				MLOG.debug( SESSFMT(SessionDataRunner, "run() buf[%ld] pos[%zu] dataLeft[%zu] waiting[%zu]"),
						mBufUser.reqId(), mBufUser.tell(), mBufUser.dataLeft(), waitingByte);
				mFsWaitCount = 0;
				if( bytesPrepared == 0 || !dataEnoughToRun(mBufUser) )
				{
					bool eagain = false;
					s2.start();
					bOK = reachFileEnd( eagain);
					s2.stop();
					if( eagain ) {
						MLOG.debug(SESSFMT(SessionDataRunner, "EAGAIN in reachFileEnd detection, wait for file data"));
						goto EXITLOOP;
					}

					if( bOK )
					{
					MLOG.info( SESSFMT(SessionDataRunner,"reach file end at [%ld]"), mOffsetInFile);
						return false;
					}
					else
					{
						if( mResponseHandler->isConnectionBroken() )
						{
							mbDataTransferComplete = false;
							MLOG(ZQ::common::Log::L_WARNING,SESSFMT(SessionDataRunner, "run() connection[%ld] broken, data transfer terminated"),
								mConnectionId );
							return false;
						}

						static int32 maxIdleCountAtFileEnd =  mConfWriter->mMaxIdleCountAtFileEnd;
						if( maxIdleCountAtFileEnd > 0 )
						{
							if( mIdleCountAtFileEnd > maxIdleCountAtFileEnd ) {
								MLOG(ZQ::common::Log::L_WARNING,SESSFMT(SessionDataRunner,"run() reach the max idle count [%d] at the end of file[%s]"),
									mIdleCountAtFileEnd , mRequestFileName.c_str() );
								return false;
							}
							else if( !mbFileReopened && mIdleCountAtFileEnd >= maxIdleCountAtFileEnd/2 ) {
								mAioFile->close();
								mAioFile = NULL;
								const std::string& reqFileName =  mRequestFileName;
								mAioFile = mCc.open( reqFileName, mReaderType, mSessId);
								if(!mAioFile )
								{
									MLOG(ZQ::common::Log::L_ERROR,SESSFMT(SessionDataRunner,"failed to re-open file[%s]"),reqFileName.c_str());
									return false;
								}
								mAioFile->setBitrate(mTransferRate);
								mbFileReopened = true;
								MLOG.info( SESSFMT(SessionDataRunner,"file[%s] re-opened at idle count[%d]"), reqFileName.c_str(), mIdleCountAtFileEnd );
							}
						} 

						++ mIdleCountAtFileEnd; //increase idle count if we are at the end of a file

						mDataTransferStartTime	= ZQ::common::now(); //reset start time to now
						mBytesSentSinceStart	= 0;
						//schedule current session in idle intervale

						mTransferState = STATE_RUNNER_TRANSFER;
						mSess->updateAsyncTimer( mConfWriter->minIdleInterval+ rand()%50 );
						goto EXITLOOP;
					}
				}
				else
				{
					mIdleCountAtFileEnd = 0;					
				}
			}
			
			sendingBytes = MIN( sendingBytes , (int64)mBufUser.dataLeft() );
			if(mbUdpSession && sendingBytes < 188) {
				MLOG.info( SESSFMT(SessionDataRunner, "insufficient udp data, wait a while and send again"));
				break;
			}

			dataSendCount++;
			MLOG.debug( SESSFMT(SessionDataRunner, "dataProcessing dataSendCount[%d]"),dataSendCount);
			watch.start();
			int64 sentBytes = 0;
			s3.start();
			int64 rc = transferData( mBufUser, sendingBytes ,sentBytes ) ;
			//mBufUser = bufUser;//keep track of sending data position
			assert(mBufUser.valid());
			runcount ++;
			s3.stop();
			if( rc < 0 )
			{
				if( rc == LibAsync::ERR_EAGAIN ) {
					//should I also register a timer here ?
					MLOG.info( SESSFMT(SessionDataRunner, "socket buffer may be full, wait for at least [%d]ms"), mConfWriter->mSocketKernelSndTimeo );
					mTransferState = STATE_RUNNER_WAITTOWRITE;
					bool bRegistered = mResponseHandler->registerWrite( mSess->getWritableCb() );
					assert( bRegistered );
					mLastNonwritableTimeStamp = ZQ::common::now();
					mSess->updateAsyncTimer( mConfWriter->mSocketKernelSndTimeo );
					return true;
				} else {
					MLOG(ZQ::common::Log::L_WARNING,SESSFMT(SessionDataRunner,"failed to transfer data, err:%ld"), rc);
					return false;
				}
			}
			watch.stop();

			// these code should be modified
			if( watch.span() >= (mConfWriter->mSocketSendMonitorThreashold * 1000) )
			{
				MLOG.debug(SESSFMT(SessionDataRunner,"run() took [%lu]ms to send [%ld/%ld]bytes data to client[%s/%ld]"), 
					watch.span()/1000 , sendingBytes,sentBytes, mSess->getTransferClientAddress().c_str() , mConnectionId  );

				mDataTransferStartTime = ZQ::common::now();
				mBytesSentSinceStart = sentBytes;
			}
			mStatistics.updateSendDataStat( watch.span() );
			if(rc == 0) {
				break; // break here to check sessionEnd of fileEnd
			}
		}

		if( sessionEndReached() )
		{//
			mStatistics.updateOneProcessCountData( dataSendCount );	
			MLOG.info(SESSFMT(C2SessionDataRunner,"reach session end, quiting"));
			return false;
		}

		if( !mbRunning )
		{
			mStatistics.updateOneProcessCountData( dataSendCount );
			MLOG.debug(SESSFMT(SessionDataRunner,"run() session data transfer was aborted"));
			return false;
		}

		if( !mBufUser.valid() && mbTmpReachFileEnd )
		{//all data read from FS have been transfered
			bool eagain = false;
			s2.start();
			bool bOK = reachFileEnd( eagain );
			s2.stop();
			if(eagain) {
				goto EXITLOOP;
			}
			if( bOK) {
				MLOG.info(SESSFMT(SessionDataRunner,"run() reached file end, runner stopped"));
				return false;//reach file end, terminate current session runner
			}
		}

		int64 now = ZQ::common::now();
		int64 transferRate = mTransferRate;
		transferRate = MAX( 1, transferRate);
		int64 calcTime = mDataTransferStartTime + mBytesSentSinceStart * 8000 / transferRate; 
		
		if(  dataSendCount >= (int32)mConfWriter->maxRoundPerRun )
		{
			static int32 minYieldInterval = mConfWriter->minYieldInterval;
//			MLOG.debug(SESSFMT(SessionDataRunner,"run() yield due to reach maxRoundPerRun[%d] per TransferDelay"),mConfWriter->maxRoundPerRun);

			mTransferState = STATE_RUNNER_TRANSFER;
			mSess->updateAsyncTimer( minYieldInterval);
			goto EXITLOOP;
		}

		if( now >= (calcTime ))
		{
			continue;
		}
		int64 timetoSleep = calcTime - now ;
		if( timetoSleep < mConfWriter->udpSessMinStreamInterval )
			continue;
		
		mStatistics.updateOneProcessCountData( dataSendCount );
		
		mTransferState = STATE_RUNNER_TRANSFER;
		MLOG.info(SESSFMT(SessionDataRunner,"dataprocessing updateAsyncTimer[%lld]"),timetoSleep);
		mSess->updateAsyncTimer( timetoSleep );
		break;
	}//while(true)

EXITLOOP:
	watchTotal.stop();
	MLOG.debug( SESSFMT(SessionDataRunner,"dataProcessing runcount[%d] cost[%lu/%lu/%lu/%lu]us total[%lu]us, current bufReqId[%ld] current dataOffset[%ld] timer[%ld/%lu]"),
			(int)runcount, s1.span(), s2.span(), s3.span(),s4.span(), watchTotal.span(), mBufUser.reqId(), mOffsetInFile, ZQ::common::now(), mSess->asyncTimerTarget());
	return mbRunning;
}



bool SessionDataRunner::run( int64 timerShift )
{
	MLOG.debug(SESSFMT(SessionDataRunner,"run() start a dataRunner"));
	ZQ::common::MutexGuard gd(mMutex);
	if( !mbRunning  || mbConnBroken)
	{
		MLOG.info(SESSFMT(SessionDataRunner,"run() should stop data runner"));
		stopRunner();
		return false;
	}
	bool bRet = false;
	{
		mStatistics.updateTimerShift(timerShift*1000);//we use microsecond
		processTimeRecord ptrecord(mStatistics);
		int32 dataSendCount = 0;
		StopWatch sw;sw.start();
		bRet = dataProcessing(dataSendCount);
		MLOG.debug(SESSFMT(SessionDataRunner,"run() datProcessing[%d]"),dataSendCount);
		sw.stop();
		if( sw.span() > 10 * 1000 ) {
			MLOG.debug(SESSFMT(dataProcessing,"took [%lu]ms to complete dataProcessing "),sw.span()/1000);
		}
		if( dataSendCount > 0 )
			mStatistics.updateOneProcessCountData( dataSendCount );
	}
	if( ( !bRet ) || mbConnBroken)
	{
		MLOG.info(SESSFMT(dataProcessing,"run() dataProcessing indicate stop runner, or connection broken[%s]"), mbConnBroken?"true":"false");
		mbRunnerComplete = true;
		stopRunner();
	}
	return bRet;
}

int64 SessionDataRunner::transferData( BufferUser& bufUser, int64& dataBytesToSend , int64& sentBytes )
{
	if( dataBytesToSend <= 0 )
	{//never be here
		dataBytesToSend = 0;
		return 0; // nothing to send
	}
	assert( bufUser.valid() );

	int64 availBytes = bufUser.dataLeft();

	//calculate how many byte should be sent out	
	if( ( mRequestByteCount > 0 ) && ( dataBytesToSend > (mRequestByteCount - (int64)mBytesTransfered.load() ) ) )
	{
		sentBytes = mRequestByteCount - (int64)mBytesTransfered.load();
	}
	else
	{
		sentBytes = dataBytesToSend;
	}
	sentBytes = MIN(sentBytes , availBytes);

	// if this is a udp session runner and requestByteCount <= 0
	// we should detect where the pes end is
	// NOTE: if mRequestByteCount > 0, we already know where we should stop at
	if (mbInUdpStateTransition){
		int64 pesEnd = -1;
		if(mbUdpSession) {
			int64 dataOffset = bufUser.offsetInFile() + bufUser.tell();
			pesEnd = findUdpSessionStopPoint(bufUser);
		}

		if(pesEnd > mOffsetInFile) {
			int64 delta = pesEnd - mOffsetInFile + 1;
			sentBytes = MIN(sentBytes, delta);
		}
	}

	if(mbUdpSession) {
		sentBytes = sentBytes / 188 * 188;
	}

	int rc = 0;

	StopWatch sendSw; sendSw.start();
	int sendCount = 0;
	if( sentBytes > 0 )
	{
		static uint32 packetSize = mConfWriter->mSendPacketSize;

		const char* pData = bufUser.data();
		int64 tmpSendByte = sentBytes;

		while( tmpSendByte > 0 )
		{
			int64 dataSize = 0 ;
			if( tmpSendByte > packetSize )
			{
				dataSize = packetSize;
			}
			else
			{
				dataSize = tmpSendByte;
			}
			if(mbUdpSession && (pData[0] != 0x47)) {
				MLOG.debug(SESSFMT(SessionDataRunner, "transferData() bad content at [%ld/%zu/%ld]"), mOffsetInFile, bufUser.tell(), (int64)(pData -bufUser.data()) );
			}
resend:
			rc = mResponseHandler->addBodyContent( pData , dataSize );
			sendCount ++;
			assert( rc <= dataSize );

			if( rc <= 0 ) {
				//failed to transfer data to peer , should I close this session ?
				if( rc == LibAsync::ERR_BUFFERTOOBIG || rc == 0 ) {
					if( rc == 0 ) {
						rc = LibAsync::ERR_EAGAIN;
					} else {
						if(!mbUdpSession) {
							dataSize /= 2;//try smaller data size
							if( dataSize <= 1) {
								rc = LibAsync::ERR_EAGAIN;
							} else {
								goto resend;
							}
						} else {
							rc = LibAsync::ERR_EAGAIN;
						}
					}
				} else if ( rc == LibAsync::ERR_EAGAIN ) {
					break;
				} else if( rc != 0 ) {
					MLOG(ZQ::common::Log::L_ERROR, SESSFMT(SessionDataRunner,"transferData() failed to transfer data through connection[%ld], %s:%s(%d)"),
							mConnectionId, mResponseHandler->lastError().c_str(), strerror(errno), errno);
				} 
				break;
			} else if( rc > 0 ) {
				pData += rc;
				tmpSendByte -= rc;
			}
		}
		sentBytes = (int64)( pData - bufUser.data() );
	}
	
	sendSw.stop();
	if(rc <= 0) {
		MLOG.debug( SESSFMT(SessionDataRunner,"transferData(),  loop[%d] data sent[%ld]bytes, cost[%lu]us, lastrc[%d], bufUserOffset[%zu]"), 
				sendCount, sentBytes, sendSw.span(), rc, bufUser.tell() );
	}

	mBytesTransfered		+= sentBytes;
	mBytesSentSinceStart	+= sentBytes;
	dataBytesToSend			-= sentBytes;
	mOffsetInFile			+= sentBytes;
	bufUser.advance(sentBytes);

	return rc;
}

int64 SessionDataRunner::findUdpSessionStopPoint(BufferUser& bu) {
	if(mUdpTsFrameStat.startPos < 0 || mOffsetInFile  >= mUdpTsFrameStat.startPos) {
		//we need to update current frame stat
		MLOG.debug(SESSFMT(findNextStopPoint, "parsing from %zu, fileOffset[%ld]"), bu.tell(), mOffsetInFile);
		mSess->parseTsPacket((unsigned char*)bu.data(), bu.dataLeft(), mUdpTsFrameStat);
		if(mUdpTsFrameStat.startPos >= 0 && mEnv.getConfig().LastFramePos != 3){
			mUdpTsFrameStat.startPos += (mOffsetInFile);//adjust so that we can compare it to file data offset
		}
	}
	//if we are in transition state, we should find where we can stop at
	if(mbInUdpStateTransition && mRequestByteCount <=0 ) {
		if( mOffsetInFile <= mUdpTsFrameStat.startPos ){
			mRequestByteCount = mUdpTsFrameStat.startPos - mRequestRange.startPos;
			MLOG.debug( SESSFMT(SessionDataRunner,"findUdpSessionStopPoint() after modify RequestByteCount[%d]"),mRequestByteCount);
			mRequestRange.bEndValid = true;
		}
		MLOG.debug(SESSFMT(findNextStopPoint, "currentOffset[%ld] pesEnd[%ld], inTransition[%s] requestByteCount[%ld]"),
				mOffsetInFile, mUdpTsFrameStat.startPos, mbInUdpStateTransition?"true":"false", mRequestByteCount	);
	}
	return mUdpTsFrameStat.startPos;
}

bool SessionDataRunner::tryStopRunner() {
	if(!mbUdpSession || mbRunnerComplete ) {
		return true; // we can stop the runner directly if this is not a udp session runner
	}
	if(mbInUdpStateTransition) {
		MLOG.debug( SESSFMT(SessionDataRunner,"tryStopRunner() InUdpStatransition requestbytecount[%d] load[%ld]"), mRequestByteCount,(int64)mBytesTransfered.load());
		return mRequestByteCount > 0 && (int64)mBytesTransfered.load() >= mRequestByteCount;
	}
	mbInUdpStateTransition = true;
	MLOG.debug( SESSFMT(SessionDataRunner,"tryStopRunner() RequestByteCount[%d]"),mRequestByteCount);
	return false;
}

void SessionDataRunner::stopRunner( bool bDestroied )
{
	ZQ::common::MutexGuard gd(mMutex);
	if( !mbRunning ) {
		mSess = NULL;
		MLOG.debug( SESSFMT(SessionDataRunner,"stopRunner() data runner is not running"));
		return;
	}

	if(!tryStopRunner()) {
		MLOG( ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner, "tryStopRunner() return") );
		return;
	}

	MLOG( ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner, "stopRunner() stopping data runner") );
	mbRunning = false;
	mSvc.unwatchSession( mSess );
	mSess->cancelAsyncTimer();
	mSess->cancelStatTimer();
	transferComplete();
	if( mAioFile )
	{
		mAioFile->close();
		mBufUser = BufferUser(mCc,NULL);
		mAioFile = 0;
	}
	mSess->changeState( SESSION_STATE_IDLE , true );

	MLOG( ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner, "stopRunner() data runner stopped") );
	
	mStatistics.updateSessionStatus( mBytesTransfered.load() , mRequestRange.startPos , 
									mRequestRange.startPos + mBytesTransfered.load() , 
									mDataRunnerStopTime -  mRealDataTransferStartTime );
	MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner, "runner destroyed, file[%s], STAT: %s"), mRequestFileName.c_str(), mStatistics.toString().c_str());
	if(  !bDestroied && mSess &&  BASE_SESSION_TYPE(mSess->getSessionType()) != SESSION_TYPE_COMPLEX )
	{
		//mSess->updateTimer(0);
	}
	SessLogMgr& sesslogmgr = mSvc.getSessLogMgr();
	sesslogmgr.EraseSess(mSessId);
	mSess->onDataRunnerStopped(mSubSessionRunnerId, mSubSessionRunnerVersion);
}



bool SessionDataRunner::startRunner( C2ResponseHandler::Ptr response, C2SessionPtr sess )
{
	if( mAioFile )
		mAioFile->close();
	mSess = sess;

	//fixup file path for session

	//int sndBufSize = (int)(mConfWriter->readBufferCount*mConfWriter->ioBlockSize*2 );
	int sndBufSize = mConfWriter->mSocketKernelSndBuf;
	if(sndBufSize < 32*1024)
		sndBufSize = 32*1024;

	MLOG.debug( SESSFMT(SessionDataRunner,"startRunner() trying to start runner: file[%s] offset[%ld] , sendPacketSize[%d] , sendPacketCount[%d] fsReadBufSize[%u], kernelSndBuf[%d]"),
		mRequestFileName.c_str() , mRequestRange.bStartValid ? mRequestRange.startPos : 0 , mEthMtu ,
		mConfWriter->mPacketsPerSend, mConfWriter->readBufferCount * mConfWriter->ioBlockSize , sndBufSize);
	

	// since transfer delay may be negative value, so we must make sure that tick count is a large enough value
	mDataTransferStartTime	= (int64)((int64)ZQ::common::now() + (int64)mTransferDelay ); 
	mRealDataTransferStartTime = mDataTransferStartTime;
	
	ZQ::common::MutexGuard gd(mMutex);
	assert( response != 0 );
	mAioFile = mCc.open( mRequestFileName, mReaderType, mSessId);
	if(!mAioFile)
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(startRunner,"failed to open file[%s]"), mRequestFileName.c_str());
		return false;
	}

	if( mRequestRange.bStartValid )
	{
		mOffsetInFile = mRequestRange.startPos;
	}

	if( mRequestRange.bEndValid )
	{
		mRequestByteCount = mRequestRange.endPos - ( mRequestRange.bStartValid ? mRequestRange.startPos : 0 ) + 1;
		mRequestByteCount = mRequestByteCount < 0 ? 0 : mRequestByteCount;
		MLOG.debug( SESSFMT(SessionDataRunner,"startRunner() request data bytes [%ld]"), mRequestByteCount);
	}
	mStatistics.updateRequestedBitrate(mTransferRate);
	mAioFile->setBitrate(mTransferRate);

	response->setCommOption( ZQHttp_OPT_WriteBufSize, sndBufSize );
	
	//response->setCommOption( ZQHttp_OPT_sendTimeo, mConfWriter->mSocketKernelSndTimeo);

	mResponseHandler			= response;

	//if(!mEnv.getConfig().diskCache.diskCacheEnabled ) {
		mBufUser = mAioFile->read(mOffsetInFile, mTransferRate); //kick off a read request, no matter how many byte do we want
	//}

	mbRunning = true;	
	mConnectionId = mResponseHandler->getConnectionId();

	MLOG.info( SESSFMT(SessionDataRunner,"startRunner() submitted reading file[%s] from offset[%ld], sendPacketSize[%u], sendPacketCount[%u] fsReadBufSize[%u], kernelSndBuf[%d] bufReqId[%ld]"),
		mRequestFileName.c_str() , mRequestRange.bStartValid ? mRequestRange.startPos : 0 , mEthMtu ,
		mConfWriter->mPacketsPerSend, mConfWriter->readBufferCount * mConfWriter->ioBlockSize , sndBufSize,
		mBufUser.bufReqId()	) ;
	return true;
}

int64 SessionDataRunner::getBytesTransfered() const {
	return mBytesTransfered.load();
}

bool SessionDataRunner::prepareUdpRunner( ) {
	mbUdpSession =true;
	//FIXME: not implemented
	if( mAioFile) {
		mAioFile->close();
	}

	MLOG.debug( SESSFMT(SessionDataRunner,"prepareUdpRunner() trying to start runner: file[%s] offset[%ld] , sendPacketSize[%d] , sendPacketCount[%d] fsReadBufSize[%u]"),
		mRequestFileName.c_str() , mRequestRange.bStartValid ? mRequestRange.startPos : 0 , mEthMtu ,
		mConfWriter->mPacketsPerSend, mConfWriter->readBufferCount * mConfWriter->ioBlockSize );

	// since transfer delay may be negative value, so we must make sure that tick count is a large enough value
	//mDataTransferStartTime	= (int64)((int64)ZQ::common::now() + (int64)mTransferDelay ); 
	//mRealDataTransferStartTime = mDataTransferStartTime;
	
	ZQ::common::MutexGuard gd(mMutex);
	assert( mResponseHandler != NULL );
	mAioFile = mCc.open( mRequestFileName, mReaderType, mSessId);
	if(!mAioFile)
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(startRunner,"failed to open file[%s]"), mRequestFileName.c_str());
		return false;
	}

	if( mRequestRange.bStartValid )
	{
		mOffsetInFile = mRequestRange.startPos;
	}

	if( mRequestRange.bEndValid )
	{
		mRequestByteCount = mRequestRange.endPos - ( mRequestRange.bStartValid ? mRequestRange.startPos : 0 ) + 1;
		mRequestByteCount = mRequestByteCount < 0 ? 0 : mRequestByteCount;
		MLOG.debug( SESSFMT(SessionDataRunner,"prepareUdpRunner() request data bytes [%ld]"), mRequestByteCount);
	}
	mStatistics.updateRequestedBitrate(mTransferRate);
	mAioFile->setBitrate(mTransferRate);
	
	//mBufUser = mAioFile->read(mOffsetInFile); //kick off a read request, no matter how many byte do we want
	int64 bytesGotten = 0;
	prepareRunnerBuffer(mBufUser, bytesGotten);

	mbRunning = true;	
	mConnectionId = mResponseHandler->getConnectionId();

    int sndbufsize = mConfWriter->mSocketKernelSndBuf;
	if(sndbufsize< 64*1024)
		sndbufsize = 64*1024;
	mResponseHandler->setCommOption(ZQHttp_OPT_WriteBufSize, sndbufsize);
	mResponseHandler->setCommOption(ZQHttp_OPT_TTL, 32);//set ttl

	MLOG.info( SESSFMT(SessionDataRunner,"prepareUdpRunner() submitted reading file[%s] from offset[%ld], bitrate[%ld] sendPacketSize[%u], sendPacketCount[%u] fsReadBufSize[%u], bufReqId[%ld]"),
			mRequestFileName.c_str() , mRequestRange.bStartValid ? mRequestRange.startPos : 0 ,
			mTransferRate, mEthMtu ,
			mConfWriter->mPacketsPerSend, mConfWriter->readBufferCount * mConfWriter->ioBlockSize,
			mBufUser.bufReqId()) ;
	return true;
}

void SessionDataRunner::transferComplete()
{
	ZQ::common::MutexGuard gd(mMutex);
	mDataRunnerStopTime = ZQ::common::now();
	if( mResponseHandler )
	{		
		int64 delta = mDataRunnerStopTime - mRealDataTransferStartTime;
		int64 avgRate = delta > 0 ? (mBytesTransfered.load() * 8000 / delta) : 0 ;
		//mResponseHandler->addBodyContent( EndOfChunkData.c_str() , EndOfChunkData.length() );
		if(!mResponseHandler->complete() ) {
			MLOG(ZQ::common::Log::L_WARNING, SESSFMT(SessionDataRunner,"failed to send 0 size chunk data to client"));
		}
		mResponseHandler = NULL; // release response object
		MLOG.info(SESSFMT(SessionDataRunner,"transferComplete() transfer stat: request range[%s] , bytetransfered[%lu] avgBitrate[%ld]"),
			 mRequestRange.toString().c_str() , mBytesTransfered.load() , avgRate);
	}
}

bool SessionDataRunner::sessionEndReached( ) const
{
	if( !mRequestRange.bEndValid )
		return false; // we must reach the end of the file to complete the session transfer
	if( mRequestByteCount > 0 &&  (int64)mBytesTransfered.load() >= mRequestByteCount )
	{
		MLOG.debug( SESSFMT(SessionDataRunner,"reached session end: request[%s], byteTransferred[%lu]"),
			 mRequestRange.toString().c_str() , mBytesTransfered.load() );
		return true;
	}
	else
	{
		return false;
	}
}

class ReachFileEndAssetQueryCB : public LibAsync::AsyncWork, public IAsyncNotifySinker {
public:
	typedef ZQ::common::Pointer<ReachFileEndAssetQueryCB>  Ptr;
	ReachFileEndAssetQueryCB( C2SessionPtr sess, AssetAttribute::Ptr attr, LibAsync::EventLoop* loop) 
		:LibAsync::AsyncWork(*loop),
		mC2Session(sess),
		mAssetAttr(attr) {
	}
	virtual ~ReachFileEndAssetQueryCB() {
	}
private:
	virtual void onNotified() {
		queueWork();
	}
	virtual void onAsyncWork() {
		mC2Session->postReachFileEnd( mAssetAttr );
	}
private:
	C2SessionPtr			mC2Session;
	AssetAttribute::Ptr 	mAssetAttr;
};

bool SessionDataRunner::postReachFileEnd( AssetAttribute::Ptr attr, bool directCall ) {
	if( attr->lastError() != 0 && !attr->pwe() ) {
		MLOG.info(SESSFMT(SessionDataRunner,"asset for [%s] attribute: lastError[%d]. attr bufReqId[%ld] take it as end-of-file"),
				mRequestFileName.c_str(),	attr->lastError(), attr->reqId() );
		if(mSess)	
			mSess->destroy();
		return true;
	}

	if( attr->pwe() ) {
		if( ( mRequestFileName.find(".index") != std::string::npos)&& ( mReaderType == CLIENT_TYPE_C2CLIENT) ){
			MLOG.info(SESSFMT(SessionDataRunner,"asset for[%s] attribute: pwe[%s], but this is a index file, just take it as end-of-file"),
					mRequestFileName.c_str(), attr->pwe() ? "true": "false");
			mAioFile->invalidateCache( mOffsetInFile );
			if(mSess)
				mSess->destroy();
			return true;
		}
		mDataFileState = DATA_FILE_STATE_WRITE;
		mOldDataFileState = mDataFileState;
		MLOG.info(SESSFMT(SessionDataRunner,"asset for[%s] attribute: pwe[%s] . continue to wait data"),
				mRequestFileName.c_str(), attr->pwe() ? "true": "false");
		mAioFile->invalidateCache( mOffsetInFile );
		if(!directCall)
			run(0);
		else
			return false;
	} else {
		mDataFileState = DATA_FILE_STATE_STABLE;
		if( mOldDataFileState == DATA_FILE_STATE_WRITE ) {
			MLOG.info( SESSFMT(SessionDataRunner,"asset for[%s]: changed from pwe to non-pwe, just continue to read data"), mRequestFileName.c_str() );

			mOldDataFileState = mDataFileState;
			mAioFile->invalidateCache( mOffsetInFile );
			if(!directCall)
				run(0);
			else
				return false;
		}
	}

	MLOG.info( SESSFMT(SessionDataRunner,"asset for [%s], attribute: inprogress[%s], pwe[%s] lastError[%d]. take it as end-of-file"),
			mRequestFileName.c_str(), attr->inprogress() ? "true":"false" , attr->pwe() ? "true":"false", attr->lastError());
	if(mSess)
		mSess->destroy();
	if(directCall)
		return true;
	return false;
}

bool SessionDataRunner::reachFileEnd( bool& eagain) {
	if( mReaderType!= CLIENT_TYPE_DISKAIO ) {
		if(!mSessProperty.queryIndex) {
			MLOG.info( SESSFMT(SessionDataRunner,"queryIndex is [%s], treate session as reach-file-end"), mSessProperty.queryIndex? "true":"false");
			return true;
		}
		C2SessionManager& sessManager = mSvc.getSessManager();
		AssetAttribute::Ptr attr = sessManager.getAssetAttribute( mSessId, mRequestFileName, mReaderType);
		assert(attr != NULL);
		
		bool inprgs = attr->inprogress();
		if( inprgs ) {
			mDataFileState = DATA_FILE_STATE_WRITE;
			mOldDataFileState = mDataFileState;
			MLOG.info(SESSFMT(SessionDataRunner,"asset for[%s], attribute: inprogress[%s]. attr reqId[%ld] continue to wait data"),
					mRequestFileName.c_str(), inprgs ? "true": "false", attr->reqId() );
			return false;
		} else {
			ReachFileEndAssetQueryCB::Ptr queryCb = new ReachFileEndAssetQueryCB(mSess, attr, mResponseHandler->getLoop() ) ;
			if( attr->asyncWait( queryCb )) {
				eagain = true;
				return false;
			}
			return postReachFileEnd(attr, true);
		}
		return true;
	}

	//FIXME: check if really reach the end
	//       in C2FE and StreamSegmenter, these codes are non-reachable
	bool bFileSteady = true;
	if( DATA_FILE_STATE_STABLE != mDataFileState )
	{
		if(!mbGetFileWritenDirection)
		{
			mbGetFileWritenDirection = true;
			
			//C2SessFile c2f(	mSess->mRequestFileName , mEnv , mSessId);
			struct stat st;
			memset( &st,0,sizeof(st) );
			std::string fullPathname =  fsConcatPath( mEnv.getDocumentRootFolder() , mRequestFileName );
			if( stat(fullPathname.c_str() , &st) != 0 )
			{
				MLOG(ZQ::common::Log::L_ERROR,SESSFMT(SessionDataRunner,"failed to get stat of file[%s]"), fullPathname.c_str() );
				mbFileReverseWritten = false;
			}
			else
			{
				mbFileReverseWritten = (int64)st.st_size >=  (20LL * GIGABYTES);
			}

			//treat it as sparse file if the data end pos > 20G
			//mbFileReverseWritten = c2f.dataEndPos() > ( 20LL * GIGABYTES );  //iBridge->isFileReverseWritten( mSess->mRequestFileName , mSessId );
			MLOG.debug(SESSFMT(SessionDataRunner,"reachFileEnd() detect file[%s] endpos[%ld], reverse write[%s]"), mRequestFileName.c_str(), (int64)(st.st_size) , mbFileReverseWritten ?"true":"false");

			if( mbFileReverseWritten)
			{
				mDataFileState = DATA_FILE_STATE_STABLE;//because sparse file is written from tail to head, if a session read the
														//file data from a position to the end, we should think that there is no more
														//data need to be read. so just terminate the session by identif that we already
														//get to the file end
				MLOG.info(SESSFMT(SessionDataRunner,"reachFileEnd() [%s] is a sparse file, treated as file end reached"),
					mRequestFileName.c_str() );
				return true;
			}
		}
		IAttrBridge* iBridge = mEnv.getAttrBridge();
		if(iBridge)
		{			
			bFileSteady = !iBridge->isFileBeingWritten( mRequestFileName , mSessId );
		}
		else
		{		
			bFileSteady = true;
		}
		mDataFileState = bFileSteady ? DATA_FILE_STATE_STABLE : DATA_FILE_STATE_WRITE;
		int64 cur = ZQ::common::now();
		if( mLastContentStateLogTime <= 0 || (cur - mLastContentStateLogTime) > 60 * 1000  || bFileSteady )
		{
			mLastContentStateLogTime = cur;

			if (bFileSteady)
			{
				MLOG.info(SESSFMT(SessionDataRunner,"reachFileEnd() content[%s] is %s being written, communicator[%ld]"),
					mRequestFileName.c_str(), bFileSteady ? "NOT" : "" ,mResponseHandler->getConnectionId() );
			}
			else
			{

				std::string fullPathname =  fsConcatPath( mEnv.getDocumentRootFolder() , mRequestFileName );
				FILE* f = fopen(fullPathname.c_str(), "rb");
				int64 oSize = 0, nSize = 0;
				if (NULL != f)
				{
					struct stat st;
					int fd = fileno(f);
					memset(&st, 0, sizeof(st));
					if (0 == fstat(fd, &st))
						oSize = st.st_size;

					fsync(fd);
					memset(&st, 0, sizeof(st));
					if (0 == fstat(fd, &st))
						nSize = st.st_size;

					fclose(f);
				}

				MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner, "reachFileEnd() file[%s] is being written, communicator[%ld], forced fsync() got %ld->%ld for next try"),
					mRequestFileName.c_str(), mResponseHandler->getConnectionId(), oSize, nSize);
			}
 		}
	}

	if( bFileSteady )//not being written
	{
		if( mOldDataFileState == DATA_FILE_STATE_WRITE ) {
			mAioFile->invalidateCache( mOffsetInFile); //invalidate last block
		}
		mOldDataFileState = mDataFileState;

		//only diskfile will trigger these code, so set dontWait = false
		mBufUser = mAioFile->read( mOffsetInFile);
		BufferUser bufUser = mBufUser;
		//FIXME: asyncWait here
		if( !bufUser.valid() )
		{
			MLOG.info( SESSFMT(SessionDataRunner,"reachFileEnd() can't get filled aio buffer, wait a little longer and try again"));
			return false;
		}
		if( bufUser.asyncWait( mSess->getReachFileEndCb(), 100 ) ) {
			eagain = true;
			MLOG.debug(SESSFMT(SessionDataRunner, "reachFileEnd() trying to read last file segment"));
			return false;
		}
		if( bufUser.dataLeft() <= 0  )
		{
			//we reach the file end
			MLOG.info( SESSFMT(SessionDataRunner,"reachFileEnd() reached EOF"));
			return true;
		}
	} else {
		mAioFile->invalidateCache( mOffsetInFile );
		//only diskfile will trigger these code, so set dontWait = false
		mBufUser = mAioFile->read(mOffsetInFile);
		mOldDataFileState = mDataFileState;
		MLOG.debug(SESSFMT(SessionDataRunner, "reachFileEnd() file is being written, wait for new file data"));
	}
	return false;
}

void SessionDataRunner::markAsConnBroken() {
	MLOG.info( SESSFMT(SessionDataRunner,"onConnBroken"));
	mbConnBroken = true;
	stopRunner();
}

void SessionDataRunner::onReadTimedout() {
	MLOG.error(SESSFMT(SessionDataRunner, "onReadTimedout() file[%s] offset[%ld bufIReqId[%ld]"), mRequestFileName.c_str(), mBufUser.offsetInFile(), mBufUser.bufReqId() );
}

void SessionDataRunner::onDataGotten( bool bCheckFileEnd) {
	if(!mbRunning ) {
		MLOG.info( SESSFMT(SessionDataRunner, "onDataGotten, session was stopped, just ignore the returned data"));
		return;
	}
	MLOG.debug( SESSFMT(SessionDataRunner,"onDataGotten, data arrived: file[%s] offset[%ld] bufReqId[%ld]"), 
			mRequestFileName.c_str(), mBufUser.offsetInFile(), mBufUser.bufReqId() );
	size_t bytesPrepared = mBufUser.dataLeft();
	mbTmpReachFileEnd = (bytesPrepared <= 0 );
	int64 waitTimeDelta = ZQ::common::now() - mLastReadWaitTimeStamp;
	if( waitTimeDelta >= (mConfWriter->mFsReadThreshold) ) {
		MLOG.debug(SESSFMT(SessionDataRunner,"prepareRunnerBuffer() took  [%ld/%lu]ms to get data from source, bufReqId[%ld], error[%d/%d]"),
			waitTimeDelta, mBufUser.bufferPrepareTime(),
			mBufUser.bufReqId(), mBufUser->errCategory(), mBufUser->errCode() );
		mStatistics.updateFsWaitStat( waitTimeDelta * 1000 );
	}
	int lastError = mBufUser.lastError();
	if( mbTmpReachFileEnd || lastError != 0 ) {
		MLOG.debug(SESSFMT(SessionDataRunner,"prepareRunnerBuffer() we may reach the file end at offset[%lu], or encounter an error[%d/%d] from bufReqId[%ld]"),
				mOffsetInFile, mBufUser->errCategory(), mBufUser->errCode(), mBufUser.bufReqId());
		if( bCheckFileEnd ) {
			MLOG.info(SESSFMT(SessionDataRunner,"reachFileEnd(), reached file end / encounter an error  at [%s:%ld], stop runner"), 
					mRequestFileName.c_str(), mOffsetInFile);
			mbRunnerComplete = true;
			stopRunner();
			return;
		}
	}
	mTransferState = STATE_RUNNER_TRANSFER;
	run(0);
}

void SessionDataRunner::onWritable() {
	MLOG.debug( SESSFMT(SessionDataRunner,"onWritable() continue sending data"));
	int64 timeDelta = ZQ::common::now() - mLastNonwritableTimeStamp;
	if( timeDelta >= mConfWriter->mSocketSendMonitorThreashold ) {
		MLOG.debug(SESSFMT(SessionDataRunner,"onWritable() took [%ld]ms to change from non-writable state"), timeDelta );
		mStatistics.updateSendDataStat( timeDelta * 1000 );
	}
	mTransferState = STATE_RUNNER_TRANSFER;
	if( !mSess ){
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(SessionDataRunner,"onWritable() the mSess ptr is NULL."));
		return;
	}
	mSess->cancelAsyncTimer(); 
	run(0);
}

void SessionDataRunner::onAsyncTimer() {
	//MLOG.debug( SESSFMT(SessionDataRunner,"onAsyncTimer call run()"));
	switch(mTransferState) {
	case STATE_RUNNER_WAITTOWRITE:
		{
			MLOG.info(SESSFMT(SessionDataRunner,"timed out while waiting for writing data, destroy session"));
			stopRunner();
			return;
		}
	default:
		break;
	}
	mTransferState = STATE_RUNNER_TRANSFER;
	run(0);
}

bool SessionDataRunner::dataEnoughToRun(const BufferUser& bu) {
	if(!mbUdpSession) {
		return bu.valid();
	}
	if(!bu.valid()) {
		return false;
	}
	if(bu.dataLeft() < 188) {
		return false;
	}
	return true;
}

void SessionDataRunner::reportStreamStatus()
{
	if (mbRunning){
		int64 timeoffset=0,byteoffset = 0,outdataoffset=0;
		C2Session::StatInfo info;
		timeoffset = getUdpSessionRunnerNpt(&byteoffset,&outdataoffset);
		info.byteOffset = byteoffset;
		info.duration = mStatistics.mDuration ;
		info.assetName = mRequestAssetName;
		info.timeOffset = timeoffset;
		if (mSess){
			info.duration = mSess->getSessionDuration();
			info.bitrate = mSess->getSessionBitrate(); 
			std::ostringstream sout;
			sout<<mSess->getTransferClientPort();
			info.destination = mSess->getTransferClientAddress() + ":" + sout.str();
			info.scale = mSess->getSessionScale();
			info.stat = UdpSessionStateToString(getUdpSessionRunnerState());
		}
		PortManager& portmgr = mSvc.getPortManager();
		std::string ip; 
		info.streamingPort = portmgr.getIpPort(mSessId,ip);
		info.streamingIP = ip;
		SessLogMgr& sesslogmgr = mSvc.getSessLogMgr();	
		sesslogmgr.UpdateMsg(mSessId,info);
    //	MLOG.debug( SESSFMT(SessionDataRunner,"reportStreamStatus() sessid[%s] assetName[%s] byteoffset[%ld] timeoffset[%ld] duration[%ld] bitrate[%ld] destination[%s] scale[%f] state[%s] streamingPortName[%s] streamingIp[%s]"),mSessId.c_str(),mRequestAssetName.c_str(),byteoffset,info.timeOffset, info.duration,info.bitrate,(info.destination).c_str(), info.scale, (info.stat), (info.streamingPort).c_str(),(info.streamingIP).c_str());
	}
}


UdpPauseFrameRunner::UdpPauseFrameRunner( C2StreamerEnv& env , C2Service& svc , C2SessionPtr sess, CacheCenter& cc, ZQ::common::Mutex& m, int64 subSessId )
:SessionDataRunner(env, svc, sess, cc, m, subSessId) {
	mBZMFrameSize = 0;
	mPZMFrameSize = 0;
	mbZeroMotionDataSent = false;
}


UdpPauseFrameRunner::~UdpPauseFrameRunner() {
}

void UdpPauseFrameRunner::setBZeroMotionFrame(const char* frame, size_t size) {
	if(!frame) {
		return;
	}
	mBZMFrameSize = MIN(size, sizeof(mBZeroMotionFrame));
	memcpy(mBZeroMotionFrame, frame, mBZMFrameSize);
}

void UdpPauseFrameRunner::setPZeroMotionFrame(const char* frame, size_t size) {
	if(!frame) {
		return;
	}
	mPZMFrameSize = MIN(size, sizeof(mPZeroMotionFrame));
	memcpy(mPZeroMotionFrame, frame, mPZMFrameSize);
}

int64 UdpPauseFrameRunner::transferData( BufferUser& bu, int64& dataBytesToSend, int64& sentBytes ) {
	if( dataBytesToSend <= 0 )
	{//never be here
		dataBytesToSend = 0;
		return 0; // nothing to send
	}
	assert( bu.valid() );

	if(!mbZeroMotionDataSent) {
		const char* pZeroMotionData = NULL;
		if(mPZMFrameSize > 0) {
			pZeroMotionData = &mPZeroMotionFrame[0];
		} else if(mBZMFrameSize > 0) {
			pZeroMotionData = &mBZeroMotionFrame[0];
		}
		if(pZeroMotionData) {
			mResponseHandler->addBodyContent(pZeroMotionData, 188);
			MLOG.info(SESSFMT(PauseFrameDataRunner,"transferData() zero motion data has been sent" ));
		} else {
			MLOG.info(SESSFMT(PauseFrameDataRunner,"transferData() no zero motion data is found" ));
		}
		mbZeroMotionDataSent = true;
	}


	int64 availBytes = bu.dataLeft() / 188 * 188;
	sentBytes = dataBytesToSend;
	sentBytes = MIN(sentBytes , availBytes);

	int rc = 0;

	StopWatch sendSw; sendSw.start();
	int sendCount = 0;
	if( sentBytes > 0 )
	{
		static uint32 packetSize = mConfWriter->mSendPacketSize;

		const char* pData = bu.data();
		int64 tmpSendByte = sentBytes;

		while( tmpSendByte > 0 )
		{
			int64 dataSize = 0 ;
			if( tmpSendByte > packetSize )
			{
				dataSize = packetSize;
			}
			else
			{
				dataSize = tmpSendByte;
			}
resend:
			rc = mResponseHandler->addBodyContent( pData , dataSize );
			sendCount ++;
			assert( rc <= dataSize );

			if( rc <= 0 ) {
				//failed to transfer data to peer , should I close this session ?
				if( rc == LibAsync::ERR_BUFFERTOOBIG || rc == 0 ) {
					if( rc == 0 ) {
						rc = LibAsync::ERR_EAGAIN;
					} else {
						if(!mbUdpSession) {
							dataSize /= 2;//try smaller data size
							if( dataSize <= 1) {
								rc = LibAsync::ERR_EAGAIN;
							} else {
								goto resend;
							}
						}
					}
				} else if ( rc == LibAsync::ERR_EAGAIN ) {
					break;
				} else	if( rc != 0 ) {
					MLOG(ZQ::common::Log::L_ERROR, SESSFMT(PauseFrameDataRunner,"transferData() failed to transfer data through connection[%ld], %s:%s(%d)"),
							mConnectionId, mResponseHandler->lastError().c_str(), strerror(errno), errno);
				} 
				break;
			} else if( rc > 0 ) {
				pData += rc;
				tmpSendByte -= rc;
			}
		}
		sentBytes = (int64)( pData - bu.data() );
	}
	
	sendSw.stop();
	MLOG.debug( SESSFMT( PauseFrameDataRunner,"transferData(), loop[%d] data sent[%ld]bytes, cost[%lu]us, lastrc[%d] bufReqId[%ld] "), 
			sendCount, sentBytes, sendSw.span(), rc, bu.bufReqId() );

	mBytesTransfered		+= sentBytes;
	mBytesSentSinceStart	+= sentBytes;
	dataBytesToSend			-= sentBytes;

	return rc;

}

bool UdpPauseFrameRunner::prepareRunnerBuffer( BufferUser& bufUser, int64& bytesPrepared ) {
	bufUser = mCc.findPauseFrame(mConfWriter);
	return bufUser.valid();
}

bool UdpPauseFrameRunner::sessionEndReached( ) const {
	return false;
}

bool UdpPauseFrameRunner::reachFileEnd( bool& eagain ) {
	eagain = false;
	return false;
}

bool UdpPauseFrameRunner::tryStopRunner()
{
	return true;
}



}//namespace C2Streamer

