#include "RespMsgProcessThread.h"
#include "Log.h"
#include "ScLog.h"

RespMsgProcessThread::RespMsgProcessThread(SMSService* pSrv)
{
	m_pService = pSrv;

	// �����߳��л�� TM ���ȴ�ʱ��
	m_WaitForTmTime = m_pService->GetWaitTime();
}

RespMsgProcessThread::~RespMsgProcessThread()
{
	glog(Log::L_DEBUG, "�߳� RespMsgProcessThread ��������");
}

bool RespMsgProcessThread::init()
{
	glog(Log::L_DEBUG, "�߳� RespMsgProcessThread ��ʼ��");
	
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
	glog(Log::L_DEBUG, "�߳� RespMsgProcessThread ����");
	
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
	glog(Log::L_DEBUG, "�߳� RespMsgProcessThread      ����");

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
		// �ȴ��� response message
		ProcessRespMessage(pMsg);
	}
}

void RespMsgProcessThread::ProcessRespMessage(LPMessage pMsg)
{
	int cmd = pMsg->GetCmd();
	
	glog(Log::L_DEBUG, "�߳� RespMsgProcessThread::Message CMD �� %x", cmd);

	if (cmd == CMD_MSG_RESP) // message response
	{
		glog(Log::L_DEBUG, "�߳� RespMsgProcessThread::Message Response, UID �� %d", pMsg->GetUID());

		if( !ValidateContentMessage(pMsg) )
		{
			glog(Log::L_DEBUG, "�߳� RespMsgProcessThread::û���ҵ�����");
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
		
		// �ҵ�ƥ��� message���� message response�� uid = content message�� uid
		if (pSmsg->GetUID() == uid)
		{
			// validated messages
			glog(Log::L_DEBUG, "�߳� RespMsgProcessThread::Message Complete, UID �� %d", uid);
			
			// SMS(TM) finished
			pSmsg->SMSFinished(true);

			// put message into db update queue
			DBThread* pDBThread = m_pService->GetDBThread();
			if (!pDBThread->putUpdateMsg(pSmsg))
			{
				// ���������¶���ʧ�ܾ��ڱ��߳�ɾ����Ϣ
				if (pSmsg)
				{
					delete pSmsg;
					pSmsg = NULL;
				}
			}

			return true;// don't put it into queue again
		}
		else // û���ҵ�ƥ��ľ����·Ż�queue��ȥ
		{
			// �������������
			DWORD waitTime = GetTickCount() - pSmsg->GetSendToTmTime();

			// ����10���Ӿ�ɾ��
			if (waitTime >= 600000)
			{
				glog(Log::L_DEBUG, "�߳� RespMsgProcessThread::����<%d> û���յ���֤���ҳ�ʱ, ���ɾ��", pSmsg->GetUID());
				
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



