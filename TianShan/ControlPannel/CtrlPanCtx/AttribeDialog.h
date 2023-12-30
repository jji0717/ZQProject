// AttribeDialog.h: interface for the CAttribeDialog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZQATTRIBEDIALOG_H
#define __ZQATTRIBEDIALOG_H

#include "resource.h"
#include <ZQEventsCtrl.h>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAttribeDialog : public   CDialogImpl<CAttribeDialog> 
{
public:
	CAttribeDialog(BOOL bListCtrlMode=TRUE,LPCTSTR szTabName=_T(""),LPCTSTR szParentTabName=_T(""));
	virtual ~CAttribeDialog();

	enum { IDD = IDD_ATTRSDIALOG };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP(CAttribeDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);

private:
	void InitControl();
private:
	string m_strWindowName;
	char m_strParentTabName[ITEMLEN];
	int m_iAttrsNum;
	CStatic  *m_pStaic;
	CEdit    *m_pEdit;
	CListViewCtrl *m_pListCtrl;
	BOOL  m_bListCtrlMode;
};
//extern GRIDDATAARRAY2  m_AttribesDatas;

#endif // !defined(AFX_ATTRIBEDIALOG_H__6CF3D09D_845F_45E8_A890_C4FFD74C5674__INCLUDED_)
