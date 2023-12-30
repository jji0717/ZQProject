// MyListViewCtrl.h: interface for the CMyListViewCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZQMYLISTVIEWCTRL_H_
#define __ZQMYLISTVIEWCTRL_H_
#include "SortHeadCtrl.h"
#include <ZQEventsCtrl.h>
#include <Locks.h>
#include "Draw.h"
#include "resource.h"
#include "ItemDialog.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _DEBUG 
	#define ASSERT_VALID_STRING( str ) ATLASSERT( !IsBadStringPtr( str, 0xfffff ) )
#else	//	_DEBUG
	#define ASSERT_VALID_STRING( str ) ( (void)0 )
#endif	//	_DEBUG

class CSortListViewCtrl : public CWindowImpl<CSortListViewCtrl, CListViewCtrl>
{
public:
	DECLARE_WND_CLASS(NULL);
	
	CSortListViewCtrl(int iMode = 0,LPCTSTR szTabName =_T(""));
	virtual ~CSortListViewCtrl();
	typedef  CWindowImpl<CSortListViewCtrl, CListViewCtrl> baseclass;
		
	BOOL PreTranslateMessage(MSG* pMsg);
	
	//BEGIN_MSG_MAP_EX(CSortListViewCtrl)
	BEGIN_MSG_MAP(CSortListViewCtrl)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		MESSAGE_HANDLER(WM_MEASUREITEM,OnMeasureItem)
		MESSAGE_HANDLER(WM_DRAWITEM,OnDrawItem)
		COMMAND_RANGE_HANDLER(ID_STORAGELINK,ID_STREAMERLINK,OnVerb)
//		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnColumnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
//		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnDblclkList)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDblclkList)
		CHAIN_MSG_MAP_MEMBER((m_pctlHeader))
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnVerb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnDrawItem(UINT , WPARAM , LPARAM , BOOL& bHandled);

//  LRESULT OnColumnClick( LPNMHDR pnmh );
	LRESULT OnColumnClick(int idCtrl,LPNMHDR pnmh, BOOL & bHandled);
//	LRESULT OnDblclkList( LPNMHDR pnmh);
   	LRESULT OnDblclkList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL    SubclassWindow(HWND hWnd);
	void    AttachHeader();
	void    SetAttribeCount(const int iAttribeCount);

public:
	void SortColumn( int iSubItem, BOOL bAscending);
	bool IsNumberItem(const int iSubItem) const;

protected:
	CSortHeadCtrl  m_pctlHeader;
	int m_iMode;
	int m_iNumColumns;
	int m_iSortColumn;
	int m_iAttribeCount;
	BOOL m_bSortAscending;
	ZQ::common::Mutex		m_Mutex;
	char  m_strText[ITEMLEN];
	char  m_strTabName[ITEMLEN];
	CSimpleArray<CString>      m_strMenuString;
	DECLARE_MESSAGE_MAP();
};

//extern GRIDDATAARRAY2  m_AttribesDatas;

#endif // !defined(AFX_MYLISTVIEWCTRL_H__4C6255C5_F501_4E44_BADD_20F9C8822A90__INCLUDED_)
