#ifndef _tianshan_ngod_session_manager_header_file_h__
#define _tianshan_ngod_session_manager_header_file_h__

#include <map>
#include <set>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <NativeThreadPool.h>
#include <SystemUtils.h>

#include "StreamSmithModule.h"
#include "./Ice/ngod.h"
#include "NgodDatabase.h"
#include "NgodScheduler.h"
#include "SessionViewImpl.h"
#include "SelectionResourceManager.h"

namespace NGOD
{

class NgodEnv;
class NgodSessionManager;

class SessionFactory : public Ice::ObjectFactory
{
public:
	SessionFactory( NgodEnv& env , NgodSessionManager& manager );
	virtual ~SessionFactory();
	virtual Ice::ObjectPtr create(const std::string&);
	virtual void destroy();

private:
	NgodEnv&				mEnv;
	NgodSessionManager&		mSessManager;
};

typedef std::map< std::string , std::string >	SESSGROUPCONNMAP;//session group to connection Id map

class NgodReplicaListener : public TianShanIce::ReplicaSubscriber
{
public:
	NgodReplicaListener(NgodEnv& env);
	virtual ~NgodReplicaListener();
	void		updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& callback, const ::TianShanIce::Replicas&, const ::Ice::Current& = ::Ice::Current());

private:

	NgodEnv&	mEnv;

};

typedef IceUtil::Handle<NgodReplicaListener> NgodReplicaListenerPtr;


class RestoreThreadRequest : public ZQ::common::ThreadRequest
{
public:
	RestoreThreadRequest( NgodDatabase& db , NgodEnv& env, ZQ::common::NativeThreadPool& pool );
	virtual ~RestoreThreadRequest();
protected:
	int		run();
	void	final(int retcode /* =0 */, bool bCancelled /* =false */);
private:
	NgodDatabase&	mDb;
	NgodEnv&		mEnv;
};


class NgodTaskBase
{
public:
	NgodTaskBase( NgodEnv& env, NgodSessionManager & sessManager ):
	  mSessManager(sessManager),mEnv(env){}
	  virtual ~NgodTaskBase(){}
public:
	///execute the task by session manager thread
	void	execute( );

protected:

	///child class should implement this function
	virtual void	run() = 0;

protected:
	NgodSessionManager&		mSessManager;
	NgodEnv&				mEnv;
};

///这个类是用来执行一些不是非常紧急的事情，这些事情允许我们串行的执行
class NgodTaskRunner : public IResourceManagerEventSink
{
public:
	NgodTaskRunner(  NgodEnv& env,NgodSessionManager & sessManager );
	virtual ~NgodTaskRunner();
public:
	
	void	stop( );
	void	execute();

protected:
	
	virtual void	onStreamerEvent( const std::string& streamerNetId , bool bUp );

private:
	NgodSessionManager&		mSessManager;
	NgodEnv&				mEnv;
	ZQ::common::Semaphore	mSem;
	ZQ::common::Mutex		mLocker;
	bool					mbQuit;
	typedef std::list<NgodTaskBase*> TASKLIST;
	TASKLIST				mTasks;
};

class NgodSessionManager : public ZQ::common::NativeThread
{
public:
	
	NgodSessionManager( NgodEnv& env );

	virtual ~NgodSessionManager(void);

public:

	bool					start( const std::string& dbPath , int32 evictorSize ,IStreamSmithSite* pSite );

	void					stop( );

	bool					processRequest( IClientRequest* clireq );

	//create a session
	NGOD::NgodSessionPrx	creatSession( const std::string& odSessId , const std::string& url );

	bool					restoreRtspSession( const std::string& rtspSess , const std::string& url );

	void					destroySession( const std::string& sessId );

	void					destroyRtspSession( const std::string& sessId );

	NGOD::NgodSessionPrx	openSession( const std::string& sessId );

	std::string				getR2ConnectionId( const std::string& sessGroupName ) const;

	void					updateR2ConnectionId( const std::string& sessGroupName , const std::string& connectionId );
	
	NgodDatabase&			getDatabase( );

	void					watch( const std::string& sessId , Ice::Long timeout );

	void					unwatch( const std::string& sessId  );

	IServerRequest*			getServerRequest( const std::string& sessId , const std::string& connId );

	void					updateTimer( const std::string& sessId , int32 intreval );

	void					cancelTimer( const std::string& sessId );

	IStreamSmithSite*		getSite() { return mpSite; };

protected:

	void					restore();

	int						run( );

	void					initialize( );

private:
	IStreamSmithSite*		mpSite;
	
