// CodMan.h : main header file for the CODMAN application
//

#if !defined(AFX_CODMAN_H__F5A4F868_2F4B_4CD2_8550_85E58910A5BD__INCLUDED_)
#define AFX_CODMAN_H__F5A4F868_2F4B_4CD2_8550_85E58910A5BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <Ice/Ice.h>

/////////////////////////////////////////////////////////////////////////////
// CCodManApp:
// See CodMan.cpp for the implementation of this class
//

class CCodManApp : public CWinApp
{
public:
	CCodManApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCodManApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CCodManApp)
	afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	Ice::CommunicatorPtr _communicator;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODMAN_H__F5A4F868_2F4B_4CD2_8550_85E58910A5BD__INCLUDED_)
