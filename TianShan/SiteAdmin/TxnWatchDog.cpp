#include "TimeUtil.h"
#include "SiteAdminEnv.h"
#include "TxnWatchDog.h"

TxnWatchDog::TxnWatchDog( SiteAdminEnv& env ,SiteAdminDb& db)
:mEnv(env),
mDb(db)
{
	mNextWakeup = 0;
	mbQuit		= false;
}

TxnWatchDog::~TxnWatchDog( )
{
	stop();
}
void TxnWatchDog::stop()
{
	if(!mbQuit)
	{
		mbQuit =true;
		mEvent.signal();
		waitHandle( 100 * 1000 );
	}
}
void TxnWatchDog::WatchMe( const std::string& sessId , int64 interval )
{
	int64 target = ZQ::common::now() + interval;
	OBJINFO info;
	info.sessId		= sessId;
	info.targetTime	= target;
	bool bSignal = false;
	{
		ZQ::common::MutexGuard gd(mLocker);
		OBJMAP::iterator it = mObjMap.find(sessId);
		if( it != mObjMap.end() )
		{
			mObjSet.erase( it->second );
			mObjMap.erase( it );
		}
		mObjMap.insert( OBJMAP::value_type( sessId , info ) );
		mObjSet.insert( info );
		bSignal = target < mNextWakeup;
	}
	if( bSignal )
		mEvent.signal();
}

void TxnWatchDog::UnWatchMe( const std::string& sessId )
{
	ZQ::common::MutexGuard gd(mLocker);
	OBJMAP::iterator it = mObjMap.find(sessId);
	if( it != mObjMap.end() )
	{
		mObjSet.erase( it->second );
		mObjMap.erase( it );
	}
}

int TxnWatchDog::run()
{	
	timeout_t interval = 1000;
	mNextWakeup = 24 * 3600 * 1000 + ZQ::common::now();
	while(!mbQuit )
	{
		std::string sessId;
		do
		{
			sessId.clear();
			{
				int64 cur = ZQ::common::now();
				interval = 1000;
				ZQ::common::MutexGuard gd(mLocker);
				if( mObjSet.empty() )
				{
					mNextWakeup = cur + interval;
				}
				else
				{
					const OBJINFO& info = (*mObjSet.begin());
					if( info.targetTime <= cur)
					{
						sessId = info.sessId;
						mObjSet.erase( mObjSet.begin() );
						mObjMap.erase( sessId );
					}
					else
					{
						if( ( info.targetTime - cur )> 60 * 1000 )
						{
							interval = 60000;
						}
						else
						{
							interval = (timeout_t)(info.targetTime - cur) ;
						}
					}
					mNextWakeup = cur + interval;
				}
			}

			if( mbQuit ) break;

			if( !sessId.empty() )
			{
				(new TxnTimerTask(mEnv,mDb,sessId))->start();
			}
		}while( !sessId.empty() );

		if( interval <= 10 )
			interval = 10;

		mEvent.wait( (timeout_t)interval ); 
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///TxnTimerTask
TxnTimerTask::TxnTimerTask( SiteAdminEnv& env , SiteAdminDb& db , const std::string& sessId )
:mEnv(env),
mDb(db),
mSessId(sessId),
ZQ::common::ThreadRequest( env.getMainThreadpool() )
{
}
TxnTimerTask::~TxnTimerTask()
{

}
void TxnTimerTask::final(int retcode /* =0 */, bool bCancelled /* =false */)
{
	delete this;
}
int TxnTimerTask::run()
{
	try
	{
		TianShanIce::Site::LiveTxnPrx txn = mDb.openTxn( mSessId );
		if(!txn)
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(TxnTimerTask,"run() txn[%s] is gone"),mSessId.c_str() );
			return 0;
		}
		txn->onTimer();
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(TxnTimerTask,"run() caught [%s] for sess[%s]"), ex.ice_name().c_str() , mSessId.c_str());
	}
	catch( ... )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TxnTimerTask,"run() caught unknown exception for sess[%s]"), mSessId.c_str() );
	}
	return 0;
}
