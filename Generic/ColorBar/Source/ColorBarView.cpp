#include "stdafx.h"
#include "ColorBarControl.h"
#include "ColorBarView.h"

CColorBarView::CColorBarView(CColorBarDoc *pDoc, CColorBarControl *pControl)
{
	m_pColorBarControl = pControl;
	m_pDoc             = pDoc;
	m_pTitleWnd        = new CToolTipWnd();
}

CColorBarView::~CColorBarView()
{
	if ( m_pColorBarControl )
		m_pColorBarControl = NULL;
	if ( m_pDoc )
		m_pDoc = NULL;
	
	if ( m_pTitleWnd)
	{	if ( m_pTitleWnd->IsWindow() )
		{
			m_pTitleWnd->DestroyWindow();
		}
		delete m_pTitleWnd;
		m_pTitleWnd = NULL;
	}
}

LRESULT CColorBarView::OnCreate(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL &bHandled)
{
	RECT rc,rc1;
	GetClientRect(&rc);
	
	rc1.left   = rc.left;
	rc1.top    = rc.top;
	rc1.right  = rc1.left + 100;
	rc1.bottom = rc1.top  + 40;
	
	DWORD dwStyle = WS_VISIBLE | WS_POPUP ;
	m_pTitleWnd->Create(*this,rc1,_T("Disp"),dwStyle,0);
	m_pTitleWnd->ShowWindow(SW_HIDE);

	/*
	m_DisplayTip.Attach(this->m_hWnd);
	m_DisplayTip.Activate(TRUE);
	m_DisplayTip.UpdateTipText(_T("Hello"),this->m_hWnd,1);
	m_DisplayTip.RelayEvent(NULL);
//	m_DisplayTip.ShowWindow(SW_SHOW);
	m_DisplayTip.ShowWindow(SW_HIDE);
	m_DisplayTip.Update();
		
	/*
	RECT rc;
	GetClientRect(&rc);
	m_DisplayTip.AddTool(this->m_hWnd,_T("Disp Dengyonghua"),&rc,1);
	m_DisplayTip.Activate(TRUE);
	m_DisplayTip.RelayEvent(NULL);
	m_DisplayTip.Update();
	*/
	return S_OK;
}

LRESULT CColorBarView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	::MoveWindow(this->m_hWnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);
	return S_OK;
}

LRESULT CColorBarView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	double dLength;
	dLength = m_pDoc->GetShowBarLength();

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
//	SetBkMode(hdc,OPAQUE);
	SetBkMode(hdc,TRANSPARENT);
	HBRUSH hBGBrush = CreateSolidBrush(RGB(255,255,255));
	HBRUSH hOldBGBrush = (HBRUSH)SelectObject(hdc,hBGBrush);
	FillRect(hdc,&rc,hBGBrush);
	SelectObject(hdc,hOldBGBrush);
	DeleteObject(hBGBrush);
	

	if ( dLength > 0.0 )
	{
		// 以下是画显示的区域图
		int iSize ; 
		int i ;
		HBRUSH hOldBrush,hBrush;
		HPEN hOldPen,hPen;
		COLORREF crColor;
		double dStartPos,dEndPos,dWidth;
		int  iStartPos,iEndPos;
		int  iXPos,iYPos,iWidth;
		
		iSize = m_pDoc->GetShowRegionDataVector().size();
		for ( i =0; i < iSize; i ++ )
		{
			crColor   = m_pDoc->GetShowRegionDataVector()[i].crColor;
			dStartPos = m_pDoc->GetShowRegionDataVector()[i].dStartPos;
			dEndPos   = m_pDoc->GetShowRegionDataVector()[i].dEndPos;
			dWidth    = dEndPos - dStartPos;

			iStartPos = m_pDoc->HourPosConvertToLPos(dStartPos);
			iEndPos   = m_pDoc->HourPosConvertToLPos(dEndPos);
			iWidth    = iEndPos - iStartPos;
			
			//创建画笔
			hPen      = CreatePen(PS_SOLID,0,crColor);
			hOldPen   = (HPEN)SelectObject(hdc,hPen);

			// 创建画刷
			hBrush    = CreateSolidBrush(crColor);
			hOldBrush = (HBRUSH)SelectObject(hdc,hBrush);
			
			//画区域框架图
			iXPos = rc.left + iStartPos;
			iYPos = rc.top ;
			Rectangle(hdc,iXPos,iYPos,iXPos+iWidth,rc.bottom);

			SelectObject(hdc,hOldBrush);
			DeleteObject(hBrush);

			SelectObject(hdc,hOldPen);
			DeleteObject(hPen);

			
			// for the test how to draw text 
			/*
			TCHAR strText[1024];
			CComBSTR bstrTemp;
			USES_CONVERSION;
		
			SetTextColor(hdc,RGB(0,0,0));
			bstrTemp = m_pDoc->GetShowRegionDataVector()[i].bstrFilmName;
			wsprintf(strText,OLE2T(bstrTemp));
			TextOut(hdc,iXPos,iYPos+(rc.bottom-rc.top)/2,strText,lstrlen(strText));
			*/
		}
		// 以上是画显示的区域图

		// 以下是画用户竖线
		MAPITERTMP itorTmp;
		for ( itorTmp = m_pDoc->GetUserLineDataMap().begin(); itorTmp != m_pDoc->GetUserLineDataMap().end();  itorTmp++)
		{
			crColor   = (*itorTmp).second.crColor;
			dStartPos = (*itorTmp).second.dCurPos;

			hPen      = CreatePen(PS_SOLID,0,crColor);
			hOldPen   = (HPEN)SelectObject(hdc,hPen);
			iXPos     = rc.left + m_pDoc->HourPosConvertToLPos(dStartPos);
			iYPos     = rc.top;
		
			if ( iXPos >= rc.left && iStartPos <= rc.right )
			{
				MoveToEx(hdc,iXPos,rc.top,(LPPOINT)NULL);
				LineTo(hdc,iXPos,rc.bottom);
			}
			SelectObject(hdc,hOldPen);
			DeleteObject(hPen);
		}
		// 以上是画用户竖线
	}

	//将内存中的图拷贝到屏幕上进行显示
	BitBlt(Readhdc,0,0,rc.right - rc.left, rc.bottom - rc.top, hdc, 0,0, SRCCOPY); //还是放在外面

	//绘图完成后的清理
	SelectObject(hdc,hOldBitmap);
	DeleteDC(hdc);
	DeleteObject(hBitmap);
	EndPaint(&ps);
	return S_OK;
}

