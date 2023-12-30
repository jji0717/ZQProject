#include "StdAfx.h"
#include ".\dodportctrl.h"
#include <initguid.h>
#include <streams.h>
#include "strmif.h"

#include "interfaceDefination.h"
#include "BroadcastGuid.h"
#include "IObjectControl.h"

typedef CDODChannel* PDODBROASCHANNEL;


HRESULT GetUnconnectedPin( IBaseFilter *pFilter,
						  PIN_DIRECTION PinDir,
						  IPin **ppPin )
{
	*ppPin = 0;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins( &pEnum );
	if( FAILED( hr ) )
	{
		return hr;
	}
	while( pEnum->Next( 1, &pPin, NULL ) == S_OK )
	{
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection( &ThisPinDir );
		if( ThisPinDir == PinDir )
		{
			IPin *pTmp = 0;
			hr = pPin->ConnectedTo( &pTmp );
			if( SUCCEEDED( hr ) )
			{
				pTmp->Release( );
			}
			else
			{
				pEnum->Release( );
				*ppPin = pPin;
				return S_OK;
			}
		}
		pPin->Release( );
	}
	pEnum->Release( );

	return E_FAIL;
}
HRESULT ConnectPins( IGraphBuilder* pGraphBuilder,
					IBaseFilter* pOutput, 
					IBaseFilter* pInput, 
					int nOutIndex, 
					int nInIndex)
{
	HRESULT hr = NOERROR;
	ULONG cFetched = 0;
	IEnumPins* pEnumPins = 0;
	IPin* pPinOutput = 0;
	IPin* pPinInput = 0;

	//enum output filter pins.
	hr = pOutput->EnumPins(&pEnumPins);
	if(FAILED(hr))
	{
		return hr;
	}
	//if the pin to connect is not the first one, we must skip nOutIndex-1 pins.
	if(nOutIndex>1)
	{
		hr = pEnumPins->Skip(nOutIndex-1);
		if(FAILED(hr))
		{
			pEnumPins->Release();
			return hr;
		}
	}
	//now we can get the output pin we wanted.
	hr = pEnumPins->Next(1,&pPinOutput,&cFetched);
	if(FAILED(hr))
	{
		pEnumPins->Release();
		return hr;
	}
	pEnumPins->Release();

	//enum input filter pins.
	hr = pInput->EnumPins(&pEnumPins);
	if(FAILED(hr))
	{
		pPinOutput->Release();
		return hr;
	}
	//if input pin is not the first to connect, we must skip nInIndex-1 pins.
	if(nInIndex>1)
	{
		hr = pEnumPins->Skip(nInIndex-1);
		if( FAILED(hr) )
		{
			pPinOutput->Release();
			pEnumPins->Release();
			return hr;
		}
	}
	//now we can get the input pin we wanted.
	hr = pEnumPins->Next(1,&pPinInput,&cFetched);
	if(FAILED(hr))
	{
		pPinOutput->Release();
		pEnumPins->Release();
		return hr;
	}
	pEnumPins->Release();

	//now connect from output pin to input pin.
	hr = pGraphBuilder->Connect(pPinOutput,pPinInput);
	pPinOutput->Release();
	pPinInput->Release();
	return hr;
}

IPin* xMyFindPin(IBaseFilter *pFilter, const WCHAR achName[])
{
	BOOL       bFound = FALSE;
	IEnumPins  *pEnum;
	IPin       *pPin;
	PIN_INFO pininfo;

	if(pFilter==NULL)
		return NULL;
	pFilter->EnumPins(&pEnum);
	while(pEnum->Next(1, &pPin, 0) == S_OK)
	{
		pPin->QueryPinInfo(&pininfo);
		pininfo.pFilter->Release();
		if (bFound = (0 == wcscmp(pininfo.achName,achName)))
			break;
		pPin->Release();
	}
	pEnum->Release();
	return (bFound ? pPin : 0);  
}


CDODPortCtrl::CDODPortCtrl(void)
{
	m_nSessionID=0;
//	m_PortInfo=NULL;
	m_dwID=0;
	Initialize();
}

