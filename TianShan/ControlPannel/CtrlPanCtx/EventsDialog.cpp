// EventsDialog.cpp: implementation of the CEventsDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EventsDialog.h"
#include "resource.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//CListViewCtrl  CEventsDialog::m_ListCtrl;
CSortListViewCtrl * CEventsDialog::m_pListCtrl = NULL;
int            CEventsDialog::m_iCateGoryCount=0;
int            CEventsDialog::m_iIndex = 0;
int			   CEventsDialog::m_iErrors= 0;
int            CEventsDialog::m_iWarnings=1;
int			   CEventsDialog::m_iInformation=2;
int            CEventsDialog::m_iShowLines=1000;
int *          CEventsDialog::m_pCateCheck=NULL;
ITEMDATA *     CEventsDialog::m_pCategorys=NULL;
bool           CEventsDialog::m_bEventStop=false;

SIMPLEARRAY    CEventsDialog::m_DataArray;			///< An array of views for the tab
int CALLBACK   CEventsDialog::OnStreamEvent(string & strCategory, int &iLevel, string & strCurTime, string & strMessage);
int            CEventsDialog::AddListCtrlData(const string & strCategory, const int &iLevel, const string & strCurTime,const  string & strMessage);
ZQ::common::Mutex		CEventsDialog::m_Mutex;
CEventFileLog * CEventsDialog::m_pEventLogFile = NULL;

#ifdef THREADMODE
	//CEventAddDataThread * CEventsDialog::m_pAddDataThread = NULL;
#endif

CEventsDialog::CEventsDialog()
{
	m_iNumColumns  =  4;
//	m_pCategorys = NULL;
//	m_pCateCheck = NULL;
	memset(&m_iColumnData,0,sizeof(m_iColumnData));
#ifdef THREADMODE
	m_pAddDataThread = NULL;
#endif
}

CEventsDialog::~CEventsDialog()
{
	// free the columns's memory
	if ( m_pCategorys)
	{
		for ( int  i = 0; i < m_iCateGoryCount; i ++)
		{
			free(m_pCategorys[i].szItemData);
		}
		delete  m_pCategorys;
	}
	if (m_pCateCheck)
	{
		delete [] m_pCateCheck;
	}
	SaveColumnLenData();
	if (m_DataArray.GetSize() > 0 )
	{
		m_DataArray.RemoveAll();
	}
#ifdef THREADMODE 
	if ( m_pAddDataThread)
	{
		delete m_pAddDataThread;
	}
	m_pAddDataThread = NULL;
#endif
	if ( m_pEventLogFile)
	{
		delete m_pEventLogFile;
	}
	m_pEventLogFile = NULL;
	if ( m_pListCtrl)
	{
		m_pListCtrl->DestroyWindow();
	}
	m_pListCtrl = NULL;
	if ( m_bkbmp.m_hBitmap)
	{
		m_bkbmp.DeleteObject();
	}
}

void CEventsDialog::RefreshListCtrl()
{
	if ( m_pAddDataThread )
	{
//		m_pAddDataThread->suspend();
	}
	//注：如果Event消息(如StreamSmith的OnProgress非常频繁，调用gd(m_Mutex)会出现死锁的现象，但如果不加锁，又会出现数据不同步的问题。2007-03-27 add by dony 
	ZQ::common::MutexGuard gd(m_Mutex);
//	m_ListCtrl.DeleteAllItems();
	m_pListCtrl->DeleteAllItems();
	int iCount = m_DataArray.GetSize();
	ALLITEMSDATA pTempData;
	
	string strCateGroy;
	LV_ITEM lvi;
	memset(&lvi,0,sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.pszText = _T("");
	lvi.iSubItem = 0;
	int iIndex = 0;
	m_iIndex = 0;
	
	for ( int i =0; i < iCount; i ++ )
	{
		pTempData = m_DataArray[i];

		if ( m_iCateGoryCount >0 )
		{
			for (int j = 0; j < m_iCateGoryCount; j ++)
			{
				if ( m_pCateCheck[j] == 1 )
				{
					strCateGroy = m_pCategorys[j].szItemData;
				}
				else
				{
					strCateGroy ="";
				}
				
				if ( pTempData.strCateGory == strCateGroy)
				{
					if  ( ( pTempData.iIndex  == m_iErrors ) || (pTempData.iIndex == m_iWarnings )
						 || ( pTempData.iIndex == m_iInformation ) )
					{
						lvi.iItem  = iIndex;
						lvi.iImage = pTempData.iIndex;

						/*
						m_ListCtrl.InsertItem(&lvi);
						m_ListCtrl.SetItemText(iIndex,1,(LPCTSTR)pTempData.strDataTime.c_str());
						m_ListCtrl.SetItemText(iIndex,2,(LPCTSTR)pTempData.strCateGory.c_str());
						m_ListCtrl.SetItemText(iIndex,3,(LPCTSTR)pTempData.strEventData.c_str());
						// 注：不作定位当前行的操作，因为Event太快，刷屏操作会占大量的CPU资源,2007-03-27 add by dony 
				//		m_ListCtrl.SelectItem(iIndex);
				*/
						m_pListCtrl->InsertItem(&lvi);
						m_pListCtrl->SetItemText(iIndex,1,(LPCTSTR)pTempData.strDataTime.c_str());
						m_pListCtrl->SetItemText(iIndex,2,(LPCTSTR)pTempData.strCateGory.c_str());
						m_pListCtrl->SetItemText(iIndex,3,(LPCTSTR)pTempData.strEventData.c_str());
						m_pListCtrl->SelectItem(iIndex);
						iIndex ++;
						m_iIndex ++;
					}
				}
			}
		}
	}
	if ( m_pAddDataThread )
	{
//		m_pAddDataThread->resume();
	}
}

LRESULT CEventsDialog::OnStopEvent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_bEventStop = !m_bEventStop;
	if ( m_bEventStop)
	{
		m_bkbmp.Detach();
		m_bkbmp.LoadBitmap(IDB_PLAYBMP);
	}
	else
	{
		m_bkbmp.Detach();
		m_bkbmp.LoadBitmap(IDB_STOPBITMAP);
	}
	m_playBtn.SetBitmap(m_bkbmp.m_hBitmap);
	m_playBtn.UpdateWindow();
	bHandled = FALSE;
	return S_OK;
}

