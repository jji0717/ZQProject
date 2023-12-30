#include <math.h>
#include "cachelayer.h"
/**
 * 关于去掉big lock的备忘
 * 1. readcache和writebuffer分别拥有自己的locker,去掉datatank中的locker
 * 2. readstatistics拥有自己的locker,不使用readcache的locker为readstatitics的数据提供保护
 * 3. 由于writebuffer会调用readcache,所以需要非常小心避免死锁. 那么readcache和writebuffer在需要调用
 * 		对方的方法的时候需要首先释放掉自己拿到的locker
 *
 * */
namespace CacheLayer {
#define MLOG (mTank.getLogger())

Sema::Sema(size_t initCount /* = 0 */) {
#ifdef ZQ_OS_MSWIN
	mSema = CreateSemaphore(0,(LONG)initCount,0x7fffffff,0);
#elif defined ZQ_OS_LINUX
	sem_init(&mSema,0,0);
#endif
}

Sema::~Sema() {
#ifdef ZQ_OS_MSWIN
	CloseHandle(mSema);
#elif defined ZQ_OS_LINUX
	sem_destroy(&mSema);	
#endif
}

void Sema::post() {
#ifdef ZQ_OS_MSWIN
	ReleaseSemaphore(mSema,1,0);
#elif defined ZQ_OS_LINUX
	int rc = sem_post(&mSema);
	assert( rc == 0);
#endif
}

bool Sema::timedWait( int64 interval ) {
#ifdef ZQ_OS_MSWIN
	DWORD dwTimeOut = static_cast<DWORD>(interval);
	DWORD ret = WaitForSingleObject(mSema,dwTimeOut);
	switch( ret ) {
case WAIT_OBJECT_0:
	return true;
default:
	return false;
	}

#elif defined ZQ_OS_LINUX
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	long long nsec = ts.tv_nsec + interval*1000000LL;
	ts.tv_sec += nsec/1000000000L;
	ts.tv_nsec = nsec%1000000000L;

	int ret = 0 ;
	while( (ret = sem_timedwait(&mSema, &ts)) == -1 && errno == EINTR )
		;
	return ret == 0 ;
#endif
}

void Sema::wait() {	
#ifdef ZQ_OS_MSWIN
	WaitForSingleObject(mSema,INFINITE);
#elif defined ZQ_OS_LINUX
	while( (sem_wait(&mSema) == -1) && (errno == EINTR) )
		;
#endif
}

//////////////////////////////////////////////////////////////////////////
///IMPL: Notifier
Notifier::Notifier():mbSignalled(false){
}
Notifier::~Notifier(){
}

void Notifier::broadcast() {
	boost::mutex::scoped_lock lock(mLocker);
	mbSignalled = true;
	mCond.notify_all();
}
void Notifier::reset( ) {
	boost::mutex::scoped_lock lock(mLocker);
	mbSignalled = false;
}

void Notifier::wait( ) {
	 boost::mutex::scoped_lock lock(mLocker);
	 if(mbSignalled)
		 return;
	 mCond.wait(lock);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///MBMetaData
std::string MBMetaData::toString() const {
	std::ostringstream oss;
	oss<<"file["<<filePath<<"] range["<<dataRange.toString()<<"]";
	return oss.str();
}

//////////////////////////////////////////////////////////////////////////
///MemoryBlockInfo
MemoryBlockInfo::MemoryBlockInfo( DataTank& tank, size_t id ) 
:mTank(tank),
mId(id),
mBuffer(0),
mBufSize(0),
mStartPos(0),
mDataSize(0),
mErrorCode(0),
mLastTouch(0),
mNotifier(0),
mUserCount(0),
mbInReadCache(0),
mbInCacheRunner(0),
mbInExpireChecker(0),
mbInFreeList(0),
mbInWriteBuffer(0),
mbInFlushRunner(0),
mbInReading(0),
mbInWriting(0),
mMetaNext(this),
mMetaPrev(this) {

}

MemoryBlockInfo::~MemoryBlockInfo() {
	destroy();
}

void MemoryBlockInfo::addToMetaList( MemoryBlockInfo* head ) {
	assert( head != 0 );
	assert( !inMetaList() );
	MemoryBlockInfo* prevNode = head->mMetaPrev;
	prevNode->mMetaNext = this;
	this->mMetaPrev = prevNode;
	this->mMetaNext = head;
	head->mMetaPrev = this;
}

void MemoryBlockInfo::removeFromMetaList( )  {
	assert( inMetaList() );
	mMetaPrev->mMetaNext = mMetaNext;
	mMetaNext->mMetaPrev = mMetaPrev;
	mMetaPrev = mMetaNext = this;
}

bool MemoryBlockInfo::create( size_t size ) {
	mBufSize = (unsigned int)size;
	mBuffer = new (std::nothrow) char[size];
	return mBuffer != 0 ;
}

void MemoryBlockInfo::destroy() {
	if(mBuffer && mBufSize > 0 ) {
		delete[] mBuffer;		
	}
	mBuffer = 0;
	mBufSize = 0;	
	mDataSize = 0;
}

void MemoryBlockInfo::reset( ) {
	mDataSize = 0;
	mStartPos = 0;
	mErrorCode= 0;
	mLastTouch = 0;
	mNotifier = 0;
	mUserCount = 0;
	mbNeedValidate = false;
	//if(mBuffer)
	//	mBuffer[0] = 0;
}

std::string MemoryBlockInfo::toString( ) const {
	std::ostringstream oss;
	oss<<"file["<<mMD.filePath<<"] dataRange["<<dataPosOfFile()<<"/"<<mDataSize<<"]";
	return oss.str();
}

void MemoryBlockInfo::status( ) const {
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///ExpirationChecker
ExpirationChecker::ExpirationChecker()
:mbRunning(false),
mExpiration(10 * 1000 ) {
}

ExpirationChecker::~ExpirationChecker( ) {
	stop();
}

bool ExpirationChecker::start(size_t expiration, size_t maxBufCount ) {
	mExpiration = expiration;
	mMaxBufCount = maxBufCount;
	mbRunning = true;
	return ZQ::common::NativeThread::start();
}
void ExpirationChecker::stop() {
	if(!mbRunning)
		return;
	mbRunning = false;
	mSema.post();
	waitHandle(-1);
}

void ExpirationChecker::touch( MemoryBlockInfo* mbi ) {
	ZQ::common::MutexGuard gd(mMutex);
	if(!mbi || !mbi->inExpireChecker() )
		return;
	removeFromExpireChecker(mbi);
	addToExpireChecker(mbi);
	mbi->touch();
}

void ExpirationChecker::removeFromExpireChecker( MemoryBlockInfo* mbi ) {
	ZQ::common::MutexGuard gd(mMutex);
	if( !mbi || !mbi->inExpireChecker() )
		return;
	mbi->removeFromExpireChecker();
	mList.erase(mbi->iterList());
}

MBLIST::iterator ExpirationChecker::addToExpireChecker( MemoryBlockInfo* mbi ) {
	ZQ::common::MutexGuard gd(mMutex);
	if( !mbi || mbi->inExpireChecker() )
		assert(false);
	mbi->addToExpireChecker();
	mList.push_front(mbi);
	mbi->iterList( mList.begin() );
	return mbi->iterList();
}

int ExpirationChecker::run( ) {
	while( mbRunning ) {
		mSema.timedWait(500);
		if(!mbRunning)
			break;
		long long timenow  = ZQ::common::now();
		while( true ) {
			MemoryBlockInfo*mbi = 0;
			{
				ZQ::common::MutexGuard gd(mMutex);
				if(mList.empty())
					break;
				mbi = mList.back();
				if(!mbi)
					break;
				if(!( ( mbi->lastTouch() > 0 )&&
							(timenow > mbi->lastTouch() ) && 
							( timenow - mbi->lastTouch()) >= (long long)mExpiration ) ) {
					break;
				}
				mList.pop_back();
			}
			mbi->removeFromExpireChecker();
			onExpired(mbi);
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///MBUser
MBUser::MBUser( DataTank& tank, MemoryBlockInfo* mbi )
:mTank(tank),
mMbi(mbi){	
}

MBUser::MBUser( const MBUser& b )
:mTank(b.mTank),
mMbi(b.mMbi){
}

MBUser& MBUser::operator=( const MBUser& b) {	
	mMbi = b.mMbi;
	return *this;
}

MBUser::~MBUser() {
}

bool MBUser::isValid() const {
	return mMbi != 0;
}

int	MBUser::lastError( ) const {
	if(!mMbi)
		return -99999;
	return mMbi->errorCode();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///MBReader
MBReader::MBReader( DataTank& tank, MemoryBlockInfo* mbi ) 
:MBUser(tank,mbi) {
	if(mbi)
		mNotifier = mbi->notifier();
}

MBReader::MBReader( const MBReader& b )
:MBUser(b){
	mNotifier = b.mNotifier;
}

MBReader& MBReader::operator=( const MBReader& b) {
	mMbi = b.mMbi;
	mNotifier = b.mNotifier;
	return *this;
}

MBReader::~MBReader() {	
}

int MBReader::read( size_t offset, char* buf, size_t& size ) {
	if(!isValid())
		return -1;

	if( mNotifier ) {
		mNotifier->wait();		
		mNotifier = 0;
	}

	if( !mTank.isSuccess( mMbi->errorCode() ) ) {
		size = 0;
		return 0;
	}

	if( offset < mMbi->dataStartPos() )
	{
		size = 0;
		return 0;
	}
	if( offset >=  mMbi->dataEndPos() )
	{
		size = 0;
		return 0;
	}

	size_t copied = MIN( size, (mMbi->dataSize() - (offset - mMbi->dataStartPos()) ) );
	memcpy( buf, mMbi->buffer() + offset, copied );
	size = copied;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///MBWriter
MBWriter::MBWriter( DataTank& tank, MemoryBlockInfo* mbi ) 
:MBUser(tank,mbi){
}

MBWriter::MBWriter( const MBWriter& b ) 
:MBUser(b){
}

MBWriter& MBWriter::operator=( const MBWriter& b) {
	mMbi = b.mMbi;
	return *this;
}

MBWriter::~MBWriter(){	
}

bool intersectOrAdjacent( uint64 b1, uint64 s1, uint64 b2 , uint64 s2 )  {
	uint64 e1 = b1 + s1 ;//s1 > 0 
	uint64 e2 = b2 + s2 ;//s2 > 0

	if( b1 > b2 ) {
		uint64 tmp;
		tmp = b1 ; b1 = b2; b2 = tmp;
		tmp = e1 ; e1 = e2; e2 = tmp;
	}
	if( e1 < b2 )
		return false;
	return true;
}

bool intersectOrAdjacent( const Range& r1, const Range& r2 ) {
	return intersectOrAdjacent( r1.begin, r1.size, r2.begin, r2.size );
}

int MBWriter::write( size_t offset, const char* buf, size_t& size, unsigned long long originalOffset, bool bDirectWrite ) 
{
	if(!isValid()) {
		assert(false);
		return -1;
	}

	if( offset >= mMbi->bufSize()) {
		assert(false);
		return -1;
	}

	bool bOK = true;
	ZQ::common::MutexGuard gd(mMbi->writerLocker() );
	assert(!mMbi->inCacheRunner());

	if( mMbi->dataSize() > 0 && size > 0 )
		bOK = intersectOrAdjacent(mMbi->dataStartPos(), mMbi->dataSize(), offset, size );

	int rc  = -1;
	if (bDirectWrite || !bOK)
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(MBWriter,"write(), file[%s] direct-io[%c] or random write detected per in-cache[%lld +%d] and req[%lld +%d], taking directWrite()"), 
			mMbi->toString().c_str(), bDirectWrite?'T':'F', (int64)mMbi->dataStartPos(), (int)mMbi->dataSize(), (int64)offset, (int)size);

		long long startPos = mMbi->dataPosOfFile();
		size_t tmpSize = mMbi->dataSize();
		MBMetaData metadata = mMbi->md();
		
		rc = mTank.directWrite( metadata.filePath, mMbi->data(), startPos, tmpSize);

		//reset current buffer to empty, so that it can hold new data
		mMbi->dataStartPos(0);
		mMbi->dataSize(0);
		if(mTank.isSuccess( rc, 0))
			rc = 0;
		else
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(MBWriter,"write(), failed to write data directly to server: %d"),rc);
			rc = -1;
		}
	
		mTank.validate_readcache( metadata.filePath, originalOffset,size);
		mTank.validate_readcache( metadata.filePath, startPos, tmpSize );
	}

	if( size > 0 ) 	{
		size_t copied = MIN( size, mMbi->bufSize()-offset );
		size_t tail = mMbi->dataEndPos();
		memcpy( mMbi->buffer()+offset,buf,copied);
		tail = tail > (offset + copied) ? tail : (offset + copied);

		if( mMbi->dataSize() <= 0)
			mMbi->dataStartPos( MAX( mMbi->dataStartPos() , offset ) );
		else
			mMbi->dataStartPos( MIN( mMbi->dataStartPos(), offset ) );

		mMbi->dataSize( tail - mMbi->dataStartPos() );
		mMbi->touch();
		size = copied;
		rc = 0;
	} 
	return rc;
}


//////////////////////////////////////////////////////////////////////////
///File2Metadata
File2Metadata::File2Metadata( DataTank& tank ):mTank(tank) {

}

File2Metadata::~File2Metadata() {

}

void File2Metadata::reserve( size_t ) {
}

void File2Metadata::addMetaData( MemoryBlockInfo* mbi ) {
	const std::string& filePath = mbi->md().filePath;
	FILE2METADATAMAP::iterator it = mMetadataMap.find(filePath);
	if( it == mMetadataMap.end() ) {
		MemoryBlockInfo* head = new MemoryBlockInfo( mTank, 0 ); //dummy node
		mbi->addToMetaList( head );
		mMetadataMap[ filePath] = head;
	} else {
		MemoryBlockInfo* head = it->second;
		mbi->addToMetaList( head );
	}
}

void File2Metadata::removeMetadata( MemoryBlockInfo* mbi) {
	FILE2METADATAMAP::iterator it = mMetadataMap.find(mbi->md().filePath);
	if( it == mMetadataMap.end())
		return ;
	mbi->removeFromMetaList();
	MemoryBlockInfo* head = it->second;
	if( !head->inMetaList() ) {
		delete head;
		mMetadataMap.erase(it);
	}
}

void File2Metadata::removeMetadata( const std::string& filepath ) {
	//mMetadataMap.erase(filepath);
	FILE2METADATAMAP::iterator it = mMetadataMap.find(filepath);
	if( it == mMetadataMap.end())
		return;
	MemoryBlockInfo* head = it->second;
	while( head->inMetaList() ) {
		MemoryBlockInfo* p = head->next();
		p->removeFromMetaList();
	}
	delete head;
	mMetadataMap.erase(it);
}

MemoryBlockInfo* File2Metadata::getMetadata( const std::string& filepath ) {
	FILE2METADATAMAP::iterator it = mMetadataMap.find(filepath);
	if( it == mMetadataMap.end())
		return 0;
	return it->second;
}

size_t File2Metadata::count( const std::string& filePath ) const {
	return 0;
}

std::vector<std::string> File2Metadata::getFileList( ) const {
	std::vector<std::string> files;
	files.reserve( mMetadataMap.size() );
	FILE2METADATAMAP::const_iterator it = mMetadataMap.begin();
	for( ; it != mMetadataMap.end() ; it ++ ) {
		files.push_back(it->first);
	}
	return files;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///FreeList
FreeList::FreeList( DataTank& tank ) 
:mTank(tank),
mCount(0){
}

FreeList::~FreeList() {
	destroy();
}

size_t FreeList::count( ) const {
	return mCount;
}

/**
 * a MB can not be add into FreeList if it is in CacheRunner WriteBuffer FlushRunner Read Write
 */
void FreeList::add( MemoryBlockInfo* mbi ) {
	assert( !mbi->inCacheRunner() );
	assert( !mbi->inWriteBuffer() );
	assert( !mbi->inFlushRunner() );
	assert( !mbi->isBeingRead() );
	assert( !mbi->isBeingWrite() );
	
	if(mbi->inFreeList( ) ) {
		assert(false);
		return ;
	}

	mbi->addToFreeList( );
	mFreeList.push_front(mbi);
	LISTITER it = mFreeList.begin();
	mbi->iterFree( it );	
	++mCount;
}

void FreeList::remove( MemoryBlockInfo* mbi ) {
	if( !mbi->inFreeList( ) ) {
		assert(false);
		return;
	}	
	mbi->removeFromFreeList( );
	assert( mbi->iterFree() != mFreeList.end() );
	mFreeList.erase( mbi->iterFree() );
	mbi->iterFree( mFreeList.end() );
	--mCount;
}

MemoryBlockInfo* FreeList::fetch( ){
	if( mFreeList.empty() ) {		
		return 0;	
	}
	assert( !mFreeList.empty() );
	MemoryBlockInfo* mbi = mFreeList.back();
	remove(mbi);

	if( mbi->inReadCache()) {
		mTank.getReadCache().remove( mbi );
	}
	mbi->reset();

	return mbi;
}

bool FreeList::fetch( size_t count, std::vector<MemoryBlockInfo*>& mbis  ) {
	if( mCount < count )
		return false;
	while( count -- > 0 ) {
		MemoryBlockInfo* mbi = fetch();
		assert( mbi != 0 );
		mbis.push_back( mbi );
	}
	return true;
}

bool FreeList::create( size_t count, size_t bufSize ) {
	for( size_t i = 0 ; i < count ; i ++ ) {
		MemoryBlockInfo* mbi =  new (std::nothrow) MemoryBlockInfo(mTank,i);
		if(!mbi)	return false;
		if(!mbi->create(bufSize)) {
			delete mbi;
			return false;
		}		
		add(mbi);
	}
	return true;
}

void FreeList::destroy() {
	while( !mFreeList.empty() ) {
		remove( mFreeList.back() );
	}
}

//////////////////////////////////////////////////////////////////////////
///CacheReadRunner
CacheReadRunner::CacheReadRunner( DataTank& tank, std::vector<MemoryBlockInfo*> mbis) 
: ZQ::common::ThreadRequest(tank.getReadThreadPool()),
mTank(tank),
mMbis(mbis){
	mStopWatch.start();
	static size_t threadCount = tank.getReadThreadPool().size();
	size_t pendingSize = tank.getReadThreadPool().pendingRequestSize();
	size_t runningCount = tank.getReadThreadPool().activeCount();
	if( pendingSize > threadCount/2 ) {
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(CacheReadRunner,"read thread pool: size[%zu], active[%zu], pending[%zu]"),
				threadCount, runningCount, pendingSize);
	}
}

CacheReadRunner::~CacheReadRunner(){
}

size_t CacheReadRunner::getTotalBufSize( std::vector<DataBuffer>& bufs) const {
	size_t total = 0 ;
	std::vector<MemoryBlockInfo*>::const_iterator it = mMbis.begin();
	while( it != mMbis.end() )
	{
		MemoryBlockInfo* mbi = *it;
		assert( ! mbi->inFlushRunner( ) );
		assert( ! mbi->inWriteBuffer( ) );
		assert( mbi->inCacheRunner() );
		total += mbi->bufSize();
		DataBuffer buf;
		buf.buf = mbi->buffer();
		buf.size = mbi->bufSize();
		bufs.push_back(buf);
		it++;
	}
	return total;
}


void CacheReadRunner::storeResult( size_t size , int err) {
	std::vector<MemoryBlockInfo*>::iterator it = mMbis.begin();
	while( it != mMbis.end() && size != 0) {
		MemoryBlockInfo* mbi = *it;
		mbi->errorCode(err);
		if( size > mbi->bufSize() ) {
			mbi->dataSize(mbi->bufSize());
			size -= mbi->bufSize();
		} else {
			mbi->dataSize(size);
			size = 0;
		}
		it++;
	}
}

void CacheReadRunner::completeRead( ) {
	mTank.onCacheComplete( mMbis);
}

int CacheReadRunner::run( ) {

	if( mMbis.size() <= 0 )
		return 0;
	mStopWatch.stop();
	StopWatch s1,s2(s1);
	std::vector<DataBuffer> bufs;
	size_t tmpSize = getTotalBufSize(bufs);
	
	const MBMetaData& md = mMbis[0]->md();
	

	int rc = mTank.directRead(md.filePath, 								
								md.dataRange.begin,
								bufs,
								tmpSize);
	s1.stop();

	mTank.isSuccess( rc, &tmpSize);	
	storeResult( tmpSize, rc);
	completeRead();
	s2.stop();

	if( tmpSize <=32 && tmpSize > 0 ) {
		std::ostringstream ossHint;
		ossHint<<"CacheReadRunner["<< md.filePath <<"] ";
		std::string strHint = ossHint.str();
		const char* recvbuff = bufs[0].buf;
		MLOG.hexDump(ZQ::common::Log::L_DEBUG, recvbuff, tmpSize, strHint.c_str());
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(CacheReadRunner, "has read file[%s] range[%llu+%u] rc[%d], time[%lld/%lld/%lld]us"),
		md.filePath.c_str(), md.dataRange.begin, (unsigned int)tmpSize, rc,
		mStopWatch.cost(), s1.cost(), s2-s1 );
	

	return 0;
}

//////////////////////////////////////////////////////////////////////////
///
FlushRunnerRequest::FlushRunnerRequest( FlushRunnerCenter& center, DataTank& tank, 
		const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& flushIndices)
:ZQ::common::ThreadRequest(tank.getFlushThreadPool()),
mFlushCenter(center),
mTank(tank),
mMbis(mbis),
mFlushIndices(flushIndices){	
}

FlushRunnerRequest::~FlushRunnerRequest() {
}

void FlushRunnerRequest::final(int retcode /* =0 */, bool bCancelled /* =false */){
	delete this;
}

int FlushRunnerRequest::run() {
	if( mMbis.size() == 0 )
		return 0;
	StopWatch s1,s2(s1);
	size_t tmpSize = 0 ;
	std::vector<DataBuffer> datas;
	std::vector<MemoryBlockInfo*>::iterator it = mMbis.begin();
	for( ; it != mMbis.end(); it ++ ) {
		MemoryBlockInfo* mbi = *it;
		assert( !mbi->isBeingWrite() );
		assert( !mbi->inFreeList() );
		assert( !mbi->inWriteBuffer() );
		assert( mbi->inFlushRunner() );
		DataBuffer buf;
		buf.offset = mbi->dataPosOfFile();
		buf.buf  = mbi->data();
		buf.size = mbi->dataSize();
		tmpSize += buf.size;
		datas.push_back(buf);
	}
	//std::sort(datas.begin(), datas.end());//already sorted in FlushRunnerCenter
	int rc = mTank.directWrite(mMbis[0]->md().filePath, datas, tmpSize);
	it = mMbis.begin();
	for( ; it != mMbis.end(); it ++ ) {
		(*it)->errorCode(rc);
	}
	
	s1.stop();
	int64 bitrate = 0xFFFFFFFF;
	if( s1.cost() >0 ) {
		bitrate = tmpSize * 8 * 1000 * 1000 /s1.cost();
	}
	
	mFlushCenter.onRunnerComplete( mMbis, mFlushIndices, bitrate );
	
	s2.stop();

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(FlushBackRequest,"flush data for file[%s] range[%s] count[%d] total[%lld] rc[%d], time[%lld/%lld]us"),
		mMbis[0]->md().filePath.c_str(), dataBuffersToRangeStr(datas).c_str(), (int)datas.size(), (long long)tmpSize, rc, 
		s1.cost(), s2.cost() );
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///ReadCache
ReadCache::ReadCache( DataTank& tank )
:File2Metadata(tank),
mTank(tank),
#ifdef _USE_HASHMAP_
mMbMap(tank.getConf().readCacheBlockSize*2),
#endif// _USE_HASHMAP_

mNAMemoryBlockCount(0),
mReadStats(tank) {
	reserve(mTank.getConf().readBlockCount);
}

ReadCache::~ReadCache( ) {
}

void ReadCache::putMemory( const MBReader& reader ) {
	putMemory(reader.mbi());
}

void ReadCache::putMemory( const std::vector<MBReader>& readers ) {
	std::vector<MBReader>::const_iterator it = readers.begin();
	while( it != readers.end() ) {
		if( it->isValid() )  {
			putMemory( it->mbi() );
		}
		it++;
	}
}

void ReadCache::putMemory( MemoryBlockInfo* mbi ) {
	
	assert( mbi != 0 );
	assert( !mbi->inFreeList() );
	assert( !mbi->inWriteBuffer() );
	assert( !mbi->inFlushRunner() );
	assert( !mbi->isBeingWrite() );

	assert( mbi->isBeingRead() );
	//assert( mbi->inReadCache() );
	//assert( mbi->inCacheRunner() );
	ZQ::common::MutexGuard gd(mLocker);	
	long user = mbi->removeUser();
	mReadingCount--;
	if( user <= 0 ) {
		mbi->removeFromRead();
		if(!mbi->inCacheRunner()) {
			onReadComplete( mbi );
		}
	}
}


std::vector<MBReader> ReadCache::getMemory( const MBMetaData& metadata ) {
	static size_t readBlockSize = mTank.getConf().readCacheBlockSize;
	StopWatch s1,s2(s1),s3(s1),s4(s1),s5(s1);

	size_t blockCount = metadata.dataRange.size / readBlockSize;
	assert( blockCount > 0 );
	assert( metadata.isValid() && blockCount * readBlockSize == metadata.dataRange.size );

	std::vector<MBReader> readers;
	readers.reserve( blockCount );

	std::vector< MBMetaData> mds;
	mds.reserve( blockCount );


	size_t readAheadCount = mReadStats.readAt( metadata.filePath, metadata.dataRange );
	s1.stop();
	bool bOK = true;

	s2.start();
	{

		ZQ::common::MutexGuard gd(mLocker);
		s5.start();
		for( size_t p = 0; p < metadata.dataRange.size ; p+= readBlockSize ) {
			MBMetaData md = metadata;
			md.dataRange.size = readBlockSize;
			md.dataRange.begin += p;

			MBMAP::const_iterator it = mMbMap.find( md );
			if( bOK && it != mMbMap.end() ) { //got it
				MemoryBlockInfo* mbi = it->second;
				assert( mbi != 0 );
				assert( !mbi->inFlushRunner());
				assert( !mbi->inWriteBuffer() );
				if( mbi->inFreeList( ) )
					mTank.getReadFreeList().remove(mbi);
				mbi->addToRead();
				mbi->addUser();
				readers.push_back( MBReader(mTank, mbi ) );
				mReadingCount ++;
			} else {
				bOK = false;
				mds.push_back( md );
			}
		}
		s5.stop();
		
		s3.start();
		if( mds.size() > 0 && !fetch( mds, readers ) ) {
			putMemory( readers );
			readers.clear();
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadCache,"getMemory() not enough free memory block, fecth[%d] usage[%s]"),
					(int)mds.size(), mbUsage().c_str() );
			return readers;
		}
		s3.stop();

	}
	s2.stop();

	
	//adjust metadata's offset to last read block so that readahead can perform
	MBMetaData md = metadata;
	md.dataRange.begin += md.dataRange.size - readBlockSize;
	md.dataRange.size = readBlockSize;

	s4.start();
	size_t actualReadAhead = readAhead( md, readAheadCount );
	s4.stop();

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ReadCache,"getMemory() %s, hit[%d] miss[%d] readAhead[%d/%d] time[%lld/%lld/%lld/%lld/%lld]us"),
			metadata.toString().c_str(), 
			(int)(blockCount - mds.size()), (int)(mds.size()),
			(int)readAheadCount,(int)actualReadAhead, 
			s1.cost(),s2.cost(), s3.cost(), s4.cost(), s5.cost() );
	return readers;
}

