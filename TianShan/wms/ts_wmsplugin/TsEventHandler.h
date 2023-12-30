// TsEventHandler.h : Declaration of the CTsEventHandler

#ifndef __TSEVENTHANDLER_H_
#define __TSEVENTHANDLER_H_

#include "resource.h"       // main symbols
// #include <wmsdefs.h>
#include <wmsbasicplugin.h>
#include <event.h>
#include <wmsserver.h>

/////////////////////////////////////////////////////////////////////////////
// CTsEventHandler
class ATL_NO_VTABLE CTsEventHandler : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTsEventHandler, &CLSID_TsEventHandler>,
	public IDispatchImpl<ITsEventHandler, &IID_ITsEventHandler, &LIBID_TS_WMSPLUGINLib>, 
	public IWMSBasicPlugin, 
	public IWMSEventNotificationPlugin
{
public:
	CTsEventHandler()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TSEVENTHANDLER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTsEventHandler)
	COM_INTERFACE_ENTRY(ITsEventHandler)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IWMSBasicPlugin)
	COM_INTERFACE_ENTRY(IWMSEventNotificationPlugin)
END_COM_MAP()

    // IWMSBasicPlugin
    STDMETHOD( InitializePlugin )( IWMSContext *pServerContext, 
		IWMSNamedValues *pNamedValues, 
		IWMSClassObject *pClassFactory );
    STDMETHOD( OnHeartbeat )( );
    STDMETHOD( GetCustomAdminInterface )( IDispatch **ppValue );
    STDMETHOD( ShutdownPlugin )();
    STDMETHOD( EnablePlugin ) ( long *pdwFlags, long *pdwHeartbeatPeriod );
    STDMETHOD( DisablePlugin )();

	// IWMSEventNotificationPlugin
    STDMETHOD( GetHandledEvents )( VARIANT *pvarHandledEvents );
    STDMETHOD( OnEvent )( WMS_EVENT *pEvent, IWMSContext *pUserCtx, 
		IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );

	HRESULT CreateArrayOfEvents( VARIANT *pvarEvents, 
		WMS_EVENT_TYPE *pWMSEvents, 
		long nNumEvents);

// ITsEventHandler
public:
	CComQIPtr<IWMSServer>		_wmsServer;
	CComBSTR					_pubPtName;
};

#endif //__TSEVENTHANDLER_H_
