// NPVRSMSUI.h : main header file for the NPVRSMSUI application
//

#if !defined(AFX_NPVRSMSUI_H__354CDF5E_3AD2_4F24_ABB5_9A280A6B254F__INCLUDED_)
#define AFX_NPVRSMSUI_H__354CDF5E_3AD2_4F24_ABB5_9A280A6B254F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIApp:
// See NPVRSMSUI.cpp for the implementation of this class
//

class CNPVRSMSUIApp : public CWinApp
{
public:
	CNPVRSMSUIApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNPVRSMSUIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CNPVRSMSUIApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NPVRSMSUI_H__354CDF5E_3AD2_4F24_ABB5_9A280A6B254F__INCLUDED_)