/**
 * 不要单独findCachedData
 * 而是在查找数据是否cache的同时准备好需要被cache的数据的metadata
 * */

size_t ReadCache::readAhead( const MBMetaData& metadata, size_t wantCacheCount ) {
	
	static size_t mbsize = mTank.getConf().readCacheBlockSize;
	static size_t maxCacheCount = mTank.getConf().readAheadCount;

	wantCacheCount = MIN( wantCacheCount, maxCacheCount );
	if( wantCacheCount == 0 )
		return 0;
	
	std::vector<MemoryBlockInfo*> mbis;
	mbis.reserve( wantCacheCount ); //it's ok to reserve more than need
	

	StopWatch s1,s2(s1),s3(s1),s4(s1);

	size_t preCachedCount = 0; // this is count from the start buffer of readahead zone
	MBMetaData nextBlock = metadata;
	std::vector<CacheReadRunner*> runners;

	size_t totalCachedCount = 0;
	unsigned long long offsetOfFile = metadata.dataRange.begin + mbsize;

	bool sawHole = false;
	{
		ZQ::common::MutexGuard gd(mLocker);
		s3.start();
		for( size_t i = 0 ; i < wantCacheCount; i ++ ) {
			nextBlock.dataRange.begin += mbsize;
			if( mMbMap.find(nextBlock) != mMbMap.end() ) {
				if(!sawHole) {
					++preCachedCount;
					if( preCachedCount >= wantCacheCount )
						return 0;
					if( preCachedCount >= maxCacheCount / 2 && wantCacheCount == maxCacheCount)
						return 0;
				}

				if( !mbis.empty() ) {
					runners.push_back( new CacheReadRunner(mTank, mbis));
					mbis.clear();
				}

				continue;
			} else {
				sawHole = true;
			}
			MemoryBlockInfo* mbi = getMb(nextBlock);
			if( !mbi ) {
				break;
			}
			s4.start();
			addMetaData( mbi );
			totalCachedCount ++;
			mNAMemoryBlockCount ++;
			mbis.push_back(mbi);
			s4.stop();
		}
		s3.stop();
	}

	s2.stop();

	
	if(!mbis.empty() ) {
		runners.push_back(  new CacheReadRunner(mTank,mbis) );
		mbis.clear();
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ReadCache,"readAhead file[%s] , suggest[%d]blocks from[%llu], issue [%d] blocks readahead, runners[%d], cost[%lld/%lld/%lld/%lld]"),
			metadata.filePath.c_str(),
			(int)wantCacheCount, offsetOfFile, (int)totalCachedCount, (int)runners.size(), 
			s1.cost(), s2.cost(), s3.cost(), s4.cost());
	

	std::vector<CacheReadRunner*>::iterator itRunner = runners.begin();
	for( ; itRunner != runners.end() ;itRunner++ ) {
		(*itRunner)->start();
	}

	return totalCachedCount ;
}

MemoryBlockInfo* ReadCache::fetch( const MBMetaData& metadata) {
	MemoryBlockInfo* mbi = 0;
	std::vector<MemoryBlockInfo*> mbis;
	mbis.reserve(1);
	{
		ZQ::common::MutexGuard gd(mLocker);
		mbi = getMb(metadata);
		if(!mbi)
			return 0;
		mNAMemoryBlockCount ++;
		addMetaData( mbi );
	}
	mbis.push_back(mbi);
	CacheReadRunner* r = new CacheReadRunner(mTank,mbis);
	r->start();
	return mbi;
}

bool ReadCache::fetch( const std::vector<MBMetaData>& mds, std::vector<MBReader>& readers ) {
	std::vector<MemoryBlockInfo*> mbis;
	mbis.reserve( mds.size() );
	{
		ZQ::common::MutexGuard gd(mLocker);
		if(!mTank.getReadFreeList().fetch( mds.size(), mbis ) ) {
			return false;
		}
		for(  size_t pos = 0 ; pos < mbis.size() ; pos ++ ) {
			if( mMbMap.find(mds[pos]) != mMbMap.end() ) {
				validate( mds[pos] ); //only uncached data will be fetched from here, so do not validate it
			}
			MemoryBlockInfo* mbi = mbis[pos];
			assert( !mbi->inFreeList() );
			assert( !mbi->inReadCache() );
			assert( !mbi->inWriteBuffer() );
			assert( !mbi->inCacheRunner() );
			assert( !mbi->inFlushRunner() );
			assert( !mbi->inExpireChecker() );
			mbi->notifier( new Notifier() );
			mbi->md( mds[pos] );
			mMbMap[ mbi->md() ] = mbi;
			mbi->addToReadCache();
			mbi->addToCacheRunner( );
			mbi->addToRead();
			mbi->addUser( );
			addMetaData( mbi);
			mNAMemoryBlockCount ++;
			readers.push_back( MBReader( mTank, mbi ));
			mReadingCount++;
		}
	}
	CacheReadRunner* r = new CacheReadRunner( mTank , mbis );

	r->start();
	return true;
}

bool ReadCache::cacheExistFor( const std::string& filepath ) const {
	ZQ::common::MutexGuard gd(mLocker);
	return const_cast<ReadCache*>(this)->getMetadata( filepath ) != NULL ;
}

void ReadCache::validate( const std::string& filepath ) {
	ZQ::common::MutexGuard gd(mLocker);
	MemoryBlockInfo* head = getMetadata(filepath);
	if(!head)
		return;
	bool bOK = head->next() != head;
	while( bOK) {
		MemoryBlockInfo* p = head->next();
		bOK = p->next() != head;
		validate(p->md());
	}
}

std::string ReadCache::mbUsage( ) const {
	std::ostringstream oss; 
	{
		ZQ::common::MutexGuard gd(mLocker);
		oss<<"cached["<<mMbMap.size()<<"] caching["<<mNAMemoryBlockCount<<"] reading["
		<<mReadingCount<<"] free["<<mTank.getReadFreeList().count()<<"]";
	}
	return oss.str();
}

//can only be called by FreeList
void ReadCache::remove( const MBMetaData& metadata ) {
	ZQ::common::MutexGuard gd(mLocker);
	MBMAP::iterator it = mMbMap.find(metadata);
	if( it == mMbMap.end() ) {
		return;
	}
	MemoryBlockInfo* mbi = it->second;	
	removeMetadata(mbi);	
	
	removeFromExpireChecker( mbi );
	mMbMap.erase(it);
	mbi->removeFromReadCache();
}

void ReadCache::remove( MemoryBlockInfo* mbi ) {
	ZQ::common::MutexGuard gd(mLocker);
	removeMetadata(mbi);
	removeFromExpireChecker(mbi);
	mMbMap.erase( mbi->md() );
	mbi->removeFromReadCache();
}