CDODPortCtrl::~CDODPortCtrl(void)
{/*
 if (m_PortInfo)
 {
 delete m_PortInfo;
 m_PortInfo=NULL;
 }*/
	try
	{
		for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
		{			
			if(m_cchannelinfo==NULL)
				continue;

			if(m_cchannelinfo[i])
			{
				delete m_cchannelinfo[i];
				m_cchannelinfo[i] = NULL;
			}	
		}

		// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月18日 13:40:08

		if (m_cchannelinfo)
		{
			delete m_cchannelinfo;
			m_cchannelinfo=NULL;
		}


		for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
		{			
			if(m_pChannel==NULL)
				continue;
			if(m_pChannel[i])
			{
				delete m_pChannel[i];
				m_pChannel[i] = NULL;
			}	
		}

		// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月18日 13:43:22

		if (m_pChannel)
		{
			delete m_pChannel;
			m_pChannel=NULL;
		}


		for (int i = 0; i <(int)m_dwCasterCount; ++i)	// Open and Start the NonBakcup Devices
		{			
			if(m_pCaster==NULL)
				continue;

			if(m_pCaster[i])
			{
				delete m_pCaster[i];
				m_pCaster[i] = NULL;
			}	
		}
		if (m_pCaster)
		{
			delete m_pCaster;
			m_pCaster=NULL;
		}
	}
	catch (...) {
		Clog( LOG_ERR, _T("CDODPortCtrl::~CDODPortCtrl() exception !"));
	}

	Clog( LOG_DEBUG, _T("CDODPortCtrl::delete itself success !"));
}
DWORD CDODPortCtrl::Initialize()
{
	m_pGraph=NULL;
	m_pMC=NULL;
	m_pME=NULL;
	
	m_nState=STATEPortinitialized;

	m_dwChannelCount=0;
	m_dwCasterCount=0;

	m_pChannel=NULL;
	m_cchannelinfo=NULL;
	m_sArrangePath[0]='\0';
	m_pCaster=NULL;/*
	for(int i=0;i< MAXCHANNELCOUNT;i++)
	{
		m_pChannel[i]=NULL;
		m_cchannelinfo[i]=NULL;
	}
*/
	m_pBroadcastfilter=NULL;

	m_IRatecontrol=NULL;
	m_IIpsetting=NULL;/*
	for(i=0;i<MAXIPPORTCOUNT;i++)
	{
		m_pCaster[i]=NULL;
	}
	*/m_bCallinitialition=FALSE;
	return 0; 
}
DWORD  CDODPortCtrl::ApplyChannelParameter()
{
	HRESULT hr = S_OK;
// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月14日 15:19:59
	try
	{
		for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
		{	
			CDODChannel *pChannel=m_pChannel[i];
			ZQSBFCHANNELINFO *pinfo=m_cchannelinfo[i];

			hr=pChannel->SetDetectedFlag(pinfo->bBeDetect);
			if (S_OK==hr)	
				hr=pChannel->SetDetectedInterval(pinfo->wBeDetectInterval);

			if (S_OK==hr)	
				hr=pChannel->SetChannelPID(pinfo->wPID);

			if (S_OK==hr)	
				hr=pChannel->SetChannelTag(pinfo->cDescriptor,4);
			// ------------------------------------------------------ Modified by zhenan_ji at 2005年8月8日 17:25:03

			if (S_OK==hr)	
				hr=pChannel->SetEncrypt((BYTE)(pinfo->wBeEncrypted));
			Clog( LOG_DEBUG, "CDODPortCtrl.ApplyChannelParameter.SetEncrypt ok channelPID=%d",pinfo->wPID);

// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月27日 16:06:46
//One BYTE to define the mode of Packaging:
//7~~3 bit : reserved
//2 bit : multiple Streams support;
//1 bit : tableID+Extension made with Destination
//0 bit : Index Table support

			hr=pChannel->SetMode(pinfo->wSendWithDestination);
			if (S_OK!=hr)	
			{
				Clog( LOG_ERR, _T("CDODPortCtrl.ApplyChannelParameter.SetMode :package mode value=%d error"),pinfo->wSendWithDestination);
				return 1;
			}
	
			Clog( LOG_DEBUG, _T("CDODPortCtrl.ApplyChannelParameter.SetStreamCount =%d , path=%s"),pinfo->wStreamCount,m_sArrangePath);
			if (pinfo->wStreamCount >0)
				hr=pChannel->SetStreamCount(pinfo->wStreamCount,m_sArrangePath);
			
			Clog( LOG_DEBUG, _T("CDODPortCtrl.ApplyChannelParameter.SetStreamCount =OK"),pinfo->wStreamCount,m_sArrangePath);

			//	CString str=pinfo->cDescriptor;
			//if(str.CompareNoCase("msg")==0)
			//	hr=pChannel->SetMode(WRAP_MODE_MSG); 
			//else
			//	if (pinfo->wStreamCount >0)
			//	{
			//		hr=pChannel->SetMode(WRAP_MODE_NE);
			//			Clog( LOG_DEBUG, "CDODPortCtrl.hr=pChannel->SetStreamCount=%d.before",pinfo->wStreamCount);
			//		if (S_OK==hr)
			//			hr=pChannel->SetStreamCount(pinfo->wStreamCount);
			//			Clog( LOG_DEBUG, "CDODPortCtrl.hr=pChannel->SetStreamCount=%d.end",pinfo->wStreamCount);

			//	}
			//	else
			//		hr=pChannel->SetMode(WRAP_MODE_COMMON);	

			Clog( LOG_DEBUG, "CDODPortCtrl.ApplyChannelParameter.SetMode (package mode value=%d)",pinfo->wSendWithDestination);
			if (S_OK==hr)	
				hr=pChannel->SetCatalogName(pinfo->szPath);
			if (S_OK!=hr)	
			{
				Clog( LOG_ERR, _T("CDODPortCtrl.ConfigChannel i Error."));
				return 1;
			}
			Clog( LOG_DEBUG, "CDODPortCtrl.ApplyChannelParameter.SetCatalogName (value=%s) ",pinfo->szPath);
		}
	}
	catch (...) 
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::~ApplyChannelParameter() exception sessionid=%d!"),m_nSessionID);
		return 1;
	}
	return 0;
}

