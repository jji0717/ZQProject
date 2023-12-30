#ifndef _tianshan_cdnss_c2streamer_header_file_h__
#define _tianshan_cdnss_c2streamer_header_file_h__

#include <Locks.h>
#include <DataPostHouse/common_define.h> //using ShareObject
#include "C2StreamerLib.h"
#include "AioFile.h"

namespace C2Streamer
{

class C2StreamerEnv;
class C2Service;
class C2Session;
typedef ZQ::DataPostHouse::ObjectHandle<C2Session> C2SessionPtr;

class DataRunnerStatistic
{
public:
	DataRunnerStatistic();	
	virtual ~DataRunnerStatistic();

public:
	
	void	updateSendDataStat( uint64 sendTime );
	
	void	updateOneProcessCountData( uint64 sendCount );
	
	void	updateOneRunStat( uint64 runTime);
	
	void	updateFsWaitStat(uint64 waitTime);	

	void	updateTimerShift( uint64 waitTime );

	void	updateIoCommit( uint64 commitTime );

	void	updateSessionStatus( uint64 sentBytes , uint64 rangeStart, uint64 rangeEnd , uint64 duration );
	
	std::string	toString() const;	

	void	reset();

public:
	
	uint64			mTotalDataSendCount;	// how many times data runner call addContentData to send data out to peer in whole session transfer process
	uint64			mMaxDataSendCount;		// MAX count that data runner call addContentData to send data out to peer in one send process
	uint64			mMinDataSendCount;		// MIN count that data runner call addContentData to send data out to peer in one send process
	uint64			mDataSendRecordCount;
	
	uint64			mTotalSendTime;			
	uint64			mMaxSendTime;
	uint64			mMinSendTime;
	uint64			mSendTimeRecordCount;
	
	
	uint64			mTotalFsWaitTime;
	uint64			mMaxFsWaitTime;
	uint64			mMinFsWaitTime;
	uint64			mFsWaitTimeRecordCount;	
	
	uint64			mTotalRunTime;
	uint64			mMaxRunTime;
	uint64			mMinRunTime;
	uint64			mRunTimeRecordCount;

	uint64			mFirstChunkWait;
	
	uint64			mTotalTimerShift;
	uint64			mMaxTimerShift;
	uint64			mMinTimerShift;
	uint64			mTimerShiftCount;

	uint64			mSentBytes;
	uint64			mRangeStart;
	uint64			mRangeEnd;
	uint64			mDuration;

	uint64			mIoCommitTimeMax;
	uint64			mIoCommitTimeMin;
	uint64			mIoCommitTimeTotal;
	uint64			mIoCommitCounts;
};
/**
	SessionDataRuner is responsible for pushing data out through socket to client
  */
class SessionDataRunner
{
public:
	SessionDataRunner( C2StreamerEnv& env , C2Service& svc , C2SessionPtr sess, CacheCenter& cc, ZQ::common::Mutex& m);
	virtual ~SessionDataRunner();

public:
	
	/// start to prime buffer data and add runner to Scheduler
	bool			startRunner( C2HttpResponseHanlderPtr response , const std::string& filename , C2SessionPtr sess );

	/// stop runner , after this , no more data will be transfered
	void			stopRunner();

	///transfer data
	///return true means timer should be updated and future works need to be performed
	bool			run( uint64 timerShift );

	uint64			getWakeupInterval();

	void			reset();// reset data runner to it's default state so that we can used it again to download another file

protected:
	enum DataFileState
	{
		DATA_FILE_STATE_NULL,
		DATA_FILE_STATE_WRITE,
		DATA_FILE_STATE_STABLE
	};

private:

	bool				transferData( BufferUser& bufUser, int64& dataBytesToSend ,int64& senBytes );

	///return true means timer should be updated and future works need to be performed
	bool				reachFileEnd();

	void				transferComplete();

	bool				sessionEndReached( ) const;

	int64				bytesToSend( int64 maxBytes  = -1 ) const;

	bool				prepareRunnerBuffer( BufferUser& bufUser, int64& bytesPrepared );
	
	bool				dataProcessing( int32& sendCount );
	
	bool				dataProcessing2( int32& sendCount );

	bool				dataProcessing3( int32& sendCount );

	void				markAsConnBroken();

private:
	friend class C2Session;

	bool						mbRunning;

	C2StreamerEnv&				mEnv;
	C2Service&					mSvc;
	CacheCenter&				mCc;
	std::string					mSessId;
	C2SessionPtr				mSess;
	ZQ::common::Mutex&			mMutex;
	std::string					mFileName;

//AioFile is changed
	AioFile*					mAioFile;
	bool						mbFileReopened;
	BufferUser					mBufUser;
// no AioBuffer anymore
//	AioBuffer*					mpAioBuffer;
//	RunnerBuffer				mRunnerBuffer;

	bool						mbTmpReachFileEnd;

	C2HttpResponseHanlderPtr	mResponse;
	
