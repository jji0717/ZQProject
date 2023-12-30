
#include "stdafx.h"

#include "DataInputPin.h"
#include "DataPullPin.h"

CDataPullPin::CDataPullPin(CDataInputPin* pPin)
: m_pPin(pPin)
{
	//OnCatalogChanged = &CatalogChanged;
}

CDataPullPin::~CDataPullPin()
{

}

HRESULT CDataPullPin::SetSubChannelList( SubChannelVector &pSubChannellist )
{
	return m_pPin->SetSubChannelList( pSubChannellist );
}

HRESULT CDataPullPin::SendFlag( BYTE byFlag )
{
	return m_pPin->SendFlag( byFlag );
}


HRESULT CDataPullPin::Receive(IMediaSample*pSample)
{
	return m_pPin->Receive(pSample, m_SubChannelList[m_iCurrSubChannel] );
}

HRESULT CDataPullPin::EndOfStream(void)
{
	return m_pPin->EndOfStream();
}

void CDataPullPin::OnError(HRESULT hr)
{
}

HRESULT CDataPullPin::BeginFlush()
{
	return m_pPin->BeginFlush();
}

HRESULT CDataPullPin::EndFlush()
{
	return m_pPin->EndFlush();
}

void CDataPullPin::InitSortBuffer()
{
	m_pPin->InitPinSortBuffer();

}
void CDataPullPin::SortBufferAndDelive()
{
	m_pPin->SortPinBufferAndDelive();
}