
#include "Scheduler.h"
#include "SsEnvironment.h"
#include <assert.h>

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

namespace ZQ
{
namespace StreamService
{

Scheduler::Scheduler( SsEnvironment* environment)
:env(environment)
{
	mbStop = true;
}

Scheduler::~Scheduler( )
{
	stop();
}
bool Scheduler::start()
{
	if(!mbStop)
	{
		stop();
	}	
	mbStop = false;
	return NativeThread::start();
}
void Scheduler::stop()
{
	if( !mbStop )
	{
		mbStop = true;
		mTasksSem.post();
		waitHandle(100*1000);
	}
}
void Scheduler::schedule( const std::string& id , const TianShanIce::Streamer::Timertype& type, 
						 ScheduleTaskPtr Task , const IceUtil::Time& interval )
{
	scheduleAt( id , type, Task , IceUtil::Time::now() + interval );
}

void Scheduler::scheduleAt( const std::string& idString , const TianShanIce::Streamer::Timertype& type, 
						   ScheduleTaskPtr Task , const IceUtil::Time& targetTime )
{
	assert(Task);
	ScheduleID id( idString , type );
	ScheduleSortID sortId( idString , type , targetTime	);
	bool bExpired = false;
	Task->updateTargetTime( targetTime );
	{
		ZQ::common::MutexGuard gd(mTasksMutex);
		
		IDMAP::iterator itId = mIds.find(id);
		TASKMAP::iterator itSort = mTasks.find(sortId);
		if( itId == mIds.end() )
		{
			mIds.insert(IDMAP::value_type(id,sortId));
			mTasks[sortId] = Task ;
		}
		else
		{
			itId->second	= sortId;
			if( itSort == mTasks.end() )
			{
				mTasks[sortId] = Task;
			}
			else
			{
				itSort->second	= Task;
			}
		}
#if defined _DEBUG || defined DEBUG
		printf("current timer object [%ld][%ld]\n",mIds.size(),mTasks.size());
#endif
		if( mNextWakeupTime > targetTime )
			bExpired = true;
	}
	if( bExpired)
	{
		mTasksSem.post();
	}
}

void Scheduler::cancelSchedule( const std::string& idString ,
							   const TianShanIce::Streamer::Timertype& type )
{
	ScheduleID id(idString,type);
	{
		ZQ::common::MutexGuard	 gd(mTasksMutex);
		IDMAP::iterator it = mIds.find(id);
		if( it != mIds.end() )
		{
			ScheduleSortID& sortId = it->second;
			mTasks.erase(sortId);
			mIds.erase(id);
		}
#if defined _DEBUG || defined DEBUG
		printf("current timer object [%ld][%ld]\n",mIds.size(),mTasks.size());
#endif
	}
}

int Scheduler::run( )
{	
	IceUtil::Time	defaultInterval = IceUtil::Time::milliSeconds( 60* 60 * 1000 );
	IceUtil::Time	interval = defaultInterval;
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(Scheduler,"Scheduler is running"));
	while( !mbStop )
	{
		{
			ZQ::common::MutexGuard gd(mTasksMutex);
			mNextWakeupTime = IceUtil::Time::now() + interval;			
		}
		mTasksSem.timedWait(static_cast<uint32>(interval.toMilliSeconds()) );

		while ( !mbStop )
		{
			ScheduleTaskPtr task = NULL;
			{
				ZQ::common::MutexGuard gd(mTasksMutex);
				if ( mTasks.size() <= 0  )
				{
					interval = defaultInterval;
					break;
				}
				TASKMAP::iterator it	= mTasks.begin();
				IceUtil::Time current	= IceUtil::Time::now();
				task					= it->second;
				if( task->getTargetTime() <= current )
				{
					ScheduleID id(it->first.idString,it->first.type);
					mIds.erase(id);
					mTasks.erase(it);
				}
				else
				{					
					interval	= task->getTargetTime() - current;
					task		= NULL;
					if( interval.toMilliSeconds() < 2 )
						interval = IceUtil::Time::milliSeconds(2);
					break;
				}
			}
			if(task)
			{
				task->runTask();
			}
		} ;
	}
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(Scheduler,"Scheduler is stopped"));
	return 1;
}


void DPCScheduleTask::runTask( )
{
	(new DPCThreadRequest( mStreamId , env , env->getMainThreadPool()) )->start() ;
}
DPCScheduleTask::DPCScheduleTask(   const Ice::Identity& id , SsEnvironment* environment )
:env(environment),mStreamId(id)
{
}

DPCScheduleTask::~DPCScheduleTask( )
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
DPCThreadRequest::DPCThreadRequest( const Ice::Identity& id,
								   SsEnvironment* environment , 
								   ZQ::common::NativeThreadPool& pool)
								   :ZQ::common::ThreadRequest(pool),
								   env(environment),
								   mStreamId(id)
{
}

DPCThreadRequest::~DPCThreadRequest( )
{
}

void DPCThreadRequest::final(int retcode , bool bCancelled )
{
	delete this;
}

int	DPCThreadRequest::run( void )
{
	try
	{
		TianShanIce::Streamer::SsPlaylistPrx prx = UCKGETOBJECT(TianShanIce::Streamer::SsPlaylistPrx,mStreamId);
		prx->onTimer( TianShanIce::Streamer::TIMERDPC );
	}
	catch( const Ice::ObjectNotExistException&)
	{
	}
	catch( const TianShanIce::BaseException& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DPCThreadRequest,"TianShan Exception caught when invoke onTimer:%s"),
			ex.ice_name().c_str() );
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DPCThreadRequest,"Ice Exception caught when invoke onTimer:%s"),
			ex.ice_name().c_str() );
	}
	catch( ... )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DPCThreadRequest,"unknown exception caught when invoke stream's onTimer"));
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
///DestroyPLRequest
DestroyPLRequest::DestroyPLRequest( ZQ::common::NativeThreadPool& pool , SsEnvironment* environment , const Ice::Identity& id)
:ZQ::common::ThreadRequest(pool),
env(environment),
mPlaylistId(id)
{
}

DestroyPLRequest::~DestroyPLRequest( )
{

}

int DestroyPLRequest::run()
{
	try
	{
		TianShanIce::Streamer::SsPlaylistPrx prx = UCKGETOBJECT( TianShanIce::Streamer::SsPlaylistPrx , mPlaylistId );
		if(!prx)
			return 1;
		Ice::Context ctx;
		ctx["caller"] = "DestroyPLRequest";
		prx->destroy( ctx );
	}
	catch( const TianShanIce::BaseException& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , CLOGFMT(DestroyPLRequest , "caught tianshan exception [%s] when destroy playlist [%s]" ),
			ex.message.c_str(),
			mPlaylistId.name.c_str());
	}
	catch( const Ice::ObjectNotExistException&)
	{
	}
	catch( const Ice::Exception& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR , CLOGFMT(DestroyPLRequest , "caught ice exception [%s] when destroy playlist [%s]" ),
			ex.ice_name().c_str(),
			mPlaylistId.name.c_str());
	}
	catch(...)
	{
		ENVLOG(ZQ::common::Log::L_ERROR , CLOGFMT(DestroyPLRequest , "caught unknown exception when destroy playlist [%s]" ),			
			mPlaylistId.name.c_str());
	}
	return 1;
}


}}//namespace ZQ::StreamService

