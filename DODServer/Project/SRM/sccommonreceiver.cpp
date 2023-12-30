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
#include "sccommonreceiver.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define PACKET_VERSION (0x00000909)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VOID CSCCommonReceiver::Start()

{
	try
	{
	ResetEvent(m_hEvent);
	//m_Thread = AfxBeginThread(&CSCCommonReceiver::Callback,this );//, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_dwThreadID = 102;
	m_Thread = CreateThread(NULL,0, Callback,(LPVOID)this,0,&m_dwThreadID);
	}
 catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSCCommonReceiver::Start") );
   }

}

VOID CSCCommonReceiver::Stop()
{
	try
	{
	SetEvent(m_hEvent);
	//Sleep(200);
	TRACE(_T("----Enter CSCCommonReceiver::Stop(). \r\n"));

	DWORD dwResult = WaitForSingleObject( m_Thread, INFINITE );
	}
   catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSCCommonReceiver::Stop") );
   }

}

DWORD WINAPI CSCCommonReceiver::Callback(LPVOID pParam)

{
	try
	{
	//LOG_ENTRY(_T("CSCCommonReceiver::Callback()"));

	CSCCommonReceiver* receiver = (CSCCommonReceiver*)pParam;
	CSCTCPSocket* socket = receiver->m_Socket;
	HANDLE event = receiver->m_hEvent;
	ReceiverNotify notify = receiver->m_Notify;
    ConnectionNotify  ConNotify=receiver->m_ConectNotify;
	ISCParser* parser = receiver->m_Parser;

	INT32 length;
	//INT32 data_len = 0;
	//INT32 version = 0;
	//BOOL skip = FALSE;

	int i = 0, nRead=0;

	while(WaitForSingleObject(event, 100)!=WAIT_OBJECT_0)
	{	
		receiver->m_iStatus = 1;

		length = RECVBUFMAX - nRead;
		if( length <= 0 )
		{
			length = RECVBUFMAX;
			nRead = 0;
		}
		if( socket->ReceiveData( receiver->m_pRecvBuf+nRead, length ) != SCS_SUCCESS )
		{
            Clog(LOG_DEBUG,"break or error when receive");
			receiver->m_iStatus = 0;
			if(notify)
			{
				notify(SC_RECEIVER_THREAD_ERROR, socket );
			}
			/*if(ConNotify)
			{
                 ConNotify();
			}*/
			ResetEvent(event);
			
			return SC_RECEIVER_THREAD_ERROR;		
		}
		//Clog(LOG_DEBUG,"receive some  data ");
		for( i=0; i<length; i++ )
		{			
			if( receiver->m_pRecvBuf[nRead+i] == MSG_END_FLAG )
			{				
				//socket->m_tmLastUpdate = GetTickCount();
				//TRACE( _T("%d\r\n"), socket->m_tmLastUpdate );
				
				CHAR* data = CSCMemoryBlock::AllocBlock(nRead+i);
				memcpy( data, receiver->m_pRecvBuf, nRead+i );

				CSCMemoryBlock * block = new CSCMemoryBlock( data, nRead+i );
				CSCMemoryBlockPtr pFredPtr( block );
                //modifyed  by whp
				//printf("receive0\n");
			    parser->Parse( pFredPtr);
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
		
		/* 
		// reserved for the other method receiving message: 
		// have length info in the header of message.

		length = sizeof(version);
		if(socket->Receive((CHAR*)&version, length) != SCS_SUCCESS)
		{
			receiver->m_iStatus = 0;
			if(notify)
			{
				notify(SC_RECEIVER_THREAD_ERROR, socket );
			}
			ResetEvent(event);
			
			//LOG_INFO(_T("CSCCommonReceiver::Callback::SC_RECEIVER_THREAD_ERROR"));
			return SC_RECEIVER_THREAD_ERROR;
		}
		version = ntohl(version);
		skip = (version != PACKET_VERSION);

		length = sizeof(data_len);
		if(socket->Receive((CHAR*)&data_len, length) != SCS_SUCCESS)
		{
			receiver->m_iStatus = 0;
			if(notify)
			{			
				notify(SC_RECEIVER_THREAD_ERROR, socket );
			}
			ResetEvent(event);
			
			//LOG_INFO(_T("CSCCommonReceiver::Callback::SC_RECEIVER_THREAD_ERROR"));
			return SC_RECEIVER_THREAD_ERROR;
		}

		length = ntohl(data_len);
		
		CHAR* data = CSCMemoryBlock::AllocBlock(length);

		if(socket->Receive(data, length) != SCS_SUCCESS)
		{
			receiver->m_iStatus = 0;
			CSCMemoryBlock::FreeBlock(data);
			if(notify)
			{
				notify(SC_RECEIVER_THREAD_ERROR, socket );
			}
			ResetEvent(event);
			
			//LOG_INFO(_T("CSCCommonReceiver::Callback::SC_RECEIVER_THREAD_ERROR"));
			return SC_RECEIVER_THREAD_ERROR;
		}

		CSCMemoryBlock block(data, length);
		if(!skip)
		{		
			parser->Parse(block);
		}
		CSCMemoryBlock::FreeBlock(data);
		*/		
	}

	receiver->m_iStatus = 0;

	TRACE(_T("----Leave CSCCommonReceiver::Callback(). \r\n"));
	//LOG_INFO(_T("CSCCommonReceiver::Callback::RETURN"));
	return SC_RECEIVER_THREAD_OK;
	}
catch(...)
   {
     Clog( LOG_DEBUG, _T("catch Error in CSCCommonReceiver::Callback") );
   }
   return -1;
}
void  CSCCommonReceiver::SetNotify(ConnectionNotify notify)
{
	this->m_ConectNotify=notify;
}