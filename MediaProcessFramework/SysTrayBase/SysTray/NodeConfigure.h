#if !defined(AFX_NODECONFIGURE_H__19ABDF97_5730_45F9_84CB_C09DC7805295__INCLUDED_)
#define AFX_NODECONFIGURE_H__19ABDF97_5730_45F9_84CB_C09DC7805295__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NodeConfigure.h : header file
//

#include "../InfoDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CNodeConfigure dialog

class CNodeConfigure : public CInfoDlg
{
// Construction
public:
	CNodeConfigure(CWnd* pParent = NULL);   // standard constructor
	~CNodeConfigure();
	virtual RpcValue& GenRpcValue(); 
	virtual void UpdateDlgList(RpcValue& result);
	void SetUerConfig(const CString strHost, const int nPort, const DWORD unTime);
	int GetNodeType() const;
	void SetNodeType(int _type);
	CString GetHost() const;
	int GetPort() const;
	DWORD GetTimeInteval() const;

// Dialog Data
	//{{AFX_DATA(CNodeConfigure)
	enum { IDD = IDD_DLG_CONFIGURE };
	CString	m_host;
	CString	m_time;
	CString	m_port;
	//}}AFX_DATA

public:
	// Attributes:
	static CString gHost;
	static int     gPort;
	static DWORD   gTimeInteval;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNodeConfigure)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNodeConfigure)
	afx_msg void OnButReset();
	virtual void OnOK( );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	HANDLE _mStop;
	int m_nNodeType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODECONFIGURE_H__19ABDF97_5730_45F9_84CB_C09DC7805295__INCLUDED_)
