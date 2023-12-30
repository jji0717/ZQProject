
#include <ZQ_common_conf.h>
#include <NativeThreadPool.h>
#include "NgodEnv.h"
#include "NgodSessionManager.h"
#include "NgodSession.h"
#include "SOPConfig.h"
#include "NgodConfig.h"
#include <TianShanIceHelper.h>
#include "SelectionResourceManager.h"



namespace NGOD
{

class ResourceManagerStreamerEventTask : public NgodTaskBase
{
public:
	ResourceManagerStreamerEventTask( NgodEnv& env, NgodSessionManager & sessManager ,const std::string& streamerNetId, bool status )
		:NgodTaskBase(env,sessManager),
		mStreamerNetId(streamerNetId),
		mStatus(status)
	{
		mTimeStart = IceUtil::Time::now();
	}
protected:
	virtual void	run()
	{
		IStreamSmithSite* pSite = mSessManager.getSite();

		if(!pSite)
			return;
		
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ResourceManagerStreamerEventTask,"notify sessions that streamer[%s] is [%s]"),
			mStreamerNetId.c_str() , mStatus ? "UP":"DOWN");
		
		std::vector<NGOD::NgodSessionPrx> prxs = mSessManager.getDatabase().openSessionByStreamer(mStreamerNetId);
		std::vector<NGOD::NgodSessionPrx>::const_iterator it = prxs.begin();
		for( ; it != prxs.end() ; it ++ )
		{
			NGOD::NgodSessionPrx sessPrx = *it;
			if(!sessPrx)
				continue;
			std::string sessId;
			try
			{
				sessId = sessPrx->getSessionId();
			}
			catch( const Ice::Exception& ex)
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ResourceManagerStreamerEventTask,"caught [%s] while getting session id"),
					ex.ice_name().c_str() );
				continue;
			}
			if( sessId.empty())
				continue;		

			IClientSession* pSess = pSite->findClientSession( sessId.c_str() , IClientSession::LocalSession );
			if(!pSess)
				continue;
			if(mStatus)
			{				
				pSess->set(KEY_REQUEST_PRIORITY,ZQ::common::Variant((int)-1));
			}
			else
			{//streamer down
				///TODO: should be a configuration
				pSess->set(KEY_REQUEST_PRIORITY,ZQ::common::Variant((int)ngodConfig.rtspSession.requestPriorityIfStreamerNA));
			}
			pSess->release();
		}
		IceUtil::Time timeStop = IceUtil::Time::now();
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ResourceManagerStreamerEventTask," %d session(s) of streamer[%s] have been set to low priority, time cost[%lld]ms"), 
			(int)prxs.size() , mStreamerNetId.c_str() , (timeStop - mTimeStart).toMilliSeconds() );
	}
private:
	std::string		mStreamerNetId;
	bool			mStatus;
	IceUtil::Time	mTimeStart;
};

class StreamFailoverTask : public NgodTaskBase {
public:
	StreamFailoverTask( NgodEnv& env, NgodSessionManager & sessManager ,const std::string& streamerNetId, bool status )
		: NgodTaskBase(env,sessManager), mStreamerNetId(streamerNetId), mStatus(status)
	{
	}

	virtual ~StreamFailoverTask( ) {}

protected:

