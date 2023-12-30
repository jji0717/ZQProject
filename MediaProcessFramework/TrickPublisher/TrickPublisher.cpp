// TrickPublisher.cpp : Defines the entry point for the DLL application.
//

#include "windows.h"

#include <streams.h>
#include <initguid.h>
#include "trickpub.h"
#include "TrickPubRender.h"
#include "ShareBufferMan.h"

// Setup data

const AMOVIESETUP_MEDIATYPE sudInputPinTypes =
{
//    &MEDIATYPE_Stream,       // Major type
	&MEDIASUBTYPE_NULL,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudOpPin =
{
    L"Input",              // Pin string name
    FALSE,                  // Is it rendered
    FALSE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudInputPinTypes 
};       // Pin details

const AMOVIESETUP_FILTER sudSSFax =
{
    &CLSID_TRICKPUBLISH,    // Filter CLSID
    L"Trick and Publish Render",       // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOpPin               // Pin details
};


// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {
  { L"Trick and Publish Render"
  , &CLSID_TRICKPUBLISH
  , CTrickPubFilter::CreateInstance
  , NULL
  , &sudSSFax }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);



STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

} // DllUnregisterServer


extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	return DllEntryPoint((HINSTANCE)(hModule), ul_reason_for_call, lpReserved);
}

