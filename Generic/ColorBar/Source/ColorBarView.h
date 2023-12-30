#ifndef __COLORBARVIEW_H_
#define __COLORBARVIEW_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include <WINDEF.H>
#include "ColorBarDoc.h"
#include "ToolTipWnd.h"

class CColorBarDoc;
class CColorBarView : public CWindowImpl<CColorBarView>
{
public:
	CColorBarView(CColorBarDoc *pDoc, CColorBarControl *pControl);
	virtual ~CColorBarView();

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL &bHanded);
	LRESULT OnSize(UINT uMsg,  WPARAM wParam, LPARAM  lParam, BOOL &bHandled);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

	BEGIN_MSG_MAP(CColorBarView)
		MESSAGE_HANDLER(WM_CREATE,OnCreate)
		MESSAGE_HANDLER(WM_PAINT,OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkgnd)
		MESSAGE_HANDLER(WM_MOUSEMOVE ,OnMouseMove)
//    	MESSAGE_HANDLER(WM_NCHITTEST,OnNcHitTest)
	END_MSG_MAP()

public:
	void Repaint(); // 重画
private:
	CComBSTR  GetFilmName(UINT iXpos);              //根据当前鼠标的X坐标位置来获取当前的节目名
private:
	CColorBarControl *m_pColorBarControl; //组件指针
	CColorBarDoc     *m_pDoc; //文档指针
//	CToolTipCtrl      m_DisplayTip;
	CToolTipWnd      *m_pTitleWnd;
};
#endif
