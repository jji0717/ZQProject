#include "StdAfx.h"
#include ".\jmsmessage.h"
#include"ActiveJMS.h"

CJMSMessage::CJMSMessage( CActiveJMS* pActiveJMS,BOOL bSend ,long lMessageID ,BOOL bMode)
{
	ASSERT(pActiveJMS!=NULL);

    m_lngQueueSession=-1;
    m_lngqueue=-1;
    m_lngReceiver=-1;
    
	m_pActiveJMS= pActiveJMS;
    m_messageId= lMessageID;
    m_bMode = bMode;
	m_bSend = bSend;
	Initialize();
 
};

CJMSMessage::CJMSMessage( CActiveJMS* pActiveJMS,char* MessageType,char* MessageCode)
 {
   ASSERT(pActiveJMS!=NULL);

    m_messageId=-1; 
    m_lngQueueSession=-1;
    m_lngqueue=-1;
    m_lngReceiver=-1;

    m_bSend=TRUE;
    m_bMode = MESSAGEMODE_PTOP;
	m_pActiveJMS= pActiveJMS;

	Initialize(TRUE,MessageType,MessageCode);
   
 }

CJMSMessage::~CJMSMessage(void)
{
  

};

//get long property from jms for receive
long CJMSMessage::GetLongProperty(char *longpropertykey)
{
	//prevent no message and get
	if (m_messageId==-1||m_bSend)
		return 0;

	try
	{
		_bstr_t Longpropertykey=longpropertykey;
		CString strlongproperty= m_pActiveJMS->objActiveJMS->getLongPropertyOnMessage(m_messageId,longpropertykey);
        return  atoi(strlongproperty.GetBuffer(0));
	 }
    catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
    }
   return 0;
};
//get int property from jms for receive
int CJMSMessage::GetIntProperty(char *strlongpropertykey)
{
	//prevent no message and get
	if (m_messageId==-1||m_bSend)
		return 0;
	try
	{
       return  m_pActiveJMS->objActiveJMS->getIntPropertyOnMessage(m_messageId,strlongpropertykey);
	}
	catch(_com_error &e)
	{
			CString msg = CString((LPCWSTR)e.Description());
			msg += L"\r\n\r\n";
			TRACE(msg);
	}
   return 0;
};
	
//get string property from jms for receive
CString CJMSMessage::GetStringProperty(char* stringpropertykey)
{
	//prevent no message and get
	if (m_messageId==-1||m_bSend)
		return "";

	try
	{
		CString cstringproperty=m_pActiveJMS->objActiveJMS->getStringPropertyOnMessage(m_messageId,stringpropertykey);
        //property =cstringproperty;
        //string =  property.GetBuffer(0);
		return cstringproperty;
	}
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
   return "";
};
   
//get double property from jms for receive
double CJMSMessage::GetDoubleProperty(char* doublepropertykey)
{
	//prevent no message and get
	if (m_messageId==-1||m_bSend)
		return 0;

	try
	{
		return m_pActiveJMS->objActiveJMS->getDoublePropertyOnMessage(m_messageId,doublepropertykey);
	}
	catch(_com_error &e)
	   {
		    CString msg = CString((LPCWSTR)e.Description());
			msg += L"\r\n\r\n";
			TRACE(msg);
	}
   return 0;
};
   
//get float property from jms for receive
float  CJMSMessage::GetFloatProperty(char *floatpropertykey)
{
	//prevent no message and get
	if (m_messageId==-1||m_bSend)
		return 0;
	try
	{
		  return  m_pActiveJMS->objActiveJMS->getFloatPropertyOnMessage(m_messageId,floatpropertykey);
	}
	catch(_com_error &e)
	{
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
	}
   return 0;
};
    
//get short property from jms for receive
short  CJMSMessage::GetShortProperty(char* shortpropertykey)
{
	//prevent no message and get
	if (m_messageId==-1||m_bSend)
		return 0;

	try
	{
		return m_pActiveJMS->objActiveJMS->getShortPropertyOnMessage(m_messageId,shortpropertykey);
	}
	 catch(_com_error &e)
	   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
	}
   return 0;
};
    
//get bool property from jms for receive
BOOL  CJMSMessage::GetBoolProperty(char* boolpropertykey)
{
	if (m_messageId==-1||m_bSend)
		return 0;

	try
	{
		BOOL temp=m_pActiveJMS->objActiveJMS->getBooleanPropertyOnMessage(m_messageId,boolpropertykey);
        return temp;
	}
	 catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
   return 0;
};


