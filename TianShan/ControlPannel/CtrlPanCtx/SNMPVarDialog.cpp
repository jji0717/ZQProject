// SNMPVarDialog.cpp: implementation of the CSNMPVarDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SNMPVarDialog.h"
#include <snmp.h> 


//extern XMLFILEDATA  gXMLFileData;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSNMPVarDialog::CSNMPVarDialog()
{
	m_bSNMPOK = TRUE;
	m_bFirst = FALSE;
	m_pSNMPVarData = NULL;
	m_iVarNum = 20;
	m_pStaic = NULL;
	m_pEdit = NULL;
//	m_pButton = NULL;
	m_hSession = NULL;
	m_manRet = ZQMAN_SESSION_ERROR;
	m_RecectVarName ="";
}

CSNMPVarDialog::~CSNMPVarDialog()
{
	for ( int i =0; i < m_iVarNum; i ++ )
	{
	    if ( m_pStaic[i].IsWindow())
		{
			m_pStaic[i].DestroyWindow();
		}
		if ( m_pEdit[i].IsWindow())
		{
			m_pEdit[i].DestroyWindow();
		}
		free(m_pSNMPVarData[i].ResultValue);
		free(m_pSNMPVarData[i].ResultVarOID);
		free(m_pSNMPVarData[i].ResultVarType);
		free(m_pSNMPVarData[i].ResultReadWrite);
	}
	delete m_pSNMPVarData;
	m_pSNMPVarData = NULL;

	delete [] m_pStaic;
	m_pStaic = NULL;
	delete [] m_pEdit;
	m_pEdit = NULL;

}

LRESULT CSNMPVarDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if ( m_manRet == ZQMAN_SUCCESS)
	{
		CString strTemp;
		strTemp = m_RecectVarName + _T("变量修改成功");
		MessageBox(strTemp,_T("提示"),MB_OK);
		m_manRet = ZQMAN_SESSION_ERROR;
	}
	bHandled = FALSE;
	return S_OK;
}

LRESULT CSNMPVarDialog::OnChangeEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if ( m_bFirst)
	{
		/*
		if ( wNotifyCode == EN_CHANGE)
		{
			TCHAR szText[150];
			::GetWindowText(hWndCtl,szText,150);
		//	MessageBox(szText);
			bHandled = FALSE;
		}
		if ( wNotifyCode == EN_UPDATE)
		{
			MessageBox("UpDATE");
		}
		*/
		if ( wNotifyCode == EN_KILLFOCUS )
		{
			if ( m_hSession )
			{
				TCHAR szVarValue[VARDATALEN]={0};
				::GetWindowText(hWndCtl,szVarValue,VARDATALEN);
				ZQMANSTATUS manRet =ZQMAN_SUCCESS;
				TCHAR szVarName[EDITMAXNUM]={0};
				int iIndex = wID - IDC_MYEDIT;
				m_pStaic[iIndex].GetWindowText(szVarName,EDITMAXNUM);
				CString strType = m_pSNMPVarData[iIndex].ResultVarType;
				INT  wType;
				if ( strType.CompareNoCase(_T("String"))==0)
				{
					wType = ZQMAN_STR;
				}
				else if ( strType.CompareNoCase(_T("Int"))==0)
				{
					wType = ZQMAN_INT;
				}
				else if ( strType.CompareNoCase(_T("Float"))==0)
				{
					wType = ZQMAN_FLOAT;
				}
				m_manRet =SNMPOperSetVarValue(m_hSession,szVarName,(DWORD)(void*)szVarValue,wType);
				if ( m_manRet == ZQMAN_SUCCESS)
				{
					m_RecectVarName = szVarName;
				}
			}
		//	MessageBox(szText);
		}
		bHandled = FALSE;

	}
	return 0;
}

LRESULT CSNMPVarDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	InitControl();
	bHandled = FALSE;
	return S_OK;
}

