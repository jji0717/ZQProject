
#include <ZQ_common_conf.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <SystemUtils.h>
#include <TimeUtil.h>
#include <CryptoAlgm.h>
#include "C2StreamerEnv.h"
#include "AioFile.h"
#include "C2SessionHelper.h"
#include "ReadClient.h"
#include <SystemInfo.h>
//#include <http.h>
#include <ReadHttp.h>
#include <ReadHttpManger.h>
#include "AquaReader.h"
#include "DiskCache.h"
#define PAGE_SIZE (4*1024)
namespace C2Streamer
{

//////////////////////////////////////////////////////////
/// CacheCenter
CacheCenter::CacheCenter( C2StreamerEnv& env)
	:mEnv(env),
	mbRunning(false),
	mBufferCount(0),
	mDefaultBufferCount(0),
	mFreeBufferCount(0),
	mOutStandingIoReqCount(0),
	mExpireChecker(env,*this),
	mBufferId(0),
	mBufferReqIdBase(0),
	mIdxRecMap(5000){
		mHead = new Buffer( *this, 0 , 0, 0 ); // a dummy one, will not be used
		mBufferCountDelta = 10;
		mConfCacheCenter = &(mEnv.getConfig().cacheCenter);
	}

CacheCenter::~CacheCenter( ) {
	stop();
	delete mHead;
}

int64 CacheCenter::genReqId() {
	ZQ::common::MutexGuard gd(mLocker);
	return ++mBufferReqIdBase;
}

void CacheCenter::genReqIdForBuffer(Buffer* buf) {
	int64 newId = genReqId();
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter, "change buf's id from [%ld] to [%ld]"),
			buf->reqId(), newId);
	buf->reqId(newId);
}

void CacheCenter::stop( ) {
	// FIXME: 未完成，需要考虑所有的buffer的引用计数应该都???.也就是是没有任何一个user在使用buffer
	// 的时候才能destroy这个buffer
	if( mbRunning ) {
		mbRunning = false;
		mWatchReqSem.post();
		ZQ::common::NativeThread::waitHandle(-1);
	}
	mReaders.clear();
	mExpireChecker.stop();
}

bool CacheCenter::start( size_t bufferSize, size_t defaultBufferCount , size_t delta ) {
	ZQ::StreamService::ReadClient::Ptr c2client = NULL;
	if(delta < 4 )
		delta = 4;  
	defaultBufferCount = ( defaultBufferCount + delta -1 ) / delta * delta;
	mDefaultBufferCount = defaultBufferCount;
	mBufferCountDelta = delta;
	mBufferSize = bufferSize;

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter,"start() create buffer, size[%zu] count[%zu]"),
			bufferSize, defaultBufferCount);
	//启动过程中，就不用加锁了
	for( size_t i = 0; i < defaultBufferCount ; i += delta ) {
		if( !addBuffer( ) ) {
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheCenter,"start() not enough memory"));
			return false;
		}
	}
	mbRunning = true;
	mExpireChecker.start();

	//if ( CLIENT_TYPE_DISKAIO == mEnv.getConfig().clientType)
	{
		AioClient::Ptr pt = new AioClient(this, mEnv);
		if ( !pt->initClient() )
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheCenter,"start() init aioclient failed."));
			pt = NULL;
			return false;
		}
		mReaders.push_back(pt);
	}
	//else if ( CLIENT_TYPE_C2CLIENT == mEnv.getConfig().clientType)
	{
		ZQ::StreamService::RequestParams requestParams;
		const ConfC2Client& confC2Client = mEnv.getConfig().c2Client;
		requestParams.url                  = confC2Client.C2ClientURL;
		requestParams.upstreamIP           = confC2Client.C2ClientUpStreamIP;
		requestParams.clientTransfer       = confC2Client.C2ClientTransfer;
		requestParams.locateIP             = confC2Client.C2ClientHttpCRGAddr;
		requestParams.locatePort           = confC2Client.C2ClientHttpCRGPort;
		requestParams.defaultGetPort       = confC2Client.C2ClientDefaultGetPort;
		requestParams.transferRate         = confC2Client.C2ClientTransferRate;
		requestParams.ingressCapacity      = confC2Client.C2ClientIngressCapacity;
		requestParams.exclusionList        = confC2Client.C2ClientExclusionList;
		requestParams.transferDelay        = confC2Client.C2ClientTransferDelay;
		requestParams.waitBufferTime       = confC2Client.C2ClientWaitBufferTime;
		requestParams.indexTimeout         = confC2Client.C2ClientIndexTimeout;
		requestParams.indexRetryTimes      = confC2Client.C2ClientIndexRetryTimes;
		requestParams.mainfileTimeout      = confC2Client.C2ClientMainfileTimeout;
		requestParams.mainfileRetryTimes   = confC2Client.C2ClientMainfileRetryTimes;
		requestParams.transferDelete       = confC2Client.C2ClientEnableTransferDelete == 0 ? false : true;
		requestParams.alignment			   = confC2Client.C2ClientAlignment;
		requestParams.maxBufferCountPerRequest	= confC2Client.C2ClientMaxBufferCountPerRequest;
		requestParams.minTransferRate = confC2Client.C2ClientMinTransferRate;
		requestParams.bitrateInflate = confC2Client.C2ClientBitrateInflate;
		requestParams.maxBitrate = confC2Client.C2ClientMaxBitrate;

		MLOG.info(CLOGFMT(CacheCenter, "create C2Client, url[%s] locateTarget[%s:%d] bitrateInflate[%d] maxBitrate[%ld]"),
				requestParams.url.c_str(), requestParams.locateIP.c_str(), requestParams.locatePort,
				requestParams.bitrateInflate, requestParams.maxBitrate);

		ZQ::StreamService::ReadClient::Ptr pt = new ZQ::StreamService::ReadClient( this, *(mEnv.getLogger()), requestParams);
		if (pt == NULL )
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheCenter,"start() create c2client  failed."));
			return false;
		}
		c2client = pt;
		mReaders.push_back(pt);
	}
	//else if ( CLIENT_TYPE_HTTPFETCHER == mEnv.getConfig().clientType )
	{
		ReadHttp::Ptr pt = new ReadHttp(this, 
				mEnv.getConfig().httpClient.httpProxyURL,
				mEnv.getConfig().httpClient.segmentURL,
				*(mEnv.getLogger()),
				mEnv.getConfig().httpClient.httpTimeOut,
				mEnv.getConfig().httpClient.httpRetry,
				false);
		if (pt == NULL)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheCenter,"start() create http client  failed."));
			return false;
		}
		mReaders.push_back(pt);
	} 
	//else if( CLIENT_TYPE_AQUAREADER == mEnv.getConfig().clientType ) 
	{
		FuseOpsConf opsConf;
		opsConf.enableCache = false;
		CacheLayer::DataTankConf& tankConf = opsConf.tankConf;
		tankConf.partitionCount 	= 1;
		tankConf.readCacheBlockSize = mEnv.getConfig().aquaReader.aquaReaderCacheBlockSize;
		tankConf.readAheadCount 	= 2;
		tankConf.readBlockCount 	= mEnv.getConfig().aquaReader.aquaReaderCacheBlockCount;
		tankConf.readAheadThreshold = 0;
		tankConf.flushThreadPoolSize = 1;

		AquaReader::Ptr r = new AquaReader( mEnv, this, opsConf );
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter,"start() create AquaReader: rootUrl:[%s] userDomain[%s] homeContainer[%s], cache[%d/%d]"), 
				mEnv.getConfig().aquaReader.aquaReaderRootUrl.c_str(),
				mEnv.getConfig().aquaReader.aquaReaderUserDomain.c_str(),
				mEnv.getConfig().aquaReader.aquaReaderHomeContainer.c_str(),
				(int)tankConf.readCacheBlockSize,
				(int)tankConf.readBlockCount);
		mReaders.push_back(r);
	}
	//else if( CLIENT_TYPE_HTPPFETCHER2 == mEnv.getConfig().clientType )
	{
		ReadHttp::Ptr pt = new ReadHttp(this, 
				mEnv.getConfig().httpClient.httpProxyURL,
				mEnv.getConfig().httpClient.segmentURL,
				*(mEnv.getLogger()),
				mEnv.getConfig().httpClient.httpTimeOut,
				mEnv.getConfig().httpClient.httpRetry,
				true);
		if (pt == NULL)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheCenter,"start() create http client  failed."));
			return false;
		}
		mReaders.push_back(pt);
	}

	//HybridReader
	{
		HybridReader::Ptr reader = new HybridReader(*this);
		reader->pushReaderType(CLIENT_TYPE_AQUAREADER);
		reader->pushReaderType(CLIENT_TYPE_C2CLIENT);
		mReaders.push_back(reader);
	}
	//else if( CLIENT_TYPE_READHTTPMANGER == mEnv.getConfig().clientType )
	{
		//ReadHttpManger(IReaderCallback* callBack,ZQ::StreamService::ReadClient::Ptr c2client,ZQ::common::Log& log,const std::string& proxyUrl, const std::string& segmenterUrl,const int mapsize = 1000, const int savecount = 10,int timeout = 10*1000,int retrytime = 0, bool replacec2 = false);
		const C2EnvConfig& conf = mEnv.getConfig();
		ReadHttpManger::Ptr pt= new ReadHttpManger(
				this,
				c2client,
				*(mEnv.getLogger()),
				conf.httpClient.httpProxyURL,
				conf.httpClient.segmentURL,
				8000,
				conf.c2HybridMaxHitCount,
				conf.c2HybridMaxTimeDuration,
				conf.httpClient.httpRetry,
				true);
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter,"start() create ReadHttpManger ProxyURL:[%s] segmentURL:[%s] maxHitCount[%d] maxDuration[%d]"),
				conf.httpClient.httpProxyURL.c_str(), conf.httpClient.segmentURL.c_str(),
				conf.c2HybridMaxHitCount, conf.c2HybridMaxTimeDuration	);
		if (pt == NULL)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheCenter,"start() create ReadHttpManger  failed."));
			return false;
		}
		else
		{
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"start() create ReadHttpManger OOOKKK."));
		}
		mReaders.push_back(pt);
	}

	if( mEnv.getConfig().diskCache.diskCacheEnabled )
	{
		//init disk cache
		const C2Streamer::DISKCACHECON& dcc =  mEnv.getConfig().diskCache.diskCacheConfig;
		DISKCACHECON::const_iterator iter = dcc.begin();
		for(; iter != dcc.end(); iter++)
		{
			XOR_Media::DiskCache::CacheDir::Ptr dir =  XOR_Media::DiskCache::CacheDir::addCacheDir( 
					*(mEnv.getLogger()), 
					iter->homePath,
					iter->totalSize,
					iter->readThreadCount,
					iter->writeThreadCount,
					iter->pendingsYield,
					mEnv.getConfig().diskCache.cacheLoaderTimeout,
					iter->LRUSize);
			dir->setFlushSpeed(iter->maxWriteMBps);
		}
		XOR_Media::DiskCache::CacheDir::setCacheCenter(this);
	}
	return ZQ::common::NativeThread::start();
}

