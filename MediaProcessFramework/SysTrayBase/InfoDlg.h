#if !defined(AFX_INFODLG_H__A6A7F316_3090_4D83_BA0C_47A586DB1D3C__INCLUDED_)
#define AFX_INFODLG_H__A6A7F316_3090_4D83_BA0C_47A586DB1D3C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoDlg.h : header file
//
#include <RpcWpValue.h>
#include "listInfo_Def.h"

#define WM_AUTOREFRESH_MESSAGE	(WM_USER+1005) 
#define IDD_DLG_INFO 1255

using namespace ZQ::rpc;
/////////////////////////////////////////////////////////////////////////////
// CInfoDlg dialog

class CInfoDlg : public CDialog
{
// Construction
public:
	CInfoDlg(DWORD _IDD=IDD, CWnd* pParent = NULL);   // standard constructor
	virtual RpcValue& GenRpcValue() = 0; 
	virtual void UpdateDlgList(RpcValue& result) = 0;
	
	void startWindowUpdate();
	virtual ~CInfoDlg();

	RpcValue params;  // the return PpcValue for children

// Dialog Data
	//{{AFX_DATA(CInfoDlg)
	enum { IDD = IDD_DLG_INFO };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInfoDlg)
	afx_msg LRESULT OnAutoRefresh(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFODLG_H__A6A7F316_3090_4D83_BA0C_47A586DB1D3C__INCLUDED_)
