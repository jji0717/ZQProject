#if !defined(AFX_CHANNELDLG_H__655C3CC2_68EC_426E_9242_C432B81D42B1__INCLUDED_)
#define AFX_CHANNELDLG_H__655C3CC2_68EC_426E_9242_C432B81D42B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChannelDlg.h : header file
//

#include "BtnST.h"

/////////////////////////////////////////////////////////////////////////////
// CChannelDlg dialog

class CChannelDlg : public CDialog
{
// Construction
public:
	CChannelDlg(BOOL readonly = TRUE, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChannelDlg)
	enum { IDD = IDD_CHANNEL };
	CEdit	m_ctlDesc;
	CEdit	m_ctlName;
	CString	m_valName;
	CString	m_valDesc;
	//}}AFX_DATA

	BOOL		m_ReadOnly;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChannelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CButtonST	m_btnOK;
	CButtonST	m_btnCancel;



	// Generated message map functions
	//{{AFX_MSG(CChannelDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANNELDLG_H__655C3CC2_68EC_426E_9242_C432B81D42B1__INCLUDED_)
