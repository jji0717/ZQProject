//------------------------------------------------------------------------------
// File: AsyncFlt.cpp
//
// Desc: DirectShow sample code - implementation of CCatalogSourceFilter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include <streams.h>

#include "asyncio.h"
#include "asyncrdr.h"

#pragma warning(disable:4710)  // 'function' not inlined (optimization)
#include "asyncflt.h"
#include "resource1.h"
#include "comdef.h"
//
// Setup data for filter registration
//
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{ &MEDIATYPE_Stream     // clsMajorType

};

const AMOVIESETUP_PIN sudOpPin =
{ L"Output"          // strName
, FALSE              // bRendered
, TRUE               // bOutput
, FALSE              // bZero
, FALSE              // bMany
, &CLSID_NULL        // clsConnectsToFilter
, L"Input"           // strConnectsToPin
, 1                  // nTypes
, &sudOpPinTypes };  // lpTypes

const AMOVIESETUP_FILTER sudAsync =
{ &CLSID_IDataWatcherSource // clsID
, L"DataWatcherSource"		 // strName
, MERIT_PREFERRED    // dwMerit
, 1                  // nPins
, &sudOpPin };       // lpPin


//
//  Object creation template
//
CFactoryTemplate g_Templates[] = {
    { 
		L"DataWatcherSource"
		, &CLSID_IDataWatcherSource
		, CCatalogSourceFilter::CreateInstance
		, NULL
		, &sudAsync 
	},

	{
		L"Catalog Configurations",
		&CLSID_CatalogConfigurePage,
		CCatalogConfigPage::CreateInstance,//added by Rose
		NULL,
		NULL
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	SMGLog_DllMain(hModule, dwReason);

	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


//* Create a new instance of this class
CUnknown * WINAPI CCatalogSourceFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	ASSERT(phr);
	
    return new CCatalogSourceFilter(pUnk, phr);
}


BOOL CCatalogSourceFilter::ReadTheFile(LPCTSTR lpszFileName)
{
	
    DWORD dwBytesRead=0;
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
		smglog(SMGLOG_ERROR, m_dwID, " CreateFile error(%s),errorcode=%d",lpszFileName,GetLastError());
        return FALSE;
    }
    ULARGE_INTEGER uliSize;
    uliSize.LowPart = GetFileSize(hFile, &uliSize.HighPart);

    PBYTE pbMem = new BYTE[uliSize.LowPart];
    if (pbMem == NULL) 
    {
		m_nErrorCode=OUTOFMEMORY;
		smglog(SMGLOG_ERROR, m_dwID, "New read buffer(%s),errorcode=%d",lpszFileName,GetLastError());
        CloseHandle(hFile);
        return FALSE;
    }

    if (!ReadFile(hFile,
                  (LPVOID) pbMem,
                  uliSize.LowPart,
                  &dwBytesRead,
                  NULL) ||
        (dwBytesRead != uliSize.LowPart))
    {
        //DbgLog((LOG_TRACE, 1, TEXT("Could not read file\n")));
		m_nErrorCode=READ_FILE_FAILED;

		smglog(SMGLOG_ERROR, m_dwID, "Read file failed(%s),errorcode=%d",lpszFileName,GetLastError());		
        delete [] pbMem;pbMem=NULL;
        CloseHandle(hFile);
        return FALSE;
    }

	m_pbData = pbMem;
	m_llSize = (LONGLONG)uliSize.QuadPart;

	delete [] pbMem; pbMem=NULL;
    CloseHandle(hFile);

    return TRUE;
}
STDMETHODIMP CCatalogSourceFilter::GetPages(CAUUID* pPages)
{
	if(!pPages)
		return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems = reinterpret_cast<GUID*>(CoTaskMemAlloc(sizeof(GUID)));
	if( pPages->pElems == NULL )
	{
			m_nErrorCode=OUTOFMEMORY;
			smglog(SMGLOG_ERROR, m_dwID, "GetPages reinterpret_cast pElems is null,errorcode=%d",GetLastError());		
		return E_OUTOFMEMORY;
	}

	*(pPages->pElems) = CLSID_CatalogConfigurePage;

	return S_OK;
}



STDMETHODIMP CCatalogSourceFilter::Run(REFERENCE_TIME tStart)
{
	if (m_pszCatalog)
	{
		int cch = lstrlenW(m_pszCatalog) + 1;
		TCHAR lpszFileName[FILENAMELEN]={0};
		WideCharToMultiByte(GetACP(), 0,m_pszCatalog , -1,lpszFileName, cch*2, NULL, NULL);
		smglog(SMGLOG_INFO, m_dwID,"Run :%s",lpszFileName);
	}
	else
		smglog(SMGLOG_INFO, m_dwID, "Run.");	

	if(m_bDetectedFlag==TRUE)
	{
		if(m_iSetCatalogTime==0)
		{
			if( NULL != m_hMonitor ) 
			{
				this->CloseThread( );
				Sleep(1);
			}
			MonitorCatalogThread();
			m_iSetCatalogTime++;
		}
	}
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月28日 13:35:28
	// Add else for prevent search file was called at the same time.
	else
		UpdateCatalog();	

	return CBaseFilter::Run(tStart);
}

