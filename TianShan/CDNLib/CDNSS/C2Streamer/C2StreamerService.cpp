
#include <ZQ_common_conf.h>
#include <SystemUtils.h>
#include <TimeUtil.h>
#include "C2StreamerEnv.h"
#include "C2SessionHelper.h"
#include "C2StreamerService.h"
#include "C2Session.h"
#include <malloc.h>

#ifdef ZQ_OS_LINUX
	#include <sys/resource.h>
#endif


////////////////////////////////////////////////////////////
//C2ThreadRequest
C2ThreadRequest::C2ThreadRequest( C2ThreadPool& pool )
:mPool(pool)
{
}
C2ThreadRequest::~C2ThreadRequest()
{
}
void C2ThreadRequest::start( )
{
	mPool.postRequest(this);
}
C2SlaveThread::C2SlaveThread( C2ThreadPool& pool )
:mPool(pool),
mRequest(NULL),
mbQuit(false)
{
}
C2SlaveThread::~C2SlaveThread()
{
}

void C2SlaveThread::stop()
{
	mbQuit = true;
	mSem.post();
	waitHandle( -1 );
}

int C2SlaveThread::run( )
{
	while(!mbQuit)
	{
		mSem.wait();
		do
		{
			if(mbQuit) break;
			if( !mRequest ) continue;
			try
			{
				if(!mRequest->init())
				{
					mRequest->final(0,true);
				}
				else
				{
					int retCode = mRequest->run();
					mRequest->final( retCode , false );
				}
			}
			catch(...)
			{
			}
			mRequest = NULL;
			mRequest = mPool.getRequest( this );
		}while(mRequest != NULL);
	}
	return 0;
}

void C2SlaveThread::comesNewRequest( C2ThreadRequest* req )
{	
	mRequest = req;
	if(req == NULL )
	{
		mbQuit = true;
	}
	mSem.post();
}

////////////////////////////////////////////////////////////////////
///C2ThreadPool
C2ThreadPool::C2ThreadPool( size_t size )
:ZQ::common::NativeThreadPool(1),
mbQuit(false)
{
	if(size < 3)	size = 3;
	for( int i = 0 ;i < (int)size ; i++ )
	{
		C2SlaveThread* t = new C2SlaveThread(*this);
		mAllThreads.push_back(t);
		mIdleThreads.push_back(t);
		t->start();
	}
}
C2ThreadPool::~C2ThreadPool()
{
	stop( );
}
void C2ThreadPool::stop( )
{
	ZQ::common::MutexGuard gd(mLocker);
	mbQuit = true;
	std::vector<C2SlaveThread*>::iterator it = mAllThreads.begin();
	for( ; it != mAllThreads.end() ; it ++ )
	{
		C2SlaveThread* t = *it;
		if(!t) continue;
		t->stop();
		delete t;
	}
	mAllThreads.clear();
	mIdleThreads.clear();
}

size_t C2ThreadPool::size() const
{
	ZQ::common::MutexGuard gd(mLocker);
	return mAllThreads.size();
}

size_t C2ThreadPool::pendingRequestSize() const
{
	ZQ::common::MutexGuard gd(mLocker);
	return mRequests.size();
}
size_t C2ThreadPool::activeCount() const
{
	ZQ::common::MutexGuard gd(mLocker);
	return mAllThreads.size() - mIdleThreads.size();
}
C2ThreadRequest* C2ThreadPool::getRequest(C2SlaveThread * t)
{
	C2ThreadRequest* r = NULL;
	{
		ZQ::common::MutexGuard gd(mLocker);
		if(mRequests.size() > 0 )
		{
			r = mRequests.front();
			mRequests.pop_front();
		}
		else
		{
			mIdleThreads.push_back(t);
		}
	}
	return r;
}
void C2ThreadPool::postRequest( C2ThreadRequest* req )
{	
	ZQ::common::MutexGuard gd(mLocker);
	if(mbQuit)	return;
	mRequests.push_back(req);
	C2ThreadRequest* r = mRequests.front();
	if( mIdleThreads.size() > 0 )
	{
		C2SlaveThread* t = mIdleThreads.front();
		mIdleThreads.pop_front();
		mRequests.pop_front();
		t->comesNewRequest(r);
	}
}