void ReadCache::validate( const MBMetaData& md ) {
	std::vector<MBMetaData> mds = mTank.convertRangeToBlock(md.filePath,
													md.dataRange.begin, md.dataRange.size, 
													true);
	ZQ::common::MutexGuard gd(mLocker);
	std::vector<MBMetaData>::const_iterator it = mds.begin();
	for( ; it != mds.end() ; it ++ ) {
		const MBMetaData& metadata = *it;
		MBMAP::iterator it = mMbMap.find(metadata);
		if( it == mMbMap.end() )
			continue;
		
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ReadCache,"void [%s]"),md.toString().c_str());
		MemoryBlockInfo* mbi = it->second;
		removeMetadata(mbi);	
		removeFromExpireChecker( mbi );
			
		assert( !mbi->inWriteBuffer() );
		assert( !mbi->inFlushRunner() );
		assert( !mbi->isBeingWrite() );
		assert( mbi->inReadCache() );

		// remove from Cache Map anyway
		if(mbi->inCacheRunner() || mbi->isBeingRead() ) {
			mbi->markValidate();
		}

		mMbMap.erase(it);	
		mbi->removeFromReadCache();

		if( mbi->inFreeList() ) {
			continue;
		}
		if( !( mbi->isBeingRead() || mbi->inCacheRunner() ) )
			mTank.getReadFreeList().add( mbi );
	}
}

void ReadCache::onReadComplete( MemoryBlockInfo* mbi ) {
	//assert( mbi->inReadCache() );
	//assert( !mbi->inCacheRunner() );
	assert( !mbi->inFreeList() );
	assert( !mbi->inWriteBuffer() );
	assert( !mbi->isBeingWrite() );
	assert( !mbi->isBeingRead() );
	ZQ::common::MutexGuard gd(mLocker);
	mTank.getReadFreeList().add(mbi);
}

void ReadCache::onCacheComplete( MemoryBlockInfo* mbi ) {
	//assert( mbi->inReadCache() );
	assert( mbi->inCacheRunner() );
	
	assert( !mbi->inExpireChecker() );
	assert( !mbi->inFreeList() );
	assert( !mbi->inWriteBuffer() );
	assert( !mbi->inFlushRunner() );
	assert( !mbi->isBeingWrite() );
	NotifierPtr n;
	{
		ZQ::common::MutexGuard gd(mLocker);
		mNAMemoryBlockCount --;	
		mbi->removeFromCacheRunner();
		n = mbi->notifier();
		mbi->notifier( 0 );
		mbi->touch();
		
		if( !mbi->needValidate() ) {
			addToExpireChecker(mbi);
		} 
		if( !mbi->isBeingRead() && !mbi->inFreeList() ) {
			mTank.getReadFreeList().add(mbi);
		}
	}
	if(n)
		n->broadcast();
}

void ReadCache::onCacheComplete( const std::vector<MemoryBlockInfo*>& mbis )  {
	ZQ::common::MutexGuard gd(mLocker);
	std::vector<MemoryBlockInfo*>::const_iterator it = mbis.begin();
	for( ; it != mbis.end(); it ++ ) {
		onCacheComplete(*it);
	}
}

void ReadCache::onExpired( MemoryBlockInfo* mbi ) {
	mTank.onReadCacheExpired(mbi);
}

void ReadCache::onMBExpired( MemoryBlockInfo* mbi ) {
	validate( mbi->md() );
}


MemoryBlockInfo* ReadCache::getMb( const MBMetaData& metadata) {
	MemoryBlockInfo* mbi = mTank.getReadFreeList().fetch();
	if(!mbi) {
		return 0;
	}
	assert( !mbi->inFreeList() );
	assert( !mbi->inReadCache() );
	assert( !mbi->inWriteBuffer() );
	assert( !mbi->inCacheRunner() );
	assert( !mbi->inFlushRunner() );
	assert( !mbi->inExpireChecker() );

	mbi->notifier( new Notifier() );
	mbi->md(metadata);

	assert( mMbMap.find(metadata) == mMbMap.end() );

	mMbMap[ metadata ] = mbi;

	mbi->addToReadCache();

	mbi->addToCacheRunner();
	return mbi;
}

//////////////////////////////////////////////////////////////////////////
///WriteBuffer
WriteBuffer::WriteBuffer( DataTank& tank ) 
:File2Metadata(tank),
mTank(tank),
mFlushCenter(tank),
#ifdef _USE_HASHMAP_
mMbMap( tank.getConf().writeBufferBlockSize*2),
#endif// _USE_HASHMAP_
mFlushingCount(0){
	reserve(mTank.getConf().writeBlockCount);
	mFlushCenter.start();
}

WriteBuffer::~WriteBuffer() {
	mFlushCenter.stop();
}

std::string WriteBuffer::mbUsage() const {
	std::ostringstream oss;
	{
		ZQ::common::MutexGuard gd(mLocker);
		oss<<"Buffered["<<mMbMap.size()<<"] Flushing["<<mFlushingCount<<"]";
	}
	return oss.str();
}

void WriteBuffer::putMemory( const MBWriter& writer ) {
	std::vector<MBWriter> writers;
	writers.push_back( writer);
	putMemory( writers );
}

void WriteBuffer::putMemory( const std::vector<MBWriter>& writers ) {
	std::vector<MemoryBlockInfo*> mbis;
	ZQ::common::MutexGuard gd(mLocker);
	std::vector<MBWriter>::const_iterator it = writers.begin();
	for ( ; it != writers.end() ; it ++ ) {
		MemoryBlockInfo* mbi = it->mbi();
		long user = mbi->removeUser();
		if( user <= 0 ) {
			mbi->removeFromWrite();
			mbis.push_back(mbi);
		}
	}
	if( mbis.size() > 0 ) {
		evict(mbis);
	}
}

void WriteBuffer::putMemory( MemoryBlockInfo* mbi ) {
	assert( mbi != 0 );
	ZQ::common::MutexGuard gd(mLocker);
	long user = mbi->removeUser();
	if( user <= 0 ) {
		mbi->removeFromWrite();
		evict( mbi->md() );
	}
}
MBWriter WriteBuffer::getMemory( const MBMetaData& metadata, int  ) {
	ZQ::common::MutexGuard gd(mLocker);
	MemoryBlockInfo* mbi = 0;
	MBMAP::iterator it = mMbMap.find( metadata);
	if( it != mMbMap.end()) {
		mbi = it->second;
	} else {
		mbi =  mTank.getWriteFreeList().fetch();
		if(!mbi) {
			forceFlushOne();
			mbi =  mTank.getWriteFreeList().fetch();
			if(!mbi)
				return MBWriter(mTank,0);
		}
		mbi->touch();
		mbi->addToWriteBuffer( );
		addToExpireChecker(mbi);
		mbi->md(metadata);
		addMetaData(mbi);
		mMbMap[metadata] = mbi;
	}
	assert( !mbi->inFreeList() );
	assert( !mbi->inFlushRunner() );
	mbi->addUser();
	mbi->addToWrite();
	MBWriter writer(mTank,mbi);
	return writer;
}

std::vector<MBWriter> WriteBuffer::getMemory( const MBMetaData& metadata ) {
	static size_t writeBlockSize = mTank.getConf().writeBufferBlockSize;
	
	size_t blockCount = metadata.dataRange.size / writeBlockSize;
	assert( blockCount > 0 );
	assert( metadata.isValid() && blockCount * writeBlockSize == metadata.dataRange.size );

	std::vector<MBWriter> writers;
	writers.reserve( blockCount );

	std::vector<MBMetaData> mds;
	mds.reserve( blockCount );

	MBMetaData md = metadata;
	md.dataRange.size = writeBlockSize;
	ZQ::common::MutexGuard gd(mLocker);
	for( size_t p = 0; p < metadata.dataRange.size ; ) {
		MBWriter writer = getMemory(md, 0);
		if( !writer.isValid() ) {
			putMemory( writers);
			writers.clear();
			return writers;
		}
		writers.push_back( writer );
		md.dataRange.begin += writeBlockSize;
		p += writeBlockSize;
	}
	return writers;
}

void WriteBuffer::validate( const std::string& filepath ) {	
	ZQ::common::MutexGuard gd(mLocker);
	MemoryBlockInfo* head = getMetadata(filepath);
	if(!head)
		return;
	bool bOK = head->next() != head;
	MemoryBlockInfo* p = head->next();
	while( bOK ) {
		bOK = p->next() != head;
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(WriteBuffer,"validate() validate data for file[%s] MBStart[%llu] length[%u]"),
			filepath.c_str(), p->md().dataRange.begin, p->md().dataRange.size );
		if(evict( p->md(),true,false) ) {
			//p = head->next();
			head = getMetadata(filepath);
			bOK = (head != NULL) && (head->next() != head);
			if(bOK) {
				p = head->next();
			}
		} else {
			p = p->next();
		}
	}
}

int WriteBuffer::waitFlush( const std::string& filePath, int index ) {
	ZQ::common::MutexGuard gd(mLocker);
	int err = 0;
	FlushRecorderMap::iterator it = mFlushingRecorders.find( filePath );
	if( it == mFlushingRecorders.end() )
		return err;

	FlushRecorder& recorder = it->second;	
	FlushRecorder::FlushingInfoMap::iterator itMap = recorder.flushingInfo.find(index);
	if(itMap == recorder.flushingInfo.end() )
		return err;
	err = itMap->second.lastError;
	if( itMap->second.count <= 0 )
		recorder.flushingInfo.erase(itMap);

	if( recorder.flushingInfo.size() == 0)
		mFlushingRecorders.erase( it );
	return err;
}

void WriteBuffer::flushAll( ) {
	ZQ::common::MutexGuard gd(mLocker);
	std::vector<std::string> files = getFileList();
	std::vector<std::string>::const_iterator it = files.begin();
	for( ; it != files.end() ; it++ ) {
		validate(*it);
	}
	//do not block this call, so return here
}

NotifierPtr WriteBuffer::flush( const std::string& filePath, int& index ) {	
	ZQ::common::MutexGuard gd(mLocker);
	validate(filePath);	

	FlushRecorderMap::iterator it = mFlushingRecorders.find( filePath );
	
	if( it == mFlushingRecorders.end() )
		return 0;

	FlushRecorder& recorder = it->second;
	index = recorder.flushIndex ++;
	FlushRecorder::FlushingInfoMap::iterator itMap = recorder.flushingInfo.find(index);
	if( itMap == recorder.flushingInfo.end() )
		return 0;
	assert( itMap->second.count >= 0 );
	assert( itMap->second.notifier );
	itMap->second.hasSinker = true;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(WriteBuffer,"flush() there are data should be flushed for file[%s] count[%d], waiting..."),
			filePath.c_str(), itMap->second.count );
	return itMap->second.notifier;
}

bool WriteBuffer::forceFlushOne( const MBMetaData& md ) {	
	if( md.isValid() ) {
		ZQ::common::MutexGuard gd(mLocker);
		MBMAP::const_iterator it = mMbMap.find(md);
		if( it == mMbMap.end() )
			return false;
		const MemoryBlockInfo* mbi = it->second;
		if(!mbi) 
			return false;
		evict(mbi->md(), true, true );
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(WriteBuffer,"forceFlushOne() flushed or marked as flush for [%s]"),md.toString().c_str());
		return true;// flush only one memory block
	} else {
		// randomly pick a memory block and flush it to server
		static size_t threadCount = mTank.getFlushThreadPool().size();
		size_t pendingSize = mTank.getFlushThreadPool().pendingRequestSize();
		size_t activeCount = mTank.getFlushThreadPool().activeCount();
		if( pendingSize >= 2* threadCount ) {
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(WriteBuffer, "forceFlushOne() threadPool usage: threadCount[%d] pendingReqs[%d] activeCount[%d], server-side too slow"),
				(int)threadCount, (int)pendingSize, (int)activeCount);

			return false;
		}
		ZQ::common::MutexGuard gd(mLocker);
		MBMAP::const_iterator it = mMbMap.begin();
		for( ; it != mMbMap.end() ; it ++ ) {
			const MemoryBlockInfo* mbi = it->second;
			if(!mbi) continue;
			if( mbi->userCount() <= 0 ) {
				evict(mbi->md(), true, true );
				return true;// flush only one memory block
			}
		}
	}
	return false;
}

