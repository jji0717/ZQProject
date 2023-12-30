// implementation file for rtfVcd class MPEG-2 sub-class
// provides MPEG2-specific functions
//

#include "RTFPrv.h"

#ifdef DO_VCD_MPEG2

#include "RTFVcdPrv.h"

// look-up tables ***********************************************************************

// PES header for a zero motion P frame
// NOTE: CONTAINS DSM TRICK MODE FIELD
static const BYTE zeroMotionPPesData[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0xc8, 0x0b, 0x31, 0x00, 0x37, 0xaa, 0x87, 0x11, 0x00,
	 0x37, 0x93, 0x11, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x54, 0xb7, 0xa3, 0x80, 0x00, 0x00, 0x01,
	 0xb5, 0x81, 0x1f, 0xf3, 0x98, 0x00};

// PES header for a zero motion P frame
// NOTE: DOES NOT CONTAIN DSM TRICK MODE FIELD
static const BYTE zeroMotionPPesDataNoDSM[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0xc0, 0x0a, 0x31, 0x00, 0x37, 0xaa, 0x87, 0x11, 0x00,
	 0x37, 0x93, 0x11, 0x00, 0x00, 0x01, 0x00, 0x00, 0x54, 0xb7, 0xa3, 0x80, 0x00, 0x00, 0x01, 0xb5,
	 0x81, 0x1f, 0xf3, 0x98, 0x00};

// PES header for a zero motion B frame
// NOTE: CONTAINS DSM TRICK MODE FIELD
static const BYTE zeroMotionBPesData[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0x80, 0x06, 0x21, 0x00, 0x01, 0x00, 0x01, 0x03, 0x00, 0x00,
	 0x01, 0x00, 0x00, 0x1f, 0xff, 0xfb, 0xb8, 0x00, 0x00, 0x01, 0xb5, 0x81, 0x11, 0x13, 0x98, 0x00};

// PES header for a zero motion B frame
// NOTE: DOES NOT CONTAIN DSM TRICK MODE FIELD
static const BYTE zeroMotionBPesDataNoDSM[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0x80, 0x05, 0x21, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
	 0x01, 0x00, 0x00, 0x1f, 0xff, 0xfb, 0xb8, 0x00, 0x00, 0x01, 0xb5, 0x81, 0x11, 0x13, 0x98, 0x00};

static const BYTE zeroMotionPSlicePrefix[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6C};

static const BYTE zeroMotionBSlicePrefix[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56};

static const BYTE zeroMotionPSliceSuffix[] =
	{0x36};

static const BYTE zeroMotionBSliceSuffix[] =
	{0x2B};

