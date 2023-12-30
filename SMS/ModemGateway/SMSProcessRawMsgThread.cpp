//#include "stdafx.h"
#include "SMSProcessRawMsgThread.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "SMSContentProcessReq.h"

#include "SMSComPortWriteThread.h"

#include "Log.h"
#include "ScLog.h"

#define ECHO_OFF 0
#define SMSSRV_TIMEOUT 100000

SMSProcessRawMsgThread::
SMSProcessRawMsgThread(SMSComPortWriteThread* pWriteThread, ModemGateway* pModemGateway)
:NativeThread()
{
	m_comportWriteThread = pWriteThread;
	m_ModemGateway = pModemGateway;
	m_list = 0;
	m_read = 0;
	m_echo = ECHO_OFF;
}

SMSProcessRawMsgThread::~SMSProcessRawMsgThread()
{
}

bool SMSProcessRawMsgThread::init(void)
{	
	glog(Log::L_DEBUG, "SMSProcessRawMsgThread::init");
	
	// create stop event
	m_hStopEvent = CreateEvent(NULL, false, false, NULL);
	if(m_hStopEvent == NULL)
	{
		glog(Log::L_INFO, "Failed to create stop event in Process Raw Msg thread");
		return false;
	}

	// create dataCom semaphore
	m_hDataComeSem = CreateSemaphore(NULL, 0, MAX_SEM_VALUE, NULL);
	if(m_hDataComeSem == NULL)
	{
		glog(Log::L_INFO, "Failed to create DataCome Semaphore in Process Raw Msg thread");
		return false;
	}

	m_sequenceID = -1;
	
	return true;
}

extern NativeThreadPool pool(5);//需注意

int SMSProcessRawMsgThread::run(void)
{
	glog(Log::L_DEBUG, "SMSProcessRawMsgThread::run");

	bool bContinue = true;
	DWORD dwWaitStatus;

	HANDLE handles[2] = { m_hStopEvent, m_hDataComeSem };

	char dbpath[100];
	memset(dbpath, 0x00, 100*sizeof(char));
	
	//m_ModemGateway->getConfig(&m_overtime, path, &m_echo);
	
	m_overtime = m_ModemGateway->getOvertime();
	wcstombs(dbpath, m_ModemGateway->getDBPath(), 100);
	m_echo = m_ModemGateway->getEcho();

	m_db = new SMSDB();
	
	if (m_db)
	{
		WCHAR sError[200];
		memset(sError, 0x00, 200*sizeof(WCHAR));
		if (m_db->ConnectDB(dbpath, sError))
		{
			glog(Log::L_DEBUG, "打开数据库 <%s> 成功", dbpath);
			
			m_dbId = m_db->GetUID();
			glog(Log::L_INFO, L"Get uid <%d> from DB", m_dbId);
			listDB();
		}
		else
		{
			glog(Log::L_DEBUG, L"打开数据库 <%s> 失败", sError);
			m_ModemGateway->OnUnInit();
		}
	}
	
	sleep(500);
	
	InitializeSMSModem();

	while(bContinue)
	{
		//具体的时间待定
		dwWaitStatus = WaitForMultipleObjects(2, handles, false, SMSSRV_TIMEOUT);
		switch(dwWaitStatus)
		{
		// received stop event, the thread will stop
		case WAIT_OBJECT_0:
			bContinue = false;
			break;

		// received the read event
		case WAIT_OBJECT_0 + 1:
			ProcessRawMsgStr();
			break;
		case WAIT_TIMEOUT:
			deleteSMS();
			listSMS();
			break;
		default:
			bContinue = false;
			break;
		}
	}
	
	return 1;
}

void SMSProcessRawMsgThread::final(void)
{
	glog(Log::L_INFO, "SMSProcessRawMsgThread::final");

	// free the memory in queue
	while (m_QueueRawMsgStr.size() > 0)
	{
		glog(Log::L_INFO, "SMSProcessRawMsgThread: free queue");
		m_QueueRawMsgStr.pop();
	}

	if (m_db != NULL)
	{
		glog(Log::L_INFO, "SMSProcessRawMsgThread: delete db");
		delete m_db;
	}
	m_db = NULL;

	// close stop event handle
	if(m_hDataComeSem != NULL)
	{
		glog(Log::L_INFO, "SMSProcessRawMsgThread: close data come handle");
		CloseHandle(m_hDataComeSem);
		m_hDataComeSem = NULL;
	}

	// close stop event handle
	if(m_hStopEvent != NULL)
	{
		glog(Log::L_INFO, "SMSProcessRawMsgThread: close stop handle");
		CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}
}

