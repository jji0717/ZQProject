#if !defined(AFX_CONNECTDLG_H__A2084787_D000_4B10_B6E7_457411D0CCC0__INCLUDED_)
#define AFX_CONNECTDLG_H__A2084787_D000_4B10_B6E7_457411D0CCC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog

#include "BtnSt.h"
#include "TestClient.h"


extern TestClient g_Client;

class CConnectDlg : public CDialog
{
// Construction
public:
	CConnectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConnectDlg)
	enum { IDD = IDD_CONNECT };
	CString	m_serverEndpoint;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	CButtonST	m_btnOK;
	CButtonST	m_btnCancel;

	// Generated message map functions
	//{{AFX_MSG(CConnectDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnConnok();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTDLG_H__A2084787_D000_4B10_B6E7_457411D0CCC0__INCLUDED_)
