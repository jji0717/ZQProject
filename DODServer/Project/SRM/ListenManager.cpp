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
//#include "common.h"
#include "listenmanager.h"
#include"TCPConnection.h"
#include"SRManager.h"
//a global pointer for CListenManager
CListenManager* CListenManager::s_ListenManager = NULL;
//start thread
VOID CListenManager::Start()
{
 try
 {
	//ResetEvent(m_hEvent);
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
  
	if ( m_pSocket->Bind(m_IP, m_Port) != SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "CSCTCPSocket bind fail, desip: %s  port: %d.", m_IP, m_Port );
		delete m_pSocket;
		m_pSocket = NULL;
		return;
	}
	Clog( LOG_DEBUG, "CSCTCPSocket bind successfully." );

	if (m_pSocket->Listen(5)!= SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "CSCTCPSocket listen fail." );
		delete m_pSocket;
		m_pSocket = NULL;
		return;
	}
	Clog( LOG_DEBUG, "CSCTCPSocket listen successfully." );
	
	m_Thread = CreateThread(NULL,0, ListenThread,(LPVOID)this,0,NULL);
	Clog( LOG_DEBUG, "CListenManager::ListenThread created." );	
	}
   catch(...)
  {
    Clog( LOG_DEBUG, _T("catch Error in CListenManager::Start") );
  }

}
//stop thread
VOID CListenManager::Stop()
{
//m_ExitThread(if
	try
	{
       // m_pSocket->Close();
	    SetEvent(m_hEvent);
		printf("CListenManager::Stop1");
	    TRACE(_T("----Enter CListenManager::Stop(). \r\n"));
	    DWORD dwResult = WaitForSingleObject( m_Thread, INFINITE );
        //DWORD dwResult = WaitForSingleObject( m_Thread, 100 );
        printf("CListenManager::Stop2");
	   // m_pSocket->Close();
       // delete  m_pSocket;
	}

catch(...)
  {
    Clog( LOG_DEBUG, _T("catch Error in CListenManager::Stop") );
  }

}
// when connection is lost, notify it
void CListenManager::Notify(INT32 status,CSCTCPSocket* pSocket)
{
if(s_ListenManager!=NULL)
{
	try
	{
     s_ListenManager->m_pOwner->SetState(pSocket,false);
	 s_ListenManager->m_pOwner->CloseConnection(pSocket);
     Clog( LOG_DEBUG, "DisConnect from client" );
	}
catch(...)
  {
    Clog( LOG_DEBUG, _T("catch Error in CListenManager::Notify") );
  }
}

}
//thread bofy
DWORD WINAPI CListenManager::ListenThread(LPVOID pParam)
{
	try
	{
	Clog( LOG_DEBUG, "Start ListenThread." );
	CListenManager* listener = (CListenManager*)pParam;
	HANDLE event = listener->m_hEvent;
	CSCTCPSocket *pSocket = listener->m_pSocket;
	CClientManager*  pOwner;
    pOwner=listener->m_pOwner;
	//always to listen and accpet
	while(WaitForSingleObject(event,100)!=WAIT_OBJECT_0)
	{	
	   // Clog( LOG_DEBUG, "Listen Thread break1" );
/////////////////////////////////////////////////
       CSCTCPSocket* pClientSocket=NULL;
       TIMEVAL time;
	   time.tv_usec=100;
	   time.tv_sec=0;
       FD_SET readset;
	   FD_ZERO(&readset);
	   FD_SET(pSocket->GetSocket(), &readset);
	   int result;
       result=select(1,&readset,NULL,NULL,&time);
	   if(result!=0)
	   {
	      pClientSocket = pSocket->Accept();
	   }
	    ///////////////////////////////////
        //CSCTCPSocket* pClientSocket = pSocket->Accept();
      	if ( pClientSocket != NULL )
		{ 
               // Clog( LOG_DEBUG, "Listen Thread break2" );
			    //pClientSocket->m_tmLastUpdate = GetTickCount();
				//connect success
                Clog( LOG_DEBUG, "Connect with client[IPAddr=%s,IPPort=%d]\r\n",pClientSocket->GetRemoteIP(),pClientSocket->GetRemotePort() );
				//TRACE("Connect with client[IPAddr=%s,IPPort=%d]\r\n",pClientSocket->GetRemoteIP(),pClientSocket->GetRemotePort());
				//add into list
				//2004/9/8 preceiver and psender should new->push->start at same time for lost data

                DWORD  recvtimeout=1000*10;
				//if(setsockopt(pClientSocket->GetSocket(),SOL_SOCKET,SO_RCVTIMEO,(char*)&recvtimeout,sizeof(recvtimeout))!=0)
				//break ;
             
                //Clog( LOG_DEBUG, "Listen Thread break3" );
				SOCKADDR_IN addr;
				LPINT pint=new int(20);
				//int len;
				getpeername(pClientSocket->GetSocket(),LPSOCKADDR(&addr),pint);
				DWORD ip=addr.sin_addr.S_un.S_addr;
				delete  pint;

                CClientParse* pClientParse=NULL;
                CTCPConnection* pConnection=NULL;
                pClientParse=new CClientParse(pConnection);
				pClientParse->SetSessionManager(pOwner->m_pOwner->m_Sessionmanager);

				CSCCommonReceiver *pReceiver = new CSCCommonReceiver(pClientSocket,/*listener->m_pServerParser*/pClientParse,listener->m_Notify);
				CSCCommonSender *pSender = new CSCCommonSender(pClientSocket,NULL/*queue is created in sender */,listener->m_Notify);
						
				pConnection=new CTCPConnection(pSender,pReceiver);
				pConnection->SetIP(ip);
				//pOwner->m_pConnectionList.push_back(pConnection);
                pClientParse->SetConnection(pConnection);
                //Clog( LOG_DEBUG, "Listen Thread break4" );
				pOwner->AddConnection(pConnection);
		     						
		}
		//continue to accept 
	}
	printf("ListenThread Exit");
    Clog( LOG_DEBUG,_T("Exit listen Thread.") );
	}
    catch(...)
    {
      Clog( LOG_DEBUG, _T("catch Error in CListenManager::ListenThread") );
    }
return 0;
}

void CListenManager::SetHost(char* IP,int Port)
 {
    m_IP=IP;
	m_Port=Port;
 }
