// implementation file for rtfIdx class
// encapsulates index file generation method
//

#include "RTFPrv.h"
#include "RTFIdxPrv.h"

// local functions **********************************************************************

// reset the index object state structure
static void rtfResetIdx( RTF_IDX *pIdx )
{
	pIdx->state = RTF_IDX_STATE_CLOSED;
	pIdx->mode = RTF_INDEX_MODE_INVALID;
	pIdx->pInputFilename = (char *)NULL;
	pIdx->indexPointCount = 0;
	pIdx->outPointCount = 0;
	pIdx->splicePointCount = 0;
	pIdx->outCount = 0;
	pIdx->indexFileOutputNumber = -1;
	pIdx->mainFileOutputNumber = -1;
	pIdx->hSes  = (RTF_SES_HANDLE)NULL;
	pIdx->hVcd  = (RTF_VCD_HANDLE)NULL;
	pIdx->hPat  = (RTF_PAT_HANDLE)NULL;
	pIdx->hPmt  = (RTF_PMT_HANDLE)NULL;
	pIdx->hPes  = (RTF_PES_HANDLE)NULL;
	pIdx->hSeq  = (RTF_SEQ_HANDLE)NULL;
	pIdx->hGop  = (RTF_GOP_HANDLE)NULL;
	pIdx->phFlt = (RTF_FLT_HANDLE *)NULL;
	pIdx->slowFileIndex = 0;
	pIdx->ffSpeedCount  = 0;
	pIdx->frSpeedCount  = 0;
	pIdx->indexType     = RTF_INDEX_TYPE_INVALID;
	pIdx->seqBoundOption= RTF_SEQBOUND_OPTION_FIXED;
	pIdx->picCount      = 0;
	memset( (void *)&pIdx->indexOption, 0, sizeof(pIdx->indexOption) );
	memset( (void *)&pIdx->indexInfo,   0, sizeof(pIdx->indexInfo)   );
	memset( (void *)pIdx->ffSpeedInfo,  0, sizeof(pIdx->ffSpeedInfo) );
	memset( (void *)pIdx->frSpeedInfo,  0, sizeof(pIdx->frSpeedInfo) );
	memset( (void *)pIdx->outRatioFix8, 0, sizeof(pIdx->outRatioFix8));
	memset( (void *)pIdx->outPicCount,	0, sizeof(pIdx->outPicCount) );
	memset( (void *)pIdx->pic,          0, sizeof(pIdx->pic)         );
}

