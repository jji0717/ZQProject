
#include <ZQ_common_conf.h>
#include <sstream>
#include <TimeUtil.h>
#include "AioFile.h"
#include "C2Session.h"
#include "C2StreamerEnv.h"
#include "C2StreamerService.h"
#include "C2SessionHelper.h"
#include "HttpEngineInterface.h"

#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"%s/%08X/[%s]\t"##y, mSessId.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"%s/%08X/[%s]\t"y, mSessId.c_str() ,  (unsigned int)gettid(),#x	
#endif	
#include <sys/times.h>

namespace C2Streamer
{
//////////////////////////////////////////////////////////////////////////
DataRunnerStatistic::DataRunnerStatistic()
:mTotalDataSendCount(0),
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
// 	oss <<"SocketSend["<<mTotalSendTime<<"/"<<mMaxSendTime<<"/"<<mMinSendTime<<"/"<<(uint64)(mSendTimeRecordCount>0 ? mTotalSendTime/mSendTimeRecordCount:0)<<"] "
// 		<<"DataSendCount["<<mTotalDataSendCount<<"/"<<mMaxDataSendCount<<"/"<<mMinDataSendCount<<"/"<<uint64(mDataSendRecordCount>0 ? mTotalDataSendCount/mDataSendRecordCount : 0 )<<"] "
// 		<<"OneRuntimeCost["<<mTotalRunTime<<"/"<<mMaxRunTime<<"/"<<mMinRunTime<<"/"<<uint64( mRunTimeRecordCount > 0 ? mTotalRunTime/mRunTimeRecordCount : 0 )<<"] "
// 		<<"FsWait["<<mTotalFsWaitTime<<"/"<<mMaxFsWaitTime<<"/"<<mMinFsWaitTime<<"/"<<uint64( mFsWaitTimeRecordCount > 0 ? mTotalFsWaitTime/mFsWaitTimeRecordCount : 0 )<<"] "
// 		<<"TimerShift["<<mTotalTimerShift<<"/"<<mMaxTimerShift<<"/"<<mMinTimerShift<<"/"<<uint64( mTimerShiftCount > 0 ? mTotalTimerShift/mTimerShiftCount : 0 )<<"] "
// 		<<"FirstChunkWait["<<mFirstChunkWait<<"] ";

	char statbuf[1024], *p = statbuf, *t= statbuf + sizeof(statbuf) -2;

#define VALID_WAIT_USEC(_VAR) ((_VAR <0 || _VAR > (mDuration*1000)) ? -1: _VAR)

	p += snprintf(p, t -p, "range[%lld +%lld]@%lldbps ", mRangeStart, mSentBytes, (uint64 (mDuration>0?mSentBytes*8000/mDuration:0)) );
	p += snprintf(p, t -p, "duration[%d]ms@%.2fppm ", mDuration, (mDuration > 0 ? ((float) VALID_WAIT_USEC(mTotalSendTime)*1000 / mDuration): 0.0));
	p += snprintf(p, t -p, "send-latency[%d,a%lld,m%lld]us ", VALID_WAIT_USEC(mMinSendTime), (uint64)(mSendTimeRecordCount>0 ? VALID_WAIT_USEC(mTotalSendTime)/mSendTimeRecordCount:0), VALID_WAIT_USEC(mMaxSendTime));
	p += snprintf(p, t -p, "buf-per-send[%d,a%lld,m%lld] ", mMinDataSendCount, (uint64)(mDataSendRecordCount>0 ? mTotalDataSendCount/mDataSendRecordCount :0), mMaxDataSendCount);
	p += snprintf(p, t -p, "read-latency[%d,a%lld,m%lld]us ", VALID_WAIT_USEC(mMinFsWaitTime), (uint64)( mFsWaitTimeRecordCount > 0 ? VALID_WAIT_USEC(mTotalFsWaitTime)/mFsWaitTimeRecordCount :0), VALID_WAIT_USEC(mMaxFsWaitTime));
//	p += snprintf(p, t -p, "read-speed[%lld]bps ", (uint64)( mFsTotalReadTime>0 ? mSentBytes*8000/mFsTotalReadTime:0);
	p += snprintf(p, t -p, "filtered-timer-lag[1st%d,%d,a%lld,m%d]us x%d ", VALID_WAIT_USEC(mFirstChunkWait), VALID_WAIT_USEC(mMinTimerShift), (uint64( mTimerShiftCount > 0 ? VALID_WAIT_USEC(mTotalTimerShift)/mTimerShiftCount :0)), VALID_WAIT_USEC(mMaxTimerShift), (int)mTimerShiftCount);

