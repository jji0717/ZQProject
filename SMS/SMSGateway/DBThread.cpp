#include "DBThread.h"

#include "Log.h"
#include "ScLog.h"

using namespace ZQ::common;

DBThread::DBThread(SMSService* pSrv)
{
	m_pService = pSrv;
	m_bDBExist = false;
	
	memset(m_dbPath, 0x00, (MAX_PATH + 1)*sizeof(char));
	strcpy(m_dbPath, m_pService->GetDBPath());

	m_DBClearHours = m_pService->GetDBClearHours();

	m_DBOverTime  = m_pService->GetDBOverTime();
}

DBThread::~DBThread()
{
	glog(Log::L_DEBUG, "�߳� DBThread             ��������");
}

bool DBThread::init()
{
	glog(Log::L_DEBUG, "�߳� DBThread             ��ʼ��");

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

int DBThread::run()
{
	glog(Log::L_DEBUG, "�߳� DBThread             ����");

	WCHAR wcsErr[200];
	memset(wcsErr, 0x00, 200*sizeof(WCHAR));

	// connect to db
	if (!m_npvrdb.ConnectDB(m_dbPath, wcsErr))
	{
		m_bDBExist = false;
		glog(Log::L_ERROR, L"���ݿ����Ӵ��� <%s>", wcsErr);
	}
	else
	{
		m_bDBExist = true;
		glog(Log::L_DEBUG, "���ݿ����ӳɹ�");
		
		DeleteOverdueMessagesInDB();
	}

	int uid = GetUID();
	m_pService->SetUID(uid);

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
			ProcessMessage();
			break;
			
		case WAIT_TIMEOUT:
			break;

		default:
			break;
		}
	}
	return 1;
}

void DBThread::final()
{
	glog(Log::L_DEBUG, "�߳� DBThread                  ����");

	if (m_hDataComeSem)
	{
		CloseHandle(m_hDataComeSem);
		m_hDataComeSem = NULL;
	}

	// clean insert queue
	while(m_InsertQueue.size() > 0)
	{
		m_InsertQueue.pop();
	}

	// clean update queue
	while(m_UpdateQueue.size() > 0)
	{
		m_UpdateQueue.pop();
	}

	if (m_hStop)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}
}

int DBThread::GetUID()
{
	if (m_bDBExist)
	{
		int uid = m_npvrdb.GetUID();
		glog(Log::L_DEBUG, "�߳� DBThread �����ݿ��� UID <%d>", uid);
		return uid;
	}
	glog(Log::L_DEBUG, "�߳� DBThread �����ݿ��� UID ʧ��, ���� UID Ϊ 0");
	return 0;
}

bool DBThread::setDataCome()
{
	long lPreCount;
	if (!ReleaseSemaphore(m_hDataComeSem, 1, &lPreCount))
	{
		int ret = GetLastError();
		glog(Log::L_ERROR, "ReleaseSemaphore <%d>", ret);
		return false;
	}
	return true;
}

bool DBThread::putInsertMsg(LPSMSMSG pSMS)
{
	if (!m_bDBExist)
	{
		return false;
	}
	
	m_InsertQueue.push(pSMS);
	
	if (!setDataCome())
	{//���ʧ�ܾͰ� DB��־ ��Ϊ��ֵ
		m_bDBExist = false;
		return false;
	}

	return true;
}

bool DBThread::putUpdateMsg(LPSMSMSG pSMS)
{
	if (!m_bDBExist)
	{
		return false;
	}

	int uid = pSMS->GetUID();
	m_UpdateQueue.push(pSMS);

	if (!setDataCome())
	{//���ʧ�ܾͰ� DB��־ ��Ϊ��ֵ
		m_bDBExist = false;
		return false;
	}

	return true;
}

void DBThread::ProcessMessage()
{
//	glog(Log::L_DEBUG, "DBThread::Process Message");
	
	// ����������
	ProcessInsertQueue();

	// ������¶���
	ProcessUpdateQueue();
}