//set raw data
BOOL CJMSMessage::SetRawData(char* buf)
{
	if (buf==NULL || m_messageId==-1||(!m_bSend))
		return FALSE;

	try
	{
     _bstr_t  strmessage =buf ;
	 return m_pActiveJMS->objActiveJMS->setTextOnTextMessage(m_messageId, strmessage);
	}
    catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
	return FALSE;
};


CString CJMSMessage::GetRawData(void)
{
  if (m_messageId==-1||m_bSend)
		return "";
	try
	{
	 _bstr_t   temp;
	  temp= m_pActiveJMS->objActiveJMS->getTextOnTextMessage(m_messageId);
	  CString sBody = temp;
	  return sBody;
	 }
	catch(_com_error &e)
    {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
    }
  
   return "";
};


//initial while user new message,
//bSend - 0 - message which is received, 1 - message which is to be sended
//bMode - 0 -PTOP message, 1 - topic message
//bSync - the sended message is to be sync send
void CJMSMessage::Initialize(BOOL bSync, char *MessageType,char* MessageCode )
{
	if (!m_bSend) //receive
		return;

	if (!bSync) //not a sync message
	{
		switch (m_bMode){
			 case MESSAGEMODE_TOPIC:
				{
				try
				{
				m_messageId = m_pActiveJMS->objActiveJMS->createTextMessageOnSession(m_pActiveJMS->m_lngTopicSendSession);
				}
				catch(_com_error &e)
				{
					CString msg = CString((LPCWSTR)e.Description());
					msg += L"\r\n\r\n";
					TRACE(msg);
				}
				}
				break;
			case MESSAGEMODE_PTOP:
				try
				{
					m_messageId=m_pActiveJMS->objActiveJMS->createTextMessageOnSession(m_pActiveJMS->m_lngQueueSendSession);
				}
				catch(_com_error &e)
				{
						CString msg = CString((LPCWSTR)e.Description());
						msg += L"\r\n\r\n";
						TRACE(msg);
				}
				break;
			}
	}
	else
	{
		try
		{
		    m_lngQueueSession = m_pActiveJMS->objActiveJMS->createQueueSessionOnQueueConnection(m_pActiveJMS->m_lngQueueConnection, VARIANT_FALSE, m_pActiveJMS->objActiveJMS->AutoAcknowledgeOnSession);
			m_lngqueue = m_pActiveJMS->objActiveJMS->createTemporaryQueueOnQueueSession(m_lngQueueSession);
		    m_lngReceiver = m_pActiveJMS->objActiveJMS->createReceiverOnQueueSession(m_lngQueueSession, m_lngqueue, L"");
			m_messageId=m_pActiveJMS->objActiveJMS->createTextMessageOnSession(m_lngQueueSession);
			
			HRESULT ret;
			ret=setJmsReplyToOnMessage(m_lngqueue);
			if(MessageType!="")
			{
	          SetStringProperty("MESSAGECLASS",MessageType);
			}
			if(MessageCode!="")
			{
		      SetStringProperty("MESSAGECODE",MessageCode);
			}
		}
		catch(_com_error &e)
		{
			CString msg = CString((LPCWSTR)e.Description());
			msg += L"\r\n\r\n";
			TRACE(msg);
		}
	}

}


HRESULT  CJMSMessage::SetLongProperty(char *longpropertykey,long Value)
{
if (m_messageId==-1||(!m_bSend))
		return 0;
	try
	{
     CString tempstr;
	 tempstr.Format("%d",Value);
     char *Longproperty=tempstr.GetBuffer(0);
	 return m_pActiveJMS->objActiveJMS->setLongPropertyOnMessage(m_messageId,longpropertykey,Longproperty);
	}
	 catch(_com_error &e)
	{
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
	return 0;
}
	
HRESULT CJMSMessage::SetIntProperty(char *strlongpropertykey,int Value)
{
  if (m_messageId==-1||(!m_bSend))
		return 0;
	try
	{
       return  m_pActiveJMS->objActiveJMS->setIntPropertyOnMessage(m_messageId,strlongpropertykey,Value);
	}
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
   return 0;
}

HRESULT CJMSMessage::SetStringProperty(char* stringpropertykey,char* Value)
{
   if (m_messageId==-1||(!m_bSend))
		return 0;
	try
	{
      return  m_pActiveJMS->objActiveJMS->setStringPropertyOnMessage(m_messageId,stringpropertykey,Value);
	}
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
   return 0;
}
   
HRESULT CJMSMessage::SetDoubleProperty(char* doublepropertykey,double Value)
{
if (m_messageId==-1||(!m_bSend))
		return 0;
	try
	{
      return  m_pActiveJMS->objActiveJMS->setDoublePropertyOnMessage(m_messageId,doublepropertykey,Value);
	}
  catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
   return 0;
}
   
HRESULT CJMSMessage::SetFloatProperty(char *floatpropertykey,float Value)
{
if (m_messageId==-1||(!m_bSend))
		return 0;
	try
	{
      return  m_pActiveJMS->objActiveJMS->setFloatPropertyOnMessage(m_messageId,floatpropertykey,Value);
	}
	catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
   return 0;
}
    
HRESULT CJMSMessage::SetShortProperty(char* shortpropertykey,short Value)
{
	if (m_messageId==-1||(!m_bSend))
		return 0;
  try
  {
    return   m_pActiveJMS->objActiveJMS->setShortPropertyOnMessage(m_messageId,shortpropertykey,Value);
  }
  catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error()
   }
   return 0;

}