//ICatalogConfigure methods
//Set catalog name -- which be monitored by DataWatcherSource filter.
STDMETHODIMP CCatalogSourceFilter::SetCatalogName(LPCOLESTR pszCatalog)
{
	USES_CONVERSION;
	CheckPointer(pszCatalog,E_POINTER);

	// lstrlenW is one of the few Unicode functions that works on win95
	int cch = lstrlenW(pszCatalog) + 1;

	if( NULL != m_pszCatalog ) { delete [] m_pszCatalog; m_pszCatalog=NULL;}
	m_pszCatalog = new WCHAR[cch];

	if (m_pszCatalog!=NULL)
		CopyMemory(m_pszCatalog, pszCatalog, cch*sizeof(WCHAR));

	cch = lstrlenW(m_pszCatalog) + 1;
	TCHAR lpszFileName[FILENAMELEN]={0};

	WideCharToMultiByte(GetACP(), 0,m_pszCatalog , -1,lpszFileName, cch*2, NULL, NULL);

	smglog(SMGLOG_INFO, m_dwID, "SetCatalogName bDetectedFlag is %d.CatalogName.%s",m_bDetectedFlag,lpszFileName);	
	
	//only for test
	LPOLESTR ppszCatalog;
	GetCatalogName(&ppszCatalog);
	//end test
	return S_OK;
}
//Get catalog name
STDMETHODIMP CCatalogSourceFilter::GetCatalogName(LPOLESTR* ppszCatalog)
{
	CheckPointer(ppszCatalog, E_POINTER);
	
	*ppszCatalog = NULL;

	//ppszCatalog
	if ( m_pszCatalog != NULL) 
	{
		*ppszCatalog = (LPOLESTR)QzTaskMemAlloc(sizeof(WCHAR)*(1+lstrlenW( m_pszCatalog)));
		if (*ppszCatalog != NULL) 
		{
			lstrcpyW(*ppszCatalog, m_pszCatalog);
		}
		else
		{
			m_nErrorCode=OUTOFMEMORY;
			smglog(SMGLOG_ERROR, m_dwID, "GetCatalogName QzTaskMemAlloc errorcode=%d",GetLastError());	
			return E_OUTOFMEMORY;
		}
	}
	return S_OK;
}
//Update catalog command.
//DataWatcherSource filter will send subchannel list to the extend module once received it. 
STDMETHODIMP	CCatalogSourceFilter::UpdateCatalog() 
{	
	smglog(SMGLOG_DETAIL, m_dwID, "UpdateCatalog start:");	

	SetEvent(m_hEvtMonitor);//stop monitor catalog thread.

	// why ..................... zhenan add 20060727
	while (m_nCanUpdateCatalog ==0)
	{
		if (m_IsStop)
		{
			smglog(SMGLOG_DETAIL, m_dwID, "UpdateCatalog : received stop command");	
			return S_OK;
		}

		Sleep(2);
	}

	// lstrlenW is one of the few Unicode functions that works on win95
	int cch = lstrlenW(m_pszCatalog) + 1;
	TCHAR lpszFileName[FILENAMELEN]={0};
	//transfom m_pszCatalog to TCHAR data type
#ifndef UNICODE
	WideCharToMultiByte(GetACP(), 0,m_pszCatalog , -1,lpszFileName, cch*2, NULL, NULL);
	#else		
			lstrcpy(lpszFileName, m_pszCatalog);
	#endif

	ClearFileVector();
	//return S_OK;
	//SearchPath
	smglog(SMGLOG_INFO, m_dwID, "UpdateCatalog SearchPath(%s) before",lpszFileName);	
	SearchPath( lpszFileName );		
	smglog(SMGLOG_INFO, m_dwID, "UpdateCatalog SearchPath path number is %d",Catalogs.size());	

	//for_each( pThis->Catalogs.begin(), pThis->Catalogs.end(),  SearchFile );
	for(std::vector<CCatalog>::iterator it=Catalogs.begin();it!=Catalogs.end();it++)
	{
		if (m_IsStop)
		{
			smglog(SMGLOG_DETAIL, m_dwID, "UpdateCatalog SearchFile : received stop command");	
			return S_OK;
		}
		SearchFile(it);
	}
	smglog(SMGLOG_INFO, m_dwID, "UpdateCatalog SearchFile ok ,file_number is %d",SubChannellists.size());	
	OnCatalogStatus(SubChannellists);
	smglog(SMGLOG_DETAIL, m_dwID, "UpdateCatalog OnCatalogStatus SubChannellists ok");	

	ResetEvent(m_hEvtMonitor);//restart monitor catalog thread.
	return S_OK;
}
//Set detected flag --	bDetected=true, monitor catalog thread run. bDetected=false, monitor catalog thread stop.
STDMETHODIMP	CCatalogSourceFilter::SetDetectedFlag (BOOL bDetected) 
{
	smglog(SMGLOG_INFO, m_dwID, "SetDetectedFlag bDetected=%d",bDetected);	
	if(bDetected == TRUE)
		ResetEvent(m_hEvtMonitor);//start monitor catalog thread.
	else
		SetEvent(m_hEvtMonitor);//stop monitor catalog thread.

	m_bDetectedFlag = bDetected;
	return S_OK;
}
//Get detected flag
STDMETHODIMP	CCatalogSourceFilter::GetDetectedFlag (BOOL* bDetected)
{
	smglog(SMGLOG_INFO, m_dwID, "GetDetectedFlag bDetected=%d",m_bDetectedFlag);	
	CheckPointer(bDetected,E_POINTER);
	*bDetected=m_bDetectedFlag;
	return S_OK;
}
//Set detected interval -- Set detected interval for monitor catalog thread. The default value is 0.
STDMETHODIMP	CCatalogSourceFilter::SetDetectedInterval (long lInterval)
{
	smglog(SMGLOG_INFO, m_dwID, "SetDetectedInterval lInterval=%ld",lInterval);	
	m_lDetectedInterval = lInterval;
	return S_OK;
}
//Get detected interval
STDMETHODIMP	CCatalogSourceFilter::GetDetectedInterval (long* lInterval)
{
	smglog(SMGLOG_INFO, m_dwID, "GetDetectedInterval lInterval=%ld",m_lDetectedInterval);		
	CheckPointer(lInterval,E_POINTER);
	*lInterval=m_lDetectedInterval;
	return S_OK;
}
//Get catalogs -- Get catalogs request—The extend module can get subchannel list from DataWatcherSource filter through this interface function.
STDMETHODIMP	CCatalogSourceFilter::GetCatalogs (vector<DWS_SubChannelList > *pSubChannellists) 
{
	smglog(SMGLOG_DETAIL, m_dwID, "GetCatalogs interface is called");			
	EnterCriticalSection(&m_lock);
	*pSubChannellists = SubChannellists;
	LeaveCriticalSection(&m_lock);
	return S_OK;
}
//Set channel event -- When subchannel list changed, DataWatcherSource filter notify the extend module through set event.
STDMETHODIMP	CCatalogSourceFilter::SetChannelEvent(HANDLE hStateEvent) 
{
	smglog(SMGLOG_INFO, m_dwID, "SetChannelEvent interface is called.--%d",hStateEvent);	
	m_hEvtSubChannelStateEvent = hStateEvent;
	return S_OK;
}
//Set subChannel rate -- Set repeat times of files  in subchannel list.
STDMETHODIMP	CCatalogSourceFilter::SetSubChannelRate(vector<DWS_SubChannelList > pSubChannellists, long nSubChannel)
{
	smglog(SMGLOG_DETAIL, m_dwID, "SetSubChannelRate interface is called");	
	// Define a template class vector of integers
    typedef vector<DWS_SubChannelList  > IntVector ;

    //Define an iterator for template class vector of integer
    typedef IntVector::iterator IntVectorIt ;

    IntVectorIt newlist,oldlist ;
	//Lock SubChannellists here
	EnterCriticalSection(&m_lock);
	for(newlist = pSubChannellists.begin(); newlist != pSubChannellists.end(); newlist++)
	{
		for(oldlist = SubChannellists.begin(); oldlist != SubChannellists.end(); oldlist++)
		{
			if(strcmp((*oldlist).szFileName , (*newlist).szFileName) == 0)
			{
				(*oldlist).dwRepeat = (*newlist).dwRepeat;
				smglog(SMGLOG_INFO, m_dwID, "SetSubChannelRate)--FileName is %s,FileRepeatTimes is %d.",(*oldlist).szFileName,(*newlist).dwRepeat);				
				break;
			}
		}
	}
	//UnLock SubChannellists here
	LeaveCriticalSection(&m_lock);
	return S_OK;
}

