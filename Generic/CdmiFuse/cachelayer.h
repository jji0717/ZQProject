#ifndef __cache_layer_zone_header_file_h__
#define __cache_layer_zone_header_file_h__

#include <ZQ_common_conf.h>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <list>
#include <sstream>
#include <assert.h>
#include <errno.h>
#include <algorithm>
#include <boost/unordered_map.hpp>
#include <boost/thread/condition_variable.hpp>
#include <TimeUtil.h>
#include <Pointer.h>
#include <Locks.h>
#include <NativeThreadPool.h>
#include <LRUMap.h>

namespace CacheLayer {

struct DataBuffer {
	unsigned long long offset;// only used by write
	char*	buf;
	size_t  size;
	DataBuffer():offset(0), buf(0),size(0){}
	//sort the DataBuffer in a list
	bool operator<( const DataBuffer& rhs ) const {
		return offset < rhs.offset;
	}
};

class MutexUnlocker {
private:
	ZQ::common::Mutex&	mLocker;
	MutexUnlocker( const MutexUnlocker&);
	MutexUnlocker&operator=( const MutexUnlocker&);
public:
	MutexUnlocker( ZQ::common::Mutex& m):mLocker(m) {
		mLocker.leave();
	}
	~MutexUnlocker( ) {
		mLocker.enter();
	}
};

std::string dataBuffersToRangeStr( const std::vector<DataBuffer>& bufs, int* mergedCount = NULL );

class StopWatch {
private:
	long long 	mStart;
	long long 	mDelta;
	bool 		mbAuotoStart;
private:
	StopWatch& operator=( const StopWatch&);

	long long getusec( ) const
	{
#ifdef ZQ_OS_MSWIN
		FILETIME ft;
		::GetSystemTimeAsFileTime(&ft);
		uint64 ltime;
		memcpy(&ltime, &ft, sizeof(ltime));
		return ltime/10; //convert nsec to msec
#else
		struct timeval tv;
		gettimeofday( &tv, 0 );
		return ((long long)tv.tv_sec)* 1000 *1000 + (( long long) tv.tv_usec);
#endif
	}

public:
	StopWatch( bool autoStart  = true )
		:mStart(0),
		mDelta(0),
		mbAuotoStart(autoStart){
			if(autoStart)
				start();
	}

	StopWatch( const StopWatch& rhs):mDelta(0),mbAuotoStart(rhs.mbAuotoStart) {
		mStart = rhs.mStart;
	}
	long long operator-( const StopWatch& rhs ) const {
		return cost() - rhs.cost();
	}
	void		start( ) {
		mStart = getusec();
	}
	void		reset() {
		mDelta = 0;
	}
	long long 	stop( ) const {
		long long delta = getusec() - mStart;
		if( delta < 0 )
			delta = 0;
		const_cast<StopWatch*>(this)->mDelta += delta;
		return mDelta;
	}
	long long cost( ) const {
		if( mDelta == 0 && mbAuotoStart )
			return stop();
		return mDelta;
	}
};

class Sema
{
public:
	Sema( size_t initCount = 0);
	~Sema();
	void		post();
	void		wait();
	bool		timedWait( int64 interval );
protected:
	Sema( const Sema& );
	Sema& operator=( const Sema&);
#ifdef ZQ_OS_MSWIN
	HANDLE		mSema;
#elif defined ZQ_OS_LINUX
	sem_t		mSema;
#endif
};


class Notifier : public ZQ::common::SharedObject
{
public:
	Notifier();
	virtual ~Notifier();	

	void	broadcast();	
	void	wait( );
	void	reset( );
private:
	boost::condition_variable 	mCond;
	boost::mutex				mLocker;
	bool						mbSignalled;
};

typedef ZQ::common::Pointer<Notifier>	NotifierPtr;

struct Range
{
	unsigned long long	begin;
	unsigned int		size;
	
	Range():begin(0),size(0){
	}

	Range( unsigned long long b, unsigned int s ) 
		:begin(b), size(s) {
	}

	Range( const Range& r ):begin(r.begin),
		size(r.size) {
	}

