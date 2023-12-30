#include "StdAfx.h"
#include ".\dodchannel.h"
#include "interfaceDefination.h"
#include "IObjectControl.h"

CDODChannel::CDODChannel(void)
{
	m_pDataWatcherSource=NULL;
	m_pDataWrapper=NULL;

	m_pCatalogConfigure=NULL;
	m_pCatalogState=NULL;
	m_pWrapperControl=NULL;

}

CDODChannel::~CDODChannel(void)
{
	ReleaseFilter();
}
DWORD CDODChannel::ReleaseFilter()
{
	SAFE_RELEASE(m_pWrapperControl);	Clog( LOG_DEBUG, _T("release  m_pWrapperControl ok"));

	SAFE_RELEASE(m_pCatalogConfigure);	Clog( LOG_DEBUG, _T("release  m_pCatalogConfigure ok"));

	SAFE_RELEASE(m_pCatalogState);	Clog( LOG_DEBUG, _T("release  m_pCatalogState ok"));

	SAFE_RELEASE(m_pDataWatcherSource);	Clog( LOG_DEBUG, _T("release  m_pDataWatcherSource ok"));

	SAFE_RELEASE(m_pDataWrapper);	Clog( LOG_DEBUG, _T("release  m_pDataWrapper ok"));

	return 0;
}
DWORD CDODChannel::CreateInterface()
{
	HRESULT hr = S_OK;

	if(m_pDataWatcherSource==NULL || m_pDataWrapper==NULL)
	{
		Clog( LOG_DEBUG, _T("CDODChannel::m_pDataWatcherSource or pDataWrapper is NULL."));
		return 1;
	}

	if(m_pCatalogConfigure || m_pCatalogState)
	{
		Clog( LOG_DEBUG, _T("CDODChannel::m_pCatalogConfigure was initialization !"));
		return 1;
	}

	if(m_pWrapperControl)
	{
		Clog( LOG_DEBUG, _T("CDODChannel::m_pWrapperControl was initialization !"));
		return 1;
	}

	if (SUCCEEDED(hr))	
		hr = m_pDataWatcherSource->QueryInterface(IID_ICatalogConfigure,(void **)&m_pCatalogConfigure);

	if (SUCCEEDED(hr))	
		hr = m_pDataWatcherSource->QueryInterface(IID_ICatalogState,(void **)&m_pCatalogState);

	if (SUCCEEDED(hr))	
		hr = m_pDataWrapper->QueryInterface(IID_IWrapperControl,(void **)&m_pWrapperControl);

	if (FAILED(hr))	
	{
		Clog( LOG_ERR, _T("CDODChannel::CreateInterface error."));
		return 1;
	}

	return 0;
}

//ICatalogConfigure methods
//Set catalog name -- which be monitored by DataWatcherSource filter.
DWORD 	CDODChannel::SetCatalogName(TCHAR *pszCatalog)
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetCatalogName CatalogName=%s m_pCatalogConfigure is null error."),pszCatalog);
		return 1;
	}
	WCHAR wFile[MAX_PATH];

	//end test
#ifndef UNICODE
	MultiByteToWideChar(CP_ACP, 0, pszCatalog, -1, wFile, MAX_PATH);
#else
	lstrcpy(wFile, pszCatalog);
#endif
	
	HRESULT hr = S_OK;
	hr=m_pCatalogConfigure->SetCatalogName(wFile);

	if (FAILED(hr))	
	{
		Clog( LOG_ERR, _T("CatalogConfigure->SetCatalogName error."));
		return 1;
	}

	//zhenan 20060818 throw out these code,because run_command will call UpdateCatalog
	//BOOL flag;
	//hr=m_pCatalogConfigure->GetDetectedFlag(&flag);
	//if (S_OK==hr)	
	//{	
	//	if(flag==FALSE)
	//	{
	//		hr=m_pCatalogConfigure->UpdateCatalog();
	//	}
	//}

	if (FAILED(hr))	
	{
		Clog( LOG_ERR, _T("CatalogConfigure->SetCatalogName or GetDetectedFlagerror."));
		return 1;
	}
	return 0;

}
DWORD	CDODChannel::GetCatalogName(TCHAR *pszCatalog)
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_DEBUG, _T("CDODChannel::GetCatalogName m_pCatalogConfigure is null error."));
		return 1;
	}