static const unsigned char TsNullPacket[] = {   0x47,0x1f,0xff,0x10,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff         };

BufferUser CacheCenter::findPauseFrame( const ConfWriter* conf) {
	assert( conf != NULL);
	if(conf->mSendPacketSize % 188 != 0) {
		MLOG.error(CLOGFMT(CacheCenter, "findPuaseFrame, sendPacketSize is not 188 aligned, reject request"));
		return BufferUser();
	}
	size_t size = mBufferSize / 188 * 188;
	if( size <= 0 ) {
		return BufferUser();
	}
	ZQ::common::MutexGuard gd(mLocker);
	std::map<size_t, Buffer*>::iterator it = mPauseFrames.find(size);
	if( it != mPauseFrames.end()) {
		return BufferUser(*this, it->second);
	}
	char *buf = (char*)malloc(size);
	for( size_t i = 0 ; i < (size/188); i ++ ) {
		memcpy(buf + i * 188, &TsNullPacket, 188);
	}

	Buffer* pB = new Buffer(*this, buf, size, genReqId(), true);
	pB->setDataSize(size);
	pB->signal( );
	mPauseFrames[size] = pB;
	return BufferUser(*this, pB);
}

bool CacheCenter::addBuffer( ) {
	int maxBuffers = (int)(mEnv.getConfig().cacheCenter.mCacheDefaultBufferCount * 1.2) ;
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter, "addBuffer() the buffer count free[%zu] total[%zu]."), mFreeBufferCount, mBufferCount );
	if( maxBuffers <= (int)mBufferCount )
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CacheCenter, "addBuffer() the buffer count[%zu] exceed 1.2 * config buffer count [%d]."), mBufferCount, maxBuffers );
		return  false;
	}
	size_t size = mBufferSize * mBufferCountDelta;
	char* addr = (char*)memalign( 4096, size );

	if(!addr) {
		MLOG(ZQ::common::Log::L_EMERG, CLOGFMT(CacheCenter,"addBuffer() not enough memory"));
		return false;
	}
	for( size_t i = 0 ; i < mBufferCountDelta ; i ++ ) {
		++mBufferCount;
		Buffer* buf = new Buffer( *this, &addr[i*mBufferSize], mBufferSize, ++mBufferId );
		chainBuffer( buf, true);
	}
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(CacheCenter, "allocated buffer no.%zu: %zuB"),    mBufferCount, mBufferSize);
	return true;
}

AioFile* CacheCenter::open( const std::string& filename, int readerType, const std::string& sessId ) {
	AioFileAttr attr;
	attr.name = filename;
	attr.readerType = readerType;
	ZQ::common::MutexGuard gd(mLocker);
	FILEINFOMAP::iterator it = mFileInfos.find( attr );
	if( it != mFileInfos.end() ) {
		it->second->open( filename, readerType );
		return it->second;
	} 
	AioFile* f = new AioFile(mEnv, *this);
	if( f->open( filename, readerType) < 0 ) {
		delete f;
		return 0;
	}
	f->sessionId(sessId);
	mFileInfos[attr] = f;
	return f;
}

void CacheCenter::close( AioFile* f ) {
	ZQ::common::MutexGuard gd(mLocker);
	if( f->clear() > 0 ) 
		return;
	AioFileAttr attr;
	attr.name = f->filename();
	attr.readerType = f->readerType();
	FILEINFOMAP::iterator it = mFileInfos.find( attr );
	assert( it != mFileInfos.end() );
	assert( it->second == f );
	mFileInfos.erase( it );
	delete f;
}

BufferUser CacheCenter::findBuInPendingSet( const std::string& filename, int64 offset ) {
	BufferUser bu;
	RangeInfo ri;
	ri.filename = filename;
	ri.offset = offset/mBufferSize*mBufferSize;
	{
		ZQ::common::MutexGuard gd(mLocker);
		PENDINGMAP::iterator it = mPendingData.find(ri);
		if(it != mPendingData.end()) {
			bu = it->second;
		}
	}
	return bu;
}

void CacheCenter::addToPendingSet( BufferUser& bu ) {
	if(!bu.valid())
		return;
	RangeInfo ri;
	ri.filename = bu.fileName();
	ri.offset = bu.offsetInFile(); // need not be ajusted
	ZQ::common::MutexGuard gd(mLocker);
	mPendingData[ri] = bu;
}

void CacheCenter::removeFromPendingSet( BufferUser& bu ) {
	if(!bu.valid())
		return;
	RangeInfo ri;
	ri.filename = bu.fileName();
	ri.offset = bu.offsetInFile(); //need not be adjusted
	ZQ::common::MutexGuard gd(mLocker);
	mPendingData.erase(ri);
}

int CacheCenter::run() {
	MLOG.info(CLOGFMT(CacheCenter, "request checker is running"));
	while(mbRunning) {
		mWatchReqSem.timedWait(1000);
		if(!mbRunning) {
			break;
		}
		WatchMap reqs;
		{
			ZQ::common::MutexGuard gd(mLocker);
			reqs = mWatchedReqs;
		}
		int64 timeNow = ZQ::common::now();
		for( WatchMap::const_iterator it = reqs.begin(); it != reqs.end(); it ++ ) {
			int64 reqTimeStamp = it->second->requestTimeStamp();
			if(reqTimeStamp == 0 ) {
				continue;
			}
			int64 delta = timeNow - reqTimeStamp;
			if( delta < 0 || delta > 1800 * 1000 ) {
				const std::string& filename = it->second->filename();
				MLOG.error(CLOGFMT(CacheCenter, "Aio Error, buffer request bufId[%ld] file[%s] timeout, request timestamp[%ld] now[%ld]"),
						it->first, filename.c_str(), reqTimeStamp, timeNow);
			}
		}

	}
	MLOG.info(CLOGFMT(CacheCenter, "request checker quit"));
	return 0;
}

void CacheCenter::makeRequest( AioFile* file, std::vector<BufferUser>& bufs ) {
	if(bufs.size() == 0) {
		return;
	}
	{
		ZQ::common::MutexGuard gd(mLocker);
		for (int i = 0; i < (int)bufs.size(); i++ )
		{
			BufferUser buf = bufs[i];
			buf->request();
			assert(!buf->mBuffStat && "one buffer request more than once.");
			buf->mBuffStat = true;
			buf->sessionId(file->sessionId());
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter, "makeRequest() for file[%s] offset[%lu], bufReqId[%ld] size[%d]."),
			      buf->filename().c_str(), buf->offsetInFile(), buf->reqId(), (int)buf->bufSize());
			mWatchedReqs[buf->reqId()] = buf.getInner();
		}
		mOutStandingIoReqCount += bufs.size();
	}
	getReader( bufs[0]->readerType() )->read(bufs);
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"makeRequest() read called with file[%s] buffers[%d] size[%ld] bufReqId[%ld]"), 
			file->filename().c_str(), (int)bufs.size(), bufs[0]->bufSize(), bufs[0]->reqId() );

}

