#include "TicpProc.h"

TicpProc::TicpProc(char* ip, int port, DWORD timeout)
{
	memset(m_ip,		  0x00, 20*sizeof(char));
	memset(m_userID,	  0x00, 128*sizeof(char));
	memset(m_sendTime,	  0x00, 20*sizeof(char));
	memset(m_phoneNumber, 0x00, 50*sizeof(char));
	memset(m_content,	  0x00, 1000*sizeof(char));
	
	strcpy(m_ip, ip);
	
	m_port	  = port;
	m_timeout = timeout;

	m_pTicpSocket = new NPVRSocket();
}

TicpProc::~TicpProc()
{
	if(m_pTicpSocket)
	{
		delete m_pTicpSocket;
	}
	m_pTicpSocket = NULL;
}

int TicpProc::TicpProcess(int	mode,
						  char* phoneNumber, 
						  int	sequenceID, 
						  char* sendTime, 
						  char* content)
{
	m_mode = mode;
	m_sequenceID = sequenceID;
	memset(m_sendTime,	  0x00, 20*sizeof(char));
	memset(m_phoneNumber, 0x00, 50*sizeof(char));
	memset(m_content,	  0x00, 1000*sizeof(char));
	strcpy(m_phoneNumber, phoneNumber);
	strcpy(m_sendTime,	  sendTime);
	strcpy(m_content,     content);
	
	if ((m_mode != PLAY) && 
		(m_mode != CHAT) && 
		(m_mode != REGISTER) && 
		(m_mode != NICKNAME))
	{
		return -1;
	}

	if (m_mode == PLAY)
	{
		if (!sendPlay())
		{
			return -2;
		}
	}
	else if (m_mode == CHAT)
	{
		if (!sendChat())
		{
			return -3;
		}
	}
	else if (m_mode == REGISTER)
	{
		if (!sendRegister())
		{
			return -4;
		}
	}
	else if (m_mode == NICKNAME)
	{
		if (!sendNickName())
		{
			return -5;
		}
	}
	return m_ret;
}


bool TicpProc::sendPlay()
{
	char identifyXmlFile[1024];
	char returnXmlFile[1024];
	memset(identifyXmlFile, 0x00, 1024*sizeof(char));
	memset(returnXmlFile,	0x00, 1024*sizeof(char));

	generateIdentifyRequest(identifyXmlFile);
	glog(Log::L_DEBUG, "身份验证的XML : %s" , identifyXmlFile);

	if (!SendAndRecv(identifyXmlFile, returnXmlFile))
	{
		return false;
	}
	glog(Log::L_DEBUG, "身份验证结果的XML : %s" , returnXmlFile);

	m_pXmlProc.CoInit();
	
	if (!ProcessRespXmlfile(returnXmlFile))
	{
		return false;
	}
	
	if (m_ret != 2)
	{
		return true;
	}
	
	char playXmlFile[1024];
	memset(playXmlFile,   0x00, 1024*sizeof(char));

	generatePlayRequest(playXmlFile);
	glog(Log::L_DEBUG, "点播的XML : %s" , playXmlFile);
	
	memset(returnXmlFile, 0x00, 1024*sizeof(char));
	if (!SendAndRecv(playXmlFile, returnXmlFile))
	{
		return false;
	}
	glog(Log::L_DEBUG, "点播结果的XML : %s" , returnXmlFile);

	if (!ProcessRespXmlfile(returnXmlFile))
	{
		return false;
	}

	m_pXmlProc.CoUnInit();

	return true;
}

void TicpProc::generateIdentifyRequest(char* identifyXmlFile)
{
	char timeChar[20];
	memset(timeChar, 0x00, 20*sizeof(char));
	CTime currentTime = CTime::GetCurrentTime();
	sprintf(timeChar, "%d-%d-%d %d:%d:%d", currentTime.GetYear(), 
										   currentTime.GetMonth(),
										   currentTime.GetDay(), 
										   currentTime.GetHour(),
										   currentTime.GetMinute(),
										   currentTime.GetSecond());

	
	sprintf(identifyXmlFile, "<Message><Head source=\"SMS\" time=\"%s\" sequence=\"%d\"/>"
							 "<Action code=\"1\" name=\"callerIdentify \" type=\"request\"/>"
							 "<Parameter sequence=\"1\" name=\"callerId\">%s</Parameter></Message>",
							 timeChar, 
							 m_sequenceID, 
							 m_phoneNumber);
}

