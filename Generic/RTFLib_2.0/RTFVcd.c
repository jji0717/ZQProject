// implementation file for rtfVcd class
// provides VCD-specific parsing functions
//

#include "RTFPrv.h"

// class definition file
#include "RTFVcdPrv.h"

// private functions ********************************************************************

// reset the video codec object
static void resetVcd( RTF_VCD *pVcd )
{
	unsigned char *pStorage;

	// initial state is closed
	pVcd->state = RTF_VCDSTATE_CLOSED;
	// handles are null
	pVcd->hSes = (RTF_SES_HANDLE)NULL;
	pVcd->hWin = (RTF_WIN_HANDLE)NULL;
	pVcd->hSeq = (RTF_SEQ_HANDLE)NULL;
	// reset the video spec pointer
	pVcd->pVidSpec = (RTF_ESTREAM_SPEC *)NULL;
	// reset the sequence start info; note that constructor sets hSeqStartPkt to NULL before
    // calling this routine to avoid decrementing an hSeqStartPkt's refcount before it's initialized
    if (pVcd->hSeqStartPkt != NULL)
    {
		rtfPktGetStorage( pVcd->hSeqStartPkt, &pStorage );
		if( pStorage != (unsigned char *)NULL )
		{
			rtfPktRemoveReference( pVcd->hSeqStartPkt, (RTF_VCD_HANDLE)pVcd );
		}
        pVcd->hSeqStartPkt = (RTF_PKT_HANDLE)NULL;
    }
	pVcd->seqStartPktOff = 0;
	// reset the picture end info
	pVcd->hPicEndPkt = (RTF_PKT_HANDLE)NULL;
	pVcd->picEndPktOff = 0;
	// reset the no-change frame info
	pVcd->insertDSM           = FALSE;
	pVcd->noChangePFrameValid = FALSE;
	pVcd->noChangeBFrameValid = FALSE;
	pVcd->noChangePFrameBytes = 0;
	pVcd->noChangeBFrameBytes = 0;
	memset( (void *)&pVcd->noChangePFrame, 0, sizeof(pVcd->noChangePFrame) );
	memset( (void *)&pVcd->noChangeBFrame, 0, sizeof(pVcd->noChangeBFrame) );
	// clear video codec specific info
	memset( (void *)&pVcd->vcdInfo, 0, sizeof(pVcd->vcdInfo) );
	RTF_CLR_STAT( pVcd->totalKeyframeCount );
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfVcdGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfVcdGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_VCD);
	bytes += rtfObjGetStorageRequirement();
	bytes += rtfWinGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfVcdConstructor( RTF_VCD_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfVcdConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD *pVcd;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the video codec object
		pVcd = (RTF_VCD *)rtfAlloc( sizeof(RTF_VCD) );
		RTF_CHK_ALLOC( pVcd );
		// return the handle
		*pHandle = (RTF_VCD_HANDLE)pVcd;
		// clear the state structure
		memset( (void *)pVcd, 0, sizeof(*pVcd) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_VCD, (RTF_HANDLE)pVcd, hParent, &pVcd->hBaseObject );
		RTF_CHK_RESULT;
		// create the "exploration" window object
		result = rtfWinConstructor( &pVcd->hWinTmp, (RTF_VCD_HANDLE)pVcd );
		RTF_CHK_RESULT;
		// reset the VCD state structure
		resetVcd( pVcd );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfVcdDestructor( RTF_VCD_HANDLE handle )
{
	RTF_FNAME( "rtfVcdDestructor" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the "exploration" window object
		result = rtfWinDestructor( pVcd->hWinTmp );
		RTF_CHK_RESULT;
		// destroy the embedded base object
		result = rtfObjDestructor( pVcd->hBaseObject, RTF_OBJ_TYPE_VCD );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// return the sequence start point info
RTF_RESULT rtfVcdGetSeqStartInfo( RTF_VCD_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								  unsigned char *pPktOff )
{
	RTF_FNAME( "rtfVcdGetSeqStartInfo" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_EQ( pVcd, RTF_VCDSTATE_OPEN );
		// make the returns
		*phPkt = pVcd->hSeqStartPkt;
		*pPktOff = pVcd->seqStartPktOff;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the picture end point info
RTF_RESULT rtfVcdGetPicEndInfo( RTF_VCD_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								unsigned char *pPktOff )
{
	RTF_FNAME( "rtfVcdGetPicEndInfo" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_EQ( pVcd, RTF_VCDSTATE_OPEN );
		// make the returns
		*phPkt = pVcd->hPicEndPkt;
		*pPktOff = pVcd->picEndPktOff;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return a pointer to a set of bytes making up a no change P frame
RTF_RESULT rtfVcdGetNCPFrame( RTF_VCD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pRegenerateNCPF )
{
	RTF_FNAME( "rtfVcdGetNCPFrame" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_EQ( pVcd, RTF_VCDSTATE_OPEN );
		// decode video codec type - see if NCPFrame requires regeneration
		switch( pVcd->pVidSpec->eStream.video.vcdType )
		{
#ifdef DO_VCD_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			// pre-generated - nothing to do here
			*pRegenerateNCPF = FALSE;
			break;
#endif
#ifdef DO_VCD_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			// generate on the fly - always changed
			*pRegenerateNCPF = TRUE;
			result = rtfVcdH264BuildNCFrame( pVcd, RTF_VCD_PPT_H264_IP, pVcd->noChangePFrame,
											 &pVcd->noChangePFrameBytes );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_VCD_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			// !!! FIX ME !!! DON'T KNOW IF VC1 CAN PRE-GENERATE B AND P FRAMES !!!
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized video codec type (%d)",
						  pVcd->pVidSpec->eStream.video.vcdType );
		}
		RTF_CHK_RESULT_LOOP;
		// make the return
		*pFrameBufferBytes = pVcd->noChangePFrameBytes;
		*ppFrameBuffer = pVcd->noChangePFrame;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return a pointer to a set of bytes making up a no change B frame
RTF_RESULT rtfVcdGetNCBFrame( RTF_VCD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pRegenerateNCBF )
{
	RTF_FNAME( "rtfVcdGetNCBFrame" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_EQ( pVcd, RTF_VCDSTATE_OPEN );
		switch( pVcd->pVidSpec->eStream.video.vcdType )
		{
#ifdef DO_VCD_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			// pre-generated - nothing to do here
			*pRegenerateNCBF = FALSE;
			break;
#endif
#ifdef DO_VCD_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			// generated on the fly - always changed
			*pRegenerateNCBF = TRUE;
			result = rtfVcdH264BuildNCFrame( pVcd, RTF_VCD_PPT_H264_IPB,
							pVcd->noChangeBFrame, &pVcd->noChangeBFrameBytes );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_VCD_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			// !!! FIX ME !!! DON'T KNOW IF VC1 CAN PRE-GENERATE B AND P FRAMES !!!
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized video codec type (%d)",
						  pVcd->pVidSpec->eStream.video.vcdType );
		}
		RTF_CHK_RESULT_LOOP;
		// make the return
		*pFrameBufferBytes = pVcd->noChangeBFrameBytes;
		*ppFrameBuffer = pVcd->noChangeBFrame;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service method functions *************************************************************

// set up a video codec object for use with a particular video stream
RTF_RESULT rtfVcdOpen( RTF_VCD_HANDLE handle, RTF_ESTREAM_SPEC *pVidSpec )
{
	RTF_FNAME( "rtfVcdOpen" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_NE( pVcd, RTF_VCDSTATE_OPEN );
		// copy the video specification pointer
		pVcd->pVidSpec = pVidSpec;
		// get the session handle
		result = rtfObjGetSession( (RTF_HANDLE)pVcd, &pVcd->hSes );
		RTF_CHK_RESULT;
		// get the parsing window handle from the session
		result = rtfSesGetWindow( pVcd->hSes, &pVcd->hWin );
		RTF_CHK_RESULT;
		// get the insert DSM trick mode field fixup flag from the session
		result = rtfSesGetInsertDSM( pVcd->hSes, &pVcd->insertDSM );
		RTF_CHK_RESULT;
		// get the sequence boundary option from the session
		result = rtfSesGetSeqBoundOption( pVcd->hSes, &pVcd->seqBoundOption );
		RTF_CHK_RESULT;
		// reset the video-codec-specific information (don't fail if type not set yet)
		switch( pVcd->pVidSpec->eStream.video.vcdType )
		{
#ifdef DO_VCD_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			pVcd->vcdInfo.mpeg2.getNCPFrameCallCount = 0;
			break;
#endif
#ifdef DO_VCD_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			pVcd->vcdInfo.h264.frNum[ 0 ] = -1;
			pVcd->vcdInfo.h264.frNum[ 1 ] = -1;
			pVcd->vcdInfo.h264.ppsID[ 0 ] = -1;
			pVcd->vcdInfo.h264.ppsID[ 1 ] = -1;
			pVcd->vcdInfo.h264.isKeyframe[ 0 ] = FALSE;
			pVcd->vcdInfo.h264.isKeyframe[ 0 ] = FALSE;
			pVcd->vcdInfo.h264.fieldPicFlag = 0;
			pVcd->vcdInfo.h264.bottomFieldFlag = 0;
			pVcd->vcdInfo.h264.picOrderCnt = 0;
			pVcd->vcdInfo.h264.idrPicID = 0;
			{
				int i;
				for( i=0; i<H264_MAX_SPS_COUNT; ++i )
				{
					pVcd->vcdInfo.h264.sps[ i ].valid = FALSE;
				}
				for( i=0; i<H264_MAX_PPS_COUNT; ++i )
				{
					pVcd->vcdInfo.h264.pps[ i ].valid = FALSE;
				}
			}
			break;
#endif
#ifdef DO_VCD_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			break;
#endif
		case RTF_VIDEO_CODEC_TYPE_INVALID:
			// allow audio-only streams
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized video codec type (%d)",
						  pVcd->pVidSpec->eStream.video.vcdType );
		}
		RTF_CHK_RESULT_LOOP;
		// set the state to open
		pVcd->state = RTF_VCDSTATE_OPEN;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set up a no-change frame (used to regulate bit rate at output)
RTF_RESULT rtfVcdSetupNCFrame( RTF_VCD_HANDLE handle )
{
	RTF_FNAME( "rtfVcdSetupNCFrame" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_EQ( pVcd, RTF_VCDSTATE_OPEN );
		// reset the video-codec-specific information (don't fail if type not set yet)
		switch( pVcd->pVidSpec->eStream.video.vcdType )
		{
#ifdef DO_VCD_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			rtfVcdMpeg2BuildNCFrames( pVcd );
			break;
#endif
#ifdef DO_VCD_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			// can't pre-build NC frames for H.264, but can do some other initialization
			rtfVcdH264Init( pVcd );
			break;
#endif
#ifdef DO_VCD_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			rtfVcdVC1BuildNCFrames( pVcd );
			break;
#endif
		case RTF_VIDEO_CODEC_TYPE_INVALID:
			// allow audio-only streams
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized video codec type (%d)",
						  pVcd->pVidSpec->eStream.video.vcdType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// reset a video codec object
RTF_RESULT rtfVcdReset( RTF_VCD_HANDLE handle )
{
	RTF_FNAME( "rtfVcdReset" );
	RTF_OBASE( handle );
	RTF_VCD *pVcd = (RTF_VCD *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		resetVcd( pVcd );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// update the boundaryt location information
RTF_RESULT rtfVcdUpdateBoundaryInfo( RTF_VCD_HANDLE handle )
{
	RTF_FNAME( "rtfVcdUpdateBoundaryInfo" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD *pVcd = (RTF_VCD *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_EQ( pVcd, RTF_VCDSTATE_OPEN );
		// record this location as the sequence boundary
		result = rtfWinGetFirstByteInfo( pVcd->hWin, &pVcd->hSeqStartPkt, &pVcd->seqStartPktOff );
		RTF_CHK_RESULT;
		result = rtfWinGetPriorByteInfo( pVcd->hWin, &pVcd->hPicEndPkt, &pVcd->picEndPktOff );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a start code from the bit stream
RTF_RESULT rtfVcdProcessStartCode( RTF_VCD_HANDLE handle, unsigned long code )
{
	RTF_FNAME( "rtfVcdProcessStartCode" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD *pVcd = (RTF_VCD *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pVcd, RTF_OBJ_TYPE_VCD );
		RTF_CHK_STATE_EQ( pVcd, RTF_VCDSTATE_OPEN );
		// what type of video codec is in use?
		switch( pVcd->pVidSpec->eStream.video.vcdType )
		{
#ifdef DO_VCD_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			result = rtfVcdMpeg2ProcessStartCode( pVcd, code );
			RTF_CHK_RESULT;
			break;
#endif // DO_VCD_MPEG2
#ifdef DO_VCD_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			result = rtfVcdH264ProcessStartCode( pVcd, code );
			RTF_CHK_RESULT;
			break;
#endif // DO_VCD_H264
#ifdef DO_VCD_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			result = rtfVcdVC1ProcessStartCode( pVcd, code );
			RTF_CHK_RESULT;
			break;
#endif // DO_VCD_VC1
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unrecognized video codec type %d",
						  pVcd->pVidSpec->eStream.video.vcdType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