class Unlocker {
public:
	Unlocker(ZQ::common::Mutex& m)
	:mLocker(m){
		mLocker.leave();
	}
	~Unlocker() {
		mLocker.enter();
	}
private:
	ZQ::common::Mutex& mLocker;
};

BufferUser CacheCenter::makeRequest( AioFile* file, uint64 offset, int readerType, bool withDiskCache) {
	BufferUser ret(*this,0);
	bool firstBlock = true;
	uint64 start = offset / mBufferSize * mBufferSize;

	static size_t readAheadCount = mEnv.getConfig().cacheCenter.mCacheReadaheadCount;
	static size_t blockCount = readAheadCount + 1;
	uint64 end = start + mBufferSize * blockCount;

	bool firstBlockAvailable = false;

	std::vector< std::vector<BufferUser> > requests;

	int64        	bufferSize = 0;
	size_t 			totalBufs = 0;
	int64			bitrate = file->getBitrate();
	{
		BufferUser bufret;
		std::vector<BufferUser> bufs;
		RangeInfo ri;
		ri.filename = file->filename();

		size_t blockIndex = 0;
		ZQ::common::MutexGuard gd(mLocker);
		for( uint64 i = start ; i < end; i += mBufferSize ) {
			ri.offset = i;
			blockIndex ++;
			CACHEDMAP::iterator it = mCachedData.find( ri );
			if( withDiskCache || it == mCachedData.end() ) {
				// create a new cache data entry
				BufferUser buf; 
				{
					//Unlocker ul(mLocker);
					BufferUser tmpBuf(*this, getBuffer());
					buf = tmpBuf;
				}
				if(!buf.valid()) {
					MLOG(ZQ::common::Log::L_EMERG, CLOGFMT(CacheCenter,"request() no available free buffer"));
					return ret;
				}
				assert(buf->vacant());
				buf->attach( file->filename(), file->fd(), i );
				//buf->reqId(genReqId());
				genReqIdForBuffer(buf.getInner());
				//buf->request();
				buf->readerType(readerType);
				buf->setBitrate(bitrate);
				MLOG.debug(CLOGFMT(CacheCenter, "makeRequest() set bitrate[%ld] to [%s], bufReqId[%ld]"),
						bitrate, file->filename().c_str(), buf->reqId());
				bufs.push_back( buf );
				bufferSize += buf->bufSize();

				if(!withDiskCache) {
					buf->mbCached = true; // mark this buffer as being cached
					mCachedData.insert( CACHEDMAP::value_type(ri, buf.getInner( ) ) );
				}

				if(firstBlock) {
					bufret = buf;
				}
			} else if( firstBlock ) {
				firstBlockAvailable = true;
				useBuffer(it->second);
				bufret = BufferUser(*this,it->second);
				if( !bufret.valid() )
				{
					assert(false && "failed to get new bufferUser.");
				}
			} else{
				/*
				if(blockIndex == 2 ) {
					if(firstBlockAvailable) {
						MLOG.info(CLOGFMT(CacheCenter,"makeRequest() first and follower block exist for [%s/%lu], skip reading more data."),file->filename().c_str(),start);
						break;
					}
				}
				*/

				if(bufs.size() > 0) {
					requests.push_back(bufs);
					totalBufs += bufs.size();
					bufs.clear();
				}
			} 

			firstBlock = false;
		}
		if(bufs.size()>0) {
			requests.push_back(bufs);
			totalBufs += bufs.size();
			bufs.clear();

			mOutStandingIoReqCount += totalBufs;
			file->newRequest( totalBufs );
		}
		ret = bufret;
	}

	std::vector< std::vector< BufferUser> > ::iterator itReq = requests.begin();
	for(; itReq != requests.end(); itReq ++ ) {
		std::vector<BufferUser>& bufs = *itReq;
		if(bufs.empty())
			continue;
		makeRequest(file, bufs);
	}
	return ret;

}

BufferUser CacheCenter::makeRequest( AioFile* file, uint64 offset, int readerType ) {
	return makeRequest( file, offset, readerType, false );
}

IDataReader::Ptr CacheCenter::getReader( int type ) {
	if( type < 0 || type >= (int)mReaders.size()) {
		assert(false && "Invalid Reader Type");
		return NULL;
	}
	return mReaders[type];
}

void CacheCenter::queryAssetAttributes(const std::string& filename, AssetAttribute::Ptr attr, int readerType ) {
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(CacheCenter,"query attribute for:%s"),filename.c_str());
	getReader(readerType)->queryIndexInfo( filename, attr);
}

BufferUser CacheCenter::findBuffer( AioFile* file, uint64 offset ) {
	uint64 start = offset / mBufferSize * mBufferSize;
	RangeInfo ri;
	ri.filename = file->filename();
	ri.offset = start;
	ZQ::common::MutexGuard gd(mLocker);
	CACHEDMAP::iterator it = mCachedData.find(ri);
	if( it == mCachedData.end())
		return BufferUser(*this,0);
	else
		return BufferUser(*this,it->second);
}

void CacheCenter::requestComplete( const std::vector<BufferUser>& bufs, bool fromDiskCache ) {
	ZQ::common::MutexGuard gd(mLocker);
	for(std::vector<BufferUser>::const_iterator itBuf = bufs.begin(); itBuf != bufs.end(); itBuf ++ ) {
		BufferUser buf = *itBuf;
		mWatchedReqs.erase(buf->reqId());
		if(!fromDiskCache){
			buf->mBuffStat = false;
		}
		AioFileAttr fileAttr;
		fileAttr.name = buf->filename();
		fileAttr.readerType = buf->readerType();
		FILEINFOMAP::iterator it = mFileInfos.find(fileAttr);
		if( it != mFileInfos.end() ) {
			AioFile* file = it->second;
			if(file->requestDone(1) <= 0 ) {
				delete file;
				mFileInfos.erase(it);
			}
		}
		int64 timeoutInterval = mEnv.getConfig().cacheCenter.defaultTimeOut;
		int lastError = buf->lastError();
		if(lastError != 0 ) {
			int category = lastError >> 28;
			switch( category ) {
				case Buffer::ECATE_FILEIO:
					timeoutInterval = mEnv.getConfig().cacheCenter.fioErrTimeOut;
					break;
				case Buffer::ECATE_HTTP:
					timeoutInterval = mEnv.getConfig().cacheCenter.httpErrTimeOut;
					break;
				case Buffer::ECATE_SOCKET:
					timeoutInterval = mEnv.getConfig().cacheCenter.socketErrTimeOut;
					break;
				case Buffer::ECATE_TIMEOUT:
					timeoutInterval = mEnv.getConfig().cacheCenter.otherErrTimeOut;
					break;
				case Buffer::ECATE_CLIENTTIMEOUT:
					timeoutInterval = 0; // evict from cache immediately
					break;
				default:
					break;
			}
		}
		if(!fromDiskCache) {
			buf->updateTimer( timeoutInterval );
			mExpireChecker.add(buf.getInner());
		}

		removeFromPendingSet(buf);
		MLOG.debug(CLOGFMT(requestComplete, "about to signal buffer[%zu] [%ld]"),
				buf.getInner()->id(), buf.getInner()->reqId());
		buf->signal();
	}
	assert( mOutStandingIoReqCount >= bufs.size() );
	mOutStandingIoReqCount -= bufs.size();
}

bool CacheCenter::listEmpty() const {
	return !mHead || mHead->vacant();
}

void CacheCenter::markAsInfly( Buffer* buf ) {
	assert( buf->flyNext() == buf && buf->flyPrev() == buf );
	Buffer* last = mHead->flyPrev();
	last->flyNext() = buf;
	mHead->flyPrev() = buf;

	buf->flyNext() = mHead;
	buf->flyPrev() = last;

	mFlyBufferCount ++;
}

void CacheCenter::unmarkInFly( Buffer* buf, bool bFirstAdd) {
	if(!bFirstAdd) {
		assert( buf->flyNext() != buf && buf->flyPrev() != buf );
	}
	buf->flyNext()->flyPrev() = buf->flyPrev();
	buf->flyPrev()->flyNext() = buf->flyNext();
	buf->flyNext() = buf->flyPrev() = buf;
	mFlyBufferCount --;
}

void CacheCenter::chainBuffer( Buffer* buf, bool bFirstAdd ) {
	if(buf->mbPauseFrameData) {
		return;
	}
	ZQ::common::MutexGuard gd(mLocker);
	if(!bFirstAdd) {
		assert(buf->users() == 0 );
	}
	Buffer* head = mHead;
	assert( head != 0 );
	unchainBuffer( buf );

	Buffer* last = head->prev();
	last->next() = buf;
	head->prev() = buf;

	buf->next() = head;
	buf->prev() = last;

	unmarkInFly(buf,bFirstAdd);

	mFreeBufferCount ++;
	//MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"chainBuffer:Id[%zu/%ld] free[%zu] total[%zu] cached[%zu] outstanding[%zu] :%p"),
	//		buf->id(), buf->reqId() , mFreeBufferCount, mBufferCount, mCachedData.size(), mOutStandingIoReqCount, buf );
}

