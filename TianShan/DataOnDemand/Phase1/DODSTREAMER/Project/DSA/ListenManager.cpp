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
//#include "StdAfx.h"
#include "common.h"
#include "listenmanager.h"
//#include "CFileWatcher.h"
#include "DeviceAgent.h"
//#include "ScheduleManager.h"
extern BOOL g_IsStopAll;

//a global pointer for CListenManager
CListenManager* CListenManager::s_ListenManager = NULL;

//start thread
VOID CListenManager::Start()
{
	ResetEvent(m_hEvent);
	g_IsStopAll = FALSE;
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
	Clog( LOG_DEBUG, "CSCTCPSocket created successfully." );

	_tcscpy( m_sysConfig.szLocalIP, _T("") );
	if ( m_pSocket->Bind( m_sysConfig.szLocalIP, m_sysConfig.wPort ) != SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "CSCTCPSocket bind fail, desip: %s  port: %d.", m_sysConfig.szLocalIP, m_sysConfig.wPort );
		delete m_pSocket;
		m_pSocket = NULL;
		return;
	}
	_tcscpy( m_sysConfig.szLocalIP, (m_pSocket->GetLocalIP()).GetBuffer( 0 ) );
	Clog( LOG_DEBUG, "CSCTCPSocket bind successfully, desip: %s  port: %d.", m_sysConfig.szLocalIP, m_sysConfig.wPort );

	if (m_pSocket->Listen(MAX_CONNECTION)!= SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "CSCTCPSocket listen fail." );
		delete m_pSocket;
		m_pSocket = NULL;
		return;
	}
	Clog( LOG_DEBUG, "CSCTCPSocket listen successfully." );

	m_dwThreadID = 103;
	m_Thread = CreateThread(NULL,0, ListenThread,(LPVOID)this,0,&m_dwThreadID);
	Clog( LOG_DEBUG, "CListenManager::ListenThread created." );
}

//stop thread
VOID CListenManager::Stop()
{
	DWORD dwResult;
	if( m_hEvent )
	{
		SetEvent(m_hEvent);
		Sleep(2);
	}
	
	if( m_Thread )	
	{
		dwResult = WaitForSingleObject( m_Thread, INFINITE );
		CloseHandle(m_Thread);
		m_Thread = NULL;
	}

	if( m_pSocket )
	{
		m_pSocket->Close();
		delete m_pSocket;
		m_pSocket = NULL;
	}

	FreeAllSocket();

	TRACE(_T("----Stop Thread: ListenThread. \r\n"));
}

// when connection is lost, notify it
void CListenManager::Notify(INT32 status,CSCTCPSocket* pSocket)
{
	TRACE(_T("----Enter Notify(). \r\n"));
	Clog(LOG_DEBUG,_T("----Enter Notify() status=%d "), status);
	//ignore no error notify
	if (status !=  SC_RECEIVER_THREAD_ERROR &&  status !=SC_SENDER_THREAD_ERROR )
		return;

	if( (!s_ListenManager->m_bFreeAllSocket) && pSocket )
	{
		Clog( LOG_DEBUG,"Disconnect client[IPAddr=%s,IPPort=%d]",pSocket->GetRemoteIP(),pSocket->GetRemotePort());
		TRACE("\n Disconnect with client[IPAddr=,IPPort=]" );

		// remove one socket from list,
		s_ListenManager->ErrorOneSocket(pSocket);

		if( s_ListenManager->m_sender.size() < 1 )
			s_ListenManager->m_bConnected = false;
	}
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
		DWORD dwError = 0;
		if( pSocket->CheckRead(500, &dwError)==1 )
		{
			CSCTCPSocket* pClientSocket = pSocket->Accept();
			if ( pClientSocket != NULL )
			{ 
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
			}
		}
		DeleteErrorSocket();
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
	LeaveCriticalSection(&s_ListenManager->m_csReleaseSocket);
}