//ICatalogState methods
//Set catalog state event -- when filter status changed it can inform the extend module.
STDMETHODIMP CCatalogSourceFilter::SetCatalogStateEvent(HANDLE hStateEvent)
{
	m_catalogStateEvent = hStateEvent;
	return S_OK;
}
//Get last error code 
STDMETHODIMP CCatalogSourceFilter::GetLastErrorCode(int* nError)
{
	CheckPointer(nError,E_POINTER);
	*nError=m_nErrorCode;
	return S_OK;
}
//Get current filter status
STDMETHODIMP CCatalogSourceFilter::GetCurrentStatus(int* pnStatus)
{
	CheckPointer(pnStatus,E_POINTER);
	*pnStatus=m_currentStatus;
	return S_OK;
}
//end ICatalogState
//Clear file vector -- clear catalogs
void CCatalogSourceFilter::ClearFileVector()
{
	Catalogs.clear();
	SubChannellists.clear();
}
//load Catalogs
BOOL CCatalogSourceFilter::DealPath( TCHAR * CurrentPath )
{
	WORD catalogid = 0;
	CCatalog pCatalog;
	pCatalog.wID = catalogid++;
	pCatalog.wCount = 0;
	_tcscpy( pCatalog.szPath, CurrentPath );
	Catalogs.push_back( pCatalog );
	return TRUE;
}
//Search path
BOOL CCatalogSourceFilter::SearchPath( TCHAR * CurrentPath )
{
	if( !CurrentPath )
		return FALSE;

	if (CurrentPath[_tcslen(CurrentPath)-1] != '\\')
	{
		_tcscat( CurrentPath, "\\" );
	}

	TCHAR szWildpath[MAX_PATH];		//260
	_tcscpy( szWildpath, CurrentPath );
	_tcscat( szWildpath, "\\*.*" );
	
	WIN32_FIND_DATA FindFileData;
	TCHAR szTempPath[MAX_PATH];	

// ------------------------------------------------------ Modified by zhenan_ji at 2005年9月12日 15:24:10
//Change the current working directory.This is lead to delete this dir error.
/*	if( _tchdir( CurrentPath ) != 0 )
	{
		return FALSE;
	}
*/		
	DealPath( CurrentPath );

	HANDLE hFindFile = FindFirstFile( szWildpath, &FindFileData );
	if( hFindFile == INVALID_HANDLE_VALUE )
		return FALSE;

	do{
      if( FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )       //子目录?
      {
		  if (m_IsStop)
		  {
			  smglog(SMGLOG_DETAIL, m_dwID, "SearchPath : received stop command");			
			  break;
		  }
          if( _tcscmp( FindFileData.cFileName, "." ) != 0 
			&& _tcscmp( FindFileData.cFileName, ".." ) != 0 )
          {
              _tcscpy( szTempPath, CurrentPath );
			  _tcscat( szTempPath, FindFileData.cFileName ); 
			  _tcscat( szTempPath, "\\" );

			  SearchPath( szTempPath );
          }
	  }
	}while( FindNextFile( hFindFile, &FindFileData ) );

	FindClose( hFindFile );
	return TRUE;
}