void  CSNMPVarDialog::InitControl()
{
	TCHAR szServiceName[XMLDATA_LEN]={0};
	TCHAR szOID[XMLDATA_LEN]={0};
	TCHAR szIP[XMLNAME_LEN]={0};
	
	// Get Serivce's name
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 gXMLFileData.strServiceName.c_str(),        // address of string to map
						 strlen(gXMLFileData.strServiceName.c_str()),      // number of bytes in string
						 szServiceName,       // address of wide-character buffer
						 XMLDATA_LEN);             // size of buffer);

#else
	sprintf(szServiceName,"%s",gXMLFileData.strServiceName.c_str());
#endif

	// Get Service's oid
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 gXMLFileData.strServiceOID.c_str(),        // address of string to map
						 strlen(gXMLFileData.strServiceOID.c_str()),      // number of bytes in string
						 szOID,       // address of wide-character buffer
						 XMLDATA_LEN);             // size of buffer);
#else
	sprintf(szOID,"%s",gXMLFileData.strServiceOID.c_str());
#endif

	// Get SNMPIP
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 gXMLFileData.strSnmpIp.c_str(),        // address of string to map
						 strlen(gXMLFileData.strSnmpIp.c_str()),      // number of bytes in string
						 szIP,       // address of wide-character buffer
						 XMLNAME_LEN);             // size of buffer);

#else
	sprintf(szIP,"%s",gXMLFileData.strSnmpIp.c_str());
