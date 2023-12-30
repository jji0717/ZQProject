#ifndef __ACTIVEJMS_H__
#define __ACTIVEJMS_H__

#import "msxml3.dll"
#include <atlbase.h>
#include <afxctl.h>

#include<list>
#include "JMSBase.h"


#include "JMSMessage.h"
#include"MyActiveJMSListener.h"
class IParse;
class CJMSBase;


//support JMS server
#define JMSSERVER_JBOSS     0
#define JMSSERVER_SWIFT     1

//while initialize, can not create provider
#define ERRORCODE_INITIALIZE_NOPROVIDER       0x0100
//topic can not be sent for there is no topic name in topic list, you should addone topic
#define ERRORCODE_TOPICSEND_NOTOPICNAME       0x0101   
//topic can not be sent for it is a received message
#define ERRORCODE_TOPICSEND_MESSAGETYPE		  0x0102
//queue can not be sent for there is no topic name in queue list, you should addone queue
#define ERRORCODE_PTOPSEND_NOQUEUENAME		  0x0103
//queue can not be sent for it is a received message
#define ERRORCODE_PTOPSEND_MESSAGETYPE		  0x0104
//topic can not be sent for no handler
#define ERRORCODE_TOPICSEND_NOHANDLER		  0x0105
//queue can not be sent for no handler
#define ERRORCODE_PTOPSEND_NOHANDLER		  0x0106
//while sync send, we should have temp queue receiver handler, it is get in message defined
#define ERRORCODE_SYNCSEND_NORECEIVEHANDLER   0x0107
//while sendmessage, mode is not ptop or topic
#define ERRORCODE_SENDMESSAGE_MESSAGEMODE     0x0108


class CActiveJMS
{
public:
	CActiveJMS(BYTE nServerType = JMSSERVER_JBOSS);
	~CActiveJMS(void);

	//initialize property, factory,etc according to JMS server,you should modify Providerkey to you server,and QueueConnectionFactoryValue,TopicConnectionFactoryValue 
	///@param 
	///for jboss :
    /// Providerkey="192.168.80.46:1099";
     //FactoryInitialKey="java.naming.provider.url";
     // FactoryInitialKey="java.naming.factory.initial";
     // FactoryInitialValue="org.jnp.interfaces.NamingContextFactory";
     // FactoryurlKey="java.naming.factory.url.pkgs";
     // FactoryurlValue="org.jboss.naming";
	 // QueueConnectionFactoryValue="ConnectionFactory1";
     // TopicConnectionFactoryValue="ConnectionFactory2";
	///for swift:

	HRESULT Initialize(CString Providerkey,CString ProviderValue,CString FactoryInitialKey,CString FactoryInitialValue,
		CString QueueConnectionFactoryValue, CString TopicConnectionFactoryValue,
		CString FactoryurlKey=""/*for jboss*/,CString FactoryurlValue=""/*for jboss*/);

   
   	//@Function  This function send messagebody to a topic
    // @Param    char* buf(IN), the char to send
    // @Param    char* TopicName(IN),identify the Topic 
	HRESULT TopicSend(char* buf,char* TopicName);

   	//@Function  This function send a message to a topic
    // @Param    CJMSMessage* IN, defined message to send
    // @Param    char* TopicName(IN),identify the Topic 
	HRESULT TopicSend(CJMSMessage* pMessage ,char* TopicName);

	//@Function  This function send messagebody to a queue
    // @Param    char* buf(IN), the char to send
    // @Param    char* QueueName(IN),identify the queue
	HRESULT PtopSend(char* buf,char *QueueName);

	//@Function  This function send a defined message to a queue
    // @Param    CJMSMessage* (IN), the char to send
    // @Param    char* QueueName(IN),identify the queue
    HRESULT PtopSend(CJMSMessage* pMessage,char *QueueName);

	//@Function  This function send messagebody to all topic in topic list
    // @Param    char* buf(IN), the char to send
	HRESULT SendToAllTopics(char* buf);

	//@Function  This function send a message to all topic in topic list
    // @Param    CJMSMessage* (IN), the char to send
	HRESULT SendToAllTopics(CJMSMessage* pMessage);

	//@Function  This function add a queue we can sent to it and receive from it
    // @Param    char* QueueName(IN), queue name
    // @Param    IParse*  parse(IN), to parse the message received
	HRESULT AddOneQueue(char* QueueName,IParse*  parse);

	//@Function  This function add a queue we only can sent to
    // @Param    char* QueueName(IN), queue name
    HRESULT AddOneSendQueue(char* QueueName);

    //@Function  This function add a queue we can sent to it but can not receive from it
    // @Param    char* QueueName(IN), queue name
    // @Param    IParse*  parse(IN), to parse the message received
	 HRESULT AddOneReceiveQueue(char* QueueName,IParse*  parse);

    //@Function  This function add a topic we can sent to it and receive from it
    // @Param    char* TopicName(IN), topic name
    // @Param    IParse*  parse(IN), to parse the message received
   	HRESULT AddOneTopic(char* TopicName,IParse*  parse);

    //@Function  This function add a topic  we only can sent to
    // @Param    char* TopicName(IN), topic name
  	HRESULT AddOneSendTopic(char* TopicName);

    //@Function  This function add a queue we can sent to it but can not receive from it
    // @Param    char* TopicName(IN), topic name
    // @Param    IParse*  parse(IN), to parse the message received
    HRESULT AddOneRecvTopic(char* TopicName,IParse*  parse);

    //@Function   This function send message to a queue and wait for a  responding message 
    // @Param     CJMSMessage* pMessage, the message to send
    // @Param     IParse Parse  the parse for the receiving message
    // @Param     int iWaitTimeOut   Wait for responding message timeout
    HRESULT SyncSend(CJMSMessage* pMessage,char*QueueName,IParse* Parse,int iWaitTimeOut);

	//Free all list
	void FreeAllList();

	// @Function   This function dispatch all lisen message to dest parse
    // @Param      long lngConsumer  the handle of the comsumer
    // @Param      CJMSMessage* message  the receive message
	void Dispatch(long lngConsumer,CJMSMessage* message);

   	//@Function  This function send a message to a topic
    // @Param    CJMSMessage* IN, defined message to send
    // @Param    long (IN),handler of send queue
    HRESULT TopicSend(CJMSMessage* pMessage,long handler);

    //@Function  This function send message to a queue
    // @Param    CJMSMessage* pMessage, the message to send
    // @Param    long handler  handle of the producer
    HRESULT QueueSend(CJMSMessage* pMessage,long handler);

public:
     //listen object
     CMyActiveJMSListener m_Listener;
     //ActiveJMS object
	 ActiveJMS::ActiveJMSDispatchPtr objActiveJMS;
public:
     
	 //Provider handle
     long m_lngProvider;

	 //Queue connection handle
	 long m_lngQueueConnection;

	 //Queue send session
	 long m_lngQueueSendSession;

	 //Queue receive session
     long m_lngQueueReceiveSession;
	
	 //topic connection handle
     long m_lngTopicConnection;

	 //topic send session
	 long m_lngTopicSendSession;

	 //topic receive session
     long m_lngTopicReceiveSession;
	     
     //Send Queue List
	 CJMSBaseList   m_QueueSendList;
     //Send Queue List
     CJMSBaseList   m_QueueRecvList;
     //Send Topic List
	 CJMSBaseList   m_TopicSendList;
     //Recv Topic List
	 CJMSBaseList   m_TopicRecvList;
         
	 BYTE   m_nServerType; //supported servertype 
};

#endif