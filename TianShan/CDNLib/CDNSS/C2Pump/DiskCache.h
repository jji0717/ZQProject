#ifndef __DiskCache_H__
#define __DiskCache_H__

#include "Pointer.h"
#include "LRUMap.h"
#include "BufferPool.h"
#include "NativeThreadPool.h"
#include "AioFile.h"

#include <map>
#include <string>
#include <vector>
#include <queue>

#define USERBUF_PER_READ_REQUEST

#define CacheCopy_Stamp_MagicNumber (0x0131C9A5)
#define URL_MAX_LENGTH              (256)
#define IO_THREADS_MIN              (2)
#define SUBDIR_TOP_MIN              (16)
#define SUBDIR_LEAF_MIN             (250)

#define DIR_ACCESS_COUNT_TIME_WIN    (10000) // 10 sec
#define DIR_ACCESS_COUNT_BPS         (10)    // 10 read/sec
#define DIR_ACCESS_COUNTS_SUBMIT_MAX (DIR_ACCESS_COUNT_TIME_WIN * DIR_ACCESS_COUNT_BPS /1000)

//@note !!! when macro READ_INSTANT_CALLBACK is defined, callback of read_async() may be triggered instantly in the same thread of the caller of read_async
// if take READ_INSTANT_CALLBACK, the programmer must avoid deadlock between this caller context and impl in the callback
#define READ_INSTANT_CALLBACK 

using namespace C2Streamer;
namespace XOR_Media {
namespace DiskCache {
class CacheDir;

//typedef ZQ::common::BufferList::Ptr BufferPtr; // this should be changed to take HongQuan's SmartPointer of 2MB Buffer

// -----------------------------
// class CounterFlow
// -----------------------------
class CounterFlow : public ZQ::common::NativeThread
{
public:
	typedef struct _Counter
	{
		int64 stampLatest;
		int64 stamp2ndLatest;
		int64 stampSince;
		size_t c;

		_Counter() { stampLatest = stamp2ndLatest = stampSince=0; c=0; } 
	} Counter;

public:

	CounterFlow(CacheDir& dir) : _dir(dir) {}
	virtual ~CounterFlow() {}

	virtual void countHit(std::string subpath, uint c=1);
	virtual void countMissed(std::string subpath, uint c=1);
	virtual void countHit(std::string subpath, const Counter& cnt);
	virtual void countMissed(std::string subpath, const Counter& cnt);

	static size_t  inc(Counter& A, size_t c=1);
	static Counter add(const Counter& A, const Counter& B);
	static Counter merge(const Counter& A, const Counter& B);
	static size_t  attenuate(Counter& A, int64 winSince);

	static std::string marshal(const Counter& A);
	static bool unmarshal(Counter& A, const std::string stream);

protected:

	virtual int run(); // impl of NativeThread

protected:
	CacheDir&   _dir;

	typedef struct _NamedCounter
	{
		std::string subpath;
		Counter cntr;
	} NamedCounter;
	typedef std::vector < NamedCounter > NamedCounterQueue;
	typedef std::map < std::string, Counter > CounterMap; // map of path to counter

	NamedCounterQueue _countersOfMissed, _countersOfHit;
	ZQ::common::Mutex _lkQueue;

	virtual void pushCounter(NamedCounterQueue& queue, std::string subpath, const Counter& counter);
	virtual size_t popMergedQueue(NamedCounterQueue& mergedQueue, bool bMissedNHit=false);
};

// -----------------------------
// class CacheCopy
// -----------------------------
class CacheCopy : public ZQ::common::SharedObject
{
public:
	CacheCopy();
	~CacheCopy();
	CacheCopy( const CacheCopy& ccp);
	CacheCopy& operator= ( const CacheCopy& ccp );
	typedef ZQ::common::Pointer< CacheCopy > Ptr;
	typedef ZQ::common::LRUMap < std::string, Ptr > LRUMap;

	typedef struct _OriginDesc
	{
		std::string url;
		int64       stampAsOfOrigin;
	} OriginDesc;

	typedef struct _ExchangeDesc
	{
		std::string pathName;
		int64       stampAsOfDisk;
	} ExchangeDesc;

	typedef enum _Error {
		eOK=0, 
		eError=100, eIOError, eNotFound, eNoHeader, eBadHeader, eBadURL, eCRCError
	} Error;

	OriginDesc   _originKey;
	//BufferPtr    _contentBody;
	BufferUser        _contentBody;
	ExchangeDesc _exchangeKey;
	CounterFlow::Counter	_cntAccess;
	
public:

	typedef struct _Header
	{
		uint32 signature;
		uint16 headerLen, version;
		int64  contentSize;
		int64  stampAsOfOrigin;
		char   url[URL_MAX_LENGTH];
	} Header;