void SMSProcessRawMsgThread::AddRawMsg(string cstRawMsg)
{
	// add data to queue
	m_QueueRawMsgStr.push(cstRawMsg);

	setDataCome();
}

bool SMSProcessRawMsgThread::stopProcessRawMsgThd()
{
	return SetEvent(m_hStopEvent);
}

void SMSProcessRawMsgThread::setDataCome()
{
	// notify processing thread to process the data
	long lPreCount;
	ReleaseSemaphore(m_hDataComeSem, 1, &lPreCount);
	
}

//////////////////////////////////////////////////////////////////////////
void SMSProcessRawMsgThread::ProcessRawMsgStr()
{
	string strRawMsg = m_QueueRawMsgStr.front();
	m_QueueRawMsgStr.pop();
	const char* pRawMsg = strRawMsg.c_str();
	glog(Log::L_DEBUG, "COM DATA:\n%s\n", pRawMsg);
	ProcessOK(pRawMsg);// process "OK"
	ProcessSMSListCmd(pRawMsg);//process "CMGL"
	ProcessSMSContent(pRawMsg);//process "CMGR"
	ProcessSMSComeFlag(pRawMsg);//process "CMTI"
}
void SMSProcessRawMsgThread::ProcessOK(const char* pRawMsg)
{
	char SMSOK[] = "OK";
	char* pOK;
	pOK = strstr(pRawMsg, SMSOK);
	if (pOK)
	{
		m_comportWriteThread->setDataCome();
	}
}
//处理新短信的标志 CMTI
void SMSProcessRawMsgThread::ProcessSMSComeFlag(const char* pRawMsg)
{
	int SIMIndex = 0;
	char SMSComeFlag[] = "CMTI";
	char* pCMTI;
	pCMTI = strstr(pRawMsg, SMSComeFlag);
	while (pCMTI)
	{
		glog(Log::L_DEBUG, "Process SMS Come Flag(CMTI)");
		SIMIndex = getSMSIndex(pCMTI);
		
		//create read cmd
		readSMS(SIMIndex);
		//read Come Array again, because maybe there are some DataComeFlags in Come Array.
		pCMTI = pCMTI+12;
		pCMTI = strstr(pCMTI, SMSComeFlag);
	}
}
//处理短信的内容 CMGR
void SMSProcessRawMsgThread::ProcessSMSContent(const char* pRawMsg)
{
	char contentComeFlag[] = "CMGR";
	char* pCMGR;
	
	if (m_read != 0)
	{
		strncat(m_readRawSMS, pRawMsg, m_read);
		m_read = 0;
		SMSContentProcessReq* p = new SMSContentProcessReq(pool, 
														   m_ModemGateway, 
														   m_readRawSMS);
		p->start();
		m_sequenceID = m_sequenceID + 2;
	}
	
	int SMSLength = 0;
	pCMGR = strstr(pRawMsg, contentComeFlag);
	while (pCMGR)
	{
		glog(Log::L_DEBUG, L"Process SMS content (CMGR)");
		SMSLength = getRawContentLength(pCMGR);
		//read raw content
		memset(m_readRawSMS, 0x00, 320*sizeof(char));
		strncpy(m_readRawSMS, pCMGR+13, SMSLength);//read raw SMS
		glog(Log::L_DEBUG, "rawSMS: %s", m_readRawSMS);
		
		m_read = SMSLength - strlen(m_readRawSMS);
		
		if (m_read == 0)
		{
			//create NativeRequest to process rawContent
			SMSContentProcessReq* p = new SMSContentProcessReq(pool, 
															   m_ModemGateway,
															   m_readRawSMS);
			p->start();
			m_sequenceID = m_sequenceID + 2;
		}
		
		//read Come Array again, because maybe there are some Content in Come Array.
		pCMGR = pCMGR + 12;
		pCMGR = strstr(pCMGR, contentComeFlag);
	}
}