HRESULT CJMSMessage::SetBoolProperty(char* boolpropertykey,BOOL Value)
{
	if (m_messageId==-1||(!m_bSend))
		return 0;
	try
	{
      return    m_pActiveJMS->objActiveJMS->setBooleanPropertyOnMessage(m_messageId,boolpropertykey,Value);
	}
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error()
   }
   return 0;

}

//send message according to mode
HRESULT CJMSMessage::MessageSend(char* sQueueTopicName)
{
	switch (m_bMode){
		case MESSAGEMODE_PTOP:
		      return m_pActiveJMS->PtopSend(this,sQueueTopicName);
			 break;
		case MESSAGEMODE_TOPIC:
             return m_pActiveJMS->TopicSend(this,sQueueTopicName);
			 break;
   }
	return ERRORCODE_SENDMESSAGE_MESSAGEMODE;
}


//send message to all topic
HRESULT CJMSMessage::SendToAllTopic()
  {
	 return m_pActiveJMS->SendToAllTopics(this);
  }

///////////////following is from JMS server function/////////////////////
HRESULT  CJMSMessage::setJmsMessageIdOnMessage (char* MessageID)
 {
	if (m_messageId==-1||(!m_bSend))
		return 0;
	try
	 {
           return m_pActiveJMS->objActiveJMS->setJmsMessageIdOnMessage(m_messageId,MessageID);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error();
   }
   return 0;
 }
        
 HRESULT  CJMSMessage::setJmsCorrelationIdOnMessage ( char* CorrelationId)
 {
   if (m_messageId==-1||(!m_bSend))
		return 0;
     try
	 {
        return  m_pActiveJMS->objActiveJMS->setJmsCorrelationIdOnMessage(m_messageId,CorrelationId);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
   }
   return 0;
 }
          
 HRESULT  CJMSMessage::setJmsDeliveryModeOnMessage (long DeliveryMode)
 {
	if (m_messageId==-1||(!m_bSend))
		return 0;
    try
	 {
          return m_pActiveJMS->objActiveJMS->setJmsDeliveryModeOnMessage(m_messageId,DeliveryMode);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error();
   }
   return 0;
 }
       
 HRESULT  CJMSMessage::setJmsCorrelationIdAsBytesOnMessage (_variant_t  &CorrelationId)
 {
   if (m_messageId==-1||(!m_bSend))
	 return 0;
     try
	 {
           return m_pActiveJMS->objActiveJMS->setJmsCorrelationIdAsBytesOnMessage(m_messageId,CorrelationId);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
 	//	return e.Error();
  }
   return 0;
 }
       
 HRESULT CJMSMessage::setJmsDestinationOnMessage (long Destination)
 {
   if (m_messageId==-1||(!m_bSend))
	return 0;
     try
	 {
		 return   m_pActiveJMS->objActiveJMS->setJmsDestinationOnMessage(m_messageId,Destination);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error();
   }
   return 0;
 }
   
 HRESULT CJMSMessage::setJmsPriorityOnMessage (long Priority)
 {
	 if (m_messageId==-1||(!m_bSend))
		return 0;
      try
	 {
           return  m_pActiveJMS->objActiveJMS->setJmsPriorityOnMessage(m_messageId,Priority);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error();
   }
   return 0;
 }
  
 HRESULT CJMSMessage::setJmsReplyToOnMessage (long Reply)
 {
if (m_messageId==-1||(!m_bSend))
		return 0;
     try
	 {
         return  m_pActiveJMS->objActiveJMS->setJmsReplyToOnMessage(m_messageId,Reply);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error();
   }
   return 0;
 }
      
 HRESULT CJMSMessage::setJmsExpirationOnMessage (char* Expiration)
 {
if (m_messageId==-1||(!m_bSend))
	return 0;
     try
	 {
         return  m_pActiveJMS->objActiveJMS->setJmsExpirationOnMessage(m_messageId,Expiration);
	 }
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		//return e.Error();
   }
   return 0;
 }
 

