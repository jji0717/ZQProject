// ChPubDlg.h : header file
//

#if !defined(AFX_CHPUBDLG_H__5F7E5D96_D048_4C53_87C3_3BDFF618FB45__INCLUDED_)
#define AFX_CHPUBDLG_H__5F7E5D96_D048_4C53_87C3_3BDFF618FB45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CChPubDlg dialog

#include "BtnST.h"
#include "TestClient.h"

class CChPubDlg : public CDialog
{
// Construction
public:
	CChPubDlg(CWnd* pParent = NULL);	// standard constructor
	~CChPubDlg();

// Dialog Data
	//{{AFX_DATA(CChPubDlg)
	enum { IDD = IDD_CHPUB_DIALOG };
	CTreeCtrl	m_Channels;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChPubDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CButtonST	m_btnConnect;
	CButtonST	m_btnRefresh;
	CButtonST	m_btnAdd;
	CButtonST	m_btnEdit;
	CButtonST	m_btnRemove;
	CButtonST	m_btnInfo;

	CImageList	m_treeImgList;
	bool		m_isConnected;

	// Generated message map functions
	//{{AFX_MSG(CChPubDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConnect();
	afx_msg void OnRefresh();
	afx_msg void OnAdd();
	afx_msg void OnEdit();
	afx_msg void OnRemove();
	afx_msg void OnInfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHPUBDLG_H__5F7E5D96_D048_4C53_87C3_3BDFF618FB45__INCLUDED_)
