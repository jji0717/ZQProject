
#ifndef _cdnss_c2streamer_aio_file_header_file_h__
#define _cdnss_c2streamer_aio_file_header_file_h__

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <ZQ_common_conf.h>
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <libaio.h>
#include <assert.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <Locks.h>
#include <NativeThread.h>
#include <Pointer.h>
#include <TimeUtil.h>
#include "C2StreamerLib.h"
#include "../IndexFileParser.h"
#include "../IdxFileParserEnvironment.h"
#include <LRUMap.h>
#undef max
#undef min

namespace C2Streamer
{
class C2StreamerEnv;
class BufferUser;

class CacheCenter;

class IAsyncNotifySinker: virtual public ZQ::common::SharedObject {
public:
	typedef ZQ::common::Pointer<IAsyncNotifySinker> Ptr;
	virtual ~IAsyncNotifySinker(){}
	virtual void onNotified( ) = 0;
};

class IReaderCallback;

class AssetAttribute : public ZQ::common::SharedObject {
public:

	  enum LASTERR{
			ASSET_SUCC    	= 0,
			ASSET_HTTP      = 1,
			ASSET_SOCKET	= 2,
			ASSET_TIMEOUT   = 3,
			ASSET_DATAERR 	= 4,
			ASSET_NOTFOUND  = 5,
	  };

	typedef ZQ::common::Pointer<AssetAttribute>	Ptr;
	
	AssetAttribute(C2StreamerEnv& env, const std::string& filename)
	:mEnv(env),
	mFileName(filename),
	mOpenForWrite(false),
	mRangeStart(-1),
	mRangeEnd(-1),
	mTimestamp(0),
	mbSignalled(false),
	mLastError(0),
	mReqId(0),
	mCurrentReaderType(-1),
	mSuggestedReaderType(-1),
	mReaderCB(NULL) {
	}

	const std::string& filename() const { return mFileName; }

	void			lastError( int error ) { 
		mLastError = error;
	}

	const std::string&	sessionId() const { return mSessId; }

	void 			sessionId(const std::string& sessId) { mSessId = sessId; }

	int64			reqId() const { return mReqId; }

	void			reqId( int64 id ) { mReqId = id; }

	int				lastError() const { return mLastError; }

	bool			pwe( ) const { return mOpenForWrite; }
	void			pwe( bool bPwe ) { mOpenForWrite = bPwe; }

	void			range( int64 rangeStart, int64 rangeEnd) {
		mRangeStart = rangeStart;
		mRangeEnd = rangeEnd;
	}
	int64			rangeStart( ) const { return mRangeStart; }
	int64			rangeEnd( ) const { return mRangeEnd; }

	void			assetBaseInfo( const std::string& info ) { mAssetBaseInfo = info; }
	const std::string& assetBaseInfo() const { return mAssetBaseInfo; }

	void			assetMemberInfo( const std::string& info ) { mAssetMemberInfo = info; }
	const std::string& assetMemberInfo() const { return mAssetMemberInfo; }

	bool			asyncWait( IAsyncNotifySinker::Ptr sinker );


	void			signal();

	bool			inprogress( ) const;

	bool			expired() const;

	void			currentReaderType( int typ ) {
		mCurrentReaderType = typ;
	}

	int 			currentReaderType() const {
		return mCurrentReaderType;
	}

	void			suggestReaderType(int typ ) {
		mSuggestedReaderType = typ;
	}

	int				suggestedReaderType() const {
		return mSuggestedReaderType;
	}

	void			setReaderCallback( IReaderCallback* cb ) {
		mReaderCB = cb;
	}

	IReaderCallback*	getReaderCallback() {
		return mReaderCB;
	}

private:

	C2StreamerEnv&		mEnv;
	std::string			mFileName;
	bool				mOpenForWrite;
	int64				mRangeStart;
	int64				mRangeEnd;
	std::string			mAssetBaseInfo;
	std::string 		mAssetMemberInfo;

	int64				mTimestamp;
	bool				mbSignalled;

	int					mLastError;
	typedef std::vector<IAsyncNotifySinker::Ptr> NOTIFYSINKERS;
	NOTIFYSINKERS		mSinkers;
	ZQ::common::Mutex	mLocker;

	int64				mReqId;

