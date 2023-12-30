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
//#include"ClientManager.h"
class CClientManager;
class CListenManager
{
public: //attribute
	///define a static global for pointer
	static CListenManager* s_ListenManager;

	// if true, the object have connections in current one at least.
	bool m_ExitThread;
	bool m_bConnected;
	CRITICAL_SECTION	m_Lock;		// protect socket release process
	///construct
	//CListenManager(struIP * pSysConfig)
    CListenManager(CClientManager* powner)
	{
        m_ExitThread=true;
   		m_pOwner=powner;
		m_pSocket = NULL;
		m_Thread = NULL;
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_Notify = Notify;
		m_bConnected = false;
     	s_ListenManager = this;
        m_IP="";
		InitializeCriticalSection( &m_Lock );
	}
	///deconstruct
	virtual ~CListenManager(void)
	{
        s_ListenManager=NULL;
		m_Notify=NULL;
        
        if(m_pSocket!=NULL)
		{
			m_pSocket->Close();
			delete  m_pSocket;
            m_pSocket=NULL;

		}
		if(m_hEvent)
		{
			BOOL bTmp = CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
		DeleteCriticalSection( &m_Lock );
		TRACE(_T(" ~CListenManager() over \r\n") );
	}

void	Lock() { EnterCriticalSection(&m_Lock); }
void	UnLock() { LeaveCriticalSection(&m_Lock);}
	// Start listen.
	VOID Start();
	/// Stop listen.
	VOID Stop();

    void SetHost(char* IP,int Port);
private:
    HANDLE m_hEvent; // thread event handle
	HANDLE m_Thread;//thread handle
	//DWORD  m_dwThreadID;
	CString  m_IP;
	DWORD    m_Port;
	CClientManager* m_pOwner;
	ReceiverNotify m_Notify; // callback pointer
	CSCTCPSocket *m_pSocket; // Listen socket
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