//Monitor catalog thread
BOOL CCatalogSourceFilter::MonitorCatalogThread()
{
	DWORD dwThreadID = 0;
	m_hEvtMonitor =  CreateEvent(NULL,TRUE,FALSE,NULL);
	if(m_hEvtMonitor==NULL)
	{
		m_nErrorCode=CREATE_EVENT_FAILED;
		smglog(SMGLOG_ERROR, m_dwID, "Create m_hEvtMonitor event failed errorcode=%d",GetLastError());			
		return FALSE;
	}
	m_hEvtStop = CreateEvent(NULL,FALSE,FALSE,NULL);
	if(m_hEvtStop==NULL)
	{
		m_nErrorCode=CREATE_EVENT_FAILED;
		smglog(SMGLOG_ERROR, m_dwID, "Create m_hEvtStop event failed errorcode=%d",GetLastError());		
		CloseHandle(m_hEvtMonitor);
		m_hEvtMonitor = 0;
		return FALSE;
	}
	//ResetEvent(m_hEvtStop);
	ResetEvent(m_hEvtMonitor);
	if(m_bDetectedFlag==TRUE)//SetDetectedFlag interface
	{
		m_hMonitor = CreateThread(NULL,0,MonitorCatalog,(LPVOID)this,0,&dwThreadID);

		if(m_hMonitor == 0)
		{
			m_nErrorCode=CREATE_THREAD_FAILED;

			smglog(SMGLOG_ERROR, m_dwID, "Create m_hMonitor thread failed errorcode=%d",GetLastError());	
			return FALSE;
		}
	}
	return TRUE;
}

