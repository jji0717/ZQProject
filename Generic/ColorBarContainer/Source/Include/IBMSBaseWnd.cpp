// TabbedSDISplitterView.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "resource.h"
#include "IBMSBaseWnd.h"

CIBMSBaseWnd::CIBMSBaseWnd()
{
}

CIBMSBaseWnd::~CIBMSBaseWnd()
{
}

BOOL CIBMSBaseWnd::save(BSTR bsFile)
{
	CComPtr<IPersistFile> pPersistFile;
	QueryControl(&pPersistFile);
	HRESULT hr = pPersistFile->Save(bsFile, FALSE);
	return TRUE;	
}

BOOL CIBMSBaseWnd::isDirty()
{
	CComPtr<IPersistFile> pPersistFile;
	QueryControl(&pPersistFile);
	HRESULT hr;
	if ( pPersistFile )
		hr = pPersistFile->IsDirty();
	if ( hr == S_OK )
		return TRUE;
	return FALSE;	
}

// Name        : OnSize
// Description : WM_SIZE handler, might be used later
// Parameters  : 
//         UINT
//         WPARAM
//         LPARAM
//         BOOL& bHandled
// Returns     : LRESULT 

LRESULT CIBMSBaseWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Invalidate();
	RECT rcClient;
	::GetClientRect(m_hWnd,&rcClient);
	CComPtr<IOleInPlaceObject>  spOleInplaceObject;
	HRESULT hr = QueryControl(&spOleInplaceObject);
	if ( SUCCEEDED( hr ) )
	{
		spOleInplaceObject->SetObjectRects(&rcClient,&rcClient);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Name        : OnLButtonDown
// Description : WM_LBUTTONDOWN handler. to activate current object
// Parameters  : 
//         UINT
//         WPARAM
//         LPARAM
//         BOOL& bHandled
// Returns     : LRESULT 
//////////////////////////////////////////////////////////////////////////////

LRESULT CIBMSBaseWnd::OnLButtonDown(UINT uMsg, WPARAM  wParam, LPARAM lParam, BOOL & bHandled)
{
   OnSetFocus(uMsg,wParam,lParam,bHandled);
   return 0L;
}

// Name        : OnVerb
// Description : do verb handler
// Parameters  : 
//         WORD wNotifyCode
//         WORD wID
//         HWND hWndCtl
//         BOOL& bHandled
// Returns     : LRESULT 
//////////////////////////////////////////////////////////////////////////////
LRESULT CIBMSBaseWnd::OnVerb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CComPtr<IOleObject>		spObj;
	CComPtr<IOleClientSite> spClientSite;

	QueryControl(&spObj);
	QueryHost(&spClientSite);

	MSG msg;
	RECT rc;
	::GetClientRect(m_hWnd,&rc);
	HRESULT hr = E_FAIL;
	if( spObj.p && spClientSite.p )
        hr = spObj->DoVerb(wID - ID_MIN_VERB,&msg,spClientSite,0,m_hWnd,&rc);

    return 0L;
}


//////////////////////////////////////////////////////////////////////////////
// Name        : OnSetFocus
// Description : WM_SETFOCUS handler
// Parameters  : 
//         UINT
//         WPARAM
//         LPARAM
//         BOOL& bHandled
// Returns     : LRESULT 
//////////////////////////////////////////////////////////////////////////////
LRESULT CIBMSBaseWnd::OnSetFocus(UINT,WPARAM,LPARAM, BOOL& bHandled)
{
    CComPtr<IOleWindow > spOleWindow;
	HRESULT hr = QueryControl(&spOleWindow);
	if ( SUCCEEDED(hr) )
	{
	  HWND pWnd;	
	  spOleWindow->GetWindow(&pWnd);
	  ::SetFocus(pWnd);
	}
	bHandled = FALSE;
	return 0L;
}


// Name        : OnNCDestroy
// Description : WM_NCDESTROY handler. destroy host window
// Parameters  : 
//         UINT
//         WPARAM
//         LPARAM
//         BOOL& bHandled
// Returns     : LRESULT 

LRESULT CIBMSBaseWnd::OnNCDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   return 0L;
}

//////////////////////////////////////////////////////////////////////////////
// Name        : OnNCHitTest
// Description : WN_HCHITTEST handler
// Parameters  : 
//         UINT
//         WPARAM
//         LPARAM lParam
//         BOOL& bHandled
// Returns     : LRESULT 
//////////////////////////////////////////////////////////////////////////////

LRESULT CIBMSBaseWnd::OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{



	return 0L;
}