LRESULT CEventsDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OPTIONDATA pTempData;
	memset((void*)&pTempData,0,sizeof(pTempData));
	pTempData.iErrors = m_iErrors;
	pTempData.iWarnings = m_iWarnings;
	pTempData.iInformation = m_iInformation;
	pTempData.iShowLines = m_iShowLines;

//	COptionsDialog dlg(pTempData);
	COptionsDialog dlg(pTempData,&m_pCategorys,&m_iCateGoryCount,m_pCateCheck);
	int iMode = dlg.DoModal();

	OPTIONDATA *pTempValue = NULL;
	pTempValue = new OPTIONDATA;
	dlg.GetOptionData(pTempValue,m_pCateCheck);
	m_iErrors = pTempValue->iErrors;
	m_iWarnings = pTempValue->iWarnings;
	m_iInformation = pTempValue->iInformation;
	m_iShowLines = pTempValue->iShowLines;
	
	RefreshListCtrl();
	if ( pTempValue)
	{
		delete pTempValue;
	}
	pTempValue = NULL;

	bHandled = FALSE;
	return S_OK;
}

int  CEventsDialog::AddListCtrlData(const string & strCategory, const int &iLevel, const string & strCurTime, const string & strMessage)
{
	if ( m_bEventStop)
	{
		return -1;
	}
	
	ZQ::common::MutexGuard gd(m_Mutex);

	ALLITEMSDATA pTempData;
	unsigned long lData ;
	lData = (unsigned long)iLevel + 1;
	
	
	string strNewCateGroy;
	for (int j = 0; j < m_iCateGoryCount; j ++)
	{
		if ( m_pCateCheck[j] == 1 )
		{
			strNewCateGroy = m_pCategorys[j].szItemData;
		}
		else
		{
			strNewCateGroy ="";
		}
		
		if ( strCategory == strNewCateGroy)
		{
   
			if  ( ( iLevel  == m_iErrors ) || ( iLevel == m_iWarnings )
					 || ( iLevel == m_iInformation ) )
			{
				LV_ITEM lvi;
				memset(&lvi,0,sizeof(lvi));
				lvi.pszText = _T("");
				lvi.iSubItem = 0;
				lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
				lvi.iItem = m_iIndex;
				lvi.lParam = (LPARAM)lData;
				lvi.iImage = iLevel;
				
				/*
				m_ListCtrl.InsertItem(&lvi);
				m_ListCtrl.SetItemText(m_iIndex,1,(LPCTSTR)strCurTime.c_str());
				m_ListCtrl.SetItemText(m_iIndex,2,(LPCTSTR)strCategory.c_str());
				m_ListCtrl.SetItemText(m_iIndex,3,(LPCTSTR)strMessage.c_str());
				// 注：不作定位当前行的操作，因为Event太快，刷屏操作会占大量的CPU资源,2007-03-27 add by dony 
		//		m_ListCtrl.SelectItem(m_iIndex);
				*/

				m_pListCtrl->InsertItem(&lvi);
				m_pListCtrl->SetItemText(m_iIndex,1,(LPCTSTR)strCurTime.c_str());
				m_pListCtrl->SetItemText(m_iIndex,2,(LPCTSTR)strCategory.c_str());
				m_pListCtrl->SetItemText(m_iIndex,3,(LPCTSTR)strMessage.c_str());
				// 注：不作定位当前行的操作，因为Event太快，刷屏操作会占大量的CPU资源,2007-03-27 add by dony 
				m_pListCtrl->SelectItem(m_iIndex);
				m_iIndex ++;
			}
		}
	}
	pTempData.iIndex = lData -1 ;
	pTempData.strDataTime =strCurTime;
	pTempData.strCateGory =strCategory;
	pTempData.strEventData=strMessage;
	m_DataArray.Add(pTempData);

	string strTotalMessage;
	char cLevel[10] ={0};
	itoa(iLevel,cLevel,10);
	strTotalMessage = cLevel;
	strTotalMessage +="         ";
	strTotalMessage +=strCurTime;
	strTotalMessage +="      ";
	strTotalMessage +=strCategory;
	strTotalMessage +="        ";
	strTotalMessage +=strMessage;
	(*m_pEventLogFile)(ZQ::common::Log::L_INFO,strTotalMessage.c_str());
	
	// clear ui operator
	if ( m_iIndex > m_iShowLines)
	{
		m_iIndex = 0;
		m_DataArray.RemoveAll();
//		m_ListCtrl.DeleteAllItems();
		m_pListCtrl->DeleteAllItems();
	}
	return 0;
}