	Range& operator=( const Range& r ) {
		begin = r.begin;
		size = r.size;
		return *this;
	}

	bool operator < ( const Range& d) const {
		return begin < d.begin;
	}

	bool operator > ( const Range& d) const {
		return begin > d.begin;
	}

	bool operator == ( const Range& d) const {
		return begin == d.begin;
	}

	void reset() {
		begin = size = 0 ;
	}

	inline
	unsigned long long start() const {
		return begin;
	}

	inline
	unsigned long long end() const {
		return begin + size;
	}

	std::string toString() const {
		std::ostringstream oss;
		oss<<begin<<"-"<< ( size>0 ? (begin + size -1) : 0 )<<"/"<<std::dec<<size;
		return oss.str();
	}

	bool legal( size_t blockSize ) const {
		return ( begin/blockSize*blockSize == begin ) && ( size / blockSize * blockSize == size ) ;
	}

	bool full( size_t blockSize ) const {
		return size == blockSize;
	}
};

struct MBMetaData {
	std::string		filePath;
	Range			dataRange;	
	MBMetaData(){
	}
	MBMetaData( const MBMetaData& info ):filePath(info.filePath),
	dataRange(info.dataRange){
	}
	MBMetaData& operator=( const MBMetaData& info )  {
		filePath = info.filePath;
		dataRange = info.dataRange;
		return *this;
	}
	bool operator==( const MBMetaData& info ) const {
		return ( info.dataRange == dataRange ) && ( info.filePath == filePath ) ;
	}
	bool isValid( ) const {
		return !filePath.empty();
	}
	void reset() {
		filePath.clear();
		dataRange.reset();
	}
	bool operator<( const MBMetaData& d ) const  {
		if( filePath == d.filePath )
			return dataRange < d.dataRange;
		else
			return filePath < d.filePath;
	}
	std::string toString() const;
};

struct MBMetaData_hash : std::unary_function<MBMetaData, std::size_t> {
	std::size_t operator()( const MBMetaData& p ) const {
		std::size_t seed = 0 ;
		boost::hash_combine( seed, p.filePath );
		boost::hash_combine( seed, p.dataRange.begin );
		return seed;
	}
};

class DataTank;
class MemoryBlockInfo;
typedef std::list<MemoryBlockInfo*>				MBLIST;

#define _USE_HASHMAP_

#ifndef _USE_HASHMAP_
	typedef std::map<MBMetaData, MemoryBlockInfo*>	MBMAP;
#else
	typedef boost::unordered_map<MBMetaData, MemoryBlockInfo*, MBMetaData_hash>	MBMAP;
#endif// _USE_HASHMAP_

typedef std::set<MBMetaData>					METADATASET;

typedef std::map<std::string, MemoryBlockInfo*>		FILE2METADATAMAP;
typedef MBLIST::iterator						LISTITER;

class MemoryBlockInfo {
private:
	DataTank&			mTank;
	size_t				mId;
	char*				mBuffer;
	size_t				mBufSize;
	size_t				mStartPos; // data start pos
	size_t				mDataSize;
	int					mErrorCode;
	long long			mLastTouch;
	NotifierPtr			mNotifier;

	LISTITER			mIterFree; 
	LISTITER			mIterList;
	ZQ::common::Mutex	mWriteLocker;

	volatile long		mUserCount;// reader or writer count
	MBMetaData			mMD;
	//for debugging
	bool				mbInReadCache;
	bool				mbInCacheRunner;
	bool				mbInExpireChecker;
	bool				mbInFreeList;
	bool				mbInWriteBuffer;
	bool				mbInFlushRunner;
	bool				mbInReading;
	bool				mbInWriting;

	bool				mbNeedValidate;

	MemoryBlockInfo*	mMetaNext;
	MemoryBlockInfo*	mMetaPrev;

public:
	MemoryBlockInfo( DataTank& tank, size_t id );
	~MemoryBlockInfo();

	void						addToMetaList( MemoryBlockInfo* head );
	void						removeFromMetaList( );
	bool						inMetaList( ) const { return mMetaPrev != this && mMetaNext != this ; }

