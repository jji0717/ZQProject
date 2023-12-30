// AdminCtrl.cpp : Implementation of CAdminCtrl

#include "stdafx.h"
#include "AdminControl.h"
#include "AdminCtrl.h"

#define  IMGPADDING  6
#define  TEXTPADDING 8
#define  IMAGESIZE   10
/////////////////////////////////////////////////////////////////////////////
// CAdminCtrl
CAdminCtrl::CAdminCtrl()
{
	m_bAutoSize = FALSE;
	m_bWindowOnly = TRUE;
}

CAdminCtrl::~CAdminCtrl()
{
	if ( m_Communicator)
	{
		m_Communicator->destroy();
		m_Communicator = NULL;
	}
}

BOOL CAdminCtrl::GetWeiwooEndPoint(char lpszEndPoint[MAX_PATH+1], char lpszPathEndPoint[MAX_PATH+1])
{
	LONG lRet;
    HKEY hKey;
    char szKey[MAX_PATH+1] ={0};
	char szProductName[TEXT_LEN] ={0};
	char szValue[] = "SiteEndpoint";
	char szValue1[] ="PathEndPoint";
    
	DWORD dwType;
    DWORD dwSize = MAX_PATH;
	GetProductName("Weiwoo",szProductName);

	sprintf(szKey,"SOFTWARE\\ZQ Interactive\\%s\\CurrentVersion\\Services\\WEIWOO",szProductName);
	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);
	if ( lRet != ERROR_SUCCESS)
	{
		sprintf(szKey,"SOFTWARE\\ZQ\\%s\\CurrentVersion\\Services\\WEIWOO",szProductName);
		lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);
	}
    if (ERROR_SUCCESS == lRet)
    {
        dwSize = MAX_PATH*(sizeof(char)); 
		lRet = RegQueryValueEx(hKey, szValue, NULL, &dwType, (LPBYTE)lpszEndPoint, &dwSize);

		dwSize = MAX_PATH*(sizeof(char)); 
		lRet = RegQueryValueEx(hKey, szValue1, NULL, &dwType, (LPBYTE)lpszPathEndPoint, &dwSize);

        RegCloseKey(hKey);

        if (ERROR_SUCCESS == lRet && REG_EXPAND_SZ == dwType)
        {
            char szEndPoint[MAX_PATH + 1];
            ExpandEnvironmentStrings(lpszEndPoint, szEndPoint, MAX_PATH);
			strncpy(lpszEndPoint, szEndPoint, MAX_PATH+1);

			memset(szEndPoint,0,sizeof(szEndPoint));
			ExpandEnvironmentStrings(lpszPathEndPoint, szEndPoint, MAX_PATH);
			strncpy(lpszPathEndPoint, szEndPoint, MAX_PATH+1);
        }
    }
    if (ERROR_SUCCESS != lRet)
    {
         strcpy(lpszEndPoint,"tcp -h 192.168.80.49 -p 10003");
		 strcpy(lpszPathEndPoint,"tcp -h 192.168.80.49 -p 10001");
    }
	return TRUE;
}

BOOL CAdminCtrl::GetProductName(const char* lpszServiceName, char lpszProductName[MAX_PATH+1])
{
    LONG lRet;
    HKEY hKey;
    char szKey[MAX_PATH+1] = "SYSTEM\\CurrentControlSet\\Services\\";
    char szValue[] = "ProductName";
    DWORD dwType;
    DWORD dwSize = MAX_PATH;

	strcat(szKey, lpszServiceName);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == lRet)
    {
        dwSize = MAX_PATH*(sizeof(char)); 
		lRet = RegQueryValueEx(hKey, szValue, NULL, &dwType, (LPBYTE)lpszProductName, &dwSize);
        RegCloseKey(hKey);

        if (ERROR_SUCCESS == lRet && REG_EXPAND_SZ == dwType)
        {
            char szProductName[MAX_PATH + 1];
            ExpandEnvironmentStrings(lpszProductName, szProductName, MAX_PATH);
			strncpy(lpszProductName, szProductName, MAX_PATH+1);
        }
    }

    if (ERROR_SUCCESS != lRet)
    {
        /////////////
        strcpy(lpszProductName,"ITV");
    }
	return TRUE;
}

HRESULT CAdminCtrl::OnDraw(ATL_DRAWINFO& di)
{
	return S_OK;
}

HRESULT CAdminCtrl::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CRect rcClient,rc1;
	GetClientRect(&rcClient);
	rc1.left = rcClient.left + rcClient.Width() / 4 ;	rc1.top = rcClient.top;	
	rc1.bottom = rcClient.bottom;	rc1.right = rcClient.right;

	m_VerSplit.Create(m_hWnd,rcClient,_T(""), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN );
	m_VerSplit.m_cxyMin = SPLIT_MINWIDTH;
	m_VerSplit.m_cxySplitBar = SPLIT_BARWIDTH;
	           
	m_VerSplit.SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	m_VerSplit.SetSplitterPos(rcClient.Width()/ 4 );
	m_VerSplit.SetActivePane(SPLIT_PANE_LEFT);
		
	DWORD dwStyle = WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS| TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT |TVS_SHOWSELALWAYS;
    m_hTreeCtrl.Create(m_VerSplit,rcDefault,NULL,dwStyle,WS_EX_CLIENTEDGE);
	m_VerSplit.SetSplitterPane(SPLIT_PANE_LEFT, m_hTreeCtrl);
	m_HorSplit.Create(m_VerSplit.m_hWnd,rc1,_T(""), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN );
	
	m_VerSplit.SetSplitterPane( SPLIT_PANE_RIGHT,m_HorSplit);

	m_imagelist.Create(16, 16, ILC_COLORDDB | ILC_MASK, 0, 1);

	
	CBitmap         bkbmp;
	bkbmp.LoadBitmap(IDB_STORAGE);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));
			
	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_STREAMER);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));


	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_SERVGROUP);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));

	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_STORAGELINK);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));


	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_STREAMERLINK);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));

	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_SITE);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));

	bkbmp.Detach();
	bkbmp.LoadBitmap(IDB_APP);
	m_imagelist.Add(bkbmp.m_hBitmap,RGB(255, 0, 255));
	

			
	m_hTreeCtrl.SetImageList(m_imagelist.m_hImageList,TVSIL_NORMAL);

	
	dwStyle =  LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
	m_hListCtrl.Create( m_HorSplit, rcDefault, NULL,
                              WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|LVS_REPORT|WS_HSCROLL|
                              LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE );
	
    m_hListCtrlLinks.Create( m_HorSplit, rcDefault, NULL,
                               WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|LVS_REPORT|WS_HSCROLL|
                               LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE );

	m_hListCtrl.SetExtendedListViewStyle(dwStyle);
    m_hListCtrlLinks.SetExtendedListViewStyle(dwStyle);

	m_HorSplit.SetSplitterPane(SPLIT_PANE_TOP, m_hListCtrl);
	m_HorSplit.SetSplitterPane(SPLIT_PANE_BOTTOM,m_hListCtrlLinks);
	m_HorSplit.SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);

	m_HorSplit.SetActivePane(SPLIT_PANE_TOP);

		
	m_HorSplit.m_cxyMin = SPLIT_MINWIDTH;
	m_HorSplit.m_cxySplitBar = SPLIT_BARWIDTH;
	m_HorSplit.SetSplitterPos(rcClient.Height()/2);

	std::string strName ="Storage";
	InsertListCtrlColumn(strName);
	InsertListCtrlLinkColumn(strName);
	int  i = 0;
	TCHAR szMsg[TEXT_LEN];
	try
	{
		if ( m_Communicator  == NULL )
		{
			m_Communicator = Ice::initialize(i, NULL);
			char szEndPoint[MAX_PATH+1]={0};
			char szEndPoint1[MAX_PATH+1]={0};
			GetWeiwooEndPoint(szEndPoint,szEndPoint1);
			std::string strEndPoint;
			std::string strPathEndPoint;
			
			strEndPoint = szEndPoint;
			strPathEndPoint=szEndPoint1;
			ConnectServe(strEndPoint,szEndPoint1); // for the test
		}
	}
	catch(const Ice::Exception& ex)
	{
		_stprintf(szMsg,_T("%s%s"),_T("Initialize Ice runtime met exception with error:"), 
						               ex.ice_name().c_str());
        ATLTRACE(szMsg);
		MessageBox(szMsg);
		return S_FALSE;
	}
	catch(...)
	{
		_stprintf(szMsg,_T("%s"),_T("Initialize Ice runtime met unknow exception"));
        ATLTRACE(szMsg);
		MessageBox(szMsg);
		return S_FALSE;
	}
	return S_OK;
}

LRESULT CAdminCtrl::OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	CRect rcClient,rc1;
	GetClientRect(&rcClient);
	rc1.left = rcClient.left + rcClient.Width() / 4;
	rc1.top = rcClient.top;	
	rc1.bottom = rcClient.bottom;	
	rc1.right = rcClient.right;
	m_VerSplit.MoveWindow(rcClient);
	m_HorSplit.MoveWindow(rc1);

	m_VerSplit.m_cxyMin = SPLIT_MINWIDTH;
	m_VerSplit.m_cxySplitBar = SPLIT_BARWIDTH;

	m_HorSplit.m_cxyMin = SPLIT_MINWIDTH;
	m_HorSplit.m_cxySplitBar = SPLIT_BARWIDTH;

	m_VerSplit.SetSplitterPos(rcClient.Width()/ 4);
	m_HorSplit.SetSplitterPos(rcClient.Height()/2);

	int i = 0;
	int icx = ( 3* rcClient.Width() )/ 4;
	icx = icx / m_iListColNum;

//	::LockWindowUpdate(m_hListCtrl);
	for ( i =0; i < m_iListColNum; i ++ )
	{
		m_hListCtrl.SetColumnWidth(i,icx);
	}
//	::LockWindowUpdate(NULL);

	
	icx = ( 3* rcClient.Width() )/ 4 ;
	icx = icx / m_iListLinkNum;
//	::LockWindowUpdate(m_hListCtrlLinks);
	for ( i =0; i <m_iListLinkNum; i ++ )
	{
		m_hListCtrlLinks.SetColumnWidth(i,icx);
	}
//	::LockWindowUpdate(NULL);
	return S_OK;
}

BOOL CAdminCtrl::DeleteTreeItem(HTREEITEM hitem)
{
//	::LockWindowUpdate( m_hTreeCtrl );
    HTREEITEM hdel = m_hTreeCtrl.GetChildItem(hitem);
	HTREEITEM htemp;
	while(hdel)
	{   
		htemp = hdel;
		hdel = m_hTreeCtrl.GetNextItem(htemp,TVGN_NEXT);
		m_hTreeCtrl.DeleteItem(htemp);
	}
//	::LockWindowUpdate( NULL);
	return TRUE;
}

LRESULT CAdminCtrl::OnSelchangedTree(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMLISTVIEW lpnmtv  = (LPNMLISTVIEW)pnmh;
	if( lpnmtv->hdr.code	 == TVN_SELCHANGED &&
		lpnmtv->hdr.hwndFrom == m_hTreeCtrl.m_hWnd )
    {
		std::string strName;
        // TODO : Add Code for control notification handler.
		HTREEITEM hSelect =NULL, hParent = NULL, hRoot = NULL; 
		hSelect = m_hTreeCtrl.GetSelectedItem();
		if(hSelect)
		{
			hParent = m_hTreeCtrl.GetNextItem(hSelect, TVGN_PARENT);
		}
		if(hParent)
		{
			hRoot = m_hTreeCtrl.GetNextItem(hParent, TVGN_PARENT);
		}
		
		// The 1st layer
		if(hSelect == m_hStreamerRoot && hParent == NULL)
		{
			DeleteTreeItem(m_hStreamerRoot);
		
			strName ="Other";
			m_FirstLayerType = 0;
			DeleteListCtrlLinkColumn();
			InsertListCtrlLinkColumn(strName);

			ListStreamerId(TRUE); // // the first layer for Streamer item
			return S_OK;
		}
		else 
		{
			if(hSelect == m_hStorageRoot)				
			{
				DeleteTreeItem(m_hStorageRoot);
				m_FirstLayerType = 0;
				strName ="Other";
				DeleteListCtrlLinkColumn();
				InsertListCtrlLinkColumn(strName);
				ListStorageId(TRUE); // the first layer for Storage item
				return S_OK;
			}
			else
			{
				if(hSelect == m_hSGRoot )			
				{
					DeleteTreeItem(m_hSGRoot);
					strName ="Other";
					m_FirstLayerType = 0;
					DeleteListCtrlLinkColumn();
					InsertListCtrlLinkColumn(strName);
					ListSGId(TRUE); // the first layer for Service item
					return S_OK;
				}
				if ( hSelect == m_hSiteRoot)
				{
					DeleteTreeItem(m_hSiteRoot);
					strName ="Site";
					m_FirstLayerType = 1;
					DeleteListCtrlLinkColumn();
					strName ="App";
					InsertListCtrlLinkColumn(strName);
					ListSite(TRUE);// // the first layer for Site item
					return S_OK;
				}
				if ( hSelect == m_hAppRoot)
				{
					DeleteTreeItem(m_hAppRoot);
					m_FirstLayerType = 0;
					strName ="App";
					DeleteListCtrlLinkColumn();
					InsertListCtrlLinkColumn(strName);
					ListApp(TRUE);  // the first layer for App item
					return S_OK;
				}
			}
		}
		if(hParent == m_hStreamerRoot )
		{
			DisplayStreamerInfo(); // the sencod layer for Stream's child item
			return S_OK;
		}
		else 
		{
			if(hParent == m_hSGRoot)
			{
				DisplaySGInfo();  // the sencod layer for Service's child item
				return S_OK;
			}
			else
			{
				if(hParent == m_hStorageRoot)
				{
					DisplayStorageInfo(); // the sencod layer for Storage's child item
					return S_OK;
				}
				if(hParent == m_hSiteRoot )
				{
					DisplaySiteInfo(); // the sencod layer for Stream's child item
					return S_OK;
				}
				if ( hParent == m_hAppRoot)
				{
					DisplayAppInfo();  // the sencod layer for App's child item
					return S_OK;
				}
				if(hRoot == m_hStreamerRoot )
				{
					DisplayStreamLinks(); // the third layer for Stream Links  item
					return S_OK;
				}
				else 
				{
					if(hRoot == m_hSGRoot)
					{
						DisplayStreamLinks(); // the third layer for Stream Links  item
						return S_OK;
					}
					else
					{
						if(hRoot == m_hStorageRoot)
						{
							DisplayStorageLinks(); // the third layer for Storage  Links  item
							return S_OK;
						}
					}
				}
			}
		}
	}
	return S_OK;
}