#if defined(_DEBUG) || 1
void dumpFilterModuleInfo()
{
	static const TCHAR* moduleName[] = { _T("DataWatcherSource.ax"), 
		_T("DataWrapper.ax"), _T("ZQBroadcastFilter.ax")};

	TCHAR szModPath[MAX_PATH];
	glog(ISvcLog::L_DEBUG, "Module info:");
	for (int i = 0; i < sizeof(moduleName) / sizeof(moduleName[0]); i ++) {
		HMODULE mod = GetModuleHandle(moduleName[i]);
		GetModuleFileName(mod, szModPath, 
			sizeof(szModPath) / sizeof(szModPath[0]));
		USES_CONVERSION;
		glog(ISvcLog::L_DEBUG, "0x%p, %s", mod, T2A(szModPath));
	}
}
#else
#define dumpFilterModuleInfo()
#endif

DWORD  CDODPortCtrl::BuildGraphManager()
{
	HRESULT hr = S_OK;

	hr=AddFilters();
	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("BuildGraphManager::AddFilters Error."));
		return 1;
	}
	
	Clog( LOG_DEBUG, _T("BuildGraphManager::m_nSessionID=%d "),m_nSessionID);

	hr=CreateInterfaceFromFilters();
	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("BuildGraphManager::CreateInterfaceFromFilters Error."));
		return 1;
	}
	Clog( LOG_DEBUG, _T("BuildGraphManager::SetChannelCount ChannelCount=%d"),m_dwChannelCount);

	hr=m_IRatecontrol->SetChannelCount(m_dwChannelCount);
	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("BuildGraphManager::m_IRatecontrol SetChannelCount Error."));
		return 1;
	}
	Clog( LOG_DEBUG, _T("BuildGraphManager::SetTotalRate TotalBitRate=%d"),m_dwTotalBitRate);

	hr=SetTotalRate(m_dwTotalBitRate);
	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("BuildGraphManager::SetTotalRate Error."));
		return 1;
	}
	//Clog( LOG_DEBUG, _T("BuildGraphManager::ConfigChannel "));

	for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
	{
	//	Clog( LOG_DEBUG, _T("BuildGraphManager::current channel ,pid=%d.,rate=%d"),m_cchannelinfo[i]->wPID,m_cchannelinfo[i]->wRate);
	//	Clog( LOG_DEBUG, _T("BuildGraphManager::current channel ,pid=%d.,rate=%d"),m_cchannelinfo[i]->wPID,m_cchannelinfo[i]->wRate);
		hr=m_IRatecontrol->ConfigChannel(i,m_cchannelinfo[i]);
		if (S_OK!=hr)	
		{
			Clog( LOG_ERR, _T("BuildGraphManager::m_IRatecontrol ConfigChannel Error."));
			return 1;
		}
	}

	hr=SetTmpFilePath(m_szBCFTempFilePath);
