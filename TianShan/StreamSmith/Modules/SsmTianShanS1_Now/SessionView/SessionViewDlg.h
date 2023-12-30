// SessionViewDlg.h : header file
//

#if !defined(AFX_SESSIONVIEWDLG_H__A08D3744_962F_42D3_BE44_9C7760073C1A__INCLUDED_)
#define AFX_SESSIONVIEWDLG_H__A08D3744_962F_42D3_BE44_9C7760073C1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSessionViewDlg dialog
#include <Ice/Ice.h>
#include "./SessionContext_ice.h"

class CSessionViewDlg : public CDialog
{
// Construction
public:
	CSessionViewDlg(CWnd* pParent = NULL);	// standard constructor
	
	bool ConnectServer(const CString& connStr);

// Dialog Data
	//{{AFX_DATA(CSessionViewDlg)
	enum { IDD = IDD_SESSIONVIEW_DIALOG };
	CButton	m_btnPrev;
	CButton	m_btnNext;
	CButton	m_btnLast;
	CButton	m_btnBegin;
	CListCtrl	m_lstViewSess;
	CString	m_sEndPoint;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSessionViewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSessionViewDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonBegin();
	afx_msg void OnButtonPrevious();
	afx_msg void OnButtonNext();
	afx_msg void OnButtonLast();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void ChangeButtonStatus();
	void ShowData(const TianShanS1::SessionDatas& datas);

private:
	TianShanS1::SessionViewPrx _pSessionView;
	::Ice::CommunicatorPtr _pCommunicator;
	Ice::Int totalSize;
	int pageSize;
	int totalPage;
	int currPage;
	Ice::Int clientId;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SESSIONVIEWDLG_H__A08D3744_962F_42D3_BE44_9C7760073C1A__INCLUDED_)
