// TrayWorkNodeDlg.h : header file
//

#if !defined(AFX_TRAYWORKNODEDLG_H__8BDD8E36_7636_423F_868B_8AECE879B1D5__INCLUDED_)
#define AFX_TRAYWORKNODEDLG_H__8BDD8E36_7636_423F_868B_8AECE879B1D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TabSheet.h"
#include "../InfoDlg.h"
#include "WN_GeneralInfo.h"
#include "WN_TaskType.h"
#include "WN_WorkInstance.h"

#define WM_TRAYMESSAGE WM_USER+1000

/////////////////////////////////////////////////////////////////////////////
// CTrayWorkNodeDlg dialog

class CTrayWorkNodeDlg : public CInfoDlg
{
// Construction
public:
	CTrayWorkNodeDlg(CWnd* pParent = NULL);	// standard constructor
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);

	// Dialog Data
	//{{AFX_DATA(CTrayWorkNodeDlg)
	enum { IDD = IDD_TRAYWORKNODE_DIALOG };
	CTabSheet	m_sheet;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrayWorkNodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	void  OnTrayMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnAutoRefresh(WPARAM wParam, LPARAM lParam);
	
	// Generated message map functions
	//{{AFX_MSG(CTrayWorkNodeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnMenuExit();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMenuRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void AddSysTray();
	NOTIFYICONDATA tnid;  //systray structure
	bool isWindowShown;   //whether the dialog is shown
	int nTimeInterval;    //Time interval for update information

	WN_GeneralInfo  infoDlg;
	WN_TaskType     tasktypeDlg;
	WN_WorkInstance insDlg;

	InfoQueryThread infoThread;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAYWORKNODEDLG_H__8BDD8E36_7636_423F_868B_8AECE879B1D5__INCLUDED_)