namespace C2Streamer
{

C2Service::C2Service(C2StreamerEnv& env )
:mEnv(env),
mPortManager(env,*this),
mSessManager(env,*this),
mClientManager(env),
mEventPublisher(env),
mC2TunnelMon(env),
mIptables(env,*this),
mHlsServer(env,*this),
mCacheCenter( env ),
mIndexRecordCenter(env),
mbQuit(false)
{
}

C2Service::~C2Service()
{
}
bool C2Service::WatchKey::operator < ( const WatchKey& key ) const
{
	if( targetTime < key.targetTime )
	{
		return true;
	}
	else if( targetTime == key.targetTime )
	{
		return sess->getSessId() < key.sess->getSessId();
	}
	else
	{
		return false;
	}
}
void C2Service::watchSession( C2SessionPtr sess , uint64 interval )
{
	uint64 target = ZQ::common::now() + interval;
	bool bSignal = false;

	WatchKey key;
	key.sess		= sess;
	key.targetTime	= target;
	std::string	sessId = sess->getSessId();
	
	//MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(WatchDog,"watch session[%s] on time[%s]"), sessId.c_str() , timeToString((int64)target).c_str() );
	
	{
		ZQ::common::MutexGuard gd(mMutex);
		if( target <= mNextWakeup )
		{
			bSignal = true;
		}
		
		WATCHKEYMAP::iterator itMap = mKeyMap.find( sess->getSessId() );
		if( itMap != mKeyMap.end() )
		{
			mWatchSet.erase( itMap->second );
		}
		mKeyMap[ sess->getSessId() ] = key;
		mWatchSet.insert( key );
	}

	if( bSignal )
		mSema.post();
}
 
void C2Service::unwatchSession( C2SessionPtr sess )
{
	ZQ::common::MutexGuard gd(mMutex);
	WATCHKEYMAP::iterator it = mKeyMap.find( sess->getSessId() );
	if( it != mKeyMap.end() )
	{
		mWatchSet.erase( it->second );
		mKeyMap.erase( it );
	}
}

void C2Service::stopService( )
{
	((C2ThreadPool*)&mEnv.getThreadPool())->stop();

	mPortManager.stop();

	if( !mEnv.getConfig().mAquaRootUrl.empty() ) {
		mHlsServer.uninit();
	} else {
		mSessManager.stop();
		mC2TunnelMon.stop();
		mIptables.stop();
	}

	mCacheCenter.stop();

	mbQuit = true;
	mSema.post();
	waitHandle( 100 * 1000 );	
	mEventPublisher.stop();
}

bool C2Service::startService( )
{
	//FIXME : not finished
	if(!mPortManager.start())
		return false;
	
	if( !mEnv.getConfig().mAquaRootUrl.empty() ) {
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2Service,"working as HLS server, disable EventPuboisher and C2TunnelMonitor"));
		if(!mHlsServer.init())
			return false;
	} else {
		if(!mEventPublisher.start())
			return false;
		if(!mC2TunnelMon.start())
			return false;

		mIptables.start();	
	}

	mCacheCenter.start(mEnv.getConfig().mCacheBufferSize, mEnv.getConfig().mCacheDefaultBufferCount, mEnv.getConfig().mCacheDeltaBufferCount );

	detectIoBlockSize();

#ifdef ZQ_OS_LINUX
	struct rlimit rlim;
	rlim.rlim_cur = 64*1024;
	rlim.rlim_max = 640*1024;
	setrlimit( RLIMIT_NOFILE, &rlim);
#endif

	uint32 bufferSize = mEnv.getConfig().ioBlockSize * mEnv.getConfig().readBufferCount / 2;
	bufferSize = MIN(bufferSize,(128*1024)) ;
	bufferSize = MAX(bufferSize,(16*1024)) ;
	mallopt(M_MMAP_THRESHOLD,bufferSize);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(C2Service,"startService() set mmap threshold to[%u]"),bufferSize);

	return start();
}

void C2Service::detectIoBlockSize()
{
	std::string docRootPath = mEnv.getDocumentRootFolder();
	if( docRootPath.empty() )
		return;
	struct stat st;
	if( stat( docRootPath.c_str() , &st ) < 0 )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2Service,"failed to get file system block size for doc root[%s] due to[%s]"),
			 docRootPath.c_str() , SYS::getErrorMessage().c_str() );
	}
	else
	{
		mEnv.mConfig.ioBlockSize = st.st_blksize;
		if( mEnv.mConfig.ioBlockSize <= 0 )
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2Service,"got [%u] for doc root[%s], set it to [%d]"),
				 mEnv.mConfig.ioBlockSize , docRootPath.c_str() , 128 * 1024);
			mEnv.mConfig.ioBlockSize = 128 * 1024;
		}
		else
		{
			MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2Service,"got block size[%u] for doc root[%s]"),
				mEnv.mConfig.ioBlockSize , docRootPath.c_str());
		}
	}
}

