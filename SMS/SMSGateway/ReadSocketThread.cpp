#include "ReadSocketThread.h"
#include "Log.h"
#include "ScLog.h"

#define CONTENT_LEN 3000

ReadSocketThread::ReadSocketThread(SMSService* pSrv):
m_pNSocket(NULL)
{
	m_pService = pSrv;
}

ReadSocketThread::~ReadSocketThread()
{
	glog(Log::L_DEBUG, "�߳� ReadSocketThread     ��������");
}

bool ReadSocketThread::init()
{
	glog(Log::L_DEBUG, "�߳� ReadSocketThread     ��ʼ��");

	// ��ʼ���������͵ȴ�ʱ��Ϊ0
	clearHeartBeatTime();
	
	clearHeartBeatCount();

	clearLostHeartBeatCount();	
	
	m_hStop = CreateEvent(NULL, false, false, NULL);
	if (m_hStop == NULL)
	{
		return false;
	} 
	
	return true;
}

int ReadSocketThread::run()
{
	glog(Log::L_DEBUG, "�߳� ReadSocketThread     ����");
	
	// ��ʼ��socket
	m_pNSocket = new NPVRSocket();
	if (!m_pNSocket->InitialSocket())
	{
		glog(Log::L_ERROR, "NPVRSocket initial failed");
		return 0;
	}

	strcpy(m_ServiceID, m_pService->GetServiceID());
	
	strcpy(m_ip, m_pService->GetTMIP());

	m_port = m_pService->GetTMPort();

	m_hWSAEvent = WSACreateEvent();
	
	if (!m_pNSocket->WSASelectC(m_hWSAEvent, FD_CLOSE | FD_CONNECT | FD_READ))
	{
		return false;
	}

	m_pNSocket->Connect(m_port, m_ip);

	WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();

	// ���ܻ�����
	char content[CONTENT_LEN];
	strnset(content, 0, CONTENT_LEN);
	
	HANDLE handles[2];
	handles[0] = m_hStop;
	handles[1] = m_hWSAEvent;
	
	DWORD dwWaitStatus = 0;
	bool bContinued = true;

	m_WaitForTmTime = m_pService->GetWaitTime();

	while (bContinued)
	{
		dwWaitStatus = WSAWaitForMultipleEvents(2, handles, false, m_WaitForTmTime, false);

		switch (dwWaitStatus)
		{
		case WAIT_OBJECT_0:
			bContinued = false;
			break;
		
		case WAIT_OBJECT_0 + 1:
			{
				WSAResetEvent(m_hWSAEvent);//set non-signaled state

				int ret = m_pNSocket->GetNetworkEventC(m_hWSAEvent);
				
				if ((ret != FD_READ) && (ret != OPER_SUCCESS))
				{
					glog(Log::L_DEBUG, "WSAEnumNetworkEvents = %d", ret);
				}
				
				if ((ret == ERROR_CONNECT) || (ret == SOCKET_ERROR)  ||
					(ret == ERROR_CLOSE)   || (ret == FD_CLOSE))
				{
					// ��������
					ReConnect();
					break;
				}

				else if (ret == FD_CONNECT)
				{
					// ����WriteSocketThread �������ӱ�־
					pWriteSocketThd->Connected(true);
					glog(Log::L_DEBUG, "Socket Connected");
					
					// Ȼ���������֤��Ϣ
					sendLoginMsg();
					
					break;
				}
				
				// û�����ӾͲ���
				if (!pWriteSocketThd->IsConnected())
				{
					glog(Log::L_DEBUG, "GetNetworkEventC = %d <%d>", ret, GetLastError());
					m_pService->OnStop();
				}

				// ��ȡ������Ϣ�ĳ���ֵ
				char buf[5];
				memset(buf, 0x00, 5*sizeof(char));
				if (!m_pNSocket->RecvC(buf, 4))
				{
					break;
				}

				// ������������ŵĳ���
				int netPackageLength;
				memcpy(&netPackageLength, buf, 4);
				int packageLength = ntohl(netPackageLength);
				if (packageLength < MESSAGE_HEAD_LENGTH)
				{
					glog(Log::L_ERROR, "packageLength <%d> can't low than 12", packageLength);
					break;
				}
				
				// ���ݳ��ȣ���4����ȡ��������				
				if (!m_pNSocket->RecvC(content, packageLength - 4) )
				{
					break;
				}
				
				int netCmd;
				memcpy(&netCmd, content, 4);
				int cmd = ntohl(netCmd);

				int netUid;
				memcpy(&netUid, content + 4, 4);
				int uid = ntohl(netUid);
				
				Classify(packageLength, cmd, uid, content);

				// ��������ȴ�ʱ��
				clearHeartBeatTime();

				// ��������Ķ�ʧ����
				clearLostHeartBeatCount();

				// ��ջ�����
				strnset(content, 0, CONTENT_LEN);
			}
			break;
			
		case WAIT_TIMEOUT:
			// ����� TIMEOUT ��û�����Ӿ�����
			if (!pWriteSocketThd->IsConnected())
			{
				ReConnect();
			}
			else
			{
				SendHeatbeat();
			}
			break;
		default:
			break;
		}
	}
	
	if (m_hWSAEvent)
	{
		WSACloseEvent(m_hWSAEvent);
	}

	return 1;
}

