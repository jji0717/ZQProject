//------------------------------------------------------------------------------
// File: AsyncRdr.cpp
//
// Desc: DirectShow sample code - base library with I/O functionality.
//       This file implements I/O source filter methods and output pin 
//       methods for CAsyncReader and CCatalogOutputPin.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <streams.h>
#include "asyncio.h"
#include "asyncrdr.h"
#include <initguid.h>
#include "asyncflt.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <crtdbg.h>

// --- CCatalogOutputPin implementation ---

CCatalogOutputPin::CCatalogOutputPin(
    HRESULT * phr,
    CAsyncReader *pReader,
    CAsyncIo *pIo,
    CCritSec * pLock)
  : CBasePin(
    NAME("Async output pin"),
    pReader,
    pLock,
    phr,
    L"Output",
    PINDIR_OUTPUT),
    m_pReader(pReader),
    m_pIo(pIo),
	m_pFunSubChannelChanged(0),//test
	m_pParamSubChannelChanged(0),//test
	m_pszFileName(0),
    m_pbData(NULL)
{
}

CCatalogOutputPin::~CCatalogOutputPin()
{
	if( NULL != m_pbData ) delete [] m_pbData;
	if( NULL != m_pszFileName ) delete [] m_pszFileName;
	if(m_pFunSubChannelChanged)
		m_pFunSubChannelChanged=0;
	if(m_pParamSubChannelChanged)
		m_pParamSubChannelChanged=0;
}

STDMETHODIMP CCatalogOutputPin::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv,E_POINTER);

    if(riid == IID_IAsyncReader)
    {
        m_bQueriedForAsyncReader = TRUE;
        return GetInterface((IAsyncReader*) this, ppv);
    }
	else if(riid == IID_ICatalogRountine )
	{
		return GetInterface( (ICatalogRountine *)this, ppv );
	}
    else
    {
        return CBasePin::NonDelegatingQueryInterface(riid, ppv);
    }
}


HRESULT CCatalogOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if(iPosition < 0)
    {
        return E_INVALIDARG;
    }
    if(iPosition > 0)
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    CheckPointer(pMediaType,E_POINTER); 
    CheckPointer(m_pReader,E_UNEXPECTED); 
    
    *pMediaType = *m_pReader->LoadType();

    return S_OK;
}


HRESULT CCatalogOutputPin::CheckMediaType(const CMediaType* pType)
{
    CAutoLock lck(m_pLock);
	return NOERROR;
}


HRESULT CCatalogOutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
    CheckPointer(ppAlloc,E_POINTER);

    HRESULT hr = NOERROR;
    CMemAllocator *pMemObject = NULL;

    *ppAlloc = NULL;

    /* Create a default memory allocator */
    pMemObject = new CMemAllocator(NAME("Base memory allocator"), NULL, &hr);
    if(pMemObject == NULL)
    {
        return E_OUTOFMEMORY;
    }
    if(FAILED(hr))
    {
        delete pMemObject;
        return hr;
    }

    /* Get a reference counted IID_IMemAllocator interface */
    hr = pMemObject->QueryInterface(IID_IMemAllocator,(void **)ppAlloc);
    if(FAILED(hr))
    {
        delete pMemObject;
        return E_NOINTERFACE;
    }

    ASSERT(*ppAlloc != NULL);
    return NOERROR;
}


