
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
// Ident : $Id: FtpConnMgr.cpp,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : impl the Connection manager class FtpConnMgr
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpConnMgr.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 6     06-08-04 13:30 Ken.qian
// move from comextra to common
// 
// 5     05-04-07 18:32 Kaliven.lee
// change because of the zqthread class change
// 
// 4     05-03-24 17:02 Kaliven.lee
// 
// 2     05-02-04 11:34 Kaliven.lee
// add comment head
// 
//
//////////////////////////////////////////////////////////////////////

#include "FtpConnMgr.h"
#include "FtpMoverConf.h"
#include "FtpConn.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern WORKQUEUE gWorkQueue;
extern ZQ::common::Mutex g_WorkQueueLock;
extern FILELOCKLIST gFileLocks;
ZQ::common::Mutex g_FileLockSection;
extern ZQ::common::Log * pGlog;

using ZQ::common::Log;
using std::vector;
FtpConnMgr::FtpConnMgr(DWORD MaxConns,DWORD ScanInterval):ZQ::common::NativeThread(),
m_dwInterval(ScanInterval),
m_dwMaxConnects(MaxConns),
m_bQuit(false)
{
	
}

FtpConnMgr::~FtpConnMgr()
{
	FTPCONNLIST::iterator Iter;	
	m_ConnLock.enter();
	while(ConnectionList.size() != 0)
	{
		Iter = ConnectionList.begin();		
		FtpConn* pConn = *Iter;
		pConn->Stop();
		ConnectionList.erase(Iter);
		delete pConn;		
	}
	m_ConnLock.leave();
	if(m_event != INVALID_HANDLE_VALUE)
		::CloseHandle(m_event);
}
int FtpConnMgr::run()
{
	DWORD rtn = 0;
	while(!m_bQuit)
	{
		rtn = ::WaitForSingleObject(m_event,m_dwInterval);
		switch(rtn) {
		case WAIT_OBJECT_0:
			return 0;
			break;
		case WAIT_TIMEOUT:
			ScanWorkQueue();
			break;
		default:
			break;
		}

	}
	return 0;
}
void FtpConnMgr::Stop(void)
{
	::SetEvent(m_event);
}
void FtpConnMgr::ScanWorkQueue(void)
{
	// TODO implement the guard
	BOOL isDeleted = FALSE;
	bool IsIdle = true;

	
	if(gWorkQueue.size() == 0)
		return;
	WORKQUEUE::iterator preIter = gWorkQueue.begin();
	WORKQUEUE::iterator Iter = gWorkQueue.begin();
	{//start lock WQ
		g_WorkQueueLock.enter();
		for (;Iter != gWorkQueue.end();)
		{				
			TRANSFERTASK* pTask = *Iter;
			switch(pTask->state) {
			case Failed:
				// try to retransfer move it to the end of the workqueue
				pTask->state = Ready;
				if(Iter == gWorkQueue.begin())
				{				
					gWorkQueue.erase(Iter);
					Iter = gWorkQueue.begin();
					preIter = Iter;
				}
				else
				{
					gWorkQueue.erase(Iter);
					Iter = preIter;
					Iter++;
				}
				gWorkQueue.push_back(pTask);				
				break;
			case Ready:
				if((pTask->state == Ready ))
				{
					//open a new ftp connection to transfer the file				
					glog(Log::L_INFO,_T("The FtpConnMgr start to process transfer file %s."),pTask->wcsFileName);			

					FtpConn* pFtpConn = NULL;
					
					m_ConnLock.enter();
					for(DWORD i = 0; i < ConnectionList.size(); i ++)
					{
						//search idle FtpConn
						pFtpConn = ConnectionList.at(i);
						if(pFtpConn->IsBusy())
							continue;			
						pTask->state = Processing;
						pFtpConn->StartTransfer(pTask);	
						IsIdle = true;
						break;					
					}
					preIter = Iter;
					Iter++;
					m_ConnLock.leave();
					if(!IsIdle)
					{
						IsIdle = false;
						glog(Log::L_INFO,_T("No idle ftp connection to process transfer file %s."),pTask->wcsFileName);						
					}					
				}
				break;
			case Finished:				
				//remove from the queue
				// check the file lock if the dwlocks = 0 delete;
				{				
					bool bDelete = false;
					FILELOCKLIST::iterator iter;
					g_FileLockSection.enter();
					for(iter  = gFileLocks.begin(); iter != gFileLocks.end(); iter ++)
					{
						FILELOCK * pFileLock = *iter;
						if(wcscmp(pFileLock->fileName,pTask->wcsFileName)==0)
						{
											
							if(pFileLock->locks == 1)
							{
								gFileLocks.erase(iter);
								delete pFileLock;
								bDelete = true;	
								break;
							}
							else 
							{
								pFileLock->locks --;
								break;	
							}
						}
					}
					g_FileLockSection.leave();
					if(bDelete)
					{
					
						::SetFileAttributes(pTask->wcsFileName,FILE_ATTRIBUTE_NORMAL);
						isDeleted = ::DeleteFile(pTask->wcsFileName);
						
						if(isDeleted == FALSE)
						{
							glog(Log::L_DEBUG,_T("Failed to delete transferred file %s from temp directory."),pTask->wcsFileName);
							continue;
						}else
						{
							glog(Log::L_DEBUG,_T("Delete transferred file %s from temp directory."),pTask->wcsFileName);
						}
				
					}	
					
					// remove from work queue
					if(Iter == gWorkQueue.begin())
					{
						gWorkQueue.erase(Iter);				
						Iter = gWorkQueue.begin();
						preIter = Iter;
					}
					else
					{
						gWorkQueue.erase(Iter);	
						Iter = preIter;
						Iter ++;
					}
					
								
					glog(Log::L_DEBUG,_T("removed task %s from the work queue."),pTask->wcsFileName);
					delete pTask;
					break;
				}

			default:
				preIter = Iter;
				Iter ++;
				break;
			}
			if(!IsIdle )
				break;									
		};
		g_WorkQueueLock.leave();	
	}	// end lock
}
void FtpConnMgr::Initialize(void)
{
	glog(Log::L_DEBUG,_T("Initialize the Ftp connection pool"));
	m_event = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	for(DWORD i = 0; i < m_dwMaxConnects; i++)
	{
		FtpConn* pFtpConn = new FtpConn();
		pFtpConn->start();
		ConnectionList.push_back(pFtpConn);
	}
	glog(Log::L_DEBUG,_T("%d ftp connections are initialized."),m_dwMaxConnects);
}
