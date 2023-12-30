#if !defined(RESPMSGPROCESSTHREAD_H)
#define RESPMSGPROCESSTHREAD_H

#include <queue>
using namespace std;

#include "SMSService.h"
#include "NativeThread.h"
#include "DBThread.h"
#include "WriteSocketThread.h"
using namespace ZQ::common;

#include "SMSMsg.h"

#include "Locks.h"

// message queue
typedef queue<LPMessage> MSGQUEUE;

// content message queue
typedef queue<LPSMSMSG>	 SMSMSGQUEUE;

class SMSService;
class WriteSocketThread;
class DBThread;

class RespMsgProcessThread : public NativeThread
{
public:
	RespMsgProcessThread(SMSService* pSrv);
	~RespMsgProcessThread();

	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);

	void  putRespMsgIntoQueue(LPMessage pMsg);
	void  putContentMsgIntoQueue(LPSMSMSG pMsg);
	void  setDataCome();

	bool  StopThread();

private:
	void ProcessMessage();

	// ���� response message
	void ProcessRespMessage(LPMessage pMsg);
	
	// ���ݷ�����response message ���Ͷ����е�content message�Ƚ�
	bool ValidateContentMessage(LPMessage pMsg);

private:
	MSGQUEUE m_queueRespMsg;
	SMSMSGQUEUE m_queueContentMSg;

	HANDLE m_hStop;
	HANDLE m_hDataComeSem;

	SMSService* m_pService;
	WriteSocketThread* m_pWriteSocketThd;

	// �ȴ�TM�����ʱ��
	DWORD m_WaitForTmTime;
};

#endif