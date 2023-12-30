#ifndef _tianshan_cdnss_c2streamer_header_file_h__
#define _tianshan_cdnss_c2streamer_header_file_h__

#include <Locks.h>
//#include <DataPostHouse/common_define.h> //using ShareObject
#include "C2StreamerLib.h"
#include "AioFile.h"
#include "C2SessionHelper.h"
#include <libasync/eventloop.h>
#include <math.h>
#include "../IndexFileParser.h"
#include "C2SessionUdpQuery.h"
#include "C2TsParser.h"

namespace C2Streamer
{

class C2StreamerEnv;
class C2Service;
class C2Session;

class DataRunnerStatistic
{
public:
	DataRunnerStatistic();	
	virtual ~DataRunnerStatistic();

public:

	void 	updateRequestedBitrate(int64 reqBitrate);
	
	void	updateSendDataStat( int64 sendTime );
	
	void	updateOneProcessCountData( int64 sendCount );
	
	void	updateOneRunStat( int64 runTime);
	
	void	updateFsWaitStat(int64 waitTime);	

	void	updateTimerShift( int64 waitTime );

	void	updateIoCommit( int64 commitTime );

	void	updateSessionStatus( int64 sentBytes , int64 rangeStart, int64 rangeEnd , int64 duration );
	
	std::string	toString() const;	

	void	reset();

public:

	int64			mRequestedBitrate;
	
	int64			mTotalDataSendCount;	// how many times data runner call addContentData to send data out to peer in whole session transfer process
	int64			mMaxDataSendCount;		// MAX count that data runner call addContentData to send data out to peer in one send process
	int64			mMinDataSendCount;		// MIN count that data runner call addContentData to send data out to peer in one send process
	int64			mDataSendRecordCount;
	
	int64			mTotalSendTime;			
	int64			mMaxSendTime;
	int64			mMinSendTime;
	int64			mSendTimeRecordCount;
	
	
	int64			mTotalFsWaitTime;
	int64			mMaxFsWaitTime;
	int64			mMinFsWaitTime;
	int64			mFsWaitTimeRecordCount;	
	
	int64			mTotalRunTime;
	int64			mMaxRunTime;
	int64			mMinRunTime;
	int64			mRunTimeRecordCount;

	int64			mFirstChunkWait;
	
	int64			mTotalTimerShift;
	int64			mMaxTimerShift;
	int64			mMinTimerShift;
	int64			mTimerShiftCount;

	int64			mSentBytes;
	int64			mRangeStart;
	int64			mRangeEnd;
	int64			mDuration;

	int64			mIoCommitTimeMax;
	int64			mIoCommitTimeMin;
	int64			mIoCommitTimeTotal;
	int64			mIoCommitCounts;
};


class C2Session;
typedef ZQ::common::Pointer<C2Session> C2SessionPtr;

class  DataReadCallback : public LibAsync::AsyncWork, public IAsyncNotifySinker {
public:
	typedef ZQ::common::Pointer<DataReadCallback> Ptr;
	
	DataReadCallback( C2SessionPtr sess, LibAsync::EventLoop* loop, bool bCheckFileEnd = false );
	virtual ~DataReadCallback();

	void	startTimer();

protected:

	virtual void onNotified();

	virtual void onAsyncWork();

	void onTimer();

private:
	LibAsync::EventLoop*	mLoop;
	C2SessionPtr			mC2Session;
	bool					mbCheckFileEnd;
	LibAsync::Timer::Ptr	mTimer;
};

class HttpWritableCallback : public ZQHttp::IChannelWritable  {
public:
	typedef ZQ::common::Pointer<HttpWritableCallback> Ptr;
	HttpWritableCallback( C2SessionPtr sess);
	virtual ~HttpWritableCallback();
private:
	virtual void onWritable();
private:
	C2SessionPtr 			mC2Session;
};

class C2SessionAsyncTimer : public LibAsync::Timer {
public:
	typedef ZQ::common::Pointer<C2SessionAsyncTimer> Ptr;
	C2SessionAsyncTimer( C2SessionPtr sess, LibAsync::EventLoop* loop );
	virtual ~C2SessionAsyncTimer();
private:
	virtual void onTimer();
private:
	C2SessionPtr	mC2Session;
};

class C2SessionStatTimer : public LibAsync::Timer{
public:
	typedef ZQ::common::Pointer<C2SessionStatTimer> Ptr;
	C2SessionStatTimer(C2SessionPtr sess, LibAsync::EventLoop* loop);
	virtual ~C2SessionStatTimer();
private:
	virtual void onTimer(){};
private:
	C2SessionPtr    mC2Session;
};

class SessionStopRunnerAsyncWork : public LibAsync::AsyncWork {
public:
	typedef ZQ::common::Pointer<SessionStopRunnerAsyncWork> Ptr;
	SessionStopRunnerAsyncWork( C2SessionPtr sess, LibAsync::EventLoop* loop );
	virtual ~SessionStopRunnerAsyncWork( );
private:
	virtual void onAsyncWork( );
private:
	C2SessionPtr				mC2Session;
};


class SessionDataRunner : public ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer<SessionDataRunner> Ptr;
	SessionDataRunner( C2StreamerEnv& env , C2Service& svc , C2SessionPtr sess, CacheCenter& cc, ZQ::common::Mutex& m, int64 subSessId);
	virtual ~SessionDataRunner();

public:
	
