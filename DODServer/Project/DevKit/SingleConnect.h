
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

//define log level for tracking
#define LOG_NORECORD 0
#define LOG_ERROR	 1
#define LOG_RELEASE	 2
#define LOG_DEBUG   0

#include "sccommonreceiver.h"
#include "sccommonsender.h"
#include "clog.h"
//#include "common.h"
#include "DeviceInfoParser.h"
extern bool m_bConnected;

class CSingleConnect
{
public: //attribute
	///define a static global for pointer
	static CSingleConnect* s_SingleConnect;
	void SetServer(char* IP,int Port);

	///a parser to parse message from server
	CDeviceInfoParser * m_pServerParser;

	//struIP m_sysConfig; //remote IP which is connected to
	CSCTCPSocket m_Socket;
	int m_iStatus;			// thread status 0 no running 1 running 2  3 quit

	CString  m_IP;
	WORD    m_Port;
	DWORD m_dwThreadID;
//	bool m_bConnected;
	BOOL m_bContinueConnect;
	///construct
	CSingleConnect(struIP * pSysConfig, CDeviceInfoParser *prase  )
	{
		m_dwThreadID = 112;
		m_Thread = NULL;
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_Notify = Notify;
		m_pReceiver = NULL;
		m_pSender = NULL;
		m_iStatus = 0;
		m_bContinueConnect=TRUE;
		m_hDisconnectEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		s_SingleConnect = this;

		m_pServerParser = prase;
	}
	///deconstruct
	virtual ~CSingleConnect(void)
	{
		if( m_iStatus != 3 )
			Stop();

		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
		if( m_hDisconnectEvent )
		{
			CloseHandle( m_hDisconnectEvent );
			m_hDisconnectEvent = NULL;
		}

		// set pointer NULL
		s_SingleConnect = NULL;
	}


public: //method
	/// Start connection.
	VOID Start();

	/// Stop connection.
	VOID Stop();

	// connect successfully, assign resource for receiving and sending data.
	void Connect();

	// free the resource assigned in the process of initialization of socket connection.
	void Disconnect();
	void ChangeIP( struIP * pSysConfig )
	{ 
		//ASSERT(pSysConfig != NULL);
	//	memcpy(&m_sysConfig,pSysConfig,sizeof(struIP));	
	}

	/// Push data into sender queue to send out
	/// @param data(IN) - data which is to be send out, pushed data is put into queue and after send success, data will be free
//	void SendData(FredPtr data);
	void SendXML(CMarkup &data,BOOL showlog=TRUE);
private: 

	CSCCommonReceiver* m_pReceiver;
	CSCCommonSender* m_pSender;

    HANDLE		m_hEvent; // thread event handle
	HANDLE  m_Thread; //thread handle

	HANDLE m_hDisconnectEvent;

	ReceiverNotify m_Notify; // callback pointer

private:
	///thread body. because we can not wait to connect,we should set a thread to manage connection
	///@param class pointer
	static DWORD WINAPI ConnectThread(LPVOID pParam); 
	int ReportHeartBeat();
	/// callback to notify connection is lost
	///@param connection status = SC_RECEIVER_THREAD_ERROR || SC_SENDER_THREAD_ERROR
	///       pSocket - lost socket
	static void Notify(INT32 status,CSCTCPSocket* pSocket);
};

#endif