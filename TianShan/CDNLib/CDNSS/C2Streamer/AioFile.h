
#ifndef _cdnss_c2streamer_aio_file_header_file_h__
#define _cdnss_c2streamer_aio_file_header_file_h__

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>


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


#undef max
#undef min



namespace C2Streamer
{

class C2StreamerEnv;
	
class Notifier: public ZQ::common::SharedObject {
public:
	Notifier( );
	~Notifier( );
	void	broadcast( );
	bool	timedwait( uint64 ms = (uint64)-1 );
private:
	Notifier( const Notifier& );
	Notifier& operator=( const Notifier& );
private:
	boost::mutex		mLocker;
	boost::condition_variable mCond;
	bool				mbSignalled;
};
typedef ZQ::common::Pointer<Notifier> NotifierPtr;

class AssetAttribute : public ZQ::common::SharedObject {
public:

	  enum LASTERR{
			ASSET_SUCC    	= 0,
			ASSET_HTTP      = 1,
			ASSET_SOCKET	= 2,
			ASSET_TIMEOUT   = 3,
			ASSET_DATAERR 	= 4,
	  };

	typedef ZQ::common::Pointer<AssetAttribute>	Ptr;
	
	AssetAttribute(C2StreamerEnv& env, const std::string& filename)
	:mEnv(env),
	mFileName(filename),
	mOpenForWrite(false),
	mRangeStart(-1),
	mRangeEnd(-1),
	mTimestamp(0),
	mLastError(0){
		mNotifier = new Notifier();
	}

	const std::string& filename() const { return mFileName; }

	void			lastError( int error ) { 
		mLastError = error;
	}

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

	void			wait() { mNotifier->timedwait(); }
	void			signal() { mTimestamp = ZQ::common::now(); mNotifier->broadcast(); }

	bool			inprogress( ) const { return !mNotifier->timedwait(0); }

	bool			expired() const;

private:
	C2StreamerEnv&		mEnv;
	std::string			mFileName;
	bool				mOpenForWrite;
	int64				mRangeStart;
	int64				mRangeEnd;
	std::string			mAssetBaseInfo;
	std::string 		mAssetMemberInfo;

	int64				mTimestamp;

	int					mLastError;

	NotifierPtr			mNotifier;
};



class Buffer;


class IDataReader :virtual public ZQ::common::SharedObject {
public:
	typedef ZQ::common::Pointer<IDataReader> Ptr;
    virtual ~IDataReader( ) { }
    virtual bool read( const std::vector<Buffer*>& bufs ) = 0;
	virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) = 0;
};

class IReaderCallback {
public:
    virtual ~IReaderCallback() { }
    virtual void onRead( const std::vector<Buffer*>& bufs ) = 0;
    virtual void onLatency(std::string& fileName, int64 offset,int64 time) = 0; 
     virtual void onError( int err ) = 0;
	virtual void onIndexInfo( AssetAttribute::Ptr attr ) = 0;
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
	virtual bool read( const std::vector<Buffer*>& bufs );
	virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) {
		return true;// do nothing in AioClient
	}

	void requestComplete( struct io_event* evts, size_t count );
	void requestComplete( const std::vector<Buffer*>& bufs ) ;
	//void  bufReady(Buffer* buf);
	bool initClient();
	void unInitClient();

protected:
	  AioRunner*	getRunner( const std::string& filename );


private:
	IReaderCallback*      _cacheCallBack;
	C2StreamerEnv&        mEnv;

	std::vector<AioRunner*> mRunners;

};

class CacheCenter;

class Buffer {
public:
	explicit Buffer( CacheCenter& cc , char* addr, size_t size, size_t id);
	~Buffer();

	enum ErrorCategory {
		ECATE_SUCC      = 0,
		ECATE_FILEIO    = 1,
		ECATE_HTTP      = 2,
		ECATE_SOCKET	= 3,
		ECATE_TIMEOUT   = 4, //server timed out while sending data
		ECATE_CLIENTTIMEOUT = 5 //timed out while waiting for new buffer
	};

	size_t	id() const { return mId;}

	bool	vacant( ) const { return mNext == this && mPrev == this;}

	int		users( ) {return mUsers.get(); }
		 
	void	setDataSize( size_t size  ) { mSize = size; }
	size_t  getDataSize() {return mSize;}

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

	size_t	bufSize( ) const { return mBufSize; }

	inline const std::string& filename() const { return mFileName; }

    inline uint64 offsetInFile( ) const{ return mOrigOffset; }
    void	attach( const std::string& filename, int fd,  uint64 offsetInFile );

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
	bool	wait( uint64 delta = -1 );


	void	get();
	void	put();

	char*	data();

	inline Buffer*& next() { return mNext; }

	inline Buffer*& prev() { return mPrev; }

	void	seek( size_t offset );
	
	int64	bufferPrepareTime( ) const {
		if( mRespTimeStamp <= mReqTimeStamp )
			return 0;
		return mRespTimeStamp - mReqTimeStamp;
	}

	int64	readyTimeStamp( ) const  {
		return mRespTimeStamp;
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
	CacheCenter&	mCc;
	size_t 			mId;
	NotifierPtr		mN;
	char*			mBuf;
	size_t			mBufSize;
	size_t			mSize;
	int 			mLastError;
	ZQ::common::AtomicInt	mUsers;
	Buffer*			mNext;
	Buffer*			mPrev;
	int				mFd;//used in AioRunner
	uint64			mOrigOffset;
	std::string		mFileName;
	int64			mReqTimeStamp;
	int64			mRespTimeStamp;

	int64 			mTimeoutTarget;

	Buffer*			mExpireNext;
	Buffer*			mExpirePrev;
	bool             mBuffStat; // 0,not using 1,buff is using to recv data
	bool			mbCached;
//public:
//	int            reqCount;
//	int            resCount;
};

class BufferUser {
public:
	BufferUser( CacheCenter& cc, Buffer* buf );
	BufferUser( const BufferUser& );
	BufferUser& operator=( const BufferUser& );
	~BufferUser( );