void CacheCenter::unchainBuffer( Buffer* buf ) { 
	assert(!buf->mbPauseFrameData);
	{
		ZQ::common::MutexGuard gd(mLocker);
		if( buf->vacant() )
			return;
		buf->next()->prev() = buf->prev();
		buf->prev()->next() = buf->next();
		buf->prev() = buf->next() = buf;
		mFreeBufferCount --;
		markAsInfly(buf);
	}
	assert(buf->vacant());
	//MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"unchainBuffer:Id[%zu/%ld] free[%zu] total[%zu] cached[%zu] outstanding[%zu]: %p"),
	//		buf->id(), buf->reqId(), mFreeBufferCount, mBufferCount, mCachedData.size(), mOutStandingIoReqCount, buf );
}

void CacheCenter::invalidateCache( const std::string& filename, uint64 offset, Buffer* hint ) {
	RangeInfo ri;
	ri.filename = filename;
	ri.offset = offset / mBufferSize * mBufferSize;

	ZQ::common::MutexGuard gd(mLocker);
	CACHEDMAP::iterator it = mCachedData.find(ri);
	if( it == mCachedData.end()) {
		return;
	}
	Buffer* buf = it->second;
	if( hint ){
		assert( buf == hint );
	}
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"invalidateCache, evict cached data: %s :%lu, now:%ld timeoutTarget:%ld"),
			filename.c_str(), offset, ZQ::common::now(), buf->timeoutTarget());
	if( buf->inExpireChecker())
		mExpireChecker.remove(buf);
	assert( buf->mbCached );
	buf->mbCached = false;
	mCachedData.erase(it);
}

void CacheCenter::useBuffer( Buffer* buf ) {
	unchainBuffer(buf);
}

BufferUser CacheCenter::makeEmptyBufUser() {
	Buffer *buf = getBuffer();
	genReqIdForBuffer(buf);
	BufferUser bu(*this, buf);
	return bu;
}

Buffer* CacheCenter::getBuffer( ) {
	bool release = false;
	{
		ZQ::common::MutexGuard gd(mLocker);
		///int minFreeBuffer = mBufferCount / 100;
		if( mBufferCount / 200 >= mFreeBufferCount )
			release = true;
	}
	if( release )
		XOR_Media::DiskCache::CacheDir::releaseBuffer();

	{
		ZQ::common::MutexGuard gd(mLocker);
		if( listEmpty() ) {
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(CacheCenter,"getBuffer() freeBuff[%zu] totalBuff[%zu], try to add new buffer."), mFreeBufferCount, mBufferCount);
			addBuffer();
			if( listEmpty() ) {
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(CacheCenter,"getBuffer() failed to allocate new buffer block"));
				return NULL;
			}
		}
		Buffer* buf = mHead->next();
		assert(!buf->mBuffStat);
		//assert(buf->reqCount == 0);
		assert(buf->users() <= 0 );
		if(buf->mbCached) {
			invalidateCache( buf->filename(), buf->offsetInFile(), buf );
		}
		if(buf->inExpireChecker())
			mExpireChecker.remove(buf);
		unchainBuffer(buf);
		//MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"getBuffer(), get from free list, info of buffer: file[%s:%ld] timeoutTarget[%ld]"),
		//		buf->filename().c_str(), buf->offsetInFile(), buf->timeoutTarget());
		assert(buf->vacant());
		buf->reset();
		return buf;
	}
}

void CacheCenter::evictCache( const std::string& filename, uint64 offset, size_t size ) {
	uint64 start = offset /mBufferSize*mBufferSize;
	uint64 end = ( offset + size + mBufferSize - 1) / mBufferSize * mBufferSize;
	for( uint64 i = start; i < end; i += mBufferSize) {
		invalidateCache( filename, i );
	}
}

void CacheCenter::innerEvictCache( const std::string& filename,uint64 offset ) {
	ZQ::common::MutexGuard gd(mLocker);
	invalidateCache( filename, offset );
}

void CacheCenter::refBuffer( Buffer* buf) {
	//int vOld = buf->mUsers.get();
	//int vNew = buf->mUsers.inc();
	buf->mUsers.inc();
	//MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter, "refBuffer() inc  reference count [%d==>%d] of buf[%ld]"),vOld, vNew, buf->reqId());
}

void CacheCenter::unrefBuffer( Buffer* buf) {
	ZQ::common::MutexGuard gd(mLocker);
	if( (buf->mUsers.dec() <= 0) && (!buf->mBuffStat ) ) {
	 //MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"unrefBuffer:Id[%zu/%ld] :%p"), buf->id(), buf->reqId() ,buf );
	 assert( buf->vacant() );
	 chainBuffer(buf);
	}
}

void CacheCenter::onIndexInfo( AssetAttribute::Ptr attr ) {
	if(!attr)
		return;
	IReaderCallback* cbInAttr = attr->getReaderCallback();
	if(cbInAttr != NULL) {
		//interception here
		cbInAttr->onIndexInfo(attr);
		return;
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(CacheCenter,"onIndexInfo() got asset attribute for [%s]: pwe[%s] lastError[%d]"),
			attr->filename().c_str(), attr->pwe()?"true":"false", attr->lastError());
	attr->signal();
}

bool CacheCenter::checkData( BufferUser& buf, const std::string& hint)
{
	std::string fileName = buf->filename();
	if( fileName.find(".0X0000") == std::string::npos) {
		return true;
	}

	size_t dataSize = buf->getDataSize();
	if(dataSize <= 0)
		return false;
	int64 offset = buf->offsetInFile();
	//int64 end = offset + dataSize;

	int64 pos = 188-offset % 188;
	for( ; pos < (int64)(dataSize - 188); pos+=188 * 200) {
		char ch = *(buf->buffer() + pos);
		if(ch != 0x47){
			MLOG.debug(CLOGFMT(CacheCenter, "checkData() file[%s] offset[%ld/%ld] bufReqId[%ld] bad data, hint[%s]"),
				buf->filename().c_str(), buf->offsetInFile(), pos, buf.bufReqId(), hint.c_str());
			break;
		}
	}
	return true;
}

void    CacheCenter::onRead( const std::vector<BufferUser>& bufs, bool fromDiskCache )
{
	StopWatch sw;sw.start();
	//MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(CacheCenter,"onRead() entry with buffers[%d]."), (int)bufs.size());
	for (int i = 0; i < (int)bufs.size(); i++ )
	{
		BufferUser buf = bufs[i];
		if(!fromDiskCache)
		{
			assert(buf->mBuffStat && "one buffer response more than once.");
			buf->mBuffStat = false;
		}
		checkData(buf, "onRead");
		const std::string& fileName = buf->filename();
		std::string fet = fileName.substr(fileName.rfind('.') + 1);
		bool cache = true;
		if( mEnv.getConfig().diskCache.cacheIgnoreFiles.find(fet) != std::string::npos)
			cache = false;

		if(cache && 
				mEnv.getConfig().diskCache.diskCacheEnabled && 
				!fromDiskCache && 
				buf->lastError() == 0 && 
				CLIENT_TYPE_DISKAIO != buf->readerType()) {
			int64 timeNow = ZQ::common::now();
			XOR_Media::DiskCache::CacheDir::write_async( buf, timeNow, this);
		}
		static bool enableCRCCheck = mEnv.getConfig().cacheCenter.calcCRCForBuffer;
		if(enableCRCCheck) {
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter, "onRead() get file[%s] data [%lu: %d], bufReqId[%ld] size[%d/%d] timeCost[%ld] crc[%X]."),
				buf->filename().c_str(), buf->offsetInFile(), (int)buf->getDataSize(), buf->reqId(), (int)buf->getDataSize(), (int)buf->bufSize(), 
				buf->bufferPrepareTime(), ZQ::common::CRC32::crc32(0, buf.data(), buf.dataLeft()));
		} else {
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter, "onRead() get file[%s] data [%lu: %d], bufReqId[%ld] size[%d/%d] timeCost[%ld]."),
				buf->filename().c_str(), buf->offsetInFile(), (int)buf->getDataSize(), buf->reqId(), (int)buf->getDataSize(), (int)buf->bufSize(), buf->bufferPrepareTime());
		}
	}
	requestComplete(bufs, fromDiskCache);
	//MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(CacheCenter,"onRead() buffers[%d] cost[%ld]us to notify BufferUser"), (int)bufs.size(), sw.stop());
	return;
}

void CacheCenter::onLatency(std::string& fileName, int64 offset,int64 time)
{
	/*
	   int lencty = mEnv.getLatencyMap(fileName, offset, time);
	   if (lencty < 0)
	   return;
	   MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter, "onLatency() get file[%s] offset[%ld] latency[%d]."),fileName.c_str() , offset, lencty);
	   if( 1 == mEnv.getConfig().clientType )
	   {
	   int64 timeNow = ZQ::common::TimeUtil::now();
	   mEnv.setLatencyMap(fileName, offset, timeNow);
	   }*/			
	return;
} 

void CacheCenter::onError( int err )
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(CacheCenter,"onError() entry with errcode[%d]."), err);
}

