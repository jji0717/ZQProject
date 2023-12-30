// definition file for PES packet object
//

#ifndef _RTF_PES_H
#define _RTF_PES_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPesGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfPesConstructor( RTF_PES_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfPesDestructor( RTF_PES_HANDLE handle );

// accessor methods *********************************************************************

// retrieve a pointer to the recorded PES packet
RTF_RESULT rtfPesGetHeader( RTF_PES_HANDLE handle, unsigned char **ppHeader, unsigned char *pLength );

// get positional information on a PES packet header
RTF_RESULT rtfPesGetHdrInfo( RTF_PES_HANDLE handle,         unsigned char *pPesHdrLength,
							 RTF_PKT_HANDLE *phPrePesPkt,   unsigned char *pPrePesOffset,
							 RTF_PKT_HANDLE *phFirstPesPkt, unsigned char *pFirstPesOffset,
							 RTF_PKT_HANDLE *phLastPesPkt,  unsigned char *pLastPesOffset,
							 RTF_PKT_HANDLE *phNextPesPkt,  unsigned char *pNextPesOffset );

// get the decoding delay (DTS-PCR) and the presentation delay (PTS-DTS) in 90 KHz clock ticks
RTF_RESULT rtfPesGetDelays( RTF_PES_HANDLE handle, int *pdecodingDelay, int*pPresentationDelay );

// service methods **********************************************************************

// reset a PES object
RTF_RESULT rtfPesReset( RTF_PES_HANDLE handle );

// validate a PAT object
RTF_RESULT rtfPesValidate( RTF_PES_HANDLE handle, BOOL *pIsValid );

// parse a PES packet header
RTF_RESULT rtfPesParse( RTF_PES_HANDLE handle, RTF_WIN_HANDLE hWindow, RTF_INDEX_TYPE indexType );

#endif // #ifndef _RTF_PES_H