long CJMSMessage::getJmsReplyToOnMessage()
{
if (m_messageId==-1||m_bSend)
		return 0;
    try
	{
         return  m_pActiveJMS->objActiveJMS->getJmsReplyToOnMessage(m_messageId);
	}
   catch(_com_error &e)
   {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
	//	return e.Error();
   }
   return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
 CString CJMSMessage::getJmsTimestampOnMessage ()
 {
   if(m_messageId==-1||!m_bSend)
   {
      try
	  {
		 CString Timestamp =  m_pActiveJMS->objActiveJMS->getJmsTimestampOnMessage(m_messageId);
		 return Timestamp;
	  }
     catch(_com_error &e)
      {
          CString msg = CString((LPCWSTR)e.Description());
		  msg += L"\r\n\r\n";
		  TRACE(msg);
          return "";
      }
   }
   else
   return "";
 }
 
  
 VARIANT_BOOL CJMSMessage::getJmsRedeliveredOnMessage ()
 {
    if(m_messageId==-1||!m_bSend)
   {
      try
	  {
        return  m_pActiveJMS->objActiveJMS->getJmsRedeliveredOnMessage(m_messageId);
	  }
     catch(_com_error &e)
      {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
        return VARIANT_FALSE;
      }
   }
   else
   return VARIANT_FALSE;
 }


 long CJMSMessage::getJmsDestinationOnMessage ()
 {
   if(m_messageId==-1||!m_bSend)
   {
      try
	  {
         return  m_pActiveJMS->objActiveJMS->getJmsDestinationOnMessage(m_messageId);
	  }
     catch(_com_error &e)
      {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
        return 0;
      }
   }
   else
   return 0;
 }
 
CString  CJMSMessage::getJmsCorrelationIdOnMessage ()
 {
   if(m_messageId==-1||!m_bSend)
   {
      try
	  {
         CString CorrelationId =  m_pActiveJMS->objActiveJMS->getJmsCorrelationIdOnMessage(m_messageId);
		 return CorrelationId;
	  }
     catch(_com_error &e)
      {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
         return "";
      }
   }
   else
   return "";
 }
      
 long CJMSMessage::getJmsDeliveryModeOnMessage ()
 {
    if(m_messageId==-1||!m_bSend)
   {
      try
	  {
          return  m_pActiveJMS->objActiveJMS->getJmsDeliveryModeOnMessage(m_messageId);
	  }
     catch(_com_error &e)
      {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
        return 0;
      }
   }
   else
   return 0;

 }
 
CString  CJMSMessage::getJmsExpirationOnMessage ()
 {

  if(m_messageId==-1||!m_bSend)
   {
      try
	  {
          CString Expiration =  m_pActiveJMS->objActiveJMS->getJmsExpirationOnMessage(m_messageId);
		  return  Expiration;
	  }
     catch(_com_error &e)
      {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
         return "";
      }
   }
   else
   return "";
 }
 
CString CJMSMessage::getJmsMessageIdOnMessage ()
 {
    if(m_messageId==-1||!m_bSend)
   {
      try
	  {
         CString MessageId =  m_pActiveJMS->objActiveJMS->getJmsMessageIdOnMessage(m_messageId);
		  return  MessageId;
	  }
     catch(_com_error &e)
      {
        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
        return "";
      }
   }
   else
   return "";

 }
 

 long CJMSMessage::getJmsPriorityOnMessage ()
 {
   if(m_messageId==-1||!m_bSend)
   {
      try
	  {
         return  m_pActiveJMS->objActiveJMS->getJmsPriorityOnMessage(m_messageId);
	  }
     catch(_com_error &e)
      {

        CString msg = CString((LPCWSTR)e.Description());
		msg += L"\r\n\r\n";
		TRACE(msg);
		return 0;

      }
   }
   else
   return 0;

 }
 
 


 