// JmsClient.h: interface for the JmsClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JMSCLIENT_H__916A11F6_1207_43AC_8144_AAF9DC1A1ADD__INCLUDED_)
#define AFX_JMSCLIENT_H__916A11F6_1207_43AC_8144_AAF9DC1A1ADD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

#define WRITESESSION 1
#define READSESSION  2
#define READANDWRITE 3

class JmsConnection
{
public:
    JmsConnection();

    int Connect(char *pcURL,char *pcNamingContextFactory,char *pcConnectionFactory);
    int DisConnect();
	~JmsConnection();
private:
	void *_jmsContext;
	void *_jmsFactory;
	void *_jmsConnection;
};

class JmsDestination  
{
public:
	JmsDestination(JmsConnection* provider);
	int Initialize(int sessionType, char *queueTopicName);
    int Send(JmsMessage* pMessage);
	int SetListener(JmsListener *listener);
	int ConsumerReceive();
    ~JmsDestination();

private:
	JmsConnection *_jmsCon;
    int _sessionType;
	char *_name;	
    void *_jmsDestination;
    void *_jmsWriterSession;
    void *_jmsReaderSession;
    void *_jmsProducer;
    void *_jmsConsumer;
};

class JmsListener
{
public:
	virtual void OnMessage(JMSMessage *message)=0;
}

class JMSMessage()
{
public:
	JMSMessage();
	JMSMessage(void *);
	~JMSMessage();
////get or set header
    int GetMessageID();
	int GetTimeStamp();
	int GetCorrelationID();
	int GetReplyTo();
	int GetDestination();
	int GetDeliveryMode();
	int GetExpiration();
	int GetPriority();

	int SetCorrelationID();
	int SetReplyTo();
	int SetDestination();
/////get or set property
	BYTE GetByteProperty(char *bytepropertykey);
	long GetLongProperty(char *longpropertykey);
	int  GetIntProperty(char *strlongpropertykey);
	char* GetStringProperty(char* stringpropertykey);
	double GetDoubleProperty(char* doublepropertykey);
	float  GetFloatProperty(char *floatpropertykey);
	short  GetShortProperty(char* shortpropertykey);
	BOOL  GetBoolProperty(char* boolpropertykey);

	//set property for application
    int  SetByteProperty(char *bytepropertykey,BYTE Value);
    int  SetLongProperty(char *longpropertykey,long Value);
	int  SetIntProperty(char *strlongpropertykey,int Value);
	int  SetStringProperty(char* stringpropertykey,char* Value);
	int  SetDoubleProperty(char* doublepropertykey,double Value);
	int  SetFloatProperty(char *floatpropertykey,float Value);
	int  SetShortProperty(char* shortpropertykey,short Value);
	int  SetBoolProperty(char* boolpropertykey,BOOL Value);

	//////////////////
	int GetType();
protected:
	void *_message;
	int _type;
}

class JMSBytesMessage:public JMSMessage
{
public:
	JMSBytesMessage();
	
	int BytesGetBodyLength();
	BytesReadBoolean();
	BYTE BytesReadByte();
	BytesReadUnsignedByte();
	short BytesReadShort();
	BytesReadUnsignedShort();
    char BytesReadChar();
	int BytesReadInt();
	long BytesReadLong();
	float BytesReadFloat();
	double BytesReadDouble();
	BytesReadUTF();
	BytesReadBytes();

	int BytesWriteBoolean();
	int BytesWriteByte();
	int BytesWriteShort();
    int BytesWriteChar();
	int BytesWriteInt();
	int BytesWriteLong();
	int BytesWriteFloat();
	int BytesWriteDouble();
	BytesWriteUTF();
	BytesWriteBytes();
}

class JMSMapMessage:public JMSMessage
{
public:
	JMSMapMessage();
    bool MapGetBoolean(char *,);
	BYTE MapGetByte(char *,);
	char MapGetChar(char *,);
	short MapGetShort(char *,);
	int MapGetInt(char *,);
	long MapGetLong(char *,);
	float MapGetFloat(char *,);
	double MapGetDouble(char *,);
	char *MapGetString(char *,);

	int MapSetBoolean(char *,bool value);
	int MapSetByte(char *,Byte value);
	int MapSetChar(char *,char value);
	int MapSetShort(char *,short value);
	int MapSetInt(char *,int value);
	int MapSetLong(char *,long value);
	int MapSetFloat(char *,float value);
	int MapSetDouble(char *,double value);
	int MapSetString(char *,char value);
}

class JMSTextMessage:public JMSMessage
{
public:
	JMSTextMessage();
    char *TextGetText();
	void TextSetText(char *value);
}


#endif // !defined(AFX_JMSCLIENT_H__916A11F6_1207_43AC_8144_AAF9DC1A1ADD__INCLUDED_)
