// AppDlg.h : header file
//

#if !defined(AFX_APPDLG_H__9F67F994_8671_436D_9549_5F639F5E8D63__INCLUDED_)
#define AFX_APPDLG_H__9F67F994_8671_436D_9549_5F639F5E8D63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAppDlg dialog


class CAppDlg : public CDialog
{
// Construction	
public:
	CAppDlg(CWnd* pParent = NULL);	// standard constructor
	~CAppDlg();	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAppDlg)
	enum { IDD = IDD_APP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	BOOL m_bEnableChannel;
	// Generated message map functions
	//{{AFX_MSG(CAppDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	virtual void OnOK();
	afx_msg void OnButton2();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	afx_msg void OnClose();
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	afx_msg LRESULT OnTrayMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButton7();
	afx_msg void OnButton8();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void TrayIcon(BOOL add);
public:
	afx_msg void OnCloseExit();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPDLG_H__9F67F994_8671_436D_9549_5F639F5E8D63__INCLUDED_)
