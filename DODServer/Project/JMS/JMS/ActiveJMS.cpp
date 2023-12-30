#include "StdAfx.h"
#include "ActiveJMS.h"
#include"IParse.h"

CActiveJMS::CActiveJMS(BYTE nServerType):
m_Listener(this)
{ 
	m_nServerType = nServerType;

    ::CoInitialize(NULL);
	try
	{
       objActiveJMS.CreateInstance(__uuidof(ActiveJMS::ActiveJMS));
	   // register listener
	   BOOL ret=AfxConnectionAdvise(objActiveJMS, __uuidof(ActiveJMS::ActiveJMSSource), m_Listener.getSource(), FALSE, m_Listener.getCookieRef());
       m_lngProvider = objActiveJMS->createProvider();
	}

	catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
    }
}

CActiveJMS::~CActiveJMS(void)
{
    
    AfxConnectionUnadvise(objActiveJMS, __uuidof(ActiveJMS::ActiveJMSSource), m_Listener.getSource(), FALSE, m_Listener.getCookie());

	//protect exception to close all connection
	if (m_lngQueueConnection>=0)
	{
		objActiveJMS->stopOnConnection(m_lngQueueConnection);
		objActiveJMS->closeOnConnection(m_lngQueueConnection);
	}

	if (m_lngTopicConnection>=0)
	{
    objActiveJMS->stopOnConnection(m_lngTopicConnection);
    objActiveJMS->closeOnConnection(m_lngTopicConnection);
	}

	if (m_lngProvider>=0)
	{
     objActiveJMS->closeOnProvider(m_lngProvider);
	}

    objActiveJMS=NULL;

	 //free all lists
	 FreeAllList();

    //::CoUninitialize();
}

//free all resources
void CActiveJMS::FreeAllList()
{
   std::list< CJMSBase >::iterator it;
   while(m_QueueSendList.size())
   {
	   it=m_QueueSendList.begin();
	   m_QueueSendList.erase(it);
   }
  while(m_QueueRecvList.size())
   {
	   it=m_QueueRecvList.begin();
	   m_QueueRecvList.erase(it);
   }
   while(m_TopicSendList.size())
   {
	   it=m_TopicSendList.begin();
	   m_TopicSendList.erase(it);
   }
   while(m_TopicRecvList.size())
   {
	   it=m_TopicRecvList.begin();
	   m_TopicRecvList.erase(it);
   }
}

void CActiveJMS::Dispatch(long lngConsumer,CJMSMessage* message)
{
    std::list< CJMSBase >::iterator it;

	for(it=m_QueueRecvList.begin();it!=m_QueueRecvList.end();it++)
	{
		if(lngConsumer==it->GetHandle())
		{
			it->GetParse()->Parse( message);
			return;
		}
	}
    for(it=m_TopicRecvList.begin();it!=m_TopicRecvList.end();it++)
	{
		if(lngConsumer==it->GetHandle())
		{
			it->GetParse()->Parse( message);
		}
	}
 }
 