void CacheCenter::onCacheRead(Error errCode, BufferUser buf)
{
	std::vector<BufferUser> bufVec(1,buf);
	{
		ZQ::common::MutexGuard gd(mLocker);
		if(buf->mbReturnedFromDiskCache) {
			MLOG.debug(CLOGFMT(CacheCenter,"onCacheRead, buf[%s:%ld] bufReqId[%ld] already returned" ),
					buf.fileName().c_str(), buf.offsetInFile(), buf.reqId() );
			return;
		}
		buf->mbReturnedFromDiskCache = true;
	}

	if(buf.valid()) {
		if( (errCode ==  DiskCacheSink::cacheErr_OK || errCode == DiskCacheSink::cacheErr_Hit) && buf.valid())	{
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter,"onCacheRead() buf[%s:%lu] bufReqId[%ld] gotten"),
					buf.fileName().c_str(), buf.offsetInFile(), buf.reqId() );
			{
				ZQ::common::MutexGuard gd(mLocker);
				mOutStandingIoReqCount ++;
			}
			//buf.mBuf->mBuffStat = true; //make onRead assert happy
			onRead(bufVec, true);
			return;
		}
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter,"onCacheRead() buf[%s:%lu] bufReqId[%ld] missed"),
				buf.fileName().c_str(), buf.offsetInFile(), buf.reqId() );
		//buf->setDataSize(0);
		AioFile *file = open(buf.fileName(), buf->readerType(), buf->sessionId());
		file->setBitrate(buf->getBitrate());
		assert( file!=NULL ); 
		makeRequest(file, bufVec );
	} else {
		MLOG.error(CLOGFMT(CacheCenter, "onCacheRead, invalid BufferUser passed in"));	
		assert(false);//logic error
	}
}

void CacheCenter::onCacheWrote(Error errCode/*, const std::string& url*/)
{
	//do nothing
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheCenter, "onCacheWrote() entry with[%s]. "), DiskCacheSink::errorToA(errCode));
}

BufferUser CacheCenter::getCacheBufUser(const std::string& fileName, const int64 stampAsOfOrign, int64 bitrate, long readerType) {
	Buffer* buf = getBuffer();
	if(!buf) {
		return BufferUser();
	}
	buf->setBitrate(bitrate);
	buf->attach(fileName, 0, stampAsOfOrign);
	//buf->reqId( genReqId());
	genReqIdForBuffer(buf);
	buf->readerType((int)readerType);
	MLOG.debug(CLOGFMT(CacheCenter, "getCacheBufUser() set bitrate[%ld] to file[%s], bufReqId[%ld]"), bitrate, fileName.c_str(),buf->reqId());
	buf->request();
	useBuffer(buf);
	assert(buf->mBuffStat == false);
	BufferUser bu(*this,buf);
	return bu;
}

void CacheCenter::getFlyBuffersStatus( std::vector<CacheBufferStatusInfo>& bufs ) const {
	ZQ::common::MutexGuard gd(mLocker);
	if(mFlyBufferCount <= 0 )
		return;
	bufs.resize( mFlyBufferCount);
	Buffer* p = mHead->flyNext();
	CacheBufferStatusInfo bufInfo;
	while( p != mHead) {
		bufInfo.bufId = p->id();
		bufInfo.reqId = p->reqId();
		bufs.push_back(bufInfo);
	}
}

IdxRecPtr CacheCenter::findByLRUMap(const std::string& filename, bool& bNew)
{
	std::string realname = filename + ".index";
	IdxRecPtr pIdxRec = NULL;
	{
		ZQ::common::MutexGuard gd(mLocker);
		pIdxRec = mIdxRecMap[realname];
		if(!pIdxRec) {
			bNew = true;
			pIdxRec = new IdxRecData();
			assert(pIdxRec);
			mIdxRecMap[realname] = pIdxRec;
			pIdxRec->SetUnParsed();
		} else {
			bNew = false;
			return pIdxRec;
		}
		/*
		std::map<std::string,IdxRecPtr>::iterator it = mIdxRecMap.find(realname);
		if (it != mIdxRecMap.end()){
			bNew = false;
			return  it->second;
		} else {
			bNew = true;
			pIdxRec = new IdxRecData;
			if (pIdxRec){
				mIdxRecMap[realname] = pIdxRec;
			}
		}
		*/
	}
	return pIdxRec ;
}
const IdxRecPtr CacheCenter::getIdxRecByIdxName(const std::string& filename,bool& bNew)
{
	IdxRecPtr idxPtr = findByLRUMap(filename, bNew);
	if (idxPtr)
	{
		if (!bNew){
			idxPtr->wait();
		}
		return idxPtr;
	}
	return NULL;
}
///////////////////////////////////////
// AioFile
AioFile::AioFile( C2StreamerEnv& env, CacheCenter& cc )
:mEnv(env),
mCc(cc),
mFd(-1),
mUsers(0),
mOutstandingReqs(0),
mbFirstRead(true),
mBitrate(0) {
	mReaderType = -1;
}

AioFile::~AioFile( ) {
	if(mFd<0)
		return;
	::close(mFd);
	mFd = -1;
}

void AioFile::close( ) {
	mCc.close(this);
}

void AioFile::setBitrate( int64 bitrate ) {
	ZQ::common::MutexGuard gd(mLocker);
	if(mBitrate == bitrate) {
		return;
	}
	//MLOG.debug(CLOGFMT(AiOFile, "trying to set bitrate [%ld] to [%s]"), bitrate, mFileName.c_str() );
	if( bitrate > mBitrate) {
		MLOG.debug(CLOGFMT(AiOFile, "set bitrate [%ld] to [%s]"), bitrate, mFileName.c_str() );
		mBitrate = bitrate;
	}
}

long AioFile::open( const std::string& filename, int readerType ) {
	mReaderType = readerType;
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AioFile,"open() entry with file[%s] readerType[%d]."), filename.c_str(), readerType);
	if (CLIENT_TYPE_DISKAIO == readerType )
	{
		std::string aioFile = C2Streamer::fsConcatPath(mEnv.getDocumentRootFolder(), filename);
		if (mFd < 0)
		{
			mFd = ::open( aioFile.c_str(), O_RDONLY|O_DIRECT );
			if( mFd < 0 ) {
				MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AioFile,"open() failed to open file[%s], error[%d]"), aioFile.c_str(), errno);
				return -1;
			}
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AioFile,"open()  open aiofile[%s] successful."), aioFile.c_str());
			mFileName = filename;
			mUsers = 1;
		}
		else{
			++ mUsers;
		}
	}
	else if ( CLIENT_TYPE_C2CLIENT == readerType || 
			CLIENT_TYPE_HTTPFETCHER == readerType ||
			CLIENT_TYPE_AQUAREADER == readerType ||
			CLIENT_TYPE_HTPPFETCHER2 == readerType || 
			CLIENT_TYPE_HYBRID == readerType ||
			CLIENT_TYPE_HYBRID_C2CLIENT == readerType)
	{
		mFd = -1;
		mFileName = filename;
		if (mUsers < 1){
			mUsers = 1;
		}
		else{
			++ mUsers;
		}
	}
	else
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AioFile,"open() not valid client type[%d]."), readerType);
		return -1;
	}
	return mUsers + mOutstandingReqs;
}

void AioFile::invalidateCache( uint64 offset ) {
	mCc.evictCache( mFileName, offset, 64*1024 );
}

long AioFile::clear( ) {
	if( -- mUsers < 0 ){
		assert(false);
		mUsers = 0;
	}
	return mUsers + mOutstandingReqs;
}

long AioFile::newRequest( long count ) {
	mOutstandingReqs += count;
	return mUsers + mOutstandingReqs;
}

long AioFile::requestDone( long count ) {
	if( ( mOutstandingReqs -= count )< 0 ) {
		//assert(false);
		mOutstandingReqs = 0 ;
	}
	return mUsers + mOutstandingReqs;
}

BufferUser AioFile::read( uint64 offset, int64 bitrate ) {
	static size_t bufferSize = mCc.getBufferSize();
	int64 reminder = offset % bufferSize;
	offset = offset - reminder;

	if(bitrate == 0) {
		bitrate = mBitrate;
	}

	BufferUser bu(mCc, NULL);
	std::string fet = mFileName.substr(mFileName.rfind('.') + 1);
	bool cache = true;
	if( mEnv.getConfig().diskCache.cacheIgnoreFiles.find(fet) != std::string::npos)
		cache = false;
	if( cache && mEnv.getConfig().diskCache.diskCacheEnabled && CLIENT_TYPE_DISKAIO != mReaderType ) {
		int64 timeNow = ZQ::common::now();
		bu = mCc.findBuInPendingSet( mFileName, offset );
		if( !bu.valid() ) {
			bu =  XOR_Media::DiskCache::CacheDir::read_async( mFileName, offset, timeNow, bitrate, mReaderType, &mCc );
			bu->setBitrate(bitrate);
			//mCc.addToPendingSet(bu);
		}
		{
			uint64 start = offset / bufferSize * bufferSize;
			static size_t readAheadCount = mEnv.getConfig().cacheCenter.mCacheReadaheadCount;
			static size_t blockCount = readAheadCount + 1;
			uint64 end = start + bufferSize * blockCount;

			start += bufferSize;//skip the first block
			for( uint64 i = start; i < end; i += bufferSize) {
				BufferUser tmpBu = XOR_Media::DiskCache::CacheDir::read_async( mFileName, i, timeNow + i, bitrate, mReaderType, &mCc );
				tmpBu->setBitrate(bitrate);
			}
		}


	} else {
		bu =  mCc.makeRequest( this, offset, mReaderType, false );
	}
	bu.seek( reminder );
	mbFirstRead = false;
	return bu;

}

