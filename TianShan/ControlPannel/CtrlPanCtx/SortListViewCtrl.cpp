// MyListViewCtrl.cpp: implementation of the CMyListViewCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "SortClass.h"
#include "SortListViewCtrl.h"
//#include "ItemDialog.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool IsNumber( LPCTSTR pszText )
{
	ASSERT_VALID_STRING( pszText );

	for( int i = 0; i < lstrlen( pszText ); i++ )
		if( !_istdigit( pszText[ i ] ) )
			return false;

	return true;
}


int NumberCompare( LPCTSTR pszNumber1, LPCTSTR pszNumber2 )
{
	ASSERT_VALID_STRING( pszNumber1 );
	ASSERT_VALID_STRING( pszNumber2 );

	const int iNumber1 = atoi( pszNumber1 );
	const int iNumber2 = atoi( pszNumber2 );

	if( iNumber1 < iNumber2 )
		return -1;
	
	if( iNumber1 > iNumber2 )
		return 1;

	return 0;
}


bool IsDate( LPCTSTR pszText )
{
	ASSERT_VALID_STRING( pszText );

	// format should be 99/99/9999.

	if( lstrlen( pszText ) != 10 )
		return false;

	return _istdigit( pszText[ 0 ] )
		&& _istdigit( pszText[ 1 ] )
		&& pszText[ 2 ] == _T('/')
		&& _istdigit( pszText[ 3 ] )
		&& _istdigit( pszText[ 4 ] )
		&& pszText[ 5 ] == _T('/')
		&& _istdigit( pszText[ 6 ] )
		&& _istdigit( pszText[ 7 ] )
		&& _istdigit( pszText[ 8 ] )
		&& _istdigit( pszText[ 9 ] );
}


int DateCompare( const CString& strDate1, const CString& strDate2 )
{
	const int iYear1 = atoi( strDate1.Mid( 6, 4 ) );
	const int iYear2 = atoi( strDate2.Mid( 6, 4 ) );

	if( iYear1 < iYear2 )
		return -1;

	if( iYear1 > iYear2 )
		return 1;

	const int iMonth1 = atoi( strDate1.Mid( 3, 2 ) );
	const int iMonth2 = atoi( strDate2.Mid( 3, 2 ) );

	if( iMonth1 < iMonth2 )
		return -1;

	if( iMonth1 > iMonth2 )
		return 1;

	const int iDay1 = atoi( strDate1.Mid( 0, 2 ) );
	const int iDay2 = atoi( strDate2.Mid( 0, 2 ) );

	if( iDay1 < iDay2 )
		return -1;

	if( iDay1 > iDay2 )
		return 1;

	return 0;
}

CSortListViewCtrl::CSortListViewCtrl(int iMode,LPCTSTR szTabName)
{
	m_iNumColumns = 4;
	m_iMode = iMode;
	memset(m_strTabName,0,sizeof(m_strTabName));
	memset(m_strText,0,sizeof(m_strText));
	sprintf(m_strTabName,"%s",szTabName);
	if ( iMode == 0)
	{
		m_iAttribeCount = -1;
		m_iSortColumn  = -1 ;
		m_bSortAscending = TRUE ;
	}
}

CSortListViewCtrl::~CSortListViewCtrl()
{

}

