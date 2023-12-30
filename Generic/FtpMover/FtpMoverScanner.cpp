
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
// Ident : $Id: FtpMoverScanner.cpp,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : impl the directory guard class FtpMoverScanner
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpMoverScanner.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 7     06-08-04 13:30 Ken.qian
// move from comextra to common
// 
// 6     05-04-06 19:54 Kaliven.lee
// add support to net directory
// 
// 5     05-04-04 13:33 Kaliven.lee
// 
// 4     05-03-24 17:02 Kaliven.lee
// 
// 2     05-02-04 11:37 Kaliven.lee
// add head
// 
// 
//
//////////////////////////////////////////////////////////////////////
// FtpMoverScanner.cpp: implementation of the FtpMoverScanner class.
//
//////////////////////////////////////////////////////////////////////
#include "FtpMoverconf.h"
#include "FtpMoverScanner.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using ZQ::common::NativeThread;
using ZQ::common::Log;
extern ZQ::common::Log* pGlog;
extern WORKQUEUE gWorkQueue;
extern ZQ::common::Mutex g_WorkQueueLock;
extern wchar_t	gwcsServer[64];
extern wchar_t	gwcsPassword[64];
extern wchar_t	gwcsUserName[64];
extern DWORD gdwPort;
extern TASKLIST gTasks;
extern FILELOCKLIST gFileLocks;
extern ZQ::common::Mutex g_FileLockSection;

FtpMoverScanner::FtpMoverScanner(DWORD dwInterval):
m_bQuit(false),
m_dwScanInterval(dwInterval),
ZQ::common::NativeThread()
{
	m_event = ::CreateEvent(NULL,false,false,NULL);

}

