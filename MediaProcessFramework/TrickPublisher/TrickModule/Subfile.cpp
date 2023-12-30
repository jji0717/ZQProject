// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: Subfile.cpp,v 1.6 2004/08/12 09:40:52 jshen Exp $
// Branch: $Name:  $
// Author: jshen
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/TrickPublisher/TrickModule/Subfile.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-04-25 15:41 Hongye.gu
// 
// 1     04-12-03 13:56 Jie.zhang
// 
// 3     04-11-22 19:48 Jie.zhang
// log message.
// Revision 1.6  2004/08/12 09:40:52  jshen
// remove output to screen
//
// Revision 1.5  2004/07/29 06:21:10  jshen
// before release
//
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:19:41  jshen
// add comments
//
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:19:43  jshen
// copied to production tree
//
// ===========================================================================

#include "tchar.h"
#include <process.h>

#include "Subfile.h"
#include "TrickImportUser.h"

/////////////////////////////////////////////////////////////////////////////
//
//  Thread start routine
//
/////////////////////////////////////////////////////////////////////////////

unsigned int __stdcall CSubfileContext::StartIoThread(PVOID context)
{
	CSubfileContext *subFile = (CSubfileContext *)context;
	WaitForSingleObject(subFile->hStartEvent, INFINITE);
	//
	// maintain a reference on importer thread
	//
	InterlockedIncrement(&subFile->refCount);
	InterlockedIncrement(&subFile->consumerContext->m_refCount);
	//
	// call IO function
	//
	subFile->ProcessWorkQueue();

	SetEvent(subFile->hStopEvent);
	
	InterlockedDecrement(&subFile->refCount);
	while(subFile->refCount != 0)
		Sleep(30);			// wait for importer thread to dereference
	//
	// release reference on importer thread
	//
	InterlockedDecrement(&subFile->consumerContext->m_refCount);
	
	delete subFile;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Constructor
//
/////////////////////////////////////////////////////////////////////////////

CSubfileContext::CSubfileContext(
			ULONG consumerIndex, 
			SubfileType sfType, 
			LONG speed, 
			CTrickImportUser *context)
	: hFile(INVALID_HANDLE_VALUE), runningByteOffset(0), refCount(0), 
		state(SUBFILE_INITIALIZING), queueDepth(0)

{
	index					= consumerIndex;
	type					= sfType;
	relativeSpeed			= speed;
	consumerContext			= context;
	fileOffset.QuadPart		= 0;

	hStartEvent = CreateEvent(0, 1,0,0);
	hStopEvent  = CreateEvent(0, 1,0,0);
	//CWinThread	*thread = AfxBeginThread(
	//		(AFX_THREADPROC)StartIoThread, 
	//		(LPVOID)this);
	m_hThread = reinterpret_cast<HANDLE> (::_beginthreadex(NULL, 0, StartIoThread, (LPVOID) this, 0, &m_uiThreadId));
}

/////////////////////////////////////////////////////////////////////////////
//
//  Destructor
//
/////////////////////////////////////////////////////////////////////////////

CSubfileContext::~CSubfileContext()
{
#ifdef _DEBUG
	DbgString(_T("~CSubfileContext()\n"));
#endif 	

	CloseHandle(hStartEvent);
	CloseHandle(hStopEvent);

	while(refCount > 0)
	{
		Sleep(50);
	}
	//
	// flush any pending output buffers
	//
	SDataBuffer *buf;
	while(ioQueue.RemoveFront(0, (PVOID *)&buf))
	{
		if (buf)
		{
			buf->freeList->Free(buf);
		}
	}
	if (m_hThread)
    {
        CloseHandle(m_hThread);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  ProcessWorkQueue
//
/////////////////////////////////////////////////////////////////////////////

void CSubfileContext::ProcessWorkQueue()
{
	DWORD finalStatus = ERROR_SUCCESS;
	
	state = SUBFILE_RUNNING;

	while(1)
	{
		SDataBuffer *buf=NULL;
		subState = SUBSTATE_QUEUE;
		ioQueue.RemoveFront(1, (PVOID *)&buf);
		if (buf)
		{
			InterlockedDecrement(&queueDepth);
//			DbgString("ProcessWorkQueue: buffer %x for file %s\n", buf, filename);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				subState = SUBSTATE_WRITE;
				DWORD amountWritten = WriteSubfile(buf->len, buf->mpegBuffer.pointer);
	
				if(buf->freeList)
				{
					if(buf->freeList)
						buf->freeList->Free(buf);
				}
				if (amountWritten == 0)
				{
					finalStatus = GetLastError();
					break;
				}
				fileOffset.QuadPart += amountWritten;
			}
			else
			{
				if(buf->freeList)
					buf->freeList->Free(buf);

				finalStatus = ERROR_OPEN_FAILED;
				_stprintf(strMsg, _T("File %d is not open"), type);
				break;
			}
		
		}
		else
		{
			_stprintf(strMsg, _T("File %s completed normally\n"), filename);
			break;
		}

	}
	//_CrtCheckMemory();
	state = SUBFILE_STOPPED;
	
}

