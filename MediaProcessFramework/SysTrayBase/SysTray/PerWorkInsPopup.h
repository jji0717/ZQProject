#if !defined(AFX_PERWORKINSPOPUP_H__897F3744_3A64_40DE_A116_865915B2106E__INCLUDED_)
#define AFX_PERWORKINSPOPUP_H__897F3744_3A64_40DE_A116_865915B2106E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PerWorkInsPopup.h : header file
//

#include "../InfoDlg.h"
#include "../InfoQueryThread.h"
/////////////////////////////////////////////////////////////////////////////
// PerWorkInsPopup dialog

class PerWorkInsPopup : public CInfoDlg
{
// Construction
public:
	PerWorkInsPopup(const char* strTaskID, CWnd* pParent = NULL);   // standard constructor
	~PerWorkInsPopup();
	virtual RpcValue& GenRpcValue(); 
	virtual void UpdateDlgList(RpcValue& result);
// Dialog Data
	//{{AFX_DATA(PerWorkInsPopup)
	enum { IDD = IDD_POPUP_PERWORKINS };
	CTreeCtrl	m_treeRpc;
	CListCtrl	m_perIns;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PerWorkInsPopup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PerWorkInsPopup)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSelchangedTreeRpcval(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

		//afx_msg void OnClickTreeRpcval(NMHDR* pNMHDR, LRESULT* pResult);
private:
	RpcValue result;
	CImageList m_imageList;
	InfoQueryThread m_TaskDetailThrad;
	HANDLE _mWait;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERWORKINSPOPUP_H__897F3744_3A64_40DE_A116_865915B2106E__INCLUDED_)