//	hr=SetTmpFilePath(NULL);
	Clog( LOG_DEBUG, _T("BuildGraphManager::SetFilePath filePath=%s "),m_szBCFTempFilePath);

	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("BuildGraphManager::m_IRatecontrol SetFilePath Error."));
		return 1;
	}

	hr=SetIPPortConfig(m_pCaster,m_dwCasterCount);
	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("BuildGraphManager::SetIPPortConfig Error"));
		return 1;
	}

	hr = SetPmtPID(m_dwPmtPID);
	if ( S_OK != hr ) {
		glog(ISvcLog::L_ERROR,
			"CDODPortCtrl::BuildGraphManager() failed.\tPmtPID = %d", 
			m_dwPmtPID);

		// Clog( LOG_ERR, _T("BuildGraphManager::SetPmtPID Error."));
		return 1;
	}

	Clog( LOG_DEBUG, _T("BuildGraphManager::SetPmtPID pmtpid=%d "),m_dwPmtPID);

	hr=ConnectFilters();
	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("BuildGraphManager::ConnectFilters Error."));
		return 1;
	}

	// for debug
	dumpFilterModuleInfo();

	return 0;
}

DWORD CDODPortCtrl::CreateInterfaceFromFilters()
{
	HRESULT hr = S_OK;
	for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
	{
		CDODChannel *pChannel=m_pChannel[i];
		if(pChannel) 
		{
			hr=pChannel->CreateInterface();
			if (S_OK!=hr)	
			{
				Clog( LOG_ERR, _T("CDODPortCtrl::pChannel->CreateInterface error."));
				return 1;
			}
		}
	}
	if(m_pBroadcastfilter==NULL)
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::m_pBroadcastfilter is NULL."));
		return PORTCTRL_BROADCASTFILTERISNULL;
	}
	
	if(m_IRatecontrol || m_IIpsetting)
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::m_IRatecontrol was initialization error !"));
		return PORTCTRL_RATECONTROLISNULL;
	}

	hr = m_pBroadcastfilter->QueryInterface(IID_IRateControl,(void **)&m_IRatecontrol);

	if (SUCCEEDED(hr))	
		hr = m_pBroadcastfilter->QueryInterface(IID_IIPSetting,(void **)&m_IIpsetting);

	if (FAILED(hr))	
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::m_IRatecontrol or m_IIpsetting error !"));
		return PORTCTRL_RATECONTROLISNULL;
	}

	return 0;
}
DWORD CDODPortCtrl::ConnectFilters()
{
	HRESULT hr = S_OK;

	for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
	{
		CDODChannel *pChannel=m_pChannel[i];

		//Clog( LOG_DEBUG, _T("ConnectFilters::xMyFindPin(pChannel->m_pDataWatcherSource,  before "));

		IPin * pPinOutSTWrapperSource = NULL;
		pPinOutSTWrapperSource=xMyFindPin(pChannel->m_pDataWatcherSource, L"Output");

		IPin * pPinInTWrapper = NULL;
		pPinInTWrapper=xMyFindPin(pChannel->m_pDataWrapper, L"Input");

		//Clog( LOG_DEBUG, _T("ConnectFilters::Connect(pPinOutSTWrapperSource,pPinInTWrapper, before "));

		if (pPinOutSTWrapperSource && pPinInTWrapper)	
			hr = m_pGraph->Connect(pPinOutSTWrapperSource,pPinInTWrapper);
			
		if(FAILED(hr))
		{
			Clog( LOG_ERR, _T("CDODPortCtrl::m_pGraph->Connect  Error."));
			return 1;
		}
	//	Clog( LOG_DEBUG, _T("ConnectFilters::Connect(pPinOutSTWrapperSource,pPinInTWrapper, before "));

		IPin * pPinOutTWrapper = NULL;
		pPinOutTWrapper=xMyFindPin(pChannel->m_pDataWrapper, L"Output Pin");


		IPin * pPinInBroadcast = NULL;

		if (SUCCEEDED(hr))	
			hr =GetUnconnectedPin(m_pBroadcastfilter,PINDIR_INPUT,&pPinInBroadcast);
//		Clog( LOG_DEBUG, _T("ConnectFilters::Connect(pPinOutTWrapper,pPinInBroadcast, before "));

		if (SUCCEEDED(hr))	
			hr = m_pGraph->Connect(pPinOutTWrapper,pPinInBroadcast);

		if(FAILED(hr))
		{
			Clog( LOG_ERR, _T("CDODPortCtrl::PinOutTWrapper,pPinInBroadcast  Error."));
			return 1;
		}

		SAFE_RELEASE(pPinOutSTWrapperSource);
		SAFE_RELEASE(pPinInTWrapper);
		SAFE_RELEASE(pPinOutTWrapper);
		SAFE_RELEASE(pPinInBroadcast);
	}
	Clog( LOG_DEBUG, _T("CDODPortCtrl::ConnectFilters() successed! SessionID=%d"),m_nSessionID);
	return 0;
}

