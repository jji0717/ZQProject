// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : impl a native objected thread
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/nPVR/importAlarm/IsaAlarm/NativeThread.cpp 1     10-11-12 16:01 Admin $
// $Log: /ZQProjs/nPVR/importAlarm/IsaAlarm/NativeThread.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     06-01-10 16:05 Hongquan.zhang
// 
// 7     05-08-25 23:02 Bernie.zhao
// 
// 6     05-06-24 16:30 Bernie.zhao
// added try--catch in thread body
// 
// 5     05-06-15 20:17 Bernie.zhao
// fixed resource leak bug -> always call ::CloseHandle()
// 
// 4     05-04-22 12:12 Hongye.gu
// 
// 3     05-04-21 11:13 Daniel.wang
// 
// 1     4/14/05 6:50p Hui.shao
// ============================================================================================

#include "NativeThread.h"
#include "Log.h" // from macro SCRTRACE, DON'T log to file in base thread

//xiaobai
#ifndef SCRTRACE
#define SCRTRACE
#endif

extern "C"
{
	#include <process.h>
	#include <stdio.h>
};

namespace ZQ {
namespace common {

// typedef	unsigned (__stdcall *exec_t)(void *);

NativeThread::NativeThread(int stacksize)
       :_thrdID(0), _status(DEFERRED)
{
	_flags.B = 0;

//	_hThread = (HANDLE)CreateThread(NULL, stacksize, _execute, (void *)this, CREATE_SUSPENDED, (unsigned long *)&_thrdID);
	_hThread = (HANDLE)_beginthreadex(NULL, stacksize, _execute, (void *)this, CREATE_SUSPENDED, (unsigned*) &_thrdID);

#ifdef _DEBUG
//	printf(LOGFMT("tid=%d\n"), _thrdID);
#endif // _DEBUG
	
	if(_hThread >0)
		return;

	setStatus(INITIAL);
}

NativeThread::~NativeThread()
{
	terminate();
	try
	{
		CloseHandle(_hThread);
	}
	catch(...){}

	_hThread = INVALID_HANDLE_VALUE;
}

void NativeThread::exit()
{
	if (isThread())
	{
		setStatus(DISABLED);
		ExitThread(0);
	}
}

bool NativeThread::isRunning(void)
#ifdef WIN32
{
	return (_thrdID != 0) ? true : false;
}
#else
#  error have been implemented
#endif

bool NativeThread::isThread(void)
#ifdef WIN32
{
	return ((_thrdID == GetCurrentThreadId())) ? true : false;
}
#else
#  error have been implemented
#endif

void NativeThread::terminate(void)
{
#ifdef _DEBUG
//	printf(LOGFMT("tid=%d\n"), _thrdID);
#endif // _DEBUG

	if(!_thrdID)
	{
		return;
	}

	SCRTRACE;

	if(_thrdID == GetCurrentThreadId())
		return;

	SCRTRACE;

	bool terminated = false;

	try
	{
		if(!_flags.b.active)
			ResumeThread(_hThread);
	}
	catch(...){}

	SCRTRACE;

	try
	{
		TerminateThread(_hThread, 0);
		terminated = true;
	}
	catch(...){}

	WaitForSingleObject(_hThread, INFINITE);

	if (terminated)
	{
		SCRTRACE;
		this->final();
	}

	_thrdID = 0;
}

// unsigned long __stdcall NativeThread::_execute(void *thread)
unsigned __stdcall NativeThread::_execute(void *thread)
{
	NativeThread *th = (NativeThread *)thread;

	int ret = -1;

	try
	{
	
		if (th->init())
		{
			th->setStatus(RUNNING);
			ret = th->run();
		}

	} catch(...) {}

	th->setStatus(DISABLED);

	try
	{
		
		if (th->_thrdID > 0)
		{
			th->_thrdID = 0;

			SCRTRACE;
			th->final();
		}
		else ret = -2; // this thread was terminated

	} catch(...) {}

	return ret;
}

void NativeThread::setStatus(const status_t st)
{
	_status = st;
}

void NativeThread::sleep(timeout_t msec)
{
#ifdef WIN32
	::SleepEx(msec, false);
#else
#error TODO : non-windows implement here !!
#endif
}

bool NativeThread::start()
{
	return (ResumeThread(_hThread)!=-1);
}

void NativeThread::final(void)
{ 
	SCRTRACE;
	return; 
}

void NativeThread::suspend(void)
{
	SuspendThread(_hThread);
}

void NativeThread::resume(void)
{
	ResumeThread(_hThread);
}

DWORD NativeThread::waitHandle(timeout_t timeout)
{
	return WaitForSingleObject(_hThread,timeout);
}

} // namespace common
} // namespace ZQ