void ReadSocketThread::final()
{
	if (m_pNSocket)
	{
		delete m_pNSocket;
		m_pNSocket = NULL;
	}
	
	if (m_hStop)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}
	
	glog(Log::L_DEBUG, "�߳� ReadSocketThread          ����");
}


bool ReadSocketThread::ParseLoginResponse(LPMessage pMsg)
{
	LPSMSMSG pSmsg = (LPSMSMSG)pMsg;
	pSmsg->ParseContent();

	int loginState = pSmsg->GetLoginState();

	if (loginState == 0)
	{
		return true;
	}
	return false;
}

void ReadSocketThread::ReConnect()
{
	glog(Log::L_DEBUG, "Sleep %d Seconds", SLEEP_TIME/1000);
	
	Sleep(SLEEP_TIME);
	
	glog(Log::L_DEBUG, "��������");

	WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
	pWriteSocketThd->Connected(false);
	pWriteSocketThd->Login(false);

	// �ر�socket
	m_pNSocket->CloseSocketC();
	
	// �ر��¼�
	WSACloseEvent(m_hWSAEvent);
	
	// ���������¼�
	m_hWSAEvent = WSACreateEvent();

	// ��������
	if (!m_pNSocket->WSASelectC(m_hWSAEvent, FD_CLOSE | FD_CONNECT | FD_READ))
	{
		return;
	}
	
	// ��������
	m_pNSocket->Connect(m_port, m_ip);
}

void ReadSocketThread::Classify(int packageLength, int cmd, int uid, char* content)
{
	RawMsgProcessThread* pRawMsgThd = m_pService->GetRawProc();

	// content message
	if (cmd == CMD_MSG)
	{		
		glog(Log::L_DEBUG, "���ݶ���, UID �� %d", uid);
		
		SMSMsg* pSMSMsg = new SMSMsg(packageLength, cmd, uid);
		pSMSMsg->AddContent(content + 8);

		// ���� RawMsgProcessThread ����
		pRawMsgThd->putMsgIntoQueue(pSMSMsg);
	}

	// message response
	else if (cmd == CMD_MSG_RESP)
	{
		glog(Log::L_DEBUG, "���ݶ��Żظ�, UID �� %d", uid);

		Message* pMsg = new Message(packageLength, cmd, uid);
		
		// ���� RawMsgProcessThread ����
		pRawMsgThd->putMsgIntoQueue(pMsg);
	}

	// login response
	else if (cmd == CMD_LOGIN_RESP)
	{
		glog(Log::L_DEBUG, "��½�� �ظ� , UID �� %d", uid);
		
		SMSMsg* pSMSMsg = new SMSMsg(packageLength, cmd, uid);
		pSMSMsg->AddContent(content + 8);
		
		if (ParseLoginResponse(pSMSMsg))
		{
			WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
			pWriteSocketThd->Login(true);
			glog(Log::L_DEBUG, "��½�ɹ�");
		}
		else
		{
			// ��½ʧ�ܣ��ط���½����
			glog(Log::L_DEBUG, "��½ʧ��");
			sendLoginMsg();
		}

		delete pSMSMsg;
	}

	// heartbeat response
	else if (cmd == CMD_HEARTBEAT_RESP)
	{	
		DWORD maxHeartbeatLogWindows = m_pService->GetHeartbeatLogWindows();
		if (m_HeartbeatCount % maxHeartbeatLogWindows == 0)
		{
			glog(Log::L_DEBUG, "�����ظ�����, UID = %d", uid);
			clearHeartBeatCount();
		}
		
		DWORD waitTime = GetTickCount() - m_HeartbeatWaitTime;
		if ((m_HeartbeatWaitTime > 0) && (waitTime > m_WaitForTmTime))
		{
			glog(Log::L_DEBUG, "*******     �������ʱ����  %d   ********", waitTime);
		}
	}

	else 
	{
		glog(Log::L_DEBUG, "δ֪����, ������ <%d>, ������ <%d>, UID�� <%d>, ���������� <%s>", 
			 packageLength, cmd, uid, content);
	}
}