HRESULT CActiveJMS::Initialize(CString Providerkey,CString ProviderValue,CString FactoryInitialKey,CString FactoryInitialValue,
		CString QueueConnectionFactoryValue, CString TopicConnectionFactoryValue,
		CString FactoryurlKey/*for jboss*/,CString FactoryurlValue/*for jboss*/)
{

      CString QueueConnectionFactoryKey="Queue Name ";
      QueueConnectionFactoryKey+=QueueConnectionFactoryValue;
      CString TopicConnectionFactoryKey="Topic Name ";
      TopicConnectionFactoryKey+=TopicConnectionFactoryValue;
    
     CString strQueueConnectionFactoryName="${";
     strQueueConnectionFactoryName+=QueueConnectionFactoryKey;
     strQueueConnectionFactoryName+="}";

     CString strTopicConnectionFactoryName="${";
     strTopicConnectionFactoryName+=TopicConnectionFactoryKey;
     strTopicConnectionFactoryName+="}";
 
     if ( m_lngProvider == -1)
			return ERRORCODE_INITIALIZE_NOPROVIDER;

	long lngConnectionFactory,lngTopicConnectionFactory;

	try
	{
		switch (m_nServerType)
		{
			case JMSSERVER_JBOSS:
				//add property
				objActiveJMS->addPropertyOnProvider(m_lngProvider, Providerkey.GetBuffer(0), ProviderValue.GetBuffer(0));
				objActiveJMS->addPropertyOnProvider(m_lngProvider, FactoryInitialKey.GetBuffer(0), FactoryInitialValue.GetBuffer(0));
				objActiveJMS->addPropertyOnProvider(m_lngProvider, FactoryurlKey.GetBuffer(0), FactoryurlValue.GetBuffer(0));
			    objActiveJMS->addPropertyOnProvider(m_lngProvider, QueueConnectionFactoryKey.GetBuffer(0), QueueConnectionFactoryValue.GetBuffer(0));
				objActiveJMS->addPropertyOnProvider(m_lngProvider, TopicConnectionFactoryKey.GetBuffer(0), TopicConnectionFactoryValue.GetBuffer(0));

				//look up Queue
				lngConnectionFactory = objActiveJMS->lookupOnProvider(m_lngProvider, strQueueConnectionFactoryName.GetBuffer(0));
				m_lngQueueConnection = objActiveJMS->createQueueConnectionOnQueueConnectionFactory(lngConnectionFactory, L"", L"");
				m_lngQueueSendSession = objActiveJMS->createQueueSessionOnQueueConnection(m_lngQueueConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
				m_lngQueueReceiveSession = objActiveJMS->createQueueSessionOnQueueConnection(m_lngQueueConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
		
				//Topic
				lngTopicConnectionFactory = objActiveJMS->lookupOnProvider(m_lngProvider, strTopicConnectionFactoryName.GetBuffer(0));
				m_lngTopicConnection = objActiveJMS->createTopicConnectionOnTopicConnectionFactory(lngTopicConnectionFactory, L"", L"");
				m_lngTopicSendSession = objActiveJMS->createTopicSessionOnTopicConnection(m_lngTopicConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
				m_lngTopicReceiveSession = objActiveJMS->createTopicSessionOnTopicConnection(m_lngTopicConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
				break;
			case JMSSERVER_SWIFT:
				//add property
				objActiveJMS->addPropertyOnProvider(m_lngProvider, Providerkey.GetBuffer(0), ProviderValue.GetBuffer(0));
				objActiveJMS->addPropertyOnProvider(m_lngProvider, FactoryInitialKey.GetBuffer(0), FactoryInitialValue.GetBuffer(0));
			    objActiveJMS->addPropertyOnProvider(m_lngProvider, QueueConnectionFactoryKey.GetBuffer(0), QueueConnectionFactoryValue.GetBuffer(0));
				objActiveJMS->addPropertyOnProvider(m_lngProvider, TopicConnectionFactoryKey.GetBuffer(0), TopicConnectionFactoryValue.GetBuffer(0));

				//look up Queue
				lngConnectionFactory = objActiveJMS->lookupOnProvider(m_lngProvider, strQueueConnectionFactoryName.GetBuffer(0));
				m_lngQueueConnection = objActiveJMS->createQueueConnectionOnQueueConnectionFactory(lngConnectionFactory, L"", L"");
				m_lngQueueSendSession = objActiveJMS->createQueueSessionOnQueueConnection(m_lngQueueConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
				m_lngQueueReceiveSession = objActiveJMS->createQueueSessionOnQueueConnection(m_lngQueueConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
		
				//Topic
				lngTopicConnectionFactory = objActiveJMS->lookupOnProvider(m_lngProvider, strTopicConnectionFactoryName.GetBuffer(0));
				m_lngTopicConnection = objActiveJMS->createTopicConnectionOnTopicConnectionFactory(lngTopicConnectionFactory, L"", L"");
				m_lngTopicSendSession = objActiveJMS->createTopicSessionOnTopicConnection(m_lngTopicConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
				m_lngTopicReceiveSession = objActiveJMS->createTopicSessionOnTopicConnection(m_lngTopicConnection, VARIANT_FALSE, objActiveJMS->AutoAcknowledgeOnSession);
				break;
			default:
				return FALSE;
		  	}

	//after initialize, start connection
    objActiveJMS->startOnConnection(m_lngTopicConnection);
    objActiveJMS->startOnConnection(m_lngQueueConnection);
	}

	catch(_com_error &e)
		{
			CString msg = CString((LPCWSTR)e.Description());
			msg += L"\r\n\r\n";
			TRACE(msg);
			return e.Error();
	    }
	
	return 0;
}


HRESULT CActiveJMS::AddOneQueue(char* QueueName,IParse*  parse)
{
   // L"${topic.connectionfactory.name}";
	try
	{
    _bstr_t strKey;
	_bstr_t strValue;
    strValue=QueueName;
    strKey=L"key name";
    strKey +=QueueName;
	
     objActiveJMS->addPropertyOnProvider(m_lngProvider, strKey, strValue);
     _bstr_t lookkey;
     lookkey=L"${";
     lookkey+=strKey;
     lookkey+=L"}";
     long lngQueue = objActiveJMS->lookupOnProvider(m_lngProvider,lookkey );
     if (0 == lngQueue)
		 lngQueue = objActiveJMS->createQueueOnQueueSession(m_lngQueueSendSession, lookkey);
	 long lngSender = objActiveJMS->createSenderOnQueueSession(m_lngQueueSendSession, lngQueue);
     long lngReceiver = objActiveJMS->createReceiverOnQueueSession(m_lngQueueReceiveSession, lngQueue, L"");
	     
     objActiveJMS->activateMessageListenerOnMessageConsumer(lngReceiver);

     CJMSBase  SendQueue(this,lngSender,lookkey,strValue);
     m_QueueSendList.push_back(SendQueue);

     CJMSBase RecvQueue(this,lngReceiver,lookkey,strValue,parse);
     m_QueueRecvList.push_back(RecvQueue);
	}

    catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
    }
	return 0;
}

HRESULT CActiveJMS::AddOneSendQueue(char* QueueName)
{
	try
	{
    _bstr_t strKey;
	_bstr_t strValue;
    strValue=QueueName;
    strKey=L"key name";
    strKey +=QueueName;

     objActiveJMS->addPropertyOnProvider(m_lngProvider, strKey, strValue);
     _bstr_t lookkey;
     lookkey=L"${";
     lookkey+=strKey;
	 lookkey+=L"}";
     long lngQueue = objActiveJMS->lookupOnProvider(m_lngProvider,lookkey );
     if (0 == lngQueue)
	   lngQueue = objActiveJMS->createQueueOnQueueSession(m_lngQueueSendSession, lookkey);
	 long lngSender = objActiveJMS->createSenderOnQueueSession(m_lngQueueSendSession, lngQueue);

     CJMSBase  SendQueue(this,lngSender,lookkey,strValue);
     m_QueueSendList.push_back(SendQueue);

	}
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
   }
   return 0;
}

HRESULT CActiveJMS::AddOneReceiveQueue(char* QueueName,IParse*  parse)
{
     try
	 {
     _bstr_t strKey;
	 _bstr_t strValue;
     strValue=QueueName;
     strKey=L"key name";
     strKey +=QueueName;

     objActiveJMS->addPropertyOnProvider(m_lngProvider, strKey, strValue);
     _bstr_t lookkey;
     lookkey=L"${";
     lookkey+=strKey;
	 lookkey+=L"}";
     long lngQueue = objActiveJMS->lookupOnProvider(m_lngProvider,lookkey );
     if (0 == lngQueue)
	   lngQueue = objActiveJMS->createQueueOnQueueSession(m_lngQueueSendSession, lookkey);

	 long lngReceiver = objActiveJMS->createReceiverOnQueueSession(m_lngQueueReceiveSession, lngQueue, L"");
     objActiveJMS->activateMessageListenerOnMessageConsumer(lngReceiver);

     CJMSBase RecvQueue(this,lngReceiver,lookkey,strValue,parse);
     m_QueueRecvList.push_back(RecvQueue);
    
	 }
    catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
    }
	return 0;
 }

//TopicList m_SendTopicList;
//  TopicList m_RecvTopicList;
HRESULT CActiveJMS::AddOneTopic(char* TopicName,IParse*  parse)
{
	try
	{
     _bstr_t strKey;
	 _bstr_t strValue;
     strValue=TopicName;
     strKey=L"key name";
     strKey +=TopicName;

     objActiveJMS->addPropertyOnProvider(m_lngProvider, strKey, strValue);
     _bstr_t lookkey;
     lookkey=L"${";
     lookkey+=strKey;
	 lookkey+=L"}";

	 long lngTopic = objActiveJMS->lookupOnProvider(m_lngProvider, lookkey);
     if (0 == lngTopic)
	 {
		lngTopic = objActiveJMS->createTopicOnTopicSession(m_lngTopicSendSession,lookkey);
	 }
	 // Create a Receiver to listen for messages
	 long lngSubScriber = objActiveJMS->createSubscriberOnTopicSession(m_lngTopicReceiveSession, lngTopic, L"",false);
     // Activate the onMessage Event
	 objActiveJMS->activateMessageListenerOnMessageConsumer(lngSubScriber);
     // Create a Producer to create and send a message
	 long lngPublish = objActiveJMS->createPublisherOnTopicSession(m_lngTopicSendSession, lngTopic);


     CJMSBase  SendTopic(this,lngPublish,lookkey,strValue);
     m_TopicSendList.push_back(SendTopic);

     CJMSBase RecvTopic(this,lngSubScriber,lookkey,strValue,parse);
     m_TopicRecvList.push_back(RecvTopic);

	}
    catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
    }
	return 0;
}

HRESULT CActiveJMS::AddOneSendTopic(char* TopicName)
{
	try
	{
     _bstr_t strKey;
	 _bstr_t strValue;
     strValue=TopicName;
     strKey=L"key name";
     strKey +=TopicName;

     objActiveJMS->addPropertyOnProvider(m_lngProvider, strKey, strValue);
     _bstr_t lookkey;
     lookkey=L"${";
     lookkey+=strKey;
	 lookkey+=L"}";

	 long lngTopic = objActiveJMS->lookupOnProvider(m_lngProvider, lookkey);
     if (0 == lngTopic)
	 {
		lngTopic = objActiveJMS->createTopicOnTopicSession(m_lngTopicSendSession,lookkey);
	 }
	 long lngPublish = objActiveJMS->createPublisherOnTopicSession(m_lngTopicSendSession, lngTopic);

     CJMSBase  SendTopic(this,lngPublish,lookkey,strValue);
     m_TopicSendList.push_back(SendTopic);

	}

	catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
    }
	return 0;

  }
HRESULT CActiveJMS::AddOneRecvTopic(char* TopicName,IParse*  parse)
{ 
	try
	{
     _bstr_t strKey;
	 _bstr_t strValue;
     strValue=TopicName;
     strKey=L"key name";
     strKey +=TopicName;

     objActiveJMS->addPropertyOnProvider(m_lngProvider, strKey, strValue);
     _bstr_t lookkey;
     lookkey=L"${";
     lookkey+=strKey;
	 lookkey+=L"}";
     long lngTopic = objActiveJMS->lookupOnProvider(m_lngProvider, lookkey);
     if (0 == lngTopic)
	 {
		lngTopic = objActiveJMS->createTopicOnTopicSession(m_lngTopicSendSession,lookkey);
	 }

	 // Create a Receiver to listen for messages
	 long lngSubScriber = objActiveJMS->createSubscriberOnTopicSession(m_lngTopicReceiveSession, lngTopic, L"",false);
	 objActiveJMS->activateMessageListenerOnMessageConsumer(lngSubScriber);

     CJMSBase RecvTopic(this,lngSubScriber,lookkey,strValue,parse);
     m_TopicRecvList.push_back(RecvTopic);
   
	}
    catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
   }
   return 0;

}


HRESULT CActiveJMS::SendToAllTopics(char* buf)
 {
	 try
	 {
           long lngMessage = objActiveJMS->createTextMessageOnSession(m_lngTopicSendSession);
           _bstr_t  message;
           message=buf;
		   objActiveJMS->setTextOnTextMessage(lngMessage, message);

          // std::list<Topic> TopicList; 
		   std::list< CJMSBase >::iterator it;
		  for(it=m_TopicSendList.begin();it!=m_TopicSendList.end();it++)
		  {
			  long sender=(*it).GetHandle();
             objActiveJMS->publishOnTopicPublisher(
                                       sender,
                                       lngMessage,
                                       objActiveJMS->DefaultDeliveryModeOnMessage,
                                       objActiveJMS->DefaultPriorityOnMessage,
                                       objActiveJMS->DefaultTimeToLiveOnMessage);
		  }
	 }
	 catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }
	 return 0;
 }

 HRESULT CActiveJMS::SendToAllTopics(CJMSMessage* pMessage)
 {
	 //if it is a received message
	 if(!pMessage->IsSendMessage())
		 return -1;

     try
	 {
          std::list< CJMSBase >::iterator it;
		  for(it=m_TopicSendList.begin();it!=m_TopicSendList.end();it++)
		  {
			  it->TopicSend(pMessage);        
		  }
	 }
	 catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }
	 //ok
	 return 0;

 }

 //set buf as a message body
 HRESULT CActiveJMS::TopicSend(char* buf,char* TopicName)
 {
	 BOOL bSendOK = FALSE; //check if it is sendout
	 try
	 {
        _bstr_t TempStr=TopicName;
	    long lngMessage = objActiveJMS->createTextMessageOnSession(m_lngTopicSendSession);
        _bstr_t  message;
        message=buf;
	    objActiveJMS->setTextOnTextMessage(lngMessage, message);

        // std::list<Topic> TopicList; 
	    std::list< CJMSBase >::iterator it;
		for(it=m_TopicSendList.begin();it!=m_TopicSendList.end();it++)
		{
            _bstr_t   Value=it->GetKeyValue();
			if(Value==TempStr)
			{
			  long sender=(*it).GetHandle();
              objActiveJMS->publishOnTopicPublisher(
                                       sender,
                                       lngMessage,
                                       objActiveJMS->DefaultDeliveryModeOnMessage,
                                       objActiveJMS->DefaultPriorityOnMessage,
                                       objActiveJMS->DefaultTimeToLiveOnMessage);
			  bSendOK = TRUE;
			}
		  }
	 }
     catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }

	 if (bSendOK)
		return 0;
	 else
	    return ERRORCODE_TOPICSEND_NOTOPICNAME;
 }

