/*******************************************************************
File Name:     Parser.h
Author:        zhenan.ji
Security:      SEACHANGE SHANGHAI
Description:   Declare the class CTaskEngine 
Function Inventory: 
Modification Log:
When           Version        Who         What
---------------------------------------------------------------------
2005/08/26     1.0            zhenan.ji   Created
********************************************************************/

// Parser.h: interface for the CJMSParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSER_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_)
#define AFX_PARSER_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "JMS.h"
class CDODClientAgent;
class CDODPort;
class CPortManager;
class CJMS;
class CJMSTxMessage;

class CJMSParser:public Listener
{
public:
public:
	CJMSParser();
	CJMSParser(CJMS *inJms,CDODClientAgent *client);
	~CJMSParser();

	/// @Function Listener will call it ,then ms will full of message value.
	/// get value by a string key. the message will be sort for different function.
	/// @param CJMSTxMessage 's baseclass
	virtual void onMessage(Message *);

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

	/// get CJMS 's point , for add queue or send reply message .
	CJMS *m_jms;	

	CString SendGetFullDateMsg(int id1,CString id2,int id3);
	CString SendGetConfig();
	CString GetDataTypeInitialMsg(CString sDataType,int nReserver);

	//come from JBoss server :Create all port and all channels
	int ReceiverPortConfigMsg(char *buf,int buffersize,int *nReserver);

	//for shared fold mode;
	int ReceiverDataFolder(char *buf,int buffersize);
	int ReceiverUpDataFolder(char *buf,int buffersize);


	// for TCP/IP message / command mode
	int ReceiverDataTCP(char *buf,int buffersize,int nGroupID_zero,int &GroupID_is_zero);
	int ReceiverUpDataTCP(char *buf,int buffersize);
	int ReceiverDataTCPToAllChannel(char *buf);

public:
	CPortManager *m_pm;
	BOOL m_bStartReceive;
	BOOL m_bIsLocalConfig;

	DWORD m_dwThreadID;
	CString m_strQueueName;
private:

	CDODClientAgent *m_Agent;
	//insert datetime for XML_Content
	CString GetCurrDateTime();

	BOOL CheckDigiteORIsalpha(BOOL Isdigiter,CString &str,CString ElementIDForLog="");
	//get one of all portgather by portID
	BOOL FindOutPortIndexAndChannelIndex(CString dataType,int GroupID,CDODPort **portaddress,int &ChannelIndex);

	CString RemoveBlank(CString oldStr);

	BOOL FindOutChannelIndex(CString dataType,CDODPort *portaddress,int &ChannelIndex);
};

#endif // !defined(AFX_PARSER_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_)
