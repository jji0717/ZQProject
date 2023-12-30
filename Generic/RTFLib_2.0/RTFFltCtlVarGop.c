// implementation file for rtfFlt class - rate controller subsystem
// abstracts frame selection and bit rate control functions of output file filter
//

#include "RTFPrv.h"
#include "RTFFltCtlVarGop.h"
#include "RTFFltCtlFixGop.h"
#include "RTFFltPrv.h"

// local functions **********************************************************************

// log an input keyframe
static RTF_RESULT rtfFltCtlVarGopLogInKeyframe( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopLogInKeyframe" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;
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
		pFlt->accTargetPacketCount += ( pFlt->trickSpec.insertPSI == FALSE ) ? 0 : 2;
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
			index = (int)( pFlt->picOutCount & pCtl->maxQueueFramesMask );
			// number of output bits being retained in queue
			retainedOutputBits = pCtl->queueTotalOutputBits - ( pCtl->queueFrameBits[ index ] + pCtl->queueStuffBits[ index ] );
			// number of input bits in queue after update
			updatedInputBits = ( pCtl->queueTotalInputBits - pCtl->queueInputBits[ index ] ) + pCtl->queueNextInputBits;
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
				dTemp -= (double)( pCtl->queueTotalKeyfrBits >> pCtl->maxQueueFramesLog2 );
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
				dTemp *= (double)pFlt->picsPerSecFix16;		// secs * ( pics / sec ) = pics
				pFlt->accTargetPictureCount = ( (int)( dTemp ) ) >> 16; // normalize
			}
#else
			{
				INT64 temp64;
				int targetOutputBits;

				// number of output bits being retained in queue
				retainedOutputBits = pCtl->queueTotalOutputBits - ( pCtl->queueFrameBits[ index ] + pCtl->queueStuffBits[ index ] );
				// number of input bits in queue after update
				updatedInputBits = ( pCtl->queueTotalInputBits - pCtl->queueInputBits[ index ] ) + pCtl->queueNextInputBits;
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
				temp64 -= pCtl->queueLastKeyframeBits;
				// packet count can never be less than the size of the access unit with no stuffing
				pFlt->accTargetPacketCount = MAX( pFlt->accTargetPacketCount, targetOutputPkts );
				// the target picture count is computed by dividing the time consumed
				// by the target packet count by the time consumed by one video frame
				temp64 = pFlt->accTargetPacketCount;		// pkts
				temp64 *= TRANSPORT_PACKET_BITS;			// pkts * bits per pkt = bits
				temp64 <<= 16;								// prescale
				temp64 /= pFlt->pProfile->bitsPerSecond;	// bits / bits per sec = sec
				temp64 >>= 16;								// normalize
				temp64 *= pFlt->picsPerSecFix16;			// sec * frames per sec = frames
				pFlt->accTargetPictureCount = (int)( temp64 >> 16 ); // normalize
			}
#endif
			// never let the targets go negative
			pFlt->accTargetPacketCount  = ( pFlt->accTargetPacketCount  < 0 ) ? 0 : pFlt->accTargetPacketCount;
			pFlt->accTargetPictureCount = ( pFlt->accTargetPictureCount < 0 ) ? 0 : pFlt->accTargetPictureCount;
			pFlt->accTargetPictureCount = MIN( pFlt->accTargetPictureCount, RTF_MAX_TARGET_PICS );
		}
		// copy the packets of the picture to the output
		// and perform fixups on them there
		result = rtfFltFixPktArray( pFlt );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// called after copying a picture (keyframe or no-change frame) to the trick file; update the indexer
