// definition file for program map table class
//

#ifndef _RTF_PMT_H
#define _RTF_PMT_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPmtGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfPmtConstructor( RTF_PMT_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfPmtDestructor( RTF_PMT_HANDLE handle );

// accessor methods *********************************************************************

// retrieve a pointer to the recorded PMT packet (returns NULL if not yet captured)
RTF_RESULT rtfPmtGetPacket( RTF_PMT_HANDLE handle, unsigned char **ppPacket );

// retrieve the stream type of a particular PID
RTF_RESULT rtfPmtGetStreamType( RTF_PMT_HANDLE handle, unsigned short pid,
							    RTF_STREAM_TYPE *pStreamType );

// retrieve the stream types of all known PIDs
RTF_RESULT rtfPmtGetAllStreamTypes( RTF_PMT_HANDLE handle,  RTF_STREAM_TYPE **ppStreamType );

// service methods **********************************************************************

// reset a PMT object
RTF_RESULT rtfPmtReset( RTF_PMT_HANDLE handle );

// validate a PMT object
RTF_RESULT rtfPmtValidate( RTF_PMT_HANDLE handle, BOOL *pIsValid );

// parse a PMT
// NOTE: this version assumes that all transport streams are
// single programs, and all PMTs are a single packet
RTF_RESULT rtfPmtParse( RTF_PMT_HANDLE handle, RTF_PKT_HANDLE hPacket );

// check a PMT packet to see if the table has changed
RTF_RESULT rtfPmtCheckChange( RTF_PMT_HANDLE handle, RTF_PKT_HANDLE hPkt );

#endif // #ifndef _RTF_PMT_H
