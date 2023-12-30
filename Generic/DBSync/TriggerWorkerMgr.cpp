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
// Ident : $Id: TriggerWorkerMgr.cpp,v 1.7 2004/07/29 06:26:30 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : impl the TriggerWorkerMgr
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/DBSync/TriggerWorkerMgr.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 12    06-08-01 18:32 Ken.qian
// Move from comextra to common
// 
// 11    05-05-09 11:26 Kaliven.lee
// 
// 10    05-04-27 10:51 Kaliven.lee
// 
// 9     05-04-26 15:09 Kaliven.lee
// avoid process same object by serveral thread
// 
// 8     05-04-13 19:17 Kaliven.lee
// thread class change
// 
// 7     05-04-07 19:17 Kaliven.lee
// 
// 6     05-04-01 16:23 Kaliven.lee
// 3.3.0 patch
// modify the borderline to decide if a worker is busy.
// 
// 5     05-03-30 17:09 Kaliven.lee
// 
// 4     05-03-25 16:16 Kaliven.lee
// 3.2.15
// 
// 3     05-03-17 19:42 Kaliven.lee
// 3.2.11
// 
// 2     05-03-14 17:48 Kaliven.lee
// 3.14
// 
// 1     05-03-07 15:29 Kaliven.lee
// file create
// TriggerWorkerMgr.cpp: implementation of the TriggerWorkerMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TriggerWorkerMgr.h"
#include "DSInterface.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define		WORKMANAGER_TIMEOUT		1000
#define		SCANSLEELP_TIMEOUT		3000

extern CDSInterface g_ds;
extern ZQ::common::Log * gpDbSyncLog;
using ZQ::common::Log;

TriggerWorkerMgr::TriggerWorkerMgr(DWORD dwWorkerCount):
ZQ::common::NativeThread(),
m_bQuit(false),
m_bThreadExit(false)
{
	m_hStop = ::CreateEvent(NULL,false,false,_T(""));
	m_dwWorkerCount = dwWorkerCount;
}

TriggerWorkerMgr::~TriggerWorkerMgr()
{
	if(m_bQuit)
	{
		stop();
	}
	if(m_hStop != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hStop);
	}
}
int TriggerWorkerMgr::stop(void)
{
	m_bQuit = true;
	::SetEvent(m_hStop);
	while(!m_bThreadExit)
	{
		Sleep(500);
	}	
	terminate(0);
	return 0;
}
int TriggerWorkerMgr::run(void)
{
	for(DWORD i = 0;i < m_dwWorkerCount; i++)
	{
		TriggerWorker* pWork = new TriggerWorker;
		if(	pWork->start())
			m_workerList.push_back(pWork);
	}
	DWORD Rtn = 0;
	while(!m_bQuit)
	{
		Rtn = ::WaitForSingleObject(m_hStop,WORKMANAGER_TIMEOUT);
		if(m_bQuit)
			break;
		switch(Rtn)
		{
			case WAIT_TIMEOUT:
			{	
				DETAILTASK* pTask = NULL;
				TriggerWorker *pWorker;
				bool bFound = false;
				while(!g_ds.m_WorkQueue.empty())
				{
					if(m_bQuit)
						break;
					g_ds.m_WQLock.enter();
					pTask = g_ds.m_WorkQueue.front();
					(*gpDbSyncLog)(Log::L_INFO,_T("Worker Manager try to process %d:%d"),pTask->dwEntryType,pTask->dwEntryUID);
					bFound = false;
					// check the working thread if a same task is allocate to process
					
					for(DWORD j = 0;j < m_workerList.size();j ++)
					{
						pWorker = m_workerList.at(j);
						if((pWorker->m_task.dwEntryUID == pTask->dwEntryUID)&&(pWorker->m_task.dwEntryType == pTask->dwEntryType))
							bFound = true;
					}
					if(bFound)
					{
						(*gpDbSyncLog)(Log::L_INFO,_T("Worker Manager found a thread already process %d:%d.Discard this task."),pTask->dwEntryType,pTask->dwEntryUID);
						g_ds.m_WorkQueue.pop();
						g_ds.m_WQLock.leave();
						continue;
					}
					
					for(DWORD i = 0; i < m_workerList.size(); i++)
					{
						if(m_bQuit)
						{							
							break;
						}
						pWorker = m_workerList.at(i);
						if(pWorker->IsProccessing())
							continue;
						(*gpDbSyncLog)(Log::L_INFO,_T("Worker Manager found a idle worker %d.And dispatch this worker to process %d:%d"),i+1,pTask->dwEntryType,pTask->dwEntryUID);
						
						pWorker->StartProc(pTask);
						bFound = true;
						{						
							
							g_ds.m_WorkQueue.pop();
							
						}
						delete pTask;
						break;
					}
					//(*gpDbSyncLog)(Log::L_INFO,_T("Worker Manager exit to process %d:%d"),pTask->dwEntryType,pTask->dwEntryUID);
					g_ds.m_WQLock.leave();
					if(!bFound)
						Sleep(SCANSLEELP_TIMEOUT);
				}
				break;
			}
			case WAIT_OBJECT_0:
				//return 0;
				m_bQuit = true;
				break;			
			default:
				break;
		}		
	}
	
	TriggerWorker * pWorker = NULL;
	while(m_workerList.size() != 0)
	{
		pWorker = m_workerList.at(m_workerList.size() - 1);
		pWorker->Stop();
		delete pWorker;
		m_workerList.pop_back();
	}
	m_bThreadExit = true;
	return 0;
}
