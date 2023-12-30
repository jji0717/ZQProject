#ifndef __MYWTLTABVIEWCTRL_H
#define __MYWTLTABVIEWCTRL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "WTLTabViewCtrl.h"
#include "DataDialog.h"

class CMyTabViewCtrl : public CWTLTabViewCtrl  
{
public:
	CMyTabViewCtrl();
	virtual ~CMyTabViewCtrl();

	DECLARE_WND_SUPERCLASS(NULL, CWTLTabViewCtrl::GetWndClassName())
	BOOL PreTranslateMessage(MSG* pMsg);
			
	BEGIN_MSG_MAP_EX(CMyTabViewCtrl)
		CHAIN_MSG_MAP(CWTLTabViewCtrl)
	END_MSG_MAP()

public:
	void AddSNMPVarWinTab( LPCTSTR szTabName);
	void AddServiceData(LPCTSTR szTabName,BOOL bShowAboveCtrl = FALSE );
	void AddServiceData1(LPCTSTR szTabName);
	void AddEventListViewTab(LPCTSTR szTabName);
		
protected:
	enum
	{
		TABWINDOW_VAR	= 0,
		TABWINDOW_DATA1	= 1,
		TABWINDOW_DATA2	= 1,
		TABWINDOW_DATA3 = 1,
		TABWINDOW_DATA4 = 1,
		TABWINDOW_DATA5 = 1,
		TABWINDOW_DATA6 = 1,
		TABWINDOW_EVENT = 2,
	};
};

extern CDataDialog *m_pDataView;

#endif