int CALLBACK  CEventsDialog::OnStreamEvent(string & strCategory, int &iLevel, string & strCurTime, string & strMessage)
{
	LV_ITEM lvi;
	memset(&lvi,0,sizeof(lvi));
	lvi.pszText = _T("");
	lvi.iSubItem = 0;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

	// for the test 
	ALLITEMSDATA pTempData;
	
	unsigned long lData ;
	lData = (unsigned long)iLevel + 1;
    lvi.iItem = m_iIndex;
    lvi.lParam = (LPARAM)lData;
    lvi.iImage = iLevel;

	/*
	m_ListCtrl.InsertItem(&lvi);
	m_ListCtrl.AddItem(m_iIndex,1,(LPCTSTR)strCurTime.c_str());
	m_ListCtrl.AddItem(m_iIndex,2,(LPCTSTR)strCategory.c_str());
	m_ListCtrl.AddItem(m_iIndex,3,(LPCTSTR)strMessage.c_str());
	*/
	m_pListCtrl->InsertItem(&lvi);
	m_pListCtrl->AddItem(m_iIndex,1,(LPCTSTR)strCurTime.c_str());
	m_pListCtrl->AddItem(m_iIndex,2,(LPCTSTR)strCategory.c_str());
	m_pListCtrl->AddItem(m_iIndex,3,(LPCTSTR)strMessage.c_str());
	pTempData.iIndex = lData -1 ;
	pTempData.strDataTime =strCurTime.c_str();
	pTempData.strCateGory =strCategory.c_str();
	pTempData.strEventData=strMessage.c_str();
	m_DataArray.Add(pTempData);
	m_iIndex ++;
	return 0;
}

/*
CListViewCtrl & CEventsDialog::getListViewCtrl()
{
//	return this->m_ListCtrl;
	return NULL;
}
*/

void CEventsDialog::SaveColumnLenData()
{
	
	DWORD dLen = sizeof(DWORD);
	LONG  status;
	HKEY hKey;
	DWORD           dwDisposition;
	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          _T("SOFTWARE\\InfoClient"),
                          0,
                          KEY_ALL_ACCESS,
                          &hKey);
	if ( status != ERROR_SUCCESS)
	{
		status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                _T("SOFTWARE\\InfoClient"),
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKey,
                                &dwDisposition);
	}

	int iColumnWidth;
	TCHAR szKey[XMLNAME_LEN]={0};

	for ( int i =0; i < m_iNumColumns-1; i ++ )
	{		
//		iColumnWidth = m_ListCtrl.GetColumnWidth(i);
		iColumnWidth = m_pListCtrl->GetColumnWidth(i);
		_stprintf(szKey,_T("%s%d"),_T("Column"),i);
		status = RegSetValueEx(hKey,
			                  szKey,
							  0,
							  REG_DWORD,
							  (BYTE*)&iColumnWidth,
							  dLen);
	}
	RegCloseKey(hKey);
}

void CEventsDialog::GetCateGoryFromDll()
{
	// Get CategorysDatas from dll,then add datas to UI Desktop
	TCHAR szTabName[XMLDATA_LEN]={0};
	
	// Get Event DSO Dll's name 
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 gXMLFileData.strEventsDso.c_str(),  // address of string to map
				 strlen(gXMLFileData.strEventsDso.c_str()),      // number of bytes in string
				 szTabName,       // address of wide-character buffer
				 XMLDATA_LEN);             // size of buffer);

