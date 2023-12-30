
#ifndef _tianshan_cdnss_c2streamer_c2streamerservice_header_file_h__
#define _tianshan_cdnss_c2streamer_c2streamerservice_header_file_h__

#include <ZQ_common_conf.h>
#undef max
#undef min
#include <boost/thread.hpp>

#include <set>
#include <map>

#include <Locks.h>
#include <NativeThreadPool.h>
#include <DataPostHouse/common_define.h>
#include <LRUMap.h>

#include "../IdxFileParserEnvironment.h"
#include "../IndexFileParser.h"


#include "C2SessionManager.h"
#include "PortManager.h"
#include "C2StreamerLib.h"
#include "C2EventUpdater.h"
#include "C2TunnelBwmon.h"
#include "AioFile.h"
#include "dummyhls.h"

class C2ThreadPool;
class C2ThreadRequest
{
public:
	C2ThreadRequest( C2ThreadPool& pool );
	virtual ~C2ThreadRequest();

	virtual bool		init( ){ return true; }
	virtual int			run( ) = 0;
	virtual void		final(int retcode =0, bool bCancelled =false){}
	void				start( );

private:
	C2ThreadPool&	mPool;
};


class C2SlaveThread: public ZQ::common::NativeThread
{
public:
	C2SlaveThread( C2ThreadPool& pool );
	virtual ~C2SlaveThread();

public:

	void	comesNewRequest( C2ThreadRequest* req = NULL );
	int		run( );
	void	stop( );

private:
	C2ThreadPool&			mPool;
	ZQ::common::Semaphore	mSem;
	C2ThreadRequest*		mRequest;
	bool					mbQuit;
};

class C2ThreadPool : public ZQ::common::NativeThreadPool
{
public:
	C2ThreadPool( size_t size = 10 );
	virtual ~C2ThreadPool();

	size_t				size() const;
	size_t				pendingRequestSize() const;
	size_t				activeCount() const;
	void				stop( );
protected:
	friend class C2SlaveThread;
	friend class C2ThreadRequest;
	void				postRequest( C2ThreadRequest* req );
	C2ThreadRequest*	getRequest(C2SlaveThread* t);
private:

	std::vector<C2SlaveThread*>		mAllThreads;
	std::list<C2SlaveThread*>		mIdleThreads;
	std::list<C2ThreadRequest*>		mRequests;
	ZQ::common::Mutex				mLocker;
	bool							mbQuit;
};


namespace C2Streamer
{


class C2StreamerEnv;
class C2Session;

typedef ZQ::DataPostHouse::ObjectHandle<C2Session> C2SessionPtr;

//class TaskRunner : public ZQ::common::ThreadRequest
class TaskRunner : public C2ThreadRequest
{
public:
	TaskRunner( C2StreamerEnv& env , C2SessionPtr sess, C2ThreadPool& pool );
	virtual ~TaskRunner();

protected:

	int			run( );
	void		final(int retcode =0, bool bCancelled =false);	

private:
	C2StreamerEnv&			mEnv;
	C2SessionPtr			mSess;
};


class C2IndexRecordCenter {
public:
	C2IndexRecordCenter( C2StreamerEnv& env);

	virtual ~C2IndexRecordCenter();
	
	class IndexRecordWrapper: public ZQ::IdxParser::IndexRecord, public ZQ::DataPostHouse::SharedObject {
	public:
		IndexRecordWrapper( C2StreamerEnv& env, const std::string& pathname)
			:mEnv(env),
			mbParsed(false),
			mFilePathname(pathname)	{
		}

		virtual ~IndexRecordWrapper() {
		}

		const std::string& filename() const {
			return mFilePathname;
		}

		//call wait before  findNextIFrame
		void	wait(){ 
			boost::unique_lock<boost::mutex> lock(mLocker);
			if(mbParsed)
				return;
			mCond.wait(lock);
		}

		void	reset( ) { mbParsed = false; }

		bool	parse( );

	private:
		C2StreamerEnv&					mEnv;
		boost::mutex					mLocker;
		boost::condition_variable		mCond;
		bool							mbParsed;
		std::string						mFilePathname;
	};

	typedef ZQ::DataPostHouse::ObjectHandle<IndexRecordWrapper> IndexRecordWrapperPtr;

	IndexRecordWrapperPtr	get( const std::string& indexFileName );

	void					tryParsing( const std::string& indexFileName, bool reParse = false );

private:
	void					parsed( const std::string& indexFileName);

