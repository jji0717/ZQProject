#include "stdafx.h"
#include <commdlg.h>
#include <initguid.h>
#include <stdio.h>
#include <string.h>
#include <atlconv.h>

#include <atlbase.h>
#include "resource.h"
#include <vector>

#include "BroadcastGuid.h"
#include "ZQBroadcastFilter.h"
#include "channel.h"
#include "buffersend.h"
#include "ConfigPage.h"
#include "ChannelManager.h"
typedef CBroadcastPin* PBROADCASTPIN;
typedef CChannel* PBROASCHANNEL;
typedef CBufferSend* PBROASSEND;
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_Stream,            // Major type
	&MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins[] =
{
	{	L"Input",                   // Pin string name
		FALSE,                      // Is it rendered
		FALSE,                      // Is it an output
		FALSE,                      // Allowed none
		FALSE,                      // Likewise many
		&CLSID_NULL,                // Connects to filter
		L"Output",                  // Connects to pin
		1,                          // Number of types
		&sudPinTypes                // Pin information
	}
};

const AMOVIESETUP_FILTER sudRender =
{
	&CLSID_ZQBroadcast,			// Filter CLSID
		L"ZQBroadcast",                   // String name
		MERIT_DO_NOT_USE,           // Filter merit
		1,                          // Number pins
		sudPins                    // Pin details
};

//
//  Object creation stuff
//
CFactoryTemplate g_Templates[]= {
	{	
		L"ZQBroadcast", &CLSID_ZQBroadcast, CBroadcastFilter::CreateInstance, NULL,&sudRender
	},
	{	L"BML Configurations",&CLSID_SendConfigurePage,CBroadcastConfigPage::CreateInstance,NULL,NULL}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer
STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

unsigned int max_udpmsg_size;
unsigned int channel_queue_size;

#include <atlbase.h>

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{

	if (dwReason == DLL_PROCESS_ATTACH) {
		char workPath[MAX_PATH];
		GetModuleFileName(NULL, workPath, sizeof(workPath));
		char* c = workPath + strlen(workPath);
		while(c != workPath) {
			if (*c == '\\') {
				*c = 0;
				break;
			}

			c --;
		}

		strcat(workPath, "\\DataStream.ini");
		max_udpmsg_size = GetPrivateProfileInt("System","MaxPacketSize", 7 * 188, workPath);
		channel_queue_size = GetPrivateProfileInt("System","ChannelQueueSize", 32, workPath);
	}
    
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

// Constructor

CBroadcastFilter::CBroadcastFilter( LPUNKNOWN pUnk,HRESULT *phr) :
CBaseFilter(NAME("ZQBroadcast"), pUnk, &m_Lock, CLSID_ZQBroadcast)
{
	m_pChannelManager=new CChannelManager();
	m_pChannelManager->m_pFilter=this;
	m_pChannelManager->m_Lock=&m_Lock;
	m_nAllChannelRatePacketNumber=0;
}
CBroadcastFilter::~CBroadcastFilter()
{
	if(m_pChannelManager)
	{
		delete m_pChannelManager;
		m_pChannelManager = NULL;
	}	
}

STDMETHODIMP CBroadcastFilter::GetPages(CAUUID* pPages)
{
	if(!pPages)
		return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems = reinterpret_cast<GUID*>(CoTaskMemAlloc(sizeof(GUID)));
	if( pPages->pElems == NULL )
	{
		return E_OUTOFMEMORY;
	}

	*(pPages->pElems) = CLSID_SendConfigurePage;

	return S_OK;
}

CUnknown * WINAPI CBroadcastFilter::CreateInstance(LPUNKNOWN lpUnknown,HRESULT *phr)
{
	return new CBroadcastFilter(lpUnknown,phr);
}

CBasePin * CBroadcastFilter::GetPin(int n)
{
	if(m_pChannelManager==NULL)
		return NULL;

	if (n <m_pChannelManager->m_nPinNumber) {
		return m_pChannelManager->m_pPinArray[n];
	} else {
		return NULL;
	}
	return NULL;
}

int CBroadcastFilter::GetPinCount()
{
	if(m_pChannelManager==NULL)
		return NULL;

	return m_pChannelManager->m_nPinNumber;	
}

STDMETHODIMP CBroadcastFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv,E_POINTER);
	
	if(riid==IID_IRateControl)
	{
		return GetInterface((IRateControl *)this,ppv);
	}
	else if(riid == IID_IIPSetting )
	{
		return GetInterface( (IIPSetting *)this, ppv );
	}
	else if(riid == IID_ISpecifyPropertyPages )
	{
		return GetInterface( (ISpecifyPropertyPages *)this, ppv );
	}

#ifndef _NO_FIX_LOG
	else if (riid == IID_IFilterInit) {
		return GetInterface((IFilterInit *) this, ppv);
	}
#endif

	else
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	} 
} 

