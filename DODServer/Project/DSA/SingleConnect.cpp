
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

//#include "StdAfx.h"
#include "SingleConnect.h"
//#include "CSCDeviceInfo.h"
#include "common.h"

//a global pointer for CSingleConnect
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
	//int iRtn = SCS_SUCCESS, iCount=0;
	Clog( LOG_DEBUG, _T("Enter Stop Connect Thread.") );

	EndHeartBeatThread();

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
	Clog( LOG_DEBUG, _T("Receive Connection Close Message.") );
	//ignore no error notify
	if (status !=  SC_RECEIVER_THREAD_ERROR &&  status !=SC_SENDER_THREAD_ERROR )
		return;
	if( !s_SingleConnect )
		return;
	if( s_SingleConnect->m_iStatus == 3 )
		return;
	//if(status == SC_RECEIVER_THREAD_ERROR)
	if( s_SingleConnect->m_bConnected )
	{
		Clog( LOG_DEBUG, _T("Connection break down.") );
		_stprintf( g_szControllerStatusText, _T("%s"), _T("Status: Connection with Playback Server break down.") );
		//::SendMessage( CDeviceInfoParser::s_pDeviceInfoParser->m_hWnd, UPDATECONTROLLERSTATUS, 0, 0 );

		s_SingleConnect->m_bConnected = false;
		//s_SingleConnect->m_pServerParser->m_bHaveReportList = false;
		s_SingleConnect->m_Socket.Close(); //m_Socket.Close()
		Clog( LOG_DEBUG, _T("Socket Close Success.") );

		if( s_SingleConnect->m_pSender )
		{
			s_SingleConnect->m_pSender->Stop();
			delete s_SingleConnect->m_pSender;
			s_SingleConnect->m_pSender = NULL;
		}

		if( s_SingleConnect->m_pReceiver )
		{
			s_SingleConnect->m_pReceiver->Stop();
			delete s_SingleConnect->m_pReceiver;
			s_SingleConnect->m_pReceiver = NULL;
		}

		Clog( LOG_DEBUG, _T("s_SingleConnect->Start().") );
		// restart thread
		s_SingleConnect->Start();
	}
}

//connection thread body
DWORD WINAPI CSingleConnect::ConnectThread(LPVOID pParam)
{
	Clog( LOG_DEBUG, "Start ConnectThread." );

	CSingleConnect* app = (CSingleConnect*)pParam;
	HANDLE event = app->m_hEvent;
	//extern CDeviceInfoParser * gServerParser;
	//extern CSCNVODPumpManager *gServerParser;
	
	app->m_bConnected = FALSE;	

	//app->m_Socket.Close();
	if(app->m_Socket.Create() != SCS_SUCCESS)
	{
		//LogMyEvent( EVENTLOG_WARNING_TYPE, 0, _T("Create Socket Fail, ConnectThread Exit.") );
		Clog( LOG_DEBUG, "Create Socket Fail, ConnectThread Exit." );
		app->m_bThreadExit = true;
		SetEvent( app->m_hShutdownEvent );
		return 0x12;
	}
	
	app->m_bThreadExit = false;	

	//INT32 sleepsec = 5;
	//for(int i = 0; i < 8; i++)
	while(WaitForSingleObject(event, app->m_sysConfig.iReConnInterval*1000 )!=WAIT_OBJECT_0)
	{		
		if( app->m_iStatus == 3)
		{
			Clog( LOG_DEBUG, _T("Break Connect Loop, m_iStatus=3.") );
			break;
		}
		//Clog( LOG_DEBUG, "Try Connect." );
		if(app->m_Socket.Connect(app->m_sysConfig.cAddr, app->m_sysConfig.wPort ) == SCS_SUCCESS)
		{
			_stprintf( g_szControllerStatusText, _T("%s"), _T("Status: Connect to Playback Server successful.") );
			//::SendMessage( CDeviceInfoParser::s_pDeviceInfoParser->m_hWnd, UPDATECONTROLLERSTATUS, 0, 0 );

			_tcscpy( app->cRemoteComputerName, app->m_Socket.GetRemoteHost() );
			_tcscpy( app->cRemoteAddr, app->m_Socket.GetRemoteIP() );
			_tcscpy( app->cLocalComputerName, app->m_Socket.GetLocalHost() );
			_tcscpy( app->cLocalAddr, app->m_Socket.GetLocalIP() );
			app->wLocalPort = app->m_Socket.GetLocalPort();
			app->wRemotePort = app->m_Socket.GetRemotePort();

			app->m_pServerParser->Handshake();
			app->m_bConnected = TRUE;
			
			Clog( LOG_DEBUG, _T("Connect Success.") );

			break;
		}
	}
	
	//app->m_iStatus = 0;

	// Not connected.
	if(!app->m_bConnected)
	{
		Clog( LOG_DEBUG, _T("Connect Fail. Close Socket.") );
		app->m_Socket.Close();
		app->m_bThreadExit = true;
		SetEvent( app->m_hShutdownEvent );
		return 0x12;
	}

	Clog( LOG_DEBUG, _T("Connect Thread Exit.") );

	app->m_pReceiver = new CSCCommonReceiver(&app->m_Socket, app->m_pServerParser, CSingleConnect::Notify);
	app->m_pSender = new CSCCommonSender(&app->m_Socket, app->m_pSenderQueue, CSingleConnect::Notify );//NULL);

	app->m_pReceiver->Start();
	app->m_pSender->Start();

	app->m_bThreadExit = true;
	SetEvent( app->m_hShutdownEvent );

	return 0x12;

}