BOOL CSortListViewCtrl::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT CSortListViewCtrl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	

	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
	if ( m_iMode == 0) // 可Sort的ListViewCtrl
	{
		/* below is the old style 
		if (  _stricmp(m_strTabName,"Content") == 0 )  // ContentStore for the test
		{

		}
		else
		{
		
			TCHAR szTabName[XMLDATA_LEN]={0};
			TCHAR szDllName[XMLDATA_LEN] ={0};
		
		// Get Grid Fun's name 
	#if defined _UNICODE || defined UNICODE
			MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 gXMLFileData.strFuncDso.c_str(),  // address of string to map
						 strlen(gXMLFileData.strFuncDso.c_str()),      // number of bytes in string
						 szTabName,       // address of wide-character buffer
						 XMLDATA_LEN);             // size of buffer);

	#else
			sprintf(szTabName,"%s",gXMLFileData.strFuncDso.c_str());
	#endif
			if  ( _tcsstr(szTabName,_T("%") )!= NULL )
			{
					TCHAR szTemp[XMLDATA_LEN]={0};
					ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
					memset(szTabName,0,sizeof(szTabName));
					_stprintf(szTabName,_T("%s"),szTemp);
			}
			_stprintf(szDllName,_T("%s"),szTabName);


			// Get Data from dll,then add datas to UI Desktop
	
	//		typedef ITEMDATA ** (CALLBACK *GetData_Proc)(ATTRIBSDATA **pAttribsData, int *iAttCount,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
	//		typedef ITEMDATA ** (CALLBACK *GetData_Proc)(GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
	//		typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
			typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc);
	
			GetData_Proc hGetData = NULL;
			// modify hLib to global var
		//	HINSTANCE hLib;
		//	hLib = LoadLibrary(szDllName);
		//  if ( hLib)
			if ( !m_hLib )
			{
				m_hLib = LoadLibrary(szDllName);
			}

			if ( m_hLib)
			{
				//hGetData = (GetData_Proc)GetProcAddress(hLib,gXMLFileData.strGridFunc.c_str());
				hGetData = (GetData_Proc)GetProcAddress(m_hLib,gXMLFileData.strGridFunc.c_str());
				if ( hGetData)
				{
				
					int iRow,iCol;
					ITEMDATA **pCellsData = NULL;
					ITEMDATA * pColumnsName = NULL;
	//				int iAttribsCount; ATTRIBSDATA *pAttribsData = NULL; // this is the old operation

	//				GRIDDATAARRAY2 m_AttribesDatas; 
					m_AttribesDatas.clear();
					
	//				pCellsData = (*hGetData)(&pAttribsData,&iAttribsCount,&pColumnsName,&iRow,&iCol,pCellsData);
					pCellsData = (*hGetData)("*",m_strTabName,m_AttribesDatas,&pColumnsName,&iRow,&iCol,pCellsData,NULL);
					m_iAttribeCount = m_AttribesDatas.size();
					if ( iCol > 0 )
					{
						m_iNumColumns = iCol ;
					
						int i = 0;
						for ( i = 0; i < iCol; i ++)
						{
							AddColumn((LPCTSTR)(pColumnsName[i].szItemData),i);
						}

						for(i=0;i<iRow;i++)
						{
							for(int j=0;j<iCol;j++)
							{
								AddItem(i,j,(LPCTSTR)((pCellsData[i][j]).szItemData));
							}
						}

						// free the columns's memory
						for (  i = 0; i < iCol; i ++)
						{
							free(pColumnsName[i].szItemData);
						}
						delete  pColumnsName;

						// free the cells's memory
						for( i=0;i<iRow;i++)
						{
							for( int j=0;j<iCol;j++)
							{
								free((pCellsData[i][j]).szItemData);
							}
						}
						for( i=0;i<iRow;i++)
						{
							delete [] pCellsData[i]; //释放列
						}
						delete pCellsData; //释放行
					}
				
				}
			//	FreeLibrary(hLib); // modify hLib to global var
			}
		}
		*/
	}
	else if ( m_iMode == 1) // Event ListViewCtrl
	{
		/*
		m_iNumColumns = 4;
		
		// below code is for the test
		int nMask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		int nFmt = LVCFMT_LEFT;
		int nSubItem = -1;
		AddColumn(_T("11"), 0,nSubItem,nMask,nFmt);
		AddColumn(_T("DataTime"),1,nSubItem,nMask,nFmt);
		AddColumn(_T("Category"),2,nSubItem,nMask,nFmt);
		AddColumn(_T("Event"), 3,nSubItem,nMask,nFmt);        
		*/
	}
	return lRet;
}

LRESULT CSortListViewCtrl::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	/* below is the old code 
	CRect rcClient;
	GetClientRect(&rcClient);
	int icx = rcClient.Width() / m_iNumColumns;
	if ( m_iMode == 0) // Event ListViewCtrl)
	{
		if (  _stricmp(m_strTabName,"Content") == 0 )  // ContentStore // for the test
		{
		}
		else
		{
			for ( int i =0; i < m_iNumColumns; i ++ )
			{
				SetColumnWidth(i,icx);
			}
		}
	}
	else if ( m_iMode == 1)
	{
	}
	*/

	bHandled = FALSE;
	return S_OK;
}

