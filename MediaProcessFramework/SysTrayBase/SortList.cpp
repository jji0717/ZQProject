// SortList.cpp : implementation file
//

#include "stdafx.h"
#include "SortList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSortList

CSortList::CSortList()
{
	m_nSortedCol = -1;
	m_fAsc=TRUE;
}

CSortList::~CSortList()
{
}


BEGIN_MESSAGE_MAP(CSortList, CListCtrl)
	//{{AFX_MSG_MAP(CSortList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSortList message handlers


int __stdcall CSortList::ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CSortList* pV = (CSortList *)lParamSort;
	// Get selected item data
	CString szComp1,szComp2;
	int iCompRes;
	szComp1=pV->GetItemText(lParam1,pV->m_nSortedCol);
	szComp2=pV->GetItemText(lParam2,pV->m_nSortedCol);

	iCompRes=szComp1.Compare(szComp2); // Compare string only! 
    // Sort with increase or decrease
	if(pV->m_fAsc)
		return iCompRes;
	else
		return -iCompRes;
}
