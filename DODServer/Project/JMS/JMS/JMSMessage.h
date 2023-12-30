#pragma once

#define MESSAGEMODE_PTOP   0
#define MESSAGEMODE_TOPIC  1

class CActiveJMS;
class CJMSMessage
{
public:
	//deault is send message,message mode is ptop
	CJMSMessage(CActiveJMS* pActiveJMS,BOOL bSend = FALSE,long lMessageID = 0,BOOL bMode=MESSAGEMODE_PTOP);
	//TempQueue  send message
    CJMSMessage( CActiveJMS* pActiveJMS,char* MessageType,char* MessageCode);

	~CJMSMessage(void);

private:
    long m_messageId; //message ID
	//queue session handle
    long m_lngQueueSession;
    // temp queue handle
    long m_lngqueue;
    // temp queue receiver handle
    long m_lngReceiver;

	CActiveJMS* m_pActiveJMS; //handle of active JMS

	//m_bMode - 0 -PTOP message, 1 - topic message
	BOOL m_bMode; //Message Sended Mode: MESSAGEMODE_PTOP, MESSAGEMODE_TOPIC
	//m_bSend - 0 - message which is received, 1 - message which is to be sended
	BOOL m_bSend; //message send or received

public:
	//judge if it is a send message
	BOOL IsSendMessage(void) { return m_bSend;}

	//get queue session handler
    long GetQueueSession(void) { return m_lngQueueSession; }

	//get temp queue receive handler
	long GetReceiveHandler(void) {   return m_lngReceiver;}

	//get temp queue handler
	long GetQueueHandler(void){ return  m_lngqueue;}


    //set the Raw data
    BOOL SetRawData(char* buf);
    //get the Raw data
	CString  GetRawData(void);
	
	//get MessageID
	long GetMessageId(void){   return m_messageId;}
	 
	//send message according to mode
	//@param sQueueTopicName - queue or topic name which is to be sent
	HRESULT MessageSend(char* sQueueTopicName);

	HRESULT SendToAllTopic();

	///////////////following is from JMS server function/////////////////////
	//get property for application
  	long GetLongProperty(char *longpropertykey);
	int GetIntProperty(char *strlongpropertykey);
	CString GetStringProperty(char* stringpropertykey);
	double GetDoubleProperty(char* doublepropertykey);
	float  GetFloatProperty(char *floatpropertykey);
	short  GetShortProperty(char* shortpropertykey);
	BOOL  GetBoolProperty(char* boolpropertykey);

	//set property for application
     HRESULT  SetLongProperty(char *longpropertykey,long Value);
	 HRESULT  SetIntProperty(char *strlongpropertykey,int Value);
	 HRESULT  SetStringProperty(char* stringpropertykey,char* Value);
	 HRESULT  SetDoubleProperty(char* doublepropertykey,double Value);
	 HRESULT  SetFloatProperty(char *floatpropertykey,float Value);
	 HRESULT  SetShortProperty(char* shortpropertykey,short Value);
	 HRESULT  SetBoolProperty(char* boolpropertykey,BOOL Value);

	//set jms attribute on message
    HRESULT  setJmsMessageIdOnMessage (char* MessageID);
    HRESULT  setJmsCorrelationIdOnMessage ( char* CorrelationId);
    HRESULT  setJmsDeliveryModeOnMessage (long DeliveryMode);
    HRESULT  setJmsCorrelationIdAsBytesOnMessage (_variant_t  &CorrelationId);
    HRESULT setJmsDestinationOnMessage (long Destination);
    HRESULT setJmsPriorityOnMessage (long Priority);
    HRESULT setJmsReplyToOnMessage (long Reply);
    HRESULT setJmsExpirationOnMessage (char* Expiration);
	//get jms attribute on message
    CString getJmsTypeOnMessage ();
    CString getJmsTimestampOnMessage ();
    long getJmsReplyToOnMessage ();
    VARIANT_BOOL getJmsRedeliveredOnMessage ();
    long getJmsDestinationOnMessage ();
    CString  getJmsCorrelationIdOnMessage ();
    long getJmsDeliveryModeOnMessage ();
    CString  getJmsExpirationOnMessage ();
    CString getJmsMessageIdOnMessage ();
    long getJmsPriorityOnMessage ();
    _variant_t getJmsCorrelationIdAsBytesOnMessage ();
	//////////////////////////////////////////////////////////////////////

  private:
	//initial while user new message,
	//@param bSend - 0 - message which is received, 1 - message which is to be sended
	//       bMode - 0 -PTOP message, 1 - topic message
	void Initialize(BOOL bSync=FALSE, char *sMessageType="",char* MessageCode="" );

  };
