#if !defined(AFX_CHANNELTV_H__018CA9F3_A7CB_4496_AF5B_D599E04AD8F1__INCLUDED_)
#define AFX_CHANNELTV_H__018CA9F3_A7CB_4496_AF5B_D599E04AD8F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChannelTV.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChannelTV view

class CChannelTV : public CTreeView
{
protected:
	CChannelTV();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CChannelTV)

// Attributes
public:

// Operations
public:
	HTREEITEM GetChildItem(HTREEITEM hParent, CString text);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChannelTV)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChannelTV();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CChannelTV)
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANNELTV_H__018CA9F3_A7CB_4496_AF5B_D599E04AD8F1__INCLUDED_)
