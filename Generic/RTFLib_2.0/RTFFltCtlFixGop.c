// implementation file for rtfFlt class - rate controller subsystem
// abstracts frame selection and bit rate control functions of output file filter
//

#include "RTFPrv.h"
#include "RTFFltCtlVarGop.h"
#include "RTFFltCtlFixGop.h"
#include "RTFFltPrv.h"
#include "RTFLib.h"

#define SYSTEM_CLOCK_FREQUENCY				27000000
#define COMPUTE_PCR(BitRate,ByteOffset)		((((ULONGLONG)ByteOffset) * 8 * SYSTEM_CLOCK_FREQUENCY) / BitRate)

// local functions **********************************************************************

// initialize the state info for the rate controller
// NOTE: assumes trick spec is available and
// that bit rate and frame rate have been set
static void Init( RTF_FLT *pFlt )
{
	RTF_FLT_CTL_FIX_GOP *pCtl = &pFlt->ctl.fix;
	UINT64 temp64;

	// set the number of pictures to include in an output GOP
	pCtl->picsPerOutputGop = DEFAULT_OUT_PICSPERGOP;
	// compute the number of 90kHz clock ticks in a single picture
	temp64 = pFlt->invPicsPerSecFix16;
	temp64 *= TRANSPORT_TS_TICKS_PER_SECOND;
	pCtl->picTicks90 = (ULONG)( temp64 >> 16 );
	// compute the number of packets to wait before inserting a PCR
	temp64 = pFlt->pProfile->bitsPerSecond / TRANSPORT_PACKET_BITS;
	temp64 = (temp64 * 40) / 1000;			// 40 ms
	pCtl->interPcrPktCount = (int)temp64;
	// compute the number of packets to wait before inserting PSI
	temp64 = pFlt->pProfile->bitsPerSecond / TRANSPORT_PACKET_BITS;
	temp64 = (temp64 * 100) / 1000;			// 100 ms
	pCtl->interPSIPktCount = (int)temp64;
	// compute the reference byte offset for the output file
	// NOTE: this is zero for forward or bidirectional files
	// this is the number of packets that would be transmitted
	// in a very large number of frame times for reverse files
	pCtl->refByteOffset = 0;
	if( pFlt->trickSpec.speedDirection < 0 )
	{
		temp64 = RTF_BIG_FRAME_COUNT;				// pictures
		temp64 *= pFlt->invPicsPerSecFix16;			// * secs / picture = seconds
		temp64 *= pFlt->pProfile->bitsPerSecond;	// * bits / sec = bits
		pCtl->refByteOffset = temp64 >> ( 16 + 3 );	// normalize to bytes
	}
	// reset the CC lists
	memset( (void *)pCtl->saveCC, 0, sizeof(pCtl->saveCC) );
	memset( (void *)pCtl->tempCC, 0, sizeof(pCtl->tempCC) );

	rtfFltResetNextCC(pFlt, 0);
	rtfFltResetNextCC(pFlt, pFlt->pProfile->pmtPID);
	rtfFltResetNextCC(pFlt, pFlt->pProfile->videoSpec.pid);
	rtfFltFindCCPID( pFlt, pFlt->pProfile->videoSpec.pid, &pFlt->ccVidIndex );
}