	/// start to prime buffer data and add runner to Scheduler
	bool			startRunner( C2ResponseHandler::Ptr response , C2SessionPtr sess );

	bool 			prepareUdpRunner( );

	/// stop runner , after this , no more data will be transfered
	void			stopRunner( bool bDestroyed = false);

	///transfer data
	///return true means timer should be updated and future works need to be performed
	bool			run( int64 timerShift = 0 );

	int64			getWakeupInterval();

	void			reset();// reset data runner to it's default state so that we can used it again to download another file

	const std::string& getFileName() const {
		return mRequestFileName;
	}

	// get data runner's Id, which is the mSubSessionRunnerId
	int64				getRunnerId() const {
		return mSubSessionRunnerId;
	}

	bool				postReachFileEnd( AssetAttribute::Ptr attr, bool directCall);

	void				markAsConnBroken();

	void                reportStreamStatus();

	void 				setReqFilename(const std::string& filename) {
		mRequestFileName = filename;
	}

	const std::string&	getReqFilename( ) const {
		return mRequestFileName;
	}

	void				setReqAssetName( const std::string& assetName ) {
		mRequestAssetName = assetName;
	}

	const std::string&	getReqAssetName( ) const {
		return mRequestAssetName;
	}

	void				setResponseHandler(C2ResponseHandler::Ptr response) {
		mResponseHandler = response;
	}

	int64 				getTransferRate() const {
		return mTransferRate;
	}

	void 				setTransferRate( int64 rate );

	int64				getTransferDelay() const {
		return mTransferDelay;
	}
	
	void 				setTransferDelay( int64 delay ) {
		mTransferDelay = delay;
	}

	int64				getBytesTransfered() const;

	int64				getReservedBandwidth() const {
		return mTransferRate;
	}

	int					getEthMtu() const {
		return mEthMtu;
	}

	int					getRunnerVersion() const {
		return mSubSessionRunnerVersion;
	}

	void				setRunnerVersion( int ver ) {
		mSubSessionRunnerVersion = ver;
	}

	void 				setTransferRange( const TransferRange& range ) {
		mRequestRange = range;
		mOffsetInFile = range.startPos;
	}

	const TransferRange& getTransferRange() const {
		return mRequestRange;
	}

	void				setConfWriter( const ConfWriter* writer) {
		mConfWriter = writer;
	}

	void				setReaderType( int typ ) {
		mReaderType = typ;
	}

	void				updateSessProperty( const SessionProperty& props) {
		mSessProperty = props;
	}

	void				setUdpSessionRunnerScale( float scale ) {
		mUdpSessionRunnerScale = scale;
	}

	float				getUdpSessionRunnerScale() const { 
		return mUdpSessionRunnerScale;
	}

	int64				getUdpSessionRunnerNpt(int64* dataOffset = NULL, int64 *outDataOffset = NULL) const;

	void				setUdpSessionRunnerStartNpt( int64 npt ) {
		mUdpSessionRunnerStartTimeStamp = 0;
		mUdpSessionRunnerStartNpt = npt;
	}

	void				setUdpSessionRunnerState( UdpSessionState state ) {
		mUdpSessionRunnerState = state;
	}

	UdpSessionState		getUdpSessionRunnerState( ) const {
		return mUdpSessionRunnerState;
	}
	
	void				onDataGotten( bool bCheckFileEnd );
	void 				onReadTimedout();
	void				onWritable();
	void				onAsyncTimer();

protected:
	enum DataFileState
	{
		DATA_FILE_STATE_NULL,
		DATA_FILE_STATE_WRITE,
		DATA_FILE_STATE_STABLE
	};

private:

