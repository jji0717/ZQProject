// implementation file for rtfCod class
// provides CODEC-specific parsing functions
//

#include "RTFPrv.h"

// class definition file
#include "RTFCodPrv.h"

// private functions ********************************************************************

// reset the codec object
static void resetCod( RTF_COD *pCod )
{
	// initial state is closed
	pCod->state = RTF_CODSTATE_CLOSED;
	// handles are null
	pCod->hSes = (RTF_SES_HANDLE)NULL;
	pCod->hWin = (RTF_WIN_HANDLE)NULL;
	pCod->hSeq = (RTF_SEQ_HANDLE)NULL;
	// codec type is invalid
	pCod->codecType = RTF_VIDEO_CODEC_TYPE_INVALID;
	// video PID is invalid
	pCod->vidPid = TRANSPORT_INVALID_PID;
	// bit rate, width, height, and frame rate are all invalid
	pCod->bitRate = 0;
	pCod->width = 0;
	pCod->height = 0;
	pCod->frameRateCode = 0;
	// reset the no-change frame info
	pCod->noChangePFrameValid = FALSE;
	pCod->noChangeBFrameValid = FALSE;
	pCod->noChangePFrameBytes = 0;
	pCod->noChangeBFrameBytes = 0;
	memset( (void *)&pCod->noChangePFrame, 0, sizeof(pCod->noChangePFrame) );
	memset( (void *)&pCod->noChangeBFrame, 0, sizeof(pCod->noChangeBFrame) );
	// clear codec specific info
	memset( (void *)&pCod->codecInfo, 0, sizeof(pCod->codecInfo) );
	RTF_CLR_STAT( pCod->totalKeyframeCount );
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfCodGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfCodGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_COD);
	bytes += rtfObjGetStorageRequirement();
	bytes += rtfWinGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfCodConstructor( RTF_COD_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfCodConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_COD *pCod;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the codec object
		pCod = (RTF_COD *)rtfAlloc( sizeof(RTF_COD) );
		RTF_CHK_ALLOC( pCod );
		// return the handle
		*pHandle = (RTF_COD_HANDLE)pCod;
		// clear the state structure
		memset( (void *)pCod, 0, sizeof(*pCod) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_COD, (RTF_HANDLE)pCod, hParent, &pCod->hBaseObject );
		RTF_CHK_RESULT;
		// create the "exploration" window object
		result = rtfWinConstructor( &pCod->hWinTmp, (RTF_COD_HANDLE)pCod );
		RTF_CHK_RESULT;
		// reset the COD state structure
		resetCod( pCod );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfCodDestructor( RTF_COD_HANDLE handle )
{
	RTF_FNAME( "rtfCodDestructor" );
	RTF_OBASE( handle );
	RTF_COD *pCod = (RTF_COD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the "exploration" window object
		result = rtfWinDestructor( pCod->hWinTmp );
		RTF_CHK_RESULT;
		// destroy the embedded base object
		result = rtfObjDestructor( pCod->hBaseObject, RTF_OBJ_TYPE_COD );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// return a pointer to a set of bytes making up a no change P frame
RTF_RESULT rtfCodGetNCPFrame( RTF_COD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pChanged )
{
	RTF_FNAME( "rtfCodGetNCPFrame" );
	RTF_OBASE( handle );
	RTF_COD *pCod = (RTF_COD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCod, RTF_OBJ_TYPE_COD );
		RTF_CHK_STATE_EQ( pCod, RTF_CODSTATE_OPEN );
		// decode codec type - see if NCPFrame requires regeneration
		switch( pCod->codecType )
		{
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			// pre-generated - never changed except on first call
			*pChanged = ( pCod->codecInfo.mpeg2.getNCPFrameCallCount++ == 0 ) ? TRUE : FALSE;
			break;
		case RTF_VIDEO_CODEC_TYPE_H264:
			// generate on the fly - always changed
			result = rtfCodH264BuildNCFrame( pCod, RTF_CODEC_PPT_H264_IP,
							pCod->noChangePFrame, &pCod->noChangePFrameBytes );
			RTF_CHK_RESULT;
			*pChanged = TRUE;
			break;
		case RTF_VIDEO_CODEC_TYPE_VC1:
			// !!! FIX ME !!! DON'T KNOW IF VC1 CAN PRE-GENERATE B AND P FRAMES !!!
			*pChanged = FALSE;
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized codec type (%d)", pCod->codecType );
			break;
		}
		RTF_CHK_RESULT_LOOP;
		// make the return
		*pFrameBufferBytes = pCod->noChangePFrameBytes;
		*ppFrameBuffer = pCod->noChangePFrame;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return a pointer to a set of bytes making up a no change B frame
RTF_RESULT rtfCodGetNCBFrame( RTF_COD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pChanged )
{
	RTF_FNAME( "rtfCodGetNCBFrame" );
	RTF_OBASE( handle );
	RTF_COD *pCod = (RTF_COD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCod, RTF_OBJ_TYPE_COD );
		RTF_CHK_STATE_EQ( pCod, RTF_CODSTATE_OPEN );
		switch( pCod->codecType )
		{
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			// pre-generated - no change
			*pChanged = FALSE;
			break;
		case RTF_VIDEO_CODEC_TYPE_H264:
			// generated on the fly
			result = rtfCodH264BuildNCFrame( pCod, RTF_CODEC_PPT_H264_IPB,
							pCod->noChangeBFrame, &pCod->noChangeBFrameBytes );
			RTF_CHK_RESULT;
			*pChanged = TRUE;
			break;
		case RTF_VIDEO_CODEC_TYPE_VC1:
			// !!! FIX ME !!! DON'T KNOW IF VC1 CAN PRE-GENERATE B AND P FRAMES !!!
			*pChanged = FALSE;
			break;
		}
		RTF_CHK_RESULT_LOOP;
		// make the return
		*pFrameBufferBytes = pCod->noChangeBFrameBytes;
		*ppFrameBuffer = pCod->noChangeBFrame;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the current frame rate code (MPEG-2 style encoding)
RTF_RESULT rtfCodGetFrameRateCode( RTF_COD_HANDLE handle, unsigned char *pFrameRateCode )
{
	RTF_FNAME( "rtfCodGetFrameRateCode" );
	RTF_OBASE( handle );
	RTF_COD *pCod = (RTF_COD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCod, RTF_OBJ_TYPE_COD );
		RTF_CHK_STATE_EQ( pCod, RTF_CODSTATE_OPEN );
		// make the return
		*pFrameRateCode = pCod->frameRateCode;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service method functions *************************************************************

// set up a codec object for use with a particular codec type
RTF_RESULT rtfCodOpen( RTF_COD_HANDLE handle, RTF_VIDEO_CODEC_TYPE codecType, unsigned short vidPid )
{
	RTF_FNAME( "rtfCodOpen" );
	RTF_OBASE( handle );
	RTF_COD *pCod = (RTF_COD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCod, RTF_OBJ_TYPE_COD );
		RTF_CHK_STATE_NE( pCod, RTF_CODSTATE_OPEN );
		// record the CODEC type and video PID
		pCod->codecType = codecType;
		pCod->vidPid = vidPid;
		// get the session handle
		result = rtfObjGetSession( (RTF_HANDLE)pCod, &pCod->hSes );
		RTF_CHK_RESULT;
		// get the parsing window handle from the session
		result = rtfSesGetWindow( pCod->hSes, &pCod->hWin );
		RTF_CHK_RESULT;
		// reset the codec-specific information (don't fail if codec type not set yet)
		switch( codecType )
		{
#ifdef DO_CODEC_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			pCod->codecInfo.mpeg2.getNCPFrameCallCount = 0;
			break;
#endif
#ifdef DO_CODEC_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			pCod->codecInfo.h264.frNum[ 0 ] = -1;
			pCod->codecInfo.h264.frNum[ 1 ] = -1;
			pCod->codecInfo.h264.ppsID[ 0 ] = -1;
			pCod->codecInfo.h264.ppsID[ 1 ] = -1;
			pCod->codecInfo.h264.isKeyframe[ 0 ] = FALSE;
			pCod->codecInfo.h264.isKeyframe[ 0 ] = FALSE;
			pCod->codecInfo.h264.fieldPicFlag = 0;
			pCod->codecInfo.h264.bottomFieldFlag = 0;
			pCod->codecInfo.h264.picOrderCnt = 0;
			pCod->codecInfo.h264.idrPicID = 0;
			{
				int i;
				for( i=0; i<H264_MAX_SPS_COUNT; ++i )
				{
					pCod->codecInfo.h264.sps[ i ].valid = FALSE;
				}
				for( i=0; i<H264_MAX_PPS_COUNT; ++i )
				{
					pCod->codecInfo.h264.pps[ i ].valid = FALSE;
				}
			}
			break;
#endif
#ifdef DO_CODEC_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			break;
#endif
		case RTF_VIDEO_CODEC_TYPE_INVALID:
			// allow audio-only streams
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized codec type (%d)", codecType );
		}
		RTF_CHK_RESULT_LOOP;
		// set the state to open
		pCod->state = RTF_CODSTATE_OPEN;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set up a no-change frame (used to regulate bit rate at output)
RTF_RESULT rtfCodSetupNCFrame( RTF_COD_HANDLE handle )
{
	RTF_FNAME( "rtfCodSetupNCFrame" );
	RTF_OBASE( handle );
	RTF_COD *pCod = (RTF_COD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCod, RTF_OBJ_TYPE_COD );
		RTF_CHK_STATE_EQ( pCod, RTF_CODSTATE_OPEN );
		// reset the codec-specific information (don't fail if codec type not set yet)
		switch( pCod->codecType )
		{
#ifdef DO_CODEC_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			rtfCodMpeg2BuildNCFrames( pCod );
			break;
#endif
#ifdef DO_CODEC_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			// can't pre-build NC frames for H.264, but can do some other initialization
			rtfCodH264Init( pCod );
			break;
#endif
#ifdef DO_CODEC_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			rtfCodVC1BuildNCFrames( pCod );
			break;
#endif
		case RTF_VIDEO_CODEC_TYPE_INVALID:
			// allow audio-only streams
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized codec type (%d)", pCod->codecType );
		}
		RTF_CHK_RESULT_LOOP;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// reset a codec object
RTF_RESULT rtfCodReset( RTF_COD_HANDLE handle )
{
	RTF_FNAME( "rtfCodOpen" );
	RTF_OBASE( handle );
	RTF_COD *pCod = (RTF_COD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCod, RTF_OBJ_TYPE_COD );
		resetCod( pCod );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a start code from the bit stream
RTF_RESULT rtfCodProcessStartCode( RTF_COD_HANDLE handle, unsigned long code )
{
	RTF_FNAME( "rtfCodProcessStartCode" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_COD *pCod = (RTF_COD *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCod, RTF_OBJ_TYPE_COD );
		RTF_CHK_STATE_EQ( pCod, RTF_CODSTATE_OPEN );
		// what type of video codec is in use?
		switch( pCod->codecType )
		{
#ifdef DO_CODEC_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			result = rtfCodMpeg2ProcessStartCode( pCod, code );
			RTF_CHK_RESULT;
			break;
#endif // DO_CODEC_MPEG2
#ifdef DO_CODEC_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			result = rtfCodH264ProcessStartCode( pCod, code );
			RTF_CHK_RESULT;
			break;
#endif // DO_CODEC_H264
#ifdef DO_CODEC_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			result = rtfCodVC1ProcessStartCode( pCod, code );
			RTF_CHK_RESULT;
			break;
#endif // DO_CODEC_VC1
		default:
			RTF_LOG_ERR0( RTF_MSG_ERR_BADSTREAM, "Unrecognized codec type" );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