STDMETHODIMP CBroadcastFilter::Stop()
{
	if (m_pChannelManager)
	{
		m_pChannelManager->m_bStop=TRUE;
		m_pChannelManager->Stop();
	}

	return CBaseFilter::Stop();
}
STDMETHODIMP CBroadcastFilter::SetChannelCount(DWORD dwChannelCount)
{
	if(m_pChannelManager==NULL)
		return 1;

	if(m_pChannelManager->m_nPinNumber>0)
		return 1;
	if(dwChannelCount <0)
	{
		#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,"SetChannelCount error");
		LogMyEvent(1,DODERROR_SETCHANNELCOUNT,strLog);
		#endif	
		return DODERROR_SETCHANNELCOUNT;
	}
	m_pChannelManager->m_nPinNumber=dwChannelCount;
	
	m_pChannelManager->m_pPinArray = new PBROADCASTPIN[dwChannelCount];//CBroadcastPin[dwChannelCount];
	m_pChannelManager->m_pChannelArray = new PBROASCHANNEL[dwChannelCount];//CBroadcastPin[dwChannelCount];

	m_pChannelManager->m_bStop=FALSE;
	for(int i=0;i<(int)dwChannelCount;i++)
	{
		CChannel *pchannel=new CChannel();
		pchannel->m_pManager=m_pChannelManager;
		pchannel->m_nPID=i+10;
		pchannel->m_nRate=i*11+3;	
		m_pChannelManager->CreatePin(i,0,pchannel->m_nPID);
		m_pChannelManager->m_pChannelArray[i]=pchannel;	
	}
	return NOERROR;
}


