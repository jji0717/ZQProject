// implementation file for rtfFlt class
// abstracts video sequence to trickfile filter
//

#include "RTFPrv.h"

// compilation switches *****************************************************************

// each filter writes a file indicating which pictures are included in the output file
#define GENERATE_TRACE_FILES		0

// constants ****************************************************************************

#define RTF_DEFAULT_QUEUE_FRAMES_LOG2	5
#define RTF_ABSMAX_QUEUE_FRAMES_LOG2	10
#define RTF_ABSMAX_QUEUE_FRAMES			( 1<<RTF_ABSMAX_QUEUE_FRAMES_LOG2 )
#define RTF_DSM_FF_BYTE					0x03	// mode=000, fieldID=00 intraSliceRefresh=0 freqTruc=11 
#define RTF_BIG_PCR_BASE				0x80000000
#define RTF_BIG_PCR_EXT					( TRANSPORT_SCR_TO_PCR_RATIO - 1 )
#define RTF_NCF_FIRST_PACKET_BYTES		182		// kludge to get around splitting start code
#define RTF_MAX_FIXUP_ADDED_PKTS		16
#define RTF_MAX_TARGET_PICS				5

// typedefs *****************************************************************************

typedef enum _RTF_FLT_STATE
{
	RTF_FLT_STATE_INVALID = 0,

	RTF_FLT_STATE_CLOSED,
	RTF_FLT_STATE_OPEN,
	RTF_FLT_STATE_READY

} RTF_FLT_STATE;

typedef enum _RTF_FLT_FIXUP
{
	RTF_FLT_FIXUP_SEQHDR = 0,
	RTF_FLT_FIXUP_SQXHDR,
	RTF_FLT_FIXUP_GOPHDR,
	RTF_FLT_FIXUP_PICHDR,
	RTF_FLT_FIXUP_CODHDR,
	RTF_FLT_FIXUP_PESHDR

} RTF_FLT_FIXUP;

typedef struct _RTF_FLT
{
	RTF_OBJ_HANDLE hBaseObject;
	RTF_FLT_STATE  state;				// state of the filter
	RTF_TRICK_SPEC trickSpec;			// trickfile fixup specification
	RTF_STREAM_PROFILE *pProfile;		// pointer to input stream profile

	// speed control
	unsigned long tgtInFrameNumber;		// "ideal" next input frame to select
	unsigned long speedRatioFix16;		// speedNum / speedDenom (16-bit fixed point)
	unsigned long framesPerSecFix16;	// input frames/sec (16 bit fixed point)
	unsigned long secsPerFrameFix16;	// input sec/frames (16-bit fixed point)
	unsigned long invSpeedRatioFix16;	// speedDenom/speedNum (16 bit fixed point)
	int speedDirection;					// +1 for forward trick files, -1 for reverse, 0 for bi-directional

	// bit rate control queue - average over last N frames
	int maxQueueFramesLog2;
	int maxQueueFrames;
	int maxQueueFramesMask;
	unsigned long queueFrameBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueStuffBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueInputBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueKeyfrBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueNextFrameBits;   // number of bits in next frame to be added to queue
	unsigned long queueNextStuffBits;   // number of stuffing bits in next frame to be added to queue
	unsigned long queueNextInputBits;	// number of input bits spanned by next frame to be added to queue
	unsigned long queueNextKeyfrBits;	// number of bits in next keyframe to be added to queue
	unsigned long queueTotalOutputBits;	// sum of all entries in both queues above
	unsigned long queueTotalInputBits;	// number of input bits spanned by output frames in queue
	unsigned long queueTotalKeyfrBits;	// number of bits in all current keyframe entries
	unsigned long queueLastKeyframeBits;// number of bits in last keyframe
	unsigned long queueLastNCFrameBits;	// number of bits in last no-change frame
	unsigned long queueLastGroupPktNum; // number of last packet of last group examined

	// info on last packet image sent to output buffer
	unsigned long  outPktFlags;
	unsigned char *pOutPktStorage;
	unsigned long  outPktNumber;
	unsigned char  outPktPayloadOffset;
	unsigned char  outPktPayloadBytes;
	unsigned char  outPktPesHdrOffset;
	unsigned char  outPktPesHdrBytes;
	unsigned char  outPktFixupFlags;
	unsigned char  outPktFixupOffset[ 8 ];

	// timestamp generation info
	int frameTicks;						// number of PCR ticks for one frame
	int decodingDelay;					// ( DTS - PCR ) from PES header
	int presentationDelay;				// ( PTS - DTS ) from PES header
	int ptsCount;						// number of PTS timestamps processed
	UINT64 lastPTS;						// value of last Presentation Time Stamp (in 90KHz clock ticks)
	UINT64 lastDTS;						// value of last Decode Time Stamp (in 90KHz clock ticks)

	// misc state info
	int filterNumber;					// the number of this filter
	unsigned char pesHdrLen;			// length of PES header in buffer below
	unsigned char firstBytePktOffset;	// offset of first byte of current scanning window
	unsigned char lastBytePktOffset;	// offset of last byte of current scanning window
	unsigned short lastVideoPacketIndex;// index of last video packet in packet array
	unsigned long targetBitRate;		// output bit rate target
	unsigned long forcePadBitRate;		// portion of output bit rate that must be nulls
	unsigned long accessUnitOutPackets;	// number of output packets in the current access unit
	unsigned long ncAccessUnitOutPackets;// number of output packets in the current NC access unit
	unsigned long packetsPerPcr;		// max number of packets between PCRs (if PCR insertion enabled)
	unsigned long lastPcrPktOutCount;	// value of pktOutCount when last PCR was output
	unsigned long accStartPktOutCount;	// value of pktOutCount when last access unit start was output
	int accTargetPacketCount;			// target packet count for current access unit as determined by bit rate
	int accTargetPictureCount;			// target picture count for current access unit as determined by bit rate and frame rate
	RTF_TIMESTAMP lastPCR;				// last PCR value written to this output
	BOOL progressiveSeq;				// true if currently working on a progressive sequence (MPEG2)
	BOOL pesCaptured;					// TRUE if PES header has been captured from input stream
	BOOL mainFileCopy;					// TRUE if this filter is producing a copy of the main file
#if GENERATE_TRACE_FILES
	int traceFile;
	unsigned long lastPictureNumber;
#endif

	// cached context
	unsigned short outPID;				// pid of last packet queued to output
	RTF_SES_HANDLE hSes;				// handle of session that owns this filter
	RTF_IDX_HANDLE hIdx;				// indexer handle
	RTF_PAT_HANDLE hPat;				// captured PAT handle
	RTF_CAT_HANDLE hCat;				// captured CAT handle
	RTF_PMT_HANDLE hPmt;				// captured PMT handle
	RTF_CAS_HANDLE hCas;				// conditional access system handle
	RTF_OUT_HANDLE hOut;				// handle of attached output object
	RTF_BUF_HANDLE hOutBuf;				// handle of output object data buffer

	// conditional access info
	BOOL catValid;						// TRUE if a conditional access table has been parsed
	unsigned short catEcmPid;			// CA Table Active ECM PID

	// current keyframe info
	RTF_PIC_HANDLE hPic;				// handle of picture
	RTF_PKT_HANDLE *phPkt;				// pointer to packet array of picture
	unsigned short pktIndex;			// index of current packet within array
	unsigned short pktCount;			// number of active packets in array
	unsigned char  gopPicCount;			// number of pictures in current group
	
	// no change frame info
	unsigned char *pNCFramePicHdr;
	unsigned char  ncFramePacketCount;
	unsigned short ncFrameTemporalRef;

	// some miscellaneous large arrays
	int incPidCount; // number of PIDs in the include list below
	int excPidCount; // number of PIDs in the include list below
	unsigned short incPidList [ RTF_TRICK_MAX_PIDCOUNT ];	// known PIDs being included in trick files
	unsigned short excPidList [ RTF_TRICK_MAX_PIDCOUNT ];	// known PIDs being excluded in trick files
	unsigned char  nextCC     [ RTF_TRICK_MAX_PIDCOUNT ];	// next CC value for each pid listed above
	unsigned char  tempPkt    [ TRANSPORT_PACKET_BYTES ];	// temporary packet buffer
	unsigned char  pesHdr     [ TRANSPORT_PACKET_BYTES ];	// captured PES header
	unsigned char  pesHdrPkt  [ TRANSPORT_PACKET_BYTES ];	// captured PES header in a packet
	unsigned char  ncFramePackets[ RTF_MAX_NCFRAME_PACKETS ][ TRANSPORT_PACKET_BYTES ];

	// some required statistics
	unsigned long picInCount;			// number of pictures input to this filter
	unsigned long picOutCount;			// number of pictures output from this filter
	unsigned long accOutCount;			// number of access units output from this filter
	unsigned long pktOutCount;			// number of packets output from this filter
	unsigned long keyStartPktCount;		// number of output packets at start of last keyframe

#ifdef DO_STATISTICS
	// input statistics
	unsigned long gopInCount;			// number of gops input to this filter
	unsigned long pktInCount;			// number of packets input to this filter
	unsigned long pcrInCount;			// number of PCRs input to this filter
	unsigned long pesInCount;			// number of PES headers input to this filter

	// output statistics
	unsigned long pesOutCount;			// number of PES headers output from this filter
	unsigned long pcrOutCount;			// number of PCRs output from this filter
	unsigned long vidOutCount;			// number of video packets output from this filter

	// insertion statistics (included in output totals above)
	unsigned long pesGenCount;			// number of PES headers inserted by this filter
	unsigned long pcrGenCount;			// number of PCRs inserted by this filter
	unsigned long padGenCount;			// number of stuffing packets inserted by this filter
	unsigned long patGenCount;			// number of PATs inserted by this filter
	unsigned long pmtGenCount;			// number of PMTs inserted by this filter
	unsigned long vidGenCount;			// number of video fluff packets inserted by this filter
	unsigned long pktGenCount;			// number of packets inserted by this filter
	unsigned long ncfGenCount;			// number of no-change frames inserted by this filter
	unsigned long augGenCount;			// number of augmentation packets inserted by this filter
	// other statistics
	unsigned long pesModCount;			// number of PES packet headers modified
	unsigned long partialFirstCount;	// number of partial first packets processed by this filter
	unsigned long partialLastCount;		// number of partial last packets processed by this filter
#endif

} RTF_FLT;

// forward declaration ******************************************************************

static RTF_RESULT rtfInsertVideoFluff( RTF_FLT *pFlt );

// private functions ********************************************************************

// set up the lists of PIDs to be included / excluded in the trick files for this filter
static RTF_RESULT rtfFltSetPidLists( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltSetPidLists" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_TRICK_SPEC *pSpec;
	RTF_STREAM_TYPE type, *pStreamTypes;
	BOOL suppress;
	unsigned short i;

	do {		 // error escape wrapper - begin

		// get the stream types from the PMT
		result = rtfPmtGetAllStreamTypes( pFlt->hPmt, &pStreamTypes );
		RTF_CHK_RESULT;
		// create a convenienece pointer to the trick spec
		pSpec = &( pFlt->trickSpec );
		// loop thru this array looking for known PIDs
		for( i=0; i<TRANSPORT_MAX_PID_VALUES; ++i )
		{
			type = pStreamTypes[ i ];
			if( type != RTF_STREAM_TYPE_UNKNOWN )
			{
				suppress = pSpec->suppressOTHER;
				switch( type )
				{
				case RTF_STREAM_TYPE_NUL:
					suppress = pSpec->suppressNUL;
					break;
				case RTF_STREAM_TYPE_PAT:
					suppress = pSpec->suppressPAT;
					break;
				case RTF_STREAM_TYPE_CAT:
					suppress = pSpec->suppressCAT;
					break;
				case RTF_STREAM_TYPE_PMT:
					suppress = pSpec->suppressPMT;
					break;
				case RTF_STREAM_TYPE_PCR:
				case RTF_STREAM_TYPE_VID:
					suppress = FALSE;
					break;
				case RTF_STREAM_TYPE_AUD:
					suppress = pSpec->suppressAUD;
					break;
				case RTF_STREAM_TYPE_DAT:
					suppress = pSpec->suppressDAT;
					break;
				case RTF_STREAM_TYPE_CA:
					suppress = pSpec->suppressCA;
					break;
				default:
					break;
				}
				if( suppress == FALSE )
				{
					pFlt->incPidList[ pFlt->incPidCount++ ] = i;
				}
				else
				{
					pFlt->excPidList[ pFlt->excPidCount++ ] = i;
				}
			} // if( type != RTF_STREAM_TYPE_UNKNOWN )
		} // for( i=0; i<TRANSPORT_MAX_PID_VALUES; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the index of a particular PID on the include PID list - -1 if it is not there
static void rtfFltFindIncPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex )
{
	int i;

	*pIndex = -1;
	// is this pid on the include list?
	for( i=0; i<pFlt->incPidCount; ++i )
	{
		if( pFlt->incPidList[ i ] == pid )
		{
			*pIndex = i;
			break;
		}
	}
}

// return the index of a particular PID on the exclude PID list - -1 if it is not there
static void rtfFltFindExcPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex )
{
	int i;

	*pIndex = -1;
	// is this pid on the exclude list?
	for( i=0; i<pFlt->excPidCount; ++i )
	{
		if( pFlt->excPidList[ i ] == pid )
		{
			*pIndex = i;
			break;
		}
	}
}

// get the current output bitstream PCR time according to position
// NOTE: for reverse trick files, this is based on a projection of
// the size of the current access unit, so this is only an estimate
static void rtfFltComputeOutputPcr( RTF_FLT *pFlt, RTF_TIMESTAMP *pPcr )
{
	RTF_TIMESTAMP offset;
	UINT64 temp64;

	// get the packet offset from the start of the output file for the current access unit
	temp64 = pFlt->accStartPktOutCount;
	// if this is a reverse trickfile, add the current access unit target size to this
	if( pFlt->trickSpec.speedDirection < 0 )
	{
		temp64 += pFlt->accTargetPacketCount;
	}
	// compute the elapsed time (in SCR clock ticks), according to the
	// number of bits computed above divided by the output bitrate
#ifdef DO_FLOATING_POINT
	{
		double dTemp;

		// packets
		dTemp = (double)temp64;
		// times bits per packet = bits
		dTemp *= (double)TRANSPORT_PACKET_BITS;
		// bits / bits per second = seconds
		dTemp /= pFlt->targetBitRate;
		// seconds times ticks per second = ticks
		dTemp *= (double)TRANSPORT_SCR_TICKS_PER_SECOND;
		// scale by the trick speed if requested
		dTemp = ( pFlt->trickSpec.interpTimeStamps == FALSE ) ? dTemp :
				pFlt->pProfile->streamPcrBase + ( ( dTemp * pFlt->trickSpec.speedNumerator ) / pFlt->trickSpec.speedDenominator );
		pPcr->base.ull = (UINT64)( dTemp / (double)TRANSPORT_SCR_TO_PCR_RATIO );
		pPcr->ext.us = (unsigned short)( dTemp - ( pPcr->base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) );
	}
#else
	{
		// packets times bits per packet = bits
		temp64 *= TRANSPORT_PACKET_BITS;
		// pre-scale
		temp64 <<= 16;
		// bits divided by bits per second = seconds
		temp64 /= pFlt->targetBitRate;
		// seconds times ticks per second = ticks
		temp64 *= TRANSPORT_SCR_TICKS_PER_SECOND;
		// normalize
		temp64 = ( temp64 + 0x8000 ) >> 16;
		// scale by the trick speed if requested
		temp64 = ( pFlt->trickSpec.interpTimeStamps == FALSE ) ? temp64 :
				 pFlt->pProfile->streamPcrBase + ( ( temp64 * pFlt->trickSpec.speedNumerator ) / pFlt->trickSpec.speedDenominator );
		pPcr->base.ull = temp64 / TRANSPORT_SCR_TO_PCR_RATIO;
		pPcr->ext.us = (unsigned short)( temp64 - ( pPcr->base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) );
	}
#endif
	// if this is a reverse trickfile, subtract the elapsed time from a large number
	if( pFlt->trickSpec.speedDirection < 0 )
	{
		pPcr->base.ull = RTF_BIG_PCR_BASE - pPcr->base.ull;
		pPcr->ext.us   = RTF_BIG_PCR_EXT  - pPcr->ext.us;
	}
#ifdef DO_FLOATING_POINT
	{
		double dTemp;

		// packets
		dTemp = pFlt->pktOutCount - pFlt->accStartPktOutCount;
		// times bits per packet = bits
		dTemp *= (double)TRANSPORT_PACKET_BITS;
		// bits / bits per second = seconds
		dTemp /= pFlt->targetBitRate;
		// seconds times ticks per second = ticks
		dTemp *= (double)TRANSPORT_SCR_TICKS_PER_SECOND;
		// scale by the trick speed if requested
		dTemp = ( pFlt->trickSpec.interpTimeStamps == FALSE ) ? dTemp :
				pFlt->pProfile->streamPcrBase + ( ( dTemp * pFlt->trickSpec.speedNumerator ) / pFlt->trickSpec.speedDenominator );
		offset.base.ull = (UINT64)( dTemp / (double)TRANSPORT_SCR_TO_PCR_RATIO );
		offset.ext.us = (unsigned short)( dTemp - ( offset.base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) );
	}
#else
	{
		// get the time offset of the current packet from the start of the current access unit
		temp64 = pFlt->pktOutCount - pFlt->accStartPktOutCount;
		temp64 *= TRANSPORT_PACKET_BITS;
		temp64 *= TRANSPORT_SCR_TICKS_PER_SECOND;
		temp64 += ( pFlt->targetBitRate>>1 );
		temp64 /= pFlt->targetBitRate;
		// scale by the trick speed if requested
		temp64 = ( pFlt->trickSpec.interpTimeStamps == FALSE ) ? temp64 :
				 pFlt->pProfile->streamPcrBase + ( ( temp64 * pFlt->trickSpec.speedNumerator ) / pFlt->trickSpec.speedDenominator );
		offset.base.ull = temp64 / TRANSPORT_SCR_TO_PCR_RATIO;
		offset.ext.us   = (unsigned short)( temp64 - ( offset.base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) );
	}
#endif
	pPcr->base.ull += offset.base.ull;
	pPcr->ext.us   += offset.ext.us;
	if( pPcr->ext.us >= TRANSPORT_SCR_TO_PCR_RATIO )
	{
		pPcr->ext.us -= TRANSPORT_SCR_TO_PCR_RATIO;
		++pPcr->base.ull;
	}
}

// generate a TTS timestamp based on current bitstream position
static RTF_RESULT rtfFltGenerateTTS( RTF_FLT *pFlt )
{
	RTF_TIMESTAMP tstamp;
	unsigned long ulTmp;
	unsigned char tts[ 4 ];

	// get the current PCR value, interpolated to the current packet
	// (as opposed to the start of the current access unit)
	rtfFltComputeOutputPcr( pFlt, &tstamp );
	// translate this to a TTS timestamp
	ulTmp    = (unsigned long)( ( tstamp.base.ull * 300 ) + tstamp.ext.us );
	tts[ 0 ] = (unsigned char)( ( ulTmp >> 24 ) & 0xFF );
	tts[ 1 ] = (unsigned char)( ( ulTmp >> 16 ) & 0xFF );
	tts[ 2 ] = (unsigned char)( ( ulTmp >> 8 ) & 0xFF );
	tts[ 3 ] = (unsigned char)( ulTmp & 0xFF );
	// queue the resulting TTS prefix to the output
	return rtfOutQueueData( pFlt->hOut, tts, TTS_HEADER_BYTES ); 
}

