#include "stdafx.h"
#include "ChannelManager.h"
#include "ZQBroadcastFilter.h"
#include "channel.h"
#include "buffersend.h"

#define QUOTIENT		0x04c11db7
#define MAX_PID_NUMBER		15

unsigned long Crc32(unsigned long crc,unsigned char* buf,int len)
{
	unsigned char       octet;
	for (int i=0; i<len; i++)
	{
		octet = *(buf++);
		for (int j=0; j<8; j++)
		{
			if ((octet >> 7) ^ (crc >> 31))
				crc = (crc << 1) ^ QUOTIENT;
			else
				crc = (crc << 1);
			octet <<= 1;
		}
	}
	return crc;             
}

CChannelManager::CChannelManager():
m_pFilter(NULL),
//m_hFile(INVALID_HANDLE_VALUE),
m_pFileName(0)
{	
	m_nState=0;
	m_pFileName2=NULL;
	
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);	
	m_nPinNumber=0;
	m_nPortNumber=0;
	m_nTotalRate=0;
	m_cDirName[0]='\0';
	InitializeCriticalSection (&csMyCriticalSection);

	m_pPinArray=NULL;
	m_pChannelArray=NULL;
	m_pSendArray=NULL;
// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月29日 14:33:16
	// m_nTotalperiod=35;
	m_checkTotalRate=0;
	m_nTotalPacketNumber=0;
	m_nPMTPID = 0;

	// modified by cary
	m_nTotalperiod = 33;
}

