// DlgTabViewCtrl.cpp: implementation of the CDlgTabViewCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AttribeDialog.h"
#include "DlgTabViewCtrl.h"
#include "ColorBar.h"
#include "ColorBar_i.c"
#include "IBMSBaseWnd.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgTabViewCtrl::CDlgTabViewCtrl()
{

}

CDlgTabViewCtrl::~CDlgTabViewCtrl()
{

}

BOOL CDlgTabViewCtrl::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

void CDlgTabViewCtrl::AddAttrsViewTab( LPCTSTR szTabName,LPCTSTR szParentTabName,BOOL bListCtrlMode)
{
	// Moduless Dialog
	CAttribeDialog*	pAttrsDlg = new CAttribeDialog(bListCtrlMode,szTabName,szParentTabName);
	pAttrsDlg->Create( *this,rcDefault,WS_CHILD|WS_HSCROLL|WS_VSCROLL|WS_BORDER|WS_CLIPCHILDREN );
	AddTab( szTabName, *pAttrsDlg, TRUE, DLGTABWINDOW_VAR, (LPARAM) pAttrsDlg );
}

void CDlgTabViewCtrl::AddTestViewTab(LPCTSTR szTabName)
{
	// for the test
	CIBMSBaseWnd * m_pContainWin = NULL; // 控件的容器
	CComPtr<IColorBarControl>  m_pColorBarInterface; //控件的接口

	try
	{
		AtlAxWinInit(); 
		
		CComPtr<IUnknown>   pContainer ;
		CComPtr<IUnknown>   pControl ;
		CComPtr<IOleObject> spObj;
		
		SIZEL sz;
		COLORREF crColor,crLine;
		double dStartPos = 0.0;
		double dEndPos = 0.0;
		double dShowStartPos = 0.0;
		double dShowEndPos = 0.0;
		crLine = RGB(0,0,0);

		RECT rcClient;
		GetClientRect(&rcClient);
		
		RECT rc1;
		rc1.left = rcClient.left;
		rc1.top = rcClient.top;
		rc1.right = rc1.left + 100;
		rc1.bottom = rcClient.top + 50;

		CComBSTR bstrName(L"焦点访谈");

		CComBSTR bstr(L"ColorBar.ColorBarControl");

		CLSID   theclsid;
		HRESULT hr = ::CLSIDFromProgID(bstr.m_str,&theclsid);
		USES_CONVERSION;
		
		LPOLESTR lpolestr;
		StringFromCLSID(theclsid,&lpolestr);

		
		m_pContainWin = new CIBMSBaseWnd();
		m_pContainWin->Create(*this,rc1, _T(""),
												 WS_CHILD | WS_VISIBLE | 
												 WS_CLIPSIBLINGS | 
												 WS_CLIPCHILDREN |
												 WS_TABSTOP,
       											 0);
				
	    m_pContainWin->IBMSCreateControlEx2(lpolestr,m_pContainWin->m_hWnd,NULL,&pContainer,&pControl);
		
		if(SUCCEEDED( m_pContainWin->QueryControl(&spObj) ))
		{
			spObj->GetExtent(DVASPECT_CONTENT,&sz);    
			AtlHiMetricToPixel(&sz,&sz);
			m_pContainWin->SetWindowPos(0,rc1.left,rc1.top,sz.cx,sz.cy,SWP_NOZORDER);
			spObj->SetExtent(DVASPECT_CONTENT,&sz);
		}
		m_pContainWin->SetWindowPos(HWND_TOP,rc1.left,rc1.top,rc1.right-rc1.left,rc1.bottom-rc1.top,SWP_SHOWWINDOW|SWP_NOZORDER);
		pControl->QueryInterface(&m_pColorBarInterface);
			

		
		dShowStartPos = 0.0;
		dShowEndPos   = 10.0;
		
		crColor     = RGB(255,0,0);
		dStartPos   = 0.0;
		dEndPos     = 3.0;
		m_pColorBarInterface->FillColor(crColor,dStartPos,dEndPos,bstrName);
		
		crColor     = RGB(0,255,0);
		dStartPos   = 3.0;
		dEndPos     = 6.0;
		bstrName.Detach();
		bstrName.Attach(L"东方时空");
		m_pColorBarInterface->FillColor(crColor,dStartPos,dEndPos,bstrName);

		crColor     = RGB(0,0,255);
		dStartPos   = 6.0;
		dEndPos     = 10.0;
		bstrName.Detach();
		bstrName.Attach(L"非常6+1");
		m_pColorBarInterface->FillColor(crColor,dStartPos,dEndPos,bstrName);

		m_pColorBarInterface->ShowRange(dShowStartPos,dShowEndPos);
		
		int iCursorId;
		for ( double i =0; i < 10; i ++)
		{
			crLine += 1000; // 颜色递增500,先暂时这么定
			m_pColorBarInterface->CreateCursor(crLine,&iCursorId);
			m_pColorBarInterface->DrawLine(iCursorId,i);
		}
		
		AddTab( szTabName, *m_pContainWin, FALSE, DLGTABWINDOW_LIST, (LPARAM)m_pContainWin );

		
		pContainer.Release();
		pControl.Release();
		spObj.Release();
		m_pColorBarInterface.Release();
		CoTaskMemFree(lpolestr);
	}
	catch(...)
	{
	}
	return ;
}


