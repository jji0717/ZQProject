//
// Copyright (c) 1994-2002 by
// SeaChange Technology Inc., 124 Acton Street, Maynard Massachusetts 01754
// All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of SeaChange  Technology  Inc.   Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from SeaChange Technology Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
// 
// The information in this software is subject to change without  notice and
// should not be construed as a commitment by SeaChange Technology Inc.
// 
// SeaChange  assumes  no  responsibility  for the use or reliability of its
// software on equipment which is not supplied by SeaChange.
// 
// RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
// Government is subject  to  restrictions  as  set  forth  in  Subparagraph
// (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
//
//
//
// title:       SeaError.mc
//
// version:     1.4
//
// facility:    ITV
//
// abstract:    This is the message compiler source file for all Seachange error
//              messages.  This module is not intended for use with the NT event
//              log, but merely as a DLL for SeaError to translate error codes
//              into strings.
//
//  Rev     Date            Who     Description
//  ------- --------------- ------- ----------------------------------------
//  1.4     2002-03-16      CEL     Created.
//
//*******************************************************************************
//* Header Section
//*
//* There will be no typedefs for SeaChange messages.  The severity names will be
//* the the default ones: SeverityNames = (Success = (DWORD)0x0, Informational = 0x1,
//* Warning = (DWORD)0x2, Error = 0x3).  Language names will likewise default to:
//* LanguageNames = (English = 1 : MSG00001)
//*
//* The only thing defined in this section is the facility codes and names.
//*******************************************************************************
// DO NOT change Generic_SeaChange_Success from (DWORD)0x000

//*******************************************************************************
//*
//* SeaChange Standard Status Return Code
//*
//*******************************************************************************

// You should be using the standard ``HRESULT'' as the return type (see winnt.h)

//*******************************************************************************
//*
//* SeaChange Success Messages
//*
//*******************************************************************************
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define THREADPOOL_FACILITY_CODE         (DWORD)0x208
#define SYNAPI_FACILITY_CODE             (DWORD)0x20E
#define SHELL_FACILITY_CODE              (DWORD)0x209
#define SEACOMSMTP_FACILITY_CODE         (DWORD)0x20C
#define REPORTER_FACILITY_CODE           (DWORD)0x207
#define PROTLIB_FACILITY_CODE            (DWORD)0x20B
#define MTTCPCOMM_FACILITY_CODE          (DWORD)0x206
#define MSME_FACILITY_CODE               (DWORD)0xF00
#define MCASTSVC_FACILITY_CODE           (DWORD)0x204
#define MANPKG_FACILITY_CODE             (DWORD)0x203
#define SEA_FACILITY_CODE                (DWORD)0x201
#define DATA_WAREHOUSE_FACILITY_CODE     (DWORD)0x20D
#define CLASSLIB_FACILITY_CODE           (DWORD)0x20A
#define CFGPKG_FACILITY_CODE             (DWORD)0x202


//
// Define the severity codes
//


//
// MessageId: SEA_SUCCESS
//
// MessageText:
//
//  (SeaChange) Normal successful completion.
//
#define SEA_SUCCESS                      (DWORD)0x20000000L

//
// MessageId: SEA_PENDING
//
// MessageText:
//
//  (SeaChange) Request successfully queued.
//
#define SEA_PENDING                      (DWORD)0x22010000L

//*******************************************************************************
//*
//* SeaChange Warning Messages
//*
//*******************************************************************************
//
// MessageId: SEA_ALREADY_INITED
//
// MessageText:
//
//  (SeaChange) A session has already been initialized.
//
#define SEA_ALREADY_INITED               (DWORD)0xA2010000L

//*******************************************************************************
//*
//* SeaChange Error Messages
//*
//*******************************************************************************
//
// MessageId: SEA_BAD_HANDLE
//
// MessageText:
//
//  (SeaChange) The handle passed is somehow invalid.
//
#define SEA_BAD_HANDLE                   (DWORD)0xE2010000L

//
// MessageId: SEA_BAD_PARAM
//
// MessageText:
//
//  (SeaChange) One of the service parameters was not valid.
//
#define SEA_BAD_PARAM                    (DWORD)0xE2010001L

//
// MessageId: SEA_OUT_OF_MEMORY
//
// MessageText:
//
//  (SeaChange) The call failed because of a memory allocation error.
//
#define SEA_OUT_OF_MEMORY                (DWORD)0xE2010002L

//
// MessageId: SEA_NOT_SUPPORTED
//
// MessageText:
//
//  (SeaChange) The call is unsupported, not implemented, outdated, whatever.
//
#define SEA_NOT_SUPPORTED                (DWORD)0xE2010003L

//
// MessageId: SEA_NOT_INITED
//
// MessageText:
//
//  (SeaChange) A session has not been initialized, or has already been terminated or aborted.
//
#define SEA_NOT_INITED                   (DWORD)0xE2010004L

//
// MessageId: SEA_INVALID_VERSION
//
// MessageText:
//
//  (SeaChange) The call cannot be supported due to version incompatibility.
//
#define SEA_INVALID_VERSION              (DWORD)0xE2010005L

//
// MessageId: SEA_NT_ERROR
//
// MessageText:
//
//  (SeaChange) A Windows NT specific error ocurred, see extended status for NT error code.
//
#define SEA_NT_ERROR                     (DWORD)0xE2010006L

//
// MessageId: SEA_WSA_ERROR
//
// MessageText:
//
//  (SeaChange) A Windows Winsock specific error occurred, see extended status for WSA error code.
//
#define SEA_WSA_ERROR                    (DWORD)0xE2010007L

//
// MessageId: SEA_SHUTTING_DOWN
//
// MessageText:
//
//  (SeaChange) A call was aborted because the package, module, or service was shutting down.
//
#define SEA_SHUTTING_DOWN                (DWORD)0xE2010008L

//
// MessageId: SEA_FILE_NOT_FOUND
//
// MessageText:
//
//  (SeaChange) The specified file name could not be found.
//
#define SEA_FILE_NOT_FOUND               (DWORD)0xE2010009L