//send message to topic
 HRESULT CActiveJMS::TopicSend(CJMSMessage* pMessage ,char* TopicName)
 {
	 BOOL bSendOK = FALSE; //check if it is sendout

	 if(!pMessage->IsSendMessage())
		 return ERRORCODE_TOPICSEND_MESSAGETYPE;

     try
	 {
        std::list< CJMSBase >::iterator it;
		for(it=m_TopicSendList.begin();it!=m_TopicSendList.end();it++)
		{  
            _bstr_t  Name=TopicName;
            _bstr_t  Value=it->GetKeyValue();
			if(Value==Name)
			{
				it->TopicSend(pMessage);
				bSendOK = TRUE;
			}
		  }
	 }
     catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }

	 if (bSendOK)
		return 0;
	 else
  	    return ERRORCODE_TOPICSEND_NOTOPICNAME;

 }

 //ptop send message body
 HRESULT CActiveJMS::PtopSend(char* buf,char *QueueName)
{
	 BOOL bSendOK = FALSE; //check if it is sendout

	try{
        _bstr_t TempStr=QueueName;
	    long lngMessage = objActiveJMS->createTextMessageOnSession(m_lngQueueSendSession);
        _bstr_t  message;
        message=buf;
	    objActiveJMS->setTextOnTextMessage(lngMessage, message);
        // std::list<Topic> TopicList; 
	    std::list< CJMSBase >::iterator it;
		for(it=m_QueueSendList.begin();it!=m_QueueSendList.end();it++)
		{
             _bstr_t   Value=it->GetKeyValue();
			if(Value==TempStr)
			{
				long sender=(*it).GetHandle();
               objActiveJMS->sendOnQueueSender(
                                       sender,
                                       lngMessage,
                                       objActiveJMS->DefaultDeliveryModeOnMessage,
                                       objActiveJMS->DefaultPriorityOnMessage,
                                       objActiveJMS->DefaultTimeToLiveOnMessage);
				bSendOK = TRUE;
			}
		  }
	}
    catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }

	 if (bSendOK)
		return 0;
	 else
  	    return ERRORCODE_PTOPSEND_NOQUEUENAME;
}