//	WCHAR wFile[MAX_PATH];

	LPOLESTR ppszCatalog;
	HRESULT hr = S_OK;
	//char cstr[MAX_PATH];
	hr=m_pCatalogConfigure->GetCatalogName(&ppszCatalog);

	if (S_OK!=hr)	
	{
		Clog( LOG_ERR, _T("CatalogConfigure->GetCatalogName error."));
		return hr;
	}
	WideCharToMultiByte(CP_ACP, 0, ppszCatalog, -1, pszCatalog, 256, 0, 0);

	return 0;
}
DWORD	CDODChannel::UpdateCatalog()
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::UpdateCatalog m_pCatalogConfigure is null, error."));
		return 1;
	}

	return m_pCatalogConfigure->UpdateCatalog();
}
DWORD	CDODChannel::SetDetectedFlag(BOOL bDetected)
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetDetectedFlag  m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogConfigure->SetDetectedFlag(bDetected);
}
DWORD	CDODChannel::GetDetectedFlag (BOOL* bDetected)
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetDetectedFlag m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogConfigure->GetDetectedFlag(bDetected);
}
DWORD	CDODChannel::SetDetectedInterval(long lInterval) 
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetDetectedInterval m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogConfigure->SetDetectedInterval(lInterval);
}
DWORD	CDODChannel::GetDetectedInterval (long* lInterval)
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetDetectedInterval m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogConfigure->GetDetectedInterval(lInterval);
}
DWORD	CDODChannel::SetSubChannelRate(vector<DWS_SubChannelList > pSubChannellists, long nSubChannel)
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetCatalogName m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogConfigure->SetSubChannelRate(pSubChannellists,nSubChannel); 
}
DWORD   CDODChannel::SetChannelEvent(HANDLE hStateEvent)
{
	if(m_pCatalogConfigure==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetCatalogName m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogConfigure->SetChannelEvent(hStateEvent);
}

/*  ICatalogState methods */
DWORD CDODChannel::SetCatalogStateEvent(HANDLE hStateEvent)
{
	if(m_pCatalogState==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetCatalogStateEvent m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogState->SetCatalogStateEvent(hStateEvent);
}

DWORD CDODChannel::GetLastErrorCode(int* nError)
{
	if(m_pCatalogState==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetLastErrorCode m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogState->GetLastErrorCode(nError);
}
DWORD CDODChannel::GetCurrentStatus(int* pnStatus)
{
	if(m_pCatalogState==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetCurrentStatus m_pCatalogConfigure is null error."));
		return 1;
	}

	return m_pCatalogState->GetCurrentStatus(pnStatus);
}
// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê3ÔÂ31ÈÕ 9:51:12

//for CDataWrapperFilter interface, extern

DWORD CDODChannel::SetWrapParaSet( DW_ParameterSet inPara )
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetWrapParaSet m_pWrapperControl is null error."));
		return 1;
	}
	return m_pWrapperControl->SetWrapParaSet(inPara);
}
DWORD CDODChannel::GetWrapParaSet( DW_ParameterSet * outPara )
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetWrapParaSet m_pWrapperControl is null error."));
		return 1;
	}
	return m_pWrapperControl->GetWrapParaSet(outPara);
}
DWORD CDODChannel::SetChannelPID(long inPID)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetChannelPID m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetChannelPID(inPID);
}
DWORD CDODChannel::GetChannelPID(long * outPID)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetChannelPID m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetChannelPID(outPID);
}
DWORD CDODChannel::SetChannelTag(const char * inTag, BYTE inLength)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetChannelTag m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetChannelTag(inTag,inLength);
}
DWORD CDODChannel::GetChannelTag(char * outTag, BYTE * outLength)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetChannelTag m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetChannelTag(outTag,outLength);
}
DWORD CDODChannel::AddVersionNumber()
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::AddVersionNumber m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->AddVersionNumber();
}
DWORD CDODChannel::ResetVersionNumber()
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::ResetVersionNumber m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->ResetVersionNumber();
}
DWORD CDODChannel::SetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::ResetVersionNumber m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetVersionNumber(inObjectKey,inObjectKeyLen,inVersion);
}
DWORD CDODChannel::GetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetVersionNumber m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetVersionNumber(inObjectKey,inObjectKeyLen,outVersion);
}
DWORD CDODChannel::SetObjectKeyLength(BYTE inLength)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetObjectKeyLength m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetObjectKeyLength(inLength);
}
DWORD CDODChannel::GetObjectKeyLength(BYTE *outLength)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetObjectKeyLength m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetObjectKeyLength(outLength);
}
DWORD CDODChannel::SetIndexDescLength(BYTE inLength)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetIndexDescLength m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetIndexDescLength(inLength);
}
DWORD CDODChannel::GetIndexDescLength(BYTE *outLength)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetIndexDescLength m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetIndexDescLength(outLength);
}	

