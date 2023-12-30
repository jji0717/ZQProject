#include "JmsProcThread.h"

/*********       ±‰¡ø       *********/
extern ManualSync_ProcData_Callback    m_ProcDataCallback;
extern CRITICAL_SECTION                m_Section;
extern bool                            m_bJmsStatus;
extern HANDLE                          m_hJmsStatus;

BOOL WINAPI HandlerRoutine(  DWORD dwCtrlType)
{
	switch( dwCtrlType ) 
	{
	case CTRL_LOGOFF_EVENT: // for LOGOFF event , do nothing but return true to notify system do not process this event any longer
		return TRUE;
		break;
	default://we only care about LOGOFF event, so for other event just ignore and return false
		return FALSE;
		break;
	}
	return true;
}

JmsProcThread::JmsProcThread(ZQ::common::Log * pLog)
{
	m_Log = pLog;
	m_bCreateProducer = false;
	m_bInitialize     = false;
	m_pJmsContext     = NULL;
	m_JmsCNFactory    = NULL;
	m_JmsConnection   = NULL;
	m_JmsSession      = NULL;
	m_hConBroken      = NULL;
	m_bConBroken      = false;
	m_bContinued      = false;
}

JmsProcThread::~JmsProcThread()
{
}

bool JmsProcThread::init()
{
	(*m_Log)(ZQ::common::Log::L_INFO, L"JmsProcThread::init()");

	GetConfiguration();
	
	m_hStop = CreateEvent(NULL, false, false, NULL);	
	if (m_hStop == NULL)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Stop Event failed, Error is %d", GetLastError());
	}

	m_hConStart = CreateEvent(NULL, false, false, NULL);
	if (m_hConStart == NULL)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Connection Start Event failed, Error is %d", GetLastError());
	}

	m_hConBroken = CreateEvent(NULL, false, false, NULL);
	if (m_hConBroken == NULL)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Connection Broken Event failed, Error is %d", GetLastError());
	}

	if(!InitJMS())
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "InitJMS failed");
		m_bJmsStatus = false;
		SetEvent(m_hJmsStatus);
		return false;
	}
	m_bJmsStatus = true;
	SetEvent(m_hJmsStatus);
	
	return true;
}

int JmsProcThread::run()
{
	DWORD ret = 0;
	ret = WaitForSingleObject(m_hConStart, INFINITE);
	if (ret != WAIT_OBJECT_0)
	{
		return 0;
	}

	if (!StartJMS())
	{
		return 0;
	}
	
	if (!SetMessageProperty())
	{
		return 0;
	}

	(*m_Log)(ZQ::common::Log::L_INFO, L"JmsProcThread::run()");

	m_bContinued = true;

	char buf[BUFSIZ];

	HANDLE handles[2] = {m_hStop, m_hConBroken};
	DWORD timeout = WAIT_TIME;

	while (m_bContinued)
	{
		DWORD status = WaitForMultipleObjects(2, handles, false, timeout);
		switch(status) 
		{
		case WAIT_OBJECT_0:
			m_bContinued = false;
			break;

		case WAIT_OBJECT_0 + 1:
			Sleep(3000);

			UnInitJms();
			
			if (!InitJMS())
			{
				timeout += WAIT_TIME;
				Sleep(2000);
				SetConBroken();
				break;
			}
			
			if (!StartJMS())
			{
				timeout += WAIT_TIME;
				Sleep(2000);
				SetConBroken();
				break;
			}
			
			SetMessageProperty();
			timeout = WAIT_TIME;
			m_bConBroken = false;
			break;

		case WAIT_TIMEOUT:
			{
				m_JmsConsumer.receive(100, m_JmsTxtMessage);
				if (!m_bConBroken && (NULL == m_JmsTxtMessage._message))
				{
					continue;
				}

				strnset(buf, 0, BUFSIZ);
				m_JmsTxtMessage.getText(buf, BUFSIZ);
				_LAM_to_DBSync ltd;
				
				(*m_Log)(ZQ::common::Log::L_INFO, 
						 "Receive Buf is <%s>", buf);
				
				if (!ParseXMLFile(buf, &ltd))
				{
					(*m_Log)(ZQ::common::Log::L_ERROR,
							 "ParseXMLFile failed (%s)", 
							 buf);
					strnset(buf, 0, BUFSIZ);		
					continue;
				}

				if (!m_bCreateProducer)
				{
					SetReplyAndProducer();
				}

				SendStartXMLFile(&ltd);

				_DBSync_to_LAM dtl;
				dtl._SynComID = ltd._SynComID;
				dtl._SyncType = ltd._SyncType;
				wcsncpy(dtl._SyncPath, ltd._SyncPath, MAX_PATH - 1);
				wcsnset(dtl._errorCode, 0, MAX_PATH);
				wcsnset(dtl._errorDescription, 0, MAX_PATH);

				m_ProcDataCallback(&ltd, &dtl);

				SendCompleteXMLFile(&dtl);
			}
			break;

		default:
			break;
		}
	}

	return 1;
}