#endif

	if (   ( *szServiceName !='\0' )  && ( *szOID !='\0' )  && ( *szIP !='\0' ))
	{
		ZQMANSTATUS manRet =ZQMAN_SESSION_ERROR;
		TCHAR *pszCommunity =_T("public");
		
		manRet =SNMPOperOpenSession(szIP,pszCommunity,&m_hSession);
		if ( manRet == ZQMAN_SUCCESS )
		{
			int iCount = 0;
			manRet = SNMPOperGetAllVarValue(m_hSession,szOID,&iCount,&m_pSNMPVarData);
			if ( manRet == ZQMAN_SUCCESS )
			{
				m_iVarNum = iCount;

				RECT rcClientRect;
				GetClientRect(&rcClientRect);

				CDCHandle hdc =GetDC();
				int iWidth =  ( rcClientRect.right - rcClientRect.left ) / 4 ;
				int iRowNum ;
				if ( m_iVarNum % 2 == 0 )
				{
					iRowNum = m_iVarNum / 2;
				}
				else
				{
					iRowNum = ( m_iVarNum + 1) / 2;
				}
				int iHight =  ( ( rcClientRect.bottom - rcClientRect.top ) - ( iRowNum + 1 )* 8 ) / iRowNum ;
				
				SIZE sizeValue;
				TCHAR szTemp[] = _T("ZQServiceTest/DatabaseFolder.path");
				hdc.GetTextExtent(szTemp,lstrlen(szTemp),&sizeValue);
				LONG iStaticWidth = sizeValue.cx;
				LONG iStaticHigth = sizeValue.cy;
					
				
				
				CString strVarData,strReadWrite;
				RECT *prc1,*prc2;
				m_pStaic = new CStatic[m_iVarNum];
//				m_pEdit  = new CEdit[m_iVarNum];
				m_pEdit  = new CEditXP[m_iVarNum];
				prc1 = new RECT[m_iVarNum];
				prc2 = new RECT[m_iVarNum];

				DWORD dwStyle,dwExStyle;
				int iLen;
				UINT nEdit ;
					
				for ( int i = 0; i < m_iVarNum; i ++ )
				{
					if ( i %2 == 0 ) // 前面的两个Control
					{
						prc1[i].left  = rcClientRect.left + 2 ;
						prc1[i].right = prc1[i].left + iStaticWidth;
						if ( i == 0 )
						{
							prc1[i].top  = rcClientRect.top + 5 ;
						}
						else 
						{
							prc1[i].top  = prc1[i-1].bottom + 8 ; 
						}
			//			prc1[i].bottom = prc1[i].top + iHight;
						prc1[i].bottom = prc1[i].top + iStaticHigth;

						prc2[i].left   = prc1[i].right + 2;
						prc2[i].top    = prc1[i].top;
						prc2[i].bottom = prc1[i].bottom + 6;
						prc2[i].right  = prc2[i].left + iWidth;
					}
					else // 后面的两个Control
					{
						prc1[i].left   = prc2[i-1].right + 5;
						prc1[i].right  = prc1[i].left + iStaticWidth;
						prc1[i].top    = prc1[i-1].top;
						prc1[i].bottom = prc1[i-1].bottom;

						prc2[i].left = prc1[i].right + 2;
						prc2[i].top = prc1[i].top;
						prc2[i].bottom = prc1[i].bottom + 6;
						prc2[i].right = prc2[i].left + iWidth;
					}
					
					
					strVarData = m_pSNMPVarData[i].ResultValue;
					strReadWrite = m_pSNMPVarData[i].ResultReadWrite;
					iLen =strVarData.Find('@');
															
					dwStyle = WS_CHILD | WS_VISIBLE;
					dwExStyle = 0;// WS_EX_STATICEDGE;//WS_EX_CLIENTEDGE;//WS_EX_OVERLAPPEDWINDOW ;
					m_pStaic[i].Create(*this,&prc1[i],_T(""),dwStyle,dwExStyle);
					m_pStaic[i].SetWindowText(strVarData.Mid(0,iLen));

					dwStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_WANTRETURN  | WS_BORDER |WS_TABSTOP | ES_AUTOHSCROLL    ;
					dwExStyle = WS_EX_LEFT | WS_EX_OVERLAPPEDWINDOW;

					nEdit = IDC_MYEDIT + i;
					_U_MENUorID menuEdit(nEdit);

//					m_pEdit[i].Create(*this,&prc2[i],_T(""),dwStyle,dwExStyle,menuEdit);
					m_pEdit[i].Create(*this,prc2[i],_T(""),dwStyle,dwExStyle,nEdit);

					m_pEdit[i].SetWindowText(strVarData.Mid(iLen+1));
					if ( strReadWrite.CompareNoCase(_T("READONLY")) == 0 )
					{
						m_pEdit[i].EnableWindow(FALSE);
					}
					else
					{
						m_pEdit[i].EnableWindow(TRUE);
					}
					m_pEdit[i].SetLimitText(150);
					m_pStaic[i].SetParent(*this);
					m_pEdit[i].SetParent(*this);
					m_pStaic[i].MoveWindow(&prc1[i]);
					m_pStaic[i].ShowWindow(SW_SHOWNORMAL);
					m_pEdit[i].MoveWindow(&prc2[i]);
					m_pEdit[i].ShowWindow(SW_SHOWNORMAL);
				}

				if ( prc1)
				{
					delete [] prc1;
				}
				prc1 = NULL;

				if ( prc2)
				{
					delete [] prc2;
				}
				prc2 = NULL;
				
				// Create the Button 
				dwStyle = WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON | BS_LEFTTEXT  | WS_BORDER | WS_TABSTOP;
				_U_MENUorID menu(IDOK);
			//	m_pButton = new CButton();
			//	m_pButton = new CXPButton();
			//	m_pButton->Create(*this,&rc3,_T("Apply"),dwStyle,dwExStyle,menu);
				nEdit = IDOK;

//				m_pButton->Create(*this,rc3,_T(""),dwStyle,dwExStyle,nEdit);
//				m_pButton->SetWindowText(_T("Apply"));
//				m_pButton->SetParent(*this);
//				m_pButton->MoveWindow(&rc3);
//				m_pButton->ShowWindow(SW_SHOWNORMAL);


				m_btnOK.SubclassWindow(GetDlgItem(IDOK));
				RECT rc3;
				m_btnOK.GetWindowRect(&rc3);
				int iBtnWidth = rc3.right - rc3.left;
				int iBtnHight = rc3.bottom - rc3.top;

				rc3.right   = rcClientRect.right - 10;
				rc3.bottom  = rcClientRect.bottom - 4;
				rc3.left = rc3.right - iBtnWidth;
				rc3.top  = rc3.bottom - iBtnHight;
				
				m_btnOK.MoveWindow(&rc3);

				ReleaseDC(hdc);
				m_bFirst = TRUE;
				m_bSNMPOK = TRUE;
			}
			else
			{
				m_bSNMPOK = FALSE;
			}
		}
	}
}