//ptop send a message
HRESULT CActiveJMS::PtopSend(CJMSMessage* pMessage,char *QueueName)
{

	if(!pMessage->IsSendMessage())
		return ERRORCODE_PTOPSEND_MESSAGETYPE;

	 BOOL bSendOK = FALSE; //check if it is sendout
    try
	 {
		_bstr_t strName;
        strName=QueueName;
        std::list< CJMSBase >::iterator it;
		for(it=m_QueueSendList.begin();it!=m_QueueSendList.end();it++)
		{
             _bstr_t  Value=it->GetKeyValue();
			if(Value==strName)
			{
				it->QueueSend(pMessage);
				bSendOK = TRUE;
			}
		  }
	 }
     catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }

	 if (bSendOK)
		return 0;
	 else
		return ERRORCODE_PTOPSEND_NOQUEUENAME;
}


//Topic Send a message according to handler
 HRESULT CActiveJMS::TopicSend(CJMSMessage* pMessage,long handler)
  {
	 if(!pMessage->IsSendMessage())
		 return ERRORCODE_TOPICSEND_MESSAGETYPE;

	 if (handler==-1)
		 return ERRORCODE_TOPICSEND_NOHANDLER;

	 try
	 {
            long lngMessage= pMessage->GetMessageId();
		   objActiveJMS->publishOnTopicPublisher(
                                       handler,
                                       lngMessage,
                                       objActiveJMS->DefaultDeliveryModeOnMessage,
                                       objActiveJMS->DefaultPriorityOnMessage,
                                       objActiveJMS->DefaultTimeToLiveOnMessage);
	 }
	 catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }
	 return 0;

  }

 //queue send according to handler
 HRESULT CActiveJMS::QueueSend(CJMSMessage* pMessage,long handler)
 {
	 if(!pMessage->IsSendMessage())
		return ERRORCODE_PTOPSEND_MESSAGETYPE;

	 if (handler==-1)
		 return ERRORCODE_PTOPSEND_NOHANDLER;

     try
	 {
           long lngMessage= pMessage->GetMessageId();
		   objActiveJMS->sendOnQueueSender(
                                       handler,
                                       lngMessage,
                                       objActiveJMS->DefaultDeliveryModeOnMessage,
                                       objActiveJMS->DefaultPriorityOnMessage,
                                       objActiveJMS->DefaultTimeToLiveOnMessage);
	 }
	 catch (_com_error &e)
	 {
		CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return e.Error();
	 }
	 return 0;

 }

 //sync send while need wait for return
HRESULT  CActiveJMS::SyncSend(CJMSMessage* pMessage,char*QueueName,IParse* Parse,int iWaitTimeOut)
 {
	 if( pMessage->GetReceiveHandler()<=0)
		 return ERRORCODE_SYNCSEND_NORECEIVEHANDLER;

	  try
	  {
       HRESULT bSendOK = PtopSend(pMessage,QueueName);
	   //send failed
	   if (bSendOK)
		   return bSendOK;
		
	   //ok send, wait to receive timeout
	    long messageId= objActiveJMS->receiveOnMessageConsumer(pMessage->GetReceiveHandler(),iWaitTimeOut);

	    if(messageId>0)
	    {//get OK
           CJMSMessage jmsmessage(this,FALSE,messageId);
	       Parse->Parse(&jmsmessage);
           objActiveJMS->purge(messageId);
	    }
		//
		objActiveJMS->closeOnMessageConsumer(pMessage->GetReceiveHandler());
  	    objActiveJMS->closeOnSession (pMessage->GetQueueSession());
	   }
       catch (_com_error &e)
	   {
		 CString msg = CString((LPCWSTR)e.Description());
		 msg += L"\r\n\r\n";
		 TRACE(msg);
		 return e.Error();
	   }
	   return 0;

 }