void JmsProcThread::final()
{
	(*m_Log)(ZQ::common::Log::L_INFO, L"JmsProcThread::final()");

	UnInitJms();

	if (m_hConStart)
	{
		CloseHandle(m_hConStart);
		m_hConStart = NULL;
	}
	
	if (m_hStop)
	{
		CloseHandle(m_hStop);
		m_hStop = NULL;
	}
	
	delete this;
}

/************************************************************************/
/*        Configuration Function                                        */
/************************************************************************/
void JmsProcThread::GetHostName()
{
	WSADATA wsadata;
	WORD version = MAKEWORD(2, 0);
	int ret = WSAStartup(version, &wsadata);
	if (ret != 0)
	{
		return;
	}
	struct hostent* phost;
	
	char hostname[50];

	gethostname(hostname, 50);
	phost = gethostbyname(hostname);

	char** names;
	names = phost->h_aliases;
	
	char** iplist;
	iplist = phost->h_addr_list;

	while (*iplist)
	{
		strcpy(m_hostname, inet_ntoa(* (struct in_addr *) * iplist));
		iplist ++;
	}
}

void JmsProcThread::GetConfiguration()
{
	GetHostName();

	DWORD value;
	m_hCfgHandle = CFG_INITEx(L"DBSync", &value, L"ITV");
	if (!m_hCfgHandle)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, L"CFG_INITEx error <%d>", ZQ::JMSCpp::getLastJmsError());
		return;
	}

	DWORD num_count = 0;
	CFGSTATUS status = CFG_SUBKEY(m_hCfgHandle, L"\\ManualSyncPlugin", &num_count);
	if (status != CFG_SUCCESS)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, L"CFG_SUBKEY error <%d>", GetLastError());
		return;
	}
	
	GetConfiguration(L"JmsServerIP",		m_ServerIP);
	GetConfiguration(L"JmsServerPort",		m_ServerPort);
	GetConfiguration(L"NamingContext",		m_NamingContext);
	GetConfiguration(L"ConnectionFactory",	m_ConnectionFactory);
	GetConfiguration(L"SendDesinationName",	m_SendDestinationName);
	GetConfiguration(L"RecvDesinationName",	m_RecvDestinationName);

	// clean up
	CFG_TERM(m_hCfgHandle);
}

void JmsProcThread::GetConfiguration(wchar_t* app_name, std::string& buf)
{
	wchar_t wBuf[BUFSIZ + 1];
	wcsnset(wBuf, 0, BUFSIZ + 1);

	char sBuf[BUFSIZ + 1];
	strnset(sBuf, 0, BUFSIZ + 1);

	DWORD dwSize = BUFSIZ * (sizeof(WCHAR)/sizeof(char));
	DWORD dwType = 0;

	if (CFG_GET_VALUE(m_hCfgHandle, app_name, (BYTE*)wBuf, &dwSize, &dwType) == S_OK)
	{
		Unicode2Ansi(wBuf, sBuf);
		buf = sBuf;
		(*m_Log)(ZQ::common::Log::L_INFO, L"Get Configuration [%s] = %s", app_name, wBuf);
	}
}

