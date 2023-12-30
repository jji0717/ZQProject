// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: TriggerWorker.h,v 1.7 2004/07/29 06:26:30 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : impl the TriggerWorker
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/TriggerWorker.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 10    07-03-09 12:03 Ken.qian
// 
// 9     06-08-01 18:32 Ken.qian
// Move from comextra to common
// 
// 8     05-10-24 15:09 Bernie.zhao
// move to Lap. support TimewindowThreshold
// 
// 7     05-04-26 15:14 Kaliven.lee
// reset the finished task
// 
// 6     05-04-13 19:17 Kaliven.lee
// thread class change
// 
// 5     05-04-01 16:23 Kaliven.lee
// 3.3.0 patch
// modify the borderline to decide if a worker is busy.
// 
// 4     05-03-30 17:09 Kaliven.lee
// 
// 3     05-03-25 16:16 Kaliven.lee
// 3.2.15
// 
// 2     05-03-17 19:42 Kaliven.lee
// 3.2.11
// 
// 1     05-03-07 15:29 Kaliven.lee
// file create

// TriggerWorker.cpp: implementation of the TriggerWorker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TriggerWorker.h"
#include "DSInterface.h"

#define WORKER_TIMEOUT	5000
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//std::map<DWORD,DWORD> TriggerWorker::m_AssetsLock;
extern ZQ::common::Log * gpDbSyncLog;
extern CDSInterface g_ds;
DWORD TriggerWorker::m_ID = 0;
TriggerWorker::TriggerWorker():ZQ::common::NativeThread(),
m_bThreadExit(false)
{
	m_hStop = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	m_bQuit = false;
	m_bProccessing = false;
	m_dwID = ++m_ID;
}

TriggerWorker::~TriggerWorker()
{
	if(!m_bQuit)
	{
		Stop();
	}
	if(m_hStop)
	{
		CloseHandle(m_hStop);
	}
	if(m_hEvent)
	{
		CloseHandle(m_hEvent);
	}
	
}

int TriggerWorker::run(void)
{
	HANDLE hEvents[2];
	hEvents[0] = m_hStop;
	hEvents[1] = m_hEvent;
	DWORD rtn = 0;
	for(;!m_bQuit;)
	{
		rtn  = ::WaitForMultipleObjects(2,hEvents,FALSE,WORKER_TIMEOUT);
		if(m_bQuit)
			break;
		switch(rtn) {
		case WAIT_TIMEOUT:
			break;
		case WAIT_OBJECT_0:
			return 0;
			break;
		case WAIT_OBJECT_0 + 1:
			if(m_bQuit)
				break;
			

			(*gpDbSyncLog)(ZQ::common::Log::L_INFO,_T("%d:Start to processing Detail entry UID = %d,type = %d"),
				GetCurrentThreadId(),m_task.dwEntryUID,m_task.dwEntryType);
			
			switch(m_task.dwEntryType)
			{
				case IDS_ASSET:
				{
					
				g_ds.SaveObjectMdToDB(m_task.dwEntryUID,m_task.dwEntryType);
				g_ds.SaveElementsToDB(m_task.dwEntryUID, FALSE);
//#pragma message (__TODO__ "add work queue here to notify NSSync that a new asset with md list added")			
				break;
				}
				case IDS_FOLDER:
				g_ds.SaveObjectMdToDB(m_task.dwEntryUID,m_task.dwEntryType);
				g_ds.SaveEntriesToDB(m_task.dwEntryUID,m_task.dwEntryType);
				break;
				
				case IDS_APPLICATION:
				{					
					g_ds.SaveAppsToDB();
					break;
				}
				
				default:
					break;
			}
			(*gpDbSyncLog)(ZQ::common::Log::L_INFO,_T("%d:End to processing Detail entry UID = %d,type = %d"),
				GetCurrentThreadId(),m_task.dwEntryUID,m_task.dwEntryType);
			m_task.dwEntryType = 0;
			m_task.dwEntryUID = 0;
			m_bProccessing = false;
			break;
		default:
			break;
		}
	}
	m_bThreadExit = true;
	return 0;
}

void TriggerWorker::SetProcTask(DETAILTASK* task)
{
	m_task = *task;
}
void TriggerWorker::Stop(void)
{
	m_bQuit = true;
	::SetEvent(m_hStop);
	while(!m_bThreadExit)
	{
		::Sleep(500);
	}
	terminate(0);
}
void TriggerWorker::StartProc(DETAILTASK* task)
{
	m_bProccessing = true;
	SetProcTask(task);
	::SetEvent(m_hEvent);
}
bool TriggerWorker::IsProccessing(void)
{
	return m_bProccessing;
}