	return statbuf;
//	return oss.str();
}
void DataRunnerStatistic::updateSessionStatus( uint64 sentBytes , uint64 rangeStart, uint64 rangeEnd , uint64 duration )
{
	mSentBytes	= sentBytes;
	mRangeStart	= rangeStart;
	mRangeEnd	= rangeEnd;
	mDuration	= duration;
}
void DataRunnerStatistic::updateSendDataStat( uint64 sendTime )
{
	mTotalSendTime += sendTime;
	mMinSendTime	= MIN( mMinSendTime , sendTime);
	mMaxSendTime	= MAX( mMaxSendTime , sendTime );
	++mSendTimeRecordCount;
}

void DataRunnerStatistic::updateOneProcessCountData( uint64 sendCount )
{
	
	mTotalDataSendCount		+= sendCount;
	mMaxDataSendCount		= MAX( mMaxDataSendCount , sendCount );
	mMinDataSendCount		= MIN( mMinDataSendCount , sendCount );
	++mDataSendRecordCount;
}

void DataRunnerStatistic::updateOneRunStat( uint64 runTime)
{
	mTotalRunTime 	+= runTime;
	mMaxRunTime		= MAX( mMaxRunTime , runTime );
	mMinRunTime		= MIN( mMinRunTime , runTime );
	++mRunTimeRecordCount;
}

void DataRunnerStatistic::updateFsWaitStat( uint64 waitTime )
{	
	mTotalFsWaitTime	+= waitTime;
	mMaxFsWaitTime		= MAX( mMaxFsWaitTime , waitTime );
	mMinFsWaitTime		= MIN( mMinFsWaitTime , waitTime );
	++mFsWaitTimeRecordCount;
}

void DataRunnerStatistic::updateTimerShift( uint64 waitTime )
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

void DataRunnerStatistic::updateIoCommit( uint64 commitTime )
{
	mIoCommitCounts++;
	mIoCommitTimeTotal += commitTime;
	mIoCommitTimeMax = MAX( mIoCommitTimeMax , commitTime );
	mIoCommitTimeMin = MIN( mIoCommitTimeMin , commitTime );
}

SessionDataRunner::SessionDataRunner( C2StreamerEnv& env , C2Service& svc , C2SessionPtr sess, CacheCenter& cc, ZQ::common::Mutex& m)
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
mSessTransferDelay(0),
mSessTransferRate(0),
mNextWakeupInterval(0),
mMaxDataSendCount(0),
mRequestByteCount(0),
mLastContentStateLogTime(0),
mDataRunnerStartTime(0),
mDataRunnerStopTime(0),
mbDataTransferComplete(true),
mbFirstChunkWait(true),
mFsWaitCount(0),
mEthMtu(1500)
{
}

SessionDataRunner::~SessionDataRunner()
{
}

void SessionDataRunner::reset( )
{
	mbRunning	= false;
	mBytesTransfered	= 0;
	mDataTransferStartTime = 0;
	mBytesSentSinceStart = 0;
	mDataFileState = DATA_FILE_STATE_NULL;
	mOldDataFileState = DATA_FILE_STATE_NULL;
	mbFileReverseWritten = false;
	mbGetFileWritenDirection = false;
	mSessTransferDelay = 0;
	mNextWakeupInterval = 0;
	mMaxDataSendCount = 0;
	mRequestByteCount = 0;
	mLastContentStateLogTime = 0;
	mRealDataTransferStartTime = 0;
	mDataRunnerStartTime = 0 ; 
	mDataRunnerStopTime = 0;
	mbDataTransferComplete = false;
	mbFirstChunkWait = true;
	mResponse = 0;
	mFsWaitCount = 0;
	mbFileReopened = false;
	mBufUser = BufferUser( mCc, NULL);
}