//处理短信列举命令 CMGL
void SMSProcessRawMsgThread::ProcessSMSListCmd(const char* pRawMsg)
{
	char listSMSCmdFlg[] = "CMGL";
	char* pCMGL;

	if (m_list != 0)
	{
		strncat(m_listRawSMS, pRawMsg, m_list);
		m_list = 0;
		SMSContentProcessReq* p = new SMSContentProcessReq(pool, 
														   m_ModemGateway, 
														   m_listRawSMS);
		p->start();
		m_sequenceID = m_sequenceID + 2;
		sleep(1000);
	}
	
	int listSMSLength = 0;
	pCMGL = strstr(pRawMsg ,listSMSCmdFlg);
	while (pCMGL)
	{
		glog(Log::L_DEBUG, "Process SMS List Cmd (CMGL)");
		int flag = -2;
		listSMSLength = getListSMSLength(pCMGL, &flag);
		memset(m_listRawSMS, 0x00, 320*sizeof(char));
		strncpy(m_listRawSMS, pCMGL+15+flag, listSMSLength);//read raw SMS
		
		m_list = listSMSLength - strlen(m_listRawSMS);

		if (m_list == 0)
		{
			glog(Log::L_DEBUG, "listRawSMS: %s", m_listRawSMS);

			//create NativeRequest to process rawContent
			SMSContentProcessReq* p = new SMSContentProcessReq(pool,
															   m_ModemGateway, 
															   m_listRawSMS);
			p->start();
			m_sequenceID = m_sequenceID + 2;
		}
		//read Come Array again, because maybe there are some Content in Come Array.
		pCMGL = pCMGL + 15;
		pCMGL = strstr(pCMGL, listSMSCmdFlg);
	}
}

///////////  function   ///////////////////////////////////////////////////////
int SMSProcessRawMsgThread::getSMSIndex(char* pCMTI)
{
	int result = 0;
	char index1 = *(pCMTI+11);//在CMTI后面11，12位是该信息在SIM卡中的序号，
	char index2 = *(pCMTI+12);//1-9的话是一位，10-20的话是二位。
	if ( ((index2-48)<=9) && ((index2-48)>=0) )//2位
	{
		result = (index1-48)*10 + (index2-48);
	}
	else//1位
	{
		result = index1-48;
	}
	glog(Log::L_DEBUG, L"SIM Index: %d", result);
	return result;
}

int SMSProcessRawMsgThread::getRawContentLength(char* pCMGR)
{
	int result = 0;
	char number1 = *(pCMGR + 9);
	char number2 = *(pCMGR + 10);
	char number3 = *(pCMGR + 11);
	if ( ((number2-48)<=9) && ((number2-48)>=0) )
	{
		if ( ((number3-48)<=9) && ((number3-48)>=0) )
		{
			result = (number1-48)*100 + (number2-48)*10 + number3;
		}
		else
		{
			result = (number1-48)*10 + (number2-48);
		}	
	}
	else
	{
		result = number1 - 48;
	}
	result = result * 2 + 18;
	glog(Log::L_DEBUG, L"Raw Content Length : %d", result);
	return result;
}
int SMSProcessRawMsgThread::getListSMSLength(char* pCMGL, int* flag)
{
	int result = 0;
	
	char number1 = *(pCMGL + 11);
	char number2 = *(pCMGL + 12);
	char number3 = *(pCMGL + 13);
	char number4 = *(pCMGL + 15);
	if ( ((number1-48)<=9) && ((number1-48)>=0) )
	{
		result *= 10;
		result += number1 - 48;
		(*flag) ++;

		if ( ((number2-48)<=9) && ((number2-48)>=0) )
		{
			result *= 10;
			result += number2 - 48;
			(*flag) ++;
		}
		
		if ( ((number3-48)<=9) && ((number3-48)>=0) )
		{
			result *= 10;
			result += number3 - 48;
			(*flag) ++;
		}
	}
	else if ( ((number2-48)<=9) && ((number2-48)>=0) )
	{
		result *= 10;
		result += number2 - 48;
		(*flag) ++;
		(*flag) ++;
		
		if ( ((number3-48)<=9) && ((number3-48)>=0) )
		{
			result *= 10;
			result += number3 - 48;
			(*flag) ++;
		}
		
		if ( ((number4-48)<=9) && ((number4-48)>=0) )
		{
			result *= 10;
			result += number4 - 48;
			(*flag) ++;
		}
	}
	
	result = result * 2 + 18;
	glog(Log::L_DEBUG, L"List Content Length : %d", result);
	return result;
}