//
// MessageId: SEA_ASSERT
//
// MessageText:
//
//  (SeaChange) The application has encountered a fatal error (ASSERT) in (%1:%2): "%3".  The application must restart.
//
#define SEA_ASSERT                       (DWORD)0xE201000AL

//*******************************************************************************
//*
//* Configuration Package (CfgPkg) Messages
//*
//*******************************************************************************
//
// MessageId: CFGPKG_FAIL
//
// MessageText:
//
//  (CfgPkg) CFGPKG_FAIL.
//
#define CFGPKG_FAIL                      (DWORD)0xE2020000L

//
// MessageId: CFGPKG_SYSERR
//
// MessageText:
//
//  (CfgPkg) CFGPKG_SYSERR.
//
#define CFGPKG_SYSERR                    (DWORD)0xE2020001L

//
// MessageId: CFGPKG_BAD_HANDLE
//
// MessageText:
//
//  (CfgPkg) A bad Configuration Package handle was used.
//
#define CFGPKG_BAD_HANDLE                (DWORD)0xE2020002L

//
// MessageId: CFGPKG_BAD_OBJTYPE
//
// MessageText:
//
//  (CfgPkg) CFGPKG_BAD_OBJTYPE.
//
#define CFGPKG_BAD_OBJTYPE               (DWORD)0xE2020003L

//
// MessageId: CFGPKG_ALREADY_INITED
//
// MessageText:
//
//  (CfgPkg) CFGPKG_ALREADY_INITED.
//
#define CFGPKG_ALREADY_INITED            (DWORD)0xE2020004L

//
// MessageId: CFGPKG_BADPARAM
//
// MessageText:
//
//  (CfgPkg) CFGPKG_BADPARAM.
//
#define CFGPKG_BADPARAM                  (DWORD)0xE2020005L

//
// MessageId: CFGPKG_DLL_NOT_FOUND
//
// MessageText:
//
//  (CfgPkg) CFGPKG_DLL_NOT_FOUND.
//
#define CFGPKG_DLL_NOT_FOUND             (DWORD)0xE2020006L

//
// MessageId: CFGPKG_ENTRY_NOT_FOUND
//
// MessageText:
//
//  (CfgPkg) CFGPKG_ENTRY_NOT_FOUND.
//
#define CFGPKG_ENTRY_NOT_FOUND           (DWORD)0xE2020007L

//
// MessageId: CFGPKG_ENTRY_WRONG_TYPE
//
// MessageText:
//
//  (CfgPkg) CFGPKG_ENTRY_WRONG_TYPE.
//
#define CFGPKG_ENTRY_WRONG_TYPE          (DWORD)0xE2020008L

//
// MessageId: CFGPKG_NO_SUCH_SUBKEY
//
// MessageText:
//
//  (CfgPkg) The specified subkey does not exist.
//
#define CFGPKG_NO_SUCH_SUBKEY            (DWORD)0xE2020009L

//*******************************************************************************
//*
//* Management Package (ManPkg) Messages
//*
//*******************************************************************************
//*******************************************************************************
//*
//* Multicast Service (MCastSvc) Messages
//*
//*******************************************************************************
//*******************************************************************************
//*
//* Multithreaded State Machine Engine (MSME) Messages
//*
//*******************************************************************************
// Note: at this time it appears MSME_STS_SUCCESS is defined as 0.
//
// MSME Exception Codes
//
//
// MessageId: MSME_EXC_TERMPKG
//
// MessageText:
//
//  (MSME) A fatal error occurred while terminating the MSME package.
//
#define MSME_EXC_TERMPKG                 (DWORD)0xEF000000L

//
// MessageId: MSME_EXC_DESTROYSM
//
// MessageText:
//
//  (MSME) A fatal error occurred while destroying a MSME state machine.
//
#define MSME_EXC_DESTROYSM               (DWORD)0xEF000001L

//
// MessageId: MSME_EXC_EXECUTEEVENT
//
// MessageText:
//
//  (MSME) A fatal error occurred executing an event.
//
#define MSME_EXC_EXECUTEEVENT            (DWORD)0xEF000002L

//
// MessageId: MSME_EXC_BADACTION
//
// MessageText:
//
//  (MSME) An event was thrown at a state machine that did not have a handler for it.
//
#define MSME_EXC_BADACTION               (DWORD)0xEF000003L

//
// MessageId: MSME_EXC_INITPKG
//
// MessageText:
//
//  (MSME) A fatal error occurred while initializing the MSME package.
//
#define MSME_EXC_INITPKG                 (DWORD)0xEF000004L

//
// MessageId: MSME_EXC_DOEVENT
//
// MessageText:
//
//  (MSME) A fatal error occurred while DOing the event.
//
#define MSME_EXC_DOEVENT                 (DWORD)0xEF000005L

//
// MessageId: MSME_EXC_WORKERTHREAD
//
// MessageText:
//
//  (MSME) A fatal error occurred in a MSME worker thread.
//
#define MSME_EXC_WORKERTHREAD            (DWORD)0xEF000006L

//
// MessageId: MSME_EXC_RUNDOWNSM
//
// MessageText:
//
//  (MSME) A fatal error occurred while running down a MSME state machine.
//
#define MSME_EXC_RUNDOWNSM               (DWORD)0xEF000007L

//
// MessageId: MSME_EXC_QEVENT
//
// MessageText:
//
//  (MSME) A fatal error occurred while queueing an event to a MSME state machine.
//
#define MSME_EXC_QEVENT                  (DWORD)0xEF000008L

//
// MessageId: MSME_EXC_TRACE
//
// MessageText:
//
//  (MSME) A fatal error occurred while tracing.
//
#define MSME_EXC_TRACE                   (DWORD)0xEF000009L

//
// MessageId: MSME_EXC_CATCHALL
//
// MessageText:
//
//  (MSME) Generic catch-all error.
//
#define MSME_EXC_CATCHALL                (DWORD)0xEF00000AL

