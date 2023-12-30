#include "JmsProcThread.h"

bool                  m_bJmsStatus = false;
HANDLE                m_hJmsStatus = NULL;
JmsProcThread*        m_JmsProcThread = NULL;
ZQ::common::Log*      m_Log = NULL;
CRITICAL_SECTION      m_Section;

ManualSync_ProcData_Callback	m_ProcDataCallback = NULL;

bool ManualSync_Init(void* pFunc, ZQ::common::Log * pLog)
{
	InitializeCriticalSection(&m_Section);
	m_Log = pLog;
	m_ProcDataCallback = (ManualSync_ProcData_Callback)pFunc;
	
	(*m_Log)(ZQ::common::Log::L_INFO, "ManualSync_Init()");

	m_hJmsStatus = CreateEvent(NULL, false, false, NULL);	
	if (m_hJmsStatus == NULL)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Stop Event failed, Error is %d", GetLastError());
	}

	// Send Thread start
	m_JmsProcThread = new JmsProcThread(m_Log);
	m_JmsProcThread->start();

	DWORD ret = 0;
	ret = WaitForSingleObject(m_hJmsStatus, INFINITE);
	if (ret == WAIT_OBJECT_0)
	{
		if (m_bJmsStatus == false)
		{
			m_JmsProcThread = NULL;
			return false;
		}
	}

	return true;
}

bool ManualSync_Run()
{
	(*m_Log)(ZQ::common::Log::L_INFO, "ManualSync_Run()");

	m_JmsProcThread->SetConStart();

	return true;
}

void ManualSync_Stop()
{
	if (m_Log)
	{
		(*m_Log)(ZQ::common::Log::L_INFO, "DBSync_LAM_Stop()");
	}

	if (m_hJmsStatus)
	{
		CloseHandle(m_hJmsStatus);
		m_hJmsStatus = NULL;
	}

	if (m_JmsProcThread)
	{
		(*m_Log)(ZQ::common::Log::L_INFO, "JmsProcThread StopThread()");
		m_JmsProcThread->StopThread();
	}

	DeleteCriticalSection(&m_Section);
}




