int64 SessionDataRunner::bytesToSend( int64 maxBytes ) const
{
	if( maxBytes > 0 )
		return maxBytes;
	if( mEnv.getConfig().mPacketsPerSend > 0 )
		return mEnv.getConfig().mPacketsPerSend * ( mEnv.getConfig().mSendPacketSize -8 );
	return mEnv.getConfig().ioBlockSize * mEnv.getConfig().readBufferCount;
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
	uint64 fsReadTime = 0;
	uint64 fsWaitTime = 0;
	
	StopWatch sw;sw.start();

	mBufUser = mAioFile->read(mOffsetInFile, 64 * 1024 );
	bufUser = mBufUser;
	if( !bufUser.valid() )
	{
		MLOG(ZQ::common::Log::L_WARNING, SESSFMT(SessionDataRunner, "prepareRunnerBuffer() failed to get filled AioBuffer, we may wait a little longer:[%lu]" ),sw.stop()/1000);
		return false;
	}

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner,"preapreRunnerBuffer() read at [%s:%ld]"),
			mFileName.c_str(), mOffsetInFile);

//	if( mLastWaitTimeStamp > 0 )
//	{
//		mStatistics.updateFsWaitStat( ZQ::common::now() - mLastWaitTimeStamp );
//		mLastWaitTimeStamp = 0; //reset
//	}
	fsReadTime = sw.stop();
	
	fsWaitTime = bufUser.bufferPrepareTime() * 1000;

	if(mbFirstChunkWait)
	{
		mbFirstChunkWait=false;
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(SessionDataRunner,"prepareRunnerBuffer() first round, took [%lu/%lu]ms to get data from FS"),
				fsReadTime/1000,fsWaitTime/1000);
	}
	if( fsReadTime >= (mEnv.getConfig().mFsReadThreshold *1000 ) )
	{
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(SessionDataRunner,"prepareRunnerBuffer() took  [%lu/%lu]ms to get data from FS"),
				fsReadTime/1000, fsWaitTime/1000);
		mStatistics.updateFsWaitStat(fsWaitTime);
	}
	bytesPrepared = bufUser.dataLeft();
		
	mbTmpReachFileEnd = ( bytesPrepared <= 0 );
	if( mbTmpReachFileEnd )
	{
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(SessionDataRunner,"prepareRunnerBuffer() we may reach the file end at offset[%lld]"),
				mOffsetInFile);
	}
	
	mLastWaitTimeStamp = ZQ::common::now();

	return true;
}