void JmsProcThread::GetConfiguration(wchar_t* app_name, DWORD& buf)
{
	char sBuf[BUFSIZ + 1];
	strnset(sBuf, 0, BUFSIZ + 1);

	DWORD dwSize = BUFSIZ * (sizeof(WCHAR)/sizeof(char));
	DWORD dwType = 0;

	if (CFG_GET_VALUE(m_hCfgHandle, app_name, (BYTE*)&buf, &dwSize, &dwType) == S_OK)
	{
		(*m_Log)(ZQ::common::Log::L_INFO, L"Get Configuration [%s] = %d", app_name, buf);
	}
}

void JmsProcThread::Unicode2Ansi(wchar_t* wch, char* ch)
{
	int len = wcslen(wch) * 2 + 1;
	WideCharToMultiByte(CP_ACP, 0, wch, -1, ch, len, NULL, NULL);
}


/************************************************************************/
/*        XML Proc Function                                             */
/************************************************************************/
void JmsProcThread::SendStartXMLFile(_LAM_to_DBSync* ltd)
{
	char xml[BUFSIZ];
	strnset(xml, 0, BUFSIZ);

	GenerateStartXMLFile(xml, ltd);

	(*m_Log)(ZQ::common::Log::L_INFO, "Start XML File is (%s)", xml);

	m_JmsSession->textMessageCreate(xml, m_JmsTxtMessage);

	SetMessageProperty();
	m_JmsProducer.send(&m_JmsTxtMessage);
}

void JmsProcThread::SendCompleteXMLFile(_DBSync_to_LAM* dtl)
{
	MutexGuard guard(m_TaskMutex);

	bool bSendStatus = true;

	if (m_SendBufQueue.size() > 0)
	{
		bSendStatus = DumpBufQueue();
	}

	char xml[BUFSIZ];	
	strnset(xml, 0, BUFSIZ);
	GenerateCompleteXMLFile(xml, dtl);

	if (!bSendStatus)
	{
		m_SendBufQueue.push(xml);

		return;
	}

	(*m_Log)(ZQ::common::Log::L_INFO, "Complete XML File is (%s)", xml);

	m_JmsSession->textMessageCreate(xml, m_JmsTxtMessage);

	SetMessageProperty();
	
	if (!m_JmsProducer.send(&m_JmsTxtMessage))
	{
		m_SendBufQueue.push(xml);
	}
}

bool JmsProcThread::DumpBufQueue()
{
	do 
	{
		m_JmsSession->textMessageCreate((char*)m_SendBufQueue.front().c_str(), m_JmsTxtMessage);
		
		SetMessageProperty();

		if (!m_JmsProducer.send(&m_JmsTxtMessage))
		{
			// if send failed did not pop msg from queue but return
			return false;
		}
		
		m_SendBufQueue.pop();
	}
	while(m_SendBufQueue.size() > 0);

	return true;
}

void JmsProcThread::GenerateStartXMLFile(char* xml, _LAM_to_DBSync* ltd)
{
	char syncpath[MAX_PATH] = {0};
	WideCharToMultiByte(CP_ACP, 0, ltd->_SyncPath, -1, syncpath, MAX_PATH, NULL, NULL);

	
	sprintf(xml, "<Msg>\n"
				 "<Header source=\"%s\" application=\"DBSync\"/>\n"
				 "<SyncResult status =\"1\" id=\"%d\" type=\"%d\" \n"
				 "path=\"%s\" errorCode=\"\" errorDescription=\"\"/>\n"
				 "</Msg>\n",
				 m_hostname,
				 ltd->_SynComID,
				 ltd->_SyncType,
				 syncpath);
}

