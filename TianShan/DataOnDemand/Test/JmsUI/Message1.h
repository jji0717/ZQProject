#if !defined(AFX_MESSAGE1_H__10F9476C_F5B8_4578_9EE8_D51AC288D9C1__INCLUDED_)
#define AFX_MESSAGE1_H__10F9476C_F5B8_4578_9EE8_D51AC288D9C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Message1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessage1 dialog

#include "Queue1Thread.h"

class Queue1Thread;

class CMessage1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CMessage1)

// Construction
public:
	CMessage1();
	~CMessage1();
	
	void setButton(bool bQueue1Busy);

// Dialog Data
	//{{AFX_DATA(CMessage1)
	enum { IDD = IDD_DIALOG_MSG1 };
	CComboBox	m_UpdateMode;
	CComboBox	m_DataType;
	CString	m_SendInterval;
	CString	m_SendTimes;
	CString	m_RootPath;
	CString	m_GroupID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMessage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMessage1)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStopSend();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void init();
	bool validate();

public:
	Queue1Thread* m_queue1Thd;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGE1_H__10F9476C_F5B8_4578_9EE8_D51AC288D9C1__INCLUDED_)