// ����������
void DBThread::ProcessInsertQueue()
{
	if (m_InsertQueue.size() > 0)
	{
		LPSMSMSG pSMS = m_InsertQueue.front();
		m_InsertQueue.pop();
		
		// only if db is connected, do db operation
		if (m_bDBExist)
		{
			// ��ȡSMSMsg������ʣ�����ݵĳ���
			int leftLen = pSMS->GetLeftContentLength();

			char leftCon[1024];
			memset(leftCon, 0x00, 1024*sizeof(char));
			
			// ƴ��ʣ��Ķ�������
			// ����ʣ���������кܶ���'\0'�������������'\0'��Ҫ��һ��'@'���
			Content* p = pSMS->m_first;
			while (p)
			{
				strcat(leftCon, p->GetKey());
				strcat(leftCon, "@");
				strcat(leftCon, p->GetLength());
				strcat(leftCon, "@");
				strcat(leftCon, p->GetValue());
				strcat(leftCon, "@");
				p = p->m_next;
			}

			//glog(Log::L_DEBUG, "Left Content <%s>", leftCon);
			m_criticalSection.Lock(2000);
			if (!m_npvrdb.Insert(pSMS->GetPackageLength(),
								 pSMS->GetCmd(),
								 pSMS->GetUID(),
								 pSMS->GetServiceCode(),
								 pSMS->GetCallNumber(),
								 pSMS->GetSendTime(),
								 pSMS->GetSMSContent(),
								 pSMS->GetSendContent(),
								 pSMS->IsTicpFinished(),
								 pSMS->IsSMSFinished(),
								 leftCon))
			{
				glog(Log::L_ERROR, "insert failed");
			}
			m_criticalSection.Lock(2000);
		}
	}
}

// ������¶���
void DBThread::ProcessUpdateQueue()
{
	if (m_UpdateQueue.size() > 0)
	{
		LPSMSMSG pSMS = m_UpdateQueue.front();
		m_UpdateQueue.pop();
		
		// only if db is connected, do db operation
		if (m_bDBExist)
		{
			if (pSMS != NULL)
			{
				char TicpContent[100];
				memset(TicpContent, 0x00, 100*sizeof(char));
				strcpy(TicpContent, pSMS->GetTicpContent());
				bool TicpFinished = pSMS->IsTicpFinished();
				bool SMSFinished = pSMS->IsSMSFinished();

				m_criticalSection.Lock(2000);
				if (!m_npvrdb.UpdateState(pSMS->GetUID(),
										  TicpFinished,
										  SMSFinished,
										  TicpContent))
				{
					glog(Log::L_ERROR, "update failed");
				}
				else
				{
					// ���³ɹ�
					if (SMSFinished)
					{
						DeleteMessage(pSMS);
					}
				}
				m_criticalSection.Unlock();
			}
		}
	}
}

void DBThread::DeleteMessage(LPSMSMSG pSMS)
{
	glog(Log::L_DEBUG, "���Ų������, �������ڴ���ɾ������ UID �� %d", pSMS->GetUID());
	if (pSMS->GetUID() < 0)
	{
		return;
	}
	try
	{
		if (pSMS)
		{
			delete pSMS;
			pSMS = NULL;
		}
	}
	catch (...)
	{
		glog(Log::L_ERROR, "ɾ������ʱ���ִ��� <%d>", GetLastError());
	}
}

