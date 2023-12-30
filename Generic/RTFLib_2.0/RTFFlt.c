// implementation file for rtfFlt class
// abstracts output file filter
//

#include <time.h>

#include "RTFPrv.h"
#include "RTFFltCtlVarGop.h"
#include "RTFFltCtlFixGop.h"
#include "RTFFltPrv.h"

// rate controller interface ************************************************************

// reset the rate controller info (called from constructor)
static void rtfFltCtlReset( P_RTF_FLT pFlt )
{
	if( pFlt->trickSpec.rateControlVarGOP == FALSE )
	{
		rtfFltCtlFixGopReset( pFlt );
	}
	else
	{
		rtfFltCtlVarGopReset( pFlt );
	}
}

// initialize the rate controller (trick spec is available)
static void rtfFltCtlOpen( P_RTF_FLT pFlt )
{
	if( pFlt->trickSpec.rateControlVarGOP == FALSE )
	{
		rtfFltCtlFixGopOpen( pFlt );
	}
	else
	{
		rtfFltCtlVarGopOpen( pFlt );
	}
}

// set the bit rate (called during "sniff" phase)
static RTF_RESULT rtfFltCtlSetBitRate( P_RTF_FLT pFlt )
{
	RTF_RESULT result;

	if( pFlt->trickSpec.rateControlVarGOP == FALSE )
	{
		result = rtfFltCtlFixGopSetBitRate( pFlt );
	}
	else
	{
		result = rtfFltCtlVarGopSetBitRate( pFlt );
	}
	return result;
}

// set the frame rate (called during "sniff" phase)
static void rtfFltCtlSetFrameRate( P_RTF_FLT pFlt )
{
	if( pFlt->trickSpec.rateControlVarGOP == FALSE )
	{
		rtfFltCtlFixGopSetFrameRate( pFlt );
	}
	else
	{
		rtfFltCtlVarGopSetFrameRate( pFlt );
	}
}