// push data into sender queue
void CListenManager::SendDataToAll(CSCMemoryBlockPtr data)
{
	EnterCriticalSection(&m_csReleaseSocket);
	// send data flag
	BYTE flag = 0;
	/// Travel list, get ip socket
	Clog( LOG_DEBUG, _T("SendDataToAll(), Socket count: %d\r\n"), m_sender.size() );
	try
	{
		for (CSCCommonSender::List::iterator it = m_sender.begin(); it != m_sender.end(); it++)
		{
			CSCTCPSocket *pSocket = (*it)->GetSocket();
			if( pSocket )
			{
				TRACE( _T("SendDataToAll, socket: %d\n"), (int)pSocket );
				Clog( LOG_DEBUG, _T("SendDataToAll, socket: %d"), (int)pSocket);
				(*it)->Push(data);
				flag = true;
			}
		}
	}
	catch(...)
	{
		Clog( LOG_ERR, _T("SendDataToAll(), error: code=%d\n"), GetLastError());
	}
	Sleep( 5 );

	LeaveCriticalSection(&s_ListenManager->m_csReleaseSocket);
}

///Free one socket which is put into sender and receiver
void CListenManager::ErrorOneSocket(CSCTCPSocket *pSocket)
{
	Clog(0, "Client(ip=%s, port=%d) is set to invalid.", pSocket->GetRemoteIP(), pSocket->GetRemotePort() );
	//remove socket from sender list,
	for ( CSCCommonSender::List::iterator iter =  m_sender.begin(); iter !=  m_sender.end(); iter++)
	{
		if ( (*iter)->GetSocket() == pSocket )
		{
			(*iter)->m_nError = 1;
			break;			
		}
	}

	for ( CSCCommonReceiver::List::iterator it = m_receiver.begin(); it != m_receiver.end(); it++)
	{
		if ( (*iter)->GetSocket() == pSocket )
		{
			(*it)->m_nError = 1;
			break;
		}
	}
}

void CListenManager::DeleteErrorSocket()
{
	CSCTCPSocket *pSocket = NULL;
	for ( CSCCommonSender::List::iterator iter =  s_ListenManager->m_sender.begin(); 
	      iter !=  s_ListenManager->m_sender.end(); 
	      iter++)
	{
		if ( (*iter)->m_nError == 1 )
		{
			pSocket = (*iter)->GetSocket();
			(*iter)->Stop();
			s_ListenManager->m_sender.erase(iter);
			break;	
		}
	}

	if( pSocket==NULL )
		return ;

	for ( CSCCommonReceiver::List::iterator it = s_ListenManager->m_receiver.begin(); 
		  it != s_ListenManager->m_receiver.end(); 
		  it++)
	{
		if ( (*it)->GetSocket() == pSocket )
		{
			//Clog(0, "ReceiveClient(ip=%s, port=%d) is to be closed.", pSocket->GetRemoteIP(), pSocket->GetRemotePort() );
			(*it)->Stop();
			s_ListenManager->m_receiver.erase(it);
			break;	
		}
	}
  
	  if( pSocket )
	  {
		  Clog(0, "Client(ip=%s, port=%d) is to be closed.", pSocket->GetRemoteIP(), pSocket->GetRemotePort() );
		  pSocket->Close();
		  delete pSocket;
		  pSocket = NULL;
	  }
}

///Free all sockets which is put into sender and receiver
void CListenManager::FreeAllSocket()
{
	Clog(LOG_DEBUG,_T("----Release Thread: m_receiver. FreeAllSocket"));
	
	m_bFreeAllSocket = true;
	
	m_bConnected = false;
		
	EnterCriticalSection(&m_csReleaseSocket);

	TRACE(_T("----Release Thread: m_receiver. \r\n"));
	for ( CSCCommonReceiver::List::iterator it = m_receiver.begin(); it != m_receiver.end(); it++)
	{
		if((*it) == NULL)
			continue;

		CSCTCPSocket *pSocket = (*it)->GetSocket();
		pSocket->m_nError = 1;
		pSocket->Close();

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

	
	if (m_pSocket)
	{
		m_pSocket->Close();
	}

	Sleep( 2 );
	//free listen socket
	if (m_pSocket)
	{
		delete m_pSocket;
		m_pSocket = NULL;
	}
}