	int					mCurrentReaderType;
	// if mReaderType is hybrid, this mSuggestedReaderType is the result we
	// query aqua or c2
	int					mSuggestedReaderType;

	// if mReaderCB is not nil
	IReaderCallback*	mReaderCB;

	std::string 		mSessId;
};



class Buffer;
class BufferUser;

class IDataReader :virtual public ZQ::common::SharedObject {
public:
	typedef ZQ::common::Pointer<IDataReader> Ptr;
    virtual ~IDataReader( ) { }
    virtual bool read( const std::vector<BufferUser>& bufs ) = 0;
	virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) = 0;
};

class IReaderCallback {
public:
    virtual ~IReaderCallback() { }
    virtual void onRead( const std::vector<BufferUser>& bufs, bool fromDiskCache = false ) = 0;
    virtual void onLatency(std::string& fileName, int64 offset,int64 time) = 0; 
    virtual void onError( int err ) = 0;
	virtual void onIndexInfo( AssetAttribute::Ptr attr ) = 0;
};

class HybridReader : public IDataReader, public IReaderCallback, public virtual ZQ::common::SharedObject {
public:
	typedef ZQ::common::Pointer<HybridReader> Ptr;
	HybridReader(CacheCenter& cc);
	virtual ~HybridReader();

	bool pushReaderType( int typ );

protected:
	virtual bool read( const std::vector<BufferUser>& bufs );
	virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr );
	virtual void onRead( const std::vector<BufferUser>& bufs, bool fromDiskCache = false );
    virtual void onLatency(std::string& fileName, int64 offset,int64 time);
    virtual void onError( int err );
	virtual void onIndexInfo( AssetAttribute::Ptr attr );

private:

	int 	findNextReader( int reader ) const;

private:
	C2StreamerEnv&	mEnv;
	CacheCenter& 	mCc;

	struct ReaderSlot {
		int			current;
		int 		next;
		ReaderSlot():
		current(-1),
		next(-1) {
		}
	};

	std::vector<ReaderSlot>	mReaders;
};

// -----------------------------
// // callback DiskCacheSink
// // -----------------------------
// // this class should be changed to take HongQuan's aio callback
class DiskCacheSink
{
	public:
		typedef enum _Error
		{
			cacheErr_OK,
			cacheErr_Hit,        // the requested file is in the cache and the cached content has been read successfully
			cacheErr_Missed,     // the requested file is not in the cache
			cacheErr_SwapMissed, // the request file is in the cache but it doesn't match the request url due to conflict hash key
			cacheErr_Timeout,
			cacheErr_StorePending,
			cacheErr_StoreFail
		} Error;

		static const char* errorToA(Error err);

	public:
		// errCode takes those of DiskCache::Error
		virtual void onCacheRead(Error errCode, BufferUser buf) =0;

		// errCode : cacheErr_OK - wrote success, cacheErr_Hit - cache already exists, cacheErr_Timeout - writting timeout
		virtual void onCacheWrote(Error errCode/*, const std::string& url*/) =0;

		// get buffer from cachecenter to read data
		virtual BufferUser  getCacheBufUser(const std::string& fileName, const int64 stampAsOfOrign, int64 bitrate, long opaqueData) = 0;

		//get a empty bufferuser
		virtual BufferUser  makeEmptyBufUser() = 0;
};

class Buffer;
class AioRunner;
///////////////////////////////////
//class AioClient to manager AioRunners
class AioClient : public IDataReader , virtual public ZQ::common::SharedObject{
public:
	typedef ZQ::common::Pointer<AioClient> Ptr;
	AioClient(IReaderCallback*pCallBack, C2StreamerEnv& env);
	virtual ~AioClient();
public:
	virtual bool read( const std::vector<BufferUser>& bufs );
	virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) {
		return true;// do nothing in AioClient
	}

	void requestComplete( struct io_event* evts, size_t count, bool cancelled = false );
	void requestComplete( const std::vector<BufferUser>& bufs ) ;
	//void  bufReady(Buffer* buf);
	bool initClient();
	void unInitClient();

protected:
	  AioRunner*	getRunner( const std::string& filename );


