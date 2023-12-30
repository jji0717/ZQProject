// CdmiFuseTray.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCdmiFuseTrayApp:
// See CdmiFuseTray.cpp for the implementation of this class
//

//define global message
#define WM_USER_EXIT   (WM_USER+1000)
#define WM_USER_SETUP  (WM_USER+1001)
#define WM_USER_CLOSE  (WM_USER+1002)

class CCdmiFuseTrayApp : public CWinApp
{
public:
	CCdmiFuseTrayApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSetup();
	afx_msg void OnExit();
	//CCdmiFuseDlg *m_dlgSettings;
};

extern CCdmiFuseTrayApp theApp;