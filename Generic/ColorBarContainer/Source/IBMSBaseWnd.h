// TabbedSDISplitterView.h
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABBEDSDISplitterVIEW_H__3A3EDE40_24E0_4EB6_8AEE_6A03B7E6BD67__INCLUDED_)
#define AFX_TABBEDSDISplitterVIEW_H__3A3EDE40_24E0_4EB6_8AEE_6A03B7E6BD67__INCLUDED_

#define ID_MIN_VERB 1000
#define ID_MAX_VERB 2000           
#define ID_CONVERT  10000
#pragma once
#include "IBMSBase.h"

#define WM_SETUSERMODE (WM_USER + 100)

class CIBMSBaseWnd :
    public CAxWindowImplT<CIBMSBaseWnd>
{
public:
	typedef CAxWindowImplT<CIBMSBaseWnd>	    baseClass1;
	CIBMSBaseWnd();
	virtual ~CIBMSBaseWnd();
	
	BEGIN_MSG_MAP(CIBMSBaseWnd)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS,OnSetFocus)
		//MESSAGE_HANDLER(WM_NCDESTROY,OnNCDestroy)
		MESSAGE_HANDLER(WM_NCHITTEST,OnNCHitTest)
		
		COMMAND_RANGE_HANDLER(ID_MIN_VERB,ID_MAX_VERB,OnVerb)
	// useless.
	/*
	ALT_MSG_MAP(1)
	    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		CHAIN_MSG_MAP(baseClass1)
	*/
	END_MSG_MAP()



	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM  lParam, BOOL& bHandled);
	LRESULT OnNCHitTest(UINT , WPARAM , LPARAM , BOOL& bHandled);
	LRESULT OnNCDestroy(UINT , WPARAM , LPARAM , BOOL& bHandled);
	LRESULT OnVerb(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	
public:
    HRESULT IBMSCreateControlEx2(LPCOLESTR lpszName, HWND hWnd,IStream* pStream = NULL, IUnknown** ppUnkContainer = NULL,
		    IUnknown** ppUnkControl = NULL,REFIID iidSink = IID_NULL, IUnknown* punkSink = NULL);

	HRESULT SetUserModeChanged(VARIANT_BOOL bUserMode);
	VARIANT_BOOL GetContainerUserMode(void);
	BOOL SetShowWindow(BOOL bShow);
	BOOL save(BSTR bsFile);
	BOOL isDirty();
    
};

class CIBMSBaseHostWnd:
    public CAxHostWindow
{
public:
	CIBMSBaseHostWnd()
	{

	}
    ~CIBMSBaseHostWnd()
	{

	}
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABBEDSDISplitterVIEW_H__3A3EDE40_24E0_4EB6_8AEE_6A03B7E6BD67__INCLUDED_)