bool WriteBuffer::evict( const MBMetaData& metadata, bool bForce, bool bDirect ) {
	ZQ::common::MutexGuard gd(mLocker);
	MemoryBlockInfo* mbi = 0;
	MBMAP::iterator it = mMbMap.find(metadata);
	if( it == mMbMap.end() )
		return false;
	mbi = it->second;
	
	if( mbi->isBeingWrite() && (mbi->userCount() > 0) )
		return false;

	if( ! (bForce || mbi->needValidate() || mbi->isDataFull() ) ) {
		return false;
	}
	assert( !mbi->isBeingWrite() );
	assert( !mbi->inFlushRunner() );
	assert( !mbi->inFreeList() );	
	mMbMap.erase(it);
	removeMetadata( mbi );
	removeFromExpireChecker( mbi );
	mbi->removeFromWriteBuffer( );

	if(mbi->dataSize() > 0 ) {
		if( !bDirect ) {
			flushByRunner(mbi);
		} else {
			flushDirect(mbi);
		}
	} else {
		mTank.getWriteFreeList().add(mbi);
	}
	return true;
}

bool WriteBuffer::evict( const std::vector<MemoryBlockInfo*>& mbis ) {
	assert( mbis.size() > 0 );
	ZQ::common::MutexGuard gd(mLocker);
	std::vector<MemoryBlockInfo*>::const_iterator it = mbis.begin();
	std::vector<MemoryBlockInfo*>::const_iterator itFirst;
	bool bFirstValid = false;
	for( ; it != mbis.end() ; it ++ ) {
		MemoryBlockInfo* mbi = *it;
		if( ( mbi->isBeingWrite() && (mbi->userCount() > 0 ) ) 
				|| 
			! ( mbi->needValidate() || mbi->isDataFull() ) ) {
			if( bFirstValid ) {
				bFirstValid = false;
				std::vector<MemoryBlockInfo*> flushingMbis;
				flushingMbis.assign(itFirst, it);
				flushByRunner( flushingMbis );
			}
			continue;
		}

		assert( !mbi->isBeingWrite() );
		assert( !mbi->inFlushRunner() );
		assert( !mbi->inFreeList() );
		
		mMbMap.erase(mbi->md());
		removeMetadata(mbi);
		removeFromExpireChecker(mbi);
		mbi->removeFromWriteBuffer();

		if(mbi->dataSize() == 0 ) {
			mTank.getWriteFreeList().add(mbi);
		} else {
			if(!bFirstValid) {
				itFirst = it;
				bFirstValid = true;
			} 
		}
	}
	if( bFirstValid ) {
		bFirstValid = false;
		std::vector<MemoryBlockInfo*> flushingMbis;
		flushingMbis.assign(itFirst, it);
		flushByRunner( flushingMbis );
	}
	return true;
}


void WriteBuffer::flushDirect( MemoryBlockInfo* mbi ) {
	MBWriter writer(mTank,mbi);
	size_t size  = 0;
	mFlushingCount ++;
	writer.write(0,0,size,0,true);
	if( mbi->dataSize() == mbi->bufSize() ) {
		//mbi->setDataAvailable();		
		//mTank.getReadCache().addCachedData(mbi);
	}
	{
		MutexUnlocker un_guard(mLocker);
		mTank.getReadCache().validate(mbi->md());
	}
	mFlushingCount --;
	mTank.getWriteFreeList().add(mbi);
}

void WriteBuffer::flushByRunner( MemoryBlockInfo* mbi ) {
	std::vector<MemoryBlockInfo*> mbis;
	mbis.push_back( mbi );
	flushByRunner( mbis );
}

void WriteBuffer::flushByRunner( const std::vector<MemoryBlockInfo*>& mbis ) {
	assert( mbis.size() > 0 );
	unsigned long long posBegin = mbis[0]->md().dataRange.start();
	unsigned long long posEnd = mbis[mbis.size()-1]->md().dataRange.end();
	const std::string& filePath = mbis[0]->md().filePath;
	std::vector<int> flushIndices;
	std::vector<MemoryBlockInfo*>::const_iterator it = mbis.begin();
	for ( ; it != mbis.end() ; it ++ ) {
		MemoryBlockInfo* mbi = *it;
		mbi->addToFlushRunner( );

		int flushIndex = 0 ;
		int flushingFileCount = 50;
		const std::string& filePath = mbi->md().filePath;
		FlushRecorderMap::iterator it = mFlushingRecorders.find(filePath);
		if( it != mFlushingRecorders.end() ) {
			FlushRecorder& recorder = it->second;
			FlushRecorder::FlushingInfoMap& infomap = recorder.flushingInfo;
			flushIndex = recorder.flushIndex;
			FlushRecorder::FlushingInfoMap::iterator itMap = infomap.find(recorder.flushIndex);
			if(itMap == infomap.end()) {
				FlushRecorder::FlushingInfo finfo;
				finfo.notifier = new Notifier();
				finfo.count = 1;
				infomap[recorder.flushIndex] = finfo;
				flushingFileCount +=1;
			} else {
				itMap->second.count ++;
				flushingFileCount += itMap->second.count;
			}
		} else {
			FlushRecorder rec;
			flushIndex = rec.flushIndex;
			rec.flushingInfo[rec.flushIndex].count = 1;
			rec.flushingInfo[rec.flushIndex].notifier =  new Notifier();
			mFlushingRecorders[filePath] = rec;
			flushingFileCount += 1;
		}

		mFlushingCount ++;
		flushIndices.push_back( flushIndex);
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(WriteBuffer,"flushByRunner() file[%s], start[%llu] end[%llu] size[%llu]"),
			filePath.c_str(), posBegin, posEnd, posEnd - posBegin);
	mFlushCenter.post( mbis, flushIndices );
}

void WriteBuffer::onExpired( MemoryBlockInfo* mbi ) {
	mTank.onWriteBufferExpired(mbi);	
}

void WriteBuffer::onMBExpired( MemoryBlockInfo* mbi ) {
	mbi->markValidate();
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(WriteBuffer,"onMBExpired() mbi[%s] expired, flush it "),
			mbi->toString().c_str());
	ZQ::common::MutexGuard gd(mLocker);
	evict( mbi->md(), true, false );
}

void WriteBuffer::onFlushComplete( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& flushIndices) {
	assert( mbis.size() == flushIndices.size());
	ZQ::common::MutexGuard gd(mLocker);
	size_t total = mbis.size();
	for( size_t i = 0 ; i < total; i ++ ) {
		onFlushComplete( mbis[i], flushIndices[i] );
	}
}
void WriteBuffer::onFlushComplete( MemoryBlockInfo* mbi , int index ) {
	ZQ::common::MutexGuard gd(mLocker);
	mFlushingCount --;
	NotifierPtr n = 0 ;
	bool hasSinker = false;
	
	const std::string& filePath = mbi->md().filePath;
	FlushRecorderMap::iterator it = mFlushingRecorders.find(filePath);
	assert( it != mFlushingRecorders.end() );	
	FlushRecorder& recorder = it->second;
	FlushRecorder::FlushingInfoMap::iterator itFlusingMap = recorder.flushingInfo.find(index);
	assert( itFlusingMap != recorder.flushingInfo.end() );
	FlushRecorder::FlushingInfo& flushingInfo = itFlusingMap->second;
	flushingInfo.count --;
	hasSinker = flushingInfo.hasSinker;

	assert( flushingInfo.count >= 0 );
	if( mTank.isSuccess( flushingInfo.lastError) )
		flushingInfo.lastError = mbi->errorCode();


	if( flushingInfo.count == 0 && hasSinker) {
		n = flushingInfo.notifier;		
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(WriteBuffer,"onFlushComplete() remove [%s] from flushing map [%d] and trying to inform event sinker"),
			mbi->md().toString().c_str(), index);
	} else {
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(WriteBuffer,"onFlushComplete() remove [%s] from flushing map [%d] count[%d]"),
			mbi->md().toString().c_str(), index, (unsigned int)flushingInfo.count);
		if( flushingInfo.count <= 0 ) {
			recorder.flushingInfo.erase(itFlusingMap); // no waiter, just erase the record
			if( recorder.flushingInfo.size() <= 0 ) 
				mFlushingRecorders.erase( it );
		}
	}
	mbi->removeFromFlushRunner( );
	//mbi->setDataAvailable();
	//mTank.getReadCache().addCachedData(mbi);
	{
		MutexUnlocker un_lock(mLocker);
		mTank.getReadCache().validate(mbi->md());
	}
	assert( !mbi->inWriteBuffer() );
	assert( !mbi->isBeingWrite() );
	assert( !mbi->inFreeList() );
	mTank.getWriteFreeList().add(mbi);
	if( hasSinker && n ){
		n->broadcast();
	} 
}

//////////////////////////////////////////////////////////////////////////
/// IMPL: DataTank
DataTank::DataTank( ZQ::common::NativeThreadPool& readPool, ZQ::common::NativeThreadPool& writePool, ZQ::common::Log& logger, DataTankConf& conf )
:mTank(*this),
mTankConf(conf),
mReadPool(readPool),
mWritePool(writePool),
mLogger(logger),
mReadFreeList(*this),
mWriteFreeList(*this),
mReadCache(*this),
mWriteBuffer(*this)
{
}

DataTank::~DataTank() {	
	destroyTank();
}

const size_t PAGE_SIZE = 4096;

bool DataTank::createTank( ) {	
	//mTankConf = conf;
	
	//round up to PAGE_SIZE align	
	mTankConf.readCacheBlockSize = (mTankConf.readCacheBlockSize + PAGE_SIZE - 1)/ PAGE_SIZE * PAGE_SIZE;
	mTankConf.writeBufferBlockSize = (mTankConf.writeBufferBlockSize + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;

	mLogger(ZQ::common::Log::L_INFO,CLOGFMT(dataTank,"createTank() create by blockSize[%d/%d] "
		"blockCount[%d/%d] invalidateInterval[%lld/%lld] "
		"readThreads[%d] writeThreads[%d] logFlag[%x] readAheadCount[%d/%d] maxRecognitions[%d] maxWriteQueueLength[%d/%d] writeYield[%d/%d/%d]"),
			(int)mTankConf.readCacheBlockSize,
			(int)mTankConf.writeBufferBlockSize,
			(int)mTankConf.readBlockCount,
			(int)mTankConf.writeBlockCount,
			(long long)mTankConf.cacheInvalidationInterval,
			(long long)mTankConf.bufferInvalidationInterval,
			(int)mReadPool.size(),
			(int)mTankConf.flushThreadPoolSize,
			(unsigned int)mTankConf.logFlag,
			(int)mTankConf.readAheadCount,
			(int)mTankConf.readAheadThreshold,
			(int)mTankConf.mergableArrayMaxItemSize,
			(int)mTankConf.maxWriteQueueMergeItemCount,
			(int)mTankConf.maxWriteQueueBufferCount,
			(int)mTankConf.writeThreadsOfYield,
			(int)mTankConf.writeYieldMax,
			(int)mTankConf.writeYieldMin
		);

	//ZQ::common::MutexGuard gd(mLocker);	

	if(!mReadFreeList.create( mTankConf.readBlockCount, mTankConf.readCacheBlockSize ) )
		return false;
	if(!mWriteFreeList.create( mTankConf.writeBlockCount, mTankConf.writeBufferBlockSize) )
		return false;
	mWritePool.resize( (int)mTankConf.flushThreadPoolSize );
	mWriteBuffer.start( (size_t)mTankConf.bufferInvalidationInterval, (size_t)mTankConf.writeBlockCount );
	mReadCache.start( (size_t)mTankConf.cacheInvalidationInterval, (size_t)mTankConf.readBlockCount );
	
	return true;
}

void DataTank::destroyTank( )
{	
	mWriteBuffer.stop();	
	mReadCache.stop();
	mReadPool.stop();
	while(mWritePool.pendingRequestSize() > 0 )
		ZQ::common::delay(10);//wait for another 10 milliseconds to wait all pending write back requests to be finished
	mWritePool.stop();
	mInvalidationSem.post();
	//ZQ::common::MutexGuard gd(mLocker);
	//TODO: not implement yet
	//      wait for all outstanding request to complete
	//      delete allocated memory block

	// can we make sure that all referrence to memory block are deleted ?
	mReadFreeList.destroy();
	mWriteFreeList.destroy();
}

int DataTank::validate_readcache( const std::string& filePathName ) {
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(DataTank,"validating read cache for file[%s]"), filePathName.c_str());
	mReadCache.validate(filePathName);
	return 0;
}