	virtual bool				tryStopRunner();

	int64 						findUdpSessionStopPoint(BufferUser& bu);

	virtual int64				transferData( BufferUser& bufUser, int64& dataBytesToSend ,int64& senBytes );

	///return true means timer should be updated and future works need to be performed
	virtual bool				reachFileEnd( bool& eagain);

	virtual void				transferComplete();

	virtual bool				sessionEndReached( ) const;

	virtual int64				bytesToSend( int64 maxBytes  = -1 ) const;

	virtual bool				prepareRunnerBuffer( BufferUser& bufUser, int64& bytesPrepared );
	
	virtual bool				dataProcessing( int32& sendCount );

	virtual bool 				dataEnoughToRun(const BufferUser& bufUser);
	

	enum TransferState {
		STATE_RUNNER_IDLE,
		STATE_RUNNER_WAITTOWRITE,
		STATE_RUNNER_WAITTOREAD,
		STATE_RUNNER_TRANSFER,
	};
	
protected:
	//friend class C2Session;

	bool						mbRunning;

	C2StreamerEnv&				mEnv;
	C2Service&					mSvc;
	CacheCenter&				mCc;
	std::string					mSessId;
	C2SessionPtr				mSess;
	ZQ::common::Mutex&			mMutex;
	// mFileName已经不被使用了，取而代之的是mRequestFileName,
	// 也就是说Session或者DataRunner并不需要知道全路径
	// 这个留给AioFile去了解
	//std::string					mFileName; 
	std::string					mRequestFileName;
	std::string					mRequestAssetName;

	AioFile*					mAioFile;
	bool						mbFileReopened;
	BufferUser					mBufUser;
	bool						mbTmpReachFileEnd;

	C2ResponseHandler::Ptr		mResponseHandler;
	
	mutable AtomicInteger<int64>	mBytesTransfered;	// transfered bytes of content	for statistics
	int64						mDataTransferStartTime; // last time when data is being started to transfer, 
	int64						mRealDataTransferStartTime;
	int64						mBytesSentSinceStart;
	int64						mOffsetInFile;
	// if we reach the end of a content while it's being written, 
	// new start time will be assigned to this member

	DataFileState				mDataFileState;
	DataFileState				mOldDataFileState;

	bool						mbFileReverseWritten;
	bool						mbGetFileWritenDirection;

	int64						mTransferDelay;
	int64						mTransferRate;		// requested transfer rate , bits per second

	TransferRange				mRequestRange;			// requested transfer range	

	int64						mNextWakeupInterval;
	
	int64						mMaxDataSendCount;	
	
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

	TransferState				mTransferState;

	int64						mLastReadWaitTimeStamp;
	int64						mLastNonwritableTimeStamp;

	bool						mbConnBroken;

	SessionProperty 			mSessProperty;


	const ConfWriter*			mConfWriter;
	int							mReaderType;

	bool						mbUdpSession;
	int64						mSubSessionRunnerId;
	int							mSubSessionRunnerVersion;
	UdpSessionState				mUdpSessionRunnerState;
	float						mUdpSessionRunnerScale;
	int64						mUdpSessionRunnerStartNpt;
	int64						mUdpSessionRunnerStartTimeStamp;
	TsParser::FrameStat			mUdpTsFrameStat;//used only for udp ts packet data
	bool 						mbInUdpStateTransition;
	bool						mbRunnerComplete;
};

class UdpPauseFrameRunner : public SessionDataRunner {
public:
	UdpPauseFrameRunner( C2StreamerEnv& env , C2Service& svc , C2SessionPtr sess, CacheCenter& cc, ZQ::common::Mutex& m, int64 subSessId );
	virtual ~UdpPauseFrameRunner();

	void 	setBZeroMotionFrame(const char* frame, size_t size);
	void 	setPZeroMotionFrame(const char* frame, size_t size);
protected:

	virtual bool  tryStopRunner();
	virtual int64 transferData( BufferUser& bufUser, int64& dataBytesToSend ,int64& senBytes );

	virtual bool prepareRunnerBuffer( BufferUser& bufUser, int64& bytesPrepared );

	virtual bool sessionEndReached( ) const;

	virtual bool reachFileEnd( bool& eagain );

