#ifndef __tianshan_siteadmin_timer_watchdog_header_file_h__
#define __tianshan_siteadmin_timer_watchdog_header_file_h__

#include <map>
#include <set>
#include <Locks.h>
#include <NativeThread.h>
#include <NativeThreadPool.h>
#include "SystemUtils.h"
#include "SiteAdminDatabase.h"

class SiteAdminEnv;
class SiteAdminDb;

class TxnWatchDog : public ZQ::common::NativeThread
{
public:
	TxnWatchDog( SiteAdminEnv& env , SiteAdminDb& db );
	virtual ~TxnWatchDog(void);

	void	WatchMe( const std::string& sessId , int64 interval );

	void	UnWatchMe( const std::string& sessId );

	void	stop();

protected:

	int		run();	

private:

	SiteAdminEnv&				mEnv;
	SiteAdminDb&				mDb;

	SYS::SingleObject			mEvent;
	bool						mbQuit;
	typedef struct  _OBJINFO
	{
		int64			targetTime;
		std::string		sessId;
		bool operator < (const _OBJINFO& b) const
		{
			if( targetTime < b.targetTime )
				return true;
			else if( targetTime == b.targetTime )
				return sessId < b.sessId;
			else
				return false;
		}
	}OBJINFO;
	typedef std::map< std::string, OBJINFO > OBJMAP;
	typedef std::set<OBJINFO> OBJSET;

	OBJMAP				mObjMap;
	OBJSET				mObjSet;	
	int64				mNextWakeup;
	ZQ::common::Mutex	mLocker;
};

class TxnTimerTask : public ZQ::common::ThreadRequest
{
public:
	TxnTimerTask( SiteAdminEnv& env , SiteAdminDb& db, const std::string& sessId );
	virtual ~TxnTimerTask();
protected:
	int		run();
	void	final(int retcode /* =0 */, bool bCancelled /* =false */);
private:
	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
	std::string				mSessId;	
};

#endif//__tianshan_siteadmin_timer_watchdog_header_file_h__

