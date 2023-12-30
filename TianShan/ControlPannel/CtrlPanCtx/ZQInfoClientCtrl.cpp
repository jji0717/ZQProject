// ZQInfoClientCtrl.cpp : Implementation of CZQInfoClientCtrl

#include "stdafx.h"
#include "ZQInfoClientControl.h"
#include "ZQInfoClientCtrl.h"
#include "resource.h"
//#define NOAUTOSIZE 1

/////////////////////////////////////////////////////////////////////////////
// CZQInfoClientCtrl

CZQInfoClientCtrl::CZQInfoClientCtrl()
{
	m_bWindowOnly = TRUE;
	m_bAutoSize = FALSE;
}

CZQInfoClientCtrl::~CZQInfoClientCtrl()
{
	if ( m_hLib)
	{
		FreeLibrary(m_hLib);
	}
	if ( m_GribTabDataArray.GetSize() > 0 )
	{
		m_GribTabDataArray.RemoveAll();
	}
}

STDMETHODIMP CZQInfoClientCtrl::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IZQInfoClientCtrl,
	};
	for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
	{
		if (::InlineIsEqualGUID(*arr[i], riid))
			return S_OK;
	}
	return S_FALSE;
}

HRESULT CZQInfoClientCtrl::OnDraw(ATL_DRAWINFO& di)
{
	return S_OK;
}

LRESULT CZQInfoClientCtrl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	InitTabCtrl();
//	InitSNMPRegister();
	return S_OK;
}

LRESULT CZQInfoClientCtrl::OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	CRect rcClient;
	CRect rcNew;
	GetClientRect(&rcClient);
	int iWidth = rcClient.right - rcClient.left;
	int iHight = rcClient.bottom - rcClient.top;

#ifdef  NOAUTOSIZE
	if ( iWidth < 660 && iHight < 500)
	{
		rcNew.left = rcClient.left;
		rcNew.top = rcClient.top;
		rcNew.right = rcClient.left + 660;
		rcNew.bottom = rcClient.top + 500;
		
		::SetWindowPos(m_hWnd,HWND_TOP,rcNew.left,rcNew.top,(rcNew.right-rcNew.left),(rcNew.bottom-rcNew.top),SWP_SHOWWINDOW);
		m_TabCtrl.MoveWindow(rcNew);
	}
	else
	{
		m_TabCtrl.MoveWindow(rcClient);
	}
#else
	m_TabCtrl.MoveWindow(rcClient);
#endif
	return S_OK;
}

LRESULT CZQInfoClientCtrl::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	return S_OK;
}

void CZQInfoClientCtrl::InitSNMPRegister()
{
	TCHAR theFileName[ MAX_PATH ] ={0};
	GetModuleFileName(NULL,theFileName,sizeof(theFileName)/sizeof(theFileName[0]));
	int iSzLen=_tcsclen(theFileName);
	if (  iSzLen > 0 )
	{
			int iTemp=iSzLen-1;
			while (theFileName[iTemp]!='\\' && iTemp >=0 )
				iTemp--;
			if(iTemp>0)
			{
				theFileName[iTemp]='\0';
			}
	}

	_tcscat(theFileName,_T("\\ZQSNMPAgent.dll"));
	DWORD dLen = sizeof(DWORD);
	LONG  status;
	HKEY hKey;
	DWORD           dwDisposition;
	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          _T("SOFTWARE\\ZQ Interactive\\ZQSnmpExtension"),
                          0,
                          KEY_ALL_ACCESS,
                          &hKey);
	if ( status != ERROR_SUCCESS)
	{
	
		status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                _T("SOFTWARE\\ZQ Interactive\\ZQSnmpExtension"),
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
	
		if (ERROR_SUCCESS == status)
		{
			dLen = _tcsclen(theFileName) * 2;
			status = RegSetValueEx(hKey,
                          _T("Pathname"),
						  0,
                          REG_SZ,
						  (BYTE*)theFileName,
						  dLen
						  );
   		}
	}
	RegCloseKey(hKey);

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          _T("SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters\\ExtensionAgents"),
                          0,
                          KEY_ALL_ACCESS,
                          &hKey);
	
	memset(theFileName,0,sizeof(theFileName));
	_stprintf(theFileName,_T("%s"),_T("SOFTWARE\\ZQ Interactive\\ZQSnmpExtension"));
	dLen = _tcsclen(theFileName) * 2;
	
    if (ERROR_SUCCESS != status)
    {
          status = RegSetValueEx(hKey,
                                 _T("20"),
								 0,
								 REG_SZ,
								 (BYTE*)theFileName,
                                 dLen);
	}
	RegCloseKey(hKey);
}

