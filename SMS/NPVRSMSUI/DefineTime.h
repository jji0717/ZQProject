#if !defined(AFX_DEFINETIME_H__7965CDB7_8EED_4214_895A_E526300610DB__INCLUDED_)
#define AFX_DEFINETIME_H__7965CDB7_8EED_4214_895A_E526300610DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefineTime.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DefineTime dialog

class DefineTime : public CDialog
{
// Construction
public:
	DefineTime(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DefineTime)
	enum { IDD = IDD_DEFINE };
	CTime	m_time;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DefineTime)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DefineTime)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFINETIME_H__7965CDB7_8EED_4214_895A_E526300610DB__INCLUDED_)
