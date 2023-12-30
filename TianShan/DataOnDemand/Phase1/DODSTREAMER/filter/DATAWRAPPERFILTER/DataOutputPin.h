
#ifndef DATAOUTPUTPIN_H
#define DATAOUTPUTPIN_H

#include "common.h"

class CDataWrapperFilter;

class CDataOutputPin : public CBaseOutputPin
{
public:
	//IMemAllocatorPtr m_pMemAlloc;
private:
	CDataWrapperFilter * m_pFilter;
public:
	CDataOutputPin(CDataWrapperFilter* pWrapper, 
		CCritSec* pLock, 
		HRESULT* phr, 
		LPCWSTR pName);

	~CDataOutputPin();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID iid, void** ppv);

	// -pure-
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType *pmt);
	/*
	HRESULT GetMediaType(CMediaType *pMediaType) 
	{ 
		CheckPointer(pMediaType,E_POINTER);
		pMediaType->InitMediaType();
		pMediaType->SetType(&MEDIATYPE_Stream);
		pMediaType->SetSubtype(&MEDIASUBTYPE_NULL);
		return S_OK;
	}*/
    HRESULT Inactive();
	// Ask for buffers of the size appropriate to the agreed media type
	HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,ALLOCATOR_PROPERTIES *pProperties);
	//HRESULT FillBuffer(IMediaSample *pSamp);
	HRESULT ReleaseBuffer( IMediaSample * pSample );
private:
};
#endif