private:
	IReaderCallback*		_cacheCallBack;
	C2StreamerEnv&			mEnv;

	std::vector<AioRunner*>	mRunners;
	ZQ::common::Mutex		mLocker;
};


class Buffer {
public:
	explicit Buffer( CacheCenter& cc , char* addr, size_t size, size_t id, bool isPauseFrameData = false);
	~Buffer();

	enum ErrorCategory {
		ECATE_SUCC      = 0,
		ECATE_FILEIO    = 1,
		ECATE_HTTP      = 2,
		ECATE_SOCKET	= 3,
		ECATE_TIMEOUT   = 4, //server timed out while sending data
		ECATE_CLIENTTIMEOUT = 5, //timed out while waiting for new buffer
		ECATE_NOTFOUND = 6
	};

	size_t	id() const { return mId;}

	int		readerType( ) const {
		return mReaderType;
	}

	void	readerType( int reader )  {
		mReaderType = reader;
	}

	const std::string& sessionId() const { return mSessId; }

	void 	sessionId(const std::string& sessId) { mSessId = sessId; }

	int64	reqId() const { return mReqId; }

	void	reqId( int64 reqId) { mReqId = reqId; }

	bool	vacant( ) const { return mNext == this && mPrev == this;}

	int		users( ) {return mUsers.get(); }
		 
	void	setDataSize( size_t size  );
	size_t  getDataSize() {return mSize.get(); }

	bool	filled() const { return mbFilled; }

	void 	setLastError( int error, ErrorCategory category ) { 
		error = error > 0 ? error : -error;
		if(error == 0 ) {
			mLastError = 0 ;
		}  else {
			mLastError = ((int)category) << 28 | error;
		}
	}

	int		lastError() const { return mLastError; }

	char*	buffer() { return mBuf; }
	const char* buffer() const { return mBuf; }

	size_t	bufSize( ) const { return mBufSize; }

	inline const std::string& filename() const { return mFileName; }

    inline uint64 offsetInFile( ) const{ return mOrigOffset; }
    void	attach( const std::string& filename, int fd,  uint64 offsetInFile );

	int64	fileSize() const { return mFileSize; }

	void	fileSize( int64 size ) { mFileSize = size; }

	bool 	isPauseFrameData() const {
		return mbPauseFrameData;
	}

	void setBitrate(int64 bitrate);

	int64	getBitrate() const {
		return mBitrate;
	}

	int		errCategory() const {
		return (int)(mLastError>>28);
	}

	int 	errCode() const {
		return (int)(mLastError&0xFFFFFFF);
	}
	
	void	get();
	void	put();

private:
	Buffer( const Buffer& );
	Buffer& operator=( const Buffer&);

	friend class CacheCenter;
	friend class BufferUser;
	friend class ExpireChecker;
	friend class AioRunner;
	friend class AioEventReaper;

	void	reset( );
	
	/**
	 * 当Buffer里面的数据可用的时候，signal函数会被调用?

	 * 用以通知等待队列该buffer可用
	 * */
	void	signal( );
	void	request( );

	/**
	 * 在使用Buffer之前需要先调用wait, 用以确保Buffer内的数据可用
	 * */
	bool	asyncWait( IAsyncNotifySinker::Ptr sinker, size_t size );


	char*	data();

	inline Buffer*& next() { return mNext; }

	inline Buffer*& prev() { return mPrev; }

	inline Buffer*& flyNext() { return mFlyNext; }

	inline Buffer*& flyPrev() { return mFlyPrev; }

	void	seek( size_t offset );
	
	int64	bufferPrepareTime( ) const {
		if( mRespTimeStamp <= mReqTimeStamp )
			return 0;
		return mRespTimeStamp - mReqTimeStamp;
	}

	int64	readyTimeStamp( ) const  {
		return mRespTimeStamp;
	}

	int64	requestTimeStamp( ) const {
		return mReqTimeStamp;
	}

	void 	updateTimer(int64 delta );

	int64	timeoutTarget() const {
		return mTimeoutTarget;
	}

	inline bool inExpireChecker( ) const {
		return mExpireNext != this &&mExpirePrev != this;
	}

	inline void addToExpireChecker( Buffer* head ) {
		assert( !inExpireChecker() );
		Buffer* nodePrev = head->prevInExpireChecker();

		nodePrev->nextInExpireChecker() = this;
		this->nextInExpireChecker() = head;
		head->prevInExpireChecker() = this;
		this->prevInExpireChecker() = nodePrev;
	}

	inline void removeFromExpireChecker( ) {
		assert( inExpireChecker() );
		prevInExpireChecker()->nextInExpireChecker() = nextInExpireChecker();
		nextInExpireChecker()->prevInExpireChecker() = prevInExpireChecker();
		nextInExpireChecker() = this;
		prevInExpireChecker() = this;
	}

	inline Buffer*&	nextInExpireChecker() { return mExpireNext; }

	inline Buffer*& prevInExpireChecker() { return mExpirePrev; }


private:

	void	notify();

private:

	struct AsyncNotifyInfo {
		IAsyncNotifySinker::Ptr sinker;
		size_t size;
	};
	typedef std::vector<AsyncNotifyInfo> AsyncNotifyInfoS;

	CacheCenter&	mCc;
	size_t 			mId;
	//NotifierPtr		mN;
	char*			mBuf;
	size_t			mBufSize;
	ZQ::common::AtomicInt		mSize;
	bool			mbFilled;
	ZQ::common::Mutex	mLocker;
	AsyncNotifyInfoS	mNotifyInfos;

	int 			mLastError;
	ZQ::common::AtomicInt	mUsers;
	Buffer*			mNext;
	Buffer*			mPrev;

	Buffer*			mFlyNext;
	Buffer*			mFlyPrev;

	int				mFd;//used in AioRunner
	uint64			mOrigOffset;
	std::string		mFileName;
	int64			mReqTimeStamp;
	int64			mRespTimeStamp;
	int64 			mTimeoutTarget;

	int64			mFileSize;//

	Buffer*			mExpireNext;
	Buffer*			mExpirePrev;

	bool            mBuffStat;
	bool			mbCached;
	bool			mbReturnedFromDiskCache;

	int64			mReqId;
	int64			mBitrate;

	int 			mReaderType;

	bool 			mbPauseFrameData;

	std::string 	mSessId;
};

class BufferUser {
public:
	BufferUser();
	BufferUser( CacheCenter& cc, Buffer* buf );
	BufferUser( Buffer* buf );
	BufferUser( const BufferUser& );
	BufferUser& operator=( const BufferUser& );
	~BufferUser( );

	bool		valid( ) const {
		return mBuf != NULL ;
	}

	bool		filled() const;
	
	size_t		seek( size_t offset );// seek from beginning

	size_t		tell( ) const;

	char*		data( );
	char*   	getBuffer();
	
	size_t		dataLeft( ) const;

	size_t		advance( size_t dataSize );

	size_t		bufferSize( ) const { return mBuf->mBufSize; }
	size_t      dataSize( ) const { return mBuf->mSize.get();}
	void        dataSize(size_t size) { mBuf->setDataSize(size);}
	uint64		bufferPrepareTime( ) const { return mBuf->bufferPrepareTime(); }

	//bool		wait( ) { if(!mBuf) return false; return mBuf->wait(); }

	//return false if the condition is met
	bool		asyncWait( IAsyncNotifySinker::Ptr sinker , size_t size );

	uint64		offsetInFile( ) const {
		return mBuf->offsetInFile();
	}

	std::string fileName( ) const {
		return mBuf->mFileName;
	}

	int64		fileSize() const {
		return mBuf->fileSize();
	}

	bool 		dataFullInBuffer( ) const {
		return mBuf->mSize.get() == (int)mBuf->bufSize();
	}

	int 		lastError() const {
		return mBuf->lastError();
	}

	int64		bufReqId( ) const {
		if(!valid())
			return -1;
		return mBuf->reqId();
	}

	inline int64 reqId() const { return bufReqId(); }

	bool		copyFrom( BufferUser& rhs );


	Buffer* operator->() {
		assert(valid());
		return mBuf;
	}

	const Buffer* operator->() const {
		assert(valid());
		return mBuf;
	}

	Buffer*	getInner() {
		return mBuf;
	}

	int users( )
	{
		return mBuf->users( );
	}

	operator bool() const {
		return valid();
	}

private:
	void adjustBitrate(const Buffer* rhs);

private:
	friend class CacheCenter;
	CacheCenter*	mCc;
	Buffer*			mBuf;
	size_t			mOffset;
};