	virtual	Error load(const std::string& pathName, BufferUser user);
	virtual Error flush();
	virtual Error unlink();

protected: // platform-specific implementation of vector-IO
	static int writev(const std::string& filename, const Header& h, const BufferUser& cont);
	static int readv(const std::string& filename, Header& h, BufferUser cont);
};
/*
// -----------------------------
// callback DiskCacheSink
// -----------------------------
// this class should be changed to take HongQuan's aio callback
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
	virtual void onCacheRead(Error errCode, CacheCopy::Ptr ccopy) =0;

	// errCode : cacheErr_OK - wrote success, cacheErr_Hit - cache already exists, cacheErr_Timeout - writting timeout
	virtual void onCacheWrote(Error errCode, const std::string& url) =0;
	
	// get buffer from cachecenter to read data
	virtual BufferUser  getCacheBufUser(const std::string& fileName, const int64 stampAsOfOrign) = 0;

	//get a empty bufferuser
	virtual BufferUser  makeEmptyBufUser() = 0;
};
*/
// -----------------------------
// class CacheLoader
// -----------------------------
#define WAIT_INTERVAL_MAX      (500)   // 500msec
#define WAIT_INTERVAL_MIN      (5)     // 5msec
class CacheLoader : public ZQ::common::NativeThread, virtual public ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer< CacheLoader > Ptr;
	typedef std::vector < Ptr > List;

	CacheLoader(CacheDir& dir) : _dir(dir) {}

protected:
	CacheDir&   _dir;
	virtual int run(); // impl of NativeThread
};

// -----------------------------
// class CacheSaver
// -----------------------------
#define FLUSH_INTERVAL_MIN      (WAIT_INTERVAL_MIN<<2)  // 20msec maps to 100MBps
#define FLUSH_INTERVAL_MAX      (WAIT_INTERVAL_MAX<<2)  // 2sec
#define FLUSH_INTERVAL_DEFAULT  (100)     // 100msec maps to 20MBps disk-write speed 
class CacheSaver : public ZQ::common::NativeThread, virtual public ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer< CacheSaver > Ptr;
	typedef std::vector < Ptr > List;

	CacheSaver(CacheDir& dir) : _dir(dir) {}

protected:
	CacheDir&   _dir;
	virtual int run(); // impl of NativeThread

};

// -----------------------------
// class CacheDir
// -----------------------------
// a CacheDir may maps to a mount point of a disk
class CacheDir : public ZQ::common::SharedObject
{
	friend class CacheLoader;
	friend class CacheSaver;
	friend class CounterFlow;

public:
	typedef ZQ::common::Pointer< CacheDir > Ptr;
	virtual ~CacheDir();

public: // main access via static, known as a CacheDir Aggregator
	static CacheDir::Ptr addCacheDir(ZQ::common::Log& log, const std::string& homePath, uint64 mbTotal, int threadsRead =4, int threadsWrite=4, int pendingsToYield=4, uint32 timeout=3000, int32 LRUSize=20);
	static CacheDir::Ptr locateCacheDir(const std::string& url, std::string& subPath);
	static void stop(void);
	static void releaseBuffer();

	//@note !!! when macro READ_INSTANT_CALLBACK is defined, the callback will be triggered instantly in the same thread of the caller of read_async
	// if take READ_INSTANT_CALLBACK, the programmer must avoid deadlock between this caller context and impl in the callback
	static BufferUser read_async(const std::string& fileName, const int64 offsetInFile, const int64 stampAsOfOrign, int64 bitrate, long opaqueData, DiskCacheSink* cbCompletion =NULL);
	static void write_async(C2Streamer::BufferUser buf,  int64 stampAsOfOrign, DiskCacheSink* cbCompletion =NULL);
	static std::string getURL(const std::string& fileName, const int64 offsetInFile);

public: // per-dir access

	void  setTimeout(uint32 msTimeout) { _timeout = msTimeout; }
	void  setMaxPending(uint16 maxDirtyCopies, uint16 maxLoadingCopies) { _maxDirtyCopies = maxDirtyCopies; _maxLoadingCopies = maxLoadingCopies; }
	void  setLRUsize(uint32 cacheCopies){ ZQ::common::MutexGuard guard(_lkCacheCopies); _cacheCopies.resize(cacheCopies);};
	void  delBuffer();// { ZQ::common::MutexGuard guard(_lkCacheCopies); _cacheCopies.erase_eldest();}
	void  subDirs(uint8 topDirs, uint8 leafDirs);

	std::string  path(void) const { return _homePath; }
	uint64       distance(const std::string& url) const;
	bool         getSpace(uint64& totalMB, uint64& freeMB);

