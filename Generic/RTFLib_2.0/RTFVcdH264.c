// implementation file for rtfVcd class H.264 sub-class
// provides H.264-specific functions
//

#include "RTFPrv.h"

#ifdef DO_VCD_H264

#include "RTFVcdPrv.h"

// macros *******************************************************************************

#define UEV_ENCODE( n, pBuf, pBits ) { unsigned long _pat = n+1;  bitAppendH264( _pat, pBuf, uevBits[n], pBits ); }
#define SEV_ENCODE( n, pBuf, pBits ) { unsigned long _pat = (n>0) ? ((n)<<1)+1 : ((-(n)<<1));  bitAppendH264( _pat, pBuf, uevBits[n], pBits ); }
#define GET_N_UBITS( n, v ) \
	{ \
		if( nextBit < (n-1) ) \
		{ \
			result = rtfFillBitBucket( pVcd->hWin, &data, &nextBit ); \
			RTF_CHK_RESULT; \
		} \
		nextBit -= (n); \
		v = ( data>>( nextBit+1 ) ) & ( ( 1<<(n) ) - 1 ); \
	}

// look-up tables ***********************************************************************

// slice type strings
static const char *pSliceTypeStr[] =
	{ "P", "B", "I", "SP", "SI", "P", "B", "I", "SP", "SI" };

// PES header for a zero motion P frame
// NOTE: CONTAINS DSM TRICK MODE FIELD
static const BYTE zeroMotionPPesData[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0xc8, 0x0b, 0x31, 0x00, 0x37, 0xaa, 0x87, 0x11, 0x00, 0x37, 0x93, 0x11, 0x03,
	 0x00, 0x00, 0x01, 0x09, 0x30};

// PES header for a zero motion B frame
// NOTE: CONTAINS DSM TRICK MODE FIELD
// NOTE: contains extra trailing zero bytes to allow for customization
// of picture parameter set number and frame number
static const BYTE zeroMotionBPesData[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0x80, 0x06, 0x31, 0x00, 0x37, 0xaa, 0x87, 0x03,
	 0x00, 0x00, 0x01, 0x09, 0x30};

// PES header for a zero motion P frame
// NOTE: CONTAINS NO DSM TRICK MODE FIELD
static const BYTE zeroMotionPPesDataNoDSM[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0xc8, 0x0a, 0x31, 0x00, 0x37, 0xaa, 0x87, 0x11, 0x00, 0x37, 0x93, 0x11,
	 0x00, 0x00, 0x01, 0x09, 0x30};

// PES header for a zero motion B frame
// NOTE: CONTAINS NO DSM TRICK MODE FIELD
// NOTE: contains extra trailing zero bytes to allow for customization
// of picture parameter set number and frame number
static const BYTE zeroMotionBPesDataNoDSM[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0x80, 0x05, 0x31, 0x00, 0x37, 0xaa, 0x87,
	 0x00, 0x00, 0x01, 0x09, 0x30};

// CABAC init tables - values for m and n
// first index is cabac_init_idc (0-2)
// second index is ctxIdx value (0-39)
static const char cabacInitM[ 3 ][ 40 ] =
{
	{ // cabac_init_idc = 0   
	    20,    2,    3,   20,    2,    3,  -28,  -23,   -6,   -1,    7,
		23,   23,   21,    1,    0,  -37,    5,  -13,  -11,    1,   12,   -4,   17,
		18,    9,   29,   26,   16,    9,  -46,  -20,    1,  -13,  -11,    1,   -6,  -17,   -6,    9
	},
	{ // cabac_init_idc = 1
	    20,    2,    3,   20,    2,    3,  -28,  -23,   -6,   -1,    7,
		22,   34,   16,   -2,    4,  -29,    2,   -6,  -13,    5,    9,   -3,   10,
		26,   19,   40,   57,   41,   26,  -45,  -15,   -4,   -6,  -13,    5,    6,  -13,    0,    8
	},
	{ // cabac_init_idc = 2
	    20,    2,    3,   20,    2,    3,  -28,  -23,   -6,   -1,    7,
		29,   25,   14,  -10,   -3,  -27,   26,   -4,  -24,    5,    6,  -17,   14,
		20,   20,   29,   54,   37,   12,  -32,  -22,   -2,   -4,  -24,    5,   -6,  -14,   -6,    4
	}
};
static const char cabacInitN[ 3 ][ 40 ] =
{
	{ // cabac_init_idc = 0 
	   -15,   54,   74,  -15,   54,   74,  127,  104,   53,   54,   51,
	    33,    2,    0,    9,   49,  118,   57,   78,   65,   62,   49,   73,   50,
		18,    9,   29,   26,   16,    9,  -46,  -20,    1,  -13,  -11,    1,   -6,  -17,   -6,    9
	},
	{ // cabac_init_idc = 1
	   -15,   54,   74,  -15,   54,   74,  127,  104,   53,   54,   51,
		25,    0,    0,    9,   41,  118,   65,   71,   79,   52,   50,   70,   54,
		34,   22,    0,    2,   36,   69,  127,  101,   76,   71,   79,   52,   69,   90,   52,   43
	},
	{ // cabac_init_idc = 2
	   -15,   54,   74,  -15,   54,   74,  127,  104,   53,   54,   51,
		16,    0,    0,   51,   62,   99,   16,   85,  102,   57,   57,   73,   57,
		40,   10,    0,    0,   42,   97,  127,  117,   74,   85,  102,   57,   93,   88,   44,   55
	}
};

// UEV encode table
// index is value to be encoded; table entry is number of bits in uev coded value
// accomodates maximum HD macroblock address (approximately 8K)
// once initialized, table does not change, so it is safe to share.
static unsigned char uevBits[ 0x2000 ];

// H.264 specific local functions *******************************************************

