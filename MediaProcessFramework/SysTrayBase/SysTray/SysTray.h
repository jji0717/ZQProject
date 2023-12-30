// SysTray.h : main header file for the SYSTRAY application
//

#if !defined(AFX_SYSTRAY_H__BD79D705_07CB_4FD1_8629_EF8BD4C1AD10__INCLUDED_)
#define AFX_SYSTRAY_H__BD79D705_07CB_4FD1_8629_EF8BD4C1AD10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSysTrayApp:
// See SysTray.cpp for the implementation of this class
//

class CSysTrayApp : public CWinApp
{
public:
	CSysTrayApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysTrayApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSysTrayApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTRAY_H__BD79D705_07CB_4FD1_8629_EF8BD4C1AD10__INCLUDED_)
