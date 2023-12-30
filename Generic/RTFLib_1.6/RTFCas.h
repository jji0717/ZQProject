// definition file for CAS class
//

#ifndef _RTF_CAS_H
#define _RTF_CAS_H 1

// typedefs *****************************************************************************

// CAS state enumerator
typedef enum _RTF_CASSTATE
{
	RTF_CASSTATE_INVALID = 0,

	RTF_CASSTATE_RESET,		// initial state; don't know what CA system, if any, is in use
	RTF_CASSTATE_READY,		// CA system recognized; ready to begin processing output packets
	RTF_CASSTATE_LOCKED,	// CA system recognized; ready to stop processing input packets

	RTF_CASSTATE_LIMIT

} RTF_CASSTATE;

// CAS type enumerator

// supported conditional access system types
typedef enum _RTF_CAS_TYPE
{
	RTF_CAS_TYPE_INVALID = 0,

	RTF_CAS_TYPE_UNKNOWN,
	RTF_CAS_TYPE_NULL,
#ifdef DO_CAS_VMX
	RTF_CAS_TYPE_VMX,
#endif
#ifdef DO_CAS_PAN
	RTF_CAS_TYPE_PAN,
#endif

	RTF_CAS_TYPE_LIMIT

} RTF_CAS_TYPE;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfCasGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfCasConstructor( RTF_CAS_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfCasDestructor( RTF_CAS_HANDLE handle );

// accessor methods *********************************************************************

// return the state of the CAS object
RTF_RESULT rtfCasGetState( RTF_CAS_HANDLE handle, RTF_CASSTATE *pState );

// service methods **********************************************************************

// reset a CAS object to the "unknown" state
RTF_RESULT rtfCasReset( RTF_CAS_HANDLE handle );

// scan an input buffer for CAS-related info
RTF_RESULT rtfCasProcessInputBuf( RTF_CAS_HANDLE handle, RTF_BUF_HANDLE hBuf,
								  RTF_CASSTATE *pState );

// process a sequence and insert any required CAS-related packets
RTF_RESULT rtfCasProcessSeq( RTF_CAS_HANDLE handle, RTF_SEQ_HANDLE hSeq );

// process a group and insert any required CAS-related packets
RTF_RESULT rtfCasProcessGop( RTF_CAS_HANDLE handle, RTF_GOP_HANDLE hGop );

// process a pioture and insert any required CAS-related packets
RTF_RESULT rtfCasProcessPic( RTF_CAS_HANDLE handle, RTF_PIC_HANDLE hPic );

// process a packet and insert any required CAS-related packets
RTF_RESULT rtfCasProcessPkt( RTF_CAS_HANDLE handle, RTF_PKT_HANDLE hPkt, RTF_OUT_HANDLE hOut );

#endif // #ifndef _RTF_CAS_H
