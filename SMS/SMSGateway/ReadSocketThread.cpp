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
	glog(Log::L_DEBUG, "线程 ReadSocketThread     析构函数");
}

bool ReadSocketThread::init()
{
	glog(Log::L_DEBUG, "线程 ReadSocketThread     初始化");

	// 初始化心跳发送等待时间为0
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
	glog(Log::L_DEBUG, "线程 ReadSocketThread     运行");
	
	// 初始化socket
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

	// 接受缓冲区
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
					// 重新连接
					ReConnect();
					break;
				}

				else if (ret == FD_CONNECT)
				{
					// 激活WriteSocketThread 的已连接标志
					pWriteSocketThd->Connected(true);
					glog(Log::L_DEBUG, "Socket Connected");
					
					// 然后发送身份验证信息
					sendLoginMsg();
					
					break;
				}
				
				// 没有连接就不读
				if (!pWriteSocketThd->IsConnected())
				{
					glog(Log::L_DEBUG, "GetNetworkEventC = %d <%d>", ret, GetLastError());
					m_pService->OnStop();
				}

				// 读取整条信息的长度值
				char buf[5];
				memset(buf, 0x00, 5*sizeof(char));
				if (!m_pNSocket->RecvC(buf, 4))
				{
					break;
				}

				// 计算出整条短信的长度
				int netPackageLength;
				memcpy(&netPackageLength, buf, 4);
				int packageLength = ntohl(netPackageLength);
				if (packageLength < MESSAGE_HEAD_LENGTH)
				{
					glog(Log::L_ERROR, "packageLength <%d> can't low than 12", packageLength);
					break;
				}
				
				// 根据长度（减4）读取整条短信				
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

				// 清空心跳等待时间
				clearHeartBeatTime();

				// 清空心跳的丢失个数
				clearLostHeartBeatCount();

				// 清空缓冲区
				strnset(content, 0, CONTENT_LEN);
			}
			break;
			
		case WAIT_TIMEOUT:
			// 如果在 TIMEOUT 中没有连接就重连
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
	
	glog(Log::L_DEBUG, "线程 ReadSocketThread          结束");
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
	
	glog(Log::L_DEBUG, "网络重连");

	WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
	pWriteSocketThd->Connected(false);
	pWriteSocketThd->Login(false);

	// 关闭socket
	m_pNSocket->CloseSocketC();
	
	// 关闭事件
	WSACloseEvent(m_hWSAEvent);
	
	// 重新生成事件
	m_hWSAEvent = WSACreateEvent();

	// 重新设置
	if (!m_pNSocket->WSASelectC(m_hWSAEvent, FD_CLOSE | FD_CONNECT | FD_READ))
	{
		return;
	}
	
	// 重新连接
	m_pNSocket->Connect(m_port, m_ip);
}

void ReadSocketThread::Classify(int packageLength, int cmd, int uid, char* content)
{
	RawMsgProcessThread* pRawMsgThd = m_pService->GetRawProc();

	// content message
	if (cmd == CMD_MSG)
	{		
		glog(Log::L_DEBUG, "内容短信, UID 是 %d", uid);
		
		SMSMsg* pSMSMsg = new SMSMsg(packageLength, cmd, uid);
		pSMSMsg->AddContent(content + 8);

		// 交由 RawMsgProcessThread 处理
		pRawMsgThd->putMsgIntoQueue(pSMSMsg);
	}

	// message response
	else if (cmd == CMD_MSG_RESP)
	{
		glog(Log::L_DEBUG, "内容短信回复, UID 是 %d", uid);

		Message* pMsg = new Message(packageLength, cmd, uid);
		
		// 交由 RawMsgProcessThread 处理
		pRawMsgThd->putMsgIntoQueue(pMsg);
	}

	// login response
	else if (cmd == CMD_LOGIN_RESP)
	{
		glog(Log::L_DEBUG, "登陆包 回复 , UID 是 %d", uid);
		
		SMSMsg* pSMSMsg = new SMSMsg(packageLength, cmd, uid);
		pSMSMsg->AddContent(content + 8);
		
		if (ParseLoginResponse(pSMSMsg))
		{
			WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
			pWriteSocketThd->Login(true);
			glog(Log::L_DEBUG, "登陆成功");
		}
		else
		{
			// 登陆失败，重发登陆命令
			glog(Log::L_DEBUG, "登陆失败");
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
			glog(Log::L_DEBUG, "心跳回复短信, UID = %d", uid);
			clearHeartBeatCount();
		}
		
		DWORD waitTime = GetTickCount() - m_HeartbeatWaitTime;
		if ((m_HeartbeatWaitTime > 0) && (waitTime > m_WaitForTmTime))
		{
			glog(Log::L_DEBUG, "*******     心跳间隔时间是  %d   ********", waitTime);
		}
	}

	else 
	{
		glog(Log::L_DEBUG, "未知短信, 长度是 <%d>, 命令是 <%d>, UID是 <%d>, 短信内容是 <%s>", 
			 packageLength, cmd, uid, content);
	}
}

bool ReadSocketThread::sendLoginMsg()
{
	int cmd = CMD_LOGIN;
	int uid = m_pService->GetUID();
	
	glog(Log::L_DEBUG, "发送 登陆包 , UID = %d", uid);
	
	char msg[110];
	memset(msg, 0x00, 110*sizeof(char));

	// 先生成一个SMS message 对象
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
	
	// m_HeartbeatWaitTime 不等于0 是指上次有个heartbeat没有收到回复
	if (m_HeartbeatWaitTime != 0)
	{
		DWORD waitTime = GetTickCount() - m_HeartbeatWaitTime;
		if ( waitTime > m_WaitForTmTime)
		{
			glog(Log::L_DEBUG, "*******     心跳间隔时间是  %d   ********", waitTime);
			
			m_LostHeartbeatCount ++;
			
			DWORD maxLostHeartbeatCount = m_pService->GetLostHeartbeatMaxCount();
			if (m_LostHeartbeatCount >= maxLostHeartbeatCount)
			{
				glog(Log::L_DEBUG, "心跳丢失个数达到 %d, 重新连接", m_LostHeartbeatCount);
				
				// 一旦超过心跳的最大丢失个数就重新连接，
				// 重新连接前把 心跳等待时间, 心跳个数 和 最大心跳丢失个数 清零
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
	
	// 当心跳个数达到一个数量才在log中记录
	m_HeartbeatCount ++;
	DWORD maxHeartbeatLogWindows = m_pService->GetHeartbeatLogWindows();
	if (m_HeartbeatCount % maxHeartbeatLogWindows == 0)
	{
		glog(Log::L_DEBUG, "发送 心跳包 , UID = %d", m_heatbeatUID);
	}
	
	int heartbeatUID = htonl(m_heatbeatUID);
	
	char msg[13];
	memset(msg, 0x00, 13*sizeof(char));
	memcpy(msg, &packageLength, 4);
	memcpy(msg + 4, &cmd, 4);
	memcpy(msg + 8, &heartbeatUID, 4);
	
	// 发送了一个 heartbeat 就记录下发送的时间
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