	virtual bool dataEnoughToRun(const BufferUser& bufUser) {
		return true;
	}

private:
	char		mBZeroMotionFrame[188];
	size_t 		mBZMFrameSize;
	char		mPZeroMotionFrame[188];
	size_t 		mPZMFrameSize;
	bool 		mbZeroMotionDataSent;
};

class C2Session : public ZQ::common::Mutex , public ZQ::common::SharedObject {
public:
    typedef ZQ::common::Pointer<C2Session> Ptr;
	C2Session( C2StreamerEnv& env , C2Service& svc ,const std::string& sessId );
	virtual ~C2Session(void);

public:
	
	///process session request such as init term status transfer
	int32					processRequest( RequestParamPtr request , RequestResponseParamPtr response );	

	//destroy session and release all allocated resources
	void					destroy();

	void					markAsConnBroken();

	// check file data
	// can only invoke this method in TransferSession state
	//int 					checkFileData(const SessionTransferParamPtr request,bool& cacheAble, SessionTransferResponseParamPtr response );
	int 					checkFileData(const RequestParamPtr  request,bool& cacheAble, RequestResponseParamPtr response,int64& filePos );
	//int         			checkFileAttr(const SessionTransferParamPtr request,bool& cacheAble, SessionTransferResponseParamPtr response, const BufferUser& bu);
	int         			checkFileAttr(const RequestParamPtr request,bool& cacheAble,  RequestResponseParamPtr response, const BufferUser& bu, int64& filePos);
	//void  					setResponse(int lastError, SessionTransferResponseParamPtr response, const SessionTransferParamPtr request, bool cacheable );
	void  					setResponse(int lastError, RequestResponseParamPtr response, const RequestParamPtr request, bool cacheable );

	void					onTimer( );

	const std::string&		getSessId( ) const;
	
	int64					getReservedBandwidth( ) const;

	SessionState			getState( ) const;

	std::string				getFileName( ) const;

	int64					getBytesTransfered( ) const;

	std::string				getTransferServerAddress( ) const;
	
	std::string				getTransferClientAddress( ) const;

	unsigned short          getTransferClientPort() const;
	
	std::string				getTransferPortName( ) const;

	int64					getTimeInState( ) const;
	
	int64					getTransferRate( ) const;

	/// start transfer file content data to client
	bool					startTransfer( );
	
	void					changeState( const SessionState& targetState ,bool bUpdateTimer = true );

	void					reportUdpSessionStateChange( int64 subSessId, UdpSessionState oldState, UdpSessionState newState, int64 timeOffset );
	void					reportUdpSessionScaleChange( int64 subSessId, float oldScale, float newScale, int64 timeOffset );

	void					reportUdpSessionDone( int64 subSessId );

	
	void					updateTimer( int64 timeInterval );
	void					cancelTimer();

	void					updateAsyncTimer( int64 interval );
	void					cancelAsyncTimer( );

	void                    updateStatTimer(int64 interval);
	void                    cancelStatTimer();

	int64 					asyncTimerTarget() const;

	int64                   getSessionDuration() const;

	int64                   getSessionBitrate() const;

	float                   getSessionScale() const;

	void					setRequestType( int32 type );

	void					onRunnerDataGotten( bool bCheckFileEnd );

	void 					onDataReadTimedout();

	void					onRunnerWritable( );

	void					onAsyncTimer();
	
	void                    onStatDumpTimer();

	void					onAsyncStopRunner( bool bDestroied);

	void                	postReachFileEnd( AssetAttribute::Ptr attr);

	void					updateLastError( RequestParamPtr request , RequestResponseParamPtr response , int errorCode , const char* fmt ,... );

	void                    startStatDumpTimer();

	DataReadCallback::Ptr	getDataReadCb() const {
		return mDataReadCb;
	}

	DataReadCallback::Ptr	getReachFileEndCb() const {
		return mReachFileEndCb;
	}

	HttpWritableCallback::Ptr getWritableCb() const {
		return mWritableCb;
	}

	C2SessionAsyncTimer::Ptr	getAsyncTimerCb() const {
		return mAsyncTimer;
	}

	SessionStopRunnerAsyncWork::Ptr	getSessionStopRunnerAsyncWorkCb() const {
		return mAsyncStopRunner;
	}

	int					getSessionType() const {
		return mSessionType;
	}

	void 				onDataRunnerStopped( int64 runnerSessionId , int version);