//Monitor catalog
DWORD WINAPI CCatalogSourceFilter::MonitorCatalog(LPVOID lParam)
{
	const DWORD cTimeout = 100;                 //wait time out.
	const DWORD cPauseTimeout = 10;             //sleep time when paused.
	const DWORD cHandes = 2;                    //number of handes to wait for.
	CCatalogSourceFilter* pThis = (CCatalogSourceFilter* )lParam; //param from StartExchangeThread.
	HANDLE hEvents[cHandes] = {pThis->m_hEvtStop,pThis->m_hEvtMonitor};//handes to wait for.
	DWORD dwStatus ;
	DWORD dwReturn = 0;
	//	HRESULT hr;// = ::CoInitialize(NULL);
	smglog(SMGLOG_DETAIL, pThis->m_dwID,"Start monitor catalog thread");	

	int numberSleep=0,numberCircel=0,nSleepIndex=0;
	if( pThis->m_lDetectedInterval >0)
	{
		numberCircel=5;
		numberSleep = pThis->m_lDetectedInterval /numberCircel;
	}
	while(1)
	{
		if (pThis->m_IsStop)
		{
			smglog(SMGLOG_DETAIL, pThis->m_dwID,"MonitorCatalog process: received stop command ");
			break ;
		}

		try
		{
			dwStatus = WaitForMultipleObjects(cHandes,hEvents,FALSE,cTimeout);
			//exit thread now.
			if(dwStatus == WAIT_OBJECT_0)       
			{
				pThis->m_currentStatus = STATE_STOP;
				dwReturn = 0;
				smglog(SMGLOG_DETAIL, pThis->m_dwID,"Monitor catalog stop ");
				break;
			}
			//dont Read data temporary.
			else if(dwStatus == WAIT_OBJECT_0+1)
			{
				pThis->m_currentStatus = STATE_READY;
				pThis->m_nCanUpdateCatalog=1;
				Sleep(cPauseTimeout);
				smglog(SMGLOG_DETAIL, pThis->m_dwID,"Monitor catalog sleep for 10 milliseconds.Thread continue");
				continue;
			}
			//read data with client through named pipe.
			else                                
			{
				// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 13:38:32
				if(WaitForSingleObject(pThis->m_hEvtStop,0)==WAIT_OBJECT_0)
				{
					return 1;
				}
				pThis->m_nCanUpdateCatalog=0;
				pThis->m_currentStatus = STATE_MONITORING;
				//TempSubChannellists = SubChannellists;
				pThis->TempSubChannellists = pThis->SubChannellists;
				pThis->ClearFileVector();

				// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 14:39:08
				TCHAR lpszFileName[(MAX_PATH * 2)];
				//transfom m_pszCatalog to TCHAR data type
				int cch = lstrlenW(pThis->m_pszCatalog ) + 1;
#ifndef UNICODE				
				WideCharToMultiByte(GetACP(), 0,pThis->m_pszCatalog , -1,lpszFileName, cch*2, NULL, NULL);
#else
				//	TCHAR lpszFileName[MAX_PATH]={0};
				lstrcpy(lpszFileName, pThis->m_pszCatalog);
#endif

				//SearchPath
				pThis->SearchPath( lpszFileName );
				//for_each( pThis->Catalogs.begin(), pThis->Catalogs.end(),  SearchFile );
				for(std::vector<CCatalog>::iterator it=pThis->Catalogs.begin();it!=pThis->Catalogs.end();it++)
				{
					if (pThis->m_IsStop)
					{			
						smglog(SMGLOG_DETAIL, pThis->m_dwID,"MonitorCatalog process:SearchFile  received stop command");
						break ;
					}
					pThis->SearchFile(it);
				}
				if( ( pThis->CompareSubchanelList(pThis->SubChannellists,pThis->TempSubChannellists) ) == 1)
				{
					smglog(SMGLOG_DETAIL, pThis->m_dwID,"SubChannellist changed,sourcefilter will call callback function");
					pThis->m_currentStatus = STATE_REPORTING;
					SetEvent(pThis->m_hEvtSubChannelStateEvent);
					pThis->OnCatalogStatus(pThis->SubChannellists);   	
				}
				else if(pThis->m_bConnectSuccessful==FALSE)
				{
					smglog(SMGLOG_DETAIL, pThis->m_dwID,"DataWatcher did not connect with DataWrapper,sourcefilter try to get callback function pointer");
					pThis->OnCatalogStatus(pThis->SubChannellists); 
				}
				pThis->m_nCanUpdateCatalog=1;

				for (nSleepIndex = 0;nSleepIndex < numberCircel ; nSleepIndex ++ )
				{
					if (pThis->m_IsStop)
					{
						smglog(SMGLOG_DETAIL, pThis->m_dwID," MonitorCatalog process:sleep  received stop command");
						break ;
					}
					Sleep(numberSleep);
				}
				//Sleep(pThis->m_lDetectedInterval);//SetDetectedInterval() function
				//}
			}
		}
		catch(...)
		{
			pThis->m_nErrorCode=MONITORCATALOG_THREAD_EXCEPTION;
			smglog(SMGLOG_ERROR, pThis->m_dwID,"Monitor thread exception!  error %d",GetLastError());
		}
	}
	smglog(SMGLOG_DETAIL, pThis->m_dwID," Monitor thread exit ok");
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 13:40:34
	//	::CoUninitialize();
	return dwReturn;
}