// calculate the total number of bytes in all of the packets of the current
// picture that will be copied to the output file if the picture is used
static RTF_RESULT CalculateFilteredPicturePkts( RTF_FLT *pFlt )
{
	RTF_FNAME( "CalculateFilteredPicturePkts" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_FIX_GOP *pCtl = &pFlt->ctl.fix;
	ULONG flags;
	USHORT pid;
	int index;
	int pmtIndex;
	int patIndex;
	int i;

	int psiPacketCountdown = pFlt->lastPSIPktOutCount - pFlt->pktOutCount + pCtl->interPSIPktCount;
	if (psiPacketCountdown < 0)
	{
		psiPacketCountdown = 1;
	}

	rtfFltFindCCPID( pFlt, 0, &patIndex );
	rtfFltFindCCPID( pFlt, pFlt->pProfile->pmtPID, &pmtIndex );
	// decriment the pCtl->tempCC value for this PID

	do {		 // error escape wrapper - begin
		//
		// reset psi packet counter
		//
		pCtl->psiPktCount = 0;

		// iterate over the packets, count the number that will be copied to the output.
		// if this is a reverse trick file, also calculate the starting CC value that will
		// result in a final CC value that is one less than the first CC of the preceding
		// packet. This so the CCs will be continuous when the file is played backwards.
		// Note: don't change CC or saveCC arrays until we know if the GOP is to be used.
		if( pFlt->trickSpec.speedDirection < 0 )
		{
			memcpy( (void *)pCtl->tempCC, (void *)pCtl->saveCC, sizeof(pCtl->tempCC) );
		}
		pCtl->filteredPicPkts = 0;
		for( i=0; i<pFlt->pktCount; ++i )
		{
			BOOL countPacket = FALSE;
			// see if this packet will be used
			// bump the packet count if yes
			result = rtfPktGetPID( pFlt->phPkt[i], &pid );
			RTF_CHK_RESULT;
			if( pFlt->trickSpec.suppressOTHER == FALSE )
			{
				rtfFltFindExcPID( pFlt, pid, &index );
				if( index < 0 )
				{
					++pCtl->filteredPicPkts;
					countPacket = TRUE;
				}
			}
			else
			{
				rtfFltFindIncPID( pFlt, pid, &index );
				if( index >= 0 )
				{
					++pCtl->filteredPicPkts;
					countPacket = TRUE;
				}
			}
			// is this packet being used, AND is this a reverse trick file?
			if( countPacket != FALSE)
			{
				if (--psiPacketCountdown == 0)
				{
					pCtl->psiPktCount += 2;
					//
					// account for in cc
					//
					--pCtl->tempCC[patIndex];
					--pCtl->tempCC[pmtIndex];
				}

				if (pFlt->trickSpec.speedDirection < 0 )
				{
					// yes - get the flags from this packet 
					result = rtfPktGetFlags( pFlt->phPkt[i], &flags );
					RTF_CHK_RESULT;
					// Count down to the starting CC for the GOP
					// The last CC of this GOP wants to be the first CC of the prior GOP
					// less one. That means that the first CC will be that value minus the
					// number of payload bearing packets from that PID included in this GOP
					if( ( flags & RTF_PKT_PAYLOADABSENT ) == 0 )
					{
						if( pFlt->trickSpec.suppressOTHER != FALSE )
						{
							// see if this PID is on the include list
							rtfFltFindIncPID( pFlt, pid, &index );
							// is it there?
							if( index >= 0 )
							{
								// yes - find the same PID on the CC list
								rtfFltFindCCPID( pFlt, pid, &index );
								// decriment the pCtl->tempCC value for this PID
								--pCtl->tempCC[index];
							}
						}
						else // if( pFlt->trickSpec.suppressOTHER != FALSE )
						{
							// see if this PID is on the exclude list
							rtfFltFindExcPID( pFlt, pid, &index );
							// is it there?
							if( index < 0 )
							{
								// no - find the same PID on the CC list
								rtfFltFindCCPID( pFlt, pid, &index );
								// decriment the pCtl->tempCC value for this PID
								--pCtl->tempCC[index];
							}
						} // if( pFlt->trickSpec.suppressOTHER != FALSE ); else
					} // if( ( flags & RTF_PKT_PAYLOADABSENT ) == 0 )
				} // if( pFlt->trickSpec.speedDirection < 0 )
			}
		} // for( i=0; i<pFlt->pktCount; ++i )
		RTF_CHK_RESULT_LOOP;
		// is this a reverse trick file?
		if( pFlt->trickSpec.speedDirection < 0 )
		{
			// yes - further adjust the saved CCs for any P frames that will be inserted
			// NOTE: all NC picture packets are video packets
			pCtl->tempCC[ pFlt->ccVidIndex ] -= ( ( pCtl->picsPerOutputGop - 1 ) * pFlt->ncFramePacketCount );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// Copy the packets of the current I picture (pFlt->hPic[ 0 ]) to the
// output file. Mix these packets with PSI, PCR, and NULL packets as needed
static RTF_RESULT OutputPictureData( RTF_FLT *pFlt )
{
	RTF_FNAME( "OutputPictureData" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_FIX_GOP *pCtl = &pFlt->ctl.fix;
	ULONG flags;
	ULONG pktCount;
	ULONG currentPktOffset;
	ULONG basePktOffset;
	int filteredPktsCopied = 0;
	int psiInsertedCount = 0;

	do {		 // error escape wrapper - begin

		// get packet flags
		result = rtfPktGetFlags( pFlt->phPkt[ 0 ], &flags );
		RTF_CHK_RESULT;
		// reset the temporal reference count
		pFlt->ncFrameTemporalRef = 0;
		// start on the first packet of the current picture
		pFlt->pktIndex = 0;
		// is PES header insertion enabled?
		// NOTE: PES header insertion doesn't work in reverse trick files
		if( ( pFlt->trickSpec.insertPES != FALSE ) && ( pFlt->trickSpec.speedDirection >= 0 ) )
		{
			// yes - does the first packet of the picture already have a PES header?
			if( ( flags & RTF_PKT_PESHDRPRESENT ) == 0 )
			{
				// no - insert a PES header
				result = rtfFltInsertPesHeader( pFlt );
				RTF_CHK_RESULT;
			} // if( ( flags & RTF_PKT_PESHDRPRESENT ) == 0 )
		} // if( pFlt->trickSpec.insertPES != FALSE )
		basePktOffset = pFlt->accStartPktOutCount;
		currentPktOffset = pFlt->pktOutCount;

		// generate packets until the target byte count for this picture is reached
		while( currentPktOffset-basePktOffset < (ULONG)pCtl->outPicTotalPktsToProduce )
		{
			// compute a byte offset for the next video packet
			INT64 expectedPktOffset = basePktOffset + ( ( filteredPktsCopied * pCtl->outPicTotalPktsToProduce ) / pCtl->filteredPicPkts );
			// if the current byte offset is within 1 packet of the next video packet position
			if( ( filteredPktsCopied == pCtl->filteredPicPkts ) ||
			    ( expectedPktOffset >= currentPktOffset+1 ) )
			{
				BOOL doPSI = pCtl->psiPktCount != 0;
				int pktCount = pFlt->pktOutCount;
				//
				// insert PSI if needed
				//
				if (doPSI)
				{
					result = rtfFltInsertPSI( pFlt );
					RTF_CHK_RESULT;
					psiInsertedCount++;
					pCtl->psiPktCount -= 2;
				}
				else
				{
					// insert a NULL packet
					result = rtfFltInsertStuffingPacket( pFlt );
					RTF_CHK_RESULT;
				}
			}
			else
			{
				if (pFlt->pktIndex == pFlt->pktCount)
				{
					printf("");
				}
				// output the next picture packet
				for( ; pFlt->pktIndex < pFlt->pktCount ; ++pFlt->pktIndex )
				{
					int pktIndex = pFlt->pktIndex;
					// get the current output packet count
					pktCount = pFlt->pktOutCount;
					// process this packet
					result = rtfFltProcessPacket( pFlt );
					RTF_CHK_RESULT;
					// was the packet used?
					if( pktCount != pFlt->pktOutCount )
					{
						// if more than 1 packet results after fixups then account for the new 
						// packet in the saved continuity counter table
						//
						if (pFlt->trickSpec.speedDirection < 0
						&& pFlt->pktOutCount - pktCount > 1)
						{
							--pCtl->saveCC[pFlt->ccVidIndex];
						}

						// yes - advance to the next packet and escape the search loop
						++pFlt->pktIndex;
						//
						// rtfFltProcessPacket may advance the pktIndex to do fixups on
						// headers that span multiple video packets
						// so increment the copied count by the count of packets actually
						// taken
						//
						filteredPktsCopied += pFlt->pktIndex - pktIndex;
						break;
					}
				} // for ...
				RTF_CHK_RESULT_LOOP;
			}
			// advance byte offset
			currentPktOffset = pFlt->pktOutCount;
		} // while ...
#if _DEBUG
		if (filteredPktsCopied != pCtl->filteredPicPkts)
		{
			printf("%d I-Frame bytes out of %d  were copied for gop %d direction %d\n",
				filteredPktsCopied, pCtl->filteredPicPkts, pFlt->gopInCount, pFlt->trickSpec.speedDirection);
		}
#endif
		RTF_CHK_RESULT_LOOP;
		// bump the output picture count
		pFlt->picOutCount++;
		// bump the running output access unit count
		++pFlt->accOutCount;
		// bump the temporal reference count
		++pFlt->ncFrameTemporalRef;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// Copy the packets of the no-change picture (pFlt->hNCPic) to the output file
static RTF_RESULT OutputNCPictureData( RTF_FLT *pFlt )
{
	RTF_FNAME( "OutputNCPictureData" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_FIX_GOP *pCtl = &pFlt->ctl.fix;
	int bytesProduced;
	ULONG baseByteOffset = pFlt->pktOutCount * TRANSPORT_PACKET_BYTES;

	do {		 // error escape wrapper - begin

		// insert the no-change picture
		result = rtfFltInsertNCFrame( pFlt );
		RTF_CHK_RESULT;
		// calculate the number of bytes produced so far
		bytesProduced = pFlt->ncFramePacketCount * TRANSPORT_PACKET_BYTES;
		// generate NULL packets until the target byte count for this picture is reached
		while( bytesProduced + TRANSPORT_PACKET_BYTES <= ( pCtl->outPicTotalPktsToProduce * TRANSPORT_PACKET_BYTES ) )
		{
			// insert a NULL packet
			result = rtfFltInsertStuffingPacket( pFlt );
			RTF_CHK_RESULT;
			bytesProduced += TRANSPORT_PACKET_BYTES;
		} // while ...
		RTF_CHK_RESULT_LOOP;
		// bump the output picture count
		pFlt->picOutCount++;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the parameters that will guide the output of a GOP
static RTF_RESULT SetOutputPictureParameters( RTF_FLT *pFlt )
{
	RTF_FNAME( "SetOutputPictureParameters" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_FIX_GOP *pCtl = &pFlt->ctl.fix;
	UINT64 temp64;
	INT64 fileByteOffset;
	INT64 refFileByteOffset;
	ULONG outPicTimeSlotNum;
	ULONG flags;
	int prefixPsiPkts;
	int normalGopPkts;
	int maxGopPkts;
	int minNullPkts;
	int	ncFramePkts;

	do {		 // error escape wrapper - begin

		// NOTE: PES, PCR, and PSI insertion doesn't really work in reverse trick files
		// calculate the number of packets required by a prefix PSI set, if one is to be generated
//		prefixPsiPkts = ( ( pFlt->gopOutCount == 0 ) && ( pFlt->trickSpec.prefixPSI != FALSE ) ) ? pCtl->psiSetPkts : 0;
		prefixPsiPkts = ( ( pFlt->gopOutCount == 0 ) && ( pFlt->trickSpec.prefixPSI != FALSE ) ) ? pCtl->psiSetPkts : 0;
		// calculate the number of packets that will be consumed by inserted PES headers
		result = rtfPktGetFlags( pFlt->phPkt[0], &flags );
		RTF_CHK_RESULT;
		// add a pes header packet if the current packet doesn't have a pes header or the DSM flags are set
		if (!(flags & RTF_PKT_PESHDRPRESENT ) || (pFlt->trickSpec.insertDSM && flags & RTF_PKT_PESHDRPRESENT) )
		{
			pCtl->pesPackets = 1;
		}
		// calculate no change frame packet count for this GOP
		ncFramePkts = pFlt->ncFramePacketCount * ( pCtl->picsPerOutputGop - 1 );
		// calculate the number of packets required by a suffix PSI set, if one is to be generated
		// calculate the file offset with respect to the reference byte offset
		fileByteOffset = pFlt->pktOutCount * TRANSPORT_PACKET_BYTES;
		refFileByteOffset = fileByteOffset + pCtl->refByteOffset;
		// get the output picture number for the first picture of this output GOP
		outPicTimeSlotNum = ( pFlt->gopInCount - 1 ) * pCtl->picsPerOutputGop;
		// calculate the minimum number of packets that will be consumed
		// by the time the first picture of the GOP is encoded
		pCtl->requiredOutPicContentPkts = prefixPsiPkts + pCtl->filteredPicPkts + pCtl->pesPackets + pCtl->psiPktCount;
		// calculate the minimum number of packets that will
		// be consumed by the time the entire GOP is encoded
		pCtl->requiredOutGopContentPkts = pCtl->requiredOutPicContentPkts + ncFramePkts;
		// get the output file offset that we expect to reach by
		// the time we have finished transmitting this output GOP
		pCtl->expectedGopOffset = outPicTimeSlotNum + pCtl->picsPerOutputGop;
		pCtl->expectedGopOffset *= pFlt->pProfile->bitsPerSecond;
		// divide the expected file offset by the frame rate and convert to a byte count
		pCtl->expectedGopOffset *= pFlt->invPicsPerSecFix16;
		pCtl->expectedGopOffset >>= ( 16 + 3 );
		// the "normal" GOP size is the bitrate divided by the frame rate converted to packets
		temp64 = pFlt->pProfile->bitsPerSecond;
		temp64 *= pFlt->invPicsPerSecFix16;
		temp64 >>= ( 16 + 3 );
		temp64 *= pCtl->picsPerOutputGop;
		temp64 += TRANSPORT_PACKET_BYTES - 1;
		normalGopPkts = (int)( temp64 / TRANSPORT_PACKET_BYTES );
		//
		// the maximum number of bytes for this output GOP is the difference between the 
		// current file offset and the expected final file offset.  A higher maximum size
		// to normal size ratio occurs when an I-Frame is too large to be inserted in the
		// stream - i.e. it's size is greater than the available GOP bytes.  When the I-Frame
		// is skipped, the available GOP size increases by 1 normal GOP size which permits
		// frames larger that the normal GOP size to be inserted.  
		//
		temp64 = pCtl->expectedGopOffset - fileByteOffset;
		temp64 += TRANSPORT_PACKET_BYTES - 1;
		maxGopPkts = (int)( temp64 / TRANSPORT_PACKET_BYTES );
		//
		// for a 3,750,000 bitrate, the normal GOP size is 31250 bytes.  For 31250 byte GOPs, 1/2 second 
		// decoding delay is 7+ GOPs or somewhere between 218,750 and 250,000 bytes (234,375 is 1/2 second 
		// based on bitrate).  The MPML video buffer size is 239376 so the normal GOP size gets 
		// dangerously close to the video buffer size at 1/2 second decoding delay.  This means there must 
		// be a population of null packets to compensate. 57 nulls is enough to keep 8 gops from overflowing
		// which comes out to be about 5 per I-Frame/P-Frame pair which is about 3% nulls.  Add a few more to 
		// cover small errors in the required gop packet content.
		//
		minNullPkts = normalGopPkts / 25;		// 4% nulls
		// are there enough bits in the bandwidth budget to generate this GOP at this time?
//		pCtl->outGopTotalPktsToProduce = maxGopPkts;
		if( maxGopPkts >= pCtl->requiredOutGopContentPkts + minNullPkts )
		{
			// both the -Frame and P-Frame will fit - select the minimum GOP size to use

			int gopPkts = min( maxGopPkts, normalGopPkts );
			// if the minimal size doesn't consume the budget, allow it to use some
			// of the difference between the normal GOP size and the maximum GOP size
			if( gopPkts >= pCtl->requiredOutGopContentPkts + minNullPkts)
			{
				// set the total packets to produce to be the minimum GOP packet size
				temp64 = gopPkts;
			}
			else
			{
				// set the total packets to produce to be the packets required plus the
				// overhead packets i.e. allow it to be larger than "normal" because we
				// have extra capacity
				temp64 = pCtl->requiredOutGopContentPkts;
			}
			
			pCtl->outGopTotalPktsToProduce = (int)temp64 + minNullPkts;

			// compute the share of this GOP's bit budget to devote to the I-Frame picture
			temp64 *= pCtl->requiredOutPicContentPkts;
			temp64 += pCtl->requiredOutGopContentPkts - 1;
			temp64 /= pCtl->requiredOutGopContentPkts;
			pCtl->outPicTotalPktsToProduce = (ULONG)temp64+minNullPkts;
			// ensure there's enough left for any required
			// no-change frames and / or trailing PSI set
			if( ( pCtl->outGopTotalPktsToProduce - pCtl->outPicTotalPktsToProduce ) < ( ncFramePkts) )
			{
				pCtl->outPicTotalPktsToProduce = pCtl->outGopTotalPktsToProduce - ( ncFramePkts);
			}
			// set the VBV delay
#if 0 // !!! FIX ME !!! VBV DELAY? !!!
			{
				UINT64 pcrTS;
				pcrTS = ( refFileByteOffset << 3 ) * TRANSPORT_TS_TICKS_PER_SECOND;
				pcrTS /= pFlt->pProfile->bitsPerSecond;
				pCtl->vbvDelay = (ULONG)( pFlt->lastDTS - pcrTS );
			}
#else
			pCtl->vbvDelay = 0xffff;
#endif
		}
		else // if( pCtl->outGopTotalPktsToProduce >= pCtl->requiredOutGopContentPkts )
		{
			// no - skip this output GOP entirely
			pCtl->outGopTotalPktsToProduce = 0;
			pCtl->outPicTotalPktsToProduce = 0;
			pCtl->vbvDelay = 0xffff;
		} // if( pCtl->outGopTotalPktsToProduce >= pCtl->requiredOutGopContentPkts )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// adjust the output paratmers for a no-change picture
static void SetNCOutputPictureParameters( RTF_FLT *pFlt )
{
	RTF_FLT_CTL_FIX_GOP *pCtl = &pFlt->ctl.fix;
	UCHAR *pHdr;

	// the the packet count for this picture
	pCtl->outPicTotalPktsToProduce = pFlt->ncFramePacketCount;
	// update the timestamps for this picture
	rtfFltCtlFixGopUpdateTimestamps( pFlt );
	// get a pointer to the PES header within the NC picture
	pHdr = &( pFlt->ncPktData[ pFlt->ncFramePesHdrPktIndex ][ pFlt->ncFramePesHdrPktOffset ] );
	// is a PTS present?
	if( ( pHdr[ 7 ] & 0x80 ) != 0 )
	{
		// yes - restamp the PTS
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
		} // if( ( pHdr[ 7 ] & 0x40 ) != 0 )
		// bump the PTS counter
		++pFlt->ptsCount;
	} // if( ( pHdr[ 7 ] & 0x80 ) != 0 )
}

// public functions ***********************************************************

// reset the rate controller info (called from constructor)
void rtfFltCtlFixGopReset( P_RTF_FLT pFlt )
{
	RTF_FLT_CTL_FIX_GOP *pCtl = &pFlt->ctl.fix;

	// these fields are constants
	pCtl->psiSetPkts = 3;
	// these fields are set when the trick spec is available
	pCtl->picsPerOutputGop = DEFAULT_OUT_PICSPERGOP;
	pCtl->interPcrPktCount = DEFAULT_BIT_RATE / ( DEFAULT_PCR_FREQUENCY * 8 * TRANSPORT_PACKET_BYTES );
	pCtl->interPSIPktCount = DEFAULT_BIT_RATE / ( DEFAULT_PSI_FREQUENCY * 8 * TRANSPORT_PACKET_BYTES );
	// these fields get set when the frame rate is established
	pCtl->picTicks90 = (ULONG)( TRANSPORT_TS_TICKS_PER_SECOND / RTF_DEFAULT_FRAME_RATE );
	// these fields change as the stream is processed
	pFlt->lastDTS = 0;
	pFlt->lastPTS = 0;
	pCtl->outGopTotalPktsToProduce = 0;
	pCtl->outPicTotalPktsToProduce = 0;
	pCtl->refByteOffset = 0;
	pCtl->vbvDelay = 0xFFFF;
}

// initialize the rate controller (trick spec is available)
void rtfFltCtlFixGopOpen( P_RTF_FLT pFlt )
{
	// reset the rate controller state info
	rtfFltCtlFixGopReset( pFlt );
}

// set the bit rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlFixGopSetBitRate( P_RTF_FLT pFlt )
{
	// if the frame rate has also been set, initialize the rate controller
	if( pFlt->frameRateSet != FALSE )
	{
		Init( pFlt );
	}
	return RTF_PASS;
}

// set the frame rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlFixGopSetFrameRate( P_RTF_FLT pFlt )
{
	// if the bit rate has also been set, initialize the rate controller
	if( pFlt->bitRateSet != FALSE )
	{
		Init( pFlt );
	}
	return RTF_PASS;
}

// log an input keyframe with the rate controller
RTF_RESULT rtfFltCtlFixGopProcess( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlFixGopProcess" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_FIX_GOP *pCtl;
	INT64 gopPcrBytes;
	BOOL frameSkipped = FALSE;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		pCtl = &pFlt->ctl.fix;
		pFlt->gopPicIndex = 0;
		// compute the number of bytes in the current picture
		// that would be copied to the output if it were used

		result = CalculateFilteredPicturePkts( pFlt );
		RTF_CHK_RESULT;
		// set up to packetize the GOP
		result = SetOutputPictureParameters( pFlt );
		RTF_CHK_RESULT;
		// is this GOP to be used?
		if( pCtl->outGopTotalPktsToProduce != 0 )
		{
			unsigned char patCC, pmtCC;
			int patIndex, pmtIndex;
			//
			// increment gop number
			//
			pFlt->gopOutCount++;

			// yes - is this a reverse trick file?
			if( pFlt->trickSpec.speedDirection < 0 )
			{
				// if PSI is being inserted in this gop
				//
				if (pCtl->psiPktCount != 0)
				{
					// save the current continuity counters for the pat and pmt
					// incase the PSI packets are never inserted
					//
					rtfFltFindCCPID( pFlt, 0, &patIndex );
					patCC = pFlt->nextCC[patIndex];
					rtfFltFindCCPID( pFlt, pFlt->pProfile->pmtPID, &pmtIndex );
					pmtCC = pFlt->nextCC[pmtIndex];
				}

				// set the next CC values for this GOP from the values calculated
				// in CalculateFilteredPicturePkts. This will allow the CC of the last
				// video packet of this GOP to be one less than the first of the prior GOP.

				memcpy( (void*)pFlt->nextCC, (void*)pCtl->tempCC, sizeof(pFlt->nextCC) );
				memcpy( (void*)pCtl->saveCC, (void*)pFlt->nextCC, sizeof(pCtl->saveCC) );

				// calculate the byte count to use in the PCR computation
				if( pCtl->baseGopPcr == 0 )
				{
					// first GOP gets a very large offset that is a frame time multiple
					gopPcrBytes = pCtl->refByteOffset;
				}
				else
				{
					// other frames step backward in time because they will be played in reverse
					gopPcrBytes = -( pCtl->outGopTotalPktsToProduce * TRANSPORT_PACKET_BYTES );
				}
			}
			else // if( pFlt->trickSpec.speedDirection < 0 )
			{
				// calculate the byte count to use in the PCR calculation
				gopPcrBytes = pCtl->lastGopSizeInPackets * TRANSPORT_PACKET_BYTES;
			} // if( pFlt->trickSpec.speedDirection < 0 ); else
			gopPcrBytes *= ( 8 * TRANSPORT_SCR_TICKS_PER_SECOND );
			gopPcrBytes += ( pFlt->pProfile->bitsPerSecond >> 1 );
			pCtl->baseGopPcr += ( gopPcrBytes / pFlt->pProfile->bitsPerSecond );

			// insert a leading PSI set if PSI prefix is enabled AND if this is the first GOP

			if( ( pFlt->gopOutCount == 1 ) && ( pFlt->trickSpec.prefixPSI != FALSE ) )
			{
				result = rtfFltInsertPSI( pFlt );
				RTF_CHK_RESULT;
			}
			// now output the keyframe of this GOP
			result = OutputPictureData( pFlt );
			RTF_CHK_RESULT;
			//
			// restore psi cc values if psi packets were not output
			//
			if (pFlt->trickSpec.speedDirection < 0 && pCtl->psiPktCount != 0)
			{
				pFlt->nextCC[patIndex] = patCC;
				pFlt->nextCC[pmtIndex] = pmtCC;
			}
			// is NC picture insertion enabled?
			if( pFlt->trickSpec.insertNCF != FALSE )
			{
				// yes - insert a set of NC pictures
				for( pFlt->gopPicIndex=1; pFlt->gopPicIndex<pCtl->picsPerOutputGop; ++pFlt->gopPicIndex )
				{
					// adjust the packetize parameters for this no-change picture
					SetNCOutputPictureParameters( pFlt );
					// packetize a no-change picture
					result = OutputNCPictureData( pFlt );
					RTF_CHK_RESULT;
				} // for ...
				RTF_CHK_RESULT_LOOP;
			} // if( pFlt->trickSpec.insertNCF != FALSE )

			// insert NULL packets until the target packet count been achieved
			while( (int)( pFlt->pktOutCount - pFlt->accStartPktOutCount ) < pCtl->outGopTotalPktsToProduce )
			{
				result = rtfFltInsertStuffingPacket( pFlt );
				RTF_CHK_RESULT;
			}
			RTF_CHK_RESULT_LOOP;
			// update the last GOP packet count
			pCtl->lastGopSizeInPackets = pFlt->pktOutCount - pFlt->accStartPktOutCount;
			// bump the output GOP count
#if _DEBUG
			if( ( pCtl->outGopTotalPktsToProduce - pCtl->lastGopSizeInPackets ) != 0 )
			{
				printf( "GOP %d size error: Target=%d pkts, actual=%d pkts, direction=%d\n", 
						pFlt->gopInCount, pCtl->outGopTotalPktsToProduce,
						pFlt->pktOutCount - pFlt->accStartPktOutCount,
						pFlt->trickSpec.speedDirection );
				printf( "expected GOP offset=%I64d, actual GOP offset=%I64d, dif=%d packets\n\n",
						pCtl->expectedGopOffset,
						(INT64)pFlt->pktOutCount * TRANSPORT_PACKET_BYTES,
						pFlt->pktOutCount - (int)( pCtl->expectedGopOffset / TRANSPORT_PACKET_BYTES ) );

				printf("");
			}
#endif
		}
		else // if( pCtl->outGopTotalPktsToProduce != 0 )
		{
			frameSkipped = TRUE;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// log the output of a packet with the rate controller (allows rate controller to react to PID)
void rtfFltCtlFixGopLogOutPkt( P_RTF_FLT pFlt )
{
	// nothing to do here?
}

// update the values of lastPTS and lastDTS for the current picture
RTF_RESULT rtfFltCtlFixGopUpdateTimestamps( P_RTF_FLT pFlt )
{
	RTF_FNAME( "rtfFltCtlFixGopUpdateTimestamps" );
	RTF_OBASE( pFlt );
	RTF_RESULT result = RTF_PASS;
	RTF_FLT_CTL_FIX_GOP *pCtl;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pFlt, RTF_OBJ_TYPE_FLT );
		pCtl = &pFlt->ctl.fix;
		// compute the DTS, based on the number of picture time slots 
		// that have elapsed since the start, plus the decoding delay
		// get the output picture number for the first picture of this output GOP
		pFlt->lastDTS = (pCtl->baseGopPcr / TRANSPORT_SCR_TO_TS_RATIO) / pCtl->picTicks90 * pCtl->picTicks90;
		pFlt->lastDTS += (pFlt->gopPicIndex*pCtl->picTicks90);
		pFlt->lastDTS += pFlt->decodingDelay;
		// the PTS value is the DTS value plus the presentation delay
//		pFlt->lastPTS = pFlt->lastDTS + pFlt->presentationDelay;
		pFlt->lastPTS = pFlt->lastDTS + pCtl->picTicks90;

	} while( 0 ); // error escape wrapper - end

	return result;
}