class AioFile {
public:
	AioFile( C2StreamerEnv& env, CacheCenter& cc );
	~AioFile( );
	inline const std::string& filename( ) const { return mFileName; }
	inline int fd( ) const { return mFd; }
	void			close( );

	BufferUser		read( uint64 offset, int64 bitrate = 0 );

	void			invalidateCache( uint64 offset );

	int 			readerType() const{
		return mReaderType;
	}

	inline bool 	firstRead() const {
		return mbFirstRead;
	}

	int64			getBitrate() const {
		ZQ::common::MutexGuard gd(mLocker);
		return mBitrate;
	}

	void			setBitrate( int64 bitrate );

	void 			sessionId(const std::string& sessId) {
		ZQ::common::MutexGuard gd(mLocker);
		if(mSessId.empty()) {//only record the first session Id
			mSessId = sessId;
		}
	}

	const std::string& sessionId() const {
		return mSessId;
	}

private:

	long			open( const std::string& filename, int readerType );
	long			clear( );
	long			newRequest( long count );
	long			requestDone( long count );

private:

	C2StreamerEnv&	mEnv;
	CacheCenter&	mCc;
	ZQ::common::Mutex mLocker;
	int				mFd;
	std::string		mFileName;
	long			mUsers;
	long			mOutstandingReqs;
	bool 			mbFirstRead;
	int64			mBitrate;
	std::string 	mSessId;

	friend class CacheCenter;

private:
	AioFile( const AioFile& );
	AioFile& operator=( const AioFile&);

	int 	mReaderType;
};

class ExpireChecker : public ZQ::common::NativeThread {
public:
	ExpireChecker( C2StreamerEnv& env, CacheCenter& cc );
	virtual ~ExpireChecker();
	bool		start( );
	void		stop( );
	void		add( Buffer* buf );
	void		remove( Buffer* buf );
private:
	int			run();
private:
	C2StreamerEnv&		mEnv;
	CacheCenter&		mCc;
	struct TimerBufferCompare {
		bool operator()( const Buffer* a, const Buffer* b) const {
			if( a->timeoutTarget() < b->timeoutTarget() )
				return true;
			else if ( a->timeoutTarget() == b->timeoutTarget() )
				return a < b;
			else
				return false;
		}
	};
	typedef std::set<Buffer*, TimerBufferCompare>	TimerSet;
	Buffer				mHead;
	TimerSet			mBuffers;
	boost::mutex		mLocker;
	boost::condition_variable mCond;
	bool 				mbRunning;
	int64 				mNextWakeup;
	bool 				mbSignalled;
};

class CacheCenter;

class AioEventReaper : public ZQ::common::NativeThread {
public:
	AioEventReaper( C2StreamerEnv& env, AioClient::Ptr ptr);
	virtual ~AioEventReaper( );
	bool		start( io_context_t* ctx );
	void		stop( );

	void 		addNewCB(struct iocb* cb);

	//after remove cb in info map, memory of cb will be freed
	void 		removeCB(struct iocb* cb);

private:

	int			run( );

private:
	C2StreamerEnv&			mEnv;
	//CacheCenter&			mCc;
	AioClient::Ptr          mClientPtr;
	io_context_t			mIoCtx;
	bool					mbRunning;
	ZQ::common::Mutex 		mLocker;
	struct CBTimeInfo {
		struct iocb *cb;
		int64 target;
		bool operator<(const struct CBTimeInfo& rhs) const {
			if(target == rhs.target) {
				return cb < rhs.cb;
			} else if( target < rhs.target) {
				return true;
			} else {
				return false;
			}
		}
	};
	typedef std::set<CBTimeInfo> CBINFOSET;
	typedef std::map<struct iocb*, CBTimeInfo> CB2INFOMAP;

	CB2INFOMAP				mCb2InfoMap;
	CBINFOSET				mCbInfoSet;
};


class AioRunner : public ZQ::common::NativeThread{
public:
	  AioRunner( C2StreamerEnv& env, AioClient::Ptr ptr);
	virtual ~AioRunner();
	size_t	pushBuffer( BufferUser b );
	size_t	pushBuffer( const std::vector<BufferUser>& bufs );