//////////////////////////////////////////////////////////
// Buffer
Buffer::Buffer( CacheCenter& cc, char* addr, size_t size, size_t id, bool isPausedFrame )
:mCc(cc),
mId(id),
mBuf(addr),
mBufSize(size),
mFileSize(-1),
mbCached(false),
mbReturnedFromDiskCache(false),
mBitrate(0),
mReaderType(-1),
mbPauseFrameData(isPausedFrame){
	mPrev = mNext = this;
	mFlyNext = mFlyPrev = this;
	mExpireNext = mExpirePrev = this;
	reset();
	//reqCount = 0;
	//resCount = 0;
	mBuffStat = false;
}

Buffer::~Buffer( ) {
}

void Buffer::reset( ) {
	mSize.set(0);
	mbFilled = false;
	mLastError = 0;
	mFd = 0;
	mOrigOffset = 0;
	mReqTimeStamp = 0;
	mRespTimeStamp = 0;
	mTimeoutTarget = 0;
	mbCached = false;
	mFileSize = -1;
	mReaderType = -1;
	mBitrate = 0;
	mbReturnedFromDiskCache = false;
}

void Buffer::setBitrate(int64 rate) {
	//mCc.getEnv().getLogger()->debug(CLOGFMT(Buffer,"setBitrate id[%zu] bufReqId[%ld] set bitrate to [%ld]"), mId, mReqId, rate);
	mBitrate = rate;
}

void Buffer::updateTimer(int64 delta) {
	assert( mTimeoutTarget == 0 );
	mTimeoutTarget = ZQ::common::now() + delta;
}

void Buffer::request( ) {
	mReqTimeStamp = ZQ::common::now();
}

bool Buffer::asyncWait( IAsyncNotifySinker::Ptr sinker, size_t size ) {
	AsyncNotifyInfo info;
	info.sinker = sinker;
	info.size = size;

	ZQ::common::MutexGuard gd(mLocker);

	if(mbFilled)
		return false;

	if ( (int)size<= mSize.get() )
		return false;

	mNotifyInfos.push_back(info);
	return true;
}

void Buffer::setDataSize( size_t size ) {
	//mCc.getEnv().getLogger()->debug(CLOGFMT(Buffer,"setDataSize() id[%zu] bufReqId[%ld] set size to [%zu]"), mId, mReqId, size);
	assert(mSize.get() <= size);
	mSize.set((int)size);
	if(size >0) {
		notify();
	}
}

void Buffer::signal() {
	mbFilled = true;
	mRespTimeStamp = ZQ::common::now();
	notify();
}

void Buffer::notify( ) {
	mRespTimeStamp = ZQ::common::now();
	AsyncNotifyInfoS infos;
	infos.reserve(16);
	AsyncNotifyInfoS retries;
	retries.reserve(16);
	{
		ZQ::common::MutexGuard gd(mLocker);
		infos.swap(mNotifyInfos);
	}
	if(infos.size() == 0 ) {
		return;
	}

	//mCc.getEnv().getLogger()->debug(CLOGFMT(Buffer,"notify() id[%zu] bufReqId[%ld] size [%zu], start to notify users"), mId, mReqId, infos.size());
	AsyncNotifyInfoS::iterator it = infos.begin();
	for( ; it != infos.end(); it ++ ) {
		if( mbFilled || (int)it->size <= mSize.get() ) {
			it->sinker->onNotified();
		} else {
			retries.push_back(*it);
		}
	}
	{
		ZQ::common::MutexGuard gd(mLocker);
		mNotifyInfos.insert( mNotifyInfos.end(), retries.begin(), retries.end());
	}
}


void Buffer::get( ) {
	mCc.refBuffer(this);
}

void Buffer::put( ) {
	mCc.unrefBuffer(this);
}

void Buffer::attach( const std::string& filename, int fd, uint64 offsetInFile ) {
	mFd = fd;
	mFileName = filename;
	mOrigOffset = offsetInFile;
}

char* Buffer::data( ) {
	return mBuf;
}

/////////////////////////////////////////////////
/// BufferUser
BufferUser::BufferUser( CacheCenter& cc, Buffer* buf )
:mCc(&cc),
mBuf(buf),
mOffset(0) {
	if(mBuf) {
		mBuf->get();
	}
}

BufferUser::BufferUser(Buffer* buf)
:mCc(NULL),
mBuf(buf),
mOffset(0) {
	if(mBuf) {
		mBuf->get();
	}
}

BufferUser::BufferUser()
:mCc(NULL),
mBuf(NULL),
mOffset(0){
}

BufferUser::BufferUser( const BufferUser& b )
:mCc(b.mCc),
mBuf(0){    
	mOffset = b.mOffset;
	if(mBuf != b.mBuf) {
		adjustBitrate(b.mBuf);
		Buffer* temp = mBuf;
		mBuf = b.mBuf;
		if(mBuf)
			mBuf->get();
		if(temp)
			temp->put();
	}   
}

BufferUser::~BufferUser() {
if(mBuf) {
	mBuf->put();
}
mBuf = 0;
}

BufferUser& BufferUser::operator=( const BufferUser& b ) {  
	mOffset = b.mOffset;
	if(mBuf != b.mBuf) {
		adjustBitrate(b.mBuf);
		Buffer* temp = mBuf;
		mBuf = b.mBuf;
		if(mBuf)
			mBuf->get();
		if(temp)
			temp->put();
	}   
	mCc = b.mCc;
	return *this;
}

bool BufferUser::asyncWait( IAsyncNotifySinker::Ptr sinker, size_t size ) {
	if( !mBuf ) {
		assert(false);
		return false;
	}
	return mBuf->asyncWait( sinker, mOffset + size );
}


bool BufferUser::filled() const {
	return mBuf != 0 && mBuf->filled();
}

size_t BufferUser::seek( size_t offset ) {
	if(!mBuf) return 0;
	mOffset = offset;
	if( mOffset > (size_t)mBuf->mBufSize ) {
		mOffset = (size_t)mBuf->mBufSize;
	}
	return mOffset;
}

size_t BufferUser::tell( ) const {
	return mOffset;
}

size_t BufferUser::dataLeft( ) const {
	if( !mBuf) {
		assert(false);
		return 0;
	}

	if( mOffset >= (size_t)mBuf->mSize.get() ) 
		return 0;
	return (size_t)mBuf->mSize.get() - mOffset;
}

size_t BufferUser::advance( size_t dataSize ) {
	mOffset += dataSize;
	assert( mOffset <= (size_t)mBuf->mSize.get() );
	return mOffset;
}

char* BufferUser::data( ) {
	if(!mBuf) return 0;
	if( dataLeft() <= 0 ) return 0;
	return mBuf->data() + mOffset;
}

char* BufferUser::getBuffer( ){
	return mBuf->data();
}

void BufferUser::adjustBitrate( const Buffer* rhs) {
	if(rhs == NULL || mBuf == NULL) {
		return;
	}
	int64 bitrate = mBuf->getBitrate();
	if(bitrate < rhs->getBitrate()) {
		bitrate = rhs->getBitrate();
	}
	mBuf->setBitrate(bitrate);
}

bool BufferUser::copyFrom(BufferUser& rhs) {
	if( !valid() || !rhs.valid()) {
		return false;
	}
	assert( mBuf->bufSize() == rhs->bufSize() );
	if(mCc) {
		mCc->checkData(*this, "diskCacheCopy");
	}
	memcpy(mBuf->buffer(), rhs->buffer(), rhs->getDataSize());
	mBuf->setDataSize(rhs->getDataSize());
	adjustBitrate(rhs.mBuf);
	return true;
}

ExpireChecker::ExpireChecker( C2StreamerEnv& env, CacheCenter& cc )
:mEnv(env),
mCc(cc),
mHead(cc,0,0,0),
mbRunning(false){
}

ExpireChecker::~ExpireChecker() {
}

bool ExpireChecker::start() {
	mbRunning = true;
	return ZQ::common::NativeThread::start();
}

void ExpireChecker::stop( ) {
	if(!mbRunning)
		return;
	mbRunning = false;
	mbSignalled = true;
	mCond.notify_one();
	waitHandle(-1);
}

void ExpireChecker::add( Buffer* buf ) {
	if(!buf)
		return;
	bool signal = false;
	{
		boost::mutex::scoped_lock  gd(mLocker);
		if(buf->inExpireChecker())
			return;
		buf->addToExpireChecker( &mHead);
		mBuffers.insert(buf);
		signal = buf->timeoutTarget() < mNextWakeup;
		if(signal)
			mbSignalled = true;
	}
	if(signal) {
		mCond.notify_one();
	}
}