// we need to return an addrefed allocator, even if it is the preferred
// one, since he doesn't know whether it is the preferred one or not.
STDMETHODIMP 
CCatalogOutputPin::RequestAllocator(
    IMemAllocator* pPreferred,
    ALLOCATOR_PROPERTIES* pProps,
    IMemAllocator ** ppActual)
{
    //CheckPointer(pPreferred,E_POINTER);
    //CheckPointer(pProps,E_POINTER);
    //CheckPointer(ppActual,E_POINTER);
    ASSERT(m_pIo);

    // we care about alignment but nothing else
    if(!pProps->cbAlign || !m_pIo->IsAligned(pProps->cbAlign))
    {
        m_pIo->Alignment(&pProps->cbAlign);
    }

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr;

    if(pPreferred)
    {
        hr = pPreferred->SetProperties(pProps, &Actual);

        if(SUCCEEDED(hr) && m_pIo->IsAligned(Actual.cbAlign))
        {
            pPreferred->AddRef();
            *ppActual = pPreferred;
            return S_OK;
        }
    }

    // create our own allocator
    IMemAllocator* pAlloc;
    hr = InitAllocator(&pAlloc);
    if(FAILED(hr))
    {
        return hr;
    }

    //...and see if we can make it suitable
    hr = pAlloc->SetProperties(pProps, &Actual);
    if(SUCCEEDED(hr) && m_pIo->IsAligned(Actual.cbAlign))
    {
        // we need to release our refcount on pAlloc, and addref
        // it to pass a refcount to the caller - this is a net nothing.
        *ppActual = pAlloc;
        return S_OK;
    }

    // failed to find a suitable allocator
    pAlloc->Release();

    // if we failed because of the IsAligned test, the error code will
    // not be failure
    if(SUCCEEDED(hr))
    {
        hr = VFW_E_BADALIGN;
    }
    return hr;
}


// queue an aligned read request. call WaitForNext to get
// completion.
STDMETHODIMP CCatalogOutputPin::Request(
    IMediaSample* pSample,
    DWORD dwUser)           // user context
{
    CheckPointer(pSample,E_POINTER);

    REFERENCE_TIME tStart, tStop;
    HRESULT hr = pSample->GetTime(&tStart, &tStop);
    if(FAILED(hr))
    {
        return hr;
    }

    LONGLONG llPos = tStart / UNITS;
    LONG lLength = (LONG) ((tStop - tStart) / UNITS);

    LONGLONG llTotal=0, llAvailable=0;

    hr = m_pIo->Length(&llTotal, &llAvailable);
    if(llPos + lLength > llTotal)
    {
        // the end needs to be aligned, but may have been aligned
        // on a coarser alignment.
        LONG lAlign;
        m_pIo->Alignment(&lAlign);

        llTotal = (llTotal + lAlign -1) & ~(lAlign-1);

        if(llPos + lLength > llTotal)
        {
            lLength = (LONG) (llTotal - llPos);

            // must be reducing this!
            ASSERT((llTotal * UNITS) <= tStop);
            tStop = llTotal * UNITS;
            pSample->SetTime(&tStart, &tStop);
        }
    }

    BYTE* pBuffer;
    hr = pSample->GetPointer(&pBuffer);
    if(FAILED(hr))
    {
        return hr;
    }

    return m_pIo->Request(llPos,
                          lLength,
                          TRUE,
                          pBuffer,
                          (LPVOID)pSample,
                          dwUser);
}


// sync-aligned request. just like a request/waitfornext pair.
STDMETHODIMP 
CCatalogOutputPin::SyncReadAligned(
                  IMediaSample* pSample)
{
    CheckPointer(pSample,E_POINTER);

    REFERENCE_TIME tStart, tStop;
    HRESULT hr = pSample->GetTime(&tStart, &tStop);
    if(FAILED(hr))
    {
		m_nErrorCode=GETTIME_FAILED;
		smglog(SMGLOG_ERROR,m_dwID,"DataWrapper call SyncReadAligned function failed. hr=%d",hr);	
        return hr;
    }

    LONGLONG llPos = tStart / UNITS;
    LONG lLength = (LONG) ((tStop - tStart) / UNITS);

    LONGLONG llTotal;
    LONGLONG llAvailable;
    hr = m_pIo->Length(&llTotal, &llAvailable);
    if(llPos + lLength > llTotal)
    {
        // the end needs to be aligned, but may have been aligned
        // on a coarser alignment.
        LONG lAlign;
        m_pIo->Alignment(&lAlign);

        llTotal = (llTotal + lAlign -1) & ~(lAlign-1);

        if(llPos + lLength > llTotal)
        {
            lLength = (LONG) (llTotal - llPos);

            // must be reducing this!
            ASSERT((llTotal * UNITS) <= tStop);
            tStop = llTotal * UNITS;
            pSample->SetTime(&tStart, &tStop);
        }
    }

    BYTE* pBuffer;
    hr = pSample->GetPointer(&pBuffer);
    if(FAILED(hr))
    {
		m_nErrorCode=GETPOINTER_FAILED;
		smglog(SMGLOG_ERROR,m_dwID,"DataWrapper call SyncReadAligned function failed.Get Pointer failed. hr=%d",hr);
        return hr;
    }

    LONG cbActual;
    hr = m_pIo->SyncReadAligned(llPos,
                                lLength,
                                pBuffer,
                                &cbActual,
                                pSample);

    pSample->SetActualDataLength(cbActual);
    return hr;
}


