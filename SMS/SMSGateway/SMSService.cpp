#include "StdAfx.h"
#include "SMSService.h"

#include "Log.h"
#include "ScLog.h"

using namespace ZQ::common;

DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

SMSService vdcpService;
BaseSchangeServiceApplication *Application = &vdcpService;

SMSService::SMSService() :	
BaseSchangeServiceApplication(),
m_dbThread(NULL),
m_pReadSocketThd(NULL), 
m_pWriteSocketThd(NULL),
m_pRawMsgProcThd(NULL),
m_pRespMsgThd(NULL),
m_ipList(NULL), 
m_StreamCtrlFlgMap(NULL),
m_SPNumberMap(NULL)
{
	memset(m_dbPath,    0x00, (MAX_PATH + 1)*sizeof(char));
	memset(m_ServiceID, 0x00, 20*sizeof(char));

	// ��TM�����������ȴ���ʱ��
	m_waitTime = SMSSRV_DEFAULT_WAIT_TIME;

	// ��TM������������ id, ȱʡֵΪ0
	m_UID = 0;
	m_MsgUID = 0;
	
	// ��TICP������������ sequence id
	m_sequenceID = -2;

	// ��TICP���������ӵ����ȴ�ʱ��
	m_timeout = SELECT_TIME_OUT;

	// ��TICP������������������
	m_TicpRedoTimes = TICP_REDO_TIMES;

	// ��TM������������������
	m_TMRedoTimes = TM_REDO_TIMES;

	// �����ݿ⣬�����Сʱ֮ǰ������
	m_DBClearHours = DB_CLEAR_HOURS;

	// ��ȡ���ݿ⣬�����ٷ���ǰ������
	m_DBOverTime = DB_OVER_TIME;

	// ����������
	m_HeartbeatLogWindows = MAX_HEARTBEAT;

	// ��ʧ����������������
	m_LostHeartbeatMaxCount = MAX_LOST_HEARTBEAT;	
}

SMSService::~SMSService()
{
	glog(Log::L_DEBUG, "SMSService ��������");
}

HRESULT SMSService::OnInit(void)
{
	BaseSchangeServiceApplication::OnInit();
	pGlog = m_pReporter;
	
	glog(Log::L_DEBUG, "SMSService::OnInit");

	if(!ReadConfig())
	{
		return S_FALSE;
	}

	m_dbThread = new DBThread(this);
	if (!m_dbThread)
	{
		glog(Log::L_ERROR, "new db thread failed");
	}

	m_pWriteSocketThd = new WriteSocketThread(this);
	if (!m_pWriteSocketThd)
	{
		glog(Log::L_ERROR, "create WriteSocketThread failed");
		OnUnInit();
		return S_FALSE;
	}

	m_pRawMsgProcThd = new RawMsgProcessThread(this);
	if (!m_pRawMsgProcThd)
	{
		glog(Log::L_ERROR, "create RawMsgProcessThread failed");
		OnUnInit();
		return S_FALSE;
	}

	m_pRespMsgThd = new RespMsgProcessThread(this);
	if (!m_pRespMsgThd)
	{
		glog(Log::L_ERROR, "create RespMsgProcessThread failed");
		OnUnInit();
		return S_FALSE;
	}

	m_pReadSocketThd = new ReadSocketThread(this);
	if (!m_pReadSocketThd)
	{
		glog(Log::L_ERROR, "create ReadSocketThread failed");
		OnUnInit();
		return S_FALSE;
	}

	return S_OK;
}

HRESULT SMSService::OnStart(void)
{
	glog(Log::L_DEBUG, "SMSService::OnStart");
	
	bool bRet = m_dbThread->start();
	if (!bRet)
	{
		glog(Log::L_ERROR, L"db thread fail to start");
	}

	bRet = m_pWriteSocketThd->start();
	if(!bRet)
	{
		glog(Log::L_ERROR, L"WriteSocketThread fail to start");
		return S_FALSE;
	}

	bRet = m_pRespMsgThd->start();
	if (!bRet)
	{
		glog(Log::L_ERROR, L"RespMsgProcessThread fail to start");
		return S_FALSE;
	}

	bRet = m_pRawMsgProcThd->start();
	if(!bRet)
	{
		glog(Log::L_ERROR, L"RawMsgProcessThread fail to start");
		return S_FALSE;
	}

	bRet = m_pReadSocketThd->start();
	if(!bRet)
	{
		glog(Log::L_ERROR, L"ReadSocketThread fail to start");
		return S_FALSE;
	}

	bRet = m_pReadSocketThd->start();
	if(!bRet)
	{
		glog(Log::L_ERROR, L"ReadSocketThread fail to start");
		return S_FALSE;
	}
	return S_OK;
}

