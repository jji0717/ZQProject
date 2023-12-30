// ButtonXP.h: interface for the CXPButtonImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ATLBTNXP_H__
#define __ATLBTNXP_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlmisc.h>

// CMemDC by Bjarke Viksoe (bjarke@viksoe.dk)

class CMemDC : public CDC
{
public:
   CDCHandle     m_dc;          // Owner DC
   CBitmap       m_bitmap;      // Offscreen bitmap
   CBitmapHandle m_hOldBitmap;  // Originally selected bitmap
   RECT          m_rc;          // Rectangle of drawing area

   CMemDC(HDC hDC, LPRECT pRect)
   {
      ATLASSERT(hDC!=NULL);
      m_dc = hDC;
      if( pRect!=NULL ) m_rc = *pRect; else m_dc.GetClipBox(&m_rc);

      CreateCompatibleDC(m_dc);
      ::LPtoDP(m_dc, (LPPOINT) &m_rc, sizeof(RECT)/sizeof(POINT));
      m_bitmap.CreateCompatibleBitmap(m_dc, m_rc.right-m_rc.left, m_rc.bottom-m_rc.top);
      m_hOldBitmap = SelectBitmap(m_bitmap);
      ::DPtoLP(m_dc, (LPPOINT) &m_rc, sizeof(RECT)/sizeof(POINT));
	  SetWindowOrg(m_rc.left, m_rc.top);
	  FillSolidRect(&m_rc, m_dc.GetBkColor());
   }
   ~CMemDC()
   {
      // Copy the offscreen bitmap onto the screen.
      m_dc.BitBlt(m_rc.left, m_rc.top, m_rc.right-m_rc.left, m_rc.bottom-m_rc.top,
                  m_hDC, m_rc.left, m_rc.top, SRCCOPY);
      //Swap back the original bitmap.
      SelectBitmap(m_hOldBitmap);
   }
};

/************************************************************************/
template<class T, class TBase = CEdit, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CEditXPImpl	: public CWindowImpl<T, TBase, TWinTraits>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, TBase::GetWndClassName())
	BEGIN_MSG_MAP(CEditXP)
		MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
	END_MSG_MAP()

	CEditXPImpl()
	{
		m_hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	}

	~CEditXPImpl()
	{
		DeleteObject(m_hPen);
	}
	
	LRESULT OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rect;
		GetWindowRect(&rect);
		CDCHandle dc = GetWindowDC();
		HPEN hOldPen = dc.SelectPen(m_hPen);
		rect.right -= rect.left;
		rect.bottom -= rect.top;
		rect.left = rect.top = 0;
		dc.Rectangle(&rect);
		dc.SelectPen(hOldPen);
		ReleaseDC(dc);
		return 0;
	}

	HPEN m_hPen;
};

class CEditXP : public CEditXPImpl<CEditXP>
{
public:
	DECLARE_WND_CLASS(_T("WTL_XPEdit"));
};

/************************************************************************/