CChannelManager::~CChannelManager()
{
//	Stop();
	//	if(m_pSendArray[i])

	for(int i=0;i<m_nPortNumber;i++)
	{
		delete m_pSendArray[i];
		m_pSendArray[i] = NULL;
	}			

	for(int i=0;i<m_nPinNumber;i++)
	{
		if(m_pChannelArray[i])
		{
			delete m_pChannelArray[i];
			m_pChannelArray[i] = NULL;
		}		

		if(m_pPinArray[i])
		{
			delete m_pPinArray[i];
			m_pPinArray[i] = NULL;
		}
	}	

	if (m_pSendArray)
	{
		delete m_pSendArray;
		m_pSendArray=NULL;
	}
	if (m_pChannelArray)
	{
		delete m_pChannelArray;
		m_pChannelArray=NULL;
	}
	if (m_pPinArray)
	{
		delete m_pPinArray;
		m_pPinArray=NULL;
	}

}
HRESULT CChannelManager::CreatePatPmt()
{
	unsigned char *Buf=m_PatPmtBuffer;
	int i=0,k;
	memset((void *)m_PatPmtBuffer,0xff,PATPMTBUFFERLEN);	

	BOOL EnableWriteFile=FALSE;

	SYSTEMTIME		St;
	GetLocalTime(&St);
	char strdddddLog[MAX_PATH];
	sprintf(strdddddLog,"d:\\%02d%02d%02dpat.ts",St.wHour,St.wMinute,St.wSecond);

	HANDLE b_hFile;
	if(EnableWriteFile)
		b_hFile= CreateFile(strdddddLog,GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);

	// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月17日 9:20:20
	//pat
	Buf[0]=0x47;
	Buf[1]=0x40;
	Buf[2]=0x00;
	Buf[3]=0x11;
	Buf[4]=0x00;
	Buf[5]=0x00;

	int sectionlen=9,streamId=2,versionNo =1,currentNext=1;
	int CRCbegin=0;
	sectionlen+=4;//CRC
	sectionlen+=4;//programnumber

	i=6;
	//	m_pat->sectionLength=0x11;
	Buf[i++]=0xb0|(sectionlen>>8);CRCbegin=i-2;
	Buf[i++]=sectionlen & 0xff;	
	Buf[i++]=streamId >> 8;
	Buf[i++]=streamId & 0xff;	
	Buf[i++]=(currentNext | (versionNo)<<1) | 0xc0;			
	Buf[i++]=0x00;//sectionnumber
	Buf[i++]=0x00;//lastsectionnumber
	Buf[i++]=0x00;
	Buf[i++]=0x00;
	Buf[i++]=0xe0;
	Buf[i++]=0x10;
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月17日 9:04:00

	Buf[i++]=0x00;
	Buf[i++]=0x03;
	Buf[i++]=m_nPMTPID >>8 | 0xe0;
	Buf[i++]=m_nPMTPID & 0xff;	


	unsigned long logic_crc=Crc32(-1,Buf+CRCbegin,(int)(sectionlen-1));
	Buf[i++]=(unsigned char)((logic_crc &0xff000000)>>24);
	Buf[i++]=(unsigned char)((logic_crc &0x00ff0000)>>16);
	Buf[i++]=(unsigned char)((logic_crc &0x0000ff00)>>8);
	Buf[i++]=(unsigned char)(logic_crc &0x000000ff);
	// pmt
	//(m_pChannelArray[k-1]->m_nPID
	Buf=m_PatPmtBuffer+188;
	Buf[0]=0x47;
	Buf[1]=0x40 |(m_nPMTPID >>8);
	Buf[2]=m_nPMTPID & 0xff;
	Buf[3]=0x11;
	Buf[4]=0x00;
	Buf[5]=0x02;

	sectionlen=9+4;

// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月8日 15:39:23
//	sectionlen+=(5+6)*m_nPinNumber;
	int pinNumber=0;
	for(k=0;k<m_nPinNumber;k++)
	{
		if (m_pChannelArray[k]->m_nStreamCount >1)
		{
			pinNumber+=(m_pChannelArray[k]->m_nStreamCount+1);			
		}
		else
			pinNumber++;
	}
	char strLog[MAX_PATH];

	sectionlen+=(5+6)*pinNumber;
	if (MAX_PID_NUMBER < pinNumber)
	{
		sprintf(strLog,":CreatePMT: PMTPID=%d,PIDNumber=%d,PIDNumber > MAX_PID_NUMBER error",m_nPMTPID,pinNumber);
		LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
		return 1;
	}
	i=6;
	Buf[i++]=0xb0|(sectionlen>>8);CRCbegin=i-2;
	Buf[i++]=sectionlen & 0xff;	
	Buf[i++]=0x00;
	Buf[i++]=0x03; 
	currentNext=1;versionNo=1;
	Buf[i++]=(currentNext | (versionNo)<<1) | 0xc0;			
	Buf[i++]=0x00;
	Buf[i++]=0x00;
	Buf[i++]=0xff ;
// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月15日 11:00:56

	Buf[i++]=0xff;	
	//  itemm->pro_info_len=0;
	Buf[i++]=0xf0;
	Buf[i++]=0x00;

	for(k=0;k<m_nPinNumber;k++)
	{
		sprintf(strLog,":CreatePMT:ElementPID=%d :StreamType=%d,m_nStreamCount=%d",m_pChannelArray[k]->m_nPID,m_pChannelArray[k]->m_nStreamType,m_pChannelArray[k]->m_nStreamCount);
		LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);

		Buf[i++]=m_pChannelArray[k]->m_nStreamType;
		Buf[i++]=0xe0 |(m_pChannelArray[k]->m_nPID >>8);
		Buf[i++]=m_pChannelArray[k]->m_nPID & 0xff;	
		int es_length=6;
		Buf[i++]=0xf0 |(es_length >>8);
		Buf[i++]=es_length & 0xff;

		Buf[i++]=m_pChannelArray[k]->m_DescTag;
		Buf[i++]=4;
		memcpy(Buf+i,m_pChannelArray[k]->m_Descrbuff,4);
		i+=4;

		if (m_pChannelArray[k]->m_nStreamCount >1)
		{
			for(int kk=1;kk<m_pChannelArray[k]->m_nStreamCount+1;kk++)
			{
			//	sprintf(strLog,":ElementPID=%d :StreamType=%d",(m_pChannelArray[k]->m_nPID+kk),(m_pChannelArray[k]->m_nStreamType+kk));
			//	LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);

				Buf[i++]=(m_pChannelArray[k]->m_nStreamType+kk);
				Buf[i++]=0xe0 |((m_pChannelArray[k]->m_nPID+kk) >>8);
				Buf[i++]=(m_pChannelArray[k]->m_nPID+kk) & 0xff;	
				int es_length=6;
				Buf[i++]=0xf0 |(es_length >>8);
				Buf[i++]=es_length & 0xff;

				Buf[i++]=m_pChannelArray[k]->m_DescTag;
				Buf[i++]=4;
				memcpy(Buf+i,m_pChannelArray[k]->m_Descrbuff,4);
				i+=4;
			}
		}
	}	

	logic_crc=Crc32(-1,Buf+CRCbegin,(int)(sectionlen-1));
	Buf[i++]=(unsigned char)((logic_crc &0xff000000)>>24);
	Buf[i++]=(unsigned char)((logic_crc &0x00ff0000)>>16);
	Buf[i++]=(unsigned char)((logic_crc &0x0000ff00)>>8);
	Buf[i++]=(unsigned char)(logic_crc &0x000000ff);
	if(EnableWriteFile)
	{
		DWORD dwWritten=0;
		for(k=0;k<10;k++)
		{
			WriteFile(b_hFile, (PVOID)(m_PatPmtBuffer), (DWORD)376,
				&dwWritten, NULL); 
		}
		CloseHandle(b_hFile); 
		b_hFile=NULL;
	}
	return NOERROR;
}
HRESULT CChannelManager::SetBufferModeLen(int nIndex,int RecvLength)
{
	CChannel *pchannel=m_pChannelArray[nIndex];

	if (pchannel==NULL)
		return 1;
	EnterCriticalSection (&(pchannel->m_channelCriticalSection));

	if(pchannel->m_bFirstIsUsing)
		 pchannel->m_npBackDataLen=RecvLength;
	else
		 pchannel->m_npCurrDataLen=RecvLength;

	LeaveCriticalSection (&(pchannel->m_channelCriticalSection));

	return NOERROR;
}
HRESULT CChannelManager::changefileFlag(int index,DWORD dwFileSize,char *strfilename)
{	
	if(index >m_nPinNumber)
		return 1;
	CChannel *pchannel=m_pChannelArray[index];

	if (pchannel==NULL)
		return 1;

	char strLog[MAX_PATH];	
//	sprintf(strLog,"PID=%d:pchannel->m_nRepeatTime (%d)  ",pchannel->m_nPID,pchannel->m_nRepeatTime);	LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);

	if ((pchannel->m_nblockSize >0) && (dwFileSize>0) && (pchannel->m_nRepeatTime>m_nTotalperiod))
	{

		int m_nNeedDataOnce;
		int m_nNeedTotalOnce;

		float ftemp=(float)((float)( pchannel->m_nRepeatTime )/(float)(m_nTotalperiod) * (float)(pchannel->m_nblockSize));		
		ftemp=ftemp/(float)PACKET_SIZE;
		m_nNeedTotalOnce=(int)ftemp;

		m_nNeedDataOnce=dwFileSize / PACKET_SIZE;

		sprintf(strLog,"PID=%d:current file size=(%d), nRepeatTime(%d),Need total data (%d)  ",pchannel->m_nPID,dwFileSize,pchannel->m_nRepeatTime,m_nNeedTotalOnce);
		LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
		if (m_nNeedTotalOnce >m_nNeedDataOnce)
		{
			BYTE TmpBuffer[PACKET_SIZE];
			BYTE PaddingBuf[PACKET_SIZE];
			int m_nNeedPaddingOnce;
			memset((void *)PaddingBuf,0xff,PACKET_SIZE);	
			PaddingBuf[0]=0x47;
			PaddingBuf[1]=0x1f;
			PaddingBuf[2]=0xff;
			PaddingBuf[3]=0x10;

			int FirstInsetPad=1;
			m_nNeedPaddingOnce=m_nNeedTotalOnce - m_nNeedDataOnce;
			if (m_nNeedDataOnce > m_nNeedPaddingOnce)
			{
				FirstInsetPad=m_nNeedDataOnce / m_nNeedPaddingOnce;
				if (m_nNeedDataOnce % m_nNeedPaddingOnce!=0)
					FirstInsetPad++;							 
			}
			else
			{
				FirstInsetPad=m_nNeedPaddingOnce / m_nNeedDataOnce;
				if (m_nNeedPaddingOnce % m_nNeedDataOnce!=0)
					FirstInsetPad++;	
			}



			sprintf(strLog,"PID=%d:padding(%d) is necessary,filePacket is %d ",pchannel->m_nPID,m_nNeedPaddingOnce,m_nNeedDataOnce);
			LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
			if (strfilename ==NULL)
			{	
				sprintf(strLog,"PID=%d:strfilename is Null ",pchannel->m_nPID);
				LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
				return 1;
			}

			char tempfilename[MAX_PATH+3];
			strcpy(tempfilename,strfilename);
			strcat(tempfilename,"tmp");
			try
			{
				rename(strfilename,tempfilename);
			}
			catch (...) 
			{
				DWORD dwErr = GetLastError();
				sprintf(strLog,"PID=%d:rename error errorcode=%d",pchannel->m_nPID,dwErr);
				LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
				return 1;
			}

			HANDLE hDestFile = CreateFile(strfilename, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
			if (hDestFile == INVALID_HANDLE_VALUE) 
			{			
				DWORD dwErr = GetLastError();
				wsprintf(strLog,"PID=%d:CreateFile error strfilename=(%s) errorcode=%d",pchannel->m_nPID,strfilename,dwErr);			
				return HRESULT_FROM_WIN32(dwErr);
			}

			HANDLE hSourceFile = CreateFile(tempfilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
			if (hSourceFile == INVALID_HANDLE_VALUE) 
			{			
				DWORD dwErr = GetLastError();
				wsprintf(strLog,"PID=%d:CreateFile error tempfilename=(%s) errorcode=%d",pchannel->m_nPID,tempfilename,dwErr);			
				return HRESULT_FROM_WIN32(dwErr);
			}
			//DWORD dwFileSize = GetFileSize(hSourceFile, NULL);
			//sprintf(strLog,"PID=%d: dwFileSize == %d ",pchannel->m_nPID,dwFileSize);
			//LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);

			BOOL bFileOver=FALSE;
			BOOL bIsFirstPadding=TRUE;
			BOOL bPaddingOver=FALSE;

			DWORD dwWritten=0;
			DWORD nRead=PACKET_SIZE;
			int nCurrentInsetPadding=0;
			int nRealInstertPadding=0;
			int i=0;
			try 
			{
				//sprintf(strLog,"PID=%d: into try  ",pchannel->m_nPID);
				//LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);

				while (bFileOver==FALSE || bPaddingOver==FALSE)
				{
					if (bFileOver==FALSE)
					{
						ReadFile(hSourceFile, TmpBuffer, PACKET_SIZE, &nRead, NULL);
						//sprintf(strLog,"PID=%d: read == %d ",pchannel->m_nPID,nRead);
						//LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
						if (nRead <=0)
							bFileOver=TRUE;
						else							
						{
							//sprintf(strLog,"PID=%d: WriteFile   ",pchannel->m_nPID);
							//LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);

							WriteFile(hDestFile, (PVOID)(TmpBuffer), (DWORD)nRead,&dwWritten, NULL);
						}
					}

					if (bPaddingOver)
						continue;
					if (FirstInsetPad + nCurrentInsetPadding >= m_nNeedPaddingOnce)
					{
						nRealInstertPadding=m_nNeedPaddingOnce-nCurrentInsetPadding;
						bPaddingOver=TRUE;
					}
					else
					{
						if (bIsFirstPadding)
							nRealInstertPadding=FirstInsetPad;
						else
							nRealInstertPadding=FirstInsetPad-1;

						bIsFirstPadding=!bIsFirstPadding;
					}
					nCurrentInsetPadding+=nRealInstertPadding;

					for (i=0;i<nRealInstertPadding;i++)
						WriteFile(hDestFile, (PVOID)(PaddingBuf), (DWORD)PACKET_SIZE,&dwWritten, NULL);
				}
				CloseHandle(hSourceFile);
				CloseHandle(hDestFile);
				DeleteFile(tempfilename);
			}
			catch (...)
			{
				DWORD dwErr = GetLastError();
				wsprintf(strLog,"PID=%d:insert padding error errorcode=%d",pchannel->m_nPID,dwErr);			
				return HRESULT_FROM_WIN32(dwErr);
			}
		}		
	}

//zhenan 20060913 add lock for seting  FileChangeFlag;
	EnterCriticalSection (&(pchannel->m_channelCriticalSection));

	pchannel->m_bFileChangeFlag=TRUE;
	sprintf(strLog,"PID=%d:changefile ok",pchannel->m_nPID);
	LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);

	LeaveCriticalSection (&(pchannel->m_channelCriticalSection));
	return NOERROR;
}
HRESULT CChannelManager::Run()
{
	for(int k=0;k<m_nPinNumber;k++)
	{
        CChannel *pchannel=m_pChannelArray[k];
        if(pchannel !=NULL)	
			pchannel->Run(); // run data prepare thread
		else {
			glog(ISvcLog::L_CRIT, "%:s\tchannel(%d) is NULL", __FUNCTION__, k);
			return 1;
		}
		
	}
	
	for(k=0;k<m_nPortNumber;k++)
	{
		CBufferSend	*pSen=m_pSendArray[k];	
		if(pSen !=NULL)	{
			pSen->Run();
		}
		else {
			glog(ISvcLog::L_CRIT, "%:s\tsender(%d) is NULL", __FUNCTION__, k);
			return 1;
		}
	}

	return NOERROR;	

}
PBYTE CChannelManager::GetBufferModeBuf(int nIndex)
{
	if(nIndex >=m_nPinNumber)
	{
#ifdef NEED_EVENTLOG
	char strLog[MAX_PATH];
	sprintf(strLog,"GetBufferModeBuf error");
	LogMyEvent(1,DODERROR_BUFFERINDEX,strLog);
#endif	
		return NULL;
	}
	BYTE * TempBuffer=NULL;

	CChannel *pchannel=m_pChannelArray[nIndex];
	EnterCriticalSection (&(pchannel->m_channelCriticalSection));
	if(pchannel->m_bFirstIsUsing)
		TempBuffer=pchannel->m_pcBackData;
	else
		TempBuffer=pchannel->m_pcCurrData;

	LeaveCriticalSection (&(pchannel->m_channelCriticalSection));
	return TempBuffer;
}
int CChannelManager::GetFileName(char *cFileName,int pinIndex)
{
	char strLog[MAX_PATH];	

	if(pinIndex >=m_nPinNumber)
		return 1;

	CChannel *pchannel=m_pChannelArray[pinIndex];
	EnterCriticalSection (&(pchannel->m_channelCriticalSection));

	if(pchannel->m_bFirstIsUsing)
		strcpy(cFileName,pchannel->m_BackFileName);
	else
		strcpy(cFileName,pchannel->m_CurrFileName);

//zhenan 20060815 modified FileChangeFlag to false;
	if (pchannel->m_bFileChangeFlag)
	{
		pchannel->m_bFileChangeFlag = FALSE;
		sprintf(strLog,"PID=%d :GetFileName:(last time filename)%s",pchannel->m_nPID,cFileName);
		LogMyEvent(1,1,strLog);
	}

	LeaveCriticalSection (&(pchannel->m_channelCriticalSection));
	return NOERROR;
}

HRESULT CChannelManager::Stop()
{
	char strLog[MAX_PATH];
	for(int k=0;k<m_nPortNumber;k++)
	{
		CBufferSend	*pSen=m_pSendArray[k];
		if (pSen)			
		{
			sprintf(strLog,"Stop Port: destIP:%s destport:%d",pSen->m_cDestIp,pSen->m_wDestPort);
			LogMyEvent(1,1,strLog);
			pSen->Stop();	
			Sleep(1);
		}
	}	

	for(k=0;k<m_nPinNumber;k++)
	{
		CChannel *pchannel=m_pChannelArray[k];
		if (pchannel)
		{
			sprintf(strLog,"Stop channel,ChannelPID %d",pchannel->m_nPID);
			LogMyEvent(1,1,strLog);
			pchannel->Stop();
		}
		Sleep(1);
	}

	sprintf(strLog,"CChannelManager Stop all channel");
	LogMyEvent(1,1,strLog);

	return NOERROR;
}

BOOL CChannelManager::GetPinType(int nIndex)
{
	CChannel *pchannel=m_pChannelArray[nIndex];
	if(pchannel)
		return pchannel->m_bIsFileMode;

	return TRUE;
}
int CChannelManager::CreatePin(int pinIndex,char ctype,int PID)
{  
	HRESULT phr=NOERROR;
	char  szFile[20];
	wsprintf(szFile,"pin PID=%d",pinIndex+1);
	LPOLESTR wstrMediaFile;
	USES_CONVERSION;
	wstrMediaFile = A2W(szFile);

	CBroadcastPin *pPin=new CBroadcastPin("PinName",m_pFilter,&phr,wstrMediaFile);
	pPin->m_pChannelManager=this;
	pPin->m_nIndex=pinIndex;
	m_pPinArray[pinIndex]=pPin;
	pPin->m_bPinEnable=TRUE;
	return NOERROR;
}