LRESULT CSNMPVarDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	if ( m_bSNMPOK )
	{
	
		RECT rcClientRect;
		GetClientRect(&rcClientRect);
		
		CDCHandle hdc =GetDC();
		int iWidth =  ( rcClientRect.right - rcClientRect.left ) / 4 ;

		int iRowNum ;
		if ( m_iVarNum % 2 == 0 )
		{
			iRowNum = m_iVarNum / 2;
		}
		else
		{
			iRowNum = ( m_iVarNum + 1) / 2;
		}
		int iHight =  ( ( rcClientRect.bottom - rcClientRect.top ) - ( iRowNum + 1 )* 8 ) / iRowNum ;
	//	int iHight = ( ( rcClientRect.bottom - rcClientRect.top ) - ( m_iVarNum + 1 )* 8 ) / m_iVarNum ;

		SIZE sizeValue;
		TCHAR szTemp[] = _T("ZQServiceTest/DatabaseFolder.path");
		hdc.GetTextExtent(szTemp,lstrlen(szTemp),&sizeValue);
		int  iStaticWidth = (int)sizeValue.cx;
		int  iStaticHigth = (int)sizeValue.cy;
		TEXTMETRIC  tm ;
		::GetTextMetrics (hdc, &tm) ;

		m_TextWidth = tm.tmAveCharWidth ;
		m_TextHight = tm.tmHeight + tm.tmExternalLeading ;
			
		/*
		// Set vertical scroll bar range and page size
		SCROLLINFO  si;
		si.cbSize = sizeof (si) ;
		si.fMask  = SIF_RANGE | SIF_PAGE ;
		si.nMin   = 0 ;
		si.nMax   =  (rcClientRect.bottom - rcClientRect.top)/m_TextHight ;
		si.nPage  = m_TextHight;
		::SetScrollInfo(m_hWnd, SB_VERT,&si, TRUE);
		
		// Set horizontal scroll bar range and page size
		si.cbSize = sizeof (si) ;
		si.fMask  = SIF_RANGE | SIF_PAGE ;
		si.nMin   = 0 ;
		si.nMax   =  (rcClientRect.right  - rcClientRect.left)/m_TextWidth;
		si.nPage  =  m_TextWidth ;
		::SetScrollInfo(m_hWnd,SB_HORZ, &si, TRUE) ;
		*/

		RECT rc3;
		m_btnOK.GetWindowRect(&rc3);
		int iBtnWidth = rc3.right - rc3.left;
		int iBtnHight = rc3.bottom - rc3.top;

		rc3.right   = rcClientRect.right - 10;
		rc3.bottom  = rcClientRect.bottom - 4;
		rc3.left = rc3.right - iBtnWidth;
		rc3.top  = rc3.bottom - iBtnHight;
		
		RECT *prc1,*prc2;
		prc1 = new RECT[m_iVarNum];
		prc2 = new RECT[m_iVarNum];
		
		for ( int i = 0; i < m_iVarNum; i ++ )
		{
			if ( i %2 == 0 ) // 前面的两个Control
			{
				prc1[i].left  = rcClientRect.left + 2 ;
				prc1[i].right = prc1[i].left + iStaticWidth;
				if ( i == 0 )
				{
					prc1[i].top  = rcClientRect.top + 5 ;
				}
				else 
				{
					prc1[i].top  = prc1[i-1].bottom + 8 ; 
				}
	//			prc1[i].bottom = prc1[i].top + iHight;
				prc1[i].bottom = prc1[i].top + iStaticHigth;

				prc2[i].left   = prc1[i].right + 2;
				prc2[i].top    = prc1[i].top;
				prc2[i].bottom = prc1[i].bottom + 6;
				prc2[i].right  = prc2[i].left + iWidth;
			}
			else // 后面的两个Control
			{
				prc1[i].left   = prc2[i-1].right + 5;
				prc1[i].right  = prc1[i].left + iStaticWidth;
				prc1[i].top    = prc1[i-1].top;
				prc1[i].bottom = prc1[i-1].bottom;

				prc2[i].left = prc1[i].right + 2;
				prc2[i].top = prc1[i].top;
				prc2[i].bottom = prc1[i].bottom + 6;
				prc2[i].right = prc2[i].left + iWidth;
			}
			m_pStaic[i].MoveWindow(&prc1[i]);
			m_pEdit[i].MoveWindow(&prc2[i]);
		}
		if ( prc1)
		{
			delete [] prc1;
		}
		prc1 = NULL;

		if ( prc2)
		{
			delete [] prc2;
		}
		prc2 = NULL;
	//	m_pButton->MoveWindow(&rc3);
		m_btnOK.MoveWindow(&rc3);
			
		ReleaseDC(hdc);
	}
	bHandled = FALSE;
	return S_OK;
}