void JmsProcThread::GenerateCompleteXMLFile(char* xml, _DBSync_to_LAM* dbSync_to_Lam)
{
	char syncpath[1024] = {0};
	WideCharToMultiByte(CP_ACP, 0, dbSync_to_Lam->_SyncPath, -1, syncpath, MAX_PATH, NULL, NULL);
	
	if (wcslen(dbSync_to_Lam->_errorCode) > 0)
	{
		char errcode[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dbSync_to_Lam->_errorCode, -1, errcode, MAX_PATH, NULL, NULL);

		char errdes[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dbSync_to_Lam->_errorDescription, -1, errdes, MAX_PATH, NULL, NULL);

		sprintf(xml, "<Msg>\n"
					 "<Header source=\"%s\" application=\"DBSync\"/>\n"
					 "<SyncResult status =\"3\" id=\"%d\" type=\"%d\" \n"
					 "path=\"%s\" errorCode=\"%s\" errorDescription=\"%s\"/>\n"
					 "</Msg>\n",
					 m_hostname,
					 dbSync_to_Lam->_SynComID,
					 dbSync_to_Lam->_SyncType,
					 syncpath,
					 errcode,
					 errdes);
	}
	else
	{
		sprintf(xml, "<Msg>\n"
					 "<Header source=\"%s\" application=\"DBSync\"/>\n"
					 "<SyncResult status =\"2\" id=\"%d\" type=\"%d\" \n"
					 "path=\"%s\" errorCode=\"\" errorDescription=\"\"/>\n"
					 "</Msg>\n",
					 m_hostname,
					 dbSync_to_Lam->_SynComID,
					 dbSync_to_Lam->_SyncType,
					 syncpath);
	}
}

bool JmsProcThread::ParseXMLFile(char* xml, _LAM_to_DBSync* lam_to_DBSync)
{
	bool ret;

	m_XmlProc.CoInit();
	ret = m_XmlProc.GetParameters(xml, 
							      lam_to_DBSync->_SynComID,
							      lam_to_DBSync->_SyncType,
							      lam_to_DBSync->_SyncPath);
	m_XmlProc.CoUnInit();
	
	if (ret)
	{
		(*m_Log)("SynComID = %d", lam_to_DBSync->_SynComID);
		(*m_Log)("SyncType = %d", lam_to_DBSync->_SyncType);
		(*m_Log)(L"SyncPath = %s", lam_to_DBSync->_SyncPath);
	}

	return ret;
}


/************************************************************************/
/*        Jms Function                                                  */
/************************************************************************/
bool JmsProcThread::InitJMS()
{
	if (m_pJmsContext)
	{
		delete m_pJmsContext;
		m_pJmsContext = NULL;
	}

	/**************    Context    **************/
	char url[40];
	strnset(url, 0, 40);
	sprintf(url, "%s:%d", m_ServerIP.c_str(), m_ServerPort);

	m_pJmsContext = new ZQ::JMSCpp::Context(url, m_NamingContext.c_str());
	if (!m_pJmsContext || !m_pJmsContext->_context)
	{
		if (m_pJmsContext)
		{
			delete m_pJmsContext;
			m_pJmsContext = NULL;
		}
		(*m_Log)(ZQ::common::Log::L_ERROR, 
			     "Init JmsContext Error, Server <%s>, NamingContext <%s>, ErrCode <%d>",
			     url,
			     m_NamingContext.c_str(),
			     ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, 
		     "Init JmsContext Success, Server <%s>, NamingContext <%s>", 
		     url,
		     m_NamingContext.c_str());

	
	/**************    ConnectionFactory    **************/
	m_JmsCNFactory = new ZQ::JMSCpp::ConnectionFactory();
	if (!m_pJmsContext->createConnectionFactory(m_ConnectionFactory.c_str(), *m_JmsCNFactory)
		|| !m_JmsCNFactory->_connectionFactory)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, 
			     "Create ConnectionFactory <%s> failed <%d>",  
			     m_ConnectionFactory.c_str(),
			     ZQ::JMSCpp::getLastJmsError());
		
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, 
		     "Create ConnectionFactory <%s> Success", 
		     m_ConnectionFactory.c_str());


	/**************     createConnection     **************/
	m_JmsConnection = new ZQ::JMSCpp::Connection();
	if (!m_JmsCNFactory->createConnection(*m_JmsConnection)
		|| !m_JmsConnection->_connection)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, 
			     "createConnection failed<%d>", 
			     ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Connection        Success");

	m_JmsConnection->SetConnectionCallback(connectionMonitor, this);
	
	
	/**************      createSession      **************/
	m_JmsSession = new ZQ::JMSCpp::Session();
	if (!m_JmsConnection->createSession(*m_JmsSession)
		|| !m_JmsSession->_session)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, 
			     "Create ReadSession failed <%d>", 
			     ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create ReadSession       Success");
	

	/************** create Send Destination  **************/
	if (!m_pJmsContext->createDestination(m_SendDestinationName.c_str(), m_JmsSendDestination))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Send Destination failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Send Destination  Success");


	/************** create Recv Destination  **************/
	if (!m_pJmsContext->createDestination(m_RecvDestinationName.c_str(), m_JmsRecvDestination))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Recv Destination failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Recv Destination  Success");
	
	
	/**************   createConsumer    **************/
	if (!m_JmsSession->createConsumer(&m_JmsRecvDestination, m_JmsConsumer))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Consumer    failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Consumer          Success");
	

	//NOTE, you must call this after you load jvm
	SetConsoleCtrlHandler(HandlerRoutine, TRUE ); //hook the system event 

	return true;
}

