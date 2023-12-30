// ToolTipWnd.cpp: implementation of the ToolTipWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTipWnd.h"
#include <comdef.h>      // 是_bstr_t的定义文件

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CToolTipWnd::CToolTipWnd()
{
	this->m_iWndHight = 30;
	this->m_iWndWidth = 100;
	m_bstrFilmName = CComBSTR(_T(""));
}

CToolTipWnd::~CToolTipWnd()
{

}

LRESULT CToolTipWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	// 我的画法
	PAINTSTRUCT ps;
	HDC Readhdc = BeginPaint(&ps); //首先定义一个显示设备对象
	HDC hdc     = CreateCompatibleDC(Readhdc); //随后建立与屏幕显示兼容的内存显示设备
	RECT rc;
	GetClientRect(&rc);
	HBITMAP hBitmap    = CreateCompatibleBitmap(Readhdc,rc.right - rc.left, rc.bottom - rc.top ); ////定义一个位图对象,面建立一个与屏幕显示兼容的位图，至于位图的大小嘛，可以用窗口的大小
	HBITMAP hOldBitmap = (HBITMAP) SelectObject(hdc,hBitmap); ////将位图选入到内存显示设备中
	//只有选入了位图的内存显示设备才有地方绘图，画到指定的位图上

	// 清楚底色,//先用背景色将位图清除干净
	// 画背景
	SetBkMode(hdc,TRANSPARENT);
	HBRUSH hBGBrush = CreateSolidBrush(RGB(255,255,240));
	HBRUSH hOldBGBrush = (HBRUSH)SelectObject(hdc,hBGBrush);
	FillRect(hdc,&rc,hBGBrush);
	
	// 画矩形框
	HPEN hOldPen,hPen;
	hPen      = CreatePen(PS_SOLID,1,RGB(0,0,0));
	hOldPen   = (HPEN)SelectObject(hdc,hPen);
	Rectangle(hdc,rc.left,rc.top,rc.right,rc.bottom );
	
	SelectObject(hdc,hOldPen);
	DeleteObject(hPen);
	SetBkMode(hdc,TRANSPARENT);


//	HFONT hFont,hOldFont; //设置字体
//  hFont = (HFONT)GetStockObject(SYSTEM_FONT); 
//	hOldFont = (HFONT)SelectObject(hdc,hFont);
   

	SetTextColor(hdc,RGB(0,0,0));
	TCHAR strText[1024];
	USES_CONVERSION;
	wsprintf(strText,OLE2T(m_bstrFilmName.m_str));
//	DrawText(hdc,strText,lstrlen(strText),&rc,DT_LEFT); //向左对齐
	DrawText(hdc,strText,lstrlen(strText),&rc,DT_CENTER|DT_VCENTER);//居中对齐
//	TextOut(hdc,rc.left,rc.top,strText,lstrlen(strText));
	
	//将内存中的图拷贝到屏幕上进行显示
	BitBlt(Readhdc,0,0,rc.right - rc.left, rc.bottom - rc.top, hdc, 0,0, SRCCOPY); //还是放在外面

//	SelectObject(hdc,hOldFont);
//	DeleteObject(hFont);

	SelectObject(hdc,hOldBGBrush);
	DeleteObject(hBGBrush);

	//绘图完成后的清理
	SelectObject(hdc,hOldBitmap);
	DeleteDC(hdc);
	DeleteObject(hBitmap);
	EndPaint(&ps);
	return S_OK;
}

LRESULT CToolTipWnd::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = TRUE;
	return S_OK;
}

UINT    CToolTipWnd::GetWndWidth() const
{
	return this->m_iWndWidth;
}

UINT    CToolTipWnd::GetWndHight() const
{
	return this->m_iWndHight;
}

CComBSTR & CToolTipWnd::GetFilmName()
{
	return this->m_bstrFilmName;
}

void  CToolTipWnd::SetWndWidth( UINT iVal ) 
{
	this->m_iWndWidth = iVal;
}

void  CToolTipWnd::SetWndHight(UINT iVal)
{
	this->m_iWndHight = iVal;
}

void   CToolTipWnd::SetFilmName(CComBSTR & bstrFilmName)
{
	this->m_bstrFilmName = bstrFilmName;

}

LRESULT CToolTipWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	RECT rc,rc1;
	GetClientRect(&rc);
	rc1.left   = rc.left;
	rc1.top    = rc.top;
	rc1.right  = rc1.left + m_iWndWidth;
	rc1.bottom = rc1.top  + m_iWndHight; 
	::MoveWindow(this->m_hWnd,rc1.left,rc1.top,rc1.right-rc1.left,rc1.bottom-rc1.top,TRUE);
	InvalidateRect(&rc1,TRUE);
	return S_OK;
}