HRESULT CAdminCtrl::OnMeasureItem(UINT , WPARAM wParam, LPARAM lParam,
                                       BOOL& bHandled)
{
    LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT)lParam;
    if( wParam == 0 && lpMeasureItem->CtlType == ODT_MENU )
    {
        int nStrID     = (int)lpMeasureItem->itemData;

        CClientDC dc(*this);
		CString str;
		if ( m_bMountApp )
		{
			str ="UMountApplication";
		}
		else
		{
			str = m_strMenuString[0];
		}
        CSize   size;

        dc.GetTextExtent( str, str.GetLength(), &size );

        lpMeasureItem->itemHeight = size.cy + 2;
        lpMeasureItem->itemWidth  = size.cx + 20;

        if( nStrID == -1 )
		{
            lpMeasureItem->itemHeight /= 2 ;
		}
    }
    return 0L;
}

LRESULT CAdminCtrl::OnDrawItem(UINT , WPARAM wParam, LPARAM lParam,
                                    BOOL& bHandled)
{

    LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
    if( wParam == 0 && lpDrawItemStruct->CtlType == ODT_MENU )
    {
        int nStrID     = (int)lpDrawItemStruct->itemData;
        CDCHandle dc   = lpDrawItemStruct->hDC;
        const RECT& rcItem = lpDrawItemStruct->rcItem;
        LPCRECT pRect  = &rcItem;
        BOOL bDisabled = lpDrawItemStruct->itemState & ODS_GRAYED;
        BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
        BOOL bChecked  = lpDrawItemStruct->itemState & ODS_CHECKED;
        COLORREF crBackImg = CLR_NONE;
        CDCHandle* pDC = &dc; 
        
        if ( bSelected && !bDisabled )
        {
            COLORREF crHighLight = ::GetSysColor (COLOR_HIGHLIGHT);
            CPenDC pen (*pDC, crHighLight);
            CBrushDC brush (*pDC, crBackImg = bDisabled ?
                HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +73, 0) :
                HLS_TRANSFORM (crHighLight, +70, -57));
            
            pDC->Rectangle (pRect);
        }
        else
        {
            CRect rc (pRect);
            
            rc.right = IMAGESIZE + IMGPADDING;
            pDC->FillSolidRect (rc, crBackImg = 
                HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +20, 0));
            rc.left = rc.right;
            rc.right = pRect->right;
            pDC->FillSolidRect (rc, 
                HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +75, 0));
        }
        if( nStrID == -1)
        {
            CPenDC pen (*pDC, HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), -18, 0));

            pDC->MoveTo (pRect->left+IMAGESIZE+IMGPADDING+TEXTPADDING,  (pRect->top+pRect->bottom)/2);
            pDC->LineTo (pRect->right-1, (pRect->top+pRect->bottom)/2);
        }
        else
        {
            CRect rc (pRect);
            CString sCaption = m_strMenuString[nStrID];
            int nTab = sCaption.Find (_T('\t'));
            
            if ( nTab >= 0 )
            {
                sCaption = sCaption.Left (nTab);
            }
            pDC->SetTextColor (bDisabled ? 
                HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), -18, 0) : 
                GetSysColor (COLOR_MENUTEXT));
            pDC->SetBkMode (TRANSPARENT);
            
            CBoldDC bold (*pDC, (lpDrawItemStruct->itemState & ODS_DEFAULT) != 0);
            
            rc.left = IMAGESIZE + IMGPADDING+TEXTPADDING;
            pDC->DrawText (sCaption, sCaption.GetLength(), rc, 
                 DT_SINGLELINE|DT_VCENTER|DT_LEFT);
            
            if ( bChecked  )
            {
                COLORREF crHighLight = ::GetSysColor (COLOR_HIGHLIGHT);
                CPenDC pen (*pDC, crHighLight);
                CBrushDC brush (*pDC, crBackImg = bDisabled ? 
                    HLS_TRANSFORM (::GetSysColor (COLOR_3DFACE), +73, 0) :
                (bSelected ? HLS_TRANSFORM (crHighLight, +50, -50) : 
                    HLS_TRANSFORM (crHighLight, +70, -57)));
                
                pDC->Rectangle (CRect (pRect->left+1, pRect->top+1, 
                    pRect->left+IMAGESIZE + 5-2, pRect->bottom-1));
            }
        }
    }
    return 0L;
}

LRESULT CAdminCtrl::OnVerb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SData sdata;
	sdata.iMaxSession=0;
	sdata.lMaxBW=0;
	sdata.flag = FALSE;
	BOOL bOpenDialog = TRUE;
	std::string strName;
	int nflag = -1;

	std::string strtemp;
	char str[TEXT_LEN];
	int nRow;
	LinkInfo linkinfo;
	TCHAR szMsg[TEXT_LEN];


	TianShanIce::Transport::StorageLinkPrx storagelinkPrx;
	TianShanIce::Transport::StreamLinkPrx  streamlinkprx;

	switch( wID )
	{
		case ID_ADDSTORAGE:
			nflag = 0;
			break;
		case ID_ADDSTREAMER:
			nflag = 1;
			break;
		case ID_ADDSG:
			nflag = 2;
			break;
		case ID_LINKSTORAGE:
			nflag = 3;
			break;
		case ID_LINKSTREAMER:
			nflag = 4;
			break;
		case ID_ADDSITE:
			nflag = 5;
			sdata.id = 0;
			break;
		case ID_ADDAPP:
			nflag = 6;
			break;
		case ID_UPSTORAGE:
			DisplayStorageInfo(&sdata);
			nflag = 0;
			break;
		case ID_UPSTREAMER:
			DisplayStreamerInfo(&sdata);
			nflag = 1;
			break;
		case ID_UPSG:
			DisplaySGInfo(&sdata);
			nflag = 2;
			break;
		case ID_UPDATESITE:
			nflag = 5;
			sdata.id = 0;
			DisplaySiteInfo(&sdata);
			break;
		case ID_SETSITEPROPERTIES:
			nflag = 5;
			sdata.netId = m_CurSelText;
			sdata.id = 1;
			break;
		case ID_UPDATEAPP:
			nflag = 6;
			DisplayAppInfo(&sdata);
			break;
		case ID_REMOVESTORAGE:
			bOpenDialog = FALSE;
			strName ="Storage";
			RemoveOper(strName,m_CurSelText);
			break;
		case ID_REMOVESERVICEGROUP:
			bOpenDialog = FALSE;
			strName ="ServiceGroup";
			RemoveOper(strName,m_CurSelText);
			break;
		case ID_REMOVESTREAMER:
			bOpenDialog = FALSE;
			strName ="Streamer";
			RemoveOper(strName,m_CurSelText);
			break;
		case ID_REMOVESITE:
			bOpenDialog = FALSE;
			strName ="Site";
			RemoveOper(strName,m_CurSelText);
			break;
		case ID_REMOVEAPP:
			bOpenDialog = FALSE;
			strName ="App";
			RemoveOper(strName,m_CurSelText);
			break;
		case ID_UMOUNTAPP:
			sdata.id  = 0;
			nflag = 7;
			break;
		case ID_MOUNTAPP:
			sdata.id = 1;
			nflag = 7;
			break;
		default:
			bOpenDialog = FALSE;
			break;
		}		
	
		if ( bOpenDialog)
		{
			CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx, nflag, sdata);
			dlg.DoModal();
		}
		switch(wID)
		{
		case ID_ADDSTORAGE:
			DeleteTreeItem(m_hStorageRoot);
			m_hListCtrl.DeleteAllItems();
			ListStorageId(FALSE); // the first layer for Storage item
			break;
		case ID_ADDSG:
			DeleteTreeItem(m_hSGRoot);
			m_hListCtrl.DeleteAllItems();
			ListSGId(FALSE);
			break;
		case ID_ADDSTREAMER:
			DeleteTreeItem(m_hStreamerRoot);
			m_hListCtrl.DeleteAllItems();
			ListStreamerId(FALSE);
			break;
		case ID_ADDSITE:
			DeleteTreeItem(m_hSiteRoot);
			m_hListCtrl.DeleteAllItems();
			ListSite(FALSE);
			break;
		case ID_ADDAPP:
			DeleteTreeItem(m_hAppRoot);
			m_hListCtrl.DeleteAllItems();
			ListApp(FALSE);
			break;
		case ID_REMOVESERVICEGROUP:
			DeleteTreeItem(m_hSGRoot);
			break;
		case ID_REMOVESTORAGE:
			DeleteTreeItem(m_hStorageRoot);
			break;
		case ID_REMOVESTREAMER:
			DeleteTreeItem(m_hStreamerRoot);
			break;
		case ID_REMOVESITE:
			DeleteTreeItem(m_hSiteRoot);
			break;
		case ID_REMOVEAPP:
			DeleteTreeItem(m_hAppRoot);
			break;
		case ID_UPDATELINK:
			if(m_blistLink)
			{
			   nRow = m_hListCtrlLinks.GetSelectionMark();
			   if(m_bStorageLink)
			   {
				   linkinfo.bStorage = 1;
				 
				   m_hListCtrlLinks.GetItemText(nRow,1,str,TEXT_LEN);
				   linkinfo.linktype = str;

				   m_hListCtrlLinks.GetItemText(nRow,2,str,TEXT_LEN);
				   linkinfo.storageNetId = str;

				   m_hListCtrlLinks.GetItemText(nRow,3,str,TEXT_LEN);
				   linkinfo.streamerNetId = str;

				   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
				   try
				   {
					   linkinfo.storagelinkPrx  = TianShanIce::Transport::StorageLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
				   }
				   catch( const ::Ice::Exception &ex)
				   {

					    _stprintf(szMsg,_T("%s%s"),_T("StorageLinkPrx::checkedCast  Ice exception with error:"), 
						               ex.ice_name().c_str());
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						linkinfo.storagelinkPrx = NULL;
				   }
				   catch(...)
				   {
					   _stprintf(szMsg,_T("%s"),_T("StorageLinkPrx::checkedCast  System exception with error:")); 
						               
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						linkinfo.storagelinkPrx = NULL;
				   }
				   CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx,3,linkinfo);
				   dlg.DoModal();
			   }
			   else
			   {
				   linkinfo.bStorage = 0;
				   
				   m_hListCtrlLinks.GetItemText(nRow,1,str,TEXT_LEN);
				   linkinfo.linktype = str;

				   m_hListCtrlLinks.GetItemText(nRow,2,str,TEXT_LEN);
				   linkinfo.streamerNetId = str;

				   m_hListCtrlLinks.GetItemText(nRow,3,str,TEXT_LEN);
				   linkinfo.SGId = atoi(str);

				   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
				   try
				   {
					   linkinfo.streamlinkprx = TianShanIce::Transport::StreamLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
				   }
				   catch( const ::Ice::Exception &ex)
				   {
					    _stprintf(szMsg,_T("%s%s"),_T("StreamLinkPrx::checkedCast  Ice exception with error:"), 
						               ex.ice_name().c_str());
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						linkinfo.streamlinkprx = NULL;
				   }
				   catch(...)
				   {
					   _stprintf(szMsg,_T("%s"),_T("StreamLinkPrx::checkedCast  System exception with error:")); 
						               
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						linkinfo.streamlinkprx = NULL;
				   }
				   
				   CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx,4,linkinfo);
				   dlg.DoModal();
			   }
			}
			break;
		case ID_DELLINK:
			if(m_blistLink)
			{
			   nRow = m_hListCtrlLinks.GetSelectionMark();
			   if(m_bStorageLink)
			   {
				   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
				   try
				   {
					   storagelinkPrx = TianShanIce::Transport::StorageLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
					   storagelinkPrx->destroy();
				   }
				   catch( const ::Ice::Exception &ex)
				   {
					    _stprintf(szMsg,_T("%s%s"),_T("StorageLinkPrx::checkedCast  Ice exception with error:"), 
						               ex.ice_name().c_str());
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						return S_FALSE;
				   }
				   catch(...)
				   {
					   _stprintf(szMsg,_T("%s"),_T("StorageLinkPrx::checkedCast  System exception with error:")); 
						               
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						return S_FALSE;
				   }
			   }
			   else
			   {
				   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
				   try
				   {
					   streamlinkprx = TianShanIce::Transport::StreamLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
					   streamlinkprx->destroy();
				   }
				   catch( const ::Ice::Exception &ex)
				   {
					    _stprintf(szMsg,_T("%s%s"),_T("StreamLinkPrx::checkedCast  Ice exception with error:"), 
						               ex.ice_name().c_str());
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						return S_FALSE;
				   }
				   catch(...)
				   {
					   _stprintf(szMsg,_T("%s"),_T("StreamLinkPrx::checkedCast  System exception with error:")); 
						               
						ATLTRACE(szMsg);
						MessageBox(szMsg);
						return S_FALSE;
				   }
			   }
			   m_hListCtrlLinks.DeleteItem(nRow);
			   nRow = m_hListCtrlLinks.GetItemCount();
			   if ( nRow >= 0 )
			   {
				   if ( nRow == 0 )
				   {
					   m_hListCtrl.DeleteAllItems();
				   }
				   else
				   {
					  m_hListCtrl.SetItemText(0,1,(LPSTR)itoa(nRow,str,10));
				   }
			   }
			}
			break;
		default:
			break;
	}
	return S_OK;
}