#else
	sprintf(szTabName,"%s",gXMLFileData.strEventsDso.c_str());
#endif
	if  ( _tcsstr(szTabName,_T("%") )!= NULL )
	{
			TCHAR szTemp[XMLDATA_LEN]={0};
			ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
			memset(szTabName,0,sizeof(szTabName));
			_stprintf(szTabName,_T("%s"),szTemp);
	}
	
	typedef int (CALLBACK *pGetCateGorys)(ITEMDATA **pCategorys,int *iCount);
	pGetCateGorys hGetCateGorys = NULL;
	// modify hLib to global var
//	HINSTANCE hLib;
//	hLib = LoadLibrary(szTabName);
//	if ( hLib)
	if ( !m_hLib)
	{
		m_hLib = LoadLibrary(szTabName);
	}
	if ( m_hLib)
	{
	//	hGetCateGorys = (pGetCateGorys)GetProcAddress(hLib,"GetCateGoryDatas");
		hGetCateGorys = (pGetCateGorys)GetProcAddress(m_hLib,"GetCateGoryDatas");
		if ( hGetCateGorys)
		{
			int iRet = (*hGetCateGorys)(&m_pCategorys,&m_iCateGoryCount);
			if ( m_iCateGoryCount > 0 )
			{
				m_pCateCheck = new int [m_iCateGoryCount];
				for (int i =0; i < m_iCateGoryCount; i ++)
				{
					m_pCateCheck[i] = 1;
				}
			}
		}
	//	FreeLibrary(hLib); // modify hLib to global var
	}
}

void CEventsDialog::InitData()
{
#ifdef THREADMODE
	m_pAddDataThread = new CEventAddDataThread(this);
	m_pAddDataThread->start();
#else
	TCHAR szTabName[XMLDATA_LEN]={0};
	TCHAR szDllName[XMLDATA_LEN]={0};
	
	// Get Event DSO Dll's name 
	#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 gXMLFileData.strEventsDso.c_str(),  // address of string to map
				 strlen(gXMLFileData.strEventsDso.c_str()),      // number of bytes in string
				 szTabName,       // address of wide-character buffer
				 XMLDATA_LEN);             // size of buffer);

	#else
		sprintf(szTabName,"%s",gXMLFileData.strEventsDso.c_str());
	#endif
	if  ( _tcsstr(szTabName,_T("%") )!= NULL )
	{
			TCHAR szTemp[XMLDATA_LEN]={0};
			ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
			memset(szTabName,0,sizeof(szTabName));
			_stprintf(szTabName,_T("%s"),szTemp);
	}
	_stprintf(szDllName,_T("%s"),szTabName);

	
	// Get Event from dll,then add datas to UI Desktop
	typedef int (CALLBACK *GetEventProc)(const STRVECTOR & attribeData,OnEvent_Proc pFun);
	GetEventProc hEventProc = NULL;

	// modify hLib to global var
//	HINSTANCE hLib;
//	hLib = LoadLibrary(szDllName);
//	if ( hLib)
	if ( !m_hLib )
	{
		m_hLib = LoadLibrary(szDllName);
	}
	if ( m_hLib )
	{
	//	hEventProc = (GetEventProc)GetProcAddress(hLib,gXMLFileData.strGridFunc.c_str());
		hEventProc = (GetEventProc)GetProcAddress(m_hLib,gXMLFileData.strGridFunc.c_str());
		if ( hEventProc)
		{
			int iReturn;
			iReturn=(*hEventProc)(OnStreamEvent);
		}
	//	FreeLibrary(hLib); // modify hLib to global var
	}
#endif
}

LRESULT CEventsDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int iMode = 1;
	DWORD dwStyle =  LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
///	m_ListCtrl.Attach(GetDlgItem(IDC_EVENTLIST));
//	m_ListCtrl.SetExtendedListViewStyle(dwStyle);

	m_pListCtrl = new CSortListViewCtrl(iMode);
//	m_pListCtrl->Create(this);
	m_pListCtrl->Attach(GetDlgItem(IDC_EVENTLIST));
	m_pListCtrl->SetExtendedListViewStyle(dwStyle);

//	m_OKBtn.Attach(GetDlgItem(IDOK));
	m_OKBtn.SubclassWindow(GetDlgItem(IDOK));
//	m_playBtn.SubclassWindow(GetDlgItem(IDC_PLAYBUTTON));
	m_playBtn.Attach(GetDlgItem(IDC_PLAYBUTTON));
	int nMask;
	int nFmt ;
	int nSubItem = -1;
	
	m_iNumColumns = 4;
	CImageList     m_imagelist;
	m_imagelist.Create(16, 16, ILC_COLORDDB | ILC_MASK, 0, 1);

	CBitmap         bkbmp;
	m_bkbmp.LoadBitmap(IDB_STOPBITMAP);
	m_playBtn.SetBitmap(m_bkbmp.m_hBitmap);