// process an input GOP through the rate controller
static RTF_RESULT rtfFltCtlProcessGop( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlProcessGop" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	BOOL isKeyframe;

	do {		 // error escape wrapper - begin

		// get the keyframe flag from the first picture of this group
		result = rtfPicGetIsKeyframe( pFlt->phPic[ 0 ], &isKeyframe );
		RTF_CHK_RESULT;
		// if the first picture of the group is not a keyframe
		// then it is only a partial GOP, and we can't use it
		if( isKeyframe == FALSE )
		{
			break;
		}
		// record the packet count at the start of this access unit
		// NOTE: if this is the first GOP, force the start to pkt 0
		pFlt->accStartPktOutCount = pFlt->pktOutCount;
		// get the packet array info from the first picture of the group
		result = rtfPicGetPacketArrayInfo( pFlt->phPic[ 0 ], &pFlt->pktCount, &pFlt->phPkt,
										   &pFlt->firstBytePktOffset,
										   &pFlt->lastBytePktOffset,
										   &pFlt->lastVideoPacketIndex );
		RTF_CHK_RESULT;
		if( pFlt->trickSpec.rateControlVarGOP == FALSE )
		{
			result = rtfFltCtlFixGopProcess( pFlt );
			RTF_CHK_RESULT;
		}
		else
		{
			result = rtfFltCtlVarGopProcess( pFlt );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// log the output of a packet with the rate controller
// (allows rate controller to react to PID)
static void rtfFltCtlLogOutPkt( P_RTF_FLT pFlt )
{
	if( pFlt->trickSpec.rateControlVarGOP == FALSE )
	{
		rtfFltCtlFixGopLogOutPkt( pFlt );
	}
	else
	{
		rtfFltCtlVarGopLogOutPkt( pFlt );
	}
}

// update the values of lastPTS and lastDTS for the current picture
// (allows different methods of timestamp calculation)
static RTF_RESULT rtfFltCtlUpdateTimestamps( P_RTF_FLT pFlt )
{
	RTF_RESULT result;

	if( pFlt->trickSpec.rateControlVarGOP == FALSE )
	{
		result = rtfFltCtlFixGopUpdateTimestamps( pFlt );
	}
	else
	{
		result = rtfFltCtlVarGopUpdateTimestamps( pFlt );
	}

	return result;
}

// local functions **********************************************************************

// reset the filter structure to a clean state
void rtfResetFlt( RTF_FLT *pFlt )
{
	pFlt->state					= RTF_FLT_STATE_CLOSED;

	pFlt->forcePadBitRate		= 0;
	pFlt->targetBitRate			= RTF_DEFAULT_BITS_PER_SECOND;

	pFlt->frameTicks			= RTF_DEFAULT_FRAME_TICKS;
	pFlt->decodingDelay			= 0;
	pFlt->presentationDelay		= 0;
	pFlt->ptsCount				= 0;
	pFlt->lastPTS				= 0;
	pFlt->lastDTS				= 0;

	pFlt->pOutPktStorage		= (unsigned char *)NULL;
	pFlt->outPktNumber			= 0xFFFFFFFF;
	pFlt->outPktFlags			= 0;
	pFlt->outPktPayloadOffset	= 0;
	pFlt->outPktPesHdrOffset	= 0;
	pFlt->outPktPayloadBytes	= 0;
	pFlt->outPktFixupFlags		= 0;
	pFlt->outPktPesHdrBytes		= 0;

	pFlt->speedRatioFix16		= RTF_DEFAULT_SPEED_RATIO_FIX16;
	pFlt->invSpeedRatioFix16	= RTF_DEFAULT_INV_SPEED_RATIO_FIX16;
	pFlt->picsPerSecFix16		= RTF_DEFAULT_FPS_FIX16;
	pFlt->invPicsPerSecFix16	= RTF_DEFAULT_SPF_FIX16;

	pFlt->pesHdrLen				= 0;
	pFlt->firstBytePktOffset	= 0;
	pFlt->lastBytePktOffset		= 0;
	pFlt->lastVideoPacketIndex	= 0;

	pFlt->filterNumber			= 0;
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
	pFlt->hGop					= (RTF_GOP_HANDLE)NULL;
	pFlt->phPic					= (RTF_PIC_HANDLE *)NULL;

	pFlt->pProfile				= (RTF_STREAM_PROFILE *)NULL;

	pFlt->catValid				= FALSE;
	pFlt->catEcmPid				= TRANSPORT_INVALID_PID;

	pFlt->pktIndex				= 0;
	pFlt->pktCount				= 0;
	pFlt->gopPicCount			= 0;

	pFlt->pNCFramePicHdr		= (unsigned char *)NULL;
	pFlt->ncFrameTemporalRef	= 0;
	pFlt->ncFramePacketCount	= 0;
	
	pFlt->pesCaptured			= FALSE;
	pFlt->mainFileCopy			= FALSE;
	pFlt->bitRateSet			= FALSE;
	pFlt->frameRateSet			= FALSE;
	pFlt->generateNCPF			= TRUE;
	pFlt->lastPcrPktOutCount	= 0;
	pFlt->accStartPktOutCount	= 0;

	pFlt->picInCount			= 0;
	pFlt->gopInCount			= 0;
	pFlt->picOutCount			= 0;
	pFlt->accOutCount			= 0;
	pFlt->pktOutCount			= 0;
	pFlt->keyStartPktCount		= 0;

#ifdef DO_STATISTICS
	pFlt->pktInCount			= 0;
	pFlt->pcrInCount			= 0;
	pFlt->pesInCount			= 0;

	pFlt->pesOutCount			= 0;
	pFlt->pcrOutCount			= 0;
	pFlt->vidOutCount			= 0;

	pFlt->pesGenCount			= 0;
	pFlt->pcrGenCount			= 0;
	pFlt->psiGenCount			= 0;
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
	pFlt->ccPidCount			= 0;
	pFlt->ccVidIndex			= 0;

	// reset the no-change picture descriptor
	rtfPicReset( pFlt->hNCPic );

	// reset the rate control info
	rtfFltCtlReset( pFlt );

	// reset some arrays in the state structure
	memset( (void *)pFlt->outPktFixupOffset,	0, sizeof(pFlt->outPktFixupOffset)	);	
	memset( (void *)&pFlt->trickSpec,			0, sizeof(pFlt->trickSpec)			);
	memset( (void *)&pFlt->lastPCR,				0, sizeof(pFlt->lastPCR)			);
	memset( (void *)pFlt->incPidList,		 0xFF, sizeof(pFlt->incPidList)			);
	memset( (void *)pFlt->excPidList,		 0xFF, sizeof(pFlt->excPidList)			);
	memset( (void *)pFlt->ccPidList,		 0xFF, sizeof(pFlt->ccPidList)			);
	memset( (void *)pFlt->nextCC,			 0x0F, sizeof(pFlt->nextCC)				);
	memset( (void *)pFlt->tempPkt,				0, sizeof(pFlt->tempPkt)			);
	memset( (void *)pFlt->pesHdr,				0, sizeof(pFlt->pesHdr)				);
	memset( (void *)pFlt->pesHdrPkt,			0, sizeof(pFlt->pesHdrPkt)			);
	memset( (void *)pFlt->ncPktData,			0, sizeof(pFlt->ncPktData)			);
}

// set up the lists of PIDs to be included / excluded in the trick files for this filter
RTF_RESULT rtfFltSetPidLists( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltSetPidLists" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_TRICK_SPEC *pSpec;
	RTF_STREAM_TYPE type, *pStreamTypes;
	BOOL suppress;
	unsigned short i;

	do {		 // error escape wrapper - begin

		// create a convenienece pointer to the trick spec
		pSpec = &( pFlt->trickSpec );
		// set up the constant PIDs first
		// note: these will never show up in a PMT
		if( pSpec->suppressPAT == FALSE )
		{
			pFlt->incPidList[ pFlt->incPidCount++ ] = TRANSPORT_PAT_PID;
		}
		else
		{
			pFlt->excPidList[ pFlt->excPidCount++ ] = TRANSPORT_PAT_PID;
		}
		if( pSpec->suppressCAT == FALSE )
		{
			pFlt->incPidList[ pFlt->incPidCount++ ] = TRANSPORT_CAT_PID;
		}
		else
		{
			pFlt->excPidList[ pFlt->excPidCount++ ] = TRANSPORT_CAT_PID;
		}
		if( pSpec->suppressNUL == FALSE )
		{
			pFlt->incPidList[ pFlt->incPidCount++ ] = TRANSPORT_PAD_PID;
		}
		else
		{
			pFlt->excPidList[ pFlt->excPidCount++ ] = TRANSPORT_PAD_PID;
		}
		// get the stream types from the PMT
		result = rtfPmtGetAllStreamTypes( pFlt->hPmt, &pStreamTypes );
		RTF_CHK_RESULT;
		// loop thru this array looking for known PIDs
		// NOTE - don't bother with PAT, CAT, or NULL; those were covered above
		for( i=TRANSPORT_CAT_PID+1; i<TRANSPORT_PAD_PID; ++i )
		{
			type = pStreamTypes[ i ];
			if( type != RTF_STREAM_TYPE_UNKNOWN )
			{
				suppress = pSpec->suppressOTHER;
				switch( type )
				{
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
void rtfFltFindIncPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex )
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
void rtfFltFindExcPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex )
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

// return the index of a particular PID on the CC tracking list - -1 if it is not there
void rtfFltFindCCPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex )
{
	int i;

	*pIndex = -1;
	// is this pid on the CC list?
	for( i=0; i<pFlt->ccPidCount; ++i )
	{
		if( pFlt->ccPidList[ i ] == pid )
		{
			*pIndex = i;
			break;
		}
	}
}

UINT64 rtfFltComputePcr( RTF_FLT *pFlt, int packetNumber )
{
	INT64 pcr = packetNumber - pFlt->accStartPktOutCount; // packets
	pcr *= TRANSPORT_PACKET_BITS;				// bits
	pcr *= TRANSPORT_SCR_TICKS_PER_SECOND;		// bits X ticks/sec
	pcr /= pFlt->pProfile->bitsPerSecond;		// ticks
	pcr += pFlt->ctl.fix.baseGopPcr;			// base PCR for this gop
	return pcr;
}

// get the current output bitstream PCR time according to position
// NOTE: for reverse trick files, this is based on a projection of
// the size of the current access unit, so this is only an estimate
void rtfFltComputeOutputPcr( RTF_FLT *pFlt, RTF_TIMESTAMP *pPcr )
{
	UINT64 temp64 = rtfFltComputePcr(pFlt, pFlt->pktOutCount-1);
	pPcr->base.ull = temp64 / TRANSPORT_SCR_TO_TS_RATIO;
	pPcr->ext.us = (unsigned short)( temp64 - ( pPcr->base.ull * TRANSPORT_SCR_TO_TS_RATIO ) );
}

// generate a TTS timestamp based on current bitstream position
RTF_RESULT rtfFltGenerateTTSPrefix( RTF_FLT *pFlt )
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

// reset the next CC for a particular PID
RTF_RESULT rtfFltResetNextCC( RTF_FLT *pFlt, unsigned short pid )
{
	RTF_FNAME( "rtfFltResetNextCC" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int index;

	do {		 // error escape wrapper - begin

		// get the index of this PID
		rtfFltFindCCPID( pFlt, pid, &index );
		// found?
		if( index < 0 )
		{
			// no - room for one more?
			if( pFlt->ccPidCount >= RTF_TRICK_MAX_PIDCOUNT )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "CC tracking array overflow" );
				break;
			}
			// create a new entry for this PID
			index = pFlt->ccPidCount++;
			pFlt->ccPidList[ index ] = pid;
		}
		// reset the CC value for this PID
		// note: due to CC pre-increment, this wants to be (0-1)%16!
		pFlt->nextCC[ index ] = 0x0F;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// find the next CC for a particular PID; return it and then increment it
RTF_RESULT rtfFltFindNextCC( RTF_FLT *pFlt, unsigned short pid, unsigned char *pCC, unsigned long flags )
{
	RTF_FNAME( "rtfFltFindNextCC" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char cc = 0;
	int index;

	do {		 // error escape wrapper - begin

		// get the index of this PID on the CC PID list
		rtfFltFindCCPID( pFlt, pid, &index );
		// found?
		if( index < 0 )
		{
			// no - room for one more?
			if( pFlt->ccPidCount >= RTF_TRICK_MAX_PIDCOUNT )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "CC tracking array overflow" );
				break;
			}
			// create a new entry for this PID
			index = pFlt->ccPidCount++;
			pFlt->ccPidList[ index ] = pid;
			// is this the video PID?
			if( pid == pFlt->pProfile->videoSpec.pid )
			{
				// yes - record the index of the video PIDs CC entry
				pFlt->ccVidIndex = index;
			}
		}
		// check the packet flags - is a payload present?
		// note - payload-free packets don't increment the CC
		cc = pFlt->nextCC[ index ];
		if( ( flags & RTF_PKT_PAYLOADABSENT ) == 0 )
		{
			// yes - pre-increment the CC
			cc = ++pFlt->nextCC[ index ];
		}
		// make the return
		*pCC = cc & 0x0F;
	} while( 0 ); // error escape wrapper - end

	return result;
}

// send a packet's worth of data to the output
RTF_RESULT rtfFltQueueData( RTF_FLT *pFlt, unsigned char *pData )
{
	RTF_FNAME( "rtfFltQueueData" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned short pid;

	do {		 // error escape wrapper - begin

		// is TTS output enabled?
		if( pFlt->trickSpec.generateTTS != FALSE )
		{
			result = rtfFltGenerateTTSPrefix( pFlt );
			RTF_CHK_RESULT;
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
		// check the pid of this packet
		pid = pData[ 1 ];
		pid = ( pid << 8 ) | pData[ 2 ];
		pid &= 0x1FFF;
		// update the record of the last packet pid
		pFlt->outPID = pid;
		// log the output data with the rate controller
		rtfFltCtlLogOutPkt( pFlt );
		// bump the video output packet count if this was a video packet
		RTF_ADD_STAT( pFlt->vidOutCount, ( pid == pFlt->pProfile->videoSpec.pid ) ? 1 : 0 );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set bit flag at offset

static void Flag(unsigned char *basePtr, int bitNum, int value)
{
	int byteNum = bitNum / 8;
	bitNum %= 8;
	
	if (value)
	{
		basePtr[byteNum] |= (1 << bitNum);
	}
	else
	{
		basePtr[byteNum] &= ~(1 << bitNum);
	}
}
//
// initialize the null packet
//

static void rtfInitNullPacket(RTF_FLT *pFlt)
{
	unsigned char *pPkt = pFlt->nullPkt;
	unsigned char *saveCountPtr;
	time_t t;
	char timebuffer[32];
	int i;

	*pPkt++ = TRANSPORT_PACKET_SYNCBYTE;
	*pPkt++ = (unsigned char)( ( TRANSPORT_PAD_PID >> 8 ) & 0xFF );
	*pPkt++ = (unsigned char)( TRANSPORT_PAD_PID & 0xFF );
	*pPkt++ = 0x10;	// note - stuffing packets don't increment their CC

	memcpy(pPkt, "Rtflib", 7);
	pPkt += 7;

	saveCountPtr = pPkt;
	*saveCountPtr = 2;
	*pPkt++;

	// *****
	//
	// the version number represents the order of the parameters
	//
	// any change that changes the parameter order needs a version number 
	// change.
	//
	// if more parameters are added and are added at the end, the version
	// number does not need to be changed.
	//
	// if parameters are superceded, the code should be updated to
	// insert a zero for the old parameter and the new parameter should be 
	// added to the end.
	//
	// *****

	*pPkt++	= 1;			// version - increment on any change that breaks the format
	(*saveCountPtr)++;

	memcpy(pPkt, "WIN", 4);
	pPkt += 4;
	(*saveCountPtr)++;

	memcpy(pPkt, __DATE__, strlen(__DATE__));
	pPkt += strlen(__DATE__);
	*pPkt++ = ' ';
	memcpy(pPkt, __TIME__, strlen(__DATE__));
	pPkt += strlen(__TIME__)+1;

	(*saveCountPtr)++;

    t = time(0);
	strftime(timebuffer, sizeof(timebuffer)-1,"%b  %#d %Y %H:%M:%S", localtime(&t));
	memcpy(pPkt, timebuffer, strlen(timebuffer)+1);
	pPkt += strlen(timebuffer)+1;
	(*saveCountPtr)++;

	if (pFlt->trickSpec.speedDirection == 0)
	{
		*pPkt++ = 'B';
	}
	else if (pFlt->trickSpec.speedDirection < 1)
	{
		*pPkt++ = 'R';
	}
	else
	{
		*pPkt++ = 'F';
	}
	(*saveCountPtr)++;

	*pPkt++ = (char)pFlt->trickSpec.speedNumerator;
	(*saveCountPtr)++;
	*pPkt++ = (char)pFlt->trickSpec.speedDenominator;
	(*saveCountPtr)++;

	memcpy(pPkt, pFlt->trickSpec.fileExtension, strlen(pFlt->trickSpec.fileExtension)+1);
	pPkt += strlen(pFlt->trickSpec.fileExtension)+1;
	(*saveCountPtr)++;

	*((unsigned long *)pPkt)++ = pFlt->pProfile->bitsPerSecond;
	(*saveCountPtr)++;

	*pPkt++	= (unsigned char)pFlt->pProfile->augmentationPidCount;
	(*saveCountPtr)++;

	if (pFlt->pProfile->augmentationPidCount)
	{
		for (i = 0 ; i < pFlt->pProfile->augmentationPidCount ; i++)
		{
			*((unsigned short *)pPkt)++ = (unsigned short)pFlt->pProfile->augmentationPids[i];
		}
	}

	*((unsigned short *)pPkt)++ = pFlt->pProfile->augmentationBaseFactor;
	(*saveCountPtr)++;
	*((unsigned short *)pPkt)++ = pFlt->pProfile->augmentationPlusFactor;
	(*saveCountPtr)++;

	i = 0;
	Flag(pPkt, i++, pFlt->trickSpec.insertPES);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.insertPSI);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.insertPCR);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.insertNCF);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.insertNUL);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.replacePAT);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.replacePMT);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressPAT);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressPMT);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressCAT);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressECM);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressAUD);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressDAT);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressNUL);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressFLUFF);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressOTHER);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.augmentWithNUL);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.augmentWithFLUFF);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.augmentWithPID);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.ignoreEncryption);
	(*saveCountPtr)++;

	Flag(pPkt, i++, pFlt->trickSpec.prefixPSI);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.constantBitRate);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.userBitRate);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.interpTimeStamps);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.forcePadding);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.optimizeForATSC);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.generateTTS);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.dittoFrames);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressInputPesHdr);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.zeroPesPktLen);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.clearDAI);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressPTSDTS);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.suppressDTS);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.restampPTSDTS);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.setPRIO);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.setRAND);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.insertDSM);
	(*saveCountPtr)++;

	Flag(pPkt, i++, pFlt->trickSpec.suppressInputPCR);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.sequentialCC);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.restampPCR);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.directionalPCR);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.clearDCON);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.remapPIDs);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.setLowDelay);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.clearGOPTime);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.clearDropFrame);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.setClosedGOP);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.setBrokenLink);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.clearTemporalRef);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.resetVBVDelay);
	(*saveCountPtr)++;
	Flag(pPkt, i++, pFlt->trickSpec.codec.mpeg2.clear32Pulldown);
	(*saveCountPtr)++;

	pPkt += (i/8 + (i%8) ? 1 : 0);
	memset(pPkt, 0xff, TRANSPORT_PACKET_BYTES-(pPkt-pFlt->nullPkt));
}

