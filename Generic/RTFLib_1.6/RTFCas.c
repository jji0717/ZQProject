// implementation file for rtfCas class
// provides Conditional Access System-specific support functions
//

#include "RTFPrv.h"

// class definition files
#include "RTFCas.h"
#include "RTFCasPrv.h"

// constants ****************************************************************************

#define RTF_CAS_MAX_RECOG_PKTS		1024

// private functions ********************************************************************

static RTF_RESULT rtfCasUnknownProcessInputPkt( RTF_CAS *pCas, RTF_PKT_HANDLE hPkt )
{
	RTF_FNAME( "rtfCasUnknownProcessInputPkt" );
	RTF_OBASE( pCas );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		// has the input packet count reached the point where we should
		// have recognized the flavor of the CAS system being used?
		if( pCas->inputPktCount >= RTF_CAS_MAX_RECOG_PKTS )
		{
			// yes. have any encrypted packets been encountered?
			if( pCas->cryptoPktCount > 0 )
			{
				// yes. if we haven't recognized it by now, we aren't going to.
				RTF_LOG_WARN0( RTF_MSG_ERR_BADSTREAM, "Unknown CA system in use" );
			}
			// assume no CA system in use
			pCas->casType = RTF_CAS_TYPE_NULL;
			// set state to ready
			pCas->state = RTF_CASSTATE_READY;
			break;
		}

#ifdef DO_CAS_VMX
		// see if Verimatrix CA is in use
		result = rtfCasVmxProcessInputPkt( pCas, hPkt );
		RTF_CHK_RESULT;
		// escape if it is
		if( pCas->casType != RTF_CAS_TYPE_UNKNOWN )
		{
			break;
		}
#endif

#ifdef DO_CAS_PAN
		// see if Panasonic CA is in use
		// !!! FIX ME !!! PANASONIC CAS RECOGNITION !!!
#endif

		// !!! add calls to detection functions for new CAS types here !!!

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfCasGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfCasGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_CAS);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfCasConstructor( RTF_CAS_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfCasConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_CAS *pCas;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the CAS object
		pCas = (RTF_CAS *)rtfAlloc( sizeof(RTF_CAS) );
		RTF_CHK_ALLOC( pCas );
		// return the handle
		*pHandle = (RTF_CAS_HANDLE)pCas;
		// clear the state structure
		memset( (void *)pCas, 0, sizeof(*pCas) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_CAS, (RTF_HANDLE)pCas, hParent, &pCas->hBaseObject );
		RTF_CHK_RESULT;
		// reset the CAS object
		rtfCasReset( pCas );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfCasDestructor( RTF_CAS_HANDLE handle )
{
	RTF_FNAME( "rtfCasDestructor" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded base object
		result = rtfObjDestructor( pCas->hBaseObject, RTF_OBJ_TYPE_CAS );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// return the state of the CAS object
RTF_RESULT rtfCasGetState( RTF_CAS_HANDLE handle, RTF_CASSTATE *pState )
{
	RTF_FNAME( "rtfCasGetState" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		// make the return
		*pState = pCas->state;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service method functions *************************************************************

// reset a CAS object to the "unknown" state
RTF_RESULT rtfCasReset( RTF_CAS_HANDLE handle )
{
	RTF_FNAME( "rtfCasReset" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		// get the session handle
		result = rtfObjGetSession( (RTF_HANDLE)pCas, &pCas->hSes );
		RTF_CHK_RESULT;
		// reset the input packet count
		pCas->inputPktCount = 0;
		// reset the encrypted input packet count
		pCas->cryptoPktCount = 0;
		// reset the CAS-specific information
#ifdef DO_CAS_VMX
		rtfCasVmxReset( pCas );
#endif
#ifdef DO_CAS_PAN
		rtfCasPanReset( pCas );
#endif
		// set the CAS type to "unknown"
		pCas->casType = RTF_CAS_TYPE_UNKNOWN;
		// set the state to RESET
		pCas->state = RTF_CASSTATE_RESET;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// scan an input buffer for CAS-related info
RTF_RESULT rtfCasProcessInputBuf( RTF_CAS_HANDLE handle, RTF_BUF_HANDLE hBuf,
								  RTF_CASSTATE *pState )
{
	RTF_FNAME( "rtfCasProcessInputBuf" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE *phPkt;
	int i;
	unsigned long flags;
	unsigned short packetCount;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		RTF_CHK_STATE_NE( pCas, RTF_CASSTATE_LOCKED );
		// get the packet array info from the buffer
		result = rtfBufGetPacketArrayInfo( hBuf, &packetCount, &phPkt );
		RTF_CHK_RESULT;
		// iterate over the packets in the array
		for( i=0; i<packetCount; ++i )
		{
			// get the flags from this packet
			result = rtfPktGetFlags( phPkt[ i ], &flags );
			RTF_CHK_RESULT;
			// does this packet have an encrypted payload?
			if( ( flags & RTF_PKT_PAYLOADENCRYPTED ) != 0 )
			{
				// yes - bump the encrypted packet count
				++pCas->cryptoPktCount;
			}
			// call the CAS-specific input packet processing function
			switch( pCas->casType )
			{
			case RTF_CAS_TYPE_UNKNOWN:
				result = rtfCasUnknownProcessInputPkt( pCas, phPkt[ i ] );
				RTF_CHK_RESULT;
				break;
			case RTF_CAS_TYPE_NULL:
				// no need to do any processing
				break;
#ifdef DO_CAS_VMX
			case RTF_CAS_TYPE_VMX:
				result = rtfCasVmxProcessInputPkt( pCas, phPkt[ i ] );
				RTF_CHK_RESULT;
				break;
#endif
#ifdef DO_CAS_PAN
			case RTF_CAS_TYPE_PAN:
				result = rtfCasPanProcessInputPkt( pCas, phPkt[ i ] );
				RTF_CHK_RESULT;
				break;
#endif
			default:
				RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unrecognized CAS type (%d)", pCas->casType );
			}
			RTF_CHK_RESULT_LOOP;
			// bump the input packet count
			++pCas->inputPktCount;
		}
		// return the updated state
		*pState = pCas->state;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a sequence and insert any required CAS-related packets
RTF_RESULT rtfCasProcessSeq( RTF_CAS_HANDLE handle, RTF_SEQ_HANDLE hSeq )
{
	RTF_FNAME( "rtfCasProcessSeq" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		RTF_CHK_STATE_NE( pCas, RTF_CASSTATE_RESET );
		// call the CAS-specific sequence processing function
		switch( pCas->casType )
		{
		case RTF_CAS_TYPE_NULL:
			// no need to do any processing
			break;
#ifdef DO_CAS_VMX
		case RTF_CAS_TYPE_VMX:
			result = rtfCasVmxProcessSeq( pCas, hSeq );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_CAS_PAN
		case RTF_CAS_TYPE_PAN:
			result = rtfCasPanProcessSeq( pCas, hSeq );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unrecognized CAS type (%d)", pCas->casType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a group and insert any required CAS-related packets
RTF_RESULT rtfCasProcessGop( RTF_CAS_HANDLE handle, RTF_GOP_HANDLE hGop )
{
	RTF_FNAME( "rtfCasProcessGop" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		RTF_CHK_STATE_NE( pCas, RTF_CASSTATE_RESET );
		// call the CAS-specific group processing function
		switch( pCas->casType )
		{
		case RTF_CAS_TYPE_NULL:
			// no need to do any processing
			break;
#ifdef DO_CAS_VMX
		case RTF_CAS_TYPE_VMX:
			result = rtfCasVmxProcessGop( pCas, hGop );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_CAS_PAN
		case RTF_CAS_TYPE_PAN:
			result = rtfCasPanProcessGop( pCas, hGop );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unrecognized CAS type (%d)", pCas->casType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a picture and insert any required CAS-related packets
RTF_RESULT rtfCasProcessPic( RTF_CAS_HANDLE handle, RTF_PIC_HANDLE hPic )
{
	RTF_FNAME( "rtfCasProcessPic" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		RTF_CHK_STATE_NE( pCas, RTF_CASSTATE_RESET );
		// call the CAS-specific picture processing function
		switch( pCas->casType )
		{
		case RTF_CAS_TYPE_NULL:
			// no need to do any processing
			break;
#ifdef DO_CAS_VMX
		case RTF_CAS_TYPE_VMX:
			result = rtfCasVmxProcessPic( pCas, hPic );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_CAS_PAN
		case RTF_CAS_TYPE_PAN:
			result = rtfCasPanProcessPic( pCas, hPic );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unrecognized CAS type (%d)", pCas->casType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a packet and insert any required CAS-related packets
RTF_RESULT rtfCasProcessPkt( RTF_CAS_HANDLE handle, RTF_PKT_HANDLE hPkt, RTF_OUT_HANDLE hOut )
{
	RTF_FNAME( "rtfCasProcessPkt" );
	RTF_OBASE( handle );
	RTF_CAS *pCas = (RTF_CAS *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCas, RTF_OBJ_TYPE_CAS );
		RTF_CHK_STATE_NE( pCas, RTF_CASSTATE_RESET );
		// call the CAS-specific picture processing function
		switch( pCas->casType )
		{
		case RTF_CAS_TYPE_NULL:
			// no need to do any processing
			break;
#ifdef DO_CAS_VMX
		case RTF_CAS_TYPE_VMX:
			result = rtfCasVmxProcessPkt( pCas, hPkt, hOut );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_CAS_PAN
		case RTF_CAS_TYPE_PAN:
			result = rtfCasPanProcessPkt( pCas, hPkt, hOut );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unrecognized CAS type (%d)", pCas->casType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