bool JmsProcThread::StartJMS()
{
	/**************    Connection Start  **************/
	if (!m_JmsConnection->start())
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Connection Start   failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Connection Start         Success");

	m_bInitialize = true;

	/**************    create text message   **************/
	if (!m_JmsSession->textMessageCreate("", m_JmsTxtMessage))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "create text message failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Text Message      Success");

	return true;
}

bool JmsProcThread::SetMessageProperty()
{
	if(!m_JmsTxtMessage.setIntProperty("MESSAGECODE", 1013))
	{
		(*m_Log)(ZQ::common::Log::L_INFO, "setIntProperty failed");
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "setIntProperty(MESSAGECODE, 1013) Success");

	if(!m_JmsTxtMessage.setStringProperty("MESSAGECLASS", "NOTIFICATION"))
	{
		(*m_Log)(ZQ::common::Log::L_INFO, "setIntProperty failed");
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "setIntProperty(MESSAGECLASS, NOTIFICATION) Success");
	return true;
}

void JmsProcThread::UnInitJms()
{
	(*m_Log)(ZQ::common::Log::L_INFO, "UnInitJms()");

	m_JmsTxtMessage.DestroyMessage();
	m_JmsProducer.close();
	m_JmsConsumer.close();
	m_JmsSendDestination.destroy();
	m_JmsRecvDestination.destroy();

	if (m_JmsSession != NULL)
	{
		m_JmsSession->close();
		delete m_JmsSession;
		m_JmsSession = NULL;
	}

	if (m_JmsConnection != NULL)
	{
		m_JmsConnection->close();
		delete m_JmsConnection;
		m_JmsConnection = NULL;
	}

	if (m_JmsCNFactory != NULL)
	{
		m_JmsCNFactory->Destroy();
		delete m_JmsCNFactory;
		m_JmsCNFactory = NULL;
	}

	if (m_pJmsContext)
	{
		delete m_pJmsContext;
		m_pJmsContext = NULL;
	}
}

void JmsProcThread::connectionMonitor(int errType,VOID* lpData)
{
	JmsProcThread* pPrcThd = (JmsProcThread*)lpData;
	pPrcThd->SetConBroken();
}

void JmsProcThread::SetReplyAndProducer()
{
	m_JmsTxtMessage.getReplyTo(m_JmsRecvDestination);
	m_JmsSession->createProducer(&m_JmsSendDestination, m_JmsProducer);
	
	m_bCreateProducer = true;
}

void JmsProcThread::StopThread()
{
	bool ret = SetEvent(m_hStop); 
	if (!ret)
	{
		int err = GetLastError();
	}
}

