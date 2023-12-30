#ifndef _STV_SSCTRL_H_
#define _STV_SSCTRL_H_


//  SSAENOTIOP
//
//  SSAENOTIOP - Indicate which operation is performed the ISS.

typedef WORD SSAENOTIOP;
#define					SAENO_PLAY 1	
						// the AE is starting.

#define					SAENO_STOP 2	
						// the AE has stopped

//  SSAENOTISTATUS
//
//  SSAENOTISTATUS - Indicate the operation status returned by the ISS.

typedef DWORD SSAENOTISTATUS;

#define					SAENS_SUCCEED	1
#define					SAENS_FAIL		2

//  STRMERRCODE
//
//  STRMERRCODE - Indicate the Error Code of the current stream.
typedef DWORD STRMERRCODE;
#define					STRM_ABORT		1	// stream aborted
#define					STRM_ABNELEMENT	2	// Abnormal Element
#define					STRM_NOPLAY		3	// Did not get STRMSTART 


//  EXPECTEDOP
//
//  EXPECTEDOP - the excepted operation to the error.
typedef DWORD EXPECTEDOP;
#define					EXOP_NONE			1	// do nothing
#define					EXOP_REPLAY			2	// replay
#define					EXOP_PLAYFILLER		3	// play filler

//-----------------------------------------------------------------------------
//  SSNotification
//
//  SSNotification - This is the asset element's status notification used to
//					 report to the MainControl.

typedef struct _SSAENotification
{
	DWORD			dwPurchaseID;	// Purchase ID
	DWORD			dwAeUID;		// asset element
	SSAENOTIOP		wOperation;		// operation type
	SSAENOTISTATUS	dwStatus;		// operation status
} SSAENotification, *PSSAENotification;

//-----------------------------------------------------------------------------
//  SSStreamNotification
//
//  SSStreamNotification - This is the stream error and excepted operation used to
//					 report to the MainControl.

typedef struct _SSStreamNotification
{
	DWORD			dwPurchaseID;	// Purchase ID
	STRMERRCODE		dwErrorCode;	// Error Code
	EXPECTEDOP		dwOperation;	// Expected Operation to the Error
}SSStreamNotification, *PSSStreamNotification;

#endif