	virtual void run( )
	{
		int64 failOverStartTime = ZQ::common::TimeUtil::now();
		int64  stepTimeStart = ZQ::common::TimeUtil::now();
		if( mStatus )
			return;// ignore streamer UP event

		if( mEnv.getFailoverTestStreamers().size() != 2 )
			return;

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamFailoverTask, "streamer[%s] is [%s], trying to failover the streams on a new node"),
			mStreamerNetId.c_str() , mStatus ? "UP":"DOWN");

		SessionStatCache::STATSET ss = mEnv.getStatCache().getCachedStatusForStreamer(mStreamerNetId);
		if( ss.empty() )
			return;

		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamFailoverTask, "got the cached status successful took [%d]ms, start failing over"), (int)(ZQ::common::TimeUtil::now() - stepTimeStart));
		
		// starting to fail over stream on the other node  geting the desStreamer netId and endpoint
		std::map <std::string , std::string > vecStreamers = mEnv.getFailoverTestStreamers();
		if (vecStreamers.end() == vecStreamers.find(mStreamerNetId))
			return;

		std::map <std::string , std::string >::const_iterator mapIter = vecStreamers.begin();
		std::string desStreamerNetId = mapIter->first;
		std::string desStreamerEndpoint = mapIter->second;
		if (desStreamerNetId == mStreamerNetId)
		{
			mapIter ++;
			desStreamerNetId    = mapIter->first;
			desStreamerEndpoint = mapIter->second;
		}

		// get the stream stat from srcStreamer
		stepTimeStart = ZQ::common::TimeUtil::now();
		SessionStatCache::STATSET::const_iterator ssIter = ss.begin();
		SessionStatCache::STATSET allocatedStat;
		for( ; ssIter != ss.end(); ssIter++)
		{
			SessionStat *currentSessStat = *ssIter;
			mEnv.getSelResManager().releaseResource(currentSessStat->selectionResource, mStreamerNetId);
			if(mEnv.getSelResManager().allocateResource(currentSessStat->selectionResource, desStreamerNetId))
			{
				allocatedStat.insert(currentSessStat);
			}
		}

		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamFailoverTask, "allocate resource took [%d]ms"), (int)(ZQ::common::TimeUtil::now() - stepTimeStart));
		//get the StreamSmithAdminPrx of desStreamer
		TianShanIce::Streamer::StreamSmithAdminPrx  desStreamerPrx = NULL;
		try 
		{
			Ice::ObjectPrx objPrx = NULL;
			desStreamerPrx  = TianShanIce::Streamer::StreamSmithAdminPrx::checkedCast(mEnv.mAdapter->getCommunicator()->stringToProxy(desStreamerEndpoint));
			#if ICE_INT_VERSION / 100 >= 306
				objPrx = desStreamerPrx->ice_collocationOptimized(false);
			#else
				objPrx = desStreamerPrx->ice_collocationOptimization(false);
			#endif
			desStreamerPrx = TianShanIce::Streamer::StreamSmithAdminPrx::uncheckedCast(objPrx);
			desStreamerPrx = TianShanIce::Streamer::StreamSmithAdminPrx::uncheckedCast(desStreamerPrx->ice_timeout(90000));
		}
		catch(::Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamFailoverTask,"failed to get StreamSmithAdmin proxy at streamer netid [%s] endpoint [%s] caught exception(%s)"), desStreamerNetId.c_str(), desStreamerEndpoint.c_str(),  ex.ice_name().c_str());
		}
		catch (...)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamFailoverTask, "failed to get StreamSmithAdmin proxy at streamer netid [%s] endpoint [%s] caught unknown exception(%d)"), desStreamerNetId.c_str(), desStreamerEndpoint.c_str(), SYS::getLastErr());
		}
		if (NULL == desStreamerPrx)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamFailoverTask, "failed to get StreamSmithAdmin proxy at streamer netid [%s] endpoint [%s]."), desStreamerNetId.c_str(), desStreamerEndpoint.c_str());
			return;
		}

		//generate the request data 
		stepTimeStart = ZQ::common::TimeUtil::now();
		SessionStatCache::STATSET::const_iterator allocIter = allocatedStat.begin();
		TianShanIce::Streamer::StreamBatchRequest amiRequest;
		for(; allocIter != allocatedStat.end(); allocIter ++)
		{
			SessionStat *currentSessStat = *allocIter;
			TianShanIce::Streamer::StreamDuplicateReq  streamReq;
			streamReq.resources = currentSessStat->resources;
			streamReq.properties = currentSessStat->properties;
			streamReq.state = currentSessStat->state;
			streamReq.timeoffset = currentSessStat->timeOffset;
			streamReq.scale = currentSessStat->scale;
			streamReq.items = currentSessStat->items;
			streamReq.reqTag = currentSessStat->rtspSessId;
			amiRequest.push_back(streamReq);

			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamFailoverTask, "failover sess[%s]: state[%d] npt[%d.%03d] scale[%.3f] items[%d]"), 
				streamReq.reqTag.c_str(), (int)streamReq.state, (int)(streamReq.timeoffset/1000), (int)(streamReq.timeoffset%1000), streamReq.scale, streamReq.items.size());
		}

		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamFailoverTask, "prepare %d failover requests took [%d]ms"), amiRequest.size(), (int)(ZQ::common::TimeUtil::now() - stepTimeStart));
		
		//AMI ICE call
		stepTimeStart = ZQ::common::TimeUtil::now();
		TianShanIce::Streamer::StreamBatchResponse amiResponse;
		try
		{
			amiResponse = desStreamerPrx->createStreamBatch(amiRequest);
		}
		catch(::Ice::Exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamFailoverTask,"failed to createStreamBatch at streamer netid [%s] endpoint [%s] caught exception(%s)"), desStreamerNetId.c_str(), desStreamerEndpoint.c_str(),  ex.ice_name().c_str());
		}
		catch (...)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamFailoverTask, "failed to createStreamBatch at streamer netid [%s] endpoint [%s] caught unknown exception(%d)"), desStreamerNetId.c_str(), desStreamerEndpoint.c_str(), SYS::getLastErr());
		}

		TianShanIce::Streamer::StreamBatchResponse::const_iterator resIter = amiResponse.begin();
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamFailoverTask, "failover step createStreamBatch took [%d]ms."), (int)(ZQ::common::TimeUtil::now() - stepTimeStart));
		//release the resource of the failed sessions 
		stepTimeStart = ZQ::common::TimeUtil::now();
		for (; resIter != amiResponse.end(); resIter ++)
		{
			if (resIter->streamSessionId.empty())
			{
				//create session on desStreamer failed
				SessionStatCache::STATSET::iterator releaseIter = allocatedStat.begin();
				for (; releaseIter != allocatedStat.end(); releaseIter ++)
				{
					if ((*releaseIter)->rtspSessId == resIter->reqTag)
					{
						mEnv.getSelResManager().releaseResource((*releaseIter)->selectionResource , desStreamerNetId);
						allocatedStat.erase(releaseIter);
						break;
					}
				}//loop for allocateStat
				continue;
			}//if the streamSeddionId is empty
			//create session successful
			try
			{
				NGOD::NgodSessionPrx ngodSessPrx = mSessManager.openSession(resIter->reqTag);
				ngodSessPrx->rebindStreamSession(resIter->streamSession, resIter->streamSessionId, desStreamerNetId);
				MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamFailoverTask, "rebind the streamsession [%d] and the rtspProxy session [%s] successful."),resIter->streamSessionId.c_str(), resIter->reqTag.c_str());
			}
			catch(::Ice::Exception& ex)
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamFailoverTask,"failed to rebindStreamSession [%s]  at streamer [%s] caught exception(%s)"), resIter->streamSessionId.c_str(), desStreamerNetId.c_str(),  ex.ice_name().c_str());
			}
			catch (...)
			{
				MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamFailoverTask, "failed to rebindStreamSession [%s]  at streamer [%s] caught unknown exception(%d)"), resIter->streamSessionId.c_str(), desStreamerNetId.c_str(),  SYS::getLastErr());
			}
		}//loop for amiResponse
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamFailoverTask, "failover step dispose the response data took [%d]ms."), (int)(ZQ::common::TimeUtil::now() - stepTimeStart));
		int  totalTime = (int)(ZQ::common::TimeUtil::now() - failOverStartTime);
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamFailoverTask, "run() try to failover [%d] streams , succesful [%d] streams took [%d]ms"), amiRequest.size(), allocatedStat.size(), totalTime);
		return;
	}	
