// DCATestDlg.h : header file
//
#include ".\jms.h"
#include "Parser.h"
#include "clog.h"

#pragma once


// CDCATestDlg dialog
class CDCATestDlg : public CDialog
{
// Construction
public:
	CDCATestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DCATEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	//major message parse from MMA service
	CJMSParser * m_Parse;


	CString m_strIPPort;
	CJMS   *m_jms;

	afx_msg void OnBnClickedCancel();
	CString GetCurrDateTime();
	bool SplitURLName( char * szUNCName, char * szAddress, char * szDirectory, int* nPort=NULL);
	afx_msg void OnBnClickedButton4();

	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
};
