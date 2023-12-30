// SortClass.h: interface for the CSortClass class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SORTCLASS_H
#define __SORTCLASS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSortClass
{
public:
	CSortClass(CListViewCtrl * _pWnd, const int _iCol, const bool _bIsNumeric);
	virtual ~CSortClass();		
	
	void Sort(const bool bAsc);	
	static int CALLBACK CompareAsc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDes(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareAscI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK CompareDesI(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
	int m_iCol;	
	CListViewCtrl * m_pWnd;	
	bool m_bIsNumeric;

public:	
	class CSortItem	
	{	
		public:		
			virtual  ~CSortItem();
			CSortItem(const DWORD _dw, const CString &_txt);		
			CString m_txt;		
			DWORD m_dw;	
	};
	class CSortItemInt	
	{	
		public:
			CSortItemInt(const DWORD _dw, const CString &_txt);		
			int m_iInt ;		
			DWORD m_dw;	
	};
};

#endif // !defined(AFX_SORTCLASS_H__98A1CC05_F306_4872_8FAA_B66262B52EA8__INCLUDED_)
