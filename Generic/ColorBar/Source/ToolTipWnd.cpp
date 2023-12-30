// ToolTipWnd.cpp: implementation of the ToolTipWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolTipWnd.h"
#include <comdef.h>      // ��_bstr_t�Ķ����ļ�

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
	// �ҵĻ���
	PAINTSTRUCT ps;
	HDC Readhdc = BeginPaint(&ps); //���ȶ���һ����ʾ�豸����
	HDC hdc     = CreateCompatibleDC(Readhdc); //���������Ļ��ʾ���ݵ��ڴ���ʾ�豸
	RECT rc;
	GetClientRect(&rc);
	HBITMAP hBitmap    = CreateCompatibleBitmap(Readhdc,rc.right - rc.left, rc.bottom - rc.top ); ////����һ��λͼ����,�潨��һ������Ļ��ʾ���ݵ�λͼ������λͼ�Ĵ�С������ô��ڵĴ�С
	HBITMAP hOldBitmap = (HBITMAP) SelectObject(hdc,hBitmap); ////��λͼѡ�뵽�ڴ���ʾ�豸��
	//ֻ��ѡ����λͼ���ڴ���ʾ�豸���еط���ͼ������ָ����λͼ��

	// �����ɫ,//���ñ���ɫ��λͼ����ɾ�
	// ������
	SetBkMode(hdc,TRANSPARENT);
	HBRUSH hBGBrush = CreateSolidBrush(RGB(255,255,240));
	HBRUSH hOldBGBrush = (HBRUSH)SelectObject(hdc,hBGBrush);
	FillRect(hdc,&rc,hBGBrush);
	
	// �����ο�
	HPEN hOldPen,hPen;
	hPen      = CreatePen(PS_SOLID,1,RGB(0,0,0));
	hOldPen   = (HPEN)SelectObject(hdc,hPen);
	Rectangle(hdc,rc.left,rc.top,rc.right,rc.bottom );
	
	SelectObject(hdc,hOldPen);
	DeleteObject(hPen);
	SetBkMode(hdc,TRANSPARENT);


//	HFONT hFont,hOldFont; //��������
//  hFont = (HFONT)GetStockObject(SYSTEM_FONT); 
//	hOldFont = (HFONT)SelectObject(hdc,hFont);
   

	SetTextColor(hdc,RGB(0,0,0));
	TCHAR strText[1024];
	USES_CONVERSION;
	wsprintf(strText,OLE2T(m_bstrFilmName.m_str));
//	DrawText(hdc,strText,lstrlen(strText),&rc,DT_LEFT); //�������
	DrawText(hdc,strText,lstrlen(strText),&rc,DT_CENTER|DT_VCENTER);//���ж���
//	TextOut(hdc,rc.left,rc.top,strText,lstrlen(strText));
	
	//���ڴ��е�ͼ��������Ļ�Ͻ�����ʾ
	BitBlt(Readhdc,0,0,rc.right - rc.left, rc.bottom - rc.top, hdc, 0,0, SRCCOPY); //���Ƿ�������

//	SelectObject(hdc,hOldFont);
//	DeleteObject(hFont);

	SelectObject(hdc,hOldBGBrush);
	DeleteObject(hBGBrush);

	//��ͼ��ɺ������
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