//
// MessageId: MSME_EXC_INVALID_SM
//
// MessageText:
//
//  (MSME) The specified state machine handle is not valid.
//
#define MSME_EXC_INVALID_SM              (DWORD)0xEF00000BL


//
// MSME Status Codes
//
//
// MessageId: MSME_STS_BUSY
//
// MessageText:
//
//  (MSME) Unable to initialize another MSME user because there are already too many users initialized.
//
#define MSME_STS_BUSY                    (DWORD)0xEF00000CL

//
// MessageId: MSME_STS_BADPARAM
//
// MessageText:
//
//  (MSME) A bad parameter was passed to a MSME function.
//
#define MSME_STS_BADPARAM                (DWORD)0xEF00000DL

//
// MessageId: MSME_STS_TOOMANYTYPES
//
// MessageText:
//
//  (MSME) Unable to define a new state machine type because there are already too many types defined.
//
#define MSME_STS_TOOMANYTYPES            (DWORD)0xEF00000EL

//
// MessageId: MSME_STS_SHUTDOWN
//
// MessageText:
//
//  (MSME) Unable to process request because either the state machine or MSME is shutting down.
//
#define MSME_STS_SHUTDOWN                (DWORD)0xEF00000FL

//
// MessageId: MSME_STS_EVENTSLEFT
//
// MessageText:
//
//  (MSME) This status code is currently not used.
//
#define MSME_STS_EVENTSLEFT              (DWORD)0xEF000010L

//
// MessageId: MSME_STS_ABORTED
//
// MessageText:
//
//  (MSME) The state machine action routine signalled that it has aborted.
//
#define MSME_STS_ABORTED                 (DWORD)0xEF000011L

//
// MessageId: MSME_STS_NOSUCHSM
//
// MessageText:
//
//  (MSME) Unable to process request because no such state machine exists.
//
#define MSME_STS_NOSUCHSM                (DWORD)0xEF000012L

//
// MessageId: MSME_STS_PKGALREADYUP
//
// MessageText:
//
//  (MSME) Unable to initialize MSME package because it is already initialized.
//
#define MSME_STS_PKGALREADYUP            (DWORD)0xEF000013L

//
// MessageId: MSME_STS_PKGNOTUP
//
// MessageText:
//
//  (MSME) Unable to process request because MSME package has not yet been initialized.
//
#define MSME_STS_PKGNOTUP                (DWORD)0xEF000014L

//
// MessageId: MSME_STS_NOMEMORY
//
// MessageText:
//
//  (MSME) Unable to process request because the system has run out of memory.
//
#define MSME_STS_NOMEMORY                (DWORD)0xEF000015L

//
// MessageId: MSME_STS_FILEIOFAIL
//
// MessageText:
//
//  (MSME) A file I/O error occurred.
//
#define MSME_STS_FILEIOFAIL              (DWORD)0xEF000016L

//
// MessageId: MSME_STS_NOTRACING
//
// MessageText:
//
//  (MSME) A tracing error has occurred.
//
#define MSME_STS_NOTRACING               (DWORD)0xEF000017L

//
// MessageId: MSME_STS_TOOMANYTHREADS
//
// MessageText:
//
//  (MSME) Unable to initialize MSME package because too many threads have been requested.
//
#define MSME_STS_TOOMANYTHREADS          (DWORD)0xEF000018L

//*******************************************************************************
//*
//* Multithreaded TCP Communications (MtTcpComm) Messages
//*
//*******************************************************************************
//*******************************************************************************
//*
//* Reporting Package (Reporter) Messages
//*
//*******************************************************************************
//*******************************************************************************
//*
//* Thread Pool Package (ScThreadPool) Messages
//*
//*******************************************************************************
//*******************************************************************************
//*
//* Shell Messages
//*
//*******************************************************************************
//
// MessageId: APPSHELL_ASSERT_EXCEPTION
//
// MessageText:
//
//  (AppShell) A user ASSERT has been converted to a raised exception.
//
#define APPSHELL_ASSERT_EXCEPTION        (DWORD)0xE2090000L

//
// MessageId: APPSHELL_SHUTDOWN_TIMEOUT
//
// MessageText:
//
//  (AppShell) The application shell has timed out during shutdown.
//
#define APPSHELL_SHUTDOWN_TIMEOUT        (DWORD)0xE2090001L

//
// MessageId: SRVSHELL_SHUTDOWN_TIMEOUT
//
// MessageText:
//
//  (SrvShell) The service shell has timed out during shutdown.
//
#define SRVSHELL_SHUTDOWN_TIMEOUT        (DWORD)0xE2090002L

//*******************************************************************************
//*
//* ClassLib Error Messages
//*
//*******************************************************************************
//
// General error codes.
//
//
// MessageId: CLASSLIB_ERROR
//
// MessageText:
//
//  (ClassLib) A generic error has occurred.
//
#define CLASSLIB_ERROR                   (DWORD)0xE20A0000L

//
// MessageId: CLASSLIB_PROCEDURE_ERROR
//
// MessageText:
//
//  (ClassLib) A procedural error has occurred.
//
#define CLASSLIB_PROCEDURE_ERROR         (DWORD)0xE20A0001L

//
// MessageId: CLASSLIB_INTERNAL_ERROR
//
// MessageText:
//
//  (ClassLib) A fatal internal error has occurred.
//
#define CLASSLIB_INTERNAL_ERROR          (DWORD)0xE20A0002L

//
// MessageId: CLASSLIB_TIMEOUT
//
// MessageText:
//
//  (ClassLib) The operation has timed out.
//
#define CLASSLIB_TIMEOUT                 (DWORD)0xE20A0003L

//
// Deathwatcher error codes.
//
//
// MessageId: CLASSLIB_DEATHWATCH
//
// MessageText:
//
//  (ClassLib) A subsystem has failed watch-dog tests.  The application is restarting.
//
#define CLASSLIB_DEATHWATCH              (DWORD)0xE20A0004L