void DataTank::flushAllWriteBuffer( ) {
	mWriteBuffer.flushAll();
}
int DataTank::validate_readcache( const std::string& filePathName , unsigned long long begin , unsigned int size ) {

	std::vector<MBMetaData> blocks = convertRangeToBlock(filePathName,begin,size,true);
	std::vector<MBMetaData>::const_iterator it = blocks.begin(); 
	for( ; it != blocks.end(); it ++ )
		mReadCache.validate( *it );
	return 0;
}

/*
int DataTank::cacheRead( const std::string& filePathName, 
						char* buffer, 
						unsigned long long begin, unsigned int& size ) {
	if(size<=0)
		return 0;

	unsigned long long origBegin = begin;
	unsigned int origSize = size;

	int64 stampStart = ZQ::common::now();
	validate_writebuffer( filePathName );

	mLogger(ZQ::common::Log::L_DEBUG, CLOGFMT(DataTank,"cacheRead() reading file[%s] data[%llu+%u]"),
		filePathName.c_str(), begin, size);

	std::vector<MBMetaData> blocks = convertRangeToBlock(filePathName,begin,size,true);
	std::vector<MBMetaData>::const_iterator it = blocks.begin();

	unsigned int sizeTotal = 0 ;
	for ( ; it != blocks.end(); it ++ ) {
		if( begin < it->dataRange.begin )
			break;
		MBReader reader(*this,0);
		bool bEnterDegrade = false;
		do {
			bEnterDegrade = false;
			{
				ZQ::common::MutexGuard gd(mLocker);
				//mWriteBuffer.forceFlushOne(*it);
				reader = mReadCache.getMemory(*it);		
				if(!reader.isValid()) {	
					if(!bEnterDegrade) {
						MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(DataTank,"cacheRead() no valid memory block is found. enter degrade mode"));
						MLOG(ZQ::common::Log::L_INFO,CLOGFMT(FreeList,"mbusage: %s %s"),
							mReadCache.mbUsage().c_str(),	mWriteBuffer.mbUsage().c_str() );
						bEnterDegrade = true;
					}
				}
			}
			if(bEnterDegrade)
				ZQ::common::delay(10);
		} while( !reader.isValid() );

		size_t sizeWant = (size_t)size;

		if( 0 != reader.read( (size_t)(begin - it->dataRange.begin), buffer, sizeWant) ) {
			size  = 0;
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DataTank,"cacheRead() failed to get any data from memory block"));
			ZQ::common::MutexGuard gd(mLocker);
			mReadCache.putMemory(reader);
			return -1;
		}

		if( !isSuccess(reader.lastError() ) ) {
			size = 0;
			ZQ::common::MutexGuard gd(mLocker);
			mReadCache.putMemory(reader);
			return reader.lastError();
		}

		size -= (unsigned int)sizeWant;
		buffer += sizeWant;
		sizeTotal += (unsigned int)sizeWant;
		begin +=sizeWant;
		{
			ZQ::common::MutexGuard gd(mLocker);
			mReadCache.putMemory(reader);
		}
	}

	size = sizeTotal;
	mLogger(ZQ::common::Log::L_INFO, CLOGFMT(DataTank,"cacheRead() has read file[%s] data[%llu+%u], took [%d]msec, received[%u]"),
		filePathName.c_str(), origBegin, origSize, (int)(ZQ::common::now() - stampStart), sizeTotal);

	return 0;
}
*/

int DataTank::cacheRead( const std::string& filePathName, 
						char* buffer, 
						unsigned long long begin, unsigned int& size ) {
	if(size<=0)
		return 0;

	static size_t readBlockSize = getConf().readCacheBlockSize;
	unsigned long long origBegin = begin;
	unsigned int origSize = size;

	StopWatch s1, s2(s1), s3(s1);

	mLogger(ZQ::common::Log::L_DEBUG, CLOGFMT(DataTank,"cacheRead() reading file[%s] data[%llu+%u]"),
		filePathName.c_str(), begin, size);

	if (mTankConf.readAfterFlushDirty)
		validate_writebuffer(filePathName);

	// std::vector<MBMetaData> blocks = convertRangeToBlock(filePathName,begin,size,true);
	MBMetaData block;
	block.filePath = filePathName;
	block.dataRange.begin = begin / readBlockSize *readBlockSize;
	unsigned long long tmpEnd = begin + size;
	tmpEnd = ( tmpEnd + readBlockSize - 1 ) / readBlockSize * readBlockSize;
	block.dataRange.size = tmpEnd - block.dataRange.begin;

	std::vector<MBReader> readers ;
	while( true ) {
		readers = mReadCache.getMemory( block );
		if( readers.size() == 0) {
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(DataTank,"cacheRead() no valid memory block is found. enter degrade mode"));
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(FreeList,"mbusage: %s"),
					mReadCache.mbUsage().c_str() );
			ZQ::common::delay(10);
		}
		else
			break;
	}
	
	s1.stop();

	unsigned int sizeTotal = 0 ;
	size_t readerIndex  = 0;
	std::vector<MBReader>::const_iterator it = readers.begin();

	for ( ; it != readers.end(); it ++ ) {
		if( begin < it->mbi()->md().dataRange.begin )
			break;
		MBReader& reader = readers[readerIndex ++];
		
		size_t sizeWant = (size_t)size;

		if( 0 != reader.read( (size_t)(begin - it->mbi()->md().dataRange.begin), buffer, sizeWant) ) {
			size  = 0;
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DataTank,"cacheRead() failed to get any data from memory block"));
			//ZQ::common::MutexGuard gd(mLocker);
			mReadCache.putMemory(readers);
			return -1;
		}

		if( !isSuccess(reader.lastError() ) ) {
			size = 0;
			//ZQ::common::MutexGuard gd(mLocker);
			mReadCache.putMemory(readers);
			return reader.lastError();
		}

		size -= (unsigned int)sizeWant;
		buffer += sizeWant;
		sizeTotal += (unsigned int)sizeWant;
		begin +=sizeWant;
	}
	s2.stop();
	
	{
		mReadCache.putMemory(readers);
	}

	size = sizeTotal;

	s3.stop();

	mLogger(ZQ::common::Log::L_INFO, CLOGFMT(DataTank,"cacheRead() has read file[%s] data[%llu+%u], took [%lld/%lld/%lld]usec, received[%u]"),
		filePathName.c_str(), origBegin, origSize, s1.cost(), s2-s1,s3-s2, sizeTotal);

	return 0;
}

void DataTank::preCacheWrite( const std::string& filePathName, size_t size ) {
	if( getConf().writeThreadsOfYield <= 0 )
		return;

	int64 bitrate = 0xFFFFFFFF;
	size_t count = mWriteBuffer.getFlushCenter().getInflightRequests( filePathName, bitrate );
	size_t delta = count;
	if( count <= getConf().writeThreadsOfYield ) 
		return;
	size_t bufferCount = getWriteFreeList().count();
	if(bufferCount >= getConf().writeBufferCountOfYield)
		return;
	bitrate *= 2 * getConf().writeThreadsOfYield;
	int64 calcLatency = (int64)size * 8000 / bitrate;
	delta -= getConf().writeThreadsOfYield;
	delta = MIN(delta,10);
	for( ; delta > 0 ; delta -- ) {
		calcLatency  += calcLatency>>1;
	}
	if( calcLatency < (int64)getConf().writeYieldMin )
		return;
	calcLatency = MIN( calcLatency, (int64)getConf().writeYieldMax);
	mLogger(ZQ::common::Log::L_INFO, CLOGFMT(DataTank,"preCacheWrite() sleep [%ld]ms for [%s] due to avgBitrate[%ld] threads[%zu] buffer[%d/%d]"),
			calcLatency, filePathName.c_str(), bitrate, count, (int)bufferCount, (int)getConf().writeBlockCount );
	ZQ::common::delay( calcLatency );
}

int DataTank::cacheWrite( const std::string& filePathName, 
						 const char* buffer, 
						 unsigned long long begin, unsigned int& size ) {
	if(!buffer || size <= 0 )
		return -1;
	preCacheWrite( filePathName, size );
	StopWatch s1, s2(s1),s3(s1);
	unsigned long long originalBegin = begin;
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(DataTank,"cacheWrite() writing file[%s] range[%llu+%u]"),
			filePathName.c_str(), begin,size);
	unsigned int sizeTotal  = 0;

	static size_t writeBlockSize = getConf().writeBufferBlockSize;
	MBMetaData block;
	block.filePath = filePathName;
	block.dataRange.begin = begin / writeBlockSize * writeBlockSize;
	unsigned long long tmpEnd = begin + size;
	tmpEnd = ( tmpEnd + writeBlockSize - 1 ) / writeBlockSize * writeBlockSize;
	block.dataRange.size = tmpEnd - block.dataRange.begin;

	std::vector<MBWriter> writers;
	while(true) {
		writers = mWriteBuffer.getMemory( block );
		if( writers.size() == 0 ) {
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(DataTank, "cacheWrite() run out of buffers: %s server-side too slow"),
					mWriteBuffer.mbUsage().c_str());
			ZQ::common::delay(10);
		} else {
			break;
		}
	}
	s1.stop();
	
	s2.start();
	size_t writerIndex = 0;
	std::vector<MBWriter>::const_iterator it = writers.begin();
	for ( ; it != writers.end(); it ++ ) {
		size_t sizeWant = size;
		size_t offset = (size_t)(begin - it->mbi()->md().dataRange.begin);
		MBWriter& writer = writers[writerIndex++];
		if( 0 != writer.write( offset, buffer, sizeWant, begin )) {
			size = 0;
			mLogger(ZQ::common::Log::L_ERROR,CLOGFMT(DataTank,"cacheWrite() failed to write data [%s:%llu:%u:%u] to buffer"),
					filePathName.c_str(),begin,(unsigned int)offset,(unsigned int)sizeWant);
			break;
		}
		size -= (unsigned int)sizeWant;
		buffer += sizeWant;
		begin += sizeWant;
		sizeTotal += (unsigned int)sizeWant;

	}
	s2.stop();
	size = sizeTotal;
	
	s3.start();
	mWriteBuffer.putMemory( writers );
	s3.stop();

	mLogger(ZQ::common::Log::L_INFO, CLOGFMT(DataTank,"cacheWrite() wrote file[%s] offset[%llu+%u] time[%lld/%lld/%lld]"),
			filePathName.c_str(), originalBegin, sizeTotal,
			s1.cost(), s2.cost(), s3.cost() );
	 return 0;
}

std::vector<MBMetaData> DataTank::convertRangeToBlock( const std::string& filename, 
													  unsigned long long begin, size_t size,
													  bool readOrWrite ) {
	  std::vector<MBMetaData> blocks;
	  MBMetaData block;
	  block.filePath = filename;
	  
	  size_t memoryBlockSize = readOrWrite ? mTankConf.readCacheBlockSize : mTankConf.writeBufferBlockSize;

	  unsigned long long startAddress = begin / memoryBlockSize * memoryBlockSize;
	  unsigned long long endAddress = (begin + size -1 + memoryBlockSize) / memoryBlockSize * memoryBlockSize;

	  while( startAddress < endAddress ) {
		  block.dataRange.begin = startAddress;
		  block.dataRange.size = static_cast<unsigned int>(memoryBlockSize - startAddress % memoryBlockSize);
		  blocks.push_back(block);
		  if( size < block.dataRange.size)
			  size = 0;
		  else
			  size -= block.dataRange.size;
		  startAddress += memoryBlockSize;
	  }
	  return blocks;
}

