#include "stdafx.h"
#include "SNMPVarDialog.h"
#include "SortListViewCtrl.h"
#include "EventsDialog.h"
//#include "DataDialog.h"
#include "MyTabViewCtrl.h"
//////////////////////////////////////////////////////////////////////
// CMyTabViewCtrl Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDataDialog *m_pDataView = NULL;

CMyTabViewCtrl::CMyTabViewCtrl()
{

}

CMyTabViewCtrl::~CMyTabViewCtrl()
{

}

BOOL CMyTabViewCtrl::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

void CMyTabViewCtrl::AddSNMPVarWinTab( LPCTSTR szTabName )
{
	// Moduless Dialog
	CSNMPVarDialog*	pSnmpView = new CSNMPVarDialog();
//	pSnmpView->Create( *this,rcDefault,WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|WS_BORDER|WS_CLIPCHILDREN );
	pSnmpView->Create( *this,rcDefault,WS_CHILD|WS_HSCROLL|WS_VSCROLL|WS_BORDER|WS_CLIPCHILDREN );
//	DWORD dwStyle =pSnmpView->GetWndStyle();
	AddTab( szTabName, *pSnmpView, TRUE, TABWINDOW_VAR, (LPARAM) pSnmpView );
}

void CMyTabViewCtrl::AddServiceData(LPCTSTR szTabName,BOOL bShowAboveCtrl)
{
	// below is the old code 20070720 
	/*
	int iMode = 0;
	CSortListViewCtrl *pListCtrl = new CSortListViewCtrl(iMode,szTabName); // My Design List Ctrl
	DWORD dwStyle;
	dwStyle =  LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
	pListCtrl->Create( *this,rcDefault, NULL,
                              WS_CHILD|WS_VSCROLL|LVS_REPORT|WS_HSCROLL|
                              LVS_SHOWSELALWAYS,WS_EX_STATICEDGE   );//,WS_EX_CLIENTEDGE  );
	pListCtrl->SetExtendedListViewStyle(dwStyle);
	pListCtrl->AttachHeader();
	
	TCHAR *pos  = _tcsrchr(szTabName, _T('1'));
	if ( pos )
	{
		TCHAR szTemp[ITEMLEN]={0};
		int iLen =_tcsclen(szTabName); 
		_tcsncpy(szTemp,szTabName,iLen-1);
		AddTab( szTemp, *pListCtrl, TRUE, TABWINDOW_DATA1, (LPARAM)pListCtrl );
	}
	else
	{
		AddTab( szTabName, *pListCtrl, TRUE, TABWINDOW_DATA1, (LPARAM)pListCtrl );
	}
	*/
	
	/*
	if (  (_tcsicmp(szTabName,_T("PlayList")) == 0 )  ) // Playlist
	{
		m_pDataView = new CDataDialog(szTabName,bShowAboveCtrl);
		m_pDataView->Create( *this,rcDefault,WS_CHILD|WS_HSCROLL|WS_VSCROLL|WS_BORDER|WS_CLIPCHILDREN );
	}
	else
	*/
	
	CDataDialog *pDataView = new CDataDialog(szTabName,bShowAboveCtrl);
	pDataView->Create( *this,rcDefault,WS_CHILD|WS_HSCROLL|WS_VSCROLL|WS_BORDER|WS_CLIPCHILDREN );
	
	TCHAR *pos  = _tcsrchr(szTabName, _T('1'));
	if ( pos )
	{
		TCHAR szTemp[ITEMLEN]={0};
		int iLen =_tcsclen(szTabName); 
		_tcsncpy(szTemp,szTabName,iLen-1);
		AddTab( szTemp, *pDataView, TRUE, TABWINDOW_DATA1, (LPARAM)pDataView );
	}
	else
	{
		AddTab( szTabName, *pDataView, TRUE, TABWINDOW_DATA1, (LPARAM)pDataView );
	}
}

void CMyTabViewCtrl::AddEventListViewTab( LPCTSTR szTabName )
{
	// Moduless Dialog
	CEventsDialog *	pEventView = new CEventsDialog();
	pEventView->Create( *this,rcDefault,WS_CHILD|WS_HSCROLL|WS_VSCROLL|WS_BORDER|WS_CLIPCHILDREN );
	AddTab( szTabName, *pEventView, TRUE, TABWINDOW_EVENT, (LPARAM) pEventView );
}

void CMyTabViewCtrl::AddServiceData1(LPCTSTR szTabName)
{
//	CListViewCtrl *pListCtrl = new CListViewCtrl(); // My Design List Ctrl
	int iMode = 1;
	CSortListViewCtrl *pListCtrl = new CSortListViewCtrl(iMode); // My Design List Ctrl
	DWORD dwStyle;
	dwStyle =  LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;
	pListCtrl->Create( *this,rcDefault, NULL,
                              WS_CHILD|WS_VSCROLL|LVS_REPORT|WS_HSCROLL|
                              LVS_SHOWSELALWAYS,WS_EX_STATICEDGE   );//,WS_EX_CLIENTEDGE  );
	pListCtrl->SetExtendedListViewStyle(dwStyle);
	AddTab( szTabName, *pListCtrl, TRUE, TABWINDOW_DATA2, (LPARAM)pListCtrl );
}