#ifndef _NO_FIX_LOG
#include "fltinit.h"
#include <assert.h>

void _Init_Filter(IUnknown* filter)
{
	IFilterInit* filterInit;

	if (service_log == NULL) {
		OutputDebugStringA("_Init_Filter():\tservice_log is NULL\n");
		assert(false);
		return;
	}

	HRESULT r = filter->QueryInterface(IID_IFilterInit, (void **)&filterInit);
	if (SUCCEEDED(r)) {
		filterInit->initLog(service_log);
	} else {
		service_log->log0(ISvcLog::L_ERROR, "QueryInterface(IID_IFilterInit) was failed.\n");
		assert(false);
	}
}

#endif // #ifndef _NO_FIX_LOG

DWORD CDODPortCtrl::AddFilters()
{	
//	Clog( LOG_DEBUG, _T("CDODPortCtrl::AddFilters ::initialition "));

	ReleaseAll();	

//	Clog( LOG_DEBUG, _T("CDODPortCtrl::AddFilters ::initialition end"));

	if(m_bCallinitialition==FALSE)
	{
		CoInitialize(NULL);
		m_bCallinitialition=TRUE;
	}
	HRESULT hr = S_OK;
	hr=CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, 
		IID_IGraphBuilder, (LPVOID *)&m_pGraph);
	if(FAILED(hr))
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::AddFilters CoCreateInstance error"));
		return 1;
	}
	hr=m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pMC);

