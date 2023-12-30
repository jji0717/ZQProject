// SNMPVarDialog.h: interface for the CSNMPVarDialog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZQSNMPVARDIALG_H
#define __ZQSNMPVARDIALG_H

#include "resource.h"
#include <ZQSNMPOperPkg.h>
#include <Locks.h>
#include "ButtonXP.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSNMPVarDialog : public CDialogImpl<CSNMPVarDialog>
{
public:
	CSNMPVarDialog();
	virtual ~CSNMPVarDialog();

	enum { IDD = IDD_SNMPVAR_FORM };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
//	    return FALSE;
		return IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP_EX(CSNMPVarDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
//		MESSAGE_HANDLER(WM_VSCROLL,OnVScroll)
//		MESSAGE_HANDLER(WM_HSCROLL,OnHScroll)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		COMMAND_RANGE_HANDLER(IDC_MYEDIT,IDC_MYEDIT+EDITMAXNUM,OnChangeEdit)
//	    COMMAND_HANDLER(IDC_MYEDIT+4, EN_CHANGE, OnChangeEdit1)
//		NOTIFY_CODE_HANDLER(EN_CHANGE,OnChangeEdit6)
//	    NOTIFY_RANGE_HANDLER(IDC_MYEDIT,IDC_MYEDIT+EDITMAXNUM,OnChangeEdit3)
//		REFLECTED_COMMAND_RANGE_CODE_HANDLER_EX(IDC_MYEDIT, IDC_MYEDIT+EDITMAXNUM,EN_CHANGE,OnChangeEdit4)
//		REFLECTED_NOTIFY_RANGE_CODE_HANDLER_EX(IDC_MYEDIT, IDC_MYEDIT+EDITMAXNUM, EN_CHANGE, OnChangeEdit5)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
		
	LRESULT OnCreate(UINT uMsg,WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnVScroll(UINT uMsg,WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnHScroll(UINT uMsg,WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnChangeEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	
private:
	void InitControl();
private:
	CStatic  *m_pStaic;
	CEdit    *m_pEdit;
//	CButton  *m_pButton;
//	CEditXP  *m_pEdit;
//	CXPButton *m_pButton;
	CXPButton m_btnOK;

	
	int      m_TextHight;
	int      m_TextWidth;
	int      m_iVarNum;
	HANDLE   m_hSession;
	BOOL     m_bFirst;
	VARDATAS * m_pSNMPVarData ;
	ZQMANSTATUS         m_manRet;
	CString             m_RecectVarName;
	BOOL                m_bSNMPOK;
	ZQ::common::Mutex	m_Mutex;
};

#endif // !defined(AFX_SNMPVARDIALOG_H__0224FDBF_9EB1_4782_9C36_421E437D53FE__INCLUDED_)

/*
LRESULT OnChangeEdit1(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
		MessageBox("ChangeEdit1");
		bHandled = FALSE;
		return 0;
}
LRESULT OnChangeEdit3(int idCtrl,LPNMHDR pnmh, BOOL & bHandled)
{
	//	if ( pnmh->code == EN_CHANGE )
		{
			
			MessageBox("ChangeEdit3");
			bHandled = FALSE;
		}
	
		return 0;
}

LRESULT OnChangeEdit6(int idCtrl,LPNMHDR pnmh, BOOL & bHandled)
{
	//	if ( pnmh->code == EN_CHANGE )
		{
			
			MessageBox("ChangeEdit6");
			bHandled = FALSE;
		}
	
		return 0;
}

	
LRESULT OnChangeEdit4(UINT uMsg,WPARAM wParam, HWND  lParam)
{
		MessageBox("ChangeEdit4");
		//	bHandled = FALSE;
			return S_OK;
}

LRESULT OnChangeEdit5(LPNMHDR lParam)
{
		MessageBox("ChangeEdit5");
		//	bHandled = FALSE;
			return S_OK;
}
*/