//Compare subchanel list
BOOL CCatalogSourceFilter::CompareSubchanelList(vector<DWS_SubChannelList > NewSubChannellists,vector<DWS_SubChannelList > OldSubChannellists)
{
	DWORD dwReturn = 0;
	// Define a template class vector of integers
	typedef vector<DWS_SubChannelList  > IntVector ;

	//Define an iterator for template class vector of integer
	typedef IntVector::iterator IntVectorIt ;

	IntVectorIt newlist,oldlist ;
	int i,j;
	//Lock SubChannellists here
	//EnterCriticalSection(&m_lock);
	for(newlist = NewSubChannellists.begin(),i=0; newlist != NewSubChannellists.end(); newlist++,i++);
	for(oldlist = OldSubChannellists.begin(),j=0; oldlist != OldSubChannellists.end(); oldlist++,j++);
	if(i!=j)
		dwReturn = 1;
	else
	{
		for(newlist = NewSubChannellists.begin(); newlist != NewSubChannellists.end(); newlist++)
		{	
			Sleep(1);

			if (m_IsStop)
			{
				smglog(SMGLOG_DETAIL, m_dwID,"MonitorCatalog process:CompareSubchanelList NewSubChannellists received stop command ");
				return  0;
			}

			int k=0;
			for(oldlist = OldSubChannellists.begin(); oldlist != OldSubChannellists.end(); oldlist++)
			{
				Sleep(1);

				if (m_IsStop)
				{
					smglog(SMGLOG_DETAIL, m_dwID,"MonitorCatalog process:CompareSubchanelList  OldSubChannellists received stop command ");
					return 0;
				}

				//sprintf(strLog,"%s",((*oldlist).szFileName));//only for test
				//sprintf(strLog,"%s",((*newlist).szFileName));//only for test
				if(	(strcmp((*oldlist).szFileName , (*newlist).szFileName) == 0) && 
					(((*oldlist).ftCreate.dwHighDateTime != (*newlist).ftCreate.dwHighDateTime || (*oldlist).ftCreate.dwLowDateTime != (*newlist).ftCreate.dwLowDateTime) || 
					//((*oldlist)->ftAccess.dwHighDateTime != (*newlist)->ftAccess.dwHighDateTime  || (*oldlist)->ftAccess.dwLowDateTime != (*newlist)->ftAccess.dwLowDateTime ) ||
					((*oldlist).ftWrite.dwHighDateTime != (*newlist).ftWrite.dwHighDateTime || (*oldlist).ftWrite.dwLowDateTime != (*newlist).ftWrite.dwLowDateTime))
					)
				{
					dwReturn = 1;
					break; 
				}
				else if( (strcmp((*oldlist).szFileName, (*newlist).szFileName) == 0) && 
					((*oldlist).ftCreate.dwHighDateTime == (*newlist).ftCreate.dwHighDateTime && (*oldlist).ftCreate.dwLowDateTime== (*newlist).ftCreate.dwLowDateTime) && 
					((*oldlist).ftAccess.dwHighDateTime == (*newlist).ftAccess.dwHighDateTime && (*oldlist).ftAccess.dwLowDateTime== (*newlist).ftAccess.dwLowDateTime) &&
					((*oldlist).ftWrite.dwHighDateTime == (*newlist).ftWrite.dwHighDateTime  && (*oldlist).ftWrite.dwLowDateTime == (*newlist).ftWrite.dwLowDateTime)
					)
				{
					dwReturn = 0;
					break;
				}
				else if ((strcmp((*oldlist).szFileName , (*newlist).szFileName) != 0) )
				{
					k++;
				}
			}
			if(k==j)
				dwReturn = 1;

			if(dwReturn == 1)
				break;
		}
	}
	//UnLock SubChannellists here
	//LeaveCriticalSection(&m_lock);
	return dwReturn;
}


// stop the thread and close the handle
HRESULT CCatalogSourceFilter::CloseThread(void)
{
// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 13:19:18
	SetEvent(m_hEvtStop);	
	SetEvent(m_hEvtMonitor);
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 13:19:18
	if (m_hMonitor) 
	{
		WaitForSingleObject(m_hMonitor,INFINITE);
	}
	if(m_hEvtStop)
	{
		CloseHandle(m_hEvtStop);
		m_hEvtStop = 0;
	}
	// signal the thread-exit object
	if(m_hEvtMonitor)
	{
		// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 13:21:51
		TerminateThread(m_hEvtMonitor,101);
		CloseHandle(m_hEvtMonitor);
		m_hEvtMonitor = 0;
	}

	ResetEvent(m_hMonitor);
	if(m_hMonitor)
	{
		WaitForSingleObject(m_hMonitor, INFINITE);
		CloseHandle(m_hMonitor);
		m_hMonitor = NULL;
	}
	smglog(SMGLOG_DETAIL, m_dwID,"Close monitor catalog thread");
    return S_OK;
}
//Notify the extend module the latest subchannellist
void CCatalogSourceFilter::OnCatalogStatus(vector<DWS_SubChannelList > pSubChannellist)
{
	if (m_IsStop)
	{
		smglog(SMGLOG_DETAIL, m_dwID,"OnCatalogStatus process: received stop command");
		return ;
	}
	else
	if (m_pszCatalog)
	{
		int cch = lstrlenW(m_pszCatalog) + 1;
		TCHAR lpszFileName[FILENAMELEN]={0};
		WideCharToMultiByte(GetACP(), 0,m_pszCatalog , -1,lpszFileName, cch*2, NULL, NULL);
		smglog(SMGLOG_INFO, m_dwID,"OnCatalogStatus process:%s",lpszFileName);
	}
	else
		smglog(SMGLOG_DETAIL, m_dwID,"OnCatalogStatus process");
	OnCatalogChanged	m_pFunSubChannelChanged;
	LPVOID				m_pParamSubChannelChanged;
	GetDetectedRoutinePara(&m_pFunSubChannelChanged,&m_pParamSubChannelChanged);
	smglog(SMGLOG_DETAIL, m_dwID,"OnCatalogStatus process:GetDetectedRoutinePara after");

	if(m_pFunSubChannelChanged )
	{
		m_pFunSubChannelChanged(pSubChannellist,m_pParamSubChannelChanged);
		m_bConnectSuccessful=TRUE;
	}
	smglog(SMGLOG_DETAIL, m_dwID,"OnCatalogStatus process end !");

}