	virtual BufferUser load_async(const std::string& subPath, const std::string& fileName, const int64 offsetInFile, const int64 stampAsOfOrign, int64 bitrate, long opaqueData, DiskCacheSink* cbCompletion =NULL);
	virtual void update_async(const std::string& subPath, BufferUser buf, const std::string& url, int64 stampAsOfOrign, DiskCacheSink* cbCompletion =NULL);
	virtual uint32 evict(const std::string& subPath);
	virtual uint32 evictByFileToSwap(const std::string& subDir, uint32 forMB);

public:
	typedef struct _fileTime
	{
		std::string fileName;
		int64    fileSize;
		int64    lastReadTime;
	} FILETIME;

	typedef std::vector<FILETIME> FileLastTimeList;


protected:
	typedef struct _AwaitIOSink
	{
		bool           bToWrite;
		std::string    subPath;
		std::string    url;
		DiskCacheSink* cb;
		int64          stampSink;
		int64 		   reqBufId;
#ifdef USERBUF_PER_READ_REQUEST
		BufferUser     bu;
#endif // BUF_BY_REQUEST

		_AwaitIOSink()
		:bToWrite(false), cb(NULL), stampSink(0), reqBufId(-1)
		{
		}
	} AwaitIOSink;

	typedef std::vector< AwaitIOSink > AwaitIOSinkList; // list of AwaitIOSink
	typedef std::map< std::string, BufferUser > AwaitBufMap;     // map from 

	typedef struct _Notification
	{
		AwaitIOSink sink;
#ifndef USERBUF_PER_READ_REQUEST
		BufferUser  bu;
#endif // USERBUF_PER_READ_REQUEST
	} Notification;
	typedef std::vector < Notification > NotificationList;

	BufferUser  scheduleRead(const std::string& subPath, const std::string& fileName, const int64 offsetInFile, const int64 stampAsOfOrign, int64 bitrate, long opaqueData, DiskCacheSink* cbCompleted);

	typedef union _hkey_t
	{
		uint64 q[2];
		uint16 w[8];
	} hkey_t;

	static bool   _calcHashKey(hkey_t& hkey, const char* buf, uint len=0);
	static uint64 _distance(const hkey_t& hkey1, const hkey_t& hkey2);
	uint64        distance(const hkey_t& hkeyURL) const;

	// private constructor
	CacheDir(ZQ::common::Log& log, const std::string& homePath, uint64 mbQuota, int threadsLoad, int threadsFlush);

	void _buildDirs();

protected:
	ZQ::common::Log& _log;

	// configurations:
	uint8  _threadsLoad, _threadsFlush;

public:
	uint64 _mbQuota;
	uint32 _timeout;
	uint16 _maxDirtyCopies, _maxLoadingCopies;
	uint8  _cdirsTop, _cdirsLeaf;
	std::string _homePath;
	uint32 _flushInterval; // interval for one segment to limit the diskwrite, speed-at-MBps can be converted by
	
	// _flushInterval = 1000 * SegmentSizeMB * 1/ SpeedMBps, in msec
	uint32 setFlushSpeed(uint32 flushMBps) { _flushInterval = (flushMBps >0) ? (1000 * 2/ flushMBps) : 0; return _flushInterval; }

	CacheCopy::LRUMap _cacheCopies; // map of subPath to CacheCopy
	std::queue < std::string > _dirtyCopies; // queue of subPath to indicate dirty copies, the write-workers read this queue to flush
	CounterFlow::Counter _cntDirAccess; // protected by _lkCacheCopies
	ZQ::common::Mutex _lkCacheCopies; // protects _cacheCopies and _dirtyCopies
	
	typedef std::map< std::string, int> LoaderMap; // map of subpath to thread id

	AwaitIOSinkList     _awaitIOSinks;
	AwaitBufMap         _awaitReadBufMap;
	LoaderMap           _busyLoaders;
	// AwaitReads       _awaitReads;
	ZQ::common::Mutex   _lkAwaitIOSinks;

	CacheSaver::List  _flushers;
	CacheLoader::List   _loaders;
	ZQ::common::Event   _eventReadWorkers;
	ZQ::common::Event   _eventWriteWorkers;

	//static ZQ::common::Event _eventWorkers;

	hkey_t              _hkey;


	typedef std::map< std::string, Ptr > DirMap; // map of pathname to CacheDir

protected: // data structure about the static access known as the CacheDir Aggregator
	static bool _bQuit;
	static DirMap _dirMap;
	static ZQ::common::Mutex _lkDirMap;

	static ZQ::common::NativeThreadPool _thpool; // for callback dispatchers

	static CacheCenter	*_cacheCenter;
public:
	static DirMap listCacheDir();
	static std::string dirOfFile(const std::string& filepath);
	static void setCacheCenter( C2Streamer::CacheCenter* cc);
};

}} // namespace

#endif // __DiskCache_H__
