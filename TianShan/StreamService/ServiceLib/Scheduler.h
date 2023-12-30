
#ifndef __ZQ_StreamService_Scheduler_header_file_h__
#define __ZQ_StreamService_Scheduler_header_file_h__

#include <Locks.h>
#include <set>
#include <map>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

#include <NativeThread.h>
#include <NativeThreadPool.h>

#include "Playlist.h"

namespace ZQ
{
namespace StreamService
{

class SsEnvironment;

class ScheduleTask : public Ice::Object
{
public:
	
	virtual void	runTask( ) = 0;

private:
	friend class Scheduler;
	void			updateTargetTime( const IceUtil::Time& target )
	{
		mTargetTime = target;
	}

	const IceUtil::Time&	getTargetTime( ) const
	{
		return mTargetTime;
	}
private:
	
	IceUtil::Time			mTargetTime;

};

typedef IceUtil::Handle<ScheduleTask> ScheduleTaskPtr;

class DPCThreadRequest : public ZQ::common::ThreadRequest
{
public:
	DPCThreadRequest( const Ice::Identity&				id,  
						SsEnvironment*					environment  , 
						ZQ::common::NativeThreadPool&	pool );

	virtual ~DPCThreadRequest( );

public:
	
	int		run( void );

	void	final(int retcode =0, bool bCancelled =false);


private:
	SsEnvironment*							env ;
	Ice::Identity							mStreamId ;
};

class DPCScheduleTask : public ScheduleTask
{
public:
	DPCScheduleTask( const Ice::Identity& id , SsEnvironment* environment );	
	
	virtual ~DPCScheduleTask( );

private:

	virtual void	runTask( );

private:
	SsEnvironment*							env;
	Ice::Identity							mStreamId;
};

class DestroyPLRequest : public ZQ::common::ThreadRequest
{
public:
	DestroyPLRequest( ZQ::common::NativeThreadPool& pool ,  SsEnvironment* environment ,  const Ice::Identity& id  );
	virtual ~DestroyPLRequest( );
public:
	
	int				run(void);

	void			final(int retcode  =0 , bool bCancelled  =false )
	{
		delete this;
	}

private:	
	SsEnvironment*					env;
	Ice::Identity					mPlaylistId;
};

class Scheduler : public ZQ::common::NativeThread
{
public:
	Scheduler( SsEnvironment*	environemnt);
	virtual ~Scheduler();

public:	

	bool			start( );

	void			stop( );

	void			schedule( const std::string& id , const TianShanIce::Streamer::Timertype& type, 
									ScheduleTaskPtr Task , const IceUtil::Time& interval );

	void			scheduleAt(  const std::string& id , const TianShanIce::Streamer::Timertype& type, 
									ScheduleTaskPtr Task , const IceUtil::Time& targetTime );

	void			cancelSchedule( const std::string& id , const TianShanIce::Streamer::Timertype& type );


protected:

	int				run( );

private:
	SsEnvironment*			env;
	
	bool					mbStop;

	IceUtil::Time			mNextWakeupTime;

	ZQ::common::Semaphore	mTasksSem;
	ZQ::common::Mutex		mTasksMutex;

	struct ScheduleID 
	{	
		ScheduleID( )
			:idString(""),
			type(TianShanIce::Streamer::TIMERNULL)
		{			
		}
		ScheduleID( const std::string& id , const TianShanIce::Streamer::Timertype t)
			:idString(id),
			type(t)
		{
		}
		ScheduleID( const ScheduleID& d )
		{
			type = d.type;
			idString = d.idString;
		}
		std::string									idString;		
		TianShanIce::Streamer::Timertype			type;		
	};

	struct ScheduleIDCmp
	{
		bool operator()(const ScheduleID& _Left, const ScheduleID& _Right) const
		{	
			if ( _Left.idString < _Right.idString )
			{
				return true;
			}
			else if(_Left.idString == _Right.idString)
			{
				return  _Left.type < _Right.type ;				
			}
			else 
			{
				return false;
			}
		}
	};
	struct ScheduleSortID : public ScheduleID
	{
		IceUtil::Time			targetTime;
		ScheduleSortID( )
			:targetTime(IceUtil::Time::milliSeconds(0))
		{			
		}
		ScheduleSortID( const std::string& id , const TianShanIce::Streamer::Timertype t , const IceUtil::Time& tm )
			:ScheduleID(id,t),
			targetTime(tm)
		{
		}
		ScheduleSortID( const ScheduleSortID& d )
		{
			type		= d.type;
			idString	= d.idString;
			targetTime	= d.targetTime;
		}
		
	};
	struct ScheduleSortIDCmp
	{
		bool operator()(const ScheduleSortID& _Left, const ScheduleSortID& _Right) const
		{
			if( _Left.targetTime < _Right.targetTime )
			{
				return true;
			}
			else if ( _Left.targetTime == _Right.targetTime )
			{
				if ( _Left.idString < _Right.idString )
				{
					return true;
				}
				else if(_Left.idString == _Right.idString)
				{
					return  _Left.type < _Right.type ;				
				}
				else 
				{
					return false;
				}
			}
			else
			{
				return false;
			}
			
		}
	};
	typedef std::map< ScheduleSortID , ScheduleTaskPtr , ScheduleSortIDCmp > TASKMAP;
	TASKMAP					mTasks;
	typedef std::map<ScheduleID , ScheduleSortID  , ScheduleIDCmp >	 IDMAP;
	IDMAP					mIds;
};

}}//namespace ZQ::StreamService

#endif//__ZQ_StreamService_Scheduler_header_file_h__
