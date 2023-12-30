#if !defined(AFX_MN_MANAGEDWORKINS_H__DF26F245_CBB9_43EA_B924_1540150012AF__INCLUDED_)
#define AFX_MN_MANAGEDWORKINS_H__DF26F245_CBB9_43EA_B924_1540150012AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MN_ManagedWorkIns.h : header file
//

#include "../SortList.h"
#include "../InfoDlg.h"
/////////////////////////////////////////////////////////////////////////////
// MN_ManagedWorkIns dialog

class MN_ManagedWorkIns : public CInfoDlg
{
// Construction
public:
	MN_ManagedWorkIns(CWnd* pParent = NULL);   // standard constructor
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);

// Dialog Data
	//{{AFX_DATA(MN_ManagedWorkIns)
	enum { IDD = IDD_DLG_MANAGED_WORKINS };
	CSortList	m_workIns;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MN_ManagedWorkIns)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MN_ManagedWorkIns)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclickListWorkins(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MN_MANAGEDWORKINS_H__DF26F245_CBB9_43EA_B924_1540150012AF__INCLUDED_)