int DataTank::validate_writebuffer( const std::string& filePathName ) {
	NotifierPtr n = 0;
	int err = 0;
	int index = -1;
	{
		//mReadCache.validate(filePathName);
		n = mWriteBuffer.flush(filePathName, index);
	}

	if( n )
	{
		n->wait();

		err  = mWriteBuffer.waitFlush(filePathName,index);
	}

	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(DataTank, "file[%s] validated, flush errno[%d]"),
		filePathName.c_str(), err );
	return err;
}

void DataTank::onCacheComplete( MemoryBlockInfo* mbi ) {
	mReadCache.onCacheComplete(mbi);
}

void DataTank::onCacheComplete( const std::vector< MemoryBlockInfo*> &mbis ) {
	mReadCache.onCacheComplete( mbis );
}

void DataTank::onFlushComplete( MemoryBlockInfo* mbi, int flushIndex ) {
	mWriteBuffer.onFlushComplete(mbi, flushIndex);
}

void DataTank::onFlushComplete( const std::vector< MemoryBlockInfo*>& mbis, const std::vector< int >& flushIndices ) {
	assert( mbis.size() == flushIndices.size() );
	mWriteBuffer.onFlushComplete( mbis, flushIndices);
}

void DataTank::onWriteComplete( MemoryBlockInfo* mbi ) {
	mWriteBuffer.evict( mbi->md() );
}

void DataTank::onReadCacheExpired( MemoryBlockInfo* mbi ) {
	mReadCache.onMBExpired(mbi);
}

void DataTank::onWriteBufferExpired( MemoryBlockInfo* mbi ) {
	mWriteBuffer.onMBExpired(mbi);
}

//////////////////////////////////////////////////////
//
ReadAheadStatisics::ReadAheadStatisics( DataTank& tank ):
mTank(tank)
//mHitMap( tank.getConf().maxReadHitTrackerCount ) 
{
}

ReadAheadStatisics::~ReadAheadStatisics( ) {
}

size_t ReadAheadStatisics::calcReadAheadCount( size_t hitCount ) {
	static size_t threshold = mTank.getConf().readAheadThreshold;
	static size_t count = mTank.getConf().readAheadCount;
	static size_t logbase = mTank.getConf().readAheadIncreamentLogBase;

	if( hitCount <= threshold )
		return 0;

	size_t exp = hitCount - threshold;
	size_t calcCount = logbase;
	while( ( calcCount < count ) && (--exp > 0 ) ) {
		calcCount = calcCount * logbase;
	}
	return MIN(calcCount,count);
}

size_t ReadAheadStatisics::readAt( const std::string& filepath, const Range& range) {
	size_t hitCount = 0;
	{
		ZQ::common::MutexGuard gd(mLocker);
		HITMAP::iterator it = mHitMap.find(filepath );
		if( it == mHitMap.end() ) {
			MergeableArrayPtr ma = new MergeableArray(mTank);
			hitCount = ma->add( range );
			mHitMap[ filepath ] = ma;
		} else {
			MergeableArrayPtr ma = it->second;
			hitCount = ma->add(range);
			mHitMap[filepath] = ma;
		}
	}
	return calcReadAheadCount( hitCount );
}


///////////////////////////////////////////////////////////////////////
//
MergeableArray::MergeableArray( DataTank& tank ):
	mTank(tank),
	mMaxSize(tank.getConf().mergableArrayMaxItemSize) {
	mMaxSize = MAX( mMaxSize, 1);
	mArray.reserve( mMaxSize + 1 );
}

MergeableArray::MergeableArray( const MergeableArray& rhs ):
	mTank(rhs.mTank),
	mArray(rhs.mArray),
	mMaxSize(rhs.mMaxSize){
	mMaxSize = MAX( mMaxSize, 1);
}

MergeableArray& MergeableArray::operator=( const MergeableArray& rhs ) {
	mMaxSize = rhs.mMaxSize;
	mArray = rhs.mArray;
	return *this;
}

MergeableArray::~MergeableArray() {
	mArray.clear();
}

bool MergeableArray::intersection( const Range& a, const Range& b ) const {
	size_t size = a.begin > b.begin ? b.size : a.size;
	unsigned long long left =  a.begin > b.begin ? b.begin : a.begin;
	unsigned long long right = a.begin < b.begin ? b.begin : a.begin;
	return ( left + size ) >= right;
}

size_t MergeableArray::add( const Range& touch ) {
	static size_t readBlockSize = mTank.getConf().readCacheBlockSize;

// outter code will check the correction of touchArea
	size_t blockCount = touch.size / readBlockSize;
	if( mArray.size() == 0 ) {
		HitStat hs;
		hs.lastAccess = touch;
		hs.hitCount = blockCount - 1;
		mArray.push_back( hs );
		return hs.hitCount;
	}

	HitStat* low = firstItem( );
	HitStat* high = lastItem( );
	HitStat* p = low;

	int hit = 0;// 0 for inserting before it
				// 1 for inserting after it
				// 2 for contained by it
	if( *low > touch ) {
		hit = 0;
		p = low;
	} else if ( *high < touch ) {
		hit = 1;
		p = high;
	} else {
		do {
			p = low + (high-low)/2;
			if( *p < touch ) {
				hit = 1;
				low = p;
			} else if( *p > touch ) {
				hit = 0;
				high = p;
			} else {
				hit = 2;
				break;
			}
		} while( low + 1 < high );
	}
	size_t hitCount = 0;
	if( hit == 2) {
		hitCount = ++(p->hitCount);
		return hitCount;
	}
	
	if( !intersection( p, touch ) ) {
		HITSTATARRAY::iterator it = mArray.begin() + (p - firstItem());
		if( hit == 1) {
			it ++;
		}
		HitStat hs;
		hs.lastAccess = touch;
		hs.hitCount= blockCount - 1;
		HITSTATARRAY::iterator itP = mArray.insert( it ,hs );
		p = firstItem() + ( itP - mArray.begin() );
	} else {
		unsigned long long lastEnd = p->lastAccess.end();
		p->lastAccess.begin = MIN(p->lastAccess.begin, touch.begin );
		p->lastAccess.size = MAX( lastEnd, touch.end() ) - p->lastAccess.begin;
		p->hitCount += blockCount;
	}

	return tryMergeWithNeighbor( p );
}

size_t MergeableArray::tryMergeWithNeighbor( HitStat* p  ) {
	assert( p >= firstItem() && p <= lastItem() );

	static size_t readBlockSize = mTank.getConf().readCacheBlockSize;
	// merge right
	while( true ) {
		HitStat* q = p + 1;
		if( q > lastItem() )
			break;
		if( !intersection( p, q ) )
			break;
		p = merge( p, q );
	}

	// merge left
	while( true ) {
		HitStat* q = p - 1;
		if( q < firstItem())
			break;
		if( !intersection( p, q ) ) 
			break;
		p = merge( q, p);
	}

	if( p->lastAccess.size / readBlockSize > 3 ) {
		unsigned long long end = p->lastAccess.end();
		p->lastAccess.begin = end - readBlockSize*3;
		p->lastAccess.size = readBlockSize*3;
	}

	size_t hitCount = p->hitCount;

	if( mArray.size() > mMaxSize ) {
		if( p == firstItem() ) {
			mArray.erase( mArray.begin() + 1 );
		} else {
			mArray.erase( mArray.begin() );
		}
	}

	return hitCount;
}

MergeableArray::HitStat* MergeableArray::merge( HitStat* to, const HitStat* from ) {
	assert( to < from );
	unsigned long long end = to->lastAccess.end();
	to->lastAccess.begin = MIN( to->lastAccess.begin, from->lastAccess.begin );
	to->lastAccess.size = MAX(end, from->lastAccess.end()) - to->lastAccess.begin;
	to->hitCount += from->hitCount + 1;

	HITSTATARRAY::iterator it = mArray.begin() + ( from - firstItem() );
	mArray.erase( it );
	return to; // to < from, so it's safe to return to directly
}

///////////////////////////////////////////////////////////////
AvgArray::AvgArray(size_t itemCount):
	maxItems( itemCount ),
	pos(0),
	avgBitrate(0) {
		bitrates.reserve(maxItems);
}

int64 AvgArray::add( int64 bitrate ) {
	if( bitrates.size() >= maxItems ) {
		avgBitrate = (avgBitrate * bitrates.size() + bitrate - bitrates[pos]) / ( bitrates.size() + 1 );
		bitrates[pos] = bitrate;
		if (++pos >= bitrates.size() )
			pos = 0;
	} else {
		avgBitrate = (avgBitrate * bitrates.size() + bitrate) / (bitrates.size() + 1);
		bitrates.push_back(bitrate);
	}
	return get();
}

int64 AvgArray::get( ) const {
	if( bitrates.size() < maxItems )
		return 0xFFFFFFFF;
	return avgBitrate;
}

///////////////////////////////////////////////////////////////
//
FlushRunnerCenter::FlushRunnerCenter( DataTank& tank ):
mTank(tank),
mFlushPool(mTank.getFlushThreadPool() ) ,
mbRunning(false){
	mOutstandingRunners=0;
}

FlushRunnerCenter::~FlushRunnerCenter( ) {
//let os reclaim the resource such as memory ?
}

bool FlushRunnerCenter::start() {
	const DataTankConf& conf = mTank.getConf();
	mbRunning = true;
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(FlushRunnerCenter,"start(), QueueBufferCount[%d/%d] MaxMergeCount[%d], writeQueueIdle[%d]ms"),
		(int)conf.maxWriteQueueBufferCount, (int)conf.minWriteQueueBufferCount,
		(int)conf.maxWriteQueueMergeItemCount,
		(int)conf.writeQueueIdleInMs);
	return ZQ::common::NativeThread::start();
}

void FlushRunnerCenter::stop() {
	mbRunning = false;
	waitHandle(-1);
}

int FlushRunnerCenter::run() {
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(FlushRunnerCenter,"FlushRunnerCenter is running"));
	static size_t minWriteQueueBufferCount = mTank.getConf().minWriteQueueBufferCount;
	static size_t maxWriteQueueBufferCount = mTank.getConf().maxWriteQueueBufferCount;
	static size_t maxWriteQueueMergeItemCount = mTank.getConf().maxWriteQueueMergeItemCount;
	static size_t flushPoolSize = mTank.getConf().flushThreadPoolSize;

	static size_t writeQueueIdleTime = mTank.getConf().writeQueueIdleInMs;
	writeQueueIdleTime = MAX(writeQueueIdleTime, 10);

	while(mbRunning) {
		
		ZQ::common::delay( writeQueueIdleTime/2 );

		{
			ZQ::common::MutexGuard gd(mLocker);
			if( mOutstandingRunners >= flushPoolSize ){
				//MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushRunnerCenter, "mOutstandingRunners[%d] flushPoolSize[%d]"), mOutstandingRunners, flushPoolSize);
				continue;
			}
			size_t delta = flushPoolSize - mOutstandingRunners;
			std::list<std::string>::iterator itFile = mFileQueue.begin();
			int64 timenow = ZQ::common::now();
			while( itFile != mFileQueue.end() && delta > 0 ) {
				const std::string& filePath = *itFile;
				FLUSHMAP::iterator itMap = mFlushMap.find(filePath);
				if( itMap == mFlushMap.end() ) {
					itFile = mFileQueue.erase(itFile);
				} else {
					FlushFileMeta& meta = itMap->second;
					bool shouldFlush = false;
					if( ( meta.lastTouch + (int64)writeQueueIdleTime ) <= timenow ) {
						shouldFlush = true;
					} 
					if( shouldFlush ) {	
						MLOG(ZQ::common::Log::L_INFO,CLOGFMT(FlushRunnerCenter,"run() make flush request due to: merged item count[%d/%d] queue buffer count[%d/%d/%d], lastTouch[%ld] now[%ld]"),
								(int)meta.mergedCount(), (int)maxWriteQueueMergeItemCount,
								(int)meta.mbis.size(), (int)maxWriteQueueBufferCount, (int)minWriteQueueBufferCount,
								meta.lastTouch, timenow);

						makeFlushRequest(meta.mbis, meta.indices );
						itFile = mFileQueue.erase( itFile );
						mFlushMap.erase(itMap);
						delta --;
					} else {
						itFile ++;
					}
				}
			}
		}
	}
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushRunnerCenter,"FlushRunnerCenter quiting"));
	return 0;
}

