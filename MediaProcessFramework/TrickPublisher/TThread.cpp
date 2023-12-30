// TThread.cpp: implementation of the TThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TThread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TThread::TThread()
{

}

TThread::~TThread()
{
	
}

bool TThread::start()
{
	DWORD dwThreadID;
	_handle = CreateThread(NULL, NULL, this->_execute, this, NULL, &dwThreadID);

	return (NULL != _handle);
}

//when the thread is running ,you can't invoke this,
//can't invoke in thread
void TThread::close()
{
	if (_handle)
	{
		CloseHandle(_handle);
		_handle = NULL;
	}	
}

bool TThread::wait(DWORD timeoutMillis)const
{
	DWORD result = ::WaitForSingleObject(_handle, timeoutMillis);
	if (result == WAIT_TIMEOUT)
		return false;
	
	if (result == WAIT_OBJECT_0)
		return true;

	return true;
}


void TThread::Sleep(DWORD msec)
{
	::SleepEx(msec, false);
}

void TThread::setStatus(int sts)
{
	_status = (Status)sts;
}