// initialize the bitrate queue to "steady state" condition to get a smooth start
static RTF_RESULT rtfInitializeBitrateQueue( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfInitializeBitrateQueue" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	UINT64 temp64;
	unsigned long queueInputBits;
	unsigned long queueOutputBits;
	unsigned long queueKeyfrBits;
	int i, iToAvg;

	do {		 // error escape wrapper - begin

		// set up the size of the bitrate control queue to suite the trick speed being generated
		i = 2 * ( ( pFlt->trickSpec.speedNumerator + pFlt->trickSpec.speedDenominator - 1 ) / pFlt->trickSpec.speedDenominator );
		while( ( pFlt->maxQueueFrames ) < i )
		{
			++pFlt->maxQueueFramesLog2;
			pFlt->maxQueueFrames = ( 1 << pFlt->maxQueueFramesLog2 );
			pFlt->maxQueueFramesMask = pFlt->maxQueueFrames - 1;
		}
		if( pFlt->maxQueueFramesLog2 > RTF_ABSMAX_QUEUE_FRAMES_LOG2 )
		{
			RTF_LOG_WARN3( RTF_MSG_WRN_BADTRICKSPEED, "Requested trick speed (%d/%d) exceeds maximum (%d/1)",
						pFlt->trickSpec.speedNumerator, pFlt->trickSpec.speedDenominator, RTF_ABSMAX_QUEUE_FRAMES );
			// substitute max values if warning is overridden
			pFlt->maxQueueFramesLog2 = RTF_ABSMAX_QUEUE_FRAMES_LOG2;
			pFlt->maxQueueFrames     = RTF_ABSMAX_QUEUE_FRAMES;
			pFlt->maxQueueFramesMask = RTF_ABSMAX_QUEUE_FRAMES - 1;
		}
		// fill the bitrate control queue with hypothetical "average" frames
		// compute the ideal "average" frame for this bitrate
		switch( pFlt->pProfile->videoSpec.eStream.video.vcdType )
		{
		case RTF_VIDEO_CODEC_TYPE_H264:
			iToAvg = RTF_NOMINAL_ITOAVG_RATIO_H264;
			break;
		case RTF_VIDEO_CODEC_TYPE_VC1:
			iToAvg = RTF_NOMINAL_ITOAVG_RATIO_VC1;
			break;
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
		default:
			iToAvg = RTF_NOMINAL_ITOAVG_RATIO_MPEG2;
		}
		// the average output queue entry should contain one frame time worth of output bits
		// bits per second
		temp64 = pFlt->targetBitRate - pFlt->forcePadBitRate;
		// avg bits per frame
		temp64 *= pFlt->secsPerFrameFix16;
		// normalize
		queueOutputBits = (unsigned long)( ( temp64 + 0x8000 ) >> 16 );
		// base the average keyframe size on the average frame size
		queueKeyfrBits = queueOutputBits * iToAvg;
		// downsize the output bit count by 1/8 to force the first frame to be selected
		queueOutputBits *= 7;
		queueOutputBits >>= 3;
		// this should be an integral number of packets
		queueOutputBits = ( ( queueOutputBits + ( TRANSPORT_PACKET_BITS>>1 ) ) / TRANSPORT_PACKET_BITS ) * TRANSPORT_PACKET_BITS;
		// compute the average number of input bits that correspond to this result
		temp64 = queueOutputBits;
		temp64 *= pFlt->speedRatioFix16;
		// normalize
		queueInputBits = (unsigned long)( ( temp64 + 0x8000 ) >> 16 );
		// this should also be an integral number of packets
		queueInputBits = ( ( queueInputBits + ( TRANSPORT_PACKET_BITS>>1 ) ) / TRANSPORT_PACKET_BITS ) * TRANSPORT_PACKET_BITS;
		// everything keyes off of maxQueueFramesLog2
		pFlt->maxQueueFrames = ( 1 << pFlt->maxQueueFramesLog2 );
		pFlt->maxQueueFramesMask = pFlt->maxQueueFrames - 1;
		// reset the queue
		memset( (void *)pFlt->queueInputBits, 0, sizeof(pFlt->queueInputBits) );
		memset( (void *)pFlt->queueFrameBits, 0, sizeof(pFlt->queueFrameBits) );
		memset( (void *)pFlt->queueStuffBits, 0, sizeof(pFlt->queueStuffBits) );
		memset( (void *)pFlt->queueKeyfrBits, 0, sizeof(pFlt->queueKeyfrBits) );
		// fill the active queue with "average" frames
		// !!! FIX ME !!! REPLICATE SELECTION CYCLE HERE? !!!
		for( i=0; i<pFlt->maxQueueFrames; ++i )
		{
			pFlt->queueInputBits[ i ] = queueInputBits;
			pFlt->queueFrameBits[ i ] = queueOutputBits;
			pFlt->queueStuffBits[ i ] = 0;
			pFlt->queueKeyfrBits[ i ] = queueKeyfrBits;
		}
		pFlt->queueTotalOutputBits	= pFlt->maxQueueFrames * queueOutputBits;
		pFlt->queueTotalInputBits	= pFlt->maxQueueFrames * queueInputBits;
		pFlt->queueTotalKeyfrBits	= pFlt->maxQueueFrames * queueKeyfrBits;
		// set up the initial estimates for the frame sizes to insure
		// that the first keyframe encountered is selected for inclusion
		pFlt->queueLastKeyframeBits = queueOutputBits * iToAvg;
		pFlt->queueLastNCFrameBits  = ( pFlt->trickSpec.insertNCF == FALSE ) ? 0 : RTF_MAX_NCFRAME_BITS;
		pFlt->queueNextInputBits	= 0;
		pFlt->queueNextFrameBits    = 0;
		pFlt->queueNextStuffBits    = 0;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// reset the filter structure to a clean state
static void rtfResetFlt( RTF_FLT *pFlt )
{
	pFlt->state					= RTF_FLT_STATE_CLOSED;

	pFlt->tgtInFrameNumber		= 0;
	pFlt->forcePadBitRate		= 0;
	pFlt->speedDirection		= 1;
	pFlt->speedRatioFix16		= RTF_DEFAULT_SPEED_RATIO_FIX16;
	pFlt->targetBitRate			= RTF_DEFAULT_BITS_PER_SECOND;
	pFlt->framesPerSecFix16		= RTF_DEFAULT_FPS_FIX16;
	pFlt->secsPerFrameFix16		= RTF_DEFAULT_SPF_FIX16;

	pFlt->frameTicks			= RTF_DEFAULT_FRAME_TICKS;
	pFlt->decodingDelay			= 0;
	pFlt->presentationDelay		= 0;
	pFlt->ptsCount				= 0;
	pFlt->lastPTS				= 0;
	pFlt->lastDTS				= 0;

	pFlt->maxQueueFramesLog2	= RTF_DEFAULT_QUEUE_FRAMES_LOG2;
	pFlt->maxQueueFrames		= ( 1 << RTF_DEFAULT_QUEUE_FRAMES_LOG2 );
	pFlt->maxQueueFramesMask	= ( 1 << RTF_DEFAULT_QUEUE_FRAMES_LOG2 ) - 1;
	pFlt->queueTotalOutputBits	= 0;
	pFlt->queueTotalInputBits	= 0;
	pFlt->queueTotalKeyfrBits	= 0;
	pFlt->queueLastKeyframeBits	= 0;
	pFlt->queueLastNCFrameBits	= 0;
	pFlt->queueNextFrameBits	= 0;
	pFlt->queueNextStuffBits    = 0;
	pFlt->queueNextKeyfrBits    = 0;
	pFlt->queueLastGroupPktNum  = 0;

	pFlt->pOutPktStorage		= (unsigned char *)NULL;
	pFlt->outPktNumber			= 0xFFFFFFFF;
	pFlt->outPktFlags			= 0;
	pFlt->outPktPayloadOffset	= 0;
	pFlt->outPktPesHdrOffset	= 0;
	pFlt->outPktPayloadBytes	= 0;
	pFlt->outPktFixupFlags		= 0;
	pFlt->outPktPesHdrBytes		= 0;

	pFlt->pesHdrLen				= 0;
	pFlt->firstBytePktOffset	= 0;
	pFlt->lastBytePktOffset		= 0;
	pFlt->lastVideoPacketIndex	= 0;

	pFlt->filterNumber			= 0;
	pFlt->accessUnitOutPackets	= 0;
	pFlt->ncAccessUnitOutPackets= 0;
	pFlt->packetsPerPcr			= 0xFFFFFFFF;
	pFlt->progressiveSeq		= FALSE;
	pFlt->outPID				= TRANSPORT_INVALID_PID;

	pFlt->hSes					= (RTF_SES_HANDLE)NULL;
	pFlt->hIdx					= (RTF_IDX_HANDLE)NULL;
	pFlt->hPat					= (RTF_PAT_HANDLE)NULL;
	pFlt->hCat					= (RTF_CAT_HANDLE)NULL;
	pFlt->hPmt					= (RTF_PMT_HANDLE)NULL;
	pFlt->hCas					= (RTF_CAS_HANDLE)NULL;
	pFlt->hOut					= (RTF_OUT_HANDLE)NULL;
	pFlt->hOutBuf				= (RTF_BUF_HANDLE)NULL;
	pFlt->hPic					= (RTF_PIC_HANDLE)NULL;

	pFlt->pProfile				= (RTF_STREAM_PROFILE *)NULL;

	pFlt->catValid				= FALSE;
	pFlt->catEcmPid				= TRANSPORT_INVALID_PID;

	pFlt->pktIndex				= 0;
	pFlt->pktCount				= 0;
	pFlt->gopPicCount			= 0;

	pFlt->pNCFramePicHdr		= (unsigned char *)NULL;
	pFlt->ncFrameTemporalRef	= 0;
	pFlt->ncFramePacketCount	= 0;
	pFlt->ncFrameTemporalRef	= 0;

	pFlt->pesCaptured			= FALSE;
	pFlt->mainFileCopy			= FALSE;
	pFlt->lastPcrPktOutCount	= 0;
	pFlt->accStartPktOutCount	= 0;
	pFlt->accTargetPacketCount	= 0;
	pFlt->accTargetPictureCount = 0;

	pFlt->picInCount			= 0;
	pFlt->picOutCount			= 0;
	pFlt->accOutCount			= 0;
	pFlt->pktOutCount			= 0;
	pFlt->keyStartPktCount		= 0;

#ifdef DO_STATISTICS
	pFlt->gopInCount			= 0;
	pFlt->pktInCount			= 0;
	pFlt->pcrInCount			= 0;
	pFlt->pesInCount			= 0;

	pFlt->pesOutCount			= 0;
	pFlt->pcrOutCount			= 0;
	pFlt->vidOutCount			= 0;

	pFlt->pesGenCount			= 0;
	pFlt->pcrGenCount			= 0;
	pFlt->patGenCount			= 0;
	pFlt->pmtGenCount			= 0;
	pFlt->vidGenCount			= 0;
	pFlt->padGenCount			= 0;
	pFlt->pktGenCount			= 0;
	pFlt->ncfGenCount			= 0;
	pFlt->augGenCount			= 0;

	pFlt->pesModCount			= 0;
	pFlt->partialFirstCount		= 0;
	pFlt->partialLastCount		= 0;
#endif

	pFlt->incPidCount           = 0;
	pFlt->excPidCount           = 0;

	memset( (void *)pFlt->outPktFixupOffset,	0, sizeof(pFlt->outPktFixupOffset)	);	
	memset( (void *)pFlt->queueFrameBits,		0, sizeof(pFlt->queueFrameBits)		);
	memset( (void *)pFlt->queueStuffBits,		0, sizeof(pFlt->queueStuffBits)		);
	memset( (void *)pFlt->queueInputBits,		0, sizeof(pFlt->queueInputBits)		);
	memset( (void *)pFlt->queueKeyfrBits,		0, sizeof(pFlt->queueKeyfrBits)		);
	memset( (void *)&pFlt->trickSpec,			0, sizeof(pFlt->trickSpec)			);
	memset( (void *)&pFlt->lastPCR,				0, sizeof(pFlt->lastPCR)			);
	memset( (void *)pFlt->incPidList,		 0xFF, sizeof(pFlt->incPidList)			);
	memset( (void *)pFlt->excPidList,		 0xFF, sizeof(pFlt->excPidList)			);
	memset( (void *)pFlt->nextCC,			 0x0F, sizeof(pFlt->nextCC)				);
	memset( (void *)pFlt->tempPkt,				0, sizeof(pFlt->tempPkt)			);
	memset( (void *)pFlt->pesHdr,				0, sizeof(pFlt->pesHdr)				);
	memset( (void *)pFlt->pesHdrPkt,			0, sizeof(pFlt->pesHdrPkt)			);
	memset( (void *)pFlt->ncFramePackets,		0, sizeof(pFlt->ncFramePackets)		);
}

// reset the next CC for a particular PID
static RTF_RESULT rtfFltResetNextCC( RTF_FLT *pFlt, unsigned short pid )
{
	RTF_FNAME( "rtfFltResetNextCC" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int index;

	do {		 // error escape wrapper - begin

		// get the index of this PID
		rtfFltFindIncPID( pFlt, pid, &index );
		// reset the CC value for this PID
		// note: due to CC pre-increment, this wants to be (0-1)%16!
		pFlt->nextCC[ index ] = 0x0F;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// find the next CC for a particular PID; return it and then increment it
static RTF_RESULT rtfFltFindNextCC( RTF_FLT *pFlt, unsigned short pid, unsigned char *pCC, unsigned long flags )
{
	RTF_FNAME( "rtfFltFindNextCC" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char cc = 0;
	int index;

	do {		 // error escape wrapper - begin

		// get the index of this PID
		rtfFltFindIncPID( pFlt, pid, &index );
		// check the packet flags - is a payload present?
		// note - payload-free packets don't increment the CC
		cc = pFlt->nextCC[ index ];
		if( ( flags & RTF_PKT_PAYLOADABSENT ) == 0 )
		{
			// yes - is this the video pid OR a forward file?
			if( ( pFlt->speedDirection >= 0 ) ||
				( pid == pFlt->pProfile->videoSpec.pid ) )
			{
				// yes - pre-increment the CC
				cc = ++pFlt->nextCC[ index ];
			}
			else
			{
				// no - post-decrement the CC
				cc = pFlt->nextCC[ index ]--;
			}
		}
		// make the return
		*pCC = cc & 0x0F;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// send a packet's worth of data to the output
static RTF_RESULT rtfFltQueueData( RTF_FLT *pFlt, unsigned char *pData )
{
	RTF_FNAME( "rtfFltQueueData" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned long bits = TRANSPORT_PACKET_BITS;
	unsigned short pid;

	do {		 // error escape wrapper - begin

		// is TTS output enabled?
		if( pFlt->trickSpec.generateTTS != FALSE )
		{
			result = rtfFltGenerateTTS( pFlt );
			RTF_CHK_RESULT;
			bits += TTS_HEADER_BITS;
		}
		result = rtfOutQueueData( pFlt->hOut, pData, TRANSPORT_PACKET_BYTES );
		RTF_CHK_RESULT;
		// get the updated buffer fill pointer
		result = rtfBufGetFillPointer( pFlt->hOutBuf, &pFlt->pOutPktStorage );
		RTF_CHK_RESULT;
		pFlt->pOutPktStorage -= TRANSPORT_PACKET_BYTES;
		// bump the running packet count
		++pFlt->pktOutCount;
		// bump the running packet generation count
		RTF_INC_STAT( pFlt->pktGenCount );
		// bump the current access unit packet count
		++pFlt->accessUnitOutPackets;
		// check the pid of this packet
		pid = pData[ 1 ];
		pid = ( pid << 8 ) | pData[ 2 ];
		pid &= 0x1FFF;
		// update the record of the last packet pid
		pFlt->outPID = pid;
		// update the apropriate next frame bit count
		if( pid == TRANSPORT_PAD_PID )
		{
			pFlt->queueNextStuffBits += bits;
		}
		else
		{
			pFlt->queueNextFrameBits += bits;
		}
		// bump the video output packet count if this was a video packet
		RTF_ADD_STAT( pFlt->vidOutCount, ( pid == pFlt->pProfile->videoSpec.pid ) ? 1 : 0 );
		// !!! TEST HACK !!! ADAPTATION PKT LEVEL DCON INSERTION FOR PANASONIC TTS DEMO
		// is all adaptation packet level DCON insertion enabled?
		if( pFlt->trickSpec.ttsDconAFPackets != FALSE )
		{
			// yes - is this a trick file?
			if( pFlt->trickSpec.speedNumerator > 1 )
			{
				// yes - does this packet have an adpatation field?
				if( ( pFlt->pOutPktStorage[3] & 0x20 ) != 0 )
				{
					// yes - set the DCON flag in the adaptation field
					pFlt->pOutPktStorage[5] |= 0x80;
				}
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a stuffing packet
static RTF_RESULT rtfFltInsertStuffingPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertStuffingPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pPkt = pFlt->tempPkt;

	do {		 // error escape wrapper - begin

		// generate a stuffing packet in the temp packet buffer
		memset( (void *)pPkt, 0xFF, TRANSPORT_PACKET_BYTES );
		// header
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( TRANSPORT_PAD_PID >> 8 ) & 0xFF );
		pPkt[ 2 ] = (unsigned char)( TRANSPORT_PAD_PID & 0xFF );
		pPkt[ 3 ] = 0x10;	// note - stuffing packets don't increment their CC
		result = rtfFltQueueData( pFlt, pPkt );
		RTF_CHK_RESULT;
		// bump the running pad generation count
		RTF_INC_STAT( pFlt->padGenCount );
		// bump the running packet generation count
		RTF_INC_STAT( pFlt->pktGenCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert an augmentation packet
static RTF_RESULT rtfFltInsertAugmentationPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertAugmentationPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned short pid;
	unsigned char *pPkt = pFlt->tempPkt;

	do {		 // error escape wrapper - begin

		// generate an augmentation packet in the temp packet buffer
		memset( (void *)pPkt, 0xFF, TRANSPORT_PACKET_BYTES );
		// header
		pid = pFlt->pProfile->augmentationPids[ 0 ];
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( pid >> 8 ) & 0xFF );
		pPkt[ 2 ] = (unsigned char)( pid & 0xFF );
		result = rtfFltFindNextCC( pFlt, pid, &( pPkt[ 3 ] ), 0 );
		RTF_CHK_RESULT;
		pPkt[ 3 ] = ( pPkt[ 3 ] & 0x0F ) | 0x10;
		result = rtfFltQueueData( pFlt, pPkt );
		RTF_CHK_RESULT;
		// bump the running augmentation packet generation count
		RTF_INC_STAT( pFlt->augGenCount );
		// bump the running packet generation count
		RTF_INC_STAT( pFlt->pktGenCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfFltCalculateTargetCounts( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltCalculateTargetCounts" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned long retainedOutputBits;
	unsigned long updatedInputBits;
	int targetOutputPkts;
	int i, vidCount, index;
	unsigned short pid, vidPid;

	do {		 // error escape wrapper - begin

		// NOTE: when this function is called, we have already
		// committed to using this keyframe in the output stream
		// compute the size of the base picture plus overhead
		// NOTE: this is an estimate!
		pFlt->accTargetPacketCount = pFlt->pktCount; // I-frame packet count
		pFlt->accTargetPacketCount += ( pFlt->trickSpec.insertPES == FALSE ) ? 0 : 1;
		pFlt->accTargetPacketCount += ( pFlt->trickSpec.insertPAT == FALSE ) ? 0 : 1;
		pFlt->accTargetPacketCount += ( pFlt->trickSpec.insertPMT == FALSE ) ? 0 : 1;
		// scan thru the packet array and subtract any packets that won't be included
		// in the trick file - also tally up the number of actual video packets
		vidCount = 0;
		vidPid = pFlt->pProfile->videoSpec.pid;
		if( pFlt->trickSpec.suppressOTHER != FALSE )
		{
			for( i=0; i<pFlt->pktCount; ++i )
			{
				result = rtfPktGetPID( pFlt->phPkt[ i ], &pid );
				RTF_CHK_RESULT;
				rtfFltFindExcPID( pFlt, pid, &index );
				pFlt->accTargetPacketCount -= ( index < 0 ) ? 0 : 1;
				vidCount += ( pid == vidPid ) ? 1 : 0;
			}
		}
		else
		{
			for( i=0; i<pFlt->pktCount; ++i )
			{
				result = rtfPktGetPID( pFlt->phPkt[ i ], &pid );
				RTF_CHK_RESULT;
				rtfFltFindIncPID( pFlt, pid, &index );
				pFlt->accTargetPacketCount -= ( index < 0 ) ? 1 : 0;
				vidCount += ( pid == vidPid ) ? 1 : 0;
			}
		}
		// if sequential CC is enabled, bring the number of
		// video packets up to (nearest multiple of 16) - 1
		if( pFlt->trickSpec.sequentialCC != FALSE )
		{
			pFlt->accTargetPacketCount += ( 0x0F - ( vidCount & 0x0F ) );
		}
		// is forced padding enabled?
		if( pFlt->trickSpec.forcePadding != FALSE )
		{
			// yes - add in the mandatory percentage of null packets
			pFlt->accTargetPacketCount += ( ( ( pFlt->accTargetPacketCount * pFlt->trickSpec.forcePaddingFactorFix8 ) + 255 ) >> 8 );
		}
		// is stuffing packet insertion enabled?
		if( ( pFlt->trickSpec.constantBitRate != FALSE ) &&
			( pFlt->trickSpec.insertNUL != FALSE ) )
		{
			// yes - compute the target packet count for this output frame.
			// this is done by getting the number of input packets that will
			// be spanned by the queue once this frame is added, divided by
			// the trick speed ratio, minus the number of output packets that
			// are retained by the queue before this frame is added.
			index = (int)( pFlt->picOutCount & pFlt->maxQueueFramesMask );
			// number of output bits being retained in queue
			retainedOutputBits = pFlt->queueTotalOutputBits - ( pFlt->queueFrameBits[ index ] + pFlt->queueStuffBits[ index ] );
			// number of input bits in queue after update
			updatedInputBits = ( pFlt->queueTotalInputBits - pFlt->queueInputBits[ index ] ) + pFlt->queueNextInputBits;
#ifdef DO_FLOATING_POINT
			{
				double dTemp;

				// target output bit count for queue is input bit total divided by speed ratio
				dTemp = (double)updatedInputBits * (double)pFlt->trickSpec.speedDenominator / (double)pFlt->trickSpec.speedNumerator;
				// target output bit count for this frame is above less retained output bits
				dTemp -= (double)retainedOutputBits;
				// this bit count would bring the trick file bitrate exactly up to the target rate
				// however, want to leave enough bandwidth so that the bitrate control loop will include
				// the next I-frame from the main file. Subtract the average keyframe size in bits
				dTemp -= (double)( pFlt->queueTotalKeyfrBits >> pFlt->maxQueueFramesLog2 );
				// get the equivalent number of packets
				dTemp /= (double)TRANSPORT_PACKET_BITS;
				targetOutputPkts = (int)dTemp;
				// packet count can never be less than the size of the access unit with no stuffing
				pFlt->accTargetPacketCount = MAX( pFlt->accTargetPacketCount, targetOutputPkts );
				// the target picture count is computed by dividing the time consumed
				// by the target packet count by the time consumed by one video frame
				dTemp =  (double)pFlt->accTargetPacketCount;	// pkts
				dTemp *= (double)TRANSPORT_PACKET_BITS;			// pkts * ( bits / pkt ) = bits
				dTemp /= (double)pFlt->pProfile->bitsPerSecond;	// bits / ( bits / sec ) = secs
				dTemp *= (double)pFlt->framesPerSecFix16;		// secs * ( pics / sec ) = pics
				pFlt->accTargetPictureCount = ( (int)( dTemp ) ) >> 16; // normalize
			}
#else
			{
				INT64 temp64;
				int targetOutputBits;

				// number of output bits being retained in queue
				retainedOutputBits = pFlt->queueTotalOutputBits - ( pFlt->queueFrameBits[ index ] + pFlt->queueStuffBits[ index ] );
				// number of input bits in queue after update
				updatedInputBits = ( pFlt->queueTotalInputBits - pFlt->queueInputBits[ index ] ) + pFlt->queueNextInputBits;
				// divided by speed ratio
				temp64 = ( (INT64)updatedInputBits ) * pFlt->invSpeedRatioFix16;
				// normalize
				targetOutputBits = (int)( ( temp64 + 0x8000 ) >> 16 );
				// subtract retained output bits (but never go negative)
				targetOutputBits -= retainedOutputBits;
				// get the equivalent number of packets
				temp64 = (INT64)( targetOutputBits ) * INV_TRANSPORT_PACKET_BITS_FIX16;
				// normalize
				targetOutputPkts = (int)( ( temp64 + 0x8000 ) >> 16 );
				// this bit count would bring the trick file bitrate exactly up to the target rate
				// however, want to leave enough bandwidth so that the bitrate control loop will include
				// the next I-frame from the main file.
				temp64 -= pFlt->queueLastKeyframeBits;
				// packet count can never be less than the size of the access unit with no stuffing
				pFlt->accTargetPacketCount = MAX( pFlt->accTargetPacketCount, targetOutputPkts );
				// the target picture count is computed by dividing the time consumed
				// by the target packet count by the time consumed by one video frame
				temp64 = pFlt->accTargetPacketCount;		// pkts
				temp64 *= TRANSPORT_PACKET_BITS;			// pkts * bits per pkt = bits
				temp64 <<= 16;								// prescale
				temp64 /= pFlt->pProfile->bitsPerSecond;	// bits / bits per sec = sec
				temp64 >>= 16;								// normalize
				temp64 *= pFlt->framesPerSecFix16;			// sec * frames per sec = frames
				pFlt->accTargetPictureCount = (int)( temp64 >> 16 ); // normalize
			}
#endif
			// never let the targets go negative
			pFlt->accTargetPacketCount  = ( pFlt->accTargetPacketCount  < 0 ) ? 0 : pFlt->accTargetPacketCount;
			pFlt->accTargetPictureCount = ( pFlt->accTargetPictureCount < 0 ) ? 0 : pFlt->accTargetPictureCount;
			pFlt->accTargetPictureCount = MIN( pFlt->accTargetPictureCount, RTF_MAX_TARGET_PICS );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// replace the oldest bitrate queue entry with a new entry
static RTF_RESULT rtfFltUpdateBitrate( RTF_FLT *pFlt, BOOL isKeyframe )
{
	RTF_FNAME( "rtfFltUpdateBitrate" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int i, index, packets, bits;

	do {		 // error escape wrapper - begin

		// get the index of the oldest bitrate queue entry
		index = (int)( pFlt->picOutCount & pFlt->maxQueueFramesMask );
		// subtract the bit count of the output frame and stuffing that are going away
		pFlt->queueTotalOutputBits -= ( pFlt->queueFrameBits[ index ] + pFlt->queueStuffBits[ index ] );
		// subtract the number of input bits that are going away
		pFlt->queueTotalInputBits -= pFlt->queueInputBits[ index ];
		// subtract the number of keyframe bits that are going away
		pFlt->queueTotalKeyfrBits -= pFlt->queueKeyfrBits[ index ];
		// is this a keyframe?
		if( isKeyframe == FALSE )
		{
			// no - update the record of the last no-change frame size
			pFlt->queueLastNCFrameBits = pFlt->queueNextFrameBits;
		}
		else
		{
			// yes - is "constant bit rate" enabled?
			//   AND is "insert null packets" enabled?
			//   AND is this a keyframe?
			if( ( pFlt->trickSpec.insertNUL != FALSE ) &&
				( pFlt->trickSpec.constantBitRate != FALSE ) &&
				( isKeyframe != FALSE ) )
			{
				// yes - add enough stuffing to reach the target packet count for the next queue entry
				packets = pFlt->accTargetPacketCount -
						  ( pFlt->queueNextFrameBits / TRANSPORT_PACKET_BITS );
				// don't insert a negative number of null packets!
				if( packets > 0 )
				{
					// insert the stuffing packets
					for( i=0; i<packets; ++i )
					{
						result = rtfFltInsertStuffingPacket( pFlt );
						RTF_CHK_RESULT;
					}
					RTF_CHK_RESULT_LOOP;
				}
			}
			// still a keyframe - are augmentation PIDs present?
			if( pFlt->pProfile->augmentationPidCount > 0 )
			{
				// yes - insert some fake augmentation packets
				// these will provide the extra bandwidth required
				// to boost the trickfile to the augmented bitrate
				// note: no need to track these packets - they do not
				// affect any of the control loops
				// the number of augmentation bits to add is the number of unaugmented bits for this frame
				bits = pFlt->queueNextFrameBits + pFlt->queueStuffBits[ index ];
				// times the ratio of the augmentation plus factor to the unaugmentation base factor
				bits *= pFlt->pProfile->augmentationPlusFactor;
				bits += ( pFlt->pProfile->augmentationBaseFactor>>1 );
				bits /= pFlt->pProfile->augmentationBaseFactor;
				packets = (int)( ( bits + ( TRANSPORT_PACKET_BITS>>1 ) ) / TRANSPORT_PACKET_BITS );
				// insert the appropriate type of augmentation packets
				if( pFlt->trickSpec.augmentWithNUL != FALSE )
				{
					for( i=0; i<packets; ++i )
					{
						result = rtfFltInsertStuffingPacket( pFlt );
						RTF_CHK_RESULT;
					}
					RTF_CHK_RESULT_LOOP;
				}
				else if( pFlt->trickSpec.augmentWithFLUFF != FALSE )
				{
					for( i=0; i<packets; ++i )
					{
						result = rtfInsertVideoFluff( pFlt );
						RTF_CHK_RESULT;
					}
					RTF_CHK_RESULT_LOOP;
				}
				else if( pFlt->trickSpec.augmentWithPID != FALSE )
				{
					for( i=0; i<packets; ++i )
					{
						result = rtfFltInsertAugmentationPacket( pFlt );
						RTF_CHK_RESULT;
					}
					RTF_CHK_RESULT_LOOP;
				}
			} // if( pFlt->pProfile->augmentationPidCount > 0 )
			// update the record of the last keyframe size
			pFlt->queueLastKeyframeBits = pFlt->queueNextFrameBits;
		} // if( isKeyframe != FALSE )
		// add the new frame to the bit rate control queue
		pFlt->queueInputBits[ index ] = pFlt->queueNextInputBits;
		pFlt->queueFrameBits[ index ] = pFlt->queueNextFrameBits;
		pFlt->queueStuffBits[ index ] = pFlt->queueNextStuffBits;
		pFlt->queueKeyfrBits[ index ] = pFlt->queueNextKeyfrBits;
		// adjust the total input bit count for the queue
		pFlt->queueTotalInputBits += pFlt->queueNextInputBits;
		// adjust the total keyframe bit count for the queue
		pFlt->queueTotalKeyfrBits += pFlt->queueNextKeyfrBits;
		// reset the next input bits counter
		pFlt->queueNextInputBits = 0;
		// adjust the total output bit count for the queue
		pFlt->queueTotalOutputBits += pFlt->queueNextFrameBits + pFlt->queueNextStuffBits;
		// reset the next frame bit counts
		pFlt->queueNextFrameBits = 0;
		pFlt->queueNextStuffBits = 0;
		// bump the output picture count
		++pFlt->picOutCount;
		// is forced padding enabled?
		if( pFlt->trickSpec.forcePadding != FALSE )
		{
			// yes - add in the required percentage of null packets
			packets = ( ( pFlt->pktCount * pFlt->trickSpec.forcePaddingFactorFix8 ) + 0x80 ) >> 8;
			for( i=0; i<packets; ++i )
			{
				result = rtfFltInsertStuffingPacket( pFlt );
				RTF_CHK_RESULT;
			}
			RTF_CHK_RESULT_LOOP;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// measure the output bitrate, averaged over the frames in the bitrate control queue
// NOTE: this routine is called before a new input frame is loaded
static void rtfFltGetNextOutputBitRates( RTF_FLT *pFlt, unsigned long *pKEYRate, unsigned long *pNCFRate )
{
	int index;

	// get index of oldest queue entry
	index = (int)( pFlt->picOutCount & pFlt->maxQueueFramesMask );
#ifdef DO_FLOATING_POINT
	{
		double dInputSeconds;
		double dOutputSeconds;
		double dRetainedOutputBits;

		dInputSeconds = pFlt->queueTotalInputBits;
		dInputSeconds /= pFlt->targetBitRate;
		dOutputSeconds = ( dInputSeconds * pFlt->trickSpec.speedDenominator ) / pFlt->trickSpec.speedNumerator;
		// get the number of output bits retained by queue after update
		dRetainedOutputBits = (double)( pFlt->queueTotalOutputBits - ( pFlt->queueFrameBits[ index ] + pFlt->queueStuffBits[ index ] ) );
		*pKEYRate = (unsigned long)( ( dRetainedOutputBits + pFlt->queueLastKeyframeBits ) / dOutputSeconds );
		*pNCFRate = (unsigned long)( ( dRetainedOutputBits + pFlt->queueLastNCFrameBits ) / dOutputSeconds );
	}
#else
	{
		UINT64 inputSecondsFix16;
		UINT64 outputSecondsFix16;
		UINT64 retainedOutputBits;

		// bits
		inputSecondsFix16 = pFlt->queueTotalInputBits;
		// pre-scale
		inputSecondsFix16 <<= 16;
		// bits divided by bits per second = seconds
		inputSecondsFix16 /= pFlt->targetBitRate;
		// scale down by trick speed ratio
		outputSecondsFix16 = ( inputSecondsFix16 * pFlt->trickSpec.speedDenominator ) / pFlt->trickSpec.speedNumerator;
		// get the number of output bits retained by queue after update
		retainedOutputBits = pFlt->queueTotalOutputBits - ( pFlt->queueFrameBits[ index ] + pFlt->queueStuffBits[ index ] );
		*pKEYRate = (unsigned long)( ( ( retainedOutputBits + pFlt->queueLastKeyframeBits ) << 16 ) / outputSecondsFix16 );
		*pNCFRate = (unsigned long)( ( ( retainedOutputBits + pFlt->queueLastNCFrameBits ) << 16 ) / outputSecondsFix16 );
	}
#endif
}

// Split the payload of the last packet sent to the output into 2 packets. Returns with
// the output packet image adjusted, and the removed portion of the payload in a packet
// in the temp buffer. The bytes in the second buffer are at the same offset as in the
// original, so the remaining fixup offsets do not need to be changed. The removed payload
// bytes are replaced with padding bytes in the adaptation field of the original packet. 
// NOTE: all fixups that map to features earlier in the packet must be performed before
// calling this routine, since this routine may split the pac
// NOTE: split off more than the bare minimum number of bytes in order to avoid having a
// header straddle more than a single packet.
static RTF_RESULT rtfFltSplitLastOutPayload( RTF_FLT *pFlt, unsigned char byteCount )
{
	RTF_FNAME( "rtfFltSplitLastOutPayload" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char oldPayloadOffset;
	unsigned char oldPayloadBytes;
	unsigned char newPayloadOffset;
	unsigned char cc;
	unsigned short pid;
	unsigned char *pOldPkt;
	unsigned char *pNewPkt;
	unsigned char *pSrc;
	unsigned char *pDst;
	int i;

	do {		 // error escape wrapper - begin

		// set a pointer to the image of the last packet sent to the output buffer
		pOldPkt = pFlt->pOutPktStorage;
		// can't split a packet with no payload
		if( ( pOldPkt[ 3 ] & 0x30 ) == 0x20 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No payload" );
			break;
		}
		// record the offset of the payload
		oldPayloadOffset = ( ( pOldPkt[ 3 ] & 0x20 ) == 0 ) ? 4 : 5 + pOldPkt[ 4 ];
		// compute payload byte count
		oldPayloadBytes = TRANSPORT_PACKET_BYTES - oldPayloadOffset;
		// compose a new packet in the temp buffer
		pid = pOldPkt[ 1 ] & 0x1F;
		pid = ( pid << 8 ) | pOldPkt[ 2 ];
		// get the next continuity counter value for this PID
		result = rtfFltFindNextCC( pFlt, pid, &cc, 0 );
		RTF_CHK_RESULT;
		pNewPkt = pFlt->tempPkt;
		memset( (void *)pNewPkt, 0xFF, TRANSPORT_PACKET_BYTES );
		pNewPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pNewPkt[ 1 ] = pOldPkt[ 1 ] & 0x1F;
		pNewPkt[ 2 ] = pOldPkt[ 2 ];
		pNewPkt[ 3 ] = ( pOldPkt[ 3 ] & 0xC0 ) | 0x30 | ( cc & 0x0F );
		newPayloadOffset = TRANSPORT_PACKET_BYTES - byteCount;
		pNewPkt[ 4 ] = TRANSPORT_PACKET_BYTES - ( 5 + byteCount );
		// no flags set in adaptation field header
		pNewPkt[ 5 ] = 0x00;
		// copy the bytes being removed from the old packet into the payload of the new packet
		memcpy( &( pNewPkt[ newPayloadOffset ] ), &( pOldPkt[ newPayloadOffset ] ), byteCount );
		// copy the remaining payload in the old packet to the end of the packet
		oldPayloadBytes -= byteCount;
		pSrc = &pOldPkt[ TRANSPORT_PACKET_BYTES - byteCount ];
		pDst = &pOldPkt[ TRANSPORT_PACKET_BYTES ];
		for( i=0; i<oldPayloadBytes; ++i )
		{
			*--pDst = *--pSrc;
		}
		// null out the new padding region in the old packet's adaptation field
		memset( (void *)&pOldPkt[ oldPayloadOffset ], 0xFF, byteCount );
		// create an adaptation field in the old packet if there isn't one already present
		if( ( pOldPkt[ 3 ] & 0x20 ) == 0 )
		{
			// yes - create a null adaptation field
			pOldPkt[ 3 ] |= 0x20;
			pOldPkt[ 4 ] = byteCount - 1;
			pOldPkt[ 5 ] = 0x00;
		}
		else
		{
			// no - adjust the old packet's adaptation field length
			pOldPkt[ 4 ] += byteCount;
			// need to claim extra byte for null adaptation flags byte
			// if old adaptation field was length zero (i.e. flags were omitted)
			if( byteCount == 169 )
			{
				pOldPkt[ 5 ] = 0;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// adjust a packet in the output buffer to include only the trailing data from the
// indicated input packet. also make all necessary adjustments to recorded fixup list
static RTF_RESULT rtfFltAdjustPartialFirstPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltAdjustPartialFirstPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int i;
	unsigned char off;

	do {		 // error escape wrapper - begin

		// want to zero out all of the bytes that precede the start of the current picture
		// however, retain any PES header that may be lurking at the start of the packet
		off = ( ( pFlt->outPktFlags & RTF_PKT_PESHDRPRESENT ) == 0 ) ?
			  pFlt->outPktPayloadOffset :
			  pFlt->outPktPesHdrOffset + pFlt->outPktPesHdrBytes;
		memset( (void *)( pFlt->pOutPktStorage + off ), 0x00, ( pFlt->firstBytePktOffset - off ) );
		// iterate through the list of fixups; remove any that are no longer relevant
		for( i=0; i<8; ++i )
		{
			// is this an active fixup?
			if( ( pFlt->outPktFixupFlags & ( 1 << i ) ) != 0 )
			{
				// yes - is the offset below the cut-off point?
				if( pFlt->outPktFixupOffset[ i ] < pFlt->firstBytePktOffset )
				{
					// yes - clear the fixup bit
					pFlt->outPktFixupFlags &= ~( 1 << i );
				}
			}
		} // for( i=0; i<8; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// adjust a packet in the output buffer to include only the leading data from the
// indicated input packet. also make all necessary adjustments to recorded fixup list
static RTF_RESULT rtfFltAdjustPartialLastPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltAdjustPartialLastPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		// want to zero out all of the bytes that follow the end of the current picture
		memset( (void *)( pFlt->pOutPktStorage + pFlt->lastBytePktOffset ), 0x00, ( TRANSPORT_PACKET_BYTES - pFlt->lastBytePktOffset ) );
		// iterate through the list of fixups; remove any that are no longer relevant
		for( i=0; i<8; ++i )
		{
			// is this an active fixup?
			if( ( pFlt->outPktFixupFlags & ( 1 << i ) ) != 0 )
			{
				// yes - is the offset above the cut-off point?
				if( pFlt->outPktFixupOffset[ i ] > pFlt->lastBytePktOffset )
				{
					// yes - clear the fixup bit
					pFlt->outPktFixupFlags &= ~( 1 << i );
				}
			}
		} // for( i=0; i<8; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// list all pending MPEG-2 specific fixups
static RTF_RESULT rtfListMpeg2Fixups( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfListMpeg2Fixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	BOOL isVirtual;

	do {		 // error escape wrapper - begin

		// is the sequence header present flag set?
		if( ( pFlt->outPktFlags & RTF_PKT_SEQHDRPRESENT ) != 0 )
		{
			// yes - currently no fixups required for sequence header
			if( 0 )
			{
				// yes - record this fixup
				pFlt->outPktFixupFlags |= ( 1 << RTF_FLT_FIXUP_SEQHDR );
				// get the offset of the start of the sequence header
				result = rtfPktGetSeqStartOffset( pFlt->phPkt[ pFlt->pktIndex ],
							&pFlt->outPktFixupOffset[ RTF_FLT_FIXUP_SEQHDR ], &isVirtual );
				RTF_CHK_RESULT;
			}
		}
		// is the sequence extension header present flag set?
		if( ( pFlt->outPktFlags & RTF_PKT_SQXHDRPRESENT ) != 0 )
		{
			// yes - is setLowDelay enabled?
			if( pFlt->trickSpec.codec.mpeg2.setLowDelay != FALSE )
			{
				// yes - record this fixup
				pFlt->outPktFixupFlags |= ( 1 << RTF_FLT_FIXUP_SQXHDR );
				// get the offset of the start of the sequence extension header
				result = rtfPktGetSqxHdrOffset( pFlt->phPkt[ pFlt->pktIndex ],
								&pFlt->outPktFixupOffset[ RTF_FLT_FIXUP_SQXHDR ] );
				RTF_CHK_RESULT;
			}
			// record the state of the progressive sequence flag
			// (referenced in repeat first field fixup)
			pFlt->progressiveSeq = ( ( pFlt->outPktFlags & RTF_PKT_PROGRESSIVESEQ ) != 0 ) ? TRUE : FALSE;
		}
		// is the group header present flag set?
		if( ( pFlt->outPktFlags & RTF_PKT_GOPHDRPRESENT ) != 0 )
		{
			// yes - is clearGopTime enabled?
			if( ( pFlt->trickSpec.codec.mpeg2.clearGOPTime   != FALSE ) ||
				( pFlt->trickSpec.codec.mpeg2.clearDropFrame != FALSE ) ||
				( pFlt->trickSpec.codec.mpeg2.setClosedGOP   != FALSE ) ||
				( pFlt->trickSpec.codec.mpeg2.setBrokenLink  != FALSE ) )
			{
				// yes - record this fixup
				pFlt->outPktFixupFlags |= ( 1 << RTF_FLT_FIXUP_GOPHDR );
				// get the offset of the start of the GOP
				result = rtfPktGetGopStartOffset( pFlt->phPkt[ pFlt->pktIndex ],
							&pFlt->outPktFixupOffset[ RTF_FLT_FIXUP_GOPHDR ], &isVirtual);
				RTF_CHK_RESULT;
			}
		}
		// is the picture start present flag set?
		if( ( pFlt->outPktFlags & RTF_PKT_PICSTARTPRESENT ) != 0 )
		{
			// yes - is clearTemporalRef or resetVBVDelay enabled?
			if( ( pFlt->trickSpec.codec.mpeg2.clearTemporalRef != FALSE ) ||
				( pFlt->trickSpec.codec.mpeg2.resetVBVDelay    != FALSE ) )
			{
				// yes - record this fixup
				pFlt->outPktFixupFlags |= ( 1 << RTF_FLT_FIXUP_PICHDR );
				// get the offset of the start of the picture header
				result = rtfPktGetPicStartOffset( pFlt->phPkt[ pFlt->pktIndex ],
								&pFlt->outPktFixupOffset[ RTF_FLT_FIXUP_PICHDR ] );
				RTF_CHK_RESULT;
			}
		}
		// is the picture coding extension header present flag set?
		if( ( pFlt->outPktFlags & RTF_PKT_CODHDRPRESENT ) != 0 )
		{
			// yes - is clear32Pulldown enabled?
			if( pFlt->trickSpec.codec.mpeg2.clear32Pulldown != FALSE )
			{
				// yes - record this fixup
				pFlt->outPktFixupFlags |= ( 1 << RTF_FLT_FIXUP_CODHDR );
				// get the offset of the start of the picture coding extension header
				result = rtfPktGetCodHdrOffset( pFlt->phPkt[ pFlt->pktIndex ],
								&pFlt->outPktFixupOffset[ RTF_FLT_FIXUP_CODHDR ] );
				RTF_CHK_RESULT;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// list all pending H264 specific fixups
static RTF_RESULT rtfListH264Fixups( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfListH264Fixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// placeholder

	} while( 0 ); // error escape wrapper - end

	return result;
}

// list all pending VC1 specific fixups
static RTF_RESULT rtfListVc1Fixups( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfListVc1Fixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// placeholder

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfFltListPayloadFixups( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltListPayloadFixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char payOff;

	do {		 // error escape wrapper - begin
		// reset the fixup flags
		pFlt->outPktFixupFlags = 0;
		// clear the fixup offset array
		RTF_CLR_STATE( pFlt->outPktFixupOffset, sizeof(pFlt->outPktFixupOffset) );
		// only apply fixups to video packets
		if( pFlt->outPID != pFlt->pProfile->videoSpec.pid )
		{
			break;
		}
		// create a list of the CODEC dependent fixups that are needed
		switch( pFlt->pProfile->videoSpec.eStream.video.vcdType )
		{
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			result = rtfListMpeg2Fixups( pFlt );
			RTF_CHK_RESULT;
			break;
		case RTF_VIDEO_CODEC_TYPE_H264:
			result = rtfListH264Fixups( pFlt );
			RTF_CHK_RESULT;
			break;
		case RTF_VIDEO_CODEC_TYPE_VC1:
			result = rtfListVc1Fixups( pFlt );
			RTF_CHK_RESULT;
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized video CODEC type (%d)",
						  pFlt->pProfile->videoSpec.eStream.video.vcdType );
		}
		RTF_CHK_RESULT_LOOP;
		// is there a PES header in this packet?
		if( ( pFlt->outPktFlags & RTF_PKT_PESHDRPRESENT ) != 0 )
		{
			// yes - is it encrypted?
			if( ( pFlt->outPktFlags & RTF_PKT_PAYLOADENCRYPTED ) != 0 )
			{
				// yes - this can cause problems. report it and turn off the PES header flag
				RTF_LOG_WARN1( RTF_MSG_WRN_CRYPTOPES, "Encrypted PES header encountered at packet %d", pFlt->outPktNumber );
				pFlt->outPktFlags &= ~RTF_PKT_PESHDRPRESENT;
				break;
			}
			// yes - are any PESHDR fixups enabled?
			if( ( pFlt->trickSpec.suppressInputPesHdr != FALSE ) ||
				( pFlt->trickSpec.zeroPesPktLen		  != FALSE ) ||
				( pFlt->trickSpec.clearDAI			  != FALSE ) ||
				( pFlt->trickSpec.suppressPTSDTS	  != FALSE ) ||
				( pFlt->trickSpec.suppressDTS		  != FALSE ) ||
				( pFlt->trickSpec.restampPTSDTS		  != FALSE ) ||
				( pFlt->trickSpec.insertDSM			  != FALSE ) )
			{
				// yes - record this fixup
				pFlt->outPktFixupFlags |= ( 1 << RTF_FLT_FIXUP_PESHDR );
				// get the pes hdr info from the packet
				result = rtfPktGetPesHdrInfo( pFlt->phPkt[ pFlt->pktIndex ], &payOff,
						&pFlt->outPktFixupOffset[ RTF_FLT_FIXUP_PESHDR ], &pFlt->outPktPesHdrBytes );
				RTF_CHK_RESULT;
			}
		} // PES header

	} while( 0 ); // error escape wrapper - end

	return result;
}

// do fixups on a PCR from the input stream that is being included in the filtered output
static RTF_RESULT rtfFltFixPCR( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltFixPCR" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_TIMESTAMP pcr;
	unsigned char *pOutPktStorage;
	unsigned char len;
	int i;

	do {		 // error escape wrapper - begin

		pOutPktStorage = pFlt->pOutPktStorage;
		// bump the PCR input count
		RTF_INC_STAT( pFlt->pcrInCount );
		// is input PCR suppression turned on?
		if( pFlt->trickSpec.suppressInputPCR != FALSE )
		{
			// yes - remove this PCR from the packet
			// make sure there is an adaptation field
			if( ( pOutPktStorage[ 3 ] & 0x20 ) == 0 )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No adaptation field" );
				break;
			}
			// make sure the PCR present flag is set
			if( ( pOutPktStorage[ 5 ] & 0x10 ) == 0 )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No PCR" );
				break;
			}
			// clear the PCR present flag
			pOutPktStorage[ 5 ] &= 0xEF;
			// get the length of the adaptation field
			len = pOutPktStorage[ 4 ];
			// collapse the adaptation field over the PCR
			for( i=6; i<len-1; ++i )
			{
				pOutPktStorage[ i ] = pOutPktStorage[ i+6 ];
			}
			// pad the end of the adaptation field to compensate
			for( ; i<(len+5); ++i )
			{
				pOutPktStorage[ i ] = 0xFF;
			}
		}
		else
		{
			// !!! TEST HACK !!! PCR LEVEL DCON INSERTION FOR PANASONIC TTS DEMO
			// is PCR level DCON insertion enabled?
			if( pFlt->trickSpec.ttsDconPcrPacket != FALSE )
			{
				// yes - is this a trick file?
				if( pFlt->trickSpec.speedNumerator > 1 )
				{
					// yes - set the DCON flag in the adaptation field
					pOutPktStorage[5] |= 0x80;
				}
			}
			// no - is PCR restamping enabled?
			if( pFlt->trickSpec.restampPCR != FALSE )
			{
				// yes - restamp this PCR
				rtfFltComputeOutputPcr( pFlt, &pcr );
				// re-write the PCR with this value		// set the PCR (5 bytes)
				pcr.base.ull <<= 7;
				pOutPktStorage[6]   = pcr.base.uc[4];
				pOutPktStorage[7]   = pcr.base.uc[3];
				pOutPktStorage[8]   = pcr.base.uc[2];
				pOutPktStorage[9]   = pcr.base.uc[1];
				pOutPktStorage[10]  = pcr.base.uc[0];
				pOutPktStorage[10] |= pcr.ext.uc[1] & 0x01;
				pOutPktStorage[11]  = pcr.ext.uc[0];
			}
			// record the last pcr packet number
			pFlt->lastPcrPktOutCount = pFlt->pktOutCount;
			// bump the running PCR output count
			RTF_INC_STAT( pFlt->pcrOutCount );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfFltFixPacketHeader( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltFixPacketHeader" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pOutPktStorage;
	unsigned short remapPid;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// make a convenience copy of the packet storage pointer
		pOutPktStorage = pFlt->pOutPktStorage;
		// is PID remapping enabled?
		if( pFlt->trickSpec.remapPIDs != FALSE )
		{
			// yes - look up the remap value of this PID
			remapPid = pFlt->trickSpec.remapList[ pFlt->outPID ];
			// is it different?
			if( remapPid != pFlt->outPID )
			{
				// yes - change the PID in the output image of the packet header
				pOutPktStorage[ 1 ] = ( pOutPktStorage[ 1 ] & (~0x1F) ) | ( (remapPid>>8) & 0x1F );
				pOutPktStorage[ 2 ] = (unsigned char)( remapPid & 0xFF );
			}
		}
		// is there a PES header in this packet?
		if( ( pFlt->outPktFlags & RTF_PKT_PESHDRPRESENT ) != 0 )
		{
			// bump the input PES header count
			RTF_INC_STAT( pFlt->pesInCount );
			// is PES header suppression enabled?
			if( pFlt->trickSpec.suppressInputPesHdr != FALSE )
			{
				// yes - clear the payload unit start bit
				pOutPktStorage[ 1 ] &= 0xBF;
			}
			// is setPRIO enabled?
			if( pFlt->trickSpec.setPRIO != FALSE )
			{
				// yes - set the estream priority flag
				pOutPktStorage[ 1 ] |= 0x20;
			}
			// is setRAND enabled?
			if( pFlt->trickSpec.setRAND != FALSE )
			{
				// yes - set the random access packet flag
				pOutPktStorage[ 1 ] |= 0x40;
			}
		}
		// is sequential CC correction enabled?
		if( pFlt->trickSpec.sequentialCC != FALSE )
		{
			// yes - find the next CC for this PID
			result = rtfFltFindNextCC( pFlt, pFlt->outPID, &cc, pFlt->outPktFlags );
			RTF_CHK_RESULT;
			// change the CC in the image of the packet data in the output buffer
			pOutPktStorage[ 3 ] = ( pOutPktStorage[ 3 ] & 0xF0 ) | cc;
		}
		// is the discontinuity flag set in this packet?
		if( ( pFlt->outPktFlags & RTF_PKT_DISCONTINUITY ) != 0 )
		{
			// yes - is clear discontinuity enabled?
			if( pFlt->trickSpec.clearDCON != FALSE )
			{
				// yes - clear the DCON bit
				pOutPktStorage[ 5 ] &= 0x7F;
			}
		}
		// is there a PCR in this packet?
		if( ( pFlt->outPktFlags & RTF_PKT_PCRPRESENT ) != 0 )
		{
			// yes - process the PCR
			result = rtfFltFixPCR( pFlt );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// queue an input packet to the output
static RTF_RESULT rtfFltQueuePacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltQueuePacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned long outPktNumber;
	unsigned long bits = TRANSPORT_PACKET_BITS;
	unsigned char *pPktStorage;

	do {		 // error escape wrapper - begin

		// get the info on this packet - update the last output pid record
		result = rtfPktGetInfo( pFlt->phPkt[ pFlt->pktIndex ], &outPktNumber, &pFlt->outPID,
								&pFlt->outPktFlags, &pPktStorage,
								&pFlt->outPktPayloadOffset, &pFlt->outPktPayloadBytes,
								&pFlt->outPktPesHdrOffset, &pFlt->outPktPesHdrBytes );
		RTF_CHK_RESULT;
		// is TTS output enabled?
		if( pFlt->trickSpec.generateTTS != FALSE )
		{
			result = rtfFltGenerateTTS( pFlt );
			RTF_CHK_RESULT;
			bits += TTS_HEADER_BITS;
		}
		// queue the packet to the output buffer
		// Note: the packet being queued is guaranteed to be
		// in the output buffer when this call is completed
		result = rtfOutQueuePacket( pFlt->hOut, pFlt->phPkt[ pFlt->pktIndex ] );
		RTF_CHK_RESULT;
		// update the bit count of the next frame to be added to the bit rate control queue
		pFlt->queueNextFrameBits += bits;
		// bump the output packet count
		++pFlt->pktOutCount;
		// bump the current access unit packet count
		++pFlt->accessUnitOutPackets;
		// bump the output video packet count if this was a video packet
		RTF_ADD_STAT( pFlt->vidOutCount, ( pFlt->outPID == pFlt->pProfile->videoSpec.pid ) ? 1 : 0 );
		// create a list of any fixups that will be required for this packet
		result = rtfFltListPayloadFixups( pFlt );
		RTF_CHK_RESULT;
		// update the packet number record
		pFlt->outPktNumber = outPktNumber;
		// get the updated buffer fill pointer
		result = rtfBufGetFillPointer( pFlt->hOutBuf, &pFlt->pOutPktStorage );
		RTF_CHK_RESULT;
		pFlt->pOutPktStorage -= TRANSPORT_PACKET_BYTES;
		// is this a partial last packet?
		// (i.e. only the leading part of the payload used in the current picture)
		// NOTE: for rtfFltBuf, lastBytePktOffset will be zero.
		if( ( pFlt->lastBytePktOffset != 0 ) &&
			( pFlt->pktIndex == pFlt->lastVideoPacketIndex ) &&
			( pFlt->lastBytePktOffset < ( pFlt->outPktPayloadOffset + pFlt->outPktPayloadBytes - 1) ) )
		{
			// yes - issue a warning
			RTF_LOG_WARN1( RTF_MSG_WRN_SPLITPKT, "Last packet of picture split between 2 pictures at packet %d", outPktNumber );
			// if warning is overridden, modify the image of the packet in the output buffer
			// and the list of fixups recorded above
			result = rtfFltAdjustPartialLastPacket( pFlt );
			RTF_CHK_RESULT;
			// bump the partial last packet count
			RTF_INC_STAT( pFlt->partialLastCount );
		}
		// is this a partial first packet?
		// (i.e. only the trailing part of the payload used in the current picture)
		// NOTE: for rtfFltBuf, lastBytePktOffset will be zero.
		if( ( pFlt->pktIndex == 0 ) &&
			( pFlt->firstBytePktOffset > pFlt->outPktPayloadOffset ) )
		{
			// yes - issue a warning
			RTF_LOG_WARN1( RTF_MSG_WRN_SPLITPKT, "First packet of picture split between 2 pictures at packet %d", outPktNumber );
			// if warning is overridden, modify the image of the packet in the output buffer
			// and the list of fixups recorded above
			result = rtfFltAdjustPartialFirstPacket( pFlt );
			RTF_CHK_RESULT;
			// bump the partial first packet count
			RTF_INC_STAT( pFlt->partialFirstCount );
		}
		// perform any required fixups on the packet header
		result = rtfFltFixPacketHeader( pFlt );
		RTF_CHK_RESULT;
		// !!! TEST HACK !!! ADAPTATION PKT LEVEL DCON INSERTION FOR PANASONIC TTS DEMO
		// is all adaptation packet level DCON insertion enabled?
		if( pFlt->trickSpec.ttsDconAFPackets != FALSE )
		{
			// yes - is this a trick file?
			if( pFlt->trickSpec.speedNumerator > 1 )
			{
				// yes - does this packet have an adpatation field?
				if( ( pFlt->pOutPktStorage[3] & 0x20 ) != 0 )
				{
					// yes - set the DCON flag in the adaptation field
					pFlt->pOutPktStorage[5] |= 0x80;
				}
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfFltQueueVideoPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltQueueVideoPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int i, index;
	unsigned short pid;

	do {		 // error escape wrapper - begin

		for( i=0; ; ++i )
		{
			// move on to the next packet in the array
			if( ++(pFlt->pktIndex) >= pFlt->pktCount )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "No next video packet in picture %d", pFlt->picOutCount );
				break;
			}
			// get the info on the next packet
			result = rtfPktGetPID( pFlt->phPkt[ pFlt->pktIndex ], &pid );
			RTF_CHK_RESULT;
			if( pFlt->trickSpec.suppressOTHER == FALSE )
			{
				// is this PID to be excluded?
				rtfFltFindExcPID( pFlt, pid, &index );
				if( index >= 0 )
				{
					// yes
					continue;
				}
			}
			else
			{
				// is this PID to be included?
				rtfFltFindIncPID( pFlt, pid, &index );
				if( index < 0 )
				{
					// no
					continue;
				}
			}
			// is encryption being ignored?
			if( pFlt->trickSpec.ignoreEncryption == FALSE )
			{
				// no - run the packet about to be output through the CA system for processing
				result = rtfCasProcessPkt( pFlt->hCas, pFlt->phPkt[ pFlt->pktIndex ], pFlt->hOut );
				RTF_CHK_RESULT;
			}
			// Queue this input packet to the output buffer
			result = rtfFltQueuePacket( pFlt );
			RTF_CHK_RESULT;
			// was it a video packet?
			if( pid == pFlt->pProfile->videoSpec.pid )
			{
				// yes - escape the search loop
				break;
			}

		} // for( i=0; ; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// fix a byte in the output buffer
// Note: this does not need to be used on packet header fixups - only payload fixups!
static RTF_RESULT rtfFltFixByte( RTF_FLT *pFlt, unsigned char andValue,
								 unsigned char orValue, unsigned char *pOffset )
{
	RTF_FNAME( "rtfFltFixByte" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char offset;

	do {		 // error escape wrapper - begin

		// is the byte to be fixed in the output buffer?
		offset = *pOffset;
		if( offset >= ( pFlt->outPktPayloadOffset + pFlt->outPktPayloadBytes ) )
		{
			// begin adjusting the offset
			offset -= ( pFlt->outPktPayloadOffset + pFlt->outPktPayloadBytes );
			// queue the next video packet to the output buffer
			result = rtfFltQueueVideoPacket( pFlt);
			RTF_CHK_RESULT;
			// further adjust the offset
			offset += pFlt->outPktPayloadOffset;
			// does the new packet have a PES header?
			if( pFlt->outPktPesHdrBytes > 0 )
			{
				// yes - is the PES header at the start of the packet?
				if( pFlt->outPktPesHdrOffset == pFlt->outPktPayloadOffset )
				{
					// yes - adjust offset to skip the PES header
					offset += pFlt->outPktPesHdrBytes;
				}
			}
			// return the corrected offset to the caller
			*pOffset = offset;
		}
		// fix the byte
		pFlt->pOutPktStorage[ offset ] = ( pFlt->pOutPktStorage[ offset ] & andValue ) | orValue;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// preform sequence header fixups
static RTF_RESULT rtfFltFixSeqHdr( RTF_FLT *pFlt, unsigned char seqHdrOffset )
{
	RTF_FNAME( "rtfFltFixSeqHdr" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// currently no fixups for sequence header

	} while( 0 ); // error escape wrapper - end

	return result;
}

// preform sequence extension header fixups
static RTF_RESULT rtfFltFixSqxHdr( RTF_FLT *pFlt, unsigned char sqxHdrOffset )
{
	RTF_FNAME( "rtfFltFixSqxHdr" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// set the low delay bit
		sqxHdrOffset += 9;
		result = rtfFltFixByte( pFlt, 0xFF, 0x80, &sqxHdrOffset );
		RTF_CHK_RESULT;
		sqxHdrOffset -= 9;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// preform GOP header fixups
static RTF_RESULT rtfFltFixGopHdr( RTF_FLT *pFlt, unsigned char gopHdrOffset )
{
	RTF_FNAME( "rtfFltFixGopHdr" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char off;

	do {		 // error escape wrapper - begin

		if( pFlt->trickSpec.codec.mpeg2.clearGOPTime != FALSE )
		{
			// clear the GOP header time field
			off = gopHdrOffset + 4;
			result = rtfFltFixByte( pFlt, 0x80, 0x00, &off );
			RTF_CHK_RESULT;
			++off;
			result = rtfFltFixByte( pFlt, 0x00, 0x08, &off );
			RTF_CHK_RESULT;
			++off;
			result = rtfFltFixByte( pFlt, 0x00, 0x00, &off );
			RTF_CHK_RESULT;
			++off;
			result = rtfFltFixByte( pFlt, 0x7F, 0x00, &off );
			RTF_CHK_RESULT;
			gopHdrOffset = off - 7;
		}
		if ( pFlt->trickSpec.codec.mpeg2.clearDropFrame != FALSE )
		{
			off = gopHdrOffset + 4;
			result = rtfFltFixByte( pFlt, 0x7F, 0x00, &off );
			RTF_CHK_RESULT;
			gopHdrOffset = off - 4;
		}
		if ( pFlt->trickSpec.codec.mpeg2.setClosedGOP  != FALSE )
		{
			off = gopHdrOffset + 7;
			result = rtfFltFixByte( pFlt, 0xFF, 0x40, &off );
			RTF_CHK_RESULT;
			gopHdrOffset = off - 7;
		}
		if ( pFlt->trickSpec.codec.mpeg2.setBrokenLink  != FALSE )
		{
			off = gopHdrOffset + 7;
			result = rtfFltFixByte( pFlt, 0xFF, 0x20, &off );
			RTF_CHK_RESULT;
			gopHdrOffset = off - 7;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// preform picture header fixups
static RTF_RESULT rtfFltFixPicHdr( RTF_FLT *pFlt, unsigned char picHdrOffset )
{
	RTF_FNAME( "rtfFltFixPicHdr" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char off;

	do {		 // error escape wrapper - begin

		// is clearTemporalRef enabled?
		if( pFlt->trickSpec.codec.mpeg2.clearTemporalRef != FALSE )
		{
			// yes - clear the PIC header temporal reference field
			off = picHdrOffset + 4;
			result = rtfFltFixByte( pFlt, 0x00, 0x00, &off );
			RTF_CHK_RESULT;
			++off;
			result = rtfFltFixByte( pFlt, 0x3F, 0x00, &off );
			RTF_CHK_RESULT;
			picHdrOffset = off - 5;
		}
		// is reset VBV delay enabled?
		if( pFlt->trickSpec.codec.mpeg2.resetVBVDelay != FALSE )
		{
			// yes - reset the VBV delay field
			off = picHdrOffset + 5;
			result = rtfFltFixByte( pFlt, 0xFF, 0x07, &off );
			RTF_CHK_RESULT;
			++off;
			result = rtfFltFixByte( pFlt, 0xFF, 0xFF, &off );
			RTF_CHK_RESULT;
			++off;
			result = rtfFltFixByte( pFlt, 0xFF, 0xF8, &off );
			RTF_CHK_RESULT;
			picHdrOffset = off - 7;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// preform picture coding header fixups
static RTF_RESULT rtfFltFixCodHdr( RTF_FLT *pFlt, unsigned char codHdrOffset )
{
	RTF_FNAME( "rtfFltFixCodHdr" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// adjust offset to flag containing repeat first field flag
		codHdrOffset += 7;

#if 0 // !!! TEST HACK !!! 3-2 PULLDOWN HACKERY
{
	// print out the fields related to 3/2 pulldown; change nothing
	unsigned char rff, tff, pfr, str;
	// focus the "fix window" on the repeat first field flag (don't change it yet)
	result = rtfFltFixByte( pFlt, 0xFF, 0x00, &codHdrOffset );
	RTF_CHK_RESULT;
	rff = ( ( pFlt->pOutPktStorage[ codHdrOffset   ] & 0x02 ) != 0 ) ? TRUE : FALSE;
	tff = ( ( pFlt->pOutPktStorage[ codHdrOffset   ] & 0x80 ) != 0 ) ? TRUE : FALSE;
	pfr = ( ( pFlt->pOutPktStorage[ codHdrOffset+1 ] & 0x80 ) != 0 ) ? TRUE : FALSE;
	str =     pFlt->pOutPktStorage[ codHdrOffset-1 ] & 0x03;
	RTF_LOG_INFO5( RTF_MSG_INF_STATS, "progseq=%d struct=%d tff=%d rff=%d progframe=%d", pFlt->progressiveSeq, str, tff, rff, pfr );
	codHdrOffset -= 7;
}
#endif

#if 0	// NOTE: get MPROBE warnings, but the trickfiles play. However, get macroblocks after a while in trick play
		// clear the repeat first field flag
		result = rtfFltFixByte( pFlt, 0xFD, 0x00, &codHdrOffset );
		RTF_CHK_RESULT;
#endif

#if 0 // NOTE: gets rid of MPROBE warnings, but trickfiles don't play on SA
		// clear the repeat first field flag and the top field first flag
		result = rtfFltFixByte( pFlt, 0x7D, 0x00, &codHdrOffset );
		RTF_CHK_RESULT;
#endif

#if 0 // NOTE: seems correct according to the spec, but makes more MPROBE warnings, and trickfiles don't play on SA
		// focus the "fix window" on the repeat first field flag (don't change it yet)
		result = rtfFltFixByte( pFlt, 0xFF, 0x00, &codHdrOffset );
		RTF_CHK_RESULT;
		// is repeat first field flag set?
		if( ( pFlt->pOutPktStorage[ codHdrOffset ] & 0x02 ) != 0 )
		{
			// yes - clear it
			result = rtfFltFixByte( pFlt, 0xFD, 0x00, &codHdrOffset );
			RTF_CHK_RESULT;
			// also clear the progressive frame flag
			++codHdrOffset;
			result = rtfFltFixByte( pFlt, 0x7F, 0x00, &codHdrOffset );
			RTF_CHK_RESULT;
		}
#endif

#if 0 // NOTE: looking at WWWest, looks like RFFIELD and PROGFRAME get set together; TFFIRST may or may not be set
		// focus the "fix window" on the repeat first field flag (don't change it yet)
		result = rtfFltFixByte( pFlt, 0xFF, 0x00, &codHdrOffset );
		RTF_CHK_RESULT;
		// is repeat first field flag set?
		if( ( pFlt->pOutPktStorage[ codHdrOffset ] & 0x02 ) != 0 )
		{
			// yes - clear repeat first field and top field first
			result = rtfFltFixByte( pFlt, 0x7D, 0x00, &codHdrOffset );
			RTF_CHK_RESULT;
			// also clear the progressive frame flag
			++codHdrOffset;
			result = rtfFltFixByte( pFlt, 0x7F, 0x00, &codHdrOffset );
			RTF_CHK_RESULT;
		}
#endif

#if 1 // NOTE: always clear TFF, RFF, and PROGFRAME
		// clear repeat first field; force top field first
		result = rtfFltFixByte( pFlt, 0xFD, 0x80, &codHdrOffset );
		RTF_CHK_RESULT;
		// also clear the progressive frame flag
		++codHdrOffset;
		result = rtfFltFixByte( pFlt, 0x7F, 0x00, &codHdrOffset );
		RTF_CHK_RESULT;
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// check for stuffing bytes in an adaptation field, if any
static int rtfGetAdaptStuffingBytes( unsigned char *pPkt )
{
	int stuffingBytes = 0;
	unsigned char flags, len, end, off;

	// is an adaptation field present?
	if( ( pPkt[ 3 ] & 0x20 ) != 0 )
	{
		// yes - record the adaptation field length
		len = pPkt[ 4 ];
		// more than just the single byte?
		if( len > 0 )
		{
			// yes - parse the flags
			end = len + 4;
			flags = pPkt[ 5 ];
			off = 5;
			off += ( ( flags & 0x10 ) != 0 ) ? 6 : 0;				// PCR
			off += ( ( flags & 0x08 ) != 0 ) ? 6 : 0;				// OPCR
			off += ( ( flags & 0x04 ) != 0 ) ? 1 : 0;				// splice point
			off += ( ( flags & 0x02 ) != 0 ) ? pPkt[ off ] + 1 : 0;	// private
			off += ( ( flags & 0x01 ) != 0 ) ? pPkt[ off ] + 1 : 0;	// extension
			// the number of stuffing bytes is the difference between
			// the final offset and the end of the adaptation field
			stuffingBytes = end - off;
		}
	}
	return stuffingBytes;
}

static int rtfGetPesHeaderStuffingBytes( unsigned char *pHdr )
{
	unsigned char act, flags;

	// calculate the number of "active" bytes in the PES header
	flags = pHdr[ 7 ];				// PES field flags
	act = 0;
	act += ( ( flags & 0x80 ) == 0 ) ? 0 : 5;		// PTS
	act += ( ( flags & 0x40 ) == 0 ) ? 0 : 5;		// DTS
	act += ( ( flags & 0x20 ) == 0 ) ? 0 : 6;		// ESCR
	act += ( ( flags & 0x10 ) == 0 ) ? 0 : 3;		// ESRATE
	act += ( ( flags & 0x08 ) == 0 ) ? 0 : 1;		// DSM
	act += ( ( flags & 0x04 ) == 0 ) ? 0 : 1;		// ACI
	act += ( ( flags & 0x02 ) == 0 ) ? 0 : 2;		// CRC
	if( ( flags & 0x01 ) != 0 )		// PES extension field flags
	{
		flags = pHdr[ ++act ];
		act += ( ( flags & 0x80 ) == 0 ) ? 0 : 128;	// PPD
		act += ( ( flags & 0x40 ) == 0 ) ? 0 : 1;	// PHF
		act += ( ( flags & 0x20 ) == 0 ) ? 0 : 2;	// PPSC
		act += ( ( flags & 0x10 ) == 0 ) ? 0 : 2;	// PSTD
		if( ( flags & 0x01 ) != 0 )	// PES extension field flags 2
		{
			act += pHdr[ ++act ] & 0x7F;
		}
	}
	// return the difference between the number of active bytes
	// and the number of data bytes recorded in the header
	return ( pHdr[ 8 ] - act );
}

// apply fixups to a PES header
// NOTE: all fixups of features that occur before the PES header must be done before this
// routine is called, since this routine may split the packet in order to insert new fields.
static RTF_RESULT rtfFltApplyPesHdrFixups( RTF_FLT *pFlt, unsigned char *pPkt, unsigned char *pHdr, BOOL *pModFlag )
{
	RTF_FNAME( "rtfFltApplyPesHdrFixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_TIMESTAMP pcr;
	unsigned char *pSrc, *pDst;
	unsigned char hdrLength, flags, byteCount;
	unsigned char delta, pesOff, payOff, dsmOff;
	BOOL splitPkt = FALSE;
	int i;

	do {		 // error escape wrapper - begin

		// is zeroPesLen enabled?
		if( pFlt->trickSpec.zeroPesPktLen != FALSE )
		{
			// yes - make sure that the pes packet length field is 0
			if( pHdr[ 4 ] != 0 )
			{
				pHdr[ 4 ] = 0;
				*pModFlag = TRUE;
			}
			if( pHdr[ 5 ] != 0 )
			{
				pHdr[ 5 ] = 0;
				*pModFlag = TRUE;
			}
		}
		// is clearDAI enabled?
		if( ( pFlt->trickSpec.clearDAI != FALSE ) & ( ( pHdr[ 6 ] & 0x04 ) != 0 ) )
		{
			// yes - clear the data alignment indicator bit
			pHdr[ 6 ] &= (~0x04);
			*pModFlag = TRUE;
		}
		// are either of the PTS / DTS flags set?
		if( ( pHdr[ 7 ] & 0xC0 ) != 0 )
		{
			// yes - get the pes header length
			hdrLength = pHdr[ 8 ];
			// is suppressPTSDTS enabled?
			if( pFlt->trickSpec.suppressPTSDTS != FALSE )
			{
				// yes - are both PTS and DTS present?
				if( ( pHdr[ 7 ] & 0xC0 ) == 0xC0 )
				{
					// yes - remove the PTS and DTS fields (10 bytes) from the PES header
					for( i=9; i<9+(hdrLength-10); ++i )
					{
						pHdr[ i ] = pHdr[ i + 10 ];
					}
					// back-fill the header with stuffing bytes
					for( ; i<(9+hdrLength); ++i )
					{
						pHdr[ i ] = 0xFF;
					}
				}
				else
				{
					// no - only PTS. Remove the PTS field (5 bytes) from the PES header
					for( i=9; i<(9+(hdrLength-5)); ++i )
					{
						pHdr[ i ] = pHdr[ i + 5 ];
					}
					// back-fill the header with stuffing bytes
					for( ; i<(9+hdrLength); ++i )
					{
						pHdr[ i ] = 0xFF;
					}
				}
				// clear the PTS and DTS flags
				pHdr[ 7 ] &= 0x3F;
				*pModFlag = TRUE;
			}
			else	// if( pFlt->trickSpec.suppressPTSDTS != FALSE )
			{
				// is suppressDTS enabled?
				if( pFlt->trickSpec.suppressDTS != FALSE )
				{
					// is a DTS present?
					if( ( pHdr[ 7 ] & 0xC0 ) == 0xC0 ) 
					{
						// yes - change the PTS / DTS flags to PTS only
						pHdr[ 7 ] &= ~0x40;
						// remove the DTS field (5 bytes) from the PES header
						for( i=14; i<(9+(hdrLength-5)); ++i )
						{
							pHdr[ i ] = pHdr[ i + 5 ];
						}
						// back-fill the header with stuffing bytes
						for( ; i<(9+hdrLength); ++i )
						{
							pHdr[ i ] = 0xFF;
						}
						*pModFlag  = TRUE;
					}
				}
				// is PTS / DTS restamping enabled?
				if( pFlt->trickSpec.restampPTSDTS != FALSE )
				{
					// yes - is a PTS present?
					if( ( pHdr[ 7 ] & 0x80 ) != 0 ) 
					{
						// yes - compute the current interpolated PCR value
						rtfFltComputeOutputPcr( pFlt, &pcr );
						// the DTS is the PCR plus or minus the decoding delay
						pFlt->lastDTS = pcr.base.ull;
						pFlt->lastDTS += ( pFlt->trickSpec.speedDirection < 0 ) ?
										 -pFlt->decodingDelay : pFlt->decodingDelay;
						// the PTS value is the DTS value plus the presentation delay
						pFlt->lastPTS = pFlt->lastDTS + pFlt->presentationDelay;
						// bump the PTS counter
						++pFlt->ptsCount;
						// restamp the PTS
						// Note: this restamping mechanism relies on the fact
						// that only I and P frames are ever present in trickfiles,
						// and decode order is the same as display order for them.
						pFlt->lastPTS <<= 3;
						pHdr[  9 ] = ( (unsigned char)( pFlt->lastPTS>>32 ) & 0x0E ) | ( ( pHdr[ 7 ] & 0xC0 ) >> 2 ) | 0x01;
						pFlt->lastPTS >>= 1;
						pHdr[ 10 ] = (unsigned char)( ( pFlt->lastPTS>>24 ) & 0xFF );
						pHdr[ 11 ] = ( (unsigned char)( pFlt->lastPTS>>16 ) & 0xFE ) | 0x01;
						pFlt->lastPTS >>= 1;
						pHdr[ 12 ] = (unsigned char)( ( pFlt->lastPTS>>8 ) & 0xFF );
						pHdr[ 13 ] = ( (unsigned char)( pFlt->lastPTS ) & 0xFE ) | 0x01;
						pFlt->lastPTS >>= 1;
						// is a DTS also present?
						if( ( pHdr[ 7 ] & 0x40 ) != 0 )
						{
							// yes - restamp the DTS
							pFlt->lastDTS <<= 3;
							pHdr[ 14 ] = ( (unsigned char)( pFlt->lastDTS>>32) & 0x0E ) | 0x11;
							pFlt->lastDTS >>= 1;
							pHdr[ 15 ] = (unsigned char)( ( pFlt->lastDTS>>24 ) & 0xFF );
							pHdr[ 16 ] = ( (unsigned char)( pFlt->lastDTS>>16 ) & 0xFE ) | 0x01;
							pFlt->lastDTS >>= 1;
							pHdr[ 17 ] = (unsigned char)( ( pFlt->lastDTS>>8 ) & 0xFF );
							pHdr[ 18 ] = ( (unsigned char)( pFlt->lastDTS ) & 0xFE ) | 0x01;
							pFlt->lastDTS >>= 1;
						}
						// set the packet modified flag
						*pModFlag  = TRUE;
					} // if( ( pHdr[ 7 ] & 0xC0 ) != 0 )
				} // if( pFlt->trickSpec.restampPTSDTS != FALSE )
			} // if( pFlt->trickSpec.suppressPTSDTS != FALSE )
		} // if( ( pHdr[ 7 ] & 0xC0 ) != 0 ) // PTS / DTS on?
		// is insertDSM enabled?
		if( pFlt->trickSpec.insertDSM != FALSE )
		{
			// yes - is the DSM trick mode flag already set?
			if( ( pHdr[ 7 ] & 0x08 ) == 0 )
			{
				// no - need 1 byte to insert DSM byte
				// is there an adaptation field in this packet with a spare stuffing byte?
				if( rtfGetAdaptStuffingBytes( pPkt ) <= 0 )
				{
					// no - calculate the offset of the first byte of the PES packet payload
					pesOff = pHdr - pPkt;
					dsmOff = pesOff + 9;
					payOff = dsmOff + pHdr[ 8 ];
					// compute the offset of the DSM target byte within the PES header
					flags = pHdr[ 7 ];				// PES field flags
					dsmOff += ( ( flags & 0x80 ) == 0 ) ? 0 : 5;		// PTS
					dsmOff += ( ( flags & 0x40 ) == 0 ) ? 0 : 5;		// DTS
					dsmOff += ( ( flags & 0x20 ) == 0 ) ? 0 : 6;		// ESCR
					dsmOff += ( ( flags & 0x10 ) == 0 ) ? 0 : 3;		// ESRATE
					// is there at least 1 padding byte in the PES header?
					if( rtfGetPesHeaderStuffingBytes( pHdr ) > 0 )
					{
						// yes - copy everything in the PES header between the target DSM byte
						// to the end of the header up one byte (overwriting the last byte)
						pDst = &pPkt[ payOff ];
						pSrc = pDst - 1;
						for( i=payOff-1; i>dsmOff; --i )
						{
							*--pDst = *--pSrc;
						}
					}
					else
					{
						// no - is the end of the PES header in this packet?
						if( payOff <= TRANSPORT_PACKET_BYTES )
						{
							// yes - move what follows the PES header into the next packet
							byteCount = TRANSPORT_PACKET_BYTES - payOff;
						}
						else
						{
							// no - move the PES header into the next packet
							byteCount = TRANSPORT_PACKET_BYTES - pesOff;
						}
						// split the packet and create at least one stuffing byte in the adaptation field
						result = rtfFltSplitLastOutPayload( pFlt, byteCount );
						RTF_CHK_RESULT;
						// the PES header was just moved to the end of the new adaptation field
						delta  = 5 + pPkt[ 4 ] - pesOff;
						pesOff = 5 + pPkt[ 4 ];
						pHdr   += delta;
						dsmOff += delta;
						// remember that we will have to send the split payload when we are done
						splitPkt = TRUE;
						// increment the PES header data length
						++( pPkt[ pesOff + 8 ] );
						// decrement the length of the packet's adaptation field to reflect
						// the padding byte that is about to be absorbed by the PES header
						--pPkt[ 4 ];
						// copy from the start of the PES header
						// to the target DSM byte downward by 1 byte
						pSrc = &pPkt[ pesOff ];
						pDst = pSrc - 1;
						for( i=pesOff; i<dsmOff; ++i )
						{
							*pDst++ = *pSrc++;
						}
						// adjust the PES header references down one byte to match
						--dsmOff;
						--pHdr;
					} // if( byteCount <= hdrLen )
					// insert the DSM trick mode byte at this point
					// note: always pretend we are trick-playing forward
					pPkt[ dsmOff ] = RTF_DSM_FF_BYTE;
					*pModFlag = TRUE;
					// set the DSM trick mode flag in the PES header
					pHdr[ 7 ] |= 0x08;
				} // if( rtfGetAdaptStuffingBytes( pPkt ) <= 0 )
			} // if( ( pHdr[ 7 ] & 0x08 ) == 0 )
		} // if( pFlt->trickSpec.insertDSM != FALSE )
		// was the packet containing the PES header just split?
		if( splitPkt != FALSE )
		{
			// yes - send the packet in tempPkt to the output
			result = rtfFltQueueData( pFlt, pFlt->tempPkt );
			RTF_CHK_RESULT;
			// update the output packet pointer
			result = rtfBufGetFillPointer( pFlt->hOutBuf, &pFlt->pOutPktStorage );
			RTF_CHK_RESULT;
			pFlt->pOutPktStorage -= TRANSPORT_PACKET_BYTES;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// suppress a PES packet header
static void rtfFltSuppressPesHdr( RTF_FLT *pFlt, unsigned char *pPkt, unsigned char payloadOffset,
								  unsigned char pesHdrOffset, unsigned char pesHdrBytes )
{
	int i;
	unsigned char *pPayload;

	// point at the payload
	pPayload = pPkt + payloadOffset;
	// adjust pes header offset to be payload relative
	pesHdrOffset -= payloadOffset;
	// shift the payload bytes between the start of the payload and
	// the start of the PES header upward to overwrite the PES header
	for( i=1; i<=pesHdrOffset; ++i )
	{
		pPayload[ pesHdrOffset + pesHdrBytes - i ] = pPayload[ pesHdrOffset - i ];
	}
	// fill the gap at the beginning of the payload with stuffing bytes
	memset( (void *)pPayload, 0xFF, pesHdrBytes );
	// is there already an adaptation field in the header of this packet?
	if( ( pPkt[ 3 ] & 0x20 ) == 0 )
	{
		// no - create a null adaptation field that is just the size of the
		// former PES header in the packet header, back in the output buffer
		pPkt[ 3 ] |= 0x20;
		pPkt[ 4 ] = pesHdrBytes - 1;
		pPkt[ 5 ] = 0;
	}
	else
	{
		// yes - extend it to include the gap
		pPkt[ 4 ] += pesHdrBytes;
	}
}

// perform PES packet header fixups
static RTF_RESULT rtfFltFixPesHdr( RTF_FLT *pFlt, unsigned char pesHdrOffset )
{
	RTF_FNAME( "rtfFltFixPesHdr" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	BOOL modFlag;
	unsigned char *pOutPktStorage;
	unsigned char *pPayload;
	unsigned char tmpBuf[ 2 * TRANSPORT_PACKET_BYTES ];

	do {		 // error escape wrapper - begin

		pOutPktStorage = pFlt->pOutPktStorage;
		// innocent until proven guilty
		modFlag = FALSE;
		// is the entire PES header in the current packet?
		if( ( pesHdrOffset + pFlt->outPktPesHdrBytes ) <= ( pFlt->outPktPayloadOffset + pFlt->outPktPayloadBytes ) )
		{
			// yes - point at the payload in the output buffer
			pPayload = pFlt->pOutPktStorage + pFlt->outPktPayloadOffset;
		}
		else
		{
			// no - is this the last packet of a picture?
			if( pFlt->pktIndex == pFlt->pktCount - 1 )
			{
				// yes - ignore the pes	header. it must belong to the next picture
				// i.e. - this will be a fragmented packet, and will be referenced by
				// the next picture. Which is just as well, since this method needs 2
				// packets to work, and would blow up on the last packet of a picture
				break;
			}
			// copy the payload of this packet to the temp buffer
			memcpy( (void *)tmpBuf, (void *)( pOutPktStorage + pFlt->outPktPayloadOffset ), pFlt->outPktPayloadBytes );
			// append the payload of the next packet
			// Note: don't queue it yet, because that might cause the first packet to
			//       be flushed out of the output buffer, and then it couldn't be fixed
			result = rtfPktCopyPayload( pFlt->phPkt[ pFlt->pktIndex+1 ], tmpBuf + pFlt->outPktPayloadBytes );
			RTF_CHK_RESULT;
			pPayload = tmpBuf;
		}
		// is PES header suppression enabled?
		if( pFlt->trickSpec.suppressInputPesHdr != FALSE )
		{
			// yes - suppress the PES packet header
			rtfFltSuppressPesHdr( pFlt, pFlt->pOutPktStorage, pFlt->outPktPayloadOffset,
								  pesHdrOffset, pFlt->outPktPesHdrBytes );
			modFlag = TRUE;
		}
		else
		{
			// no - perform any fixups required by this pass-thru PES packet header
			result = rtfFltApplyPesHdrFixups( pFlt, pOutPktStorage, &pOutPktStorage[ pesHdrOffset ], &modFlag );
			RTF_CHK_RESULT;
		}
		// yes - was the payload modified?
		if( modFlag != FALSE )
		{
			// yes - bump the modified pes packet header counter
			RTF_INC_STAT( pFlt->pesModCount );
			// was a fragmented payload joined? (i.e. joined packet in tmpBuf?)
			if( pPayload == tmpBuf )
			{
				// yes - copy the modified payload of the first packet
				// back into the image of that packet in the output buffer
				memcpy( (void *)( pFlt->pOutPktStorage + pFlt->outPktPayloadOffset ),
						(void *)tmpBuf, pFlt->outPktPayloadBytes );
				// queue the next video packet to the output buffer
				result = rtfFltQueueVideoPacket( pFlt );
				RTF_CHK_RESULT;
				// copy the modified payload of the second packet
				// onto the image of that packet in the output buffer
				memcpy( (void *)( pFlt->pOutPktStorage + pFlt->outPktPayloadOffset ),
						(void *)tmpBuf, pFlt->outPktPayloadBytes );
			}
		}
		RTF_INC_STAT( pFlt->pesOutCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// perform all payload fixups in order of increasing packet offset
static RTF_RESULT rtfFltDoPayloadFixups( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltDoPayloadFixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int i, nextFixup;
	unsigned char offset;
	unsigned char lastoutPktFixupFlags;

	do {		 // error escape wrapper - begin

		// iterate over the fixups and implement them in packet offset order
		// note: clear the fixup records in the packet descriptor as they are done
		// because the packet may be split as partial last / partial first. Some
		// of the fixups will apply to one image, the rest will apply to the other
		lastoutPktFixupFlags = pFlt->outPktFixupFlags;
		while( pFlt->outPktFixupFlags != 0 )
		{
			// find the remaining fixup with the lowest offset
			nextFixup = -1;
			offset = TRANSPORT_PACKET_BYTES;
			for( i=0; i<8; ++i )
			{
				if( ( pFlt->outPktFixupFlags & ( 1 << i ) ) != 0 )
				{
					if( pFlt->outPktFixupOffset[ i ] < offset )
					{
						offset = pFlt->outPktFixupOffset[ i ];
						nextFixup = i;
					}
				}
			}
			// clear the fixup flag that is about to be serviced
			pFlt->outPktFixupFlags &= ~( 1 << nextFixup );
			lastoutPktFixupFlags = pFlt->outPktFixupFlags;
			// perform the indicated fixup
			switch( nextFixup )
			{
			case RTF_FLT_FIXUP_SEQHDR:
				result = rtfFltFixSeqHdr( pFlt, offset );
				RTF_CHK_RESULT;
				break;
			case RTF_FLT_FIXUP_SQXHDR:
				result = rtfFltFixSqxHdr( pFlt, offset );
				RTF_CHK_RESULT;
				break;
			case RTF_FLT_FIXUP_GOPHDR:
				result = rtfFltFixGopHdr( pFlt, offset );
				RTF_CHK_RESULT;
				break;
			case RTF_FLT_FIXUP_PICHDR:
				result = rtfFltFixPicHdr( pFlt, offset );
				RTF_CHK_RESULT;
				break;
			case RTF_FLT_FIXUP_CODHDR:
				result = rtfFltFixCodHdr( pFlt, offset );
				RTF_CHK_RESULT;
				break;
			case RTF_FLT_FIXUP_PESHDR:
				result = rtfFltFixPesHdr( pFlt, offset );
				RTF_CHK_RESULT;
				break;
			default:
				RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized fixup (%d)", nextFixup );
			}
			RTF_CHK_RESULT_LOOP;
		}	// while

	} while( 0 ); // error escape wrapper - end

	return result;
}

// perform a PID remap operation on the output image of a packet, if necessary
static RTF_RESULT rtfFltRemapOutPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltRemapOutPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// do nothing if PID remapping is not enabled
		if( pFlt->trickSpec.remapPIDs == FALSE )
		{
			break;
		}
		// get the PID from the output image
		pFlt->pOutPktStorage;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// send a packet object to the output; perform fixups there
static RTF_RESULT rtfFltProcessPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltProcessPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPkt;
	unsigned long flags;
	unsigned short pid;
	int index;

	do {		 // error escape wrapper - begin

		// get the packet handle
		hPkt = pFlt->phPkt[ pFlt->pktIndex ];
		// get the flags from this packet
		result = rtfPktGetFlags( hPkt, &flags );
		RTF_CHK_RESULT;
		// does a group start in this packet?
		if( ( flags & RTF_PKT_GOPSTARTPRESENT ) != 0 )
		{
			// yes - record the output packet count at this point
			pFlt->keyStartPktCount = pFlt->pktOutCount;
		}
		// get the PID from the packet
		result = rtfPktGetPID( hPkt, &pid );
		RTF_CHK_RESULT;
		if( pFlt->trickSpec.suppressOTHER == FALSE )
		{
			// is this PID to be excluded?
			rtfFltFindExcPID( pFlt, pid, &index );
			if( index >= 0 )
			{
				// yes
				break;
			}
		}
		else
		{
			// is this PID to be included?
			rtfFltFindIncPID( pFlt, pid, &index );
			if( index < 0 )
			{
				// no
				break;
			}
		}
		// is encryption being ignored?
		if( pFlt->trickSpec.ignoreEncryption == FALSE )
		{
			// no - run the packet about to be output through the CA system for processing
			result = rtfCasProcessPkt( pFlt->hCas, hPkt, pFlt->hOut );
			RTF_CHK_RESULT;
		}
		// Queue the next input packet to the output buffer
		// NOTE: It is important that all fixups be performed in packet offset order,
		//		 as this allows us to deal with constructs that span packet boundaries
		// NOTE: Packet is guaranteed to remain in the output buffer after this call
		result = rtfFltQueuePacket( pFlt );
		RTF_CHK_RESULT;
		// is this a video PID packet?
		if( pid == pFlt->pProfile->videoSpec.pid )
		{
			// yes - is there a payload in this packet?
			if( ( pFlt->outPktFlags & RTF_PKT_PAYLOADABSENT ) == 0 )
			{
				// yes - perform any required fixups on the payload
				result = rtfFltDoPayloadFixups( pFlt );
				RTF_CHK_RESULT;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse the critical field lengths out of a packet
static void rtfGetPacketLengths( unsigned char *pStorage, 
								 unsigned char *pHdrLen, unsigned char *pActLen,
								 unsigned char *pPadLen, unsigned char *pPayLen )
{
	unsigned char adaLen = 0;
	unsigned char actLen = 0;
	unsigned char flags;

	// is there an adaptation field?
	if( ( pStorage[ 3 ] & 0x20 ) != 0 )
	{
		// yes - figure out how many active bytes are in the adaptation field
		adaLen = pStorage[ 4 ];
		actLen = 2;
		flags = pStorage[ 5 ];
		// is the PCR flag set?
		if( ( flags & 0x10 ) != 0 )
		{
			// yes - adjust the active length
			actLen += 6;
		}
		// is the OPCR flag set?
		if( ( flags & 0x08 ) != 0 )
		{
			// yes - adjust the active length
			actLen += 6;
		}
		// is the splicing point flag set?
		if( ( flags & 0x04 ) != 0 )
		{
			// yes - adjust the active length
			actLen += 1;
		}
		// is the private data flag set?
		if( ( flags & 0x02 ) != 0 )
		{
			// yes - adjust the active length
			actLen += pStorage[ actLen + 4 ] + 1;
		}
		// is the extension flag set?
		if( ( flags & 0x01 ) != 0 )
		{
			// yes - adjust the active length
			actLen += pStorage[ actLen + 4 ] + 1;
		}
	}
	// return the field lengths
	*pHdrLen = 4;
	*pActLen = actLen;
	*pPadLen = adaLen - actLen;
	*pPayLen = ( TRANSPORT_PACKET_BYTES - 4 ) + adaLen;
}

// attempt to insert a field into the last output packet
static RTF_RESULT rtfFltLastPacketInsert( RTF_FLT *pFlt, unsigned short insPid,
										  unsigned char insLen, unsigned char *pDat,
										  unsigned short newFlags, BOOL isPayload,
										  unsigned char **ppPkt, unsigned char **ppHdr,
										  BOOL *pSuccess )
{
	RTF_FNAME( "rtfFltLastPacketInsert" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pStorage;
	unsigned char *pSrc;
	unsigned char *pDst;
	unsigned char hdrLen;
	unsigned char actLen;
	unsigned char padLen;
	unsigned char payLen;
	int i;

	do {		 // error escape wrapper - begin

		// guilty until proven innocent
		*pSuccess = FALSE;
		// is the pid of the last output packet the same as the insertion pid?
		if( pFlt->outPID == insPid )
		{
			// yes - get the output buffer fill pointer
			result = rtfBufGetFillPointer( pFlt->hOutBuf, &pStorage );
			RTF_CHK_RESULT;
			// back up to the beginning of the last queued packet
			// note: last queued packet is guaranteed to still be in the output buffer
			pStorage -= TRANSPORT_PACKET_BYTES;
			*ppPkt = pStorage;
			// get the number of adaptation and adaptation stuffing bytes from the packet
			rtfGetPacketLengths( pStorage, &hdrLen, &actLen, &padLen, &payLen );
			// is there enough adaptation stuffing to allow the
			// PES header to be inserted into the previous packet?
			if( padLen >= insLen )
			{
				// is the insertion to be part of the payload?
				if( isPayload != FALSE )
				{
					// yes - insert the new field at the end of the packet,
					// shifting the payload down to use the stuffing space
					pDst = pStorage + hdrLen + actLen;
					*ppHdr = pDst;
					pSrc = pDst + padLen;
					for( i=0; i<payLen; ++i )
					{
						*pDst++ = *pSrc++;
					}
					memcpy( (void *)pSrc, (void *)pDat, insLen );
					// adjust the adaptation field length field
					pStorage[ 4 ] -= insLen;
					// also figure out what new flags, if any, need to be applied
					if( ( newFlags & RTF_PKT_PAYLOADUNITSTART ) != 0 )
					{
						pStorage[ 1 ] |= 0x40;
					}
				}
				else
				{
					// no - figure out where in the adaptation field this thing goes
					// also figure out what new flags, if any, need to be applied
					pDst = pStorage + hdrLen;
					if( ( newFlags & RTF_PKT_PCRPRESENT ) != 0 )
					{
						pDst += 2;
						pStorage[ 5 ] |= 0x10;
					}
					// insert the new data over the adaptation stuffing bytes
					memcpy( (void *)pDst, (void *)pDat, insLen );
				}
				*pSuccess = TRUE;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a PES header
static RTF_RESULT rtfFltInsertPesHeader( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertPesHeader" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pHdr;
	unsigned char *pPesPkt;
	unsigned char *pPesPktField;
	unsigned char hdrLen;
	unsigned char cc;
	BOOL modFlag = FALSE;
	BOOL success = FALSE;

	do {		 // error escape wrapper - begin

		// did the user supply a PES packet header?
		if( pFlt->trickSpec.replacePES != FALSE )
		{
			// yes - point at the user's replacement PES header
			pHdr = pFlt->trickSpec.pesHdr;
			hdrLen = pFlt->trickSpec.pesHdrLen;
		}
		else
		{
			// no - has a PES packet header been captured?
			if( pFlt->pesCaptured != TRUE )
			{
				RTF_LOG_WARN0( RTF_MSG_WRN_PESNOTCAPTURED, "PES header not yet captured" );
				break;
			}
			// point at the captured PES header
			pHdr = pFlt->pesHdr;
			hdrLen = pFlt->pesHdrLen;
		}
		// attempt to insert the PES header into the previous packet
		result = rtfFltLastPacketInsert( pFlt, pFlt->pProfile->videoSpec.pid, hdrLen,
										 pHdr, RTF_PKT_PAYLOADUNITSTART, TRUE, &pPesPkt,
										 &pPesPktField, &success );
		// did the insertion work?
		if( success != FALSE )
		{
			// yes - perform fixups on the inserted PES header
			result = rtfFltApplyPesHdrFixups( pFlt, pPesPkt, pPesPktField, &modFlag );
			RTF_CHK_RESULT;
		}
		else
		{
			// no - update the PES header packet and send it to the output
			result = rtfFltApplyPesHdrFixups( pFlt, pFlt->pesHdrPkt,
						pFlt->pesHdrPkt+TRANSPORT_PACKET_BYTES-pFlt->pesHdrLen, &modFlag );
			RTF_CHK_RESULT;
			// get the next CC value for the output video PID
			result = rtfFltFindNextCC( pFlt, pFlt->pProfile->videoSpec.pid, &cc, 0 );
			RTF_CHK_RESULT;
			// put that CC in the pes header packet
			pFlt->pesHdrPkt[ 3 ] = ( pFlt->pesHdrPkt[ 3 ] & 0xF0 ) | cc;
			// output the PES header packet
			result = rtfFltQueueData( pFlt, pFlt->pesHdrPkt );
			RTF_CHK_RESULT;
		}
		// bump the running PES header generation count
		RTF_INC_STAT( pFlt->pesGenCount );
		RTF_INC_STAT( pFlt->pesOutCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a PAT packet
static RTF_RESULT rtfFltInsertPatPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertPatPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pTable;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// is PAT replacement enabled?
		if( pFlt->trickSpec.replacePAT != FALSE )
		{
			// yes - point at the replacement PAT packet in the filter settings structure
			pTable = pFlt->trickSpec.pat;
			// make sure there really is a packet there
			if( pTable == (unsigned char *)NULL )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Null filterspec PAT pointer" );
				break;
			}
		}
		else
		{
			// no - get a pointer to the recorded PAT packet
			result = rtfPatGetTable( pFlt->hPat, &pTable );
			RTF_CHK_RESULT;
			// make sure there really is a packet there
			if( pTable == (unsigned char *)NULL )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_BADSTATE, "PAT not yet captured" );
				break;
			}
		}
		// get the next continuity counter for this PID
		// note: it should almost never happen that the previous packet was a PAT packet
		// so don't even bother looking.
		result = rtfFltFindNextCC( pFlt, TRANSPORT_PAT_PID, &cc, 0 );
		RTF_CHK_RESULT;
		// install the continuity counter
		pTable[ 3 ] = ( pTable[ 3 ] & 0xF0 ) | ( cc & 0x0F );
		// output the indicated packet data
		result = rtfFltQueueData( pFlt, pTable );
		RTF_CHK_RESULT;
		// bump the running PAT generation count
		RTF_INC_STAT( pFlt->patGenCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a PMT packet
static RTF_RESULT rtfFltInsertPmtPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertPmtPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pTable;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// PMT replacement enabled?
		if( pFlt->trickSpec.replacePMT != FALSE )
		{
			// yes - point at the replacement PMT packet in the filter settings structure
			pTable = pFlt->trickSpec.pmt;
			// make sure there really is a packet there
			if( pTable == (unsigned char *)NULL )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Null filterspec PMT pointer" );
				break;
			}
		}
		else
		{
			// no - get a pointer to the recorded PMT packet
			result = rtfPmtGetPacket( pFlt->hPmt, &pTable );
			RTF_CHK_RESULT;
			// make sure there really is a packet there
			if( pTable == (unsigned char *)NULL )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_BADSTATE, "PMT not yet captured" );
				break;
			}
		}
		// get the next continuity counter for this PID
		result = rtfFltFindNextCC( pFlt, pFlt->pProfile->pmtPID, &cc, 0 );
		RTF_CHK_RESULT;
		// install the continuity counter
		pTable[ 3 ] = ( pTable[ 3 ] & 0xF0 ) | ( cc & 0x0F );
		// output the indicated packet data
		result = rtfFltQueueData( pFlt, pTable );
		RTF_CHK_RESULT;
		// bump the running PMT generation count
		RTF_INC_STAT( pFlt->pmtGenCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a PCR in the output stream
static RTF_RESULT rtfFltInsertPcr( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertPcr" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pPkt = pFlt->tempPkt;
	unsigned char cc;
	unsigned char tmp[ 6 ];
	unsigned char *pPcrPkt;
	unsigned char *pPcrPktField;
	BOOL success;
	UINT64 temp64;
	RTF_TIMESTAMP pcr;
	RTF_TIMESTAMP offset;

	do {		 // error escape wrapper - begin

		// get the current PCR value
		rtfFltComputeOutputPcr( pFlt, &pcr );
		// set the PCR (6 bytes)
		pcr.base.ull <<= 7;
		tmp[ 0 ]  = pcr.base.uc[ 4 ];
		tmp[ 1 ]  = pcr.base.uc[ 3 ];
		tmp[ 2 ]  = pcr.base.uc[ 2 ];
		tmp[ 3 ]  = pcr.base.uc[ 1 ];
		tmp[ 4 ]  = pcr.base.uc[ 0 ];
		tmp[ 4 ] |= pcr.ext.uc[ 1 ] & 0x01;
		tmp[ 5 ]  = pcr.ext.uc[ 0 ];
		// attempt to insert the PCR into the previous packet
		result = rtfFltLastPacketInsert( pFlt, pFlt->pProfile->pcrPID, 6, tmp, RTF_PKT_PCRPRESENT,
										 FALSE, &pPcrPkt, &pPcrPktField, &success );
		RTF_CHK_RESULT;
		// did the insertion work?
		if( success == FALSE )
		{
			// no - get the next CC value for the PCR PID
			result = rtfFltFindNextCC( pFlt, pFlt->pProfile->pcrPID, &cc, RTF_PKT_PAYLOADABSENT );
			RTF_CHK_RESULT;
			// increase the PCR value by one packet
			temp64 = (UINT64)TRANSPORT_PACKET_BITS * TRANSPORT_SCR_TICKS_PER_SECOND;
			temp64 = temp64 / pFlt->targetBitRate;
			// scale by the trick speed if requested
			temp64 = ( pFlt->trickSpec.interpTimeStamps == FALSE ) ? temp64 :
					 pFlt->pProfile->streamPcrBase + ( ( temp64 * pFlt->trickSpec.speedNumerator ) / pFlt->trickSpec.speedDenominator );
			offset.base.ull = temp64 / TRANSPORT_SCR_TO_PCR_RATIO;
			offset.ext.us   = (unsigned short)( temp64 - ( offset.base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) );
			pcr.base.ull >>= 7;
			pcr.base.ull += offset.base.ull;
			pcr.ext.us   += offset.ext.us;
			if( pcr.ext.us >= TRANSPORT_SCR_TO_PCR_RATIO )
			{
				pcr.ext.us -= TRANSPORT_SCR_TO_PCR_RATIO;
				++pcr.base.ull;
			}
			// set the PCR (6 bytes)
			pcr.base.ull <<= 7;
			tmp[ 0 ]  = pcr.base.uc[ 4 ];
			tmp[ 1 ]  = pcr.base.uc[ 3 ];
			tmp[ 2 ]  = pcr.base.uc[ 2 ];
			tmp[ 3 ]  = pcr.base.uc[ 1 ];
			tmp[ 4 ]  = pcr.base.uc[ 0 ];
			tmp[ 4 ] |= pcr.ext.uc[ 1 ] & 0x01;
			tmp[ 5 ]  = pcr.ext.uc[ 0 ];
			// generate a adaptation-only packet with PCR in the temp packet buffer
			memset( (void *)pPkt, 0xFF, TRANSPORT_PACKET_BYTES );
			// header
			pPkt[  0 ] = TRANSPORT_PACKET_SYNCBYTE;
			pPkt[  1 ] = (unsigned char)( ( pFlt->pProfile->pcrPID >> 8 ) & 0xFF );
			pPkt[  2 ] = (unsigned char)( pFlt->pProfile->pcrPID & 0xFF );
			pPkt[  3 ] = 0x20 | cc;						// adaptation field only
			// adaptation field
			pPkt[  4 ] = 183;							// adaptation field length
			pPkt[  5 ] = 0x10;							// flags - PCR present
			// copy in the PCR
			pPkt[  6 ] = tmp[ 0 ];
			pPkt[  7 ] = tmp[ 1 ];
			pPkt[  8 ] = tmp[ 2 ];
			pPkt[  9 ] = tmp[ 3 ];
			pPkt[ 10 ] = tmp[ 4 ];
			pPkt[ 11 ] = tmp[ 5 ];
			// send the packet to the output
			result = rtfFltQueueData( pFlt, pPkt );
			RTF_CHK_RESULT;
			// bump the running packet generation count
			RTF_INC_STAT( pFlt->pktGenCount );
		}
		// !!! TEST HACK !!! PCR LEVEL DCON INSERTION FOR PANASONIC TTS DEMO
		// is PCR level DCON insertion enabled?
		if( pFlt->trickSpec.ttsDconPcrPacket != FALSE )
		{
			// yes - is this a trick file?
			if( pFlt->trickSpec.speedNumerator > 1 )
			{
				// yes - set the DCON flag in the adaptation field
				pFlt->pOutPktStorage[5] |= 0x80;
			}
		}
		// record the PCR packet number
		pFlt->lastPcrPktOutCount = pFlt->pktOutCount;
		// bump the running PCR generation count
		RTF_INC_STAT( pFlt->pcrGenCount );
		// bump the running PCR output count
		RTF_INC_STAT( pFlt->pcrOutCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a video "fluff" packet (zero-length payload, but not flagged as adaptation only)
static RTF_RESULT rtfInsertVideoFluff( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfInsertVideoFluff" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pPkt;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// fill the temp packet with fluff
		memset( (void *)pFlt->tempPkt, 0xFF, sizeof(pFlt->tempPkt) );
		// get the next CC for the video pid
		result = rtfFltFindNextCC( pFlt, pFlt->pProfile->videoSpec.pid, &cc, 0 );
		RTF_CHK_RESULT;
		pPkt = pFlt->tempPkt;
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( pFlt->pProfile->videoSpec.pid >> 8 ) & 0xFF );
		pPkt[ 2 ] = (unsigned char)( pFlt->pProfile->videoSpec.pid & 0xFF );
		pPkt[ 3 ] = 0x30 | (cc & 0x0F);
		pPkt[ 4 ] = 182;
		pPkt[ 5 ] = 0;
		pPkt[ TRANSPORT_PACKET_BYTES-1 ] = 0;
		// send the packet to the output
		result = rtfFltQueueData( pFlt, pPkt );
		RTF_CHK_RESULT;
		// bump the running video fluff generation count
		RTF_INC_STAT( pFlt->vidGenCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// !!! TEST HACK !!! FLUFF PACKET WITH DCON MARKER FOR PANASONIC TTS DEMO !!!
// insert a video "fluff" packet (zero-length payload, but not flagged as adaptation only)
static RTF_RESULT rtfInsertVideoFluffDcon( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfInsertVideoFluffDcon" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pPkt;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// fill the temp packet with fluff
		memset( (void *)pFlt->tempPkt, 0xFF, sizeof(pFlt->tempPkt) );
		// get the next CC for the video pid
		result = rtfFltFindNextCC( pFlt, pFlt->pProfile->videoSpec.pid, &cc, 0 );
		RTF_CHK_RESULT;
		pPkt = pFlt->tempPkt;
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( pFlt->pProfile->videoSpec.pid >> 8 ) & 0xFF );
		pPkt[ 2 ] = (unsigned char)( pFlt->pProfile->videoSpec.pid & 0xFF );
		pPkt[ 3 ] = 0x30 | (cc & 0x0F);
		pPkt[ 4 ] = 182;
		pPkt[ 5 ] = 0x80;
		pPkt[ TRANSPORT_PACKET_BYTES-1 ] = 0;
		// send the packet to the output
		result = rtfFltQueueData( pFlt, pPkt );
		RTF_CHK_RESULT;
		// bump the running video fluff generation count
		RTF_INC_STAT( pFlt->vidGenCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// prepare the no-change video frame packets
static RTF_RESULT rtfFltPrepareNCFrame( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltPrepareNCFrame" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD_HANDLE hVcd;
	BOOL changed;
	unsigned char *pFrame;
	unsigned char *pTgt;
	int i, j, bytes, adaptLen;
	unsigned char payloadOffset;
	unsigned char payloadBytes;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// at this point, the video codec must have been initialized
		// get the handle from the session
		result = rtfSesGetVideoCodec( pFlt->hSes, &hVcd );
		RTF_CHK_RESULT;
		// get the no-change P frame info from the codec
		result = rtfVcdGetNCPFrame( hVcd, &pFrame, &bytes );
		RTF_CHK_RESULT;
		// iterate over the the no-change frame and package it as transport packets
		pFlt->ncFramePacketCount = 0;
		memset( (void *)pFlt->ncFramePackets, 0xFF, sizeof(pFlt->ncFramePackets) );
		for( i=0; i<RTF_MAX_NCFRAME_PACKETS; ++i )
		{
			if( bytes <= 0 )
			{
				break;
			}
			++pFlt->ncFramePacketCount;
			pFlt->ncFramePackets[ i ][ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
			pFlt->ncFramePackets[ i ][ 1 ] = (unsigned char)( ( pFlt->pProfile->videoSpec.pid >> 8 ) & 0xFF );
			pFlt->ncFramePackets[ i ][ 2 ] = (unsigned char)( pFlt->pProfile->videoSpec.pid & 0xFF );
			if( i == 0 )
			{
				pFlt->ncFramePackets[ i ][ 1 ] |= 0x40;
			}
			if( bytes >= RTF_NCF_FIRST_PACKET_BYTES )
			{
				payloadBytes = RTF_NCF_FIRST_PACKET_BYTES;
			}
			else
			{
				payloadBytes = bytes;
			}
			result = rtfFltFindNextCC( pFlt, pFlt->pProfile->videoSpec.pid, &cc, 0 );
			RTF_CHK_RESULT;
			adaptLen = ( TRANSPORT_PACKET_BYTES - 5 ) - payloadBytes;
			pFlt->ncFramePackets[ i ][ 3 ] = 0x30 | ( cc & 0x0F );
			pFlt->ncFramePackets[ i ][ 4 ] = adaptLen;
			pFlt->ncFramePackets[ i ][ 5 ] = 0;
			payloadOffset = 5 + adaptLen;
			memcpy( (void *)&(pFlt->ncFramePackets[ i ][ payloadOffset ]), (void *)pFrame, payloadBytes );
			// is this an MPEG-2 stream?
			if( pFlt->pProfile->videoSpec.eStream.video.vcdType == RTF_VIDEO_CODEC_TYPE_MPEG2 )
			{
				// scan the packet payload
				for( j=0; j<payloadBytes-4; ++j )
				{
					pTgt = &( pFlt->ncFramePackets[ i ][ payloadOffset + j ] );
					// is this a PES packet header?
					if( ( pTgt[ 0 ] == 0x00 ) &&
						( pTgt[ 1 ] == 0x00 ) &&
						( pTgt[ 2 ] == 0x01 ) &&
						( ( pTgt[ 3 ] & 0xF0 ) == 0xE0 ) )
					{
						// yes - apply PES header fixups
						result = rtfFltApplyPesHdrFixups( pFlt, pFlt->ncFramePackets[ i ], pTgt, &changed );
						RTF_CHK_RESULT;
					}
					// is this an MPEG-2 picture header?
					if( ( pFlt->pProfile->videoSpec.eStream.video.vcdType == RTF_VIDEO_CODEC_TYPE_MPEG2 ) &&
						( pTgt[ 0 ] == 0x00 ) &&
						( pTgt[ 1 ] == 0x00 ) &&
						( pTgt[ 2 ] == 0x01 ) &&
						( pTgt[ 3 ] == 0x00 ) )
					{
						// yes - make sure the temporal ref and
						// the vbv delay are also in this payload
						if( ( payloadBytes - j ) < 7 )
						{
							RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Split NCF picture header" );
							break;
						}
						// record a pointer to the start of the picture header
						pFlt->pNCFramePicHdr = pTgt;
						// make the VBV delay field = 0xFFFF
						pTgt[ 5 ] |= 0x07;
						pTgt[ 6 ] |= 0xFF;
						pTgt[ 7 ] |= 0xF8;
					}
				}
				RTF_CHK_RESULT_LOOP;
			}
			bytes  -= payloadBytes;
			pFrame += payloadBytes;
		} // for( i=0; i<RTF_MAX_NCFRAME_PACKETS; ++i )
		RTF_CHK_RESULT_LOOP;
		// were all of the bytes copied?
		if( bytes > 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No change frame too large" );
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a no-change frame
static RTF_RESULT rtfFltInsertNCFrame( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertNCFrame" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned long packets;
	unsigned long i;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// preset packet count to zero, in case NCFrame insertion is disabled
		pFlt->ncAccessUnitOutPackets = 0;
		// is no-change frame insertion enabled?
		if( pFlt->trickSpec.insertNCF != FALSE )
		{
			// yes - update the NCFrame, if required
			result = rtfFltPrepareNCFrame( pFlt );
			RTF_CHK_RESULT;
			// is this MPEG-2?
			if( pFlt->pProfile->videoSpec.eStream.video.vcdType == RTF_VIDEO_CODEC_TYPE_MPEG2 )
			{
				// yes - adjust the temporal reference field of the no-change frame
				// (number of frames in current output group)
				pFlt->pNCFramePicHdr[ 4 ]  = (unsigned char)( ( pFlt->ncFrameTemporalRef>>2 ) & 0xFF );
				pFlt->pNCFramePicHdr[ 5 ] &= 0x3F;
				pFlt->pNCFramePicHdr[ 5 ] |= (unsigned char)( ( pFlt->ncFrameTemporalRef<<6 ) & 0xFF );
				// bump the temporal reference
				++pFlt->ncFrameTemporalRef;
			}
			// is sequential CC correction enabled?
			if( pFlt->trickSpec.sequentialCC != FALSE )
			{
				// yes - reset the video PID CC
				result = rtfFltResetNextCC( pFlt, pFlt->pProfile->videoSpec.pid );
				RTF_CHK_RESULT;
			}
			// perform fixups on the packets of the NC frame
			for( i=0; i<pFlt->ncFramePacketCount; ++i )
			{
				// fix the CC in the next packet of the no-change frame
				result = rtfFltFindNextCC( pFlt, pFlt->pProfile->videoSpec.pid, &cc, 0 );
				RTF_CHK_RESULT;
				pFlt->ncFramePackets[ i ][ 3 ] &= 0xF0;
				pFlt->ncFramePackets[ i ][ 3 ] |= ( cc & 0x0F );
				// send it to the output
				result = rtfFltQueueData( pFlt, pFlt->ncFramePackets[ i ] );
				RTF_CHK_RESULT;
			}
			RTF_CHK_RESULT_LOOP;
			pFlt->ncAccessUnitOutPackets = pFlt->ncFramePacketCount;
			// is sequential CC correction enabled?
			if( pFlt->trickSpec.sequentialCC != FALSE )
			{
				// yes - insert enough "fluff" packets to bring the
				// video CC value around back to 0x0F
				while( ((i++)&0x0F) != 0 )
				{
					result = rtfInsertVideoFluff( pFlt );
					RTF_CHK_RESULT;
					++pFlt->ncAccessUnitOutPackets;
				}
				RTF_CHK_RESULT_LOOP;
			}
			// is forced padding enabled?
			if( pFlt->trickSpec.forcePadding != FALSE )
			{
				// yes - add in the required percentage of null packets (round up)
				packets = ( ( i * pFlt->trickSpec.forcePaddingFactorFix8 ) + 255 ) >> 8;
				for( i=0; i<packets; ++i )
				{
					result = rtfFltInsertStuffingPacket( pFlt );
					RTF_CHK_RESULT;
				}
				RTF_CHK_RESULT_LOOP;
			}
			// bump the no-change frame generation count
			RTF_INC_STAT( pFlt->ncfGenCount );
		} // if( pFlt->trickSpec.insertNCF != FALSE )
		// add this no-change frame to the bitrate control queue
		result = rtfFltUpdateBitrate( pFlt, FALSE );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfFltFixPktArray( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltFixPktArray" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// !!! TEST HACK !!! ACCESS UNIT LEVEL DCON INSERTION FOR PANASONIC TTS DEMO !!!
		// is this the first packet?
		// if( pFlt->pktOutCount == 0 )
		// is access unit DCON insertion enabled?
		int first = 0;
		if( pFlt->trickSpec.ttsDconAccessUnit != FALSE )
		{
			// yes - is this a trick file?
			if( pFlt->trickSpec.speedNumerator > 1 )
			{
				// yes - insert a fluff packet with a discontinuity marker
				result = rtfInsertVideoFluffDcon( pFlt );
				RTF_CHK_RESULT;
				first = 1;
			}
		}
		// is this the first packet?
		if( pFlt->pktOutCount == first )
		// !!! END TEST HACK !!!
		{
			// is prefixPSI turned on?
			if( pFlt->trickSpec.prefixPSI != FALSE )
			{
				// yes - insert PAT and PMT packets
				result = rtfFltInsertPatPacket( pFlt );
				RTF_CHK_RESULT;
				result = rtfFltInsertPmtPacket( pFlt );
				RTF_CHK_RESULT;
			}
		}
		// is sequential CC fixup enabled?
		if( pFlt->trickSpec.sequentialCC != FALSE )
		{
			// yes - is this a trick file?
			if( pFlt->mainFileCopy == FALSE )
			{
				// yes - set the next video PID CC to zero
				result = rtfFltResetNextCC( pFlt, pFlt->pProfile->videoSpec.pid );
				RTF_CHK_RESULT;
			}
		} // if( pFlt->trickSpec.sequentialCC != FALSE )
		// is PES header insertion enabled?
		if( pFlt->trickSpec.insertPES != FALSE )
		{
			result = rtfFltInsertPesHeader( pFlt );
			RTF_CHK_RESULT;
		}
		// copy the packets of the picture to the output buffer
		// perform any necessary fixups there
		for( pFlt->pktIndex=0; pFlt->pktIndex<pFlt->pktCount; ++pFlt->pktIndex)
		{
			// process the next packet in the array
			result = rtfFltProcessPacket( pFlt );
			RTF_CHK_RESULT;
			// is PCR insertion turned on?
			if( pFlt->trickSpec.insertPCR != FALSE )
			{
				// yes - is it time to insert a PCR?
				if( ( pFlt->pktOutCount - pFlt->lastPcrPktOutCount ) >= pFlt->packetsPerPcr )
				{
					// yes - insert a PCR
					result = rtfFltInsertPcr( pFlt );
					RTF_CHK_RESULT;
				}
			}
		}
		RTF_CHK_RESULT_LOOP;
		// is sequential CC fixup enabled?
		if( pFlt->trickSpec.sequentialCC != FALSE )
		{
			// yes - is this a trick file?
			if( pFlt->mainFileCopy == FALSE )
			{
				// get the next video CC (don't increment)
				result = rtfFltFindNextCC( pFlt, pFlt->pProfile->videoSpec.pid, &cc, RTF_PKT_PAYLOADABSENT );
				RTF_CHK_RESULT;
				while( ( (cc++) & 0x0F ) != 0x0F )
				{
					result = rtfInsertVideoFluff( pFlt );
					RTF_CHK_RESULT;
				}
				RTF_CHK_RESULT_LOOP;
			} // if( pFlt->mainFileCopy == FALSE )
		} // if( pFlt->trickSpec.sequentialCC != FALSE )
		// update the running input packet count
		RTF_ADD_STAT( pFlt->pktInCount, pFlt->pktCount );
		// is PAT insertion enabled?
		if( pFlt->trickSpec.insertPAT != FALSE )
		{
			// insert a PAT packet
			result = rtfFltInsertPatPacket( pFlt );
			RTF_CHK_RESULT;
		}
		// is PMT insertion enabled?
		if( pFlt->trickSpec.insertPMT != FALSE )
		{
			// insert a PMT packet
			result = rtfFltInsertPmtPacket( pFlt );
			RTF_CHK_RESULT;
		}
		// bump the running output access unit count
		++pFlt->accOutCount;
		// reset the temporal reference for any generated no-change frames that may follow
		pFlt->ncFrameTemporalRef = 1;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// wrap an access unit around a picture - send it to the output
static RTF_RESULT rtfFltGenerateAccessUnit( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltGenerateAccessUnit" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned long firstPktCount;
	int picPkts;
	int nulPkts;
	int copies;
	int bits;
	int i, j;

	do {		 // error escape wrapper - begin

#if GENERATE_TRACE_FILES
		{
			unsigned long number;
			char buf[ 20 ];
			rtfPicGetNumber( pFlt->hPic, &number );
			sprintf( buf, "%d\n", ( number - pFlt->lastPictureNumber ) );
			pFlt->lastPictureNumber = number;
			_write( pFlt->traceFile, buf, strlen( buf ) );
		}
#endif

		// record the packet count at the start of this access unit
		pFlt->accStartPktOutCount = pFlt->pktOutCount;
		// get the packet array info from the picture
		result = rtfPicGetPacketArrayInfo( pFlt->hPic, &pFlt->pktCount, &pFlt->phPkt,
								&pFlt->firstBytePktOffset, &pFlt->lastBytePktOffset,
								&pFlt->lastVideoPacketIndex );
		RTF_CHK_RESULT;
		// compute the target packet and frame counts for this access unit
		result = rtfFltCalculateTargetCounts( pFlt );
		RTF_CHK_RESULT;
		// record the output packet count at the start of this interval
		firstPktCount = pFlt->pktOutCount;
		// perform fixups on this picture's packet array
		result = rtfFltFixPktArray( pFlt );
		RTF_CHK_RESULT;
		// record the "naked" keyframe size (no bitrate stuffing added)
		pFlt->queueNextKeyfrBits = pFlt->queueNextFrameBits;
		// should copies of the I-frame be substituted for null padding?
		if( pFlt->trickSpec.dittoFrames != FALSE )
		{
			// yes - get the number of packets occuppied by the picture without any stuffing
			picPkts = (int)( pFlt->queueNextFrameBits / TRANSPORT_PACKET_BITS );
			// watch out - actual picture can be larger because of fixups
			picPkts += RTF_MAX_FIXUP_ADDED_PKTS;
			// calculate the number of copies of this I-frame that would fit
			// in the available bits (note: count includes original copy)
			copies = pFlt->accTargetPacketCount / picPkts;
			// note: do not exceed the picture count target
			copies = MIN( copies, (int)pFlt->accTargetPictureCount );
			// any additional copies to be made?
			if( copies > 1 )
			{
				// distribute any "leftover" packet counts among the copies
				nulPkts = ( pFlt->accTargetPacketCount / copies ) - picPkts;
				// fill the gap with copies of the last I-frame
				for( i=1; i<copies; ++i )
				{
					// insert a set of null packets to go with the previous picture
					for( j=0; j<nulPkts; ++j)
					{
						result = rtfFltInsertStuffingPacket( pFlt );
						RTF_CHK_RESULT;
					}
					RTF_CHK_RESULT_LOOP;
					// for bit rate accounting purposes, we want to look at the stuffing packets that
					// were just added as part of the picture data rather than post-picture stuffing
					bits = nulPkts * TRANSPORT_PACKET_BITS;
					pFlt->queueNextStuffBits -= bits;
					pFlt->queueNextFrameBits += bits;
					// register the previous picture with the indexer
					result = rtfIdxRecordKeyframe( pFlt->hIdx, pFlt->filterNumber, firstPktCount, pFlt->pktOutCount );
					RTF_CHK_RESULT;
					// record the opening packet count for the next copy of the picture
					firstPktCount = pFlt->pktOutCount;
					// create then next image of the picture in the output
					result = rtfFltFixPktArray( pFlt );
					RTF_CHK_RESULT;
				}
				RTF_CHK_RESULT_LOOP;
			} // if( copies > 1 )
		} // if( pFlt->trickSpec.dittoFrames != FALSE )
		// update the bitrate control queue for the index interval
		result = rtfFltUpdateBitrate( pFlt, TRUE );
		RTF_CHK_RESULT;
		// register this final picture with the indexer
		result = rtfIdxRecordKeyframe( pFlt->hIdx, pFlt->filterNumber, firstPktCount, pFlt->pktOutCount );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// filter a keyframe
static RTF_RESULT rtfFltProcessKeyframe( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltProcessKeyframe" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned long keyRate, ncfRate;
	int keyDiff, ncfDiff;

	do {		 // error escape wrapper - begin

		// always reset the access unit packet count before processing a keyframe
		pFlt->accessUnitOutPackets = 0;
		// get the next output bit rate for either a keyframe or an ncframe
		rtfFltGetNextOutputBitRates( pFlt, &keyRate, &ncfRate );
		keyDiff = (int)keyRate - (int)pFlt->targetBitRate;
		ncfDiff = (int)ncfRate - (int)pFlt->targetBitRate;
		keyDiff = ABS( keyDiff );
		ncfDiff = ABS( ncfDiff );
		// is this the first picture for this filter OR
		// was the last index interval keyframe limited OR
		// does adding a keyframe bring us closer to the target bitrate than a no-change frame? 
		if( ( pFlt->picInCount == 0 ) ||
			( keyDiff <= ncfDiff ) )
		{
			// yes - wrap an access unit around this keyframe and send it to the output
			result = rtfFltGenerateAccessUnit( pFlt );
			RTF_CHK_RESULT;
			RTF_LOG_PIC( "I" );
		}
		else
		{
			// no - insert a no-change frame in place of the keyframe
			result = rtfFltInsertNCFrame( pFlt );
			RTF_CHK_RESULT;
			RTF_LOG_PIC( "r" );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// filter a non-keyframe - output any ECM packets found there
static RTF_RESULT rtfFltProcessNonKeyframe( RTF_FLT *pFlt, RTF_PIC_HANDLE hPic )
{
	RTF_FNAME( "rtfFltProcessNonKeyframe" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE *phPacket;
	unsigned short packetCount;
	unsigned short pid;
	unsigned short lastVideoPacketIndex;
	unsigned char firstBytePacketOffset;
	unsigned char lastBytePacketOffset;

	do {		 // error escape wrapper - begin

		// do nothing if the CAT is not valid
		if( pFlt->catValid == FALSE )
		{
			break;
		}
		// get the packet array info from the picture
		result = rtfPicGetPacketArrayInfo( hPic, &packetCount, &phPacket, &firstBytePacketOffset,
												 &lastBytePacketOffset, &lastVideoPacketIndex );
		// loop over the packets in the picture
		for( pFlt->pktIndex=0; pFlt->pktIndex<packetCount; ++pFlt->pktIndex )
		{
			// get this packet's PID
			result = rtfPktGetPID( phPacket[ pFlt->pktIndex ], &pid );
			RTF_CHK_RESULT;
			// is this the ECM PID?
			if( pid == pFlt->catEcmPid )
			{
				// yes - queue this packet to the output
				result = rtfFltQueuePacket( pFlt );
				RTF_CHK_RESULT;
			}
		}
		RTF_CHK_RESULT_LOOP;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record the input frame rate code
static RTF_RESULT rtfFltSetInputFrameRateCode( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltSetInputFrameRateCode" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// record the input frame rate in 16 bit fixed point
		// also record the number of PCR clock ticks for one frame time
		switch( pFlt->pProfile->videoSpec.eStream.video.frameRateCode )
		{
		case 1:			// 23.976 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 23.976 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 23.976 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 23.976 ) + 0.5 );
			break;
		case 2:			// 24.000 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 24.000 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 24.000 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 24.000 ) + 0.5 );
			break;
		case 3:			// 25.000 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 25.000 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 25.000 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 25.000 ) + 0.5 );
			break;
		case 4:			// 29.970 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 29.970 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 29.970 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 29.970 ) + 0.5 );
			break;
		case 5:			// 30.000 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 30.000 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 30.000 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 30.000 ) + 0.5 );
			break;
		case 6:			// 50.000 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 50.000 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 50.000 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 50.000 ) + 0.5 );
			break;
		case 7:			// 59.940 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 59.940 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 59.940 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 59.940 ) + 0.5 );
			break;
		case 8:			// 60.000 fps
			pFlt->secsPerFrameFix16 = (unsigned long)( ( 65536.0 / 60.000 ) + 0.5 );
			pFlt->framesPerSecFix16 = (unsigned long)( ( 60.000 * 65536.0 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 60.000 ) + 0.5 );
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Invalid frame rate code %d",
						  pFlt->pProfile->videoSpec.eStream.video.frameRateCode );
		}
		RTF_CHK_RESULT_LOOP;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set up the input bit rate info
static RTF_RESULT rtfFltSetInputBitRate( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltSetInputBitRate" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	UINT64 temp64;

	do {		 // error escape wrapper - begin

		// set the target output rate
		if( pFlt->trickSpec.userBitRate != FALSE )
		{
			pFlt->targetBitRate = pFlt->trickSpec.userBitsPerSecond;
		}
		else
		{
			pFlt->targetBitRate = pFlt->pProfile->bitsPerSecond;
		}
		// is TTS output enabled?
		if( ( pFlt->pProfile->flags & RTF_PROFILE_TTS_MASK ) != 0 )
		{
			// yes - bump up the output bitrate to account for the additional headers
			temp64 = pFlt->targetBitRate;
			temp64 *= TTS_PACKET_BYTES;
			temp64 *= INV_TRANSPORT_PACKET_BYTES_FIX16;
			pFlt->targetBitRate = (unsigned long)( ( temp64 + 0x8000 ) >> 16 );
		}
		// is forced padding enabled?
		if( pFlt->trickSpec.forcePadding != FALSE )
		{
			// yes - re-calculate the null packet bit rate
			temp64 = pFlt->targetBitRate;
			temp64 *= pFlt->trickSpec.forcePaddingFactorFix8;
			pFlt->forcePadBitRate = (unsigned long)( ( temp64 + 0x80 ) >> 8 );
		}
		// re-initialize the bitrate queue
		result = rtfInitializeBitrateQueue( pFlt );
		RTF_CHK_RESULT;
		// is PCR generation enabled?
		if( pFlt->trickSpec.insertPCR != FALSE )
		{
			// transport trickSpec says PCRs per second must be at least 10
			if( pFlt->trickSpec.PCRsPerSecond < 10 )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Invalid PCRs per second (%d)", pFlt->trickSpec.PCRsPerSecond );
				break;
			}
			// compute the number of packets between PCRs
			if( pFlt->trickSpec.userBitRate != FALSE )
			{
				pFlt->packetsPerPcr = pFlt->trickSpec.userBitsPerSecond;
			}
			else
			{
				result = rtfSesGetInputBitrate( pFlt->hSes, &pFlt->packetsPerPcr );
				RTF_CHK_RESULT;
			}
			pFlt->packetsPerPcr /= (TRANSPORT_PACKET_BITS * pFlt->trickSpec.PCRsPerSecond );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set up the PES header info
static RTF_RESULT rtfFltSetPES( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltSetPes" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_PES_HANDLE hPes;
	unsigned char *pPkt = pFlt->pesHdrPkt;
	unsigned char *pHdrSrc;
	unsigned char *pHdrDst;
	unsigned char hdrLength;

	do {		 // error escape wrapper - begin

		// get the PES handle from the session
		result = rtfSesGetPes( pFlt->hSes, &hPes );
		// get the PES header
		result = rtfPesGetHeader( hPes, &pHdrSrc, &hdrLength );
		RTF_CHK_RESULT;
		// make sure that the byte count is legitimate
		if( hdrLength >= TRANSPORT_PACKET_BYTES - TRANSPORT_MIN_ADAPT_BYTES )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Captured PES header exceeds 1-pkt max (%d)", hdrLength );
			break;
		}
		// get the delays from the PES header
		result = rtfPesGetDelays( hPes, &pFlt->decodingDelay, &pFlt->presentationDelay );
		RTF_CHK_RESULT;
		// record the PES header
		memcpy( (void *)pFlt->pesHdr, pHdrSrc, hdrLength );
		// record the length
		pFlt->pesHdrLen = hdrLength;
		// for efficiency, set up the PES header as the sole payload
		// of a packet. Create a null adaptation field with enough
		// stuffing to fill the packet.
		// set the payload unit start flag
		pPkt = pFlt->pesHdrPkt;
		memset( (void *)pPkt, 0xFF, TRANSPORT_PACKET_BYTES );
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( pFlt->pProfile->videoSpec.pid >> 8 ) & 0xFF ) | 0x40;
		pPkt[ 2 ] = (unsigned char)( pFlt->pProfile->videoSpec.pid & 0xFF );
		pPkt[ 3 ] = 0x30;
		if( TRANSPORT_PACKET_BYTES == ( hdrLength + 5 ) )
		{
			pPkt[ 4 ] = 0x00;
		}
		else
		{
			pPkt[ 4 ] = TRANSPORT_PACKET_BYTES - ( hdrLength + 5 );
			pPkt[ 5 ] = 0x00;
		}
		// copy the PES packet header into the end of the transport packet
		pHdrDst = pPkt + ( TRANSPORT_PACKET_BYTES - hdrLength );
		memcpy( (void *)pHdrDst, (void *)pHdrSrc, hdrLength );
		// Note: CC and other fixups will be applied prior to each use
		pFlt->pesCaptured = TRUE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfFltGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfFltGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_FLT);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfFltConstructor( RTF_FLT_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfFltConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT *pFlt;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the filter object
		pFlt = (RTF_FLT *)rtfAlloc( sizeof(RTF_FLT) );
		RTF_CHK_ALLOC( pFlt );
		// return the handle
		*pHandle = (RTF_FLT_HANDLE)pFlt;
		// clear the state structure
		memset( (void *)pFlt, 0, sizeof(*pFlt) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_FLT, (RTF_HANDLE)pFlt, hParent, &pFlt->hBaseObject );
		RTF_CHK_RESULT;
		// reset the filter (note: leaves session handle alone)
		rtfResetFlt( pFlt );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfFltDestructor( RTF_FLT_HANDLE handle )
{
	RTF_FNAME( "rtfFltDestructor" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded base object
		result = rtfObjDestructor( pFlt->hBaseObject, RTF_OBJ_TYPE_FLT );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the output packet count from the filter
RTF_RESULT rtfFltGetOutPktCount( RTF_FLT_HANDLE handle, unsigned long *pOutPktCount )
{
	RTF_FNAME( "rtfFltGetOutPktCount" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_NE( pFlt, RTF_FLT_STATE_CLOSED );
		// make the return
		*pOutPktCount = pFlt->pktOutCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the output packet count at the start of the last keyframe from the filter
RTF_RESULT rtfFltGetKeyStartPktCount( RTF_FLT_HANDLE handle, unsigned long *pKeyStartPktCount )
{
	RTF_FNAME( "rtfFltGetKeyStartPktCount" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_NE( pFlt, RTF_FLT_STATE_CLOSED );
		// make the return
		*pKeyStartPktCount = pFlt->keyStartPktCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the speed info from the filter
RTF_RESULT rtfFltGetSpeedInfo( RTF_FLT_HANDLE handle, int *pDirection, unsigned long *pNumerator,
							   unsigned long *pDenominator, char **ppExtension )
{
	RTF_FNAME( "rtfFltGetSpeedInfo" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_NE( pFlt, RTF_FLT_STATE_CLOSED );
		// make the returns
		*pDirection   = pFlt->trickSpec.speedDirection;
		*pNumerator   = pFlt->trickSpec.speedNumerator;
		*pDenominator = pFlt->trickSpec.speedDenominator;
		*ppExtension  = pFlt->trickSpec.fileExtension;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset the filter to a closed, empty state
RTF_RESULT rtfFltReset( RTF_FLT_HANDLE handle )
{
	RTF_FNAME( "rtfFltReset" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		rtfResetFlt( pFlt );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// open the filter
RTF_RESULT rtfFltOpen( RTF_FLT_HANDLE handle, int filterNumber, RTF_OUT_HANDLE hOut,
					   RTF_TRICK_SPEC *pSpec )
{
	RTF_FNAME( "rtfFltOpen" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;
	UINT64 temp64;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_EQ( pFlt, RTF_FLT_STATE_CLOSED );
		// reset the filter
		rtfResetFlt( pFlt );
		// make sure this isn't an index file
		if( pSpec->speedNumerator == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Trick speed numerator is zero" );
			break;
		}
		// make sure the denominator is non-zero
		if( pSpec->speedDenominator == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Trick speed denominator is zero" );
			break;
		}
		// record the trickfile direction
		pFlt->speedDirection = pSpec->speedDirection;
		// record inverse trick speed in fixed point
		pFlt->invSpeedRatioFix16 = ( pSpec->speedDenominator << 16 ) / pSpec->speedNumerator;
		// set a flag if this filter is generating a copy of the main file
		pFlt->mainFileCopy = ( ( pSpec->speedNumerator   == 1 ) &&
							   ( pSpec->speedDenominator == 1 ) &&
							   ( pSpec->speedDirection   == 1 ) ) ? TRUE : FALSE;
		// compute the fixed-point speed ratio (saves divides later)
		pFlt->speedRatioFix16 = ( ( ( (unsigned long)pSpec->speedNumerator ) << 16 ) + ( pSpec->speedDenominator>>1 ) ) / pSpec->speedDenominator;
		// record the filter number
		pFlt->filterNumber = filterNumber;
		// record the handle of the owning session
		result = rtfObjGetSession( (RTF_HANDLE)pFlt, &pFlt->hSes );
		RTF_CHK_RESULT;
		// record handle of the output assigned to this filter
		pFlt->hOut = hOut;
		// get the handle of the output buffer for that output
		result = rtfOutGetBuffer( hOut, &pFlt->hOutBuf );
		RTF_CHK_RESULT;
		// record the indexer handle
		result = rtfSesGetIndexer( pFlt->hSes, &pFlt->hIdx );
		RTF_CHK_RESULT;
		// record the conditional access system handle
		result = rtfSesGetCASystem( pFlt->hSes, &pFlt->hCas );
		RTF_CHK_RESULT;
		// make a local copy of the trickSpec
		memcpy( (void *)&pFlt->trickSpec, (void *)pSpec, sizeof(RTF_TRICK_SPEC) );
		// reset the record of the last pid
		pFlt->outPID = TRANSPORT_INVALID_PID;
		// set a default output bit rate
		pFlt->targetBitRate = ( pFlt->trickSpec.userBitRate != FALSE ) ?
				pFlt->trickSpec.userBitsPerSecond : RTF_DEFAULT_BITS_PER_SECOND;
		// compute the rate at which to add null packets
		pFlt->forcePadBitRate = 0;
		if( pFlt->trickSpec.forcePadding != FALSE )
		{
			temp64 = pFlt->targetBitRate;
			temp64 *= pFlt->trickSpec.forcePaddingFactorFix8;
			pFlt->forcePadBitRate = (unsigned long)( ( temp64 + 255 ) >> 8 );
		}
		// change the state to open
		pFlt->state = RTF_FLT_STATE_OPEN;
#if GENERATE_TRACE_FILES
		{
			char fname[ 20 ];
			sprintf( fname, "trace_%d.txt", pFlt->filterNumber );
			pFlt->traceFile = _open( fname, O_WRONLY | O_BINARY | O_SEQUENTIAL | O_CREAT, S_IREAD | S_IWRITE );
			pFlt->lastPictureNumber = 0;
		}
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// prepare the filter for streaming
RTF_RESULT rtfFltPrepareForStreaming( RTF_FLT_HANDLE handle )
{
	RTF_FNAME( "rtfFltPrepareForStreaming" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;
	BOOL isValid;
	unsigned short descriptorSid;
	unsigned char descriptorTag;
	unsigned char descriptorDatLen;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_EQ( pFlt, RTF_FLT_STATE_OPEN );
		// get the PSI handles from the session object
		result = rtfSesGetPSI( pFlt->hSes, &pFlt->hPat, &pFlt->hCat, &pFlt->hPmt );
		RTF_CHK_RESULT;
		// compose a list of PIDs to be included in the trick files
		result = rtfFltSetPidLists( pFlt );
		RTF_CHK_RESULT;
		// get the stream profile from the session object
		result = rtfSesGetStreamProfile( pFlt->hSes, &pFlt->pProfile );
		RTF_CHK_RESULT;
		// is a video PID defined?
		if( pFlt->pProfile->videoSpec.pid != TRANSPORT_INVALID_PID )
		{
			// yes - set up the PES header info
			result = rtfFltSetPES( pFlt );
			RTF_CHK_RESULT;
			// set up the frame rate info
			result = rtfFltSetInputFrameRateCode( pFlt );
			RTF_CHK_RESULT;
			// prepare the no change frame packets
			result = rtfFltPrepareNCFrame( pFlt );
			RTF_CHK_RESULT;
		}
		// set up the input bit rate info
		result = rtfFltSetInputBitRate( pFlt );
		RTF_CHK_RESULT;
		// is the conditional access table valid?
		result = rtfCatValidate( pFlt->hCat, &isValid );
		RTF_CHK_RESULT;
		if( isValid != FALSE )
		{
			pFlt->catValid = TRUE;
			// get the active descriptor info
			result = rtfCatGetActiveDescriptorInfo( pFlt->hCat, &descriptorTag, &descriptorSid,
													&pFlt->catEcmPid, &descriptorDatLen );
		}
		// change state to ready
		pFlt->state = RTF_FLT_STATE_READY;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// filter an input buffer to the attached output
// NOTE: only called for optional filtered copy of main file
RTF_RESULT rtfFltBuf( RTF_FLT_HANDLE handle, RTF_BUF_HANDLE hBuf )
{
	RTF_FNAME( "rtfFltBuf" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_EQ( pFlt, RTF_FLT_STATE_READY );
		// get the list of packets in this input buffer
		result = rtfBufGetPacketArrayInfo( hBuf, &pFlt->pktCount, &pFlt->phPkt );
		RTF_CHK_RESULT;
		result = rtfFltFixPktArray( pFlt );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// filter a group of pictures to the attached output
// NOTE: only called for trick file outputs
RTF_RESULT rtfFltGop( RTF_FLT_HANDLE handle, RTF_GOP_HANDLE hGop )
{
	RTF_FNAME( "rtfFltGop" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PIC_HANDLE *phPic;
	INT64 lastPTS;
	INT64 tempPTS;
	BOOL isKeyframe;
	unsigned long firstPktNum;
	unsigned long lastPktNum;
	int i, outPointCounter;
	int distThis, distNext;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_EQ( pFlt, RTF_FLT_STATE_READY );
		// bump the group input count
		RTF_INC_STAT( pFlt->gopInCount );
		// get the picture array info from this group object
		result = rtfGopGetPicArrayInfo( hGop, &pFlt->gopPicCount, &phPic );
		RTF_CHK_RESULT;
		// Scan the pictures of the group for out points before processing keyframes.
		// This is so that outpoint info will be available when the index entry is written.
		// NOTE: only do this once per GOP - use filter 1 as the vehicle
		if( pFlt->filterNumber == 1 )
		{
			// get the PTS of the keyframe of this group
			result = rtfPicGetSrcPTS( phPic[ 0 ], &lastPTS );
			RTF_CHK_RESULT;
			// search the rest of the group for pictures whose PTS is later than any preceding
			outPointCounter = 0;
			for( i=1; i<pFlt->gopPicCount; ++i )
			{
				// get the PTS of the next picture
				result = rtfPicGetSrcPTS( phPic[ i ], &tempPTS );
				RTF_CHK_RESULT;
				// is the end of this picture a potential out point?
				if( tempPTS > lastPTS )
				{
					// yes - get the first packet number of this picture
					result = rtfPicGetFirstPktNum( phPic[ i ], &firstPktNum );
					RTF_CHK_RESULT;
					// only record every second potential outpoint
					if( ( ++outPointCounter & 0x01 ) == 0 )
					{
						// inform the indexer of the out point
						result = rtfIdxRecordOutpoint( pFlt->hIdx, firstPktNum );
						RTF_CHK_RESULT;
					}
					// update the last PTS record
					lastPTS = tempPTS;
				}
			}
			RTF_CHK_RESULT_LOOP;
		}
		// get the keyframe flag from the first picture of this group
		result = rtfPicGetIsKeyframe( phPic[ 0 ], &isKeyframe );
		RTF_CHK_RESULT;
		// get the number of the last packet in this group
		result = rtfGopGetLastPktNum( hGop, &lastPktNum );
		RTF_CHK_RESULT;
		pFlt->queueNextInputBits += ( ( lastPktNum - pFlt->queueLastGroupPktNum ) * TRANSPORT_PACKET_BITS );
		pFlt->queueLastGroupPktNum = lastPktNum;
		// see if the target frame is closer to the beginning
		// of the next group than the beginning of this group
		distThis = (int)pFlt->tgtInFrameNumber - (int)pFlt->picInCount;
		distNext = (int)( pFlt->picInCount + pFlt->gopPicCount ) -
				   (int)pFlt->tgtInFrameNumber;
		// process this frame as a keyframe?
		if( ( isKeyframe == FALSE ) ||
			( ABS( distNext ) < ABS( distThis ) ) )
		{
			// no - treat it as a non-keyframe
			result = rtfFltProcessNonKeyframe( pFlt, phPic[ 0 ] );
			RTF_CHK_RESULT;
		}
		else
		{
			// yes - process the keyframe
			pFlt->hPic = phPic[ 0 ];
			result = rtfFltProcessKeyframe( pFlt );
			RTF_CHK_RESULT;
		}
		// advance the input picture counter to the end of the group
		pFlt->picInCount +=  pFlt->gopPicCount;
		// is there an active conditional access table?
		if( pFlt->catValid != FALSE )
		{
			// yes - iterate over the remaining pictures in the group
			// and output any ECM packets found
			for( i=1; i<pFlt->gopPicCount; ++i )
			{
				result = rtfFltProcessNonKeyframe( pFlt, phPic[ i ] );
				RTF_CHK_RESULT;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// close the filter
RTF_RESULT rtfFltClose( RTF_FLT_HANDLE handle )
{
	RTF_FNAME( "rtfFltClose" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_NE( pFlt, RTF_FLT_STATE_CLOSED );
#ifdef DO_STATISTICS
		RTF_LOG_INFO5( RTF_MSG_INF_STATS,
			"Filter #%d: Final PicInCount=%d GopInCount=%d picOutCount=%d AccOutCount=%d",
			pFlt->filterNumber, pFlt->picInCount, pFlt->gopInCount,
			pFlt->picOutCount, pFlt->accOutCount );
#endif
		// set filter state to closed
		pFlt->state = RTF_FLT_STATE_CLOSED;
#if GENERATE_TRACE_FILES
		_close( pFlt->traceFile );
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef _DEBUG
char *rtfFltStateString[] =
{	"invalid", "closed", "open", "ready", "unrecognized" };
// log the state of the output object
RTF_RESULT rtfFltLogState( RTF_FLT_HANDLE handle )
{
	RTF_FNAME( "rtfFltLogState" );
	RTF_OBASE( handle );
	RTF_FLT *pFlt = (RTF_FLT *)handle;
	RTF_RESULT result = RTF_PASS;
	int state;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		state = ( pFlt->state > RTF_FLT_STATE_READY ) ? RTF_FLT_STATE_READY+1 : pFlt->state;
		RTF_LOG_INFO2( RTF_MSG_INF_STATS, ">> Flt %d: state=%s", pFlt->filterNumber, rtfFltStateString[ state ] );
#ifdef DO_STATISTICS
		RTF_LOG_INFO4( RTF_MSG_INF_STATS, ">> input counts: gop=%d pic=%d pkt=%d pcr=%d",
					   pFlt->gopInCount, pFlt->picInCount, pFlt->pktInCount, pFlt->pcrInCount );
		RTF_LOG_INFO4( RTF_MSG_INF_STATS, ">> output counts: acc=%d pcr=%d pkt=%d, vid=%d",
					   pFlt->accOutCount, pFlt->pcrOutCount, pFlt->pktOutCount, pFlt->vidOutCount );
		RTF_LOG_INFO7( RTF_MSG_INF_STATS, ">> insert counts: pcr=%d pad=%d pat=%d pmt=%d vid=%d pkt=%d ncf=%d",
					   pFlt->pcrGenCount, pFlt->padGenCount, pFlt->patGenCount,
					   pFlt->pmtGenCount, pFlt->vidGenCount, pFlt->pktGenCount, pFlt->ncfGenCount );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, ">> other counts: pesMod=%d parFirst=%d parLast=%d",
					   pFlt->pesModCount, pFlt->partialFirstCount, pFlt->partialLastCount ); 
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif
