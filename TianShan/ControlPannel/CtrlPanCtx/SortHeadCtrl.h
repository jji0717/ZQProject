// SortHeadCtrl.h: interface for the CSortHeadCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZQMYHEADERCTRL_H
#define __ZQMYHEADERCTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSortHeadCtrl : public CWindowImpl<CSortHeadCtrl, CHeaderCtrl>
{
public:
	CSortHeadCtrl();
	virtual ~CSortHeadCtrl();
	void SetSortArrow( const int iColumn, const BOOL bAscending );

	BEGIN_MSG_MAP(CSortHeadCtrl)
//		MESSAGE_HANDLER(OCM_DRAWITEM,OnDrawItem)
//		MESSAGE_HANDLER(WM_DRAWITEM,OnDrawItem)
//		MESSAGE_HANDLER(WM_CREATE,OnCreate)
	END_MSG_MAP()
public:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void SetHeadItem( int iSubItem,const BOOL bAscending );
	void SetHeadItem( );
	void RemoveAllSortImages();
	void RemoveSortImage( int iItem );

protected:
	int  m_iSortColumn;
	BOOL m_bSortAscending;
	CBitmap m_bmpArrowUp;
	CBitmap m_bmpArrowDown;
private:
	LRESULT OnDrawItem(UINT , WPARAM , LPARAM , BOOL& bHandled);
};



#endif // !defined(AFX_SORTHEADCTRL_H__94DFCC2F_6D75_43B6_BFD4_A9AD311613E7__INCLUDED_)
