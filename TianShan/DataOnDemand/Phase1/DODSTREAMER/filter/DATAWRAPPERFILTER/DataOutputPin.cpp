
#include "stdafx.h"

#include "DataWrapperFilter.h"
#include "DataOutputPin.h"

//
// Constructor
//
CDataOutputPin::CDataOutputPin(CDataWrapperFilter* pWrapper, 
							   CCritSec* pLock, 
							   HRESULT* phr, 
							   LPCWSTR pName) :
CBaseOutputPin( NAME("DataOutputPin"), pWrapper, pLock, phr, pName)
{
	m_pFilter = pWrapper;
}

CDataOutputPin::~CDataOutputPin()
{
	if( m_pAllocator )
	{
		m_pAllocator->Release();
		m_pAllocator = NULL;
	}
}

STDMETHODIMP 
CDataOutputPin::NonDelegatingQueryInterface(REFIID iid, void** ppv)
{
		return CBaseOutputPin::NonDelegatingQueryInterface(iid, ppv);
}

HRESULT CDataOutputPin::CheckMediaType(const CMediaType *pMediaType)
{
	//CAutoLock lock(m_pFilter->m_csFilter );
/*
	CMediaType mt;
	GetMediaType(&mt);

	if (mt == *pMediaType) {
		return NOERROR;
	}*/

	return S_OK;
}

HRESULT CDataOutputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
	CheckPointer(pmt,E_POINTER);

//	CAutoLock cAutoLock(m_pFilter->pStateLock());
	if(iPosition < 0)
	{
		return E_INVALIDARG;
	}
	if (iPosition > 0)
	{
		return VFW_S_NO_MORE_ITEMS;
	}

	return S_OK; //GetMediaType( 0, pmt );
}

// Ask for buffers of the size appropriate to the agreed media type
HRESULT CDataOutputPin::DecideBufferSize(IMemAllocator *pIMemAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
	CheckPointer(pIMemAlloc,E_POINTER);
	CheckPointer(pProperties,E_POINTER);

	//CAutoLock cAutoLock(m_pFilter->pStateLock());
	HRESULT hr = NOERROR;

	pProperties->cBuffers = 1;
	pProperties->cbBuffer = OUTPUTSAMPLEMAXLEN;

	ASSERT(pProperties->cbBuffer);

	// Ask the allocator to reserve us some sample memory, NOTE the function
	// can succeed (that is return NOERROR) but still not have allocated the
	// memory that we requested, so we must check we got whatever we wanted

	ALLOCATOR_PROPERTIES Actual;
	hr = pIMemAlloc->SetProperties(pProperties,&Actual);
	if(FAILED(hr))
	{
		return hr;
	}

	// Is this allocator unsuitable

	if(Actual.cbBuffer < pProperties->cbBuffer)
	{
		return E_FAIL;
	}

	// Make sure that we have only 1 buffer (we erase the ball in the
	// old buffer to save having to zero a 200k+ buffer every time
	// we draw a frame)

	//m_pMemAlloc = pIMemAlloc;
	m_pAllocator = pIMemAlloc;
	m_pAllocator->AddRef();

	ASSERT(Actual.cBuffers == 1);
	return NOERROR;
}

//
// FillBuffer
//
// Plots a ball into the supplied video buffer
//
//HRESULT CDataOutputPin::FillBuffer(IMediaSample *pms)
//{
//    CheckPointer(pms,E_POINTER);
//
//    BYTE *pData;
//    long lDataLen;
//
//    pms->GetPointer(&pData);
//    lDataLen = pms->GetSize();
//
//    //ZeroMemory(pData, lDataLen);
//    {
//        //CAutoLock cAutoLockShared(&m_cSharedState);
//
//        // Increment to find the finish time
//    }
//
//    pms->SetSyncPoint(TRUE);
//    return NOERROR;
//
//} // FillBuffer

HRESULT CDataOutputPin::ReleaseBuffer( IMediaSample * pSample )
{
	CheckPointer(pSample,E_POINTER);

	if( m_pAllocator )
		m_pAllocator->ReleaseBuffer( pSample );
	return S_OK;
}
HRESULT CDataOutputPin::Inactive()
{
	if (m_pAllocator != NULL)
	{
		char strLog[MAX_PATH];	
		sprintf(strLog,"PID=%d: CDataOutputPin Inactive!", m_pFilter->m_nBasePID);
		LogMyEvent(1,1,strLog);
	}
	return S_OK;
}