bool CSingleConnect::m_bHeartBeatThreadAlive = false;
bool CSingleConnect::m_bHeartBeatThreadExit = true;
DWORD CSingleConnect::m_iHeartBeatInterval = 3;
DWORD CSingleConnect::m_iHeartBeatReferVal = 0;

// heart beat thread function.
DWORD  WINAPI CSingleConnect::HeartBeatThread(LPVOID pParam)
{
	CSingleConnect *pMainControl = (CSingleConnect *)pParam;
	m_bHeartBeatThreadAlive = true;
	m_bHeartBeatThreadExit = false;

	// because heartbeat report is in other thread, it is impossible that both thread call p_mConnectManager->SendData() in the same time, 
	// so i use g_csSendProtect to protect the message list that will be sent.

	DWORD wCurrent;

	Clog( LOG_DEBUG, "Start HeartBeat Thread." );
	while( m_bHeartBeatThreadAlive )
	{
		wCurrent = GetTickCount();

		if( (wCurrent-m_iHeartBeatReferVal) >= m_iHeartBeatInterval*1000 )
		{//send heart beat package
			m_iHeartBeatReferVal = GetTickCount();
			if( pMainControl->m_bConnected )
				pMainControl->ReportHeartBeat();	
		}

		Sleep( 200 );
	}

	m_bHeartBeatThreadExit = true;
	Clog( LOG_DEBUG, _T("Set flag for Exit HeartBeat Thread.") );

	return 0x11;
}
// end heartbeat thread.
void CSingleConnect::EndHeartBeatThread()
{
	// close heartbeat thread
	m_bHeartBeatThreadAlive = false;
	if( WaitForSingleObject( m_HeartBeatThread, 1000 ) != WAIT_OBJECT_0 )
	{
		if( !m_bHeartBeatThreadExit )
		{
			Clog( LOG_DEBUG, "CMainControl::Unload(): Exit Hearbeat Thread by call TerminateThread()." );
			TerminateThread( m_HeartBeatThread, 0x01 );
		}
	}	
	
}

void CSingleConnect::ReportHeartBeat()
{
	//Clog( LOG_DEBUG, _T("Enter -ReportHeartBeat().") );	

	TCHAR * pcData = CSCMemoryBlock::AllocBlock(3);
	pcData[0] = '\r';
	pcData[1] = '\n';
	pcData[2] = MSG_END_FLAG;
	//_stprintf( pcData, _T("\r\n") );
	CSCMemoryBlock * mBlock = new CSCMemoryBlock( pcData, 3 );

	CSCMemoryBlockPtr pMemBlock( mBlock );

	SendData( pMemBlock );
}


// push data into sender queue
void CSingleConnect::SendData(CSCMemoryBlockPtr data)
{
	m_pSenderQueue->Push(data);
}
