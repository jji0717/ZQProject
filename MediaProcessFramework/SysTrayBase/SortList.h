#if !defined(AFX_SORTLIST_H__6ACE2F6F_AEFE_11D3_BDE9_F4145AA4F676__INCLUDED_)
#define AFX_SORTLIST_H__6ACE2F6F_AEFE_11D3_BDE9_F4145AA4F676__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SortList.h : header file
//

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////
// CSortList window

class CSortList : public CListCtrl
{
// Construction
public:
	CSortList();
	BOOL m_fAsc;         // sort with increase
	int m_nSortedCol;    // current sort column
	static int __stdcall CSortList::ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSortList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSortList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSortList)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SORTLIST_H__6ACE2F6F_AEFE_11D3_BDE9_F4145AA4F676__INCLUDED_)
