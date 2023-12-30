
#include "renewTicket.h"
#include "SsServiceImpl.h"
#include <stdlib.h>
#include <time.h>

#ifdef ZQ_OS_MSWIN
#include "memoryDebug.h"
#endif

namespace ZQ
{
namespace StreamService
{


//////////////////////////////////////////////////////////////////////////
///RenewTicketTask
RenewTicketTask::RenewTicketTask( RenewTicketCenter& rtCenter ,const Ice::Identity& id )
:center(rtCenter),
plId(id)
{
}

RenewTicketTask::~RenewTicketTask( )
{

}

void RenewTicketTask::runTask()
{
	center.notify(plId);
}
//////////////////////////////////////////////////////////////////////////
///TicketRenewRunner
TicketRenewRunner::TicketRenewRunner( SsEnvironment* environment, RenewTicketCenter& rtCenter , ZQ::common::NativeThreadPool& pool , const Ice::Identity& id)
:ZQ::common::ThreadRequest(pool),
mRenewCenter(rtCenter),
mId(id),
env(environment)
{
}
TicketRenewRunner::~TicketRenewRunner()
{
}

int	TicketRenewRunner::run( )
{
	TianShanIce::Transport::PathTicketPrx ticket = mRenewCenter.getTicket(mId);
	if(!ticket) return 0;
	do
	{
		try
		{
			IceUtil::Time renewStart = IceUtil::Time::now();
			ticket->renew(  env->getConfig().iRenewTicketInterval * 2 );
			IceUtil::Time renewEnd = IceUtil::Time::now();
			ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RenewTicketCenter,"renew ticket ok for [%s] and ticket is[%s], timecost[%lld]"),
				mId.name.c_str(), env->getCommunicator()->proxyToString(ticket).c_str() , (renewEnd-renewStart).toMilliSeconds() );
		}
		catch( const Ice::ObjectNotExistException& )
		{
			ENVLOG(ZQ::common::Log::L_INFO,	CLOGFMT(TicketRenewRunner,"ticket [%s] associated with playlist[%s] does not exist any more, destroy playlist"),
				env->getCommunicator()->proxyToString(ticket).c_str(), mId.name.c_str());
			DestroyPLRequest* pRequest = new DestroyPLRequest(env->getMainThreadPool() , env , mId );
			assert( pRequest );
			pRequest->start();
			break;
		}
		catch( const Ice::Exception& ex)
		{
			ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TicketRenewRunner,"caught [%s] when renew ticket[%s] for session [%s]"),
				ex.ice_name().c_str(), env->getCommunicator()->proxyToString(ticket).c_str() , mId.name.c_str() );
		}
		catch(...)
		{
			ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TicketRenewRunner,"unknown exception caught"));
		}
		mRenewCenter.scheduleRenewCommand( mId );
	}while(0);

	return 0;
}
void TicketRenewRunner::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
///RenewTicketCenter
RenewTicketCenter::RenewTicketCenter( SsEnvironment* environment ,SsServiceImpl& svc)
:env(environment),
mSvcImpl(svc)
{
	mbQuit = false;
}

RenewTicketCenter::~RenewTicketCenter( )
{
}

void RenewTicketCenter::notify( const Ice::Identity& id )
{
	int32 activeCount = env->getRenewTicketThreadPool().activeCount();
	int32 pendingCount = env->getRenewTicketThreadPool().pendingRequestSize();
	int32 totalThreads = env->getRenewTicketThreadPool().size();
	if( pendingCount > 20 )
	{
		ENVLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(RenewTicketCenter,"tickt renew, pendingrequest[%d] activethreads[%d],totalthreads[%d]"),
			pendingCount,activeCount,totalThreads);
	}
	TicketRenewRunner* p = new TicketRenewRunner(env,*this,env->getRenewTicketThreadPool(),id);
	p->start();
// 	//TODO: fixme
// 	{
// 		ZQ::common::MutexGuard gd( mMutex);		
// 		mTickets.push_back(id);
// 	}	
// 	mSemaphore.post();	
}
bool	RenewTicketCenter::start( )
{
	srand( (unsigned int) time(NULL) );
	return ZQ::common::NativeThread::start();
}

void RenewTicketCenter::stop( )
{
	if(isRunning())
	{
		mbQuit = true;
		mSemaphore.post();
		waitHandle(100*1000);
	}
}

TianShanIce::Transport::PathTicketPrx RenewTicketCenter::getTicket( const Ice::Identity& id ) const
{
	ZQ::common::MutexGuard gd(mMutex) ;
	ID2TICKETMAP::const_iterator it = mIdTicketMap.find( id );
	if( it == mIdTicketMap.end() )
		return NULL;
	return it->second;
}