void DBThread::SelectUnfinishedFromDB()
{
	CTime current = CTime::GetCurrentTime();
	current = current - CTimeSpan(0, 0, m_DBOverTime, 0);
	
	char time[20];
	memset(time, 0x00, 20*sizeof(char));
	sprintf(time, "%d-%02d-%02d %02d:%02d:%02d", current.GetYear()
												, current.GetMonth()
												, current.GetDay()
												, current.GetHour()
												, 0, 0);

	WCHAR wcsErr[256];
	memset(wcsErr, 0x00, 256*sizeof(WCHAR));
	if(!m_npvrdb.SelectUnfinished(time, wcsErr))
	{
		return;
	}
	
	int	packageLength;
	int	cmd;
	int	uid;
	char serviceCode[20];
	char callerNumber[20];
	char sendTime[30];
	char SMSContent[100];
	char SendContent[100];
	char leftContent[256];
	char TicpContent[100];
	bool TicpFinished;
	bool SmsFinished;

	memset(serviceCode, 0x00, 20*sizeof(char));
	memset(serviceCode, 0x00, 20*sizeof(char));
	memset(sendTime,	0x00, 30*sizeof(char));
	memset(SMSContent,	0x00, 100*sizeof(char));
	memset(SendContent, 0x00, 100*sizeof(char));
	memset(leftContent, 0x00, 256*sizeof(char));
	memset(TicpContent, 0x00, 100*sizeof(char));

	while (m_npvrdb.getData(packageLength,
							cmd,
							uid,
							serviceCode,
							callerNumber,
							sendTime,
							SMSContent,
							SendContent,
							leftContent,
							TicpContent,
							TicpFinished,
							SmsFinished))
	{
		SMSMsg* pMsg = new SMSMsg(packageLength, cmd, uid);
		pMsg->SetServiceCode(serviceCode);
		pMsg->SetCallNumber(callerNumber);
		pMsg->SetSendTime(sendTime);
		pMsg->AddSMSContent(SMSContent);
		pMsg->AddSendContent(SendContent);
		
		if (strlen(TicpContent) > 0)
		{
			pMsg->AddTicpContent(TicpContent);
		}

		// ����SMS����TICP��ɵ�״̬
		pMsg->TicpFinished(TicpFinished);

		// ����SMS����TM��ɵ�״̬
		pMsg->SMSFinished(SmsFinished);

		// ��Ž������left content
		char parsedContent[1024];
		memset(parsedContent, 0x00, 1024*sizeof(char));
		
		char *p, *q;
		p = strstr(leftContent, "@");
		q = leftContent;

		// db left content ����
		int qLen = 0;

		while(p)
		{
			strncpy(parsedContent + qLen , q, p - q);// ��q��ʼ����(p-q)���ַ�
			qLen = qLen + 20;//����Key�ĳ���
			q = p + 1;//q ָ�� p�ĺ���һ��������'@'
			p = strstr(q, "@");
			
			strncpy(parsedContent + qLen, q, p - q);
			qLen = qLen + 4; //����Length�ĳ���
			q = p + 1;
			p = strstr(q, "@");

			strncpy(parsedContent + qLen, q, p - q);
			qLen = qLen + p - q; //����Value�ĳ���
			q = p + 1;
			p = strstr(q, "@");
		}
		
		pMsg->ParseLeftContentFromDB(parsedContent, qLen);
		pMsg->TraceSMS();

		if (pMsg->IsTicpFinished())
		{// Ticp operation is ok and put it into WriteSocketThread
			WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
			pWriteSocketThd->putContentMsgIntoQueue(pMsg);
		}
		else
		{// Ticp operation is not ok and put it into RawMsgProcessThread

			// TICP����������1���Ժ�0������Ϊ0�Ļ������·���
			pMsg->SetTicpTimes(1);
			
			RawMsgProcessThread* pRawMsgProcessThd = m_pService->GetRawProc();
			pRawMsgProcessThd->putMsgIntoQueue(pMsg);
		}
		
		// clear bufs
		memset(serviceCode, 0x00, 20*sizeof(char));
		memset(serviceCode, 0x00, 20*sizeof(char));
		memset(sendTime,	0x00, 30*sizeof(char));
		memset(SMSContent,	0x00, 100*sizeof(char));
		memset(SendContent, 0x00, 100*sizeof(char));
		memset(leftContent, 0x00, 256*sizeof(char));
		memset(TicpContent, 0x00, 100*sizeof(char));
	}
}

void DBThread::DeleteOverdueMessagesInDB()
{
	CTime current = CTime::GetCurrentTime();
	current = current - CTimeSpan(m_DBClearHours, 0, 0, 0);
	
	char time[20];
	memset(time, 0x00, 20*sizeof(char));
	sprintf(time, "%d-%02d-%02d %02d:%02d:%02d", current.GetYear()
												, current.GetMonth()
												, current.GetDay()
												, current.GetHour()
												, 0, 0);

	if (m_npvrdb.DeleteOverdueMessage(time))
	{
		return;
	}
	glog(Log::L_DEBUG, "û�ж�����Ҫ�����ݿ���ɾ��");
}

bool DBThread::StopThread()
{
	return SetEvent(m_hStop);
}