//	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_ERRORBITMAP);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));
			
	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_WARNINGBITMAP);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));

	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_INFOBITMAP);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));
//	m_ListCtrl.SetImageList(m_imagelist.m_hImageList,LVSIL_SMALL   );
	m_pListCtrl->SetImageList(m_imagelist.m_hImageList,LVSIL_SMALL   );

	
	/*
	CImageList     imagelist;
	imagelist.Create(9, 9, ILC_COLORDDB | ILC_MASK, 0, 1);
	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_LEVEL);
	imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));


	
//	m_ListCtrl.AddColumn(_T(""), 0,nSubItem,nMask,nFmt);
	CHeaderCtrl  pctlHeader;
	pctlHeader = m_ListCtrl.GetHeader();
	pctlHeader.SetImageList(imagelist.m_hImageList);
	HD_ITEM Item ={0};	
	memset(&Item,0,sizeof(Item));
	
	Item.mask = HDI_IMAGE | HDI_TEXT;
	Item.fmt = HDF_IMAGE;
	Item.pszText=_T("");
	Item.iImage=0;
//	Item.cchTextMax =0;
//	pctlHeader.InsertItem(0,&Item);
//	pctlHeader.SetItem(0,&Item);
//	m_ListCtrl.GetHeader().InsertItem(0,&Item);
	m_ListCtrl.GetHeader().SetItem(0,&Item);
	
	nMask =HDI_BITMAP | HDI_FORMAT;
	nFmt = HDF_BITMAP;
	CHeaderCtrl  pctlHeader;
	pctlHeader = m_ListCtrl.GetHeader();

//	m_ListCtrl.AddColumn(_T(""), 0,nSubItem,nMask,nFmt);
	HD_ITEM Item;	
	Item.mask = HDI_FORMAT;
	m_ListCtrl.GetHeader().GetItem( 0, &Item );
	Item.mask = HDI_BITMAP | HDI_FORMAT;	
	Item.fmt |= HDF_BITMAP   ;
	Item.hbm = (HBITMAP)bkbmp.m_hBitmap;
	pctlHeader.InsertItem(0,&Item);
//	m_ListCtrl.GetHeader().SetItem(0, &Item );
	*/
	
	nMask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	nFmt = LVCFMT_CENTER | LVCFMT_BITMAP_ON_RIGHT;
//	m_ListCtrl.AddColumn(_T("Level"), 0,nSubItem,nMask,nFmt);
	m_pListCtrl->AddColumn(_T("Level"), 0,nSubItem,nMask,nFmt);
	
	nFmt = LVCFMT_LEFT;
	/*
	m_ListCtrl.AddColumn(_T("DataTime"),1,nSubItem,nMask,nFmt);
	m_ListCtrl.AddColumn(_T("Category"),2,nSubItem,nMask,nFmt);
	m_ListCtrl.AddColumn(_T("Message"), 3,nSubItem,nMask,nFmt);        
	*/
	m_pListCtrl->AddColumn(_T("DataTime"),1,nSubItem,nMask,nFmt);
	m_pListCtrl->AddColumn(_T("Event"),2,nSubItem,nMask,nFmt);
	m_pListCtrl->AddColumn(_T("Message"), 3,nSubItem,nMask,nFmt);        
	
	RECT rcClientRect;
	GetClientRect(&rcClientRect);
	
	RECT rc1,rc2,rc3;
	m_OKBtn.GetWindowRect(&rc2);
	rc3.bottom = rcClientRect.bottom -3;
	rc3.right = rcClientRect.right - 5;
	rc3.top = rc3.bottom - (rc2.bottom - rc2.top );
	rc3.left = rc3.right - ( rc2.right - rc2.left);
	m_OKBtn.MoveWindow(&rc3);

	rc1.left = rcClientRect.left;
	rc1.right = rcClientRect.right;
	rc1.top = rcClientRect.top;
	rc1.bottom = rc3.top - 2;
