#if !defined(AFX_CHANNELEDITDLG_H__9BE546D4_09EC_41E9_B1A0_A9E929F01F14__INCLUDED_)
#define AFX_CHANNELEDITDLG_H__9BE546D4_09EC_41E9_B1A0_A9E929F01F14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChannelEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChannelEditDlg dialog

class CChannelEditDlg : public CDialog
{
// Construction
public:
	CChannelEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChannelEditDlg)
	enum { IDD = IDD_ChannelEdit };
	CEdit	m_cChnlName;
	CString	m_sChnlName;
	CString	m_sDesc;
	CString	m_sNetIds;
	CString	m_sOnDmdName;
	UINT	m_iMaxBit;
	//}}AFX_DATA
public: 	
	BOOL editMode;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChannelEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChannelEditDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANNELEDITDLG_H__9BE546D4_09EC_41E9_B1A0_A9E929F01F14__INCLUDED_)
