// NPVRSMSUIView.h : interface of the CNPVRSMSUIView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPVRSMSUIVIEW_H__F45821CD_4694_4655_AA05_D8D8389930AD__INCLUDED_)
#define AFX_NPVRSMSUIVIEW_H__F45821CD_4694_4655_AA05_D8D8389930AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NPVRSMSDB.h"

class CNPVRSMSUIView : public CListView
{
protected: // create from serialization only
	CNPVRSMSUIView();
	DECLARE_DYNCREATE(CNPVRSMSUIView)

// Attributes
public:
	CNPVRSMSUIDoc* GetDocument();

private:
	NPVRSMSDB m_db;

// Operations
public:

private:
	void	listSetup();
	bool	dbInitail();
	void	selectAll();
	void	listData();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNPVRSMSUIView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNPVRSMSUIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CNPVRSMSUIView)
	afx_msg void OnDefine();
	afx_msg void OnRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in NPVRSMSUIView.cpp
inline CNPVRSMSUIDoc* CNPVRSMSUIView::GetDocument()
   { return (CNPVRSMSUIDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NPVRSMSUIVIEW_H__F45821CD_4694_4655_AA05_D8D8389930AD__INCLUDED_)