LRESULT CColorBarView::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
//	bHandled = FALSE;
	bHandled = TRUE;
	return S_OK;
}

void CColorBarView::Repaint()
{
	RECT rc;
	GetClientRect(&rc);
	InvalidateRect(&rc,TRUE);
	this->RedrawWindow();
}

CComBSTR  CColorBarView::GetFilmName(UINT iXpos)              //根据当前鼠标的X坐标位置来获取当前的节目名
{
	int iSize;
	int i;
	int iXPosStartTemp,iXPosEndTemp;
	double dStartPos,dEndPos;
	CComBSTR bstrReturn(_T(""));
	
	RECT rc;
	iSize = m_pDoc->GetShowRegionDataVector().size();
	GetClientRect(&rc);
	
	for ( i =0; i < iSize; i ++ )
	{
		dStartPos = m_pDoc->GetShowRegionDataVector()[i].dStartPos;
		dEndPos   = m_pDoc->GetShowRegionDataVector()[i].dEndPos;
		
		iXPosStartTemp = rc.left + m_pDoc->HourPosConvertToLPos(dStartPos);
		iXPosEndTemp   = rc.left + m_pDoc->HourPosConvertToLPos(dEndPos);

		if ( iXpos >= iXPosStartTemp && iXpos <= iXPosEndTemp )
		{
			bstrReturn = m_pDoc->GetShowRegionDataVector()[i].bstrFilmName;
			break;
		}
	}
	return bstrReturn;
}

/*
// for the log
#include <string>
#include <fstream>
using std::string;
std::ofstream osfile("Test.log");
*/
	
