#include "WriteSocketThread.h"
#include "Log.h"
#include "ScLog.h"

WriteSocketThread::WriteSocketThread(SMSService* pSrv)
{
	m_pService = pSrv;
	
	// 初始化是没有登陆
	m_bLogined = false;

	// 初始化时没有连接
	m_bConnected = false;

	// 从主线程中获得TM最大重做次数
	m_TMRedoMaxTimes = m_pService->GetTMRedoTimes();

	// 从主线程中获得TM最大等待时间
	m_WaitForTmTime =  m_pService->GetWaitTime();
}

WriteSocketThread::~WriteSocketThread()
{
	glog(Log::L_DEBUG, "线程 WriteSocketThread    析够函数");
}

bool WriteSocketThread::init()
{
	glog(Log::L_DEBUG, "线程 WriteSocketThread    初始化");
	
	m_hStop = CreateEvent(NULL, false, false, NULL);
	if (m_hStop == NULL)
	{
		glog(Log::L_DEBUG, "Failed to create stop event in Process Raw Msg thread");
		return false;
	}

	m_hDataComeSem = CreateSemaphore(NULL, 0, MAX_SEM_VALUE, NULL);
	if (m_hDataComeSem == NULL)
	{
		glog(Log::L_DEBUG, "Failed to create DataCome Semaphore in Process Raw Msg thread");
		return false;
	}
	
	return true;
}

int WriteSocketThread::run()
{
	glog(Log::L_DEBUG, "线程 WriteSocketThread    运行");
	
	bool bContinued = true;
	DWORD dwWaitStatus;
	HANDLE handles[2] = {m_hStop, m_hDataComeSem};

	while (bContinued)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		case WAIT_OBJECT_0:
			bContinued = false;
			break;
		
		case WAIT_OBJECT_0 + 1:
			ProcessSendMsg();
			break;
		
		case WAIT_TIMEOUT:
		default:
			break;
		}
	}
	return 1;
}

void WriteSocketThread::final()
{
	glog(Log::L_DEBUG, "线程 WriteSocketThread         结束");
	
	if (m_hDataComeSem)
	{
		CloseHandle(m_hDataComeSem);
		m_hDataComeSem = NULL;
	}

	while(m_queueHighSendMsg.size() > 0)
	{
		m_queueHighSendMsg.pop();
	}

	while(m_queueLowSendMsg.size() > 0)
	{
		m_queueLowSendMsg.pop();
	}

	if (m_hStop)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}
}

bool WriteSocketThread::putRespMsgIntoQueue(LPMessage pMsg)
{
	m_queueHighSendMsg.push(pMsg);

	// only Connected can be signaled
	if (IsConnected())
	{
		setDataCome();
	}
	else
	{
		glog(Log::L_DEBUG, "DisConnected");
	}
	return false;
}

bool WriteSocketThread::putContentMsgIntoQueue(LPSMSMSG pSMSMsg)
{
	m_queueLowSendMsg.push(pSMSMsg);
	
	// only Connected can be signaled
	if (IsConnected())
	{
		setDataCome();
		
		return true;
	}
	else
	{
		glog(Log::L_DEBUG, "DisConnected");
	}
	return false;
}

void WriteSocketThread::setDataCome()
{
	long lPreCount;
	if (!ReleaseSemaphore(m_hDataComeSem, 1, &lPreCount))
	{
		int ret = GetLastError();
		glog(Log::L_ERROR, "WriteSocketThread::ReleaseSemaphore <%d>", ret);
	}
}

void WriteSocketThread::ProcessSendMsg()
{
	// if not Logined, should send login message
	if (!IsLogined())
	{
		ReadSocketThread* pReadSocketThread = m_pService->GetReadSocket();
		pReadSocketThread->ReConnect();
		return;
	}

	// in this case, the state is Logined and Connected
	CreateSendMessage();
}

void WriteSocketThread::CreateSendMessage()
{
	// 处理回复
	CreateResponse();

	// 处理内容
	CreateContent();	
}

