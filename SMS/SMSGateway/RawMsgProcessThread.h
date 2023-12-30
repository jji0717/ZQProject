#if !defined (RAWMSGPROCESSTHREAD_H)
#define RAWMSGPROCESSTHREAD_H

#include <queue>
using namespace std;

#include "SMSService.h"
#include "SMSMsg.h"
#include "RespMsgProcessThread.h"
#include "WriteSocketThread.h"

#include "NativeThread.h"
using namespace ZQ::common;
#include "NativeThreadPool.h"

#include "TICPMsgProcessRequest.h"

#include "Locks.h"

typedef queue<LPMessage> MSGQUEUE;

class SMSService;
class WriteSocketThread;

class RawMsgProcessThread : public NativeThread
{
public:
	RawMsgProcessThread(SMSService* pSrv);
	~RawMsgProcessThread();

	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);


	void  setDataCome();
	void  putMsgIntoQueue(LPMessage pMsg);

	bool  StopThread();

private:
	void ProcessRawMessage();

	void CreateAndSendMsgResponse(LPMessage pMsg);

private:
	MSGQUEUE m_queueMsg;

	SMSService* m_pService;

	HANDLE m_hStop;
	HANDLE m_hDataComeSem;
};

#endif