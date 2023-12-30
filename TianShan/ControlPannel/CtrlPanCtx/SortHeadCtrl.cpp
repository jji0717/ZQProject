// SortHeadCtrl.cpp: implementation of the CSortHeadCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "SortHeadCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSortHeadCtrl::CSortHeadCtrl( )
{
	m_iSortColumn = 0;
	m_bSortAscending = TRUE;
	m_bmpArrowDown.LoadBitmap(IDB_ARROWDOWN);
	m_bmpArrowUp.LoadBitmap(IDB_ARROWUP);
	
}

CSortHeadCtrl::~CSortHeadCtrl()
{
	m_bmpArrowUp.DeleteObject();
	m_bmpArrowDown.DeleteObject();
}

LRESULT CSortHeadCtrl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{	
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
	{
		/*
		CBitmap pTempBmp ;
		pTempBmp.LoadBitmap(IDB_LEVEL);
		
		HD_ITEM Item;	
		Item.mask = HDI_FORMAT;
		GetItem( 0, &Item );
		Item.mask = HDI_BITMAP | HDI_FORMAT;	
		Item.fmt |= HDF_BITMAP   ;
		Item.hbm = (HBITMAP)pTempBmp.m_hBitmap;
		InsertItem( 0, &Item );

		memset(&Item,0,sizeof(Item));
		Item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		Item.fmt  = LVCFMT_LEFT;
		Item.cchTextMax = 100;
		Item.pszText=_T("DateTime");
		InsertItem(1,&Item);

		Item.pszText=_T("Category");
		InsertItem(2,&Item);

		Item.pszText=_T("Message");
		InsertItem(3,&Item);
		*/
	}
	return lRet;
}
void CSortHeadCtrl::SetSortArrow( const int iColumn, const BOOL bAscending )
{
		m_iSortColumn = iColumn;
		m_bSortAscending = bAscending;

		// change the item to owner drawn.
		HD_ITEM hditem;
		hditem.mask = HDI_FORMAT;
		GetItem( iColumn, &hditem ) ;
		hditem.fmt |= HDF_OWNERDRAW;
		SetItem( iColumn, &hditem ) ;
		Invalidate();
	
		::UpdateWindow(m_hWnd);

}

void CSortHeadCtrl::RemoveAllSortImages()
{
		int iCount = GetItemCount();
		for( int i = 0; i < iCount; i++ )
		{
			RemoveSortImage( i );
		}

}

void CSortHeadCtrl::RemoveSortImage( int iItem )
{
		if( iItem != -1 )
		{
			HD_ITEM hditem;	
			hditem.mask = HDI_FORMAT;
			GetItem( iItem, &hditem );
			hditem.mask = HDI_FORMAT;	
			hditem.fmt &= ~HDF_BITMAP;
			SetItem( iItem, &hditem );
		}
}

void CSortHeadCtrl::SetHeadItem( )
{
		CBitmap pTempBmp;
		pTempBmp.LoadBitmap(IDB_LEVEL);
			
		HD_ITEM Item;	
		Item.mask = HDI_FORMAT;
		GetItem( 0, &Item );
		Item.mask = HDI_BITMAP | HDI_FORMAT;	
		Item.fmt |= HDF_BITMAP   ;
		Item.hbm = (HBITMAP)pTempBmp.m_hBitmap;
		SetItem( 0, &Item );
}

void CSortHeadCtrl::SetHeadItem( int iSubItem,const BOOL bAscending )
{

		CBitmap *pTempBmp = NULL;
		if( bAscending )
			pTempBmp = &m_bmpArrowUp;
		else
			pTempBmp = &m_bmpArrowDown;
		
		HD_ITEM Item;	
		Item.mask = HDI_FORMAT;
		GetItem( iSubItem, &Item );
		Item.mask = HDI_BITMAP | HDI_FORMAT;	
		Item.fmt |= HDF_BITMAP   ;
		Item.hbm = (HBITMAP)pTempBmp->m_hBitmap;
		SetItem( iSubItem, &Item );

		m_iSortColumn = iSubItem;
		m_bSortAscending = bAscending;
}