	MemoryBlockInfo*			next() const{ return mMetaNext; }
	MemoryBlockInfo*			prev() const { return mMetaPrev; }

	bool						create( size_t size );
	void						destroy();
	void						reset( );

	inline size_t				id() const { return mId; }

	std::string					toString( ) const;

	inline long long			lastTouch() const { return mLastTouch; }
	void						touch( ){ mLastTouch = ZQ::common::now(); }

	inline const MBMetaData&	md() const { return mMD;}
	inline void					md( const MBMetaData& md ) { mMD = md; }

	inline char*				buffer() { return mBuffer; }
	inline size_t				bufSize() const { return mBufSize; }
	char*						data() { return mBuffer + mStartPos; }
	inline size_t				dataStartPos() const { return mStartPos; }
	inline void					dataStartPos( size_t pos ) { mStartPos = pos; }
	
	inline unsigned long long	dataPosOfFile() const { return mMD.dataRange.begin + mStartPos ; }

	inline size_t				dataEndPos() const{ return mStartPos + mDataSize; }
	inline size_t				dataSize() const { return mDataSize; }
	inline void					dataSize( size_t size ) { mDataSize = size; }

	inline bool					isDataFull() const { return mDataSize == mBufSize; }

	inline int					errorCode( ) const { return mErrorCode; }
	inline void					errorCode( int code ) { mErrorCode = code; }

	inline NotifierPtr			notifier() { return mNotifier; }
	inline void					notifier( NotifierPtr n ) { mNotifier = n; }

	inline LISTITER				iterList() { return mIterList; }
	inline void					iterList( LISTITER it ) { mIterList = it; }

	inline LISTITER				iterFree( ) { return mIterFree; }
	inline void					iterFree( LISTITER it ) { mIterFree = it ; }

	inline long					addUser( ) { return ++mUserCount; }
	inline long					removeUser( ) { return --mUserCount; }
	inline long					userCount( ) const{ return mUserCount; }

	inline ZQ::common::Mutex&	writerLocker() { return mWriteLocker; }

	void						status( ) const;

	inline bool					inReadCache( ) const { return mbInReadCache; }
	inline void					addToReadCache( ) { mbInReadCache = true; status(); }
	inline void					removeFromReadCache( ) { mbInReadCache = false; status(); }

	inline bool					inCacheRunner( ) const { return mbInCacheRunner; }
	inline void					addToCacheRunner() { mbInCacheRunner = true; status(); }
	inline void					removeFromCacheRunner() { mbInCacheRunner = false; status(); }

	inline bool					inExpireChecker() const { return mbInExpireChecker; }
	inline void					addToExpireChecker( ) { mbInExpireChecker = true; status(); }
	inline void					removeFromExpireChecker( ) { mbInExpireChecker = false; status(); }

	inline bool					inFreeList( ) const { return mbInFreeList; }
	inline void					addToFreeList( ) { mbInFreeList = true; status(); }
	inline void					removeFromFreeList( ) { mbInFreeList = false; status(); }

	inline bool					inWriteBuffer( ) const { return mbInWriteBuffer; }
	inline void					addToWriteBuffer( ) { mbInWriteBuffer = true; status(); }
	inline void					removeFromWriteBuffer( ) { mbInWriteBuffer = false; status(); }

	inline bool					inFlushRunner( ) const { return mbInFlushRunner; }
	inline void					addToFlushRunner( ) { mbInFlushRunner = true; status(); }
	inline void					removeFromFlushRunner( ) { mbInFlushRunner = false; status(); }

	inline bool					isBeingRead( ) const { return mbInReading; }
	inline void					addToRead( ) { mbInReading = true; status(); }
	inline void					removeFromRead() { mbInReading = false; status(); }

	inline bool					isBeingWrite( ) const { return mbInWriting; }
	inline void					addToWrite( )  { mbInWriting = true; status(); }
	inline void					removeFromWrite( )  { mbInWriting = false; status(); }

	inline bool					needValidate( ) const { return mbNeedValidate; }
	inline void					markValidate( ) { mbNeedValidate = true ; }

private:
	// non-copyable
	MemoryBlockInfo( const MemoryBlockInfo& );
	MemoryBlockInfo& operator=( const MemoryBlockInfo& );
};

class ExpirationChecker : public ZQ::common::NativeThread {
private:
	MBLIST			mList;
	bool			mbRunning;
	Sema			mSema;
	ZQ::common::Mutex			mMutex;
	size_t			mExpiration;
	size_t			mMaxBufCount;
public:
	ExpirationChecker();
	virtual ~ExpirationChecker();
	bool		start(size_t expiration, size_t maxBufCount );
	void		stop();
	void		touch( MemoryBlockInfo* mbi );
	void		removeFromExpireChecker( MemoryBlockInfo* mbi );
	MBLIST::iterator addToExpireChecker( MemoryBlockInfo* mbi );
private:

	int				run();
private:
	virtual void	onExpired( MemoryBlockInfo* mbi ) = 0;
};

class MBUser {
public:
	MBUser( DataTank& tank, MemoryBlockInfo* mbi );
	MBUser( const MBUser& b);
	MBUser& operator=( const MBUser& b);
	virtual ~MBUser();
	bool	isValid() const;
	int		lastError( ) const;
	MemoryBlockInfo* mbi() const { return mMbi; }
protected:
	DataTank&				mTank;
	MemoryBlockInfo*		mMbi;
};

class MBReader : public MBUser{
private:
	NotifierPtr		mNotifier;
public:
	MBReader( DataTank& tank, MemoryBlockInfo* mbi );
	MBReader( const MBReader& b );
	MBReader& operator=( const MBReader& b );
	virtual ~MBReader();

	virtual int	read( size_t offset, char* buf, size_t& size );
};

class MBWriter : public MBUser {
public:
	MBWriter( DataTank& tank, MemoryBlockInfo* mbi );
	MBWriter( const MBWriter& b );
	MBWriter& operator=( const MBWriter& b);
	virtual ~MBWriter();

	int		write( size_t offset, const char* buf, size_t& size,  unsigned long long originalOffset, bool bDirectWrite = false);
};


class File2Metadata {
private:
	DataTank&				mTank;
	FILE2METADATAMAP		mMetadataMap;
public:
	File2Metadata( DataTank& tank );
	virtual ~File2Metadata();
	void		reserve( size_t maxBufCount );
	
	void 		addMetaData( MemoryBlockInfo* mbi ); 
	void 		removeMetadata( MemoryBlockInfo* mbi );
	void 		removeMetadata( const std::string& filepath );
	MemoryBlockInfo* getMetadata( const std::string& filepath );
	size_t		count( const std::string& filePath ) const;
	std::vector<std::string> getFileList( ) const;
private:
};

class FreeList {
private:
	DataTank&	mTank;
	MBLIST		mFreeList;
	size_t		mCount;
public:
	FreeList( DataTank& tank );
	~FreeList();
	bool		create( size_t count, size_t bufSize );
	void		destroy();

	void		add( MemoryBlockInfo* mbi );
	void		remove( MemoryBlockInfo* mbi );

	size_t		count() const;

	///fetch an un-used memory block	
	MemoryBlockInfo* fetch( );
	bool fetch( size_t count, std::vector<MemoryBlockInfo*>& mbis);
};

class CacheReadRunner : public ZQ::common::ThreadRequest {
private:
	DataTank&						mTank;
	std::vector<MemoryBlockInfo*>	mMbis;
public:
	CacheReadRunner( DataTank& tank, const std::vector<MemoryBlockInfo*> mbis );
	virtual ~CacheReadRunner();
private:
	size_t	getTotalBufSize( std::vector<DataBuffer>& bufs ) const;
	void	storeResult( size_t size , int err );
	void	completeRead( );
	int		run();
	void	final(int retcode /* =0 */, bool bCancelled /* =false */){
		delete this;
	}
private:
	StopWatch	mStopWatch;
};

class MergeableArray :public ZQ::common::SharedObject {
public:
	struct HitStat {
		Range 			lastAccess;
		size_t			hitCount;
		HitStat():hitCount(0) {
		}
		bool operator==( const HitStat&  rhs ) const {
			return lastAccess == rhs.lastAccess;
		}
		bool operator < ( const HitStat& rhs ) const {
			return lastAccess < rhs.lastAccess;
		}
		bool operator > ( const HitStat& rhs ) const {
			return lastAccess > rhs.lastAccess;
		}