//	Clog( LOG_DEBUG, _T("CDODPortCtrl::AddFilters QueryInterface m_pMC"));

	if(FAILED(hr))
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::AddFilters IID_IMediaControl Error."));
		return 1;
	}

	if (m_dwChannelCount>0)
	{
		m_pChannel=new PDODBROASCHANNEL[m_dwChannelCount];
	}
	else
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::m_dwChannelCount <0 Error."));
		return 1;
	}


	for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
	{
		CDODChannel *pChannel=new CDODChannel;

		if (SUCCEEDED(hr))	
			hr = CoCreateInstance(CLSID_IDataWatcherSource, NULL, CLSCTX_INPROC, 
			IID_IBaseFilter, (LPVOID *)&(pChannel->m_pDataWatcherSource)) ;		

		if (SUCCEEDED(hr))
			hr = m_pGraph->AddFilter(pChannel->m_pDataWatcherSource,L"DataWatcherSource");

		if(FAILED(hr))
		{
			Clog( LOG_ERR, _T("CDODPortCtrl::DataWatcherSource Error."));
			return 1;
		}

#ifndef _NO_FIX_LOG
		_Init_Filter(pChannel->m_pDataWatcherSource);
#endif

		if (SUCCEEDED(hr))	
			hr = CoCreateInstance(CLSID_DataWrapper, NULL, CLSCTX_INPROC, 
			IID_IBaseFilter, (LPVOID *)&(pChannel->m_pDataWrapper)) ;		

		if (SUCCEEDED(hr))
			hr = m_pGraph->AddFilter(pChannel->m_pDataWrapper,L"Data Wrapper");

		if(FAILED(hr))
		{
			Clog( LOG_ERR, _T("CDODPortCtrl::m_pDataWrapper Error."));
			return 1;
		}

#ifndef _NO_FIX_LOG
		_Init_Filter(pChannel->m_pDataWrapper);
#endif

		m_pChannel[i]=pChannel;
	}
	
	if (SUCCEEDED(hr))	
		hr = CoCreateInstance(CLSID_ZQBroadcast, NULL, CLSCTX_INPROC, 
		IID_IBaseFilter, (LPVOID *)&m_pBroadcastfilter) ;	

	if (SUCCEEDED(hr))
		hr = m_pGraph->AddFilter(m_pBroadcastfilter,L"ZQBroadcast");

	if(FAILED(hr))
	{
		Clog( LOG_ERR, _T("AddFilter(m_pBroadcastfilter,ZQBroadcast."));
		return 1;
	}

#ifndef _NO_FIX_LOG
	_Init_Filter(m_pBroadcastfilter);
#endif

	return 0;
}

//Functions to set and get specified port information.
//0 indicates it is successful.
DWORD CDODPortCtrl::GetPortState(DWORD* pdwPortState)
{
	return 0;
}

DWORD CDODPortCtrl::GetPort(CDODPort* pDODPort)
{
	return 0;
}