//	m_ListCtrl.MoveWindow(&rc1);

	m_pListCtrl->MoveWindow(&rc1);
	
	int icx1 = 50;
	int icx2 = 149;
	int icx3 = 118;
	int icx4;

	m_pListCtrl->SetColumnWidth(0,icx1);
	m_pListCtrl->SetColumnWidth(1,icx2);
	m_pListCtrl->SetColumnWidth(2,icx3);
	icx4 = ( rcClientRect.right - rcClientRect.left - icx1 - icx2 -icx3 );
	m_pListCtrl->SetColumnWidth(3,icx4);
	m_pListCtrl->SetFocus();
	
	GetCateGoryFromDll();
	InitData();

	// Init Event Log
	
	TCHAR szTabName[XMLDATA_LEN]={0};
	TCHAR szDllName[XMLDATA_LEN] ={0};
	
	// Get Grid Fun's name 
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
					 CP_ACP,         // code page
					 0,              // character-type options
					 gXMLFileData.strEventLogName.c_str(),  // address of string to map
					 strlen(gXMLFileData.strEventLogName.c_str()),      // number of bytes in string
					 szTabName,       // address of wide-character buffer
					 XMLDATA_LEN);             // size of buffer);

#else
	sprintf(szTabName,"%s",gXMLFileData.strEventLogName.c_str());
#endif
	if  ( _tcsstr(szTabName,_T("%") )!= NULL )
	{
			TCHAR szTemp[XMLDATA_LEN]={0};
			ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
			memset(szTabName,0,sizeof(szTabName));
			_stprintf(szTabName,_T("%s"),szTemp);
	}
	_stprintf(szDllName,_T("%s"),szTabName);
	TCHAR *pos = NULL;
	pos  = _tcsrchr(szDllName,_T('\\'));
	if ( pos )
	{
		TCHAR szDirectory[XMLDATA_LEN]={0};
		int iLen = _tcsclen(szDllName);
		iLen = iLen - _tcslen(pos);
		_tcsncpy(szDirectory,szDllName,iLen+1);
		CreateDirectory(szDirectory,NULL);
	}
	m_pEventLogFile = new CEventFileLog(szDllName,ZQ::common::Log::L_INFO);
	(*m_pEventLogFile).setVerbosity(ZQ::common::Log::L_INFO);
	string strTotalMessage;
	strTotalMessage ="Level     DateTime                 Event            Message";
	(*m_pEventLogFile)(ZQ::common::Log::L_INFO,strTotalMessage.c_str());
	bHandled = FALSE;
	return S_OK;
}

LRESULT CEventsDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	RECT rcClientRect;
	GetClientRect(&rcClientRect);
	
	RECT rc1,rc2,rc3,rc4;
	m_OKBtn.GetWindowRect(&rc2);
	rc3.bottom = rcClientRect.bottom -3;
	rc3.right = rcClientRect.right - 5;
	rc3.top = rc3.bottom - (rc2.bottom - rc2.top );
	rc3.left = rc3.right - ( rc2.right - rc2.left);
	m_OKBtn.MoveWindow(&rc3);

	m_playBtn.GetWindowRect(&rc4);
	int iWidth = rc4.right - rc4.left;

	rc4.top = rc3.top;
	rc4.bottom = rc3.bottom;
	rc4.right = rc3.left - 10;
	rc4.left = rc4.right - iWidth;
	m_playBtn.MoveWindow(&rc4);
	

	rc1.left = rcClientRect.left;
	rc1.right = rcClientRect.right;
	rc1.top = rcClientRect.top;
	rc1.bottom = rc3.top - 2;
//	m_ListCtrl.MoveWindow(&rc1);
	m_pListCtrl->MoveWindow(&rc1);

	int icx1 = 50;
	int icx2 = 149;
	int icx3 = 118;
	int icx4;

	m_pListCtrl->SetColumnWidth(0,icx1);
	m_pListCtrl->SetColumnWidth(1,icx2);
	m_pListCtrl->SetColumnWidth(2,icx3);
	icx4 = ( rcClientRect.right - rcClientRect.left - icx1 - icx2 -icx3 );
	m_pListCtrl->SetColumnWidth(3,icx4);
	m_pListCtrl->SetFocus();

	
	bHandled = FALSE;
	return S_OK;
}



//////////////////////////////////////////////////////////////////////
// COptionsDialog Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
COptionsDialog::COptionsDialog(OPTIONDATA &OptionData,ITEMDATA **pCategorys,int *iCount,int *pCateCheck)
{
	m_OptionData.iErrors = OptionData.iErrors;
	m_OptionData.iWarnings = OptionData.iWarnings;
	m_OptionData.iInformation = OptionData.iInformation;
	m_OptionData.iShowLines  = OptionData.iShowLines;
	m_pCategorys = pCategorys;
	m_iCateGoryCount = *iCount;
	m_pCheckButton = NULL;
	m_pCateCheck = pCateCheck;
}

COptionsDialog::~COptionsDialog()
{
	if ( m_pCheckButton)
	{
		for ( int i =0; i < m_iCateGoryCount; i ++ )
		{
			if ( m_pCheckButton[i].IsWindow())
			{
				m_pCheckButton[i].DestroyWindow();
			}
		}
		delete [] m_pCheckButton;
		m_pCheckButton = NULL;
	}
	if ( m_pCategorys)
	{
		m_pCategorys = NULL;
	}
	if ( m_pCateCheck)
	{
		m_pCateCheck = NULL;
	}
}

