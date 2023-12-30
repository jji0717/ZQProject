// JmsUI.h : main header file for the JMSUI application
//

#if !defined(AFX_JMSUI_H__8CC68E88_B90C_4FDD_8110_5491523FAB55__INCLUDED_)
#define AFX_JMSUI_H__8CC68E88_B90C_4FDD_8110_5491523FAB55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CJmsUIApp:
// See JmsUI.cpp for the implementation of this class
//

extern bool g_Queue1_Busy;
extern bool g_Queue2_Busy;
extern int  g_Message_ID;

class CJmsUIApp : public CWinApp
{
public:
	CJmsUIApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJmsUIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CJmsUIApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JMSUI_H__8CC68E88_B90C_4FDD_8110_5491523FAB55__INCLUDED_)