	bool	start( size_t groupCount );
	void	stop( );

private:

	int		run( );

private:
	C2StreamerEnv&		mEnv;
//	CacheCenter&		mCc;
	AioClient::Ptr     mClientPtr;
	io_context_t		mIoCtx;
	bool				mbRunning;
	typedef std::list<BufferUser> BufferList;
	size_t				mBufferCount;
	BufferList 			mList;
	boost::mutex 		mLocker;
	boost::condition_variable mCond;
	bool				mbSignalled;
	AioEventReaper		mReaper;
};

class IdxRecData : public ZQ::IdxParser::IndexRecord, public ZQ::common::SharedObject
{
private:
	boost::condition_variable mCond;
	boost::mutex        mboostLocker;
	bool                mbParsed ;
public:
	IdxRecData(){mbParsed = false;}
	void signal()
	{
		boost::mutex::scoped_lock gd(mboostLocker);
		mbParsed = true;
		mCond.notify_all();
	}
	void wait()
	{
		boost::mutex::scoped_lock gd(mboostLocker);
		if (!mbParsed)
		{
			mCond.wait(gd);
		//	mbParsed = false;
		}
	}
	bool IsParsed()
	{
		boost::mutex::scoped_lock gd(mboostLocker);
		return mbParsed;
	}
	void SetUnParsed()
	{
		boost::mutex::scoped_lock gd(mboostLocker);
		mbParsed = false;
	}
};
typedef ZQ::common::Pointer<IdxRecData> IdxRecPtr;

class CacheCenter : public IReaderCallback, public DiskCacheSink, public ZQ::common::NativeThread {
public:
	CacheCenter( C2StreamerEnv& env );
	virtual ~CacheCenter( );

	C2StreamerEnv& getEnv() {
		return mEnv;
	}

	int64		genReqId();

	void 		genReqIdForBuffer(Buffer* buf);

	bool		start( size_t bufferSize = 2 * 1024 * 1024 , size_t defaultBufferCount = 100 , size_t delta = 10);

	void		stop( );

	AioFile*	open( const std::string& filename, int readerType, const std::string& sessId );

	void		close( AioFile* );

	void		queryAssetAttributes(const std::string& filename, AssetAttribute::Ptr attr, int readerType);
	

	BufferUser	makeRequest( AioFile* file, uint64 offset, int readerType);
	BufferUser	makeRequest( AioFile* file, uint64 offset, int readerType, bool withDiskCache );
	void 		makeRequest( AioFile* file, std::vector<BufferUser>& bufs);
	BufferUser	findBuInPendingSet( const std::string& filename, int64 offset );
	void 		addToPendingSet( BufferUser& bu);
	void		removeFromPendingSet(BufferUser& buf);

	BufferUser findPauseFrame( const ConfWriter* conf);


	BufferUser	findBuffer( AioFile* file, uint64 offset );

	inline size_t getBufferSize( ) const { return mBufferSize; }

	void		evictCache( const std::string& filename, uint64 offset, size_t size );

	void		chainBuffer( Buffer* buf, bool firstAdd = false);

	void		unchainBuffer( Buffer* buf);

	void		refBuffer( Buffer* buf);

	void		unrefBuffer( Buffer* buf);

	//void		requestComplete( struct io_event*evts, size_t count );
	virtual     void    onRead( const std::vector<BufferUser>& bufs, bool fromDiskCache = false );
	virtual     void    onLatency(std::string& fileName, int64 offset,int64 time); 
	virtual     void    onError( int err );
	virtual 	void 	onIndexInfo( AssetAttribute::Ptr attr );

	//virtual fun for disk cache
	
	//errCode takes those of DiskCache::Error
	virtual void onCacheRead(Error errCode, BufferUser buf);

	// errCode : cacheErr_OK - wrote success, cacheErr_Hit - cache already exists, cacheErr_Timeout - writting timeout
	virtual void onCacheWrote(Error errCode/*, const std::string& url*/);

	// get buffer from cachecenter to read data
	virtual BufferUser getCacheBufUser(const std::string& fileName, const int64 stampAsOfOrign, int64 bitrate, long opaqueData);
	//make a empty bufferuser using in disk cache
	virtual BufferUser  makeEmptyBufUser();