// insert a stuffing packet
RTF_RESULT rtfFltInsertStuffingPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertStuffingPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pPkt = pFlt->tempPkt;

	do {		 // error escape wrapper - begin
#if 1
		if (pFlt->nullPkt[0] == 0)
		{
			rtfInitNullPacket(pFlt);
		}
		result = rtfFltQueueData( pFlt, pFlt->nullPkt);
#else
		// generate a stuffing packet in the temp packet buffer
		memset( (void *)pPkt, 0xFF, TRANSPORT_PACKET_BYTES );
		// header
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( TRANSPORT_PAD_PID >> 8 ) & 0xFF );
		pPkt[ 2 ] = (unsigned char)( TRANSPORT_PAD_PID & 0xFF );
		pPkt[ 3 ] = 0x10;	// note - stuffing packets don't increment their CC
		result = rtfFltQueueData( pFlt, pPkt );
#endif
		RTF_CHK_RESULT;
		// bump the running pad generation count
		RTF_INC_STAT( pFlt->padGenCount );
		// bump the running packet generation count
		RTF_INC_STAT( pFlt->pktGenCount );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert an augmentation packet
RTF_RESULT rtfFltInsertAugmentationPacket( RTF_FLT *pFlt )
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

// Split the payload of the last packet sent to the output into 2 packets. Returns with
// the output packet image adjusted, and the removed portion of the payload in a packet
// in the temp buffer. The bytes in the second buffer are at the same offset as in the
// original, so the remaining fixup offsets do not need to be changed. The removed payload
// bytes are replaced with padding bytes in the adaptation field of the original packet. 
// NOTE: all fixups that map to features earlier in the packet must be performed before
// calling this routine, since this routine may split the pac
// NOTE: split off more than the bare minimum number of bytes in order to avoid having a
// header straddle more than a single packet.
RTF_RESULT rtfFltSplitLastOutPayload( RTF_FLT *pFlt, unsigned char byteCount )
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
RTF_RESULT rtfFltAdjustPartialFirstPacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltAdjustPartialFirstPacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	int off;
	int len;

	do {		 // error escape wrapper - begin
		// want to zero out all of the bytes that precede the start of the current picture
		// however, retain any PES header that may be lurking at the start of the packet
		//
		// determine if the access unit data precedes the pes header or follows it
		//
		if (pFlt->outPktPesHdrOffset == pFlt->outPktPayloadOffset)
		{
			// access unit data follows pes header - set offset to point after the pes header
			//
			off = pFlt->outPktPesHdrOffset + pFlt->outPktPesHdrBytes;
			//
			// set length to the offset to the first start code minus the starting offset
			//
			len = pFlt->firstBytePktOffset - off;
		}
		else
		{
			// access unit data precedes the pes header (which is illegal btw)
			// set offset to be the first payload byte
			//
			off = pFlt->outPktPayloadOffset;
			//
			// the length is the difference between the pes header offset and the payload offset
			//
			len = pFlt->outPktPesHdrOffset - pFlt->outPktPayloadOffset;
		}
		//
		// set bytes of prior access unit to zeros
		memset( pFlt->pOutPktStorage + off, 0x00, len);

	} while( 0 ); // error escape wrapper - end

	return result;
}