template<class T, class TBase = CButton, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CXPButtonImpl : public CWindowImpl<T, TBase, TWinTraits>
{
public:
	DECLARE_WND_SUPERCLASS(NULL ,CButton::GetWndClassName() );
	BEGIN_MSG_MAP(CXPButtonImpl)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_MOUSEHOVER, OnMouseHover)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	//ALT_MSG_MAP(1)
		MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	CXPButtonImpl()
	{
		m_BoundryPen.CreatePen(PS_INSIDEFRAME|PS_SOLID, 1, RGB(0, 0, 0));
		m_InsideBoundryPenLeft.CreatePen(PS_INSIDEFRAME|PS_SOLID,3,RGB(250,196,88)); 
		m_InsideBoundryPenRight.CreatePen(PS_INSIDEFRAME|PS_SOLID,3,RGB(251,202,106));
		m_InsideBoundryPenTop.CreatePen(PS_INSIDEFRAME|PS_SOLID,2,RGB(252,210,121));
		m_InsideBoundryPenBottom.CreatePen(PS_INSIDEFRAME|PS_SOLID,2,RGB(229,151,0));
	
		m_FillActive.CreateSolidBrush(RGB(223,222,236));
		m_FillInactive.CreateSolidBrush(RGB(222,223,236));

		m_InsideBoundryPenLeftSel.CreatePen(PS_INSIDEFRAME|PS_SOLID,3,RGB(153,198,252)); 
		m_InsideBoundryPenTopSel.CreatePen(PS_INSIDEFRAME|PS_SOLID,2,RGB(162,201,255));
		m_InsideBoundryPenRightSel.CreatePen(PS_INSIDEFRAME|PS_SOLID,3,RGB(162,189,252));
		m_InsideBoundryPenBottomSel.CreatePen(PS_INSIDEFRAME|PS_SOLID,2,RGB(162,201,255));

		m_bOver = m_bSelected = m_bTracking = m_bFocus = FALSE;
	}
	virtual ~CXPButtonImpl()
	{
		m_BoundryPen.DeleteObject();
		m_InsideBoundryPenLeft.DeleteObject();
		m_InsideBoundryPenRight.DeleteObject();
		m_InsideBoundryPenTop.DeleteObject();
		m_InsideBoundryPenBottom.DeleteObject();

		m_FillActive.DeleteObject();
		m_FillInactive.DeleteObject();

		m_InsideBoundryPenLeftSel.DeleteObject();
		m_InsideBoundryPenRightSel.DeleteObject();
		m_InsideBoundryPenTopSel.DeleteObject();
		m_InsideBoundryPenBottomSel.DeleteObject();
	}

	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl<T, TBase, TWinTraits>::SubclassWindow(hWnd);
		ModifyStyle(0, BS_OWNERDRAW);
		return bRet;
	}

	void DoGradientFill(CDC *pDC,CRect* rect)
	{
		CBrushHandle pBrush[64];
		int nWidth = rect->Width(); //rect.right - rect.left;
		int nHeight = rect->Height(); //.bottom - rect.top;
		CRect rct;
	
		for(int i=0; i<64; i++){
			if (m_bOver) {
				if (m_bFocus) {
					pBrush[i].CreateSolidBrush(RGB(255-(i/4), 255-(i/4), 255-(i/3)));
				}
				else{
					pBrush[i].CreateSolidBrush(RGB(255-(i/4), 255-(i/4), 255-(i/5)));
				}
			}
			else{
				if (m_bFocus) {
					pBrush[i].CreateSolidBrush(RGB(255-(i/3), 255-(i/3), 255-(i/4)));
				}
				else{
					pBrush[i].CreateSolidBrush(RGB(255-(i/3), 255-(i/3), 255-(i/5)));
				}
			}
		}

		for (i=rect->top; i<=nHeight+2; i++) 
		{
			rct.SetRect (rect->left, i, nWidth+2, i+1);
			pDC->FillRect (&rct, pBrush[((i * 63) / nHeight)]);
		}
	
		for (i=0; i<64; i++)
		{
			pBrush[i].DeleteObject();
		}
	}

	void DrawInsideBorder(CDC *pDC,CRect* rect)
	{
		CPen *left, *right, *top, *bottom;
	
		if (m_bSelected && !m_bOver) {
			left = & m_InsideBoundryPenLeftSel;
			right = &m_InsideBoundryPenRightSel;
			top = &m_InsideBoundryPenTopSel;
			bottom = &m_InsideBoundryPenBottomSel;
		}
		else{
			left = &m_InsideBoundryPenLeft;
			right = &m_InsideBoundryPenRight;
			top = &m_InsideBoundryPenTop;
			bottom = &m_InsideBoundryPenBottom;
		}
	
		CPoint oldPoint = pDC->MoveTo(rect->left, rect->bottom-1);
		HPEN hOldPen = pDC->SelectPen(*left);
		pDC->LineTo(rect->left, rect->top+1);
		pDC->SelectPen(*right);
		pDC->MoveTo(rect->right-1,rect->bottom-1);
		pDC->LineTo(rect->right-1,rect->top);
		pDC->SelectPen(*top);
		pDC->MoveTo(rect->left-1,rect->top);
		pDC->LineTo(rect->right-1,rect->top);
		pDC->SelectPen(*bottom);
		pDC->MoveTo(rect->left,rect->bottom);
		pDC->LineTo(rect->right-1,rect->bottom);
		pDC->SelectPen(hOldPen);
		pDC->MoveTo(oldPoint);
	}

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
	{
		CDCHandle dc = lpDrawItemStruct->hDC;
		CRect &rect = (CRect&) lpDrawItemStruct->rcItem;
		CMemDC pDC(dc,NULL);
		UINT state = lpDrawItemStruct->itemState;
		POINT pt ;
		TCHAR szText[MAX_PATH+1];
		::GetWindowText(m_hWnd, szText, MAX_PATH);
		pt.x = 5;
		pt.y = 5;
		HPEN hOldPen = pDC.SelectPen(m_BoundryPen);
		pDC.RoundRect(&rect, pt);
	
		if (state & ODS_FOCUS) {
			m_bFocus = TRUE;
			m_bSelected = TRUE;
		}
		else{
			m_bFocus = FALSE;
			m_bSelected = FALSE;
		}
		if (state & ODS_SELECTED || state & ODS_DEFAULT){
			m_bFocus = TRUE;
		}
		pDC.SelectPen(hOldPen);

		rect.DeflateRect( CSize(GetSystemMetrics(SM_CXEDGE), GetSystemMetrics(SM_CYEDGE)));

		HBRUSH hOldBrush;
		if (m_bOver) {
			hOldBrush = pDC.SelectBrush(m_FillActive);
			DoGradientFill(&pDC, &rect);
		}
		else{
			hOldBrush = pDC.SelectBrush(m_FillInactive);
			DoGradientFill(&pDC, &rect);
		}
	
		if (m_bOver || m_bSelected) {
			DrawInsideBorder(&pDC, &rect);
		}

		pDC.SelectBrush(hOldBrush);

		if (szText!=NULL) {
			CSize Extent;
			HFONT hFont = AtlGetDefaultGuiFont();
			HFONT hOldFont = pDC.SelectFont(hFont);
			pDC.GetTextExtent(szText, lstrlen(szText) , &Extent);
			CPoint pt( rect.CenterPoint().x - Extent.cx/2, 
				rect.CenterPoint().y - Extent.cy/2 );
			if (state & ODS_SELECTED) 
				pt.Offset(1,1);
			int nMode = pDC.SetBkMode(TRANSPARENT);
			if (state & ODS_DISABLED)
				pDC.DrawState(pt, Extent, szText, DSS_DISABLED, TRUE, 0, (HBRUSH)NULL);
			else
				pDC.DrawState(pt, Extent, szText, DSS_NORMAL, TRUE, 0, (HBRUSH)NULL);
			pDC.SelectFont(hOldFont);
			pDC.SetBkMode(nMode);
		}
	}

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		DrawItem((LPDRAWITEMSTRUCT) lParam);
		return (LRESULT) TRUE;
	}
	
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (!m_bTracking) {
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = m_hWnd;
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.dwHoverTime = 1;
			m_bTracking = _TrackMouseEvent(&tme);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_bOver = FALSE;
		m_bTracking = FALSE;
		InvalidateRect(NULL, FALSE);
		return 0;
	}

	LRESULT OnMouseHover(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_bOver = TRUE;
		InvalidateRect(NULL);
		return 0;
	}
protected:
	CPen m_BoundryPen;
	
	CPen m_InsideBoundryPenLeft;
	CPen m_InsideBoundryPenRight;
	CPen m_InsideBoundryPenTop;
	CPen m_InsideBoundryPenBottom;

	CPen m_InsideBoundryPenLeftSel;
	CPen m_InsideBoundryPenRightSel;
	CPen m_InsideBoundryPenTopSel;
	CPen m_InsideBoundryPenBottomSel;

	CBrushHandle m_FillActive;
	CBrushHandle m_FillInactive;
	
	BOOL m_bOver;
	BOOL m_bTracking;
	BOOL m_bSelected;
	BOOL m_bFocus;
};

class CXPButton : public CXPButtonImpl<CXPButton>
{
public:
	DECLARE_WND_CLASS(_T("WTL_XPButton"))
};
/************************************************************************/
#endif // __ATLBTNXP_H__
