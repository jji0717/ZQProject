// ItemDialog.h: interface for the CItemDialog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZQITEMDIALOG_H
#define __ZQITEMDIALOG_H
#include "resource.h"
#include "DlgTabViewCtrl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CItemDialog : public CDialogImpl<CItemDialog> 
{
public:
	CItemDialog(LPCTSTR szWinName=_T(""),LPCTSTR szParentTabName=_T(""),BOOL bListCtrlMode=TRUE);
	virtual ~CItemDialog();

	enum { IDD = IDD_ITEMDIALOG };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP(CItemDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL& bHandled);

private:
	CDlgTabViewCtrl       m_ItemCtrl;
	char				  m_ItemWinName[256];
	char                  m_strParentTabName[256];
	BOOL                  m_bListCtrlMode;
};

#endif // !defined(AFX_ITEMDIALOG_H__55EF0D39_CD09_41A8_9E03_88A0B50F77AF__INCLUDED_)