private:
	std::string			mStreamerNetId;
	bool				mStatus;
};

void NgodTaskBase::execute( )
{
	try
	{
		run();
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodTaskBase,"caught unknown exception while executing task"));
	}
}
NgodTaskRunner::NgodTaskRunner( NgodEnv& env,NgodSessionManager & sessManager )
:mSessManager(sessManager),
mEnv(env),
mbQuit(false)
{
}
NgodTaskRunner::~NgodTaskRunner()
{
	stop();
	{
		ZQ::common::MutexGuard gd(mLocker);
		TASKLIST::iterator it = mTasks.begin();
		for( ; it != mTasks.end() ; it ++ )
		{
			if(*it)
			{
				delete *it;
			}
		}
		mTasks.clear();
	}
}

void NgodTaskRunner::stop()
{
	if(mbQuit)
		return;
	mbQuit = true;
	mSem.post();
	mSessManager.waitHandle(10000);
}

void NgodTaskRunner::execute( )
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodTaskRunner,"running..."));
	mbQuit = false;
	while(!mbQuit)
	{
		mSem.timedWait(5000);
		if(mbQuit) break;

		int64 stampNow = ZQ::common::now();
		// trigger to snmp_refreshSOPUsage(0);
		static int64 stampLastRefreshUsage = 0;
		if (stampNow - stampLastRefreshUsage > (30 * 1000))
		{
			mEnv.snmp_refreshSOPUsage(0);
			stampLastRefreshUsage = stampNow;
		}

		while (true)
		{
			NgodTaskBase* task = NULL;
			{
				ZQ::common::MutexGuard gd(mLocker);
				if( mTasks.empty() )
					break;
				task = mTasks.front();
				mTasks.pop_front();
			}

			if(!task)
				break;
			task->execute();
			delete task;
		}
	}

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodTaskRunner,"quitting..."));
}

void NgodTaskRunner::onStreamerEvent( const std::string& streamerNetId , bool bUp )
{
	ResourceManagerStreamerEventTask* pTask = new ResourceManagerStreamerEventTask(mEnv,mSessManager,streamerNetId,bUp);
	StreamFailoverTask* pFailOverTask = new StreamFailoverTask(mEnv,mSessManager,streamerNetId,bUp);
	{
		ZQ::common::MutexGuard gd(mLocker);
		//FIXME: add a configuration here, only enabled fail over when configuration is ON
		mTasks.push_back(pFailOverTask);
		mTasks.push_back(pTask);
	}
	mSem.post();
}

SessionFactory::SessionFactory( NgodEnv& env , NgodSessionManager& manager )
:mEnv(env),mSessManager(manager)
{
}
SessionFactory::~SessionFactory()
{
}

Ice::ObjectPtr SessionFactory::create(const std::string& type)
{
	if( type == NGOD::NgodSession::ice_staticId() )
	{
		return new NgodSessionI( mEnv , mSessManager );
	}
	return NULL;
}
void SessionFactory::destroy()
{

}

NgodSessionManager::NgodSessionManager( NgodEnv& env )
:mEnv(env),
mDb(env),
mpSite(NULL),
mScheduler(*this,mEnv.getThreadPool()),
mTaskRunner(env,*this)
{
}

NgodSessionManager::~NgodSessionManager(void)
{
}

bool NgodSessionManager::restoreRtspSession( const std::string& rtspSess , const std::string& url )
{
	if(!mpSite)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSessionManager,"restoreRtspSession() can't restore rtsp session due to missing rtspproxy core interface"));
		return false;
	}
	try
	{
		IClientSession* pSess = mpSite->createClientSession( rtspSess.c_str() , url.c_str() );
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionManager,"restoreRtspSession() rtsp session[%s] with url[%s] is restored"),
			rtspSess.c_str() , url.c_str() );
		return pSess != NULL;
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSessionManager,"caught unknown exception while restoring rtsp session"));
		return false;
	}
	return true;
}

NGOD::NgodSessionPrx NgodSessionManager::creatSession( const std::string& odsessId , const std::string& url )
{
	assert( mpSite != NULL );
	//create session instance and add it into database
	int iRetry = 0 ;
	std::string sessId;
	
	do
	{
		IClientSession* pSess = mpSite->createClientSession( NULL , url.c_str() );
		if( !pSess )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSessionManager,"failed to create rtsp session with url [%s] odsessid[%s]"),url.c_str() , odsessId.c_str() );
			return NULL;
		}
		
		sessId = pSess->getSessionID();

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionManager,"session[%s] created with odsess[%s]"),
			sessId.c_str() , odsessId.c_str() );

		NgodSessionI* pNgodSess = new NgodSessionI( mEnv , *this);
		assert( pNgodSess != NULL );
		pNgodSess->create( sessId , odsessId );
		
		NGOD::NgodSessionPtr p = pNgodSess;

		int64 dbSaveStart = ZQTianShan::now();
		if(mDb.addSession(sessId,p))
		{
			int64 dbSaveStop = ZQTianShan::now();
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionManager,"session[%s] odsess[%s] add record to db used [%lld]ms"),
				sessId.c_str() , odsessId.c_str(), (dbSaveStop - dbSaveStart) );
			break;
		}
		//failed to add session into db, destroy created RTSP session and try again
		mpSite->destroyClientSession( pSess->getSessionID() );

	}while( iRetry++ < 3 );

	return mDb.openSession( sessId );
}

std::string NgodSessionManager::getR2ConnectionId( const std::string& sessGroupName ) const
{
	std::string connId;
	{
		ZQ::common::MutexGuard gd(mSessGroupConnMapLocker);
		SESSGROUPCONNMAP::const_iterator it = mSessGroupConnMap.find( sessGroupName );
		if( it != mSessGroupConnMap.end() )
		{
			connId = it->second;
		}
	}
	return connId;
}