		bool operator == ( const Range& rhs ) const {
			return lastAccess == rhs ;
		}

		bool operator < ( const Range& rhs ) const {
			return lastAccess < rhs;
		}

		bool operator > ( const Range& rhs ) const {
			return lastAccess > rhs;
		}

	};
	
	MergeableArray( DataTank& tank);
	~MergeableArray();
	MergeableArray( const MergeableArray& rhs );
	MergeableArray& operator=( const MergeableArray& rhs );

	void 		reserve( size_t count) {
		mArray.reserve(count);
	}
	size_t		add( const Range& touchArea );
	size_t		itemCount( ) const {
		return mArray.size();
	}

private:

	HitStat*	firstItem( ) {
		return &mArray[0];
	}

	HitStat*	lastItem( ) {
		assert( mArray.size() >=1 );
		return &mArray[ mArray.size() - 1];
	}
	
	typedef std::vector<HitStat> 	HITSTATARRAY;

	bool		intersection( const Range& a, const Range& b ) const;
	bool		intersection( const HitStat* a, const HitStat* b ) const {
		return intersection( a->lastAccess, b->lastAccess );
	}
	bool 		intersection( const HitStat* a, const Range& b ) const {
		return intersection( a->lastAccess, b );
	}
	size_t		tryMergeWithNeighbor( HitStat* p  );

	HitStat*	merge( HitStat* to, const HitStat* from );

private:
	DataTank&				mTank;	
	HITSTATARRAY		 	mArray;
	size_t 					mMaxSize;
};

class ReadAheadStatisics {
public:
	ReadAheadStatisics( DataTank& tank );
	~ReadAheadStatisics( );
public:

	size_t 	readAt( const std::string& filepath, const Range& touchedArea );

private:

	size_t	calcReadAheadCount( size_t hitCount );

private:
	DataTank&			mTank;
	typedef ZQ::common::Pointer<MergeableArray> MergeableArrayPtr;
	typedef ZQ::common::LRUMap<std::string, MergeableArrayPtr> HITMAP;
	//typedef std::map<std::string, MergeableArray> HITMAP;
	HITMAP				mHitMap;
	ZQ::common::Mutex 	mLocker;
};


class ReadCache : public File2Metadata, public ExpirationChecker {
protected:
	DataTank&			mTank;
	MBMAP				mMbMap;
	int					mNAMemoryBlockCount;
	int					mReadingCount;
	ReadAheadStatisics	mReadStats;
	ZQ::common::Mutex	mLocker;
public:
	ReadCache( DataTank& tank );
	virtual ~ReadCache();

	//MBReader			getMemory( const MBMetaData& metadata );
	std::vector<MBReader> getMemory( const MBMetaData& metadata );
	std::vector<MBReader> getMemory( const std::vector<MBMetaData>& mds, bool& allReady );
	void				putMemory( const MBReader& reader );
	void				putMemory( const std::vector<MBReader>& readers );
	void				putMemory( MemoryBlockInfo* mbi );

	//void				addCachedData( MemoryBlockInfo* mbi );

	void				validate( const MBMetaData& metadata );
	void				validate( const std::string& filepath );
	void				remove( const MBMetaData& metadata );
	void				remove( MemoryBlockInfo* mbi );

	std::string			mbUsage( ) const;

	void				onReadComplete( MemoryBlockInfo* mbi );	
	void				onMBExpired( MemoryBlockInfo* mbi );
	void				onCacheComplete( MemoryBlockInfo* mbi );
	void				onCacheComplete( const std::vector<MemoryBlockInfo*>& mbis );