//End Class CCatalogSourceFilter
//////////////////////////////////////////////////////////////////////////////////////////
//SearchFile,DelCatalog,DealFile
//SearchFile
void CCatalogSourceFilter::SearchFile(std::vector<CCatalog>::iterator it )
{
	char szWildpath[CATALOGPATHLEN];		//260

	
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月14日 14:32:31
	try
	{
		memset(szWildpath,0,CATALOGPATHLEN);
		int lenstr=strlen(it->szPath);
		if (it->szPath[lenstr-1] != '\\')
		{
			strcat( it->szPath, "\\" );
		}
	//	sprintf(strLog,"CCatalogSourceFilter::SearchFile  memset(szWildpath");		LogMyEvent(0,0,strLog);
		if (lenstr > CATALOGPATHLEN -3)
		{
			//it->szPath[lenstr-4]='\0';
			return ;
		}
		//_tcscpy( szWildpath, it->szPath );
		//_tcscat( szWildpath, "*.*" );

		strcpy(szWildpath,it->szPath );
		strcat(szWildpath, "*.*" );
		//	sprintf(strLog,"CCatalogSourceFilter::SearchFile strcpy(szWildpath,it->szPath )",it->szPath);		LogMyEvent(0,0,strLog);
//	sprintf(strLog,"CCatalogSourceFilter::SearchFile strcat(szWildpath, *.* );");		LogMyEvent(0,0,strLog);

		// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月14日 14:34:44

		WIN32_FIND_DATA FindFileData;
		HANDLE hFindFile = FindFirstFile( szWildpath, &FindFileData );
		if( hFindFile == INVALID_HANDLE_VALUE )
			return;

		int iCount = 0;
		char szTempPath[CATALOGPATHLEN];	
		memset(szTempPath,0,CATALOGPATHLEN);
		do{

			if (m_IsStop)
			{
				smglog(SMGLOG_DETAIL, m_dwID,"SearchFile process: received stop command !");
				break ;
			}

			if( FindFileData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY )       //子目录?
			{
// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月22日 11:45:44
				//_tcscpy( szTempPath, it->szPath );
				//_tcscat( szTempPath, FindFileData.cFileName );				
				strncpy(szTempPath,it->szPath, MAX_PATH);
				strncat(szTempPath, FindFileData.cFileName, MAX_PATH);

				LoadSubChannelList( szTempPath, it->wID);//Load SubChannel List
				iCount++;
			}
		}while( FindNextFile( hFindFile, &FindFileData ) );
		FindClose( hFindFile );
		it->wCount = iCount;
//		sprintf(strLog,"CCatalogSourceFilter::SearchFile it->wCount = iCount");		LogMyEvent(0,0,strLog);

	}
	catch (...) 
	{
		smglog(SMGLOG_ERROR, m_dwID,"CCatalogSourceFilter::SearchFile exception! errorcode=%d",GetLastError());
	}
}