IServerRequest* NgodSessionManager::getServerRequest(  const std::string& sessId , const std::string& connId )
{
	assert( mpSite != NULL );
	return mpSite->newServerRequest( sessId.c_str() , connId.c_str() );
}

void NgodSessionManager::updateR2ConnectionId( const std::string& sessGroupName , const std::string& connectionId )
{
	bool bChanged = false;
	{
		ZQ::common::MutexGuard gd(mSessGroupConnMapLocker);
		SESSGROUPCONNMAP::iterator it = mSessGroupConnMap.find(sessGroupName);
		if( it != mSessGroupConnMap.end() )
		{
			if( it->second != connectionId )
			{
				bChanged = true;
				it->second = connectionId;
			}
		}
		else
		{
			bChanged = true;
			mSessGroupConnMap[sessGroupName] = connectionId;
		}
	}

	if( bChanged )
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionManager,"update R2 connection [%s] to session group[%s]"), connectionId.c_str() , sessGroupName.c_str() );
}

NgodDatabase& NgodSessionManager::getDatabase( )
{
	return mDb;
}

NGOD::NgodSessionPrx NgodSessionManager::openSession( const std::string& sessId )
{
	return mDb.openSession( sessId );
}

void NgodSessionManager::restore()
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionManager,"restore started"));
	NgodSessionPrxS sessions = mDb.openAllSessions();
	NgodSessionPrxS::iterator itSess =sessions.begin();
	for( ; itSess != sessions.end() ; itSess ++ )
	{
		try
		{
			(*itSess)->onRestore();
		}
		catch( const Ice::Exception& )
		{
		}
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(NgodSessionManager,"restore stopped"));
}

bool NgodSessionManager::start( const std::string& dbPath , int32 evictorSize ,IStreamSmithSite* pSite )
{
	assert( pSite != NULL );
	if( !mDb.openDatabase(dbPath , evictorSize ))
	{
		return false;
	}

	try
	{
		mReplicaListener = new NgodReplicaListener( mEnv );
		assert( mReplicaListener );	
		mEnv.getObjAdapter()->ZQADAPTER_ADD( mEnv.getCommunicator(), mReplicaListener, "ReplicaSubscriber");

		mSessionView	= new SessionViewImpl( mEnv , *this );
		assert(mSessionView );
		mEnv.getObjAdapter()->ZQADAPTER_ADD( mEnv.getCommunicator() , mSessionView , "Ngod2View" );
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(NgodSessionManager,"failed to add SessionView/ReplicaUpdater into IE runtime due to %s, quiting ..."), ex.ice_name().c_str() );
		return false;
	}
	
	mpSite = pSite;
	
	restore();

	mScheduler.start();

	return ZQ::common::NativeThread::start();
}
void NgodSessionManager::destroySession( const std::string& sessId )
{
	NGOD::NgodSessionPrx prx = mDb.openSession( sessId);
	if( !prx )
	{
		return;
	}

	try
	{
		prx->destroy(NULL);
	}
	catch( const Ice::Exception&){}
}


void NgodSessionManager::destroyRtspSession( const std::string& sessId )
{
	assert( mpSite != NULL );
	mpSite->destroyClientSession( sessId.c_str() );
}

void NgodSessionManager::stop()
{
	mScheduler.stop();
	mDb.closeDatabase();
	mTaskRunner.stop();
}

void NgodSessionManager::cancelTimer( const std::string& sessId )
{
	mScheduler.cancel( sessId );
}
void NgodSessionManager::updateTimer( const std::string& sessId , int32 interval )
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(NgodSessionManager,"session[%s] update timer with interval[%d]"),sessId.c_str() , interval );
	mScheduler.scheduleAt(sessId, interval );
}

int NgodSessionManager::run( )
{
	mEnv.getSelResManager().attachEventSinker(&mTaskRunner);
	mTaskRunner.execute();
	return 0;
}

