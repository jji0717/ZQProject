/*****************************************************************************
File Name:     Connection.h
Author:        Haiping.Wan
Security:      SEACHANGE SHANGHAI
Description:   define class CConnection
Function Inventory: 
Modification Log:
When           Version        Who						What
---------------------------------------------------------------------
2005/04/21     1.0            Haiping.Wan					Created
*******************************************************************************/
#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include <list>
#include"sccommonsender.h"
#include"sccommonreceiver.h"
class CTCPConnection
{
public:
	CTCPConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver/*,CParse *pParse*/);
	//SetConnection(CSCCommonSender* pSender,CSCCommonReceiver* pRecerver);
	virtual ~CTCPConnection(void);
	
private:
	bool m_IsConnect;
    //have a send thread to send data
    //if the send queue have data
	//the thread get the data from the  queue
	//and send the data
    CSCCommonSender*   m_pSender;
	//have a recv thread to receive data
	CSCCommonReceiver* m_pRecerver;
    CRITICAL_SECTION   m_lock;
	//IP address the connect from
	DWORD IPFrom;
public:
	CSCCommonReceiver* GetReceive()
	{
        CSCCommonReceiver* pReceiver;
        Lock();
        pReceiver=m_pRecerver;
        UnLock();
		return pReceiver;
	};
	void SetReceive(CSCCommonReceiver* pReceive)
	{
       Lock();
       m_pRecerver=pReceive;
       UnLock();
	};
    CSCCommonSender* GetSender()
	{
        CSCCommonSender* pSender;
        Lock();
        pSender=m_pSender;
        UnLock();
		return pSender;
	};
	void SetSender(CSCCommonSender* pSender)
	{
       Lock();
       m_pSender=pSender;
       UnLock();
	};
	//Set the Connection State 
	//true connected false disconnected 
    void SetState(bool state);
	//Get the Connection State
	bool GetState();
	//make the send ,receive  thread to work
	void Start();
	//put the data to send queue
	void SendData(char* data,int length);
	//close the send receive thread 
	void Close();
    //static  void Notify();
	//static  CConnection* connection;
	void Lock(){::EnterCriticalSection(&m_lock);};
	void UnLock() {::LeaveCriticalSection(&m_lock);};
	DWORD GetIP() { return IPFrom;};
	void SetIP(DWORD ip) { IPFrom=ip;};

public:
    //ConnectionNotify m_ConectNotify;

 };
//connection list
typedef std::list< CTCPConnection* > CConnectionPtrList;

#endif