DWORD CDODChannel::SetEncrypt(BYTE inEncrypt)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetEncrypt m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetEncrypt(inEncrypt);
}

DWORD CDODChannel::GetEncrypt(BYTE * outEncrypt)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetEncrypt m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetEncrypt(outEncrypt);
}

DWORD CDODChannel::SetMode(int innMode)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetMode m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetMode(innMode);
}

DWORD CDODChannel::GetMode(BYTE *OutnMode)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetMode m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetMode(OutnMode);
}
DWORD CDODChannel::SetStreamCount(int inCount,char *pcharPath)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::SetStreamCount m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->SetStreamCount(inCount,pcharPath);
}
DWORD CDODChannel::GetStreamCount(int *OutCount)
{
	if(m_pWrapperControl==NULL)
	{
		Clog( LOG_ERR, _T("CDODChannel::GetStreamCount m_pWrapperControl is null.parameter error."));
		return 1;
	}
	return m_pWrapperControl->GetStreamCount(OutCount);

}
/*
DWORD CDODChannel::SetObjectKey( const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen )
{
	if(m_pWrapperControl==NULL)
		return 1;
	m_pWrapperControl->SetObjectKey(inObjectName,inObjectNameLen,inObjectKey,inObjectKeyLen);
	return 0;
}
DWORD CDODChannel::GetObjectKey( const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen)
{
	if(m_pWrapperControl==NULL)
		return 1;
	m_pWrapperControl->GetObjectKey(inObjectName,inObjectNameLen,outObjectKey,outObjectKeyLen);
	return 0;
}
DWORD CDODChannel::SetMaxSizeOfSample(long inSize)
{
	if(m_pWrapperControl==NULL)
		return 1;
	m_pWrapperControl->SetMaxSizeOfSample(inSize);
	return 0;
}
DWORD CDODChannel::GetMaxSizeOfSample(long *outSize)
{
	if(m_pWrapperControl==NULL)
		return 1;
	m_pWrapperControl->GetMaxSizeOfSample(outSize);
	return 0;
}	
DWORD CDODChannel::SetMinSizeOfSample(long inSize)
{
	if(m_pWrapperControl==NULL)
		return 1;
	m_pWrapperControl->SetMinSizeOfSample(inSize);
	return 0;
}
DWORD CDODChannel::GetMinSizeOfSample(long *outSize)
{
	if(m_pWrapperControl==NULL)
		return 1;
	m_pWrapperControl->GetMinSizeOfSample(outSize);
	return 0;
}	*/

































