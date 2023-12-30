// TrayManageNode.h : main header file for the TRAYMANAGENODE application
//

#if !defined(AFX_TRAYMANAGENODE_H__D7E92B24_587C_4589_A5C2_6DE1CED79E72__INCLUDED_)
#define AFX_TRAYMANAGENODE_H__D7E92B24_587C_4589_A5C2_6DE1CED79E72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTrayManageNodeApp:
// See TrayManageNode.cpp for the implementation of this class
//

class CTrayManageNodeApp : public CWinApp
{
public:
	CTrayManageNodeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrayManageNodeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTrayManageNodeApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAYMANAGENODE_H__D7E92B24_587C_4589_A5C2_6DE1CED79E72__INCLUDED_)