// VLC table for macroblock address increment values
// note: bits are left justified. bottom 4 bits are bit count
static const unsigned char mbaiVLC[][2] =
{
	{ 0x80, 0x01 },		// mbai=1	code=1				bits=1
	{ 0x60, 0x03 },		// mbai=2	code=011			bits=3
	{ 0x40, 0x03 },		// mbai=3	code=010			bits=3
	{ 0x30, 0x04 },		// mbai=4	code=0011			bits=4
	{ 0x20, 0x04 },		// mbai=5	code=0010			bits=4
	{ 0x18, 0x05 },		// mbai=6	code=00011			bits=5
	{ 0x10, 0x05 },		// mbai=7	code=00010			bits=5
	{ 0x0E, 0x07 },		// mbai=8	code=0000111		bits=7
	{ 0x0C, 0x07 },		// mbai=9	code=0000110		bits=7
	{ 0x0B, 0x08 },		// mbai=10	code=00001011		bits=8
	{ 0x0A, 0x08 },		// mbai=11	code=00001010		bits=8
	{ 0x09, 0x08 },		// mbai=12	code=00001001		bits=8
	{ 0x08, 0x08 },		// mbai=13	code=00001000		bits=8
	{ 0x07, 0x08 },		// mbai=14	code=00000111		bits=8
	{ 0x06, 0x08 },		// mbai=15	code=00000110		bits=8
	{ 0x05, 0xCA },		// mbai=16	code=0000010111		bits=10
	{ 0x05, 0x8A },		// mbai=17	code=0000010110		bits=10
	{ 0x05, 0x4A },		// mbai=18	code=0000010101		bits=10
	{ 0x05, 0x0A },		// mbai=19	code=0000010100		bits=10
	{ 0x04, 0xCA },		// mbai=20	code=0000010011		bits=10
	{ 0x04, 0x8A },		// mbai=21	code=0000010010		bits=10
	{ 0x04, 0x6B },		// mbai=22	code=00000100011	bits=11
	{ 0x04, 0x4B },		// mbai=23	code=00000100010	bits=11
	{ 0x04, 0x2B },		// mbai=24	code=00000100001	bits=11
	{ 0x04, 0x0B },		// mbai=25	code=00000100000	bits=11
	{ 0x03, 0xEB },		// mbai=26	code=00000011111	bits=11
	{ 0x03, 0xCB },		// mbai=27	code=00000011110	bits=11
	{ 0x03, 0xAB },		// mbai=28	code=00000011101	bits=11
	{ 0x03, 0x8B },		// mbai=29	code=00000011100	bits=11
	{ 0x03, 0x6B },		// mbai=30	code=00000011011	bits=11
	{ 0x03, 0x4B },		// mbai=31	code=00000011010	bits=11
	{ 0x03, 0x2B },		// mbai=32	code=00000011001	bits=11
	{ 0x03, 0x0B },		// mbai=33	code=00000011000	bits=11
	{ 0x01, 0x0B }		// escape	code=00000001000	bits=11
};

// MPEG-2 specific local functions ********************************************