bool SessionDataRunner::dataProcessing(int32& dataSendCount)
{
	dataSendCount = 0;
	StopWatch watch(false);

	StopWatch watchTotal;
	
	int64 sendingBytes = 0;

	BufferUser		bufUser(mSvc.getCacheCenter(), 0);
	watchTotal.start();	
	StopWatch s1,s2,s3,s4;
	size_t runcount = 0 ;
	while( mbRunning)
	{
		//first we need know how many bytes we should send so far
		sendingBytes = bytesToSend( );
		
		if( ( mRequestByteCount > 0 ) && ( sendingBytes > (mRequestByteCount - (int64)mBytesTransfered ) ) )
		{
			sendingBytes = mRequestByteCount - (int64)mBytesTransfered;
		}

		while( sendingBytes > 0 && mbRunning )
		{
			if( !bufUser.valid() || bufUser.dataLeft() <= 0 )
			{
				int64 bytesPrepared = 0;
				s1.start();
				bool bOK = prepareRunnerBuffer( bufUser, bytesPrepared ) ;
				s1.stop();
				if( !bOK )
				{
					mFsWaitCount ++;
					if( mFsWaitCount > mEnv.getConfig().mFsDataWaitCount )
					{
						MLOG(ZQ::common::Log::L_ERROR,SESSFMT(SessionDataRunner,"run() no more data read from file[%s] retried [%d], quit reading"),
							mFileName.c_str(), mFsWaitCount);
						return false;
					}
					uint64 interval = 50 + rand() % 50;
					//MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"run() no data read from file[%s] retried [%d], updateTimer[%lld]"),
					//	mFileName.c_str(), mFsWaitCount, interval);
					mSess->updateTimer(interval);
					goto EXITLOOP;
				}
				mFsWaitCount = 0;
				if( bytesPrepared == 0 )
				{
					s2.start();
					bOK = reachFileEnd();
					s2.stop();
					if( bOK )
					{
						return false;
					}
					else
					{
						if( mResponse->isConnectionBroken() )
						{
							mbDataTransferComplete = false;
							MLOG(ZQ::common::Log::L_WARNING,SESSFMT(SessionDataRunner, "run() connection[%lld] broken, data transfer terminated"),
								mConnectionId );
							return false;
						}

						static int32 maxIdleCountAtFileEnd =  mEnv.getConfig().mMaxIdleCountAtFileEnd;
						if( maxIdleCountAtFileEnd > 0 )
						{
							if( mIdleCountAtFileEnd > maxIdleCountAtFileEnd ) {
								MLOG(ZQ::common::Log::L_WARNING,SESSFMT(SessionDataRunner,"run() reach the max idle count [%ld] at the end of file[%s]"),
									mIdleCountAtFileEnd , mFileName.c_str() );
								return false;
							}
							else if( !mbFileReopened && mIdleCountAtFileEnd >= maxIdleCountAtFileEnd/2 ) {
								mAioFile->close();
								mAioFile = NULL;
								const std::string& reqFileName =  mSess->getFileName();
								mAioFile = mCc.open( reqFileName );
								if(!mAioFile)
								{
									MLOG(ZQ::common::Log::L_ERROR,SESSFMT(SessionDataRunner,"failed to re-open file[%s]"),reqFileName.c_str());
									return false;
								}
								mbFileReopened = true;
								MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"file[%s] re-opened at idle count[%d]"), reqFileName.c_str(), mIdleCountAtFileEnd );
							}
						} 

						++ mIdleCountAtFileEnd; //increase idle count if we are at the end of a file

						mDataTransferStartTime	= ZQ::common::now(); //reset start time to now
						mBytesSentSinceStart	= 0;
						//schedule current session in idle intervale
						mSess->updateTimer( mEnv.getConfig().minIdleInterval+ rand()%50 );
						goto EXITLOOP;
					}
				}
				else
				{
					mIdleCountAtFileEnd = 0;					
				}
			}
			
			sendingBytes = MIN( sendingBytes , (int64)bufUser.dataLeft() );

			dataSendCount++;
			watch.start();
			int64 sentBytes = 0;
			s3.start();
			bool bOK = transferData( bufUser, sendingBytes ,sentBytes ) ;
			runcount ++;
			s3.stop();
			if( !bOK )
			{
				return false;
			}		
			watch.stop();
			if( watch.span() >= (mEnv.getConfig().mSocketSendMonitorThreashold * 1000) )
			{
				MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(SessionDataRunner,"run() took [%lld]ms to send [%lld/%lld]bytes data to client[%s/%d]"), 
					watch.span()/1000 , sendingBytes,sentBytes, mSess->getTransferClientAddress().c_str() , mConnectionId  );

				mDataTransferStartTime = ZQ::common::now();
				mBytesSentSinceStart = sentBytes;
			}
			mStatistics.updateSendDataStat( watch.span() );
		}

		if( sessionEndReached() )
		{//
			mStatistics.updateOneProcessCountData( dataSendCount );	
			return false;
		}

		if( !mbRunning )
		{
			mStatistics.updateOneProcessCountData( dataSendCount );
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"run() session data transfer was aborted"));
			return false;
		}

		if( !bufUser.valid() && mbTmpReachFileEnd )
		{//all data read from FS have been transfered
			s2.start();
			bool bOK = reachFileEnd();
			s2.stop();
			if( bOK) {
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"run() reached file end, runner stopped"));
				return false;//reach file end, terminate current session runner
			}
		}

		uint64 now = ZQ::common::now();
		int64 transferRate = mSessTransferRate;
		transferRate = MAX( 1, transferRate);
		uint64 calcTime = mDataTransferStartTime + mBytesSentSinceStart * 8000 / transferRate; 
		
		if(  dataSendCount >= mEnv.getConfig().maxRoundPerRun )
		{
			static int32 minYieldInterval = mEnv.getConfig().minYieldInterval;
			MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(SessionDataRunner,"run() yield due to reach maxRoundPerRun[%d] per TransferDelay"),mEnv.getConfig().maxRoundPerRun);
			mSess->updateTimer( minYieldInterval);
			goto EXITLOOP;
		}

		if( now >= (calcTime ))
		{
			continue;
		}
		uint64 timetoSleep = calcTime - now ;
		if( timetoSleep < 10 )
			continue;
		
		mStatistics.updateOneProcessCountData( dataSendCount );
		
		mSess->updateTimer( timetoSleep );
		break;
	}//while(true)