bool NgodSessionManager::processRequest( IClientRequest* clireq )
{
	assert( clireq != NULL );
	REQUEST_VerbCode verbCode = clireq->getVerb();

	NgodClientRequestIPtr request;	

	switch (  verbCode )
	{
	case RTSP_MTHD_SETUP:
		{
			request = new NgodRequestSetup( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_PLAY:
		{
			request = new NgodRequestPlay( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_PAUSE:
		{
			request = new NgodRequestPause( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_TEARDOWN:
		{
			request = new NgodRequestTeardown( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_GET_PARAMETER:
		{
			request = new NgodRequestGetParameter( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_OPTIONS:
		{
			//request = new NgodRequestOptions( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_DESCRIBE:
		{
			//request = new NgodRequestPause( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_SET_PARAMETER:
		{
			request = new NgodRequestSetParam( mEnv , *this , clireq );
		}
		break;
	case RTSP_MTHD_RESPONSE:
		{			
			request = new NgodRequestAnnounceResponse( mEnv , *this , clireq );
		}
		break;
	default:
		{

		}
		break;
	}

	if( request )
		request->process();

	return true;
}

//////////////////////////////////////////////////////////////////////////
///EventSinkI
EventSinkI::EventSinkI( NgodEnv& env , NgodSessionManager& manager ,StreamEventDispatcher& dispatcher )
:mEnv(env),
mSessManager(manager),
mDispatcher(dispatcher)
{

}

EventSinkI::~EventSinkI()
{

}

void EventSinkI::sendEvent( const StreamEventRoutine& r , const StreamEventAttr& a ) const
{
	mEnv.updateLastEventRecvTime( ZQ::common::now() );
	mDispatcher.pushEvent(r,a);
}

//////////////////////////////////////////////////////////////////////////
///StreamEventI
void initStreamEventAttr( StreamEventAttr& a)
{
	a.previousSpeed				= 0.0f;
	a.currentSpeed				= 0.0f;
	a.prevState					= TianShanIce::Streamer::stsStop;
	a.currentState				= TianShanIce::Streamer::stsStop;
	a.exitCode					= 0;
	a.prevCtrlNum				= 0;
	a.currentCtrlNum			= 0;
	a.eventIndex				= -1;
}
StreamEventI::StreamEventI(  NgodEnv& env , NgodSessionManager& manager ,StreamEventDispatcher& eventDispatcher)
:EventSinkI(env,manager,eventDispatcher)
{
}

StreamEventI::~StreamEventI()
{
}

void StreamEventI::ping(::Ice::Long , const ::Ice::Current& ic )
{
}
void StreamEventI::OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic/*= ::Ice::Current()*/) const
{
    MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventI, "OnEndOfStream() [%s]"), uid.c_str());
	StreamEventAttr a;	initStreamEventAttr(a);
	a.proxyString		= proxy;
	a.playlistString	= uid;
	a.props				= props;
	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);
	
	sendEvent( streamEventENDOFSTREAM , a );

}
void StreamEventI::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic/*= ::Ice::Current()*/) const
{
    MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventI, "OnBeginningOfStream() [%s]"), uid.c_str());
	StreamEventAttr a;	initStreamEventAttr(a);
	a.proxyString		= proxy;
	a.playlistString	= uid;
	a.props				= props;
	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);

	sendEvent( streamEventBEGINOFSTREAM, a );
}

void StreamEventI::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props, const ::Ice::Current& ic/* = ::Ice::Current()*/) const
{
	if( ngodConfig.announce.useTianShanAnnounceCodeScaleChanged <= 0)
		return;

    MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventI, "OnSpeedChanged() [%s]"), uid.c_str());
	StreamEventAttr a;	initStreamEventAttr(a);
	a.proxyString		= proxy;
	a.playlistString	= uid;
	a.props				= props;
	a.previousSpeed		= prevSpeed;
	a.currentSpeed		= currentSpeed;
	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);

	sendEvent( streamEventSPEEDCHANGE,  a );
}

void StreamEventI::OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::Streamer::StreamState prevState, 
								  ::TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties& props, const ::Ice::Current& ic) const 
{
	if( ngodConfig.announce.useTianShanAnnounceCodeStateChanged <= 0 )
		return;

    MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventI, "OnStateChanged() [%s]"), uid.c_str());
	StreamEventAttr a;	initStreamEventAttr(a);
	a.proxyString		= proxy;
	a.playlistString	= uid;
	a.props				= props;
	a.prevState			= prevState;
	a.currentState		= currentState;
	ZQTianShan::Util::getPropertyDataWithDefault(props,"scale",0.0f,a.currentSpeed);
	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);

	sendEvent( streamEventSTATECHANGE , a );
}
void StreamEventI::OnExit(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic /*= ::Ice::Current()*/) const
{

}
void StreamEventI::OnExit2(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const TianShanIce::Properties&, const ::Ice::Current& ic /*= ::Ice::Current()*/) const
{

}

