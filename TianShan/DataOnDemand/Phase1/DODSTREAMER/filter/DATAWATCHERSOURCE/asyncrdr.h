//------------------------------------------------------------------------------
// File: AsyncRdr.h
//
// Desc: DirectShow sample code - base library for I/O functionality.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __ASYNCRDR_H__
#define __ASYNCRDR_H__

#include "log_lxg.h"
#include "common.h"
#include "interfaceDefination.h"
//#include "asyncflt.h"
//
// AsyncRdr
//
// Defines an IO source filter.
//
// This filter (CAsyncReader) supports IBaseFilter and IFileSourceFilter interfaces from the
// filter object itself. It has a single output pin (CCatalogOutputPin)
// which supports IPin and IAsyncReader.
//
// This filter is essentially a wrapper for the CAsyncFile class that does
// all the work.
//


// the filter class (defined below)
class CAsyncReader;

// the output pin class
class CCatalogOutputPin
  : public IAsyncReader,
    public CBasePin,
	public ICatalogRountine
{
private:
	LONGLONG   m_llSize;
    PBYTE      m_pbData;
protected:
    CAsyncReader* m_pReader;
    CAsyncIo * m_pIo;

    //  This is set every time we're asked to return an IAsyncReader
    //  interface
    //  This allows us to know if the downstream pin can use
    //  this transport, otherwise we can hook up to thinks like the
    //  dump filter and nothing happens
    BOOL   m_bQueriedForAsyncReader;

    HRESULT InitAllocator(IMemAllocator **ppAlloc);

public:
	OnCatalogChanged	m_pFunSubChannelChanged;
	LPVOID				m_pParamSubChannelChanged;
	LPOLESTR			m_pszFileName;
	int					m_nErrorCode;			//error code

public:
    // constructor and destructor
    CCatalogOutputPin(
        HRESULT * phr,
        CAsyncReader *pReader,
        CAsyncIo *pIo,
        CCritSec * pLock);

    ~CCatalogOutputPin();

    // --- CUnknown ---

    // need to expose IAsyncReader
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // --- IPin methods ---
    STDMETHODIMP Connect(
        IPin * pReceivePin,
        const AM_MEDIA_TYPE *pmt   // optional media type
    );

    // --- CBasePin methods ---

    // return the types we prefer - this will return the known
    // file type
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

    // can we support this type?
    HRESULT CheckMediaType(const CMediaType* pType);

    // Clear the flag so we see if IAsyncReader is queried for
    HRESULT CheckConnect(IPin *pPin)
    {
        m_bQueriedForAsyncReader = FALSE;
        return CBasePin::CheckConnect(pPin);
    }

    // See if it was asked for
    HRESULT CompleteConnect(IPin *pReceivePin)
    {
        if (m_bQueriedForAsyncReader) {
            return CBasePin::CompleteConnect(pReceivePin);
        } else {

#ifdef VFW_E_NO_TRANSPORT
            return VFW_E_NO_TRANSPORT;
#else
            return E_FAIL;
#endif
        }
    }

    //  Remove our connection status
    HRESULT BreakConnect();
    //{
    //    m_bQueriedForAsyncReader = FALSE;
    //    return CBasePin::BreakConnect();
    //}

    // --- IAsyncReader methods ---
    // pass in your preferred allocator and your preferred properties.
    // method returns the actual allocator to be used. Call GetProperties
    // on returned allocator to learn alignment and prefix etc chosen.
    // this allocator will be not be committed and decommitted by
    // the async reader, only by the consumer.
    STDMETHODIMP RequestAllocator(
                      IMemAllocator* pPreferred,
                      ALLOCATOR_PROPERTIES* pProps,
                      IMemAllocator ** ppActual);

    // queue a request for data.
    // media sample start and stop times contain the requested absolute
    // byte position (start inclusive, stop exclusive).
    // may fail if sample not obtained from agreed allocator.
    // may fail if start/stop position does not match agreed alignment.
    // samples allocated from source pin's allocator may fail
    // GetPointer until after returning from WaitForNext.
    STDMETHODIMP Request(
                     IMediaSample* pSample,
                     DWORD dwUser);         // user context

    // block until the next sample is completed or the timeout occurs.
    // timeout (millisecs) may be 0 or INFINITE. Samples may not
    // be delivered in order. If there is a read error of any sort, a
    // notification will already have been sent by the source filter,
    // and STDMETHODIMP will be an error.
    STDMETHODIMP WaitForNext(
                      DWORD dwTimeout,
                      IMediaSample** ppSample,  // completed sample
                      DWORD * pdwUser);         // user context

    // sync read of data. Sample passed in must have been acquired from
    // the agreed allocator. Start and stop position must be aligned.
    // equivalent to a Request/WaitForNext pair, but may avoid the
    // need for a thread on the source filter.
    STDMETHODIMP SyncReadAligned(
                      IMediaSample* pSample);


    // sync read. works in stopped state as well as run state.
    // need not be aligned. Will fail if read is beyond actual total
    // length.
    STDMETHODIMP SyncRead(
                      LONGLONG llPosition,  // absolute file position
                      LONG lLength,         // nr bytes required
                      BYTE* pBuffer);       // write data here

    // return total length of stream, and currently available length.
    // reads for beyond the available length but within the total length will
    // normally succeed but may block for a long period.
    STDMETHODIMP Length(
                      LONGLONG* pTotal,
                      LONGLONG* pAvailable);

    // cause all outstanding reads to return, possibly with a failure code
    // (VFW_E_TIMEOUT) indicating they were cancelled.
    // these are defined on IAsyncReader and IPin
    STDMETHODIMP BeginFlush(void);
    STDMETHODIMP EndFlush(void);

	//ICatalogRoutine
	STDMETHODIMP SetDetectedRoutine(OnCatalogChanged pOnCatalogChanged, LPVOID lParam);
	STDMETHODIMP SetFileName (LPCOLESTR lpwszFileName);
	BOOL ReadTheFile(LPCTSTR lpszFileName);
	public:
		HRESULT Active()
		{
			loglog("CCatalogOutputPin active before.");
			HRESULT hr=CBasePin::Active();
			loglog("CCatalogOutputPin active after.");
			return hr;
		}
		HRESULT Inactive();
		//{
		//	loglog("CCatalogOutputPin inactive before.");
		//	HRESULT hr=CBasePin::Inactive();
		//	loglog("CCatalogOutputPin inactive after.");
		//	return hr;
		//}
public:
	DWORD m_dwID;
};