	uint64						mBytesTransfered;	// transfered bytes of content	for statistics
	uint64						mDataTransferStartTime; // last time when data is being started to transfer, 
	uint64						mRealDataTransferStartTime;
	uint64						mBytesSentSinceStart;
	uint64						mOffsetInFile;
	// if we reach the end of a content while it's being written, 
	// new start time will be assigned to this member

	DataFileState				mDataFileState;
	DataFileState				mOldDataFileState;
	bool						mbFileReverseWritten;
	bool						mbGetFileWritenDirection;

	//FIXME: how to deal with transfer delay
	int64						mSessTransferDelay;		//I am not clear with this
	int64						mSessTransferRate;		// requested transfer rate , bits per second

	TransferRange				mRequestRange;			// requested transfer range	

	uint64						mNextWakeupInterval;
	
	uint64						mMaxDataSendCount;	
	
	DataRunnerStatistic			mStatistics;
	
	int64						mRequestByteCount;
	
	int32						mIdleCountAtFileEnd;

	int64						mLastContentStateLogTime;

	int64						mDataRunnerStartTime;
	int64						mDataRunnerStopTime;
	int64						mConnectionId;
	bool						mbDataTransferComplete;
	bool						mbFirstChunkWait;

	uint32						mFsWaitCount;

	int							mEthMtu;

	int64						mLastWaitTimeStamp;
};


class C2Session : public ZQ::common::Mutex , public ZQ::DataPostHouse::SharedObject 
{
public:
	C2Session( C2StreamerEnv& env , C2Service& svc ,const std::string& sessId );
	virtual ~C2Session(void);

public:
	
	///process session request such as init term status transfer
	int32					processRequest( const RequestParamPtr request , RequestResponseParamPtr response );	

	//destroy session and release all allocated resources
	void					destroy();

	void					markAsConnBroken();

	// check file data
	// can only invoke this method in TransferSession state
	int 					checkFileData( const SessionTransferParamPtr request,bool& cacheAble);

	void					onTimer( );

	const std::string&		getSessId( ) const;
	
	int64					getReservedBandwidth( ) const;

	SessionState			getState( ) const;

	std::string				getFileName( ) const;

	int64					getBytesTransfered( ) const;

	std::string				getTransferServerAddress( ) const;
	
	std::string				getTransferClientAddress( ) const;
	
	std::string				getTransferPortName( ) const;

	int64					getTimeInState( ) const;
	
	int64					getTransferRate( ) const;

	/// start transfer file content data to client
	bool					startTransfer( );
	
	void					changeState( const SessionState& targetState ,bool bUpdateTimer = true );
	
	void					updateTimer( uint64 timeInterval );
	void					cancelTimer();

	void					setRequestType( int32 type );
protected:
	
	friend class SessionDataRunner;

	int32					processTransferInit( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response );

	int32					processTransferTerm( const TransferTermRequestParamPtr request , TransferTermResponseParamPtr response );

	int32					processTransferSession( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response );

	int32					processSessionStatus( const SessionStatusRequestParamPtr request , SessionStatusResponseParamPtr response );	
	
protected:
	
	void					updateLastError( RequestParamPtr request , RequestResponseParamPtr response , int errorCode , const char* fmt ,... );
	
	int32					registerSession( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response );

	int32					registerShadowSession( const TransferInitRequestParamPtr request, TransferInitResponseParamPtr response);
	
	void					unregisterSession();
	
	int32					checkState( SessionState targetState );
	
	void					runSession( uint64 timeshift);

	///detect if we reach the of the session
	bool					reachSessionEnd() const;

protected:
	
		
	C2StreamerEnv&			mEnv;	
	C2Service&				mSvc;

	std::string				mSessionId;			// transferId as well
	std::string				mFileName;			// full path file name of content which is used to be pumped
	std::string				mRequestFileName;	// requested file name, not the full path file name

	std::string				mContentName;
	std::set<std::string>	mRequestFileExts;	// for file set downloading, this is a request file name, not full path file name
												// only valid if requesType = SESSION_TYPE_COMPLEX
	
	std::string				mClientTransferAddress;	// client transfer address
	std::string				mServerTransferAddress;	// server transfer address
	std::string				mServerTransferPortName;

	
	int32					mTimeoutInterval;	//	
	uint64					mStartTimeInState;	// since when current session is in current state

	SessionState			mSessionState;		// session state	
	
	SessionDataRunner*		mDataRunner;
	
	uint64					mNextWakeUpTime;

	int32					mSessionType;// 0 -normal session, 1 -SeaChange request session

	bool					mbMainFile;	// only main file's off set can be adjusted by SeekIFrame
	std::string				mIndexFilePathname;
};

typedef ZQ::DataPostHouse::ObjectHandle<C2Session>  C2SessionPtr;




}//namespace C2Streamer

#endif//_tianshan_cdnss_c2streamer_header_file_h__