FtpMoverScanner::~FtpMoverScanner()
{
	if(m_event != INVALID_HANDLE_VALUE)
		::CloseHandle(m_event);
}
void FtpMoverScanner::ScanDirectory(void)
{
	// prepare search file

	BOOL rtn = FALSE;
	wchar_t OldPath[MAX_PATH] = _T("");	
	wchar_t FindPath[MAX_PATH] = _T("");	
	wchar_t NewPath[MAX_PATH] = _T("");
	WIN32_FIND_DATA FileFindData;
	HANDLE hTemp = 0;
	DWORD i = 0;
	DWORD taskCount = gTasks.size();
	// process each task
	for(i = 0; i< taskCount; i++)
	{
		TASK* pTask = gTasks.at(i);
		wcscpy(FindPath,pTask->wcsGuardDir);
		if(FindPath[wcslen(FindPath) -1] !=_T('\\'))
		{
			wcscat(FindPath,_T("\\"));
		}
		wcscat(FindPath,_T("*"));

		HANDLE hFind = ::FindFirstFile(FindPath,&FileFindData);

		// jump "." and ".." in dir
		BOOL isFileFind;
		isFileFind = FindNextFile(hFind,&FileFindData);
		//skip if search net directory
		if(wcscpy(FileFindData.cFileName,_T(".."))!=0 )
			isFileFind = FindNextFile(hFind,&FileFindData);			
		// file search loop
		while(isFileFind != FALSE)
		{
			bool isInWQ = false;
			glog(Log::L_DEBUG,_T("File %s found in Guard directory."),FileFindData.cFileName);
			
			wcscpy(OldPath,pTask->wcsGuardDir);
			if(OldPath[wcslen(OldPath) -1] !=_T('\\'))
			{
				wcscat(OldPath,_T("\\"));
			}
			wcscat(OldPath,FileFindData.cFileName);
					
			wcscpy(NewPath,pTask->wcsTempDir);
			if(NewPath[wcslen(NewPath) -1] !=_T('\\'))
			{
				wcscat(NewPath,_T("\\"));
			}
			wcscat(NewPath,FileFindData.cFileName);
		
			// reverse search
			{// begin guard
				FILELOCK * pFileLock = NULL;
				bool bFileLocked = false;
				g_WorkQueueLock.enter();
				g_FileLockSection.enter();
				for(DWORD i = 0; i < gFileLocks.size(); i ++)
				{
					pFileLock = gFileLocks.at(i);
					
					if(pFileLock->fileName == NewPath)// found same file is in workqueue					
					{
					// scan work queue 
						bFileLocked = true;
						WORKQUEUE::iterator iter = NULL;
						TRANSFERTASK* pTransferTask = NULL;
						bool bFound = false;
						for (iter = gWorkQueue.begin();iter != gWorkQueue.end();iter++)
						{
							pTransferTask = *iter;
							if(wcscmp(pTransferTask->wcsFileName,NewPath) == 0)
							{
								if(pTransferTask->state == Processing)
									bFound = true;
								break;
							}
						}
						// find a task processing,left the file in guard directory
						if(bFound)
						{
							glog(Log::L_DEBUG,_T("find a task transferring the same file.left it in the guard directory."));
						}
						else// else move the file and insert it into workqueue
						{
							glog(Log::L_INFO,_T("Find same file waiting to be transferred in work queue.add new tasks"));
							rtn = ::SetFileAttributes(OldPath,FILE_ATTRIBUTE_NORMAL);
							DWORD dwErr = GetLastError();
							rtn = ::MoveFileEx(OldPath,NewPath,MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
							if(rtn != FALSE)
							{
								glog(Log::L_INFO,_T("Add new tasks to work queue."));
								AddNewTask(NewPath,FileFindData.cFileName,pTask->FtpServerList);
								rtn = ::FindClose(hFind);					
								hFind = ::FindFirstFile(FindPath,&FileFindData);
								//skip the file . and ..								
								//skip if search net directory
								if(wcscpy(FileFindData.cFileName,_T(".."))!=0 )
									isFileFind = FindNextFile(hFind,&FileFindData);
							}
							break;
						}						 
					}//end find sample file
				}// end for 
				g_FileLockSection.leave();
				
				if(!bFileLocked)
				{					
					// move the file add task
					glog(Log::L_DEBUG,_T("Can not find same file in work queue, copy the file to tmp path"));
					rtn = ::SetFileAttributes(OldPath,FILE_ATTRIBUTE_NORMAL);
					
					rtn = ::MoveFileEx(OldPath,NewPath,MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);			
					//add new task		
				
					if(rtn != FALSE)
					{
						
						glog(Log::L_DEBUG,_T("Add the task into work queue."));
						AddNewTask(NewPath,FileFindData.cFileName,pTask->FtpServerList);	
						rtn = ::SetFileAttributes(NewPath,FILE_ATTRIBUTE_NORMAL);
						rtn = ::FindClose(hFind);
						hFind = ::FindFirstFile(FindPath,&FileFindData);					
						//skip if search net directory
						if(wcscpy(FileFindData.cFileName,_T(".."))!=0 )
							rtn = ::FindNextFile(hFind,&FileFindData);								
					}
					else
					{
						glog(Log::L_DEBUG,_T("Failed to move the file.left it in guard directory."));
					}	
				}			
				g_WorkQueueLock.leave();
				isFileFind = FindNextFile(hFind,&FileFindData);
			}// end guard
		}//end scan file
		::FindClose(hFind);
	}// end for task cycle
	
}
int FtpMoverScanner::run()
{
	DWORD rtn = 0;
	InitScan();
	while(!m_bQuit)
	{
		rtn = ::WaitForSingleObject(m_event,m_dwScanInterval);
		if(m_bQuit)
			return 0;
		switch(rtn) {
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			ScanDirectory();
			break;
		default:
			break;
		}
	}
	return 0;
}
void FtpMoverScanner::Stop(void)
{
	::SetEvent(m_event);
	m_bQuit = true;	
}

void FtpMoverScanner::AddNewTask(wchar_t* transferFile,wchar_t* remoteFile,SERVERLIST FtpServerList)
{
	// to do:: need to be guard
	g_WorkQueueLock.enter();
	
	for (DWORD i =0 ; i < FtpServerList.size(); i ++)
	{
		FTPSERVER* pFtpServer = FtpServerList.at(i);
		TRANSFERTASK *pNewTask = new TRANSFERTASK;
		wcscpy(pNewTask->wcsFileName,transferFile);
		wcscpy(pNewTask->wcsRemoteFileName,remoteFile);
		wcscpy(pNewTask->wcsDestineServer,pFtpServer->wcsServer);
		wcscpy(pNewTask->wcsUserName,pFtpServer->wcsUserName);
		wcscpy(pNewTask->wcsPassword,pFtpServer->wcsPassword);
		pNewTask->dwPort = pFtpServer->dwPort;
		pNewTask->state = Ready;
		pNewTask->percentage = 0;
		pNewTask->dwFtpConnID = 0;
		gWorkQueue.push_back(pNewTask);
	}	
	g_WorkQueueLock.leave();	
		//add lock 
	bool bFound = false;
	g_FileLockSection.enter();
	for(DWORD j = 0; j < gFileLocks.size(); j ++)
	{
		FILELOCK* fileLock = gFileLocks.at(j);
		if(fileLock->fileName == transferFile)
		{
			bFound = true;
			fileLock->locks += FtpServerList.size();
			break;
		}
	}
	if(!bFound)
	{
		FILELOCK *fileLock = new FILELOCK;
		fileLock->fileName = transferFile;
		fileLock->locks = FtpServerList.size();
		gFileLocks.push_back(fileLock);
	}
	g_FileLockSection.leave();
}
void FtpMoverScanner::InitScan(void)
{
	wchar_t FindPath[MAX_PATH]= _T("");
	for (DWORD i =0; i < gTasks.size() ; i++)
	{		
		wcscpy(FindPath,gTasks.at(i)->wcsTempDir);
		if(FindPath[wcslen(FindPath) -1] !=_T('\\'))
		{
			wcscat(FindPath,_T("\\"));
		}
		wcscat(FindPath,_T("*"));

		WIN32_FIND_DATA FileFindData;
		HANDLE hTemp = 0;

		HANDLE hFind = ::FindFirstFile(FindPath,&FileFindData);

		// jump "." and ".." in dir
		BOOL isFileFind;
		isFileFind = FindNextFile(hFind,&FileFindData);
		isFileFind = FindNextFile(hFind,&FileFindData);	
		if(isFileFind == FALSE)
		{
			glog(Log::L_INFO,_T("No File in the Temp Directory %s."),gTasks.at(i)->wcsTempDir);
			continue;
		}
		glog(Log::L_INFO,_T("found unfinished Files in the Temp Directory %s.Add them to work queue."),gTasks.at(i)->wcsTempDir);

		while (isFileFind!= FALSE)
		{
			// parse the file name
			CString str = gTasks.at(i)->wcsTempDir;		
			str += FileFindData.cFileName;

			AddNewTask((LPTSTR)(LPCTSTR)str,FileFindData.cFileName,gTasks.at(i)->FtpServerList);
			isFileFind = FindNextFile(hFind,&FileFindData);
		}
		FindClose(hFind);
	}
	
}
