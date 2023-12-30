
/*
**	FILENAME			SingleConnect.cpp
**						
**	PURPOSE				The file is a implement file, used along with SingleConnect.h.
**						a class CSingleConnect is defined in the file, and is used to 
**						manager auto-connect process to server.
**						
**						
**	CREATION DATE		22-11-2004
**	LAST MODIFICATION	22--2004
**
**	AUTHOR				Leon.li (ZQ Interactive)
**
**
*/
#include "StdAfx.h"
#include "SingleConnect.h"
#include"SRManager.h"
CSingleConnect* CSingleConnect::s_SingleConnect = NULL;
//start thread
VOID CSingleConnect::Start()
{
	Clog( LOG_DEBUG, _T("Enter Connect Thread Start.") );
	m_iStatus = 0;
	if( m_bThreadExit )
	{
		ResetEvent(m_hEvent);
		ResetEvent(m_hShutdownEvent);
		m_dwThreadID = 104;
		m_Thread = CreateThread(NULL,0, ConnectThread,(LPVOID)this,0,&m_dwThreadID);
		Clog( LOG_DEBUG, _T("Create Connect Thread.") );
	}
}
//stop thread
VOID CSingleConnect::Stop()
{
	int iRtn = SCS_SUCCESS, iCount=0;
	Clog( LOG_DEBUG, _T("Enter Stop Connect Thread.") );
	//EndHeartBeatThread();
	if( !m_bThreadExit )
		SetEvent(m_hEvent);
		if( WaitForSingleObject( m_hShutdownEvent, 4000 ) != WAIT_OBJECT_0 )
		{			
			if( !m_bThreadExit )
			{
				Clog( LOG_DEBUG, _T("Connect Thread Exit Call TerminateThread.") );
				TerminateThread( m_Thread, 0x12 );
			}
		}
	else
	Clog( LOG_DEBUG, _T("Connect Thread Exit Normally.") );
	Clog( LOG_DEBUG, "Close Socket." );
	m_iStatus = 3;
	m_Socket.Close();
	m_bConnected = false;
	Clog( LOG_DEBUG, "Socket Closed." );
}
// when connection is lost, notify it
void CSingleConnect::Notify(INT32 status,CSCTCPSocket* pSocket)
{
	s_SingleConnect->m_pOwner->SetState(false);
	s_SingleConnect->m_pOwner->CloseSocket();
    if( s_SingleConnect->m_bConnected )
	{
       s_SingleConnect->Start();
       s_SingleConnect->m_bConnected=false;
       
	}
}
//connection thread body
DWORD WINAPI CSingleConnect::ConnectThread(LPVOID pParam)
{
	Clog( LOG_DEBUG, "Start ConnectThread." );
	CSingleConnect* app = (CSingleConnect*)pParam;
	HANDLE event = app->m_hEvent;
	CDSAResource* pOwner=app->m_pOwner;
    CSCTCPSocket* pSocket=new CSCTCPSocket();
	//app->m_Socket.Close();
	if(pSocket->Create() != SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "Create Socket Fail, ConnectThread Exit." );
		app->m_bThreadExit = true;
		SetEvent( app->m_hShutdownEvent );
		return 0x12;
	}
	app->m_bThreadExit = false;	
/////////////////////////////////////////////////////////////////////////////////////////////////
    while(WaitForSingleObject(event, 1000 )!=WAIT_OBJECT_0)
	{	
		if(pSocket->Connect(app->m_IP, app->m_Port ) == SCS_SUCCESS)
	    {
			app->m_bConnected = TRUE;
			Clog( LOG_DEBUG, _T("Connect Success.") );
			break;
		}
	}
	if(!app->m_bConnected)
	{
		Clog( LOG_DEBUG, _T("Connect Fail. Close Socket.") );
		pSocket->Close();
		app->m_bThreadExit = true;
		SetEvent( app->m_hShutdownEvent );
		return 0x12;
	}
	Clog( LOG_DEBUG, _T("Connect Thread Exit.") );
	CTCPConnection* pConnection=NULL;
	CDSAParse* pParse;
	pParse=new CDSAParse(pOwner);
	pParse->SetSessionManager(pOwner->m_pOwner->m_pOwner->m_Sessionmanager);
   	CSCCommonReceiver* pCSCCommonReceiver = new CSCCommonReceiver(pSocket/*&app->m_Socket*/, /*app->m_pServerParser*/pParse, CSingleConnect::Notify);
	CSCCommonSender*  pCSCCommonSender = new CSCCommonSender(pSocket/*&app->m_Socket*/, NULL, CSingleConnect::Notify );//NULL);
    pConnection=new CTCPConnection(pCSCCommonSender,pCSCCommonReceiver);
	pOwner->SetConnection(pConnection);
    pConnection->Start();
    app->m_bThreadExit = true;
	SetEvent( app->m_hShutdownEvent );
	return 0x12;
}

void CSingleConnect::SetServer(char* IP,int Port)
{
   m_IP=IP;
   m_Port=Port;
}
