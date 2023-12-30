// TsEventHandler.cpp : Implementation of CTsEventHandler
#include "stdafx.h"
#include "Ts_wmsplugin.h"
#include "TsEventHandler.h"

/////////////////////////////////////////////////////////////////////////////
// CTsEventHandler

//////////////////////////////////////////////////////////////////////////
// IWMSBasicPlugin
STDMETHODIMP CTsEventHandler::InitializePlugin( IWMSContext *pServerContext, 
	IWMSNamedValues *pNamedValues, 
	IWMSClassObject *pClassFactory )
{
	IWMSServer* wmsServer = NULL;
	ATLTRACE("+++ CTsEventHandler::InitializePlugin() +++\n");
	pServerContext->GetAndQueryIUnknownValue(WMS_SERVER, WMS_SERVER_ID, 
		IID_IWMSServer, (IUnknown** )&wmsServer, 0);
	if (wmsServer) {
		_wmsServer.Attach(wmsServer);
	} else {
		ATLTRACE("pServerContext->GetAndQueryIUnknownValue() failed.\n");
	}
	
	ATLTRACE("--- CTsEventHandler::InitializePlugin() ---\n");
	return S_OK;
}

STDMETHODIMP CTsEventHandler::OnHeartbeat( )
{
	ATLTRACE("+++ CTsEventHandler::OnHeartbeat() +++\n");
	ATLTRACE("--- CTsEventHandler::OnHeartbeat() ---\n");
	return S_OK;
}

STDMETHODIMP CTsEventHandler::GetCustomAdminInterface( IDispatch **ppValue )
{
	ATLTRACE("+++ CTsEventHandler::GetCustomAdminInterface() +++\n");

	// i have no admin interface, sorry!
	*ppValue = NULL;

	ATLTRACE("--- CTsEventHandler::GetCustomAdminInterface() ---\n");
	return S_OK;
}

STDMETHODIMP CTsEventHandler::ShutdownPlugin()
{
	ATLTRACE("+++ CTsEventHandler::ShutdownPlugin +++\n");
	ATLTRACE("--- CTsEventHandler::ShutdownPlugin ---\n");
	return S_OK;
}

STDMETHODIMP CTsEventHandler::EnablePlugin( long *pdwFlags, 
										   long *pdwHeartbeatPeriod )
{
	ATLTRACE("+++ CTsEventHandler::EnablePlugin() +++\n");
	ATLTRACE("--- CTsEventHandler::EnablePlugin() ---\n");
	return S_OK;
}

STDMETHODIMP CTsEventHandler::DisablePlugin( )
{
	ATLTRACE("+++ CTsEventHandler::DisablePlugin() +++\n");
	ATLTRACE("--- CTsEventHandler::DisablePlugin() ---\n");
	return S_OK;
}

// IWMSEventNotificationPlugin

// const NUM_HANDLED_EVENTS = 27;
const NUM_HANDLED_EVENTS = 10;

STDMETHODIMP CTsEventHandler::GetHandledEvents ( VARIANT *pvarHandledEvents )
{
	ATLTRACE("+++ CTsEventHandler::GetHandledEvents() +++\n");
	    HRESULT hr = S_OK;
    if( NULL == pvarHandledEvents )
    {
        return( E_POINTER );
    }

    int nIndex = 0;
    WMS_EVENT_TYPE wmsHandledEvents[ NUM_HANDLED_EVENTS ];
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_STOP;
	wmsHandledEvents[ nIndex++ ] = WMS_EVENT_CLOSE;
	wmsHandledEvents[ nIndex++ ] = WMS_EVENT_DISCONNECT;
    hr = CreateArrayOfEvents( pvarHandledEvents, wmsHandledEvents, 
		nIndex + 1 );
	ATLTRACE("--- CTsEventHandler::GetHandledEvents() ---\n");
	return hr;
}

STDMETHODIMP CTsEventHandler::OnEvent( WMS_EVENT *pEvent, 
									  IWMSContext *pUserCtx, 
									  IWMSContext *pPresentationCtx, 
									  IWMSCommandContext *pCommandCtx )
{
	ATLTRACE("+++ CTsEventHandler::OnEvent() +++\n");
	ATLTRACE("pEvent->Type = %d\n", pEvent->Type);
	HRESULT hr;

	if (pEvent->Type == WMS_EVENT_STOP || 
		pEvent->Type == WMS_EVENT_CLOSE || 
		pEvent->Type == WMS_EVENT_DISCONNECT) {
		if (_wmsServer) {
			CComQIPtr<IWMSContext> pCmdCtx;
			hr = pCommandCtx->GetCommandResponse(&pCmdCtx);
			if (FAILED(hr)) {
				ATLTRACE("pCommandCtx->GetCommandResponse() failed.\n");
				return hr;
			}

			PWSTR pubPtName;
			hr = pCmdCtx->GetStringValue( 
				WMS_COMMAND_CONTEXT_PUBPOINT_NAME,
				WMS_COMMAND_CONTEXT_PUBPOINT_NAME_ID,
				&pubPtName,
				0 );

			if (FAILED(hr)) {
				ATLTRACE("pCmdCtx->GetStringValue() failed. hr = %x\n", hr);
				return S_OK;
			}

			CComQIPtr<IWMSPublishingPoints> pubPts;
			_wmsServer->get_PublishingPoints(&pubPts);
			hr = pubPts->Remove(CComVariant(pubPtName));
			ATLTRACE("Removing publishing point: %s, result = %x\n", 
				pubPtName, hr);
		}
	}

	ATLTRACE("--- CTsEventHandler::OnEvent() ---\n");
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////

HRESULT CTsEventHandler::CreateArrayOfEvents( VARIANT *pvarEvents, 
											 WMS_EVENT_TYPE *pWMSEvents, 
											 long nNumEvents)
{
    HRESULT hr = S_OK;
    long iEvents = 0;
    SAFEARRAY *psa = NULL;
    SAFEARRAYBOUND rgsabound[1];
    
    if( NULL == pvarEvents )
    {
        return( E_POINTER );
    }

    if( NULL == pWMSEvents || 0 >= nNumEvents )
    {
        return( E_INVALIDARG );   
    }
        
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = nNumEvents;

    psa = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    if( NULL == psa )
    {
        return ( E_OUTOFMEMORY );
    }

    for( iEvents = 0; iEvents < nNumEvents && SUCCEEDED( hr ); iEvents++ )
    {
        VARIANT varElement;

        VariantInit( &varElement );

        V_VT( &varElement ) = VT_I4;
        V_I4( &varElement ) = pWMSEvents[ iEvents ];

        hr = SafeArrayPutElement( psa, &iEvents, &varElement );
        VariantClear( &varElement );
    }

    if( FAILED( hr ) )
    {
        SafeArrayDestroy( psa );
        psa = NULL;
    }
    else
    {
        V_VT( pvarEvents ) = VT_ARRAY | VT_VARIANT;
        V_ARRAY( pvarEvents ) = psa;
    }

    return ( hr );
}