STDMETHODIMP CBroadcastFilter::ConfigChannel(int nPinIndex, ZQSBFCHANNELINFO* pChannelInfo)
{
	if(nPinIndex <0 || nPinIndex >m_pChannelManager->m_nPinNumber)
		return 1;
//	if(m_pChannelManager->m_nTotalPacketNumber <1)
//		return 1;
	char strLog[MAX_PATH];

	CChannel *pchannel=m_pChannelManager->m_pChannelArray[nPinIndex];
	pchannel->m_nPID=pChannelInfo->wPID;
	pchannel->m_nRate=pChannelInfo->wRate;

	m_pChannelManager->m_checkTotalRate+=pChannelInfo->wRate;

	float ftemp=(float)((float)(pChannelInfo->wRate) * (float)(m_pChannelManager->m_nTotalperiod))/(float)8000;
	ftemp=ftemp/(float)188+(float)0.5;
	pchannel->m_nblockSize=(int)ftemp;
	m_pChannelManager->m_nTotalPacketNumber+=pchannel->m_nblockSize;
	sprintf(strLog,"ChannelPID=%d, bitRate=%d ,packetNum=%d",pchannel->m_nPID,pchannel->m_nRate,pchannel->m_nblockSize);
	LogMyEvent(1,DODERROR_RESET_PIDRATE,strLog);

	pchannel->m_nblockSize=188 * pchannel->m_nblockSize;
	pchannel->m_nRepeatTime=pChannelInfo->wRepeatTime;
	if(m_pChannelManager->m_checkTotalRate > m_pChannelManager->m_nTotalRate)
	{
#ifdef NEED_EVENTLOG
		sprintf(strLog,"SetChannelCount m_nRate > m_pChannelManager->m_nTotalRate error");
		LogMyEvent(1,DODERROR_RESET_PIDRATE,strLog);
#endif	
	return DODERROR_RESET_PIDRATE;
	}

	pchannel->m_bIsFileMode=(pChannelInfo->wAccessType==1)?TRUE:FALSE;
	pchannel->m_bEnable=pChannelInfo->bEnable;
	strcpy(pchannel->m_cDescriptor,pChannelInfo->cDescriptor);
	pchannel->m_nStreamType=pChannelInfo->nStreamType; 	
	pchannel->m_nStreamCount=pChannelInfo->wStreamCount;	
	return NOERROR;
} 
STDMETHODIMP  CBroadcastFilter::Enable( WORD wPID, BOOL bAnable)
{
	if(m_pChannelManager==NULL)
		return 1;

	if(wPID <0 || wPID >0x1fff)
	{

#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,"BroadcastFilter::Enable channel pid=%d error",wPID);
		LogMyEvent(1,DODERROR_INPUTPID_VALUE,strLog);
#endif	
		return DODERROR_INPUTPID_VALUE;
	}

	int dwChannelCount=m_pChannelManager->m_nPinNumber;
	for(int i=0;i<dwChannelCount;i++)
	{
		CChannel *pchannel=m_pChannelManager->m_pChannelArray[i];
		if(wPID == pchannel->m_nPID)
		{
			m_pChannelManager->m_pPinArray[i]->m_bPinEnable=bAnable;
			pchannel->m_bEnable=bAnable;
			break;
		}
	}
	return NOERROR;
}
STDMETHODIMP CBroadcastFilter::SetTotalRate(DWORD wTotalRate)
{
	if(m_pChannelManager->m_nTotalRate >0)
		return 1;
	if(wTotalRate <1)
		return 1;

	m_pChannelManager->m_nTotalRate=wTotalRate;
	m_pChannelManager->m_nTotalPacketNumber=0;
	m_nAllChannelRatePacketNumber=0;
	m_pChannelManager->m_checkTotalRate=0; 

	float ftemp=(float)((float)(wTotalRate) * (float)(m_pChannelManager->m_nTotalperiod))/(float)8000;
	ftemp=ftemp/(float)188+(float)0.5;
	m_nAllChannelRatePacketNumber=(int)ftemp;
	return NOERROR;
}
STDMETHODIMP CBroadcastFilter::SetFilePath(char *cfilepath)
{
	char strLog[MAX_PATH];
	if (cfilepath==NULL)
	{
		sprintf(strLog,"CBroadcastFilter::SetFilePath parameter is null");
		LogMyEvent(1,DODERROR_FILEPATHOVERLAP,strLog);
		return DODERROR_FILEPATHOVERLAP;
	}
	else
		strcpy(m_pChannelManager->m_cDirName,cfilepath);
	
	/*if(strlen(m_pChannelManager->m_cDirName)==0)
		wsprintf(m_pChannelManager->m_cDirName,"%s",cfilepath);
	else
	{
#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,"SetFilePath m_cDirName=%s error",cfilepath);
		LogMyEvent(1,DODERROR_FILEPATHOVERLAP,strLog);
#endif	

		return DODERROR_FILEPATHOVERLAP;
	}

	if(strlen(m_pChannelManager->m_cDirName)==0)*/

	//GetTempPath(256,m_pChannelManager->m_cDirName);

	sprintf(strLog,"DirName =%s",m_pChannelManager->m_cDirName);
	LogMyEvent(1,1,strLog);

	if(strlen(m_pChannelManager->m_cDirName)==0)
	{
#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,"DirName =%s error",m_pChannelManager->m_cDirName);
		LogMyEvent(1,DODERROR_FILEPATHOVERLAP,strLog);
#endif	
		return DODERROR_FILEPATHOVERLAP;
	}


//create file or buffer
	for(int i=0;i<m_pChannelManager->m_nPinNumber;i++)
	{
		CChannel *pchannel=m_pChannelManager->m_pChannelArray[i];
		if(pchannel->m_bIsFileMode)
		{
			SYSTEMTIME		St;
			GetLocalTime(&St);
			//stTimestamp.Format(_T("%02d/%02d %02d:%02d:%02d"),St.wMonth,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds);
            wsprintf(pchannel->m_CurrFileName,"%s\\PID%dcur%d%d%d%d%d%d.ts",m_pChannelManager->m_cDirName,pchannel->m_nPID,i+1,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds);
			HANDLE hFile;
			hFile = CreateFile(pchannel->m_CurrFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) 
			{
#ifdef NEED_EVENTLOG
				char strLog[MAX_PATH];
				sprintf(strLog,"CreatecurentFile fileName=%s error",pchannel->m_CurrFileName);
				LogMyEvent(1,DODERROR_CREATECURFILE,strLog);
#endif	
				return DODERROR_CREATECURFILE;
			}
			CloseHandle(hFile); 
			hFile=NULL;
			wsprintf(pchannel->m_BackFileName,"%s\\PID%dbak%d%d%d%d%d%d.ts",m_pChannelManager->m_cDirName,pchannel->m_nPID,i+1,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds);

		//	wsprintf(pchannel->m_BackFileName,"%s\\pinbak%d.ts",m_pChannelManager->m_cDirName,i+1);
			hFile = CreateFile(pchannel->m_BackFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) 
			{
#ifdef NEED_EVENTLOG
				char strLog[MAX_PATH];
				sprintf(strLog,"CreatebakFile fileName=%s error",pchannel->m_BackFileName);
				LogMyEvent(1,DODERROR_CREATEBAKFILE,strLog);
#endif					
				return DODERROR_CREATEBAKFILE;
			}
			CloseHandle(hFile); 
			hFile=NULL;
		}		
	}
	return NOERROR;
}
STDMETHODIMP CBroadcastFilter::SetPmtPID( WORD wPID)
{
	if(m_pChannelManager->m_nPMTPID > 0)
	{
#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,"SetPmtPID error pmtPID was set (old value = %d)", 
			m_pChannelManager->m_nPMTPID);
		LogMyEvent(1,DODERROR_PMTPID_TWICE,strLog);
#endif	
		return DODERROR_PMTPID_TWICE;
	}	

	if(m_pChannelManager->m_nPinNumber < 1) {
		glog(ISvcLog::L_DEBUG, 
			"CBroadcastFilter::SetPmtPID()\tm_nPinNumber < 1");
		return 1;
	}

	if(wPID <1 || wPID >0x1fff)
	{
#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,"SetPmtPID error PID=%d",wPID);
		LogMyEvent(1,DODERROR_PMTPID_VALUE,strLog);
#endif	
		return DODERROR_PMTPID_VALUE;
	}
	m_pChannelManager->m_nPMTPID=wPID;
	m_pChannelManager->CreatePatPmt();

	m_pChannelManager->m_nTotalPacketNumber+=2;

	if (m_nAllChannelRatePacketNumber > m_pChannelManager->m_nTotalPacketNumber)
	{
		m_pChannelManager->m_nTotalPacketNumber=m_nAllChannelRatePacketNumber;
	}	

// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月1日 11:17:20
//"2" indicate: pat,pmt padding(prevent lose packet probability)
//	if((m_nAllChannelRatePacketNumber + 2) > m_pChannelManager->m_nTotalPacketNumber)
//	{
//#ifdef NEED_EVENTLOG
//		char strLog[MAX_PATH];
//		sprintf(strLog,"AllChannelRateSum >= Totalrate rate config.........error");
//		LogMyEvent(1,DODERROR_RESET_PIDRATE,strLog);
//#endif	
//		m_pChannelManager->m_nTotalPacketNumber=m_nAllChannelRatePacketNumber + 2;
//	}

	int dwChannelCount=m_pChannelManager->m_nPinNumber;
	for(int i=0;i<dwChannelCount;i++)
	{
		CChannel *pchannel=m_pChannelManager->m_pChannelArray[i];
		pchannel->Init(); 
	}
	return NOERROR;
}

STDMETHODIMP CBroadcastFilter::GetChannelInfo(WORD wPID, ZQSBFCHANNELINFO* pChannelInfo)
{
	if(wPID <0 || wPID >0x1fff)
	{
		pChannelInfo=NULL;
		return 1;
	}

	int dwChannelCount=m_pChannelManager->m_nPinNumber;
	for(int i=0;i<dwChannelCount;i++)
	{
		CChannel *pchannel=m_pChannelManager->m_pChannelArray[i];
		if(pChannelInfo->wPID == pchannel->m_nPID)
		{
            pChannelInfo->bEnable=pchannel->m_bEnable;
			strcpy(pChannelInfo->cDescriptor,pchannel->m_cDescriptor);

			pChannelInfo->wAccessType=(pchannel->m_bIsFileMode==TRUE)?1:0;
			pChannelInfo->wPID=pchannel->m_nPID;
			pChannelInfo->wRate=pchannel->m_nRate;
			break;
		}
	}
	if(i==dwChannelCount)
	{
#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,"BroadcastFilter::Enable pid=%d error",wPID);
		LogMyEvent(1,DODERROR_INPUTPID_VALUE,strLog);
#endif	
		return DODERROR_INPUTPID_VALUE;
	}
		return NOERROR;
}
STDMETHODIMP CBroadcastFilter::GetTotalRate(DWORD* pwTotalRate)
{
	*pwTotalRate=m_pChannelManager->m_nTotalRate;
	return NOERROR;
}
STDMETHODIMP CBroadcastFilter::GetFilePath(char *cfilepath)
{
	if(strlen(m_pChannelManager->m_cDirName)==0)
		cfilepath=NULL;
	else
		strcpy(cfilepath,m_pChannelManager->m_cDirName);
	return NOERROR;
}
STDMETHODIMP CBroadcastFilter::GetPmtPID(WORD* pwPID)
{
	*pwPID=m_pChannelManager->m_nPMTPID;
return NOERROR;
}


