// DataWrapper.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#include "DataWrapperFilter.h"

// self-registration entrypoint
STDAPI DllRegisterServer()
{
	// base classes will handle registration using the factory template table
	HRESULT hr = AMovieDllRegisterServer2(true);

	return hr;
}

STDAPI DllUnregisterServer()
{
	// base classes will handle de-registration using the factory template table
	HRESULT hr = AMovieDllRegisterServer2(false);

	return hr;
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	return DllEntryPoint((HINSTANCE)(hModule), ul_reason_for_call, lpReserved);
    //return TRUE;
}