void COptionsDialog::GetOptionData(OPTIONDATA * pValue,int *pCateCheck)
{
	pValue->iErrors =m_OptionData.iErrors ;
	pValue->iWarnings =m_OptionData.iWarnings;
	pValue->iInformation =m_OptionData.iInformation ;
	pValue->iShowLines = m_OptionData.iShowLines;
	pCateCheck = m_pCateCheck;
}

LRESULT COptionsDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	
	if ( m_iCateGoryCount > 0 )
	{
		/* // for the dyanmic create checkbox to allocate the checkbox layout,buf it is failed.
		RECT *prc;
		RECT rc,rc1;
		HWND hWnd;
		hWnd = GetDlgItem(IDC_CATEGROUP);
		::GetWindowRect(hWnd,&rc);

		hWnd = GetDlgItem(IDC_WARNCHECK);
		::GetWindowRect(hWnd,&rc1);
	
			
		int iHight = rc1.bottom - rc1.top;
		int iBlunkHigh = ( rc.bottom - rc.top - 20 - iHight*m_iCateGoryCount) / ( m_iCateGoryCount -1 );
		int iWidth = ( rc.right - rc.left - 10);
				
		for ( int i = 0; i < m_iCateGoryCount; i ++ )
		{
			prc[i].left  = rc.left   + 4;;
			prc[i].right = rc.right - 10;
			if ( i == 0)
			{
				prc[i].top = rc.top + 5;
			}
			else
			{
				prc[i].top  = prc[i-1].bottom + 16; 
			}
			prc[i].bottom = prc[i].top + iHight;
		//	ScreenToClient(&prc[i]);
				
			m_pCheckButton[i].MoveWindow(&prc[i]);
			m_pCheckButton[i].ShowWindow(SW_SHOWNORMAL);
		}
		if ( prc)
		{
			delete [] prc;
		}
		prc = NULL;
		*/
		
	}
	bHandled = FALSE;
	return S_OK;
}

LRESULT COptionsDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	m_ErrorBtn.Attach(GetDlgItem(IDC_ERRORCHECK));
	m_WarningBtn.Attach(GetDlgItem(IDC_WARNCHECK));
	m_InforamtionBtn.Attach(GetDlgItem(IDC_INFOCHECK));
	m_OKBtn.SubclassWindow(GetDlgItem(IDOK));
//	m_LineEdit.Attach(GetDlgItem(IDC_LINEEDIT));
	m_LineEdit.SubclassWindow(GetDlgItem(IDC_LINEEDIT));
	TCHAR s1[20]={0};
	_itot(m_OptionData.iShowLines,s1,10);
	m_LineEdit.SetWindowText(s1);
	if ( m_OptionData.iErrors == 0)
	{
		m_ErrorBtn.SetCheck(1);
	}
	else
	{
		m_ErrorBtn.SetCheck(0);
	}
	if ( m_OptionData.iWarnings == 1)
	{
		m_WarningBtn.SetCheck(1);
	}
	else
	{
		m_WarningBtn.SetCheck(0);
	}
	if ( m_OptionData.iInformation == 2)
	{
		m_InforamtionBtn.SetCheck(1);
	}
	else
	{
		m_InforamtionBtn.SetCheck(0);
	}

	UINT nCheckBox ;
	for ( int i =0; i < 8; i ++ )
	{
		nCheckBox = IDC_CHECKBOX + i+1;
		::ShowWindow(GetDlgItem(nCheckBox), SW_HIDE);
	}

	// Create the Button 
	if ( m_iCateGoryCount > 0 )
	{
		m_pCheckButton = new CButton[m_iCateGoryCount];
		
		
		/* // for the dynamic create checkbox and layout the checkbox,but it is failed,the reason is not to be finded now add by dony 
		DWORD dwStyle;
		dwStyle = WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON |  WS_TABSTOP | BS_AUTOCHECKBOX | BS_NOTIFY  ;
		RECT *prc;
		prc = new RECT[m_iCateGoryCount];

		RECT rc,rc1;
		HWND hWnd;
		hWnd = GetDlgItem(IDC_CATEGROUP);
		::GetWindowRect(hWnd,&rc);

		hWnd = GetDlgItem(IDC_WARNCHECK);
		::GetWindowRect(hWnd,&rc1);
		
		int iHight = rc1.bottom - rc1.top;
		int iBlunkHigh = ( rc.bottom - rc.top - 20 - iHight*m_iCateGoryCount) / ( m_iCateGoryCount -1 );
		int iWidth = ( rc.right - rc.left - 10);
		*/

		string strText;
		for ( int i = 0; i < m_iCateGoryCount; i ++ )
		{
			if ( m_iCateGoryCount == 2)
			{
				nCheckBox = IDC_CHECKBOX + i*(m_iCateGoryCount*2)+1;
				::ShowWindow(GetDlgItem(nCheckBox), SW_SHOW);
			}
			else if ( m_iCateGoryCount == 3)
			{
				nCheckBox = IDC_CHECKBOX + i*(m_iCateGoryCount)+1;
			}
			else
			{
				nCheckBox = IDC_CHECKBOX + i+1;
				::ShowWindow(GetDlgItem(nCheckBox), SW_SHOW);
			}
			
			/*
			prc[i].left  = rc.left   + 4;;
			prc[i].right = rc.right - 10;
			if ( i == 0)
			{
				prc[i].top = rc.top + 5;
			}
			else
			{
				prc[i].top  = prc[i-1].bottom + 16; 
			}
			prc[i].bottom = prc[i].top + iHight;
		//	ScreenToClient(&prc[i]);
			_U_MENUorID menu(nCheckBox);
		//  m_pCheckButton[i].Create(*this,&prc[i],_T(""),dwStyle,NULL,menu);
		    m_pCheckButton[i].MoveWindow(&prc[i]);
          */
			strText = (*m_pCategorys)[i].szItemData;
			m_pCheckButton[i].Attach(GetDlgItem(nCheckBox));
			m_pCheckButton[i].SetWindowText(strText.c_str());
			m_pCheckButton[i].SetParent(*this);
			m_pCheckButton[i].ShowWindow(SW_SHOWNORMAL);

			if (m_pCateCheck[i] == 1)
			{
				m_pCheckButton[i].SetCheck(1);
			}
			else
			{
				m_pCheckButton[i].SetCheck(0);
			}
		}
		/*
		if ( prc)
		{
			delete [] prc;
		}
		prc = NULL;
		*/
	}
	bHandled = FALSE;
	
	return S_OK;
}

