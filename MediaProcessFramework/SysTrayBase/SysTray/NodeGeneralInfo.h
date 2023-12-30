#if !defined(AFX_NODEGENERALINFO_H__7E5BBB94_FFF8_4F90_BD09_58F2DD6806CF__INCLUDED_)
#define AFX_NODEGENERALINFO_H__7E5BBB94_FFF8_4F90_BD09_58F2DD6806CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NodeGeneralInfo.h : header file
//

#include "../InfoDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CNodeGeneralInfo dialog

class CNodeGeneralInfo : public CInfoDlg
{
// Construction
public:
	CNodeGeneralInfo(CWnd* pParent = NULL);   // standard constructor
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);

// Dialog Data
	//{{AFX_DATA(MN_GeneralInfo)
	enum { IDD = IDD_DLG_GENERALINFO };
	CString	m_edCpu;
	CString	m_edInt;
	CString	m_edMem;
	CString	m_edNet;
	CString	m_edNid;
	CString	m_edOs;
	CString	m_edPnm;
	CString	m_edVer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MN_GeneralInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MN_GeneralInfo)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	void FormatMemoryStyle(CString& mem);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEGENERALINFO_H__7E5BBB94_FFF8_4F90_BD09_58F2DD6806CF__INCLUDED_)