//
// MessageId: CLASSLIB_DEATHWATCHER_NOT_ARMED
//
// MessageText:
//
//  (ClassLib) A deathwatch function could not be completed because the deathwatcher was not armed.
//
#define CLASSLIB_DEATHWATCHER_NOT_ARMED  (DWORD)0xE20A0005L

//
// Timer error codes.
//
//
// MessageId: CLASSLIB_TIMER_INTERNAL_ERROR
//
// MessageText:
//
//  (ClassLib) A fatal internal error has occurred in a timer.
//
#define CLASSLIB_TIMER_INTERNAL_ERROR    (DWORD)0xE20A0006L

//
// MessageId: CLASSLIB_TIMER_NOT_ENABLED
//
// MessageText:
//
//  (ClassLib) A timer function could not be completed because the timer was not previously enabled.
//
#define CLASSLIB_TIMER_NOT_ENABLED       (DWORD)0xE20A0007L

//
// MessageId: CLASSLIB_INVALID_TIMER_ID
//
// MessageText:
//
//  (ClassLib) An invalid timer ID was specified.
//
#define CLASSLIB_INVALID_TIMER_ID        (DWORD)0xE20A0008L

//
// MessageId: CLASSLIB_INVALID_TIMER_TYPE
//
// MessageText:
//
//  (ClassLib) A timer function could not be completed because a timer with an incorrect type was specified.
//
#define CLASSLIB_INVALID_TIMER_TYPE      (DWORD)0xE20A0009L

//
// Socket error codes.
//
//
// MessageId: CLASSLIB_SOCKET_INVALID_TYPE
//
// MessageText:
//
//  (ClassLib) An operation was attempted on a socket which is not appropriate given its type.
//
#define CLASSLIB_SOCKET_INVALID_TYPE     (DWORD)0xE20A000AL

//
// MessageId: CLASSLIB_SOCKET_BUSY
//
// MessageText:
//
//  (ClassLib) A new socket was unable to be created, because the socket object was already in use.
//
#define CLASSLIB_SOCKET_BUSY             (DWORD)0xE20A000BL

//
// MessageId: CLASSLIB_SOCKET_INVALID
//
// MessageText:
//
//  (ClassLib) The specified socket could not be associated with the socket manager because the socket is invalid.
//
#define CLASSLIB_SOCKET_INVALID          (DWORD)0xE20A000CL

//
// MessageId: CLASSLIB_SOCKET_NOT_FOUND
//
// MessageText:
//
//  (ClassLib) The specified socket could not be removed from the socket manager because it was not found.
//
#define CLASSLIB_SOCKET_NOT_FOUND        (DWORD)0xE20A000DL

//
// MessageId: CLASSLIB_SOCKET_CONFLICTING_ADDRESS_TYPES
//
// MessageText:
//
//  (ClassLib) The specified socket operation could not be completed because of conflicting address types (Example IPv4 vs. IPv6).
//
#define CLASSLIB_SOCKET_CONFLICTING_ADDRESS_TYPES (DWORD)0xE20A000EL

//
// Clog error codes.
//
//
// MessageId: CLASSLIB_CLOG_ERROR
//
// MessageText:
//
//  (ClassLib) A Clog error has occurred.  Check the extended status code for the Clog specific error.
//
#define CLASSLIB_CLOG_ERROR              (DWORD)0xE20A000FL

//
// Registry error codes.
//
//
// MessageId: CLASSLIB_REGISTRY_KEY_NOT_OPEN
//
// MessageText:
//
//  (ClassLib) A registry operation could not be performed because the key was not open.
//
#define CLASSLIB_REGISTRY_KEY_NOT_OPEN   (DWORD)0xE20A0010L

//
// MessageId: CLASSLIB_REGISTRY_INCORRECT_TYPE
//
// MessageText:
//
//  (ClassLib) Unable to read registry value because the actual versus specified type does not match.
//
#define CLASSLIB_REGISTRY_INCORRECT_TYPE (DWORD)0xE20A0011L

//
// ManPkg error codes.
//
//
// MessageId: CLASSLIB_MANPKG_ERROR
//
// MessageText:
//
//  (ClassLib) A Management Package error has occurred.  Check the extended status code for the ManPkg specific error.
//
#define CLASSLIB_MANPKG_ERROR            (DWORD)0xE20A0012L

//
// MessageId: CLASSLIB_MANPKG_BAD_HANDLE
//
// MessageText:
//
//  (ClassLib) Unable to join Management Package session because the session handle was invalid.
//
#define CLASSLIB_MANPKG_BAD_HANDLE       (DWORD)0xE20A0013L

//
// CfgPkg error codes.
//
//
// MessageId: CLASSLIB_CFGPKG_INIT_ERROR
//
// MessageText:
//
//  (ClassLib) Unable to open Configuration Package session.
//
#define CLASSLIB_CFGPKG_INIT_ERROR       (DWORD)0xE20A0014L

//
// MessageId: CLASSLIB_CFGPKG_BAD_HANDLE
//
// MessageText:
//
//  (ClassLib) Unable to join Configuration Package session because the session handle was invalid.
//
#define CLASSLIB_CFGPKG_BAD_HANDLE       (DWORD)0xE20A0015L

//
// Thread error codes.
//
//
// MessageId: CLASSLIB_THREAD_EXECUTE
//
// MessageText:
//
//  (ClassLib) Unable to create a new thread of execution for the specified thread object.  See the extended status code for the NT error that occurred.
//
#define CLASSLIB_THREAD_EXECUTE          (DWORD)0xE20A0016L

//
// MessageId: CLASSLIB_THREADFUNCTION_EXCEPTION
//
// MessageText:
//
//  (ClassLib) An exception was raised in a thread object's thread of execution.
//
#define CLASSLIB_THREADFUNCTION_EXCEPTION (DWORD)0xE20A0017L