bool ReadSocketThread::sendLoginMsg()
{
	int cmd = CMD_LOGIN;
	int uid = m_pService->GetUID();
	
	glog(Log::L_DEBUG, "���� ��½�� , UID = %d", uid);
	
	char msg[110];
	memset(msg, 0x00, 110*sizeof(char));

	// ������һ��SMS message ����
	SMSMsg* pSmg = new SMSMsg(12, cmd, uid);

	char srvIdLen[5];
	memset(srvIdLen, 0x00, 5*sizeof(char));
	sprintf(srvIdLen, "%d", strlen(m_ServiceID));

	pSmg->AddContent("ServerID", srvIdLen, m_ServiceID);
	pSmg->AddContent("Password", "12", "DigitTV_Pass");
	pSmg->AddContent("UserName", "7", "DigitTV");
	
	pSmg->ParseContent();

	int packagelength = pSmg->GetPackageLength();

//	pSmg->TraceSMS();
	int temp = pSmg->CombineContentMessage();

	if (temp != packagelength)
	{
		return false;
	}

	memcpy(msg, pSmg->GetContent(), packagelength);

	if(!m_pNSocket->SendC(msg, packagelength))
	{
		return false;
	}

	return true;
}

void ReadSocketThread::SendHeatbeat()
{
	WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
	if (!pWriteSocketThd->IsConnected())
	{
		return;
	}
	
	// m_HeartbeatWaitTime ������0 ��ָ�ϴ��и�heartbeatû���յ��ظ�
	if (m_HeartbeatWaitTime != 0)
	{
		DWORD waitTime = GetTickCount() - m_HeartbeatWaitTime;
		if ( waitTime > m_WaitForTmTime)
		{
			glog(Log::L_DEBUG, "*******     �������ʱ����  %d   ********", waitTime);
			
			m_LostHeartbeatCount ++;
			
			DWORD maxLostHeartbeatCount = m_pService->GetLostHeartbeatMaxCount();
			if (m_LostHeartbeatCount >= maxLostHeartbeatCount)
			{
				glog(Log::L_DEBUG, "������ʧ�����ﵽ %d, ��������", m_LostHeartbeatCount);
				
				// һ���������������ʧ�������������ӣ�
				// ��������ǰ�� �����ȴ�ʱ��, �������� �� ���������ʧ���� ����
				clearHeartBeatTime();
				clearLostHeartBeatCount();

				ReConnect();
				
				return;
			}
		}
	}

	int packageLength = htonl(12);
	int cmd = htonl(CMD_HEARTBEAT);
	m_heatbeatUID = m_pService->GetUID();
	
	// �����������ﵽһ����������log�м�¼
	m_HeartbeatCount ++;
	DWORD maxHeartbeatLogWindows = m_pService->GetHeartbeatLogWindows();
	if (m_HeartbeatCount % maxHeartbeatLogWindows == 0)
	{
		glog(Log::L_DEBUG, "���� ������ , UID = %d", m_heatbeatUID);
	}
	
	int heartbeatUID = htonl(m_heatbeatUID);
	
	char msg[13];
	memset(msg, 0x00, 13*sizeof(char));
	memcpy(msg, &packageLength, 4);
	memcpy(msg + 4, &cmd, 4);
	memcpy(msg + 8, &heartbeatUID, 4);
	
	// ������һ�� heartbeat �ͼ�¼�·��͵�ʱ��
	if (m_HeartbeatWaitTime == 0)
	{
		m_HeartbeatWaitTime = GetTickCount();
	}

	if (!m_pNSocket->SendC(msg, 12))
	{
		ReConnect();
	}
}

bool ReadSocketThread::StopThread()
{
	return SetEvent(m_hStop);
}