LRESULT CSNMPVarDialog::OnCreate(UINT uMsg,WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bHandled =FALSE;
	return S_OK;
}

LRESULT CSNMPVarDialog::OnHScroll(UINT uMsg,WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{

	SCROLLINFO  si;
	si.cbSize = sizeof (si) ;
	si.fMask  = SIF_ALL ;

	// Save the position for comparison later on
	::GetScrollInfo(m_hWnd,SB_HORZ, &si) ;
	int iHorzPos = si.nPos ;
	switch (LOWORD (wParam))
	{
	  case SB_LINELEFT:
		   si.nPos -= 1 ;
		   break ;
       
	  case SB_LINERIGHT:
		   si.nPos += 1 ;
		   break ;
       
	  case SB_PAGELEFT:
		   si.nPos -= si.nPage ;
		   break ;
       
	  case SB_PAGERIGHT:
		   si.nPos += si.nPage ;
		   break ;
       
	  case SB_THUMBPOSITION:
		   si.nPos = si.nTrackPos ;
		   break ;
       
	  default :
		   break ;
	}
	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.

	si.fMask = SIF_POS ;
	::SetScrollInfo(m_hWnd,SB_HORZ, &si, TRUE) ;
	::GetScrollInfo(m_hWnd,SB_HORZ, &si) ;
  
   // If the position has changed, scroll the window 

	if (si.nPos != iHorzPos)
	{
		::ScrollWindow(m_hWnd,m_TextWidth * (iHorzPos - si.nPos), 0, 
						 NULL, NULL) ;
		UpdateWindow();
	}
  
	bHandled = FALSE;
	return S_OK;
}

LRESULT CSNMPVarDialog::OnVScroll(UINT uMsg,WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	SCROLLINFO  si;
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    ::GetScrollInfo(m_hWnd,SB_VERT, &si) ;
	int iVertPos = si.nPos ;
	
	switch (LOWORD (wParam))
    {
      case SB_TOP:
           si.nPos = si.nMin ;
           break ;
          
      case SB_BOTTOM:
           si.nPos = si.nMax ;
           break ;
           
      case SB_LINEUP:
           si.nPos -= 1 ;
           break ;
           
      case SB_LINEDOWN:
           si.nPos += 1 ;
           break ;
           
      case SB_PAGEUP:
           si.nPos -= si.nPage ;
           break ;
           
      case SB_PAGEDOWN:
           si.nPos += si.nPage ;
           break ;
           
      case SB_THUMBTRACK:
           si.nPos = si.nTrackPos ;
           break ;
           
      default:
           break ;         
    }
    // Set the position and then retrieve it.  Due to adjustments
    //   by Windows it may not be the same as the value set.

    si.fMask = SIF_POS ;
    ::SetScrollInfo(m_hWnd,SB_VERT, &si, TRUE) ;
    ::GetScrollInfo(m_hWnd,SB_VERT, &si) ;

    if (si.nPos != iVertPos)
    {                    
		::ScrollWindow(m_hWnd,0,m_TextHight * (iVertPos - si.nPos), 
						NULL, NULL) ;
		Invalidate();
        UpdateWindow();
    }
    bHandled = FALSE;
//	OnSize(0,0,0,bHandled);
	return S_OK;
}
