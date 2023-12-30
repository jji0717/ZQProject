#include "TICPMsgProcessRequest.h"

TICPMsgProcessRequest::TICPMsgProcessRequest(ZQ::common::NativeThreadPool& Pool, SMSService* pSrv, LPMessage pMsg)
:ThreadRequest(Pool)
{
	m_pSMSMsg = (SMSMsg*)pMsg;
	m_pService = pSrv;

	int ret = 0;

	memset(m_ip, 0x00, 20*sizeof(char));

	m_pService->GetTicpIP(m_ip, m_port);

	m_timeout = m_pService->GetSelectTimeOut();

	m_sequenceID = m_pService->GetSequenceID();
	glog(Log::L_DEBUG, "SequenceID = %d", m_sequenceID);

	// 接收到的短信内容中的开头标志，从主线程去读取
	memset(m_ACFlag, 0x00, 10*sizeof(char));
	strcpy(m_ACFlag, m_pService->GetACFlag());

	memset(m_CTFlag, 0x00, 10*sizeof(char));
	strcpy(m_CTFlag, m_pService->GetCTFlag());
	
	memset(m_RGFlag, 0x00, 10*sizeof(char));
	strcpy(m_RGFlag, m_pService->GetRGFlag());
	
	memset(m_NCFlag, 0x00, 10*sizeof(char));
	strcpy(m_NCFlag, m_pService->GetNCFlag());

	// low case
	_strlwr(m_ACFlag);
	_strlwr(m_CTFlag);
	_strlwr(m_RGFlag);
	_strlwr(m_NCFlag);
	
	// TICP redo max times 
	m_MaxRedoTimes = m_pService->GetTicpRedoTimes();

	// TICP 当前处理过的次数
	m_processTimes = m_pSMSMsg->GetTicpTimes();

	// 从主线程中获得 Response Flag
	m_Response = m_pService->GetResponseFlag();

	// 从主线程中获得 Error Response Flag
	m_ErrorResponse = m_pService->GetErrorResponseFlag();

	m_ticpProc = new TicpProc(m_ip, m_port, m_timeout);

	glog(Log::L_DEBUG, "线程 TICPMsgProcessRequest 新建");
}

TICPMsgProcessRequest::~TICPMsgProcessRequest()
{
}

void TICPMsgProcessRequest::final(int retcode, bool bCancelled)
{	
	glog(Log::L_DEBUG, "线程 TICPMsgProcessRequest 结束");
	
	if (retcode == 1)
	{
		EndRequest();
	}
	else
	{
		this->start();
	}
}

int TICPMsgProcessRequest::run()
{
	glog(Log::L_DEBUG, "线程 TICPMsgProcessRequest 运行 <%d> 次", m_processTimes);
	
	// 没有被处理过一次
	if (m_processTimes == 0)
	{
		//glog(Log::L_DEBUG, "PackageLength <%d>, Cmd <%d>, Uid <%d>", m_pSMSMsg->GetPackageLength(),
		//															m_pSMSMsg->GetCmd(),
		//															m_pSMSMsg->GetUID());
		
		m_pSMSMsg->ParseContent();

		m_pSMSMsg->TraceSMS();

		// 从主线程获取 uid
		int uid = m_pService->GetUID(true);
		
		// 设置 uid
		m_pSMSMsg->SetUID(uid);
		
		// 处理给 TICP 的短信内容
		if(!ProcSendContent())
		{
			m_ret = -1;
			
			// 设置错误的内容
			SetMesssageContent(m_ret);

			// 交给WriteSocketThread去处理
			LastProcessMessage();

			// insert db
			InsertDB();

			return 1;
		}
		
		// insert db
		InsertDB();
	}

	m_processTimes ++;//record times that it has been dealed with

	char realTelephoneNumber[50];
	strnset(realTelephoneNumber, 0, 50);
	GetRealTelephoneNumber(realTelephoneNumber);

	m_ret = m_ticpProc->TicpProcess( m_mode,
									 realTelephoneNumber,
									 m_sequenceID,
									 m_pSMSMsg->GetSendTime(),
									 m_pSMSMsg->GetSendContent());

	if (m_ret >= 0)
	{	// TICP Process success
		
		//m_pSMSMsg->TraceSMS();
		SetMesssageContent(m_ret);

		m_pSMSMsg->TicpFinished(true);
	}
	else
	{	// TICP Process failed

		//m_pSMSMsg->TraceSMS();

		glog(Log::L_DEBUG, "TICP 操作失败");

		if (m_processTimes < m_MaxRedoTimes)
		{
			glog(Log::L_DEBUG, "操作 <%d> 次, 请求没有完成, 重新做一次", m_processTimes);
			return 0;
		}
		SetMesssageContent(0);
	}
	
	LastProcessMessage();
	
	return 1;
}

