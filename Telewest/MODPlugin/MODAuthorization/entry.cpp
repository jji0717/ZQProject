
#include "mactst.h"
#include <assert.h>
#include <stdio.h>
#include <process.h>


#ifdef __cplusplus
    extern "C" {
#endif

// Define the global constants for DllExport.lib
const WCHAR* DLL_DISPLAY_NAME = { L"Sample MOD Authorization Check Plug-In DLL" };

ITVVERSION DLL_VERSION = { 0x0007 };

const WCHAR* DLL_UNID = { L"1f6cadd0-bff1-11d4-a375-00b0d02b1884" };

#ifdef __cplusplus
}
#endif
//#include "DllExport.h"
#include "mac_interfaces.h"

#include "ModAuthReporter.h"
extern CModAuthReporter* g_pAuthReporter;

BOOL gModAuthInitialized;

/*
 * Procedure:   DllMain
 *
 * Description: This function gets called when the DLL is loaded or
 *              unloaded
 *
 * Arguments:   hinstDLL
 *                  Handle to DLL module
 *
 *              fdwReason
 *                  Reason for calling function
 *
 *              lpvReserved
 *                  Reserved
 *
 * Returns:     None.
 *
 * Effects:     None
 */

BOOL WINAPI DllMain ( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {

	switch (fdwReason) {

		case DLL_PROCESS_ATTACH:       // Process loaded DLL
              break;

        case DLL_PROCESS_DETACH:       // Process unloaded DLL
              // Attempt to do proper cleanup.
              break;

        case DLL_THREAD_ATTACH:        // Current process created new thread
              break;

        case DLL_THREAD_DETACH:        // Thread exited normally
              break;

        default:
              break;
	}

	return ( TRUE );
}

/*
 * Procedure:   MacInitialize
 *
 * Description: This function is used to initialize this Plug-In DLL
 *
 * Arguments:   pHandles
 *                  Pointer to management handle of MOD Service
 *
 *              pCallbacks
 *                  Pointer to MOD Service routine to signal an unload request
 *
 * Returns:     None.
 *
 * Effects:     None
 */

ITVSTATUS MacInitialize ( MACHANDLES *pHandles, MACCALLBACKS *pCallbacks ) {

    CModAuth   *pModAuth;
    ITVSTATUS err;
    MANSTATUS manRet;
    DWORD     dwError;

    // If already initialized then return success
    if ( gModAuthInitialized == TRUE ) {
        return ( ITV_SUCCESS );
    }

    // Check parameters
    if ( ( pHandles == NULL ) || ( pCallbacks == NULL ) ) {
        return ( ITV_BAD_PARAM );
    }

    // Check version of structures
    if ( pHandles->Version.wVersion != MAC_VERSION_2_0 ) {
        return ( ITV_INVALID_VERSION );
    }

    // Construct the MOD Plug-In DLL class
    pModAuth = CModAuth::Instance(pHandles, pCallbacks);
    if ( pModAuth == NULL ) {
        return ( ITV_NT_ERROR );
    }

    if ( pModAuth->IsValid() == FALSE ) {
        CModAuth::FreeInstance();
        return ( ITV_NT_ERROR );
    }

    // Initialize the MOD Plug-In DLL class
    err = pModAuth->Initialize();
    if ( ( err != ITV_SUCCESS ) ) {
        CModAuth::FreeInstance();
        return ( err );
    }

    // If a management handle was provided then initialize interface for DLL
    if ( pModAuth->GetManHandle() != NULL ) {
        manRet = ManManageVar ( pModAuth->GetManHandle(), pModAuth->GetName(),
                                MAN_COMPLEX, (DWORD) MacManUtilCB, TRUE, &dwError);
        if ( manRet != MAN_SUCCESS ) {
            pModAuth->Uninitialize();
			CModAuth::FreeInstance();
            return ( ITV_NT_ERROR );
        }
    }

    // Finally, denote that we are initialized.
    gModAuthInitialized = TRUE;

	if(g_pAuthReporter != NULL)
		g_pAuthReporter->ReportLog(REPORT_DEBUG, L"Exit MacInitialize");

    return ( ITV_SUCCESS );
}

/*
 * Procedure:   MacUninitialize
 *
 * Description: This function is used to uninitialize this Plug-In DLL
 *
 * Arguments:   None
 *
 * Returns:     None.
 *
 * Effects:     None
 */

ITVSTATUS MacUninitialize () {

    CModAuth *pModAuth = CModAuth::Instance();
    DWORD   dwError;

    // Unitialize DLL
    if ( pModAuth != NULL ) {

        // Uninitialize management interface
		if ( pModAuth->GetManHandle() != NULL ) {
			ManUnmanageVar ( pModAuth->GetManHandle(), pModAuth->GetName(), &dwError );
			pModAuth->SetManHandle(NULL);
		}

        pModAuth->Uninitialize();
		CModAuth::FreeInstance();
    }

    // Finally, denote that we are no longer initialized.
    gModAuthInitialized = FALSE;

    return ( ITV_SUCCESS );
}

/*
 * Procedure:   MacProcess
 *
 * Description: This function is used invoke a test process routine.
 *
 * Arguments:   pMacData
 *                  Block of input data on which authorization check will be performed
 *
 *              pbStatus
 *                  Address of flag used to indicate results of authorization check
 *
 * Returns:     None.
 *
 * Effects:     None
 */

ITVSTATUS MacProcess ( BYTE *pRequest, BOOL *pbResponse ) {

	CModAuth *pModAuth = CModAuth::Instance();

	// If not initialized then return error.
	if ( gModAuthInitialized == FALSE ) {
        return ( ITV_NOT_INITED );
	}
	
    return ( pModAuth->Process ( pRequest, pbResponse ) );
}

/*
 * Procedure:   MacProcess
 *
 * Description: This function is used invoke a test process routine.
 *
 * Arguments:   pMacData
 *                  Block of input data on which authorization check will be performed
 *
 *              pbStatus
 *                  Address of flag used to indicate results of authorization check
 *
 * Returns:     None.
 *
 * Effects:     None
 */

ITVSTATUS MacReport ( BYTE *pRequest, BOOL *pbResponse ) {

	CModAuth *pModAuth = CModAuth::Instance();

	// If not initialized then return error.
	if ( gModAuthInitialized == FALSE ) {
        return ( ITV_NOT_INITED );
	}

    return ( pModAuth->Report( pRequest, pbResponse ) );
}
