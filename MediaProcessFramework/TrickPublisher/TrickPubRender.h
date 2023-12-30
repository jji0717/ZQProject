// TrickPubRender.h: interface for the TrickPubRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRICKPUBRENDER_H__3C9D6BED_9C0B_4954_8202_7A0E926DAC61__INCLUDED_)
#define AFX_TRICKPUBRENDER_H__3C9D6BED_9C0B_4954_8202_7A0E926DAC61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "TrickJob.h"
#include "MCPBaseObject.h"
#include "mcpeventcode.h"

class CTrickPin;//input pin

class CTrickPubFilter  : public CMCPBaseObject,public ITrickParam
{
public:
	CTrickPubFilter(LPUNKNOWN  pUnk, HRESULT *phr);

	virtual ~CTrickPubFilter();

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	//overide from basefilter
    CBasePin * GetPin(int n);
    int GetPinCount();

	STDMETHODIMP Stop();
    STDMETHODIMP Pause();

    // the start parameter is the difference to be added to the
    // sample's stream time to get the reference time for
    // its presentation
    STDMETHODIMP Run(REFERENCE_TIME tStart);


	//ITrickParam
	HRESULT STDMETHODCALLTYPE SetTargetName(BSTR sFilename);
	HRESULT SetMaxMpegCodingErrors(int nCode);
    HRESULT STDMETHODCALLTYPE SetMulticastAddress(BSTR strAddr);
	HRESULT STDMETHODCALLTYPE SetSourcePort(UINT uPort);
	HRESULT STDMETHODCALLTYPE SetDestport(UINT uPort);
	HRESULT STDMETHODCALLTYPE SetSubscribeFilename(BSTR strFileName);
	HRESULT STDMETHODCALLTYPE SetPublisherIP(BSTR strAddr);
	HRESULT STDMETHODCALLTYPE SetPublisherInterfaceIP(BSTR strAddr);
	HRESULT STDMETHODCALLTYPE AddSubscriber(BSTR ip1,BSTR ip2);


	DECLARE_IUNKNOWN;
public:
    CPosPassThru *m_pPosition;      // Renderer position controls

private:

	CCritSec		m_Lock;
	CCritSec		m_ReceiveLock;
	CTrickPin*		m_pPin;
	LPUNKNOWN		m_pUnk;
	BOOL			m_bFilterFirst;
};


class CTrickPin: public CRenderedInputPin
{
private:
    CCritSec * const m_pReceiveLock;    // Sample critical section
    REFERENCE_TIME m_tLast;             // Last sample receive time
	CTrickPubFilter *m_pFilter;
public:	
	void AttachFilterToPin(CTrickPubFilter*pFilter);
	BOOL SetPosObjPtr(CPosPassThru *pPosition);


	void SetTarName(BSTR pTarName);
	void SetMaxMpegCodingErrors(int nCode);
    void SetMulticastAddress(BSTR strAddr);
	void SetSourcePort(UINT uPort);
	void SetDestport(UINT uPort);
	void SetSubscribeFilename(BSTR strFileName);
	void SetPublisherIP(BSTR strAddr);
	void SetPublisherInterfaceIP(BSTR strAddr);
	void AddSubscriber(BSTR ip1,BSTR ip2);


    CTrickPin(LPUNKNOWN pUnk,
                  CBaseFilter *pFilter,
                  CCritSec *pLock,
                  CCritSec *pReceiveLock,
                  HRESULT *phr);

	virtual ~CTrickPin();

    // Do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample);
    STDMETHODIMP EndOfStream(void);
    // Write detailed information about this sample to a file

    // Check if the pin can support this specific proposed type and format
    HRESULT CheckMediaType(const CMediaType *);

    // Break connection
    HRESULT BreakConnect();

	HRESULT Active(void);

	HRESULT Inactive(void);


	CTrickJob	m_Job;

private:

	HANDLE		m_hJobThread;
	int			m_nSampleSize;
	int			m_nQueCount;
	int			m_nIC;

};

#endif