LRESULT CAdminCtrl::OnLclkList(int idCtrl, LPNMHDR pnmh, BOOL & bHandled)
{
	LPNMLISTVIEW lpnmtv  = (LPNMLISTVIEW)pnmh;
	if( lpnmtv->hdr.code	 == NM_CLICK &&
		lpnmtv->hdr.hwndFrom == m_hListCtrl.m_hWnd )
	{
		if ( m_FirstLayerType == 1)
		{
			std::string strtemp;
			char str[TEXT_LEN];
			strtemp ="Site";
			DeleteListCtrlLinkColumn();
			InsertListCtrlLinkColumn(strtemp);

			int nRow = m_hListCtrl.GetSelectionMark();
			m_hListCtrl.GetItemText(nRow,0,str,TEXT_LEN);
			strtemp = str;
			ShowSite(strtemp);
		}
	}
	return S_OK;
}

LRESULT CAdminCtrl::OnDblclkList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMLISTVIEW lpnmtv  = (LPNMLISTVIEW)pnmh;
	if( lpnmtv->hdr.code	 == NM_DBLCLK &&
		lpnmtv->hdr.hwndFrom == m_hListCtrlLinks.m_hWnd )
	{

		std::string strtemp;
		char str[TEXT_LEN];
		LinkInfo linkinfo;
		TCHAR szMsg[TEXT_LEN];
		    
		if(m_blistLink)
		{
		   int nRow = m_hListCtrlLinks.GetSelectionMark();
		   if(m_bStorageLink)
		   {
			   linkinfo.bStorage = 1;
			 
			   m_hListCtrlLinks.GetItemText(nRow,1,str,TEXT_LEN);
			   linkinfo.linktype = str;

			   m_hListCtrlLinks.GetItemText(nRow,2,str,TEXT_LEN);
			   linkinfo.storageNetId = str;

			   m_hListCtrlLinks.GetItemText(nRow,3,str,TEXT_LEN);
			   linkinfo.streamerNetId = str;

			   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
			   try
			   {
				   linkinfo.storagelinkPrx  = TianShanIce::Transport::StorageLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
			   }
			   catch( const ::Ice::Exception &ex)
			   {
					_stprintf(szMsg,_T("%s%s"),_T("StorageLinkPrx::checkedCast  Ice exception with error:"), 
						           ex.ice_name().c_str());
					ATLTRACE(szMsg);
					MessageBox(szMsg);
					linkinfo.storagelinkPrx = NULL;
			   }
			   catch(...)
			   {
				   _stprintf(szMsg,_T("%s"),_T("StorageLinkPrx::checkedCast  System exception with error:")); 
						           
					ATLTRACE(szMsg);
					MessageBox(szMsg);
					linkinfo.storagelinkPrx = NULL;
			   }
			   
			   CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx,3,linkinfo);
			   dlg.DoModal();
		   }
		   else
		   {
			   linkinfo.bStorage = 0;
			   
			   m_hListCtrlLinks.GetItemText(nRow,1,str,TEXT_LEN);
			   linkinfo.linktype = str;

			   m_hListCtrlLinks.GetItemText(nRow,2,str,TEXT_LEN);
			   linkinfo.streamerNetId = str;

			   m_hListCtrlLinks.GetItemText(nRow,3,str,TEXT_LEN);
			   linkinfo.SGId = atoi(str);

			   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
			   
			   try
			   {
				   linkinfo.streamlinkprx = TianShanIce::Transport::StreamLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
			   }
			   catch( const ::Ice::Exception &ex)
			   {
					_stprintf(szMsg,_T("%s%s"),_T("StreamLinkPrx::checkedCast  Ice exception with error:"), 
						           ex.ice_name().c_str());
					ATLTRACE(szMsg);
					MessageBox(szMsg);
					linkinfo.streamlinkprx = NULL;
			   }
			   catch(...)
			   {
				   _stprintf(szMsg,_T("%s"),_T("StreamLinkPrx::checkedCast  System exception with error:")); 
						           
					ATLTRACE(szMsg);
					MessageBox(szMsg);
					linkinfo.streamlinkprx = NULL;
			   }

			   CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx,4,linkinfo);
			   dlg.DoModal();
		   }
		}
	}
	return S_OK;
}

LRESULT CAdminCtrl::OnRclickTree(int idCtrl,LPNMHDR pnmh, BOOL & bHandled)
{
	CMenu popMenu,SubMenu,MenuTrackPopup;
	int nItemCount,i;

	LPNMLISTVIEW lpnmtv  = (LPNMLISTVIEW)pnmh;
	if( lpnmtv->hdr.code	 == NM_RCLICK )
	{
		if (lpnmtv->hdr.hwndFrom == m_hTreeCtrl.m_hWnd )
		{

			m_strMenuString.RemoveAll();
			   					
			POINT pos;
			::GetCursorPos(&pos);
			// Retreive the coordinates of the treecrtl viwndow
			RECT cRect ;
			::GetWindowRect(m_hTreeCtrl.m_hWnd, &cRect);
			
			pos.x += -cRect.left;
			pos.y += -cRect.top;
			
			std::string strName,strTextName;

			TVHITTESTINFO ht;
			ht.pt = pos;
			HTREEITEM hSelect, hParent;
			hSelect =m_hTreeCtrl.HitTest(&ht);
			if ( hSelect == NULL)
			{
				hSelect = m_hTreeCtrl.GetSelectedItem(); // add by dony 
			}
			m_bMountApp = FALSE;
			if ( hSelect ) 
			{
				char strText[TEXT_LEN];
				m_hTreeCtrl.SelectItem(hSelect);
				if(hSelect)
				{
					hParent = m_hTreeCtrl.GetNextItem(hSelect,TVGN_PARENT);
				}
				
				m_hTreeCtrl.GetItemText(hSelect,strText,TEXT_LEN);
				strTextName = strText;
				m_CurSelText = strTextName;

				if(!strcmp(strText, "Storage Links"))
				{ 
#ifdef DRAWMENU
					popMenu.LoadMenu(IDR_STORAGELINK);
#else
					m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_STORAGELINK));
#endif

				}
				else if(!strcmp(strText, "Streamer Links"))
				{
#ifdef DRAWMENU
					popMenu.LoadMenu(IDR_STREAMERLINK);
#else
					m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_STREAMERLINK));		
#endif
				}
				if(hParent == NULL)
				{
					if(hSelect == m_hStreamerRoot )
					{
#ifdef DRAWMENU
						popMenu.LoadMenu(IDR_ADDSTREAMER);
#else
						m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDSTREAMER));
#endif
					}
					else 
					{
						if(hSelect == m_hSGRoot)
						{
#ifdef DRAWMENU
							popMenu.LoadMenu(IDR_ADDSG);
#else
							m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDSG));
#endif
						}
						else
						{
							if(hSelect == m_hStorageRoot)
							{
#ifdef DRAWMENU
								popMenu.LoadMenu(IDR_ADDSTORAGE);
#else
								m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDSTORAGE));
#endif
							}
							if ( hSelect == m_hSiteRoot)
							{
#ifdef DRAWMENU
								popMenu.LoadMenu(IDR_ADDSITE);
#else
								m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDSITE));
#endif
							}
							if ( hSelect == m_hAppRoot)
							{
#ifdef DRAWMENU
								popMenu.LoadMenu(IDR_ADDAPP);
#else
								m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_ADDAPP));
#endif
	
							}
						}
					}
				}
				else
				{
					if(hParent == m_hStreamerRoot )
					{
#ifdef DRAWMENU
						popMenu.LoadMenu(IDR_UPSTREAMER);
#else
						m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPSTREAMER));
#endif
					}
					else 
					{
						if(hParent == m_hSGRoot)
						{
#ifdef DRAWMENU
							popMenu.LoadMenu(IDR_UPSG);
#else
							m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPSG));
#endif
						}
						else
						{
							if(hParent == m_hStorageRoot)
							{
#ifdef DRAWMENU
								popMenu.LoadMenu(IDR_UPSTORAGE);
#else
								m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPSTORAGE));
#endif
							}
							if ( hParent == m_hSiteRoot)
							{
#ifdef DRAWMENU
								popMenu.LoadMenu(IDR_UPDATESITE);
#else
								m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPDATESITE));
#endif
								m_bMountApp = TRUE;
							}
							if ( hParent == m_hAppRoot)
							{
#ifdef DRAWMENU
								popMenu.LoadMenu(IDR_UPDATEAPP);
#else
								m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_UPDATEAPP));
#endif
								m_bMountApp = TRUE;
							}
						}
					}
				}
#ifdef DRAWMENU				
				MenuTrackPopup = CreatePopupMenu();
				SubMenu = popMenu.GetSubMenu(0);
				nItemCount = SubMenu.GetMenuItemCount();
				for( i = 0; i < nItemCount; i++ )
				{
					
					MENUITEMINFO mii = {0};
					TCHAR  buf[512] = {0};
					mii.cbSize = sizeof(MENUITEMINFO);
					mii.fMask  = MIIM_ID|MIIM_STRING;
					mii.cch    = 512;
					mii.dwTypeData = buf;
					BOOL k = SubMenu.GetMenuItemInfo( i, TRUE, &mii );
        
					if( mii.fType == MFT_STRING ) 
					{
						CString szTitle = buf;
						
						m_strMenuString.Add( szTitle );
						MenuTrackPopup.InsertMenu(MenuTrackPopup.GetMenuItemCount(), MF_BYCOMMAND|MF_OWNERDRAW,
										  mii.wID,(LPCTSTR)(m_strMenuString.GetSize() - 1) );
					}
					else
					{
						MenuTrackPopup.InsertMenu(MenuTrackPopup.GetMenuItemCount(), MF_BYPOSITION|MF_SEPARATOR|MF_OWNERDRAW,
										  NULL, (LPCTSTR)-1 );
					}
				}
				
				
				POINT pt; 
				GetCursorPos(&pt);	
				MenuTrackPopup.TrackPopupMenu( TPM_RIGHTALIGN | TPM_RIGHTBUTTON,
										   pt.x, pt.y,m_hWnd);