DWORD CDODPortCtrl::ReleaseAll()
{
	try
	{
		if(m_pMC)
			m_pMC->Stop();
		//	m_nState = 1;
		if (m_pME) 
		{
			SAFE_RELEASE(m_pME);
		}
		HRESULT hr = S_OK;

		for (int i = 0; i <(int)m_dwChannelCount; ++i)	// Open and Start the NonBakcup Devices
		{			
			if (m_pChannel==NULL)
			{
				continue ;
			}
			Clog( LOG_DEBUG, _T("release channel index (%d)"),i);

			if(m_pChannel[i] && m_pGraph)
			{
				if(m_pChannel[i]->m_pDataWatcherSource)
				{
					hr=m_pGraph->RemoveFilter(m_pChannel[i]->m_pDataWatcherSource);
					if(hr!=S_OK)
					{
						Clog( LOG_DEBUG, _T("RemoveFilter(m_pChannel[i]->m_pDataWatcherSource"));
					}
				}
				Clog( LOG_DEBUG, _T("release channel index (%d) remove pDataWatcherSource ok"),i);

				if(m_pChannel[i]->m_pDataWrapper)
				{
					hr=m_pGraph->RemoveFilter(m_pChannel[i]->m_pDataWrapper);
					Clog( LOG_DEBUG, _T("RemoveFilter (%d) remove DataWrapper ok"),i);
					if(hr!=S_OK)
					{
						Clog( LOG_ERR, _T("RemoveFilter(m_pChannel[i]->m_pDataWrapper) failed."));
					}
				}
				m_pChannel[i]->ReleaseFilter();
				Clog( LOG_DEBUG, _T("release channel index (%d) remove ReleaseFilter ok"),i);

				delete m_pChannel[i];
				m_pChannel[i] = NULL;
				
				Clog( LOG_DEBUG, _T("DWORD CDODPortCtrl::ReleaseAll() delete m_pChannel[%d] ok "),i);
			}
		}

		//	Clog( LOG_DEBUG, _T("CDODPortCtrl:::initialition begin "));

		if (m_pChannel)
		{
			delete m_pChannel;
			m_pChannel=NULL;
		}
		Clog( LOG_DEBUG, _T("DWORD CDODPortCtrl::delete  all Channel ok "),i);

		if(m_pBroadcastfilter)
		{
			hr=m_pGraph->RemoveFilter(m_pBroadcastfilter);
			if(hr!=S_OK)
			{
				Clog( LOG_ERR, _T("m_pBroadcastfilter RemoveFilter error"));
			}
		}

		Clog( LOG_DEBUG, _T("CDODPortCtrl::ReleaseAll() SAFE_RELEASE(m_IRatecontrol) OK"));

		SAFE_RELEASE(m_IRatecontrol);
		SAFE_RELEASE(m_IIpsetting);
		if(m_pBroadcastfilter)
		{
			int nreturn=0;
			nreturn=m_pBroadcastfilter->Release();
			m_pBroadcastfilter=NULL;
		}

		SAFE_RELEASE(m_pBroadcastfilter);	

		SAFE_RELEASE(m_pMC);
		Clog( LOG_DEBUG, _T("CDODPortCtrl::ReleaseAll() SAFE_RELEASE(Broadcastfilter) OK"));

		SAFE_RELEASE(m_pGraph);
		if(m_bCallinitialition==TRUE)
		{
			CoUninitialize();
			m_bCallinitialition=FALSE;
		}

		Clog( LOG_DEBUG, _T("CDODPortCtrl::ReleaseAll() call CoUninitialize"));
	}
	catch (...) 
	{
		Clog( LOG_ERR, _T("CDODPortCtrl::ReleaseAll() exception  ! errorcode=%d"),GetLastError());
	}
	return 0;
}

DWORD CDODPortCtrl::ConfigChannel( int nPinIndex, ZQSBFCHANNELINFO* pChannelInfo)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->ConfigChannel(nPinIndex,pChannelInfo);
}	
 
DWORD CDODPortCtrl::SetTotalRate(DWORD wTotalRate)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->SetTotalRate(wTotalRate);
}	
 
DWORD CDODPortCtrl::SetTmpFilePath(char *cfilepath)
{
	if(m_IRatecontrol==NULL)
		return 1;
return	m_IRatecontrol->SetFilePath(cfilepath);	
}	
 
DWORD CDODPortCtrl::SetPmtPID( WORD wPID)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->SetPmtPID(wPID);
}	
 
DWORD CDODPortCtrl::EnablePin( WORD wPID, BOOL bAnable)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->Enable(wPID,bAnable);
}	

DWORD CDODPortCtrl::GetChannelInfo( WORD wPID, ZQSBFCHANNELINFO* pChannelInfo)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->GetChannelInfo(wPID,pChannelInfo);
}	

DWORD CDODPortCtrl::GetTotalRate(DWORD* pwTotalRate)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->GetTotalRate(pwTotalRate);
}	
 
DWORD CDODPortCtrl::GetTmpFilePath(char *cfilepath)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->GetFilePath(cfilepath);
}	
 
DWORD CDODPortCtrl::GetPmtPID( WORD* pwPID)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IRatecontrol->GetPmtPID(pwPID);
}	
//for  IIPSetting

DWORD CDODPortCtrl::SetIPPortConfig( ZQSBFIPPORTINFO* pInfoList[], int nCount)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IIpsetting->SetIPPortConfig(pInfoList,nCount);
}	
 
