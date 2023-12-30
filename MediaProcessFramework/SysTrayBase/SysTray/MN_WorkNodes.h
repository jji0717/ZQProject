#if !defined(AFX_MN_WORKNODES_H__AC3A8C86_80BA_4FC9_BB3F_6433D155E753__INCLUDED_)
#define AFX_MN_WORKNODES_H__AC3A8C86_80BA_4FC9_BB3F_6433D155E753__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MN_WorkNodes.h : header file
//

#include "../SortList.h"
#include "../InfoDlg.h"
/////////////////////////////////////////////////////////////////////////////
// MN_WorkNodes dialog

class MN_WorkNodes : public CInfoDlg
{
// Construction
public:
	MN_WorkNodes(CWnd* pParent = NULL);   // standard constructor
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);

// Dialog Data
	//{{AFX_DATA(MN_WorkNodes)
	enum { IDD = IDD_DLG_WORKNODE };
	CSortList	m_worknode;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MN_WorkNodes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MN_WorkNodes)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclickListWorknode(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void removeCR(CString &strObject);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MN_WORKNODES_H__AC3A8C86_80BA_4FC9_BB3F_6433D155E753__INCLUDED_)