EXITLOOP:
	watchTotal.stop();
	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner,"dataProcessing runcount[%d] cost[%lu/%lu/%lu/%lu]us total[%lu]us"),
			(int)runcount, s1.span(), s2.span(), s3.span(),s4.span(), watchTotal.span());
	return mbRunning;
}



bool SessionDataRunner::run( uint64 timerShift )
{
	ZQ::common::MutexGuard gd(mMutex);
	if( !mbRunning )
	{
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"run() data runner is stopped"));
		stopRunner();
		return false;
	}
	mStatistics.updateTimerShift(timerShift*1000);//we use microsecond
	processTimeRecord ptrecord(mStatistics);
	int32 dataSendCount = 0;
	StopWatch sw;sw.start();
	bool bRet = dataProcessing(dataSendCount);
	sw.stop();
	if( sw.span() > 10 * 1000 ) {
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(dataProcessing,"took [%lld]ms to complete dataProcessing "),sw.span()/1000);
	}
	if( dataSendCount > 0 )
		mStatistics.updateOneProcessCountData( dataSendCount );
	if( !bRet )
	{
		stopRunner();
	}
	return bRet;
}

bool SessionDataRunner::transferData( BufferUser& bufUser, int64& dataBytesToSend , int64& sentBytes )
{
	if( dataBytesToSend <= 0 )
	{//never be here
		dataBytesToSend = 0;
		return true;
	}
	assert( bufUser.valid() );

	int64 availBytes = bufUser.dataLeft();

	//calculate how many byte should be sent out	
	if( ( mRequestByteCount > 0 ) && ( dataBytesToSend > (mRequestByteCount - (int64)mBytesTransfered ) ) )
	{
		sentBytes = mRequestByteCount - (int64)mBytesTransfered;
	}
	else
	{
		sentBytes = dataBytesToSend;
	}
	sentBytes = MIN(sentBytes , availBytes);

	if( sentBytes > 0 )
	{
		static uint32 packetSize = mEthMtu - 8;

		const char* pData = bufUser.data();
		int64 tmpSendByte = sentBytes;
	
		//mResponse->setCommOption( ZQHttp_OPT_NoDelay , 0 );
		if( mEnv.getConfig().mbUseTcpCork)
			mResponse->setCommOption( ZQHttp_OPT_HoldOn , 1 );
		while( tmpSendByte > 0 )
		{
			int64 dataSize = 0 ;
			if( tmpSendByte > packetSize )
			{
				dataSize = packetSize;
				tmpSendByte -= dataSize;
			}
			else
			{
				dataSize = tmpSendByte;
				tmpSendByte = 0;
			}
			if(!mResponse->addBodyContent( pData , dataSize ) )
			{
				//failed to transfer data to peer , should I close this session ?
				MLOG(ZQ::common::Log::L_ERROR, SESSFMT(SessionDataRunner,"transferData() failed to transfer data to peer through connection[%lld], last error[%s]"),
					mConnectionId, mResponse->lastError().c_str() );
				return false;
			}
			pData += dataSize;
// 			if( ( (++packetsCount) % mEnv.getConfig().mPacketsPerSend) == 0)
// 			{				
// 				packetsCount = 0;
// 				mResponse->setCommOption( ZQHttp_OPT_HoldOn , 0 );//flush data out
// 				mResponse->setCommOption( ZQHttp_OPT_HoldOn , 1 );//hold data for next iteration of data preparetion
// 				ZQ::common::delay(1);
// 			}
		}
		if( mEnv.getConfig().mbUseTcpCork)
			mResponse->setCommOption( ZQHttp_OPT_HoldOn , 0 );
		//mResponse->setCommOption( ZQHttp_OPT_NoDelay , 1 );
	}
	
	mBytesTransfered		+= sentBytes;
	mBytesSentSinceStart	+= sentBytes;
	dataBytesToSend			-= sentBytes;
	mOffsetInFile			+= sentBytes;
	bufUser.advance(sentBytes);

	return true;
}

