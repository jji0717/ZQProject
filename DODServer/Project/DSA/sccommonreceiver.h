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

#if !defined(AFX_SCCOMMONRECEIVER_H__A97CAF90_BAFC_4E75_B95E_09F51B7FCCDE__INCLUDED_)
#define AFX_SCCOMMONRECEIVER_H__A97CAF90_BAFC_4E75_B95E_09F51B7FCCDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "scsocket.h"
#include "scqueue.h"

//#include "DeviceInfo.h"
//#include "scsocket.h"
//#include "scqueue.h"
#include "DeviceInfoParser.h"

#define SC_RECEIVER_THREAD_OK		(0x00)
#define SC_RECEIVER_THREAD_ERROR	(0x01)

#define RECVBUFMAX		65535

#define MSG_END_FLAG	127

// ReceiverNotify function type declaration.
typedef VOID (*ReceiverNotify)(INT32 Status, CSCTCPSocket* pSocket);

class CSCCommonReceiver  
{
public:
	int m_nError;
public:
	/// Marker list type in stl	
	typedef std::list<CSCCommonReceiver*> List;

	/// Ctor
	CSCCommonReceiver(CSCTCPSocket* pSocket, ISCParser* pParser, ReceiverNotify pNotify)
	{
		ASSERT(pSocket != NULL);
		ASSERT(pParser != NULL);

		m_Socket = pSocket;
		m_Parser = pParser;
		m_Notify = pNotify;
		m_Thread = NULL;
		m_hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		m_iStatus = 0;
		m_nError = 0;
	}
	
	/// Dtor
	virtual ~CSCCommonReceiver()
	{
		m_Socket = NULL;
		m_Parser = NULL;
		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
	}
	
	/// Start the common receiver.
	VOID Start();

	/// Stop the common receiver.
	VOID Stop();

	///Get connect socket
	///return @ CSCTCPSocket * pointer
	CSCTCPSocket* GetSocket()
	{
		return m_Socket;
	}

private:
	ISCParser* m_Parser;
	CSCTCPSocket* m_Socket;
    HANDLE m_hEvent;
	HANDLE m_Thread;
	DWORD m_dwThreadID;
	int m_iStatus;
	ReceiverNotify m_Notify;
	char	m_pRecvBuf[RECVBUFMAX];
private:
	static DWORD WINAPI Callback(LPVOID pParam);	
};

#endif // !defined(AFX_SCCOMMONRECEIVER_H__A97CAF90_BAFC_4E75_B95E_09F51B7FCCDE__INCLUDED_)
