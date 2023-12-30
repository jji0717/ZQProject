// JMSDispatch.h : main header file for the JMSDISPATCH DLL
//

#if !defined(AFX_JMSDISPATCH_H__7F879E1B_B172_47F4_B6C2_5D534B2D4156__INCLUDED_)
#define AFX_JMSDISPATCH_H__7F879E1B_B172_47F4_B6C2_5D534B2D4156__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJMSDispatchApp
// See JMSDispatch.cpp for the implementation of this class
//

class CJMSDispatchApp : public CWinApp
{
public:
	CJMSDispatchApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJMSDispatchApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CJMSDispatchApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMSDISPATCH_H__7F879E1B_B172_47F4_B6C2_5D534B2D4156__INCLUDED_)