	bool				cacheExistFor( const std::string& filepath ) const;

private:
	MemoryBlockInfo* 	fetch( const MBMetaData& metadata);
	bool				fetch( const std::vector<MBMetaData>& mds, std::vector<MBReader> &readers) ;
	MemoryBlockInfo*	getMb( const MBMetaData& metadata );
	virtual void		onExpired( MemoryBlockInfo* mbi );
	size_t				readAhead( const MBMetaData& metadata, size_t count );
	size_t				findCachedData( const MBMetaData& metadata, size_t maxCount );
	MBMetaData			normalize( const MBMetaData&  md ) const;
};

class FlushRunnerCenter;

class FlushRunnerRequest : public ZQ::common::ThreadRequest {
private:
	FlushRunnerCenter&	mFlushCenter;
	DataTank&			mTank;
	std::vector<MemoryBlockInfo*>	mMbis;
	std::vector<int>				mFlushIndices;
public:
	FlushRunnerRequest( FlushRunnerCenter& center, DataTank& tank, 
			const std::vector<MemoryBlockInfo*>& mbi, 
			const std::vector<int>& flushIndices);
	virtual ~FlushRunnerRequest();
	
private:
	int		run();
	void	final(int retcode /* =0 */, bool bCancelled /* =false */);
};

class AvgArray {
private:
	std::vector<int64> 	bitrates;
	size_t				maxItems;
	size_t				pos;
	int64				avgBitrate;
public:
	AvgArray( size_t maxItems );
	int64				add( int64 bitrate );
	int64				get( ) const;
};

class FlushRunnerCenter : public ZQ::common::NativeThread {
public:
	FlushRunnerCenter( DataTank& tank );
	virtual ~FlushRunnerCenter( );
	void 	post( MemoryBlockInfo* mbi, int index);
	void 	post( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& indices);
	size_t	getInflightRequests( const std::string& filename, int64& avgBitrate ) const;
	bool					start();
	void					stop();
protected:
	void					tryToMergeFlushRequest( MemoryBlockInfo* mbi, int index );
	void					tryToMergeFlushRequest( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& indices );
	friend class FlushRunnerRequest;
	void 					onRunnerComplete( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& indices, int64 bitrate);
	void					addInflightReq( const std::string& filepath);
	void					removeInflightReq( const std::string&filepath, int64 bitrate);
	void					makeFlushRequest( MemoryBlockInfo* mbi, int index );
	void					makeFlushRequest( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int> indices );
	int						run();
private:
	DataTank&				mTank;
	ZQ::common::NativeThreadPool& mFlushPool;
	ZQ::common::Mutex 		mLocker;
	size_t					mOutstandingRunners;
	std::list<std::string>	mFileQueue;

	struct FlushFileMeta {
		DataTank&					  mTank;
		std::list<std::string>::iterator itFileQueue;

		std::vector<MemoryBlockInfo*> mbis;
		std::vector<int> indices;
		int64			lastTouch;

		struct MergedInfo {
			Range	range;
			size_t  mbiCount;
			MergedInfo():mbiCount(0){}
		};
		std::vector<MergedInfo>  mergedData;

		FlushFileMeta( DataTank& tank, std::list<std::string>::iterator it )
			:mTank(tank),itFileQueue(it),lastTouch(0) {
		}
		void add( MemoryBlockInfo*mbi, int index);
		size_t mergedCount( ) const;
	};
	typedef std::map< std::string, FlushFileMeta > FLUSHMAP;
	FLUSHMAP 								mFlushMap;
	struct InflightInfo {
		int			reqs;
		AvgArray	avgBitrate;
		InflightInfo(size_t maxItems):
			reqs(0), avgBitrate(maxItems){
		}
		InflightInfo( const InflightInfo& rhs ):reqs(rhs.reqs), avgBitrate(rhs.avgBitrate) {
		}
	};
	std::map<std::string,InflightInfo>				mInflightReqs;
	bool											mbRunning;
};

class WriteBuffer : public File2Metadata, public ExpirationChecker {
private:
	struct FlushRecorder {		
		int				flushIndex;
		struct FlushingInfo 
		{
			int				lastError;
			int				count;
			NotifierPtr		notifier;
			bool			hasSinker;
			FlushingInfo():lastError(0),count(0),hasSinker(false){}
		};
		FlushRecorder()
			:flushIndex(1) 
		{}
		typedef std::map<int,FlushingInfo>	FlushingInfoMap;
		FlushingInfoMap	flushingInfo;
		//      index
	};

