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
// ===========================================================================

#include "stdafx.h"
#include "sccommonsender.h"
//#include "CSCDeviceInfo.h"
//#include "CFileWatcher.h"
#include "clog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define PACKET_VERSION (0x00000909)

typedef struct 
{
	INT32 version;
	INT32 length;
} PACKET_HDR;

extern CSCMemoryBlockQueue *m_pKitSenderQueue;

VOID CSCCommonSender::Start()
{
	//LOG_ENTRY(_T("CSCCommonSender::Start()"));
	DWORD m_dwThreadID = 0;
	ResetEvent(m_hEvent);
	m_Thread = CreateThread(NULL,0, Callback,(LPVOID)this,0,&m_dwThreadID);
}

VOID CSCCommonSender::Stop()
{
	if(m_hEvent)
	{
		SetEvent(m_hEvent);
		Sleep(2);
	}
	if(m_Thread)
	{
		DWORD dwResult = WaitForSingleObject( m_Thread, INFINITE );
		CloseHandle(m_Thread);
		m_Thread = NULL;
	}
	
	if(m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
	m_iStatus = 0;
	Clog( LOG_DEBUG, _T("-----Enter CSCCommonSender::Stop()"));
}

DWORD WINAPI  CSCCommonSender::Callback(LPVOID pParam)
{
	CSCCommonSender* sender = (CSCCommonSender*)pParam;
	CSCTCPSocket* socket = sender->m_Socket;
//	CSCMemoryBlockQueue* queue = sender->m_Queue;
	HANDLE event = sender->m_hEvent;
	SenderNotify notify = sender->m_Notify;

	INT32 length = 0;

	DWORD sleepsecs = 10;
	DWORD sleepcnt = 0;

	try
	{
		while(WaitForSingleObject(event, sleepsecs)!=WAIT_OBJECT_0)
		{		
			if(socket->m_nError)
			{
				Clog( LOG_DEBUG, _T("CSCCommonSender() check m_nError is true, exit thread! "));
				break;
			}		
			if (m_pKitSenderQueue ==NULL)
			{
				continue;
			}

			if(!m_pKitSenderQueue->Empty())
			{		
				sender->m_iStatus = 1;

				FredPtr block = m_pKitSenderQueue->Front();
				length = block->GetSize();	
				if(socket->Send(block->GetBlock(), length) != SCS_SUCCESS)
				{
					if ( socket->m_nError )
					{
						break;
					}
					else
						socket->m_nError = 1;
					Clog( LOG_DEBUG, _T("Send thread:send data error=%d!"),GetLastError());
					sender->m_iStatus = 0;
					if(notify)
					{
						notify(SC_SENDER_THREAD_ERROR, socket );
					}
					return SC_SENDER_THREAD_ERROR;
				}

				m_pKitSenderQueue->Pop();

				sleepsecs = 1;
				sleepcnt = 0;
			}
			else
			{
				// Adaptive sleep.
				sleepsecs = ((sleepcnt / 10) + sleepsecs) > 100 ? 100 : ((sleepcnt / 10) + sleepsecs);
				sleepcnt++;
			}
		Sleep( 2 );
		}
	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("Send thread:send data exception : error=%d!"),GetLastError());
	}
	sender->m_iStatus = 0;

	Clog( LOG_DEBUG, _T("Send thread:exit!"));
	return SC_SENDER_THREAD_OK;
}