//
// collect the next ready sample
STDMETHODIMP
CCatalogOutputPin::WaitForNext(
    DWORD dwTimeout,
    IMediaSample** ppSample,  // completed sample
    DWORD * pdwUser)        // user context
{
    CheckPointer(ppSample,E_POINTER);

    LONG cbActual;
    IMediaSample* pSample=0;

    HRESULT hr = m_pIo->WaitForNext(dwTimeout,
                                    (LPVOID*) &pSample,
                                    pdwUser,
                                    &cbActual);

    if(SUCCEEDED(hr))
    {
        pSample->SetActualDataLength(cbActual);
    }

    *ppSample = pSample;
    return hr;
}


//
// synchronous read that need not be aligned.
STDMETHODIMP
CCatalogOutputPin::SyncRead(
    LONGLONG llPosition,    // absolute Io position
    LONG lLength,           // nr bytes required
    BYTE* pBuffer)          // write data here
{
    return m_pIo->SyncRead(llPosition, lLength, pBuffer);
}


// return the length of the file, and the length currently
// available locally. We only support locally accessible files,
// so they are always the same
STDMETHODIMP
CCatalogOutputPin::Length(
    LONGLONG* pTotal,
    LONGLONG* pAvailable)
{
    HRESULT hr= m_pIo->Length(pTotal, pAvailable);
	return hr;
}


STDMETHODIMP
CCatalogOutputPin::BeginFlush(void)
{
    return m_pIo->BeginFlush();
}


STDMETHODIMP
CCatalogOutputPin::EndFlush(void)
{
    return m_pIo->EndFlush();
}
HRESULT CCatalogOutputPin::Inactive()
{
	smglog(SMGLOG_DETAIL,m_dwID,"CCatalogOutputPin Inactive.");
	return S_OK;
}

STDMETHODIMP
CCatalogOutputPin::Connect(
    IPin * pReceivePin,
    const AM_MEDIA_TYPE *pmt   // optional media type
)
{
    return m_pReader->Connect(pReceivePin, pmt);
}
HRESULT CCatalogOutputPin::BreakConnect()
{
	smglog(SMGLOG_DETAIL,m_dwID,"BreakConnect was called.");

    m_bQueriedForAsyncReader = FALSE;
	m_pReader->m_bConnectSuccessful=FALSE;//added by Rose
	m_pFunSubChannelChanged=0;
	m_pParamSubChannelChanged=0;
    return CBasePin::BreakConnect();
}