	struct StatInfo
	{
		std::string   assetName;
		std::string   destination;
		std::string   streamingPort;
		std::string   streamingIP;
		std::string   streamType;
		std::string   OnDemandSession;
		const char*   stat;
		int64         bitrate;
		int64         byteOffset;
		int64         timeOffset;
		int64         duration;
		int64         updatetime;//update it when update sess
		float         scale;
		StatInfo()
			:byteOffset(0),
			timeOffset(0),
			duration(0),
			updatetime(0),
			bitrate(0),
			scale(0.0){}
	};
	
	void 	parseTsPacket(unsigned char* data, size_t len, TsParser::FrameStat& stat);

protected:
	//friend class SessionDataRunner;

	void					postDestroy();

	int32					processTransferInit( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response );
	//init udp session 
	int32                   processUdpInit( const UdpStreamInitRequestParamPtr request , UdpStreamInitResponseParamPtr response );
	
	int32					processTransferTerm( const TransferTermRequestParamPtr request , TransferTermResponseParamPtr response );

	int32					processTransferSession( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response );

	int32                   processUdpSession( const SessionUdpRequestParamPtr request , SessionUdpResponseParamPtr response );

	int32					processSessionStatus( const SessionStatusRequestParamPtr request , SessionStatusResponseParamPtr response );

	int32					processSessionUdpControl( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response);
	
protected:
	//int32					registerSession( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response );

	int32					registerShadowSession( const TransferInitRequestParamPtr request, TransferInitResponseParamPtr response);
	
	int32					registerSession( const RequestParamPtr request , RequestResponseParamPtr response );
	
	void					unregisterSession( );
	
	int32					checkState( SessionState targetState );
	
	void					runSession( int64 timeshift );

	///detect if we reach the of the session
	bool					reachSessionEnd( ) const;

	SessionDataRunner::Ptr	getCurrentDataRunner( ) const {
		return mCurrentDataRunner;
	}

	void 					clearAllRunners( );

	int32					udpLoad( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response );

	int32					udpPlay( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response );

	int32					udpPause( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response );

	int32					udpUnload( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response );

	int32					udpInfoQuery( const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response );

	int32	                parseUdpRequest( const SessionUdpControlRequestParamPtr request,  SessionUdpControlResponseParamPtr response, bool tobeLoad);

	bool					isAboutToChange( const SessionUdpControlRequestParamPtr request ) const;

	enum UdpRequestVerb {
		UDP_CHANGE_NONE = 0,
		UDP_CHANGE_OFFSET = 1 << 0,
		UDP_CHANGE_SCALE = 1 << 1,
		UDP_CHANGE_STATE = 1 << 2
	};

	int 					detectUdpRequestVerb( const SessionUdpControlRequestParamPtr request, const SessionUdpControlResponseParamPtr response ) const;


	void					stopUdpSessionRunner( SessionDataRunner::Ptr runner );
	void					runCurrentSessionRunner( );

	SessionDataRunner::Ptr makeShadowUdpSessionRunner( SessionDataRunner::Ptr from , SessionUdpControlResponseParamPtr response, bool pauseRunner = false);
	
protected:
	
	C2StreamerEnv&			mEnv;	
	C2Service&				mSvc;

	std::string				mSessionId;			// transferId as well

	unsigned short          mClientTransferport;//client transfer port
	std::string				mClientTransferAddress;	// client transfer address
	std::string				mServerTransferAddress;	// server transfer address
	std::string				mServerTransferPortName;

	std::string				mFileName;			// full path file name of content which is used to be pumped
	std::string				mRequestFileName;	// requested file name, not the full path file name

	std::string				mContentName;
	std::set<std::string>	mRequestFileExts;	// for file set downloading, this is a request file name, not full path file name
												// only valid if requesType = SESSION_TYPE_COMPLEX
	
	UdpSessionState			mUdpSessionState;
	int64					mSessionTransferRate; //FIXME: to be fixed
	
	
	int32					mTimeoutInterval;	//	
	int64					mStartTimeInState;	// since when current session is in current state

	SessionState			mSessionState;		// session state	

	C2ResponseHandler::Ptr	mResponseHandler; // used for DataRunner intialize
	
	SessionDataRunner::Ptr	mCurrentDataRunner;
	SessionDataRunner::Ptr	mShadowDataRunner; // for reposition use
	std::vector<SessionDataRunner::Ptr> 	mBackupRunners;

	IndexDataReader::Ptr	mIndexDataReader;
	
	int64					mNextWakeUpTime;

	int32					mSessionType;// 0 -normal session, 1 -SeaChange request session