LRESULT CSortHeadCtrl::OnDrawItem(UINT , WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{

	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)lParam;
	CDCHandle dc   = lpDrawItemStruct->hDC;
	CDCHandle* pDC = &dc;

	// Get the column rect
	CRect rcLabel(lpDrawItemStruct->rcItem);
	
	// Draw the background
	CBrush brush(CreateSolidBrush(GetSysColor( COLOR_3DFACE )));
	pDC->FillRect(rcLabel,brush.m_hBrush);

	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	
	SIZE sizeValue;
	pDC->GetTextExtent(_T(" "), 1,&sizeValue);
	int offset = sizeValue.cx*2;

	// Get the column text and format
	TCHAR buf[256];
	HD_ITEM hditem;	

	hditem.mask = HDI_TEXT | HDI_FORMAT;
	hditem.pszText = buf;
	hditem.cchTextMax = 255;
	
	GetItem(lpDrawItemStruct->itemID, &hditem);

	// Determine format for drawing column label
	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS ;

	if (hditem.fmt & HDF_CENTER)
		uFormat |= DT_CENTER;
	else if (hditem.fmt & HDF_RIGHT)
		uFormat |= DT_RIGHT;
	else
		uFormat |= DT_LEFT;

	// Adjust the rect if the mouse button is pressed on it
	if (lpDrawItemStruct->itemState == ODS_SELECTED)
	{
		rcLabel.left++;
		rcLabel.top += 2;
		rcLabel.right++;
	}

	// Adjust the rect further if Sort arrow is to be displayed
	if (lpDrawItemStruct->itemID == static_cast<UINT>(m_iSortColumn))
	{
		rcLabel.right -= 3 * offset;
	}
	
	rcLabel.left += offset;
	rcLabel.right -= offset;
	
	// Draw column label
	pDC->DrawText(buf, -1, rcLabel, uFormat);
	
	
	// Draw the Sort arrow
	if (lpDrawItemStruct->itemID == static_cast<UINT>(m_iSortColumn))
	{
		CRect rcIcon(lpDrawItemStruct->rcItem);

		
		// Set up pens to use for drawing the triangle
		CPen penLight;
		penLight.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
		
		CPen penShadow;
		penShadow.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		HPEN pOldPen = pDC->SelectPen(penLight.m_hPen);
						
		if (m_bSortAscending)
		{
			// Draw triangle pointing upwards
			pDC->MoveTo(rcIcon.right - 2*offset, offset - 1);
			pDC->LineTo(rcIcon.right - 3*offset/2, rcIcon.bottom - offset);
			pDC->LineTo(rcIcon.right - 5*offset/2-2, rcIcon.bottom - offset);
			pDC->MoveTo(rcIcon.right - 5*offset/2-1, rcIcon.bottom - offset - 1);

			pDC->SelectPen(penShadow.m_hPen);
			pDC->LineTo(rcIcon.right - 2*offset, offset-2);
		}
		else	
		{
			// Draw triangle pointing downwards
			pDC->MoveTo(rcIcon.right - 3*offset/2, offset-1);
			pDC->LineTo(rcIcon.right - 2*offset-1, rcIcon.bottom - offset + 1);
			pDC->MoveTo(rcIcon.right - 2*offset-1, rcIcon.bottom - offset);

			pDC->SelectPen(penShadow.m_hPen);
			pDC->LineTo(rcIcon.right - 5*offset/2-1, offset - 1);
			pDC->LineTo(rcIcon.right - 3*offset/2, offset - 1);
		}
		
		// Restore the pen
		pDC->SelectPen(pOldPen);
		penLight.DeleteObject();
		penShadow.DeleteObject();
		
	}
	return S_OK;
}