#else
				//  remark the old oper style
				m_hMenu = ::GetSubMenu(m_hMenu, 0) ;		
				POINT pt; 
				GetCursorPos(&pt);	

				BOOL btest = ::TrackPopupMenu(m_hMenu, TPM_RIGHTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0,m_hTreeCtrl.m_hWnd, NULL) ;
				SData sdata;
				sdata.iMaxSession=0;
				sdata.lMaxBW=0;
				sdata.flag = FALSE;
				BOOL bOpenDialog = TRUE;

				
				int nflag = -1;
				if(btest)
				{
					switch(btest)
					{
					case ID_ADDSTORAGE:
						nflag = 0;
						break;
					case ID_ADDSTREAMER:
						nflag = 1;
						break;
					case ID_ADDSG:
						nflag = 2;
						break;
					case ID_LINKSTORAGE:
						nflag = 3;
						break;
					case ID_LINKSTREAMER:
						nflag = 4;
						break;
					case ID_ADDSITE:
						nflag = 5;
						sdata.id = 0;
						break;
					case ID_ADDAPP:
						nflag = 6;
						break;
					case ID_UPSTORAGE:
						DisplayStorageInfo(&sdata);
						nflag = 0;
						break;
					case ID_UPSTREAMER:
						DisplayStreamerInfo(&sdata);
						nflag = 1;
						break;
					case ID_UPSG:
						DisplaySGInfo(&sdata);
						nflag = 2;
						break;
					case ID_UPDATESITE:
						nflag = 5;
						sdata.id = 0;
						DisplaySiteInfo(&sdata);
						break;
					case ID_SETSITEPROPERTIES:
						nflag = 5;
						sdata.netId = strTextName;
						sdata.id = 1;
						break;
					case ID_UPDATEAPP:
						nflag = 6;
						DisplayAppInfo(&sdata);
						break;
					case ID_REMOVESTORAGE:
						bOpenDialog = FALSE;
						strName ="Storage";
						RemoveOper(strName,strTextName);
						break;
					case ID_REMOVESERVICEGROUP:
						bOpenDialog = FALSE;
						strName ="ServiceGroup";
						RemoveOper(strName,strTextName);
						break;
					case ID_REMOVESTREAMER:
						bOpenDialog = FALSE;
						strName ="Streamer";
						RemoveOper(strName,strTextName);
						break;
					case ID_REMOVESITE:
						bOpenDialog = FALSE;
						strName ="Site";
						RemoveOper(strName,strTextName);
						break;
					case ID_REMOVEAPP:
						bOpenDialog = FALSE;
						strName ="App";
						RemoveOper(strName,strTextName);
						break;
					case ID_UMOUNTAPP:
						sdata.id  = 0;
						nflag = 7;
						break;
					case ID_MOUNTAPP:
						sdata.id = 1;
						nflag = 7;
						break;
					default:
						break;
					}			
				if ( bOpenDialog)
				{
					CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx, nflag, sdata);
					dlg.DoModal();
				}
				switch(btest)
				{
					case ID_ADDSTORAGE:
						DeleteTreeItem(m_hStorageRoot);
						m_hListCtrl.DeleteAllItems();
						ListStorageId(FALSE); // the first layer for Storage item
						break;
					case ID_ADDSG:
						DeleteTreeItem(m_hSGRoot);
						m_hListCtrl.DeleteAllItems();
						ListSGId(FALSE);
						break;
					case ID_ADDSTREAMER:
						DeleteTreeItem(m_hStreamerRoot);
						m_hListCtrl.DeleteAllItems();
						ListStreamerId(FALSE);
						break;
					case ID_ADDSITE:
						DeleteTreeItem(m_hSiteRoot);
						m_hListCtrl.DeleteAllItems();
						ListSite(FALSE);
						break;
					case ID_ADDAPP:
						DeleteTreeItem(m_hAppRoot);
						m_hListCtrl.DeleteAllItems();
						ListApp(FALSE);
						break;
					case ID_REMOVESERVICEGROUP:
						DeleteTreeItem(m_hSGRoot);
						break;
					case ID_REMOVESTORAGE:
						DeleteTreeItem(m_hStorageRoot);
						break;
					case ID_REMOVESTREAMER:
						DeleteTreeItem(m_hStreamerRoot);
						break;
					case ID_REMOVESITE:
						DeleteTreeItem(m_hSiteRoot);
						break;
					case ID_REMOVEAPP:
						DeleteTreeItem(m_hAppRoot);
						break;
					}
				}
#endif				
			}
		}
		else if (lpnmtv->hdr.hwndFrom == m_hListCtrlLinks.m_hWnd )
		{
#ifdef DRAWMENU										
			POINT pt; 
			GetCursorPos(&pt);	
			if ( m_hListCtrlLinks.GetItemCount() > 0 )
			{
				
				m_strMenuString.RemoveAll();
				popMenu.LoadMenu(IDR_LINKMENU);

				MenuTrackPopup = CreatePopupMenu();
				SubMenu = popMenu.GetSubMenu(0);

				nItemCount = SubMenu.GetMenuItemCount();
				for( i = 0; i < nItemCount; i++ )
				{
				
					MENUITEMINFO mii = {0};
					TCHAR  buf[512] = {0};
					mii.cbSize = sizeof(MENUITEMINFO);
					mii.fMask  =  MIIM_ID|MIIM_STRING|MIIM_FTYPE;
					mii.cch    = 512;
					mii.dwTypeData = buf;
					BOOL k = SubMenu.GetMenuItemInfo( i, TRUE, &mii );
    
					if( mii.fType == MFT_STRING ) 
					{
						CString szTitle = buf;
					
						m_strMenuString.Add( szTitle );
						MenuTrackPopup.InsertMenu(MenuTrackPopup.GetMenuItemCount(), MF_BYCOMMAND|MF_OWNERDRAW,
										  mii.wID, (LPCTSTR)(m_strMenuString.GetSize() - 1) );
									  
					}
					else
					{
						MenuTrackPopup.InsertMenu(MenuTrackPopup.GetMenuItemCount(), MF_BYPOSITION|MF_SEPARATOR|MF_OWNERDRAW,
									  NULL, (LPCTSTR)-1 );
					}
				}
				
				MenuTrackPopup.TrackPopupMenu( TPM_RIGHTALIGN | TPM_RIGHTBUTTON,
									   pt.x, pt.y,m_hWnd );
#else
				m_hMenu = ::LoadMenu(_Module.m_hInstResource,MAKEINTRESOURCE(IDR_LINKMENU));
				m_hMenu = ::GetSubMenu(m_hMenu, 0) ;		

				std::string strtemp;
				char str[TEXT_LEN];
				int nRow;
				LinkInfo linkinfo;
			
				TianShanIce::Transport::StorageLinkPrx storagelinkPrx;
				TianShanIce::Transport::StreamLinkPrx  streamlinkprx;

				
				
				BOOL btest = ::TrackPopupMenu(m_hMenu, TPM_RIGHTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0,m_hListCtrlLinks.m_hWnd, NULL) ;
				if(btest)
				{
					switch(btest)
					{
					case ID_UPDATELINK:
						if(m_blistLink)
						{
						   nRow = m_hListCtrlLinks.GetSelectionMark();
						   if(m_bStorageLink)
						   {
							   linkinfo.bStorage = 1;
							 
							   m_hListCtrlLinks.GetItemText(nRow,1,str,TEXT_LEN);
							   linkinfo.linktype = str;

							   m_hListCtrlLinks.GetItemText(nRow,2,str,TEXT_LEN);
							   linkinfo.storageNetId = str;

							   m_hListCtrlLinks.GetItemText(nRow,3,str,TEXT_LEN);
							   linkinfo.streamerNetId = str;

							   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
							   linkinfo.storagelinkPrx  = TianShanIce::Transport::StorageLinkPrx::checkedCast(m_Communicator->stringToProxy(str));

							   CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx,3,linkinfo);
							   dlg.DoModal();
						   }
						   else
						   {
							   linkinfo.bStorage = 0;
							   
							   m_hListCtrlLinks.GetItemText(nRow,1,str,TEXT_LEN);
							   linkinfo.linktype = str;

							   m_hListCtrlLinks.GetItemText(nRow,2,str,TEXT_LEN);
							   linkinfo.streamerNetId = str;

							   m_hListCtrlLinks.GetItemText(nRow,3,str,TEXT_LEN);
							   linkinfo.SGId = atoi(str);

							   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
							   linkinfo.streamlinkprx = TianShanIce::AccreditedPath::StreamLinkPrx::checkedCast(m_Communicator->stringToProxy(str));

							   CAddDataDlg dlg(m_Communicator,m_adminPrx,m_bizPrx,4,linkinfo);
							   dlg.DoModal();
						   }
						}
						break;
					case ID_DELLINK:
						if(m_blistLink)
						{
						   nRow = m_hListCtrlLinks.GetSelectionMark();
						   if(m_bStorageLink)
						   {
							   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
							   storagelinkPrx = TianShanIce::Transport::StorageLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
							   storagelinkPrx->destroy();
						   }
						   else
						   {
							   m_hListCtrlLinks.GetItemText(nRow,4,str,TEXT_LEN);
							   streamlinkprx = TianShanIce::Transport::StreamLinkPrx::checkedCast(m_Communicator->stringToProxy(str));
							   streamlinkprx->destroy();
							   
						   }
						   m_hListCtrlLinks.DeleteItem(nRow);
						   nRow = m_hListCtrlLinks.GetItemCount();
						   if ( nRow >= 0 )
						   {
							   if ( nRow == 0 )
							   {
								   m_hListCtrl.DeleteAllItems();
							   }
							   else
							   {
								  m_hListCtrl.SetItemText(0,1,(LPSTR)itoa(nRow,str,10));
							   }
						   }
						}
						break;
					}
				} 
#endif
			}
		}
	}
	return S_OK;
}


LRESULT CAdminCtrl::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	ResetCtrl();
	return S_OK;
}

BOOL  CAdminCtrl::ResetCtrl()
{
//	::LockWindowUpdate( m_hTreeCtrl );
	m_hTreeCtrl.DeleteAllItems();
//	::LockWindowUpdate( NULL );

//	::LockWindowUpdate(m_hListCtrl);
	m_hListCtrl.DeleteAllItems();
//	::LockWindowUpdate(NULL);

//	::LockWindowUpdate(m_hListCtrlLinks);
	m_hListCtrlLinks.DeleteAllItems();
//	::LockWindowUpdate(NULL);
	return TRUE;
}

BOOL CAdminCtrl::InitTreeCtrl()
{
//	::LockWindowUpdate( m_hTreeCtrl );
	TVITEM tvi = {0};
	TVINSERTSTRUCT tvs;
	tvs.hInsertAfter = TVI_LAST; 
	tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE ;
	
	tvi.iImage = 0;
	tvi.iSelectedImage = 0;
	tvi.pszText = "Storage";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hStorageRoot = m_hTreeCtrl.InsertItem(&tvs);
    ListStorageId(TRUE);

	tvi.iImage  = 1;
	tvi.iSelectedImage =1;
	tvi.pszText = "Streamer";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hStreamerRoot = m_hTreeCtrl.InsertItem(&tvs);	
	ListStreamerId(FALSE);
	
	tvi.iImage = 2;
	tvi.iSelectedImage = 2;
	tvi.pszText = "ServiceGroup";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hSGRoot = m_hTreeCtrl.InsertItem(&tvs);	
	ListSGId(FALSE);

	
	tvi.iImage = 5;
	tvi.iSelectedImage = 5;
	tvi.pszText = "Site";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hSiteRoot= m_hTreeCtrl.InsertItem(&tvs);
    ListSite(FALSE);

	tvi.iImage  = 6;
	tvi.iSelectedImage =6;
	tvi.pszText = "App";
	tvs.hParent = TVI_ROOT;
	tvs.item = tvi;	
	m_hAppRoot = m_hTreeCtrl.InsertItem(&tvs);	
	ListApp(FALSE);

//	::LockWindowUpdate( NULL );
	return TRUE;
}

BOOL  CAdminCtrl::RemoveOper(std::string strName,std::string id)
{
	BOOL bReturn;
	TCHAR szMsg[TEXT_LEN];
	try
	{
		if ( strName =="Storage")
		{
			bReturn =  m_adminPrx->removeStorage(id);
		}
		else if ( strName =="Streamer")
		{
			bReturn = m_adminPrx->removeStreamer(id);
		}
		else if ( strName =="Site")
		{
			bReturn = m_bizPrx->removeSite(id);
		}
		else if ( strName =="App")
		{
			bReturn = m_bizPrx->removeApplication(id);
		}
		else  
		{
			int idvalue = atoi(id.c_str());
			bReturn = m_adminPrx->removeServiceGroup(idvalue);
		}
	}
	catch( const ::Ice::Exception &ex)
	{
		_stprintf(szMsg,_T("%s%s"),_T("Remove operator Ice exception with error:"), 
					   ex.ice_name().c_str());
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}
    catch(...)
	{
	    _stprintf(szMsg,_T("%s"),_T("Remove operator  System exception with error:")); 
					   
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}
	return bReturn;
}