HRESULT SMSService::OnStop(void)
{
	glog(Log::L_DEBUG, "SMSService::OnStop");
	
	bool bRet;
	bRet = m_pReadSocketThd->StopThread();
	if (bRet)
	{
		glog(Log::L_DEBUG, "���� �߳� ReadSocketThread     ֹͣ");	
	}
	else
	{
		glog(Log::L_DEBUG, "���� �߳� ReadSocketThread     ֹͣʧ��");
	}

	bRet = m_pRawMsgProcThd->StopThread();
	if (bRet)
	{
		glog(Log::L_DEBUG, "���� �߳� RawMsgProcessThread  ֹͣ");
	}
	else
	{
		glog(Log::L_DEBUG, "���� �߳� RawMsgProcessThread  ֹͣʧ��");
	}
	
	bRet = m_pRespMsgThd->StopThread();
	if (bRet)
	{
		glog(Log::L_DEBUG, "���� �߳� RespMsgProcessThread ֹͣ");
	}
	else
	{
		glog(Log::L_DEBUG, "���� �߳� RespMsgProcessThread ֹͣʧ��");
	}

	bRet = m_pWriteSocketThd->StopThread();
	if (bRet)
	{
		glog(Log::L_DEBUG, "���� �߳� WriteSocketThread    ֹͣ");
	}
	else
	{
		glog(Log::L_DEBUG, "���� �߳� WriteSocketThread    ֹͣʧ��");
	}

	bRet = m_dbThread->StopThread();
	if (bRet)
	{
		glog(Log::L_DEBUG, "���� �߳� db thread            ֹͣ");
	}
	else
	{
		glog(Log::L_DEBUG, "���� �߳� db thread            ֹͣʧ��");
	}

	BaseSchangeServiceApplication::OnStop();

	return S_OK;
}

HRESULT SMSService::OnUnInit(void)
{
	glog(Log::L_DEBUG, "SMSService::OnUnInit");

	if (m_pReadSocketThd)
	{
		delete m_pReadSocketThd;
		m_pReadSocketThd = NULL;
	}

	if (m_pRawMsgProcThd)
	{
		delete m_pRawMsgProcThd;
		m_pRawMsgProcThd = NULL;
	}

	if (m_pRespMsgThd)
	{
		delete m_pRespMsgThd;
		m_pRespMsgThd = NULL;
	}

	if (m_pWriteSocketThd)
	{
		delete m_pWriteSocketThd;
		m_pWriteSocketThd = NULL;
	}
	
	if (m_dbThread)
	{
		delete m_dbThread;
		m_dbThread = NULL;
	}

	if (m_StreamCtrlFlgMap)
	{
		glog(Log::L_DEBUG, "��� ý����ƶ�Ӧ��");
		delete m_StreamCtrlFlgMap;
		m_StreamCtrlFlgMap = NULL;
	}

	if (m_ipList)
	{
		glog(Log::L_DEBUG, "��� TICP IP ��ַ���б�");
		delete m_ipList;
		m_ipList = NULL;
	}
	
	if (m_SPNumberMap)
	{
		glog(Log::L_DEBUG, "��� ���SPNumber�ı�");
		delete m_SPNumberMap;
		m_SPNumberMap = NULL;
	}

	return S_OK;
}

