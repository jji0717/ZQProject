#include "StdAfx.h"
#include "ModemGateway.h"

#include "Log.h"
#include "ScLog.h"

using namespace ZQ::common;

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

ModemGateway vdcpService;
BaseSchangeServiceApplication *Application = &vdcpService;

ModemGateway::ModemGateway() : BaseSchangeServiceApplication()
{
	m_comWriteThd = NULL;
	m_comReadThd = NULL;
	m_processRawMsgThd = NULL;

	m_com           =  SMSSRV_DEFAULT_COM;
	m_echo          =  SMSSRV_DEFAULT_ECHO;
	m_port          =  SMSSRV_DEFAULT_PORT;
	m_times         =  SMSSRV_DEFAULT_TIMES;
	m_timeout       =  SMSSRV_DEFAULT_TIMEOUT;
	m_overtime      =  SMSSRV_DEFAULT_OVER_TIME;

	memset(m_dbPath,       0x00,  BUFSIZ*sizeof(wchar_t));
}

ModemGateway::~ModemGateway()
{
}

HRESULT ModemGateway::OnInit(void)
{
	// set log pointer
	BaseSchangeServiceApplication::OnInit();
	pGlog = m_pReporter;

	glog(Log::L_DEBUG, L"****************	Entering SMS VOD Service	****************");

	getInitialize();

	m_comWriteThd = new SMSComPortWriteThread(this ,m_com);
	glog(Log::L_DEBUG, L"create SMSComPortWriteThread");

	m_processRawMsgThd = new SMSProcessRawMsgThread(m_comWriteThd, this);
	glog(Log::L_DEBUG, L"create SMSProcessRawMsgThread");

	m_comReadThd = new SMSComPortReadThread(m_comWriteThd->GetComCommunication(), m_processRawMsgThd, m_comWriteThd);
	glog(Log::L_DEBUG, L"create SMSComPortReadThread");

	glog(Log::L_DEBUG, L"ModemGateway::OnInit");
	
	return S_OK;
}

HRESULT ModemGateway::OnUnInit(void)
{
	m_comReadThd->stopReadThd();
	if (m_comReadThd != NULL)
	{
		delete m_comReadThd;
	}
	m_comReadThd = NULL;

	m_comWriteThd->stopWriteThd();
	if (m_comWriteThd != NULL)
	{
		delete m_comWriteThd;
	}
	m_comWriteThd = NULL;

	m_processRawMsgThd->stopProcessRawMsgThd();
	if (m_processRawMsgThd != NULL)
	{
		delete m_processRawMsgThd;
	}
	m_processRawMsgThd = NULL;
	
	glog(Log::L_INFO, "ModemGateway::OnUnInit");
	return S_OK;
}

HRESULT ModemGateway::OnStart(void)
{
	glog(Log::L_DEBUG, L"ModemGateway::OnStart");

	bool bRet = m_comWriteThd->start();
	if(bRet)
	{
		glog(Log::L_DEBUG, L"SMSComPortWriteThread start");
	}
	else
	{
		glog(Log::L_INFO, L"SMSComPortWriteThread fail to start");
	}

	Sleep(1000);
	
	bRet = m_processRawMsgThd->start();
	if(bRet)
	{
		glog(Log::L_DEBUG, L"SMSProcessRawMsgThread start");
	}
	else
	{
		glog(Log::L_INFO, L"SMSProcessRawMsgThread fail to start");
	}

	bRet = m_comReadThd->start();
	if(bRet)
	{
		glog(Log::L_DEBUG, L"SMSComPortReadThread start");
	}
	else
	{
		glog(Log::L_INFO, L"SMSComPortReadThread fail to start");
	}

	return S_OK;
}