// top up the bit bucket, if necessary
static RTF_RESULT rtfFillBitBucket( RTF_WIN_HANDLE hWin, unsigned long *pData, int *pNextBit )
{
	RTF_FNAME( "rtfFillBitBucket" );
	RTF_OBASE( hWin );
	RTF_RESULT result = RTF_PASS;
	int bytes;

	do {		 // error escape wrapper - begin

		bytes = 3 - ( *pNextBit >> 3 );
		if( bytes > 0 )
		{
			result = rtfWinSmallAdvance( hWin, bytes, pData );
			RTF_CHK_RESULT;
			*pNextBit += ( bytes << 3 );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// decode an unsigned Exp-Golumb bit string
static RTF_RESULT rtfUevDecode( RTF_WIN_HANDLE hWin, unsigned long *pData, int *pNextBit,
							    unsigned long *pValue )
{
	RTF_FNAME( "rtfUevDecode" );
	RTF_OBASE( hWin );
	RTF_RESULT result = RTF_PASS;
	int limit, nextBit;
	int zeroBits, codewordBits;
	unsigned long data, codeWord;
	long lTmp;

	do {		 // error escape wrapper - begin

		// room for more bits in the data register?
		if( *pNextBit < 24 )
		{
			rtfFillBitBucket( hWin, pData, pNextBit );
			RTF_CHK_RESULT;
		}
		// record inputs
		data = *pData;
		nextBit = *pNextBit;
		// limit the number of bits to search
		limit = nextBit >> 1;
		// left justify the coded field in a signed temp
		lTmp = (long)data << (31 - nextBit );
		// count the number of leading zeros
		for( zeroBits=0; zeroBits<limit; ++zeroBits, lTmp <<= 1 )
		{
			if( lTmp < 0 )
			{
				break;
			}
		}
		if( zeroBits > limit )
		{
			// codeword is too big!
			TELL_ME_IF_YOU_GET_HERE();
		}
		// the number of bits in the codeword is 2n + 1
		codewordBits = ( zeroBits << 1 ) + 1;
		// update the next bit number
		*pNextBit = nextBit - codewordBits;
		// remove the leading 1
		lTmp <<= 1;
		// get the codeword less the leading "1"
		codeWord = ( (unsigned long)lTmp ) >> ( 32 - zeroBits );
		// the value of the codeword ((2^zeroBits)-1) + codeWord
		// note: no zeros is a special case
		*pValue = ( zeroBits == 0 ) ? 0 : ( ( 1 << zeroBits ) - 1 ) + codeWord;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// decode a signed Exp-Golumb bit string
static RTF_RESULT rtfSevDecode( RTF_WIN_HANDLE hWin, unsigned long *pData, int *pNextBit,
								long *pValue )
{
	RTF_FNAME( "rtfSevDecode" );
	RTF_OBASE( hWin );
	RTF_RESULT result = RTF_PASS;
	long lTemp;

	do {		 // error escape wrapper - begin

		// decode the field as an unsigned
		result = rtfUevDecode( hWin, pData, pNextBit, (unsigned long *)&lTemp );
		RTF_CHK_RESULT;
		// return codeword converted to signed value
		*pValue = ( ( ( lTemp & 1 ) == 0 ) ? -( lTemp>>1 ) : ( lTemp >> 1 ) );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// append a set of bytes which are right justified in an unsigned long
// MSB first to the end of a buffer, starting at the next byte boundary
static void byteAppendH264( unsigned long src, unsigned char *pDst,
						    unsigned char appendByteCount, unsigned long *pTotalBitCount )
{
	int nextByte, i, shift;

	// get a byte index to the destination (pad to next byte boundary)
	nextByte = ( ( *pTotalBitCount + 0x07 ) >> 3 ) + 1;
	// compute the shift factor for the MSB of the source
	shift = ( ( appendByteCount - 1 ) << 3 );
	for( i = 0; i < appendByteCount; ++i )
	{
		pDst[ nextByte + i ] = (unsigned char)( ( src >> shift ) & 0xFF );
		shift -= 8;
	}
	// return the adjusted bit count
	*pTotalBitCount = ( nextByte + appendByteCount ) << 3;
}

// append a set of bits which are right justified in an unsigned long
// MSB first to the end of a bit buffer
static void bitAppendH264( unsigned long src, unsigned char *pDst,
						   unsigned char appendBitCount, unsigned long *pTotalBitCount )
{
	int i, totalBitCount;
	int srcBit;
	int dstByte, dstBit;
	unsigned char dstTemp;

	// get the current total bit count
	totalBitCount = *pTotalBitCount;
	// set up the initial source index and mask
	// note: bits are moved starting from the MSB and moving downward 
	// note: the source patterns are stored as right justified longwords
	srcBit = appendBitCount - 1;
	// set up the initial destination index and mask
	dstByte = totalBitCount >> 3;
	dstBit  = 7 - ( totalBitCount - ( dstByte << 3 ) );
	// preload the current bytes of the src and dst
	dstTemp = pDst[ dstByte ];
	// iterate over the input bits (low to high) and append them to the destination
	for( i=0; i<appendBitCount; ++i )
	{
		// move a bit from src to dst
		dstTemp &= ~( 1 << dstBit );
		dstTemp |= ( (unsigned char)( ( src >> srcBit-- ) & 0x01 ) << dstBit-- );
		// set up the next bit - watch out for byte boundaries
		if( dstBit < 0 )
		{
			pDst[ dstByte++ ] = dstTemp;
			dstBit = 7;
			dstTemp = pDst[ dstByte ];
		}
	}
	// make sure final byte is written
	if( dstBit != 7 )
	{
		pDst[ dstByte ] = dstTemp;
	}
	// update the total bit count
	*pTotalBitCount += appendBitCount;
}

// check for parameter sets
static RTF_RESULT rtfCheckH264ParamSets( RTF_VCD *pVcd, unsigned long ppsId, BOOL *pNotFound )
{
	RTF_FNAME( "rtfCheckH264ParamSets" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD_PPS_H264 *pPPS;
	RTF_VCD_SPS_H264 *pSPS;
	int i;

	do {		 // error escape wrapper - begin

		// guilty until proven innocent
		*pNotFound = TRUE;
		// search for the referenced picture parameter set
		pPPS = pVcd->vcdInfo.h264.pps;
		for( i=0; i<H264_MAX_PPS_COUNT; ++i, ++pPPS )
		{
			if( ( pPPS->valid ) && ( pPPS->ppsID == ppsId ) )
			{
				break;
			}
		}
		// did we find it?
		if( i >= H264_MAX_PPS_COUNT )
		{
			// no - escape
			break;
		}
		// yes - bump the use count of the referenced PPS
		++pPPS->useCount;
		// also search for the sequence parameter set referenced by the PPS
		pSPS = pVcd->vcdInfo.h264.sps;
		for( i=0; i<H264_MAX_SPS_COUNT; ++i, ++pSPS )
		{
			if( ( pSPS->valid ) && ( pSPS->spsID == pPPS->spsID ) )
			{
				break;
			}
		}
		// did we find it?
		if( i >= H264_MAX_SPS_COUNT )
		{
			// no - escape
			break;
		}
		// bump the use count of the referenced SPS
		++pSPS->useCount;
		// reset the "not found" flag
		*pNotFound = FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse an H.264 acccess unit delimiter
static RTF_RESULT rtfParseH264AccessUnitDelimiter( RTF_VCD *pVcd )
{
	RTF_FNAME( "rtfParseH264AccessUnitDelimiter" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	unsigned long data;
	RTF_VCD_PPT_H264 type;

	do {		 // error escape wrapper - begin

		// get the next byte ( primary_picture_type plus rbsp_trailing_bits() )
		result = rtfWinSmallAdvance( pVcd->hWin, 1, &data );
		RTF_CHK_RESULT;
		// parse the primary picture type
		switch( ( ( data >> 5 ) & 0x07 ) )
		{
		case 0:
			type = RTF_VCD_PPT_H264_I;
			break;
		case 1:
			type = RTF_VCD_PPT_H264_IP;
			break;
		case 2:
			type = RTF_VCD_PPT_H264_IPB;
			break;
		case 3:
			type = RTF_VCD_PPT_H264_SI;
			break;
		case 4:
			type = RTF_VCD_PPT_H264_SISP;
			break;
		case 5:
			type = RTF_VCD_PPT_H264_ISI;
			break;
		case 6:
			type = RTF_VCD_PPT_H264_ISIPSP;
			break;
		case 7:
			type = RTF_VCD_PPT_H264_ISIPSPB;
			break;
		}
		pVcd->vcdInfo.h264.ppTyp[ 1 ] = pVcd->vcdInfo.h264.ppTyp[ 0 ];
		pVcd->vcdInfo.h264.ppTyp[ 0 ] = type;
		pVcd->vcdInfo.h264.isKeyframe[ 1 ] = ( type == RTF_VCD_PPT_H264_I ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse an H.264 slice header
static RTF_RESULT rtfParseH264SliceHdr( RTF_VCD *pVcd )
{
	RTF_FNAME( "rtfParseH264SliceHdr" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD_INFO_H264 *pVcdInfo;
	RTF_SEQ_HANDLE hSeq;
	RTF_GOP_HANDLE hGop;
	RTF_PIC_HANDLE hPic;
	RTF_PKT_HANDLE *phPkt;
	RTF_ESTREAM_VIDEO *pVStream;
	BOOL notFound;
	unsigned long ulTemp;
	unsigned long data;
	unsigned long sliceType;
	int gopCount;
	int nextBit;
	int picCount;
	unsigned short pktCount, lastVidIdx;
	unsigned char firstOff, lastOff;

	do {		 // error escape wrapper - begin

		// generate a convenience pointer to the codec info structure
		pVcdInfo = &pVcd->vcdInfo.h264;
		// and one to the video stream descriptor
		pVStream = &( pVcd->pVidSpec->eStream.video );
		// clone the parsing window so that exploring won't disturb context
		result = rtfWinCopyState( pVcd->hWin, pVcd->hWinTmp );
		RTF_CHK_RESULT;
		// get the next 4 bytes
		result = rtfWinSmallAdvance( pVcd->hWinTmp, 4, &data );
		RTF_CHK_RESULT;
		nextBit = 31;
		// parse the first_mb_in_slice address
		result = rtfUevDecode( pVcd->hWinTmp, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		// parse and record the slice type
		result = rtfUevDecode( pVcd->hWinTmp, &data, &nextBit, &sliceType );
		RTF_CHK_RESULT;
		// parse the pic_parameter_set_id
		result = rtfUevDecode( pVcd->hWinTmp, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		pVcdInfo->ppsID[ 1 ] = pVcdInfo->ppsID[ 0 ];
		pVcdInfo->ppsID[ 0 ] = ulTemp;
		// check for the referenced parameter sets
		result = rtfCheckH264ParamSets( pVcd, pVcdInfo->ppsID[ 0 ], &notFound );
		RTF_CHK_RESULT;
		// do we have them?
		if( notFound != FALSE )
		{
			// no - flush the current content
			result = rtfSesFlush( pVcd->hSes );
			RTF_CHK_RESULT;
			break;
		}
		// parse the frame_number
		result = rtfUevDecode( pVcd->hWinTmp, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		// is frame_mbs_only_flag set in the active sequence parameter set?
		if( pVcdInfo->sps[0].frameMbsOnlyFlag == 0 )
		{
			// no - record the value of field_pic_flag
			GET_N_UBITS( 1, pVcdInfo->fieldPicFlag );
			// was field_pic_flag set?
			if( pVcdInfo->fieldPicFlag != FALSE )
			{
				// no - record the value of bottom_field_flag
				GET_N_UBITS( 1, pVcdInfo->bottomFieldFlag );
			}
		}
		pVcdInfo->frNum[ 1 ] = pVcdInfo->frNum[ 0 ];
		pVcdInfo->frNum[ 0 ] = ulTemp;
		// is this a new picture?
		if( ( pVcdInfo->frNum[ 0 ] != pVcdInfo->frNum[ 1 ] ) ||
			( pVcdInfo->ppTyp[ 0 ] != pVcdInfo->ppTyp[ 1 ] ) || 
			( pVcdInfo->ppsID[ 0 ] != pVcdInfo->ppsID[ 1 ] ) ) 
		{
			// yes - advance all of the above indicators so as not to get false positives
			pVcdInfo->frNum[ 1 ] = pVcdInfo->frNum[ 0 ];
			pVcdInfo->ppTyp[ 1 ] = pVcdInfo->ppTyp[ 0 ];
			pVcdInfo->ppsID[ 1 ] = pVcdInfo->ppsID[ 0 ];
			// advance the keyframe flags
			pVcdInfo->isKeyframe[ 0 ] = pVcdInfo->isKeyframe[ 1 ];
			pVcdInfo->isKeyframe[ 1 ] = FALSE;
			// get the current picture info from the session
			result = rtfSesGetCurrentPicInfo( pVcd->hSes, &hPic, &picCount );
			RTF_CHK_RESULT;
			// is there a current picture?
			if( hPic != (RTF_PIC_HANDLE)NULL )
			{
				// yes - get the packet array info from the current picture
				result = rtfPicGetPacketArrayInfo( hPic, &pktCount, &phPkt, &firstOff, &lastOff, &lastVidIdx );
				RTF_CHK_RESULT;
				// are there already packets in the current picture?
				if( pktCount > 0 )
				{
					// yes - close the current picture; add it to the current group
					result = rtfSesEndPic( pVcd->hSes );
					RTF_CHK_RESULT;
				}
			}
			// is the new frame a keyframe?
			if( pVcdInfo->isKeyframe[ 0 ] != FALSE )
			{
				// yes - tell the session to start a new sequence
				// back where we started exploring
				result = rtfSesStartSeq( pVcd->hSes, TRUE );
				RTF_CHK_RESULT;
				// record some info in the sequence object
				result = rtfSesGetSequence( pVcd->hSes, &hSeq );
				RTF_CHK_RESULT;
				result = rtfSeqSetVideoInfo( hSeq, pVStream->width, pVStream->height,
											 pVStream->bitsPerSecond );
				RTF_CHK_RESULT;
			}
			// tell the session to start a new picture
			// back where we started exploring
			result = rtfSesStartPic( pVcd->hSes );
			RTF_CHK_RESULT;
			// get the updated current picture info from the session
			result = rtfSesGetCurrentPicInfo( pVcd->hSes, &hPic, &picCount );
			RTF_CHK_RESULT;
			if( hPic == (RTF_PIC_HANDLE)NULL )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No active Picture" );
				break;
			}
			// record some info in the new picture object
			result = rtfPicSetH264Info( hPic, pVcdInfo->isKeyframe[ 0 ], pVcdInfo->ppsID[ 0 ], pVcdInfo->frNum[ 0 ] );
			RTF_CHK_RESULT;
			// is this a keyframe?
			if( pVcdInfo->isKeyframe[ 0 ] == FALSE )
			{
				// no - not a keyframe - is this the first picture of a group?
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
				} // if( picCount == 1 )
			}
			else
			{
				// yes - bump the keyframe counter
				RTF_INC_STAT( pVcd->totalKeyframeCount );
			}  // if( pVcdInfo->isKeyframe[ 0 ] == FALSE ) ; else
		} // if( ( pVcdInfo->frNum[ 0 ] != pVcdInfo->frNum[ 1 ] ) || ...
		// is this a non-intra coded slice in an intra coded picture?
		if( ( pVcdInfo->isKeyframe[ 0 ] != FALSE  ) &&
			( sliceType != RTF_SLICE_TYPE_H264_I  ) &&
			( sliceType != RTF_SLICE_TYPE_H264_I2 ) )
		{
			// get the current picture info from the session
			result = rtfSesGetTotalPicCount( pVcd->hSes, (unsigned long *)&picCount );
			RTF_CHK_RESULT;
			RTF_LOG_WARN2( RTF_MSG_WRN_BADSLICETYPE,
					"Non-intra slice type %s found in intra-coded in picture %d",
					pSliceTypeStr[ sliceType ], picCount );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse an H.264 sequence parameter set
static RTF_RESULT rtfParseH264SPS( RTF_VCD *pVcd )
{
	RTF_FNAME( "rtfParseH264SPS" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD_SPS_H264 *pSPS;
	unsigned char *pPkt;
	unsigned char *pSrc;
	unsigned char *pDst;
	unsigned long data;
	unsigned long ulTemp;
	unsigned long frameHeightInMbs;
	long lTemp;
	int i, frc;
	int nextBit;
	int byteCount;
	unsigned char adaptLen;

	do {		 // error escape wrapper - begin

		// record the presence of an SPS - marks next frame as a keyframe
		pVcd->vcdInfo.h264.isKeyframe[ 1 ] = TRUE;
		// scan the SPS array - try to find an empty entry (not valid),
		// or one that was active (i.e. valid and has a non-zero use count)
		pSPS = pVcd->vcdInfo.h264.sps;
		for( i=0; i<H264_MAX_SPS_COUNT; ++i, ++pSPS )
		{
			if( pSPS->valid == FALSE )
			{
				break;
			}
			else if( pSPS->useCount > 0 )
			{
				break;
			}
		}
		// did we find one?
		if( i >= H264_MAX_SPS_COUNT )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_OVERFLOW, "Sequence parameter set array overflow" );
			break;
		}
		// got one. start recording
		result = rtfWinStartRecord( pVcd->hWin, pSPS->pkt, TRANSPORT_MAX_PAYLOAD_BYTES+3 );
		RTF_CHK_RESULT;
		// skip the start code; get the next 4 bytes beyond that
		result = rtfWinAdvance( pVcd->hWin, 4, &data );
		RTF_CHK_RESULT;
		// prime the bit-field parsing pump
		nextBit = 31;
		// get the profile_idc field
		GET_N_UBITS( 8, ulTemp );
		// get the 3 constraint_set_flags and the 5 reserved bits
		GET_N_UBITS( 8, ulTemp );
		// get the level_idc field
		GET_N_UBITS( 8, ulTemp );
		// parse and record the seq_parameter_set_id field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pSPS->spsID );
		RTF_CHK_RESULT;
		// parse the log2_max_frame_num_minus4 field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		pSPS->log2MaxFrameNum = ulTemp + 4;
		// parse the pic_order_cnt_type field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pSPS->picOrderCntType );
		RTF_CHK_RESULT;
		if( pSPS->picOrderCntType == 0 )
		{
			// parse the log2_max_pic_order_cnt_lsb_minus4 field
			result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
			RTF_CHK_RESULT;
			pSPS->log2MaxPicOrderCntLsb = ulTemp + 4;
		}
		else if( pSPS->picOrderCntType == 1 )
		{
			// record the delta_pic_order_always_zero_flag bit
			GET_N_UBITS( 1, pSPS->deltaPicOrderAlwaysZeroFlag );
			// parse the offset_for_non_ref_pic field
			result = rtfSevDecode( pVcd->hWin, &data, &nextBit, &lTemp );
			RTF_CHK_RESULT;
			// parse the offset_for_top_to_bottom field
			result = rtfSevDecode( pVcd->hWin, &data, &nextBit, &lTemp );
			RTF_CHK_RESULT;
			// parse the num_ref_frames_in_pic_order_cnt_cycle field
			result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
			RTF_CHK_RESULT;
			// parse the offset_for_ref_frame fields
			for( i=0; i<(int)ulTemp; ++i )
			{
				result = rtfSevDecode( pVcd->hWin, &data, &nextBit, &lTemp );
				RTF_CHK_RESULT;
			}
			RTF_CHK_RESULT_LOOP;
		}
		// parse the num_ref_frames field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		// get the gaps_in_frame_num_value_allowed_flag field
		GET_N_UBITS( 1, ulTemp );
		// parse the pic_width_in_mbs_minus1 field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		// record picWidthInMbs
		pSPS->picWidthInMbs = ulTemp + 1;
		// record the picture width
		pSPS->picWidth = ( ulTemp + 1 ) * H264_MB_PIXEL_WIDTH;
		// parse the pic_height_in_map_units
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		// record pic height in map units
		pSPS->picHeightInMapUnits = ulTemp + 1;
		// record the frame_mbs_only_flag field
		GET_N_UBITS( 1, pSPS->frameMbsOnlyFlag );
		// compute frame height in macroblocks
		frameHeightInMbs = ( pSPS->frameMbsOnlyFlag == FALSE ) ? pSPS->picHeightInMapUnits<<1 : pSPS->picHeightInMapUnits;
		// record the pic height in macroblocks
		pSPS->picHeightInMbs = ( pVcd->vcdInfo.h264.fieldPicFlag == 0 ) ? frameHeightInMbs : frameHeightInMbs>>1;
		// record pic height
		pSPS->picHeight = pSPS->picHeightInMbs * H264_MB_PIXEL_HEIGHT;
		// record pic size in macroblocks
		pSPS->picSizeInMbs = pSPS->picWidthInMbs * pSPS->picHeightInMbs;
		// is the frame_mbs_only_flag field set?
		if( pSPS->frameMbsOnlyFlag == 0 )
		{
			// no - record the mb_adaptive_frame_field_flag
			GET_N_UBITS( 1, pSPS->mbAdaptiveFrameFieldFlag );
		}
		// get the direct_8x8_inference_flag
		GET_N_UBITS( 1, ulTemp );
		// get the frame_cropping_flag
		GET_N_UBITS( 1, ulTemp );
		// is the frame_cropping_flag set?
		if( ulTemp != 0 )
		{
			// yes - get the frame_crop_left_offset
			result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
			RTF_CHK_RESULT;
			// get the frame_crop_right_offset
			result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
			RTF_CHK_RESULT;
			// get the frame_crop_top_offset
			result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
			RTF_CHK_RESULT;
			// get the frame_crop_bottom_offset
			result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
			RTF_CHK_RESULT;
		}
		// get the vui_parameters_present_flag
		GET_N_UBITS( 1, ulTemp );
		// is the vui_parameters_present_flag set?
		if( ulTemp != 0 )
		{
			// yes - get the aspect_ratio_info_present_flag
			GET_N_UBITS( 1, ulTemp );
			// is the aspect_ratio_info_present_flag set?
			if( ulTemp != 0 )
			{
				// yes - get the aspect_ratio_idc field
				GET_N_UBITS( 8, ulTemp );
				// extended aspect ratio mode?
				if( ulTemp == H264_EXTENDED_SAR )
				{
					// yes - get the sar_width field
					GET_N_UBITS( 16, ulTemp );
					// get the sar_height field
					GET_N_UBITS( 16, ulTemp );
				}
			}
			// get the overscan_info_present_flag
			GET_N_UBITS( 1, ulTemp );
			// is the overscan_info_present_flag set?
			if( ulTemp != 0 )
			{
                // yes - get the overscan_appropriate_flag				}
				GET_N_UBITS( 1, ulTemp );
			}
			// get the video_signal_type_present_flag
			GET_N_UBITS( 1, ulTemp );
			// is the video_signal_type_present_flag set?
			if( ulTemp != 0 )
			{
				// yes - get the video_format field
				GET_N_UBITS( 3, pSPS->videoFormat );
				// decode the video format to get the implied frame rate
				switch( pSPS->videoFormat )
				{
				case 1:		// PAL = 25.00 fps
				case 3:		// SECAM = 25.00 fps
					frc = 3;
					break;
				case 2:		// NTSC = 29.97 fps
					frc = 4;
					break;
				case 4:		// HD-MAC = 50 fps
					frc = 6;
					break;
				case 0:		// component
				case 5:		// unspecified
				default:	// reserved
					frc = 0;
					break;
				}
				// was a frame rate code recognized?
				if( frc != 0 )
				{
					// yes - record it in the profile
					result = rtfSesSetFrameRateCode( pVcd->hSes, frc );
					RTF_CHK_RESULT;
				}
				// get the video_full_range_flag
				GET_N_UBITS( 1, ulTemp );
				// get the colour_description_present_flag
				GET_N_UBITS( 1, ulTemp );
				// is the colour_description_present_flag set?
				if( ulTemp != 0 )
				{
					// yes - get the colour_primaries field
					GET_N_UBITS( 8, ulTemp );
					// get the transfer_characteristics field
					GET_N_UBITS( 8, ulTemp );
					// get the matrix_coefficients field
					GET_N_UBITS( 8, ulTemp );
				}
			}
			// get the chroma_loc_info_present_flag
			GET_N_UBITS( 1, ulTemp );
			// is the chroma_loc_info_present_flag set?
			if( ulTemp != 0 )
			{
				// yes - get the chroma_sample_loc_type_top_field
				result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
				RTF_CHK_RESULT;
				// get the chroma_sample_loc_type_bottom_field
				result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
				RTF_CHK_RESULT;
			}
			// get the timing_info_present_flag (finally!!!)
			GET_N_UBITS( 1, ulTemp );
			// is the timing_info_present_flag set?
			if( ulTemp != 0 )
			{
				unsigned long numUnitsInTick;
				unsigned long timeScale;
				unsigned long fixedFrameRate;
				int frameRateFix8, frc;

				// yes - get the num_units_in_tick field (32 bits)
				GET_N_UBITS( 16, numUnitsInTick );
				GET_N_UBITS( 16, ulTemp );
				numUnitsInTick = ( numUnitsInTick << 16 ) | ulTemp;
				// get the time_scale field (32 bits)
				GET_N_UBITS( 16, timeScale );
				GET_N_UBITS( 16, ulTemp );
				timeScale = ( timeScale << 16 ) | ulTemp;
				// get the fixed_frame_rate_flag
				GET_N_UBITS( 1, fixedFrameRate );
				// compute the frame rate (note: assume frame pictures, not field pictures)
				frameRateFix8 = ( ( ( timeScale << 8 ) + numUnitsInTick ) >> 1 )  / numUnitsInTick;
				// translate to a frame rate code
				switch( frameRateFix8 )
				{
				case ( (int)( 23.976 * 256.0 + 0.5) ):
					frc = 1;
					break;
				case ( (int)( 24.000 * 256.0 + 0.5 ) ):
					frc = 2;
					break;
				case ( (int)( 25.000 * 256.0 + 0.5 ) ):
					frc = 3;
					break;
				case ( (int)( 29.970 * 256.0 + 0.5 ) ):
					frc = 4;
					break;
				case ( (int)( 30.000 * 256.0 + 0.5 ) ):
					frc = 5;
					break;
				case ( (int)( 50.000 * 256.0 + 0.5 ) ):
					frc = 6;
					break;
				case ( (int)( 59.940 * 256.0 + 0.5 ) ):
					frc = 7;
					break;
				case ( (int)( 60.000 * 256.0 + 0.5 ) ):
					frc = 8;
					break;
				default:
					frc = 0;
					break;
				}
				if( frc != 0 )
				{
					// set the video frame rate code
					result = rtfSesSetFrameRateCode( pVcd->hSes, frc );
					RTF_CHK_RESULT;
				}
			}
			// not interested in the rest of the VUI
		}
		// not interested in the rest of the SPS
		// set the bit rate
		result = rtfSesGetInputBitrate( pVcd->hSes, &pVcd->pVidSpec->eStream.video.bitsPerSecond );
		RTF_CHK_RESULT;
		// we have what we need. Fast forward to the next start code
		while( ( data & 0xFFFFFF ) != 1 )
		{
			result = rtfWinSmallAdvance( pVcd->hWin, 1, &data );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// the first byte of the window is the last byte of the SPS, so stop recording
		result = rtfWinStopRecord( pVcd->hWin, &byteCount );
		RTF_CHK_RESULT;
		// byte count includes the next start code prefix, so adjust it.
		byteCount -= 3;
		// make a packet out of the recorded SPS NAL unit
		// copy the NAL unit to the end of the packet buffer
		pPkt = pSPS->pkt;
		pSrc = pPkt + byteCount - 1;
		pDst = pPkt + TRANSPORT_PACKET_BYTES - 1;
		for( i=0; i<byteCount; ++i )
		{
			*pDst-- = *pSrc--;
		}
		// compute the size of the adaptation field
		// required to fill the rest of the packet
		adaptLen = TRANSPORT_PACKET_BYTES - ( byteCount + 4 );
		// set up the packet header
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( pVcd->pVidSpec->pid >> 8 ) & 0xFF );
		pPkt[ 2 ] = (unsigned char)( pVcd->pVidSpec->pid & 0xFF );
		if( adaptLen == 0 )
		{
			pPkt[ 3 ] = 0x10;
		}
		else
		{
			pPkt[ 3 ] = 0x30;
			pPkt[ 4 ] = adaptLen - 1;
			if( adaptLen > 1 )
			{
				pPkt[ 5 ] = 0x00;
				for( i=5; i<(TRANSPORT_PACKET_BYTES-byteCount); ++i )
				{
					pPkt[ i ] = 0xFF;
				}
			}
		}
		pSPS->valid = TRUE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse an H.264 picture parameter set
static RTF_RESULT rtfParseH264PPS( RTF_VCD *pVcd )
{
	RTF_FNAME( "rtfParseH264PPS" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_VCD_PPS_H264 *pPPS;
	long lTemp;
	unsigned char *pPkt;
	unsigned char *pSrc;
	unsigned char *pDst;
	unsigned long data;
	unsigned long ulTemp;
	unsigned long picSizeInMapUnitsMinus1;
	int i, j;
	int nextBit;
	int byteCount;
	int sliceGroupIdBits;
	unsigned char adaptLen;

	do {		 // error escape wrapper - begin

		// scan the PPS array - try to find an empty entry (not valid),
		// or one that was active (i.e. valid and has a non-zero use count)
		pPPS = pVcd->vcdInfo.h264.pps;
		for( i=0; i<H264_MAX_PPS_COUNT; ++i, ++pPPS )
		{
			if( pPPS->valid == FALSE )
			{
				break;
			}
			else if( pPPS->useCount > 0 )
			{
				break;
			}
		}
		// did we find one?
		if( i >= H264_MAX_PPS_COUNT )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_OVERFLOW, "Picture parameter set array overflow" );
			break;
		}
		// got one. start recording
		result = rtfWinStartRecord( pVcd->hWin, pPPS->pkt, TRANSPORT_MAX_PAYLOAD_BYTES+3 );
		RTF_CHK_RESULT;
		// skip the start code - get the 4 bytes beyond that
		result = rtfWinAdvance( pVcd->hWin, 4, &data );
		RTF_CHK_RESULT;
		nextBit = 31;
		// parse and record the pic_parameter_set_id field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pPPS->ppsID );
		RTF_CHK_RESULT;
		// parse and record the seq_parameter_set_id field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pPPS->spsID );
		RTF_CHK_RESULT;
		// record entropy_coding_mode_flag field
		GET_N_UBITS( 1, pPPS->entropyCodingModeFlag );
		// record pic_order_present_flag field
		GET_N_UBITS( 1, pPPS->picOrderPresentFlag );
		// record num_slice_groups_minus_1 field
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pPPS->numSliceGroupsMinus1 );
		RTF_CHK_RESULT;
		if( pPPS->numSliceGroupsMinus1 > 0 )
		{
			// record slice_group_map_type
			result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pPPS->sliceGroupMapType );
			RTF_CHK_RESULT;
			switch( pPPS->sliceGroupMapType )
			{
			case 0:
				// iterate over the group run lengths
				for( i=0; i<=(int)pPPS->numSliceGroupsMinus1; ++i )
				{
					// parse the run length for this group
					result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
					RTF_CHK_RESULT;
				}
				break;
			case 2:
				// iterate over the group corners
				for( i=0; i<(int)pPPS->numSliceGroupsMinus1; ++i )
				{
					// parse top_left for this group
					result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
					RTF_CHK_RESULT;
					// parse bottom_right for this group
					result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
					RTF_CHK_RESULT;
				}
				break;
			case 3:
			case 4:
			case 5:
				// skip slice_group_change_direction_flag field
				GET_N_UBITS( 1, ulTemp );
				// record slice_group_change_rate_minus_1
				result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pPPS->sliceGroupChangeRate );
				RTF_CHK_RESULT;
				++pPPS->sliceGroupChangeRate;
				break;
			case 6:
				// record pic_size_in_map_units_minus1
				result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &picSizeInMapUnitsMinus1 );
				RTF_CHK_RESULT;
				sliceGroupIdBits = 0;
				ulTemp = picSizeInMapUnitsMinus1 + 1;
				while( ulTemp != 0 )
				{
					ulTemp >>= 1;
					++sliceGroupIdBits;
				}
				// iterate over the map units
				for( i=0; i<=(int)picSizeInMapUnitsMinus1; ++i )
				{
					for( j=0; j<(int)sliceGroupIdBits; ++j )
					{
						GET_N_UBITS( 1, ulTemp );
					}
				}
				break;
			default:
				break;
			}
		}
		RTF_CHK_RESULT_LOOP;
		// record num_ref_idx_l0_active_minus1
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &pPPS->numRefIdxL0ActiveMinus1 );
		RTF_CHK_RESULT;
		// parse num_ref_idx_l1_active_minus1
		result = rtfUevDecode( pVcd->hWin, &data, &nextBit, &ulTemp );
		RTF_CHK_RESULT;
		// record the weighted_pred_flag
		GET_N_UBITS( 1, pPPS->weighted_pred_flag );
		// skip weighted_bipred_idc
		GET_N_UBITS( 2, ulTemp );
		// parse pic_init_qp_minus26
		result = rtfSevDecode( pVcd->hWin, &data, &nextBit, &lTemp );
		RTF_CHK_RESULT;
		// parse pic_init_qs_minus26
		result = rtfSevDecode( pVcd->hWin, &data, &nextBit, &lTemp );
		RTF_CHK_RESULT;
		// parse chroma_qp_index_offset
		result = rtfSevDecode( pVcd->hWin, &data, &nextBit, &lTemp );
		RTF_CHK_RESULT;
		// record deblocking_filter_control_present_flag
		GET_N_UBITS( 1, pPPS->deblockingFilterControlPresentFlag );
		// skip constrained_intra_pred_flag
		GET_N_UBITS( 1, ulTemp );
		// record redundant_pic_cnt_present_flag
		GET_N_UBITS( 1, pPPS->redundantPicCntPresentFlag );
		// fast forward to the next start code
		while( ( data & 0xFFFFFF ) != 1 )
		{
			result = rtfWinSmallAdvance( pVcd->hWin, 1, &data );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// the first byte of the window is the last byte of the PPS, so stop recording
		result = rtfWinStopRecord( pVcd->hWin, &byteCount );
		RTF_CHK_RESULT;
		// byte count includes the next start code prefix, so adjust it.
		byteCount -= 3;
		// make a packet out of the recorded SPS NAL unit
		// copy the NAL unit to the end of the packet buffer
		pPkt = pPPS->pkt;
		pSrc = pPkt + byteCount - 1;
		pDst = pPkt + TRANSPORT_PACKET_BYTES - 1;
		for( i=0; i<byteCount; ++i )
		{
			*pDst-- = *pSrc--;
		}
		// compute the size of the adaptation field
		// required to fill the rest of the packet
		adaptLen = TRANSPORT_PACKET_BYTES - ( byteCount + 4 );
		// set up the packet header
		pPkt[ 0 ] = TRANSPORT_PACKET_SYNCBYTE;
		pPkt[ 1 ] = (unsigned char)( ( pVcd->pVidSpec->pid >> 8 ) & 0xFF );
		pPkt[ 2 ] = (unsigned char)( pVcd->pVidSpec->pid & 0xFF );
		if( adaptLen == 0 )
		{
			pPkt[ 3 ] = 0x10;
		}
		else
		{
			pPkt[ 3 ] = 0x30;
			pPkt[ 4 ] = adaptLen - 1;
			if( adaptLen > 1 )
			{
				pPkt[ 5 ] = 0x00;
				for( i=5; i<(TRANSPORT_PACKET_BYTES-byteCount); ++i )
				{
					pPkt[ i ] = 0xFF;
				}
			}
		}
		pPPS->valid = TRUE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfGetH264StartCodeType( RTF_VCD *pVcd, unsigned long code, RTF_STARTCODE_TYPE_H264 *pType )
{
	RTF_FNAME( "rtfGetH264StartCodeType" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_STARTCODE_TYPE_H264 type = RTF_STARTCODE_TYPE_H264_INVALID;

	do {

		if( ( code & 0xFFFFFF00 ) != TRANSPORT_STARTCODE_BASE )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Invalid H.264 start code (0x%x)", code );
			break;
		}
		// mask out the prefix, the forbidden zero bit, and the NAL REF IDC field
		code &= 0x1F;
		// decode what remains
		if( code >= 0x18 )
		{
			*pType = RTF_STARTCODE_TYPE_H264_UNSPECIFIED;
			break;
		}
		if( code >= 0x13 )
		{
			*pType = RTF_STARTCODE_TYPE_H264_RESERVED;
			break;
		}
		switch( code )
		{
		case 0x00:
			*pType = RTF_STARTCODE_TYPE_H264_UNSPECIFIED;
			break;
		case 0x01:
			*pType = RTF_STARTCODE_TYPE_H264_SLICENONIDR;
			break;
		case 0x02:
			*pType = RTF_STARTCODE_TYPE_H264_SLICEPARTA;
			break;
		case 0x03:
			*pType = RTF_STARTCODE_TYPE_H264_SLICEPARTB;
			break;
		case 0x04:
			*pType = RTF_STARTCODE_TYPE_H264_SLICEPARTC;
			break;
		case 0x05:
			*pType = RTF_STARTCODE_TYPE_H264_SLICEIDR;
			break;
		case 0x06:
			*pType = RTF_STARTCODE_TYPE_H264_SEI;
			break;
		case 0x07:
			*pType = RTF_STARTCODE_TYPE_H264_SEQPARAMSET;
			break;
		case 0x08:
			*pType = RTF_STARTCODE_TYPE_H264_PICPARAMSET;
			break;
		case 0x09:
			*pType = RTF_STARTCODE_TYPE_H264_ACCESSUNITDELIMITER;
			break;
		case 0x0A:
			*pType = RTF_STARTCODE_TYPE_H264_ENDOFSEQ;
			break;
		case 0x0B:
			*pType = RTF_STARTCODE_TYPE_H264_ENDOFSTREAM;
			break;
		case 0x0C:
			*pType = RTF_STARTCODE_TYPE_H264_FILLERDATA;
			break;
		default:
			*pType = RTF_STARTCODE_TYPE_H264_SYSTEM;
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// initalize the UEV bits table
static void rtfInitUevBits()
{
	int range = 1;
	int bits = 1;
	int n = 0;
	int i;

	do
	{
		for( i=0; i<range; ++i )
		{
			if( n >= sizeof(uevBits) )
			{
				break;
			}
			uevBits[ n++ ] = bits;
		}
		bits += 2;
		range <<= 1;
	} while ( n < sizeof(uevBits) );
}

// initialize the CABAC encoding engine
static void rtfInitCABAC( P_RTF_VCD pVcd )
{
	// !!! FIX ME !!! CABAC VCDING NOT YET SUPPORTED !!!
}

// create a no-change slice header
// NOTE: many of the fields in a slice header vary from slice to slice, and from
// picture to picture. Some are predictable within a picture (ex: first macroblock
// address), while others are not (ex: picture parameter set ID). This means that
// NC frames (when used) must be generated on the fly. Yuck! Please don't use them
// unless absolutely necessary!
// NOTE: slice type is assumed to be RTF_SLICE_TYPE_H264_P
// NOTE: nal unit type is assumed to be RTF_NAL_UNIT_TYPE_NONIDR
// NOTE: nal reference idc is assumed to be 0 (slice is not referenced)
static void rtfGenerateNCSliceHeader( P_RTF_VCD pVcd, unsigned char *pBuf, unsigned long *pBits )
{
	RTF_VCD_INFO_H264 *pVcdInfo;
	RTF_VCD_SPS_H264 *pSPS;
	RTF_VCD_PPS_H264 *pPPS;
	unsigned long ulTemp;
	int i, bits;

	// generate a convenience pointer to the codec info structure
	pVcdInfo = &pVcd->vcdInfo.h264;
	// generate a convenience pointer to the current sequence parameter set
	pSPS = &pVcdInfo->sps[ 0 ];
	// generate a convenience pointer to the current picture parameter set
	pPPS = &pVcdInfo->pps[ 0 ];
	// set the slice start code
	byteAppendH264( 0x00000101, pBuf, 4, pBits );
	// encode address of first macroblock in this slice
	// note: since NC pictures have only one slice, this is always 0
	UEV_ENCODE( 0, pBuf, pBits );
	// encode slice type
	UEV_ENCODE( RTF_SLICE_TYPE_H264_P, pBuf, pBits );
	// encode current picture parameter set id
	UEV_ENCODE( pVcdInfo->ppsID[0], pBuf, pBits );
	// encode current frame number
	bitAppendH264( pVcdInfo->frNum[0], pBuf, (unsigned char)( pSPS->log2MaxFrameNum & 0xFF ), pBits );
	if( pSPS->frameMbsOnlyFlag == FALSE )
	{
		// insert field_pic_flag;
		bitAppendH264( pVcdInfo->fieldPicFlag, pBuf, 1, pBits );
		if( pVcdInfo->fieldPicFlag != 0 )
		{
			// insert bottom_field_flag;
			bitAppendH264( pVcdInfo->bottomFieldFlag, pBuf, 1, pBits );
		}
	}
	// NOTE: nal unit type is not 5
	if( pSPS->picOrderCntType == 0 )
	{
		// insert the pic_order_cnt_lsb field
		bitAppendH264( pVcdInfo->picOrderCnt, pBuf, (unsigned char)( pSPS->log2MaxPicOrderCntLsb & 0xFF ), pBits );
	}
	if( ( pPPS->picOrderPresentFlag != 0 ) && ( pVcdInfo->fieldPicFlag == 0 ) )
	{
		// insert the delta_pic_order_cnt_bottom field
		SEV_ENCODE( pVcdInfo->deltaPicOrderCntBottom, pBuf, pBits );
	}
	if( ( pSPS->picOrderCntType != FALSE ) && ( pSPS->deltaPicOrderAlwaysZeroFlag == FALSE ) )
	{
		// insert delta_pic_order_cnt[ 0 ] = 0
		SEV_ENCODE( 0, pBuf, pBits );
		if( ( pPPS->picOrderPresentFlag != FALSE ) && ( pVcdInfo->fieldPicFlag == FALSE ) )
		{
			// insert delta_pic_order_cnt[ 1 ] = 0
			SEV_ENCODE( 0, pBuf, pBits );
		}
	}
	if( pPPS->redundantPicCntPresentFlag != FALSE )
	{
		// insert redundant pic count of 0
		UEV_ENCODE( 0, pBuf, pBits );
	}
	// note: slice type *is not* B
	// note: slice type *is* P
	// insert num_ref_ids_active_override_flag of 0 - skip idx10 and idx11
	bitAppendH264( 0, pBuf, 1, pBits );
	// ref_pic_list_reordering: slice type is P - insert flag10 of 0
	bitAppendH264( 0, pBuf, 1, pBits );
	// slice type is P
	if( pPPS->weighted_pred_flag != FALSE )
	{
		// pred_weight_table ********************
		// insert luma_log2_weight_denom of 0
		UEV_ENCODE( 0, pBuf, pBits );
		// insert chroma_lo2_weight_denom of 0
		UEV_ENCODE( 0, pBuf, pBits );
		// iterate of the table entries
		for( i=0; i<=(int)pPPS->numRefIdxL0ActiveMinus1; ++i )
		{
			// insert luma_weight_l0_flag of 0 - skip weighting factors
			bitAppendH264( 0, pBuf, 1, pBits );
			// insert chroma_weight_l0_flag of 0 - skip weighting factors
			bitAppendH264( 0, pBuf, 1, pBits );
		}
	}
	// note: nal_ref_idc is 0; skip dec_ref_pic_marking
	// note: slice type is P
	if( pPPS->entropyCodingModeFlag != FALSE )
	{
		// insert a cabac_init_idc field of value 0
		UEV_ENCODE( 0, pBuf, pBits );
	}
	// insert slice_qp_delta of 0
	SEV_ENCODE( 0, pBuf, pBits );
	// note - slice type is not SP - skip sp_for_switch_flag and slice_qs_delta
	if( pPPS->deblockingFilterControlPresentFlag != FALSE )
	{
		// insert disable_deblocking_filter_idc of 1 - skip alpha, beta
		UEV_ENCODE( 1, pBuf, pBits );
	}
	if( ( pPPS->numSliceGroupsMinus1 > 0 ) &&
		( pPPS->sliceGroupMapType >=3    ) &&
		( pPPS->sliceGroupMapType <=5    ) )
	{
		// compute the number of bits in the slice_group_change_cycle field
		ulTemp = ( pSPS->picWidthInMbs * pSPS->picHeightInMapUnits ) / pPPS->sliceGroupChangeRate;
		bits = 0;
		while( ulTemp != 0 )
		{
			ulTemp >>= 1;
			++bits;
		}
		// insert a slice_group_change_cycle field of 0
		bitAppendH264( 0, pBuf, bits, pBits );
	}
}

// generate data for a no-change slice - non entropy coded
static void rtfGenerateNCSliceDataNEC( P_RTF_VCD pVcd, unsigned char *pBuf, unsigned long *pBits )
{
	RTF_VCD_INFO_H264 *pVcdInfo;
	RTF_VCD_SPS_H264 *pSPS;
	RTF_VCD_PPS_H264 *pPPS;

	// generate some convenience pointers
	pVcdInfo = &pVcd->vcdInfo.h264;
	pPPS = &pVcdInfo->pps[ 0 ];
	pSPS = &pVcdInfo->sps[ 0 ];
	// note - slice type is not I or SI
	// use mb_skip_run syntax - all macroblocks in picture are skipped
	UEV_ENCODE( pSPS->picSizeInMbs, pBuf, pBits );
}

// generate data for a no-change slice - entropy coded
static void rtfGenerateNCSliceDataEC( P_RTF_VCD pVcd, unsigned char *pBuf, unsigned long *pBits )
{
	RTF_VCD_INFO_H264 *pVcdInfo;
	RTF_VCD_SPS_H264 *pSPS;
	RTF_VCD_PPS_H264 *pPPS;
	unsigned long i;

	// generate some convenience pointers
	pVcdInfo = &pVcd->vcdInfo.h264;
	pPPS = &pVcdInfo->pps[ 0 ];
	pSPS = &pVcdInfo->sps[ 0 ];
	// byte-align
	while( ( *pBits & 0x07 ) != 0 )
	{
		bitAppendH264( 1, pBuf, 1, pBits );
	}
	// iterate over the macroblocks in the picture - all are skipped
	for( i=0; i<pSPS->picSizeInMbs; ++i )
	{
		// note - slice type is not I or SI
		// use mb_skip_flag semantics
		// !!! FIX ME !!! AE(V) CABAC ENCODING NOT YET SUPPORTED !!!
	}
	// insert end-of-slice flag
	// !!! FIX ME !!! AE(V) CABAC ENCODING NOT YET SUPPORTED !!!
}

// generate a no-change slice
// note: slice is full picture with all skipped macroblocks
static void rtfGenerateNCSlice( P_RTF_VCD pVcd, unsigned char *pBuf, unsigned long *pBits )
{
	// generate an NC slice header
	rtfGenerateNCSliceHeader( pVcd, pBuf, pBits );
	// generate the slice data
	if( pVcd->vcdInfo.h264.pps[ 0 ].entropyCodingModeFlag == FALSE )
	{
		rtfGenerateNCSliceDataNEC( pVcd, pBuf, pBits );
	}
	else
	{
		rtfGenerateNCSliceDataEC( pVcd, pBuf, pBits );
	}
	// insert trailing RBSP bits
	while( ( *pBits & 0x07 ) != 0 )
	{
		bitAppendH264( 0, pBuf, 1, pBits );
	}
}

// H.264 Vcdec specific public functions ************************************************

// initialize the H.264 codec
void rtfVcdH264Init( P_RTF_VCD pVcd )
{
	// initialize the UEV bits table
	rtfInitUevBits();
}

// process an H.264 start code
RTF_RESULT rtfVcdH264ProcessStartCode( P_RTF_VCD pVcd, unsigned long code )
{
	RTF_FNAME( "rtfProcessH264StartCode" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	RTF_STARTCODE_TYPE_H264 type;

	do {		 // error escape wrapper - begin

		// decode the start code type
		result = rtfGetH264StartCodeType( pVcd, code, &type );
		RTF_CHK_RESULT;
		switch( type )
		{
		case RTF_STARTCODE_TYPE_H264_ACCESSUNITDELIMITER:
			// parse the access unit delimiter
			result = rtfParseH264AccessUnitDelimiter( pVcd );
			RTF_CHK_RESULT;
			// update the boundary info on encountering a non-picture element
			RTF_VCD_UPDATE_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_H264_SLICEIDR:
		case RTF_STARTCODE_TYPE_H264_SLICENONIDR:
			// parse the slice header
			result = rtfParseH264SliceHdr( pVcd );
			RTF_CHK_RESULT;
			// reset the boundary info on encountering a picture element
			RTF_VCD_RESET_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_H264_SEQPARAMSET:
			// parse the sequence parameter set (SPS)
			result = rtfParseH264SPS( pVcd );
			RTF_CHK_RESULT;
			// update the boundary info on encountering a non-picture element
			RTF_VCD_UPDATE_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_H264_PICPARAMSET:
			// parse the picture parameter set (PPS)
			result = rtfParseH264PPS( pVcd );
			RTF_CHK_RESULT;
			// update the boundary info on encountering a non-picture element
			RTF_VCD_UPDATE_BOUNDARY;
			break;
		case RTF_STARTCODE_TYPE_H264_ENDOFSEQ:
			// terminate the current sequence (if one is active)
			result = rtfSesEndSeq( pVcd->hSes );
			RTF_CHK_RESULT;
			// reset the boundary info on encountering a picture element
			RTF_VCD_RESET_BOUNDARY;
			break;
		default:
			// not currently interested in any other start code types
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// build an H264 no change frame
// !!! WARNING !!! VODDRV EMBEDS ZERO MOTION P AND B FRAMES IN INDEX, EXPECTING THEM
// TO BE FIXED BIT PATTERNS. THIS IS NOT THE CASE FOR H.264 IN WHICH SLICES CAN USE
// VARIABLE PICTURE PARAMETER SET NUMBERS AND FRAME NUMBERS. DON'T TRY THIS AT HOME!!!
RTF_RESULT rtfVcdH264BuildNCFrame( P_RTF_VCD pVcd, RTF_VCD_PPT_H264 pictureType,
								   unsigned char *pBuf, unsigned long *pBufBits )
{
	RTF_FNAME( "rtfVcdH264BuildNCFrame" );
	RTF_OBASE( pVcd );
	RTF_RESULT result = RTF_PASS;
	void *pDat;
	int len;

	do {		 // error escape wrapper - begin

		// set up some picture type dependent info
		if ( ( pictureType == RTF_VCD_PPT_H264_IPB     ) ||
			 ( pictureType == RTF_VCD_PPT_H264_ISIPSPB ) )
		{
			if( pVcd->insertDSM == FALSE )
			{
				len = sizeof(zeroMotionBPesDataNoDSM);
				pDat = ( void * )zeroMotionBPesDataNoDSM;
			}
			else
			{
				len = sizeof(zeroMotionBPesData);
				pDat = ( void * )zeroMotionBPesData;
			}
		}
		else
		{
			if( pVcd->insertDSM == FALSE )
			{
				len = sizeof(zeroMotionPPesDataNoDSM);
				pDat = ( void * )zeroMotionPPesDataNoDSM;
			}
			else
			{
				len = sizeof(zeroMotionPPesData);
				pDat = ( void * )zeroMotionPPesData;
			}
		}
		// setup the pes header and initial h.264 headers
		memcpy(pVcd->noChangePFrame, pDat, len);
		*pBufBits = ( len << 3 );
		// generate a single slice with all skipped macroblocks
		rtfGenerateNCSlice( pVcd, pBuf, pBufBits );
		// record the length of the no-change frame and set the valid flag
		switch( pictureType )
		{
		case RTF_VCD_PPT_H264_IP:
			pVcd->noChangePFrameBytes = ( *pBufBits + 7 ) >> 3;
			pVcd->noChangePFrameValid = TRUE;
			break;
		case RTF_VCD_PPT_H264_IPB:
			pVcd->noChangeBFrameBytes = ( *pBufBits + 7 ) >> 3;
			pVcd->noChangeBFrameValid = TRUE;
			break;
		case RTF_VCD_PPT_H264_I:
		case RTF_VCD_PPT_H264_SI:
		case RTF_VCD_PPT_H264_ISI:
		case RTF_VCD_PPT_H264_ISIPSP:
		case RTF_VCD_PPT_H264_ISIPSPB:
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Invalid primary picture type (%d)\n", pictureType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

#endif // #ifdef DO_VCD_H264
