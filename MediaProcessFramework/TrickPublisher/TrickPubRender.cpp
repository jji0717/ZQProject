// TrickPubRender.cpp: implementation of the TrickPubRender class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "trickpub.h"

#include "common.h"
//#include "stdio.h"
#include "TrickPubRender.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrickPubFilter::CTrickPubFilter(LPUNKNOWN  pUnk, HRESULT *phr)    
#ifdef UNICODE
:CMCPBaseObject("CTrickPubFilter",pUnk,&m_Lock,CLSID_TRICKPUBLISH)
#else
:CMCPBaseObject(NAME("CTrickPubFilter"),pUnk,&m_Lock,CLSID_TRICKPUBLISH)
#endif
,m_pUnk(pUnk),m_pPosition(NULL),m_bFilterFirst(TRUE)
{
	ASSERT(phr);
	
	m_pPin = new CTrickPin(pUnk,
		this,
		&m_Lock,
		&m_ReceiveLock,
		phr);
}

CTrickPubFilter::~CTrickPubFilter()
{
	OutputDebugString("GHY*************::~CTrickPubFilter()\n");
	if(m_pPosition)
	{
		delete m_pPosition;
		m_pPosition=NULL;
	}
	if(m_pPin)
	{
		delete m_pPin;
		m_pPin=NULL;
	}
}

CUnknown * WINAPI CTrickPubFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    ASSERT(phr);
    
    CTrickPubFilter *pNewObject = new CTrickPubFilter( punk,phr);
    if (pNewObject == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return pNewObject;

} // CreateInstance

CBasePin * CTrickPubFilter::GetPin(int n)
{
    if (n == 0) {
        return m_pPin;
    } else {
        return NULL;
    }
}

//
// GetPinCount
//
int CTrickPubFilter::GetPinCount()
{
	if(m_bFilterFirst)
	{
		m_pPin->AttachFilterToPin(this);
		m_bFilterFirst=FALSE;
	}
    return 1;
}

STDMETHODIMP CTrickPubFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{  
	CAutoLock lock(&m_Lock);
	
	if (riid == IID_IBaseFilter || riid == IID_IMediaFilter || riid == IID_IPersist) {

        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    } 
    else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) 
	{

        if (m_pPosition == NULL) 
        {

            HRESULT hr = S_OK;

			m_pPosition = new CPosPassThru(NAME("Trick Pass Through"),
                                           (IUnknown *) GetOwner(),
                                           (HRESULT *) &hr, m_pPin);

            if (m_pPosition == NULL) 
                return E_OUTOFMEMORY;

            if (FAILED(hr)) 
            {
                delete m_pPosition;
                m_pPosition = NULL;
                return hr;
            }
        }
		
		m_pPin->SetPosObjPtr(m_pPosition);
        return m_pPosition->NonDelegatingQueryInterface(riid, ppv);
    } 
	else if(riid==IID_ITrickParam)
	{
		return GetInterface((ITrickParam*)this,ppv);
	}
	else
		return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CTrickPubFilter::Stop()
{
	CAutoLock cObjectLock(m_pLock);
	return CBaseFilter::Stop();
}

HRESULT CTrickPubFilter::Pause()
{
	CAutoLock cObjectLock(m_pLock);
	return CBaseFilter::Pause();
}

HRESULT CTrickPubFilter::Run(REFERENCE_TIME tStart)
{
	CAutoLock cObjectLock(m_pLock);
	return CBaseFilter::Run(tStart);
}


HRESULT CTrickPubFilter::SetTargetName(BSTR sFilename)
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetTarName(sFilename);
	}
	return S_OK;
}

HRESULT SetMaxMpegCodingErrors() 
{
	return S_OK;
}

HRESULT CTrickPubFilter::SetMaxMpegCodingErrors(int nCode) 
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetMaxMpegCodingErrors(nCode);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CTrickPubFilter::SetMulticastAddress(BSTR strAddr)
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetMulticastAddress(strAddr);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CTrickPubFilter::SetSourcePort(UINT uPort)
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetSourcePort(uPort);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CTrickPubFilter::SetDestport(UINT uPort)
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetDestport(uPort);
		return S_OK;
	}

	return S_FALSE;
}


