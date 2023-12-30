// DummyAddin.h : main header file for the DUMMYADDIN DLL
//

#if !defined(AFX_DUMMYADDIN_H__71A6F223_1AAA_4510_BEFD_FF43478F17BB__INCLUDED_)
#define AFX_DUMMYADDIN_H__71A6F223_1AAA_4510_BEFD_FF43478F17BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDummyAddinApp
// See DummyAddin.cpp for the implementation of this class
//

class CDummyAddinApp : public CWinApp
{
public:
	CDummyAddinApp();
	~CDummyAddinApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDummyAddinApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDummyAddinApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DUMMYADDIN_H__71A6F223_1AAA_4510_BEFD_FF43478F17BB__INCLUDED_)
