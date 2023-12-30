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
#include "DeviceAgent.h"

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

VOID CSCCommonSender::Start()
{
	ResetEvent(m_hEvent);
	m_dwThreadID = 101;
	m_Thread = CreateThread(NULL,0, Callback,(LPVOID)this,0,&m_dwThreadID);
}

VOID CSCCommonSender::Stop()
{
	SetEvent(m_hEvent);
	//Sleep(50);
	TRACE(_T("----Enter CSCCommonSender::Stop(). \r\n"));
	DWORD dwResult = WaitForSingleObject( m_Thread, INFINITE );
/*
		switch( dwResult )
		{
			case WAIT_TIMEOUT:
			TRACE(_T("----Abnormal Release SenderThread: WAIT_TIMEOUT. \r\n"));
			break;
			case WAIT_FAILED:		
			TRACE(_T("----Abnormal Release SenderThread: WAIT_FAILED. \r\n"));
			break;
			case WAIT_ABANDONED:		
			TRACE(_T("----Abnormal Release SenderThread: WAIT_ABANDONED. \r\n"));
			break;
			case WAIT_OBJECT_0:		
			TRACE(_T("----Abnormal Release SenderThread: WAIT_OBJECT_0. \r\n"));
			break;
		}*/
}

DWORD WINAPI  CSCCommonSender::Callback(LPVOID pParam)
{
	//LOG_ENTRY(_T("CSCCommonSender::Callback()"));

	CSCCommonSender* sender = (CSCCommonSender*)pParam;
	CSCTCPSocket* socket = sender->m_Socket;
	CSCMemoryBlockQueue* queue = sender->m_Queue;
	HANDLE event = sender->m_hEvent;
	SenderNotify notify = sender->m_Notify;

	INT32 length = 0;

	DWORD sleepsecs = 10;
	DWORD sleepcnt = 0;

	while(WaitForSingleObject(event, sleepsecs)!=WAIT_OBJECT_0)
	{		
		if(!queue->Empty())
		{		
			sender->m_iStatus = 1;

			CSCMemoryBlockPtr &block = queue->Front();

			/* 
			// reserved for the other method sending message: 
			// have length info in the header of message.

			PACKET_HDR hdr = { htonl(PACKET_VERSION), htonl(block.GetSize())};
			length = sizeof(hdr);

			if(socket->Send((CHAR*)&hdr, length) != SCS_SUCCESS)
			{
				sender->m_iStatus = 0;
				if(notify)
				{
					notify(SC_SENDER_THREAD_ERROR, socket );
				}
				ResetEvent(event);
				//LOG_ENTRY(_T("CSCCommonSender::Callback()::SC_SENDER_THREAD_ERROR"));
				return SC_SENDER_THREAD_ERROR;
			}*/

			length = block->GetSize();
			if(socket->Send(block->GetBlock(), length) != SCS_SUCCESS)
			{
				sender->m_iStatus = 0;
				if(notify)
				{
					notify(SC_SENDER_THREAD_ERROR, socket );
				}
				ResetEvent(event);
				
				//LOG_ENTRY(_T("CSCCommonSender::Callback()::SC_SENDER_THREAD_ERROR"));
				return SC_SENDER_THREAD_ERROR;
			}

			Clog( 3, _T("Send() successfully.") );
			//---liqing-----------
			//if( block->m_iBlockNumber != -1 )
			//	g_pFileWatcher->DelNotificationSent( block->m_iBlockNumber );
			//---liqing-----------

			queue->Pop();

			sleepsecs = 1;
			sleepcnt = 0;
		}
		else
		{
			// Adaptive sleep.
			sleepsecs = ((sleepcnt / 10) + sleepsecs) > 100 ? 100 : ((sleepcnt / 10) + sleepsecs);
			sleepcnt++;
		}
	}
	sender->m_iStatus = 0;

	TRACE(_T("----Leave CSCCommonSender::Callback(). \r\n"));
	return SC_SENDER_THREAD_OK;
}

//push data into queue
void CSCCommonSender::Push(CSCMemoryBlockPtr data)
{
	if ( m_Queue == NULL )
		return;
	m_Queue->Push(data);
}