bool TICPMsgProcessRequest::ProcSendContent()
{
	char tempFlag[10];
	memset(tempFlag, 0x00, 10*sizeof(char));
	
	// get Short Message Content
	char content[160];
	memset(content, 0x00, 160*sizeof(char));
	strcpy(content, m_pSMSMsg->GetSMSContent());
	// low case
	_strlwr(content);
	
	//去掉空格
	int len = 0;
	while(content[len] == 32)
	{
		len ++;
	}

	// get and compare AC flag in Short Message Content
	char* p = strstr((content + len), m_ACFlag);
	char* q = strstr((content + len), m_CTFlag);
	char* r = strstr((content + len), m_RGFlag);
	char* s = strstr((content + len), m_NCFlag);
	
	// AC flag often is at the first position
	if ( p == (content + len) )
	{
		m_mode = PLAY;
		strcpy(tempFlag, m_ACFlag);
		glog(Log::L_DEBUG, "点播标志");
	}
	else if ( q == (content + len) )
	{
		m_mode = CHAT;
		strcpy(tempFlag, m_CTFlag);
		glog(Log::L_DEBUG, "聊天标志");
	}
	else if ( r == (content + len) )
	{
		m_mode = REGISTER;
		strcpy(tempFlag, m_RGFlag);
		glog(Log::L_DEBUG, "注册标志");
	}
	else if ( s == (content + len) )
	{
		m_mode = NICKNAME;
		strcpy(tempFlag, m_NCFlag);
		glog(Log::L_DEBUG, "昵称注册标志");
	}
	else
	{
		glog(Log::L_DEBUG, "没有发现标志，错误的格式 ");
		return false;
	}

	//跳过开头的标志符
	len = len + strlen(tempFlag);
	//去掉标志符后面的空格
	while (content[len] == 32)
	{
		len ++;
	}
	//如果去掉空格后就没有了，就退出
	if (content[len] == '\0')
	{
		glog(Log::L_DEBUG, "标志符后面没有内容");
		return false;
	}

	if (m_mode == PLAY)
	{
		// get content flag to validate, for example the content flag is "play","stop","ff"
		// and get the real content flag and TicpValue
		char contentFlag[7];
		memset(contentFlag, 0x00, 7*sizeof(char));
		char realFlag[7];
		memset(realFlag, 0x00, 7*sizeof(char));
		strncpy(contentFlag, content + len, 6);
		
		int TicpFlag = m_pService->GetTicpFlag(contentFlag, realFlag);
		if (TicpFlag == -1)//点播
		{
			glog(Log::L_DEBUG, "没有发现流控制标志符，所以这是一个点播");

			// add content which sent to Ticp to SMSMsg 
			m_pSMSMsg->AddSendContent(content + len);
		}
		else//流控
		{
			glog(Log::L_DEBUG, "发现流控制标志符<%s>", realFlag);
			// the data from content+len is the data without flag only asset code and delay
			// 跳过操作符
			len = len + strlen(realFlag);

			// create content which send to TICP
			char SendContent[20];
			memset(SendContent, 0x00, 20*sizeof(char));
			sprintf(SendContent, "%d", TicpFlag);

			// add content which sent to Ticp to SMSMsg 
			m_pSMSMsg->AddSendContent(SendContent);

			//glog(Log::L_DEBUG, "Send Content: %s",m_pSMSMsg->GetSendContent());
		}
	}
	else if (m_mode == CHAT || m_mode == REGISTER || m_mode == NICKNAME)
	{
		// add content which sent to Ticp to SMSMsg 
		m_pSMSMsg->AddSendContent(content + len);
	}

	return true;
}

