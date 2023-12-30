// ColorBarControl.cpp : Implementation of CColorBarControl

#include "stdafx.h"
#include "ColorBar.h"
#include "ColorBarControl.h"

/////////////////////////////////////////////////////////////////////////////
// CColorBarControl
CColorBarControl::CColorBarControl()
{
	m_bWindowOnly = TRUE;
	this->m_pDoc  = new CColorBarDoc(this);
	this->m_pView = new CColorBarView(m_pDoc,this);
}

CColorBarControl::~CColorBarControl()
{
	if ( m_pDoc )
	{
		delete m_pDoc;
		m_pDoc = NULL;
	}
	if ( m_pView )
	{
		delete m_pView;
		m_pView = NULL;
	}
}

LRESULT CColorBarControl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	RECT rcClient;
	GetClientRect(&rcClient);
//    RECT rc;
//	rc.left   = rcClient.left;rc.right  = rcClient.left + iControlWidth;rc.top    = rcClient.top;rc.bottom = rcClient.top  + iControlHigth;
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS;
//	this->m_pView->Create(*this,rc,_T("Test ATL"),dwStyle,0);
	this->m_pView->Create(*this,rcClient,_T("Test ATL"),dwStyle,0);
	return S_OK;
}

LRESULT CColorBarControl::OnSize(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL & bHandled)
{
	RECT rcClient;
	GetClientRect(&rcClient);
	
//	RECT rc; rc.left   = rcClient.left;rc.right  = rcClient.left + iControlWidth;rc.top    = rcClient.top;rc.bottom = rcClient.top  + iControlHigth;
//	::SetWindowPos(*m_pView,HWND_TOP,rc.left,rc.top,rc.right - rc.left,rc.bottom - rc.top,SWP_SHOWWINDOW);
	::SetWindowPos(*m_pView,HWND_TOP,rcClient.left,rcClient.top,rcClient.right - rcClient.left,rcClient.bottom - rcClient.top,SWP_SHOWWINDOW);
	int ITemp = (rcClient.right - rcClient.left );
	m_pDoc->SetBarWidthPixes(ITemp);
	ITemp      = (UINT)(rcClient.bottom - rcClient.top);
	m_pDoc->SetBarHightPixes(ITemp);
	m_pDoc->ReCacluteLenTimeRatio();
	return S_OK;
}

LRESULT CColorBarControl::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bHandled = FALSE;
	return S_OK;
}

LRESULT CColorBarControl::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bHandled = TRUE;
	return S_OK;
}

STDMETHODIMP CColorBarControl::FillColor( COLORREF crColor, double dStartPos,double dEndPos, BSTR bstrName )
{
	if ( crColor < 0 || dStartPos < 0.00 || dEndPos < 0.00 )
	{
		ATLTRACE(_T("参数设置非法,不进行该操作"));
//		MessageBox(_T("参数设置非法,不进行该操作"),_T("提示"),MB_OK); //注意，由于MessageBox要用户进行操作，因而不便于用户操作
		return S_FALSE;
	}
	if ( IsWindow() )
	{
		if ( dEndPos != dStartPos )
		{
			VECTORTYPE  DataTmp;
			memset((void*)&DataTmp,0,sizeof(DataTmp));

// 		    _bstr_t strname = _bstr_t(bstrName); 
//			lstrcpy(DataTmp.sFilmName,(TCHAR*)strname);

			DataTmp.bstrFilmName = bstrName;
			DataTmp.crColor      = crColor;
			DataTmp.dStartPos    = dStartPos;
			DataTmp.dEndPos      = dEndPos;
			m_pDoc->SetFillRegionDataVector(DataTmp);
//			SysFreeString(strname);
		}
	}
	return S_OK;
}

STDMETHODIMP CColorBarControl::ShowRange( double dStartPos, double dEndPos)
{
	if ( dStartPos < 0.00 || dEndPos < 0.00 )
	{
		ATLTRACE(_T("参数设置非法,不进行该操作")); //注意，由于MessageBox要用户进行操作，因而不便于用户操作
//		MessageBox(_T("参数设置非法,不进行该操作"),_T("提示"),MB_OK);
		return S_FALSE;
	}
	if ( IsWindow() )
	{
		if ( dEndPos != dStartPos) 
		{
			double dTemp;
			dTemp = m_pDoc->GetFillBarStartPos();
			if ( dStartPos < dTemp)
			{
				dStartPos = dTemp;
			}
			dTemp = m_pDoc->GetFillBarEndPos();
			if ( dEndPos > dTemp )
			{
				dEndPos = dTemp;
			}
			SHOWREGIONSTRUCT pTemp;
			memset((void*)&pTemp,0,sizeof(pTemp));
			pTemp.dStartPos = dStartPos;
			pTemp.dEndPos   = dEndPos;
			m_pDoc->SetShowRegionData(pTemp);
			m_pView->Repaint();
			RECT rc;
			GetClientRect(&rc);
			InvalidateRect(&rc,TRUE);
			FireViewChange();
		}
	}
	return S_OK;
}