bool SMSService::ReadConfig()
{
	m_pXmlProc.CoInit();
	
	// ���XML�����ļ���·��
	DWORD dSize = MaxPath * 2;
	WCHAR configFilePath[MAX_PATH + 1];
	memset(configFilePath, 0x00, (MAX_PATH + 1)*sizeof(WCHAR));
	getConfigValue(_T("ConfigFilePath"),   configFilePath, (wchar_t*)configFilePath, &dSize, true, true);
	wcstombs(m_info._configFilePath, configFilePath, wcslen(configFilePath));
	

	// ���DB��·��
	dSize = MaxPath * 2;
	WCHAR dbPath[MAX_PATH + 1];
	memset(dbPath, 0x00, (MAX_PATH + 1)*sizeof(WCHAR));
	getConfigValue(_T("DBPath"),  dbPath, (wchar_t*)dbPath, &dSize, true, true);
	wcstombs(m_dbPath, dbPath, wcslen(dbPath));

	// ��XML�����ļ��л�ȡ��ʼ����Ϣ
	if(!ReadConfigXmlFile())
	{
		return false;
	}
	
	// TICP ����������
	getConfigValue(_T("Times"),			&m_TicpRedoTimes, m_TicpRedoTimes, true, true);
	
	// TM ����������
	getConfigValue(_T("TMTimes"),		&m_TMRedoTimes,	  m_TMRedoTimes, true, true);
	
	// ��ȡ�ȴ�TM�ظ������ʱ��
	getConfigValue(_T("WaitTime"),      &m_waitTime, m_waitTime, true, true);
	
	// ��ȡ�ȴ�TICP�����ʱ��
	getConfigValue(_T("SelectTimeOut"), &m_timeout,  m_timeout,  true, true);

	// ���ݿ������ݵ�ʱ��
	getConfigValue(_T("DBClearHours"),   &m_DBClearHours, m_DBClearHours, true, true);

	// ���ݿ����ݹ��ڵ�ʱ��
	getConfigValue(_T("OverTime"),		&m_DBOverTime, m_DBOverTime, true, true);

	// ������������
	getConfigValue(_T("HeartBeatLogWindow"),	&m_HeartbeatLogWindows, m_HeartbeatLogWindows, true, true);

	// ��ʧ����������������
	getConfigValue(_T("LostHeartBeatMaxCount"),	&m_LostHeartbeatMaxCount, m_LostHeartbeatMaxCount, true, true);

	DWORD serviceID = TM_SERVICE_ID;
	getConfigValue(_T("ServiceID"),		&serviceID, serviceID, true, true);
	sprintf(m_ServiceID, "%d", serviceID);

	glog(Log::L_DEBUG, "SMSServer ip<%s> and port<%d>", m_info._TMIp, 
														m_info._TMPort);
	
	glog(Log::L_DEBUG, "Play      flag  is  <%s>", m_info._ACFlag);
	glog(Log::L_DEBUG, "Chat      flag  is  <%s>", m_info._CTFlag);
	glog(Log::L_DEBUG, "Register  flag  is  <%s>", m_info._RGFlag);
	glog(Log::L_DEBUG, "NickName  flag  is  <%s>", m_info._NCFlag);
	glog(Log::L_DEBUG, "Response        is  <%d>", m_info._response);
	glog(Log::L_DEBUG, "ErrorResponse   is  <%d>", m_info._errorResponse);
	glog(Log::L_DEBUG, "ReplyHistory    is  <%d>", m_info._replyHistory);

	m_pXmlProc.CoUnInit();
	
	return true;
}

int SMSService::GetUID(bool content)
{
	if (content)
	{
		return m_MsgUID ++;
	}
	return m_UID ++;
}

DWORD SMSService::GetSequenceID()
{
	m_sequenceID += 2;
	return m_sequenceID;
}