STDMETHODIMP 
CCatalogOutputPin::SetDetectedRoutine(OnCatalogChanged pOnCatalogChanged, LPVOID lParam)
{
	smglog(SMGLOG_DETAIL,m_dwID,"DataWrapper set callback function pointer");	
	m_pFunSubChannelChanged = pOnCatalogChanged;
	m_pParamSubChannelChanged = lParam;
	return S_OK;
}
STDMETHODIMP 
CCatalogOutputPin::SetFileName (LPCOLESTR lpwszFileName)
{
	if (m_pReader->m_IsStop)
	{
		smglog(SMGLOG_INFO,m_dwID,"SetFileName :received stop command( %s ) ",lpwszFileName);	
		return S_FALSE;
	}

	USES_CONVERSION;
	CheckPointer(lpwszFileName, E_POINTER);	
    // lstrlenW is one of the few Unicode functions that works on win95
    int cch = lstrlenW(lpwszFileName) + 1;
	TCHAR lpszFileName[FILENAMELEN]={0};
#ifndef UNICODE
// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 17:31:24
     WideCharToMultiByte(GetACP(), 0, lpwszFileName, -1,lpszFileName, cch*2, NULL, NULL);
#else
    lstrcpy(lpszFileName, lpwszFileName);
#endif
	//zhenan add =NULL;
    if( NULL != m_pszFileName )
	{
		delete [] m_pszFileName;
		m_pszFileName=NULL;

	}
	m_pszFileName = new WCHAR[cch];
    if (m_pszFileName==NULL)
	{
		smglog(SMGLOG_ERROR,m_dwID,"Create file name failed! {m_pszFileName = new WCHAR[cch]} Error=%d ,%s",GetLastError(),lpszFileName );
		return S_FALSE;

	}
    CopyMemory(m_pszFileName, lpwszFileName, cch*sizeof(WCHAR));
	smglog(SMGLOG_INFO,m_dwID,"SetFileName, File:%s",lpszFileName);

	  if (!ReadTheFile(lpszFileName)) 
		{
// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 17:30:53
/*	#ifndef UNICODE
	        delete [] lpszFileName;
	#endif*/
			m_nErrorCode=READ_FILE_FAILED;

			smglog(SMGLOG_ERROR,m_dwID,"DataWrapper set file name for reading. load the file failed!");
// ------------------------------------------------------ Modified by zhenan_ji at 2006年6月28日 18:01:44
//if openFile failed, when datawrapper call m_Stream->read .at last will return S_FALSE
			m_pReader->m_Stream.Init(NULL, 0);//it's very important!
	        return S_FALSE;
	    }
	m_pReader->m_Stream.Init(m_pbData, m_llSize);//it's very important!
	return S_OK;
}
BOOL CCatalogOutputPin::ReadTheFile(LPCTSTR lpszFileName)
{
	DWORD dwBytesRead;
//zhenan add =Null for m_pbData 20060424
	if( m_pbData != NULL ) 
	{
		delete [] m_pbData;
		m_pbData=NULL;
	}

	if (m_pReader->m_IsStop)
	{	
		smglog(SMGLOG_DETAIL,m_dwID,"ReadTheFile( %s ) received stop command delete buf",lpszFileName );
		return FALSE;
	}

    HANDLE hFile = CreateFile(lpszFileName,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL);
    if (hFile == INVALID_HANDLE_VALUE) 
    {
		m_nErrorCode=OPEN_FILE_FAILED;
		smglog(SMGLOG_ERROR,m_dwID,"Create file handle failed! GetLastError=%d ,%s",GetLastError(),lpszFileName );
        return FALSE;
    }
    ULARGE_INTEGER uliSize;
    uliSize.LowPart = GetFileSize(hFile, &uliSize.HighPart);

	if (m_pReader->m_IsStop)
	{
		smglog(SMGLOG_DETAIL,m_dwID,"ReadTheFile( %s ) received stop command :getfilesize",lpszFileName );
		CloseHandle(hFile);
		return FALSE;
	}
    PBYTE pbMem = new BYTE[uliSize.LowPart];
    if (pbMem == NULL) 
    {
        CloseHandle(hFile);
		m_nErrorCode=OUTOFMEMORY;
		smglog(SMGLOG_ERROR,m_dwID,"read file failed-- pbMem == NULL errorcode=%d",GetLastError());
        return FALSE;
    }

    if (!ReadFile(hFile,
                  (LPVOID) pbMem,
                  uliSize.LowPart,
                  &dwBytesRead,
                  NULL) ||
        (dwBytesRead != uliSize.LowPart))
    {
        delete [] pbMem;
        CloseHandle(hFile);
		m_nErrorCode=READ_FILE_FAILED;
		smglog(SMGLOG_ERROR,m_dwID,"ReadFile function failed! file is (GetLastError=%d) %s",GetLastError(),lpszFileName);
        return FALSE;
    }

	m_pbData = pbMem;
	m_llSize = (LONGLONG)uliSize.QuadPart;

	//delete [] pbMem;
    CloseHandle(hFile);
	smglog(SMGLOG_DETAIL,m_dwID,"SetFileName OK");
    return TRUE;
}
// --- CAsyncReader implementation ---

#pragma warning(disable:4355)

CAsyncReader::CAsyncReader(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    CAsyncStream *pStream,
    HRESULT *phr)
  : CBaseFilter(
                pName,
                pUnk,
                &m_csFilter,
                CLSID_IDataWatcherSource,
                NULL
                ),
    m_OutputPin(
                phr,
                this,
                &m_Io,
                &m_csFilter),
    m_Io(pStream)
{
	m_IsStop = FALSE;
	m_bConnectSuccessful= FALSE;
}

