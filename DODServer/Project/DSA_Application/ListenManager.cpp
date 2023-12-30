//ListenManager.cpp : 
//
/*****************************************************************************
File Name:		ListenManager.cpp
Author:			Simin.Xue (Interactive ZQ) 
Security:		Confidential
Description:	Listen to clients which is connected to me, It is a thread and once accpeted, we will maintain this 
				connection.
Function Inventory:  	
Modification Log:
When		Version     Who			What
2004/7/19	1.0			Simin.Xue	Original Program
*****************************************************************************/
#include "StdAfx.h"
#include "common.h"
#include "listenmanager.h"
//#include "CFileWatcher.h"
#include "DeviceAgent.h"
//#include "ScheduleManager.h"
//extern CScheduleManager *gScheduleManager;

//a global pointer for CListenManager
CListenManager* CListenManager::s_ListenManager = NULL;

// use a thread deal heart beat mechanism.
bool CListenManager::m_bCheckStatusThreadAlive = false;
int  CListenManager::m_iInterval_Del_New = 2000;	// 

// check the current status of all socket connected, 
// if a socket hasn't receive heartbeat message in long time,
// close the socket.
DWORD WINAPI CListenManager::CheckStatusThread(LPVOID pParam)
{
	Clog( LOG_DEBUG,_T("Enter CheckStatusThread."));

	CListenManager * pManager = (CListenManager*)pParam;
	HANDLE event = pManager->m_ShutdownEvent;
	m_bCheckStatusThreadAlive = true;

	DWORD dwCurrTime = 0, dwOldTime = GetTickCount();
	while( WaitForSingleObject(event, m_iInterval_Del_New )!=WAIT_OBJECT_0 )
	{		
		dwCurrTime = GetTickCount();

		// check socket connection
		EnterCriticalSection(&(pManager->m_csReleaseSocket));
		
		dwCurrTime = GetTickCount();
		CSCCommonReceiver::List::iterator it;		
		for (  it = pManager->m_receiver.begin(); it != pManager->m_receiver.end(); it++)
		{
			CSCTCPSocket *pSocket = (*it)->GetSocket();
			if( (dwCurrTime - pSocket->m_tmLastUpdate) > 6*m_iInterval_Del_New )
			{
				TRACE(_T("----CheckStatusThread: Close one socket. \r\n"));
				pSocket->Close();
			}
		}

		LeaveCriticalSection(&(pManager->m_csReleaseSocket));
	}

	m_bCheckStatusThreadAlive = false;
	return 0;
}

//start thread
VOID CListenManager::Start()
{
	ResetEvent(m_hEvent);

	ASSERT(m_pSocket==NULL);
	m_pSocket = new CSCTCPSocket;
	Clog( LOG_DEBUG, "CSCTCPSocket is new." );

	if ( m_pSocket->Create() != SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "Create CSCTCPSocket fail." );
		delete m_pSocket;
		m_pSocket = NULL;
		return;
	}
	Clog( LOG_DEBUG, "CSCTCPSocket is created." );

	_tcscpy( m_sysConfig.szLocalIP, (m_pSocket->GetLocalIP()).GetBuffer( 0 ) );
	if ( m_pSocket->Bind( m_sysConfig.szLocalIP, m_sysConfig.wPort ) != SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "CSCTCPSocket bind fail, desip: %s  port: %d.", m_sysConfig.szLocalIP, m_sysConfig.wPort );
		delete m_pSocket;
		m_pSocket = NULL;
		return;
	}
	Clog( LOG_DEBUG, "CSCTCPSocket bind successfully." );

	if (m_pSocket->Listen(MAX_CONNECTION)!= SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "CSCTCPSocket listen fail." );
		delete m_pSocket;
		m_pSocket = NULL;
		return;
	}
	Clog( LOG_DEBUG, "CSCTCPSocket listen successfully." );

	//m_Thread = AfxBeginThread(&CListenManager::ListenThread,this ); //, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED );
	m_dwThreadID = 103;
	m_Thread = CreateThread(NULL,0, ListenThread,(LPVOID)this,0,&m_dwThreadID);
	Clog( LOG_DEBUG, "CListenManager::ListenThread created." );

	m_ShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_dwCheckThreadID = 103;
	m_CheckStatusThread = CreateThread(NULL,0, CheckStatusThread,(LPVOID)this,0,&m_dwThreadID);
	Clog( LOG_DEBUG,_T("m_CheckStatusThread Created.") );
}

