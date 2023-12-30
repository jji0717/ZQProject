#ifndef __IPARSE_H__
#define __IPARSE_H__
#include"ActiveJMS.h"
#include "JMSMessage.h"


//virtual class you should over  the Parse(CJMSMessage* message)=0;
class IParse
{
 public:
   	virtual void Parse(CJMSMessage* message)=0;
};

/*virtual class you should over the ParseCommand(CJMSMessage* pmessage)=0;
                             ParseStatus(CJMSMessage* pmessage)=0;
                             parseNotification(CJMSMessage* pmessage)=0;*/
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


//example1
////////////////////////////////////////////////////////////////////////////////
class MyParse: public IParse
{ 
public:
  
    MyParse(CActiveJMS* pActiveJMS)
	{
        m_pActiveJMS=pActiveJMS;
	};
	~MyParse(void){};
public:

	virtual void Parse(CJMSMessage* pMessage)
	{
		CString data=pMessage->GetRawData();
		/*double temp=message->GetDoubleProperty("double");
		BOOL BoolTemp=message->GetBoolProperty("BOOL");
		float FloatTemp=message->GetFloatProperty("FLOAT");
		int IntTemp=message->GetIntProperty("INT");
		long LongTemp=message->GetLongProperty("LONG");
		short ShortTemp=message->GetShortProperty("SHORT");
        CString StringTemp=message->GetStringProperty("STRING");*/
		AfxMessageBox(data);
	};
public:
	CActiveJMS* m_pActiveJMS;
};
//example2
////////////////////////////////////////////////////////////////////////////////

class MyCommandParse :public  CParse
{
   public:
	MyCommandParse(CActiveJMS* pActiveJMS):
    CParse(pActiveJMS)
	{
        
	};
	~MyCommandParse(void){};

public:
	virtual CJMSMessage  ParseCommand(CJMSMessage* pMessage)
	{
          CString data=pMessage->GetRawData();
		 // double temp=pMessage->GetDoubleProperty("double");
		 // BOOL BoolTemp=pMessage->GetBoolProperty("BOOL");
		 //float FloatTemp=pMessage->GetFloatProperty("FLOAT");
		 // int IntTemp=pMessage->GetIntProperty("INT");
		 // long LongTemp=pMessage->GetLongProperty("LONG");
		 // short ShortTemp=pMessage->GetShortProperty("SHORT");
		 //CString tempqueue=pMessage->GetStringProperty("TEMPQUEUE");

		// m_pActiveJMS->AddOneSendQueue(tempqueue.GetBuffer(0));
        // long sender= GetSender(tempqueue.GetBuffer(0));
         CJMSMessage destmessage(m_pActiveJMS,TRUE,0,MESSAGEMODE_PTOP);
		 CString replystring="reply ";
         replystring+=data;
		 destmessage.SetStringProperty("MESSAGECODE","3456");
	     destmessage.SetRawData(replystring.GetBuffer(0));
		 return destmessage;
	}
	virtual int ParseStatus(CJMSMessage* pMessage)
	{
	   CString data=pMessage->GetRawData();
	   AfxMessageBox(data);
	   return 0;
	 }
	virtual int parseNotification(CJMSMessage* pMessage)
	{
		CString data=pMessage->GetRawData();
	    AfxMessageBox(data);
		return 0;
	}

};

#endif