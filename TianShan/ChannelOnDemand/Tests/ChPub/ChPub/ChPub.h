// ChPub.h : main header file for the CHPUB application
//

#if !defined(AFX_CHPUB_H__5430F51D_DF2B_4C80_A565_C4B48402E3DB__INCLUDED_)
#define AFX_CHPUB_H__5430F51D_DF2B_4C80_A565_C4B48402E3DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CChPubApp:
// See ChPub.cpp for the implementation of this class
//

class CChPubApp : public CWinApp
{
public:
	CChPubApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChPubApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CChPubApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHPUB_H__5430F51D_DF2B_4C80_A565_C4B48402E3DB__INCLUDED_)
