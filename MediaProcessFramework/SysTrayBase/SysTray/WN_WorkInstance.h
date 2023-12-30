#if !defined(AFX_WN_WORKINSTANCE_H__6FD5B804_0296_4EC2_96C9_C7BA109D7D06__INCLUDED_)
#define AFX_WN_WORKINSTANCE_H__6FD5B804_0296_4EC2_96C9_C7BA109D7D06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WN_WorkInstance.h : header file
//

#include "../InfoDlg.h"
#include "../SortList.h"
/////////////////////////////////////////////////////////////////////////////
// WN_WorkInstance dialog

class WN_WorkInstance : public CInfoDlg
{
// Construction
public:
	WN_WorkInstance(CWnd* pParent = NULL);   // standard constructor

	virtual RpcValue& GenRpcValue(); 
	virtual void UpdateDlgList(RpcValue& result);

// Dialog Data
	//{{AFX_DATA(WN_WorkInstance)
	enum { IDD = IDD_DLG_WORKINSTANCE };
	CSortList	m_instance;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WN_WorkInstance)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WN_WorkInstance)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkListWorkinstance(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickListWorkinstance(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WN_WORKINSTANCE_H__6FD5B804_0296_4EC2_96C9_C7BA109D7D06__INCLUDED_)
