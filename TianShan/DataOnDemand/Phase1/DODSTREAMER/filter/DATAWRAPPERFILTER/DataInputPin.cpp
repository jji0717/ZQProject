
#include "stdafx.h"

//#include "common.h"
#include "DataPullPin.h"
#include "DataInputPin.h"
#include "DataWrapperFilter.h"



CDataInputPin::CDataInputPin(CDataWrapperFilter* pFilter, CCritSec* pLock, HRESULT* phr)
: m_pPull(NULL),
m_pFilter(pFilter),
m_bIsInactive(false),
CBasePin(NAME("DataInputPin"), pFilter, pLock, phr, L"Input", PINDIR_INPUT)
{

}

CDataInputPin::~CDataInputPin()
{
	// only delete it if it exists!
	// thanks to Paul Hatcher for pointing out this bug.
	char strLog[MAX_PATH];	

	sprintf(strLog,"PID=%d:Enter delete CDataInputPin !", m_pFilter->m_nBasePID);	LogMyEvent(1,1,strLog);

	if (m_pPull)
	{
		delete m_pPull;
		sprintf(strLog,"PID=%d: delete CDataInputPin del m_pPull ok !", m_pFilter->m_nBasePID);	LogMyEvent(1,1,strLog);
		m_pPull = NULL;
	}
	sprintf(strLog,"PID=%d:delete CDataInputPin end!", m_pFilter->m_nBasePID);	LogMyEvent(1,1,strLog);
}

// base pin overrides
HRESULT 
CDataInputPin::CheckMediaType(const CMediaType* pmt)
{	
	if (*pmt->Type() == MEDIATYPE_Stream)
	{
		return S_OK;
	}
	return S_FALSE;
}

HRESULT 
CDataInputPin::GetMediaType(int iPosition, CMediaType* pmt)
{
	if (iPosition != 0)
	{
		return VFW_S_NO_MORE_ITEMS;
	}
	pmt->InitMediaType();
	pmt->SetType(&MEDIATYPE_Stream);
	pmt->SetSubtype(&MEDIASUBTYPE_NULL);
	return S_OK;
}

STDMETHODIMP 
CDataInputPin::BeginFlush()
{
	// pass the flush call downstream via the filter
	return S_OK;//m_pFilter->BeginFlush();
}

STDMETHODIMP 
CDataInputPin::EndFlush()
{
	return S_OK;//m_pFilter->EndFlush();
}

HRESULT 
CDataInputPin::CompleteConnect(IPin* pPeer)
{
	HRESULT hr = CBasePin::CompleteConnect(pPeer);

	if (SUCCEEDED(hr))
	{
		// validate input with parser
		hr = m_pFilter->CompleteConnect(pPeer);
	}

	if (SUCCEEDED(hr))
	{

		IMemAllocatorPtr pAlloc;
		hr = CreateMemoryAllocator(&pAlloc);
		if (FAILED(hr))
		{
			return hr; 
		}

		// we need to verify that the output pin does support
		// IAsyncReader, and create a Pull object. This contains a worker
		// thread and performs the pull of data from the output pin.
		// We use the synchronous rather than the asynchronous reading option: there is
		// little difference in performance at these bit rates, and the synchronous option
		// allows us to avoid reading excessively beyond the stop point.

		m_pPull = new CDataPullPin(this);
		hr = m_pPull->Connect(pPeer, pAlloc, true);		// true: bSync

		char strLog[MAX_PATH];	
		sprintf(strLog,"PID=%d: PullPin connect AsyncReader:result=%d !", m_pFilter->m_nBasePID,hr);
		LogMyEvent(1,1,strLog);
	}
	return hr;
}

// called to seek and (if necessary) activate the pulling worker.
// -- we cannot do this in Active, since the filter is still
// marked "State_Stopped" until Active returns, so any
// data arriving too soon from the other thread could be discarded.
HRESULT 
CDataInputPin::Seek(REFERENCE_TIME tStart)
{
	return S_OK;
}

bool 
CDataInputPin::Suspend()
{
	bool bShouldRestart = false;
	if (m_pPull)
	{
		HRESULT hr = m_pPull->PauseThread(); //Suspend();
		if( SUCCEEDED(hr) )
			bShouldRestart = true;
	}
	return bShouldRestart;
}

void 
CDataInputPin::Resume()
{
	char chLogMsg[MAX_PATH];
	wsprintf(chLogMsg,"PID=%d:CDataInputPin::Resume.", m_pFilter->m_nBasePID);
	LogMyEvent(1,0,chLogMsg);

	//if (m_pPull)
	//{
	//	m_pPull->StartThread();
	//}
}

HRESULT 
CDataInputPin::Inactive()
{
	m_bIsInactive = true;
	if (m_pPull != NULL)
	{
		m_pPull->Inactive();
		char strLog[MAX_PATH];	
		sprintf(strLog,"PID=%d: CDataInputPin Inactive!", m_pFilter->m_nBasePID);
		LogMyEvent(1,1,strLog);
	}
	return S_OK;
}

HRESULT CDataInputPin::SetSubChannelList( SubChannelVector &pSubChannellist )
{
	return m_pFilter->SetSubChannelList( pSubChannellist );
}

HRESULT CDataInputPin::SendFlag( BYTE byFlag )
{
	return m_pFilter->SendFlag( byFlag );
}

STDMETHODIMP 
CDataInputPin::Receive(IMediaSample* pSample, DWS_SubChannelList pSubChannel)
{	
	if (m_bIsInactive)
	{
		char strLog[MAX_PATH];	
		sprintf(strLog,"PID=%d: CDataInputPin Inactive!", m_pFilter->m_nBasePID);
		LogMyEvent(1,1,strLog);
		return S_FALSE;
	}

	return m_pFilter->Receive(pSample, pSubChannel );
}

STDMETHODIMP 
CDataInputPin::EndOfStream()
{
	return S_FALSE; //m_pFilter->EndOfStream();
}

// -leon-	Actually, the base class -CBasePin do nothing only returning NOERROR.
HRESULT CDataInputPin::Active()
{
	if (m_pPull != NULL)
	{
		m_pPull->Active();
		char strLog[MAX_PATH];	
		sprintf(strLog,"PID=%d: CDataInputPin Active!", m_pFilter->m_nBasePID);
		LogMyEvent(1,1,strLog); 
	}
	return S_OK;
}

// -leon- this process is same with the base class.
HRESULT CDataInputPin::Run(REFERENCE_TIME tStart)
{
	UNREFERENCED_PARAMETER(tStart);

	return CBasePin::Run( tStart ); //NOERROR;
}

// -leon-
HRESULT CDataInputPin::CheckConnect(IPin *pPin)
{
	return CBasePin::CheckConnect( pPin );
}
// -leon-
HRESULT CDataInputPin::BreakConnect()
{
	if (m_pPull)
	{
		// must stop the thread before the
		// derived class is destroyed or you will
		// get a pure virtual function call during dtor.
		//m_pPull->Inactive();
		m_pPull->Disconnect();
		delete m_pPull;
		m_pPull = NULL;
	}

	return CBasePin::BreakConnect(); //NOERROR;
}

void CDataInputPin::SortPinBufferAndDelive()
{
	m_pFilter->RealSortAndDelive();
	m_pFilter->DelArrangeBufFile();
}

void CDataInputPin::InitPinSortBuffer()
{
	m_pFilter->RealInitPinSortBuffer();
}
///////////////////////////////////////////////////////////////////////