HRESULT ModemGateway::OnStop(void)
{
	glog(Log::L_INFO, "ModemGateway::OnStop");

	bool bStop = m_comReadThd->stopReadThd();
	if (bStop)
	{
		glog(Log::L_INFO, L"SMSComPortReadThread stop");
	}
	else
	{
		glog(Log::L_INFO, L"SMSComPortReadThread fail to stop");
	}

	bStop = m_processRawMsgThd->stopProcessRawMsgThd();
	if (bStop)
	{
		glog(Log::L_INFO, L"SMSProcessRawMsgThread stop");
	}
	else
	{
		glog(Log::L_INFO, L"SMSProcessRawMsgThread fail to stop");
	}

	bStop = m_comWriteThd->stopWriteThd();
	if (bStop)
	{
		glog(Log::L_INFO, L"SMSComPortWriteThread stop");
	}
	else
	{
		glog(Log::L_INFO, L"SMSComPortWriteThread fail to stop");
	}

	BaseSchangeServiceApplication::OnStop();

	return S_OK;
}
void ModemGateway::getInitialize()
{
	getConfigValue(_T("Echo"),			&m_echo,			m_echo,			 true, true);
	getConfigValue(_T("ComPort"),		&m_com,				m_com,			 true, true);
	getConfigValue(_T("Times"),			&m_times,			m_times,		 true, true);
	getConfigValue(_T("TicpPort"),		&m_port,			m_port,			 true, true);
	getConfigValue(_T("Interval"),		&m_timeout,			m_timeout,		 true, true);
	getConfigValue(_T("OverTime"),		&m_overtime,		m_overtime,		 true, true);
	
	DWORD dwSize = MaxPath*2;
	getConfigValue(_T("TicpIpAddress"), m_wszIP,			m_wszIP,		 &dwSize, true, true);

	
	wchar_t ConfileFile[BUFSIZ];
	memset( ConfileFile,  0x00,  BUFSIZ*sizeof(wchar_t));
	getDefaultPath(m_dbPath, ConfileFile);
	
	dwSize = MaxPath*2;
	getConfigValue(_T("DBPath"),		m_dbPath,			m_dbPath,        &dwSize, true, true);
	
	dwSize = MaxPath*2;
	getConfigValue(_T("ConfigFile"),    ConfileFile,        ConfileFile,     &dwSize, true, true);
	wcstombs(m_info._configFilePath,    ConfileFile,        wcslen(ConfileFile));

	readConfigXml();
}

void ModemGateway::getDefaultPath(wchar_t* dbPath, wchar_t* configFile)
{
	wchar_t temp[100];
	memset(temp, 0x00, 100*sizeof(wchar_t));
	GetModuleFileName(NULL, temp, 100);
	wchar_t* p = wcsstr(temp, L"\\");
	wchar_t* q;
	
	while (p)
	{
		p = p + 1;
		q = p;
		p = wcsstr(p, L"\\");
	}
	if (q)
	{
		wcsncpy(dbPath, temp, wcslen(q));
	}
	else
	{
		wcscpy(dbPath, temp);
	}
	swprintf(configFile, L"%scongfig.xml", dbPath);
	wcscat(dbPath, L"SMSDB.mdb");
}

bool ModemGateway::readConfigXml()
{
	m_pXmlProc.CoInit();

	if (!m_pXmlProc.XmlGetConfig(&m_info))
	{
		return false;
	}
	
	int len = m_info._ActionCodeList.GetCount();
	m_ReturnCodeMap = new CMapStringToString(len);
	for (int i = 0; i < len; i++)
	{
		m_ReturnCodeMap->SetAt(m_info._ActionCodeList.GetHead(), m_info._ReturnTextList.GetHead());
		
		m_info._ActionCodeList.RemoveHead();
		m_info._ReturnTextList.RemoveHead();
	}

	m_pXmlProc.CoUnInit();

	CString actionCode, returnText;
	for (POSITION pos = m_ReturnCodeMap->GetStartPosition(); pos != NULL; )
	{
		m_ReturnCodeMap->GetNextAssoc(pos, actionCode, returnText);

		glog(Log::L_INFO, L"ReturnText<%s> <%s>", actionCode, returnText);
	}

	glog(Log::L_DEBUG, "Play      flag  is  <%s>",  m_info._ACFlag);
	glog(Log::L_DEBUG, "Chat      flag  is  <%s>",  m_info._CTFlag);
	glog(Log::L_DEBUG, "Register  flag  is  <%s>",  m_info._RGFlag);
	glog(Log::L_DEBUG, "NickName  flag  is  <%s>",  m_info._NCFlag);
	glog(Log::L_DEBUG, "Response        is  <%d>",  m_info._response);
	glog(Log::L_DEBUG, "ErrorResponse   is  <%d>",  m_info._errorResponse);

	return true;
}

bool ModemGateway::GetReturnCode(int actionCode, char* returnText)
{
	CString value, key;
	int iKey;

	for (POSITION pos = m_ReturnCodeMap->GetStartPosition(); pos != NULL; )
	{
		m_ReturnCodeMap->GetNextAssoc(pos, key, value);
		iKey = _wtoi(key);

		if (iKey == actionCode)
		{
			int len = value.GetLength()*2 + 1;
			int res = WideCharToMultiByte(CP_ACP, 0, value, -1, returnText, len, NULL, NULL);
			return true;
		}
	}
	return false;
}
