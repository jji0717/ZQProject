
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

//a global pointer for CSingleConnect
CSingleConnect* CSingleConnect::s_SingleConnect = NULL;

extern CSCMemoryBlockQueue *m_pKitSenderQueue;
VOID CSingleConnect::Start()
{
	Clog( LOG_DEBUG, _T("Connect SRM  Thread Start.") );
	m_iStatus = 0;
// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê9ÔÂ5ÈÕ 11:48:50
	m_bConnected = false;
	m_Thread = CreateThread(NULL,0,ConnectThread,(LPVOID)this,0,&m_dwThreadID);			
}

//stop thread
VOID CSingleConnect::Stop()
{
	try
	{
		m_bContinueConnect = FALSE;
		Clog( LOG_DEBUG, _T("Enter Stop Connection function.") );
		m_bConnected = false;

		if(m_hEvent)
		{
			SetEvent(m_hEvent);
			Sleep(2);
		}
		if(m_Thread)
		{
			DWORD dwResult = WaitForSingleObject( m_Thread, 10000 );
			CloseHandle(m_Thread);
			m_Thread = NULL;
		}
		Clog( LOG_DEBUG, _T("Connect Thread Exit Normally.") );

		m_iStatus = 3;
		m_Socket.Close();
		if( m_pSender )
		{
			m_pSender->Stop();
			delete m_pSender;
			m_pSender = NULL;
		}

		if( m_pReceiver )
		{
			m_pReceiver->Stop();
			delete m_pReceiver;
			m_pReceiver = NULL;
		}
		Clog( LOG_DEBUG, "Delete send_thread and receiver_thread success" );
	}
	catch (...) 
	{
		Clog( LOG_DEBUG, _T("Stop socket exception.error=%d"),GetLastError());
	}
}

// when connection is lost, notify it
void CSingleConnect::Notify(INT32 status,CSCTCPSocket* pSocket)
{
	Clog( LOG_DEBUG, _T("Receive callback semaphore.status= %d"),status);
	//ignore no error notify
	if (status !=  SC_RECEIVER_THREAD_ERROR &&  status !=SC_SENDER_THREAD_ERROR )
		return;
	if( !s_SingleConnect )
		return;
	if( s_SingleConnect->m_iStatus == 3 )
		return;
	if( m_bConnected )
	{
		s_SingleConnect->m_iStatus = 1;
		SetEvent( s_SingleConnect->m_hDisconnectEvent );
	}
}

//connection thread body
DWORD WINAPI CSingleConnect::ConnectThread(LPVOID pParam)
{
//	Clog( LOG_DEBUG, "Start ConnectThread." );

	CSingleConnect* app = (CSingleConnect*)pParam;
	HANDLE event = app->m_hEvent;
	
	m_bConnected = FALSE;	

	int SendCount=0;
	while(WaitForSingleObject(event, 1000)!=WAIT_OBJECT_0)
	{		
		if(m_bConnected)
		{
			SendCount++;
			if(SendCount>5)
			{
				app->ReportHeartBeat();
				SendCount=0;
			}

			if( WaitForSingleObject( app->m_hDisconnectEvent, 100 ) == WAIT_OBJECT_0 )
			{	// socket disconnect 
				// free the resource assigned in the process of initialization of socket connection.
				Sleep( 2 );
				Clog( LOG_DEBUG, _T("By Disconnect Event,release all send_thread and Receiver_thread") );
				app->Disconnect();
			}
		}
		else			
		{
			if (app->m_bContinueConnect==FALSE)
				break;
			
			// connect to remote machine.
			Clog( LOG_DEBUG, _T("Restart Connect remote machine ...") );
			app->Connect();
		}
	}
	return 0x12;
}
// connect successfully, assign resource for receiving and sending data.
void CSingleConnect::Connect()
{
	if(m_Socket.Create() != SCS_SUCCESS)
	{
		Clog( LOG_DEBUG, "connect Fail,because Socket create error=%d.",GetLastError());
		return ;
	}
	try
	{
		if( m_pSender )
		{
			Clog( LOG_DEBUG, _T("Connect Operation:release Sender."));
			m_pSender->Stop();
			delete m_pSender;
			m_pSender = NULL;
		}
		if( m_pReceiver )
		{
			Clog( LOG_DEBUG, _T("Connect Operation:release Receiver."));
			m_pReceiver->Stop();
			delete m_pReceiver;
			m_pReceiver = NULL;
		}

		if( m_Socket.Connect(m_IP, m_Port) == SCS_SUCCESS)
		{
			Clog( LOG_DEBUG, _T("Connect IP port(%s:%d) OK."), m_IP,m_Port);
			m_Socket.m_nError = 0;
			m_pReceiver = new CSCCommonReceiver(&m_Socket, m_pServerParser, Notify);
			m_pSender = new CSCCommonSender(&m_Socket, NULL, Notify);//Notify );//

			m_pReceiver->Start();
			m_pSender->Start();

			m_iStatus = 0;
			m_bConnected = TRUE;

			Clog( LOG_DEBUG, _T("Connect SRM Success.") );
		}
		else
			Clog( LOG_DEBUG, _T("Connect SRM failure."));
	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("Connect SRM exception errorcode=%d"),GetLastError());
	}
}