LRESULT CColorBarView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
//	Sleep(150); //延迟1.5秒，与标准的ToolTips控件延迟时间一致

    if ( !m_pDoc->GetShowRegionDataVector().empty())
	{
		POINT p1,p2,p3; // 
		CComBSTR bstrFilmName;

		p1.x = LOWORD(lParam);
		p1.y = HIWORD(lParam);
		GetCursorPos(&p3);
		ScreenToClient(&p3);
		SetCapture(); //扑捉鼠标
	//	ScreenToClient(&p1); 因为从lParam参数中获取的是客户区的坐标，所以不用调ScreenToClient 这个API函数
		RECT rcClient;
		GetClientRect(&rcClient);
		if ( ( p1.x >= rcClient.left && p1.x <= rcClient.right ) && ( p1.y >= rcClient.top && p1.y <= rcClient.bottom ) )
		{
			p2.x = rcClient.left + p1.x;
			p2.y = rcClient.top  + p1.y;
			bstrFilmName = GetFilmName(p2.x);

			ClientToScreen(&p2); // 这时需要把客户区坐标转换成屏幕的坐标
			p3.x = p2.x;
			p3.y = p2.y + 25; 
			
			HDC hdc;
			hdc =GetDC();
			
			TEXTMETRIC tm;
			GetTextMetrics(hdc,&tm);
			UINT iHight  = tm.tmHeight + 4;
			m_pTitleWnd->SetFilmName(bstrFilmName);


			USES_CONVERSION;
			UINT iWidth;

			SIZE sz;
			GetTextExtentPoint32(hdc, OLE2T(bstrFilmName.m_str),_tcslen(OLE2T(bstrFilmName.m_str)), &sz); 
			iWidth = sz.cx + 4;
			iHight = sz.cy + 2;

			ReleaseDC(hdc);
			m_pTitleWnd->MoveWindow(p3.x,p3.y,iWidth,iHight,TRUE);
			m_pTitleWnd->ShowWindow(SW_SHOW);

			/*
			string strTemp;
			char s1[20];
			strTemp  ="Pt.x=";
			itoa(p2.x,s1,10);
			
			strTemp +=s1;
			strTemp +="Pt.y=";
			itoa(p2.y,s1,10);
			
			strTemp +=s1;
			strTemp +="FilmName=";
			memset(s1,0,sizeof(s1));
			strcpy(s1,(char*)OLE2T(bstrFilmName.m_str));
			strTemp +=s1;

//			osfile << strTemp.c_str();
//			MessageBox(OLE2T(bstrFilmName.m_str));
*/
		

		}
		else
		{
			ReleaseCapture(); //释放扑捉鼠标
			m_pTitleWnd->ShowWindow(SW_HIDE);
		}

		


	}
	return S_OK;
}

LRESULT CColorBarView::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL &bHanded)
{

	if ( !m_pDoc->GetShowRegionDataVector().empty())
	{
		POINT p1,p2,p3; // 
		CComBSTR bstrFilmName;

		p1.x = LOWORD(lParam);
		p1.y = HIWORD(lParam);
		GetCursorPos(&p3);
		ScreenToClient(&p3);
		SetCapture(); //扑捉鼠标
	//	ScreenToClient(&p1); 因为从lParam参数中获取的是客户区的坐标，所以不用调ScreenToClient 这个API函数
		RECT rcClient;
		GetClientRect(&rcClient);
		if ( ( p1.x >= rcClient.left && p1.x <= rcClient.right ) && ( p1.y >= rcClient.top && p1.y <= rcClient.bottom ) )
		{
			p2.x = rcClient.left + p1.x;
			p2.y = rcClient.top  + p1.y;
			bstrFilmName = GetFilmName(p2.x);

			ClientToScreen(&p2); // 这时需要把客户区坐标转换成屏幕的坐标
			p3.x = p2.x;
			p3.y = p2.y + 25; 
			
			HDC hdc;
			hdc =GetDC();
			
			TEXTMETRIC tm;
			GetTextMetrics(hdc,&tm);
			UINT iHight  = tm.tmHeight + 4;
			m_pTitleWnd->SetFilmName(bstrFilmName);


			USES_CONVERSION;
			UINT iWidth;

			SIZE sz;
			GetTextExtentPoint32(hdc, OLE2T(bstrFilmName.m_str),_tcslen(OLE2T(bstrFilmName.m_str)), &sz); 
			iWidth = sz.cx + 4;
			iHight = sz.cy + 2;

			ReleaseDC(hdc);
			m_pTitleWnd->MoveWindow(p3.x,p3.y,iWidth,iHight,TRUE);
			m_pTitleWnd->ShowWindow(SW_SHOW);

			/*
			string strTemp;
			char s1[20];
			strTemp  ="Pt.x=";
			itoa(p2.x,s1,10);
			
			strTemp +=s1;
			strTemp +="Pt.y=";
			itoa(p2.y,s1,10);
			
			strTemp +=s1;
			strTemp +="FilmName=";
			memset(s1,0,sizeof(s1));
			strcpy(s1,(char*)OLE2T(bstrFilmName.m_str));
			strTemp +=s1;

//			osfile << strTemp.c_str();
//			MessageBox(OLE2T(bstrFilmName.m_str));
*/
		

		}
		else
		{
			ReleaseCapture(); //释放扑捉鼠标
			m_pTitleWnd->ShowWindow(SW_HIDE);
		}

		


	}
	return S_OK;
}