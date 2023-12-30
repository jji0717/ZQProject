
#include "TxnTransfer.h"
#include "SiteAdminEnv.h"
#include "SiteAdminDatabase.h"

TxnTransfer::TxnTransfer( SiteAdminEnv& env , SiteAdminDb& db)
:mEnv(env),
mDb(db)
{
	mbQuit = false;
}

TxnTransfer::~TxnTransfer(void)
{
}

void TxnTransfer::pushSess( const std::string& sessId )
{
	{
		ZQ::common::MutexGuard gd(mLocker);
		mSessIds.push_back(sessId);
	}
	mEvent.signal();
}

void TxnTransfer::stop( )
{
	mbQuit = true;
	mEvent.signal();
	waitHandle( 100 * 1000 );
}

int TxnTransfer::run( )
{
	while( !mbQuit )
	{
		while(true)
		{
			std::string sessId;
			{
				ZQ::common::MutexGuard gd(mLocker);
				if( mSessIds.empty())
					break;
				sessId = mSessIds.front();
				mSessIds.pop_front();
			}
			if( mbQuit ) break;
			if( !sessId.empty() )
			{
				TianShanIce::Site::LiveTxnPtr txn = mDb.removeTxn(sessId);
				if( txn )
				{//do some thing here ?
#pragma message(__MSGLOC__"TODO: should I move the txn instance into YTD data base ?")
				}
			}
		}
		mEvent.wait( 10* 1000 );
	}
	return 0;
}