//////////////////////////////////////////////////////////////////////////
///PlaylistEventI
PlaylistEventI::PlaylistEventI(NgodEnv& env , NgodSessionManager& manager, StreamEventDispatcher& eventDispatcher)
:EventSinkI(env,manager,eventDispatcher)
{

}
PlaylistEventI::~PlaylistEventI()
{

}
void PlaylistEventI::ping(::Ice::Long lv, const ::Ice::Current& ic/* = ::Ice::Current()*/)
{

}

void PlaylistEventI::OnItemStepped(	const ::std::string& proxy, const ::std::string& playlistId, 
						   ::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, 
						   const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic /*= ::Ice::Current()*/) const
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventI, "OnItemStepped() [%s]"), playlistId.c_str());
	if (ngodConfig.sessionHistory.enableHistory <= 0 && ngodConfig.announce.includeTransition <= 0
		&& ngodConfig.announce.useTianShanAnnounceCodeScaleChanged <= 0) // because "lastScale" updated per ItemStepped maybe referenced in scaleChanged announce
		return; 

	StreamEventAttr a;	initStreamEventAttr(a);
	a.proxyString		= proxy;
	a.playlistString	= playlistId;
	a.props				= ItemProps;
	a.prevCtrlNum		= prevUserCtrlNum;
	a.currentCtrlNum	= currentUserCtrlNum;

	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);
	ZQTianShan::Util::getPropertyDataWithDefault(a.props,"scale",0.0f,a.currentSpeed);//new
	a.props["reason"] = "SERVER";
	{
		std::string tmpstr;
		ZQTianShan::Util::getPropertyDataWithDefault(ItemProps, "ItemExitReason", "", tmpstr);
		std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), tolower);
		if (std::string::npos != tmpstr.find("userreq"))
			a.props["reason"] = "USER";
	}

	sendEvent( streamEventITEMSTEP , a );
}

//////////////////////////////////////////////////////////////////////////
///PauseTimeoutEventSinkI

PauseTimeoutEventSinkI::PauseTimeoutEventSinkI(NgodEnv& env , NgodSessionManager& manager , StreamEventDispatcher& eventDispatcher)
:EventSinkI(env,manager, eventDispatcher )
{
}
PauseTimeoutEventSinkI::~PauseTimeoutEventSinkI()
{
}
void PauseTimeoutEventSinkI::post(  const ::std::string& category, ::Ice::Int eventId,  const ::std::string& eventName,
							   const ::std::string& stampUTC,  const ::std::string& sourceNetId,  const ::TianShanIce::Properties& params,
							   const ::Ice::Current& ic  )
{
	std::string playlistId; 
	ZQTianShan::Util::getPropertyDataWithDefault(params, "streamSessionId", "",  playlistId);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(PauseTimeoutEventSinkI, "Event(%s) [%s]"), eventName.c_str(), playlistId.c_str());

	StreamEventAttr a;	initStreamEventAttr(a);
	a.playlistString	= playlistId;
	a.props				= params;
	a.props["reason"] = "USER";
	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);

	sendEvent( streamEventPauseTimeout , a );
}

//////////////////////////////////////////////////////////////////////////
///PlaylistEventI
RepositionEventSinkI::RepositionEventSinkI(NgodEnv& env , NgodSessionManager& manager , StreamEventDispatcher& eventDispatcher)
:EventSinkI(env,manager , eventDispatcher)
{
}

RepositionEventSinkI::~RepositionEventSinkI()
{
}

void RepositionEventSinkI::ping(::Ice::Long, const ::Ice::Current&)
{

}

void RepositionEventSinkI::post(
							 const ::std::string& category,
							 ::Ice::Int eventId,
							 const ::std::string& eventName,
							 const ::std::string& stampUTC,
							 const ::std::string& sourceNetId,
							 const ::TianShanIce::Properties& params,
							 const ::Ice::Current& ic )
{
	//playlistµÄsession id 
	std::string playlistId; 
	ZQTianShan::Util::getPropertyDataWithDefault(params, "streamSessionId", "",  playlistId);

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(RepositionEventSinkI, "Event(%s) [%s]"), eventName.c_str(), playlistId.c_str());

	if (ngodConfig.sessionHistory.enableHistory <= 0)
		return;

	if(playlistId.empty())
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RepositionEventSinkI, "Can not get streamID from Properties"));
		return;
	}

	StreamEventAttr a;	initStreamEventAttr(a);
	a.playlistString	= playlistId;
	a.props				= params;
	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);
	sendEvent( streamReposition , a );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///

