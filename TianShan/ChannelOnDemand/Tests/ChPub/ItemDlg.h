#if !defined(AFX_ITEMDLG_H__9DAC0227_FFB5_4173_A3CB_3340C6587ADB__INCLUDED_)
#define AFX_ITEMDLG_H__9DAC0227_FFB5_4173_A3CB_3340C6587ADB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ItemDlg.h : header file
//

#include "BtnST.h"

/////////////////////////////////////////////////////////////////////////////
// CItemDlg dialog

class CItemDlg : public CDialog
{
// Construction
public:
	CItemDlg(BOOL edit = TRUE, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CItemDlg)
	enum { IDD = IDD_ITEM };
	CEdit	m_ctlStart;
	CEdit	m_ctlExpiration;
	CEdit	m_ctlContent;
	CString	m_valContent;
	CString	m_valExpiration;
	CString	m_valStart;
	long	m_valSpliceOut;
	long	m_valSpliceIn;
	BOOL	m_valForceNormalSpeed;
	BOOL	m_valPlayable;
	BOOL	m_valOutTimeOffSet;
	BOOL	m_valInTimeOffSet;
	//}}AFX_DATA


	BOOL		m_bEdit;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CItemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CButtonST	m_btnOK;
	CButtonST	m_btnCancel;

	// Generated message map functions
	//{{AFX_MSG(CItemDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITEMDLG_H__9DAC0227_FFB5_4173_A3CB_3340C6587ADB__INCLUDED_)
