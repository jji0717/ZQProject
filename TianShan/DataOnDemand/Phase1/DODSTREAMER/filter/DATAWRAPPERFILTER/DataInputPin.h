
#ifndef DATAINPUTPIN_H
#define DATAINPUTPIN_H

//#include "common.h"
#include "DataPullPin.h"

class CDataWrapperFilter;
//class CDataPullPin;


//void CatalogChang(vector<DWS_SubChannelList > pSubChannel, LPVOID lParam);


class CDataInputPin : public CBasePin
{
public:
	CDataInputPin(CDataWrapperFilter* pFilter, CCritSec* pLock, HRESULT* phr);
	~CDataInputPin();

	HRESULT SetSubChannelList( SubChannelVector &pSubChannellist );

	HRESULT SendFlag( BYTE byFlag );

	void SetMode( BYTE byMode ){ m_pPull->SetMode( byMode ); };
	void SetChannelPID(long inPID){ m_pPull->SetChannelPID( inPID ); };
	void SetStreamCount( int nCount,int nPID ){  m_pPull->SetStreamCount( nCount,nPID ); };

    // base pin overrides
    HRESULT CheckMediaType(const CMediaType* pmt);
    HRESULT GetMediaType(int iPosition, CMediaType* pmt);
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    HRESULT CompleteConnect(IPin* pPeer);
    HRESULT Inactive();
    STDMETHODIMP Receive(IMediaSample* pSample, DWS_SubChannelList pSubChannel);
	STDMETHODIMP EndOfStream();

	HRESULT Active();							// -leon-
	HRESULT Run(REFERENCE_TIME tStart);			// -leon-
	HRESULT CheckConnect(IPin *pPin);			// -leon-
	HRESULT BreakConnect();						// -leon-
	void SortPinBufferAndDelive();
	void InitPinSortBuffer();
	// called from filter
	HRESULT Seek(REFERENCE_TIME tStart);
	bool Suspend();
	void Resume();
private:
	CDataPullPin *m_pPull;
	CDataWrapperFilter * m_pFilter;
	bool m_bIsInactive;
};

#endif