void SMSProcessRawMsgThread::readSMS(int SIMIndex)
{
	if (SIMIndex > 0)
	{
		char readCmd[20];
		memset(readCmd, 0x00, 20*sizeof(char));
		sprintf(readCmd, "at+cmgr=%d\n", SIMIndex);
		//put read cmd in write com queue
		m_comportWriteThread->AddWriteMsg(readCmd);
		glog(Log::L_DEBUG, "ReadCmd: %s", readCmd);
	}
	else
	{
		listSMS();
	}
}

void SMSProcessRawMsgThread::listDB()
{
	glog(Log::L_DEBUG, "SMSProcessRawMsgThread::listDB");

	int		packagelen, mode, id;
	char	telephone[12];
	char	entryTime[20];
	char	content[260];
	char	ticpContent[260];
	bool	ticpFinished;
	bool	smsFinished;
	memset(telephone,   0x00, 12*sizeof(char));
	memset(entryTime,   0x00, 20*sizeof(char));
	memset(content,     0x00, 260*sizeof(char));
	memset(ticpContent, 0x00, 260*sizeof(char));
	
	CTime current = CTime::GetCurrentTime();
	current = current - CTimeSpan(0, m_overtime, 0, 0);

	char ctime[20];
	memset(ctime, 0x00, 20*sizeof(char));
	sprintf(ctime, "%d-%02d-%02d %02d:%02d:%02d", current.GetYear()
												, current.GetMonth()
												, current.GetDay()
												, current.GetHour()
												, 0, 0);

	WCHAR sError[200];
	memset(sError, 0x00, 200*sizeof(WCHAR));
	if (!m_db->SelectUnfinished(ctime, sError))
	{
		if (wcslen(sError) != 0)
		{
			glog(Log::L_DEBUG, "%s", sError);
		}
	}
	else
	{
		while (m_db->getData(packagelen, mode, id,
							 NULL,
							 telephone,
							 entryTime,
							 content,
							 ticpContent,
							 NULL,
							 NULL,
							 ticpFinished,
							 smsFinished))
		{
			SMSContentProcessReq* p = new SMSContentProcessReq(pool,
															   m_ModemGateway,  
															   id,
															   mode,
															   telephone, 
															   entryTime, 
															   ticpContent, 
															   1);
			p->start();
		}
	}
}

void SMSProcessRawMsgThread::InitializeSMSModem()
{
	char cmd[16];
	memset(cmd, 0x00, 16*sizeof(char));
	
	// ECHO OFF
	sprintf(cmd, "ATE%d\r", m_echo);
	m_comportWriteThread->AddWriteMsg(cmd);
	glog(Log::L_DEBUG, "ECHO OFF: %s", cmd);

	listSMS();
}

void SMSProcessRawMsgThread::listSMS()
{
	char cmd[16];
	
	memset(cmd, 0x00, 16*sizeof(char));
	// In PDU mode
	sprintf(cmd, "at+cmgf=0\r");
	m_comportWriteThread->AddWriteMsg(cmd);
	
	glog(Log::L_DEBUG, "设置PDU模式: %s", cmd);

	memset(cmd, 0x00, 16*sizeof(char));

	//list Unread SMS
	sprintf(cmd, "at+cmgl=0\r");
	
	//sprintf(cmd, "at+cmgl=1\r");//just for test
	m_comportWriteThread->AddWriteMsg(cmd);
	
	glog(Log::L_DEBUG, "List Unread Cmd: %s", cmd);
}

void SMSProcessRawMsgThread::deleteSMS()
{
	char deleteCmd[20];
	memset(deleteCmd, 0x00, 20*sizeof(char));
	sprintf(deleteCmd, "AT+CMGD=1,1\n");
	m_comportWriteThread->AddWriteMsg(deleteCmd);
	glog(Log::L_DEBUG, "删除命令: %s", deleteCmd);
}

int SMSProcessRawMsgThread::getSequenceID()
{
	m_sequenceID = m_sequenceID + 2;
	return m_sequenceID;
}

int SMSProcessRawMsgThread::getDBID()
{
	return  m_dbId++;
}