STDMETHODIMP CColorBarControl::DeleteCursor(int  pID)
{
	int iUserNum;
	iUserNum = m_pDoc->GetiCurUserNum();
	if ( iUserNum > 0 )
	{
		m_pDoc->ResetSetUserLineDataMap(pID);
	}
	return S_OK;
}

STDMETHODIMP CColorBarControl::CreateCursor(COLORREF crColor, int *pID)
{
	if ( crColor < 0 )
	{
		*pID = -1;
		ATLTRACE(_T("参数设置非法,不进行该操作")); //注意，由于MessageBox要用户进行操作，因而不便于用户操作
//		MessageBox(_T("参数设置非法,不进行该操作"),_T("提示"),MB_OK);
		return S_FALSE;
	}

	bool bIsExist;
	int iUserNum;
	int iIDTmp;
	iUserNum = m_pDoc->GetiCurUserNum();
	if ( iUserNum > (int)iMaxUserNum )
	{
		ATLTRACE(_T("创建的用户数超过最大数:150,不能创建新的用户")); //注意，由于MessageBox要用户进行操作，因而不便于用户操作
//		MessageBox(_T("创建的用户数超过最大数:150,不能创建新的用户"),_T("提示"),MB_OK);
		*pID = 0;
		return S_FALSE;
	}
	bIsExist = m_pDoc->bIsExistInMap(crColor,&iIDTmp);
	if ( bIsExist ) //该颜色用户已存在,则不创建新的用户ID值,仍使用老用户的ID值.
	{
		*pID = iIDTmp;
	}
	else
	{
		iUserNum ++;
		*pID = iUserNum;

		USERDATASTRUCT  mapDataTmp;
		memset((void*)&mapDataTmp,0,sizeof(mapDataTmp));

		mapDataTmp.crColor = crColor;
		mapDataTmp.dCurPos = 0.0; //设在默认值
		m_pDoc->SetUserLineDataMap(iUserNum,mapDataTmp);
	}
	return S_OK;
}

STDMETHODIMP CColorBarControl::DrawLine( int iID, double dCurPos)
{
	if ( m_pDoc->GetUserLineDataMap().empty()) //如果用户画线的MAP容器为空，即如果还没有创建用户Cursor的操作
	{
	   ATLTRACE(_T("还没有创建用户CursorID,所以不能进行画线操作")); //注意，由于MessageBox要用户进行操作，因而不便于用户操作
//	   MessageBox(_T("还没有创建用户CursorID,所以不能进行画线操作"),_T("提示"),MB_OK);
	   return S_FALSE;
	}
	if ( iID  < 0 || dCurPos < 0.00 )
    {
		ATLTRACE(_T("参数设置非法,不进行该操作")); //注意，由于MessageBox要用户进行操作，因而不便于用户操作
//		MessageBox(_T("参数设置非法,不进行该操作"),_T("提示"),MB_OK);
		return S_FALSE;
    }
    if ( IsWindow() )
    {
	   // 如果用户ID号小于MAP表中
	   int iIDTemp;
	   iIDTemp = m_pDoc->GetMapStartValue();
	   if ( iID < iIDTemp ) 
	   {
		   iID = iIDTemp;
	   }
	   iIDTemp = m_pDoc->GetMapEndValue();
	   if ( iID > iIDTemp )
	   {
		   iID = iIDTemp;
	   }
	   // 如果小于显示区域的起始位置，则设置显示区域的起始位置
	   if ( dCurPos < m_pDoc->GetShowRegionData().dStartPos )
	   {
		   dCurPos = m_pDoc->GetShowRegionData().dStartPos;
	   }
	   // 如果大于显示区域的结束位置，则设置显示区域的结束位置
	   if ( dCurPos > m_pDoc->GetShowRegionData().dEndPos )
	   {
		   dCurPos = m_pDoc->GetShowRegionData().dEndPos ;
	   }

	   USERDATASTRUCT  mapDataTmp;
	   memset((void*)&mapDataTmp,0,sizeof(mapDataTmp));

	   MAPITERTMP itorTmp;
	   itorTmp    = m_pDoc->GetUserLineDataMap().find(iID);
	   mapDataTmp = itorTmp->second;
	   mapDataTmp.crColor = mapDataTmp.crColor;
	   mapDataTmp.dCurPos = dCurPos; //重设该用户线的时间点值
	   m_pDoc->SetUserLineDataMap(iID,mapDataTmp);
  	  
	   m_pView->Repaint();
	   RECT rc;
	   GetClientRect(&rc);
	   InvalidateRect(&rc,TRUE);
	   FireViewChange();
	}
	return S_OK;
}

STDMETHODIMP CColorBarControl::DeletePaint()
{
	if ( IsWindow() )
	{
	   double dTemp = 0.0;
	   m_pDoc->SetShowBarLength(dTemp);
	   if ( !m_pDoc->GetFillRegionDataVector().empty())
	   {
		   m_pDoc->GetFillRegionDataVector().clear();
	   }
	   if ( !m_pDoc->GetUserLineDataMap().empty())
	   {
		   m_pDoc->GetUserLineDataMap().clear();
	   }
	   m_pView->Repaint();
	   RECT rc;
	   GetClientRect(&rc);
	   InvalidateRect(&rc,TRUE);
	   FireViewChange();
	}
	return S_OK;
}