// translate an MPEG2 start code into a start code type
static RTF_RESULT rtfGetMpeg2StartCodeType( RTF_VCD *pVcd, unsigned long code, RTF_STARTCODE_TYPE_MPEG2 *pType )
{
	RTF_FNAME( "rtfGetMpeg2StartCodeType" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_STARTCODE_TYPE_MPEG2 type = RTF_STARTCODE_TYPE_MPEG2_INVALID;

	do {

		if( ( code & 0xFFFFFF00 ) != TRANSPORT_STARTCODE_BASE )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Invalid MPEG2 start code 0x%x", code );
			break;
		}
		code &= 0xFF;
		if( code == 0 )
		{
			*pType = RTF_STARTCODE_TYPE_MPEG2_PICTURE;
			break;
		}
		if( code < 0xB0 )
		{
			*pType = RTF_STARTCODE_TYPE_MPEG2_SLICE;
			break;
		}
		if( code > 0xBC )
		{
			*pType = RTF_STARTCODE_TYPE_MPEG2_STREAMID;
			break;
		}
		switch( code )
		{
		case 0xB0:
			*pType = RTF_STARTCODE_TYPE_MPEG2_RESERVED0;
			break;
		case 0xB1:
			*pType = RTF_STARTCODE_TYPE_MPEG2_RESERVED1;
			break;
		case 0xB2:
			*pType = RTF_STARTCODE_TYPE_MPEG2_USERDATA;
			break;
		case 0xB3:
			*pType = RTF_STARTCODE_TYPE_MPEG2_SEQHEADER;
			break;
		case 0xB4:
			*pType = RTF_STARTCODE_TYPE_MPEG2_SEQERROR;
			break;
		case 0xB5:
			*pType = RTF_STARTCODE_TYPE_MPEG2_SEQEXTENSION;
			break;
		case 0xB6:
			*pType = RTF_STARTCODE_TYPE_MPEG2_RESERVED2;
			break;
		case 0xB7:
			*pType = RTF_STARTCODE_TYPE_MPEG2_SEQEND;
			break;
		case 0xB8:
			*pType = RTF_STARTCODE_TYPE_MPEG2_GROUP;
			break;
		case 0xB9:
			*pType = RTF_STARTCODE_TYPE_MPEG2_ISOEND;
			break;
		case 0xBA:
			*pType = RTF_STARTCODE_TYPE_MPEG2_PACK;
			break;
		default:
			*pType = RTF_STARTCODE_TYPE_MPEG2_SYSTEM;
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse an MPEG-2 sequence header
static RTF_RESULT rtfParseMpeg2SeqHdr( RTF_VCD *pVcd )
{
	RTF_FNAME( "rtfParseMpeg2SeqHdr" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_SEQ_HANDLE hSeq;
	RTF_PKT_HANDLE hFirstBytePacket;
	unsigned long data;
	unsigned char firstByteOffset;

	do {		 // error escape wrapper - begin

		// start a new sequence in the session
		result = rtfSesStartSeq( pVcd->hSes, FALSE );
		RTF_CHK_RESULT;
		// get the position of the first byte of the start code
		result = rtfWinGetFirstByteInfo( pVcd->hWin, &hFirstBytePacket, &firstByteOffset );
		RTF_CHK_RESULT;
		// record the offset of the sequence header in the packet
		result = rtfPktSetSeqStartOffset( hFirstBytePacket, firstByteOffset, FALSE );
		RTF_CHK_RESULT;
		// advance to the end of the vertical size value field (7th byte)
		result = rtfWinSmallAdvance( pVcd->hWin, 3, &data );
		RTF_CHK_RESULT;
		// extract the vertical and horizontal sizes
		pVcd->pVidSpec->eStream.video.width  = (unsigned short)( data >> 12 ) & 0x0FFF;
		pVcd->pVidSpec->eStream.video.height = ( (unsigned short)data & 0xFFF );
		// advance to the byte containing the end of the bitRate field (11th byte)
		result = rtfWinSmallAdvance( pVcd->hWin, 4, &data );
		RTF_CHK_RESULT;
		// set the frame rate code
		result = rtfSesSetFrameRateCode( pVcd->hSes, (int)( ( data >> 24 ) & 0x0F ) );
		RTF_CHK_RESULT;
		// extract the bit rate value field
		pVcd->pVidSpec->eStream.video.bitsPerSecond = ( ( data >> 6 ) & 0x3FFFF ) * 400;
		// record this info in the sequence object
		result = rtfSesGetSequence( pVcd->hSes, &hSeq );
		RTF_CHK_RESULT;
		result = rtfSeqSetVideoInfo( hSeq, pVcd->pVidSpec->eStream.video.width,
									 pVcd->pVidSpec->eStream.video.height,
									 pVcd->pVidSpec->eStream.video.bitsPerSecond );
		RTF_CHK_RESULT;
		// advance to the byte containing the load_intra_quantizer_matrix flag (12th byte)
		result = rtfWinSmallAdvance( pVcd->hWin, 1, &data );
		RTF_CHK_RESULT;
		// is an intra_quantizer_matrix present?
		if( ( data & 0x0002 ) != 0 )
		{
			// yes - advance past it
			result = rtfWinAdvance( pVcd->hWin, 64, &data );
			RTF_CHK_RESULT;
		}
		// is an non-intra_quantizer_matrix present?
		if( ( data & 0x0001 ) != 0 )
		{
			// yes - advance past it
			result = rtfWinAdvance( pVcd->hWin, 64, &data );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse an MPEG2 extension header
static RTF_RESULT rtfParseMpeg2ExtHdr( RTF_VCD *pVcd )
{
	RTF_FNAME( "rtfParseMpeg2ExtHdr" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hFirstWinPkt;
	unsigned long data;
	unsigned char firstWinOffset;
	BOOL progressiveSeq;

	do {		 // error escape wrapper - begin

		// get the position of the first byte of the extension header start code
		result = rtfWinGetFirstByteInfo( pVcd->hWin, &hFirstWinPkt, &firstWinOffset );
		RTF_CHK_RESULT;
		// advance to the byte containing the extension_start_code_identifier
		result = rtfWinSmallAdvance( pVcd->hWin, 1, &data );
		RTF_CHK_RESULT;
		// decode the extension type
		switch( ( data >> 4 ) & 0x0F )
		{
		case 0x01:
			// record the offset of the sequence extension header in the first packet
			result = rtfPktSetSqxHdrOffset( hFirstWinPkt, firstWinOffset );
			RTF_CHK_RESULT;
			// advance to the byte containing the progressive sequence flag
			result = rtfWinSmallAdvance( pVcd->hWin, 1, &data );
			RTF_CHK_RESULT;
			// record the state of the progressive sequence flag
			progressiveSeq = ( ( data & 0x08 ) != 0 ) ? TRUE : FALSE;
			result = rtfPktSetProgressiveSeq( hFirstWinPkt, progressiveSeq );
			RTF_CHK_RESULT;
			break;
		case 0x08:
			// record the offset of the picture coding extension header in the first packet
			result = rtfPktSetCodHdrOffset( hFirstWinPkt, firstWinOffset );
			RTF_CHK_RESULT;
			break;
		default:
			// not currently interested in other extension headers
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse an MPEG2 picture header
static RTF_RESULT rtfParseMpeg2PicHdr( RTF_VCD *pVcd, unsigned long code )
{
	RTF_FNAME( "rtfParseMpeg2PicHdr" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_GOP_HANDLE hGop;
	RTF_PIC_HANDLE hPic;
	unsigned long pictureCodingType;
	unsigned short temporalReference;
	unsigned long data;
	int picCount;
	int gopCount;

	do {		 // error escape wrapper - begin

		// parse the picture header. get the next 2 bytes
		result = rtfWinSmallAdvance( pVcd->hWin, 2, &data );
		RTF_CHK_RESULT;
		// get the current picture info from the session
		result = rtfSesGetCurrentPicInfo( pVcd->hSes, &hPic, &picCount );
		RTF_CHK_RESULT;
		if( hPic == (RTF_PIC_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No active Picture" );
			break;
		}
		// extract the temporal reference from the picture header
		temporalReference = ( ( (unsigned short) data ) >> 6 ) & 0x3FF;
		// record the temporal reference in the current picture
		result = rtfPicSetMpeg2Info( hPic, temporalReference );
		RTF_CHK_RESULT;
		// parse the picture coding type
		pictureCodingType = ( ( data >> 3 ) & 0x07 );
		// is this is an I-frame?
		if( pictureCodingType == 1 )
		{
			// yes - set the keyframe flag in the current picture
			result = rtfPicSetIsKeyframe( hPic, TRUE );
			RTF_CHK_RESULT;
			RTF_INC_STAT( pVcd->totalKeyframeCount );
			// is this the first picture of the current group?
			if( picCount != 1 )
			{
				// no - tell the session to end this group and start a new virtual group
				// note: pictures don't get added to the group until the picture is closed
				result = rtfSesStartGop( pVcd->hSes, TRUE );
				RTF_CHK_RESULT;
			}
		}
		else
		{
			// not a keyframe - is this the first picture of the group?
			if( picCount == 1 )
			{
				// yes - get the group info from the session
				result = rtfSesGetCurrentGopInfo( pVcd->hSes, &hGop, &gopCount );
				RTF_CHK_RESULT;
				// ignore it if this is the first group, otherwise the group is damaged
				if( gopCount > 1 )
				{
					RTF_LOG_WARN1( RTF_MSG_WRN_FIRSTPICNOTKEY, "First picture not keyframe in group %d", gopCount-1 );
					result = rtfGopSetIsDamaged( hGop, TRUE );
					RTF_CHK_RESULT;
				}
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

static void bitAppendMpeg2( const unsigned char *pSrc, unsigned char *pDst,
					   const unsigned char appendBitCount, int *pTotalBitCount )
{
	int i, totalBitCount;
	int srcByte, srcBit;
	int dstByte, dstBit;
	unsigned char srcTemp, dstTemp, wrkTemp;

	// get the current total bit count
	totalBitCount = *pTotalBitCount;
	// set up the initial source index and mask
	// note: bits are moved starting from the MSB and moving downward 
	// note: the source patterns are stored as left justified longwords
	//       so start from the most significant byte
	srcByte = 0;
	srcBit  = 7;
	// set up the initial destination index and mask
	dstByte = totalBitCount >> 3;
	dstBit  = 7 - ( totalBitCount - ( dstByte << 3 ) );
	// preload the current bytes of the src and dst
	// zero out the dst bits that are to be replaced
	srcTemp = pSrc[ srcByte ];
	dstTemp = pDst[ dstByte ] & ( 0xFE << dstBit );
	// iterate over the input bits and append them to the destination
	for( i=0; i<appendBitCount; ++i )
	{
		// move a bit from srcTemp to dstTemp
		wrkTemp = srcTemp;
		wrkTemp >>= srcBit;
		wrkTemp &= 0x01;
		wrkTemp <<= dstBit;
		dstTemp |= wrkTemp;
		// next bit - watch out for byte boundaries
		if( srcBit-- == 0 )
		{
			srcBit = 7;
			srcTemp = pSrc[ ++srcByte ];
		}
		if( dstBit-- == 0 )
		{
			pDst[ dstByte++ ] = dstTemp;
			dstBit = 7;
			dstTemp = 0;
		}
	}
	// make sure final byte is written
	pDst[ dstByte ] = dstTemp;
	// update the total bit count
	*pTotalBitCount += appendBitCount;
}

static void bitAppendMpeg2MBAI( unsigned char *pDst, int macroblocks, int *pTotalBitCount )
{
	// reduce macroblock count by 1 - (first macroblock doesn't count)
	macroblocks-=2;
	// insert any required macroblock address increment escape codes
	while( macroblocks > 33 )
	{
		bitAppendMpeg2( mbaiVLC[ 33 ], pDst, ( mbaiVLC[ 33 ][ 1 ] & 0x0F ), pTotalBitCount );
		macroblocks -= 33;
	}
	// insert the final mba increment code, if required
	if( macroblocks > 0 )
	{
		bitAppendMpeg2( mbaiVLC[ macroblocks ], pDst, ( mbaiVLC[ macroblocks ][ 1 ] & 0x0F ), pTotalBitCount );
	}
} 

// MPEG-2 VCD specific public functions ***********************************************

// process and MPEG2 start code
RTF_RESULT rtfVcdMpeg2ProcessStartCode( P_RTF_VCD pVcd, unsigned long code )
{
	RTF_FNAME( "rtfProcessMpeg2StartCode" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_STARTCODE_TYPE_MPEG2 type;

	do {		 // error escape wrapper - begin

		// decode the start code type
		result = rtfGetMpeg2StartCodeType( pVcd, code, &type );
		RTF_CHK_RESULT;
		switch( type )
		{
		case RTF_STARTCODE_TYPE_MPEG2_PICTURE:
			result = rtfSesStartPic( pVcd->hSes );
			RTF_CHK_RESULT;
			result = rtfParseMpeg2PicHdr( pVcd, code );
			RTF_CHK_RESULT;
			// reset the boundary info on encountering a picture element
			RTF_VCD_RESET_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_MPEG2_GROUP:
			result = rtfSesStartGop( pVcd->hSes, FALSE );
			RTF_CHK_RESULT;
			// update the boundary info on encountering a non-picture element
			RTF_VCD_UPDATE_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_MPEG2_SEQHEADER:
			result = rtfParseMpeg2SeqHdr( pVcd );
			RTF_CHK_RESULT;
			// update the boundary info on encountering a non-picture element
			RTF_VCD_UPDATE_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_MPEG2_SEQEXTENSION:
			result = rtfParseMpeg2ExtHdr( pVcd );
			RTF_CHK_RESULT;
			// update the boundary info on encountering a non-picture element
			RTF_VCD_UPDATE_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_MPEG2_SEQEND:
			result = rtfSesEndSeq( pVcd->hSes );
			RTF_CHK_RESULT;
			// reset the boundary info on encountering a picture element
			RTF_VCD_RESET_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_MPEG2_SLICE:
			// reset the boundary info on encountering a picture element
			RTF_VCD_RESET_BOUNDARY;
			break;
		default:
			// not currently interested in any other MPEG2 start code types
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// build the MPEG-2 no change frames
void rtfVcdMpeg2BuildNCFrames( P_RTF_VCD pVcd )
{
	int i;
	RTF_ESTREAM_VIDEO *pVStream;
	int pSliceBits, pSliceBytes, pLen = 0;
	int bSliceBits, bSliceBytes, bLen = 0;
	unsigned char pSlice[ MAX_ZSLICE_BYTES ];
	unsigned char bSlice[ MAX_ZSLICE_BYTES ];

	// manually generate the P-frame slice data
	pVStream = &pVcd->pVidSpec->eStream.video;
	// pre-clear the buffer
	memset( pSlice, 0, sizeof(pSlice) );
	// copy in the b-slice prefix
	memcpy( pSlice, zeroMotionPSlicePrefix, ZMPS_PREFIX_BYTE_COUNT );
	pSliceBits = ZMPS_PREFIX_BIT_COUNT;
	// append the necessary macroblock address increment codes
	bitAppendMpeg2MBAI( pSlice, pVStream->width>>4, &pSliceBits );
	// append the p-slice suffix
	bitAppendMpeg2( zeroMotionPSliceSuffix, pSlice, ZMPS_SUFFIX_BIT_COUNT, &pSliceBits );
	// calculate the number of bytes (round up)
	pSliceBytes = ( pSliceBits + 7 ) >> 3;
	// manually generate the B-frame slice data
	// pre-clear the buffer
	memset( bSlice, 0, sizeof(bSlice) );
	// copy in the b-slice prefix
	memcpy( bSlice, zeroMotionBSlicePrefix, ZMBS_PREFIX_BYTE_COUNT );
	bSliceBits = ZMBS_PREFIX_BIT_COUNT;
	// append the necessary macroblock address increment codes
	bitAppendMpeg2MBAI( bSlice, pVStream->width>>4, &bSliceBits );
	// append the b-slice suffix
	bitAppendMpeg2( zeroMotionBSliceSuffix, bSlice, ZMBS_SUFFIX_BIT_COUNT, &bSliceBits );
	// calculate the number of bytes (round up)
	bSliceBytes = ( bSliceBits + 7 ) >> 3;
	if( pVcd->insertDSM == FALSE )
	{
		// setup the pes header and initial mpeg headers
		pLen = sizeof(zeroMotionPPesDataNoDSM);
		memcpy( (void *)pVcd->noChangePFrame, (void *)zeroMotionPPesDataNoDSM, pLen);
		bLen = sizeof(zeroMotionBPesDataNoDSM);
		memcpy( (void *)pVcd->noChangeBFrame, (void *)zeroMotionBPesDataNoDSM, bLen);
	}
	else
	{
		// setup the pes header and initial mpeg headers
		pLen = sizeof(zeroMotionPPesData);
		memcpy( (void *)pVcd->noChangePFrame, (void *)zeroMotionPPesData, pLen);
		bLen = sizeof(zeroMotionBPesData);
		memcpy( (void *)pVcd->noChangeBFrame, (void *)zeroMotionBPesData, bLen);
	}
	// for each verical macroblock (slice)
	for (i = 0 ; i < ( pVStream->height + 15 )/16 ; i++)
	{
		// copy in the slice data
		memcpy(pVcd->noChangePFrame+pLen, pSlice, pSliceBytes);
		memcpy(pVcd->noChangeBFrame+bLen, bSlice, bSliceBytes);
		// adjust the row numbers
		pVcd->noChangePFrame[pLen+3] = i + 1;
		pVcd->noChangeBFrame[bLen+3] = i + 1;
		// bump the lengths
		pLen += pSliceBytes;
		bLen += bSliceBytes;
	}
	// record the length of the no-change frames
	pVcd->noChangePFrameBytes = pLen;
	pVcd->noChangeBFrameBytes = bLen;
	// set the valid flags
	pVcd->noChangePFrameValid = TRUE;
	pVcd->noChangeBFrameValid = TRUE;
}

#endif // #ifdef DO_VCD_MPEG2
