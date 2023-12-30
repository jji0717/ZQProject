
#ifndef __SEA_COMMON_H__
#define __SEA_COMMON_H__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SEACOMMON_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SEACOMMON_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef SEACOMMON_EXPORTS
#define SEACOMMON_API extern "C" __declspec(dllexport)
#else
#define SEACOMMON_API extern "C" __declspec(dllimport)
#endif

#include <stdio.h>

//: SeaChange assertion macro
//
#define S_ASSERT(x) (void)( (x) || (__SeaAssert(#x, __FILE__, __LINE__), 0) )

#define IN_THE_FUTURE 0

#if IN_THE_FUTURE

#if !defined(SEACOMMON_EXPORTS)

    // The following doesn't work!!
    //#pragma comment(linker, "/libpath:L:\\Lib")

	#if defined(_DEBUG)
		#pragma comment(lib, "SeaCommon_d.lib")
	#else
		#pragma comment(lib, "SeaCommon.lib")
	#endif
#endif

SEACOMMON_API void __SeaAssert(
	IN	const char		*szExpression,
	IN	const char		*szFile,
	IN	int				nLine);

#else // if IN_THE_FUTURE

#pragma warning (disable : 4505)

static void __SeaAssert(
	IN	const char		*szExpression,
	IN	const char		*szFile,
	IN	int				nLine)
{   
    HANDLE hEventLog = RegisterEventSourceA(NULL, "SEACHANGE");
    if (NULL != hEventLog)
    {
        char szLine[32];
        LPCSTR ppStrings[3] = { szFile, szLine, szExpression };

        sprintf(szLine, "%d", nLine);

        ReportEventA
        (
            hEventLog,              // handle returned by RegisterEventSource
            EVENTLOG_ERROR_TYPE,    // event type to log
            0,                      // event category
            SEA_ASSERT,             // event identifier
            NULL,                   // user security identifier (optional)
            3,                      // number of strings to merge with message
            0,                      // size of binary data, in bytes
            ppStrings,              // array of strings to merge with message
            NULL                    // address of binary data
        );
        DeregisterEventSource(hEventLog);
    }

	/*
    Log(LEVEL_ERROR, _T("ASSERT"),
        _T("The DevMgr module has encountered a fatal error (assert) in (%s:%d): \"%s\".  Forcably restarting application..."),
        szFile, nLine, szExpression);

	DevMgrModule::GetInstance().GetLog().Flush();
	*/

    RaiseException(APPSHELL_ASSERT_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, 0);
}

#endif // if IN_THE_FUTURE

#endif // if defined(__SEA_COMMON_H__)