// adjust a packet in the output buffer to include only the leading data from the
// indicated input packet. also make all necessary adjustments to recorded fixup list
RTF_RESULT rtfFltAdjustPartialLastPacket( RTF_FLT *pFlt )
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
RTF_RESULT rtfListMpeg2Fixups( RTF_FLT *pFlt )
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
RTF_RESULT rtfListH264Fixups( RTF_FLT *pFlt )
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
RTF_RESULT rtfListVc1Fixups( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfListVc1Fixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// placeholder

	} while( 0 ); // error escape wrapper - end

	return result;
}

// list all of the fixups required in the payload of an input packet
RTF_RESULT rtfFltListPayloadFixups( RTF_FLT *pFlt )
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
RTF_RESULT rtfFltFixPCR( RTF_FLT *pFlt )
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

// perform fixups on the packet header of the current output packet
RTF_RESULT rtfFltFixPacketHeader( RTF_FLT *pFlt )
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
RTF_RESULT rtfFltQueuePacket( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltQueuePacket" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned long outPktNumber;
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
			result = rtfFltGenerateTTSPrefix( pFlt );
			RTF_CHK_RESULT;
		}
		// queue the packet to the output buffer
		// Note: the packet being queued is guaranteed to be
		// in the output buffer when this call is completed
		result = rtfOutQueuePacket( pFlt->hOut, pFlt->phPkt[ pFlt->pktIndex ] );
		RTF_CHK_RESULT;
		// update the bit count of the next frame to be added to the bit rate control queue
		rtfFltCtlLogOutPkt( pFlt );
		// bump the output packet count
		++pFlt->pktOutCount;
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
			// yes - modify the image of the packet in the output buffer
			// and the list of fixups recorded above
			result = rtfFltAdjustPartialLastPacket( pFlt );
			RTF_CHK_RESULT;
			// bump the partial last packet count
			RTF_INC_STAT( pFlt->partialLastCount );
		}
		if (pFlt->pktIndex == 0 && pFlt->outPktFlags & RTF_PKT_PESHDRPRESENT)
		{
			// if the first access unit byte is different from the end of the pes header then some
			// payload from a prior access unit exists in this packet and we want to zap it
			//
			if (pFlt->firstBytePktOffset != pFlt->outPktPesHdrOffset + pFlt->outPktPesHdrBytes)
			{
				// yes - modify the image of the packet in the output buffer
				// and the list of fixups recorded above
				result = rtfFltAdjustPartialFirstPacket( pFlt );
				RTF_CHK_RESULT;
				// bump the partial first packet count
				RTF_INC_STAT( pFlt->partialFirstCount );
			}
		}
		// perform any required fixups on the packet header
		result = rtfFltFixPacketHeader( pFlt );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// queue then next video packet of a picture to the output