/*
bool InitJMS()
{
	if (m_pJmsContext)
	{
		delete m_pJmsContext;
		m_pJmsContext = NULL;
	}

	//**************    Context    
	char url[40];
	strnset(url, 0, 40);
	sprintf(url, "%s:%s", m_ServerIP.c_str(), m_ServerPort.c_str());

	m_pJmsContext = new ZQ::JMSCpp::Context(url, m_NamingContext.c_str());
	if (!m_pJmsContext)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "new Context failed");
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "new Context");

	if (!m_pJmsContext->_context)
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

	
	//**************    ConnectionFactory    
	if (!m_pJmsContext->createConnectionFactory(m_ConnectionFactory.c_str(), m_JmsCNFactory))
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


	//**************     createConnection     
	if (!m_JmsCNFactory.createConnection(m_JmsConnection))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, 
			     "createConnection failed<%d>", 
			     ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Connection        Success");
	

	if (!m_JmsConnection._connection)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "connection handle is invalid");
		return false;
	}

	m_JmsConnection.SetConnectionCallback(ConnectionMonitor, NULL);
	
	
	//**************      createSession      
	if (!m_JmsConnection.createSession(m_JmsReadSession))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, 
			     "Create ReadSession failed <%d>", 
			     ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create ReadSession       Success");
	

	if (!m_JmsConnection.createSession(m_JmsWriteSession))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create WriteSession failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create WriteSession      Success");


	//**************     createDestination  
	if (!m_pJmsContext->createDestination(m_DestinationName.c_str(), m_JmsDestination))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Destination failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Destination       Success");
	
	
	//**************   createConsumer    
	if (!m_JmsReadSession.createConsumer(&m_JmsDestination, m_JmsConsumer))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create Consumer    failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Consumer          Success");
	

	//**************    Connection Start  
	if (!m_JmsConnection.start())
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Connection Start   failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Connection Start         Success");
	

	m_bInitialize = true;


	//**************    create text message   
	if (!m_JmsReadSession.textMessageCreate("", m_JmsTxtMessage))
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "create text message failed <%d>", ZQ::JMSCpp::getLastJmsError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Text Message      Success");
	
	return true;
}

void UnInitJms()
{
	(*m_Log)(ZQ::common::Log::L_INFO, "UnInitJms()");

	if (m_pJmsContext)
	{
		if (m_bInitialize)
		{
			m_JmsConnection.stop();
		}
		delete m_pJmsContext;
		m_pJmsContext = NULL;
	}
}


void ConnectionMonitor(int ErrType, void* lpData)
{
	(*m_Log)(ZQ::common::Log::L_INFO, "Connection Monitor");
	m_bConnection = true;
}

void    GenerateStartXMLFile(char* xml, int id)
{
	sprintf(xml, "<Msg>"
				 "<Header source=\"%s\" application=\"DBSync\"/>"
				 "<SyncResult status =\"1\" id=\"%d\" type=\"\" "
				 "path=\"\" errorCode=\"\" errorDescription=\"\"/>"
				 "</Msg>",
				 m_hostname,
				 id);
}

void    GenerateCompleteXMLFile(char* xml, _DBSync_to_LAM* dbSync_to_Lam)
{
	sprintf(xml, "<Msg>"
				 "<Header source=\"%s\" application=\"DBSync\"/>"
				 "<SyncResult status =\"2\" id=\"%d\" type=\"%d\" "
				 "path=\"%S\" errorCode=\"%S\" errorDescription=\"%S\"/>"
				 "</Msg>",
				 m_hostname,
				 dbSync_to_Lam->_SynComID,
				 dbSync_to_Lam->_SyncType,
				 dbSync_to_Lam->_SyncPath,
				 dbSync_to_Lam->_errorCode,
				 dbSync_to_Lam->_errorDescription);
}

bool	ParseXMLFile(char* xml, _LAM_to_DBSync* lam_to_DBSync)
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
		(*m_Log)("SyncPath = %S", lam_to_DBSync->_SyncPath);
	}

	return ret;
}

void Unicode2Ansi(wchar_t* wch, char* ch)
{
	int len = wcslen(wch) * 2 + 1;
	WideCharToMultiByte(CP_ACP, 0, wch, -1, ch, len, NULL, NULL);
}

void GetConfiguration()
{
	GetHostName();
	
	//****    Default Value    ****
//	m_ServerAddress     = "192.168.80.130:1099";
//	m_NamingContext     = "org.jnp.interfaces.NamingContextFactory";
//	m_DestinationName   = "topic/testTopic";
//	m_ConnectionFactory = "ConnectionFactory";
//	m_sourceMachine		= "192.168.0.1";
	

	DWORD value;
	m_hCfgHandle = CFG_INITEx(L"DBSync", &value, L"ITV");
	if (!m_hCfgHandle)
	{
		(*m_Log)("CFG_INITEx error <%d>", ZQ::JMSCpp::getLastJmsError());
		return;
	}

	DWORD num_count = 0;
	CFGSTATUS status = CFG_SUBKEY(m_hCfgHandle, L"\\RTSyncAddIn", &num_count);
	if (status != CFG_SUCCESS)
	{
		(*m_Log)("CFG_SUBKEY error <%d>", ZQ::JMSCpp::getLastJmsError());
		return;
	}
	
	GetConfiguration(L"ServerIP",		    m_ServerIP);
	GetConfiguration(L"ServerPort",		    m_ServerPort);
	GetConfiguration(L"NamingContext",		m_NamingContext);
	GetConfiguration(L"DesinationName",		m_DestinationName);
	GetConfiguration(L"ConnectionFactory",	m_ConnectionFactory);
	GetConfiguration(L"LogFileName",		m_LogPath);

	// clean up
	CFG_TERM(m_hCfgHandle);
}

void    GetConfiguration(wchar_t* app_name, std::string& buf)
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

void   SetReplyAndProducer()
{
	m_JmsTxtMessage.getReplyTo(m_JmsDestination1);
	m_JmsWriteSession.createProducer(&m_JmsDestination1, m_JmsProducer);
	
	m_bCreateProducer = true;
}

void   SendStartXMLFile(_LAM_to_DBSync* ltd)
{
	char xml[BUFSIZ];
	strnset(xml, 0, BUFSIZ);

	GenerateStartXMLFile(xml, ltd->_SynComID);

	(*m_Log)(ZQ::common::Log::L_INFO, "Start XML File is (%s)", xml);

	m_JmsWriteSession.textMessageCreate(xml, m_JmsTxtMessage);

	m_JmsProducer.send(&m_JmsTxtMessage);
}

void   SendCompleteXMLFile(_DBSync_to_LAM* dtl)
{
	char xml[BUFSIZ];
	strnset(xml, 0, BUFSIZ);

	GenerateCompleteXMLFile(xml, dtl);

	(*m_Log)(ZQ::common::Log::L_INFO, "Complete XML File is (%s)", xml);

	m_JmsWriteSession.textMessageCreate(xml, m_JmsTxtMessage);

	m_JmsProducer.send(&m_JmsTxtMessage);
}



DWORD WINAPI SendMessageThread(LPVOID parameter)
{
	m_ProcDataCallback = (DBSync_Lam_ProcData_Callback)parameter;

	m_bCreateProducer = false;

	if(!InitJMS())
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "InitJMS failed");
		return false;
	}

	DWORD threadID;
	HANDLE hRecvThread = CreateThread(NULL, 0, RecvMessageThread, NULL, 0, &threadID);
	if (hRecvThread == NULL)
	{
		(*m_Log)(ZQ::common::Log::L_ERROR, "Create New Thread failed, err is %d", GetLastError());
		return false;
	}
	(*m_Log)(ZQ::common::Log::L_INFO, "Create Recv Thread       Success");

	DWORD ret = 0;

	HANDLE handles[2];
	handles[0] = m_hStop;
	handles[1] = m_hDataCome;

	bool bContinued = true;
	
	while (1)
	{
		ret = WaitForMultipleObjects(2, handles, false, INFINITE);
		
		switch (ret)
		{
			case WAIT_OBJECT_0:
				bContinued = false;
			    break;

			case WAIT_OBJECT_0 + 1:
				{
					EnterCriticalSection(&m_Section);
					_LAM_to_DBSync ltd = m_bufQueue.front();
					m_bufQueue.pop();
					LeaveCriticalSection(&m_Section);
					
					if (!m_bCreateProducer)
					{
						SetReplyAndProducer();
					}

					SendStartXMLFile(&ltd);

					_DBSync_to_LAM dtl;

					m_ProcDataCallback(&ltd, &dtl);

					SendCompleteXMLFile(&dtl);

					if (m_bufQueue.size() > 0)
					{
						SetEvent(m_hDataCome);
					}
				}
				
			case WAIT_TIMEOUT:
			default:
				break;
		}
	}
	
	return 1;
}

DWORD WINAPI RecvMessageThread(LPVOID parameter)
{
	(*m_Log)("RecvMessageThread run");

	char buf[BUFSIZ];

	while(m_JmsConsumer.receive(0, m_JmsTxtMessage))
	{
		EnterCriticalSection(&m_Section);
		
		strnset(buf, 0, BUFSIZ);
		m_JmsTxtMessage.getText(buf, BUFSIZ);
		_LAM_to_DBSync ltd;
		
		(*m_Log)(ZQ::common::Log::L_INFO, 
				 "Receive Buf is <%s>", buf);
		
		if (ParseXMLFile(buf, &ltd))
		{
			m_bufQueue.push(ltd);
		
			LeaveCriticalSection(&m_Section);
			
			if (!SetEvent(m_hDataCome))
			{
				(*m_Log)(ZQ::common::Log::L_ERROR, 
						 "SetEvent failed, Error is %d", 
						 GetLastError());
			}
		}
		else
		{
			(*m_Log)(ZQ::common::Log::L_ERROR,
					 "ParseXMLFile failed (%s)", 
					 buf);
		}
		//(*m_Log)(ZQ::common::Log::L_INFO, L"Receive Message is <%s>", buf);
	}
	
	(*m_Log)(ZQ::common::Log::L_INFO, 
		     "Receive error <%d>", 
		     ZQ::JMSCpp::getLastJmsError());
	return 1;
}



void         GetHostName()
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

void Stop()
{
	SetEvent(m_hStop);
}
*/

