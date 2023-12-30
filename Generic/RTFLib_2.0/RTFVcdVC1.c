// implementation file for rtfVcd class VC-1 sub-class
// provides VC1-specific functions
//

#include "RTFPrv.h"

#ifdef DO_VCD_VC1

#include "RTFVcdPrv.h"

// look-up tables ***********************************************************************

// VC1 specific private functions *******************************************************

static RTF_RESULT rtfGetVC1StartCodeType( RTF_VCD *pVcd, unsigned long code, RTF_STARTCODE_TYPE_VC1 *pType )
{
	RTF_FNAME( "rtfGetVC1StartCodeType" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_STARTCODE_TYPE_H264 type = RTF_STARTCODE_TYPE_VC1_INVALID;

	do {

		// !!! FIX ME !!! START VCDES FOR VC-1 !!!

	} while( 0 ); // error escape wrapper - end

	return result;
}

// VC-1 VCD specific public functions *************************************************

// process a VC-1 start code
RTF_RESULT rtfVcdVC1ProcessStartCode( P_RTF_VCD pVcd, unsigned long code )
{
	RTF_FNAME( "rtfProcessVC1StartCode" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_STARTCODE_TYPE_VC1 type = RTF_STARTCODE_TYPE_VC1_INVALID;

	do {		 // error escape wrapper - begin

		// decode the start code type
		result = rtfGetVC1StartCodeType( pVcd, code, &type );
		RTF_CHK_RESULT;
		switch( type )
		{
		// !!! FIX ME !!! START VCDES FOR VC-1 !!!
		default:
			// not currently interested in any other start code types
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// build the VC-1 no change frames
void rtfVcdVC1BuildNCFrames( P_RTF_VCD pVcd )
{
	// !!! FIX ME !!! ADD NCFRAME IMPLEMENTATION TO VC-1 !!!
}

#endif // #ifdef DO_VCD_VC1