NgodReplicaListener::NgodReplicaListener(NgodEnv& env)
:mEnv(env)
{
}
NgodReplicaListener::~NgodReplicaListener()
{
}
void NgodReplicaListener::updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& callback, const ::TianShanIce::Replicas& reps, const ::Ice::Current&/* = ::Ice::Current()*/)
{
	mEnv.getSelResManager().updateReplica( reps );
	try
	{
		callback->ice_response( sopConfig.sopRestrict.replicaUpdateInterval );//replica update interval use second instead of millisecond
	}
	catch(const Ice::Exception& )
	{
	}
}

//////////////////////////////////////////////////////////////////////////
//RestoreThreadRequest
RestoreThreadRequest::RestoreThreadRequest( NgodDatabase& db , NgodEnv& env, ZQ::common::NativeThreadPool& pool )
:mDb(db),mEnv(env),ZQ::common::ThreadRequest(pool)
{
}
RestoreThreadRequest::~RestoreThreadRequest()
{
}

void RestoreThreadRequest::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	delete this;
}

int RestoreThreadRequest::run()
{
	
	return 0;
}


StreamEventDispatcher::StreamEventDispatcher( NgodEnv& env , NgodSessionManager& manager )
:mEnv(env),mManager(manager)
{
	
}
StreamEventDispatcher::~StreamEventDispatcher()
{
}
void StreamEventDispatcher::start()
{
	int maxThreads = ngodConfig.announce.announceSenderThreadCount;
	maxThreads = MAX(maxThreads,5);	
	mPool.resize(maxThreads);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventDispatcher,"start StreamEventDispatcher with threadcount[%d]"),maxThreads);
}
void StreamEventDispatcher::stop( )
{
	mPool.stop();
}


void StreamEventDispatcher::pushEvent( const StreamEventRoutine& r, const StreamEventAttr& a )
{
	StreamEventDispatchInfo info;
	info.r = r;
	info.a = a;
	info.timestamp = ZQ::common::now();

	static int32 maxThread = mPool.size();
	int32 pendingEvent = mPool.pendingRequestSize();	
	int32 runningThreads = mPool.activeCount();
	if( pendingEvent > 3 * maxThread )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(StreamEventDispatcher,"StreamEventDispatcher() pendingEvent[%d] runningThread[%d] maxThread[%d]"),
			pendingEvent , runningThreads , maxThread );
	}
	StreamEventDispatchRequest* req = new StreamEventDispatchRequest(mEnv,mManager,info,mPool);
	req->start();
}


StreamEventDispatchRequest::StreamEventDispatchRequest(NgodEnv& env , NgodSessionManager& manager , StreamEventDispatchInfo& info , ZQ::common::NativeThreadPool& pool)
:mEnv(env),mManager(manager),mEventInfo(info),ZQ::common::ThreadRequest(pool)
{
}

StreamEventDispatchRequest::~StreamEventDispatchRequest()
{

}
int	StreamEventDispatchRequest::run()
{
	static int64 eventTTL = ngodConfig.announce.eventTTL;
	int64 delta = ZQ::common::now() - mEventInfo.timestamp;
	if( delta > eventTTL )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(StreamEventDispatchRequest,"event[%s] streamSession[%s] delayed for a long time[%lld]ms, discard it"),
			streamEventAttrToString(mEventInfo.r,mEventInfo.a).c_str(), mEventInfo.a.playlistString.c_str(), delta );
		return 0;
	}

	int64 startTime = ZQ::common::now();
	
	NgodDatabase& db = mManager.getDatabase();
	
	NgodSessionPrx sess = db.openSessionByStream( mEventInfo.a.playlistString );
	if( !sess )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(StreamEventDispatchRequest,"failed to find session according to streamSession [%s]"), mEventInfo.a.playlistString.c_str() );
		return -1;
	}
	
	try
	{
		sess->onStreamSessionEvent( mEventInfo.r , mEventInfo.a );
		int64 stopTime = ZQ::common::now();
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(StreamEventDispatchRequest,"event[%s] processed, timecost[%lld]ms"),
			streamEventAttrToString( mEventInfo.r , mEventInfo.a ).c_str() , stopTime - startTime );
	}
	catch( const Ice::Exception& )
	{
	}
	catch(...){}
	return 0;
}
void StreamEventDispatchRequest::final(int retcode ,bool bCancelled)
{
	delete this;
}


}//namespace NGOD