void ExpireChecker::remove( Buffer *buf ) {
	if(!buf)
		return;
	boost::mutex::scoped_lock gd(mLocker);
	if(!buf->inExpireChecker())
		return;
	buf->removeFromExpireChecker();
	mBuffers.erase(buf);
}

int ExpireChecker::run( ){
	int64 idledefault = 10 * 1000;
	int64 idlemin = 5;
	int64 idle = idledefault;
	mNextWakeup = ZQ::common::now() + idle;
	mbSignalled = false;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(expireChecker,"running"));
	while( mbRunning ) {
		{
			boost::mutex::scoped_lock gd(mLocker);
			if(!mbSignalled)
				mCond.timed_wait(gd, boost::posix_time::milliseconds(idle));
			mbSignalled = false;
		}
		if(!mbRunning)
			break;
		while( true ) {
			Buffer* buf = 0;
			{
				boost::mutex::scoped_lock gd(mLocker);
				int64 timenow = ZQ::common::now();
				if( mBuffers.empty() ) {
					idle = idledefault;
					mNextWakeup = timenow + idle;
					break;
				}
				buf = *mBuffers.begin();
				if( buf->timeoutTarget() <= timenow ) {
					buf->removeFromExpireChecker();
					mBuffers.erase(mBuffers.begin());
					MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheCenter,"ExpireChecker() buffer[%s:%ld] target:%ld, now:%ld"),
							buf->filename().c_str(), buf->offsetInFile(), buf->timeoutTarget(), timenow );
				} else {
					idle = timenow - buf->timeoutTarget();
					if( idle < idlemin ) 
						idle = idlemin;
					mNextWakeup = timenow + idle;
					buf = 0;
					break;
				}
			}
			if(buf)
				mCc.innerEvictCache( buf->filename(), buf->offsetInFile() );
		}
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(expireChecker,"stopped"));
	return 0;
}


///////////////////////////////////
//AioRunner
AioRunner::AioRunner( C2StreamerEnv& env, AioClient::Ptr ptr ):
	mEnv(env), mClientPtr(ptr),mbRunning(false), mReaper(env,ptr) {
		memset( &mIoCtx, 0, sizeof(mIoCtx) );
	}

AioRunner::~AioRunner(){
	stop();
	mClientPtr = NULL;
}

bool AioRunner::start( size_t groupCount) {
	stop();
	int rc = io_setup( 65536/groupCount,&mIoCtx);
	if( rc != 0 ) {
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AioRunner,"failed to invoke io_setup:[%d]"),rc);
		return false;
	}
	mbRunning = true;
	if(!mReaper.start(&mIoCtx))
		return false;
	return ZQ::common::NativeThread::start();
}

void AioRunner::stop( ) {
	if( !mbRunning )
		return;
	mbRunning = false;
	waitHandle(-1);
	mReaper.stop();
	io_destroy(mIoCtx);
}

size_t AioRunner::pushBuffer( BufferUser b ) {
	boost::mutex::scoped_lock gd(mLocker);
	mList.push_back(b);
	mbSignalled = true;
	mCond.notify_one();
	return ++mBufferCount;
}

size_t AioRunner::pushBuffer( const std::vector<BufferUser>& bufs ) {
	boost::mutex::scoped_lock gd(mLocker);
	std::vector<BufferUser>::const_iterator it = bufs.begin();
	for( ; it != bufs.end(); it ++ ) {
		mList.push_back(*it);
	}
	mbSignalled = true;
	mCond.notify_one();
	return mBufferCount += bufs.size();
}


int AioRunner::run( ) {
	{
		int policy;
		struct sched_param param;

		pthread_getschedparam(pthread_self(), &policy, &param);
		param.sched_priority = sched_get_priority_max(policy);
		pthread_setschedparam(pthread_self(), policy, &param);
	}

	BufferList bufs;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(AioRunner,"running"));

	std::vector<struct iocb*> iocbs;
	iocbs.reserve(32);

	while(mbRunning) {
		bufs.clear();
		{
			boost::mutex::scoped_lock gd(mLocker);
			if(mList.size() == 0 ) {
				mCond.timed_wait( gd, boost::posix_time::milliseconds(500) );
			}
			mbSignalled = false;
			if(!mbRunning)
				break;
			//bufs.swap(mList);
			if(mList.size() > 0 ) {
				bufs.push_back(mList.front());
				mList.pop_front();
				mBufferCount --;
			}
		}
		if( bufs.size() == 0 )
			continue;

		if( iocbs.capacity() < bufs.size() ) {
			iocbs.reserve(bufs.size());
		}
		iocbs.clear();

		BufferList::iterator it = bufs.begin();
		size_t pos = 0;
		for( ; it != bufs.end(); it ++ ) {
			BufferUser& buf = *it;
			// 因为可能存在一个情况就是没有任何一个地方存放有对Buffer的应用，那么会造成
			// Buffer被提前释放， 所以需要在这里持有对于Buffer的引用
			// 然后在收到读完成时间以后再去掉这个引用
			buf.getInner()->get();
			struct iocb* cb = (struct iocb*)malloc(sizeof(struct iocb));
			io_prep_pread( cb, buf->mFd, buf->mBuf, buf->mBufSize, buf->mOrigOffset );
			cb->data = reinterpret_cast<void*>(buf.getInner());
			iocbs[pos] = cb;
			pos ++;
		}

		StopWatch sw;sw.start();
		int rc = io_submit(mIoCtx, (int)bufs.size(), &iocbs[0]);
		sw.stop();
		if( rc != (int)bufs.size()) {
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AioRunner,"failed to submit io request: error[%d]"),rc);
			it = bufs.begin();
			size_t committedIoCb = 0;
			if(rc >0 && rc < (int)bufs.size() ) {
				for( int commitedioCb = 0 ; committedIoCb< rc; committedIoCb ++ ) {
					mReaper.addNewCB(iocbs[commitedioCb]);
					it++;
				}
			}						
			for( ; it != bufs.end(); it ++ ) {
				BufferUser& buf = *it;
				buf.getInner()->put(); //commit失败，那么现在去掉所有的引用？
				free(iocbs[committedIoCb++]);
			}
		} else {
			for(int i = 0 ; i < rc; i++ ) {
				mReaper.addNewCB(iocbs[i]);
			}
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AioRunner, "took [%lu]us to commit %d requests"), sw.span(), (int)bufs.size());
		}
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(AioRunner,"stopped"));
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//
AioEventReaper::AioEventReaper( C2StreamerEnv& env, AioClient::Ptr ptr) :
	mEnv(env),
	mClientPtr(ptr),
	mbRunning(false) {
		memset( &mIoCtx,0,sizeof(mIoCtx) );
	}

AioEventReaper::~AioEventReaper() {
	stop();
	mClientPtr = NULL;
}

bool AioEventReaper::start( io_context_t* ctx ) {
	memcpy(&mIoCtx,ctx,sizeof(mIoCtx) );
	stop( );
	mbRunning = true;
	return ZQ::common::NativeThread::start();
}

void AioEventReaper::stop( ) {
	if(!mbRunning)
		return;
	mbRunning = false;
	waitHandle(-1);
}

void AioEventReaper::addNewCB(struct iocb* cb) {
	ZQ::common::MutexGuard gd(mLocker);
	if(mCb2InfoMap.find(cb) != mCb2InfoMap.end()) {
		assert(false && "iocb already exist");
	}
	static int64 timeout = 60 * 1000;
	CBTimeInfo info;
	info.cb = cb;
	info.target = ZQ::common::now() + timeout;

	mCbInfoSet.insert(info);
	mCb2InfoMap[cb] = info;
}

void AioEventReaper::removeCB(struct iocb *cb) {
	{
		ZQ::common::MutexGuard gd(mLocker);
		CB2INFOMAP::iterator it = mCb2InfoMap.find(cb);
		if(it == mCb2InfoMap.end()) {
			return;
		}
		mCbInfoSet.erase(it->second);
		mCb2InfoMap.erase(it);
	}
	free(cb);
}

int AioEventReaper::run( ) {
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(AioEventReaper,"running"));
	struct timespec ts;
	ts.tv_sec = 1;
	ts.tv_nsec = 0;
	const size_t MAX_EVENT_COUNT = 1204;
	struct io_event events[ MAX_EVENT_COUNT ];
	while(mbRunning) {
		int rc = io_getevents( mIoCtx, 1, MAX_EVENT_COUNT, events, &ts );
		if(!mbRunning)
			break;
		if( rc < 0 ) {
			if( -rc != EINTR ) {
				MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AioEventReaper,"run() got an error[%d] in io_getevents"),rc);
			}
			continue;
		} else if( rc == 0 ) {
			continue;
		}
		for(int i = 0 ; i < rc; i ++ ) {
			removeCB(events[i].obj);
		}
		//mCc.requestComplete( events, (size_t)rc );
		mClientPtr->requestComplete(events, (size_t)rc);

		//check if any timed out iocb exist
		while(true) {
			int64 timenow = ZQ::common::now();
			struct iocb* cb = NULL;
			{
				ZQ::common::MutexGuard gd(mLocker);
				if( mCbInfoSet.empty() ) {
					break;
				}
				CBINFOSET::iterator it = mCbInfoSet.begin();
				if(it->target < timenow) {
					cb = it->cb;
					removeCB(cb);
				} else {
					cb = NULL;
					break;
				}
			}
			if(cb) {
				struct io_event evt;
				memset(&evt, 0, sizeof(evt));
				do {
					rc = io_cancel(mIoCtx, cb, &evt);
				} while(-rc == EAGAIN);
				if( rc != 0 ) {
					MLOG.error(CLOGFMT(AioEventReaper, "failed to cancel an outstanding io request due to [%s]"), strerror(-rc));
				} else {
					if(evt.data != NULL) {
						mClientPtr->requestComplete(&evt, 1, true);
					}
				}
			}
		}
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(AioEventReaper,"stopped"));
	return 0;
}