	DataTank&			mTank;
	FlushRunnerCenter	mFlushCenter;
	MBMAP				mMbMap;
	int					mFlushingCount;
	ZQ::common::Mutex	mLocker;
	
	typedef std::map<std::string, FlushRecorder> FlushRecorderMap;
	FlushRecorderMap	mFlushingRecorders;

public:
	WriteBuffer( DataTank& tank );
	~WriteBuffer();

	FlushRunnerCenter&		getFlushCenter() { return mFlushCenter; }

	std::vector<MBWriter> getMemory( const MBMetaData& metadata );
	void		putMemory( const MBWriter& writer );
	void		putMemory( const std::vector<MBWriter>& writers );

	void		flushAll( );
	NotifierPtr	flush( const std::string& filePath, int& index );
	int			waitFlush( const std::string& filePath, int index );

	void		validate( const std::string& filepath );

	bool		evict( const MBMetaData& metadata, bool bForce = false ,bool bDirectWrite = false );
	bool		evict( const std::vector<MemoryBlockInfo*>& mbis);

	std::string mbUsage() const;

	void		onFlushComplete( MemoryBlockInfo* mbi, int flushIndex );
	void		onFlushComplete( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>&flushIndices);

	void		onMBExpired( MemoryBlockInfo* mbi );

private:
	void		putMemory( MemoryBlockInfo* mbi );
	MBWriter	getMemory( const MBMetaData& metadata, int i );
	bool		forceFlushOne( const MBMetaData& md = MBMetaData() );
	void		flushByRunner( MemoryBlockInfo* mbi );
	void		flushByRunner( const std::vector<MemoryBlockInfo*>& mbis );
	void		flushDirect( MemoryBlockInfo* mbi );
	virtual void		onExpired( MemoryBlockInfo* mbi );
};

struct DataTankConf {
	size_t			logFlag;
	size_t			flushThreadPoolSize;
	size_t			readCacheBlockSize;
	size_t			writeBufferBlockSize;
	size_t			readBlockCount;
	size_t			writeBlockCount;
	int64			cacheInvalidationInterval;
	int64			bufferInvalidationInterval;
	size_t			readAheadCount;
	size_t			readAheadThreshold;
	size_t			readAheadIncreamentLogBase;
	size_t			mergableArrayMaxItemSize;
	size_t			maxReadHitTrackerCount;
	size_t			maxWriteQueueMergeItemCount;
	size_t			maxWriteQueueBufferCount;
	size_t			minWriteQueueBufferCount;
	size_t			writeQueueIdleInMs;
	size_t			partitionCount;
	size_t          readAfterFlushDirty;

	size_t			writeThreadsOfYield;
	size_t			writeYieldMax;
	size_t			writeAvgWinSizeForYield;
	int64			writeYieldMin;
	size_t			writeBufferCountOfYield;

	DataTankConf()	{
		logFlag						= 0;
		flushThreadPoolSize			= 10;
		readCacheBlockSize			= 128 * 1024; // for read cache
		writeBufferBlockSize		= 2 * 1024 * 1024;// for write buffer
		readBlockCount				= 400;
		writeBlockCount				= 50;

		cacheInvalidationInterval	= 5 * 60 * 1000;
		bufferInvalidationInterval	= 5 * 1000;
		readAheadCount				= 32;
		readAheadThreshold			= 4;

		mergableArrayMaxItemSize	= 100;
		maxReadHitTrackerCount 		= 1000;

		maxWriteQueueMergeItemCount = 10;
		maxWriteQueueBufferCount	= 1000;
		minWriteQueueBufferCount	= 10;
		writeQueueIdleInMs			= 50;
		readAheadIncreamentLogBase	= 4;

		partitionCount				= 1;

		writeThreadsOfYield 		= 5;
		writeYieldMax				= 1000;
		writeAvgWinSizeForYield		= 5;
		writeYieldMin				= 20;
		writeBufferCountOfYield		= 0;
	}
};

class DataTank {
public:
	DataTank( ZQ::common::NativeThreadPool& readPool, ZQ::common::NativeThreadPool& writePool, ZQ::common::Log& logger, DataTankConf& conf );
	virtual ~DataTank();

public:

