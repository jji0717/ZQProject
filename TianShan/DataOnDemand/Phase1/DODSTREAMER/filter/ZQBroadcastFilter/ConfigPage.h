#pragma once
#include "stdafx.h"
#include <commdlg.h>
#include <initguid.h>
#include <stdio.h>
#include <atlconv.h>

#include <atlbase.h>
#include "resource.h"
#include <vector>

#include "BroadcastGuid.h"


class CBroadcastConfigPage : public CBasePropertyPage
{
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpUnknown,HRESULT *pHr);

public:
	CBroadcastConfigPage(LPUNKNOWN lpUnknown,HRESULT *pHr);
	~CBroadcastConfigPage();

public:
	virtual HRESULT OnConnect(IUnknown *pUnknown);
	virtual HRESULT OnDisconnect();
	virtual HRESULT OnActivate();
	virtual HRESULT OnDeactivate();
	virtual HRESULT OnApplyChanges();
	virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

public:
	HRESULT ReflectBML();
	HRESULT EnterBML();

protected:
	
	// config channelinfo interface
	IRateControl* m_pBMLConfig;

	// config sendout info interface.
	IIPSetting* m_pIPConfig;
//	HWND m_hWndFile;

	//get channels total numbers
	HWND m_hWndChannelCount;

	//for config a channel once.It is index;
	HWND m_hWndIndex;

	// channel PID
	HWND m_hWndPID;

	// channel rate.It is not total rate;
	HWND m_hWndRate;

};
