/*****************************************************************************
File Name:     ClientManager.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CClientManager
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/
#ifndef __CCLIENTMANAGER_H__
#define __CCLIENTMANAGER_H__

#include"TCPConnection.h"
#include"ClientParse.h"
#include"ListenManager.h"
//class CListenManager;
class CSRManager;
class CClientManager  
{ 
public:
	CClientManager(CSRManager* pOwner);
	virtual ~CClientManager(void);
public:
    CConnectionPtrList m_pConnectionList;  //connection list
    //CClientParse             m_parse;              
public:
	//set Local IP ,listen Port
	void SetHost(char*IP,int Port);
	//Start lisen thread to accept the client connection
	void StartListen();
    //Set the connection State in connection list
	//which have the pSocket
	void SetState(CSCTCPSocket* pSocket,bool state);
	void CloseConnection(CSCTCPSocket* pSocket);
	void AddConnection(CTCPConnection* pConnection);
	void Lock();
	void UnLock();
	void Destroy(void);


    static DWORD WINAPI ThreadEntry(LPVOID pParam);

	//static DWORD WINAPI ThreadEntry((LPVOID pParam);
	void ThreadProc();
	void SendHeartBeat();
	void CloseThread();
    HANDLE		 m_hThread;              
	HANDLE		 m_hEvent;

	

public:
    CListenManager  m_pListenManager;
    
    CSRManager* m_pOwner;               //pointer to CSAManager

private:
     CRITICAL_SECTION   m_lock;

 };
#endif