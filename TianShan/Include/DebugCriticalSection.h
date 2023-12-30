#ifndef __DEBUGCRITICALSECTION_H__
#define __DEBUGCRITICALSECTION_H__

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#if !defined(_SEACHANGE_H_)
#error "You must include 'zq.h' before 'debugcriticalsection.h'!"
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Dr. Watson Critical Section Debugging Technique
//
//                                                     Christiaan Lutzer
//                                                     Fri May 10 15:55:02 2002
//
// NAME
//      DebugEnterCriticalSection, DebugLeaveCriticalSection
//      
// SYNOPSIS
//
//      #include "DebugCriticalSection.h"
//
//      VOID DebugEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
//      VOID DebugLeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
//
// DESCRIPTION
//
// The DebugEnterCriticalSection and DebugLeaveCriticalSection functions are
// implemented as macros that call functions of the corresponding same name with
// two underscore characters prefixing the name.  The reason for the use of the
// macros is to automate the task of placing certain fields of the
// CRITICAL_SECTION object as parameters on the call stack.  If a program
// exception occurs, a Dr. Watson dump of the process will show the value of
// these parameters in the stack trace section for a given thread.
//
// The parameters that are placed onto the stack for debugging purposes are:
//
//      CRITICAL_SECTION  - The address of the critcal section object itself.
//      OwningThreadId      - The thread that currently owns the critical section
//                          object.  
//      LockCount         - The lock-count of the critical section.
//      RecursionCount    - The recursion count of the critical section.
//
// For example:
//
// FramePtr ReturnAd Param#1  Param#2  Param#3  Param#4  Function Name
// 0053FE64 77F838C6 00433A00 00401054 00433A78 0053FF1C ntdll!NtWaitForSingleObject 
// 0053FEC4 004010A1 00433A78 0053FF80 97969594 002F2C90 ntdll!NtQueryDefaultLocale 
// 
// 0053FF1C 00401166 00433A78 0000072C 00000000 00000001 !DebugEnterCriticalSection 
// 
// 0053FF80 00401408 00000000 93929190 97969594 002F2C90 !lock_thread 
// 0053FFB4 77E96523 002F2C90 93929190 97969594 002F2C90 !_beginthread 
// 0053FFEC 00000000 00000000 00000000 00000000 00000000 kernel32!TlsSetValue 
// 
// The line in question illustrates that for the CRITICAL_SECTION object pointed
// to by Param#1: 0x433a78, the owning thread is 0x72c, the lock count is 0, and
// the recursion count is 1.
//
// USAGE
//
// Simple, if you include this header file then implicitly all of your
// EnterCriticalSection() calls will be transformed into using the new
// __DebugEnterCriticalSection() call.  If you want to be 100% sure that you are
// using the new debugging macros, then use DebugEnterCriticalSection() and
// DebugLeaveCriticalSection().
//
// HISTORY
//
// I created this technique from the desire to find out which thread owns a
// critical section given nothing but a thread stack trace from Dr. Watson.
// More specifically, when a MSME based system detects a deadlock in its state
// machine engine, it dumps out a trace log which details the history of the
// engine.  From this MSME trace log (you can convert it into text via
// MSME_TP.EXE) you can determine which client and action routine caused the
// deadlock.  If the application crashes itself to invoke a Dr. Watson dump, one
// can inspect the thread ID belonging to the MSME worker thread that was
// deadlocked.  Typically those deadlocked threads will be in a NTDLL.DLL
// routine WaitForSingleObject (meaning they are waiting for a lock).  With this
// new debugging technique, we can now determine which thread owns that lock,
// and perhaps diagnose why we were deadlocked - 2002/05/10 [CEL].
//
////////////////////////////////////////////////////////////////////////////////

//
// Make sure that we don't already have these macros defined, otherwise we will
// get into an infinite loop situation.
//
#if defined(EnterCriticalSection)
#pragma message("Warning: EnterCriticalSection MACRO already defined - un-defining it!")
#undef EnterCriticalSection
#endif

#if defined(LeaveCriticalSection)
#pragma message("Warning: LeaveCriticalSection MACRO already defined - un-defining it!")
#undef LeaveCriticalSection
#endif

#ifdef __BORLANDC__
//#   pragma option push -w-pch
#   pragma warn -pch
#endif

static void
MicrosoftWillOptimizeThisStackFrameAway_Enter(LPCRITICAL_SECTION lpCriticalSection)
{
    EnterCriticalSection( lpCriticalSection );
}

static void
MicrosoftWillOptimizeThisStackFrameAway_Leave(LPCRITICAL_SECTION lpCriticalSection)
{
    LeaveCriticalSection( lpCriticalSection );
}

#ifdef __BORLANDC__
//#   pragma option pop
#endif

#ifdef _MSC_VER
#   pragma warning (disable : 4100) // disable unreferenced variables warning
#endif

static VOID
__DebugEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection,
                            HANDLE             OwningThreadId,
                            LONG               LockCount,
                            LONG               RecursionCount)
{

    MicrosoftWillOptimizeThisStackFrameAway_Enter( lpCriticalSection );
}

static VOID
__DebugLeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection,
                            HANDLE             OwningThreadId,
                            LONG               LockCount,
                            LONG               RecursionCount)
{
    HANDLE CurrentThreadId = ULongToHandle( GetCurrentThreadId() );
    if (CurrentThreadId != OwningThreadId)
        RaiseException( 0x80001000, EXCEPTION_NONCONTINUABLE, 0, 0 );
    MicrosoftWillOptimizeThisStackFrameAway_Leave( lpCriticalSection );
}

#ifdef _MCS_VER
#   pragma warning (default : 4100)
#endif

//
// Macros for using the debug enter and leave critical section functions.
//
#define DebugEnterCriticalSection(x) __DebugEnterCriticalSection(x,(x)->OwningThread,(x)->LockCount,(x)->RecursionCount)
#define DebugLeaveCriticalSection(x) __DebugLeaveCriticalSection(x,(x)->OwningThread,(x)->LockCount,(x)->RecursionCount)

//
// Convenience MACROS for those who do not wish to alter their source files.
// *** Note: these macros must be defined after the previous functions that
// actually have to call the real Win32 API EnterCriticalSection and
// LeaveCriticalSection functions.
//
#define EnterCriticalSection(x) DebugEnterCriticalSection(x)
#define LeaveCriticalSection(x) DebugLeaveCriticalSection(x)


#endif
