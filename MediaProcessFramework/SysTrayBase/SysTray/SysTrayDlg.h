// SysTrayDlg.h : header file
//

#if !defined(AFX_SYSTRAYDLG_H__83BFB0BE_F9F3_4C12_A8F6_272FD055AA62__INCLUDED_)
#define AFX_SYSTRAYDLG_H__83BFB0BE_F9F3_4C12_A8F6_272FD055AA62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../TabSheet.h"
#include "../InfoDlg.h"
#include "NodeConfigure.h"
#include "NodeGeneralInfo.h"
#include "MN_WorkNodes.h"
#include "MN_Sessions.h"
#include "MN_ManagedWorkIns.h"
#include "WN_TaskType.h"
#include "WN_WorkInstance.h"
#include "../InfoQueryThread.h"

#define WM_TRAYMESSAGE			        (WM_USER+1000)
/////////////////////////////////////////////////////////////////////////////
// CSysTrayDlg dialog

class CSysTrayDlg : public CInfoDlg
{
// Construction
public:
	CSysTrayDlg(CWnd* pParent = NULL);	// standard constructor
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);
	void SetRefreshEnable(bool isEnable);	
	void CreateManageNode(void);
	void CreateWorkNode(void);
	void CreateMixedNode(void);


// Dialog Data
	//{{AFX_DATA(CSysTrayDlg)
	enum { IDD = IDD_SYSTRAY_DIALOG };
	CTabSheet	m_sheet;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysTrayDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON	m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSysTrayDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnAutoRefresh(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSysTrayMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuitemExit();
	afx_msg void OnMenuitemRefresh();
	afx_msg void OnMenuitemConfig();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void AddSysTray();

	NOTIFYICONDATA tnid;
	CInfoDlg* m_pInfoDlg[10];
	int       m_nInfoDlgCount;

	bool				m_bWait;
	InfoQueryThread*	m_pInfoThread;
	CMenu				m_menu;
	bool				isRereshEnable;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTRAYDLG_H__83BFB0BE_F9F3_4C12_A8F6_272FD055AA62__INCLUDED_)
