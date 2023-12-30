// Parser.h: interface for the CJMSParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSER_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_)
#define AFX_PARSER_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include"ActiveJMS.h"
#include "JMSMessage.h"

typedef struct PORTINFO 
{								    
	char cAgentServerIp[16]; 
	int  wAgentServerPort;
	char cSrmtIp[16];
	int wSrmPort;
} PPsortInfo;

class CDODPort;
class IParse
{
 public:
   	virtual void Parse(CJMSMessage* message)=0;
};
class CRequestMsg;
class CPortManager;
class CParse: public IParse
{ 
public:
     CParse(CActiveJMS* pActiveJMS)
	 {
        m_pActiveJMS=pActiveJMS;
	 };
	virtual  CJMSMessage ParseCommand(CJMSMessage* pMessage)=0;

    virtual int ParseStatus(CJMSMessage* pMessage)=0;

    virtual int parseNotification(CJMSMessage* pMessage)=0;

   	~CParse(void){};
public:
	virtual void Parse(CJMSMessage* message)
	{
		CString strClass=message->GetStringProperty("MESSAGECLASS");
		if(strClass=="COMMAND")
			{
			CString tempqueue=message->GetStringProperty("TEMPQUEUE");
            long sender= GetTempQueueSenderHandle(tempqueue.GetBuffer(0));
			
            CJMSMessage  pmessage = ParseCommand(message);

            m_pActiveJMS->QueueSend(&pmessage,sender);
			//it is close by sender
			//m_pActiveJMS->objActiveJMS->deleteOnDestination(lngQueue);

   			}
        else if (strClass=="STATUS")
		{
            ParseStatus(message);      
		}
		else if(strClass=="NOTIFICATION")
		{
            parseNotification(message);
		}
	};

public:
	CActiveJMS* m_pActiveJMS;

private:
	long  GetTempQueueSenderHandle(char* QueueName)
	{
		try
		{
		_bstr_t strKey;
		_bstr_t strValue;
		strValue=QueueName;
		strKey=L"key name";
		strKey +=QueueName;

		m_pActiveJMS->objActiveJMS->addPropertyOnProvider(m_pActiveJMS->m_lngProvider, strKey, strValue);
		_bstr_t lookkey;
		lookkey=L"${";
		lookkey+=strKey;
		lookkey+=L"}";
		long lngQueue = m_pActiveJMS->objActiveJMS->createQueueOnQueueSession(m_pActiveJMS->m_lngQueueSendSession, lookkey);
		return m_pActiveJMS->objActiveJMS->createSenderOnQueueSession(m_pActiveJMS->m_lngQueueSendSession, lngQueue);
		}
		catch (_com_error &e)
		{
			CString msg = CString((LPCWSTR)e.Description());
			msg += L"\r\n\r\n";
			TRACE(msg);
		 }
	 return -1;
	 };
};


class CJMSParser :public CParse
{
public:
	CJMSParser(CActiveJMS* pActiveJMS):CParse(pActiveJMS)
	{
		m_pm=NULL;
	}
	~CJMSParser(void);

	// duplicate function of inherit class or hierarchy class.
	virtual CJMSMessage  ParseCommand(CJMSMessage* pMessage);
	virtual int ParseStatus(CJMSMessage* pMessage);
	virtual int parseNotification(CJMSMessage* pMessage);

	CString SendGetFullDateMsg(int id1,int id2,int id3);
	CString SendGetConfig();

	//come from JBoss server :Create all port and all channels
	int ReceiverPortConfigMsg(char *buf,int buffersize,PPsortInfo *info);

	//for shold fold mode;
	int ReceiverDataFolder(char *buf,int buffersize);
	int ReceiverUpDataFolder(char *buf,int buffersize);


	// for TCP/IP message / command mode
	int ReceiverDataTCP(char *buf,int buffersize);
	int ReceiverUpDataTCP(char *buf,int buffersize);

public:
	CPortManager *m_pm;
private:

	//insert datetime for XML_Content
	CString GetCurrDateTime();

	//get one of all portgather by portID
	BOOL FindOutPortIndexAndChannelIndex(int dataType,int GroupID,CDODPort **portaddress,int &ChannelIndex);
};

#endif // !defined(AFX_PARSER_H__41A43996_E52E_42BE_9348_2F9827DB4CB5__INCLUDED_)
