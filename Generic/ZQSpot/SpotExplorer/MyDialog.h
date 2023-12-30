// MyDialog.h: interface for the CMyDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYDIALOG_H__A7B811F0_4437_4BCB_B8F6_F44DC9B63EC1__INCLUDED_)
#define AFX_MYDIALOG_H__A7B811F0_4437_4BCB_B8F6_F44DC9B63EC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

// -----------------------------
// class SpotStatusQuery
// -----------------------------

class CMyDialog : public CDialogImpl<CMyDialog>  
{
public:
	CMyDialog();
	virtual ~CMyDialog();
	
	enum { IDD = IDD_DIALOG1 };

	BEGIN_MSG_MAP(CMyDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		ALT_MSG_MAP(123)	//contained window's messages come here...

	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		// when the dialog box is created, subclass its edit control:
		bHandled = FALSE;
		::SetWindowText(GetDlgItem(IDC_EDIT1), "127.0.0.1");
		::SetWindowText(GetDlgItem(IDC_EDIT2), "20001");
		return 0;
	}

	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		// when the dialog box is created, subclass its edit control:
		char str1[30],str2[15];
		
		::GetDlgItemText(m_hWnd, IDC_EDIT1, str1,30) ;
		::GetDlgItemText(m_hWnd, IDC_EDIT2, str2,15) ;
		g_Ip = new char[strlen(str1) +1];
		strcpy(g_Ip, str1);
		g_Port = new char[strlen(str2) +1];
		strcpy(g_Port, str2);
		EndDialog(0);
		return 0;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		// when the dialog box is created, subclass its edit control:
		return 0;
	}

private:
//   CContainedWindow		m_wndContained;
};

#endif // !defined(AFX_MYDIALOG_H__A7B811F0_4437_4BCB_B8F6_F44DC9B63EC1__INCLUDED_)