void SessionDataRunner::stopRunner()
{
	ZQ::common::MutexGuard gd(mMutex);
	if( !mbRunning )
	{
		mSess = NULL;
		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner,"stopRunner() data runner is not running"));
		return;
	}
	MLOG( ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner, "stopRunner() stopping data runner") );
	mbRunning = false;
	mSvc.unwatchSession( mSess );
	transferComplete();
	if( mAioFile )
	{
		mAioFile->close();
		mBufUser = BufferUser(mCc,NULL);
		mAioFile = 0;
	}
	mSess->changeState( SESSION_STATE_IDLE , true );
	MLOG( ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner, "stopRunner() data runner stopped") );
	
	mStatistics.updateSessionStatus( mBytesTransfered , mRequestRange.startPos , 
									mRequestRange.startPos + mBytesTransfered , 
									mDataRunnerStopTime -  mRealDataTransferStartTime );
	if( mSess &&  BASE_SESSION_TYPE(mSess->mSessionType) != SESSION_TYPE_COMPLEX )
	{
		mSess->updateTimer(0);
	}
}

bool SessionDataRunner::startRunner( C2HttpResponseHanlderPtr response, const std::string& filename , C2SessionPtr sess )
{
	if( mAioFile )
		mAioFile->close();
	mSess = sess;

	//int sndBufSize = (int)(mEnv.getConfig().readBufferCount*mEnv.getConfig().ioBlockSize*2 );
	int sndBufSize = mEnv.getConfig().mSocketKernelSndBuf;
	if(sndBufSize < 32*1024)
		sndBufSize = 32*1024;

	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner,"startRunner() trying to start runner: file[%s] offset[%lld] , sendPacketSize[%u] , sendPacketCount[%d] fsReadBufSize[%u], kernelSndBuf[%d]"),
		filename.c_str() , mRequestRange.bStartValid ? mRequestRange.startPos : 0 , mEthMtu ,
		mEnv.getConfig().mPacketsPerSend, mEnv.getConfig().readBufferCount * mEnv.getConfig().ioBlockSize , sndBufSize);

	// since transfer delay may be negative value, so we must make sure that tick count is a large enough value
	mDataTransferStartTime	= (uint64)((int64)ZQ::common::now() + (int64)mSessTransferDelay ); 
	mRealDataTransferStartTime = mDataTransferStartTime;
	
	ZQ::common::MutexGuard gd(mMutex);
	assert( response != 0 );
	std::string reqFileName =  sess->getFileName();
	mAioFile = mCc.open( reqFileName );
	if(!mAioFile)
	{
		MLOG(ZQ::common::Log::L_ERROR,SESSFMT(startRunner,"failed to open file[%s]"),reqFileName.c_str());
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
		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner,"startRunner() request data bytes [%lld]"), mRequestByteCount);
	}
	
	response->setCommOption( ZQHttp_OPT_WriteBufSize, sndBufSize );
	
	response->setCommOption( ZQHttp_OPT_sendTimeo, mEnv.getConfig().mSocketKernelSndTimeo);

	mResponse			= response;
	mFileName			= filename;

	mAioFile->read( mOffsetInFile, 64 * 1024, true); //kick off a read request, no matter how many byte do we want

	mbRunning = true;	
	mConnectionId = mResponse->getConnectionId();

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"startRunner() submitted reading file[%s] from offset[%lld], sendPacketSize[%u], sendPacketCount[%d] fsReadBufSize[%u], kernelSndBuf[%d]"),
		filename.c_str() , mRequestRange.bStartValid ? mRequestRange.startPos : 0 , mEthMtu ,
		mEnv.getConfig().mPacketsPerSend, mEnv.getConfig().readBufferCount * mEnv.getConfig().ioBlockSize , sndBufSize);
	return true;
}