	virtual int	cacheRead( const std::string& filePathName, char* buffer, unsigned long long begin, unsigned int& size );

	virtual int cacheWrite( const std::string& filePathName, const char* buffer, unsigned long long begin, unsigned int& size );

	virtual bool createTank( );

	virtual void destroyTank( );

	
	void		flushAllWriteBuffer( );

	int			validate_writebuffer( const std::string& filePathName );
	int			validate_readcache( const std::string& filePathName, unsigned long long begin , unsigned int size );
	int			validate_readcache( const std::string& filePathName );

	inline
		FreeList&	getReadFreeList() { return mReadFreeList; }

	inline
		FreeList&	getWriteFreeList() { return mWriteFreeList; }

	inline 
		ReadCache& getReadCache() { return mReadCache; }

	inline
		WriteBuffer& getWriteBuffer() { return mWriteBuffer; }

	inline
		const DataTankConf&	getConf() const { return mTankConf;}

	inline ZQ::common::NativeThreadPool& getReadThreadPool( ) { return mReadPool; }
	inline ZQ::common::NativeThreadPool& getFlushThreadPool( ) { return mWritePool; }

	inline ZQ::common::Log&	getLogger() { return mLogger; }

	inline size_t	getMaxFlushRequestCount() const { return mWritePool.size(); }
	inline int64	getReadcacheExpireTime() const { return mTankConf.cacheInvalidationInterval; }
	inline size_t	getReadaheadCount() const { return mTankConf.readAheadCount; }

	void			onCacheComplete( MemoryBlockInfo* mbi );
	void			onCacheComplete( const std::vector<MemoryBlockInfo*>& mbis );
	void			onFlushComplete( MemoryBlockInfo* mbi, int flushIndex );
	void			onFlushComplete( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& flushIndices );
	void			onWriteComplete( MemoryBlockInfo* mbi );
	void			onReadCacheExpired( MemoryBlockInfo* mbi );
	void			onWriteBufferExpired( MemoryBlockInfo* mbi );

	virtual	int directWrite( const std::string& filename, const char* buf, unsigned long long offset , size_t& size ) { return 0; }

	virtual	int	directRead( const std::string& filename, char* buf, unsigned long long offset, size_t& size ) { return 0; }
	
	virtual	int	directRead( const std::string& filename, unsigned long long offset, const std::vector<DataBuffer>& bufs, size_t& sizeTotal)	{
		return 0; 
	}

	virtual int	directWrite( const std::string& filename, const std::vector<DataBuffer>& bufs, size_t& sizeTotal) {
		return 0;
	}

	virtual bool isSuccess( int err , size_t* size = 0 ) const { return false; }

	std::vector<MBMetaData> convertRangeToBlock( const std::string& filename, unsigned long long begin, size_t size, bool readOrWrite );
private:

	void		preCacheWrite( const std::string& filePathName, size_t size );

protected:
	DataTank&						mTank;
	DataTankConf					mTankConf;

	ZQ::common::NativeThreadPool&	mReadPool;
	ZQ::common::NativeThreadPool&	mWritePool;
	ZQ::common::Log&				mLogger;

private:

	FreeList						mReadFreeList;
	FreeList						mWriteFreeList;
	ReadCache						mReadCache;
	WriteBuffer						mWriteBuffer;

	//ZQ::common::Mutex				mLocker;
	
	Sema							mInvalidationSem;
};


}//namespace CacheZone

#endif//__cache_layer_zone_header_file_h__
