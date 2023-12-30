// WTLTabViewCtrl.h: interface for the CWTLTabViewCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __WTLTABVIEWCTRL_H
#define __WTLTABVIEWCTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
#ifndef __ATLMISC_H__
	#include <atlmisc.h>
#endif

#ifndef __ATLCTRLS_H__
	#include <atlctrls.h>
#endif

#ifndef __ATLCRACK_H__
	#include <atlcrack.h>
#endif
*/

class CWTLTabViewCtrl : public CWindowImpl<CWTLTabViewCtrl,CTabCtrl>  
{
public:
	CWTLTabViewCtrl();
	virtual ~CWTLTabViewCtrl();

	//	Message Map
public:
	BEGIN_MSG_MAP_EX(CWTLTabViewCtrl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnWindowPosChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnSelectionChanged)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelectionChanged( LPNMHDR pnHdr );
		
	
	//Public Interface
public:
    /*		AddTab:	Append a tab to the end of the tab control.
	 *		@param	inLabel:      [in] The label to appear on the tab control.
	 *		@param	inTabWindow:  [in] The child window to use as the view for the tab. The window
										must have the WS_CHILD style bit set and the WS_VISIBLE style bit not set.
	 *		@param	inActiveFlag: [in, optional] TRUE to make the tab active, FALSE to just add the tab.
	 *		@param	inImage:   	  [in, optional] The index into the image list for the image to display by the tab label.
	 *		@param	inParam:	  [in, optional] The param value to associate with the tab.
	 *		@return	Zero based index of the new tab, -1 on failure
	 */
	BOOL  AddTab( LPCTSTR inLabel, HWND inTabWindow, BOOL inActiveFlag = TRUE, int inImage = -1, LPARAM inParam = 0 );
	
	/*		GetTabCount:	Return the current number of tabs.
	 *		@return	The current number of tabs.
	 */
	LONG  GetTabCount();
	
	/*		GetTab:	Get the view window associated with the tab.
	 *		@param	inTab:	[in] The index of the tab to return the view window.
	 *		@return	The handle of the view window.
     */
	HWND GetTab( int inTab );

	/*	 OnTabRemoved:	Virtual method that is called when a tab is removed.
     *	 @param	inTabIndex:[in] The index of the tab to return the view window.
	 *		@return	The handle of the view window.
	 */
	virtual void OnTabRemoved( int inTabIndex );

	/*   RemoveTab:	Remove the specified tab.
	 *	 @param	inTab:	[in] The index of the tab to remove.
	 *	 @return	Returns the HWND of the deleted view window.
     */
	HWND RemoveTab( int inTab );

	/*	RemoveAllTabs:	Remove all the tabs from the tab control.
	 *	@return	void
	 */
	void RemoveAllTabs();
	
	/*	SetActiveTab:	Activate the specified tab.
	 *	@param	inNewTab: Select the zero based tab at this index.
	 *	@return	void
	 */
	void SetActiveTab(int inNewTab);

	/*	GetActiveTab:	Return the HWND to the active tab.
	 *	@return	The HWND to the active view.
	 */
	HWND GetActiveTab();

	/*	GetActiveTabIndex:	Return the index of the active tab.
	 *	@return	The index of the active tab.
	 */
	LONG GetActiveTabIndex();

	/*	GetTabText:	Return the label of the specified tab.
	 *	@param	inTab:[in] The index of the tab for which to return the text.
	 *	@param	inSize:[in] The size of the text string buffer.
	 *	@param	outText:[out] The text string buffer.
	 *	@return	void
	 */
	void GetTabText(int inTab,int inSize, char* outText);

	/*	GetTabParam:	Return the param of the specified tab.
	 *	@param	inTab:	[in] The index of the tab for which to return the param.
	 *	@return	LPARAM
	 */
	LPARAM GetTabParam( int inTab );

	/*	GetTabImage:	Return the param of the specified tab.
	 *	@param	inTab:	[in] The index of the tab for which to return the image.
	 *	@return	int
	 */
	int GetTabImage( int inTab );

	/*
	 *		ModifyTabStyle:	This method modifies the window styles of the CWindow 
	 *			object. Styles to be added or removed can be combined by using the 
	 *			bitwise OR ( | ) operator.
	 *		@param	dwRemove:[in] Specifies the window styles to be removed during style modification.
	 *		@param	dwAdd:	 [in] Specifies the window styles to be added during style modification.
	 *		@param	nFlags:  [in] Window-positioning flags. For a list of possible values, 
	 *				see theSetWindowPos function in the Win32 SDK.
	 *	 		    If nFlags is nonzero, ModifyStyle calls the Win32 function SetWindowPos, 
	 * 		        and redraws the window by combining nFlags with the following four flags: 
	 *		    SWP_NOSIZE   Retains the current size.
	 *		    SWP_NOMOVE   Retains the current position.
	 *		    SWP_NOZORDER   Retains the current Z order.
	 *		    SWP_NOACTIVATE   Does not activate the window. 
	 *		To modify a window's extended styles, call ModifyStyleEx.
	 *		@return	TRUE if the window styles are modified; otherwise, FALSE.
	 */
	BOOL ModifyTabStyle( DWORD dwRemove, DWORD dwAdd, UINT nFlags );


	//	Implementation
protected:

	/*	CalcViewRect:	Calculate the client rect for contained views.
		@param	pRect:  [out] Returns the new view rect.
		 @return	void
	*/
	void CalcViewRect(CRect* pRect);

	/*	UpdateViews:	Update the position of all the contained views.
	 *	@return	void
	 */
	void UpdateViews();

	/*	SetTabFont:	Set the font to be used by the tab control.
	 *	@param	inStyleBits:[in] The style bits to use to calculate the font to use. 
	 *	@return	void
	 */
	void SetTabFont( DWORD inStyleBits );


	//Constants
private:
	LONG m_TABVIEW_BORDER ;         ///< Border Width
	LONG m_TABVIEW_EDGE   ;			///< Distance of tab from content
	
	//	Fields
private:
	CSimpleArray<CWindow>		m_Views;			///< An array of views for the tab
	LONG						m_ActiveTabIndex;	///< The index of the active tab
	CWindow						m_ActiveTabWindow;	///< The active tab window
	CFont						m_HorizFont;		///< Top/Bottom font used by tab control
	CFont						m_LeftFont;			///< Left font used by tab control
	CFont						m_RightFont;		///< Right font used by tab control
};
#endif 