	bool					mbMainFile;	// only main file's off set can be adjusted by SeekIFrame
	std::string				mIndexFilePathname;

	DataReadCallback::Ptr		mDataReadCb;
	DataReadCallback::Ptr		mReachFileEndCb;
	HttpWritableCallback::Ptr 	mWritableCb;
	C2SessionAsyncTimer::Ptr	mAsyncTimer;
	C2SessionStatTimer::Ptr     mSessStatTimer;
	SessionStopRunnerAsyncWork::Ptr	mAsyncStopRunner;

	const ConfWriter*			mConfWriter;
	int							mReaderType;

	TsParser					mTsParser;
};

typedef ZQ::common::Pointer<C2Session> C2SessionPtr;


//class TransferInitCallbackForLoop
class TransferInitCallbackForLoop : public LibAsync::AsyncWork, public IAsyncNotifySinker{
public:
	  typedef ZQ::common::Pointer<TransferInitCallbackForLoop> Ptr;
	  TransferInitCallbackForLoop(C2StreamerEnv&	 env, C2Session::Ptr sess, LibAsync::EventLoop* loop, TransferInitResponseParamPtr response, AssetAttribute::Ptr attr);
	  virtual ~TransferInitCallbackForLoop();
protected:
	  virtual void onNotified();
	  virtual void onAsyncWork();
private:
	  C2StreamerEnv&				mEnv;
	  C2Session::Ptr   mC2Session;
	  TransferInitResponseParamPtr mResponse;
	  AssetAttribute::Ptr                   mAttr;
};

//class TransferInitCallbackForOther
class TransferInitCallbackForOther : public IAsyncNotifySinker{
public:
	  typedef ZQ::common::Pointer<TransferInitCallbackForOther> Ptr;
	  TransferInitCallbackForOther(C2StreamerEnv&	 env, C2Session::Ptr sess, AssetAttribute::Ptr attr);
	  virtual ~TransferInitCallbackForOther();
	  void   transferInitWait() {
			mSemWait.wait();
	  }
protected:
	  virtual  void onNotified();
private:
	  C2StreamerEnv&							mEnv;
	  ZQ::common::Semaphore  			        mSemWait;
	  C2Session::Ptr                            mC2Session;
	  AssetAttribute::Ptr               	    mAttr;
};


//class TransferSessionCallbackForBuf
class TransferSessionCallbackForBuf : public LibAsync::AsyncWork, public IAsyncNotifySinker{
public:
	  typedef ZQ::common::Pointer<TransferSessionCallbackForBuf>   Ptr;
	  TransferSessionCallbackForBuf(C2StreamerEnv&	 env, C2Session::Ptr sess,LibAsync::EventLoop* loop, RequestParamPtr request, RequestResponseParamPtr response, BufferUser us);
	  virtual ~TransferSessionCallbackForBuf();
protected:
	  virtual void onNotified();
	  virtual void onAsyncWork();
private:
	  C2StreamerEnv&				mEnv;
	  C2Session::Ptr   mC2Session;
	  //SessionTransferResponseParamPtr mResponse;
	  RequestResponseParamPtr           mResponse;
	  RequestParamPtr                   mRequest;
	  BufferUser                               mBufUser;
	  //SessionTransferParamPtr         mRequest;
};

//class TransferSessionCallbackForAtt
class TransferSessionCallbackForAtt : public LibAsync::AsyncWork, public IAsyncNotifySinker{
public:
	  typedef   ZQ::common::Pointer<TransferSessionCallbackForAtt> Ptr;
	  TransferSessionCallbackForAtt(C2StreamerEnv&	 env, C2Session::Ptr sess, LibAsync::EventLoop* loop, RequestParamPtr request, RequestResponseParamPtr response, AssetAttribute::Ptr attr);
	  virtual ~TransferSessionCallbackForAtt();
protected:
	  virtual void onNotified();
	  virtual void onAsyncWork();
private:
	  C2StreamerEnv&				mEnv;
	  C2Session::Ptr   mC2Session;
	  //SessionTransferResponseParamPtr mResponse;
		
	  RequestResponseParamPtr           mResponse;
	  RequestParamPtr                   mRequest;
	  AssetAttribute::Ptr                           mAttr;
	  //SessionTransferParamPtr          mRequest;
};

inline bool floatEqual(float a, float b) {
	return fabs(a-b) < 0.01f;
}

}//namespace C2Streamer

#endif//_tianshan_cdnss_c2streamer_header_file_h__