//
// Persistence error codes.
//
//
// MessageId: CLASSLIB_PERSISTENCE_EVENT_ERROR
//
// MessageText:
//
//  (ClassLib) An internal error occurred with the persistence flusher event.  Forcing an application restart.
//
#define CLASSLIB_PERSISTENCE_EVENT_ERROR (DWORD)0xE20A0018L

//
// MessageId: CLASSLIB_PERSISTENCE_INVALID_SLOTSIZE
//
// MessageText:
//
//  (ClassLib) A slot size that was too small was specified to the persistence interface.
//
#define CLASSLIB_PERSISTENCE_INVALID_SLOTSIZE (DWORD)0xE20A0019L

//
// MessageId: CLASSLIB_PERSISTENCE_CREATE
//
// MessageText:
//
//  (ClassLib) Unable to create a persistence interface file on disk.  See the extended status code for the NT-specific error that occurred.
//
#define CLASSLIB_PERSISTENCE_CREATE      (DWORD)0xE20A001AL

//
// MessageId: CLASSLIB_PERSISTENCE_OPEN
//
// MessageText:
//
//  (ClassLib) Unable to open a persistence interface file on disk.  See the extended status code for the NT-specific error that occurred.
//
#define CLASSLIB_PERSISTENCE_OPEN        (DWORD)0xE20A001BL

//
// MessageId: CLASSLIB_PERSISTENCE_BAD_SEEK
//
// MessageText:
//
//  (ClassLib) Unable to seek to the specified slot in the persistence interface file.  See the extended status code for the NT-specific error that occurred.
//
#define CLASSLIB_PERSISTENCE_BAD_SEEK    (DWORD)0xE20A001CL

//
// MessageId: CLASSLIB_PERSISTENCE_IO_ERROR
//
// MessageText:
//
//  (ClassLib) An I/O error occurred in the persistence interface.  See the extended status code for the NT-specific error that occurred.
//
#define CLASSLIB_PERSISTENCE_IO_ERROR    (DWORD)0xE20A001DL

//
// Database error codes.
//
//
// MessageId: CLASSLIB_ODBC_ERROR
//
// MessageText:
//
//  (ClassLib) An ODBC error occurred.  See the extended status code for the ODBC-specific error that occurred.
//
#define CLASSLIB_ODBC_ERROR              (DWORD)0xE20A001EL

//
// MessageId: CLASSLIB_ODBC_CONNECT_ERROR
//
// MessageText:
//
//  (ClassLib) An ODBC error occurred.  A connection using the specified SQL driver could not be established.  Please check your connection parameters.
//
#define CLASSLIB_ODBC_CONNECT_ERROR      (DWORD)0xE20A001FL

//
// MessageId: CLASSLIB_DATABASE_NO_SERVERS_CONFIGURED
//
// MessageText:
//
//  (ClassLib) Unable to establish multi-rail database connection because no server names were specified.
//
#define CLASSLIB_DATABASE_NO_SERVERS_CONFIGURED (DWORD)0xE20A0020L

//
// Msme error codes.
//
//
// MessageId: CLASSLIB_MSME_BAD_SHANDLE
//
// MessageText:
//
//  (ClassLib) Unable to queue an event to a MSME state-machine because the state-machine handle was invalid.
//
#define CLASSLIB_MSME_BAD_SHANDLE        (DWORD)0xE20A0021L

//
// MessageId: CLASSLIB_MSME_CREATE_SM_ERROR
//
// MessageText:
//
//  (ClassLib) Unable to create a new instance of a MSME state-machine.
//
#define CLASSLIB_MSME_CREATE_SM_ERROR    (DWORD)0xE20A0022L

//
// MessageId: CLASSLIB_MSME_SM_ALREADY_CREATED
//
// MessageText:
//
//  (ClassLib) Unable to create a new instance of a MSME state-machine because it has already been created.  The state-machine must be destroyed first.
//
#define CLASSLIB_MSME_SM_ALREADY_CREATED (DWORD)0xE20A0023L

//
// LoadLibrary error codes.
//
//
// MessageId: CLASSLIB_LOADLIBRARY_ENTRY_POINT_NOT_FOUND
//
// MessageText:
//
//  (ClassLib) The specified entry-point could not be located in the loaded library.
//
#define CLASSLIB_LOADLIBRARY_ENTRY_POINT_NOT_FOUND (DWORD)0xE20A0024L

//
// RegionalSetting error codes.
//
//
// MessageId: CLASSLIB_REGIONALSETTING_BAD_CURRENCY_FORMAT
//
// MessageText:
//
//  (ClassLib) Unable to format currency.
//
#define CLASSLIB_REGIONALSETTING_BAD_CURRENCY_FORMAT (DWORD)0xE20A0025L

//*******************************************************************************
//*
//* ProtLib Messages
//*
//*******************************************************************************
// Error Messages
//
// MessageId: PROTLIB_TLV_BUFFER_OVERRUN
//
// MessageText:
//
//  (ProtLib) A buffer was overrun while traversing a Type Length Value (TLV) buffer.
//
#define PROTLIB_TLV_BUFFER_OVERRUN       (DWORD)0xE20B0000L

//
// MessageId: PROTLIB_TLV_BUFFER_UNDERRUN
//
// MessageText:
//
//  (ProtLib) A buffer was underrun while traversing a Type Length Value (TLV) buffer.
//
#define PROTLIB_TLV_BUFFER_UNDERRUN      (DWORD)0xE20B0001L

//
// MessageId: PROTLIB_TLV_COUNT_MISMATCH
//
// MessageText:
//
//  (ProtLib) The number of types specified does not match the number found while traversing the Type Length Value (TLV) buffer.
//
#define PROTLIB_TLV_COUNT_MISMATCH       (DWORD)0xE20B0002L

//
// MessageId: PROTLIB_CONNMGR_ALREADY_LISTENING
//
// MessageText:
//
//  (ProtLib) Unable to start a protocol listener because one is already started.
//
#define PROTLIB_CONNMGR_ALREADY_LISTENING (DWORD)0xE20B0003L

