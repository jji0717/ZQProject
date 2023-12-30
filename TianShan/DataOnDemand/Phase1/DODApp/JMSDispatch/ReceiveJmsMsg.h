#if !defined(AFX_RECEIVE_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_)
#define AFX_RECEIVE_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "JMS.h"
class CJMS;
class CJMSTxMessage;
class CJMSPortManager;
class CReceiveJmsMsg
{
public:

  CReceiveJmsMsg(CJMSPortManager *client);
  ~CReceiveJmsMsg();

public:
	ZQ::JMSCpp::Session *m_Session;  
	ZQ::JMSCpp::Destination *m_destination;
	Consumer *m_Consumer;
    ZQ::JMSCpp::TextMessage m_jmsTextMsg;

    HANDLE m_hStartReceive;
    HANDLE m_hSendThread;
public:
     BOOL init();
	/// @Function parse command message 
	/// @return CJMSTxMessage on success,Null on failure
	/// but the return value is necessary
	/// @param : it come from OnMessage()
	virtual  CJMSTxMessage ParseCommand(CJMSTxMessage* pMessage);

	/// @Function . parse state type message
	/// @return true on success,false on failure
	/// @param : it come from OnMessage()
	virtual int ParseStatus(CJMSTxMessage* pMessage);
	/// @Function . parse notification type message
	/// @return true on success,false on failure
	/// @param : it come from OnMessage()
	virtual int parseNotification(CJMSTxMessage* pMessage);

	CString SendGetFullDateMsg(int id1,CString id2,int id3);
	CString SendGetConfig();
	CString GetDataTypeInitialMsg(CString sDataType,int nReserver);

	//come from JBoss server :Create all port and all channels
	BOOL ReceiverPortConfigMsg(char *buf,int buffersize,int *nReserver);

	//for shared fold mode;
	int ReceiverDataFolder(char *buf,int buffersize);

	// for TCP/IP message / command mode
	int ReceiverDataTCP(char *buf,int buffersize);
public:
	DWORD m_dwThreadID;
	CString m_strQueueName;
    CJMSPortManager *m_pJmsPortManage;

private:
	//insert datetime for XML_Content
	CString GetCurrDateTime();

	BOOL CheckDigiteORIsalpha(BOOL Isdigiter,CString &str,CString ElementIDForLog="");
	//get one of all portgather by portID
	CString RemoveBlank(CString oldStr);
	CString m_sDataTypeInitial;
	CString m_CacheDir;  
};

class CJmsProcThread
{
public:
	static HANDLE m_hReConnectJBoss;
	HANDLE m_hConnectThread;
public:	
	CJmsProcThread();
	~CJmsProcThread();
	BOOL init();
};

#endif // !defined(AFX_RECEIVE_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_)