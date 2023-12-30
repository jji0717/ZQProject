// TrayManageNodeDlg.h : header file
//

#if !defined(AFX_TRAYMANAGENODEDLG_H__2462B963_7B1C_4E22_B382_8593BFB86BF2__INCLUDED_)
#define AFX_TRAYMANAGENODEDLG_H__2462B963_7B1C_4E22_B382_8593BFB86BF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	DEAULT_REFRESH_WAIT		5000

#define WM_TRAYMESSAGE			(WM_USER+1000)

#include "../TabSheet.h"
#include "MN_GeneralInfo.h"
#include "MN_WorkNodes.h"
#include "MN_Sessions.h"
#include "MN_ManagedWorkIns.h"
#include "../InfoQueryThread.h"

/////////////////////////////////////////////////////////////////////////////
// CTrayManageNodeDlg dialog

class CTrayManageNodeDlg : public CInfoDlg
{
// Construction
public:
	CTrayManageNodeDlg(CWnd* pParent = NULL);	// standard constructor
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);

// Dialog Data
	//{{AFX_DATA(CTrayManageNodeDlg)
	enum { IDD = IDD_TRAYMANAGENODE_DIALOG };
	CTabSheet	m_sheet;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrayManageNodeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	void  OnTrayMessage(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CTrayManageNodeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnMenuExit();
	afx_msg void OnMenuRefresh();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnAutoRefresh(WPARAM wParam, LPARAM lParam); 
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void AddSysTray();
	NOTIFYICONDATA tnid;
	bool isWindowShown;

	MN_GeneralInfo     infoDlg;
	MN_WorkNodes       workndeDlg;
	MN_Sessions        sessionDlg;
	MN_ManagedWorkIns  workinsDlg;

	int nTimeInterval;
	InfoQueryThread infoThread;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAYMANAGENODEDLG_H__2462B963_7B1C_4E22_B382_8593BFB86BF2__INCLUDED_)