//
// MessageId: PROTLIB_CONNMGR_NOT_LISTENING
//
// MessageText:
//
//  (ProtLib) Unable to stop a protocol listener because one isn't listening.
//
#define PROTLIB_CONNMGR_NOT_LISTENING    (DWORD)0xE20B0004L

//
// MessageId: PROTLIB_CONNMGR_CONNECTION_EXISTS
//
// MessageText:
//
//  (ProtLib) Unable to complete operation because the specified connection already exists.
//
#define PROTLIB_CONNMGR_CONNECTION_EXISTS (DWORD)0xE20B0005L

//
// MessageId: PROTLIB_CONNMGR_CONNECTION_DOES_NOT_EXIST
//
// MessageText:
//
//  (ProtLib) Unable to complete operation because the specified connection does not exist.
//
#define PROTLIB_CONNMGR_CONNECTION_DOES_NOT_EXIST (DWORD)0xE20B0006L

//
// MessageId: PROTLIB_INVALID_SOCK_TYPE
//
// MessageText:
//
//  (ProtLib) Unable to complete operation because an invalid socket type (SOCK_...) was specified.
//
#define PROTLIB_INVALID_SOCK_TYPE        (DWORD)0xE20B0007L

//
// MessageId: PROTLIB_DSMCC_INVALID_MESSAGE_TYPE
//
// MessageText:
//
//  (ProtLib) An invalid DSM-CC message type was specified.
//
#define PROTLIB_DSMCC_INVALID_MESSAGE_TYPE (DWORD)0xE20B0008L

//
// MessageId: PROTLIB_DSMCC_INVALID_RESOURCE_TYPE
//
// MessageText:
//
//  (ProtLib) An invalid DSM-CC resource type was specified.
//
#define PROTLIB_DSMCC_INVALID_RESOURCE_TYPE (DWORD)0xE20B0009L

//
// MessageId: PROTLIB_DSMCC_INVALID_PROTOCOL_DISCRIMINATOR
//
// MessageText:
//
//  (ProtLib) An invalid DSM-CC protocol discriminator was specified.
//
#define PROTLIB_DSMCC_INVALID_PROTOCOL_DISCRIMINATOR (DWORD)0xE20B000AL

//
// MessageId: PROTLIB_DSMCC_INVALID_DSMCC_TYPE
//
// MessageText:
//
//  (ProtLib) An invalid DSM-CC type was specified.
//
#define PROTLIB_DSMCC_INVALID_DSMCC_TYPE (DWORD)0xE20B000BL

//
// MessageId: PROTLIB_DSMCC_UNSUPPORTED_MESSAGE_TYPE
//
// MessageText:
//
//  (ProtLib) An unsupported DSM-CC message type was specified.
//
#define PROTLIB_DSMCC_UNSUPPORTED_MESSAGE_TYPE (DWORD)0xE20B000CL

//
// MessageId: PROTLIB_DSMCC_BAD_RESOURCE_LENGTH
//
// MessageText:
//
//  (ProtLib) An incorrect resource length was encountered while parsing resource data.
//
#define PROTLIB_DSMCC_BAD_RESOURCE_LENGTH (DWORD)0xE20B000DL

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while parsing the message.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN     (DWORD)0xE20B000EL

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_HEADER
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while parsing the DSM-CC message header.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_HEADER (DWORD)0xE20B000FL

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_ADAPTATION_HEADER
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while parsing the DSM-CC message adaptation header.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_ADAPTATION_HEADER (DWORD)0xE20B0010L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_MESSAGE_LENGTH
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message is smaller than what is specified as the message length in the DSM-CC header.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_MESSAGE_LENGTH (DWORD)0xE20B0011L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_MESSAGE_DATA
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while parsing the body of the message.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_MESSAGE_DATA (DWORD)0xE20B0012L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_RESOURCE_COUNT
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while reading the number (count) of resources contained in the message.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_RESOURCE_COUNT (DWORD)0xE20B0013L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_RESOURCE_HEADER
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while parsing a resource descriptor header.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_RESOURCE_HEADER (DWORD)0xE20B0014L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_RESOURCE
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while parsing a resource descriptor.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_RESOURCE (DWORD)0xE20B0015L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_UUDATA_COUNT
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while reading the number (count) of UU data bytes.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_UUDATA_COUNT (DWORD)0xE20B0016L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_UUDATA
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while reading the UU data.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_UUDATA (DWORD)0xE20B0017L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_PRIVATEDATA_COUNT
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while reading the number (count) of private data bytes.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_PRIVATEDATA_COUNT (DWORD)0xE20B0018L

//
// MessageId: PROTLIB_DSMCC_BUFFER_OVERRUN_IN_PRIVATEDATA
//
// MessageText:
//
//  (ProtLib) The buffer containing a DSM-CC message was overrun while reading the private data.
//
#define PROTLIB_DSMCC_BUFFER_OVERRUN_IN_PRIVATEDATA (DWORD)0xE20B0019L

//
// MessageId: PROTLIB_DSMCC_FACTORY_WRONG_MESSAGE_ID
//
// MessageText:
//
//  (ProtLib) An incorrect call to finalize a message under production was used by a factory method.
//
#define PROTLIB_DSMCC_FACTORY_WRONG_MESSAGE_ID (DWORD)0xE20B001AL

//
// MessageId: PROTLIB_DSMCC_CONNECTION_EXISTS
//
// MessageText:
//
//  (ProtLib) A connection to the specified Network Service Access Point (NSAP) exists.
//
#define PROTLIB_DSMCC_CONNECTION_EXISTS  (DWORD)0xE20B001BL

//
// MessageId: PROTLIB_DSMCC_CONNECTION_DOES_NOT_EXIST
//
// MessageText:
//
//  (ProtLib) A connection to the specified Network Service Access Point (NSAP) does not exist.
//
#define PROTLIB_DSMCC_CONNECTION_DOES_NOT_EXIST (DWORD)0xE20B001CL