void TICPMsgProcessRequest::SetMesssageContent(int ret)
{
	char returnText[200];
	memset(returnText, 0x00, 200*sizeof(char));

	if (ret == -1)
	{
		strcpy(returnText, "格式错误");
		
	}
	else if (!m_pService->GetReturnCode(ret, returnText))
	{
		return;
	}

	if (m_pService->GetReplyHistoryFlag())
	{
		strcat(returnText, "  <");
		strcat(returnText, m_pSMSMsg->GetSMSContent());
		strcat(returnText, ">");
	}
	m_pSMSMsg->AddTicpContent(returnText);
	glog(Log::L_DEBUG, "Return Text = %s", returnText);
}

void TICPMsgProcessRequest::EndRequest()
{	
	if (m_ticpProc)
	{
		delete m_ticpProc;
	}
	m_ticpProc = NULL;

	delete this;
}

void TICPMsgProcessRequest::LastProcessMessage()
{
	if (m_Response == 0) //无论成功还是失败都不回
	{
		glog(Log::L_DEBUG, "成功或是失败都不回短信");
		m_pSMSMsg->SMSFinished(true);
	}
	else if (m_ErrorResponse == 1 && m_Response == 1) //即不管成功与否都回
	{
		glog(Log::L_DEBUG, "成功失败都回短信");
		
		PutContentIntoWriteThrd();
	}
	else if (m_ErrorResponse == 0 && m_Response == 1) //即成功不发，失败发
	{
		glog(Log::L_DEBUG, "失败才回短信");
		if (m_pService->IsReturnCodeSuccess(m_ret)) // 操作成功
		{
			// 由于成功不发,所以也就不用再等待对方的确认信息,直接设置成完成就可以
			m_pSMSMsg->SMSFinished(true);
		}
		else //操作失败
		{
			//glog(Log::L_DEBUG, "Ticp Operation failed");
			
			PutContentIntoWriteThrd();
		}
	}
	
	// update db TICP finished and TICP content
	UpdateDB();
}

void TICPMsgProcessRequest::PutContentIntoWriteThrd()
{
	WriteSocketThread* pWriteSocketThd = m_pService->GetWriteSocket();
	pWriteSocketThd->putContentMsgIntoQueue(m_pSMSMsg);
}

void TICPMsgProcessRequest::InsertDB()
{
	DBThread* pDB = m_pService->GetDBThread();
	pDB->putInsertMsg(m_pSMSMsg);
}

void TICPMsgProcessRequest::UpdateDB()
{
	DBThread* pDB = m_pService->GetDBThread();
	pDB->putUpdateMsg(m_pSMSMsg);
}

void TICPMsgProcessRequest::GetRealTelephoneNumber(char* realTelephoneNumber)
{
	char telephoneProfix[20];
	strnset(telephoneProfix, 0, 20);
	
	char* telephone = m_pSMSMsg->GetCallNumber();

	if(m_pService->GetTelephoneNumberProfix(m_pSMSMsg->GetServiceCode(), telephoneProfix))
	{
		char* temp = strstr(m_pSMSMsg->GetCallNumber(), telephoneProfix);
		
		if (telephone == temp)
		{
			telephone = telephone + strlen(telephoneProfix);
		}
	}

	strcpy(realTelephoneNumber, telephone);
}