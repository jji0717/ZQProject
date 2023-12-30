//ListenManager.h : 
//
/*****************************************************************************
File Name:		ListenManager.h
Author:			Simin.Xue (Interactive ZQ) 
Security:		Confidential
Description:	Listen to controllers which is connected to me, It is a thread and once accpeted, we will maintain this 
				connection.
Function Inventory:  	
Modification Log:
When		Version     Who			What
2004/7/19	1.0			Simin.Xue	Original Program
*****************************************************************************/
#pragma once


#include "sccommonreceiver.h"
#include "sccommonsender.h"
#include "DeviceInfoParser.h"

class CListenManager
{
public: //attribute

	///define a static global for pointer
	static CListenManager* s_ListenManager;

	///define a receiver list for connected socket
	CSCCommonReceiver::List m_receiver;

	///define a sender list for connected socket
	CSCCommonSender::List m_sender;

	///a parser to parse message from server
	CDeviceInfoParser *m_pServerParser;

	// include ip and port etc.
	struIP m_sysConfig;

	// if true, the object have connections in current one at least.
	bool m_bConnected;

	bool m_bFreeAllSocket;

	// use a thread deal heart beat mechanism.
	CRITICAL_SECTION	m_csReleaseSocket;		// protect socket release process

	///construct
	CListenManager(struIP * pSysConfig)
	{
		ASSERT(pSysConfig != NULL);
		memcpy(&m_sysConfig, pSysConfig, sizeof(struIP));
		m_pSocket = NULL;
		m_Thread = NULL;
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_Notify = Notify;

		m_bConnected = false;

		m_bFreeAllSocket = false;

		// set this pointer
		s_ListenManager = this;

		//new client Parser
		m_pServerParser = new CDeviceInfoParser( NULL );

		InitializeCriticalSection( &m_csReleaseSocket );
	}
	///deconstruct
	virtual ~CListenManager(void)
	{
		if(m_hEvent)
		{
			BOOL bTmp = CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}

		//free Parser
		if (m_pServerParser!=NULL)
		{
			delete m_pServerParser;
			m_pServerParser = NULL;
		}

		DeleteCriticalSection( &m_csReleaseSocket );

		TRACE(_T(" ~CListenManager() over \r\n") );
	}

	//method
	/// Start listen.
	VOID Start();

	/// Stop listen.
	VOID Stop();

	/// push data into sender queue
	/// @param data(IN) - data which is to be sended
	///        ip(IN) -   which ip socket
	void SendData(CSCMemoryBlockPtr data,CSCTCPSocket* pSocket=NULL) ; //,struIP ip);

	/// push data into sender queue
	/// @param data(IN) - data which is to be sended
	void SendDataToAll(CSCMemoryBlockPtr data);

	///Free all Socket which is connected to
	void FreeAllSocket();

	void ErrorOneSocket(CSCTCPSocket *pSocket);
	static void DeleteErrorSocket();

private:
	
    HANDLE m_hEvent; // thread event handle
	HANDLE m_Thread;//thread handle
	DWORD  m_dwThreadID;

	ReceiverNotify m_Notify; // callback pointer
	
	CSCTCPSocket *m_pSocket; //Listen socket
private:
	///thread body. because we can not wait to connect,we should set a thread to manage connection
	///@param class pointer
	///@return UINT listen result FALSE - failed, TRUE - success
	static DWORD WINAPI ListenThread(LPVOID pParam); 

	/// callback to notify connection is lost
	///@param connection status = SC_RECEIVER_THREAD_ERROR || SC_SENDER_THREAD_ERROR
	///       pSocket - lost socket
	static void Notify(INT32 status,CSCTCPSocket* pSocket);

};