	//test for check data 
	bool        checkData(BufferUser&  buf, const std::string& hint);

	void		getFlyBuffersStatus( std::vector<CacheBufferStatusInfo>& bufs ) const;
    
    const IdxRecPtr getIdxRecByIdxName(const std::string& filename,bool& bNew);


private:

	void 		markAsInfly( Buffer* buf);

	void		unmarkInFly( Buffer* buf,bool bFirstAdd);

	int 		run();

private:
	friend class ExpireChecker;
	void		innerEvictCache( const std::string& filename, uint64 offset );
	void		requestComplete( const std::vector<BufferUser>& bufs, bool fromDiskCache );

	struct RangeInfo {
		std::string		filename;
		uint64			offset;
		RangeInfo():offset(0){}

		bool operator<(const RangeInfo& b ) const {
			if( filename < b.filename )
				return true;
			else if(filename == b.filename )
				return offset < b.offset;
			else
				return false;
		}
		bool operator==( const RangeInfo& b ) const {
			return (filename == b.filename) && (offset == b.offset);
		}
	};

	struct RangeInfo_hash : public  std::unary_function<RangeInfo, std::size_t> {
		 std::size_t operator()( const RangeInfo& p ) const {
			 std::size_t seed = 0;
			 boost::hash_combine( seed, p.filename );
			 boost::hash_combine( seed, p.offset );
			 return seed;
		 }
	};

	struct AioFileAttr {
		std::string name;
		int readerType;
		bool operator<(const AioFileAttr& rhs) const {
			if( readerType == rhs.readerType ) {
				return name < rhs.name;
			} else {
				return readerType < rhs.readerType;
			}
		}
	};
	typedef std::map< AioFileAttr, AioFile* >	FILEINFOMAP;
	//typedef std::map< RangeInfo, Buffer* >		CACHEDMAP;
	typedef boost::unordered_map<RangeInfo,Buffer*,RangeInfo_hash> CACHEDMAP;
	typedef boost::unordered_map<RangeInfo,BufferUser, RangeInfo_hash> PENDINGMAP;


	bool		addBuffer( );
	/**
	 * 从freelist里面挑选一个buffer用于承载新的数据，如果没有free的buffer可用
	 * 那么就新创建一个，如果创建失败。那么就只好等待?

	 * */
	Buffer*		getBuffer( );
	void		putBuffer( Buffer* b);

	void		useBuffer( Buffer* buf );

	bool		listEmpty( ) const;

	void		invalidateCache( const std::string& filename, uint64 offset, Buffer* hint = NULL );

    IdxRecPtr findByLRUMap(const std::string& filename, bool& bNew);

protected:

	void		invalidateCache( Buffer* buf);

	//AioRunner*	getRunner( const std::string& filename );
	IDataReader::Ptr getReader( int type );

private:
	C2StreamerEnv&	mEnv;

	std::map<size_t,Buffer*>	mPauseFrames;

	int				mEventFd;
	bool			mbRunning;
	size_t			mBufferSize;
	size_t			mBufferCount;
	size_t			mDefaultBufferCount;
	size_t			mBufferCountDelta;
	FILEINFOMAP		mFileInfos;
	CACHEDMAP		mCachedData;
	PENDINGMAP 		mPendingData;
	Buffer*			mHead; // free list header
	size_t			mFreeBufferCount;
	size_t			mFlyBufferCount;
	size_t			mOutStandingIoReqCount;
	ZQ::common::Mutex	mLocker;

	ExpireChecker	mExpireChecker;
	
	//NOTICE, keep the readers sequence as defined in C2StreamerEnv.h
	std::vector<IDataReader::Ptr> mReaders;
	//IDataReader::Ptr     mReaderPtr;

	size_t			mBufferId;
	int64			mBufferReqIdBase;

    ZQ::common::LRUMap<std::string, IdxRecPtr> mIdxRecMap;
	const ConfCacheCenter*	mConfCacheCenter;
	typedef std::map<int64, Buffer*>	WatchMap;
	WatchMap		mWatchedReqs;
	ZQ::common::Semaphore mWatchReqSem;
};

}//namespace C2Streamer

#endif//_cdnss_c2streamer_aio_file_header_file_h__


