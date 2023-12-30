// TrayWorkNode.h : main header file for the TRAYWORKNODE application
//

#if !defined(AFX_TRAYWORKNODE_H__416C43F8_B3E1_417D_951A_146353D7E1F8__INCLUDED_)
#define AFX_TRAYWORKNODE_H__416C43F8_B3E1_417D_951A_146353D7E1F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTrayWorkNodeApp:
// See TrayWorkNode.cpp for the implementation of this class
//

class CTrayWorkNodeApp : public CWinApp
{
public:
	CTrayWorkNodeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrayWorkNodeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTrayWorkNodeApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAYWORKNODE_H__416C43F8_B3E1_417D_951A_146353D7E1F8__INCLUDED_)
