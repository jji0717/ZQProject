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

#include "common.h"
#include "sccommonreceiver.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define PACKET_VERSION (0x00000909)
extern BOOL g_IsStopAll;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VOID CSCCommonReceiver::Start()
{
	if( !m_iStatus )
	{
		ResetEvent(m_hEvent);
		m_Thread = CreateThread(NULL,0,Callback,(LPVOID)this,0,&m_dwThreadID);
	}
}

VOID CSCCommonReceiver::Stop()
{
	if(m_hEvent)
	{
		SetEvent(m_hEvent);
		Sleep(2);
	}
	//Sleep(100);
	TRACE(_T("----Enter CSCCommonReceiver::Stop(). \r\n"));
	Clog( LOG_DEBUG,_T("----Enter CSCCommonReceiver::Stop(). "));

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
}

DWORD WINAPI CSCCommonReceiver::Callback(LPVOID pParam)
{
	CSCCommonReceiver* receiver = (CSCCommonReceiver*)pParam;
	CSCTCPSocket* socket = receiver->m_Socket;
	HANDLE event = receiver->m_hEvent;
	ReceiverNotify notify = receiver->m_Notify;
	ISCParser* parser = receiver->m_Parser;
	receiver->m_iStatus = 1;

	INT32 length;
	int i = 0, nRead=0;
	while(WaitForSingleObject(event, 100)!=WAIT_OBJECT_0)
	{	
		length = RECVBUFMAX - nRead;
		if( length <= 0 )
		{
			length = RECVBUFMAX;
			nRead = 0;
		}
		if(g_IsStopAll)
			break;

		if(socket->m_nError)
		{
			Clog( LOG_ERR, _T("CSCCommonReceiver() check m_nError is true, exit thread! "));
			break;
		}

		int nCheckRet = 0;
		DWORD dwError = 0;

		nCheckRet = socket->CheckRead(500, &dwError);
		if( nCheckRet==0 )
		{
			continue;
		}
		else if( nCheckRet == SOCKET_ERROR )
		{
			socket->m_nError = 1;
			if( notify )
			{
				notify(SC_RECEIVER_THREAD_ERROR, socket );
			}
			receiver->m_iStatus = 0;			
			return SC_RECEIVER_THREAD_ERROR;
		}

		if( socket->ReceiveData( receiver->m_pRecvBuf+nRead, length ) != SCS_SUCCESS )
		{
			if(socket->m_nError)
				break;
			else
				socket->m_nError = 1;
			if(g_IsStopAll)
				break;

			Clog( LOG_ERR, _T("receiver(), error: code=%d"), GetLastError());

			if(notify)
			{
				notify(SC_RECEIVER_THREAD_ERROR, socket );
			}
			receiver->m_iStatus = 0;	
			Clog( LOG_DEBUG, _T("receiver(), call notify end:"));
			return SC_RECEIVER_THREAD_ERROR;		
		}
		
		for( i=0; i<length; i++ )
		{	
			if(g_IsStopAll)
				break;

			if( receiver->m_pRecvBuf[nRead+i] == MSG_END_FLAG )
			{				
		
				CHAR* data = CSCMemoryBlock::AllocBlock(nRead+i);
				memcpy( data, receiver->m_pRecvBuf, nRead+i );

				CSCMemoryBlock * block = new CSCMemoryBlock( data, nRead+i );
				CSCMemoryBlockPtr pFredPtr( block );

				parser->Parse( socket, pFredPtr );
				//CSCMemoryBlock::FreeBlock(data);				
								
				if( (i+1) < length )
				{
					MoveMemory( receiver->m_pRecvBuf, receiver->m_pRecvBuf+nRead+i+1, length-i-1 );					
					length = length-i-1;
					i = 0;
					nRead = 0;
				}
				else
				{
					nRead = 0;
					break;
				}
			}
		}
		if( i == length )
		{
			nRead += length;
		}
		Sleep( 2 );
	}

	receiver->m_iStatus = 0;
	Clog( LOG_DEBUG,_T("--Leave CSCCommonReceiver::Callback(). "));
	TRACE(_T("----Leave CSCCommonReceiver::Callback(). \r\n"));
	return SC_RECEIVER_THREAD_OK;
}
