#if !defined(AFX_MESSAGE4_H__49D19335_9CE0_4A90_B1DC_F6AEFB7AB562__INCLUDED_)
#define AFX_MESSAGE4_H__49D19335_9CE0_4A90_B1DC_F6AEFB7AB562__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Message4.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMessage4 dialog
#include "Queue2Thread.h"

class CMessage4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CMessage4)

// Construction
public:
	CMessage4();
	~CMessage4();

// Dialog Data
	//{{AFX_DATA(CMessage4)
	enum { IDD = IDD_DIALOG_MSG4 };
	CComboBox	m_OperationCode;
	CComboBox	m_DestinationType;
	CComboBox	m_Destination;
	CComboBox	m_DataType;
	CString	m_ExpiredTime;
	CString	m_SendInterval;
	CString	m_SendTimes;
	CString	m_GroupID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMessage4)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMessage4)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStopSend();
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

#endif // !defined(AFX_MESSAGE4_H__AB5EF8B0_44DB_437F_964D_E7EAE4F2BC46__INCLUDED_)
