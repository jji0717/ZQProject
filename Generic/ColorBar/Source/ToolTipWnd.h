// ToolTipWnd.h: interface for the ToolTipWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CTOOLTIPWND_H__940D92EE_9E60_4363_8A8F_1F0371C0E8F0__INCLUDED_)
#define AFX_CTOOLTIPWND_H__940D92EE_9E60_4363_8A8F_1F0371C0E8F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CToolTipWnd : public CWindowImpl<CToolTipWnd>
{
public:
	CToolTipWnd();
	virtual ~CToolTipWnd();

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	
	BEGIN_MSG_MAP(CToolTipWnd)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
//		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
	END_MSG_MAP()

public:
	UINT        GetWndWidth() const;
	UINT        GetWndHight() const;
	CComBSTR  & GetFilmName();
	void        SetWndWidth(UINT iVal);
	void        SetWndHight(UINT iVal);
	void        SetFilmName(CComBSTR &bstrFilmName);
private:
	UINT        m_iWndWidth;
	UINT        m_iWndHight;
	CComBSTR    m_bstrFilmName;
};

#endif // !defined(AFX_TOOLTIPWND_H__940D92EE_9E60_4363_8A8F_1F0371C0E8F0__INCLUDED_)
