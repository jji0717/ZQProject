//#include "stdafx.h"
#include "SMSComPortReadThread.h"
#include "NativeThread.h"
#include "ComCommunication.h"
#include "SMSProcessRawMsgThread.h"
#include "SMSComPortWriteThread.h"
#include "Log.h"
#include "ScLog.h"


SMSComPortReadThread::SMSComPortReadThread(CComCommunication* pComm, SMSProcessRawMsgThread* processThread, SMSComPortWriteThread* pWriteCom)
:NativeThread()
{
	m_pCommunication = pComm;//获得Com的指针
	m_pWriteCom = pWriteCom;
	m_pProcessRawMsg = processThread;
}

SMSComPortReadThread::~SMSComPortReadThread()
{
	if (m_hDataComeFromComPort != NULL)
	{
		CloseHandle(m_hDataComeFromComPort);
	}
	m_hDataComeFromComPort = NULL;
	
	// close stop event handle
	if(m_hStopEvent != NULL)
	{
		CloseHandle(m_hStopEvent);
	}
	m_hStopEvent = NULL;
}

bool SMSComPortReadThread::init(void)
{
	glog(Log::L_DEBUG, L"SMSComPortReadThread::init");

	m_hStopEvent = CreateEvent(NULL, false, false, NULL);
	if(m_hStopEvent == NULL)
	{
		glog(Log::L_INFO, L"Failed to create stop event in Read thread");
		return false;
	}
	
	return true;
}

int SMSComPortReadThread::run(void)
{
	glog(Log::L_DEBUG, L"SMSComPortReadThread::run");

	DWORD dwReceivedBytes = 0;
	m_dwBytesToRead = 128;

	memset(m_dataBuffer, 0x00, 1024*sizeof(char));
	
	if(!m_pCommunication->ReadComm(m_dataBuffer, m_dwBytesToRead))
	{
		// log error
		char errMsg[512];
		int  errCode;
		m_pCommunication->GetLastError(&errCode, errMsg, 512);

	    wchar_t wszMsg[512];
		memset(wszMsg, 0x00, 512*sizeof(wchar_t));
		mbstowcs(wszMsg, errMsg, 512);

		glog(Log::L_INFO, L"%s [ErrorCode: %d]", wszMsg, errCode);
		
		return false;
	}

	glog(Log::L_DEBUG, "Succeed to ReadData");
	int nHandleCount;
	m_pCommunication->GetBlockHandleForReading(&m_hDataComeFromComPort, &nHandleCount);
	
	HANDLE hEvents[2];
	hEvents[0] = m_hStopEvent;
	hEvents[1] = m_hDataComeFromComPort;

	DWORD dwWaitStatus = 0;

	bool bContinue = true;

	while(bContinue)
	{
		dwWaitStatus = WaitForMultipleObjects(2, hEvents, false, 4000);

		switch(dwWaitStatus)
		{
		// receive stop event, stop this thread
		case WAIT_OBJECT_0:
			stopReadThd();
			bContinue = false;
			break;
			
		// received the read overlap event
		case WAIT_OBJECT_0 + 1:			
			if(m_pCommunication->CheckOverlappedResult(&m_dwBytesToRead))
			{
				m_pProcessRawMsg->AddRawMsg(m_dataBuffer);
				memset(m_dataBuffer, 0x00, 1024*sizeof(char));
			}
			m_dwBytesToRead = 128;
			if(!m_pCommunication->ReadComm(m_dataBuffer, m_dwBytesToRead))
			{
				glog(Log::L_INFO, "Failed to read next data");
			}
			break;
		case WAIT_TIMEOUT:
			m_pWriteCom->setDataCome();
			break;

		default:
			bContinue = false;
			break;
		}
	}
	
	return 1;
}

void SMSComPortReadThread::final(void)
{
	glog(Log::L_INFO, L"SMSComPortReadThread::final");
}

bool SMSComPortReadThread::stopReadThd()
{
	return SetEvent(m_hStopEvent);
}
