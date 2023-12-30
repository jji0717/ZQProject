
#ifndef _tianshan_ngod_scheduler_header_file_h__
#define _tianshan_ngod_scheduler_header_file_h__

#include <ZQ_common_conf.h>
#include <Locks.h>
#include <NativeThreadPool.h>
#include <map>
#include <set>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>


namespace NGOD
{

class NgodSessionManager;
class ScheduleRunner : public ZQ::common::ThreadRequest
{
public:
	ScheduleRunner( NgodSessionManager& manager , ZQ::common::NativeThreadPool& pool , const std::string& sessId );
	virtual ~ScheduleRunner();

protected:
	
	int			run();
	
	void		final(int retcode /* =0 */, bool bCancelled /* =false */);
	
	
protected:
	NgodSessionManager&		mSessManager;
	std::string				mSessId;
};

class NgodScheduler : public ZQ::common::NativeThread
{
public:
	NgodScheduler( NgodSessionManager& manager , ZQ::common::NativeThreadPool& pool );
	virtual ~NgodScheduler(void);

public:

	bool start( );

	void stop( );

	void scheduleAt( const std::string& id , int64 interval );

	void cancel( const std::string& id );

protected:
	
	int		run( );

private:

	NgodSessionManager&			mSessManager;
	ZQ::common::NativeThreadPool& mPool;
	
	struct  Key
	{
		int64			targetTime;
		std::string		sessId;		
		bool operator<( const Key& b ) const
		{
			if( targetTime < b.targetTime )
				return true;
			else if( targetTime == b.targetTime )
			{
				return sessId < b.sessId;
			}
			else
				return false;
		}
	};

	typedef std::map<std::string , Key> KeyMap;
	KeyMap			mKeys;
	typedef std::set<Key>	ScheduleSet;
	ScheduleSet		mObjs;

	ZQ::common::Mutex			mMutex;
	ZQ::common::Semaphore		mSem;
	int64						mNextWakeup;
	bool						mbQuit;
};

}//namespace NGOD

#endif//_tianshan_ngod_scheduler_header_file_h__