void CZQInfoClientCtrl::InitTabCtrl()
{
	CRect rcClient;
	CRect rcNew;
	GetClientRect(&rcClient);
	rcNew.left = rcClient.left;
	rcNew.top = rcClient.top;
	rcNew.right = rcClient.left + 660;
	rcNew.bottom = rcClient.top + 500;
#ifdef  NOAUTOSIZE
	m_TabCtrl.Create( m_hWnd, rcNew, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE );
#else
	m_TabCtrl.Create( m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE );
#endif
	
	
	CImageList     Imagelist;
	Imagelist.Create( IDB_TABBITMAP, 16, 1, RGB( 0,255,0 ));
//	Imagelist.Create( 48, 16,  ILC_COLORDDB | ILC_MASK, 0, 1);

	m_TabCtrl.SetImageList( Imagelist );

	// Get Data From XML File
	ReadXMLFileData();
	TCHAR szTabName[XMLDATA_LEN]={0};

	// Get Servicename 
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 gXMLFileData.strServiceName.c_str(),  // address of string to map
				 strlen(gXMLFileData.strServiceName.c_str()),      // number of bytes in string
				 szTabName,       // address of wide-character buffer
				 XMLDATA_LEN);             // size of buffer);

#else
	sprintf(szTabName,"%s",gXMLFileData.strServiceName.c_str());
#endif
	m_TabCtrl.AddSNMPVarWinTab(szTabName);

	// Get Grid name
	int iCount = m_GribTabDataArray.GetSize();
	BOOL bShowAboveCtrl = FALSE;
	for ( int i =0; i < iCount; i ++ )
	{
		memset(szTabName,0,sizeof(szTabName));
	#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
					 CP_ACP,         // code page
					 0,              // character-type options
					 m_GribTabDataArray[i].c_str(),  // address of string to map
					 strlen(m_GribTabDataArray[i].c_str()),      // number of bytes in string
					 szTabName,       // address of wide-character buffer
					 XMLDATA_LEN);             // size of buffer);

	#else
		sprintf(szTabName,"%s",m_GribTabDataArray[i].c_str());
	#endif

		if (  (_tcsicmp(szTabName,_T("Content")) == 0 )  ) // ContentStore
		{
			bShowAboveCtrl = TRUE;
		}
		else
		{
			bShowAboveCtrl = FALSE;
		}
		if (  ( _tcsicmp(szTabName,_T("Streamer")) == 0 ) && ( _stricmp(gXMLFileData.strServiceName.c_str(),"Weiwoo") == 0 ) )
		{
			_tcscat(szTabName,_T("1"));
			m_TabCtrl.AddServiceData(szTabName);
		}
		else
		{
			m_TabCtrl.AddServiceData(szTabName,bShowAboveCtrl);
		}
				
//		m_TabCtrl.AddServiceData1(szTabName);
	}
	m_TabCtrl.AddEventListViewTab(_T("Events"));
}