DWORD CDODPortCtrl::GetIPPortConfig( int nIndex, ZQSBFIPPORTINFO* pInfo)
{
	if(m_IRatecontrol==NULL)
		return 1;
	return m_IIpsetting->GetIPPortConfig(nIndex,pInfo);
}	


//  ICatalogConfigure methods

DWORD CDODPortCtrl::SetCatalogName(int index,TCHAR *pszCatalog)
{
	if(m_pChannel[index]==NULL) 
		return 1;

	return m_pChannel[index]->SetCatalogName(pszCatalog);
}

DWORD CDODPortCtrl::GetCatalogName(int index,TCHAR *pszCatalog)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetCatalogName(pszCatalog);
}

DWORD CDODPortCtrl::UpdateCatalog(int index)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->UpdateCatalog();
}

DWORD CDODPortCtrl::SetDetectedFlag(int index,BOOL bDetected)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->SetDetectedFlag(bDetected);
}

DWORD CDODPortCtrl::GetDetectedFlag (int index,BOOL* bDetected)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetDetectedFlag(bDetected);
}

DWORD CDODPortCtrl::SetDetectedInterval(int index,long lInterval) 
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->SetDetectedInterval(lInterval);
}

DWORD CDODPortCtrl::GetDetectedInterval (int index,long* lInterval)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetDetectedInterval(lInterval);
}

DWORD CDODPortCtrl::SetSubChannelRate(vector<DWS_SubChannelList *> pSubChannellists, long nSubChannel)
{
	//if(m_pChannel[index]==NULL) 
	//	return 1;
	
	return 0;
}
DWORD CDODPortCtrl::SetChannelEvent(int index,HANDLE hStateEvent)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->SetChannelEvent(hStateEvent);
}

//  ICatalogState methods 
DWORD CDODPortCtrl::SetCatalogStateEvent(int index,HANDLE hStateEvent)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->SetCatalogStateEvent(hStateEvent);
}	 

DWORD CDODPortCtrl::GetLastErrorCode(int index,int* nError)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetLastErrorCode(nError);
}

DWORD CDODPortCtrl::GetCurrentStatus(int index,int* pnStatus)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetCurrentStatus(pnStatus);
}

// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月31日 9:57:56
//for CDataWrapperFilter interface, extern
DWORD CDODPortCtrl::SetWrapParaSet(int index, DW_ParameterSet inPara )
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->SetWrapParaSet(inPara);
}

DWORD CDODPortCtrl::GetWrapParaSet(int index, DW_ParameterSet * outPara )
{
	if(m_pChannel[index]==NULL)
		return 1;
	return m_pChannel[index]->GetWrapParaSet(outPara);
}

DWORD CDODPortCtrl::AddVersionNumber(int index)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->AddVersionNumber();
}
DWORD CDODPortCtrl::ResetVersionNumber(int index)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->ResetVersionNumber();
}

DWORD CDODPortCtrl::SetVersionNumber(int index,const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion)
{
	if(m_pChannel[index]==NULL)
		return 1;
	return m_pChannel[index]->SetVersionNumber(inObjectKey,inObjectKeyLen,inVersion);
}

DWORD CDODPortCtrl::GetVersionNumber(int index,const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetVersionNumber(inObjectKey,inObjectKeyLen,outVersion);
}

DWORD CDODPortCtrl::SetObjectKeyLength(int index,BYTE inLength)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->SetObjectKeyLength(inLength);
}

DWORD CDODPortCtrl::GetObjectKeyLength(int index,BYTE *outLength)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetObjectKeyLength(outLength);
}

DWORD CDODPortCtrl::SetIndexDescLength(int index,BYTE inLength)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->SetIndexDescLength(inLength);
}

DWORD CDODPortCtrl::GetIndexDescLength(int index,BYTE *outLength)
{
	if(m_pChannel[index]==NULL) 
		return 1;
	return m_pChannel[index]->GetIndexDescLength(outLength);
}	

