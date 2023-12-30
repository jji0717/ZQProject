// definition file for program access table class
//

#ifndef _RTF_PAT_H
#define _RTF_PAT_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPatGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfPatConstructor( RTF_PAT_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfPatDestructor( RTF_PAT_HANDLE handle );

// accessor methods *********************************************************************

// retrieve the PMT pid from the recorded PAT packet (returns INVALID if not yet captured)
RTF_RESULT rtfPatGetPmtPid( RTF_PAT_HANDLE handle, unsigned short *pPmtPid );

// retrieve the program number from the recorded PAT packet (returns 0 if not yet captured)
RTF_RESULT rtfPatGetProgramNumber( RTF_PAT_HANDLE handle, unsigned short *pProgramNumber );

// retrieve a pointer to the recorded PAT packet (returns NULL if not yet captured)
RTF_RESULT rtfPatGetTable( RTF_PAT_HANDLE handle, unsigned char **ppTable );

// service methods **********************************************************************

// reset a PAT object
RTF_RESULT rtfPatReset( RTF_PAT_HANDLE handle );

// validate a PAT object
RTF_RESULT rtfPatValidate( RTF_PAT_HANDLE handle, BOOL *pIsValid );

// parse a PAT
// NOTE: this version assumes that all transport streams are
// single programs, and all PATs are a single packet
RTF_RESULT rtfPatParse( RTF_PAT_HANDLE handle, RTF_PKT_HANDLE hPacket );

// check a PAT packet to see if the table has changed
RTF_RESULT rtfPatCheckChange( RTF_PAT_HANDLE handle, RTF_PKT_HANDLE hPkt );

#endif // #ifndef _RTF_PAT_H