void CZQInfoClientCtrl::ReadXMLFileData()
{
	ZQ::common::MutexGuard gd(m_Mutex);
	try
	{
		ZQ::common::XMLPrefDoc* pXMLDoc;
		ZQ::common::ComInitializer* pcomInit;
			
		pcomInit = new ZQ::common::ComInitializer();
		pXMLDoc = new ZQ::common::XMLPrefDoc(*pcomInit);
		char strFile[MAX_PATH] ={0};
		TCHAR theFileName[ MAX_PATH ] ={0};
		GetModuleFileName(NULL,theFileName,sizeof(theFileName)/sizeof(theFileName[0]));
//		::GetCurrentDirectory(sizeof(theFileName),theFileName);
		
		int iSzLen=_tcsclen(theFileName);
		if (  iSzLen > 0 )
		{
				int iTemp=iSzLen-1;
				while (theFileName[iTemp]!='\\' && iTemp >=0 )
					iTemp--;
				iTemp--;//skip '\'
				while (theFileName[iTemp]!='\\' && iTemp >=0 )
					iTemp--;
				if(iTemp>0)
				{
					theFileName[iTemp]='\0';
				}
				_tcsncat(theFileName,_T("\\etc"),_tcsclen(_T("\\etc")));
		}
		_tcscat(theFileName,_T("\\InfoClientConfig.xml"));
		
	#if defined _UNICODE || defined UNICODE
		WideCharToMultiByte(CP_ACP,NULL,theFileName,-1,strFile,sizeof(strFile),NULL,NULL);
	#else
		sprintf(strFile,"%s",theFileName);
	#endif
		if ( pXMLDoc )
		{
			bool bRes;
			bRes = pXMLDoc->open(strFile);
			if ( !bRes )
			{
				return ;
			}
		}
	
		char szNodeName[XMLNAME_LEN]={0};
		char szNodeValue[XMLDATA_LEN]={0};
		string strTabName;
		
		ZQ::common::IPreference* rootIpref = pXMLDoc->root();
		ZQ::common::IPreference* itemIpref = NULL;
		ZQ::common::IPreference* itemIpref1 = NULL;
		
		
		itemIpref = rootIpref->firstChild(); 
		while(itemIpref != NULL)
		{
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			// Service name
			if ( _stricmp(szNodeName,SERVICENAME) == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(VARNAME,szNodeValue);
				gXMLFileData.strServiceName = szNodeValue;

				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(SERVICEOID,szNodeValue);
				gXMLFileData.strServiceOID = szNodeValue;
			}
			// Config SnmpIp
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,CONFIG) == 0 )
			{
				// SNMPIP Value
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(SNMPIP,szNodeValue);
				gXMLFileData.strSnmpIp = szNodeValue;
			}

			// Grid TabName
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,GRIDNAME) == 0 )
			{
				itemIpref1 = itemIpref->firstChild();
				while(itemIpref1 != NULL)
				{
					memset(szNodeName,0,sizeof(szNodeName));
					itemIpref1->name(szNodeName);
					if ( _stricmp(szNodeName,TABNAME) == 0 )
					{
						memset(szNodeValue,0,sizeof(szNodeValue));
						itemIpref1->get(VARNAME,szNodeValue);
						strTabName = szNodeValue;
						m_GribTabDataArray.Add(strTabName);
					}
					
					/*
					memset(szNodeName,0,sizeof(szNodeName));
					itemIpref1->name(szNodeName);
					if ( _stricmp(szNodeName,DATASOURCE) == 0 )
					{
						memset(szNodeValue,0,sizeof(szNodeValue));
						itemIpref1->get(FEEDER,szNodeValue);
						gXMLFileData.strGridFunc = szNodeValue;

						memset(szNodeValue,0,sizeof(szNodeValue));
						itemIpref1->get(DSODATA,szNodeValue);
						gXMLFileData.strFuncDso = szNodeValue;
					}
					*/
					itemIpref1 = itemIpref->nextChild();
				}
			}

			// Function Feeder
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,DATASOURCE) == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(FEEDER,szNodeValue);
				gXMLFileData.strGridFunc = szNodeValue;
			}


			//Events Feeder
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,EVENTSOURCE) == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(FEEDER,szNodeValue);
				gXMLFileData.strEventsFeeder = szNodeValue;

//				memset(szNodeValue,0,sizeof(szNodeValue));
//				itemIpref->get(DSODATA,szNodeValue);
//				gXMLFileData.strEventsDso = szNodeValue;
			}

			// DSO value
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,DSODATA) == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(VARNAME,szNodeValue);
				gXMLFileData.strFuncDso = szNodeValue;
				gXMLFileData.strEventsDso = szNodeValue;
			}



			// IceStorm EndPoint
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,ICESTORM) == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(ENDPOINT,szNodeValue);
				gXMLFileData.strIceStormEndPoint = szNodeValue;
			}

			// Service EndPoint
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,SERVICE) == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(ENDPOINT,szNodeValue);
				gXMLFileData.strServcieEndPoint = szNodeValue;
				
			}

			// Event log file
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,EVENTLOG) == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get(FILENAME,szNodeValue);
				gXMLFileData.strEventLogName= szNodeValue;
			}
			itemIpref = rootIpref->nextChild();
		}
		if ( itemIpref )
		{
			itemIpref->free();
		}
		rootIpref->free();
		
		if(pcomInit != NULL)
			delete pcomInit;
		pcomInit = NULL;
		pXMLDoc = NULL;
	}
	catch(...)
	{
		return ;
	}
}