void TicpProc::generatePlayRequest(char* playXmlFile)
{
	char timeChar[20];
	memset(timeChar, 0x00, 20*sizeof(char));
	CTime currentTime = CTime::GetCurrentTime();
	sprintf(timeChar, "%d-%d-%d %d:%d:%d", currentTime.GetYear(), 
										   currentTime.GetMonth(),
										   currentTime.GetDay(), 
										   currentTime.GetHour(),
										   currentTime.GetMinute(),
										   currentTime.GetSecond());
	
	m_sequenceID ++;
	
	sprintf(playXmlFile, "<Message><Head source=\"SMS\" time=\"%s\" sequence=\"%d\" sendTime=\"%s\"/>"
						 "<Action code=\"7\" name=\"playAsset\" type=\"request\"/>"
						 "<Parameter sequence=\"1\" name=\"userId\">%s</Parameter>"
						 "<Parameter sequence=\"2\" name=\"OperationCode\">%s</Parameter>"
						 "</Message>", 
						 timeChar, 
						 m_sequenceID, 
						 m_sendTime, 
						 m_userID, 
						 m_content);
}

bool TicpProc::sendChat()
{
	char chatXmlFile[1024];
	char returnXmlFile[1024];
	memset(chatXmlFile,	  0x00, 1024*sizeof(char));
	memset(returnXmlFile, 0x00, 1024*sizeof(char));
	
	generateChatRequest(chatXmlFile);
	glog(Log::L_DEBUG, "聊天的XML : %s" , chatXmlFile);

	if (!SendAndRecv(chatXmlFile, returnXmlFile))
	{
		return false;
	}
	glog(Log::L_DEBUG, "聊天结果的XML : %s" , returnXmlFile);

	m_pXmlProc.CoInit();

	if (!ProcessRespXmlfile(returnXmlFile))
	{
		return false;
	}
	
	m_pXmlProc.CoUnInit();
	return true;
}

void TicpProc::generateChatRequest(char* chatXmlFile)
{
	char timeChar[20];
	memset(timeChar, 0x00, 20*sizeof(char));
	CTime currentTime = CTime::GetCurrentTime();
	sprintf(timeChar, "%d-%d-%d %d:%d:%d", currentTime.GetYear(), 
										   currentTime.GetMonth(),
										   currentTime.GetDay(), 
										   currentTime.GetHour(),
										   currentTime.GetMinute(),
										   currentTime.GetSecond());
	

	sprintf(chatXmlFile, "<Message><Head source=\"SMS\" time=\"%s\" sequence=\"%d\" sendTime=\"%s\"/>"
						 "<Action code=\"11\" name=\"playAsset\" type=\"request\"/>"
						 "<Parameter sequence=\"1\" name=\"userId\">%s</Parameter>"
						 "<Parameter sequence=\"2\" name=\"MessageContent\">%s</Parameter>"
						 "<Parameter sequence=\"3\" name=\"PhoneNumber\">%s</Parameter>"
						 "</Message>", 
						 timeChar, 
						 m_sequenceID, 
						 m_sendTime, 
						 m_phoneNumber,
						 m_content,
						 m_phoneNumber);
}

bool TicpProc::sendRegister()
{
	char RegXmlFile[1024];
	char returnXmlFile[1024];
	memset(RegXmlFile,    0x00, 1024*sizeof(char));
	memset(returnXmlFile, 0x00, 1024*sizeof(char));

	generateRegisterRequest(RegXmlFile);
	glog(Log::L_DEBUG, "注册的XML : %s" , RegXmlFile);

	if (!SendAndRecv(RegXmlFile, returnXmlFile))
	{
		return false;
	}	
	glog(Log::L_DEBUG, "注册结果的XML : %s" , returnXmlFile);

	m_pXmlProc.CoInit();

	if (!ProcessRespXmlfile(returnXmlFile))
	{
		return false;
	}
	
	m_pXmlProc.CoUnInit();
	return true;
}

void TicpProc::generateRegisterRequest(char* regXmlFile)
{
	char timeChar[20];
	memset(timeChar, 0x00, 20*sizeof(char));
	CTime currentTime = CTime::GetCurrentTime();
	sprintf(timeChar, "%d-%d-%d %d:%d:%d", currentTime.GetYear(), 
										   currentTime.GetMonth(),
										   currentTime.GetDay(), 
										   currentTime.GetHour(),
										   currentTime.GetMinute(),
										   currentTime.GetSecond());


	sprintf(regXmlFile,  "<Message><Head source=\"SMS\" time=\"%s\" sequence=\"%d\" sendTime=\"%s\"/>"
						 "<Action code=\"15\" name=\"Register\" type=\"request\"/>"
						 "<Parameter sequence=\"1\" name=\"RegisterCode\">%s</Parameter>"
						 "</Message>", 
						 timeChar, 
						 m_sequenceID, 
						 m_sendTime, 
						 m_content);
}

bool TicpProc::sendNickName()
{
	char identifyXmlFile[1024];
	char returnXmlFile[1024];
	memset(identifyXmlFile, 0x00, 1024*sizeof(char));
	memset(returnXmlFile,	0x00, 1024*sizeof(char));

	generateIdentifyRequest(identifyXmlFile);
	glog(Log::L_DEBUG, "身份验证的XML : %s" , identifyXmlFile);

	if (!SendAndRecv(identifyXmlFile, returnXmlFile))
	{
		return false;
	}
	glog(Log::L_DEBUG, "身份验证结果的XML : %s" , returnXmlFile);

	m_pXmlProc.CoInit();
	
	if (!ProcessRespXmlfile(returnXmlFile))
	{
		return false;
	}
	
	if (m_ret != 2)
	{
		return true;
	}

	char nickNameXmlFile[1024];
	memset(nickNameXmlFile, 0x00, 1024*sizeof(char));
	generateNickNameRequest(nickNameXmlFile);
	glog(Log::L_DEBUG, "昵称的XML : %s" , nickNameXmlFile);

	memset(returnXmlFile, 0x00, 1024*sizeof(char));
	
	if (!SendAndRecv(nickNameXmlFile, returnXmlFile))
	{
		return false;
	}
	glog(Log::L_DEBUG, "昵称结果的XML : %s" , returnXmlFile);

	if (!ProcessRespXmlfile(returnXmlFile))
	{
		return false;
	}

	m_pXmlProc.CoUnInit();


	return true;
}

void TicpProc::generateNickNameRequest(char* nickNameXmlFile)
{
	char timeChar[20];
	memset(timeChar, 0x00, 20*sizeof(char));
	CTime currentTime = CTime::GetCurrentTime();
	sprintf(timeChar, "%d-%d-%d %d:%d:%d", currentTime.GetYear(), 
										   currentTime.GetMonth(),
										   currentTime.GetDay(), 
										   currentTime.GetHour(),
										   currentTime.GetMinute(),
										   currentTime.GetSecond());

	m_sequenceID ++;
	
	sprintf(nickNameXmlFile, "<Message><Head source=\"SMS\" time=\"%s\" sequence=\"%d\" sendTime=\"%s\"/>"
							 "<Action code=\"17\" name=\"RegisterNickName\" type=\"request\"/>"
							 "<Parameter sequence=\"1\" name=\"userId\">%s</Parameter>"
							 "<Parameter sequence=\"2\" name=\"NickName\">%s</Parameter>"
							 "<Parameter sequence=\"3\" name=\"RegisterOrUnregister\">1</Parameter>"
							 "</Message>",
							 timeChar,
							 m_sequenceID,
							 m_sendTime, 
							 m_userID, 
							 m_content);

	
}

bool TicpProc::ProcessRespXmlfile(char* Xmlfile)
{
	char sequenceId[10];
	char actionCode[10];
	char parameter1[30];
	char parameter2[30];
	memset(sequenceId, 0x00, 10*sizeof(char));
	memset(actionCode, 0x00, 10*sizeof(char));
	memset(parameter1, 0x00, 30*sizeof(char));
	memset(parameter2, 0x00, 30*sizeof(char));
	
	Xmlfile = Xmlfile +10;

	m_pXmlProc.XmlProc(Xmlfile, sequenceId, actionCode, parameter1, parameter2);

	if (m_sequenceID != atoi(sequenceId))
	{
		glog(Log::L_DEBUG, "ID: %d , 错误的Sequence ID" , m_sequenceID);
		return false;
	}
	
	m_ret = atoi(parameter1);
	if (m_ret == 2)
	{
		strcpy (m_userID, parameter2);
	}
	else if (m_ret < 0)
	{
		return false;
	}

	return true;
}


bool TicpProc::SendAndRecv(char* xmlfile, char* returnXmlfile)
{
	if (!m_pTicpSocket->InitialSocket())
	{
		m_pTicpSocket->CloseSocketC();
		return false;
	}

	// connect
	if (!m_pTicpSocket->Connect(m_port, m_ip))
	{
		m_pTicpSocket->CloseSocketC();
		return false;
	}
	
	// send
	wchar_t wcs[1034];
	memset(wcs, 0x00, 1034*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, xmlfile, -1, wcs, strlen(xmlfile) + 1);

	int length = wcslen(wcs);

	char temp[1024];
	memset(temp, 0x00, 1024*sizeof(char));
	sprintf(temp, "%010d%s", length, xmlfile);
	glog(Log::L_DEBUG, "发送的XML: %s" , temp);
	
	if (!m_pTicpSocket->SendC( temp, strlen(temp) ) )
	{
		m_pTicpSocket->CloseSocketC();
		return false;
	}
	
	// select
	if ( !m_pTicpSocket->SelectC(m_timeout) )
	{
		m_pTicpSocket->CloseSocketC();
		return false;
	}
	
	// recv
	if ( !m_pTicpSocket->RecvC(returnXmlfile, 1023))
	{
		m_pTicpSocket->CloseSocketC();
		return false;
	}
	
	m_pTicpSocket->CloseSocketC();
	
	return true;
}