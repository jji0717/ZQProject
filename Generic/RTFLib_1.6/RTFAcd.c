// implementation file for rtfAcd class
// provides ACD-specific parsing functions
//

#include "RTFPrv.h"

// class definition file
#include "RTFAcdPrv.h"

// private functions ********************************************************************

// reset the audio codec object
static void resetAcd( RTF_ACD *pAcd )
{
	// clear the structure
	memset( (void *)pAcd, 0, sizeof(*pAcd) );
	// initial state is closed
	pAcd->state = RTF_ACDSTATE_CLOSED;
	// audio codec type is invalid
	pAcd->audioSpec.eStream.audio.acdType = RTF_AUDIO_CODEC_TYPE_INVALID;
	// audio PID is invalid
	pAcd->audioSpec.pid = TRANSPORT_INVALID_PID;
	// reset the audio-codec-specific information (don't fail if type not set yet)
	switch( pAcd->audioSpec.eStream.audio.acdType )
	{
#ifdef DO_ACD_AC3
	case RTF_AUDIO_CODEC_TYPE_AC3:
		rtfAcdAC3Reset( pAcd );
		break;
#endif
#ifdef DO_ACD_MPEG2
	case RTF_AUDIO_CODEC_TYPE_MPEG2:
		rtfAcdMPEG2Reset( pAcd );
		break;
#endif
	default:
		memset( (void *)&pAcd->acdInfo, 0, sizeof(pAcd->acdInfo) );
		break;
	}
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfAcdGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfAcdGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_ACD);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfAcdConstructor( RTF_ACD_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfAcdConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_ACD *pAcd;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the audio codec object
		pAcd = (RTF_ACD *)rtfAlloc( sizeof(RTF_ACD) );
		RTF_CHK_ALLOC( pAcd );
		// return the handle
		*pHandle = (RTF_ACD_HANDLE)pAcd;
		// clear the state structure
		memset( (void *)pAcd, 0, sizeof(*pAcd) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_ACD, (RTF_HANDLE)pAcd, hParent, &pAcd->hBaseObject );
		RTF_CHK_RESULT;
		// reset the ACD state structure
		resetAcd( pAcd );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfAcdDestructor( RTF_ACD_HANDLE handle )
{
	RTF_FNAME( "rtfAcdDestructor" );
	RTF_OBASE( handle );
	RTF_ACD *pAcd = (RTF_ACD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded base object
		result = rtfObjDestructor( pAcd->hBaseObject, RTF_OBJ_TYPE_ACD );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get a silence frame (used during splicing operations)
RTF_RESULT rtfAcdGetSilenceFrame( RTF_ACD_HANDLE handle, unsigned char **ppData, unsigned long *pBytes )
{
	RTF_FNAME( "rtfAcdGetSilenceFrame" );
	RTF_OBASE( handle );
	RTF_ACD *pAcd = (RTF_ACD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		switch( pAcd->audioSpec.eStream.audio.acdType )
		{
#ifdef DO_ACD_AC3
		case RTF_AUDIO_CODEC_TYPE_AC3:
			result = rtfAcdAC3GetSilenceFrame( pAcd, ppData, pBytes );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_ACD_MPEG2
		case RTF_AUDIO_CODEC_TYPE_MPEG2:
			result = rtfAcdMPEG2GetSilenceFrame( pAcd, ppData, pBytes );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized audio codec type (%d)",
						  pAcd->audioSpec.eStream.audio.acdType );
		}
		RTF_CHK_RESULT_LOOP;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service method functions *************************************************************

// set up a audio codec object for use with a particular audio stream
RTF_RESULT rtfAcdOpen( RTF_ACD_HANDLE handle, RTF_ESTREAM_SPEC *pAudioSpec )
{
	RTF_FNAME( "rtfAcdOpen" );
	RTF_OBASE( handle );
	RTF_ACD *pAcd = (RTF_ACD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pAcd, RTF_OBJ_TYPE_ACD );
		RTF_CHK_STATE_NE( pAcd, RTF_ACDSTATE_OPEN );
		// record the audio spec
		memcpy( (void *)&pAcd->audioSpec, (void *)pAudioSpec, sizeof(pAcd->audioSpec) );
		// get the session handle
		result = rtfObjGetSession( (RTF_HANDLE)pAcd, &pAcd->hSes );
		RTF_CHK_RESULT;
		// set the state to open
		pAcd->state = RTF_ACDSTATE_OPEN;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// reset a audio codec object
RTF_RESULT rtfAcdReset( RTF_ACD_HANDLE handle )
{
	RTF_FNAME( "rtfAcdReset" );
	RTF_OBASE( handle );
	RTF_ACD *pAcd = (RTF_ACD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pAcd, RTF_OBJ_TYPE_ACD );
		resetAcd( pAcd );

	} while( 0 ); // error escape wrapper - end

	return result;
}
