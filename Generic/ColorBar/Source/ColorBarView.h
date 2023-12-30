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
	void Repaint(); // �ػ�
private:
	CComBSTR  GetFilmName(UINT iXpos);              //���ݵ�ǰ����X����λ������ȡ��ǰ�Ľ�Ŀ��
private:
	CColorBarControl *m_pColorBarControl; //���ָ��
	CColorBarDoc     *m_pDoc; //�ĵ�ָ��
//	CToolTipCtrl      m_DisplayTip;
	CToolTipWnd      *m_pTitleWnd;
};
#endif
