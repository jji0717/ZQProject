// BroadcastChPPTestView.h : interface of the BroadcastChPPTestView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CODMANVIEW_H__CA582EC0_FFB9_475A_9CC6_91BC0120DD7E__INCLUDED_)
#define AFX_CODMANVIEW_H__CA582EC0_FFB9_475A_9CC6_91BC0120DD7E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class BroadcastChPPTestView : public CView
{
protected: // create from serialization only
	BroadcastChPPTestView();
	DECLARE_DYNCREATE(BroadcastChPPTestView)

// Attributes
public:
	BroadcastChPPTestDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BroadcastChPPTestView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~BroadcastChPPTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(BroadcastChPPTestView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in CodManView.cpp
inline BroadcastChPPTestDoc* BroadcastChPPTestView::GetDocument()
   { return (BroadcastChPPTestDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODMANVIEW_H__CA582EC0_FFB9_475A_9CC6_91BC0120DD7E__INCLUDED_)
