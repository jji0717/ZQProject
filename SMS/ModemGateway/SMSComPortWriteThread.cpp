//#include "stdafx.h"
#include "SMSComPortWriteThread.h"
#include "NativeThread.h"

#include "Log.h"
#include "ScLog.h"
#include "mbstring.h"

#define SMSSRV_TIMEOUT 1000000

SMSComPortWriteThread::SMSComPortWriteThread(ModemGateway* pSMSSrv, int comPort)
:NativeThread()
{
	m_pCommunication = new CComCommunication(comPort);
	m_pSMSSrv = pSMSSrv;
	m_calc = GetTickCount();
}

SMSComPortWriteThread::~SMSComPortWriteThread()
{
}

bool SMSComPortWriteThread::init(void)
{
	glog(Log::L_DEBUG, L"SMSComPortWriteThread::init");

	// initialize stop handle
	m_hStopEvent = CreateEvent(NULL, false, false, NULL);
	if(m_hStopEvent == NULL)
	{
		glog(Log::L_INFO, L"Failed to create stop event in Write Thread");
		return false;
	}

	// create dataCom handle
	m_hDataCome = CreateEvent(NULL, false, false, NULL);
	if (m_hDataCome == NULL)
	{
		glog(Log::L_INFO, L"Failed to create stop event in Write Thread");
		return false;
	}
	
	// initialize the communication with automation
	bool bResult = m_pCommunication->Initialize();
	if(!bResult)
	{
		char errMsg[512];
		int  errCode;
		m_pCommunication->GetLastError(&errCode, errMsg, 512);

		wchar_t wszMsg[512];
		memset(wszMsg, 0x00, 512*sizeof(wchar_t));
		mbstowcs(wszMsg, errMsg, 512);

		glog(Log::L_INFO, L"%s [ErrorCode: %d]", wszMsg, errCode);
		
		glog(Log::L_INFO, L"初始化串口失败");
		
		sleep (1000);
		
		m_pSMSSrv->OnUnInit();

		return false;
	}

	glog(Log::L_DEBUG, L"初始化串口成功");

	return true;
}

int SMSComPortWriteThread::run(void)
{
	glog(Log::L_DEBUG, L"SMSComPortWriteThread::run");

	bool bContinue = true;
	DWORD dwWaitStatus;

	HANDLE handles[2] = { m_hStopEvent, m_hDataCome };

	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			bContinue = false;
			break;

		// received the read overlap event
		case WAIT_OBJECT_0 + 1:
			sendCmdToComPort();
			break;

		// received timeout or failed, exit the thread.
		case WAIT_TIMEOUT:
			break;
		case WAIT_FAILED:
		default:
			bContinue = false;
			break;
		}
	}
	
	return 1;
}

void SMSComPortWriteThread::final(void)
{
	glog(Log::L_INFO, L"SMSComPortWriteThread::final");
	
	// free the memory in queue
	while (m_QueueWriteMsgStr.size() > 0)
	{
		m_QueueWriteMsgStr.pop();
	}

	if (m_pCommunication != NULL)
	{
		delete m_pCommunication;
	}
	m_pCommunication = NULL;

	//close stop event handle
	if(m_hStopEvent != NULL)
	{
		CloseHandle(m_hStopEvent);
	}
	m_hStopEvent = NULL;

	if(m_hDataCome != NULL)
	{
		CloseHandle(m_hDataCome);
	}
	m_hDataCome = NULL;
}

void SMSComPortWriteThread::AddWriteMsg(string msg)
{
	// add data to queue
	m_QueueWriteMsgStr.push(msg);
}
//////////////////////////////////////////////////////////////////////////
CComCommunication* 
SMSComPortWriteThread::GetComCommunication()
{
	glog(Log::L_DEBUG, "SMSComPortWriteThread::GetComCommunication");
	return m_pCommunication;
}

bool SMSComPortWriteThread::stopWriteThd()
{
	return SetEvent(m_hStopEvent);
}

void SMSComPortWriteThread::setDataCome()
{
	// notify processing thread to process the data
	SetEvent(m_hDataCome);
}
void SMSComPortWriteThread::sendCmdToComPort()
{
	if (!m_QueueWriteMsgStr.empty())
	{
		if((GetTickCount() - m_calc) >= SMSSRV_TIMEOUT)
		{
			setPduMode();
			m_calc = GetTickCount();
			sleep(2000);
		}
		
		string strCmd = m_QueueWriteMsgStr.front();
		const char* pcCmd = strCmd.c_str();
		
		char cCmd[160];
		memset(cCmd, 0x00, 160*sizeof(char));
		strcpy(cCmd, pcCmd);
		glog(Log::L_DEBUG, "写入串口的数据是:  <%s>", cCmd);
		m_pCommunication->WriteComm(cCmd, strlen(cCmd));
		
		m_QueueWriteMsgStr.pop();
	}
}

void SMSComPortWriteThread::setPduMode()
{
	char cmd[16];
	memset(cmd, 0x00, 16*sizeof(char));
	
	// In PDU mode
	sprintf(cmd, "at+cmgf=0\r");
	m_pCommunication->WriteComm(cmd, strlen(cmd));
	glog(Log::L_DEBUG, "设置PDU模式: <%s>", cmd);
}