static RTF_RESULT rtfFltCtlVarGopLogOutPicture( P_RTF_FLT pFlt, BOOL isKeyframe )
{
	RTF_FNAME( "rtfFltCtlVarGopLogOutPicture" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;
	int i, index, packets, bits;

	do {		 // error escape wrapper - begin

		// get the index of the oldest bitrate queue entry
		index = (int)( pFlt->picOutCount & pCtl->maxQueueFramesMask );
		// subtract the bit count of the output frame and stuffing that are going away
		pCtl->queueTotalOutputBits -= ( pCtl->queueFrameBits[ index ] + pCtl->queueStuffBits[ index ] );
		// subtract the number of input bits that are going away
		pCtl->queueTotalInputBits -= pCtl->queueInputBits[ index ];
		// subtract the number of keyframe bits that are going away
		pCtl->queueTotalKeyfrBits -= pCtl->queueKeyfrBits[ index ];
		// is this a keyframe?
		if( isKeyframe == FALSE )
		{
			// no - update the record of the last no-change frame size
			pCtl->queueLastNCFrameBits = pCtl->queueNextFrameBits;
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
						  ( pCtl->queueNextFrameBits / TRANSPORT_PACKET_BITS );
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
				bits = pCtl->queueNextFrameBits + pCtl->queueStuffBits[ index ];
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
			pCtl->queueLastKeyframeBits = pCtl->queueNextFrameBits;
		} // if( isKeyframe != FALSE )
		// add the new frame to the bit rate control queue
		pCtl->queueInputBits[ index ] = pCtl->queueNextInputBits;
		pCtl->queueFrameBits[ index ] = pCtl->queueNextFrameBits;
		pCtl->queueStuffBits[ index ] = pCtl->queueNextStuffBits;
		pCtl->queueKeyfrBits[ index ] = pCtl->queueNextKeyfrBits;
		// adjust the total input bit count for the queue
		pCtl->queueTotalInputBits += pCtl->queueNextInputBits;
		// adjust the total keyframe bit count for the queue
		pCtl->queueTotalKeyfrBits += pCtl->queueNextKeyfrBits;
		// reset the next input bits counter
		pCtl->queueNextInputBits = 0;
		// adjust the total output bit count for the queue
		pCtl->queueTotalOutputBits += pCtl->queueNextFrameBits + pCtl->queueNextStuffBits;
		// reset the next frame bit counts
		pCtl->queueNextFrameBits = 0;
		pCtl->queueNextStuffBits = 0;
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

// log an output keyframe with the rate controller (final padded frame size is available)
static RTF_RESULT rtfFltCtlVarGopLogOutKeyframe( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopLogOutKeyframe" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;
	int picPkts;
	int nulPkts;
	int copies;
	int bits;
	int i, j;

	do {		 // error escape wrapper - begin

		// record the "naked" keyframe size (no bitrate stuffing added)
		pCtl->queueNextKeyfrBits = pCtl->queueNextFrameBits;
		// should copies of the I-frame be substituted for null padding?
		if( pFlt->trickSpec.dittoFrames != FALSE )
		{
			// yes - get the number of packets occuppied by the picture without any stuffing
			picPkts = (int)( pCtl->queueNextFrameBits / TRANSPORT_PACKET_BITS );
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
					pCtl->queueNextStuffBits -= bits;
					pCtl->queueNextFrameBits += bits;
					// register the previous picture with the indexer
					result = rtfIdxRecordKeyframe( pFlt->hIdx, pFlt->filterNumber,
												   pFlt->pktOutCount, pFlt->pktOutCount );
					RTF_CHK_RESULT;
					// create the next image of the picture in the output
					result = rtfFltFixPktArray( pFlt );
					RTF_CHK_RESULT;
				}
				RTF_CHK_RESULT_LOOP;
			} // if( copies > 1 )
		} // if( pFlt->trickSpec.dittoFrames != FALSE )
		// update the bitrate controller for the index interval
		result = rtfFltCtlVarGopLogOutPicture( pFlt, TRUE );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// decide whether or not to copy the keyframe of the current input GOP to the trick file
static RTF_RESULT rtfFltCtlVarGopGetOutGopDecision( P_RTF_FLT pFlt, BOOL *pIncludeGop )
{
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;
	unsigned long keyRate, ncfRate;
	int keyDiff, ncfDiff;
	int index;

	// NOTE: this routine is called before a new input frame is loaded
	// get index of oldest queue entry
	index = (int)( pFlt->picOutCount & pCtl->maxQueueFramesMask );
#ifdef DO_FLOATING_POINT
	{
		double dInputSeconds;
		double dOutputSeconds;
		double dRetainedOutputBits;

		dInputSeconds = pCtl->queueTotalInputBits;
		dInputSeconds /= pFlt->targetBitRate;
		dOutputSeconds = ( dInputSeconds * pFlt->trickSpec.speedDenominator ) / pFlt->trickSpec.speedNumerator;
		// get the number of output bits retained by queue after update
		dRetainedOutputBits = (double)( pCtl->queueTotalOutputBits - ( pCtl->queueFrameBits[ index ] + pCtl->queueStuffBits[ index ] ) );
		keyRate = (unsigned long)( ( dRetainedOutputBits + pCtl->queueLastKeyframeBits ) / dOutputSeconds );
		ncfRate = (unsigned long)( ( dRetainedOutputBits + pCtl->queueLastNCFrameBits ) / dOutputSeconds );
	}
#else
	{
		UINT64 inputSecondsFix16;
		UINT64 outputSecondsFix16;
		UINT64 retainedOutputBits;

		// bits
		inputSecondsFix16 = pCtl->queueTotalInputBits;
		// pre-scale
		inputSecondsFix16 <<= 16;
		// bits divided by bits per second = seconds
		inputSecondsFix16 /= pFlt->targetBitRate;
		// scale down by trick speed ratio
		outputSecondsFix16 = ( inputSecondsFix16 * pFlt->trickSpec.speedDenominator ) / pFlt->trickSpec.speedNumerator;
		// get the number of output bits retained by queue after update
		retainedOutputBits = pCtl->queueTotalOutputBits - ( pCtl->queueFrameBits[ index ] + pCtl->queueStuffBits[ index ] );
		keyRate = (unsigned long)( ( ( retainedOutputBits + pCtl->queueLastKeyframeBits ) << 16 ) / outputSecondsFix16 );
		ncfRate = (unsigned long)( ( ( retainedOutputBits + pCtl->queueLastNCFrameBits ) << 16 ) / outputSecondsFix16 );
	}
#endif
	keyDiff = (int)keyRate - (int)pFlt->targetBitRate;
	ncfDiff = (int)ncfRate - (int)pFlt->targetBitRate;
	keyDiff = ABS( keyDiff );
	ncfDiff = ABS( ncfDiff );
	// is this the first picture for this filter OR
	// was the last index interval keyframe limited OR
	// does adding a keyframe bring us closer to the target bitrate than a no-change frame? 
	*pIncludeGop = ( ( pFlt->picInCount == 0 ) || ( keyDiff <= ncfDiff ) ) ? TRUE : FALSE;
	return RTF_PASS;
}

// filter a keyframe
static RTF_RESULT rtfFltCtlVarGopProcessKeyframe( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopProcessKeyframe" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	BOOL includeGop;
	unsigned long firstPktCount;

	do {		 // error escape wrapper - begin

		// get the next output bit rate for either a keyframe or an ncframe
		result = rtfFltCtlVarGopGetOutGopDecision( pFlt, &includeGop );
		RTF_CHK_RESULT;
		// is this the first picture for this filter OR
		// was the last index interval keyframe limited OR
		// does adding a keyframe bring us closer to the target bitrate than a no-change frame? 
		if( includeGop != FALSE )
		{
			// yes - wrap an access unit around this keyframe and send it to the output
#if GENERATE_TRACE_FILES
		{
			unsigned long number;
			char buf[ 20 ];
			rtfPicGetNumber( pFlt->hPic[ 0 ], &number );
			sprintf( buf, "%d\n", ( number - pFlt->lastPictureNumber ) );
			pFlt->lastPictureNumber = number;
			_write( pFlt->traceFile, buf, strlen( buf ) );
		}
#endif
			// log the processing of an input keyframe with the rate controller
			result = rtfFltCtlVarGopLogInKeyframe( pFlt );
			RTF_CHK_RESULT;
			RTF_LOG_PIC( "I" );
			// record the output packet count at the start of this interval
			firstPktCount = pFlt->pktOutCount;
			// log the output of a keyframe with the rate controller
			result = rtfFltCtlVarGopLogOutKeyframe( pFlt );
			RTF_CHK_RESULT;
			// register this final picture with the indexer
			result = rtfIdxRecordKeyframe( pFlt->hIdx, pFlt->filterNumber, firstPktCount, pFlt->pktOutCount );
			RTF_CHK_RESULT;
		}
		else
		{
			// no - insert a no-change frame in place of the keyframe
			result = rtfFltInsertNCFrame( pFlt );
			RTF_CHK_RESULT;
			RTF_LOG_PIC( "r" );
			// update the bitrate controller for this picture
			result = rtfFltCtlVarGopLogOutPicture( pFlt, FALSE );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// filter a non-keyframe - output any ECM packets found there
static RTF_RESULT rtfFltCtlVarGopProcessNonKeyframe( RTF_FLT *pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopProcessNonKeyframe" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	unsigned short pid;

	do {		 // error escape wrapper - begin

		// do nothing if the CAT is not valid
		if( pFlt->catValid == FALSE )
		{
			break;
		}
		// get the packet array from the picture
		result = rtfPicGetPacketArrayInfo( pFlt->phPic[ pFlt->gopPicIndex ],
										   &pFlt->pktCount, &pFlt->phPkt,
										   &pFlt->firstBytePktOffset,
										   &pFlt->lastBytePktOffset,
										   &pFlt->lastVideoPacketIndex );
		RTF_CHK_RESULT;
		// loop over the packets in the picture
		for( pFlt->pktIndex=0; pFlt->pktIndex<pFlt->pktCount; ++pFlt->pktIndex )
		{
			// get this packet's PID
			result = rtfPktGetPID( pFlt->phPkt[ pFlt->pktIndex ], &pid );
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

// rate control functions ***************************************************************

// reset the rate controller info (called from constructor)
void rtfFltCtlVarGopReset( P_RTF_FLT pFlt )
{
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;

	pFlt->accTargetPacketCount	= 0;
	pFlt->accTargetPictureCount = 0;
	pCtl->tgtInFrameNumber		= 0;
	pCtl->maxQueueFramesLog2	= RTF_DEFAULT_QUEUE_FRAMES_LOG2;
	pCtl->maxQueueFrames		= ( 1 << RTF_DEFAULT_QUEUE_FRAMES_LOG2 );
	pCtl->maxQueueFramesMask	= ( 1 << RTF_DEFAULT_QUEUE_FRAMES_LOG2 ) - 1;
	pCtl->queueTotalOutputBits	= 0;
	pCtl->queueTotalInputBits	= 0;
	pCtl->queueTotalKeyfrBits	= 0;
	pCtl->queueLastKeyframeBits	= 0;
	pCtl->queueLastNCFrameBits	= 0;
	pCtl->queueNextFrameBits	= 0;
	pCtl->queueNextStuffBits	= 0;
	pCtl->queueNextKeyfrBits	= 0;
	pCtl->queueLastGroupPktNum	= 0;
	// reset some arrays in the control state structure
	memset( (void *)pCtl->queueFrameBits, 0, sizeof(pCtl->queueFrameBits) );
	memset( (void *)pCtl->queueStuffBits, 0, sizeof(pCtl->queueStuffBits) );
	memset( (void *)pCtl->queueInputBits, 0, sizeof(pCtl->queueInputBits) );
	memset( (void *)pCtl->queueKeyfrBits, 0, sizeof(pCtl->queueKeyfrBits) );
}

// initialize the rate controller (trick spec is available)
void rtfFltCtlVarGopOpen( P_RTF_FLT pFlt )
{
	rtfFltCtlVarGopReset( pFlt );
}

// set the bit rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlVarGopSetBitRate( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopSetBitRate" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;
	UINT64 temp64;
	unsigned long queueInputBits;
	unsigned long queueOutputBits;
	unsigned long queueKeyfrBits;
	int i, iToAvg;

	do {		 // error escape wrapper - begin

		// initialize the bitrate queue to "steady state" condition
		// computed from the bitrate to get a smooth start-up
		// set up the size of the bitrate control queue to suite the trick speed being generated
		i = 2 * ( ( pFlt->trickSpec.speedNumerator + pFlt->trickSpec.speedDenominator - 1 ) / pFlt->trickSpec.speedDenominator );
		while( ( pCtl->maxQueueFrames ) < i )
		{
			++pCtl->maxQueueFramesLog2;
			pCtl->maxQueueFrames = ( 1 << pCtl->maxQueueFramesLog2 );
			pCtl->maxQueueFramesMask = pCtl->maxQueueFrames - 1;
		}
		if( pCtl->maxQueueFramesLog2 > RTF_ABSMAX_QUEUE_FRAMES_LOG2 )
		{
			RTF_LOG_WARN3( RTF_MSG_WRN_BADTRICKSPEED, "Requested trick speed (%d/%d) exceeds maximum (%d/1)",
						   pFlt->trickSpec.speedNumerator, pFlt->trickSpec.speedDenominator, RTF_ABSMAX_QUEUE_FRAMES );
			// substitute max values if warning is overridden
			pCtl->maxQueueFramesLog2 = RTF_ABSMAX_QUEUE_FRAMES_LOG2;
			pCtl->maxQueueFrames     = RTF_ABSMAX_QUEUE_FRAMES;
			pCtl->maxQueueFramesMask = RTF_ABSMAX_QUEUE_FRAMES - 1;
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
		temp64 *= pFlt->invPicsPerSecFix16;
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
		pCtl->maxQueueFrames = ( 1 << pCtl->maxQueueFramesLog2 );
		pCtl->maxQueueFramesMask = pCtl->maxQueueFrames - 1;
		// reset the queue
		memset( (void *)pCtl->queueInputBits, 0, sizeof(pCtl->queueInputBits) );
		memset( (void *)pCtl->queueFrameBits, 0, sizeof(pCtl->queueFrameBits) );
		memset( (void *)pCtl->queueStuffBits, 0, sizeof(pCtl->queueStuffBits) );
		memset( (void *)pCtl->queueKeyfrBits, 0, sizeof(pCtl->queueKeyfrBits) );
		// fill the active queue with "average" frames
		// !!! FIX ME !!! REPLICATE SELECTION CYCLE HERE? !!!
		for( i=0; i<pCtl->maxQueueFrames; ++i )
		{
			pCtl->queueInputBits[ i ] = queueInputBits;
			pCtl->queueFrameBits[ i ] = queueOutputBits;
			pCtl->queueStuffBits[ i ] = 0;
			pCtl->queueKeyfrBits[ i ] = queueKeyfrBits;
		}
		pCtl->queueTotalOutputBits	= pCtl->maxQueueFrames * queueOutputBits;
		pCtl->queueTotalInputBits	= pCtl->maxQueueFrames * queueInputBits;
		pCtl->queueTotalKeyfrBits	= pCtl->maxQueueFrames * queueKeyfrBits;
		// set up the initial estimates for the frame sizes to insure
		// that the first keyframe encountered is selected for inclusion
		pCtl->queueLastKeyframeBits = queueOutputBits * iToAvg;
		pCtl->queueLastNCFrameBits  = ( pFlt->trickSpec.insertNCF == FALSE ) ? 0 : RTF_MAX_NCFRAME_BITS;
		pCtl->queueNextInputBits	= 0;
		pCtl->queueNextFrameBits    = 0;
		pCtl->queueNextStuffBits    = 0;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the frame rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlVarGopSetFrameRate( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopSetFrameRate" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;

	do {		 // error escape wrapper - begin

		// nothing to do here?

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process an input GOP through the rate controller
RTF_RESULT rtfFltCtlVarGopProcess( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopProcess" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;
	unsigned long lastPktNum;
	int distThis;
	int distNext;

	do {		 // error escape wrapper - begin

		// start with the first picture of the current GOP
		pFlt->gopPicIndex = 0;
		// get the number of the last packet in this group
		result = rtfGopGetLastPktNum( pFlt->hGop, &lastPktNum );
		RTF_CHK_RESULT;
		pCtl->queueNextInputBits += ( ( lastPktNum - pCtl->queueLastGroupPktNum ) * TRANSPORT_PACKET_BITS );
		pCtl->queueLastGroupPktNum = lastPktNum;
		// see if the target frame is closer to the beginning
		// of the next group than the beginning of this group
		distThis = (int)pCtl->tgtInFrameNumber - (int)pFlt->picInCount;
		distNext = (int)( pFlt->picInCount + pFlt->gopPicCount ) -
				   (int)pCtl->tgtInFrameNumber;
		// is this keyframe closer than the next?
		if( ABS( distThis ) < ABS( distNext ) )
		{
			// yes - process this keyframe
			// note: the keyframe may yet be skipped if there is
			// insufficient output bandwidth to accomodate it
			result = rtfFltCtlVarGopProcessKeyframe( pFlt );
			RTF_CHK_RESULT;
		}
		else
		{
			// no - treat it as a non-keyframe
			// copy any ECM packets it contains to the output, and
			// replace it with a no-chance frame, if appropriate
			result = rtfFltCtlVarGopProcessNonKeyframe( pFlt );
			RTF_CHK_RESULT;
		}
		// advance the input picture counter to the end of the group
		pFlt->picInCount += pFlt->gopPicCount;
		// is there an active conditional access table?
		if( pFlt->catValid != FALSE )
		{
			// yes - iterate over the remaining pictures in the group
			// and output any ECM packets found
			for( ; pFlt->gopPicIndex<pFlt->gopPicCount; ++pFlt->gopPicIndex )
			{
				result = rtfFltCtlVarGopProcessNonKeyframe( pFlt );
				RTF_CHK_RESULT;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// log the output of a packet with the rate controller (allows rate controller to react to PID)
void rtfFltCtlVarGopLogOutPkt( P_RTF_FLT pFlt )
{
	RTF_FLT_CTL_VAR_GOP *pCtl = &pFlt->ctl.var;
	int bits;

	// update the apropriate next frame bit count
	bits = ( pFlt->trickSpec.generateTTS == 0 ) ? TRANSPORT_PACKET_BYTES * 8 : TTS_PACKET_BYTES * 8;
	if( pFlt->outPID == TRANSPORT_PAD_PID )
	{
		pCtl->queueNextStuffBits += bits;
	}
	else
	{
		pCtl->queueNextFrameBits += bits;
	}
}

// update the values of lastPTS and lastDTS for the current picture
RTF_RESULT rtfFltCtlVarGopUpdateTimestamps( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlVarGopUpdateTimestamps" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_TIMESTAMP pcr;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		// compute the current interpolated PCR value
		rtfFltComputeOutputPcr( pFlt, &pcr );
		// the DTS is the PCR plus or minus the decoding delay
		pFlt->lastDTS = pcr.base.ull;
		pFlt->lastDTS += ( pFlt->trickSpec.speedDirection < 0 ) ?
						   -pFlt->decodingDelay : pFlt->decodingDelay;
		// the PTS value is the DTS value plus the presentation delay
		pFlt->lastPTS = pFlt->lastDTS + pFlt->presentationDelay;

	} while( 0 ); // error escape wrapper - end

	return result;
}
