#if !defined(AFX_MESSAGE2_H__F0FEB2FA_B6B3_483F_9ACB_2267E700EDF2__INCLUDED_)
#define AFX_MESSAGE2_H__F0FEB2FA_B6B3_483F_9ACB_2267E700EDF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Message2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessage2 dialog

#include "Queue1Thread.h"

class Queue1Thread;

class CMessage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CMessage2)

// Construction
public:
	CMessage2();
	~CMessage2();
	void setButton(bool bQueue1Busy);

// Dialog Data
	//{{AFX_DATA(CMessage2)
	enum { IDD = IDD_DIALOG_MSG2 };
	CComboBox	m_DestinationType;
	CComboBox	m_Destination;
	CComboBox	m_DataType;
	CString	m_ExpiredTime;
	CString	m_SendInterval;
	CString	m_SendTimes;
	CString	m_GroupID;
	CString	m_MessageID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMessage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMessage2)
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

#endif // !defined(AFX_MESSAGE2_H__F0FEB2FA_B6B3_483F_9ACB_2267E700EDF2__INCLUDED_)
