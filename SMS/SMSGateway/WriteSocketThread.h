#if !defined (WRITESOCKETTHREAD_H)
#define WRITESOCKETTHREAD_H

#include "NPVRSocket.h"
#include "SMSService.h"
#include "NativeThread.h"
using namespace ZQ::common;
#include "RespMsgProcessThread.h"
#include "ReadSocketThread.h"

#include <queue>
using namespace std;

// message queue
typedef queue<LPMessage> MSGQUEUE;

// content message queue
typedef queue<LPSMSMSG>	 SMSMSGQUEUE;

class SMSService;
class RespMsgProcessThread;
class ReadSocketThread;

class WriteSocketThread : public NativeThread
{
public:
	WriteSocketThread(SMSService* pSrv);
	~WriteSocketThread();

	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);

	// login operation
	void Login(bool bState)	 { m_bLogined = bState;  };
	bool IsLogined()		 { return m_bLogined;  };

	// connect operation
	void Connected(bool bState)	{ m_bConnected = bState;  };
	bool IsConnected()			{ return m_bConnected;  };


	// put response messages into queue
	bool putRespMsgIntoQueue(LPMessage pMsg);

	// put content messages into queue
	bool putContentMsgIntoQueue(LPSMSMSG pSMSMsg);
	
	bool StopThread();

private:
	void setDataCome();

	void ProcessSendMsg();

	// ���ɷ��Ͷ��ŵ�����
	void CreateSendMessage();
	void CreateResponse();
	void CreateContent();

	void PutBackToRespThread(LPSMSMSG pSMSMsg);

private:
	SMSService* m_pService;

private:
	HANDLE m_hStop;
	HANDLE m_hDataComeSem;

	// store response msg
	MSGQUEUE	m_queueHighSendMsg;
	
	// store content msg
	SMSMSGQUEUE m_queueLowSendMsg;

	// �Ƿ񾭹�TM��������֤�ı�־
	bool   m_bLogined;

	// �Ƿ����ӵ�TM�������ı�־
	bool   m_bConnected;
	
	// �ȴ�TM�������ظ������ʱ��
	DWORD  m_WaitForTmTime;

	// ��TM�ط���������
	DWORD  m_TMRedoMaxTimes;
};

#endif