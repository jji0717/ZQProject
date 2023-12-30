// implementation file for rtfIdx class / VVX subless
//

#include "RTFPrv.h"

#ifdef DO_INDEX_VVX

#include "RTFIdxPrv.h"

// VVX specific local functions ***************************

static RTF_RESULT rtfInitFilesVVX( RTF_IDX *pIdx )
{
	RTF_FNAME( "rtfInitFilesVVX" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VVX_INDEX_CONTEXT *pCtxt;
	OUTPUT_FILES *pOutFile;
	int currentIndex, i;
	BYTE *pFrame;

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the index context structure
		pCtxt = &pIdx->indexInfo.vvx.context;
		// is there a video PID?
		if( pIdx->pProfile->videoSpec.pid != TRANSPORT_INVALID_PID )
		{
			// yes - get a copy of the no-change P frame from the codec
			result = rtfVcdGetNCPFrame( pIdx->hVcd, &pFrame, &pCtxt->lenZeroMotionP );
			RTF_CHK_RESULT;
			memcpy( (void *)pCtxt->zeroMotionP, (void *)pFrame, pCtxt->lenZeroMotionP );
			// get a copy of the no-change B frame from the codec
			result = rtfVcdGetNCBFrame( pIdx->hVcd, &pFrame, &pCtxt->lenZeroMotionB );
			RTF_CHK_RESULT;
			memcpy( (void *)pCtxt->zeroMotionB, (void *)pFrame, pCtxt->lenZeroMotionB );
		}
		else
		{
			// no - therefore no zero motion frames
			pCtxt->lenZeroMotionP = 0;
			pCtxt->lenZeroMotionB = 0;
		}
		// copy in the trick speed counts
		pCtxt->ffSpeedCount = pIdx->ffSpeedCount;
		pCtxt->frSpeedCount = pIdx->frSpeedCount;
		// set up the file descriptor for the 1X file
		currentIndex = 0;
		pOutFile = &pCtxt->outputFiles[ 0 ];
		pOutFile->numerator      = 1;
		pOutFile->denominator    = 1;
		pOutFile->index          = currentIndex++;
		pOutFile->extension[ 0 ] = 0;
		// iterate over the trickfiles and set up their file descriptors
		for( i=0; i<RTF_MAX_TRICKSPEEDS; ++i )
		{
			if( i < pIdx->ffSpeedCount )
			{
				pOutFile = &pCtxt->outputFiles[ currentIndex ];
				pOutFile->numerator   = pIdx->ffSpeedInfo[ i ].numerator;
				pOutFile->denominator = pIdx->ffSpeedInfo[ i ].denominator;
				pOutFile->index = currentIndex++;
				strcpy( pOutFile->extension, pIdx->ffSpeedInfo[ i ].ext );
			}
			if( i < pIdx->frSpeedCount )
			{
				pOutFile = &pCtxt->outputFiles[ currentIndex ];
				pOutFile->numerator   = -(pIdx->frSpeedInfo[ i ].numerator);
				pOutFile->denominator = pIdx->frSpeedInfo[ i ].denominator;
				pOutFile->index = currentIndex++;
				strcpy( pOutFile->extension, pIdx->frSpeedInfo[ i ].ext );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record the current time code
static RTF_RESULT rtfSetTimeCodeVVX( RTF_IDX *pIdx )
{
	RTF_FNAME( "rtfSetTimeCodeVVX" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	TIME_CODE *pTimeCode;
	RTF_GOP_HANDLE hGop;
	RTF_PIC_HANDLE *phPic;
	unsigned long picNumber, seconds, ulTemp;
	int gopCount;
	unsigned char picCount;

	do {

		pTimeCode = &pIdx->indexInfo.vvx.timeCode;
		result = rtfSesGetCurrentGopInfo( pIdx->hSes, &hGop, &gopCount );
		RTF_CHK_RESULT;
		if( hGop == (RTF_PIC_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No active Group" );
			break;
		}
		result = rtfGopGetPicArrayInfo( hGop, &picCount, &phPic );
		RTF_CHK_RESULT;
		if( *phPic == (RTF_PIC_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No active Picture" );
			break;
		}
		result = rtfPicGetNumber( phPic[ 0 ], &picNumber );
		ulTemp = seconds = (unsigned long)( ( picNumber<<8 ) / pIdx->fix8FPS );
		pTimeCode->hours = (unsigned char)( ( ulTemp / ( 60 * 60 ) ) & 0xFF );
		ulTemp -= (unsigned long)pTimeCode->hours * ( 60 * 60 );
		pTimeCode->minutes = (unsigned char)( ( ulTemp / 60 ) & 0xFF );
		ulTemp -= (unsigned long)pTimeCode->minutes * 60;
		pTimeCode->seconds = (unsigned char)( ulTemp & 0xFF );
		pTimeCode->frames  = (unsigned char)( ( picNumber - ( ( seconds * pIdx->fix8FPS ) >> 8 ) ) & 0xFF );
		// there may be some inevitable inaccuracy - compensate for it
		while( pTimeCode->frames >= ( pIdx->fix8FPS >> 8 ) )
		{
			++pTimeCode->seconds;
			pTimeCode->frames -= (unsigned char)( ( pIdx->fix8FPS >> 8 ) & 0xFF );
		}
		while( pTimeCode->seconds >= 60 )
		{
			++pTimeCode->minutes;
			pTimeCode->seconds -=60;
		}
		while( pTimeCode->minutes >= 60 )
		{
			++pTimeCode->hours;
			pTimeCode->minutes -= 60;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// VVX index public functions ***********************************************************

// initialize a VVX index
RTF_RESULT rtfIdxInitIndexVVX( P_RTF_IDX pIdx )
{
	RTF_FNAME( "rtfIdxInitIndexVVX" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VVX_INDEX_CONTEXT *pCtxt;
	unsigned short progNum;
	unsigned char *pPat;
	unsigned char *pPmt;
	int presentationDelay;
	int iResult, i, j;
	unsigned long indexBuffer[ RTF_INDEX_BUFFER_BYTES / 4 ];

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the ingest context structure
		pCtxt = &pIdx->indexInfo.vvx.context;
		// clear the ingest context structure
		memset( (void *)pCtxt, 0, sizeof(*pCtxt) );
		// reset the time code and the start, index, size, and time tables
		memset( (void *)&(pIdx->indexInfo.vvx.timeCode), 0, sizeof(pIdx->indexInfo.vvx.timeCode) );
		memset( (void *)pIdx->indexInfo.vvx.startTable,  0, sizeof(pIdx->indexInfo.vvx.startTable) );
		memset( (void *)pIdx->indexInfo.vvx.indexTable,  0, sizeof(pIdx->indexInfo.vvx.indexTable) );
		memset( (void *)pIdx->indexInfo.vvx.sizeTable,   0, sizeof(pIdx->indexInfo.vvx.sizeTable)  );
		// record the minor version number
		pCtxt->minorVersion = ( pIdx->indexOption.vvx == RTF_INDEX_OPTION_VVX_7_2 ) ? V7VVX_CURRENT_MINORVERSION : V7VVX_NEXT_MINORVERSION;
		// set up the outfile array
		result = rtfInitFilesVVX( pIdx );
		RTF_CHK_RESULT;
		// get the input stream profile pointer from the session
		result = rtfSesGetStreamProfile( pIdx->hSes, &pIdx->pProfile );
		RTF_CHK_RESULT;
		// copy some info from the input stream profile into the context structure
		pCtxt->videoBitRate     = (int)pIdx->pProfile->videoSpec.eStream.video.bitsPerSecond;
		pCtxt->frame_rate_code  = (int)pIdx->pProfile->videoSpec.eStream.video.frameRateCode;
		pCtxt->horizontal_size  = (int)pIdx->pProfile->videoSpec.eStream.video.width;
		pCtxt->vertical_size    = (int)pIdx->pProfile->videoSpec.eStream.video.height;
		pCtxt->videoPid         = (int)pIdx->pProfile->videoSpec.pid;
		pCtxt->transportBitRate = (int)pIdx->bitsPerSecond;
		pCtxt->pmtPid           = (int)pIdx->pProfile->pmtPID;
		pCtxt->pcrPid           = (int)pIdx->pProfile->pcrPID;
		// if the video pid is undefined, substitute PID 0 (as per request from Greenville)
		pCtxt->videoPid = ( pCtxt->videoPid == 0xFFFF ) ? 0 : pCtxt->videoPid;
		// get the program number
		result = rtfPatGetProgramNumber( pIdx->hPat, &progNum );
		RTF_CHK_RESULT;
		pCtxt->program_number = (int)progNum;
		// is a video PID defined?
		if( pIdx->pProfile->videoSpec.pid != TRANSPORT_INVALID_PID )
		{
			// yes - get the decoding delay and presentation delay from the PES header
			// NOTE: the label "dtsPtsTimeOffset" is deceiving - it is really DTS - PCR
			result = rtfPesGetDelays( pIdx->hPes, &pCtxt->dtsPtsTimeOffset, &presentationDelay );
			RTF_CHK_RESULT;
		}
		else
		{
			// no - offsets are zero
			pCtxt->dtsPtsTimeOffset = 0;
			presentationDelay = 0;
		}
		pCtxt->zeroMotionFrameType = RTF_ZERO_MOTION_FRAME_TYPE;
		strncpy( pCtxt->filename, pIdx->pInputFilename, MAX_PATHLEN );
		result = rtfPatGetTable( pIdx->hPat, &pPat );
		RTF_CHK_RESULT;
		memcpy( (void *)pCtxt->pat, (void *)pPat, TRANSPORT_PACKET_BYTES );
		result = rtfPmtGetPacket( pIdx->hPmt, &pPmt );
		RTF_CHK_RESULT;
		memcpy( (void *)pCtxt->pmt, (void *)pPmt, TRANSPORT_PACKET_BYTES );
		pCtxt->writeContext = pIdx->phOut[ pIdx->indexFileOutputNumber ];
		pCtxt->writeRtn = rtfOutWriteIndex;
		// create a table to translate output number to index file record order
		pIdx->indexInfo.vvx.outputNumberToRecordOrder[ pIdx->indexFileOutputNumber ] = 0;
		for( i=0, j=0; i<MIN(pIdx->ffSpeedCount, pIdx->frSpeedCount); ++i )
		{
			// set the entry for the next forward speed
			pIdx->indexInfo.vvx.outputNumberToRecordOrder[ pIdx->ffSpeedInfo[ i ].outNumber ] = ++j;
			// set the entry for the next reverse speed
			pIdx->indexInfo.vvx.outputNumberToRecordOrder[ pIdx->frSpeedInfo[ i ].outNumber ] = ++j;
		}
		for( ; i<pIdx->ffSpeedCount; ++i )
		{
			pIdx->indexInfo.vvx.outputNumberToRecordOrder[ pIdx->ffSpeedInfo[ i ].outNumber ] = ++j;
		}
		for( ; i<pIdx->frSpeedCount; ++i )
		{
			pIdx->indexInfo.vvx.outputNumberToRecordOrder[ pIdx->frSpeedInfo[ i ].outNumber ] = ++j;
		}
		// set up the initial VVX index header structure
		if( ( iResult = vvxSetupInitialIndexHeader( pCtxt ) ) != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vvxSetupInitialIndexHeader returned error (%d)", iResult );
			break;
		}
#ifdef DO_DEBUGIO
		memset( (void *)indexBuffer, 0, sizeof(indexBuffer) );
#endif
		// write it out
		if( ( iResult = vvxWriteIndexHeader( pCtxt, (unsigned char *)indexBuffer, sizeof(indexBuffer), pIdx->pProgramVersionStr ) ) != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vvxWriteIndexHeader returned error (%d)", iResult );
			break;
		}
#ifdef DO_DEBUGIO
		memset( (void *)indexBuffer, 0, sizeof(indexBuffer) );
#endif
		// write the initial first index record (will be overwritten later)
		if( ( iResult = vvxWriteFirstRecord( pCtxt, (unsigned char *)indexBuffer, sizeof(indexBuffer) ) ) != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vvxWriteFirstRecord returned error (%d)", iResult );
			break;
		}
		// bump the index point count
		++pIdx->indexPointCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process index info before the start of a group of pictures for a VVX index file
RTF_RESULT rtfIdxProcessBeforeGroupVVX( P_RTF_IDX pIdx )
{
	RTF_FNAME( "rtfIdxProcessBeforeGroupVVX" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// compute the current time code, VVX style
		result = rtfSetTimeCodeVVX( pIdx );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process index info after the end of a group of pictures for a VVX index file
RTF_RESULT rtfIdxProcessAfterGroupVVX( P_RTF_IDX pIdx )
{
	RTF_FNAME( "rtfIdxProcessAfterGroupVVX" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VVX_INDEX_CONTEXT *pCtxt;
	INT64 *pStartTable;
	INT64 *pIndexTable;
	INT64 *pSizeTable;
	INT64 delta;
	BOOL writeFlag;
	int i, j, iResult;
	unsigned long pktCount;
	unsigned long indexBuffer[ RTF_INDEX_BUFFER_BYTES / 4 ];

	do {		 // error escape wrapper - begin

		// get the output byte count from each output filter
		// (sample the source file in place of the index file)
		// record these as the new file size entries
		// also, see how much each trickfile advanced
		// if the number of bytes a trickfile advances is less
		// than a threshold (i.e. it is an NCFrame), don't let
		// the index point advance.
		pCtxt = &pIdx->indexInfo.vvx.context;
		pStartTable = pIdx->indexInfo.vvx.startTable;
		pIndexTable = pIdx->indexInfo.vvx.indexTable;
		pSizeTable  = pIdx->indexInfo.vvx.sizeTable;
		writeFlag   = TRUE;
		// get the output byte count from each output filter
		// (sample the source file in place of the index file)
		// record these offsets as potential index points
		for( i=0; i<pIdx->outCount; ++i )
		{
			// get the record order number for this output
			j = pIdx->indexInfo.vvx.outputNumberToRecordOrder[ i ];
			// is this the index file output?
			if( i == pIdx->indexFileOutputNumber )
			{
				// yes - set up the source file's info in place of the index file
				// get the input stream offset of the first packet of the sequence
				result = rtfSeqGetOutputStreamStartOffset( pIdx->hSeq, &pStartTable[ j ] );
				RTF_CHK_RESULT;
				// and the last packet of the sequence
				result = rtfSeqGetOutputStreamEndPktNum( pIdx->hSeq, &pktCount );
				RTF_CHK_RESULT;
				// count (unit indexed) is one more than number (zero indexed)
				++pktCount;
			}
			else
			{
				// no - get the output stream offset at the start of the keyframe
				result = rtfFltGetKeyStartPktCount( pIdx->phFlt[ i ], &pktCount );
				RTF_CHK_RESULT;
				pStartTable[ j ] = (INT64)pktCount * TRANSPORT_PACKET_BYTES;
				// get the output packet count at the end of the group
				result = rtfFltGetOutPktCount( pIdx->phFlt[ i ], &pktCount );
				RTF_CHK_RESULT;
			}
			// compute the new size table entry
			pSizeTable[ j ] = (INT64)pktCount * TRANSPORT_PACKET_BYTES;
			// calculate the amount that this file advanced
			delta = pSizeTable[ j ] - pStartTable[ j ];
			// did it exceed the threshold?
			// (i.e. is it a real keyframe, not an NC frame?)
			// NOTE: this is guaranteed to be true for the source file
			if( delta > RTF_IDX_ADVANCE_BYTES )
			{
				// yes - allow the index to advance for this file
				pIndexTable[ j ] = pStartTable[ j ];
			}
			else
			{
				// no - is this the slowest trickfile?
				if( i == pIdx->slowFileIndex )
				{
					// yes - are we indexing only the keyframes that make it into a trickfile?
					if( pIdx->indexOption.vvx == RTF_INDEX_OPTION_VVX_7_2 )
					{
						// yes - don't record an index point in this case
						writeFlag = FALSE;
					}
				}
			}
		} // for( i=0; i<pIdx->outCount; ++i )
		RTF_CHK_RESULT_LOOP;
		// are we writing an index record here?
		if( writeFlag != FALSE )
		{
#ifdef DO_DEBUGIO
		memset( (void *)indexBuffer, 0, sizeof(indexBuffer) );
#endif
			// create an index entry for this keyframe
			iResult = vvxWriteIndexRecord( pCtxt, pIdx->indexInfo.vvx.timeCode, pIndexTable, pSizeTable,
										   (unsigned char *)indexBuffer, sizeof(indexBuffer) );
			if( iResult != 0 )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vvxWriteIndexRecord returned error (%d)", iResult );
				break;
			}
			++pIdx->indexPointCount;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// prepare a splice point for a VVX index file
RTF_RESULT rtfIdxPrepareSplicePointVVX( P_RTF_IDX pIdx, unsigned long packetNumber )
{
	RTF_FNAME( "rtfIdxPrepareSplicePointVVX" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VVX_INDEX_CONTEXT *pCtxt;

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the VVX context structure
		pCtxt = &pIdx->indexInfo.vvx.context;

		// !!! FIX ME !!! SPLICING !!!

	} while( 0 ); // error escape wrapper - end

	return result;
}

// perform a final update of a VVX index file
RTF_RESULT rtfIdxFinalizeVVX( P_RTF_IDX pIdx, INT64 totalInputByteCount )
{
	RTF_FNAME( "rtfIdxFinalizeVVX" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VVX_INDEX_CONTEXT *pCtxt;
	INT64 *pSizeTable;
	int i, j, iResult;
	unsigned long outPktCount;
	unsigned long indexBuffer[ RTF_INDEX_BUFFER_BYTES / 4 ];

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the VVX context structure
		pCtxt = &pIdx->indexInfo.vvx.context;
		// update the input file entry in the size table
		pSizeTable = pIdx->indexInfo.vvx.sizeTable;
		pSizeTable[ pIdx->indexFileOutputNumber ] = totalInputByteCount;
#ifdef DO_DEBUGIO
		memset( (void *)indexBuffer, 0, sizeof(indexBuffer) );
#endif
		// rewind to the final terminator record and re-write it with the updated file sizes
		if( ( iResult = vvxWriteFinalTerminator( pCtxt, pSizeTable, (unsigned char *)indexBuffer,
												 sizeof(indexBuffer) ) ) != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vvxWriteFinalTerminator returned error (%d)", iResult );
			break;
		}
		// update the index header file size entry for each output file
		for( i=0; i<pIdx->outCount; ++i )
		{
			// is this an entry for a copy of the main file?
			if( i == pIdx->mainFileOutputNumber )
			{
				// yes - skip it.
				continue;
			}
			// is this the index file's entry?
			if( i == pIdx->indexFileOutputNumber )
			{
				// yes - describe the 1X file in place of the index file
				pCtxt->outputFiles[ i ].fileSize = totalInputByteCount;
			}
			else
			{
				// no - describe the trickfile
				j = pIdx->indexInfo.vvx.outputNumberToRecordOrder[ i ];
				result = rtfFltGetOutPktCount( pIdx->phFlt[ i ], &outPktCount );
				RTF_CHK_RESULT;
				pCtxt->outputFiles[ j ].fileSize = (INT64)outPktCount * TRANSPORT_PACKET_BYTES;
			}
		}
		RTF_CHK_RESULT_LOOP;
#ifdef DO_DEBUGIO
		memset( (void *)indexBuffer, 0, sizeof(indexBuffer) );
#endif
		// re-write the index header
		if( ( iResult = vvxWriteIndexHeader( pCtxt, (unsigned char *)indexBuffer, sizeof(indexBuffer), pIdx->pProgramVersionStr ) ) != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vvxWriteIndexHeader returned error (%d)", iResult );
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

#endif // DO_INDEX _VVX