STDMETHODIMP CBroadcastFilter::Run(REFERENCE_TIME tStart)
{
	if(m_pChannelManager==NULL)
		return 1;

	m_pChannelManager->m_fWriteError = FALSE;

	if (m_pChannelManager)
	{
		m_pChannelManager->m_fWriteError = FALSE;
		m_pChannelManager->Run();
	}

	return CBaseFilter::Run(tStart);
}

STDMETHODIMP CBroadcastFilter::SetIPPortConfig( ZQSBFIPPORTINFO* pInfoList[], int nCount)
{
	 if(m_pChannelManager->m_nPortNumber>0)
		return 1;
	if(nCount <0)
		return 1;
	m_pChannelManager->m_nPortNumber=nCount;
	m_pChannelManager->m_pSendArray = new PBROASSEND[nCount];//CBroadcastPin[dwChannelCount];

	for(int i=0;i<nCount;i++)
	{
		CBufferSend	*pSen=new CBufferSend(); 
		pSen->m_pManager=m_pChannelManager;
		if(pInfoList[i]->wSendType> SENDTTYPETOTAL)
		{			
#ifdef NEED_EVENTLOG
			char strLog[MAX_PATH];
			sprintf(strLog,"SetIPPortConfig error type=%d error",pInfoList[i]->wSendType);
			LogMyEvent(1,DODERROR_SENDTYPE,strLog);
#endif	
			return DODERROR_SENDTYPE;
		}
		pSen->m_wSendType=pInfoList[i]->wSendType;
		strcpy(pSen->m_cSourceIp,pInfoList[i]->cSourceIp);
		pSen->m_wSourcePort=pInfoList[i]->wSourcePort;	
		strcpy(pSen->m_cDestIp,pInfoList[i]->cDestIp);
		pSen->m_wDestPort=pInfoList[i]->wDestPort;	
		pSen->m_nIndex=i;	
		pSen->m_nToralperiod=m_pChannelManager->m_nTotalperiod;
		m_pChannelManager->m_pSendArray[i]=pSen;	
	}

	return NOERROR;
}
STDMETHODIMP CBroadcastFilter::GetIPPortConfig( int nIndex, ZQSBFIPPORTINFO* pInfo)
{
	if(nIndex <0 || nIndex >m_pChannelManager->m_nPortNumber)
// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月28日 10:27:20
	{
		pInfo=NULL;
		return 1;
	}
	CBufferSend	*pSen=m_pChannelManager->m_pSendArray[nIndex]; 
	strcpy(pInfo->cSourceIp,pSen->m_cSourceIp);
	strcpy(pInfo->cDestIp,pSen->m_cDestIp);
	pInfo->wDestPort=pSen->m_wDestPort;
	pInfo->wSendType=pSen->m_wSendType;
	pInfo->wSourcePort=pSen->m_wSourcePort;
	return NOERROR;
}
//
//  Definition of CDumpInputPin
//
CBroadcastPin::CBroadcastPin(TCHAR   *pName,CBroadcastFilter *pFilter,													
							HRESULT *phr,LPOLESTR  pPinName):