	NgodEnv&				mEnv;

	NgodDatabase			mDb;

	SESSGROUPCONNMAP		mSessGroupConnMap;
	ZQ::common::Mutex		mSessGroupConnMapLocker;

	NgodScheduler			mScheduler;
	NgodReplicaListenerPtr	mReplicaListener;
	SessionViewImplPtr		mSessionView;
	NgodTaskRunner			mTaskRunner;
};

struct StreamEventDispatchInfo 
{
	StreamEventRoutine	r;
	StreamEventAttr		a;
	int64				timestamp;
	StreamEventDispatchInfo()
	{
		timestamp = 0;
	}
};


class StreamEventDispatchRequest: public ZQ::common::ThreadRequest
{
public:
	StreamEventDispatchRequest(NgodEnv& env , NgodSessionManager& manager , StreamEventDispatchInfo& info, ZQ::common::NativeThreadPool& pool);		
	virtual ~StreamEventDispatchRequest();
protected:
	int		run();		
	void	final(int retcode =0, bool bCancelled =false);
private:
	NgodEnv&						mEnv ;
	NgodSessionManager&				mManager;
	StreamEventDispatchInfo			mEventInfo;
};


class StreamEventDispatcher
{
public:
	StreamEventDispatcher( NgodEnv& env , NgodSessionManager& manager );
	virtual ~StreamEventDispatcher();
	void start();
	void stop( );	
	

	void pushEvent( const StreamEventRoutine& r, const StreamEventAttr& a );
	
private:

	NgodEnv&				mEnv ;
	NgodSessionManager&		mManager;
	ZQ::common::NativeThreadPool mPool;
};

class EventSinkI 
{
public:
	EventSinkI( NgodEnv& env , NgodSessionManager& manager , StreamEventDispatcher& eventDispatcher );
	virtual ~EventSinkI();

protected:

	void sendEvent( const StreamEventRoutine& r, const StreamEventAttr& a ) const;

protected:
	NgodEnv&				mEnv;
	NgodSessionManager&		mSessManager;
	StreamEventDispatcher&	mDispatcher;
};

class StreamEventI : public EventSinkI , public TianShanIce::Streamer::StreamEventSink 
{
public:
	StreamEventI( NgodEnv& env , NgodSessionManager& manager, StreamEventDispatcher& eventDispatcher );
	virtual ~StreamEventI();
public:
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current()) const;
	void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic= ::Ice::Current()) const;
	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props, const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const TianShanIce::Properties& props, const ::Ice::Current& = ::Ice::Current()) const ;
	void OnExit(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnExit2(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const TianShanIce::Properties&, const ::Ice::Current& ic = ::Ice::Current()) const;
};

class PlaylistEventI : public EventSinkI , public TianShanIce::Streamer::PlaylistEventSink
{
public:
	PlaylistEventI( NgodEnv& env , NgodSessionManager& manager ,StreamEventDispatcher& eventDispatcher );
	virtual ~PlaylistEventI();
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	virtual void OnItemStepped(	const ::std::string& proxy, const ::std::string& playlistId, 
								::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, 
								const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic = ::Ice::Current()) const;
};

class RepositionEventSinkI : public EventSinkI , public ::TianShanIce::Events::GenericEventSink  
{
public:
	RepositionEventSinkI(NgodEnv& env , NgodSessionManager& manager , StreamEventDispatcher& eventDispatcher);
	virtual ~RepositionEventSinkI();

	virtual void post(
		const ::std::string& category,
		::Ice::Int eventId,
		const ::std::string& eventName,
		const ::std::string& stampUTC,
		const ::std::string& sourceNetId,
		const ::TianShanIce::Properties& params,
		const ::Ice::Current& /* = ::Ice::Current */
		);

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());
};

class PauseTimeoutEventSinkI :public EventSinkI , public ::TianShanIce::Events::GenericEventSink  
{
public:
	PauseTimeoutEventSinkI(NgodEnv& env , NgodSessionManager& manager , StreamEventDispatcher& eventDispatcher);
	virtual ~PauseTimeoutEventSinkI();

	virtual void post(
		const ::std::string& category,
		::Ice::Int eventId,
		const ::std::string& eventName,
		const ::std::string& stampUTC,
		const ::std::string& sourceNetId,
		const ::TianShanIce::Properties& params,
		const ::Ice::Current& /* = ::Ice::Current */
		);

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current()){}
};

}//namespace NGOD

#endif//_tianshan_ngod_session_manager_header_file_h__