int C2Service::run( )
{
	uint64	defaultInterval = 200;
	uint32	waitInterval;
	C2ThreadPool* pool = (C2ThreadPool*)(&mEnv.getThreadPool());
	do
	{
		C2SessionPtr sess = NULL;
		while(!mbQuit)
		{
			{				
				sess	= NULL;
				ZQ::common::MutexGuard gd(mMutex);
				
				uint64 curTime = ZQ::common::now();

				if( mWatchSet.size() <= 0 )
				{
					waitInterval = defaultInterval;
					break;
				}
				else
				{
					WATCHSET::iterator it = mWatchSet.begin();
					if( it->targetTime <= curTime )
					{
						sess = it->sess;
						mKeyMap.erase( sess->getSessId() );
						mWatchSet.erase( it );
					}
					else
					{
						waitInterval = it->targetTime - curTime;
						break;
					}
				}
			}

			if( sess )
			{
				static int totalThreadSize = pool->size();
				int pendingRequest 	= pool->pendingRequestSize();
				int activeCount 	= pool->activeCount();
				
				if( pendingRequest > (int)mEnv.getConfig().mPendingSizeThreshold )	
					MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(WatchDog, "threadpool[%d/%d] busy: pendingRequest[%d]"), activeCount, totalThreadSize, pendingRequest);
				
				( new TaskRunner(mEnv,sess,*pool) )->start();
			}
		}
		
		if( mbQuit ) break;

		{
			ZQ::common::MutexGuard gd(mMutex);
			waitInterval = MAX( 2 , waitInterval );
			mNextWakeup = ZQ::common::now() + waitInterval;			
		}
		mSema.timedWait( waitInterval );
	}while( !mbQuit );
	return 0;
}

void addSessionStatusRecord( C2SessionPtr sess , SessionStatusResponseParamPtr& response )
{
	AggregateStatisticsParam& stats			= response->statistics;
	std::vector<SessionStatusInfo>& infos	= response->sessionInfos;
	//FIXME: not finished yet
	SessionStatusInfo info;
	info.bytesTransfered 		= sess->getBytesTransfered();
	info.clientTransfer			= sess->getTransferClientAddress();
	info.fileName				= sess->getFileName();
	info.sessionState			= sess->getState();
	info.timeInState			= sess->getTimeInState();
	info.transferAddress		= sess->getTransferServerAddress();
	info.transferId				= sess->getSessId();
	info.transferPortName		= sess->getTransferPortName();
	info.transferRate			= sess->getTransferRate();
	
	switch( info.sessionState )
	{
	case SESSION_STATE_ACTIVE:
		stats.activeSessions ++;
	default:
		stats.idleSessions ++;
	}
	infos.push_back( info );
	///FIXME: fill up the rest parameter in AggregateStatisticsParam
}

void addClientSessionStatusRecord( const C2ClientManager::ClientAttr& client ,const C2SessionManager& sessManager , SessionStatusResponseParamPtr& response )
{
	C2ClientManager::SESSIONATTRS::const_iterator it = client.sessInfos.begin();
	for( ; it != client.sessInfos.end() ; it ++ )
	{
		C2SessionPtr sess = sessManager.findSession( it->first ) ;
		if( sess )
		{
			addSessionStatusRecord( sess , response );
		}
	}
}

bool C2Service::getSessionStatus( const SessionStatusRequestParamPtr& request , SessionStatusResponseParamPtr& response ) const
{
	if( request->clientTransfers.size() <= 0 )
	{	
		C2ClientManager::ClientMap clients;
		mClientManager.getClientsAttr( clients);
		C2ClientManager::ClientMap::const_iterator itClient = clients.begin();
		for( ; itClient != clients.end() ; itClient ++ )
		{
			addClientSessionStatusRecord( itClient->second , mSessManager , response );
		}
	}
	else
	{
		C2ClientManager::ClientAttr client;
		const std::vector<std::string>& clientIps = request->clientTransfers;
		std::vector<std::string>::const_iterator itClient = clientIps.begin();
		for( ; itClient != clientIps.end() ; itClient++ )
		{
			if( mClientManager.getClientAttr( *itClient , client) )
				addClientSessionStatusRecord( client , mSessManager , response );
		}
	}
	return true;
}

void addResourceStatus( const PortManager::PortAttr& attr , ResourceStatusResponseParamPtr& response )
{
	std::vector<ResourceStatusInfo>& portInfos = response->portInfos;
	ResourceStatusInfo info;
	
	mergeVector( info.portAddressIpv4 , attr.ipv4 );
	mergeVector( info.portAddressIpv6 , attr.ipv6 );
	
	info.activeBandwidth		= attr.usedBW;				//FIXME: include all sessions
	info.activeTransferCount	= attr.sessionInfos.size();	//FIXME: include all sessions in current implementation
	info.capacity				= attr.capacity;
	info.portName				= attr.portName;
	info.portState				= attr.bUp ? PORT_STATE_UP : PORT_STATE_DOWN;
	info.tcpPortNumber			= attr.tcpPortNumber ;
	portInfos.push_back( info );
}

