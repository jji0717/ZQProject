#if !defined(AFX_CHNLITEMEDITDLG_H__159194B9_25CF_4B37_8FB0_74B8F740AA8C__INCLUDED_)
#define AFX_CHNLITEMEDITDLG_H__159194B9_25CF_4B37_8FB0_74B8F740AA8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChnlItemEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChnlItemEditDlg dialog

class CChnlItemEditDlg : public CDialog
{
// Construction
public:
	CChnlItemEditDlg(CWnd* pParent = NULL);   // standard constructor
	BOOL editMode;

// Dialog Data
	//{{AFX_DATA(CChnlItemEditDlg)
	enum { IDD = IDD_ChannelItemEdit };
	CEdit	m_cItemName;
	CString	m_sBroadcast;
	CString	m_sExpiration;
	BOOL	m_bForceSpeed;
	CString	m_sItemName;
	long	m_inTimeOffset;
	BOOL	m_bSpliceIn;
	BOOL	m_bSpliceOut;
	BOOL	m_bPlayable;
	long	m_outTimeOffset;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChnlItemEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChnlItemEditDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHNLITEMEDITDLG_H__159194B9_25CF_4B37_8FB0_74B8F740AA8C__INCLUDED_)
