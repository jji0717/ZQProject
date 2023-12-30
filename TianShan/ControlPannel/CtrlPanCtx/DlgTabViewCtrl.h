// DlgTabViewCtrl.h: interface for the CDlgTabViewCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __DLGTABVIEWCTRL_H
#define __DLGTABVIEWCTRL_H


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "WTLTabViewCtrl.h"

class CDlgTabViewCtrl : public CWTLTabViewCtrl  
{
public:
	CDlgTabViewCtrl();
	virtual ~CDlgTabViewCtrl();

	DECLARE_WND_SUPERCLASS(NULL, CWTLTabViewCtrl::GetWndClassName())
	BOOL PreTranslateMessage(MSG* pMsg);
			
	BEGIN_MSG_MAP_EX(CDlgTabViewCtrl)
		CHAIN_MSG_MAP(CWTLTabViewCtrl)
	END_MSG_MAP()

public:
	void AddAttrsViewTab(LPCTSTR szTabName,LPCTSTR szParentTabName,BOOL bListCtrlMode);
	void AddTestViewTab( LPCTSTR szTabName);
	
	
protected:
	enum
	{
		DLGTABWINDOW_VAR   = 0,
		DLGTABWINDOW_LIST	= 1,
	};
};

#endif // !defined(AFX_DLGTABVIEWCTRL_H__211597D0_75B8_489D_B148_DE178D16CAAA__INCLUDED_)