	class IndexParserRunner : public C2ThreadRequest {
	public:
		IndexParserRunner( C2StreamerEnv& env, C2IndexRecordCenter& center, IndexRecordWrapperPtr record )
		:C2ThreadRequest( *(C2ThreadPool*)&env.getThreadPool()),
		mCenter(center),
		mRecord(record) {
		}
		virtual ~IndexParserRunner() {
		}
	protected:
		int run() {
			mRecord->parse();
			return 0;
		}
		void final( int, bool) {
			mCenter.parsed( mRecord->filename() );
			delete this;
		}
	private:
		C2IndexRecordCenter&	mCenter;
		IndexRecordWrapperPtr 	mRecord;
	};

	struct IndexRecordInfo {
		bool		parsing;// is this record being parsed?
		IndexRecordWrapperPtr record;
		IndexRecordInfo():parsing(false){
		}
	};
	C2StreamerEnv&			mEnv;
	typedef ZQ::common::LRUMap<std::string, IndexRecordInfo> INDEXRECORDMAP;
	INDEXRECORDMAP*			mRecords;
	ZQ::common::Mutex		mLocker;
};

class C2Service : public ZQ::common::NativeThread
{
public:
	C2Service( C2StreamerEnv& env );
	virtual ~C2Service();
	
public:
	
	bool						startService( );
	
	void						stopService( );
	
	inline C2SessionManager&	getSessManager();
	
	inline C2ClientManager&		getClientManager();
	
	inline PortManager&			getPortManager();

	inline C2EventUpdater&		getEventPublisher();

	inline C2IpConnTackMon&		getC2TunnelMon();

	inline IptablesRule&		getIptablesRule();

	inline HLSServer&			getHlsServer();

	inline CacheCenter&			getCacheCenter();

	inline C2IndexRecordCenter&	getIndexRecordCenter();
	
	void						watchSession( C2SessionPtr sess , uint64 interval );

	void						unwatchSession( C2SessionPtr sess );

	bool						getSessionStatus( const SessionStatusRequestParamPtr& request , SessionStatusResponseParamPtr& response ) const;

	bool						getResourceStatus( const ResourceStatusRequestParamPtr& request , ResourceStatusResponseParamPtr& response ) const;


protected:

	int							run( );
	
	void						detectIoBlockSize();
	
private:
	
	C2StreamerEnv&				mEnv;
	PortManager					mPortManager;
	C2SessionManager			mSessManager;
	C2ClientManager				mClientManager;
	C2EventUpdater				mEventPublisher;
	C2IpConnTackMon				mC2TunnelMon;
	IptablesRule				mIptables;
	HLSServer					mHlsServer;
	CacheCenter					mCacheCenter;
	C2IndexRecordCenter 		mIndexRecordCenter;
	struct WatchKey 
	{
		C2SessionPtr	sess;
		uint64			targetTime;
		bool operator < ( const WatchKey& key ) const;		
	};

	typedef std::set< WatchKey > WATCHSET;

	typedef std::map< std::string , WatchKey > WATCHKEYMAP;		 
	///               session id , watchKey
	
	WATCHSET				mWatchSet;	
	WATCHKEYMAP				mKeyMap;
	ZQ::common::Mutex		mMutex;
	ZQ::common::Semaphore	mSema;
	bool					mbQuit;
	uint64					mNextWakeup;
};

C2SessionManager& C2Service::getSessManager()
{
	return mSessManager;
}

C2ClientManager& C2Service::getClientManager()
{
	return mClientManager;
}

PortManager& C2Service::getPortManager()
{
	return mPortManager;
}

C2EventUpdater& C2Service::getEventPublisher()
{
	return mEventPublisher;
}

C2IpConnTackMon& C2Service::getC2TunnelMon()
{
	return mC2TunnelMon;
}

IptablesRule& C2Service::getIptablesRule()
{
	return mIptables;
}

HLSServer&	C2Service::getHlsServer()
{
	return mHlsServer;
}

CacheCenter& C2Service::getCacheCenter( ) 
{
	return mCacheCenter;
}

C2IndexRecordCenter& C2Service::getIndexRecordCenter() {
	return mIndexRecordCenter;
}

extern C2Service* getC2StreamerService();


}//namespace C2Streamer



#endif//_tianshan_cdnss_c2streamer_c2streamerservice_header_file_h__