//
// MessageId: PROTLIB_DSMCC_SSP_UNSUPPORTED_PROTOCOL_ID
//
// MessageText:
//
//  (ProtLib) An unsupported Session Setup Protocol (SSP) protocol ID was specified.
//
#define PROTLIB_DSMCC_SSP_UNSUPPORTED_PROTOCOL_ID (DWORD)0xE20B001DL

//
// MessageId: PROTLIB_DSMCC_SSP_UNSUPPORTED_PROTOCOL_VERSION
//
// MessageText:
//
//  (ProtLib) An unsupported Session Setup Protocol (SSP) protocol version was specified.
//
#define PROTLIB_DSMCC_SSP_UNSUPPORTED_PROTOCOL_VERSION (DWORD)0xE20B001EL

//
// MessageId: PROTLIB_DSMCC_SSP_CORRUPTED_BUFFER
//
// MessageText:
//
//  (ProtLib) A corrupted Session Setup Protocol (SSP) buffer was detected.
//
#define PROTLIB_DSMCC_SSP_CORRUPTED_BUFFER (DWORD)0xE20B001FL

//
// MessageId: PROTLIB_DSMCC_SSPSC_UNSUPPORTED_PROTOCOL_ID
//
// MessageText:
//
//  (ProtLib) An unsupported SeaChange Session Setup Protocol (SC SSP) protocol ID was specified.
//
#define PROTLIB_DSMCC_SSPSC_UNSUPPORTED_PROTOCOL_ID (DWORD)0xE20B0020L

//
// MessageId: PROTLIB_DSMCC_SSPSC_UNSUPPORTED_PROTOCOL_VERSION
//
// MessageText:
//
//  (ProtLib) An unsupported SeaChange Session Setup Protocol (SC SSP) protocol version was specified.
//
#define PROTLIB_DSMCC_SSPSC_UNSUPPORTED_PROTOCOL_VERSION (DWORD)0xE20B0021L

//
// MessageId: PROTLIB_DSMCC_SSPSC_CORRUPTED_BUFFER
//
// MessageText:
//
//  (ProtLib) A corrupted SeaChange Session Setup Protocol (SSP) buffer was detected.
//
#define PROTLIB_DSMCC_SSPSC_CORRUPTED_BUFFER (DWORD)0xE20B0022L

//
// MessageId: PROTLIB_LSC_BUFFER_OVERRUN
//
// MessageText:
//
//  (ProtLib) The buffer containing a LSC message was overrun while parsing the message.
//
#define PROTLIB_LSC_BUFFER_OVERRUN       (DWORD)0xE20B0023L

//
// MessageId: PROTLIB_LSC_BUFFER_OVERRUN_IN_HEADER
//
// MessageText:
//
//  (ProtLib) The buffer containing a LSC message was overrun while parsing the LSC message header.
//
#define PROTLIB_LSC_BUFFER_OVERRUN_IN_HEADER (DWORD)0xE20B0024L

//
// MessageId: PROTLIB_LSC_UNSUPPORTED_PROTOCOL_VERSION
//
// MessageText:
//
//  (ProtLib) An unsupported LSC protocol version was specified.
//
#define PROTLIB_LSC_UNSUPPORTED_PROTOCOL_VERSION (DWORD)0xE20B0025L

//
// MessageId: PROTLIB_LSC_BUFFER_OVERRUN_IN_MESSAGE_LENGTH
//
// MessageText:
//
//  (ProtLib) The buffer containing a LSC message is smaller than what is defined as the size should be given the operation code.
//
#define PROTLIB_LSC_BUFFER_OVERRUN_IN_MESSAGE_LENGTH (DWORD)0xE20B0026L

//
// MessageId: PROTLIB_LSC_BUFFER_OVERRUN_IN_MESSAGE_DATA
//
// MessageText:
//
//  (ProtLib) The buffer containing a LSC message was overrun while parsing the body of the message.
//
#define PROTLIB_LSC_BUFFER_OVERRUN_IN_MESSAGE_DATA (DWORD)0xE20B0027L

//
// MessageId: PROTLIB_LSC_INVALID_MESSAGE_SIZE
//
// MessageText:
//
//  (ProtLib) A LSC message was received with an invalid size.  The message cannot be processed.
//
#define PROTLIB_LSC_INVALID_MESSAGE_SIZE (DWORD)0xE20B0028L

//
// MessageId: PROTLIB_RTSP_CONNECTION_EXISTS
//
// MessageText:
//
//  (ProtLib) A connection to the specified RTSP Network Service Access Point (NSAP) exists.
//
#define PROTLIB_RTSP_CONNECTION_EXISTS   (DWORD)0xE20B0029L

//
// MessageId: PROTLIB_RTSP_CONNECTION_DOES_NOT_EXIST
//
// MessageText:
//
//  (ProtLib) A connection to the specified RTSP Network Service Access Point (NSAP) does not exist.
//
#define PROTLIB_RTSP_CONNECTION_DOES_NOT_EXIST (DWORD)0xE20B002AL

//
// MessageId: PROTLIB_RTSP_BAD_REQUEST_LINE_STRUCTURE
//
// MessageText:
//
//  (ProtLib) The syntax of the RTSP Request line is incorrect.
//
#define PROTLIB_RTSP_BAD_REQUEST_LINE_STRUCTURE (DWORD)0xE20B002BL

//
// MessageId: PROTLIB_RTSP_BAD_RESPONSE_LINE_STRUCTURE
//
// MessageText:
//
//  (ProtLib) The syntax of the RTSP Response line is incorrect.
//
#define PROTLIB_RTSP_BAD_RESPONSE_LINE_STRUCTURE (DWORD)0xE20B002CL

//
// MessageId: PROTLIB_RTSP_BAD_MESSAGE_STRUCTURE
//
// MessageText:
//
//  (ProtLib) The syntax of the RTSP Message is incorrect.
//
#define PROTLIB_RTSP_BAD_MESSAGE_STRUCTURE (DWORD)0xE20B002DL