void SessionDataRunner::transferComplete()
{
	ZQ::common::MutexGuard gd(mMutex);
	mDataRunnerStopTime = ZQ::common::now();
	if( mResponse )
	{		
		int64 delta = mDataRunnerStopTime - mRealDataTransferStartTime;
		int64 avgRate = delta > 0 ? (mBytesTransfered*8000/delta) : 0 ;
		//mResponse->addBodyContent( EndOfChunkData.c_str() , EndOfChunkData.length() );
		if(!mResponse->complete() ) {
			MLOG(ZQ::common::Log::L_WARNING, SESSFMT(SessionDataRunner,"failed to send 0 size chunk data to client"));
		}
		mResponse = NULL; // release response object
		MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"transferComplete() transfer stat: request range[%s] , bytetransfered[%llu] avgBitrate[%lld]"),
			 mRequestRange.toString().c_str() , mBytesTransfered , avgRate);
	}
}

bool SessionDataRunner::sessionEndReached( ) const
{
	if( !mRequestRange.bEndValid )
		return false; // we must reach the end of the file to complete the session transfer
	if( mRequestByteCount > 0 &&  (int64)mBytesTransfered >= mRequestByteCount )
	{
		MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(SessionDataRunner,"reached session end: request[%s], byteTransferred[%llu]"),
			 mRequestRange.toString().c_str() , mBytesTransfered );
		return true;
	}
	else
	{
		return false;
	}
}

