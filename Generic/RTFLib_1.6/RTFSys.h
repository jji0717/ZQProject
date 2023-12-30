// definition file for rtfSys class
// encapsulates all hardware / OS dependencies of sys
// also serves as embedded resource manager 
//

#ifndef _RTF_SYS_H
#define _RTF_SYS_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to rtfSysConstructor (below)
unsigned long rtfSysGetStorageRequirement( unsigned long maxSessionCount,
					unsigned long maxInputBuffersPerSession, unsigned long maxGroupsPerSequence,
					unsigned long maxPicturesPerGroup, unsigned long maxInputBufferBytes );

// constructor / destructor *************************************************************

// construct a real-time trickfile library system object (once only!)
RTF_RESULT rtfSysConstructor( RTF_APP_HEAP_HANDLE hAppHeap, RTF_ALLOC_FUNCTION allocFunc,
							  RTF_FREE_FUNCTION freeFunc, RTF_LOG_FUNCTION logFunc,
							  RTF_LIBRARY_ERROR_NOTIFIER notifier,
							  unsigned long maxSessionCount, unsigned long maxInputBuffersPerSession,
							  unsigned long maxGroupsPerSequence, unsigned long maxPicturesPerGroup,
							  unsigned long maxInputBufferBytes, int sessionFailThreshold );

// destroy a real-time trickfile library system object
RTF_RESULT rtfSysDestructor();

// service methods **********************************************************************

// get the priority and the short form (<32 bytes for embedded!) of a message string
void rtfSysGetMsgInfo( RTF_MSG msg, RTF_MSG_PRIO *pMsgPrio, char **ppShortMsgStr );

// notify the user that set up the library of an error
void rtfSysErrorNotifier( char *pMessage );

// get the default input stream profile for the system
RTF_RESULT rtfSysGetStreamProfile( RTF_STREAM_PROFILE **ppProfile );

#if ( defined(_EMBEDDED) && defined(_LINUX) )
// helper function to do 64 bit divisions (ARM compiler has a bug)
INT64 RTFDiv64( INT64 numerator, INT64 denominator );
#endif

#endif // #ifndef _RTF_SYS_H
