#include "RawMsgProcessThread.h"
#include "Log.h"
#include "ScLog.h"

RawMsgProcessThread::RawMsgProcessThread(SMSService* pSrv)
{
	m_pService = pSrv;
}

RawMsgProcessThread::~RawMsgProcessThread()
{
	glog(Log::L_DEBUG, "线程 RawMsgProcessThread  析构函数");
}

bool RawMsgProcessThread::init()
{
	glog(Log::L_DEBUG, "线程 RawMsgProcessThread  初始化");

	m_hStop = CreateEvent(NULL, false, false, NULL);
	if (m_hStop == NULL)
	{
		glog(Log::L_ERROR, "Failed to create stop event in Process Raw Msg thread");
		return false;
	}

	m_hDataComeSem = CreateSemaphore(NULL, 0, MAX_SEM_VALUE, NULL);
	if (m_hDataComeSem == NULL)
	{
		glog(Log::L_ERROR, "Failed to create DataCome Semaphore in Process Raw Msg thread");
		return false;
	}
	return true;
}

extern NativeThreadPool pool(30);

int RawMsgProcessThread::run()
{
	glog(Log::L_DEBUG, "线程 RawMsgProcessThread  运行");
	
	HANDLE handles[2] = {m_hStop, m_hDataComeSem};
	
	bool bContinued = true;
	DWORD dwWaitStatus;

	while (bContinued)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		case WAIT_OBJECT_0:
			bContinued = false;
			break;
		
		case WAIT_OBJECT_0 + 1:
			ProcessRawMessage();
			break;
		
		case WAIT_TIMEOUT:
		default:
			break;
		}
	}
	return 1;
}

void RawMsgProcessThread::final()
{
	glog(Log::L_DEBUG, "线程 RawMsgProcessThread       结束");
	
	if (m_hDataComeSem)
	{
		CloseHandle(m_hDataComeSem);
		m_hDataComeSem = NULL;
	}

	while(m_queueMsg.size() > 0)
	{
		m_queueMsg.pop();
	}

	if (m_hStop)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}
}


void RawMsgProcessThread::putMsgIntoQueue(LPMessage pMsg)
{
	m_queueMsg.push(pMsg);
	setDataCome();
}

void RawMsgProcessThread::setDataCome()
{
	long lPreCount;
	if (!ReleaseSemaphore(m_hDataComeSem, 1, &lPreCount))
	{
		int ret = GetLastError();
		glog(Log::L_ERROR, "RawMsgProcessThread::ReleaseSemaphore <%d>", ret);
	}
}

void RawMsgProcessThread::ProcessRawMessage()
{
	int queueSize = m_queueMsg.size();
	if (queueSize <= 0)
	{
		return;
	}
	
	LPMessage pMsg = m_queueMsg.front();
	m_queueMsg.pop();
	
	if (pMsg->GetCmd() == CMD_MSG)
	{
		CreateAndSendMsgResponse(pMsg);
		
		glog(Log::L_DEBUG, "线程 RawMsgProcessThread 创建一个 TICPMsgProcessRequest");

		TICPMsgProcessRequest* pTicpMsgProcReq = new TICPMsgProcessRequest(pool, m_pService, pMsg);

		glog(Log::L_DEBUG, "线程 RawMsgProcessThread 启动该   TICPMsgProcessRequest");
		pTicpMsgProcReq->start();
	}
	else
	{
		glog(Log::L_DEBUG, "RawMsgProcessThread ::Message CMD 是 %x", pMsg->GetCmd());

		RespMsgProcessThread* pRespMsgPrcThd = m_pService->GetRspMsgProc();
		
		// put it into RespMsgProcessThread queue
		pRespMsgPrcThd->putRespMsgIntoQueue(pMsg);
	}
}

void RawMsgProcessThread::CreateAndSendMsgResponse(LPMessage pSMsg)
{
	glog(Log::L_DEBUG, "RawMsgProcessThread ::CreateAndSendMsgResponse(), UID 是 %d", pSMsg->GetUID());
	int packagelength = 12;
	int cmd = CMD_MSG_RESP;
	int uid = pSMsg->GetUID();

	Message* pMsg = new Message(packagelength, cmd, uid);

	WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
	pWriteSocketThd->putRespMsgIntoQueue(pMsg);
}

bool RawMsgProcessThread::StopThread()
{
	return SetEvent(m_hStop);
}