void FlushRunnerCenter::addInflightReq( const std::string& filepath ) {
	assert(!filepath.empty());
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string,InflightInfo>::iterator it = mInflightReqs.find(filepath);
	if( it == mInflightReqs.end() ) {
		InflightInfo info( mTank.getConf().writeAvgWinSizeForYield );
		info.reqs = 1;
		std::pair<std::map<std::string,InflightInfo>::iterator, bool >  ret = mInflightReqs.insert( std::map<std::string,InflightInfo>::value_type(filepath,info) );
		assert( ret.second );//insert can not fail here
	} else {
		it->second.reqs ++;
		assert( it->second.reqs > 1 );
	}
}

void FlushRunnerCenter::removeInflightReq( const std::string& filepath, int64 bitrate ) {
	assert(!filepath.empty());
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string, InflightInfo>::iterator it = mInflightReqs.find(filepath);
	if( it == mInflightReqs.end() ){
		//assert(false); //to satisfy smoke test, comment this line
	} else {
		InflightInfo& info = it->second;
		info.reqs--;
		assert( info.reqs>= 0 );
		if( info.reqs == 0 ) {
			mInflightReqs.erase(it);
			return;
		}
		info.avgBitrate.add(bitrate);
	}
}

size_t FlushRunnerCenter::getInflightRequests( const std::string& filename, int64& avgBitrate ) const {
	ZQ::common::MutexGuard gd(mLocker); 
	std::map<std::string,InflightInfo>::const_iterator it = mInflightReqs.find( filename );
	if( it == mInflightReqs.end() )
		return 0;
	assert( it->second.reqs >= 0 );
	avgBitrate = it->second.avgBitrate.get();
	return (size_t)it->second.reqs;
}

void FlushRunnerCenter::post( MemoryBlockInfo* mbi, int index ) {
	std::vector<MemoryBlockInfo*> mbis;
	mbis.push_back( mbi );
	std::vector<int> indices;
	indices.push_back(index);
	post( mbis, indices );
}

void FlushRunnerCenter::post( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& indices) {
	assert( mbis.size() > 0 && mbis.size() == indices.size() );
	//static size_t flushPoolSize = mTank.getConf().flushThreadPoolSize;
	{
		ZQ::common::MutexGuard gd( mLocker );
		//if( mOutstandingRunners >= flushPoolSize ) {
			tryToMergeFlushRequest( mbis, indices );
		//} else {
		//	makeFlushRequest( mbis, indices );
		//}
	}
}

void FlushRunnerCenter::makeFlushRequest( MemoryBlockInfo* mbi, int index ) {

	std::vector<MemoryBlockInfo*> mbis;
	mbis.push_back( mbi );
	std::vector<int> indices;
	indices.push_back( index );
	makeFlushRequest( mbis, indices );
}

void FlushRunnerCenter::makeFlushRequest( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int> indices ) {
	assert( mbis.size() > 0 );
	const std::string& filePath = mbis[0]->md().filePath;
	assert(!filePath.empty());
	ZQ::common::MutexGuard gd(mLocker);
	addInflightReq(filePath);
	mOutstandingRunners ++;
	( new FlushRunnerRequest( *this, mTank, mbis, indices) )->start();
}

void FlushRunnerCenter::FlushFileMeta::add( MemoryBlockInfo* mbi, int index ) {
	Range touchRange(mbi->dataPosOfFile(), mbi->dataSize() );
	if( mergedData.size() == 0 ) {
		mbis.push_back(mbi);
		indices.push_back(index);
		MergedInfo mi; 
		mi.range = touchRange;
		mi.mbiCount = 1;
		mergedData.push_back(mi);
		return;
	}
	size_t pos = 0;
	std::vector<MergedInfo>::iterator it = mergedData.begin();
	bool merged = false;
	while( it != mergedData.end() ) {
		//const Range& touchRange = mbi->md().dataRange;
		if( intersectOrAdjacent( it->range, touchRange ) ){
			if( it->range > touchRange ) {
				unsigned long long end = it->range.end();
				it->range.begin = touchRange.start();
				it->range.size = end - it->range.start();
			} else {
				it->range.size = touchRange.end() - it->range.start();
				pos += it->mbiCount;
			}
			it->mbiCount++;
			merged = true;
			break;
		} else if( it->range > touchRange ) {
			break;
		}

		pos += it->mbiCount;
		it++;
	}
	if( !merged ) {
		MergedInfo mi;
		mi.range = touchRange;
		mi.mbiCount = 1;
		it = mergedData.insert( it, mi);
	}
	mbis.insert( mbis.begin() + pos , mbi );
	indices.insert( indices.begin() + pos , index );

	if( it != mergedData.begin() ) {
		std::vector<MergedInfo>::iterator itPrev = it - 1;
		if( itPrev->range.end() == it->range.start() ){
			itPrev->range.size = it->range.end() - itPrev->range.start();
			itPrev->mbiCount += it->mbiCount;
			pos = itPrev - mergedData.begin();
			mergedData.erase( it );
			it = mergedData.begin() + pos;
		}
	} 

	if( (it + 1) != mergedData.end() ) {
		std::vector<MergedInfo>::iterator itNext = it + 1;
		if( it->range.end() == itNext->range.start() ) {
			it->range.size = itNext->range.end() - it->range.start();
			it->mbiCount += itNext->mbiCount;
			pos = it - mergedData.begin();
			mergedData.erase(itNext);
			it = mergedData.begin() + pos;
		}
	}
}

size_t FlushRunnerCenter::FlushFileMeta::mergedCount() const {
	return mergedData.size();
}


void FlushRunnerCenter::tryToMergeFlushRequest( MemoryBlockInfo* mbi, int index ) {
	std::vector<MemoryBlockInfo*> mbis;
	mbis.push_back( mbi );
	std::vector<int> indices;
	indices.push_back(index);
	tryToMergeFlushRequest( mbis, indices );
}

void FlushRunnerCenter::tryToMergeFlushRequest( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& indices ) {
	assert( mbis.size() > 0 && indices.size() == mbis.size() );
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushRunnerCenter, "tryToMergeFlushRequest() file[%s]"), mbis[0]->toString().c_str());

	static size_t minWriteQueueBufferCount = mTank.getConf().minWriteQueueBufferCount;
	static size_t maxWriteQueueBufferCount = mTank.getConf().maxWriteQueueBufferCount;
	static size_t maxWriteQueueMergeItemCount = mTank.getConf().maxWriteQueueMergeItemCount;
	static size_t flushPoolSize = mTank.getConf().flushThreadPoolSize;
	//locker is held by outter function, so we are in thread safe  code
	const std::string& filePath = mbis[0]->md().filePath;
	FLUSHMAP::iterator it = mFlushMap.find( filePath );
	if( it == mFlushMap.end() ) {
		FlushFileMeta meta(mTank, mFileQueue.begin());
		for(size_t i = 0; i < mbis.size() ; i ++ ) {
			meta.add(mbis[i],indices[i]);
		}
		meta.lastTouch = ZQ::common::now();
		if( mOutstandingRunners < flushPoolSize && mbis.size() >= minWriteQueueBufferCount ) {
			makeFlushRequest(meta.mbis, meta.indices);
		} else {
			mFileQueue.push_front( filePath );
			meta.itFileQueue = mFileQueue.begin();
			mFlushMap.insert( FLUSHMAP::value_type(filePath, meta) );
		}
	} else {
		FlushFileMeta& meta = it->second;

		for(size_t i = 0; i < mbis.size() ; i ++ ) {
			meta.add(mbis[i],indices[i]);
		}
		meta.lastTouch = ZQ::common::now();
	
		if( ( meta.mergedCount() >= mTank.getConf().maxWriteQueueMergeItemCount ) ||
			( meta.mbis.size() >= maxWriteQueueBufferCount  ) ||
			( mOutstandingRunners < flushPoolSize && meta.mbis.size() >= minWriteQueueBufferCount ) ) {
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(FlushRunnerCenter,"make flush request due to: merged item count[%d/%d] queue buffer count[%d/%d/%d]"),
					(int)meta.mergedCount(), (int)maxWriteQueueMergeItemCount,
					(int)meta.mbis.size(), (int)maxWriteQueueBufferCount, (int)minWriteQueueBufferCount );
			
			makeFlushRequest( meta.mbis, meta.indices);
			
			mFileQueue.erase( meta.itFileQueue );
			mFlushMap.erase(it);
		}
	}
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(FlushRunnerCenter, "end of tryToMergeFlushRequest() file[%s]"), mbis[0]->toString().c_str());
}

void FlushRunnerCenter::onRunnerComplete( const std::vector<MemoryBlockInfo*>& mbis, const std::vector<int>& indices, int64 bitrate) {
	assert( mbis.size() == indices.size() );
	{
		ZQ::common::MutexGuard gd(mLocker); 
		assert( mOutstandingRunners > 0 );
		mOutstandingRunners --;
		if(mbis.size() > 0 ) {
			removeInflightReq( mbis[0]->md().filePath, bitrate );
		}
		if( !mFileQueue.empty() ) {
			std::string filepath = mFileQueue.back();
			FLUSHMAP::iterator it = mFlushMap.find( filepath );
			assert( it != mFlushMap.end() );
			if( it != mFlushMap.end() )  {
				FlushFileMeta& meta = it->second;
				assert( meta.mbis[0]->md().filePath == filepath );
				static size_t minWriteQueueBufferCount = mTank.getConf().minWriteQueueBufferCount;
				if( meta.mbis.size() > minWriteQueueBufferCount ) {
					makeFlushRequest( meta.mbis, meta.indices);
					mFlushMap.erase(it);
					mFileQueue.pop_back();
				}
			}
		}
	}
	mTank.onFlushComplete( mbis, indices );
}

std::string dataBuffersToRangeStr( const std::vector<DataBuffer>& bufs, int* mergedCount ) {
	if( bufs.empty())
		return std::string("");
	std::ostringstream ossRange;
	std::vector<CacheLayer::DataBuffer>::const_iterator itBuf = bufs.begin();

	//bufs.size() >= 1, so this is ok to take bufs.begin()'s value
	unsigned long long begin = itBuf->offset;
	unsigned long long end = begin + itBuf->size;
	itBuf++;

	bool 	 adjacent= true;
	int count = 0 ;
	do {
		if( itBuf == bufs.end())
			break;
		adjacent = end == itBuf->offset;
		if( !adjacent) {
			ossRange<<begin<<"-"<<end-1;
			if( itBuf != bufs.end() )
				ossRange<<";";
			adjacent = true;
			begin = itBuf->offset;
			end = begin + itBuf->size;
			count++;
		} else {
			end = itBuf->offset + itBuf->size;
		}
		itBuf ++;		
	} while(itBuf != bufs.end() );
	ossRange << begin <<"-" << end-1;
	if(mergedCount)
		*mergedCount = count;
	return ossRange.str();
}
}//namespace CacheZone