bool SMSService::ReadConfigXmlFile()
{	
	if(!m_pXmlProc.XmlGetConfig(&m_info))
	{
		return false;
	}
	
	ReturnText rt;
	
	int len = m_info._ActionCodeList.GetCount();
	for (int i = 0; i < len; i++)
	{
		char temp[200];
		memset(temp, 0x00, 200*sizeof(char));
		WideCharToMultiByte(CP_ACP, 0, m_info._ReturnTextList.GetHead(), -1, temp, 200, NULL, NULL);
		
		rt.m_index = _wtoi(m_info._ActionCodeList.GetHead());
		rt.setText(temp);
		rt.m_successFlag = m_info._success[rt.m_index];

		m_vec.push_back(rt);
		
		m_info._ActionCodeList.RemoveHead();
		m_info._ReturnTextList.RemoveHead();
	}

	std::vector<ReturnText>::iterator it;
	for (it = m_vec.begin(); it != m_vec.end(); it++)
	{
		glog(Log::L_DEBUG, "ReturnText<%d> <%s> <%d>", (*it).m_index,
													   (*it).m_text,
													   (*it).m_successFlag);
	}
	
	len = m_info._TMFlagList.GetCount();
	m_StreamCtrlFlgMap = new CMapStringToString(len);
	for (i = 0; i < len; i++)
	{
		m_StreamCtrlFlgMap->SetAt(m_info._TMFlagList.GetHead(), m_info._TicpFlagList.GetHead());
		m_info._TMFlagList.RemoveHead();
		m_info._TicpFlagList.RemoveHead();
	}

	if (!SetIPList())
	{
		return false;
	}

	len = m_info._SPList.GetCount();
	m_SPNumberMap = new CMapStringToString(len);
	for (i = 0; i < len; i++)
	{
		m_SPNumberMap->SetAt(m_info._SPList.GetHead(), m_info._SPNumberList.GetHead());
		m_info._SPList.RemoveHead();
		m_info._SPNumberList.RemoveHead();
	}

	CString SP, SPNumber;
	for (POSITION pos = m_SPNumberMap->GetStartPosition(); pos != NULL; )
	{
		m_SPNumberMap->GetNextAssoc(pos, SP, SPNumber);
		glog(Log::L_DEBUG, L"SP <%s>, SPNumber <%s>", SP, SPNumber);
	}

	return true;
}

int SMSService::GetTicpFlag(char* contentFlag, char* realFlag)
{
	CString key, value, strContentFlag;

	int ret, len = strlen(contentFlag);
	
	// char -> CString
	strContentFlag = contentFlag;
	
	for (POSITION pos = m_StreamCtrlFlgMap->GetStartPosition(); pos != NULL; )
	{
		m_StreamCtrlFlgMap->GetNextAssoc(pos, key, value);
		
		key.MakeLower();//low case
		ret = strContentFlag.Find(key);
		if (ret == 0)
		{
			// CStirng(WCHAR*) -> char
			len = key.GetLength();
			wcstombs(realFlag, key, len);
			//glog(Log::L_DEBUG, "Real Flag = <%s> <%d>", realFlag, _wtoi(value));

			return _wtoi(value);
		}
	}
	return -1;
}

bool SMSService::SetIPList()
{
	if (m_info._TicpIplist.GetCount() <= 0)
	{
		return false;
	}
	
	m_ipList = new IPList(m_info._TicpIplist, m_info._TicpPortlist);

	return true;
}

int	SMSService::GetTicpIP(char* ip, long& port)
{
	return m_ipList->GetIP(ip, port);
}

bool SMSService::GetReturnCode(int actionCode, char* returnText)
{
	std::vector<ReturnText>::iterator it;
	for (it = m_vec.begin(); it != m_vec.end(); it++)
	{
		if (actionCode == (*it).m_index)
		{
			strcpy(returnText, (*it).m_text);
			return true;
		}
	}
	
	return false;
}

bool SMSService::IsReturnCodeSuccess(int ret)
{
	std::vector<ReturnText>::iterator it;
	for (it = m_vec.begin(); it != m_vec.end(); it++)
	{
		if (ret == (*it).m_index)
		{
			if ((*it).m_successFlag == 1)
			{
				return true;
			}
		}
	}
	return false;
}

bool SMSService::GetTelephoneNumberProfix(char* SPNumber, char* telephoneProfix)
{
	CString strSPNumber = SPNumber;
	
	CString key, value;
	for (POSITION pos = m_SPNumberMap->GetStartPosition(); pos != NULL; )
	{
		m_SPNumberMap->GetNextAssoc(pos, key, value);
		
		if (key == strSPNumber)
		{
			int len = value.GetLength() * 2 + 1;
			WideCharToMultiByte(CP_ACP, 0, value, -1, telephoneProfix, len, NULL, NULL);
			return true;
		}
	}
	return false;
}
