// WTLTabViewCtrl.cpp: implementation of the CWTLTabViewCtrl class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "WTLTabViewCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWTLTabViewCtrl::CWTLTabViewCtrl()
{
	m_TABVIEW_BORDER   = 3;
	m_TABVIEW_EDGE	   = 5;
    m_ActiveTabIndex   = -1;
}

CWTLTabViewCtrl::~CWTLTabViewCtrl()
{
	if ( m_Views.GetSize() >0 )
	{
		m_Views.RemoveAll();
	}
}

BOOL  CWTLTabViewCtrl::AddTab( LPCTSTR inLabel, HWND inTabWindow, BOOL inActiveFlag, int inImage , LPARAM inParam )
{
		BOOL	theReturn;
		CWindow theTabWindow( inTabWindow );

		// Make sure it's a real window
		ATLASSERT( theTabWindow.IsWindow( ) );

		// Make sure it's a child window and is not visible
		ATLASSERT( ( theTabWindow.GetStyle() & WS_CHILD ) != 0 );
		ATLASSERT( ( theTabWindow.GetStyle() & WS_VISIBLE ) == 0 );

		// Hide the view window
		theTabWindow.EnableWindow( FALSE );
		theTabWindow.ShowWindow( SW_HIDE );

		// Store the required data for the list
		m_Views.Add( theTabWindow );

		// Add the tab to the tab control
		TC_ITEM theItem = {0};
		theItem.mask = TCIF_TEXT;
		theItem.pszText = const_cast<char*>( inLabel );

		// Add an image for the tab
		if ( inImage != -1 )
		{
			theItem.mask |= TCIF_IMAGE;
			theItem.iImage = inImage;
		}

		// Add the param for the tab
		if ( inParam != 0 )
		{
			theItem.mask |= TCIF_PARAM;
			theItem.lParam = inParam;
		}

		// Insert the item at the end of the tab control
		theReturn = InsertItem( 32768, &theItem );

		// Set the position for the window
		CRect rcChild;
		CalcViewRect( &rcChild );
		theTabWindow.MoveWindow( rcChild );

		// Select the tab that is being added, if desired
		LONG theTabIndex = GetTabCount( ) - 1; 
		if ( inActiveFlag || theTabIndex == 0 )
		{
			SetActiveTab( theTabIndex );
		}
		return theReturn;
}

LONG CWTLTabViewCtrl::GetTabCount()
{
	return m_Views.GetSize();
}

HWND CWTLTabViewCtrl::GetTab( int inTab )
{
	HWND theTabHwnd = NULL;
	if ( inTab >= 0 && inTab < GetTabCount( ) )
	{
		m_Views[ inTab ];
	}
	return theTabHwnd;
}

void CWTLTabViewCtrl::OnTabRemoved( int inTabIndex )
{
	UNREFERENCED_PARAMETER( inTabIndex );
}

HWND CWTLTabViewCtrl::RemoveTab( int inTab )
{	
	HWND theTabHwnd = NULL;
	LONG theNewTab = -1;

	if ( inTab >= 0 && inTab < GetTabCount( ) )
	{
		// Select a new tab if the tab is active
		if ( m_ActiveTabIndex == inTab )
		{
			m_ActiveTabIndex = -1;
			m_ActiveTabWindow = NULL;

			if ( GetTabCount( ) > 1 )
			{
				theNewTab = ( inTab > 0 ) ? ( inTab - 1 ) : 0;
			}
		}

		// Save the window that is begin removed
		theTabHwnd = m_Views[ inTab ];

		// Virtual method to notify subclasses that a tab was removed
		OnTabRemoved( inTab );

		// Remove the item from the view list
		m_Views.RemoveAt( inTab );

		// Remove the tab
		if ( IsWindow() )
		{
			DeleteItem( inTab );
		}
		SetActiveTab( theNewTab );
	}
	return theTabHwnd;
}

void CWTLTabViewCtrl::RemoveAllTabs()
{
	LONG theCount = GetTabCount( );
	for ( LONG theIndex = theCount - 1; theIndex >= 0; theIndex-- ) 
	{
		RemoveTab( theIndex );
	}
}

void CWTLTabViewCtrl::SetActiveTab( int inNewTab )
{
	if ( inNewTab >= 0 && inNewTab < GetTabCount( ) &&	// Validate the tab index range
		 inNewTab != m_ActiveTabIndex )					// Don't select if already selected
	{
		// Disable the old tab
		if ( m_ActiveTabWindow.IsWindow( ) ) 
		{
			m_ActiveTabWindow.EnableWindow( FALSE );
			m_ActiveTabWindow.ShowWindow( SW_HIDE );
		}

		// Enable the new tab
		m_ActiveTabWindow = m_Views[ inNewTab ];

		m_ActiveTabWindow.EnableWindow( TRUE );
		m_ActiveTabWindow.ShowWindow( SW_SHOW );
		m_ActiveTabWindow.SetFocus( );

		m_ActiveTabWindow.Invalidate( TRUE );
		
		m_ActiveTabIndex = inNewTab;

		// Select the tab (if tab programmatically changed)
		SetCurSel( m_ActiveTabIndex );
	}
}

HWND CWTLTabViewCtrl::GetActiveTab()
{
	return GetTab( m_ActiveTabIndex );
}

LONG CWTLTabViewCtrl::GetActiveTabIndex()
{
	return m_ActiveTabIndex;
}

void CWTLTabViewCtrl::GetTabText(int inTab,int inSize, char* outText)
{
	if ( inTab >= 0 && inTab < GetTabCount( ) )
	{
		// Get tab item info
		TCITEM tci;
		tci.mask = TCIF_TEXT;
		tci.pszText = outText;
		tci.cchTextMax = inSize;
		GetItem( inTab, &tci );
	}
}