// free the resource assigned in the process of initialization of socket connection.
void CSingleConnect::Disconnect()
{
	try
	{
		m_bConnected = false;
		m_Socket.Close();

		if( m_pSender )
		{
			m_pSender->Stop();
			delete m_pSender;
			m_pSender = NULL;
		}
		if( m_pReceiver )
		{
			m_pReceiver->Stop();
			delete m_pReceiver;
			m_pReceiver = NULL;
		}
		Clog( LOG_DEBUG, _T("Disconnect successful.") );
	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("Disconnect Socket release all thread error=%d"),GetLastError());
	}
}

// push data into sender queue
void CSingleConnect::SendXML(CMarkup &data,BOOL showlog)
{
	if(m_bConnected ==false) 
		return;
	CString sTmp = data.GetDoc();
	if (showlog)
		Clog( LOG_DEBUG, _T("send message %s"),sTmp );

	try
	{
		int read = sTmp.GetLength();

		TCHAR * pcData = CSCMemoryBlock::AllocBlock(read+1);
		memcpy(pcData,sTmp.GetBuffer(read),read);
		pcData[read] = MSG_END_FLAG;
		CSCMemoryBlock  * mBlock = new CSCMemoryBlock( pcData, read+1 );
		//	mBlock->m_iBlockNumber = iPackageNumber;
		FredPtr pFredPtr( mBlock );
		m_pKitSenderQueue->Push(pFredPtr);
	}
	catch (...)
	{
		Clog( LOG_DEBUG, _T("SendXML:push data to queue exception error=%d"),GetLastError());
	}
}

void CSingleConnect::SetServer(char* IP,int Port)
{
	m_IP=IP;
	m_Port=Port;
}
int CSingleConnect::ReportHeartBeat()
{
	CMarkup tmpXmlDOM;
	tmpXmlDOM.AddElem( _T("Message") );
	tmpXmlDOM.IntoElem();
	tmpXmlDOM.AddElem( _T("MessageHeader") );
	tmpXmlDOM.AddAttrib( _T("MessageCode"), _T("0") );  
	tmpXmlDOM.AddAttrib( _T("SessionID"),"0" );  
	tmpXmlDOM.AddAttrib( _T("MessageTime"), "0" ); 
	tmpXmlDOM.AddAttrib( _T("BeReturn"), _T("0") ); 
	tmpXmlDOM.AddAttrib( _T("CommandID"),"0" );
	
	Clog( LOG_DEBUG, _T("HB") );
	SendXML(tmpXmlDOM,FALSE);
	return 0;
}