void CCatalogSourceFilter::DelCatalog( CCatalog * pCatalog )
{
	if( pCatalog )
	{
		delete pCatalog;
		pCatalog = NULL;
	}
}
//DelSubChannellists
void CCatalogSourceFilter::DelSubChannellists( DWS_SubChannelList * pSubChannelList )
{
	if( pSubChannelList )
	{
		delete pSubChannelList;
		pSubChannelList = NULL;

	}
}
//LoadSubChannelList
BOOL CCatalogSourceFilter::LoadSubChannelList( char * FileName, WORD id  )
{
	DWORD dwSize=0;
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;
	char lpszString[FILENAMELEN];
	/*char strLog[256];*/

	//DWS_SubChannelList *pSubChannel = new DWS_SubChannelList;
	DWS_SubChannelList pSubChannel;
	pSubChannel.wID = id;

// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月22日 13:41:46
	if (strlen(FileName) > FILENAMELEN )
	{
		smglog(SMGLOG_ERROR, m_dwID,"CCatalogSourceFilter::LoadSubChannelList  FileName lenth > FILENAMELEN,filename len=%d",strlen(FileName));
		return FALSE;
	}

	strcpy ( pSubChannel.szFileName ,FileName );	
	HANDLE hFile = ::CreateFile( pSubChannel.szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, 0, NULL );
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//delete pSubChannel;
		//pSubChannel = NULL;
		return FALSE;
	}
	dwSize = GetFileSize(hFile, NULL) ; 
	if (dwSize == INVALID_FILE_SIZE)  
	{
		CloseHandle(hFile);
		return FALSE;
	}
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		 return FALSE;
	
	// Convert the last-write time to local time.
    FileTimeToSystemTime(&ftWrite, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // Build a string showing the date and time.
    sprintf(lpszString, "%02d/%02d/%d  %02d:%02d",
        stLocal.wDay, stLocal.wMonth, stLocal.wYear,
        stLocal.wHour, stLocal.wMinute);

	pSubChannel.dwFileSize = dwSize;
	pSubChannel.ftCreate = ftCreate;
	pSubChannel.ftAccess = ftAccess;
	pSubChannel.ftWrite = ftWrite;
	pSubChannel.dwRepeat = 1;//the default value is 1;
	//dwRepeat
	if(TempSubChannellists.size()!=0)
	{
		for(std::vector<DWS_SubChannelList>::iterator it=TempSubChannellists.begin();it!=TempSubChannellists.end();it++)
		{
			if(	(strcmp((*it).szFileName, pSubChannel.szFileName) == 0)  )
			{
				pSubChannel.dwRepeat=(*it).dwRepeat;
			}
		}
	}
	CloseHandle( hFile );
	SubChannellists.push_back( pSubChannel );
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//class CCatalogConfigPage
CUnknown * WINAPI CCatalogConfigPage::CreateInstance(LPUNKNOWN lpUnknown,HRESULT *pHr)
{
	CUnknown* pUnknown = new CCatalogConfigPage(lpUnknown, pHr);

	return pUnknown;
}

CCatalogConfigPage::CCatalogConfigPage(LPUNKNOWN lpUnknown,HRESULT *pHr):
m_pCatalogConfig(0), 
m_hWndCatalog(0),
CBasePropertyPage(NAME("Catalog Configuration"), lpUnknown, IDD_CATALOGCONFIGURATIONPAGE, IDS_CATALOGCONFIGURATIONPAGE_TITLE)
{
	*pHr = S_OK;
	m_bDirty = 1;
}

CCatalogConfigPage::~CCatalogConfigPage()
{
	if( m_pCatalogConfig )
	{
		m_pCatalogConfig->Release();
		m_pCatalogConfig = 0;
	}
}
HRESULT CCatalogConfigPage::OnConnect(IUnknown *pUnknown)
{
	if( m_pCatalogConfig )
	{
		m_pCatalogConfig->Release();
		m_pCatalogConfig= 0;
	}
	HRESULT hr = pUnknown->QueryInterface(IID_ICatalogConfigure, (void **)&m_pCatalogConfig);
	if( FAILED(hr) )
	{
		m_pCatalogConfig = 0;
		return E_NOINTERFACE;
	}

	return NOERROR;
}

HRESULT CCatalogConfigPage::OnDisconnect()
{
	if( m_pCatalogConfig )
	{
		m_pCatalogConfig->Release();
		m_pCatalogConfig = 0;
	}
	return NOERROR;
}

HRESULT CCatalogConfigPage::OnActivate()
{
	ReflectCatalog();
	return NOERROR;
}

HRESULT CCatalogConfigPage::OnDeactivate()
{
	return NOERROR;
}

HRESULT CCatalogConfigPage::OnApplyChanges()
{
	EnterCatalog();
	return NOERROR;
}

INT_PTR CCatalogConfigPage::OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if( uMsg==WM_INITDIALOG )
	{
		m_hWndCatalog = GetDlgItem(hwnd, IDC_EDIT1_Catalog );
	}
	else if( uMsg==WM_COMMAND )
	{
	}
	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

HRESULT CCatalogConfigPage::ReflectCatalog()
{
	USES_CONVERSION;

	HRESULT hr = S_OK;
	if( m_hWndCatalog==0  || m_pCatalogConfig==0 )
	{
		return E_FAIL;
	}
	LPOLESTR lpCatalog = 0;

	hr = m_pCatalogConfig->GetCatalogName(&lpCatalog);
	if( FAILED(hr) )
	{
		return hr;
	}
	
	SetWindowText(m_hWndCatalog, OLE2A(lpCatalog) );

	return S_OK;
}
HRESULT CCatalogConfigPage::EnterCatalog()
{
	USES_CONVERSION;

	HRESULT hr = S_OK;
	if( m_hWndCatalog==0 || m_pCatalogConfig==0 )
	{
		return E_FAIL;
	}

	char strCatalog[MAX_PATH];
	int bDetectedFlag=0;

	GetWindowText(m_hWndCatalog, strCatalog, MAX_PATH-1);

	hr = m_pCatalogConfig->SetCatalogName(A2OLE(strCatalog));

	return hr;
}