int	RenewTicketCenter::run( )
{
	while( !mbQuit )
	{		
		mSemaphore.timedWait(1000);
		if(mbQuit) break;
		
		Ice::Identity id;
		TianShanIce::Transport::PathTicketPrx ticketPrx = NULL;
		do
		{
			bool	bGet = false;
			{
				ZQ::common::MutexGuard gd(mMutex) ;
				bGet = mTickets.size() > 0;
				if( bGet )
				{
					id = mTickets.front();
					mTickets.pop_front();
					ID2TICKETMAP::iterator it = mIdTicketMap.find( id );
					if( mIdTicketMap.end() != it )
					{
						ticketPrx = it->second;
					}
					else
					{
						ticketPrx = NULL;
					}
				}
				else
				{
					ticketPrx = NULL;
				}
			}
			if( !bGet )  break;
			
			if( ticketPrx )
			{
				//renew 2 * config time interval
				do
				{
					try
					{
						IceUtil::Time renewStart = IceUtil::Time::now();
						ticketPrx->renew(  env->getConfig().iRenewTicketInterval * 2 );
						IceUtil::Time renewEnd = IceUtil::Time::now();
						ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RenewTicketCenter,"renew ticket ok for [%s] and ticket is[%s], timecost[%lld]"),
							id.name.c_str(), env->getCommunicator()->proxyToString(ticketPrx).c_str() , (renewEnd-renewStart).toMilliSeconds() );
					}
					catch( const Ice::ObjectNotExistException& )
					{
						ENVLOG(ZQ::common::Log::L_INFO,	CLOGFMT(RenewTicketCenter,"ticket [%s] associated with playlist[%s] does not exist any more, destroy playlist"),
							env->getCommunicator()->proxyToString(ticketPrx).c_str(), id.name.c_str());
						DestroyPLRequest* pRequest = new DestroyPLRequest(env->getMainThreadPool() , env , id );
						assert( pRequest );
						pRequest->start();
						break;
					}
					catch( const Ice::Exception& ex)
					{
						ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(RenewTicketCenter,"caught [%s] when renew ticket for session [%s] , with ticket[%s]"),
							ex.ice_name().c_str(), id.name.c_str(), env->getCommunicator()->proxyToString(ticketPrx).c_str() );
					}
					catch(...)
					{
						ENVLOG(ZQ::common::Log::L_ERROR,CLOGFMT(RenewTicketCenter,"unknown exception caught"));
					}
					
					scheduleRenewCommand( id );

				}while(0);
			}
			if(mbQuit) break;
		}while(true);
	}
	return 1;
}

void RenewTicketCenter::scheduleRenewCommand( const Ice::Identity& id , int multi )
{
	if( !mSvcImpl.isObjectExist(id) )
	{
		doUnregister(id);
		return;
	}

	RenewTicketTask* pTask = new RenewTicketTask(*this, id );
	
	Ice::Int configInterval = env->getConfig().iRenewTicketInterval;
	
	configInterval = configInterval < 5000 ? 5000 : configInterval;

	if ( multi < 0)
	{
		IceUtil::Time	interval = IceUtil::Time::milliSeconds(  200 );
		env->getSceduler().schedule( id.name , TianShanIce::Streamer::TIMERRENEWTICKET,pTask, interval );
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(scheduleRenewCommand,"schedule session [%s] to renew ticket in [%lld] ms "),
				id.name.c_str() , interval.toMilliSeconds());
	}
	else
	{
		assert( configInterval > 1000 );//should I check it in release version ?
		IceUtil::Time	interval = IceUtil::Time::milliSeconds(  configInterval + (rand()%(configInterval)/2) * multi );
		env->getSceduler().schedule( id.name , TianShanIce::Streamer::TIMERRENEWTICKET,pTask, interval );
		ENVLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(scheduleRenewCommand,"schedule session [%s] to renew ticket in [%lld] ms , ticket renew interval configuration is [%d] "),
			id.name.c_str() , interval.toMilliSeconds() , env->getConfig().iRenewTicketInterval);
	}
}

void RenewTicketCenter::doRegister( const Ice::Identity& id , TianShanIce::Transport::PathTicketPrx ticket )
{
	{
		ZQ::common::MutexGuard gd( mMutex );
		mIdTicketMap[id] = ticket;
	}
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(doRegister,"register sess[%s] with Ticket[%s]"),
		id.name.c_str() , env->getCommunicator()->proxyToString(ticket).c_str() );
	scheduleRenewCommand( id , -1 );
}

void RenewTicketCenter::doUnregister( const Ice::Identity& id )
{
	ENVLOG(ZQ::common::Log::L_INFO,CLOGFMT(doUnregister,"unregister sess[%s] from RenewTicketCenter"),
		id.name.c_str() );
	{
		ZQ::common::MutexGuard gd( mMutex );
		mIdTicketMap.erase(id);
	}
}


}}