LRESULT COptionsDialog::OnErrorClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	/*
	int iCheckValue;
	iCheckValue = m_ErrorBtn.GetCheck();
	
	if ( iCheckValue == 1)
	{
		m_ErrorBtn.SetCheck(0);
		m_OptionData.iErrors = -1;
	}
	else
	{
		m_ErrorBtn.SetCheck(0);
		m_OptionData.iErrors = 0;
	}
	*/
	bHandled = FALSE;
	return S_OK;
}

LRESULT COptionsDialog::OnWarnClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	/*
	int iCheckValue;
	iCheckValue = m_WarningBtn.GetCheck();
	if ( iCheckValue == 1)
	{
		m_WarningBtn.SetCheck(0);
		m_OptionData.iWarnings = -1;
	}
	else
	{
		m_WarningBtn.SetCheck(1);
		m_OptionData.iWarnings = 1;
	}
	*/
	bHandled = FALSE;
	return S_OK;
}

LRESULT COptionsDialog::OnInfoClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	/*
	int iCheckValue;
	iCheckValue = m_InforamtionBtn.GetCheck();
	if ( iCheckValue == 1)
	{
		m_InforamtionBtn.SetCheck(0);
		m_OptionData.iInformation = -1;
	}
	else
	{
		m_InforamtionBtn.SetCheck(1);
		m_OptionData.iInformation = 2;
	}
	*/
	bHandled = FALSE;
	return S_OK;
}

LRESULT COptionsDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int iCheckValue;
	iCheckValue = m_ErrorBtn.GetCheck();
	if ( iCheckValue == 1)
	{
		m_OptionData.iErrors =0;
	}
	else
	{
		m_OptionData.iErrors = -1;
	}

	iCheckValue = m_WarningBtn.GetCheck();
	if ( iCheckValue == 1)
	{
		m_OptionData.iWarnings = 1;
	}
	else
	{
		m_OptionData.iWarnings = -1;
	}

	iCheckValue = m_InforamtionBtn.GetCheck();
	if ( iCheckValue == 1)
	{
		m_OptionData.iInformation = 2;
	}
	else
	{
		m_OptionData.iInformation = -1;
	}
	TCHAR szLine[20]={0};
	::GetWindowText(m_LineEdit.m_hWnd,szLine,20);
	m_OptionData.iShowLines = _ttoi(szLine);

	if (m_iCateGoryCount >0)
	{
		for ( int i = 0; i < m_iCateGoryCount; i ++ )
		{
			m_pCateCheck[i]= m_pCheckButton[i].GetCheck();
		}
	}
	EndDialog(0);
	bHandled = FALSE;
	return S_OK;
}

