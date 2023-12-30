// JmsUIDlg.h : header file
//

#if !defined(AFX_JMSUIDLG_H__22D1807F_FD46_4AA5_BD8E_306D8D8A0CCC__INCLUDED_)
#define AFX_JMSUIDLG_H__22D1807F_FD46_4AA5_BD8E_306D8D8A0CCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CJmsUIDlg dialog

#include "JMSPublisher.h"
#include "FileLog.h"
#include "Message1.h"
#include "Message2.h"
#include "Message3.h"
#include "Message4.h"

using namespace TianShanIce::common;
using namespace ZQ::common;

class Queue1Thread;
class Queue2Thread;

class CJmsUIDlg : public CDialog
{
// Construction
public:
	CJmsUIDlg(CWnd* pParent = NULL);	// standard constructor

	bool pushShareFolder(const char* str);
	bool pushMessage(const char* str);

// Dialog Data
	//{{AFX_DATA(CJmsUIDlg)
	enum { IDD = IDD_JMSUI_DIALOG };
	CTabCtrl	m_tab;
	CString	m_Queue1_DEST;
	CString	m_Queue2_DEST;
	CString	m_Queue_IP;
	CString	m_Queue_Port;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJmsUIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CJmsUIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnShareFolder();
	afx_msg void OnMessage();
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void init();
	void ShowTab1(bool bShow);
	void ShowTab2(bool bShow);
	void EnableTab1(bool bQueue1Connect, bool bQueue2Connect);
	void EnableTab2(bool bQueue1Connect, bool bQueue2Connect);
	bool ValidIP(CString& queueIp);
	bool ConnectJBoss();

private:
	JMSPublisherManager* m_JmsPublisherMgr;
	FileReport* m_FileReport;
	Queue1Thread* m_Queue1Thd;
	Queue2Thread* m_Queue2Thd;

	bool m_bShowFolderConnect;
	bool m_bMessageConnect;

	bool m_bConnectJBoss;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMSUIDLG_H__22D1807F_FD46_4AA5_BD8E_306D8D8A0CCC__INCLUDED_)