//
// MessageId: PROTLIB_RTSP_BAD_HEADER_STRUCTURE
//
// MessageText:
//
//  (ProtLib) The syntax of the RTSP Header is incorrect.
//
#define PROTLIB_RTSP_BAD_HEADER_STRUCTURE (DWORD)0xE20B002EL

//
// MessageId: PROTLIB_RTSP_UNKNOWN_METHOD
//
// MessageText:
//
//  (ProtLib) An unknown RTSP Request Method has been encountered.
//
#define PROTLIB_RTSP_UNKNOWN_METHOD      (DWORD)0xE20B002FL

//
// MessageId: PROTLIB_RTSP_NO_HEADERS
//
// MessageText:
//
//  (ProtLib) A random RTSP message has been encountered.
//
#define PROTLIB_RTSP_NO_HEADERS          (DWORD)0xE20B0030L

//
// MessageId: PROTLIB_RTSP_ERROR_SERIALIZING
//
// MessageText:
//
//  (ProtLib) An error was encountered while attempting to serialize the RTSP Message.
//
#define PROTLIB_RTSP_ERROR_SERIALIZING   (DWORD)0xE20B0031L

//
// MessageId: PROTLIB_RTSP_INVALID_MESSAGE_SIZE
//
// MessageText:
//
//  (ProtLib) The RTSP request message exceeded the maximum allowable size.
//
#define PROTLIB_RTSP_INVALID_MESSAGE_SIZE (DWORD)0xE20B0032L

// Warning Messages
//
// MessageId: PROTLIB_TEMPLATE_WARNING_MESSAGE
//
// MessageText:
//
//  (ProtLib) This is a template warning message.
//
#define PROTLIB_TEMPLATE_WARNING_MESSAGE (DWORD)0xA20B0000L

// Informational Messages
//
// MessageId: PROTLIB_LSC_SERVER_SEACHANGE_LSC
//
// MessageText:
//
//  (ProtLib) The LSC server is using "SeaChange LSC" as opposed to "Standard LSC".
//
#define PROTLIB_LSC_SERVER_SEACHANGE_LSC (DWORD)0x620B0000L

//*******************************************************************************
//*
//* SeaCOMSMTP Messages
//*
//*******************************************************************************
//
// MessageId: SEACOMSMTP_ERROR
//
// MessageText:
//
//  (SeaCOMSMTP) SeaCOMSMTP error:- %1.
//
#define SEACOMSMTP_ERROR                 (DWORD)0xE20C0000L

//
// MessageId: SEACOMSMTP_CONNECT_ERROR
//
// MessageText:
//
//  (SeaCOMSMTP) Error connecting to SMTP server! %1.
//
#define SEACOMSMTP_CONNECT_ERROR         (DWORD)0xE20C0001L

//*******************************************************************************
//*
//* Data Warehouse Messages
//*
//*******************************************************************************
//
// MessageId: DWH_IMPORT_FAILURE
//
// MessageText:
//
//  A Data Warehouse import job failed: %1 %2
//
#define DWH_IMPORT_FAILURE               (DWORD)0xE20D0000L

//
// MessageId: DWH_BACKUP_FAILURE
//
// MessageText:
//
//  A Data Warehouse database backup failed: %1 %2
//
#define DWH_BACKUP_FAILURE               (DWORD)0xE20D0001L

//
// MessageId: DWH_FTP_UPLOAD_FAILURE
//
// MessageText:
//
//  A Data Warehouse FTP upload failed: %1 %2
//
#define DWH_FTP_UPLOAD_FAILURE           (DWORD)0xE20D0002L

//
// MessageId: DWH_SCHEDULED_REPORT_FAILURE
//
// MessageText:
//
//  A Data Warehouse scheduled report failed: %1 %2
//
#define DWH_SCHEDULED_REPORT_FAILURE     (DWORD)0xE20D0003L

//
// MessageId: DWH_SCHEDULED_MAINTENTANCE_FAILURE
//
// MessageText:
//
//  A Data Warehouse scheduled report failed: %1 %2
//
#define DWH_SCHEDULED_MAINTENTANCE_FAILURE (DWORD)0xE20D0004L

//*******************************************************************************
//*
//* SynApi Messages
//*
//*******************************************************************************
// Error Messages
//
// MessageId: SYNAPI_DUPLICATE_TYPEINST
//
// MessageText:
//
//  (SynApi) An action could not be performed due to a duplicate type-inst being
//  specified.
//
#define SYNAPI_DUPLICATE_TYPEINST        (DWORD)0xE20E0000L

//
// MessageId: SYNAPI_UNKNOWN_TYPEINST
//
// MessageText:
//
//  (SynApi) An action could not be performed due to an unknown type-inst being
//  specified.
//
#define SYNAPI_UNKNOWN_TYPEINST          (DWORD)0xE20E0001L

//
// MessageId: SYNAPI_MSME_DEFINE_ERROR
//
// MessageText:
//
//  (SynApi) The synchronizer module can not continue due to the inability to define
//  a MSME state machine type.
//
#define SYNAPI_MSME_DEFINE_ERROR         (DWORD)0xE20E0002L

//
// MessageId: SYNAPI_TCPSTATE_INIT_ERROR
//
// MessageText:
//
//  (SynApi) The synchronizer could not properly initialize due to a MtTcpComm
//  initialization failure.
//
#define SYNAPI_TCPSTATE_INIT_ERROR       (DWORD)0xE20E0003L

//
// MessageId: SYNAPI_TCPSTATE_START_LISTENER_ERROR
//
// MessageText:
//
//  (SynApi) The synchronizer could not properly start a listener for protocol
//  messages.
//
#define SYNAPI_TCPSTATE_START_LISTENER_ERROR (DWORD)0xE20E0004L

//
// MessageId: SYNAPI_UNKNOWN_IP_ADDRESS
//
// MessageText:
//
//  (SynApi) Unable to translate an IP address into a TYPE/INST because a channel
//  has not yet been successfully opened.
//
#define SYNAPI_UNKNOWN_IP_ADDRESS        (DWORD)0xE20E0005L

// Warning Messages
// Informational Messages