bool SessionDataRunner::reachFileEnd()
{
	if( mEnv.getConfig().clientType != 0 ) {
		// non-local disk read, just return false 
		// let sessionEnd to detect if it's complete
		C2SessionManager& sessManager = mSvc.getSessManager();
		AssetAttribute::Ptr attr = sessManager.getAssetAttribute( mFileName );
		assert(attr != NULL);
		
		bool inprgs = attr->inprogress();
		if( inprgs ) {
			mDataFileState = DATA_FILE_STATE_WRITE;
			mOldDataFileState = mDataFileState;
			MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"asset for[%s], attribute: inprogress[%s] . continue to wait data"),
					mFileName.c_str(), inprgs ? "true": "false");
			return false;
		} else {
			attr->wait();

			if( attr->lastError() != 0 ) {
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"asset for [%s] attribute: lastError[%d] . take it as end-of-file"),
					mFileName.c_str(),	attr->lastError());
				return true;
			}

			if( attr->pwe() ) {
				if( ( mFileName.find(".index") != std::string::npos)&& (mEnv.getConfig().clientType == 1) ){
					MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"asset for[%s] attribute: pwe[%s], but this is a index file, just take it as end-of-file"),
						mFileName.c_str(), attr->pwe() ? "true": "false");
					mAioFile->invalidateCache( mOffsetInFile, 64*1024);
					return true;
				}
				mDataFileState = DATA_FILE_STATE_WRITE;
				mOldDataFileState = mDataFileState;
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"asset for[%s] attribute: pwe[%s] . continue to wait data"),
						mFileName.c_str(), attr->pwe() ? "true": "false");
				mAioFile->invalidateCache( mOffsetInFile, 64 * 1024 );
				return false;
			} else {
				mDataFileState = DATA_FILE_STATE_STABLE;
				if( mOldDataFileState == DATA_FILE_STATE_WRITE ) {
					MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"asset for[%s]: changed from pwe to non-pwe, just continue to read data"), mFileName.c_str() );

					mOldDataFileState = mDataFileState;
					mAioFile->invalidateCache( mOffsetInFile, 64 * 1024 );
					return false;
				}
			}
			
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"asset for [%s], attribute: inprogress[%s], pwe[%s] lastError[%d]. take it as end-of-file"),
					mFileName.c_str(), inprgs ? "true":"false" , attr->pwe() ? "true":"false", attr->lastError());
		}

		return true;
	}
	//FIXME: check if really reach the end
	bool bFileSteady = true;
	if( DATA_FILE_STATE_STABLE != mDataFileState )
	{
		if(!mbGetFileWritenDirection)
		{
			mbGetFileWritenDirection = true;
			
			//C2SessFile c2f(	mSess->mRequestFileName , mEnv , mSessId);
			struct stat st;
			memset( &st,0,sizeof(st) );
			if( stat(mFileName.c_str() , &st) != 0 )
			{
				MLOG(ZQ::common::Log::L_ERROR,SESSFMT(SessionDataRunner,"failed to get stat of file[%s]"),mFileName.c_str() );
				mbFileReverseWritten = false;
			}
			else
			{
				mbFileReverseWritten = (int64)st.st_size >=  (20LL * GIGABYTES);
			}

			//treat it as sparse file if the data end pos > 20G
			//mbFileReverseWritten = c2f.dataEndPos() > ( 20LL * GIGABYTES );  //iBridge->isFileReverseWritten( mSess->mRequestFileName , mSessId );
			MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"reachFileEnd() file[%s] endpos[%lld], reverse write[%s]"), 
				mSess->mRequestFileName.c_str(), (int64)st.st_size , mbFileReverseWritten ?"true":"false");

			if( mbFileReverseWritten)
			{
				mDataFileState = DATA_FILE_STATE_STABLE;//because sparse file is written from tail to head, if a session read the
														//file data from a position to the end, we should think that there is no more
														//data need to be read. so just terminate the session by identif that we already
														//get to the file end
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"reachFileEnd() [%s] is a sparse file, treated as file end reached"),
					mFileName.c_str() );
				return true;
			}
		}
		IAttrBridge* iBridge = mEnv.getAttrBridge();
		if(iBridge)
		{			
			bFileSteady = !iBridge->isFileBeingWritten( mSess->mRequestFileName , mSessId );
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
			MLOG(ZQ::common::Log::L_INFO,SESSFMT(SessionDataRunner,"reachFileEnd() content[%s] is %s being written, communicator[%lld]"),
				mSess->mRequestFileName.c_str(), bFileSteady ? "NOT" : "" ,mResponse->getConnectionId() );
 		}
	}

	if( bFileSteady )//not being written
	{
		uint64 fsWaitTime = 0;
		uint64 fsReadTime = 0;
		if( mOldDataFileState == DATA_FILE_STATE_WRITE ) {
			mAioFile->invalidateCache( mOffsetInFile,64*1024); //invalidate last block
		}
		mOldDataFileState = mDataFileState;
		BufferUser bufUser = mAioFile->read( mOffsetInFile, 64 * 1024 );
		if( !bufUser.valid() )
		{
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"reachFileEnd() can't get filled aio buffer, wait a little longer and try again"));
			return false;
		}
		if( bufUser.dataLeft() <= 0  )
		{
			//we reach the file end
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"reachFileEnd() reached EOF"));
			return true;
		}
		MLOG(ZQ::common::Log::L_INFO, SESSFMT(SessionDataRunner,"some more data need to be read"));
	} else {
		mAioFile->invalidateCache( mOffsetInFile, 64*1024);
		mAioFile->read(mOffsetInFile,64*1024);
		mOldDataFileState = mDataFileState;
	}
	return false;
}

void SessionDataRunner::markAsConnBroken() {
}


}//namespace C2Streamer
