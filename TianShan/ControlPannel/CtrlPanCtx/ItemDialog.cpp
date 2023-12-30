// ItemDialog.cpp: implementation of the CItemDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemDialog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemDialog::CItemDialog(LPCTSTR szWinName,LPCTSTR szParentTabName,BOOL bListCtrlMode)
{
	
	memset(m_ItemWinName,0,sizeof(m_ItemWinName));
	memset(m_strParentTabName,0,sizeof(m_strParentTabName));
	sprintf(m_ItemWinName,"%s",szWinName);
	sprintf(m_strParentTabName,"%s",szParentTabName);
	m_bListCtrlMode = bListCtrlMode;
}

CItemDialog::~CItemDialog()
{

}

LRESULT CItemDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	
	m_ItemCtrl.Create( m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE );
	m_ItemCtrl.AddAttrsViewTab(m_ItemWinName,m_strParentTabName,m_bListCtrlMode);
//	m_ItemCtrl.AddTestViewTab(_T("Graph"));
	bHandled = FALSE;
	CenterWindow();
	return S_OK;
}

LRESULT CItemDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	m_ItemCtrl.MoveWindow(&rcClient);
	bHandled = FALSE;
	return S_OK;
}

LRESULT CItemDialog::OnClose(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	EndDialog(0);
	bHandled = FALSE;
	return 0;
}