CAsyncReader::~CAsyncReader()
{
}

HRESULT CAsyncReader::SetID()
{
	m_OutputPin.m_dwID = m_dwID;
	return S_OK; 
}
HRESULT CAsyncReader::Stop()
{
	m_IsStop = TRUE;
	CBaseFilter::Stop();
	smglog(SMGLOG_DETAIL,m_dwID,"CAsyncReader::Stop");
	return S_OK;
}


int CAsyncReader::GetPinCount()
{
    return 1;
}
BOOL CAsyncReader::GetDetectedRoutinePara(OnCatalogChanged *pOnCatalogChanged, LPVOID *lParam)
{
	USES_CONVERSION;
	CheckPointer(pOnCatalogChanged,E_POINTER);
	CheckPointer(lParam,E_POINTER);

	//test
    //m_OutputPin.SetDetectedRoutine(OnCatalogChangedFun, NULL);
	//TCHAR lpszFileName[MAX_PATH]={0};
	//LPWSTR lpwszFileName;
	//strcpy(lpszFileName,"D:\\temp\\doc\\1.txt");
	//WCHAR wFile[MAX_PATH];
	//wcsncpy(wFile, T2W(lpszFileName), NUMELMS(wFile)-1);
	//wFile[MAX_PATH-1] = 0;
	//m_OutputPin.SetFileName(wFile);
	//BYTE pBuffer;
	//m_OutputPin.SyncRead(0,5,&pBuffer);
	//end test
	*pOnCatalogChanged = m_OutputPin.m_pFunSubChannelChanged;
	*lParam = m_OutputPin.m_pParamSubChannelChanged;
	return 1;
}

//Event log and dbglog
void CAsyncReader::LogMyEvent(int errorLevel,int errorcode,char* errorStr)
{
	char    chMsg[FILENAMELEN];
// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月21日 17:46:26
	SYSTEMTIME		St;
	GetLocalTime(&St);
	wsprintf(chMsg,"%02d-%02d %02d:%02d:%02d:%03d %s",St.wMonth,St.wDay,St.wHour,St.wMinute,St.wSecond,St.wMilliseconds,errorStr);
	#ifdef NEED_LOGINFORMATION
	//	DbgLog((LOG_ERROR, 0, TEXT(chMsg)));
	#endif 

	if (errorLevel == 0)
	{
		// normal infomation
		return;
	}

	HANDLE  hEventSource;
    LPTSTR  lpszStrings[1];
	sprintf(chMsg,"Object=DataWatcherSource filter,Code=%d,Spec=%s",errorcode,errorStr);
    lpszStrings[0] = chMsg;
	DWORD lastError;
	
    // Get a handle to use with ReportEvent().
    hEventSource = RegisterEventSource(NULL, "DOD Server");
    if (hEventSource != NULL)
    {
        // Write to event log.
		if(errorLevel==0)
			ReportEvent(hEventSource, EVENTLOG_SUCCESS, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==1)
			ReportEvent(hEventSource, EVENTLOG_ERROR_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==2)
			ReportEvent(hEventSource, EVENTLOG_WARNING_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==3)
			ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==4)
			ReportEvent(hEventSource, EVENTLOG_AUDIT_SUCCESS, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		else if(errorLevel==5)
			ReportEvent(hEventSource, EVENTLOG_AUDIT_FAILURE, 0, 110, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);

        DeregisterEventSource(hEventSource);
    }
	else
	{
		lastError=GetLastError();
		DbgLog((LOG_ERROR, 0, TEXT("LogMyEvent: fail to Register Event Source.")));
	}

}
//only for test
void OnCatalogChangedFun(vector<DWS_SubChannelList *> pSubChannel, LPVOID lParam)
{
	int i=0;
	i++;
}
//end test
CBasePin * CAsyncReader::GetPin(int n)
{
    if((GetPinCount() > 0) && (n == 0))
    {
        return &m_OutputPin;
    }
    else
    {
        return NULL;
    }
}