// set up fixed-point representations of the number of frames per second
static RTF_RESULT rtfIdxSetFrameRateInfo( RTF_IDX *pIdx )
{
	RTF_FNAME( "rtfIdxSetFrameRateInfo" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// record fixed point representations of the frame rate
		switch( pIdx->pProfile->videoSpec.eStream.video.frameRateCode )
		{
		case 1:			// 23.976 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 23.976 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 23.976 );
			break;
		case 2:			// 24.000 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 24.000 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 24.000 );
			break;
		case 3:			// 25.000 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 25.000 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 25.000 );
			break;
		case 4:			// 29.970 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 29.970 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 29.970 );
			break;
		case 5:			// 30.000 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 30.000 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 30.000 );
			break;
		case 6:			// 50.000 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 50.000 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 50.000 );
			break;
		case 7:			// 59.940 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 59.940 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 59.940 );
			break;
		case 8:			// 60.000 fps
			pIdx->fix8FPS  = (unsigned long)( ( ( 60.000 * 512.0 ) + 1.0 ) / 2.0 );
			pIdx->fix16SPF = (unsigned long)( 65536.0 / 60.000 );
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Invalid frame rate code %d",
						  pIdx->pProfile->videoSpec.eStream.video.frameRateCode );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// sort the trickfiles by speed
static RTF_RESULT rtfSortTrickSpeeds( RTF_IDX *pIdx )
{
	RTF_FNAME( "rtfSortTrickSpeeds" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	RTF_SPEED_INFO tempInfo;
	int direction, ratioFix8;
	unsigned long numerator, denominator;
	int i, j;
	char *pExt;

	do {		 // error escape wrapper - begin

		// reset the speed counts
		pIdx->ffSpeedCount = 0;
		pIdx->frSpeedCount = 0;
		// initialize the speed arrays
		for( i=0; i<(int)(pIdx->outCount); ++i )
		{
			// is this the index file or a main file copy?
			if( ( i == pIdx->indexFileOutputNumber ) ||
				( i == pIdx->mainFileOutputNumber  ) )
			{
				// yes - skip it
				continue;
			}
			// get the speed of this trickfile
			result = rtfFltGetSpeedInfo( pIdx->phFlt[ i ], &direction, &numerator, &denominator, &pExt );
			RTF_CHK_RESULT;
			if( denominator == 0 )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Invalid denominator (0) for trickfile %d", i );
				break;
			}
			ratioFix8 = ( numerator * 256 ) / denominator;
			if( ratioFix8 < 256 )
			{
				RTF_LOG_ERR3( RTF_MSG_ERR_BADPARAM, "Invalid speed (%d/%d) for trickfile %d", numerator, denominator, i );
				break;
			}
			else if( direction >= 0 )
			{
				pIdx->ffSpeedInfo[ pIdx->ffSpeedCount ].outNumber   = i;
				pIdx->ffSpeedInfo[ pIdx->ffSpeedCount ].ratioFix8   = ratioFix8;
				pIdx->ffSpeedInfo[ pIdx->ffSpeedCount ].direction   = direction;
				pIdx->ffSpeedInfo[ pIdx->ffSpeedCount ].numerator   = numerator;
				pIdx->ffSpeedInfo[ pIdx->ffSpeedCount ].denominator = denominator;
				strncpy( pIdx->ffSpeedInfo[ pIdx->ffSpeedCount++ ].ext, pExt, RTF_MAX_EXT_LEN );
			}
			else
			{
				pIdx->frSpeedInfo[ pIdx->frSpeedCount ].outNumber   = i;
				pIdx->frSpeedInfo[ pIdx->frSpeedCount ].ratioFix8   = ratioFix8;
				pIdx->frSpeedInfo[ pIdx->frSpeedCount ].direction   = direction;
				pIdx->frSpeedInfo[ pIdx->frSpeedCount ].numerator   = numerator;
				pIdx->frSpeedInfo[ pIdx->frSpeedCount ].denominator = denominator;
				strncpy( pIdx->frSpeedInfo[ pIdx->frSpeedCount++ ].ext, pExt, RTF_MAX_EXT_LEN );
			}
			pIdx->outRatioFix8[ i ] = ratioFix8;
		} //for( i=0; i<(int)(pIdx->outCount); ++i )
		// perform a bubble sort on the forward speeds
		for( i=0; i<pIdx->ffSpeedCount; ++i )
		{
			for( j=i+1; j<pIdx->ffSpeedCount; ++j )
			{
				if( pIdx->ffSpeedInfo[ j ].ratioFix8 < pIdx->ffSpeedInfo[ i ].ratioFix8 )
				{
					// swap i and j
					memcpy( (void *)&tempInfo, (void *)&pIdx->ffSpeedInfo[ j ].ratioFix8, sizeof(tempInfo) );
					memcpy( (void *)&pIdx->ffSpeedInfo[ j ].ratioFix8, (void *)&pIdx->ffSpeedInfo[ i ].ratioFix8, sizeof(tempInfo) );
					memcpy( (void *)&pIdx->ffSpeedInfo[ i ].ratioFix8, (void *)&tempInfo, sizeof(tempInfo) );
				}
			}
		} // for( i=0; i<pIdx->ffSpeedCount; ++i )
		// perform a bubble sort on the reverse speeds
		for( i=0; i<pIdx->frSpeedCount; ++i )
		{
			for( j=i+1; j<pIdx->frSpeedCount; ++j )
			{
				if( pIdx->frSpeedInfo[ j ].ratioFix8 < pIdx->frSpeedInfo[ i ].ratioFix8 )
				{
					// swap i and j
					memcpy( (void *)&tempInfo, (void *)&pIdx->frSpeedInfo[ j ].ratioFix8, sizeof(tempInfo) );
					memcpy( (void *)&pIdx->frSpeedInfo[ j ].ratioFix8, (void *)&pIdx->frSpeedInfo[ i ].ratioFix8, sizeof(tempInfo) );
					memcpy( (void *)&pIdx->frSpeedInfo[ i ].ratioFix8, (void *)&tempInfo, sizeof(tempInfo) );
				}
			}
		} // for( i=0; i<pIdx->frSpeedCount; ++i )
		// record the index of (one of) the slowest trickfiles
		pIdx->slowFileIndex = pIdx->ffSpeedInfo[ 0 ].outNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfIdxGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfIdxGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_IDX);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfIdxConstructor( RTF_IDX_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfIdxConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_IDX *pIdx;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the IDX object
		pIdx = (RTF_IDX *)rtfAlloc( sizeof(RTF_IDX) );
		RTF_CHK_ALLOC( pIdx );
		// return the handle
		*pHandle = (RTF_IDX_HANDLE)pIdx;
		// clear the state structure
		memset( (void *)pIdx, 0, sizeof(*pIdx) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_IDX, (RTF_HANDLE)pIdx, hParent, &pIdx->hBaseObject );
		RTF_CHK_RESULT;
		// reset the index object
		rtfResetIdx( pIdx );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfIdxDestructor( RTF_IDX_HANDLE handle )
{
	RTF_FNAME( "rtfIdxDestructor" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded resource object (note: performs CHK_OBJ)
		result = rtfObjDestructor( pIdx->hBaseObject, RTF_OBJ_TYPE_IDX );
		RTF_CHK_RESULT;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the state of an index object
RTF_RESULT rtfIdxGetState( RTF_IDX_HANDLE handle, RTF_IDX_STATE *pState )
{
	RTF_FNAME( "rtfIdxGetState" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		// make the return
		*pState = pIdx->state;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the sequence boundary location option
RTF_RESULT rtfIdxGetSeqBoundOption( RTF_IDX_HANDLE handle, RTF_SEQBOUND_OPTION *pSeqBoundOption )
{
	RTF_FNAME( "rtfIdxGetSeqBoundOption" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		// make the return
		*pSeqBoundOption = pIdx->seqBoundOption;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// open an index generator for use with a particular indexing method
RTF_RESULT rtfIdxOpen( RTF_IDX_HANDLE handle, RTF_INDEX_MODE mode,
					   RTF_INDEX_TYPE indexType, RTF_INDEX_OPTION indexOption,
					   char *pInputFilename, INT64 inputBytes, int outCount,
					   int indexFileOutputNumber, int mainFileOutputNumber,
					   RTF_SES_HANDLE hSes, RTF_VCD_HANDLE hVcd,
					   RTF_PAT_HANDLE hPat, RTF_PMT_HANDLE hPmt,
					   RTF_PES_HANDLE hPes, RTF_SEQ_HANDLE hSeq,
					   RTF_OUT_HANDLE *phOut, RTF_FLT_HANDLE *phFlt )
{
	RTF_FNAME( "rtfIdxOpen" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;
	INT64 temp64;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_NE( pIdx, RTF_IDX_STATE_OPEN );
		// get a pointer to the liubrary version string
		rtfGetProgramVersionString( &pIdx->pProgramVersionStr );
		// record the parameters
		pIdx->mode = mode;
		pIdx->pInputFilename = pInputFilename;
		pIdx->indexFileOutputNumber = indexFileOutputNumber;
		pIdx->mainFileOutputNumber = mainFileOutputNumber;
		pIdx->outCount = outCount;
		pIdx->hSes = hSes;
		pIdx->hVcd = hVcd;
		pIdx->hPat = hPat;
		pIdx->hPmt = hPmt;
		pIdx->hPes = hPes;
		pIdx->hSeq = hSeq;
		pIdx->phOut = phOut;
		pIdx->phFlt = phFlt;
		// record the index type
		pIdx->indexType = indexType;
		// record the index option
		pIdx->indexOption = indexOption;
		// get the session's parsing window handle
		result = rtfSesGetWindow( hSes, &pIdx->hWin );
		// get the input stream profile from the session
		result = rtfSesGetStreamProfile( hSes, &pIdx->pProfile );
		RTF_CHK_RESULT;
		// record the output bit rate
		pIdx->bitsPerSecond = ( pIdx->pProfile->userBitsPerSecond == 0 ) ?
								pIdx->pProfile->bitsPerSecond :
								pIdx->pProfile->userBitsPerSecond;
		// adjust the output bitrate upward if TTS headers are being generated
		if( ( pIdx->pProfile->flags & RTF_PROFILE_TTS_MASK ) != 0 )
		{
			temp64 = pIdx->bitsPerSecond;
			temp64 *= TTS_PACKET_BYTES;
			temp64 += ( TRANSPORT_PACKET_BYTES >> 1 );
			temp64 /= TRANSPORT_PACKET_BYTES;
			pIdx->bitsPerSecond = (unsigned long)temp64;
		}
		// sort the trickfiles by speed
		result = rtfSortTrickSpeeds( pIdx );
		RTF_CHK_RESULT;
		// skip the frame rate if there is no video PID
		if( pIdx->pProfile->videoSpec.pid != TRANSPORT_INVALID_PID )
		{
			// set up the fixed-point versions of the frame rate
			result = rtfIdxSetFrameRateInfo( pIdx );
			RTF_CHK_RESULT;
		}
		// perform the method-specific initialization
		switch( indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			result = rtfIdxInitIndexVVX( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			result = rtfIdxInitIndexVV2( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized index type (%d)", indexType );
		}
		RTF_CHK_RESULT_LOOP;
		// set the state to open
		pIdx->state = RTF_IDX_STATE_OPEN;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// reset an index object
RTF_RESULT rtfIdxReset( RTF_IDX_HANDLE handle )
{
	RTF_FNAME( "rtfIdxReset" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		// reset the index object
		rtfResetIdx( pIdx );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process index info at the start of a group of pictures
RTF_RESULT rtfIdxProcessBeforeGroup( RTF_IDX_HANDLE handle, RTF_GOP_HANDLE hGop )
{
	RTF_FNAME( "rtfIdxProcessBeforeGroup" );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_EQ( pIdx, RTF_IDX_STATE_OPEN );
		// record the new group handle
		pIdx->hGop = hGop;
		// perform index method specific processing
		switch( pIdx->indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			result = rtfIdxProcessBeforeGroupVVX( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			result = rtfIdxProcessBeforeGroupVV2( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized index type (%d)", pIdx->indexType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record index info after the end of a group of pictures
RTF_RESULT rtfIdxProcessAfterGroup( RTF_IDX_HANDLE handle )
{
	RTF_FNAME( "rtfIdxProcessAfterGroup" );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_EQ( pIdx, RTF_IDX_STATE_OPEN );
		// perform index method specific processing
		switch( pIdx->indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			result = rtfIdxProcessAfterGroupVVX( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			result = rtfIdxProcessAfterGroupVV2( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized index type (%d)", pIdx->indexType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record the location and size of a keyframe
RTF_RESULT rtfIdxRecordKeyframe( RTF_IDX_HANDLE handle, int filterNumber,
								 unsigned long firstPacket, unsigned long lastPacket )
{
	RTF_FNAME( "rtfIdxRecordKeyframe" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_EQ( pIdx, RTF_IDX_STATE_OPEN );
		// perform index specific action
		switch( pIdx->indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			// nothing to do here?
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			result = rtfIdxRecordKeyframeVV2( pIdx, filterNumber, firstPacket, lastPacket );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized index type (%d)", pIdx->indexType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record the location of a potential "out" point
RTF_RESULT rtfIdxRecordOutpoint( RTF_IDX_HANDLE handle, unsigned long packetNumber )
{
	RTF_FNAME( "rtfIdxRecordOutpoint" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_EQ( pIdx, RTF_IDX_STATE_OPEN );
		// perform index specific action
		switch( pIdx->indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			// nothing to do here?
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			result = rtfIdxRecordOutpointVV2( pIdx, packetNumber );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized index type (%d)", pIdx->indexType );
		}
		RTF_CHK_RESULT_LOOP;
		// bump the out point counter
		++pIdx->outPointCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// prepare a splice point
RTF_RESULT rtfIdxPrepareSplicePoint( RTF_IDX_HANDLE handle, unsigned long packetNumber )
{
	RTF_FNAME( "rtfIdxPrepareSplicePoint" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_EQ( pIdx, RTF_IDX_STATE_OPEN );
		// perform index specific action
		switch( pIdx->indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			result = rtfIdxPrepareSplicePointVVX( pIdx, packetNumber );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			result = rtfIdxPrepareSplicePointVV2( pIdx, packetNumber );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized index type (%d)", pIdx->indexType );
		}
		RTF_CHK_RESULT_LOOP;
		// bump the splice point counter
		++pIdx->splicePointCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// perform a final update of an index file
RTF_RESULT rtfIdxFinalize( RTF_IDX_HANDLE handle, INT64 totalInputByteCount )
{
	RTF_FNAME( "rtfIdxFinalize" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_EQ( pIdx, RTF_IDX_STATE_OPEN );
		// perform index method specific finalization
		switch( pIdx->indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			result = rtfIdxFinalizeVVX( pIdx, totalInputByteCount );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			result = rtfIdxFinalizeVV2( pIdx, totalInputByteCount );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized index type (%d)", pIdx->indexType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// remove all references to trickfiles from the index
RTF_RESULT rtfIdxAbortTrickFiles( RTF_IDX_HANDLE handle )
{
	RTF_FNAME( "rtfIdxAbortTrickFiles" );
	RTF_OBASE( handle );
	RTF_IDX *pIdx = (RTF_IDX *)handle;
	RTF_RESULT result = RTF_PASS;
	INT64 position;
	int i, iResult;
	unsigned char junkBuf[ 16 ];

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pIdx, RTF_OBJ_TYPE_IDX );
		RTF_CHK_STATE_EQ( pIdx, RTF_IDX_STATE_OPEN );
		// reset the state variables dealing with trick files
		pIdx->indexPointCount = 0;
		pIdx->outPointCount = 0;
		pIdx->splicePointCount = 0;
		pIdx->outCount = 0;
		pIdx->ffSpeedCount = 0;
		pIdx->frSpeedCount = 0;
		pIdx->picCount = 0;
		memset( (void *)pIdx->outPicCount, 0, sizeof(pIdx->outPicCount) );
		// get the current position of the index file
		result = rtfOutGetPosition( pIdx->phOut[ pIdx->indexFileOutputNumber ], &position );
		RTF_CHK_RESULT;
		// rewind the index file to the beginning
		iResult = rtfOutWriteIndex( pIdx->phOut[ pIdx->indexFileOutputNumber ], -position, junkBuf, 0 );
		// check for error
		if( iResult < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INDEXER, "Failed to rewind index file" );
			break;
		}
		// reset the type-specific index context
		// then rewrite the index header
		switch( pIdx->indexType )
		{
#ifdef DO_INDEX_VVX
		case RTF_INDEX_TYPE_VVX:
			{
				VVX_INDEX_CONTEXT *pCtxtVVX;
				// create a convenience pointer to the VVX context info
				pCtxtVVX = &pIdx->indexInfo.vvx.context;
				for( i=1; i<10; ++i )
				{
					memset( (void *)&pCtxtVVX->outputFiles[ i ], 0, sizeof(VVX_INDEX_CONTEXT) );
				}
				pCtxtVVX->ffSpeedCount = 0;
				pCtxtVVX->frSpeedCount = 0;
				pCtxtVVX->videoPid = 0;
				pCtxtVVX->trickIndexRecordCount = 0;
				pCtxtVVX->frameDataCount = 0;
				pCtxtVVX->fileCount = 1;
				pCtxtVVX->firstRecordWritten = 0;
				pCtxtVVX->currentIndexFilePos = 0;
			}
			result = rtfIdxInitIndexVVX( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_INDEX_VV2
		case RTF_INDEX_TYPE_VV2:
			{
				VV2_INDEX_CONTEXT *pCtxtVV2;
				// create a convenience pointer to the VV2 context info
				pCtxtVV2 = &pIdx->indexInfo.vv2.context;
				// reset the VV2 context parameters for no trickfiles
				pCtxtVV2->outCount = 1;
				pCtxtVV2->outCount += ( pCtxtVV2->mainFileOutputNumber >= 0 ) ? 1 : 0;
				pCtxtVV2->indexPointCount = 0;
				pCtxtVV2->seqNumber = 0;
				pCtxtVV2->groupOutpointCount = 0;
				pCtxtVV2->idxPicCount = 0;
			}
			result = rtfIdxInitIndexVV2( pIdx );
			RTF_CHK_RESULT;
			break;
#endif
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized index type (%d)", pIdx->indexType );
		}
		RTF_CHK_RESULT_LOOP;

	} while( 0 ); // error escape wrapper - end

	return result;
}

