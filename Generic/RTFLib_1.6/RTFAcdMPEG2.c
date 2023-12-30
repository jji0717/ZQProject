// implementation file for rtfAcd class VC-1 sub-class
// provides MPEG2-specific functions
//

#include "RTFPrv.h"

#ifdef DO_ACD_MPEG2

#include "RTFAcdMPEG2.h"
#include "RTFAcdPrv.h"

// look-up tables ***********************************************************************

// MPEG2 specific private functions *******************************************************

// MPEG2 specific public functions ********************************************************

// reset the MPEG2-specific state info
void rtfAcdMPEG2Reset( P_RTF_ACD pAcd )
{
	// !!! FIX ME !!! MPEG2 RESET !!!
}

// retreive the MPEG2 silence frame info
RTF_RESULT rtfAcdMPEG2GetSilenceFrame( RTF_ACD_HANDLE handle, unsigned char **ppData, unsigned long *pBytes )
{
	// !!! FIX ME !!! MPEG2 GET SILENCE FRAME !!!
	return RTF_PASS;
}

#endif // #ifdef DO_ACD_MPEG2
