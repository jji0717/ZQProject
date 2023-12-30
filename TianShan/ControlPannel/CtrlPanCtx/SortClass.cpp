#include "stdafx.h"
#include "SortClass.h"
/////////////////////////////////////////////////////////////////////////////
// CSortClass
CSortClass::CSortClass(CListViewCtrl * _pWnd, const int _iCol, const bool _bIsNumeric )
{	
	m_iCol = _iCol;	
	m_pWnd = _pWnd;	
	m_bIsNumeric = _bIsNumeric;		
	ATLASSERT(m_pWnd);
	int max =m_pWnd->GetItemCount();	
	DWORD dw;	
	CString txt;	
	TCHAR szTmp[150]={0};
	if (m_bIsNumeric)	
	{
		for (int t = 0; t < max; t++)		
		{			
			dw = m_pWnd->GetItemData(t);
			m_pWnd->GetItemText(t,m_iCol,szTmp,150);
			txt = szTmp;
			m_pWnd->SetItemData(t, (DWORD)new CSortItemInt(dw, txt));		
			memset(szTmp,0,sizeof(szTmp));
		}	
	}	
	else
	{
		for (int t = 0; t < max; t++)	
		{			
			dw = m_pWnd->GetItemData(t);
			m_pWnd->GetItemText(t,m_iCol,szTmp,150);
			txt = szTmp;
			m_pWnd->SetItemData(t, (DWORD)new CSortItem(dw, txt));
			memset(szTmp,0,sizeof(szTmp));
		}
	}
}

CSortClass::~CSortClass()
{	
	ATLASSERT(m_pWnd);
	int max = m_pWnd->GetItemCount();
	if (m_bIsNumeric)	
	{		
		CSortItemInt * pItem;		
		for (int t = 0; t < max; t++)	
		{
			pItem = (CSortItemInt *)m_pWnd->GetItemData(t);			
			ATLASSERT(pItem);
			m_pWnd->SetItemData(t, pItem->m_dw);	
			delete pItem;	
		}
	}	
	else
	{
		CSortItem * pItem;
		for (int t = 0; t < max; t++)
		{
			pItem = (CSortItem *)m_pWnd->GetItemData(t);
			ATLASSERT(pItem);
			m_pWnd->SetItemData(t, pItem->m_dw);
			delete pItem;
		}
	}
	if ( m_pWnd)
	{
		m_pWnd = NULL; 

	}
}

void CSortClass::Sort(const bool bAsc)
{	
	if (m_bIsNumeric)	
	{		
		if (bAsc)
		{
			m_pWnd->SortItems(CompareAscI, 0L);
		}
		else
		{
			m_pWnd->SortItems(CompareDesI, 0L);
		}
	}
	else
	{	
		if (bAsc)	
		{
			m_pWnd->SortItems(CompareAsc, 0L);	
		}
		else
		{
			m_pWnd->SortItems(CompareDes, 0L);
		}
	}
}

int CALLBACK CSortClass::CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;	
	ATLASSERT(i1 && i2);
	return i1->m_txt.CompareNoCase(i2->m_txt);
}

int CALLBACK CSortClass::CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItem * i1 = (CSortItem *) lParam1;
	CSortItem * i2 = (CSortItem *) lParam2;	
	ATLASSERT(i1 && i2);
	return i2->m_txt.CompareNoCase(i1->m_txt);
}

int CALLBACK CSortClass::CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ATLASSERT(i1 && i2);
	if (i1->m_iInt == i2->m_iInt) 
		return 0;	
	return i1->m_iInt > i2->m_iInt ? 1 : -1;
}

int CALLBACK CSortClass::CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{	
	CSortItemInt * i1 = (CSortItemInt *) lParam1;
	CSortItemInt * i2 = (CSortItemInt *) lParam2;
	ATLASSERT(i1 && i2);
	if (i1->m_iInt == i2->m_iInt)
		return 0;	
	return i1->m_iInt < i2->m_iInt ? 1 : -1;
}

CSortClass::CSortItem::CSortItem(const DWORD _dw, const CString & _txt)
{
	m_dw = _dw;
	m_txt = _txt;
}

CSortClass::CSortItem::~CSortItem()
{
}

CSortClass::CSortItemInt::CSortItemInt(const DWORD _dw, const CString & _txt)
{
	m_iInt = atoi(_txt);	
	m_dw = _dw;
}