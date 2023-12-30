
#if !defined(AFX_PARSER_H__41A43996_E52E_42BE_9348_CQUEUEMANAGEMENT__INCLUDED_)
#define AFX_PARSER_H__41A43996_E52E_42BE_9348_CQUEUEMANAGEMENT__INCLUDED_

#pragma once
#include <vector>

typedef struct JmsQueue_Info
{
	//queue name ,in sending message, it will be used.
	CString strQueueName;

	/// nFomrat:send:nFomrat			= INT_ONLY_SEND
	///			receive:nFomrat			= INT_ONLY_RECEIVE
	///			send and receive:nFomrat= INT_SENDANDRECE
	int nFomrat;	

	/// Current queuename indicates queue or topic: 
	BOOL IsQueue;

}ZQJmsQueueInfo;

typedef std::vector<ZQJmsQueueInfo > zqJmsQueueInfoVector;
class CQueueManageMent
{
public:
	CQueueManageMent(void);
	~CQueueManageMent(void);

	/// @Function  Check Queue is added into JMS by queueName;
	/// @return No exist indicates failure. exist indicates success 
	/// @param nFomrat :send:nFomrat			= INT_ONLY_SEND
	///					receive:nFomrat			= INT_ONLY_RECEIVE
	///					send and receive:nFomrat= INT_SENDANDRECE
	/// @param IsQueue: queue is true,topic is false;
	BOOL QueueIsExist(CString strQueueName,int nFormat,BOOL IsQueue);
	zqJmsQueueInfoVector m_queueVect;
private:

	/// storage adding queue name;


};
#endif