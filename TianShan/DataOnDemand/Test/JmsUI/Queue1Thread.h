#ifndef __QUEUE1THREAD_H__
#define __QUEUE1THREAD_H__

#include "NativeThread.h"
#include "Log.h"
#include "JMSPublisher.h"
#include "Message1.h"
#include "Message2.h"

class Queue1Thread : public ZQ::common::NativeThread
{
public:
	Queue1Thread(const char* desname, 
				 TianShanIce::common::JMSPublisherManager* jmsPublisherMgr);
	~Queue1Thread();

	virtual int run();

	void stop() { SetEvent(m_hStop); };
	void stopSend() { m_bStopSend = true; };
	
	void pushMessage1(DWORD sendTimes,
					  DWORD sendInterval,
					  const char* groupID,
					  const char* dataType,
					  const char* updateMode,
					  const char* rootPath);

	void pushMessage2(DWORD sendTimes,
					  DWORD sendInterval,
					  const char* groupID,
					  const char* dataType,
					  const char* destination,
					  const char* destinationType,
					  const char* expiredTime,
					  const char* messgeID = NULL);

private:
	void sendMessage1();
	void sendMessage2();

	void createTime(char* strTime);

private:
	int m_Message;
	HANDLE m_hStop;
	HANDLE m_hDataCome;
	bool m_bStopSend;
	TianShanIce::common::JMSPublisherManager* m_JmsPublisherMgr;

	char m_destName[MAX_PATH];

	/************************************************************************/
	/*              Message 1 的 参数                                       */
	/************************************************************************/
	char m_UpdateMode[20];
	char m_RootPath[MAX_PATH];
	
	/************************************************************************/
	/*              Message 2 的 参数                                       */
	/************************************************************************/
	char m_Destination[20];
	char m_DestinationType[20];
	char m_ExpiredTime[20];
	char m_MessageID[20];
	
	/************************************************************************/
	/*              两个Message的共同参数                                   */
	/************************************************************************/
	char m_GroupID[20];
	char m_DataType[20];
	DWORD m_SendTimes;
	DWORD m_SendInterval;
};

#endif