void WriteSocketThread::CreateResponse()
{
	// cmd message, like heartbeat
	if (m_queueHighSendMsg.size() > 0)
	{
		glog(Log::L_DEBUG, "线程 WriteSocketThread 生成短信回复");

		// 获取NPVRSOCKET的指针
		ReadSocketThread* pReadSocketThread = m_pService->GetReadSocket();
		NPVRSocket* pNSocket = pReadSocketThread->GetNPVRSocket();

		LPMessage pMsg = m_queueHighSendMsg.front();
		m_queueHighSendMsg.pop();

		// get times which send to tm
		int TMRedoTimes = pMsg->GetSMSTimes();

		if (TMRedoTimes <= m_TMRedoMaxTimes)
		{
			if(pMsg->GetPackageLength() == 12)
			{
				char content[13];
				memset(content, 0x00, 13*sizeof(char));
				pMsg->CombineMessage(content);

				// set times which send to tm
				pMsg->SetSMSTimes(TMRedoTimes + 1);
				
				if(!pNSocket->SendC(content, 12))
				{
					// put response messages pack to High Queue
					m_queueHighSendMsg.push(pMsg);

					glog(Log::L_ERROR, "发送短信回复失败, UID 是 %d", pMsg->GetUID());
					
					//重新连接
					pReadSocketThread->ReConnect();
					
					return;
				}
				else
				{
					glog(Log::L_DEBUG, "发送短信回复成功, UID 是 %d", pMsg->GetUID());
					
					// delete send successfully message
					if (pMsg)
					{
						delete pMsg;
						pMsg = NULL;
					}
				}
			}
			else // content message
			{
				glog(Log::L_DEBUG, "not response message");
			}
		}
		else
		{//已经达过重发次数，删除
			if (pMsg)
			{
				delete pMsg;
				pMsg = NULL;
			}
		}
	}
}

void WriteSocketThread::CreateContent()
{
	// content message
	if (m_queueLowSendMsg.size() > 0)
	{
		glog(Log::L_DEBUG, "线程 WriteSocketThread 生成下行数据");

		// 获取NPVRSOCKET的指针
		ReadSocketThread* pReadSocketThread = m_pService->GetReadSocket();
		NPVRSocket* pNSocket = pReadSocketThread->GetNPVRSocket();

		LPSMSMSG pSMsg = m_queueLowSendMsg.front();
		m_queueLowSendMsg.pop();

		// get times which send to tm
		int TMRedoTimes = pSMsg->GetSMSTimes();

		if (TMRedoTimes <= m_TMRedoMaxTimes)
		{
			if (pSMsg->GetPackageLength() > 12)
			{
				//设置短信内容的命令
				pSMsg->SetCmd(CMD_MSG);
				
				// 拼接短信回复内容 
				int len = pSMsg->CombineContentMessage();

				char* content = new char[len + 1];
				memset(content, 0x00, (len + 1)*sizeof(char));
				memcpy(content, pSMsg->GetContent(), len);

				// set time which send to tm
				pSMsg->SetSendToTmTime(GetTickCount());
				
				// set times which send to tm
				pSMsg->SetSMSTimes(TMRedoTimes + 1);
			
				glog(Log::L_DEBUG, "CONTENT length = %d", len);
				if (!pNSocket->SendC(content, len))
				{
					glog(Log::L_ERROR, "发送下行数据失败, UID 是 %d", pSMsg->GetUID());
					
					// put sended message back into Response Message Thread Queue to wait for validation
					PutBackToRespThread(pSMsg);

					// 重新连接
					pReadSocketThread->ReConnect();

					return;
				}

				glog(Log::L_DEBUG, "发送下行数据成功, UID 是 %d",pSMsg->GetUID());
				delete content;
				
				// put sended message back into Response Message Thread Queue to wait for validation
				PutBackToRespThread(pSMsg);
			}
			else
			{
				glog(Log::L_DEBUG, "没有下行数据");
			}
		}
		else
		{//已经达到重发最大次数
			if (pSMsg)
			{
				delete pSMsg;
				pSMsg = NULL;
			}
		}
	}
}

// put sended message back into Response Message Thread Queue to wait for validation
void WriteSocketThread::PutBackToRespThread(LPSMSMSG pSMSMsg)
{
	RespMsgProcessThread* pRespMsgProcThd = m_pService->GetRspMsgProc();
	pRespMsgProcThd->putContentMsgIntoQueue(pSMSMsg);
}

bool WriteSocketThread::StopThread()
{
	return SetEvent(m_hStop);
}