AioClient::AioClient( IReaderCallback* pCallBack, C2StreamerEnv& env)
	:_cacheCallBack(pCallBack), mEnv(env)
{
	//initClient();
}

AioClient::~AioClient()
{
	unInitClient();
	_cacheCallBack = NULL;
}

bool AioClient::read( const std::vector<BufferUser>& bufs )
{
	if (bufs.empty())
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(AioClient,"read() entry with empty bufs."));
		return true;
	}
	BufferUser buf = bufs[0];
	std::string fileName = buf->filename();
	if (fileName.empty())
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(AioClient,"read() get the fileName failed."));
		return false;
	}
	Ptr pt(this);
	AioRunner* r = getRunner( fileName );
	assert( r != 0 );
	size_t pendingReqs = 0;
	std::vector<BufferUser>::const_iterator it = bufs.begin();
	pendingReqs = r->pushBuffer( bufs);
	if(pendingReqs > 10 ) 
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AioClient,"read(), too many pending io_requests[%d]"),(int)pendingReqs);
	}
	pt = NULL;
	return true;
}

bool AioClient::initClient()
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AioClient,"initClient() entry."));
	size_t numcore = boost::thread::hardware_concurrency() * 2;
	if(numcore == 0 || numcore > 128 ) {
		numcore = 128;
	}
	Ptr p(this);
	for(size_t i = 0 ; i < numcore; i ++ ) {
		AioRunner* r = new AioRunner( mEnv, p);
		if(!r->start(numcore)) {
			r->stop();
			delete r;
			return false;
		}
		mRunners.push_back(r);
	}
	return mRunners.size() > 0 ;
}

void AioClient::unInitClient()
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AioClient,"unInitClient() entry."));
	std::vector<AioRunner*>::iterator it = mRunners.begin();
	for( ; it != mRunners.end(); it ++ ) {
		(*it)->stop();
	}
	mRunners.clear();
}

AioRunner*  AioClient::getRunner( const std::string& filename ) {
	std::size_t seed = 0;
	boost::hash_combine( seed, filename );
	seed = seed % mRunners.size();
	return mRunners[seed];
}

void AioClient::requestComplete( struct io_event* evts, size_t count, bool cancelled ) {
	if (count <= 0)
		return;
	std::vector<BufferUser> bufs;
	for(size_t i = 0 ; i < count; i ++ ) 
	{
		Buffer* buf = reinterpret_cast<Buffer*>(evts[i].data);
		long result = (long)evts[i].res;
		if( result<0) {
			buf->setLastError( (int)result, Buffer::ECATE_FILEIO);
		}
		if(cancelled) {
			buf->setLastError(ETIME, Buffer::ECATE_TIMEOUT);
		} else{
			buf->setDataSize( result < 0 ? 0 : result );
		}
		bufs.push_back( BufferUser(buf));
		buf->put(); // 在AioRunner::run里面添加了一次引用，所以需要在这里去掉
	} 
	_cacheCallBack->onRead(bufs);
	return;
}

void AioClient::requestComplete( const std::vector<BufferUser>& bufs ) {
	_cacheCallBack->onRead(bufs);
	return;
}

bool AssetAttribute::expired( )const {
	if( inprogress() )
		return false;
	if( mOpenForWrite || ( mLastError != 0 ) ) {
		if( mTimestamp > 0 &&(ZQ::common::now() - mTimestamp) > (int64)mEnv.getConfig().cacheCenter.assetAttributesTimeoutInPwe ) 
			return true;
	}
	return false;
}

bool AssetAttribute::asyncWait( IAsyncNotifySinker::Ptr sinker ) {
	ZQ::common::MutexGuard gd(mLocker);
	if(mbSignalled) {
		return false;
	}
	mSinkers.push_back(sinker);
	return true;
}

bool AssetAttribute::inprogress() const {
	ZQ::common::MutexGuard gd(mLocker);
	return !mbSignalled;
}

void AssetAttribute::signal() {
	NOTIFYSINKERS sinkers;
	{
		ZQ::common::MutexGuard gd(mLocker);
		mbSignalled = true;
		mTimestamp = ZQ::common::now();
		sinkers.swap(mSinkers);
	}
	NOTIFYSINKERS::iterator it = sinkers.begin();
	for( ; it != sinkers.end(); it ++ ) {
		(*it)->onNotified();
	}
}

const char* DiskCacheSink::errorToA(Error err)
{
#define ERRSTR_CASE(ERR) case cacheErr_##ERR: return #ERR
	switch(err)
	{
		ERRSTR_CASE(OK);
		ERRSTR_CASE(Hit);
		ERRSTR_CASE(Missed);
		ERRSTR_CASE(StorePending);
		ERRSTR_CASE(StoreFail);
		ERRSTR_CASE(SwapMissed);
		ERRSTR_CASE(Timeout);
	}
#undef ERRSTR_CASE

	return "Unknown";
}

HybridReader::HybridReader( CacheCenter& cc)
:mEnv(cc.getEnv()),
mCc(cc){
}

HybridReader::~HybridReader() {
}

bool HybridReader::pushReaderType( int typ ) {
	bool bFound = false;
	size_t idx = 0;
	for( ; idx < mReaders.size(); idx ++ ) {
		if( mReaders[idx].current == typ ) {
			bFound = true;
			break;
		}
	}
	if(bFound) {
		MLOG.error(CLOGFMT(HybridReader, "reader [%d] has already been registered"), typ);
		return false;
	}
	if(idx>0) {
		mReaders[idx-1].next = typ;
	}
	ReaderSlot slot;
	slot.current = typ;
	mReaders.push_back(slot);
	MLOG.info(CLOGFMT(HybridReader, "reader [%d] registed"), typ);
	return true;
}

int HybridReader::findNextReader(int reader) const {
	std::vector<ReaderSlot>::const_iterator it = mReaders.begin();
	for( ; it != mReaders.end(); it ++ ) {
		if( it->current == reader ) {
			return it->next;
		}
	}
	return -1;
}

bool HybridReader::read( const std::vector<BufferUser>& bufs ) {
	MLOG.error(CLOGFMT(HybridReader, "read is not implemented"));
	return false;
}

void HybridReader::onRead( const std::vector<BufferUser>& bufs, bool fromDiskCache ) {
	MLOG.error(CLOGFMT(HybridReader, "onRead should not be invoked"));
}

void HybridReader::onLatency(std::string& fileName, int64 offset,int64 time) {

}

void HybridReader::onError( int err ) {

}

bool HybridReader::queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) {
	assert(mReaders.size() > 0);
	int readerType = mReaders[0].current;
	attr->currentReaderType(readerType);
	attr->setReaderCallback(this);
	MLOG.info(CLOGFMT(HybridReader, "queryIndexInfo() file[%s], using reader[%d]"), filename.c_str(), readerType);
	mCc.queryAssetAttributes(filename, attr, readerType);
	return true;
}

void HybridReader::onIndexInfo( AssetAttribute::Ptr attr ) {
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(HybridReader,"onIndexInfo() got asset attribute for [%s]: pwe[%s] lastError[%d]"),
			attr->filename().c_str(), attr->pwe()?"true":"false", attr->lastError());
	int lastError = attr->lastError();
	if(lastError == AssetAttribute::ASSET_NOTFOUND ) {
		int readerType = findNextReader(attr->currentReaderType());
		if( readerType >= 0 ) {
			attr->lastError(AssetAttribute::ASSET_SUCC);
			attr->currentReaderType(readerType);
			MLOG.info(CLOGFMT(HybridReader, "onIndexInfo() file[%s], using reader[%d]"), attr->filename().c_str(), readerType);
			mCc.queryAssetAttributes(attr->filename(), attr, readerType);
			return;
		}
	} else if(lastError == AssetAttribute::ASSET_SUCC) {
		MLOG.info(CLOGFMT(HybridReader, "onIndexInfo() file[%s], sguggest reader [%d]"), attr->filename().c_str(), attr->currentReaderType() );
		attr->suggestReaderType(attr->currentReaderType());
	}
	attr->setReaderCallback(NULL);
	mCc.onIndexInfo(attr);
}

}//namespace C2Streamer

