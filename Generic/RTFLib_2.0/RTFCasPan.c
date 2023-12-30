// implementation file for rtfCas class, PAN (Panasonic) sub-class
// provides PAN-specific functions
//

#include "RTFPrv.h"

#ifdef DO_CAS_PAN

#include "RTFCas.h"
#include "RTFCasPrv.h"

// PAN specific private functions *******************************************************

// PAN CAS specific public functions ****************************************************

// reset the PAN-specific portion of the CAS state structure
void rtfCasPanReset( P_RTF_CAS pCas )
{
	// !!! FIX ME !!! CAS PAN - ANYTHING TO BE DONE HERE? !!!
	// nothing to do here
}

// process an input packet for PAN-specific CAS information
RTF_RESULT rtfCasPanProcessInputPkt( P_RTF_CAS pCas, RTF_PKT_HANDLE hPkt )
{
	// nothing to do here
	return RTF_PASS;
}

// process a sequence and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessSeq( RTF_CAS_HANDLE handle, RTF_SEQ_HANDLE hSeq )
{
	// nothing to do here
	return RTF_PASS;
}

// process a group and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessGop( RTF_CAS_HANDLE handle, RTF_GOP_HANDLE hGop )
{
	// nothing to do here
	return RTF_PASS;
}

// process a pioture and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessPic( RTF_CAS_HANDLE handle, RTF_PIC_HANDLE hPic )
{
	// nothing to do here
	return RTF_PASS;
}

// process a packet and insert any required PAN-specific CAS-related packets
RTF_RESULT rtfCasPanProcessPkt( RTF_CAS_HANDLE handle, RTF_PKT_HANDLE hPkt, RTF_OUT_HANDLE hOut )
{
	// nothing to do here
	return RTF_PASS;
}

#endif // #ifdef DO_CAS_PAN
