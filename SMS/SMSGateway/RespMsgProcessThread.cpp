#include "RespMsgProcessThread.h"
#include "Log.h"
#include "ScLog.h"

RespMsgProcessThread::RespMsgProcessThread(SMSService* pSrv)
{
	m_pService = pSrv;

	// 从主线程中获得 TM 最大等待时间
	m_WaitForTmTime = m_pService->GetWaitTime();
}

RespMsgProcessThread::~RespMsgProcessThread()
{
	glog(Log::L_DEBUG, "线程 RespMsgProcessThread 析构函数");
}

bool RespMsgProcessThread::init()
{
	glog(Log::L_DEBUG, "线程 RespMsgProcessThread 初始化");
	
	m_hStop = CreateEvent(NULL, false, false, NULL);
	if (m_hStop == NULL)
	{
		glog(Log::L_ERROR, "Failed to create stop event in Process Rsp Msg thread");
		return false;
	}

	m_hDataComeSem = CreateSemaphore(NULL, 0, MAX_SEM_VALUE, NULL);
	if (m_hDataComeSem == NULL)
	{
		glog(Log::L_ERROR, "Failed to create DataCome Semaphore in Process Rsp Msg thread");
		return false;
	}

	m_pWriteSocketThd = m_pService->GetWriteSocket();
	if (!m_pWriteSocketThd)
	{
		glog(Log::L_ERROR, "Failed to get write socket thread pointer");
		return false;
	}
	
	return true;
}

int RespMsgProcessThread::run()
{
	glog(Log::L_DEBUG, "线程 RespMsgProcessThread 运行");
	
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
		case WAIT_TIMEOUT:
			ProcessMessage();
			break;
			
		default:
			break;
		}
	}
	
	return 1;
}

void RespMsgProcessThread::final()
{
	glog(Log::L_DEBUG, "线程 RespMsgProcessThread      结束");

	if (m_hDataComeSem)
	{
		CloseHandle(m_hDataComeSem);
		m_hDataComeSem = NULL;
	}

	while(m_queueRespMsg.size() > 0)
	{
		m_queueRespMsg.pop();
	}

	while(m_queueContentMSg.size() > 0)
	{
		m_queueContentMSg.pop();
	}

	if (m_hStop)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}
}

void RespMsgProcessThread::putRespMsgIntoQueue(LPMessage pMsg)
{
	m_queueRespMsg.push(pMsg);
	setDataCome();
}

void RespMsgProcessThread::putContentMsgIntoQueue(LPSMSMSG pMsg)
{
	m_queueContentMSg.push(pMsg);
	setDataCome();
}

void RespMsgProcessThread::setDataCome()
{
	long lPreCount;
	if (!ReleaseSemaphore(m_hDataComeSem, 1, &lPreCount))
	{
		int ret = GetLastError();
		glog(Log::L_ERROR, "RespMsgProcessThread::ReleaseSemaphore <%d>", ret);
	}
}

void RespMsgProcessThread::ProcessMessage()
{
	int len = m_queueRespMsg.size();
	if (len <= 0)
	{
		return;
	}
	
	LPMessage pMsg = m_queueRespMsg.front();
	m_queueRespMsg.pop();
	if (pMsg->GetCmd() != CMD_MSG)
	{
		// 先处理 response message
		ProcessRespMessage(pMsg);
	}
}

void RespMsgProcessThread::ProcessRespMessage(LPMessage pMsg)
{
	int cmd = pMsg->GetCmd();
	
	glog(Log::L_DEBUG, "线程 RespMsgProcessThread::Message CMD 是 %x", cmd);

	if (cmd == CMD_MSG_RESP) // message response
	{
		glog(Log::L_DEBUG, "线程 RespMsgProcessThread::Message Response, UID 是 %d", pMsg->GetUID());

		if( !ValidateContentMessage(pMsg) )
		{
			glog(Log::L_DEBUG, "线程 RespMsgProcessThread::没有找到短信");
		}
		if (pMsg)
		{
			delete pMsg;
			pMsg = NULL;
		}
	}
}

// validate messages
bool RespMsgProcessThread::ValidateContentMessage(LPMessage pMsg)
{
	int uid = pMsg->GetUID();

	int queueSize = m_queueContentMSg.size();
	
	for (int i = 0; i < queueSize; i++)
	{	
		LPSMSMSG pSmsg = m_queueContentMSg.front();
		m_queueContentMSg.pop();
		
		// 找到匹配的 message，即 message response的 uid = content message的 uid
		if (pSmsg->GetUID() == uid)
		{
			// validated messages
			glog(Log::L_DEBUG, "线程 RespMsgProcessThread::Message Complete, UID 是 %d", uid);
			
			// SMS(TM) finished
			pSmsg->SMSFinished(true);

			// put message into db update queue
			DBThread* pDBThread = m_pService->GetDBThread();
			if (!pDBThread->putUpdateMsg(pSmsg))
			{
				// 如果插入更新队列失败就在本线程删短消息
				if (pSmsg)
				{
					delete pSmsg;
					pSmsg = NULL;
				}
			}

			return true;// don't put it into queue again
		}
		else // 没有找到匹配的就重新放回queue中去
		{
			// 检查其生存周期
			DWORD waitTime = GetTickCount() - pSmsg->GetSendToTmTime();

			// 超过10分钟就删掉
			if (waitTime >= 600000)
			{
				glog(Log::L_DEBUG, "线程 RespMsgProcessThread::短信<%d> 没有收到验证并且超时, 因此删除", pSmsg->GetUID());
				
				if (pSmsg)
				{
					delete pSmsg;
					pSmsg = NULL;
				}
			}
			else
			{
				// not validated messages
				m_queueContentMSg.push(pSmsg);//put it into queue again
			}
		}
	}
	return false;
}

bool RespMsgProcessThread::StopThread()
{
	return SetEvent(m_hStop);
}