HRESULT CSortListViewCtrl::OnMeasureItem(UINT , WPARAM wParam, LPARAM lParam,
                                       BOOL& bHandled)
{
    LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT)lParam;
    if( wParam == 0 && lpMeasureItem->CtlType == ODT_MENU )
    {
        int nStrID     = (int)lpMeasureItem->itemData;

        CClientDC dc(*this);
		CString str;
		
		str = "Storage Links";
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



LRESULT CSortListViewCtrl::OnDrawItem(UINT , WPARAM wParam, LPARAM lParam,
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
//LRESULT CSortListViewCtrl::OnColumnClick( LPNMHDR pnmh )
LRESULT CSortListViewCtrl::OnColumnClick(int idCtrl,LPNMHDR pnmh, BOOL & bHandled)
{
	if ( m_iMode == 0)
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pnmh;
		const int iColumn = pNMListView->iSubItem;
		HWND hWnd = GetHeader().m_hWnd;
		SortColumn(iColumn, iColumn == m_iSortColumn ? !m_bSortAscending : TRUE  );
	}
	return S_OK;
}

//LRESULT CSortListViewCtrl::OnDblclkList( LPNMHDR pnmh)
LRESULT CSortListViewCtrl::OnDblclkList(int idCtrl,LPNMHDR pnmh, BOOL & bHandled)
{
	// 注意每个Page的Key值位置不同，即每个Page取的列表的Item Index不同.
	if ( m_iMode == 0)
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pnmh;
		const int iItem = pNMListView->iItem;
//		int iSubItem = pNMListView->iSubItem;
		int iSubItem;
		TCHAR szText[XMLDATA_LEN] ={0};
		int iCount = GetItemCount();
		if ( iCount > 0  && m_iAttribeCount > 0 )
		{
			if ( _stricmp(m_strTabName,"Streamer1") == 0 ) // Streamer Tab
			{
				CMenu popMenu,SubMenu,MenuTrackPopup;
				POINT pt; 
				GetCursorPos(&pt);	
				m_strMenuString.RemoveAll();
				popMenu.LoadMenu(IDR_STREAMERMENU);

				MenuTrackPopup = CreatePopupMenu();
				SubMenu = popMenu.GetSubMenu(0);

				int nItemCount = SubMenu.GetMenuItemCount();
				for( int i = 0; i < nItemCount; i++ )
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
				iSubItem = 0;
				GetItemText(iItem,iSubItem,szText,XMLDATA_LEN);
				sprintf(m_strText,"%s",szText);
			}
			else
			{
				iSubItem = 0;
				GetItemText(iItem,iSubItem,szText,XMLDATA_LEN);
				CItemDialog m_dlg(szText,m_strTabName);
				m_dlg.DoModal();
			}
		}
	}
	return S_OK;
}

LRESULT CSortListViewCtrl::OnVerb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	char szText[XMLDATA_LEN] ={0};
	switch( wID )
	{
		case ID_STREAMERLINK:
			sprintf(szText,"%s","Streamer1");
			break;
		case ID_STORAGELINK:
			sprintf(szText,"%s","Streamer2");
			break;
		default:
			sprintf(szText,"%s","Streamer1");
			break;
	}
	CItemDialog m_dlg(m_strText,szText);
			m_dlg.DoModal();
	return 0L;
}

bool  CSortListViewCtrl::IsNumberItem(const int iSubItem) const
{
	bool bIsNumberItem = false;
	int max =GetItemCount();	
	if ( max > 0 )
	{
		TCHAR szTmp[150]={0};
		GetItemText(0,iSubItem,szTmp,150);
		bIsNumberItem = IsNumber(szTmp);
	}
	return bIsNumberItem;
}

void CSortListViewCtrl::SortColumn( int iSubItem,  BOOL bAscending)
{
	if ( m_iMode == 0)
	{
		m_iSortColumn = iSubItem;
		m_bSortAscending = bAscending;

		bool bIsNumber = IsNumberItem(iSubItem);
		CSortClass csc(this,iSubItem,bIsNumber);
		csc.Sort( m_bSortAscending );
		m_pctlHeader.RemoveAllSortImages();
		m_pctlHeader.SetHeadItem(iSubItem,m_bSortAscending);
	}
}

void CSortListViewCtrl::AttachHeader()
{
	HWND hWnd = GetHeader().m_hWnd;
	m_pctlHeader.SubclassWindow(hWnd);
}

BOOL CSortListViewCtrl::SubclassWindow(HWND hWnd)
{
	BOOL bRet = CWindowImpl<CSortListViewCtrl, CListViewCtrl>::SubclassWindow(hWnd);
	return bRet;
}

void  CSortListViewCtrl::SetAttribeCount(const int iAttribeCount)
{
	m_iAttribeCount = iAttribeCount;
}