bool C2Service::getResourceStatus( const ResourceStatusRequestParamPtr& request , ResourceStatusResponseParamPtr& response ) const
{
	const std::vector<std::string>& portNames = request->portNames;
	if( portNames.empty() )
	{
		PortManager::PORTS ports;
		mPortManager.getPortsAttr( ports );
		PortManager::PORTS::const_iterator itPort = ports.begin();
		
		for( ; itPort != ports.end() ; itPort ++ )
		{
			if ( !(itPort->second.bConf) )
				continue;
			addResourceStatus( itPort->second , response );
		}
	}
	else
	{
		std::vector<std::string>::const_iterator it = portNames.begin() ;
		for( ; it != portNames.end() ; it ++ )
		{
			PortManager::PortAttr attr;
			if( mPortManager.getPortAttr( *it, attr ) )
			{
				if(!attr.bConf)
					continue;
				addResourceStatus( attr , response );
			}
		}
	}
	return response->portInfos.size() > 0 ;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
TaskRunner::TaskRunner( C2StreamerEnv& env , C2SessionPtr sess, C2ThreadPool& pool )
:C2ThreadRequest(pool),
mEnv(env),
mSess(sess)
{
}
TaskRunner::~TaskRunner()
{

}
int TaskRunner::run()
{
	if( !mSess )
		return -1;

	try
	{
		mSess->onTimer();
	}
	catch( ... )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TaskRunner,"caught unknown exception when executing onTimer routine of C2Session"));
	}
	return 0;
}
void TaskRunner::final(int retcode , bool bCancelled )
{
	delete this;
}


///////////////////////////////////////////////////////////////////////

C2IndexRecordCenter::C2IndexRecordCenter( C2StreamerEnv& env)
:mEnv(env),
mRecords(NULL){
	mRecords = new INDEXRECORDMAP(env.getConfig().mIndexRecordCacheSize);
}

C2IndexRecordCenter::~C2IndexRecordCenter() {
	if(mRecords)
		delete mRecords;
}

C2IndexRecordCenter::IndexRecordWrapperPtr C2IndexRecordCenter::get( const std::string& indexFileName ) {
	IndexRecordWrapperPtr record = NULL;
	{
		ZQ::common::MutexGuard gd(mLocker);
		INDEXRECORDMAP::const_iterator it = mRecords->find( indexFileName );
		if( it == mRecords->end())
			return NULL;
		record = (*mRecords)[indexFileName].record;
	}
	if(!record)
		return NULL;
	record->wait();
	return record;
}

void C2IndexRecordCenter::parsed( const std::string& indexFileName) {
	ZQ::common::MutexGuard gd(mLocker);
	INDEXRECORDMAP::iterator it = mRecords->find( indexFileName );
	if( it == mRecords->end())
		return;
	(*mRecords)[indexFileName].parsing = false;
}

void C2IndexRecordCenter::tryParsing( const std::string& indexFileName, bool reparse ) {
	IndexRecordWrapperPtr record = NULL;
	{
		ZQ::common::MutexGuard gd(mLocker);
		INDEXRECORDMAP::iterator it = mRecords->find(indexFileName);
		if( it == mRecords->end() ) {
			IndexRecordInfo info;
			info.parsing = true;
			info.record = record =  new IndexRecordWrapper( mEnv, indexFileName);
			(*mRecords)[indexFileName] = info;
		} else {
			IndexRecordInfo& info = it->second;
			if( reparse && !info.parsing) {
				record = info.record;
			}
		}
	}
	if( !record)
		return;
	(new  IndexParserRunner( mEnv, *this, record))->start();
}

bool C2IndexRecordCenter::IndexRecordWrapper::parse( ) {
	reset();
	using namespace ZQ::IdxParser;
	IdxParserEnv env; env.AttchLogger( mEnv.getLogger() );
	IndexFileParser parser(env);
	
	StopWatch s;s.start();
	bool bOK= parser.ParseIndexRecordFromCommonFS(mFilePathname, *this, mFilePathname);
	s.stop();

	if(!bOK) {
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2IndexRecordCenter,"failed to parse index file[%s] cost[%llu]"),
				mFilePathname.c_str(), (unsigned long long)s.span() );
	} else {
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(C2IndexRecordCenter,"took [%llu]us to parse [%s]:filesize[%lld] subfiles[%d] records[%d] duration[%u]ms"),
				s.span(), mFilePathname.c_str(), (long long)indexFilesize(), (int)subfileCount(), (int)recordCount(), lastTimeOffset() );
	}
	
	mbParsed = true;
	mCond.notify_all();
	return bOK;
}

}//namespace C2Streamer
