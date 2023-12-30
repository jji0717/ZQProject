// TaskScheduler.h: interface for the TaskScheduler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASKSCHEDULER_H__8F05FBD3_7143_4CC5_B859_FCDABA9703CE__INCLUDED_)
#define AFX_TASKSCHEDULER_H__8F05FBD3_7143_4CC5_B859_FCDABA9703CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <string>
#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Locks.h>

class TaskScheduler  : public ZQ::common::NativeThread
{
public:
	TaskScheduler(ZQ::common::NativeThreadPool& pool);
	virtual ~TaskScheduler();
public:
	bool		init();
	int			run(void);
	void		final();

	void		watchme(void* task,unsigned long schedule);
	
private:
	typedef std::map<void*,unsigned long>	TASKMAP;
	TASKMAP									_taskMap;
	ZQ::common::Mutex						_lockMap;
	ZQ::common::NativeThreadPool&			_pool;
	bool									_bQuit;
	HANDLE									_hEvent;
	unsigned long							_maxWait;
};

#endif // !defined(AFX_TASKSCHEDULER_H__8F05FBD3_7143_4CC5_B859_FCDABA9703CE__INCLUDED_)