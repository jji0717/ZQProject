#include "stdafx.h"
#include ".\buffersend.h"
#include "channel.h"
#include "scqueue.h"
#include "ChannelManager.h"

CBufferSend::CBufferSend()
{
	m_wSendType=1;
	m_cSourceIp[0]='\0';
	m_cDestIp[0]='\0';
	m_wDestPort=0;
	m_nIndex=0;
	m_pManager=NULL;
//	
//	m_StopFlag=FALSE;
	m_TCPSocke=NULL;
	m_pMulitcastSocket=NULL;
	m_wSourcePort=2000;
	m_SendSize=0;
	m_nToralperiod=0;
	LARGE_INTEGER liFreq;
	QueryPerformanceFrequency(&liFreq);
	g_nFreq=liFreq.QuadPart;
	m_pSendBuf=NULL;

	m_ISWriteFile=FALSE;
	if( m_ISWriteFile)
	{
		SYSTEMTIME		St;
		GetLocalTime(&St);
		sprintf(m_outfileName,"C:\\log\\%02d%02d%02dtmp.ts",St.wHour,St.wMinute,St.wSecond);
		HANDLE b_hFile=CreateFile(m_outfileName,GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
		if (b_hFile)
		{
			CloseHandle(b_hFile);
		}
	}
}

CBufferSend::~CBufferSend()
{
	if (m_pSendBuf)
	{
		delete[] m_pSendBuf;
		m_pSendBuf=NULL;
	}
}
HRESULT CBufferSend::Stop(void)  
{
	//char szMsg[MAX_PATH];

	SetEvent(m_hStopEvent);
	//sprintf(szMsg,"Stop Port:m_hStopEvent destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	Sleep(1);	
	CallWorker(0); 
	//sprintf(szMsg,"Stop Port:CallWorker destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	DeleteCriticalSection(&m_SendCriticalSection);

	//sprintf(szMsg,"Stop Port:DeleteCriticalSection destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	m_Socket.Close();

	//sprintf(szMsg,"Stop Port:Socket.Close destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	if(m_TCPSocke)
	{
		m_TCPSocke->Close();
		delete m_TCPSocke;
		m_TCPSocke=NULL;
	}
	//sprintf(szMsg,"Stop Port:delete m_TCPSocke destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

// ------------------------------------------------------ Modified by zhenan_ji at 2005年4月12日 15:21:27

	if(m_pMulitcastSocket)
	{
		m_pMulitcastSocket->Leave();
		m_pMulitcastSocket->Close();
		delete m_pMulitcastSocket;
		m_pMulitcastSocket=NULL;
	}

	//sprintf(szMsg,"Stop Port:Mulitcast destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);


	Close();
	//sprintf(szMsg,"Stop Port:Close destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	m_ccArr.clear();

	//sprintf(szMsg,"Stop Port:clear destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	if (m_pSendBuf)
	{
		delete[] m_pSendBuf;
		m_pSendBuf=NULL;
	}	
	//sprintf(szMsg,"Stop Port:delete[] m_pSendBuf destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	if (m_hStopEvent)
	{
		CloseHandle(m_hStopEvent);
		m_hStopEvent=NULL;
	}
	//sprintf(szMsg,"Stop Port:CloseHandle destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);

	return S_OK;
}

void CBufferSend::SendData(CHAR* pBuffer,int len) 
{
	if(m_ISWriteFile)
	{
		DWORD dwWritten=0;
		HANDLE b_hFile = CreateFile(m_outfileName,
			GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (b_hFile == INVALID_HANDLE_VALUE) 
		{

			glog(ISvcLog::L_DEBUG,"Could not open %s\n", m_outfileName);
			glog(ISvcLog::L_ERROR,"m_pIO->GetData return %s", m_outfileName);
			// DbgLog((LOG_TRACE, 2, TEXT("Could not open %s\n"), m_outfileName));
			// DbgLog((LOG_ERROR,1,TEXT("m_pIO->GetData return %s"),m_outfileName));
			return ;
		}

		SetFilePointer (b_hFile, 0, NULL, FILE_END);
		WriteFile(b_hFile, (PVOID)pBuffer, (DWORD)len,&dwWritten, NULL); 
		CloseHandle(b_hFile); 
	}



	//char strLog[MAX_PATH];

	if(m_wSendType==1)
	{			
		//sprintf(strLog,"SendData to server before:%s - %d",m_cDestIp,m_wDestPort);	
		//LogMyEvent(2,1,strLog);

		if(m_Socket.Send(pBuffer,len,m_destAddr.s_addr, m_wDestPort)==SCS_FAILED)
		{
			char strLog[MAX_PATH];
			sprintf(strLog,"SendData to server FAILED:%s - %d",m_cDestIp,m_wDestPort);
			LogMyEvent(1,DODERROR_BUFFERSENDERROR,strLog);
		}
		//sprintf(strLog,"SendData to server end:%s - %d",m_cDestIp,m_wDestPort);	
		//LogMyEvent(2,1,strLog);


	}else
		if(m_wSendType==0)
		{
			if(m_TCPSocke!=NULL)
			{
				if(SCS_FAILED==(m_TCPSocke->Send(pBuffer,len)))
				{
#ifdef NEED_EVENTLOG
					char strLog[MAX_PATH];
					sprintf(strLog,"SendData to server FAILED:%s - %d",m_cDestIp,m_wDestPort);
					LogMyEvent(1,DODERROR_BUFFERSENDERROR,strLog);
#endif					
				}
			}
		}	
		else
			if(m_wSendType==2)
			{
				if(m_pMulitcastSocket!=NULL) 
				{
					if(SCS_FAILED==(m_pMulitcastSocket->Send(pBuffer,len)))
					{
#ifdef NEED_EVENTLOG
						char strLog[MAX_PATH];
						sprintf(strLog,"SendData to server FAILED:%s - %d",m_cDestIp,m_wDestPort);
						LogMyEvent(1,DODERROR_BUFFERSENDERROR,strLog);
#endif					
					}
				}

			}		
	if (m_pSendBuf==NULL) 
		return;
	memset(m_pSendBuf,0xff,m_SendSize);
	for(int i=0;i<(m_SendSize)/PACKET_SIZE;i++)
	{
		if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
		{
			return;
		}
		if (m_pSendBuf==NULL) 
			return;
		m_pSendBuf[i*PACKET_SIZE]=0x47;
		m_pSendBuf[i*PACKET_SIZE+1]=0x1f;
		m_pSendBuf[i*PACKET_SIZE+2]=0xff;
		m_pSendBuf[i*PACKET_SIZE+3]=0x10;
	}
	//sprintf(strLog,"SendData to server return:%s - %d",m_cDestIp,m_wDestPort);	
	//LogMyEvent(2,1,strLog);


}
DWORD CBufferSend::ThreadProc()
{
    BOOL bShutDown = FALSE;

    while (1)
    {
        DWORD req = GetRequest();       

		if (req==1)
		{
			Reply(S_OK);
			Process();
		}
		else
			if(req==0)
			{
				Reply(S_OK);
				return 0;
			}
			else
			{
				Reply(S_OK);
			} 
	}
	return 1;
}
HRESULT CBufferSend::Init(void)  
{
	if(m_wSourcePort==0 || m_wDestPort==0)
		return 0;

	if(strlen(m_cSourceIp)==0 || strlen(m_cDestIp)==0)
		return 0;
	char strLog[MAX_PATH];

	//int j=(m_SendSize *1024 +188*2)/PACKET_SIZE;

	if(m_wSendType==2)
	{
		if(m_pMulitcastSocket==NULL) 
		{
			m_pMulitcastSocket = new CSCMulticastSocket;
			m_pMulitcastSocket->Create();
//			strcpy(m_cSourceIp,"");
//			strcpy(m_cSourceIp,"192.8.8.8");
//			strcpy(m_cSourceIp,"192.168.80.206");
#ifdef NEED_EVENTLOG
			char strLog[MAX_PATH];
			sprintf(strLog,"pMulitcastSocket Join Dest=(%s %d),SourceIp=%s",m_cDestIp,m_wDestPort,m_cSourceIp);
			LogMyEvent(1,DODERROR_MULITCASTSOCKETJOINERROR,strLog);
#endif	
			if(SCS_FAILED==(m_pMulitcastSocket->Join(m_cDestIp,m_wDestPort,m_wSourcePort,m_cSourceIp)))
			{
				//connect success
#ifdef NEED_EVENTLOG
				sprintf(strLog,"m_pMulitcastSocket setsockopt error - %d",m_cDestIp,m_wDestPort);
				LogMyEvent(1,DODERROR_MULITCASTSOCKETJOINERROR,strLog);
#endif	
			}
		}
	}
	else if(m_wSendType==1)
	{
		int result=m_Socket.Create();
		sprintf(strLog,"Socket.bind IP, SourceIp:%s port:%d",m_cSourceIp,m_wDestPort);		LogMyEvent(1,1,strLog);
		if (0 != result)
		{
			int dd=WSAGetLastError();
			sprintf(strLog,"Socket.Create error, SourceIp:%s port:%d, error: %d",m_cSourceIp,dd);			LogMyEvent(1,1,strLog);
			return INVALID_SOCKET;
		}
		m_Socket.BindAndOtherPara(m_cSourceIp,m_wDestPort);
		sprintf(strLog,"Destination address: DestIp:%s Destport:%d",m_cDestIp,m_wDestPort);		LogMyEvent(1,1,strLog);
	}

	m_destAddr.s_addr = inet_addr(m_cDestIp);

	RefreshParameter();
	return CallWorker(2); 
}
HRESULT CBufferSend::RefreshParameter(void)
{
	if (m_pManager->m_nTotalPacketNumber >0)
		m_SendSize=m_pManager->m_nTotalPacketNumber*188;
	return NOERROR;
}
HRESULT CBufferSend::Run(void)   
{	
	InitializeCriticalSection(&m_SendCriticalSection);
	m_ccArr.clear();

	//800K ,it nee send 350 packets once agin
	m_hStopEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	for(int i=0;i<m_pManager->m_nPinNumber;i++)	
	{
		CCInfo1 arr;
		if (m_pManager->m_pChannelArray[i]->m_nRate >0)
			arr.nSendPeriod=m_pManager->m_pChannelArray[i]->m_nblockSize / m_pManager->m_pChannelArray[i]->m_nRate +1;
		arr.nSendPeriod=40;
		arr.m_bEnable=m_pManager->m_pChannelArray[i]->m_bEnable;
		arr.nCC=0;
		arr.pid=m_pManager->m_pChannelArray[i]->m_nPID;

		m_ccArr.push_back(arr);
		if (m_pManager->m_pChannelArray[i]->m_nStreamCount >0)
		{
			for (int kk=0;kk< (m_pManager->m_pChannelArray[i]->m_nStreamCount);kk++)
			{

				CCInfo1 arr2;
				if (m_pManager->m_pChannelArray[i]->m_nRate >0)
					arr2.nSendPeriod=m_pManager->m_pChannelArray[i]->m_nblockSize / m_pManager->m_pChannelArray[i]->m_nRate +1;
				arr2.nSendPeriod=40;
				arr2.m_bEnable=m_pManager->m_pChannelArray[i]->m_bEnable;
				arr2.nCC=0;
				arr2.pid=m_pManager->m_pChannelArray[i]->m_nPID + kk + 1;
				m_ccArr.push_back(arr2);
			}
		}
	}

	/*
	char *lpwszFileName="pgmts.ts";
	HANDLE b_hFile = CreateFile("e:\\ttt.ts",
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (b_hFile == INVALID_HANDLE_VALUE) 
	{
		DbgLog((LOG_TRACE, 2, TEXT("Could not open %s\n"), lpwszFileName));
		DbgLog((LOG_ERROR,1,TEXT("m_pIO->GetData return %s"),lpwszFileName));
		return E_OUTOFMEMORY;
	}
	char *g_pbMem = new char[188*7*34];
	DWORD read=0;
	m_Socket.Create(); 

	TCHAR szMsg[MAX_PATH];
	LARGE_INTEGER liEnd;
	LONGLONG lBack=0;
	int iTick = GetTickCount();
	int iDelay = iTick;
	long aaaa;
	int i=0;
	for(;;)
	{	
		
		iDelay = GetTickCount();
		if( (iDelay - iTick) >= 100 )
		{
			iTick = GetTickCount();

			//count = 0;

			//QueryPerformanceCounter(&liEnd);
			ReadFile(b_hFile, g_pbMem, 188*7*34, &read, NULL); 
			if(read <188*7*34)
			{
				SetFilePointer (b_hFile, 0, NULL, FILE_BEGIN);
				continue;
			}

			for(i=0;i<34;i++)
				SendData(g_pbMem+i*188*7,188*7);
		}


	//	aaaa=(liEnd.QuadPart - lBack)*1000/g_nFreq;
	//	lBack=liEnd.QuadPart;
	//	if(aaaa >4)
		//	TRACE("\n v=%d,n=%d",aaaa);

	//	{	wsprintf(szMsg,"liEnd.QuadPart=%ld  ",aaaa);		LogToFile(2,szMsg);}
	}

	return 1;
*/

	//m_StopFlag=FALSE;
	HRESULT hr;
	if (!ThreadExists()) {		

		// start thread
		if (!Create()) {
			return E_FAIL;
		}
		hr=Init();
	}

	return CallWorker(1); 
}

#ifndef _NO_FIX_DOD

#define SEND_PERIOD		1000

void CBufferSend::Process(void)
{
	//create a socket
	int curlen=0;
	// LARGE_INTEGER lTimeEnd;
	// LARGE_INTEGER lTimeBack;
	
	DWORD nBeforeSend, nAfterSend, nAfterSleep;

	int i=0,k;
	int Channelcount=m_pManager->m_nPinNumber;
	// BOOL bIsNeedSend=FALSE;
	
	int Timeinterval=0;
	LONGLONG patpmtpart=0;
	int patpmt_cc=1;	
	unsigned char PatPmtBuffer[PATPMTBUFFERLEN];
	int patpmt_interver=30;
	memcpy(PatPmtBuffer,m_pManager->m_PatPmtBuffer,188*2);
	TCHAR szMsg[MAX_PATH];

	int nDebtTime = 0;

	if (m_pSendBuf)
	{
		delete[] m_pSendBuf;
		m_pSendBuf=NULL;
	}
	if (m_SendSize>0)
	{
		m_pSendBuf=new BYTE[m_SendSize];
		memset(m_pSendBuf,0xff,m_SendSize);
	}

	sprintf(szMsg,"TotalSendSize = %d,nToralperiod=%d ",m_SendSize,m_nToralperiod);	
	LogMyEvent(3,1,szMsg);

	sprintf(szMsg," g_nFreq=%ld",g_nFreq);	
	LogMyEvent(3,1,szMsg);

	sprintf(szMsg,"big circle for TotalSendSize = %d",m_SendSize);	
	LogMyEvent(3,1,szMsg);

	// 填空包
	for(i=0;i<(m_SendSize)/PACKET_SIZE;i++)
	{
		if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
		{
			return;
		}
		m_pSendBuf[i*PACKET_SIZE]=0x47;
		m_pSendBuf[i*PACKET_SIZE+1]=0x1f;
		m_pSendBuf[i*PACKET_SIZE+2]=0xff;
		m_pSendBuf[i*PACKET_SIZE+3]=0x10;
	}

	
	for(;;)
	{
		// QueryPerformanceCounter(&lTimeBack);
		nBeforeSend = timeGetTime();

		if(m_wSendType==0)
		{
			if(m_TCPSocke==NULL)
			{
				m_TCPSocke = new CSCTCPSocket;
				while(1)
				{		
					if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
					{
						return;
					}
					if ( m_TCPSocke->Connect(m_cDestIp,m_wDestPort) == SCS_SUCCESS)
					{  
						//connect success
#ifdef NEED_EVENTLOG
						char strLog[MAX_PATH];
						sprintf(strLog,"Connect to server success:%s - %d",m_cDestIp,m_wDestPort);
						LogMyEvent(1,DODERROR_SOCKETCREATSUCCESS,strLog);
#endif	
						break;
					}
				}
				Sleep( 3000 );
			}
		}

		if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0) {
			sprintf(szMsg,"Stop Port:thrid StopEvent destIP:%s destport:%d",m_cDestIp,m_wDestPort);	
			LogMyEvent(1,1,szMsg);
			return;
		}

		curlen = 0;

		// 改 pat 与 pmt 的 sequence
		BYTE* pBuf=PatPmtBuffer;
		pBuf[3]=(pBuf[3]&0xf0)|patpmt_cc;
		pBuf[PACKET_SIZE+3]=(pBuf[PACKET_SIZE+3]&0xf0)|patpmt_cc;
		if(patpmt_cc<15)
			patpmt_cc++;
		else
			patpmt_cc=0;

		memcpy(m_pSendBuf+curlen,pBuf,PACKET_SIZE * 2);
		curlen = PACKET_SIZE * 2;

		for(i=0;i<Channelcount;i++)
		{
			if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
			{
				sprintf(szMsg,"Stop Port:first StopEvent destIP:%s destport:%d",m_cDestIp,m_wDestPort);	
				LogMyEvent(1,1,szMsg);
				return;
			}

			if(m_pManager->m_pChannelArray[i]->m_bEnable == FALSE)
			{
				sprintf(szMsg,"channel(%d).m_bEnable is FALSE ",m_pManager->m_pChannelArray[i]->m_nPID);	
				LogMyEvent(1,1,szMsg);
				continue;
			}

			CChannel *ch=m_pManager->m_pChannelArray[i];
			
			// EnterCriticalSection(&m_SendCriticalSection);
			CSCMemoryBlockQueue* queue=ch->m_Queue[m_nIndex];
			FredPtr block;
			if (!queue->Pop(block, 0) || block == NULL) {
				LeaveCriticalSection(&m_SendCriticalSection);
				continue;
			}

			int length = block->GetSize(); 
			glog(ISvcLog::L_DEBUG_DETAIL,"block GetSize %d", length);
			memcpy(m_pSendBuf+curlen,block->GetBlock(),length);
			block = NULL;
			// LeaveCriticalSection(&m_SendCriticalSection);

			// wsprintf(szMsg,"queue->Pop queue Size:%d",queue->Size());
			// LogMyEvent(3,1,szMsg);

			// 修正 sequence
			BYTE *buffer=m_pSendBuf+curlen; 
			for(k = 0;k < length / PACKET_SIZE; k ++)
			{
				USHORT nPID=(buffer[k*PACKET_SIZE+1]& 0x1f)<<8 | buffer[k*PACKET_SIZE+2];	
				if (nPID==0x1fff)
					continue;
				USHORT ncc=-1;
				ncc=GetContinueCount(nPID);
				if (ncc==-1)
				{
					wsprintf(szMsg,"The PID of the data which come from datawrapp is not found! error",nPID);
					LogMyEvent(1,1,szMsg);
					return ;
				}

				buffer[k*PACKET_SIZE+3]=(buffer[k*PACKET_SIZE+3] & 0xf0)|ncc;
			}

			curlen += length;							
		}

		
		DWORD nBeofreSend2 = timeGetTime();

		glog(ISvcLog::L_DEBUG_DETAIL, 
			"Before SendData() PmtPid = %d, IP = %s, Port = %u, SendSize = %d", 
			m_pManager->m_nPMTPID, m_cDestIp, m_wDestPort, m_SendSize);

		SendData((CHAR*)m_pSendBuf, m_SendSize);
		nAfterSend = timeGetTime();

		glog(ISvcLog::L_DEBUG_DETAIL, 
			"After SendData() PmtPid = %d, IP = %s, Port = %u, Duration = %u", 
			m_pManager->m_nPMTPID, m_cDestIp, m_wDestPort, 
			nAfterSend - nBeofreSend2);

		// QueryPerformanceCounter(&lTimeEnd);		

		// int nDuration = (int)((lTimeEnd.QuadPart - lTimeBack.QuadPart) * 1000 / g_nFreq);
		int nDuration = nAfterSend - nBeforeSend;
		if (nDuration < 0)
			nDuration *= -1;

		//glog(ISvcLog::L_DEBUG_DETAIL, "TimeBack = %llu, TimeEnd = %llu, nDuration = %d", 
		//	lTimeBack.QuadPart, lTimeEnd.QuadPart, nDuration);

		// const int nRepair = 1; // 补偿 1 ms
		long nSleepTime = m_nToralperiod - nDuration + nDebtTime /* - nRepair */;

		glog(ISvcLog::L_DEBUG_DETAIL, "PmtPid = %d, nBeforeSend = %u, nAfterSend = %u, "
			"nDuration = %d, nSleepTime = %d, nDebtTime = %d", m_pManager->m_nPMTPID,
			nBeforeSend, nAfterSend, nDuration, nSleepTime, nDebtTime);
		
		if (nSleepTime > 0) {
			Sleep(nSleepTime);
			nAfterSleep = timeGetTime();
			int nSleepCost = nAfterSleep - nAfterSend;
			if (nSleepCost < 0)
				nSleepCost *= -1;

			nDebtTime = nSleepTime - nSleepCost;
			glog(ISvcLog::L_DEBUG_DETAIL, "PmtPid = %d, nSleepTime = %d, nSleepCost = %d", 
				m_pManager->m_nPMTPID, nSleepTime, nSleepCost);

		} else {
			nDebtTime = nSleepTime;
		}

		if (nDebtTime < -SEND_PERIOD) {
			nDebtTime = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
#else
void CBufferSend::Process(void)
{
	//create a socket
	int curlen=0;
	LARGE_INTEGER lTimeEnd;
	LARGE_INTEGER lTimeBack;
	int i=0,k;
	int Channelcount=m_pManager->m_nPinNumber;
	BOOL bIsNeedSend=FALSE;

	int Timeinterval=0;
	LONGLONG patpmtpart=0;
	int patpmt_cc=1;	
	unsigned char PatPmtBuffer[PATPMTBUFFERLEN];
	int patpmt_interver=30;
	memcpy(PatPmtBuffer,m_pManager->m_PatPmtBuffer,188*2);
	TCHAR szMsg[MAX_PATH];
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月24日 11:08:43

	int Tmp_Current_Once=1;	 

	if (m_pSendBuf)
	{
		delete[] m_pSendBuf;
		m_pSendBuf=NULL;
	}

	if (m_SendSize>0)
	{
		m_pSendBuf = new BYTE[m_SendSize];

		if (m_pSendBuf==NULL) {
			sprintf(szMsg,"m_pSendBuf==NULL,new() failed. ");	
			LogMyEvent(1,1,szMsg);
			return;
		}

		memset(m_pSendBuf,0xff,m_SendSize);
	}

	sprintf(szMsg,"TotalSendSize = %d,nToralperiod=%d ",m_SendSize,m_nToralperiod);	LogMyEvent(1,1,szMsg);
	sprintf(szMsg," g_nFreq=%ld",g_nFreq);	LogMyEvent(1,1,szMsg);

	for(i=0;i<(m_SendSize)/PACKET_SIZE;i++)
	{
		if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
		{
			return;
		}
		m_pSendBuf[i*PACKET_SIZE]=0x47;
		m_pSendBuf[i*PACKET_SIZE+1]=0x1f;
		m_pSendBuf[i*PACKET_SIZE+2]=0xff;
		m_pSendBuf[i*PACKET_SIZE+3]=0x10;
	}
	QueryPerformanceCounter(&lTimeBack);

	for(;;)
	{
		
		sprintf(szMsg,"big circle for TotalSendSize = %d",m_SendSize);	LogMyEvent(2,1,szMsg);

		if(m_wSendType==0)
		{
			if(m_TCPSocke==NULL)
			{
				m_TCPSocke = new CSCTCPSocket;
				while(1)
				{		
					if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
					{
						return;
					}
					if ( m_TCPSocke->Connect(m_cDestIp,m_wDestPort) == SCS_SUCCESS)
					{  
						//connect success
#ifdef NEED_EVENTLOG
						char strLog[MAX_PATH];
						sprintf(strLog,"Connect to server success:%s - %d",m_cDestIp,m_wDestPort);
						LogMyEvent(1,DODERROR_SOCKETCREATSUCCESS,strLog);
#endif	
						break;
					}
				}
				Sleep( 3000 );
			}
		}

		if(QueryPerformanceCounter(&lTimeEnd)==FALSE)
		{
			wsprintf(szMsg,"QueryPerformanceCounter new version errorcode.=%d ",GetLastError());		LogMyEvent(1,1,szMsg);

		}

		wsprintf(szMsg,"QueryPerformanceCounter new version liEnd.=%ld ",lTimeEnd.QuadPart);		LogMyEvent(2,1,szMsg);
		wsprintf(szMsg,"lBack=%ld",lTimeBack.QuadPart);		LogMyEvent(2,1,szMsg);

		if(bIsNeedSend==TRUE)
		{
			if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
			{
				sprintf(szMsg,"Stop Port:thrid StopEvent destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);
				return;
			}
			BYTE* pBuf=PatPmtBuffer;
			pBuf[3]=(pBuf[3]&0xf0)|patpmt_cc;
			pBuf[PACKET_SIZE+3]=(pBuf[PACKET_SIZE+3]&0xf0)|patpmt_cc;
			if(patpmt_cc<15)
				patpmt_cc++;
			else
				patpmt_cc=0;

			memcpy(m_pSendBuf+curlen,pBuf,PACKET_SIZE*2);
			curlen+=PACKET_SIZE*2;
			for(i=0;i<Channelcount;i++)
			{
				if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
				{
					sprintf(szMsg,"Stop Port:first StopEvent destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);
					return;
				}
				if(m_pManager->m_pChannelArray[i]->m_bEnable == FALSE)
				{
					sprintf(szMsg,"channel(%d).m_bEnable is FALSE ",m_pManager->m_pChannelArray[i]->m_nPID);	LogMyEvent(1,1,szMsg);
					continue;
				}
				CChannel *ch=m_pManager->m_pChannelArray[i];

				if(1)
				{
					EnterCriticalSection(&m_SendCriticalSection);
					CSCMemoryBlockQueue* queue=ch->m_Queue[m_nIndex];
					// ------------------------------------------------------ Modified by zhenan_ji at 2005年5月31日 11:50:06
					//	wsprintf(szMsg,"qSize=%d",queue->Size());		LogToFile(2,szMsg);
					if (queue==NULL || queue->Size()==0)
					{
						LeaveCriticalSection(&m_SendCriticalSection);	
						wsprintf(szMsg,"queue(%x)==NULL || queue->Size==0",(DWORD)queue);		LogMyEvent(2,1,szMsg);
						continue;
					}
					FredPtr block = queue->Front();
					int length = block->GetSize(); 
					wsprintf(szMsg,"block GetSize %d",length);		LogMyEvent(2,1,szMsg);
					memcpy(m_pSendBuf+curlen,block->GetBlock(),length);
					queue->Pop();
					LeaveCriticalSection(&m_SendCriticalSection);		
					wsprintf(szMsg,"queue->Pop queue Size:%d",queue->Size());		LogMyEvent(2,1,szMsg);

					BYTE *buffer=m_pSendBuf+curlen; 
					for(k=0;k<length/PACKET_SIZE;k++)
					{
						USHORT nPID=(buffer[k*PACKET_SIZE+1]& 0x1f)<<8 | buffer[k*PACKET_SIZE+2];	
						if (nPID==0x1fff)
							continue;
						USHORT ncc=-1;
						ncc=GetContinueCount(nPID);
						if (ncc==-1)
						{
							wsprintf(szMsg,"The PID of the data which come from datawrapp is not found! error",nPID);		LogMyEvent(1,1,szMsg);
							return ;
						}							
						buffer[k*PACKET_SIZE+3]=(buffer[k*PACKET_SIZE+3]&0xf0)| ncc;
					}
					curlen+=length;							
				}
			}
		}

		int xxx=(int)((lTimeEnd.QuadPart - lTimeBack.QuadPart)*1000/g_nFreq);
		wsprintf(szMsg,"time intervale xxx =%d  curlen=%d",xxx,curlen);	LogMyEvent(2,1,szMsg);
		if (xxx <=0)
		{
			xxx=xxx * (-1);
		}

		//		if((liEnd.QuadPart - lBack)*1000/g_nFreq >= (m_nToralperiod-Tmp_Current_Once))
		if(xxx >= (m_nToralperiod-Tmp_Current_Once))
		{	
			/*wsprintf(szMsg,"liEnd.QuadPart=%ld  ",(liEnd.QuadPart - lBack)*1000/g_nFreq);	
			LogMyEvent(1,1,szMsg);
			Timeinterval=(liEnd.QuadPart - lBack)*1000/g_nFreq;
			if(Timeinterval < 5000 && Timeinterval>2500 )
			{
			#ifdef NEED_EVENTLOG
			char strLog[MAX_PATH];
			sprintf(strLog,"Opposite server could not be discover");
			LogMyEvent(1,DODERROR_SOCKETCREATSUCCESS,strLog);
			#endif	
			return ;
			}*/
			wsprintf(szMsg,"Send before:DestIp:%s,Port:%d,",m_cDestIp,m_wDestPort);						LogMyEvent(2,1,szMsg);
			SendData((CHAR*)m_pSendBuf,m_SendSize);			
			wsprintf(szMsg,"Send after;cDestIp:%s,Port:%d,",m_cDestIp,m_wDestPort);						LogMyEvent(2,1,szMsg);

			lTimeBack=lTimeEnd;
			curlen=0;
			bIsNeedSend=TRUE;
		}
		else
			bIsNeedSend=FALSE;
		if(WaitForSingleObject(m_hStopEvent,0)==WAIT_OBJECT_0)
		{
			sprintf(szMsg,"Stop Port:seconde StopEvent destIP:%s destport:%d",m_cDestIp,m_wDestPort);	LogMyEvent(1,1,szMsg);
			return;
		}
		Sleep(1);
	}
}

#endif

int CBufferSend::GetContinueCount(USHORT pid)
{
	for (int i=0;i<(int)(m_ccArr.size());i++)
	{
		if (pid == m_ccArr[i].pid)
		{
			if(m_ccArr[i].nCC<15)
				(m_ccArr[i].nCC)++;
			else
				m_ccArr[i].nCC=0;
			return m_ccArr[i].nCC;
		}		
	}
	return -1;
}
