#if !defined(AFX_MN_SESSIONS1_H__F3EF2F79_A836_43E2_8453_E3B0ECCB454B__INCLUDED_)
#define AFX_MN_SESSIONS1_H__F3EF2F79_A836_43E2_8453_E3B0ECCB454B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MN_Sessions1.h : header file
//

#include "../SortList.h"
#include "../InfoDlg.h"
/////////////////////////////////////////////////////////////////////////////
// MN_Sessions dialog

class MN_Sessions : public CInfoDlg
{
// Construction
public:
	MN_Sessions(CWnd* pParent = NULL);   // standard constructor
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);

// Dialog Data
	//{{AFX_DATA(MN_Sessions)
	enum { IDD = IDD_DLG_SESSIONS };
	CSortList	m_sessions;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MN_Sessions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MN_Sessions)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclickListSessions(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListSessions(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MN_SESSIONS1_H__F3EF2F79_A836_43E2_8453_E3B0ECCB454B__INCLUDED_)
