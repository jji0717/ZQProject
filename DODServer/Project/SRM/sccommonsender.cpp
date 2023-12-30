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
//#include "DeviceAgent.h"

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
	try
	{
	Lock();
	if(!IsThread)
	{
	  ResetEvent(m_hEvent);
	  m_dwThreadID = 101;
	  m_Thread = CreateThread(NULL,0, Callback,(LPVOID)this,0,&m_dwThreadID);
	}
	UnLock();
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSCCommonSender::Start") );
   }

}

VOID CSCCommonSender::Stop()

{
	try
	{
    //Clog( LOG_DEBUG, "Sender Stop break1" );
	Lock();
    //Clog( LOG_DEBUG, "Sender Stop break2" );
	//Sleep(100);
	if(IsThread)
	{
     // Clog( LOG_DEBUG, "Sender Stop break3" );
	  SetEvent(m_hEvent);
	  //Sleep(50);
	  TRACE(_T("----Enter CSCCommonSender::Stop(). \r\n"));
	  //DWORD dwResult = WaitForSingleObject( m_Thread, INFINITE );
       DWORD dwResult = WaitForSingleObject( m_Thread, 100 );
      //Clog( LOG_DEBUG, "Sender Stop break4" );
	}
	UnLock();
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSCCommonSender::Stop") );
   }
}

DWORD WINAPI  CSCCommonSender::Callback(LPVOID pParam)
{
	try
	{
	CSCCommonSender* sender = (CSCCommonSender*)pParam;
	CSCTCPSocket* socket = sender->m_Socket;
	CSCMemoryBlockQueue* queue = sender->m_Queue;
	HANDLE event = sender->m_hEvent;
	SenderNotify notify = sender->m_Notify;
    ConnectionNotify    ConNotify=sender->m_ConNotify;
	sender->IsThread=true;

	INT32 length = 0;
	DWORD sleepsecs = 10;
	DWORD sleepcnt = 0;
	while(WaitForSingleObject(event, sleepsecs)!=WAIT_OBJECT_0)
	{	
		//sender->Lock();
		if(!queue->Empty())
		{		
			sender->m_iStatus = 1;

			CSCMemoryBlockPtr &block = queue->Front();
			length = block->GetSize();
			if(socket->Send(block->GetBlock(), length) != SCS_SUCCESS)
			{
                Clog(LOG_DEBUG,"break or error when send");
				sender->m_iStatus = 0;
				if(notify)
				{
					notify(SC_SENDER_THREAD_ERROR, socket );
				}
              	ResetEvent(event);
				sender->IsThread=false;
				//sender->UnLock();
				return SC_SENDER_THREAD_ERROR;
			}
           // Clog(LOG_DEBUG,"succsessfully send");
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
       // sender->UnLock();
	}
	sender->m_iStatus = 0;
	sender->IsThread=false;
	return SC_SENDER_THREAD_OK;
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSCCommonSender::Callback") );

   }
   return -1;
}

//push data into queue
void CSCCommonSender::Push(CSCMemoryBlockPtr data)
{
	try
	{
	if ( m_Queue == NULL )
		return;
	m_Queue->Push(data);
	}
    catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSCCommonSender::Push") );

   }
}

void CSCCommonSender::SetNotify(ConnectionNotify notify)
 {
      m_ConNotify=notify;
 }
 void CSCCommonSender::Lock()
 {
	 ::EnterCriticalSection(&m_Lock);
 }
 void CSCCommonSender::UnLock()
 {
	 ::LeaveCriticalSection(&m_Lock);
 }


