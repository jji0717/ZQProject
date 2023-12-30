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

#if !defined(AFX_SCCOMMONSENDER_H__E42979D5_95EC_4538_9EDE_2B7200CEB0BE__INCLUDED_)
#define AFX_SCCOMMONSENDER_H__E42979D5_95EC_4538_9EDE_2B7200CEB0BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "DeviceInfo.h"
#include "scsocket.h"
#include "scqueue.h"

#define SC_SENDER_THREAD_OK		(0x00)
#define SC_SENDER_THREAD_ERROR	(0x01)

// SenderNotify function type declaration.
typedef VOID (*SenderNotify)(INT32 Status,CSCTCPSocket* pSocket);
typedef VOID (*ConnectionNotify)();
class CSCCommonSender  
{
public:
	typedef std::list<CSCCommonSender*> List;

	/// Ctor
	CSCCommonSender(CSCTCPSocket* pSocket, CSCMemoryBlockQueue* pQueue, SenderNotify pNotify)
	{
		ASSERT(pSocket != NULL);
		//ASSERT(pQueue != NULL);
        IsThread=false;
		m_Socket = pSocket;
		m_Queue = new CSCMemoryBlockQueue;
		m_Thread = NULL;
		m_iStatus = 0;
		m_Notify = pNotify;
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		::InitializeCriticalSection(&m_Lock);
	}
   CSCCommonSender(CSCTCPSocket* pSocket, CSCMemoryBlockQueue* pQueue)
	{
		ASSERT(pSocket != NULL);
        IsThread=false;
		//ASSERT(pQueue != NULL);
		m_Socket = pSocket;
		m_Queue = new CSCMemoryBlockQueue;
		m_Thread = NULL;
		m_iStatus = 0;
		m_Notify = NULL;
        m_ConNotify=NULL;
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        ::InitializeCriticalSection(&m_Lock);
	}
   void  SetNotify(ConnectionNotify notify);
    //{
    //  m_ConNotify=notify;
   // };

	/// Dtor
	virtual ~CSCCommonSender()
	{/*
		if( m_Thread && m_iStatus  )
		{
			if( !TerminateThread( m_Thread, 102 ) )
			{//If the function succeeds, the return value is nonzero.
				if( m_Thread )
				{
					delete m_Thread;
					m_Thread = NULL;
				}
			}
		}*/
		m_Socket = NULL;

		// free sender queue
		if (m_Queue!=NULL)
		{
			INT32 size = m_Queue->Size();
			while(size)
			{
				//CSCMemoryBlock& block = m_Queue->Front();
				//block.m_iRefenceCount--;
				//if( block.m_iRefenceCount < 1 )
				//	CSCMemoryBlock::FreeBlock(block.GetBlock());
				m_Queue->Pop();
				size--;
			}
			delete m_Queue;
			m_Queue = NULL;
		}
		//m_Queue = NULL;

		//m_Thread = NULL;
		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
      ::DeleteCriticalSection(&m_Lock);
	}

	/// Start the common sender.
	VOID Start();

	/// Stop the common sender.
	VOID Stop();

		///Get connect socket
	///return @ CSCTCPSocket * pointer
	CSCTCPSocket* GetSocket()
	{
		return m_Socket;
	}

	///push data into queue
	///@param data(IN) - data to be sended out
	void Push(CSCMemoryBlockPtr data);
	void Lock();
	void UnLock();
    bool IsThread;
private:
	CSCTCPSocket* m_Socket;
	CSCMemoryBlockQueue* m_Queue;
    HANDLE m_hEvent;
	HANDLE m_Thread;
	DWORD m_dwThreadID;
	int m_iStatus;
	SenderNotify m_Notify;
    ConnectionNotify m_ConNotify;

	CRITICAL_SECTION m_Lock;
	//bool IsThread;
private:
	static DWORD WINAPI  Callback(LPVOID pParam);
};

#endif // !defined(AFX_SCCOMMONSENDER_H__E42979D5_95EC_4538_9EDE_2B7200CEB0BE__INCLUDED_)