	bool		valid( ) const;
	
	size_t		seek( size_t offset );// seek from beginning

	size_t		tell( ) const;

	char*		data( );

	size_t		dataLeft( ) const;

	size_t		advance( size_t dataSize );

	size_t		bufferSize( ) const { return mBuf->mBufSize; }

	uint64		bufferPrepareTime( ) const { return mBuf->bufferPrepareTime(); }

	bool		wait( ) { if(!mBuf) return false; return mBuf->wait(); }

	uint64		offsetInFile( ) const {
		return mBuf->offsetInFile();
	}

	bool 		dataFullInBuffer( ) const {
		return mBuf->mSize == mBuf->mBufSize;
	}

	int 		lastError() const {
		return mBuf->lastError();
	}
private:
	CacheCenter&	mCc;
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

	/**
	 * read 函数返回一个BufferUser
	 * 当获得返回值以后，首先需要检查BufferUser.valid().如果为false，表示当前的请求已经提交。但是还没有收到结果?

	 * 如果返回为true,那么就可以使用BufferUser来读取数据了
	 * */
	BufferUser		read( uint64 offset, size_t size , bool preRead = false);

	void			invalidateCache( uint64 offset, size_t size );

private:
	long			open( const std::string& filename );
	long			clear( );
	long			newRequest( long count );
	long			requestDone( long count );
private:
	C2StreamerEnv&	mEnv;
	CacheCenter&	mCc;
	int				mFd;
	std::string		mFileName;
	long			mUsers;
	long			mOutstandingReqs;

	friend class CacheCenter;
private:
	AioFile( const AioFile& );
	AioFile& operator=( const AioFile&);
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
private:
	int			run( );

private:
	C2StreamerEnv&			mEnv;
	//CacheCenter&			mCc;
	AioClient::Ptr          mClientPtr;
	io_context_t			mIoCtx;
	bool					mbRunning;
};

class AioRunner : public ZQ::common::NativeThread{
public:
	  AioRunner( C2StreamerEnv& env, AioClient::Ptr ptr);
	virtual ~AioRunner();
	size_t	pushBuffer( Buffer* b );
	size_t	pushBuffer( const std::vector<Buffer*>& bufs );

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
	typedef std::list<Buffer*> BufferList;
	size_t				mBufferCount;
	BufferList 			mList;
	boost::mutex 		mLocker;
	boost::condition_variable mCond;
	bool				mbSignalled;
	AioEventReaper		mReaper;
};

class CacheCenter : public IReaderCallback  {
public:
	CacheCenter( C2StreamerEnv& env );
	virtual ~CacheCenter( );

	bool		start( size_t bufferSize = 2 * 1024 * 1024 , size_t defaultBufferCount = 100 , size_t delta = 10);

	void		stop( );

	AioFile*	open( const std::string& filename );

	void		close( AioFile* );

	void		queryAssetAttributes(const std::string& filename, AssetAttribute::Ptr attr );

	BufferUser	makeRequest( AioFile* file, uint64 offset, size_t size, bool preRead );
	BufferUser	makeRequest( AioFile* file, uint64 offset, size_t size, int);
	BufferUser	findBuffer( AioFile* file, uint64 offset );

	inline size_t getBufferSize( ) const { return mBufferSize; }

	void		evictCache( const std::string& filename, uint64 offset, size_t size );

	void		chainBuffer( Buffer* buf);

	void		unchainBuffer( Buffer* buf);

	void		refBuffer( Buffer* buf);

	void		unrefBuffer( Buffer* buf);

	//void		requestComplete( struct io_event*evts, size_t count );
	virtual     void    onRead( const std::vector<Buffer*>& bufs );
	virtual     void    onLatency(std::string& fileName, int64 offset,int64 time); 
	virtual     void    onError( int err );
	virtual 	void 	onIndexInfo( AssetAttribute::Ptr attr );
private:
	friend class ExpireChecker;
	void		innerEvictCache( const std::string& filename, uint64 offset );
	void		requestComplete( const std::vector<Buffer*>& bufs );

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

	typedef std::map< std::string, AioFile* >	FILEINFOMAP;
	//typedef std::map< RangeInfo, Buffer* >		CACHEDMAP;
	typedef boost::unordered_map<RangeInfo,Buffer*,RangeInfo_hash> CACHEDMAP;

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

protected:

	void		invalidateCache( Buffer* buf);

	//AioRunner*	getRunner( const std::string& filename );

private:
	C2StreamerEnv&	mEnv;

	int				mEventFd;
	bool			mbRunning;
	size_t			mBufferSize;
	size_t			mBufferCount;
	size_t			mDefaultBufferCount;
	size_t			mBufferCountDelta;
	FILEINFOMAP		mFileInfos;
	CACHEDMAP		mCachedData;
	Buffer*			mHead; // free list header
	size_t			mFreeBufferCount;
	size_t			mOutStandingIoReqCount;

	ZQ::common::Mutex	mLocker;

	ExpireChecker	mExpireChecker;
	IDataReader::Ptr     mReaderPtr;
	size_t			mBufferId;
};

}//namespace C2Streamer

#endif//_cdnss_c2streamer_aio_file_header_file_h__


