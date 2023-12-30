
#include <ZQ_common_conf.h>
#include <TianShanDefines.h>
#include "NgodScheduler.h"
#include "SelectionResourceManager.h"
#include "NgodSessionManager.h"
#include "NgodEnv.h"

namespace NGOD
{

ScheduleRunner::ScheduleRunner( NgodSessionManager& manager , ZQ::common::NativeThreadPool& pool , const std::string& sessId )
:ZQ::common::ThreadRequest(pool),
mSessManager(manager),
mSessId(sessId)
{
}
ScheduleRunner::~ScheduleRunner()
{

}
void ScheduleRunner::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	delete this;
}
int ScheduleRunner::run()
{
	NGOD::NgodSessionPrx sess = mSessManager.openSession( mSessId );
	if( sess )
	{
		try
		{
			sess->onTimer();
		}
		catch( const Ice::Exception& )
		{
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///
NgodScheduler::NgodScheduler( NgodSessionManager& manager , ZQ::common::NativeThreadPool& pool  )
:mSessManager(manager),
mPool(pool),
mbQuit(false)
{
}

NgodScheduler::~NgodScheduler(void)
{
}

bool NgodScheduler::start()
{
	mbQuit = false;
	return ZQ::common::NativeThread::start();
}

void NgodScheduler::stop()
{
	mbQuit = true;
	mSem.post();
	waitHandle(50*1000);
}
void NgodScheduler::scheduleAt( const std::string& id , int64 interval )
{
	if( mbQuit)
		return;
	int64 target = ZQTianShan::now() + interval;
	bool bSignal = false;
	{
		ZQ::common::MutexGuard gd( mMutex );
		if( target <= mNextWakeup )
			bSignal = true;
		Key k;
		k.sessId	= id;
		k.targetTime= target;

		KeyMap::iterator it = mKeys.find(id);
		if( it != mKeys.end() )
		{
			mObjs.erase( it->second );
			it->second = k;
		}
		else
		{
			mKeys.insert( KeyMap::value_type( id , k ) );
		}
		mObjs.insert( k );
	}
	if( bSignal )
		mSem.post();
}

void NgodScheduler::cancel( const std::string& id )
{
	ZQ::common::MutexGuard gd(mMutex);
	KeyMap::iterator it = mKeys.find(id);
	if( it == mKeys.end() )
		return;
	mObjs.erase( it->second );
	mKeys.erase(it);
}

int NgodScheduler::run( )
{
	int32 defaultWait = 600 * 1000;
	int32 waitInterval = defaultWait;
	std::string sessId;
	while(!mbQuit)
	{		
		while(!mbQuit)
		{
			int64 now = ZQTianShan::now();
			{
				ZQ::common::MutexGuard gd(mMutex);

				if( mObjs.empty() )
				{
					sessId = "";
					waitInterval =defaultWait;	
					mNextWakeup = now + waitInterval;
					break;
				}
				else
				{
					ScheduleSet::iterator it = mObjs.begin();
					if( it->targetTime <= now )
					{
						sessId = it->sessId;
						mObjs.erase(it);
						mKeys.erase(sessId);
					}
					else
					{
						sessId = "";
						waitInterval = (int32)(it->targetTime - now);
						waitInterval = max(5,waitInterval);
						mNextWakeup = now + waitInterval;
						break;
					}
				}				
			}
			if( !sessId.empty() )
			{
				(new ScheduleRunner(mSessManager,mPool,sessId))->start();
			}
		}
		mSem.timedWait( waitInterval );
	}
	return 0;
}

}

