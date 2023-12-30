// implementation file for rtfIdx class / VV2 subless
//

#include "RTFPrv.h"

#ifdef DO_INDEX_VV2

#include "RTFIdxPrv.h"

// VV2 specific local functions ***************************

static RTF_RESULT rtfInitFilesVV2( RTF_IDX *pIdx )
{
	RTF_FNAME( "rtfInitFilesVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	RTF_INDEX_INFO_VV2 *pInfo;
	VV2_INDEX_CONTEXT *pCtxt;
	VV2_FILE_INFO *pFileInfo;
	int currentIndex;
	int currentFileId;
	int i;

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the index info and context structures
		pInfo = &pIdx->indexInfo.vv2;
		pCtxt = &pInfo->context;
		// record the index file output number
		pCtxt->indexFileOutputNumber = pIdx->indexFileOutputNumber;
		// record the number of outputs
		pCtxt->outCount = pIdx->outCount;
		// create an array that translates session fileid to VV2 sub file number
		// NOTE: session fileid may be any order, but VV2 must be specific order:
		// main file or main file copy = 0
		// then slowest trick file to fastest trick file
		// with ff file first, then fr file (if present)
		// set all unused array entries to -1
		memset( (void *)pInfo->fileIdToSubFileNumber, -1, sizeof(pInfo->fileIdToSubFileNumber) );
		// set up an entry for the main file
		currentFileId = pCtxt->indexFileOutputNumber;
		pInfo->fileIdToSubFileNumber[ currentFileId ] = 0;
		// set up the file info structure for the 1X file
		pFileInfo = &pCtxt->fileInfo[ 0 ];
		pFileInfo->extension[ 0 ] = 0;
		pFileInfo->numerator      = 1;
		pFileInfo->denominator    = 1;
		pFileInfo->direction      = 1;
		pFileInfo->bitrate        = pIdx->bitsPerSecond;
		pFileInfo->fileid         = pIdx->indexFileOutputNumber;
		// is there an active main file copy?
		if( pCtxt->mainFileOutputNumber > 0 )
		{
			// yes - use it instead of the original main file
			pInfo->fileIdToSubFileNumber[ pCtxt->indexFileOutputNumber ] = -1;
			pInfo->fileIdToSubFileNumber[ pCtxt->mainFileOutputNumber  ] = 0;
			pFileInfo->fileid = pCtxt->mainFileOutputNumber;
			// and decrease the output count by one
			--pCtxt->outCount;
		}
		// copy the trick file outputs into the array by increasing trick speed
		// note: the ff and fr arrays have already been sorted by ascending speed.
		currentIndex = 0;
		for( i=0; i<RTF_MAX_TRICKSPEEDS; ++i )
		{
			// is there an ff speed at this level?
			if( ( currentFileId = pIdx->ffSpeedInfo[ i ].outNumber ) >= 0 )
			{
				// yes - but skip invalid entries
				if( pIdx->ffSpeedInfo[ i ].numerator <= 1 )
				{
					continue;
				}
				// fill in this entry in the translation array
				pInfo->fileIdToSubFileNumber[ currentFileId ] = ++currentIndex;
				// and fill in the file info structure for this entry
				pFileInfo = &pCtxt->fileInfo[ currentIndex ];
				strcpy( pFileInfo->extension, pIdx->ffSpeedInfo[ i ].ext );
				pFileInfo->numerator      = pIdx->ffSpeedInfo[ i ].numerator;
				pFileInfo->denominator    = pIdx->ffSpeedInfo[ i ].denominator;
				pFileInfo->direction      = pIdx->ffSpeedInfo[ i ].direction;
				pFileInfo->bitrate        = pIdx->bitsPerSecond;
				pFileInfo->fileid         = currentFileId;
			}
			// is there an fr speed at this level?
			if( ( currentFileId = pIdx->frSpeedInfo[ i ].outNumber ) > 0 )
			{
				// yes - fill in this entry in the translation array
				pInfo->fileIdToSubFileNumber[ currentFileId ] = ++currentIndex;
				// and fill in the file info structure for this entry
				pFileInfo = &pCtxt->fileInfo[ currentIndex ];
				strcpy( pFileInfo->extension, pIdx->frSpeedInfo[ i ].ext );
				pFileInfo->numerator      = pIdx->frSpeedInfo[ i ].numerator;
				pFileInfo->denominator    = pIdx->frSpeedInfo[ i ].denominator;
				pFileInfo->direction      = pIdx->frSpeedInfo[ i ].direction;;
				pFileInfo->bitrate        = pIdx->bitsPerSecond;
				pFileInfo->fileid         = currentFileId;
			}
		} // for( i=0; i<RTF_MAX_TRICKSPEEDS; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfGetTimeVV2( RTF_IDX *pIdx )
{
	RTF_FNAME( "rtfGetTimeVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	RTF_PIC_HANDLE *phPic;
	INT64 delta;
	unsigned long number;
	unsigned char gopPicCount;
	int subfile;

	do {

		// is the current picture the main file picture?
		if( pIdx->picCount == 0 )
		{
			// yes - is this the first index point?
			if( pIdx->indexPointCount == 1 )
			{
				// yes - force the NPT to 0
				pIdx->pic[ 0 ].npt = 0;
			}
			else
			{
				// no - get the handle of the first picture in the current group
				result = rtfGopGetPicArrayInfo( pIdx->hGop, &gopPicCount, &phPic );
				RTF_CHK_RESULT;
				// any pictures in this group?
				if( gopPicCount == 0 )
				{
					rtfGopGetNumber( pIdx->hGop, &number );
					RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "No pictures in group #%d", number );
					break;
				}
				// compute the NPT from the packet number of the
				// first packet of the picture and the bit rate
				// note: NPT relates to base transport stream; ignore augmentation if present
				// note: bitrate does not reflect TTS, if present, so use transport packet size
				result = rtfPicGetFirstPktMapNum( phPic[ 0 ], &number );
				RTF_CHK_RESULT;
				pIdx->pic[ 0 ].npt = (int)( ( ( (INT64)number ) * TRANSPORT_PACKET_BYTES * 8 * 1000 ) / pIdx->pProfile->bitsPerSecond );
			} // if( pIdx->indexPointCount == 0 )
		}
		else // if( pIdx->picCount == 0 )
		{
			// no - the NPTs of pictures in the trick files are interpolated
			// from the NPTs of the corresponding pictures in the main file
			subfile = pIdx->pic[ pIdx->picCount ].subfilenum;
			delta = pIdx->outPicCount[ subfile ];		// trickfile pics
			delta *= pIdx->outRatioFix8[ subfile ];		// trickfile pics * <trick speed> = mainfile pics
			delta >>= 8;								// normalize
			delta *= pIdx->fix16SPF;					// pics * ( secs / pic ) = secs
			delta *= 1000;								// secs * 1000 = msecs
			delta >>= 16;								// normalize
			pIdx->pic[ pIdx->picCount ].npt = pIdx->pic[ 0 ].npt + (unsigned long)delta;
		} // if( pIdx->picCount == 0 ) ; else

	} while( 0 ); // error escape wrapper - end

	return result;
}

// VV2 specific public functions ********************************************************

// initialize a VV2 index object
RTF_RESULT rtfIdxInitIndexVV2( P_RTF_IDX pIdx )
{
	RTF_FNAME( "rtfIdxInitIndexVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	RTF_STREAM_PROFILE *pProfile;
	VV2_INDEX_CONTEXT *pCtxt;
	INT64 inputFileBytes;
	int iResult;

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the index context structure
		pCtxt = &pIdx->indexInfo.vv2.context;
		// clear the VV2 context structure
		memset( (void *)pCtxt, 0, sizeof(*pCtxt) );
		// record the program version string pointer
		pCtxt->pProgramVersionStr = pIdx->pProgramVersionStr;
		// set up the index writing callback routine info
		pCtxt->writeContext = pIdx->phOut[ pIdx->indexFileOutputNumber ];
		pCtxt->writeRtn = rtfOutWriteIndex;
		pCtxt->nextWriteOffset = 0;
		// get the transport flavor from the session object
		result = rtfSesGetStreamProfile( pIdx->hSes, &pProfile );
		RTF_CHK_RESULT;
		pCtxt->transportFlavor = ( ( pProfile->flags & RTF_PROFILE_TTS_MASK ) != 0 ) ? VV2_TRANSPORT_FLAVOR_TTS : VV2_TRANSPORT_FLAVOR_TS;
		// record the output packet size
		pCtxt->outputPacketBytes = ( pCtxt->transportFlavor == VV2_TRANSPORT_FLAVOR_TTS ) ? TTS_PACKET_BYTES : TRANSPORT_PACKET_BYTES;
		// decode the index mode
		switch( pIdx->mode )
		{
		case RTF_INDEX_MODE_OFFLINE:
			pCtxt->mode = VV2_INDEX_MODE_OFFLINE;
			result = rtfSesGetInputFileBytes( pIdx->hSes, &inputFileBytes );
			pCtxt->playDurationMsecs = (int)( ( inputFileBytes * 8 * 1000 ) / pIdx->bitsPerSecond );
			break;
		case RTF_INDEX_MODE_REALTIME:
			pCtxt->mode = VV2_INDEX_MODE_REALTIME;
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized index mode (%d)", pIdx->mode );
		}
		RTF_CHK_RESULT_LOOP;
		// decode the index option
		switch( pIdx->indexOption.vv2 )
		{
		case RTF_INDEX_OPTION_VV2_ON2RTP:
			pCtxt->driverFlavor = VV2_DRIVER_FLAVOR_ON2RTP;
			pCtxt->option = VV2_INDEX_OPTION_ON2RTP;
			pCtxt->recfmt = 1;
			break;
		case RTF_INDEX_OPTION_VV2_TSOIP:
			pCtxt->driverFlavor = VV2_DRIVER_FLAVOR_TSOIP;
			pCtxt->option = VV2_INDEX_OPTION_TSOIP;
			pCtxt->recfmt = 3;
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized index option (%d)",
						  pIdx->indexOption );
		}
		RTF_CHK_RESULT_LOOP;
		// is TTS transport selected?
		if( pCtxt->transportFlavor == VV2_TRANSPORT_FLAVOR_TTS )
		{
			// yes - override the record format selection
			pCtxt->recfmt = 4;
		}
		// set up the driver-specific info
		switch( pCtxt->driverFlavor )
		{
		case VV2_DRIVER_FLAVOR_ON2RTP:
			// placeholder
			pCtxt->driverInfo.rtp.foo = 0;
			break;
		case VV2_DRIVER_FLAVOR_TSOIP:
			// placeholder
			pCtxt->driverInfo.tso.bar = 0;
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized driver flavor (%d)",
						  pCtxt->driverFlavor );
		}
		RTF_CHK_RESULT_LOOP;
		// record the number of output files
		pCtxt->outCount = pIdx->outCount;
		// record the index and main file numbers
		pCtxt->indexFileOutputNumber = pIdx->indexFileOutputNumber;
		pCtxt->mainFileOutputNumber  = pIdx->mainFileOutputNumber;
		// set up the outfile array
		result = rtfInitFilesVV2( pIdx );
		RTF_CHK_RESULT;
		iResult = vv2WriteIndexHeader( &pIdx->indexInfo.vv2.context );
		if( iResult != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vv2WriteIndexHeader returned error (%d)",
						  iResult );
			break;
		}
		iResult = vv2OpenDataSection( &pIdx->indexInfo.vv2.context );
		if( iResult != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vv2OpenDataSection returned error (%d)",
						  iResult );
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process index info before the start of a group of pictures for a VV2 index file
RTF_RESULT rtfIdxProcessBeforeGroupVV2( P_RTF_IDX pIdx )
{
	RTF_FNAME( "rtfIdxProcessBeforeGroupVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VV2_INDEX_CONTEXT *pCtxt;
	RTF_PKT_HANDLE hPkt;
	RTF_IDX_PIC tmpPic;
	unsigned long pktNum;
	unsigned char pktOff;
	int picLimit;
	int iResult;
	int i, j;

	do {		 // error escape wrapper - begin

		// is this the first index point?
		if( pIdx->indexPointCount++ > 0 )
		{
			// no - finish off the preceding index interval
			// get the starting offset of the new sequence in the output stream
			result = rtfSeqGetStartInfo( pIdx->hSeq, &hPkt, &pktOff );
			RTF_CHK_RESULT;
			result = rtfPktGetOutPktNumber( hPkt, &pktNum );
			RTF_CHK_RESULT;
			// make a convenience pointer to the index context structure
			pCtxt = &pIdx->indexInfo.vv2.context;
			// record a final out point for the preceding
			// index point at the start of the new group
			iResult = vv2RecordOutpoint( pCtxt, pktNum );
			if( iResult != 0 )
			{
				RTF_LOG_WARN1( RTF_MSG_WRN_OUTPOINTSKIPPED, "Failed to record outpoint at packet #%d", pktNum );
			}
			// sort the list by ascending NPT
			picLimit = MIN( pIdx->picCount, RTF_IDX_MAX_PICS );
			for( i=0; i<picLimit-1; ++i )
			{
				for( j=i+1; j<picLimit; ++j )
				{
					if( pIdx->pic[ i ].npt > pIdx->pic[ j ].npt )
					{
						tmpPic = pIdx->pic[ i ];
						pIdx->pic[ i ] = pIdx->pic[ j ];
						pIdx->pic[ j ] = tmpPic;
					}
				}
			}
			// copy the indexable picture list into the VV2 index context structure
			for( i=0; i<pIdx->picCount; ++i )
			{
				pCtxt->idxPic[ i ].filenum   = pIdx->pic[ i ].subfilenum;
				pCtxt->idxPic[ i ].pktNumber = pIdx->pic[ i ].firstPktCount;
				pCtxt->idxPic[ i ].pktCount  = pIdx->pic[ i ].lastPktCount - pIdx->pic[ i ].firstPktCount;
				pCtxt->idxPic[ i ].npt		 = pIdx->pic[ i ].npt;
			}
			pCtxt->idxPicCount = pIdx->picCount;
			// write an index record for the index point just completed
			iResult = vv2WriteIndexRecord( pCtxt );
			if( iResult != 0 )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vv2WriteIndexRecord returned error (%d)", iResult );
				break;
			}
			// reset the indexable picture list and the picture count info
			// note that the first picture entry is reserved for the main file
			pIdx->picCount = 0;
			memset( (void *)pIdx->outPicCount, 0, sizeof(pIdx->outPicCount) );
			memset( (void *)pIdx->pic,		   0, sizeof(pIdx->pic)         );
		} // if( pIdx->indexPointCount++ > 0 )
		// start the next index interval
// !!! FIX ME !!! ACTUALLY WANT TO SKIP ORIGINAL MAIN FILE ENTRY, NOT MAIN FILE COPY !!!
		// record the first and last packet numbers for the main file
		result = rtfSeqGetFirstPktNum( pIdx->hSeq, (unsigned long *)&pIdx->pic[ 0 ].firstPktCount );
		RTF_CHK_RESULT;
		result = rtfSeqGetOutputStreamEndPktNum( pIdx->hSeq, (unsigned long *)&pIdx->pic[ 0 ].lastPktCount );
		RTF_CHK_RESULT;
		// record the time at the start of the current picture
		result = rtfGetTimeVV2( pIdx );
		RTF_CHK_RESULT;
		// bump the picture counts for the main file
		// (note: this would normally be done in recordKeyframe, but that only
		// gets called via the filters - so it doesn't happen for the main file
		++pIdx->picCount;
		++pIdx->outPicCount[ 0 ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process index info after the end of a group of pictures for a VV2 index file
RTF_RESULT rtfIdxProcessAfterGroupVV2( P_RTF_IDX pIdx )
{
	RTF_FNAME( "rtfIdxProcessAfterGroupVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// nothing to do here currently

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record the location and size of a keyframe in a VV2 index file
RTF_RESULT rtfIdxRecordKeyframeVV2( P_RTF_IDX pIdx, int filterNumber,
								    unsigned long firstPkt, unsigned long lastPkt )
{
	RTF_FNAME( "rtfIdxRecordKeyframeVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// is there room for another picture on the list?
		if( pIdx->picCount >= RTF_IDX_MAX_PICS )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_OVERFLOW, "Keyframe list overflow at packet %d", firstPkt );
			break;
		}
		// record the keyframe
		pIdx->pic[ pIdx->picCount ].firstPktCount = firstPkt;
		pIdx->pic[ pIdx->picCount ].lastPktCount  = lastPkt;
		pIdx->pic[ pIdx->picCount ].subfilenum    = pIdx->indexInfo.vv2.fileIdToSubFileNumber[ filterNumber ];
		result = rtfGetTimeVV2( pIdx );
		RTF_CHK_RESULT;
		// bump the picture counts
		++pIdx->picCount;
		++pIdx->outPicCount[ filterNumber ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record an out point in a VV2 index file
RTF_RESULT rtfIdxRecordOutpointVV2( P_RTF_IDX pIdx, unsigned long packetNumber )
{
	RTF_FNAME( "rtfIdxRecordOutpointVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VV2_INDEX_CONTEXT *pCtxt;
	int iResult;

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the VV2 context structure
		pCtxt = &pIdx->indexInfo.vv2.context;
		iResult = vv2RecordOutpoint( pCtxt, packetNumber );
		if( iResult != 0 )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_OUTPOINTSKIPPED, "Failed to record outpoint at packet #%d", packetNumber );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// prepare a splice point for a VV2 index file
RTF_RESULT rtfIdxPrepareSplicePointVV2( P_RTF_IDX pIdx, unsigned long packetNumber )
{
	RTF_FNAME( "rtfIdxPrepareSplicePointVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VV2_INDEX_CONTEXT *pCtxt;

	do {		 // error escape wrapper - begin

		// create a convenience pointer to the VV2 context structure
		pCtxt = &pIdx->indexInfo.vv2.context;

		// !!! FIX ME !!! SPLICING !!!

	} while( 0 ); // error escape wrapper - end

	return result;
}

// finalize a VV2 index file
RTF_RESULT rtfIdxFinalizeVV2( P_RTF_IDX pIdx, INT64 totalInputByteCount )
{
	RTF_FNAME( "rtfIdxFinalizeVV2" );
	RTF_OBASE( pIdx );
	RTF_RESULT result = RTF_PASS;
	VV2_INDEX_CONTEXT *pCtxt;
	unsigned long msecs;
	int iResult;

	do {		 // error escape wrapper - begin

		// compute the final input file duration in milliseconds
		msecs = (unsigned long)( ( totalInputByteCount * 8 * 1000 ) / pIdx->bitsPerSecond );
		// create a convenience pointer to the VV2 context structure
		pCtxt = &pIdx->indexInfo.vv2.context;
		iResult = vv2CloseDataSection( pCtxt, msecs );
		if( iResult != 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INDEXER, "vv2CloseDataSection returned error (%d)", iResult );
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

#endif // DO_INDEX_VV2
