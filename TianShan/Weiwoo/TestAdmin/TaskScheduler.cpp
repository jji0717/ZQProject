// TaskScheduler.cpp: implementation of the TaskScheduler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scriptrequest.h"
#include "TaskScheduler.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaskScheduler::TaskScheduler(ZQ::common::NativeThreadPool& pool):_pool(pool)
{
	_bQuit = false;
	_hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
}
TaskScheduler::~TaskScheduler()
{
	_bQuit = true;
	SetEvent(_hEvent);
	waitHandle(1000);
	CloseHandle(_hEvent);
}
void TaskScheduler::watchme(void* task,unsigned long schedule)
{
	ZQ::common::MutexGuard gd(_lockMap);
	_taskMap[task] = schedule;
	if (_maxWait > schedule-GetTickCount()) 
	{
		SetEvent(_hEvent);
	}	
}
bool TaskScheduler::init()
{
	return true;
}
int TaskScheduler::run()
{
#define MAX_WAIT	1000*1000
	_maxWait = MAX_WAIT;
	std::vector<ScriptRequest*>	_vecScript;;
	while(!_bQuit)
	{
		_vecScript.clear();
		WaitForSingleObject(_hEvent,_maxWait);
		if (_bQuit){ break;}
		//find out all expired task
		{
			ZQ::common::MutexGuard gd(_lockMap);
			_maxWait = MAX_WAIT;
			DWORD	dwNow = GetTickCount();
			TASKMAP::iterator it = _taskMap.begin();
			for ( ; it!=_taskMap.end() ; it++) 
			{
				if ( it->second <= dwNow ) 
				{
					_vecScript.push_back((ScriptRequest*)it->first);
				}
				else if (_maxWait > ( it->second - dwNow)) 
				{
					_maxWait = it->second - dwNow;
				}
			}
			std::vector<ScriptRequest*>::iterator itVec = _vecScript.begin();
			for ( ; itVec != _vecScript.end() ; itVec++ ) 
			{
				_taskMap.erase(*itVec);
			}

		}
		{
			std::vector<ScriptRequest*>::iterator it = _vecScript.begin();
			for ( ; it != _vecScript.end() ; it++ ) 
			{
				(*it)->start();
			}
		}
	}
	return 1;
}
void TaskScheduler::final()
{
}
