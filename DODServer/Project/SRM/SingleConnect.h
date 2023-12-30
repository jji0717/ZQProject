
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
#include"DSAResource.h"
class CSingleConnect
{
public: //attribute
	///define a static global for pointer
	static CSingleConnect* s_SingleConnect;
	CSCTCPSocket m_Socket;
	bool m_bConnected;
	int m_iStatus;			// thread status 0 no running 1 running 2  3 quit
    CSingleConnect(CDSAResource* pOwner)
	{
		//initial thread
        m_pOwner=pOwner;
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
		
	}
	///deconstruct
	virtual ~CSingleConnect(void)
	{
		Stop();
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
		s_SingleConnect = NULL;
	}
public: //method
	/// Start connection.
	VOID Start();
	/// Stop connection.
	VOID Stop();
    	void SetServer(char* IP,int Port);
	CDSAResource* m_pOwner;
private:
	CSCCommonReceiver* m_pReceiver;
	CSCCommonSender* m_pSender;
    CDSAParse *m_pServerParser;
    HANDLE		m_hEvent; // thread event handle
	HANDLE		m_Thread; //thread handle
	DWORD		m_dwThreadID;
    CString  m_IP;
	DWORD    m_Port;
	bool   m_bThreadExit;
	HANDLE m_hShutdownEvent;

	ReceiverNotify m_Notify; // callback pointer
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