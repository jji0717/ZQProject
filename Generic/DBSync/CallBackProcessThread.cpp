// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: CallBackProcessThread.cpp,v 1.9 2006/05/19 07:24:54 Ken Qian $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : Define the encapsulated class for IDS callback data
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/CallBackProcessThread.cpp $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 11    09-02-24 6:21 Ken.qian
// fix ACE-3033 - the updateAll() does not complete since the thread was
// exit caused by memory access violation
// 
// 10    09-01-04 11:01 Ken.qian
// change log position
// 
// 9     08-06-04 4:06 Ken.qian
// fix the dead loop in Callback Process thread
// 
// 8     08-04-03 14:50 Ken.qian
// mutex change in run() 
// 
// 7     07-05-14 16:04 Ken.qian
// 
// 6     07-04-12 17:35 Ken.qian
// Change DBSync auto-restart logic
// 
// 5     07-03-13 19:23 Ken.qian
// 
// 4     06-12-26 22:19 Ken.qian
// 
// 3     06-12-25 13:50 Ken.qian
// 
// 2     06-10-19 14:45 Shuai.chen
// 
// 1     06-05-22 13:59 Ken.qian
// It is avaliable since DBSync3.6.0 to support taking callback while
// doing the full synchronization
// 
//
// Revision 1.0  2006/05/19 16:20:20  Ken Qian
// Initial codes, it is avaliable since DBSync 3.6.0
//
// ===========================================================================

#include "stdafx.h"
#include "DBSyncServ.h"
#include "CallBackProcessThread.h"

extern ZQ::common::Log* gpDbSyncLog;
extern DBSyncServ g_server;

bool m_isBuy = false;
using ZQ::common::Log;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CallBackProcessThread::CallBackProcessThread()
{
	// the event creation must be here, can NOT put into init()
	// coz the thread thread is started after updateAll(), 
	// only after start(), the init() can be invoked.
	m_hStop = CreateEvent(NULL, false, false, NULL);
	if(m_hStop == NULL)
	{
		(*gpDbSyncLog)(Log::L_ERROR, L"Failed to Create Stop Event for CallBack Processing thread");
	}

	m_hNotify = CreateEvent(NULL, false, false, NULL);
	if(m_hNotify == NULL)
	{
		(*gpDbSyncLog)(Log::L_ERROR, L"Failed to Create Notify Event for CallBack Processing thread");
	}

	m_bContinue =  true;
}

CallBackProcessThread::~CallBackProcessThread()
{
	if(isRunning())
	{
		StopProcessing();
		waitHandle(100);
	}
}

bool CallBackProcessThread::init(void)
{
	return true;
}

int CallBackProcessThread::run(void)
{
	(*gpDbSyncLog)(Log::L_DEBUG, L"Entering CallBackProcessThread::run()");
	
	m_bContinue = true;
	DWORD dwWaitStatus;

	HANDLE handles[2] = { m_hStop, m_hNotify};

	while(m_bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			m_bContinue = false;
			break;

		// received the notify event to indicate new callback coming in
		case WAIT_OBJECT_0 + 1:
			{
				while(m_bContinue)
				{	
					DSCallBackBase* pCallBack = NULL;
					{
						ZQ::common::MutexGuard guard(m_mutex);
						(*gpDbSyncLog)(Log::L_INFO, L"There are %d callbacks are queued", m_cbQueue.size());

						if(m_cbQueue.size() > 0)
						{
							ConnChecker* pChecker = g_server.GetConnChecker();
							if(pChecker != NULL && pChecker->isBroken())
							{
								(*gpDbSyncLog)(Log::L_INFO, L"IDS connection was not re-established yet, stop handle callbacks, wait for 5 seconds");
								Sleep(5000);
								continue;
							}

							pCallBack = m_cbQueue.front();
							m_cbQueue.pop();
						}
						else
						{
							break;
						}
					}

					if(pCallBack != NULL)
					{
						pCallBack->Process();						
						delete pCallBack;
					}
				}

				// do something in the idle time
				g_server.doIdle();
			}
			break;
		// received timeout or failed, exit the thread.
		case WAIT_TIMEOUT:
		case WAIT_FAILED:
		default:
			m_bContinue = false;
			break;
		}
	}
	(*gpDbSyncLog)(Log::L_DEBUG, L"Leaving CallBackProcessThread::run()");
	
	return 1;
}

void CallBackProcessThread::final(void)
{
	glog(Log::L_DEBUG, L"Entering CallBackProcessThread::final()");

	// free the memory in queue
	while(m_cbQueue.size() > 0)
	{
		DSCallBackBase* pCallBack = m_cbQueue.front();
		m_cbQueue.pop();

		delete pCallBack;
	}
	// close stop event handle
	if(m_hStop != NULL)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}

	// close the queue event
	if(m_hNotify != NULL)
	{
		CloseHandle(m_hNotify);
		m_hNotify = NULL;
	}
	glog(Log::L_DEBUG, L"Leaving CallBackProcessThread::final()");
}

void CallBackProcessThread::StopProcessing(void)
{
	m_bContinue = false;
	SetEvent(m_hStop);
}

bool CallBackProcessThread::AddCallback(DSCallBackBase* pCallback)
{
	m_mutex.enter();
	
	m_cbQueue.push(pCallback);
	
	SetEvent(m_hNotify);
	
	m_mutex.leave();

	return true;
}

void CallBackProcessThread::RemoveAllCallback()
{
	ZQ::common::MutexGuard guard(m_mutex);
	
	while(m_cbQueue.size() > 0)
	{
		DSCallBackBase* pCallBack = m_cbQueue.front();
		m_cbQueue.pop();

		delete pCallBack;
	}	
}

int  CallBackProcessThread::GetQueueSize(void)
{
	int qsize = 0;

	m_mutex.enter();

	qsize = m_cbQueue.size();

	m_mutex.leave();

	return qsize;
}