//stop thread
VOID CListenManager::Stop()
{
	m_pSocket->Close();

	SetEvent(m_hEvent);

	TRACE(_T("----Enter CListenManager::Stop(). \r\n"));
	//Sleep(100);

	DWORD dwResult = WaitForSingleObject( m_Thread, INFINITE );
/*
		switch( dwResult )
		{
			case WAIT_TIMEOUT:
			TRACE(_T("----Abnormal Release ListenThread: WAIT_TIMEOUT. \r\n"));
			break;
			case WAIT_FAILED:		
			TRACE(_T("----Abnormal Release ListenThread: WAIT_FAILED. \r\n"));
			break;
			case WAIT_ABANDONED:		
			TRACE(_T("----Abnormal Release ListenThread: WAIT_ABANDONED. \r\n"));
			break;
			case WAIT_OBJECT_0:		
			TRACE(_T("----Abnormal Release ListenThread: WAIT_OBJECT_0. \r\n"));
			break;
		}*/

	SetEvent( m_ShutdownEvent );

	dwResult = WaitForSingleObject( m_CheckStatusThread, INFINITE );

	CloseHandle( m_ShutdownEvent );

	TRACE(_T("----Stop Thread: ListenThread. \r\n"));
}

// when connection is lost, notify it
void CListenManager::Notify(INT32 status,CSCTCPSocket* pSocket)
{
	TRACE(_T("----Enter Notify(). \r\n"));
	//ignore no error notify
	if (status !=  SC_RECEIVER_THREAD_ERROR &&  status !=SC_SENDER_THREAD_ERROR )
		return;

	//if App exit and disconnect, no need to restart
	//if ( gAppInfo.bExit )
	//	return;

	//EnterCriticalSection(&s_ListenManager->m_csReleaseSocket);

	if( (!s_ListenManager->m_bFreeAllSocket) && pSocket )
	{
		//TRACE("\n Disconnect with client[IPAddr=%s,IPPort=%d]",pSocket->GetRemoteIP(),pSocket->GetRemotePort());
		TRACE("\n Disconnect with client[IPAddr=,IPPort=]" );
		Clog( 3, "\n Disconnect with client[IPAddr=,IPPort=]" );

		// remove one socket from list,
		s_ListenManager->FreeOneSocket(pSocket);

		if( s_ListenManager->m_sender.size() < 1 )
			s_ListenManager->m_bConnected = false;
	}

	//LeaveCriticalSection(&s_ListenManager->m_csReleaseSocket);

	TRACE(_T("----Leave Notify(). \r\n"));
}

//thread bofy
DWORD WINAPI CListenManager::ListenThread(LPVOID pParam)
{
	Clog( LOG_DEBUG, "Start ListenThread." );

	CListenManager* listener = (CListenManager*)pParam;
	HANDLE event = listener->m_hEvent;
	CSCTCPSocket *pSocket = listener->m_pSocket;
	

	//always to listen and accpet
	while(WaitForSingleObject(event,100)!=WAIT_OBJECT_0)
	{	
		CSCTCPSocket* pClientSocket = pSocket->Accept();
		if ( pClientSocket != NULL )
			{ 
				pClientSocket->m_tmLastUpdate = GetTickCount();
				//connect success
				TRACE("Connect with client[IPAddr=%s,IPPort=%d]\r\n",pClientSocket->GetRemoteIP(),pClientSocket->GetRemotePort());
				Clog( 3, "Connect with client[IPAddr=%s,IPPort=%d]\r\n",pClientSocket->GetRemoteIP(),pClientSocket->GetRemotePort());
								//add into list
				//2004/9/8 preceiver and psender should new->push->start at same time for lost data
				CSCCommonReceiver *pReceiver = new CSCCommonReceiver(pClientSocket,listener->m_pServerParser,listener->m_Notify);
				CSCCommonSender *pSender = new CSCCommonSender(pClientSocket,NULL/*queue is created in sender */,listener->m_Notify);

				listener->m_receiver.push_back(pReceiver);	
				listener->m_sender.push_back(pSender);		

				//should accroding to this sequence
				pSender->Start();
				pReceiver->Start();

				listener->m_bConnected = true;
				//g_pFileWatcher->OnConnectSuccess();								
			}
		//continue to accept 
	}

	return 0;
}


void CListenManager::SendData(CSCMemoryBlockPtr data,CSCTCPSocket* pSocket)
{	
	EnterCriticalSection(&m_csReleaseSocket);
	BYTE flag = 0;
	/// Travel list, get ip socket
	for (CSCCommonSender::List::iterator it = m_sender.begin(); it != m_sender.end(); it++)
	{
		CSCTCPSocket *pItSocket = (*it)->GetSocket();
		if (pSocket== pItSocket)
		{//((CSCCommonSender*)
			(*it)->Push(data);			
			flag = true;
		}
	}
	Sleep( 100 );
	//if not connect, free it
	//if ( !flag )
		//CSCMemoryBlock::FreeBlock(data.GetBlock());
	LeaveCriticalSection(&s_ListenManager->m_csReleaseSocket);
}