RTF_RESULT rtfFltQueueVideoPacket( RTF_FLT *pFlt )
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
				rtfFltFindIncPID( pFlt, pid, &index );
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
RTF_RESULT rtfFltFixByte( RTF_FLT *pFlt, unsigned char andValue,
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
RTF_RESULT rtfFltFixSeqHdr( RTF_FLT *pFlt, unsigned char seqHdrOffset )
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
RTF_RESULT rtfFltFixSqxHdr( RTF_FLT *pFlt, unsigned char sqxHdrOffset )
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
RTF_RESULT rtfFltFixGopHdr( RTF_FLT *pFlt, unsigned char gopHdrOffset )
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
RTF_RESULT rtfFltFixPicHdr( RTF_FLT *pFlt, unsigned char picHdrOffset )
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

// perform picture coding header fixups
RTF_RESULT rtfFltFixCodHdr( RTF_FLT *pFlt, unsigned char codHdrOffset )
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
int rtfGetAdaptStuffingBytes( unsigned char *pPkt )
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

// compute the number of stuffing bytes in a PES packet header
int rtfGetPesHeaderStuffingBytes( unsigned char *pHdr )
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
RTF_RESULT rtfFltApplyPesHdrFixups( RTF_FLT *pFlt, unsigned char *pPkt, unsigned char *pHdr, BOOL *pModFlag )
{
	RTF_FNAME( "rtfFltApplyPesHdrFixups" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pSrc, *pDst;
	unsigned char hdrLength, flags, byteCount;
	unsigned char delta, pesOff, payOff, dsmOff;
	BOOL splitPkt = FALSE;
	int i;

	do {		 // error escape wrapper - begin

		// is zeroPesLen enabled?
		if( pFlt->trickSpec.zeroPesPktLen != FALSE )
		{
			// yes - make sure that the PES packet length field is 0
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
				// no - is suppressDTS enabled?
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
						// if here during initialization before actually processing
						// gop data then skip updating the timestamps
						//
						if (pFlt->gopInCount)
						{
							// calculate dts/pts for this pes header
							result = rtfFltCtlUpdateTimestamps( pFlt );
							RTF_CHK_RESULT;
							// bump the PTS counter
							++pFlt->ptsCount;
						}
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
void rtfFltSuppressPesHdr( RTF_FLT *pFlt, unsigned char *pPkt, unsigned char payloadOffset,
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
RTF_RESULT rtfFltFixPesHdr( RTF_FLT *pFlt, unsigned char pesHdrOffset )
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
RTF_RESULT rtfFltDoPayloadFixups( RTF_FLT *pFlt )
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
RTF_RESULT rtfFltRemapOutPacket( RTF_FLT *pFlt )
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
RTF_RESULT rtfFltProcessPacket( RTF_FLT *pFlt )
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
		// NOTE: this also performs packet header fixups and lists payload fixups
		// NOTE: Packet is guaranteed to remain in the output buffer after this call
		// NOTE: It is important that all fixups be performed in packet offset order,
		//		 as this allows us to deal with constructs that span packet boundaries
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
void rtfGetPacketLengths( unsigned char *pStorage, 
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
RTF_RESULT rtfFltLastPacketInsert( RTF_FLT *pFlt, unsigned short insPid,
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
RTF_RESULT rtfFltInsertPesHeader( RTF_FLT *pFlt )
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

// insert a set of PSI tables
RTF_RESULT rtfFltInsertPSI( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertPSI" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pTable;
	unsigned char cc;
	BOOL firstPSI = pFlt->lastPSIPktOutCount == 0;
	static int counter = 0;

	do {		 // error escape wrapper - begin

		// is PAT replacement enabled?
		if( pFlt->trickSpec.replacePAT != FALSE )
		{
			// yes - point at the replacement PAT packet in the filter settings structure
			pTable = pFlt->trickSpec.pat;
			// make sure there really is a packet there
			if( pTable == (unsigned char *)NULL )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Null trickspec PAT pointer" );
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

//		*(int*)(pTable+184)=counter++;

		// output the indicated packet data
		result = rtfFltQueueData( pFlt, pTable );
		RTF_CHK_RESULT;
		// PMT replacement enabled?
		if( pFlt->trickSpec.replacePMT != FALSE )
		{
			// yes - point at the replacement PMT packet in the filter settings structure
			pTable = pFlt->trickSpec.pmt;
			// make sure there really is a packet there
			if( pTable == (unsigned char *)NULL )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Null trickspec PMT pointer" );
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
		// insert a null packet to record the trickspec context
		if (firstPSI)
		{
			result = rtfFltInsertStuffingPacket( pFlt );
			RTF_CHK_RESULT;
		}
		// bump the running PSI generation count
		RTF_INC_STAT( pFlt->psiGenCount );

	} while( 0 ); // error escape wrapper - end
	//
	// record most recent psi
	//
	pFlt->lastPSIPktOutCount = pFlt->pktOutCount;

	return result;
}

// insert a PCR in the output stream
RTF_RESULT rtfFltInsertPcr( RTF_FLT *pFlt )
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
			offset.base.ull = temp64 / TRANSPORT_SCR_TO_TS_RATIO;
			offset.ext.us   = (unsigned short)( temp64 - ( offset.base.ull * TRANSPORT_SCR_TO_TS_RATIO ) );
			pcr.base.ull >>= 7;
			pcr.base.ull += offset.base.ull;
			pcr.ext.us   += offset.ext.us;
			if( pcr.ext.us >= TRANSPORT_SCR_TO_TS_RATIO )
			{
				pcr.ext.us -= TRANSPORT_SCR_TO_TS_RATIO;
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
RTF_RESULT rtfInsertVideoFluff( RTF_FLT *pFlt )
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

// prepare the no-change video frame
RTF_RESULT rtfFltPrepareNCFrame( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltPrepareNCFrame" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD_HANDLE hVcd;
	RTF_PICSTATE picState;
	BOOL changed;
	BOOL junkBool;
	unsigned char *pFrame;
	unsigned char *pTgt;
	int i, j, bytes, adaptLen;
	unsigned char payloadOffset;
	unsigned char payloadBytes;

	do {		 // error escape wrapper - begin

		// at this point, the video codec must have been initialized
		// get the video codec object handle from the session
		result = rtfSesGetVideoCodec( pFlt->hSes, &hVcd );
		RTF_CHK_RESULT;
		// get the no-change P frame info from the codec
		result = rtfVcdGetNCPFrame( hVcd, &pFrame, &bytes, &pFlt->generateNCPF );
		RTF_CHK_RESULT;
		// iterate over the the no-change frame data and packetize it
		// run fixups on the packets as needed
		pFlt->ncFramePacketCount = 0;
		memset( (void *)pFlt->ncPktData, 0xFF, sizeof(pFlt->ncPktData) );
		for( i=0; i<RTF_MAX_NCFRAME_PACKETS; ++i )
		{
			if( bytes <= 0 )
			{
				break;
			}
			++pFlt->ncFramePacketCount;
			pFlt->ncPktData[ i ][ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
			pFlt->ncPktData[ i ][ 1 ] = (unsigned char)( ( pFlt->pProfile->videoSpec.pid >> 8 ) & 0xFF );
			pFlt->ncPktData[ i ][ 2 ] = (unsigned char)( pFlt->pProfile->videoSpec.pid & 0xFF );
			pFlt->ncPktData[ i ][ 5 ] = 0;
			if( i == 0 )
			{
				pFlt->ncPktData[ 0 ][ 1 ] |= 0x40;
				pFlt->ncPktData[ 0 ][ 5 ]  = 0x10;
			}
			if( bytes >= RTF_NCF_FIRST_PACKET_BYTES )
			{
				payloadBytes = RTF_NCF_FIRST_PACKET_BYTES;
				if( i == 0 )
				{
					payloadBytes -= 7;
				}
			}
			else
			{
				payloadBytes = bytes;
			}
			adaptLen = ( TRANSPORT_PACKET_BYTES - 5 ) - payloadBytes;
			pFlt->ncPktData[ i ][ 3 ] = 0x30 | ( i & 0x0F );
			pFlt->ncPktData[ i ][ 4 ] = adaptLen;
			payloadOffset = 5 + adaptLen;
			memcpy( (void *)&(pFlt->ncPktData[ i ][ payloadOffset ]), (void *)pFrame, payloadBytes );
			// is this an MPEG-2 stream?
			if( pFlt->pProfile->videoSpec.eStream.video.vcdType == RTF_VIDEO_CODEC_TYPE_MPEG2 )
			{
				// scan the packet payload
				for( j=0; j<payloadBytes-4; ++j )
				{
					pTgt = &( pFlt->ncPktData[ i ][ payloadOffset + j ] );
					// is this a PES packet header?
					if( ( pTgt[ 0 ] == 0x00 ) &&
						( pTgt[ 1 ] == 0x00 ) &&
						( pTgt[ 2 ] == 0x01 ) &&
						( ( pTgt[ 3 ] & 0xF0 ) == 0xE0 ) )
					{
						// yes - record the location of the PES header
						pFlt->ncFramePesHdrPktIndex  = i;
						pFlt->ncFramePesHdrPktOffset = payloadOffset + j;
						// apply PES header fixups
						result = rtfFltApplyPesHdrFixups( pFlt, pFlt->ncPktData[ i ], pTgt, &changed );
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
			else
			{
				// scan the packet payload
				for( j=0; j<payloadBytes-4; ++j )
				{
					pTgt = &( pFlt->ncPktData[ i ][ payloadOffset + j ] );
					// is this a PES packet header?
					if( ( pTgt[ 0 ] == 0x00 ) &&
						( pTgt[ 1 ] == 0x00 ) &&
						( pTgt[ 2 ] == 0x01 ) &&
						( ( pTgt[ 3 ] & 0xF0 ) == 0xE0 ) )
					{
						// yes - record the location of the PES header
						pFlt->ncFramePesHdrPktIndex  = i;
						pFlt->ncFramePesHdrPktOffset = payloadOffset + j;
					}
				}
			}
			// advance to the next packet's data
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
		// get the state of the no-change picture
		result = rtfPicGetState( pFlt->hNCPic, &picState );
		RTF_CHK_RESULT;
		// has the no-change picture been set up yet?
		if( picState != RTF_PICSTATE_CLOSED )
		{
			// yes - need to release it before proceeding
			result = rtfPicRelease( pFlt->hNCPic, pFlt->hSes );
			RTF_CHK_RESULT;
		}
		// re-open the no-change picture descriptor
		result = rtfPicOpen( pFlt->hNCPic, -1, -1, pFlt->pProfile->videoSpec.pid,
							 pFlt->pProfile->videoSpec.eStream.video.vcdType );
		RTF_CHK_RESULT;
		// iterate over the packets - map them and add them to the no-change picture
		for( i=0; i<pFlt->ncFramePacketCount; ++i )
		{
			result = rtfPktMap( pFlt->hNCPkt[ i ], (RTF_BUF_HANDLE)NULL,
								pFlt->ncPktData[ i ], i, i, &junkBool );
			RTF_CHK_RESULT;
			result = rtfPicAddPacket( pFlt->hNCPic, pFlt->hNCPkt[ i ] );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// record the first and last byte offsets
		pFlt->ncFrameFirstBytePktOffset = 4;
		pFlt->ncFrameLastBytePktOffset  = TRANSPORT_PACKET_BYTES-1;
		// close the no-change picture object
		result = rtfPicClose( pFlt->hNCPic );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// insert a no-change frame
RTF_RESULT rtfFltInsertNCFrame( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltInsertNCFrame" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	LONGLONG pcr;
	unsigned char *pPcr = (unsigned char *)&pcr;
	LONGLONG ext;
	unsigned char *pExt = (unsigned char *)&ext;
	unsigned long packets;
	unsigned long i;

	do {		 // error escape wrapper - begin

		// is no-change frame insertion enabled?
		if( pFlt->trickSpec.insertNCF != FALSE )
		{
			// yes - update the NCFrame, if required
			if( pFlt->generateNCPF != FALSE )
			{
				result = rtfFltPrepareNCFrame( pFlt );
				RTF_CHK_RESULT;
			}
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
			// 
			// update pcr - note pktOutCount hasn't been incremented yet so we can use
			// rtfFltComputeOutputPcr
			//
//			pcr = rtfFltComputePcr(pFlt, pFlt->pktOutCount+1);
			pcr = rtfFltComputePcr(pFlt, pFlt->pktOutCount);
			ext = pcr % TRANSPORT_SCR_TO_TS_RATIO;
			pcr /= TRANSPORT_SCR_TO_TS_RATIO;

			pcr <<= 7;
			pFlt->ncPktData[0][6]   = pPcr[4];
			pFlt->ncPktData[0][7]   = pPcr[3];
			pFlt->ncPktData[0][8]   = pPcr[2];
			pFlt->ncPktData[0][9]   = pPcr[1];
			pFlt->ncPktData[0][10]  = pPcr[0];
			pFlt->ncPktData[0][10] |= pExt[1] & 0x01;
			pFlt->ncPktData[0][11]  = pExt[0];

			pFlt->lastPcrPktOutCount = pFlt->pktOutCount;

			// send the packets of the NC frame to the output
			for( i=0; i<pFlt->ncFramePacketCount; ++i )
			{
				// is sequential CC enabled?
				if( pFlt->trickSpec.sequentialCC != FALSE )
				{
					unsigned char cc;

					// yes - get the next video PID CC
					result = rtfFltFindNextCC( pFlt, pFlt->pProfile->videoSpec.pid, &cc, 0 );
					RTF_CHK_RESULT;
					// insert it into the packet
					pFlt->ncPktData[ i ][ 3 ] &= 0xF0;
					pFlt->ncPktData[ i ][ 3 ] |= cc & 0x0F;
				}
				result = rtfFltQueueData( pFlt, pFlt->ncPktData[ i ] );
				RTF_CHK_RESULT;
			}
			RTF_CHK_RESULT_LOOP;
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

	} while( 0 ); // error escape wrapper - end

	return result;
}

// perform the fixups required by the packets of the current picture
RTF_RESULT rtfFltFixPktArray( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltFixPktArray" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned char cc;

	do {		 // error escape wrapper - begin

		// reset temporal reference
		pFlt->ncFrameTemporalRef = 0;

		// is this the first packet?
		if( pFlt->pktOutCount == 0 )
		{
			// is prefixPSI turned on?
			if( pFlt->trickSpec.prefixPSI != FALSE )
			{
				// yes - insert PAT and PMT packets
				result = rtfFltInsertPSI( pFlt );
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
		// is PSI insertion enabled?
		if( pFlt->trickSpec.insertPSI != FALSE )
		{
			// yes - insert PAT and PMT packets
			result = rtfFltInsertPSI( pFlt );
			RTF_CHK_RESULT;
		}
		// bump the running output access unit count
		++pFlt->accOutCount;
		// bump the temporal reference
		++pFlt->ncFrameTemporalRef;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set up the input bit rate info
RTF_RESULT rtfFltSetInputBitRate( RTF_FLT *pFlt )
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
		pFlt->bitRateSet = TRUE;
		// set the bitrate for the rate controller
		result = rtfFltCtlSetBitRate( pFlt );
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

// set the video frame rate (called during "sniff" phase)
static RTF_RESULT rtfFltSetFrameRate( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltSetFrameRate" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// record the input frame rate in 16 bit fixed point
		// also record the number of PCR clock ticks for one frame time
		switch( pFlt->pProfile->videoSpec.eStream.video.frameRateCode )
		{
		case 1:			// 23.976 fps
			pFlt->picsPerSecFix16 = (unsigned long)( ( 23.976 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 23.976 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 23.976 ) + 0.5 );
			break;
		case 2:			// 24.000 fps
			pFlt->picsPerSecFix16 = (unsigned long)( ( 24.000 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 24.000 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 24.000 ) + 0.5 );
			break;
		case 3:			// 25.000 fps
			pFlt->picsPerSecFix16    = (unsigned long)( ( 25.000 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 25.000 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 25.000 ) + 0.5 );
			break;
		case 4:			// 29.970 fps
			pFlt->picsPerSecFix16    = (unsigned long)( ( 29.970 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 29.970 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 29.970 ) + 0.5 );
			break;
		case 5:			// 30.000 fps
			pFlt->picsPerSecFix16    = (unsigned long)( ( 30.000 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 30.000 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 30.000 ) + 0.5 );
			break;
		case 6:			// 50.000 fps
			pFlt->picsPerSecFix16    = (unsigned long)( ( 50.000 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 50.000 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 50.000 ) + 0.5 );
			break;
		case 7:			// 59.940 fps
			pFlt->picsPerSecFix16    = (unsigned long)( ( 59.940 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 59.940 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 59.940 ) + 0.5 );
			break;
		case 8:			// 60.000 fps
			pFlt->picsPerSecFix16    = (unsigned long)( ( 60.000 * 65536.0 ) + 0.5 );
			pFlt->invPicsPerSecFix16 = (unsigned long)( ( 65536.0 / 60.000 ) + 0.5 );
			pFlt->frameTicks = (int)( ( TRANSPORT_SCR_TICKS_PER_SECOND / 60.000 ) + 0.5 );
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Invalid frame rate code %d",
						  pFlt->pProfile->videoSpec.eStream.video.frameRateCode );
		}
		RTF_CHK_RESULT_LOOP;
		pFlt->frameRateSet = TRUE;
		// give the rate controller a chance to set up for the frame rate also
		rtfFltCtlSetFrameRate( pFlt );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set up the PES header info
RTF_RESULT rtfFltSetPES( RTF_FLT *pFlt )
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
	int i;

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
		// create a captive picture object for the no-change frame
		result = rtfPicConstructor( &pFlt->hNCPic, (RTF_HANDLE)pFlt );
		RTF_CHK_RESULT;
		// create a set of captive packet objects for the no-change frame
		for( i=0; i<RTF_MAX_NCFRAME_PACKETS; ++i )
		{
			result = rtfPktConstructor( &pFlt->hNCPkt[ i ], (RTF_HANDLE)pFlt );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// reset the filter (note: leaves session handle and embedded objects alone)
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
	int i;

	do {		 // error escape wrapper - begin

		// destroy the set of captive packet objects for the no-change frame
		for( i=0; i<RTF_MAX_NCFRAME_PACKETS; ++i )
		{
			result = rtfPktDestructor( pFlt->hNCPkt[ i ] );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// destroy the captive picture object
		result = rtfPicDestructor( pFlt->hNCPic );
		RTF_CHK_RESULT;
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
		pFlt->targetBitRate = ( pSpec->userBitRate != FALSE ) ?
							  pSpec->userBitsPerSecond : RTF_DEFAULT_BITS_PER_SECOND;
		// compute the rate at which to add null packets
		pFlt->forcePadBitRate = 0;
		if( pSpec->forcePadding != FALSE )
		{
			temp64 = pFlt->targetBitRate;
			temp64 *= pSpec->forcePaddingFactorFix8;
			pFlt->forcePadBitRate = (unsigned long)( ( temp64 + 255 ) >> 8 );
		}
		// set a flag if this filter is generating a copy of the main file
		pFlt->mainFileCopy = ( ( pSpec->speedNumerator   == 1 ) &&
							   ( pSpec->speedDenominator == 1 ) &&
							   ( pSpec->speedDirection   == 1 ) ) ? TRUE : FALSE;
		// compute the fixed-point speed ratio (saves divides later)
		pFlt->speedRatioFix16 = ( ( ( (unsigned long)pFlt->trickSpec.speedNumerator ) << 16 ) + ( pFlt->trickSpec.speedDenominator>>1 ) ) / pFlt->trickSpec.speedDenominator;
		// record inverse trick speed in fixed point
		pFlt->invSpeedRatioFix16 = ( pFlt->trickSpec.speedDenominator << 16 ) / pFlt->trickSpec.speedNumerator;
		// open the rate control portion of the filter
		rtfFltCtlOpen( pFlt );
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
			// set up the frame rate
			result = rtfFltSetFrameRate( pFlt );
			RTF_CHK_RESULT;
			// prepare the no change frame
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
		// process the packets to the output
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
	INT64 lastPTS;
	INT64 tempPTS;
	unsigned long firstPktNum;
	int outPointCounter;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_EQ( pFlt, RTF_FLT_STATE_READY );
		// bump the group input count
		++pFlt->gopInCount;
		// record the GOP handle
		pFlt->hGop = hGop;
		// get the picture array info from this group object
		result = rtfGopGetPicArrayInfo( hGop, &pFlt->gopPicCount, &pFlt->phPic );
		RTF_CHK_RESULT;
		// Scan the pictures of the group for out points before processing keyframes.
		// This is so that outpoint info will be available when the index entry is written.
		// NOTE: only do this once per GOP - use filter 1 as the vehicle
		if( pFlt->filterNumber == 1 )
		{
			// get the PTS of the keyframe of this group
			result = rtfPicGetSrcPTS( pFlt->phPic[ 0 ], &lastPTS );
			RTF_CHK_RESULT;
			// search the rest of the group for pictures whose
			// PTS is later than any preceding picture
			outPointCounter = 0;
			for( i=1; i<pFlt->gopPicCount; ++i )
			{
				// get the PTS of the next picture
				result = rtfPicGetSrcPTS( pFlt->phPic[ i ], &tempPTS );
				RTF_CHK_RESULT;
				// is the end of this picture a potential out point?
				if( tempPTS > lastPTS )
				{
					// yes - get the first packet number of this picture
					result = rtfPicGetFirstPktNum( pFlt->phPic[ i ], &firstPktNum );
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
				} // if( tempPTS > lastPTS )
			} // for( i=1; i<pFlt->gopPicCount; ++i )
			RTF_CHK_RESULT_LOOP;
		} // if( pFlt->filterNumber == 1 )
		// process the input GOP through the rate controller
		result = rtfFltCtlProcessGop( pFlt );
		RTF_CHK_RESULT;

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
		int i;
		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		RTF_CHK_STATE_NE( pFlt, RTF_FLT_STATE_CLOSED );

		rtfPicRelease( pFlt->hNCPic, pFlt->hSes );

		for( i=0; i<pFlt->ncFramePacketCount; ++i )
		{
			if (pFlt->hNCPkt[ i ])
			{
				result = rtfPktUnmap( pFlt->hNCPkt[ i ]);
			}
		}

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
		RTF_LOG_INFO6( RTF_MSG_INF_STATS, ">> insert counts: pcr=%d pad=%d pat=%d pmt=%d vid=%d pkt=%d ncf=%d",
					   pFlt->pcrGenCount, pFlt->padGenCount, pFlt->psiGenCount,
					   pFlt->vidGenCount, pFlt->pktGenCount, pFlt->ncfGenCount );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, ">> other counts: pesMod=%d parFirst=%d parLast=%d",
					   pFlt->pesModCount, pFlt->partialFirstCount, pFlt->partialLastCount ); 
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif
