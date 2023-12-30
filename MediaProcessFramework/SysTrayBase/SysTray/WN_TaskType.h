#if !defined(AFX_WN_TASKTYPE_H__05DB8EE5_FE6C_4809_9755_4DB17F05EEBA__INCLUDED_)
#define AFX_WN_TASKTYPE_H__05DB8EE5_FE6C_4809_9755_4DB17F05EEBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WN_TaskType.h : header file
//

#include "../InfoDlg.h"
#include "../SortList.h"
/////////////////////////////////////////////////////////////////////////////
// WN_TaskType dialog

class WN_TaskType : public CInfoDlg
{
// Construction
public:
	WN_TaskType(CWnd* pParent = NULL);   // standard constructor

	virtual RpcValue& GenRpcValue(); 
	virtual void UpdateDlgList(RpcValue& result);

// Dialog Data
	//{{AFX_DATA(WN_TaskType)
	enum { IDD = IDD_DLG_TASKTYPE };
	CSortList	m_taskType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WN_TaskType)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WN_TaskType)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnColumnclickListTasktype(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WN_TASKTYPE_H__05DB8EE5_FE6C_4809_9755_4DB17F05EEBA__INCLUDED_)
