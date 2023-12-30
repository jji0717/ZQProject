#ifndef __QUEUE2THREAD_H__
#define __QUEUE2THREAD_H__

#include "NativeThread.h"
#include "Log.h"
#include "JMSPublisher.h"

class Queue2Thread : public ZQ::common::NativeThread
{
public:
	Queue2Thread(const char* desname, TianShanIce::common::JMSPublisherManager* jmsPublisherMgr);
	~Queue2Thread();

	virtual int run();

	void stop() { SetEvent(m_hStop); };
	void stopSend() { m_bStopSend = true; };
	
	void pushMessage3(DWORD sendTimes,
					  DWORD sendInterval,
					  const char* groupID,
					  const char* dataType,
					  const char* updateMode,
					  const char* filePath,
					  const char* rootPath);
	
	void pushMessage4(DWORD sendTimes,
					  DWORD sendInterval,
					  const char* groupID,
					  const char* dataType,
					  const char* destination,
					  const char* expiredTime,
					  const char* operationCode,
					  const char* destinaiontype);

private:
	void sendMessage3();
	void sendMessage4();

	void createTime(char* strTime);

private:
	int m_Message;
	HANDLE m_hStop;
	HANDLE m_hDataCome;
	bool m_bStopSend;
	TianShanIce::common::JMSPublisherManager* m_JmsPublisherMgr;

	char m_destName[MAX_PATH];

	/************************************************************************/
	/*              Message 3 的 参数                                       */
	/************************************************************************/
	char m_UpdateMode[20];
	char m_FilePath[MAX_PATH];
	char m_RootPath[MAX_PATH];
	
	/************************************************************************/
	/*              Message 4 的 参数                                       */
	/************************************************************************/
	char m_Destination[20];
	char m_ExpiredTime[20];
	char m_OperationCode[20];
	char m_DestinationType[20];

	/************************************************************************/
	/*              两个 Message 共同参数                                   */
	/************************************************************************/
	DWORD m_SendTimes;
	DWORD m_SendInterval;
	char m_GroupID[20];
	char m_DataType[20];
};

#endif