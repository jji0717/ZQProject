#if !defined(AFX_OPENCODDLG_H__537E32BB_B46A_444D_A7B3_EC35DE5B0413__INCLUDED_)
#define AFX_OPENCODDLG_H__537E32BB_B46A_444D_A7B3_EC35DE5B0413__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpenCodDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpenCodDlg dialog

class COpenCodDlg : public CDialog
{
// Construction
public:
	COpenCodDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COpenCodDlg)
	enum { IDD = IDD_OpenCodDlg };
	CString	m_strCodEndPoint;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenCodDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COpenCodDlg)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENCODDLG_H__537E32BB_B46A_444D_A7B3_EC35DE5B0413__INCLUDED_)