HRESULT CTrickPubFilter::SetSubscribeFilename(BSTR strFileName)
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetSubscribeFilename(strFileName);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CTrickPubFilter::SetPublisherIP(BSTR strAddr)
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetPublisherIP(strAddr);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CTrickPubFilter::SetPublisherInterfaceIP(BSTR strAddr)
{
	if(m_pPin!=NULL)
	{
		m_pPin->SetPublisherInterfaceIP(strAddr);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CTrickPubFilter::AddSubscriber(BSTR ip1,BSTR ip2)
{
	if(m_pPin!=NULL)
	{
		m_pPin->AddSubscriber(ip1,ip2);
		return S_OK;
	}

	return S_FALSE;
}


//************************pin class******************//
CTrickPin::CTrickPin(LPUNKNOWN pUnk,
                     CBaseFilter *pFilter,
                     CCritSec *pLock,
                     CCritSec *pReceiveLock,
                     HRESULT *phr) :
#ifdef UNICODE
CRenderedInputPin("CDumpInputPin",
                  pFilter,                   // Filter
                  pLock,                     // Locking
                  phr,                       // Return code
                  L"Input"),                 // Pin name
#else
CRenderedInputPin(NAME("CDumpInputPin"),
                  pFilter,                   // Filter
                  pLock,                     // Locking
                  phr,                       // Return code
                  L"Input"),                 // Pin name
#endif
    m_pReceiveLock(pReceiveLock),
    m_hJobThread(NULL)
{
	m_nSampleSize=0;
	m_nQueCount=0;
	m_nIC=0;
}

CTrickPin::~CTrickPin()
{
	WaitForSingleObject(m_hJobThread,-1);
	if(m_hJobThread)
	{
		CloseHandle(m_hJobThread);
		m_hJobThread=NULL;
	}
}

HRESULT CTrickPin::BreakConnect()
{
    if (m_pFilter->m_pPosition != NULL) {
        m_pFilter->m_pPosition->ForceRefresh();
    }
	
    return CRenderedInputPin::BreakConnect();
}

HRESULT CTrickPin::CheckMediaType(const CMediaType *)
{
	return S_OK;
}

STDMETHODIMP CTrickPin::EndOfStream(void)
{
    CAutoLock lock(m_pReceiveLock);

	if(m_Job.IsRunning())
	{
		m_Job.SetData(NULL,0);
		m_Job.SetJobStopFlag();
	}

    return CRenderedInputPin::EndOfStream();

} // EndOfStream


STDMETHODIMP CTrickPin::Receive(IMediaSample *pSample)
{
	static int nCount=0;

    CheckPointer(pSample,E_POINTER);
    CAutoLock lock(m_pReceiveLock);
    PBYTE pbData=NULL;

    // Copy the data to the file
	int nLen=pSample->GetActualDataLength();

	if(nLen==0)
	{
		return S_FALSE;
	}

	if(m_nIC==0)
	{

		//create job
		DWORD dwThreadID;
		extern UINT ThreadProc(LPVOID lpParam);
	
		m_nSampleSize=pSample->GetSize();
		m_Job.SetBufferSize(m_nSampleSize);
		
		//put data to shared que
		
		if(NT_MPEG_BUFFER_LENGTH>m_nSampleSize)
			m_nQueCount=(NT_MPEG_BUFFER_LENGTH/m_nSampleSize+1)*2;
		else
			m_nQueCount=JOB_QUE_COUNT;
		
		m_hJobThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadProc,(LPVOID)&m_Job,0,&dwThreadID);
		Sleep(100);
		
		
	/*	char szLog[20];
		ZeroMemory(szLog,20);
		sprintf(szLog,"%d",nCount++);

		HANDLE hFile=CreateFile("c:\\ghy.txt",GENERIC_READ|GENERIC_WRITE,
								FILE_SHARE_READ|FILE_SHARE_WRITE,
								NULL,OPEN_ALWAYS,NULL,NULL);
		DWORD dwWrite;
		WriteFile(hFile,szLog,20,&dwWrite,NULL);
		CloseHandle(hFile);*/
		//m_nIC=1;
	}

    HRESULT hr = pSample->GetPointer(&pbData);
    if (FAILED(hr)) {
        return hr;
    }

	if(pbData==NULL)return S_FALSE;

	while(m_Job.GetJobCount()>m_nQueCount)
	{
		Sleep(10);
	}
	
	m_Job.SetData(pbData,nLen);

	char szBuf[100];
	sprintf(szBuf,"Recieve  %d times",m_nIC);
	m_pFilter->Report((long)EC_PROGRESS,m_pFilter->EVENT_LOG,NULL,szBuf,0);
	m_nIC++;

	return S_OK;
}

BOOL CTrickPin::SetPosObjPtr(CPosPassThru *pPosition)
{
	if(pPosition==NULL)return FALSE;
	m_pFilter->m_pPosition=pPosition;
	
	return TRUE;
}

void CTrickPin::AttachFilterToPin(CTrickPubFilter *pFilter)
{
	m_pFilter=pFilter;
}

HRESULT CTrickPin::Active(void)
{
	return CRenderedInputPin::Active();
}

HRESULT CTrickPin::Inactive(void)
{
	return CRenderedInputPin::Inactive();
}


void CTrickPin::SetTarName(BSTR pTarName)
{
	m_Job.SetFilePathName(pTarName);
}

void CTrickPin::SetMaxMpegCodingErrors(int nCode)
{
	m_Job.SetMaxMpegCodingErrors(nCode);
}

void CTrickPin::SetMulticastAddress(BSTR strAddr)
{
	m_Job.SetMulticastAddress(strAddr);
}

void CTrickPin::SetSourcePort(UINT uPort)
{
	m_Job.SetSourcePort(uPort);
}

void CTrickPin::SetDestport(UINT uPort)
{
	m_Job.SetDestport(uPort);
}

void CTrickPin::SetSubscribeFilename(BSTR strFileName)
{
	m_Job.SetSubscribeFilename(strFileName);
}

void CTrickPin::SetPublisherIP(BSTR strAddr)
{
	m_Job.SetPublisherIP(strAddr);
}

void CTrickPin::SetPublisherInterfaceIP(BSTR strAddr)
{
	m_Job.SetPublisherInterfaceIP(strAddr);
}

void CTrickPin::AddSubscriber(BSTR ip1,BSTR ip2)
{
	m_Job.AddSubscriber(ip1,ip2);
}