//////////////////////////////////////////////////////////////////////////////
// Name        : SetUserModeChanged
// Description : switch between User(Run) mode and Edit mode
// Parameters  : 
//         VARIANT_BOOL bUserMode - VARIANT_TRUE, run; VARIANT_FALSE, edit
// Returns     : HRESULT 
//////////////////////////////////////////////////////////////////////////////
HRESULT CIBMSBaseWnd::SetUserModeChanged(VARIANT_BOOL bUserMode)
{
    VARIANT_BOOL b = bUserMode ? VARIANT_TRUE : VARIANT_FALSE;
	CComPtr<IUnknown> spUnk;
    HRESULT hr = QueryHost(&spUnk);
	if( SUCCEEDED( hr ) && spUnk )
	{
		CComPtr<IAxWinAmbientDispatch> spAmbDispatch;
        hr = spUnk->QueryInterface(&spAmbDispatch);
		if( SUCCEEDED( hr ) && spAmbDispatch)
		{
			hr = spAmbDispatch->put_UserMode( b );
		}
	}
    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// Name        : GetContainerUserMode
// Description : get current user mode
// Parameters  : 
//         void
// Returns     : VARIANT_BOOL 
//     VARIANT_BOOL bUserMode - VARIANT_TRUE, run; VARIANT_FALSE, edit
/////////////////////////////////////////////////////////////////////////////
VARIANT_BOOL CIBMSBaseWnd::GetContainerUserMode(void)
{
	VARIANT_BOOL b = VARIANT_FALSE;
	CComPtr<IUnknown> spUnk;
    HRESULT hr = QueryHost(&spUnk);
	if( SUCCEEDED( hr ) && spUnk )
	{
		CComPtr<IAxWinAmbientDispatch> spAmbDispatch;
        hr = spUnk->QueryInterface(&spAmbDispatch);
		if( SUCCEEDED( hr ) && spAmbDispatch)
		{
			hr = spAmbDispatch->get_UserMode( &b );
		}
	}

	return b;
}

//////////////////////////////////////////////////////////////////////////////
// Name        : SetShowWindow
// Description : set the object's window  display mode
// Parameters  : 
// [in] BOOL bShow : the object's window display set mode
// Returns     : BOOL
//     TRUE: Show, FALSE : Hide
/////////////////////////////////////////////////////////////////////////////

BOOL CIBMSBaseWnd::SetShowWindow(BOOL bShow)
{
	CComPtr<IOleClientSite> spClientSite;
	QueryHost(&spClientSite);

	spClientSite->OnShowWindow(bShow);

/*
	CComPtr<IOleClientSite> spClientSite;
	CComPtr<IOleObject> spObj;

	if(SUCCEEDED( pNavigate->QueryControl(&spObj) ))
	{
		//if ( SUCCEEDED( pNavigate->QueryControl(&spClientSite) ) )
		if ( SUCCEEDED( spObj->GetClientSite(&spClientSite) ) )
		{
			//spObj->SetClientSite(spClientSite);
			spClientSite->OnShowWindow(bVisible);
		}

	}
*/
	return bShow;
}

//////////////////////////////////////////////////////////////////////////////
// Name        : CreateControlEx
// Description : Create Control in the Container
// Parameters  : 
//  LPCOLOESTR , lpszName: A pointer to a string to be passed to the control;
//  HWND  hWnd : Handle to the window that the control will be attached to;
//  IStream * pStream : A pointer to a stream that is used to initialize the properties of the control;]
//  IUnknown** ppUnkContainer : The address of a pointer that will receive the IUnknown of the container;
//  IUnknown** ppUnkControl:  The address of a pointer that will receive the IUnknown of the created control;
//  REFIID iidSink : The interface identifier of an outgoing interface on the contained object.;
//  IUnknown * punkSink : A pointer to the IUnknown interface of the sink object to be connected to the connection point specified by iidSink on the contained object after the contained object has been successfully created;
// Returns     : HRESUTL
/////////////////////////////////////////////////////////////////////////////

HRESULT  CIBMSBaseWnd::IBMSCreateControlEx2(LPCOLESTR lpszName, HWND hWnd,IStream* pStream , IUnknown** ppUnkContainer ,
		    IUnknown** ppUnkControl ,REFIID iidSink  , IUnknown* punkSink )

{
	CComPtr<IAxWinHostWindow> pAxWindow;
	QueryHost(&pAxWindow);
   	CComBSTR bstrName(lpszName);
	CComPtr<IUnknown> spUnkControl;

	HRESULT hr = pAxWindow->CreateControlEx(bstrName,hWnd , pStream, &spUnkControl , iidSink, punkSink);
//	      IBMSCreateControlEx2(lpolestr,m_pChild->m_hWnd,NULL,&pContainer,&(*ppControl));
	
	if (ppUnkControl != NULL)
	{
		if (SUCCEEDED(hr))
		{
			*ppUnkControl = SUCCEEDED(hr) ? spUnkControl.p : NULL;
			spUnkControl.p = NULL;
		}
		else
			*ppUnkControl = NULL;
	}
	return hr;

//	return AxCreateControl2(lpszName, hWnd,  pStream,  ppUnkContainer, ppUnkControl , iidSink , punkSink );
}





   //以下是作为在实现过程中做过的测试代码，可以作为以后的参考价值

    //以下是作为一个测试，在容器窗体包含一个ActiveX Control ，but i failed and do not know why?
	/*
	AtlAxWinInit();
	RECT rect;
	//::GetClientRect(m_hWnd,&rect);
	GetClientRect(&rect);
	HWND hwndContainer = m_Test.Create(m_hWnd,rect,"test window",WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);

	LPOLESTR spTest;
	StringFromCLSID(CLSID_BullsEye,&spTest);

	
	CComPtr<IAxWinHostWindow> spTestHost;
	HRESULT hr = m_Test.QueryHost(&spTestHost);
	if ( SUCCEEDED(hr))
	{
		
		hr = spTestHost->CreateControlEx(spTest,m_Test,NULL,&spTestIUn,IID_NULL,NULL);
		if ( SUCCEEDED(spTestIUn->QueryInterface(IID_IBullsEye,(void**)&spTestWnd)))
		{
			
		}
	}
	*/

   /*
	CLSID   theclsid;
	HRESULT hr = ::CLSIDFromProgID(bsAppID,&theclsid);
	USES_CONVERSION;
	
	
	LPOLESTR lpolestr;
	StringFromCLSID(theclsid,&lpolestr);
	
	IUnknown * pIContainer = 0;
	hr = IBMSAtlAxCreateControlEx(lpolestr,m_viewTree.m_hWnd,NULL,&pIContainer,&(*ppControl),IID_NULL,NULL); //(IUnknown**)&(*ppHostWindow)

	if ( SUCCEEDED(hr))
	{
		CComPtr<IOleObject> spoo;
		//if ( SUCCEEDED(pIContainer->QueryInterface(&spoo)) )
		if ( SUCCEEDED((*ppControl)->QueryInterface(&spoo)) )
		{
			RECT rect,rect1;
			::GetClientRect(m_viewTree.m_hWnd,&rect);
			::GetWindowRect(m_viewTree.m_hWnd,&rect1);
			SIZE Size ;
			Size.cx = rect1.right  ;
			Size.cy = rect.bottom  ;
			spoo->SetExtent(DVASPECT_CONTENT,&Size);
			CComPtr<IOleClientSite> spcs;
			if ( SUCCEEDED((*ppControl)->QueryInterface(&spcs)) )
			//if ( SUCCEEDED(pIContainer->QueryInterface(&spcs)) )
			{
				spoo->SetClientSite(spcs);
				spoo->DoVerb(OLEIVERB_PROPERTIES,0,spcs,-1,m_viewTree.m_hWnd,&rect);
			//	pIContainer->Release();
			}
		//pIContainer->Release();
		}
	    if ( SUCCEEDED(pIContainer->QueryInterface(&(*ppHostWindow)) ))
		{
 		  pIContainer->Release();
		}
	}
	CoTaskMemFree(lpolestr);
	*/


/*
    CLSID   theclsid;
	HRESULT hr = ::CLSIDFromProgID(bsAppID,&theclsid);
	USES_CONVERSION;
	
	CIBMSBaseWnd* pChild = new CIBMSBaseWnd;
	pChild->Create(m_tabbedChildWindow,rcDefault, W2A(bsAppID),WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	
	LPOLESTR lpolestr;
	StringFromCLSID(theclsid,&lpolestr);
	
	IUnknown * pIContainer = 0;
	hr = IBMSAtlAxCreateControlEx(lpolestr,pChild->m_hWnd,NULL,&pIContainer,&(*ppControl),IID_NULL,NULL); // (IUnknown**)&(*ppHostWindow)

	if ( SUCCEEDED(hr))
	{
		CComPtr<IOleObject> spoo;
		//if ( SUCCEEDED(pIContainer->QueryInterface(&spoo)) )
		if ( SUCCEEDED((*ppControl)->QueryInterface(&spoo)) )
		{
			RECT rect,rect1;
			::GetClientRect(pChild->m_hWnd,&rect);
			::GetWindowRect(pChild->m_hWnd,&rect1);
			SIZE Size ;
			Size.cx = rect1.right  ;
			Size.cy = rect.bottom  ;
			spoo->SetExtent(DVASPECT_CONTENT,&Size);
			CComPtr<IOleClientSite> spcs;
			if ( SUCCEEDED((*ppControl)->QueryInterface(&spcs)) )
			//if ( SUCCEEDED(pIContainer->QueryInterface(&spcs)) )
			{
				spoo->SetClientSite(spcs);
				spoo->DoVerb(OLEIVERB_PROPERTIES,0,spcs,-1,pChild->m_hWnd,&rect);
			//	pIContainer->Release();
			}
		//pIContainer->Release();
		}
	    if ( SUCCEEDED(pIContainer->QueryInterface(&(*ppHostWindow)) ))
		{
 		  pIContainer->Release();
		}
	}
	CoTaskMemFree(lpolestr);
	m_tabbedChildWindow.DisplayTab(pChild->m_hWnd,TRUE,TRUE);
	*/




		