CBaseInputPin(pName, pFilter, pFilter, phr, pPinName),m_hFile(0)
{
	m_pChannelManager=NULL;
	m_cCurFileName[0]='\0';
	m_nIndex=0;
	m_RecvLength=0;
	m_pcReceiveData=0;
//	m_nbufOffset=0;
}
CBroadcastPin::~CBroadcastPin()
{
	if (m_hFile != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(m_hFile); 
		m_hFile=NULL;
	}
}
HRESULT CBroadcastPin::Active()
{
	HRESULT hr;
	hr = CBasePin::Active();
	if(FAILED(hr))
	{
		return hr;
	}
	return hr;
}

HRESULT CBroadcastPin::CheckMediaType(const CMediaType *pMediaType)
{

	CheckPointer(pMediaType,E_POINTER);

	//if( *(pMediaType->Type()) == MEDIATYPE_NULL)
	return NOERROR;
}

HRESULT CBroadcastPin::BreakConnect()
{	
	return NOERROR;
}


HRESULT	 CBroadcastPin::Receive(IMediaSample *pSample)
{
	TCHAR szMsg[MAX_PATH];	//wsprintf(szMsg," CBroadcastPin.m_nIndex=%d  ",m_nIndex);		LogToFile(2,szMsg);

	wsprintf(szMsg,"Receive data ");	
	LogMyEvent(1,1,szMsg);

	if(m_bPinEnable==FALSE)
		return NOERROR;

	if(m_pChannelManager->m_bStop)
		return NOERROR;

	CheckPointer(pSample,E_POINTER);
    PBYTE pbData;
	HRESULT hr = pSample->GetPointer(&pbData);
	if (FAILED(hr)) {
		return hr;
	}
	int RecvLength=pSample->GetActualDataLength(); 

	if(RecvLength<sizeof(SampBlock))
	{
#ifdef NEED_EVENTLOG
		char strLog[MAX_PATH];
		sprintf(strLog,":Receive(IMediaSample,RecvLength<sizeof(sample) error");
		LogMyEvent(1,DODERROR_STRUCT_DATA_PART_LOST,strLog);
#endif	
		return DODERROR_STRUCT_DATA_PART_LOST;	
	}


	SampBlock * pBlpok = (SampBlock *)pbData;

	BYTE flag=pBlpok->nControlFlag;
//	RecvLength= (int)(ahigh<<16)+alow;  d:\temp\doc1

	BYTE * pbb =(BYTE *) &(pBlpok->dwLength);
	int ahigh=pbb[0]*256+pbb[1];
	int alow= pbb[2]*256+pbb[3];
	RecvLength= (int)(ahigh<<16)+alow;
	if(m_pChannelManager && m_pChannelManager->m_pChannelArray[m_nIndex])
	{	
		if (m_pChannelManager->m_pChannelArray[m_nIndex]->m_bOccurError)
		{
			wsprintf(szMsg,"rcPID=%d,len=%d,flag=%d channel_error",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,RecvLength,flag);	
			LogMyEvent(1,1,szMsg);
			return 0;
		}
		wsprintf(szMsg,"rcPID=%d,len=%d,flag=%d ",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,RecvLength,flag);	
		LogMyEvent(1,1,szMsg);
	}

	if(m_pChannelManager->GetPinType(m_nIndex))
	{// file mode
		if(flag==0)
		{//begin
			m_RecvLength=0;

			if (m_hFile != INVALID_HANDLE_VALUE) 
			{
				CloseHandle(m_hFile);
				m_hFile=INVALID_HANDLE_VALUE;
			}
			if (strlen(m_cCurFileName)!=0)
			{
				m_cCurFileName[0]='\0';
				wsprintf(szMsg,"recePID=%d:: Endfile flag is not found",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID);					LogMyEvent(1,1,szMsg);
			}
			while (strlen(m_cCurFileName)==0) 
			{
				m_pChannelManager->GetFileName(m_cCurFileName,m_nIndex);
				if(strlen(m_cCurFileName)==0) 
					Sleep(5);
			}	

// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月5日 15:44:41
//			m_hFile = CreateFile(m_cCurFileName, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
			m_hFile = CreateFile(m_cCurFileName, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
			
			if (m_hFile == INVALID_HANDLE_VALUE) 
			{			
				DWORD dwErr = GetLastError();
				wsprintf(szMsg,"recePID=%d::CreateFile(%s) error.errorcode=%d ",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName,dwErr);					LogMyEvent(1,1,szMsg);
				return HRESULT_FROM_WIN32(dwErr);
			}
			if(RecvLength >0)
			{
				DWORD dwWritten=0;
				if (!WriteFile(m_hFile, (PVOID)(pbData+28), (DWORD)RecvLength,
					&dwWritten, NULL)) 
				{
					DWORD dwErr = GetLastError();
					wsprintf(szMsg,"recePID=%d::WriteFile(%s) error.errorcode=%d ",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName,dwErr);					LogMyEvent(1,1,szMsg);
					CloseHandle(m_hFile); 
					m_hFile = INVALID_HANDLE_VALUE;
					return (HandleWriteFailure());
				}
			}
		}
		else if(flag==1)
		{//continue;S

			if (m_hFile == INVALID_HANDLE_VALUE) 
			{
				wsprintf(szMsg,"recePID=%d::flag is 1,but filehandle is null!(%s) error",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName);					LogMyEvent(1,1,szMsg);
				return 1;
			}
			if(RecvLength >0)
			{
				DWORD dwWritten=0;
				if (!WriteFile(m_hFile, (PVOID)(pbData+28), (DWORD)RecvLength,
					&dwWritten, NULL)) 
				{
					DWORD dwErr = GetLastError();
					wsprintf(szMsg,"recePID=%d::WriteFile(%s) error.errorcode=%d ",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName,dwErr);					LogMyEvent(1,1,szMsg);
					CloseHandle(m_hFile); 
					m_hFile = INVALID_HANDLE_VALUE;
					return (HandleWriteFailure());
				}
			}
		}
		else if(flag==2)
		{//end
			if (m_hFile == INVALID_HANDLE_VALUE) 
			{
				wsprintf(szMsg,"recePID=%d::flag is 2,but filehandle is null!(%s) error",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName);					LogMyEvent(1,1,szMsg);
				return 1;
			}
			if(RecvLength >0)
			{
				DWORD dwWritten=0;
				if (!WriteFile(m_hFile, (PVOID)(pbData+28), (DWORD)RecvLength,
					&dwWritten, NULL)) 
				{	
					DWORD dwErr = GetLastError();
					wsprintf(szMsg,"PID=%d::WriteFile(%s) error.errorcode=%d ",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName,dwErr);					LogMyEvent(1,1,szMsg);
					CloseHandle(m_hFile); 
					m_hFile = INVALID_HANDLE_VALUE;
					return (HandleWriteFailure());
				}
			}
			CloseHandle(m_hFile); 
			m_hFile = INVALID_HANDLE_VALUE;

			m_hFile = CreateFile(m_cCurFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if( m_hFile==INVALID_HANDLE_VALUE )
			{
				DWORD dwErr = GetLastError();
				wsprintf(szMsg,"getfilesize recePID=%d::CreateFile(%s) error.errorcode=%d ",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName,dwErr);					LogMyEvent(1,1,szMsg);
				return HRESULT_FROM_WIN32(dwErr);
			}

			DWORD dwFileSize = GetFileSize(m_hFile, NULL);
			if( dwFileSize==INVALID_FILE_SIZE )
			{
				DWORD dwErr = GetLastError();
				wsprintf(szMsg,"getfilesize recePID=%d::GetFileSize(%s) error.errorcode=%d ",m_pChannelManager->m_pChannelArray[m_nIndex]->m_nPID,m_cCurFileName,dwErr);					LogMyEvent(1,1,szMsg);
				CloseHandle(m_hFile);
				return FALSE;
			}
			CloseHandle(m_hFile); 
			m_hFile = INVALID_HANDLE_VALUE;
			
			m_pChannelManager->changefileFlag(m_nIndex,dwFileSize,m_cCurFileName);
			m_cCurFileName[0]='\0';
		}
	}
	else
	{
// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月21日 13:49:52
		//Memory mode
		if(flag==0)
		{//begin
			m_RecvLength=0;

			m_pcReceiveData=NULL;
			while (m_pcReceiveData==NULL) 
			{
				m_pcReceiveData=m_pChannelManager->GetBufferModeBuf(m_nIndex);				
				if(m_pcReceiveData==NULL) 
					Sleep(5);
			}				

			memcpy(m_pcReceiveData,pbData+28,RecvLength);
			m_RecvLength=RecvLength;
		}
		else 
			if(flag==1)
		{//continue;S

			if (m_pcReceiveData == NULL) 
			{
#ifdef NEED_EVENTLOG
				char strLog[MAX_PATH];
				sprintf(strLog,":Receive(IMediaSample,buffer=NULL error");
				LogMyEvent(1,DODERROR_BUFFERISNULL,strLog);
#endif	
				return DODERROR_BUFFERISNULL;
			}

			m_RecvLength+=RecvLength;
			if(m_RecvLength)
			{
#ifdef NEED_EVENTLOG
				char strLog[MAX_PATH];
				sprintf(strLog,":Receive(IMediaSample,m_RecvLength> error");
				LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
#endif	
				return DODERROR_BUFFERLENGTOOLEN;
			}
			memcpy(m_pcReceiveData+m_RecvLength,pbData+28,RecvLength);		

		}
		else if(flag==2)
		{//end
			m_RecvLength+=RecvLength;
			if(m_RecvLength)
			{
#ifdef NEED_EVENTLOG
				char strLog[MAX_PATH];
				sprintf(strLog,":Receive(IMediaSample,m_RecvLength> error");
				LogMyEvent(1,DODERROR_BUFFERLENGTOOLEN,strLog);
#endif	
				return DODERROR_BUFFERLENGTOOLEN;
			}
			m_pChannelManager->SetBufferModeLen(m_nIndex,m_RecvLength);
			memcpy(m_pcReceiveData+m_RecvLength,pbData+28,RecvLength);		
			m_pChannelManager->changefileFlag(m_nIndex,0,NULL);			
		}
	}

	return NOERROR;
}

HRESULT CBroadcastPin::HandleWriteFailure(void)
{
	DWORD dwErr = GetLastError();

	if (dwErr == ERROR_DISK_FULL)
	{
		// Close the dump file and stop the filter, 
		// which will prevent further write attempts
		m_pFilter->Stop();

		// Set a global flag to prevent accidental deletion of the dump file
		//m_fWriteError = TRUE;

		// Display a message box to inform the developer of the write failure
		TCHAR szMsg[MAX_PATH + 80];
		wsprintf(szMsg, TEXT("The disk containing dump file has run out of space, ")
			TEXT("so the dump filter has been stopped.\r\n\r\n")
			TEXT("You must set a new dump file name or restart the graph ")
			TEXT("to clear this filter error."));
		MessageBox(NULL, szMsg, TEXT("Broadcast Filter failure"), MB_ICONEXCLAMATION);
	}

	return HRESULT_FROM_WIN32(dwErr);
}