class CMemStream : public CAsyncStream
{
public:
    CMemStream() :
        m_llPosition(0)
    {
    }

    /*  Initialization */
    void Init(LPBYTE pbData, LONGLONG llLength, DWORD dwKBPerSec = INFINITE)
    {
        m_pbData = pbData;
        m_llLength = llLength;
        m_dwKBPerSec = dwKBPerSec;
        m_dwTimeStart = timeGetTime();
    }

    HRESULT SetPointer(LONGLONG llPos)
    {
        if (llPos < 0 || llPos > m_llLength) 
		{
            return S_FALSE;
        } 
		else 
		{
            m_llPosition = llPos;
            return S_OK;
        }
    }

    HRESULT Read(PBYTE pbBuffer,
                 DWORD dwBytesToRead,//this parameter should be post from unwrapping filter.
                 BOOL bAlign,
                 LPDWORD pdwBytesRead)
    {
// ------------------------------------------------------ Modified by zhenan_ji at 2006Äê6ÔÂ28ÈÕ 18:04:29
		if (m_llLength==0)
			return S_FALSE;

		CAutoLock lck(&m_csLock);
        DWORD dwReadLength;

        /*  Wait until the bytes are here! */
        DWORD dwTime = timeGetTime();
        // if the left length is less than dwBytesToRead(188), read the left
        if (m_llPosition + dwBytesToRead > m_llLength) 
		{
            dwReadLength = (DWORD)(m_llLength - m_llPosition);
        } 
		else 
		{
            dwReadLength = dwBytesToRead;
        }
        DWORD dwTimeToArrive =((DWORD)m_llPosition + dwReadLength) / m_dwKBPerSec;

        if (dwTime - m_dwTimeStart < dwTimeToArrive) 
		{
            Sleep(dwTimeToArrive - dwTime + m_dwTimeStart);
        }

        CopyMemory((PVOID)pbBuffer, (PVOID)(m_pbData + m_llPosition),
                   dwReadLength);

        m_llPosition += dwReadLength;
        *pdwBytesRead = dwReadLength;
        return S_OK;
    }

    LONGLONG Size(LONGLONG *pSizeAvailable)
    {
		DWORD dwTime=timeGetTime() - m_dwTimeStart;
        LONGLONG llCurrentAvailable =
            static_cast <LONGLONG> (UInt32x32To64(dwTime,m_dwKBPerSec));
 
       *pSizeAvailable =  min(m_llLength, llCurrentAvailable);
	   	//char pp[256];
		//sprintf(pp,"dwTime=%d,dwKBPerSec=%d.",dwTime,m_dwKBPerSec);
	    //sprintf(pp,"......Total=%d,Available=%d,dwTime=%d.",m_llLength,*pSizeAvailable,dwTime);
	    //logFile.Write(pp);
        return m_llLength;
    }

    DWORD Alignment()
    {
        return 1;
    }

    void Lock()
    {
        m_csLock.Lock();
    }

    void Unlock()
    {
        m_csLock.Unlock();
    }

private:
    CCritSec       m_csLock;
    PBYTE          m_pbData;
    LONGLONG       m_llLength;
    LONGLONG       m_llPosition;
    DWORD          m_dwKBPerSec;
    DWORD          m_dwTimeStart;

};
//
// The filter object itself. Supports IBaseFilter through
// CBaseFilter and also IFileSourceFilter directly in this object

class CAsyncReader : public CBaseFilter
{

public:
	
	STDMETHODIMP Stop();
	HRESULT SetID();
protected:
    // filter-wide lock
    CCritSec m_csFilter;

    // all i/o done here
    CAsyncIo m_Io;

    // our output pin
    CCatalogOutputPin m_OutputPin;

    // Type we think our data is
    CMediaType m_mt;

	// zhenan add this parameter for stop command;

public:
	BOOL m_IsStop;
        
    // construction / destruction

    CAsyncReader(
        TCHAR *pName,
        LPUNKNOWN pUnk,
        CAsyncStream *pStream,
        HRESULT *phr);

    ~CAsyncReader();


    // --- CBaseFilter methods ---
    int GetPinCount();
    CBasePin *GetPin(int n);
	BOOL GetDetectedRoutinePara(OnCatalogChanged *pOnCatalogChanged, LPVOID *lParam);
    // --- Access our media type
    const CMediaType *LoadType() const
    {
        return &m_mt;
    }

    virtual HRESULT Connect(
        IPin * pReceivePin,
        const AM_MEDIA_TYPE *pmt   // optional media type
    )
    {
        return m_OutputPin.CBasePin::Connect(pReceivePin, pmt);
    }

	CMemStream m_Stream;//added by Rose
	BOOL	   m_bConnectSuccessful;
	//Log function
	void LogMyEvent(int errorLevel,int errorcode,char* errorStr);

public:
	DWORD m_dwID;

};

void OnCatalogChangedFun(vector<DWS_SubChannelList *> pSubChannel, LPVOID lParam);//only for test

#endif //__ASYNCRDR_H__
