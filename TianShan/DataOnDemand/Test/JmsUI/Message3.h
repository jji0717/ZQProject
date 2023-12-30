#if !defined(AFX_MESSAGE3_H__AB5EF8B0_44DB_437F_964D_E7EAE4F2BC46__INCLUDED_)
#define AFX_MESSAGE3_H__AB5EF8B0_44DB_437F_964D_E7EAE4F2BC46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Message3.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessage3 dialog

#include "Queue2Thread.h"

class CMessage3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CMessage3)

// Construction
public:
	CMessage3();
	~CMessage3();

// Dialog Data
	//{{AFX_DATA(CMessage3)
	enum { IDD = IDD_DIALOG_MSG3 };
	CComboBox	m_UpdateMode;
	CComboBox	m_DataType;
	CString	m_FilePath;
	CString	m_RootPath;
	CString	m_GroupID;
	CString	m_SendInterval;
	CString	m_SendTimes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMessage3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMessage3)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnStopSend();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void init();
	bool validate();
	void setButton(bool bQueue2Busy);

public:
	Queue2Thread* m_queue2Thd;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGE3_H__AB5EF8B0_44DB_437F_964D_E7EAE4F2BC46__INCLUDED_)