BOOL  CAdminCtrl::ListStorageId(BOOL bUpdate)
{
	 HTREEITEM hTemp;
     TVITEM tvi = {0};
     TVINSERTSTRUCT tvs;
     tvs.hInsertAfter = TVI_LAST; 
     tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE ;
	 TCHAR szMsg[TEXT_LEN];

	 LVITEM lvItem   = {0};
	 lvItem.mask     = LVIF_TEXT| LVIF_IMAGE;
	 lvItem.state = 0;

	 if ( bUpdate )
	 {
		std::string strName ="Storage";
		DeleteListCtrlColumn();
		InsertListCtrlColumn(strName);
	 }
	 m_blistLink = FALSE;
	 try
	 {
		TianShanIce::Transport::Storages StorageData = m_adminPrx->listStorages();
		int size = StorageData.size();
		
		for(int j = 0; j < size; j ++)
		{ 

		   tvi.iImage = 0;
		   tvi.iSelectedImage =0;
		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = (LPSTR)(StorageData[j].netId).c_str();
		   tvs.hParent = m_hStorageRoot;
		   tvs.item = tvi;	
		   hTemp=m_hTreeCtrl.InsertItem(&tvs);
		   
		   tvi.iImage = 3;
		   tvi.iSelectedImage =3;
		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText =_T("Storage Links");
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   m_hTreeCtrl.InsertItem(&tvs);
		   
		   if ( bUpdate )
		   {
			   lvItem.iItem = j;
		   
			   lvItem.iSubItem  = 0;
			   lvItem.pszText   =(LPSTR)(StorageData[j].netId).c_str();
	   		   m_hListCtrl.InsertItem(&lvItem);

			   lvItem.iSubItem  = 1;
			   lvItem.pszText   =(LPSTR)(StorageData[j].type).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);

			   lvItem.iSubItem  = 2;
			   lvItem.pszText   =(LPSTR)(StorageData[j].ifep).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);

			   lvItem.iSubItem  = 3;
			   lvItem.pszText   =(LPSTR)(StorageData[j].desc).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);
		   }
		}
		StorageData.clear();
		if ( bUpdate)
		{
			m_hTreeCtrl.Expand(m_hStorageRoot,TVE_EXPAND);
		}
	 }
	 catch( const ::Ice::Exception &ex)
	 {
			_stprintf(szMsg,_T("%s%s"),_T("Call listStorages  Ice exception with error:"), 
						   ex.ice_name().c_str());
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 catch(...)
	 {
		   _stprintf(szMsg,_T("%s"),_T("Call listStorages  System exception with error:")); 
						   
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 return TRUE;
}

BOOL  CAdminCtrl::ListStreamerId(BOOL bUpdate )
{
	 HTREEITEM hTemp;
     TVITEM tvi = {0};
     TVINSERTSTRUCT tvs;
     tvs.hInsertAfter = TVI_LAST; 
     tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE ;
	 TCHAR  szMsg[TEXT_LEN];

	 LVITEM lvItem   = {0};
	 if ( bUpdate )
	 {
		DeleteListCtrlColumn();
		std::string strName ="Storage";
		InsertListCtrlColumn(strName);

		lvItem.mask     = LVIF_TEXT| LVIF_IMAGE;
		lvItem.state = 0;
	 }
	 m_blistLink = FALSE;
	 
	 try
	 {
//		 std::string str1 =m_Communicator->proxyToString(m_adminPrx);

		TianShanIce::Transport::Streamers StreamerIdData = m_adminPrx->listStreamers();
		int size = StreamerIdData.size();

		for(int j = 0; j < size; j ++)
		{ 
			tvi.iImage = 1;
			tvi.iSelectedImage =1;

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = (LPSTR)(StreamerIdData[j].netId).c_str();
		   tvs.hParent = m_hStreamerRoot;
		   tvs.item = tvi;	
		   hTemp=m_hTreeCtrl.InsertItem(&tvs);
		   
		   tvi.iImage = 3;
	       tvi.iSelectedImage =3;

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText =_T("Storage Links");
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   m_hTreeCtrl.InsertItem(&tvs);


		   tvi.iImage = 4;
	       tvi.iSelectedImage =4;

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText =_T("Streamer Links");
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   m_hTreeCtrl.InsertItem(&tvs);


		   if ( bUpdate )
   		   {
			   lvItem.iItem = j;
			   lvItem.iSubItem  = 0;
			   lvItem.pszText   =(LPSTR)(StreamerIdData[j].netId).c_str();
	   		   m_hListCtrl.InsertItem(&lvItem);

			   lvItem.iSubItem  = 1;
			   lvItem.pszText   =(LPSTR)(StreamerIdData[j].type).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);

			   lvItem.iSubItem  = 2;
			   lvItem.pszText   =(LPSTR)(StreamerIdData[j].ifep).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);

			   lvItem.iSubItem  = 3;
			   lvItem.pszText   =(LPSTR)(StreamerIdData[j].desc).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);
		   }
		}
		StreamerIdData.clear();
		if ( bUpdate)
		{
			m_hTreeCtrl.Expand(m_hStreamerRoot,TVE_EXPAND);
		}	
	 }
	 catch( const ::Ice::Exception &ex)
	 {
			_stprintf(szMsg,_T("%s%s"),_T("Call liststreams  Ice exception with error:"), 
						   ex.ice_name().c_str());
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 catch(...)
	 {
		   _stprintf(szMsg,_T("%s"),_T("Call liststreams  System exception with error:")); 
						   
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 
	 return TRUE;
}
BOOL CAdminCtrl::ListSGId(BOOL bUpdate )
{
	 HTREEITEM hTemp;
     TVITEM tvi = {0};
     TVINSERTSTRUCT tvs;
     tvs.hInsertAfter = TVI_LAST; 
     tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE ;
 
	 TCHAR  szMsg[TEXT_LEN];

	 char   szTemp[20];
	 LVITEM lvItem   = {0};
	 if ( bUpdate )
	 {
		DeleteListCtrlColumn();
		std::string strName ="ServiceGroup";
		InsertListCtrlColumn(strName);
		
		lvItem.mask     = LVIF_TEXT| LVIF_IMAGE;
		lvItem.state = 0;
	 }	 

	 
	 m_blistLink = FALSE;
	 try
	 {
		TianShanIce::Transport::ServiceGroups SgData = m_adminPrx->listServiceGroups();
		
		int size = SgData.size();
		
		for(int j = 0; j < size; j ++)
		{ 
			
		   tvs.hInsertAfter = TVI_LAST; 
		   itoa(SgData[j].id,szTemp,10);

		   tvi.iImage = 2;
		   tvi.iSelectedImage =2;

		   tvi.pszText = (LPTSTR)szTemp;
		   tvs.hParent = m_hSGRoot;
		   tvs.item = tvi;	
		   hTemp=m_hTreeCtrl.InsertItem(&tvs);
		  
		   tvi.iImage = 4;
	       tvi.iSelectedImage =4;
		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText =_T("Streamer Links");
		   tvs.hParent = hTemp;
		   tvs.item = tvi;
		   m_hTreeCtrl.InsertItem(&tvs);


		   if ( bUpdate)
		   {
				lvItem.iItem = j;
		   		lvItem.iSubItem  = 0;
				lvItem.pszText   =(LPSTR)szTemp;
	   			m_hListCtrl.InsertItem(&lvItem);

				lvItem.iSubItem  = 1;
				lvItem.pszText   =(LPSTR)(SgData[j].desc).c_str();
	   			m_hListCtrl.SetItem(&lvItem);
		   }
		}
		SgData.clear();
		if ( bUpdate )
		{
			m_hTreeCtrl.Expand(m_hSGRoot,TVE_EXPAND);
		}
	 }
	 catch( const ::Ice::Exception &ex)
	 {
			_stprintf(szMsg,_T("%s%s"),_T("Call listsevicegroup  Ice exception with error:"), 
						   ex.ice_name().c_str());
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 catch(...)
	 {
		   _stprintf(szMsg,_T("%s"),_T("Call listsevicegroup  System exception with error:")); 
						   
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 
	 return TRUE;
}

BOOL  CAdminCtrl::ListSite(BOOL bUpdate)
{
	 TCHAR szMsg[TEXT_LEN];
	 HTREEITEM hTemp;
     TVITEM tvi = {0};
     TVINSERTSTRUCT tvs;
     tvs.hInsertAfter = TVI_LAST; 
     tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE ;
	 
	 LVITEM lvItem   = {0};
	 lvItem.mask     = LVIF_TEXT| LVIF_IMAGE;
	 lvItem.state = 0;

	 if ( bUpdate )
	 {
		std::string strName ="Site";
		DeleteListCtrlColumn();
		InsertListCtrlColumn(strName);
	 }
	 m_blistLink = FALSE;
	 try
	 {
		 
		TianShanIce::Site::VirtualSites SiteData = m_bizPrx->listSites();
				
		int size = SiteData.size();
		
		for(int j = 0; j < size; j ++)
		{ 

		   tvi.iImage = 5;
		   tvi.iSelectedImage =5;
		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = (LPSTR)(SiteData[j].name).c_str();
		   tvs.hParent = m_hSiteRoot;
		   tvs.item = tvi;	
		   hTemp=m_hTreeCtrl.InsertItem(&tvs);
		   
		   if ( bUpdate )
		   {
			   lvItem.iItem = j;
		   
			   lvItem.iSubItem  = 0;
			   lvItem.pszText   =(LPSTR)(SiteData[j].name).c_str();
	   		   m_hListCtrl.InsertItem(&lvItem);

			   lvItem.iSubItem  = 1;
			   lvItem.pszText   =(LPSTR)(SiteData[j].desc).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);

		   }
		}
		SiteData.clear();
		if ( bUpdate )
		{
			m_hTreeCtrl.Expand(m_hSiteRoot,TVE_EXPAND);
		}
	 }
	 catch( const ::Ice::Exception &ex)
	 {
			_stprintf(szMsg,_T("%s%s"),_T("Call listSites  Ice exception with error:"), 
						   ex.ice_name().c_str());
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 catch(...)
	 {
		   _stprintf(szMsg,_T("%s"),_T("Call listSites  System exception with error:")); 
						   
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 return TRUE;
}

BOOL  CAdminCtrl::ListApp(BOOL bUpdate )
{
	 TCHAR szMsg[TEXT_LEN];
	 HTREEITEM hTemp;
     TVITEM tvi = {0};
     TVINSERTSTRUCT tvs;
     tvs.hInsertAfter = TVI_LAST; 
     tvi.mask = TVIF_TEXT | TVIF_SELECTEDIMAGE | TVIF_IMAGE ;
	 
	 LVITEM lvItem   = {0};
	 if ( bUpdate )
	 {
		DeleteListCtrlColumn();
		std::string strName ="App";
		InsertListCtrlColumn(strName);

		lvItem.mask     = LVIF_TEXT| LVIF_IMAGE;
		lvItem.state = 0;
	 }
	 m_blistLink = FALSE;
	 	 
	 try
	 {
		TianShanIce::Site::AppInfos AppData = m_bizPrx->listApplications();
		
		int size = AppData.size();

		for(int j = 0; j < size; j ++)
		{ 
			tvi.iImage = 6;
			tvi.iSelectedImage =6;

		   tvs.hInsertAfter = TVI_LAST; 
		   tvi.pszText = (LPSTR)(AppData[j].name).c_str();
		   tvs.hParent = m_hAppRoot;
		   tvs.item = tvi;	
		   hTemp=m_hTreeCtrl.InsertItem(&tvs);
		   
		   		   
		   if ( bUpdate )
   		   {
			   lvItem.iItem = j;
			   lvItem.iSubItem  = 0;
			   lvItem.pszText   =(LPSTR)(AppData[j].name).c_str();
	   		   m_hListCtrl.InsertItem(&lvItem);

			   lvItem.iSubItem  = 1;
			   lvItem.pszText   =(LPSTR)(AppData[j].endpoint).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);

			   lvItem.iSubItem  = 2;
			   lvItem.pszText   =(LPSTR)(AppData[j].desc).c_str();
	   		   m_hListCtrl.SetItem(&lvItem);

		   }
		}
		AppData.clear();
		if ( bUpdate )	
		{
			m_hTreeCtrl.Expand(m_hAppRoot,TVE_EXPAND);
		}
	 }
	 catch( const ::Ice::Exception &ex)
	 {
			_stprintf(szMsg,_T("%s%s"),_T("Call listApplications  Ice exception with error:"), 
						   ex.ice_name().c_str());
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 catch(...)
	 {
		   _stprintf(szMsg,_T("%s"),_T("Call listApplications  System exception with error:")); 
						   
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
	 }
	 return TRUE;
}

BOOL CAdminCtrl::DisplayStreamerInfo(SData *data)
{
   	HTREEITEM hSelect = hSelect = m_hTreeCtrl.GetSelectedItem();
	char temp[TEXT_LEN], *key;
	char strText[TEXT_LEN];
	std::string strTemp;
		
	std::string strName ="Stream";
	DeleteListCtrlLinkColumn();
	InsertListCtrlLinkColumn(strName);


	m_blistLink = FALSE;
	m_hTreeCtrl.GetItemText(hSelect,temp,TEXT_LEN);
	strTemp = temp;
	try
	{
//		std::string str1 =m_Communicator->proxyToString(m_adminPrx);
		TianShanIce::Transport::Streamers streamers = m_adminPrx->listStreamers();
		
		int size = streamers.size();
	   
		for( int j = 0; j < size; j ++)
		{ 		
		   if(streamers[j].netId == strTemp)
			   break;		   
		}
		if( j == size)
		{
		   return FALSE;
		}
	   
		DeleteListCtrlColumn();
		strName ="Stream";
		InsertListCtrlColumn(strName);
		
			   
		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
		lvItem.state = 0;
		int i = -1;
				
		lvItem.iItem = 0;
		lvItem.iSubItem  = 0;
		lvItem.pszText   =(LPSTR)(streamers[j].netId).c_str();
	    m_hListCtrl.InsertItem(&lvItem);

 	    lvItem.iSubItem  = 1;
	    lvItem.pszText   =(LPSTR)(streamers[j].type).c_str();
	    m_hListCtrl.SetItem(&lvItem);

	    lvItem.iSubItem  = 2;
	    lvItem.pszText   =(LPSTR)(streamers[j].ifep).c_str();
	    m_hListCtrl.SetItem(&lvItem);

	    lvItem.iSubItem  = 3;
	    lvItem.pszText   =(LPSTR)(streamers[j].desc).c_str();
	    m_hListCtrl.SetItem(&lvItem);

	
	   TianShanIce::ValueMap vale =  streamers[j].privateData;
	   TianShanIce::ValueMap::iterator pos = vale.begin();

	   
	   while(pos != vale.end())
	   {
		   if(!GetPrivateDataStr(strText,&(pos->second)))
		   {
			   MessageBox("GetPrivateDataStr error!!");
			   return FALSE;	
		   } 
		   lvItem.iItem = ++i;
		   key = (LPSTR) pos->first.c_str();
		   lvItem.pszText =(LPSTR)key; 
		   m_hListCtrlLinks.InsertItem(&lvItem);
		   m_hListCtrlLinks.SetItemText(i,1,(LPSTR)strText);
		   ++pos;
	   }
	   if(data)
	   {
		   data->netId = streamers[j].netId;
		   data->desc = streamers[j].desc;
		   data->type = streamers[j].type;
		   data->ifep = streamers[j].ifep;	
		   data->flag = TRUE;
	   }
	   streamers.clear();
	}
	catch( const ::Ice::Exception &ex)
	{
		memset(strText,0,sizeof(strText));
		sprintf(strText,("%s%s"),("Call listStreams  Ice exception with error:"), 
					   ex.ice_name().c_str());
		ATLTRACE(strText);
		MessageBox(strText);
		return FALSE;
	}
	catch(...)
	{
	    sprintf(strText,("%s"),("Call listStreams  System exception with error:")); 
					   
		ATLTRACE(strText);
		MessageBox(strText);
		return FALSE;
	}
	return TRUE;
}

BOOL CAdminCtrl::DisplayStorageInfo(SData *data)
{
   	HTREEITEM hSelect = hSelect = m_hTreeCtrl.GetSelectedItem();
	char temp[TEXT_LEN], *key;
	char strText[TEXT_LEN];
	std::string strtemp;
	
	std::string strName ="Stream";
	DeleteListCtrlLinkColumn();
	InsertListCtrlLinkColumn(strName);


	m_blistLink = FALSE;
	m_hTreeCtrl.GetItemText(hSelect,temp,TEXT_LEN);
	strtemp = temp;

	try
	{
	   TianShanIce::Transport::Storages storages = m_adminPrx->listStorages();
	   int size = storages.size();
	   
	   for( int j = 0; j < size; j ++)
	   { 		
		   if(storages[j].netId == strtemp)
			   break;		   
	   }
	   if( j == size)
	   {
		   return FALSE;
	   }
	
	    DeleteListCtrlColumn();
		strName ="Storage";
		InsertListCtrlColumn(strName);

			   
		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
		lvItem.state = 0;
		int i = -1;
				
		lvItem.iItem = 0;
		lvItem.iSubItem  = 0;
		lvItem.pszText   =(LPSTR)(storages[j].netId).c_str();
	    m_hListCtrl.InsertItem(&lvItem);

 	    lvItem.iSubItem  = 1;
	    lvItem.pszText   =(LPSTR)(storages[j].type).c_str();
	    m_hListCtrl.SetItem(&lvItem);

	    lvItem.iSubItem  = 2;
	    lvItem.pszText   =(LPSTR)(storages[j].ifep).c_str();
	    m_hListCtrl.SetItem(&lvItem);

	    lvItem.iSubItem  = 3;
	    lvItem.pszText   =(LPSTR)(storages[j].desc).c_str();
	    m_hListCtrl.SetItem(&lvItem);

			   
	    TianShanIce::ValueMap vale =  storages[j].privateData;
	    TianShanIce::ValueMap::iterator pos = vale.begin();
       
	    while(pos != vale.end())
		{
		   if(!GetPrivateDataStr(strText,&(pos->second)))
		   {
			   MessageBox("GetPrivateDataStr error!!");
			   return FALSE;	
		   } 
		   lvItem.iItem = ++i;
		   key = (LPSTR) pos->first.c_str();	  
		   lvItem.pszText =(LPSTR)key; 
		   m_hListCtrlLinks.InsertItem(&lvItem);
		   m_hListCtrlLinks.SetItemText(i,1,(LPSTR)strText);
		   ++pos;
	   }
      
	   if(data)	
	   {
		   data->netId = storages[j].netId;
		   data->desc = storages[j].desc;
		   data->type = storages[j].type;
		   data->ifep = storages[j].ifep;
		   data->flag = TRUE;
	   }
	   storages.clear();	
	}
	catch( const ::Ice::Exception &ex)
	{
		memset(strText,0,sizeof(strText));
		_stprintf(strText,_T("%s%s"),_T("Call liststorages  Ice exception with error:"), 
					   ex.ice_name().c_str());
		ATLTRACE(strText);
		MessageBox(strText);
		return FALSE;
	}
	catch(...)
	{
        memset(strText,0,sizeof(strText));
	    _stprintf(strText,_T("%s"),_T("Call liststorages  System exception with error:")); 
					   
		ATLTRACE(strText);
		MessageBox(strText);
		return FALSE;
	}
	return TRUE;
}

BOOL CAdminCtrl::DisplaySGInfo(SData *data)
{
	HTREEITEM hSelect = hSelect = m_hTreeCtrl.GetSelectedItem();
	char temp[TEXT_LEN];
	int nId;
		
	std::string strName ="Other";
	DeleteListCtrlLinkColumn();
	InsertListCtrlLinkColumn(strName);

	m_blistLink = FALSE;
	m_hTreeCtrl.GetItemText(hSelect,temp,TEXT_LEN);
	nId = atoi(temp);

	try
	{
	   TianShanIce::Transport::ServiceGroups serviestore = m_adminPrx->listServiceGroups();



	   int size = serviestore.size();
	   
	   for( int j = 0; j < size; j ++)
	   { 		
		   if(serviestore[j].id == nId)
			   break;		   
	   }
	   
	   if( j == size)
	   {
		   return FALSE;
	   }

        DeleteListCtrlColumn();
		strName ="ServiceGroup";
		InsertListCtrlColumn(strName);

	   
	    LVITEM lvItem;
	    lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	    lvItem.state = 0;
	  

	    lvItem.iItem = 0;
		lvItem.iSubItem  = 0;
		lvItem.pszText   =(LPSTR)temp;
		m_hListCtrl.InsertItem(&lvItem);

		lvItem.iSubItem  = 1;
		lvItem.pszText   =(LPSTR)(serviestore[j].desc).c_str();
		m_hListCtrl.SetItem(&lvItem);
	   
	   if(data)
	   {
		   data->id = nId;
		   data->desc = serviestore[j].desc;
		   data->flag = TRUE;
	   }
	   serviestore.clear();		   
	}
	catch( const ::Ice::Exception &ex)
	{
		memset(temp,0,sizeof(temp));
		sprintf(temp,("%s%s"),("Call listServiceGroups  Ice exception with error:"), 
					   ex.ice_name().c_str());
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	catch(...)
	{
		memset(temp,0,sizeof(temp));
	    sprintf(temp,("%s"),("Call listServiceGroups  System exception with error:")); 
					   
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	return TRUE;
}

BOOL CAdminCtrl::ShowSite(std::string  sitename)
{
	 TCHAR szMsg[TEXT_LEN];
	 int cProp = 0;
	 std::string strTemp;
	 char sTemp[20];
	 int iSite,iApp;

	 LVITEM lvItem ;
	 lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
     lvItem.state = 0;
	 lvItem.iSubItem = 0;
	     
	 try
	 {
		m_hListCtrlLinks.DeleteAllItems();

		TianShanIce::Site::VirtualSites collection = m_bizPrx->listSites();
		TianShanIce::Site::VirtualSites::iterator it;
		for (it = collection.begin(); it < collection.end(); it++)
		{
			if (sitename == it->name)
			{
				break;
			}
		}
		if (it >= collection.end())
		{
			return FALSE;
		}
		
		iSite = it->properties.size();

		for (TianShanIce::Properties::iterator pit = it->properties.begin(); pit != it->properties.end(); pit++, cProp++)
		{

			lvItem.iSubItem = 0;
			lvItem.iItem = cProp;
			strTemp  ="[";
			strTemp += pit->first.c_str();
			strTemp +="]";
			strTemp +=" = ";
			strTemp +=pit->second.c_str();

			lvItem.pszText = (LPSTR)strTemp.c_str(); 
			m_hListCtrlLinks.InsertItem(&lvItem);
		}

		memset(sTemp,0,sizeof(sTemp));
		memset(&lvItem,0,sizeof(lvItem));
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 

		lvItem.iSubItem = 0;
		lvItem.iItem = cProp;
		itoa(cProp,sTemp,10);

		strTemp  = sTemp;
		strTemp += " properties in the site";

		lvItem.pszText = (LPSTR)strTemp.c_str(); 
		m_hListCtrlLinks.InsertItem(&lvItem);

		
		TianShanIce::Site::AppMounts mounts =  m_bizPrx->listMounts(it->name);
		iApp = mounts.size();

		if ( iApp > iSite)
		{
			for ( int i = (iSite+1); i < (iApp+1); i ++)
			{
				lvItem.iSubItem = 0;
				lvItem.iItem = i;
				lvItem.pszText = _T(""); 
				m_hListCtrlLinks.InsertItem(&lvItem);
			}
		}
	
		cProp = 0;
		for (TianShanIce::Site::AppMounts::iterator ait = mounts.begin(); ait != mounts.end(); ait++)
		{
			TianShanIce::Site::AppMountPrx mount = *ait;
			if (!mount)
				continue;
			
			strTemp  ="path[";
			strTemp += mount->getMountedPath().c_str();
			strTemp +="]";
			strTemp +=" -> app[";
			strTemp += mount->getAppName().c_str();
			strTemp +="]";

		    m_hListCtrlLinks.SetItemText(cProp,1,(LPSTR)strTemp.c_str());
			cProp++;
		}
		memset(sTemp,0,sizeof(sTemp));
		itoa(cProp,sTemp,10);
		strTemp  = sTemp;
		strTemp += " applications in the site";
		m_hListCtrlLinks.SetItemText(cProp,1,(LPSTR)strTemp.c_str());
	}
	catch( const ::Ice::Exception &ex)
	{
		_stprintf(szMsg,_T("%s%s"),_T("Call  listSites method  failed Ice Exception with error "),ex.ice_name().c_str());
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}
	catch(...)
	{
	   _stprintf(szMsg,_T("%s"),_T("Call listServiceGroups  System exception with error:")); 
					   
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}
	return TRUE;
}

BOOL  CAdminCtrl::DisplaySiteInfo(SData *data )
{
	HTREEITEM hSelect = hSelect = m_hTreeCtrl.GetSelectedItem();
	char temp[TEXT_LEN];
	std::string strtemp;
	
	std::string strName ="Site";
	DeleteListCtrlLinkColumn();
	InsertListCtrlLinkColumn(strName);
	m_blistLink = FALSE; 

	
	m_hTreeCtrl.GetItemText(hSelect,temp,TEXT_LEN);
	strtemp = temp;

	try
	{
		TianShanIce::Site::VirtualSites stores = m_bizPrx->listSites();
		int size = stores.size();
		
	    for( int j = 0; j < size; j ++)
		{ 		
		   if(stores[j].name == strtemp)
			   break;		   
		}
	    if( j == size)
		{
		   return FALSE;
		}
	
	    DeleteListCtrlColumn();
		strName ="Site";
		InsertListCtrlColumn(strName);
			   
		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
		lvItem.state = 0;
		int i = -1;
				
		lvItem.iItem = 0;
		lvItem.iSubItem  = 0;
		lvItem.pszText   =(LPSTR)(stores[j].name).c_str();
	    m_hListCtrl.InsertItem(&lvItem);

 	    lvItem.iSubItem  = 1;
	    lvItem.pszText   =(LPSTR)(stores[j].desc).c_str();
	    m_hListCtrl.SetItem(&lvItem);
		
		if(data)	
		{
		   data->netId    = stores[j].name; // the same to name;
		   data->desc     = stores[j].desc;
		   data->iMaxSession=stores[j].maxSessions;
		   data->lMaxBW=stores[j].maxDownstreamBwKbps;
		   data->flag     = TRUE;
		}
		stores.clear();
		ShowSite(strtemp);
	}
	catch( const ::Ice::Exception &ex)
	{
		memset(temp,0,sizeof(temp));
		sprintf(temp,("%s%s"),("Call  listSites method  failed Ice Exception with error "),ex.ice_name().c_str());
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	catch(...)
	{
		memset(temp,0,sizeof(temp));
		sprintf(temp,("%s"),"Call listSites  System exception with error:"); 
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	return TRUE;
}

BOOL  CAdminCtrl::DisplayAppInfo(SData *data )
{
	HTREEITEM hSelect = hSelect = m_hTreeCtrl.GetSelectedItem();
	char temp[TEXT_LEN];
	std::string strtemp;
	
	std::string strName ="Other";
	DeleteListCtrlLinkColumn();
	InsertListCtrlLinkColumn(strName);
	m_blistLink = FALSE;

	
	m_hTreeCtrl.GetItemText(hSelect,temp,TEXT_LEN);
	strtemp = temp;

	try
	{
		
		TianShanIce::Site::AppInfos stores = m_bizPrx->listApplications();
				
		int size = stores.size();
		
	    for( int j = 0; j < size; j ++)
		{ 		
		   if(stores[j].name == strtemp)
			   break;		   
		}
	    if( j == size)
		{
		   return FALSE;
		}
	
	    DeleteListCtrlColumn();
		strName ="App";
		InsertListCtrlColumn(strName);

			   
		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
		lvItem.state = 0;
		int i = -1;
				
		lvItem.iItem = 0;
		lvItem.iSubItem  = 0;
		lvItem.pszText   =(LPSTR)(stores[j].name).c_str();
	    m_hListCtrl.InsertItem(&lvItem);

		lvItem.iSubItem  = 1;
	    lvItem.pszText   =(LPSTR)(stores[j].endpoint).c_str();
	    m_hListCtrl.SetItem(&lvItem);

 	    lvItem.iSubItem  = 2;
	    lvItem.pszText   =(LPSTR)(stores[j].desc).c_str();
	    m_hListCtrl.SetItem(&lvItem);

		if(data)	
		{
		   data->netId    = stores[j].name;
		   data->ifep     = stores[j].endpoint; // the same to endpoint
		   data->desc     = stores[j].desc;
		   data->flag     = TRUE;
		}
		stores.clear();
	}
	catch( const ::Ice::Exception &ex)
	{
		memset(temp,0,sizeof(temp));
		sprintf(temp,("%s%s"),("Call  listApplications method  failed Ice Exception with error:"),ex.ice_name().c_str());
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	catch(...)
	{
		memset(temp,0,sizeof(temp));
		sprintf(temp,("%s"),"Call listApplications  System exception with error:"); 
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	return TRUE;
}

BOOL CAdminCtrl::DisplayStorageLinks()
{
	char temp[20];
	std::string strName ="StorageLink";
	DeleteListCtrlLinkColumn();
	InsertListCtrlLinkColumn(strName);
     
   	HTREEITEM hSelect ,hParent; 
	char strTreeText[TEXT_LEN];
	hSelect = m_hTreeCtrl.GetSelectedItem();

	hParent = m_hTreeCtrl.GetNextItem(hSelect, TVGN_PARENT);
		
	m_blistLink = TRUE;


	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
    lvItem.state = 0;
	lvItem.iSubItem = 0;
    int i = -1;
	std::string strtemp;
	
	m_hTreeCtrl.GetItemText(hParent,strTreeText,TEXT_LEN);
	strtemp = strTreeText;

	try
	{
		TianShanIce::Transport::StorageLinks storagelinks = m_adminPrx->listStorageLinksByStorage(strtemp);

		DeleteListCtrlColumn();
		std::string strName ="key";
		InsertListCtrlColumn(strName);

		lvItem.iItem = 0;
		lvItem.iSubItem  = 0;
		lvItem.pszText =_T("StorageLinks Count");
	   	m_hListCtrl.InsertItem(&lvItem);

		
		lvItem.iItem = 0;
		lvItem.iSubItem  = 1;
		lvItem.pszText =(LPSTR)itoa(storagelinks.size(), temp , 10);
		m_hListCtrl.SetItem(&lvItem);

		memset(&lvItem,0,sizeof(lvItem));
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 

 		for (TianShanIce::Transport::StorageLinks::iterator it = storagelinks.begin(); it < storagelinks.end(); it++)
		{
			
			lvItem.state = 0;
			lvItem.iSubItem = 0;

		    lvItem.iItem = ++i;
		    strtemp = (*it)->getIdent().name;
		    lvItem.pszText = (LPSTR)strtemp.c_str(); 
			m_hListCtrlLinks.InsertItem(&lvItem);
		   
		    strtemp =(*it)->getType() ;
		    m_hListCtrlLinks.SetItemText(i,1,(LPSTR)strtemp.c_str());

		   
		   strtemp = (*it)->getStorageId() ;
		   m_hListCtrlLinks.SetItemText(i,2,(LPSTR)strtemp.c_str());

		   
		   strtemp = (*it)->getStreamerId();
		   m_hListCtrlLinks.SetItemText(i,3,(LPSTR)strtemp.c_str());
		   
		   strtemp = m_Communicator->proxyToString(*it);
		   m_hListCtrlLinks.SetItemText(i,4,(LPSTR)strtemp.c_str());

		}
	}
	catch( const ::Ice::Exception &ex)
	{
		memset(strTreeText,0,sizeof(strTreeText));
		sprintf(strTreeText,("%s%s"),("Call  listStorageLinksByStorage method  failed Ice Exception with error :"),ex.ice_name().c_str());
		ATLTRACE(strTreeText);
		MessageBox(strTreeText);
		return FALSE;
	}
	catch(...)
	{
		memset(strTreeText,0,sizeof(strTreeText));
		sprintf(strTreeText,("%s"),"Call listStorageLinksByStorage  System exception with error:"); 
		ATLTRACE(strTreeText);
		MessageBox(strTreeText);
		return FALSE;
	}
	return TRUE;
}

BOOL CAdminCtrl::DisplayStreamLinks()
{
	char temp[15];
	std::string strName ="StreamLink";
	
	HTREEITEM hSelect ,hParent,hRoot; 
	char strTreeText[TEXT_LEN];
	char szMsg[TEXT_LEN];
	hSelect = m_hTreeCtrl.GetSelectedItem();

	hParent = m_hTreeCtrl.GetNextItem(hSelect, TVGN_PARENT);
	hRoot = m_hTreeCtrl.GetNextItem(hParent, TVGN_PARENT); 
	m_hTreeCtrl.GetItemText(hSelect,strTreeText,TEXT_LEN);
	m_blistLink = TRUE;

	LVITEM lvItem ;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
    lvItem.state = 0;
	lvItem.iSubItem = 0;
    int i = -1;
	std::string strtemp;


	DeleteListCtrlColumn();
	strName ="key";
	InsertListCtrlColumn(strName);
			

	// 
	if(!strcmp(strTreeText,"Storage Links"))
	{
		m_hTreeCtrl.GetItemText(hParent,strTreeText,TEXT_LEN);
		strtemp = strTreeText;

		try
		{
	
			TianShanIce::Transport::StorageLinks storagelinks = m_adminPrx->listStorageLinksByStreamer(strtemp);

			lvItem.iItem = 0;
			lvItem.iSubItem  = 0;
			lvItem.pszText =_T("storagelinks Count");
			m_hListCtrl.InsertItem(&lvItem);


			lvItem.iItem = 0;
			lvItem.iSubItem  = 1;
			lvItem.pszText =(LPSTR)itoa(storagelinks.size(), temp , 10);
			m_hListCtrl.SetItem(&lvItem);

			strName ="StorageLink";
			DeleteListCtrlLinkColumn();
			InsertListCtrlLinkColumn(strName);

			memset(&lvItem,0,sizeof(lvItem));
			lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 

			for (TianShanIce::Transport::StorageLinks::iterator it = storagelinks.begin(); it < storagelinks.end(); it++)
			{
				
				lvItem.state = 0;
				lvItem.iSubItem = 0;

				lvItem.iItem = ++i;
				strtemp = (*it)->getIdent().name;
				lvItem.pszText = (LPSTR)strtemp.c_str(); 
				m_hListCtrlLinks.InsertItem(&lvItem);
				
				strtemp =(*it)->getType() ;
				m_hListCtrlLinks.SetItemText(i,1,(LPSTR)strtemp.c_str());
				
				strtemp = (*it)->getStorageId() ;
				m_hListCtrlLinks.SetItemText(i,2,(LPSTR)strtemp.c_str());

				
				strtemp = (*it)->getStreamerId();
				m_hListCtrlLinks.SetItemText(i,3,(LPSTR)strtemp.c_str());
				
				strtemp = m_Communicator->proxyToString(*it);
				m_hListCtrlLinks.SetItemText(i,4,(LPSTR)strtemp.c_str());
			 }
		}
		catch( const ::Ice::Exception &ex)
		{
			memset(szMsg,0,sizeof(szMsg));
			sprintf(szMsg,("%s%s"),("Call  listStorageLinksByStreamer method  failed Ice Exception with error :"),ex.ice_name().c_str());
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
		}
		catch(...)
		{
			memset(szMsg,0,sizeof(szMsg));
			sprintf(szMsg,("%s"),"Call listStorageLinksByStreamer  System exception with error:"); 
			ATLTRACE(szMsg);
			MessageBox(szMsg);
			return FALSE;
		}
	}
	else 
	{
		if(!strcmp(strTreeText,"Streamer Links")&& hRoot == m_hStreamerRoot)
		{
			m_hTreeCtrl.GetItemText(hParent,strTreeText,TEXT_LEN);
			strtemp = strTreeText;

			try
			{
				TianShanIce::Transport::StreamLinks streamlinks = m_adminPrx->listStreamLinksByStreamer(strtemp);

				lvItem.iItem = 0;
				lvItem.iSubItem  = 0;
				lvItem.pszText =_T("StreamLinks Count");
				m_hListCtrl.InsertItem(&lvItem);


				lvItem.iItem = 0;
				lvItem.iSubItem  = 1;
				lvItem.pszText =(LPSTR)itoa(streamlinks.size(), temp , 10);
				m_hListCtrl.SetItem(&lvItem);

				strName ="StreamLink";
				DeleteListCtrlLinkColumn();
				InsertListCtrlLinkColumn(strName);

				memset(&lvItem,0,sizeof(lvItem));
     			lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 


				for (TianShanIce::Transport::StreamLinks::iterator it = streamlinks.begin(); it < streamlinks.end(); it++)
				{
					
					lvItem.iItem = ++i;
					strtemp = (*it)->getIdent().name;
					lvItem.pszText = (LPSTR)strtemp.c_str(); 
					m_hListCtrlLinks.InsertItem(&lvItem);
					
					strtemp =(*it)->getType() ;
					m_hListCtrlLinks.SetItemText(i,1,(LPSTR)strtemp.c_str());

					strtemp = (*it)->getStreamerId() ;
					m_hListCtrlLinks.SetItemText(i,2,(LPSTR)strtemp.c_str());
					m_hListCtrlLinks.SetItemText(i,3,(LPSTR)itoa((*it)->getServiceGroupId(),temp, 10));
					
					strtemp = m_Communicator->proxyToString(*it);
					m_hListCtrlLinks.SetItemText(i,4,(LPSTR)strtemp.c_str());
				}
			}
			catch( const ::Ice::Exception &ex)
			{
				memset(szMsg,0,sizeof(szMsg));
				sprintf(szMsg,("%s%s"),("Call  listStreamLinksByStreamer method  failed Ice Exception with error :"),ex.ice_name().c_str());
				ATLTRACE(szMsg);
				MessageBox(szMsg);
				return FALSE;
			}
			catch(...)
			{
				memset(szMsg,0,sizeof(szMsg));
				sprintf(szMsg,("%s"),"Call listStreamLinksByStreamer  System exception with error:"); 
				ATLTRACE(szMsg);
				MessageBox(szMsg);
				return FALSE;
			}
			
		}
		else
		{
			m_hTreeCtrl.GetItemText(hParent,strTreeText,TEXT_LEN);
		
			try
			{
			
				TianShanIce::Transport::StreamLinks streamlinks = m_adminPrx->listStreamLinksByServiceGroup(atoi(strTreeText));

				lvItem.iItem = 0;
				lvItem.iSubItem  = 0;
				lvItem.pszText =_T("StreamLinks Count");
				m_hListCtrl.InsertItem(&lvItem);


				lvItem.iItem = 0;
				lvItem.iSubItem  = 1;
				lvItem.pszText =(LPSTR)itoa(streamlinks.size(), temp , 10);
				m_hListCtrl.SetItem(&lvItem);

				strName ="StreamLink";
				DeleteListCtrlLinkColumn();
				InsertListCtrlLinkColumn(strName);

				memset(&lvItem,0,sizeof(lvItem));
     			lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 

				for (TianShanIce::Transport::StreamLinks::iterator it = streamlinks.begin(); it < streamlinks.end(); it++)
				{
					
					lvItem.iItem = ++i;
					strtemp = (*it)->getIdent().name;
					lvItem.pszText = (LPSTR)strtemp.c_str(); 

					m_hListCtrlLinks.InsertItem(&lvItem);
					
					strtemp =(*it)->getType() ;
					m_hListCtrlLinks.SetItemText(i,1,(LPSTR)strtemp.c_str());

					strtemp = (*it)->getStreamerId() ;
					m_hListCtrlLinks.SetItemText(i,2,(LPSTR)strtemp.c_str());

					m_hListCtrlLinks.SetItemText(i,3,(LPSTR)itoa((*it)->getServiceGroupId(),temp, 10));
					
					strtemp = m_Communicator->proxyToString(*it);
					m_hListCtrlLinks.SetItemText(i,4,(LPSTR)strtemp.c_str());

				}
			}
			catch( const ::Ice::Exception &ex)
			{
				memset(szMsg,0,sizeof(szMsg));
				sprintf(szMsg,("%s%s"),("Call  listStreamLinksByServiceGroup method  failed Ice Exception with error :"),ex.ice_name().c_str());
				ATLTRACE(szMsg);
				MessageBox(szMsg);
				return FALSE;
			}
			catch(...)
			{
				memset(szMsg,0,sizeof(szMsg));
				sprintf(szMsg,("%s"),"Call listStreamLinksByServiceGroup  System exception with error:"); 
				ATLTRACE(szMsg);
				MessageBox(szMsg);
				return FALSE;
			}
			
			
		}
	}
	return TRUE;
}

BOOL  CAdminCtrl::DeleteListCtrlColumn()
{
//	::LockWindowUpdate( m_hListCtrl );
	m_hListCtrl.DeleteAllItems();
	int ncol = m_iListColNum;
	while(ncol--)
	{
		m_hListCtrl.DeleteColumn(ncol);
	}
//	::LockWindowUpdate( NULL );
	return TRUE;
}

BOOL  CAdminCtrl::DeleteListCtrlLinkColumn()
{
//	::LockWindowUpdate( m_hListCtrlLinks );
	m_hListCtrlLinks.DeleteAllItems();
	
	int ncol = m_iListLinkNum;
	while(ncol--)
	{
		m_hListCtrlLinks.DeleteColumn(ncol);
	}
//	::LockWindowUpdate( NULL );
	return TRUE;
}

BOOL  CAdminCtrl::InsertListCtrlColumn(std::string strName)
{
//	::LockWindowUpdate( m_hListCtrl );
	LV_COLUMN lvCol ={0};

	CRect rcClient;
	GetClientRect(&rcClient);
	
	int icx = ( 3* rcClient.Width() )/ 4;
	
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;

	if ( strName =="ServiceGroup")
	{
		m_iListColNum = 2;
		icx = icx / m_iListColNum;
		lvCol.cx = icx;
		lvCol.pszText = _T("ID");
		m_hListCtrl.InsertColumn(0,&lvCol);
	
		lvCol.cx = icx;
		lvCol.pszText = "Description";
		m_hListCtrl.InsertColumn(1,&lvCol);
	}
	else if ( strName =="key")
	{
		m_iListColNum = 2;
		icx = icx / m_iListColNum;
		lvCol.cx = icx;
		lvCol.pszText = _T("Key");
		m_hListCtrl.InsertColumn(0,&lvCol);
	
		lvCol.cx = icx;
		lvCol.pszText = "Value";
		m_hListCtrl.InsertColumn(1,&lvCol);
	}
	else if ( strName =="Site")
	{
		m_iListColNum = 2;
		icx = icx / 2;
		lvCol.cx = icx;
		lvCol.pszText = _T("Name");
		m_hListCtrl.InsertColumn(0,&lvCol);
	
		lvCol.cx = icx;
		lvCol.pszText = "Description";
		m_hListCtrl.InsertColumn(1,&lvCol);
	}
	else if ( strName =="App")
	{
		m_iListColNum = 3;
		icx = icx / 3;
		lvCol.cx = icx - 45 ;
		lvCol.pszText = _T("App");
		m_hListCtrl.InsertColumn(0,&lvCol);
	
		lvCol.cx = icx + 45;
		lvCol.pszText = "EndPoint";
		m_hListCtrl.InsertColumn(1,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "Description";
		m_hListCtrl.InsertColumn(3,&lvCol);
	}
	else 
	{
		m_iListColNum = LISTCOLUMN_NUM;
		icx = icx / LISTCOLUMN_NUM;
		lvCol.cx = icx;
		lvCol.pszText = _T("NetId");
		m_hListCtrl.InsertColumn(0,&lvCol);
	
		lvCol.cx = icx;
		lvCol.pszText = "Type";
		m_hListCtrl.InsertColumn(1,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "EndPoint";
		m_hListCtrl.InsertColumn(2,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "Description";
		m_hListCtrl.InsertColumn(3,&lvCol);
	}
//	::LockWindowUpdate( NULL );
//	BOOL b1 = FALSE;
//	OnSize(0,0,0,b1);
	return TRUE;
}

BOOL  CAdminCtrl::InsertListCtrlLinkColumn(std::string strName)
{
//	::LockWindowUpdate( m_hListCtrlLinks );
	LV_COLUMN lvCol ={0};
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	CRect rcClient;
	GetClientRect(&rcClient);
	int icx = ( 3* rcClient.Width() )/ 4;

	if ( strName =="Stream")
	{
		m_iListLinkNum = 2;
		icx = icx / m_iListLinkNum;

		lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
		lvCol.fmt = LVCFMT_LEFT;
		lvCol.iSubItem = 0;
		
		lvCol.cx = icx;
		lvCol.pszText = _T("PrivateData Key");
		m_hListCtrlLinks.InsertColumn(0,&lvCol);
		
		lvCol.cx = icx;
		lvCol.pszText = "Value";
		m_hListCtrlLinks.InsertColumn(1,&lvCol);
		
	}
	else if ( strName =="StreamLink")
	{
		m_iListLinkNum = LISTLINKCOLUMN_NUM;
		m_bStorageLink = FALSE;
		icx = icx / LISTLINKCOLUMN_NUM;
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
		lvCol.fmt = LVCFMT_LEFT;
		lvCol.iSubItem = 0;
		
		lvCol.cx = icx;
		lvCol.pszText = _T("Ident");
		m_hListCtrlLinks.InsertColumn(0,&lvCol);
		
		lvCol.cx = icx;
		lvCol.pszText = "Type";
		m_hListCtrlLinks.InsertColumn(1,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "StreamerId";
		m_hListCtrlLinks.InsertColumn(2,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "ServiceGroupId";
		m_hListCtrlLinks.InsertColumn(3,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "ProxyString";
		m_hListCtrlLinks.InsertColumn(4,&lvCol);
	}
	else if ( strName =="StorageLink")
	{
		m_iListLinkNum = LISTLINKCOLUMN_NUM;
		m_bStorageLink = TRUE;
		icx = icx / LISTLINKCOLUMN_NUM;
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
		lvCol.fmt = LVCFMT_LEFT;
		lvCol.iSubItem = 0;
		
		lvCol.cx = icx;
		lvCol.pszText = _T("Ident");
		m_hListCtrlLinks.InsertColumn(0,&lvCol);
		
		lvCol.cx = icx;
		lvCol.pszText = "Type";
		m_hListCtrlLinks.InsertColumn(1,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "StorageId";
		m_hListCtrlLinks.InsertColumn(2,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "StreamerId";
		m_hListCtrlLinks.InsertColumn(3,&lvCol);

		lvCol.cx = icx;
		lvCol.pszText = "ProxyString";
		m_hListCtrlLinks.InsertColumn(4,&lvCol);
	}
	else if ( strName =="Site")
	{
		m_iListLinkNum = 2;
		
		icx = icx / 2;
		lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
		lvCol.fmt = LVCFMT_LEFT;
		lvCol.iSubItem = 0;
		
		lvCol.cx = icx;
		lvCol.pszText = _T("Properties");
		m_hListCtrlLinks.InsertColumn(0,&lvCol);
		
		lvCol.cx = icx;
		lvCol.pszText = "AppInfos";
		m_hListCtrlLinks.InsertColumn(1,&lvCol);
	}
	else
	{
		m_iListLinkNum = 1;
		lvCol.cx =icx ;
		lvCol.pszText = "";
		m_hListCtrlLinks.InsertColumn(0,&lvCol);
	}
//	::LockWindowUpdate( NULL );
//	BOOL b1 = FALSE;
//	OnSize(0,0,0,b1);
	return TRUE;
}

BOOL CAdminCtrl::ConnectServe(std::string strEndPoint,std::string strPathEndPoint)
{
	
	TCHAR szMsg[TEXT_LEN]={0};
	ResetCtrl();
	
	//std::string endpoint = "tcp -h 192.168.80.49  -p 10001";
	std::string endpoint = strEndPoint;

	try
	{
		m_adminPrx = TianShanIce::Transport::PathAdminPrx::checkedCast(m_Communicator->stringToProxy(std::string(ADAPTER_NAME_PathManager) + ":" + strPathEndPoint));
		m_bizPrx = TianShanIce::Site::SiteAdminPrx::checkedCast(m_Communicator->stringToProxy(std::string(SERVICE_NAME_BusinessRouter) +":" + endpoint));
//		m_sessPrx = TianShanIce::SRM::SessionAdminPrx::checkedCast(m_Communicator->stringToProxy(std::string(SERVICE_NAME_SessionManager ":") + endpoint));
	}
	catch( const ::Ice::Exception &ex)
	{
		_stprintf(szMsg,_T("%s%s"),_T("CheckedCast  Ice exception with error:"), 
					   ex.ice_name().c_str());
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}
	catch(...)
	{
		_stprintf(szMsg,_T("%s%s"),_T("Unknown exception got when parsing!"),endpoint.c_str());
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}

	if(!m_adminPrx)
	{
		_stprintf(szMsg,_T("%s%s"),_T("Failed to connect to PathAdminPrx,PathAdminPrx:!"),endpoint.c_str());
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}
	if(!m_bizPrx)
	{
		_stprintf(szMsg,_T("%s%s"),_T("Failed to connect to BusinessAdmin,BusinessAdmin:!"),endpoint.c_str());
		ATLTRACE(szMsg);
		MessageBox(szMsg);
		return FALSE;
	}
/*
	if(!m_sessPrx)
	{
		_stprintf(szMsg,_T("%s%s"),_T("Failed to connect to SessionAdmin,SessionAdmin:%s!"),endpoint.c_str());
		ATLTRACE(szMsg);
		return FALSE;
	}
*/
	if (  m_bizPrx & m_adminPrx)
	{
		InitTreeCtrl();
	}
	return TRUE;
}

STDMETHODIMP CAdminCtrl::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IAdminCtrl,
	};
	for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++)
	{
		if (::InlineIsEqualGUID(*arr[i], riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CAdminCtrl::SetPathEndPoint(BSTR bstrIpAddress, BSTR bstrPort)
{
	USES_CONVERSION;
	std::string strEndPoint;
	
	strEndPoint  ="tcp -h ";
	strEndPoint += OLE2T(bstrIpAddress);
	strEndPoint += " -p ";
	strEndPoint += OLE2T(bstrPort);
	ConnectServe(strEndPoint,"test");
	BOOL b = FALSE;
	OnSize(0,0,0,b);
	return S_OK;
}