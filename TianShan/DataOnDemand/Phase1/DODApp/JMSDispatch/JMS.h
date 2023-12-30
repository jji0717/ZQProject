#pragma once

#include "..\include\jmshead.h"

using namespace ZQ::JMSCpp;
class CJMSBaseContent;
class  CJMSPortManager;
class CReceiveJmsMsg;
typedef std::list< CJMSBaseContent *> CJMSBaseList;

#define INT_ONLY_SEND		1
#define INT_ONLY_RECEIVE	2
#define INT_SENDANDRECE		3
#define INT_MSGMAXLENGTH	(64*1024)

class CJMSTxMessage:public TextMessage
{
public:
	int GetDataSize() {return INT_MSGMAXLENGTH;}

};
class CJMS
{
public:
	CJMS(char *inStr);
	~CJMS();

	/// @Function .Add a queue or topic to JMS client. 
	/// @return true on success,false on failure
	/// if return failure,The queue or topic name may not register in JMS server.
	/// @param bIsQueue: It is queue ,bIsQueue is true. It is topic .Its value is false. 
	/// @param queueName: It is queueName, its format is "queue/quque_cf"
	/// @param nFomrat :send:nFomrat			= INT_ONLY_SEND
	///					receive:nFomrat			= INT_ONLY_RECEIVE
	///					send and receive:nFomrat= INT_SENDANDRECE
	/// @param Inparse :If the queue is receive-queue, the parse will get reply message.
	BOOL AddQueueOrTopic(BOOL bIsQueue,char *queueName,int nFomrat,CReceiveJmsMsg *Inparse=NULL);

	/// @Function :the current client will connect JMS server.
	/// @return true on success,false on failure
	/// if return failure, now ,there is not automatic after
	/// @param :no
	BOOL StartConnect();

	/// @Function .send message to queue or topic by queuename. 
	/// @return true on success,false on failure
	/// if return failure,the client connection may disconnect.
	/// @param bIsQueue: It is queue ,bIsQueue is true. It is topic .Its value is false. 
	/// @param queueName: It is queueName, its format is "queue/quque_cf"
	/// @param MsgData:sending's data.
	/// @param intkey and nPara1,the nPara1 is int_value.
	/// @param strkey and cPara2,the cPara2 is str_value.
	BOOL SendMsg(BOOL IsQueue,char *queueName,char * MsgData,char *intkey=NULL,int nPara1=0,char *strkey=NULL,char *cPara2=NULL);

	/// @Function .send message to queue or topic by queuename. 
	/// @return true on success,false on failure
	/// if return failure,the client connection may disconnect.
	/// @param bIsQueue: It is queue ,bIsQueue is true. It is topic .Its value is false. 
	/// @param queueName: It is queueName, its format is "queue/quque_cf"
	/// @param MsgData:sending's data.
	/// @param responseMsg: return message.
	/// @param nTimeOut:The user 's sustaind time.(unit:millisecond)
	/// @param intkey and nPara1,the nPara1 is int_value.
	/// @param strkey and cPara2,the cPara2 is str_value.
	BOOL SyncSendMsg(BOOL IsQueue,char *queueName,char * MsgData,CJMSTxMessage &responseMsg,int nTimeOut=5000,char *intkey=NULL,int nPara1=0,char *strkey=NULL,char *cPara2=NULL);

	//zhenan add for stop receiver msg.
	BOOL  ConnectStop();
private:
	/// @Function : free all lists resources,
	/// @return no
	/// @param :no
	void FreeAllList();

	/// @Function :found queue handle in input list by queuename.
	/// if return failure,the queue name was found in list .notice: it is not Unique.
	/// @param :queueName : format "queue/queue_cf".
	BOOL AssureUniquelyList(char *queueName,CJMSBaseList &listjms);
	
public:

	Context *m_JndiContext;
	ConnectionFactory m_ConnectionFactory;  
	Connection m_Connection; 

	BOOL m_bConnectionOK;
	
//Recv Queue List
	CJMSBaseList   m_QueueRecvList;

//Recv Topic List
	CJMSBaseList   m_TopicRecvList;
private:
	//Send Queue List
	CJMSBaseList   m_QueueSendList;
	
	//Send Topic List
	CJMSBaseList   m_TopicSendList;

};

class CJMSBaseContent
{
public:
	CJMSBaseContent(CJMS *jms,char *queueName,BOOL bIsSend);
	~CJMSBaseContent();

	Session m_Session;  
	Destination m_destination;
	Producer m_producer;
	Consumer m_consumer;
	BOOL m_bError;
	BOOL m_bIsSend;
	char m_QueueName[MAX_PATH];
};