#if !defined(AFX_POPSESSIONDETAIL_H__71DF5547_6A2A_44F5_B5E8_619EBFDAA87F__INCLUDED_)
#define AFX_POPSESSIONDETAIL_H__71DF5547_6A2A_44F5_B5E8_619EBFDAA87F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PopSessionDetail.h : header file
//

#include "../InfoDlg.h"
#include "../InfoQueryThread.h"
/////////////////////////////////////////////////////////////////////////////
// CPopSessionDetail dialog

class CPopSessionDetail : public CInfoDlg
{
// Construction
public:
	CPopSessionDetail(const char* sessionID, CWnd* pParent = NULL);   // standard constructor
	~CPopSessionDetail();
	virtual RpcValue& GenRpcValue();
	virtual void UpdateDlgList(RpcValue& result);
	
// Dialog Data
	//{{AFX_DATA(CPopSessionDetail)
	enum { IDD = IDD_DLG_SESSIONDETAIL };
	CTreeCtrl	m_tree;
	CListCtrl	m_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPopSessionDetail)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPopSessionDetail)
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	RpcValue result;
	CImageList m_imageList;
	InfoQueryThread m_TaskDetailThrad;
	HANDLE _mWait;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POPSESSIONDETAIL_H__71DF5547_6A2A_44F5_B5E8_619EBFDAA87F__INCLUDED_)
