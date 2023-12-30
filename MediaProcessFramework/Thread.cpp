/**********************************************************
 $Id: Thread.cpp,v 1.7 2003/06/12 18:04:41 shao Exp $
 $Author: Admin $
 $Revision: 1 $
 $Date: 10-11-12 16:01 $

Copyright (C) 2002-2003 Hui Shao.
All Rights Reserved, Hui Shao.
**********************************************************/

#include "Thread.h"

MPF_NAMESPACE_BEGIN

Thread::Thread()
:_hThread(NULL), _Status(DEFERRED)
{

	memset(_msgbuf,0,512);

	setStatus(INITIAL);

	_evtExit=CreateEvent(NULL,FALSE,FALSE,NULL);
}

Thread::~Thread()
{
	terminate();
}

bool Thread::isRunning(void)
{
	DWORD nStatus;
	if(GetExitCodeThread(_hThread,&nStatus))
	{
		if(nStatus==STILL_ACTIVE)
			return true;
	}
	
	return false;
}

void Thread::terminate(void)
{
	if(isRunning())
	{
		SetEvent(_evtExit);
		WaitForSingleObject(_hThread,-1);
		CloseHandle(_hThread);
		_hThread=NULL;

		setStatus(DISABLED);
	}
}

DWORD __stdcall Thread::_execute(void *thread)
{
	Thread *th = (Thread *)thread;
	if(th==NULL)return 0;

	try
	{
		if (th->init())
		{
			th->setStatus(RUNNING);
			th->run();
		}
		
		
		th->setStatus(DISABLED);
			
		th->final();
		
		
	}
	catch(...)
	{
		th->final();
		printf("&&&&&&&&&&&&&&&&exit thread _execute by exeption!\n");
		//log
	}

	return 0;
}

bool Thread::start()
{
	DWORD dwID;
	_hThread=CreateThread(NULL,NULL,_execute,(void*)this,
										NULL,&dwID);

	return (_hThread!=NULL);
}


bool Thread::waitExit(long timeout)
{
	if(_evtExit)
		if(WaitForSingleObject(_evtExit,timeout)==WAIT_OBJECT_0)
			return true;
	
	return false;
}


MPF_NAMESPACE_END