// push data into sender queue
void CListenManager::SendDataToAll(CSCMemoryBlockPtr data)
{
	EnterCriticalSection(&m_csReleaseSocket);
	// send data flag
	BYTE flag = 0;
	/// Travel list, get ip socket
	TRACE( _T("SendDataToAll(), Socket count: %d\r\n"), m_sender.size() );
	for (CSCCommonSender::List::iterator it = m_sender.begin(); it != m_sender.end(); it++)
	{
		CSCTCPSocket *pSocket = (*it)->GetSocket();
		if( pSocket )
		{
			TRACE( _T("SendDataToAll, socket: %d"), (int)pSocket );
			Clog( LOG_DEBUG, _T("SendDataToAll, socket: %d"), (int)pSocket);
			(*it)->Push(data);
			//CSCMemoryBlock::FreeBlock(data.GetBlock());
			flag = true;
		}
	}
	Sleep( 100 );

	//if not connect, free it
	//if ( !flag )
		//CSCMemoryBlock::FreeBlock(data.GetBlock());
	LeaveCriticalSection(&s_ListenManager->m_csReleaseSocket);
}

///Free one socket which is put into sender and receiver
void CListenManager::FreeOneSocket(CSCTCPSocket *pSocket)
{
	TRACE(_T("----Enter FreeOneSocket(). \r\n"));
	
	//remove socket from sender list,
	for ( CSCCommonSender::List::iterator iter =  m_sender.begin(); iter !=  m_sender.end(); iter++)
	{
		if ( (*iter)->GetSocket() == pSocket )
		{
			(*iter)->Stop();

			delete (*iter);

			/// Found, erase that
			m_sender.erase(iter);

			TRACE( "m_sender.erase(), new size: %d\r\n", m_sender.size() );
			Clog( LOG_DEBUG, "m_sender.erase(), new size: %d", m_sender.size() );
			break;			
		}
	}

	for ( CSCCommonReceiver::List::iterator it = m_receiver.begin(); it != m_receiver.end(); it++)
	{
		if ( (*it)->GetSocket() == pSocket )
		{
			//if no connect send to server
			//free socket
			pSocket->Close();

			if( pSocket )
			{
				delete pSocket;
				pSocket = NULL;
			}

			(*it)->Stop();

			delete (*it);
			/// Found, erase that
			m_receiver.erase(it);
			TRACE( "m_receiver.erase(), new size: %d\r\n", m_receiver.size() );
			Clog( LOG_DEBUG, "m_receiver.erase(), new size: %d", m_receiver.size() );
			
			break;			
		}
	}

	TRACE(_T("----FreeOneSocket() over. \r\n"));
}

///Free all sockets which is put into sender and receiver
void CListenManager::FreeAllSocket()
{
		m_bFreeAllSocket = true;

		m_bConnected = false;

		if (m_pSocket)
		{
			m_pSocket->Close();
		}

		//stop thread
		Stop();

		EnterCriticalSection(&m_csReleaseSocket);

		TRACE(_T("----Release Thread: m_receiver. \r\n"));
		for ( CSCCommonReceiver::List::iterator it = m_receiver.begin(); it != m_receiver.end(); it++)
		{
			CSCTCPSocket *pSocket = (*it)->GetSocket();
			pSocket->Close();

			//close socket force thread stopped, but it can not be delete because thread will notify disconnect
			//so delete it in notify function
			if (pSocket)
			{
				delete pSocket;
				pSocket = NULL;
			}

			(*it)->Stop();
			delete (*it);
		}
		m_receiver.clear();

		//remove socket from sender list
		TRACE(_T("----Release Thread: m_sender. \r\n"));
		for (CSCCommonSender::List::iterator iter = m_sender.begin(); iter != m_sender.end(); iter++)
		{
			(*iter)->Stop();
			delete *iter;
			//same socket pointer in receiver, so should not delete again
		}
		TRACE(_T("----m_sender.clear(). \r\n"));
		m_sender.clear();

		LeaveCriticalSection(&m_csReleaseSocket);

		Sleep( 3000 );
		//free listen socket
		if (m_pSocket)
		{
			delete m_pSocket;
			m_pSocket = NULL;
		}
}

