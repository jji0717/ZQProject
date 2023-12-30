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

class CBaseSource;
class CJMS;
class CJMSTxMessage;

class CJMSParser:public Listener
{
public:
public:
	CJMSParser();
	CJMSParser(CJMS *inJms,CBaseSource *pSour);
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

	//CString m_strCurrentEXEPath;
	//CString SendGetFullDateMsg(int id1,int id2,int id3);
	//CString SendGetConfig();

	//come from JBoss server :Create all port and all channels
	//int ReceiverPartAssetInfoRefresh(char *buf,int buffersize,PPsortInfo *info);



public:
//	CPortManager *m_pm;
//	BOOL m_bStartReceive;
//	BOOL m_bIsLocalConfig;
private:

	CBaseSource *m_pBaseSource;
	/// @Function check string is digiter or alpha. 
	/// @return No MCCAPI_SUCCESS indicates failure. MCCAPI_SUCCESS indicates success 
	/// @param :received string from lam..
	int Safeatoi(CString str);

	//int FillCacheByID(CString &strname);
	//int FillCacheByInfo(CString &strInfo);
	//int ChangeXMLTobuf(CString &strxml,ASSETINFO *info);
	//get one of all port gather by portID
	//BOOL FindOutPortIndexAndChannelIndex(int dataType,int GroupID,CDODPort **portaddress,int &ChannelIndex) {};

	/// @Function check statues from lam. 
	/// @return No MCCAPI_SUCCESS indicates failure. MCCAPI_SUCCESS indicates success 
	/// @param :received string from lam..
	//int CheckStatusFromMMA(int nState);
};

#endif // !defined(AFX_PARSER_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_)
