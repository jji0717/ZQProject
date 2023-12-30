// implementation file for rtfAcd class VC-1 sub-class
// provides AC3-specific functions
//

#include "RTFPrv.h"

#ifdef DO_ACD_AC3

#include "RTFAcdAC3.h"
#include "RTFAcdPrv.h"

// look-up tables ***********************************************************************

// AC3 specific private functions *******************************************************

// AC3 specific public functions ********************************************************

// reset the AC3-specific state info
void rtfAcdAC3Reset( P_RTF_ACD pAcd )
{
	// !!! FIX ME !!! AC3 RESET !!!
}

// retreive the AC3 silence frame info
RTF_RESULT rtfAcdAC3GetSilenceFrame( RTF_ACD_HANDLE handle, unsigned char **ppData, unsigned long *pBytes )
{
	// !!! FIX ME !!! AC3 GET SILENCE FRAME !!!
	return RTF_PASS;
}

#endif // #ifdef DO_ACD_AC3
