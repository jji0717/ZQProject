
/*
**	FILENAME			SingleConnect.h
**						
**	PURPOSE				The file is a header file, used along with SingleConnect.cpp.
**						a class CSingleConnect is declared in the file, and is used to 
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

#ifndef SingleConnectH
#define SingleConnectH

#include "sccommonreceiver.h"
#include "sccommonsender.h"
#include "clog.h"
//#include "common.h"
#include "DeviceInfoParser.h"

class CSingleConnect
{
public: //attribute
	///define a static global for pointer
	static CSingleConnect* s_SingleConnect;

	///a send queue for send net message
	CSCMemoryBlockQueue * m_pSenderQueue;
	//CSCMutexMemroyBlockQueue * m_pSenderQueue;

	///a parser to parse message from server
	CDeviceInfoParser * m_pServerParser;

	struIP m_sysConfig; //remote IP which is connected to
	CSCTCPSocket m_Socket;
	bool m_bConnected;
	int m_iStatus;			// thread status 0 no running 1 running 2  3 quit

	HWND m_hWnd;

	TCHAR	cLocalAddr[30];			// destination server IP address and
	WORD	wLocalPort;				// port
	TCHAR	cLocalComputerName[128];	// computer name

	TCHAR	cRemoteAddr[30];			// destination server IP address and
	WORD	wRemotePort;				// port
	TCHAR	cRemoteComputerName[128];	// computer name

	///construct
	CSingleConnect(struIP * pSysConfig, HWND hWnd  )
	{
		ASSERT(pSysConfig != NULL);
		memcpy(&m_sysConfig,pSysConfig, sizeof(struIP));
		
		//initial thread
		m_Thread = NULL;
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_Notify = Notify;
		m_pReceiver = NULL;
		m_pSender = NULL;
		m_iStatus = 0;
		
		m_hShutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_bConnected = false;
		m_bThreadExit = true;
		// set this pointer
		s_SingleConnect = this;

		m_hWnd = hWnd;

		// new sender queue
		m_pSenderQueue = new CSCMemoryBlockQueue;
		
		//new Server Parser
		m_pServerParser = new CDeviceInfoParser( hWnd );
	}
	///deconstruct
	virtual ~CSingleConnect(void)
	{
		//free thread
		//Clog( LOG_DEBUG, "~CSingleConnect() m_bThreadExit: %d", m_bThreadExit );		
		if( m_iStatus != 3 )
			Stop();

		//Clog( LOG_DEBUG, "Close Socket." );
		//m_iStatus = 3;	//quit
		//m_Socket.Close();
		//Clog( LOG_DEBUG, "Socket Closed." );
		
		if( m_pReceiver )
		{
			m_pReceiver->Stop();
			delete m_pReceiver;
			m_pReceiver = NULL;
		}
		if( m_pSender )
		{
			m_pSender->Stop();
			delete m_pSender;
			m_pSender = NULL;
		}


		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}

		if( m_hShutdownEvent )
		{
			CloseHandle( m_hShutdownEvent );
			m_hShutdownEvent = NULL;
		}

		// free sender queue
		
		INT32 size = m_pSenderQueue->Size();

		while(size)
		{
			//CSCMemoryBlock::FreeBlock(m_pSenderQueue->Front().GetBlock());
			m_pSenderQueue->Pop();
			size--;
		}

		
		if ( m_pSenderQueue!=NULL )
		{
			delete m_pSenderQueue;
			m_pSenderQueue = NULL;
		}
		
		//free Parser
		if (m_pServerParser!=NULL)
		{
			delete m_pServerParser;
			m_pServerParser = NULL;
		}

		// set pointer NULL
		s_SingleConnect = NULL;
	}


public: //method
	/// Start connection.
	VOID Start();

	/// Stop connection.
	VOID Stop();

	void ChangeIP( struIP * pSysConfig )
	{ 
		ASSERT(pSysConfig != NULL);
		memcpy(&m_sysConfig,pSysConfig,sizeof(struIP));	
	}

	/// Push data into sender queue to send out
	/// @param data(IN) - data which is to be send out, pushed data is put into queue and after send success, data will be free
	void SendData(CSCMemoryBlockPtr data);

private:

	CSCCommonReceiver* m_pReceiver;
	CSCCommonSender* m_pSender;

    HANDLE		m_hEvent; // thread event handle
	HANDLE		m_Thread; //thread handle
	DWORD		m_dwThreadID;

	bool   m_bThreadExit;
	HANDLE m_hShutdownEvent;

	ReceiverNotify m_Notify; // callback pointer

	static DWORD		m_iHeartBeatInterval;	// second
	static DWORD		m_iHeartBeatReferVal;	// after each heart beat, this value is reset.
	//CWinThread
	HANDLE				m_HeartBeatThread;		// use a thread deal heart beat mechanism.
	DWORD				m_dwHBThreadID;

	static bool			m_bHeartBeatThreadAlive;// control heartbeat thread.
	static bool			m_bHeartBeatThreadExit;	// check whether heartbeat over.	

	// heart beat thread function.
	static DWORD	 WINAPI HeartBeatThread(LPVOID pParam);
	void	EndHeartBeatThread();
	void	ReportHeartBeat();

private:
	///thread body. because we can not wait to connect,we should set a thread to manage connection
	///@param class pointer
	static DWORD WINAPI ConnectThread(LPVOID pParam); 

	/// callback to notify connection is lost
	///@param connection status = SC_RECEIVER_THREAD_ERROR || SC_SENDER_THREAD_ERROR
	///       pSocket - lost socket
	static void Notify(INT32 status,CSCTCPSocket* pSocket);
};

#endif