LPARAM CWTLTabViewCtrl::GetTabParam( int inTab )
{
	TCITEM tci = {0};
	if ( inTab >= 0 && inTab < GetTabCount( ) )
	{
		// Get tab item info
		tci.mask = TCIF_PARAM;
		GetItem( inTab, &tci );
	}

	return tci.lParam;
}

int CWTLTabViewCtrl::GetTabImage( int inTab )
{
	TCITEM tci = {0};
	if ( inTab >= 0 && inTab < GetTabCount( ) )
	{
		// Get tab item info
		tci.mask = TCIF_IMAGE;
		GetItem( inTab, &tci );
	}
	return tci.iImage;
}

BOOL CWTLTabViewCtrl::ModifyTabStyle( DWORD dwRemove, DWORD dwAdd, UINT nFlags )
{
	// Modify the style
	BOOL theReturn = ModifyStyle( dwRemove, dwAdd, nFlags );
	
	// Update all the views in case the window positions changes
	UpdateViews( );

	// Set the font in case the tab position changed
	SetTabFont( dwAdd );

	return theReturn;
}


void CWTLTabViewCtrl::CalcViewRect(CRect* pRect)
{
	GetClientRect( (*pRect) );

	if ( pRect->Height() > 0 && pRect->Width() > 0 )
	{
		// Calculate the Height (or Width) of the tab . . .
		// cause it could be Multiline
		CRect theTabRect;
		GetItemRect( 0, &theTabRect );

		LONG theRowCount = GetRowCount( );
		LONG theEdgeWidth = ( theTabRect.Width() * theRowCount ) + m_TABVIEW_EDGE;
		LONG theEdgeHeight = ( theTabRect.Height() * theRowCount ) + m_TABVIEW_EDGE;

		// Set the size based on the style
		DWORD dwStyle = GetStyle();
		if ((dwStyle & TCS_BOTTOM) && !(dwStyle & TCS_VERTICAL)) 
		{		// Bottom
			(*pRect).top	+= m_TABVIEW_BORDER;
			(*pRect).left	+= m_TABVIEW_BORDER;
			(*pRect).right	-= m_TABVIEW_BORDER;
			(*pRect).bottom	-= theEdgeHeight;
		}
		else if ((dwStyle & TCS_RIGHT) && (dwStyle & TCS_VERTICAL)) 
		{	// Right
			(*pRect).top	+= m_TABVIEW_BORDER;
			(*pRect).left	+= m_TABVIEW_BORDER;
			(*pRect).right	-= theEdgeWidth;
			(*pRect).bottom	-= m_TABVIEW_BORDER;
		}
		else if (dwStyle & TCS_VERTICAL)
		{								// Left
			(*pRect).top	+= m_TABVIEW_BORDER;
			(*pRect).left	+= theEdgeWidth;
			(*pRect).right	-= m_TABVIEW_BORDER;
			(*pRect).bottom	-= m_TABVIEW_BORDER;
		}
		else
		{															// Top
			(*pRect).top	+= theEdgeHeight;
			(*pRect).left	+= m_TABVIEW_BORDER;
			(*pRect).right	-= m_TABVIEW_BORDER;
			(*pRect).bottom	-= m_TABVIEW_BORDER;
		}
	}
}

void CWTLTabViewCtrl::UpdateViews()
{
	CRect rcChild;
	CalcViewRect( &rcChild );

	LONG theCount = GetTabCount( );
	for ( LONG theIndex = 0; theIndex < theCount; theIndex++ ) 
	{
		m_Views[ theIndex ].MoveWindow( rcChild );
	}
}

LRESULT CWTLTabViewCtrl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& )
{
	LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);

	// Get the log font.
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,	sizeof(NONCLIENTMETRICS), &ncm, 0);
	
	// Top and Bottom Tab Font
	m_HorizFont.CreateFontIndirect(&ncm.lfMessageFont);

	// Left Tab Font
	ncm.lfMessageFont.lfOrientation = 900;
	ncm.lfMessageFont.lfEscapement  = 900;
	m_LeftFont.CreateFontIndirect(&ncm.lfMessageFont);

	// Right Tab Font	
	ncm.lfMessageFont.lfOrientation = 2700;
	ncm.lfMessageFont.lfEscapement  = 2700;
	m_RightFont.CreateFontIndirect(&ncm.lfMessageFont);

	// Check styles to determine which font to set
	SetTabFont( GetStyle( ) );
	return lRet;
}

LRESULT CWTLTabViewCtrl::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	RemoveAllTabs( );
	bHandled = FALSE;
	return 0;
}

//==============================================================================
/**
 *		OnWindowPosChanged:	Called in response to the WM_WNDPOSCHANGED message
 *		@return	0
*/
//==============================================================================
LRESULT CWTLTabViewCtrl::OnWindowPosChanged(UINT, WPARAM, LPARAM, BOOL& bHandled )
{
	UpdateViews( );
	bHandled = FALSE;
	return 0;
}

//==============================================================================
/**
 *		OnSelectionChanged:	Called in response to the TCN_SELCHANGE message
 *		@return	0
 */
//==============================================================================
LRESULT CWTLTabViewCtrl::OnSelectionChanged( LPNMHDR pnHdr )
{
//	MessageBox("Select Tab");
	SetActiveTab( GetCurSel( ) );
	return 0;
}


void CWTLTabViewCtrl::SetTabFont( DWORD inStyleBits )
{
	if ( inStyleBits & TCS_VERTICAL ) 
	{
		if ( inStyleBits & TCS_RIGHT )
		{
			SetFont( m_RightFont );
		}

		else
		{
			SetFont( m_LeftFont );
		}
	}

	else
	{
		SetFont( m_HorizFont );
	}
}


