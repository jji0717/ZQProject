// CdmiFuseTrayDlg.h : header file
//

#pragma once
#include "CdmiFuseDlg.h"


// CCdmiFuseTrayDlg dialog
class CCdmiFuseTrayDlg : public CDialog
{
// Construction
public:
	CCdmiFuseTrayDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CDMIFUSETRAY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);


// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_Nid;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	
	/*handle user message*/
	afx_msg LRESULT OnExit(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnSetup(WPARAM wParam,LPARAM lParam);
    
	/* main dialog is moved to main window*/
	CCdmiFuseDlg m_dlgSettings;
};
