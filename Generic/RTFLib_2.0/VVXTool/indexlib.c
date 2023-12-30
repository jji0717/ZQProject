// $Header: /ZQProjs/Generic/RTFLib_2.0/VVXTool/indexlib.c 1     10-11-12 15:59 Admin $
//
// Copyright (c) 2006 SeaChange International
//
// Module Name:
//
//    	IndexLib.c
//
// Abstract:
//
//		A set of subroutines to create an VOD VVX style index file
//
//		SetupInitialIndexHeader
//			Initialize a VOD VVX index file header
//
//		WriteIndexHeader
//			Format output buffer with VVX VVX header data
//
//		WriteFirstRecord
//			Write a single index record which contains
//
//		WriteIndexRecord
//			Write two index records overwriting 1 prior record entry.
//
// Author:
//
//		Philip Wells
//
// Environment:
//
//		Any
//
// Notes:
//
//		Module written to run in user mode and kernel mode 
//
// Revision History:
//
// $Log: /ZQProjs/Generic/RTFLib_2.0/VVXTool/indexlib.c $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 1     08-02-26 11:02 Ken.qian
//

#include "RTFPrv.h"

#define DO_ADAPTIVE_ZFRAMES					1

//
// verify that the indicated mpeg descriptor is a AC3 registration descriptor
//
#define IS_AC3_DESCRIPTOR(a)	(a[1] == 4 && a[2] == 'A' && a[3] == '-' && a[4] == '3')
//
//  macro used to compute a memory pointer from a base address and byte offset
//
#define BYTE_OFFSET_POINTER(Address, Offset)		((PVOID) (((PUCHAR)(Address)) + (Offset)))
//
// VOD VVX index headers
//
typedef struct _extension_headers
{
	VVX_TS_INFORMATION			tsInfo;
	VVX_ES_DESCRIPTOR_DATA		esDescriptors[VVX_MAX_SUBFILE_COUNT];
	VVX_ES_INFORMATION			esInfo[VVX_MAX_SUBFILE_COUNT];
	VVX_V7_SUBFILE_INFORMATION	si[VVX_MAX_SUBFILE_COUNT];
} EXTENSION_HEADERS, *PEXTENSION_HEADERS;

//
// create byte arrays containing data to be used for zero motion PES packet
//
static BYTE zeroMotionBPesData[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0x88, 0x06, 0x21, 0x00, 0x01, 0x00, 0x01, 0x03, 0x00, 
	 0x00, 0x01, 0x00, 0x00, 0x1f, 0xff, 0xfb, 0xb8, 0x00, 0x00, 0x01, 0xb5, 0x81, 0x11, 0x13, 0x98,
	 0x00};

static BYTE zeroMotionPPesData[] = 
	{0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x80, 0xc8, 0x0b, 0x31, 0x00, 0x37, 0xaa, 0x87, 0x11, 0x00,
	 0x37, 0x93, 0x11, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x54, 0xb7, 0xa3, 0x80, 0x00, 0x00, 0x01,
	 0xb5, 0x81, 0x1f, 0xf3, 0x98, 0x00};

#ifdef DO_ADAPTIVE_ZFRAMES

#define MAX_ZSLICE_BYTES		16

// slice data - all bits left justified in byte 0

#define ZMBS_PREFIX_BIT_COUNT	47
#define ZMBS_PREFIX_BYTE_COUNT	( ( ZMBS_PREFIX_BIT_COUNT + 7 ) >> 3 )
static const BYTE zeroMotionBSlicePrefix[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56};

#define ZMBS_SUFFIX_BIT_COUNT 8
#define ZMBS_SUFFIX_BYTE_COUNT	( ( ZMBS_SUFFIX_BIT_COUNT + 7 ) >> 3 )
static const BYTE zeroMotionBSliceSuffix[] =
	{0x2B};

#define ZMPS_PREFIX_BIT_COUNT 46
#define ZMPS_PREFIX_BYTE_COUNT	( ( ZMPS_PREFIX_BIT_COUNT + 7 ) >> 3 )
static const BYTE zeroMotionPSlicePrefix[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6C};

#define ZMPS_SUFFIX_BIT_COUNT 7
#define ZMPS_SUFFIX_BYTE_COUNT	( ( ZMPS_SUFFIX_BIT_COUNT + 7 ) >> 3 )
static const BYTE zeroMotionPSliceSuffix[] =
	{0x36};

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

#else // #ifdef DO_ADAPTIVE_ZFRAMES

static BYTE zeroMotionBSliceData352[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56, 0x09, 0x15, 0x80};
static BYTE zeroMotionBSliceData528[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56, 0x06, 0x45, 0x60};
static BYTE zeroMotionBSliceData544[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56, 0x06, 0x05, 0x60};
static BYTE zeroMotionBSliceData704[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56, 0x02, 0x02, 0xCA, 0xC0};
static BYTE zeroMotionBSliceData720[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56, 0x02, 0x02, 0x8A, 0xC0};
static BYTE zeroMotionBSliceData1920[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x56, 0x02, 0x00, 0x40, 0x08, 0x04, 0xCA, 0xC0};

static BYTE zeroMotionPSliceData352[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6c, 0x12, 0x36};
static BYTE zeroMotionPSliceData528[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6c, 0x0c, 0x9b};
static BYTE zeroMotionPSliceData544[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6c, 0x0c, 0x1b};
static BYTE zeroMotionPSliceData704[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6c, 0x04, 0x05, 0x9B};
static BYTE zeroMotionPSliceData720[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6c, 0x04, 0x05, 0x1B};
static BYTE zeroMotionPSliceData1920[] =
	{0x00, 0x00, 0x01, 0x01, 0x7a, 0x6c, 0x04, 0x00, 0x80, 0x10, 0x09, 0x9B};

#endif // #ifdef DO_ADAPTIVE_ZFRAMES

#ifdef _LINUX

#define LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_HI 116444
#define LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_LO 73600

static void GetSystemTimeAsFileTime( PFILETIME pTime )
{
	time_t rawTime;
	INT64 winFileTime;
	INT64 adjust;

#ifdef __KERNEL__
	struct timeval tv;
	do_gettimeofday( &tv );
	rawTime = tv.tv_sec;
#else
	// get seconds since the Linux Epoch (midnight GMT 1/1/1970)
	time( &rawTime );
#endif
	// convert to Windows Epoch (midnight GMT 1/1/1601)
	winFileTime = (INT64)rawTime;
	// RATS! NO 64 BIT CONSTANTS!
	adjust = LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_HI;
	adjust *= 100000;
	adjust += LINUX_TO_WIN32_DELTA_EPOCH_SECONDS_LO;
	winFileTime += adjust;
	// FILETIME is 100 ns intervals, so multiply by 10E7
	winFileTime *= 10000000;
	// fill in the FILETIME structure
	pTime->dwHighDateTime = (DWORD)(winFileTime>>32);
	pTime->dwLowDateTime  = (DWORD)(winFileTime);
}
#endif

#ifdef DO_ADAPTIVE_ZFRAMES

static void bitAppend( const unsigned char *pSrc, unsigned char *pDst,
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

static void bitAppendMBAI( unsigned char *pDst, int macroblocks, int *pTotalBitCount )
{
	// reduce macroblock count by 1 - (first macroblock doesn't count)
	macroblocks-=2;
	// insert any required macroblock address increment escape codes
	while( macroblocks > 33 )
	{
		bitAppend( mbaiVLC[ 33 ], pDst, ( mbaiVLC[ 33 ][ 1 ] & 0x0F ), pTotalBitCount );
		macroblocks -= 33;
	}
	// insert the final mba increment code, if required
	if( macroblocks > 0 )
	{
		bitAppend( mbaiVLC[ macroblocks ], pDst, ( mbaiVLC[ macroblocks ][ 1 ] & 0x0F ), pTotalBitCount );
	}
} 

#endif // #ifdef DO_ADAPTIVE_ZFRAMES

//
// function to create the zero motion frames
//
static void BuildZeroMotionFrames(VVX_INDEX_CONTEXT *pContext)
{
	int i;
	int bLen = 0, pLen = 0;

#ifdef DO_ADAPTIVE_ZFRAMES

	int bSliceBits,  pSliceBits;
	int bSliceBytes, pSliceBytes;
	unsigned char bSlice[ MAX_ZSLICE_BYTES ];
	unsigned char pSlice[ MAX_ZSLICE_BYTES ];

	// manually generate the B-frame slice data
	// pre-clear the buffer
	memset( bSlice, 0, sizeof(bSlice) );
	// copy in the b-slice prefix
	memcpy( bSlice, zeroMotionBSlicePrefix, ZMBS_PREFIX_BYTE_COUNT );
	bSliceBits = ZMBS_PREFIX_BIT_COUNT;
	// append the necessary macroblock address increment codes
	bitAppendMBAI( bSlice, pContext->horizontal_size>>4, &bSliceBits );
	// append the b-slice suffix
	bitAppend( zeroMotionBSliceSuffix, bSlice, ZMBS_SUFFIX_BIT_COUNT, &bSliceBits );
	// calculate the number of bytes (round up)
	bSliceBytes = ( bSliceBits + 7 ) >> 3;

	// manually generate the P-frame slice data
	// pre-clear the buffer
	memset( pSlice, 0, sizeof(pSlice) );
	// copy in the b-slice prefix
	memcpy( pSlice, zeroMotionPSlicePrefix, ZMPS_PREFIX_BYTE_COUNT );
	pSliceBits = ZMPS_PREFIX_BIT_COUNT;
	// append the necessary macroblock address increment codes
	bitAppendMBAI( pSlice, pContext->horizontal_size>>4, &pSliceBits );
	// append the p-slice suffix
	bitAppend( zeroMotionPSliceSuffix, pSlice, ZMPS_SUFFIX_BIT_COUNT, &pSliceBits );
	// calculate the number of bytes
	pSliceBytes = ( pSliceBits + 7 ) >> 3;

#endif

	//
	// for each verical macroblock (slice)
	//
	for (i = 0 ; i < ( pContext->vertical_size + 15 )/16 ; i++)
	{
		//
		// setup the pes header and initial mpeg headers
		//
		if (i == 0)
		{
			bLen = sizeof(zeroMotionBPesData);
			memcpy(pContext->zeroMotionB, zeroMotionBPesData, bLen);

			pLen = sizeof(zeroMotionPPesData);
			memcpy(pContext->zeroMotionP, zeroMotionPPesData, pLen);
		}

#ifdef DO_ADAPTIVE_ZFRAMES

		// copy in the slice data
		memcpy(pContext->zeroMotionB+bLen, bSlice, bSliceBytes);
		memcpy(pContext->zeroMotionP+pLen, pSlice, pSliceBytes);
		// adjust the row number
		pContext->zeroMotionB[bLen+3] = pContext->zeroMotionP[pLen+3] = i+1;
		// bump the lengths
		bLen += bSliceBytes;
		pLen += pSliceBytes;

#else // #ifdef DO_ADAPTIVE_ZFRAMES

		//
		// copy the data for this slice
		//
		switch( pContext->horizontal_size )
		{
		case 1920:
			memcpy(pContext->zeroMotionB+bLen, zeroMotionBSliceData1920, sizeof(zeroMotionBSliceData1920));
			memcpy(pContext->zeroMotionP+pLen, zeroMotionPSliceData1920, sizeof(zeroMotionPSliceData1920));
			pContext->zeroMotionB[bLen+3] = pContext->zeroMotionP[pLen+3] = i+1;		
			bLen += sizeof(zeroMotionBSliceData1920);
			pLen += sizeof(zeroMotionPSliceData1920);
			break;
		case 720:
			memcpy(pContext->zeroMotionB+bLen, zeroMotionBSliceData720, sizeof(zeroMotionBSliceData720));
			memcpy(pContext->zeroMotionP+pLen, zeroMotionPSliceData720, sizeof(zeroMotionPSliceData720));
			pContext->zeroMotionB[bLen+3] = pContext->zeroMotionP[pLen+3] = i+1;		
			bLen += sizeof(zeroMotionBSliceData720);
			pLen += sizeof(zeroMotionPSliceData720);
			break;
		case 704:
			memcpy(pContext->zeroMotionB+bLen, zeroMotionBSliceData704, sizeof(zeroMotionBSliceData704));
			memcpy(pContext->zeroMotionP+pLen, zeroMotionPSliceData704, sizeof(zeroMotionPSliceData704));
			pContext->zeroMotionB[bLen+3] = pContext->zeroMotionP[pLen+3] = i+1;		
			bLen += sizeof(zeroMotionBSliceData704);
			pLen += sizeof(zeroMotionPSliceData704);
			break;
		case 544:
			memcpy(pContext->zeroMotionB+bLen, zeroMotionBSliceData544, sizeof(zeroMotionBSliceData544));
			memcpy(pContext->zeroMotionP+pLen, zeroMotionPSliceData544, sizeof(zeroMotionPSliceData544));
			pContext->zeroMotionB[bLen+3] = pContext->zeroMotionP[pLen+3] = i+1;		
			bLen += sizeof(zeroMotionBSliceData544);
			pLen += sizeof(zeroMotionPSliceData544);
			break;
		case 528:
			memcpy(pContext->zeroMotionB+bLen, zeroMotionBSliceData528, sizeof(zeroMotionBSliceData528));
			memcpy(pContext->zeroMotionP+pLen, zeroMotionPSliceData528, sizeof(zeroMotionPSliceData528));
			pContext->zeroMotionB[bLen+3] = pContext->zeroMotionP[pLen+3] = i+1;		
			bLen += sizeof(zeroMotionBSliceData528);
			pLen += sizeof(zeroMotionPSliceData528);
			break;
		default:
			PRINTF("Warning: Unsupported video resolution (%dX%d)\n",
				   pContext->horizontal_size, pContext->vertical_size);
		case 352:
			memcpy(pContext->zeroMotionB+bLen, zeroMotionBSliceData352, sizeof(zeroMotionBSliceData352));
			memcpy(pContext->zeroMotionP+pLen, zeroMotionPSliceData352, sizeof(zeroMotionPSliceData352));
			pContext->zeroMotionB[bLen+3] = pContext->zeroMotionP[pLen+3] = i+1;		
			bLen += sizeof(zeroMotionBSliceData352);
			pLen += sizeof(zeroMotionPSliceData352);
			break;
		}

#endif // #ifdef DO_ADAPTIVE_ZFRAMES

	}
	//
	// save away the lengths
	//
	pContext->lenZeroMotionB = bLen;
	pContext->lenZeroMotionP = pLen;
}
//
// Here to check PMT registration descriptor for AC-3 stream.  It is assumed that a stream 
// type of 0x81 is AC3
//
// Looks for a registration descriptor for the stream type 0x81 that indicates it is not 
// an AC3 stream or a AC3 descriptor for a non 0x81 stream type.
//
static void CheckRegistrationDescriptor(VVX_INDEX_CONTEXT *pContext)
{
	//
	// form a pointer to the elemtary stream info section of the pmt
	//
	PBYTE esData = pContext->pmt+pContext->patEsInfoOffset;
	//
	// get a pointer to our extension headers
	//
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;

	int i;
	int esInfoBytes = pContext->pmtEsInfoLength;
	while(esInfoBytes > 0)
	{
		//
		// load the pid in the descriptor
		//
		int streamType = esData[0];
		int pid = ((esData[1] & 0x1f) << 8) | esData[2];
		int esInfoLength = ((esData[3] & 0x0f) << 8) | esData[4];
		PBYTE descriptor = esData+5;
		//
		// setup for next stream
		//
		esData += esInfoLength + 5;
		esInfoBytes -= esInfoLength + 5;
		//
		// locate entry in our es info table for this pid
		//
		for (i = 0 ; i < pContext->pmtEsInfoCount ; i++)
		{
			if (eh->esInfo[i].pid == pid)
			{
				break;
			}
		}

		if (i != pContext->pmtEsInfoCount)
		{
			//
			// dafault to ac3 audio if stream type is 81
			//
			if (streamType == 0x81)
			{
				eh->esInfo[i].streamFlags |= VVX_ESFLAG_AC3_AUDIO;
			}
			//
			// process each descriptor for this stream
			//
			while(esInfoLength)
			{
				if (descriptor[0] == 5)		// if registration descriptor
				{
					if (IS_AC3_DESCRIPTOR(descriptor) && streamType != 0x81)
					{
						// AC-3 reg desc found, set audio flag in flags byte
						//
						eh->esInfo[i].streamFlags |= VVX_ESFLAG_AC3_AUDIO;
					}
					else if (esData[0] == 0x81)
					{
						// A non-AC-3 reg descriptor found for stream type 0x81
						// Clear the flag if it was set before
						//
						eh->esInfo[i].streamFlags &= ~VVX_ESFLAG_AC3_AUDIO;
					}

					break;
				}

				esInfoLength -= 2 + descriptor[1];
				descriptor += 2 + descriptor[1];
			}
		}
	}
}
//
// get byte offset of elementary stream data in pmt
//
static int SniffPmt(VVX_INDEX_CONTEXT *pContext, PBYTE packet, int byteOffset, int sectionLength)
{
	int					status = TRUE;
	int					bytesRemaining;
	int					programInfoLength;
	int					programNumber;
	PBYTE				esData;
	//
	// initialize count of elementary stream descriptors
	//
	pContext->pmtEsInfoCount = 0;
	//
	// Calculate the number of bytes remaining in the PMT (4 is the crc32 length)
	//
	bytesRemaining = (sectionLength - 4);
	//
	// Pick up program_number
	//
	programNumber = (packet[byteOffset] << 8) | packet[byteOffset+1];
	byteOffset += 2;
	//
	// Process the pmt 
	//
	if (programNumber != 0)
	{
		//
		// Skip over reserved					(2 bits)
		// Skip over version_number				(5 bits)
		// Skip over current_next_indicator		(1 bit)
		// Skip over section_number				(8 bits)
		// Skip over last_section_number		(8 bits)
		//
		byteOffset += 3;
		//
		// skip PCR_PID
		//
		byteOffset += 2;
		//
		// Pick up program_info_length
		// Skip over program descriptors
		//
		programInfoLength = ((packet[byteOffset] & 0x0f) << 8) | packet[byteOffset+1];
		byteOffset +=  programInfoLength + 2;
		pContext->patEsInfoOffset = byteOffset;
		pContext->pmtEsInfoCount = 0;
		pContext->pmtEsInfoLength = 0;
		esData = BYTE_OFFSET_POINTER(pContext->pmt, byteOffset);
		//
		// syntax check es info section and compute count of pids described
		//
		bytesRemaining -= programInfoLength + 9;	// 9 = fixed section header size 
		while (bytesRemaining > 0)
		{
			//
			// accumulate sum of esinfolengths
			//
			int esInfoLength = ((packet[byteOffset+pContext->pmtEsInfoLength+3] & 0x0f) << 8) | packet[byteOffset+pContext->pmtEsInfoLength+4];
			if (bytesRemaining >= esInfoLength + 5)
			{
				bytesRemaining -= esInfoLength + 5; 
				pContext->pmtEsInfoLength += esInfoLength + 5;
				pContext->pmtEsInfoCount++;
			}
			else
			{
				PRINTF("Syntax error parsing PMT ES info section\n");
				status = FALSE;
				break;
			}
		}
	}

	return status;
}
//
// scan the PSI tables for the program info table
//
static int SniffPmtPid(VVX_INDEX_CONTEXT *pContext)
{
	int		transportTableFound = 0;
	PBYTE	packet = pContext->pmt;
	int		status = NO_ERROR;
	int		adaptationFieldControl;
	int		byteOffset = 0;
	int		tableId;
	int		bytesRemaining;
	int		bytesThisTable;
	//
	// skip over transport packet header
	//
	bytesRemaining = TRANSPORT_PACKET_SIZE - 4;
	byteOffset = 4;
	//
	// check for adaptation field
	//
	adaptationFieldControl = (packet[3] & 0x30) >> 4;
	if ((adaptationFieldControl == 0x2) || (adaptationFieldControl == 0x3))
	{
		//
		// skip over adaptation field
		//
		byteOffset += packet[byteOffset] + 1;
	}
	//
	// Parse PMT table contents
	//
	if ((adaptationFieldControl == 0x01) || (adaptationFieldControl == 0x03))
	{
		//
		// skip to start of table
		//
		byteOffset += packet[byteOffset] + 1;
		//
		// parse pmt tables
		//
		while(!transportTableFound && byteOffset < bytesRemaining)
		{
			int sectionLength;
			//
			// Pick up table id
			//
			tableId = packet[byteOffset];
			byteOffset++;
			//
			// Pick up section_length
			//
			// section_length is the number of bytes following the section_length byte
			// including the crc32 at the end
			//
			sectionLength = ((packet[byteOffset] & 0x0f) << 8) | packet[byteOffset+1];
			byteOffset += 2;
			//
			// dispatch on table type
			//
			switch(tableId)
			{
			case 0xff:
				//
				// filler byte is end of table data
				//
				byteOffset = bytesRemaining;
				break;

			default:
				//
				// conditional access table or user private table
				//
				// don't overflow a single packet
				//
				bytesThisTable = sectionLength;
				if (bytesThisTable + byteOffset > bytesRemaining)
				{
					bytesThisTable = bytesRemaining - byteOffset;
				}

				byteOffset += bytesThisTable;
				break;

			case MPEG2_TRANSPORT_PMT_TABLE_ID:
				transportTableFound = 1;
				//
				// process pmt
				//
				status = SniffPmt(pContext, packet, byteOffset, sectionLength);
				break;
			}

			if (status != NO_ERROR)
				break;
		}
	}

	return status;
}

//
// compute a checksum for the vvx index.  Checksum is valid for V6.1 vvx files 
// and later
//
// Returns:
//
//	!= 0	valid checksum
//	== 0	checksum not supported in vvx version
//
static ULONG TrickFilesComputeVvxChecksum(PVOID vvxHeader)
{
	register ULONG checksum = 0;
	PBYTE vvxIndexHeader = (PBYTE)vvxHeader;
	int len 			= GETLONG(vvxIndexHeader+VVX_V7IH_HEADERLENGTH_OFFSET) - sizeof(ULONG);
	
	int alignedCount 	= len % sizeof(ULONG);
	int mask 			= (ULONG)(-1) >> ((sizeof(ULONG) - alignedCount) * 8);

	register int l1 	= (len - alignedCount) / sizeof(ULONG);
	register PULONG ptr = (PULONG) vvxIndexHeader;

	while(l1)
	{
		PULONG p = ptr;
		switch(l1 % 10)
		{
			case 0: checksum += *ptr++;
			case 9: checksum += *ptr++;
			case 8: checksum += *ptr++;
			case 7: checksum += *ptr++;
			case 6: checksum += *ptr++;
			case 5: checksum += *ptr++;
			case 4: checksum += *ptr++;
			case 3: checksum += *ptr++;
			case 2: checksum += *ptr++;
			case 1: checksum += *ptr++;
		}
		l1 -= (ptr - p);
	}
	//
	// fold in unaligned bytes
	//
	if (alignedCount)
		checksum += (*ptr & mask);

	if (checksum == 0) 
		checksum = 0xffffffff;
		
	return checksum;
}
//
// This function is called to collect the information required to construct a VOD VVX index
// header and extension headers
//
// Return errno, 0 == success
//
EXPORT int vvxSetupInitialIndexHeader(VVX_INDEX_CONTEXT *pContext)
{
	int						status = 0;		// no error
	int						i;
	PEXTENSION_HEADERS		eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	int						pid;
	//
	// construct the zero motion B and P Frames
	BuildZeroMotionFrames(pContext);
	//
	// sniff the pmt
	//
	SniffPmtPid(pContext);
	//
	// record current time
	//
	GetSystemTimeAsFileTime(&pContext->systemTime);
	//
	// compute the number of files being created
	//
	pContext->fileCount = 1 + pContext->ffSpeedCount + pContext->frSpeedCount;
	//
	// format subfile info section
	//
	for (i = 0 ; i < (int)pContext->fileCount ; i++)
	{
		strcpy((char *)(eh->si[i].fileExtension), pContext->outputFiles[i].extension);
		eh->si[i].fileType						= VVX_SUBFILE_TYPE_SPTS;
		eh->si[i].startingByte					= 0;
		eh->si[i].endingByte					= pContext->outputFiles[i].fileSize;
		eh->si[i].playTimeInMilliSeconds		= (ULONG)RTF_DIV64((pContext->outputFiles[i].fileSize * 8000), pContext->transportBitRate);
		//
		// record the speed of this file
		//
		eh->si[i].speed.numerator				= pContext->outputFiles[i].numerator;
		eh->si[i].speed.denominator				= pContext->outputFiles[i].denominator;
		eh->si[i].recordIndex					= pContext->outputFiles[i].index;
	}
	//
	// format transport stream information data
	//
	eh->tsInfo.programNumber					= pContext->program_number;
	eh->tsInfo.pmtPid							= pContext->pmtPid;
	eh->tsInfo.pcrPid							= pContext->pcrPid;
	eh->tsInfo.videoPid							= pContext->videoPid;
	//
	// insert elementary stream information data
	//
	if (pContext->pmtEsInfoLength)
	{
		int index = 0;
		PBYTE esData = pContext->pmt + pContext->patEsInfoOffset;
		int esInfoBytesRemaining = pContext->pmtEsInfoLength;
		esData = BYTE_OFFSET_POINTER(pContext->pmt, pContext->patEsInfoOffset);
		while(esInfoBytesRemaining > 0)
		{
			int esInfoLength = ((esData[3] & 0x0f) << 8) | esData[4];
			pid = ((esData[1] & 0x1f) << 8) | esData[2];
			//
			// Loop through existing es table to see if we already have this stream
			//
			for (i = 0; i < pContext->pmtEsInfoCount; i++)
			{
				if (eh->esInfo[i].pid == pid)
				{
					break;
				}
			}
			//
			// Add this stream to table if not currently known
			//
			if (i == pContext->pmtEsInfoCount)
			{
				eh->esInfo[index].streamType			= esData[0];
				eh->esInfo[index].streamFlags			= 0;
				eh->esInfo[index].pid					= pid;
				index++;
			}
			esData += esInfoLength + 5;
			esInfoBytesRemaining -= esInfoLength + 5;
		}
		//
		// fixup AC-3 audio streams if needed
		//
		CheckRegistrationDescriptor(pContext);
		//
		// reset the current index file position
		//
		pContext->currentIndexFilePos = 0;
	}
	return status;
}
//
// call this function to write a leading index record to the index file.  This must be called
// after WriteIndexHeader so that the byte offset is correct.  A leading record is written in case
// the video is to be played while it is being created.  In this case, the record acts as an "indicator"
// record where the file byte offsets contain file size data.  Since this is the first record, the 
// file sizes are all zero.
//
//	Returns errno, 0 == success
//
EXPORT int vvxWriteFirstRecord(VVX_INDEX_CONTEXT *pContext, PBYTE buffer, int bufferLen)
{
	int status = 0;
	if (bufferLen >= VVX_V7R_SIZE)
	{
		int offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
		//
		// zero the record
		//
		memset(buffer, 0, VVX_V7R_SIZE);
		SETLONG(buffer+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);	// recordType
		SETLONG(buffer+VVX_V7R_TRICK_TIMECODE_OFFSET, 0);		// trick.timeCode.time

		status = pContext->writeRtn(pContext->writeContext, 0, buffer, VVX_V7R_SIZE);
		if (status)
		{
			status = 0;
		}
		else
		{
			status = EIO;
		}
		//
		// flag record written
		//
		pContext->firstRecordWritten = 1;
		//
		// update current index file position
		//
		pContext->currentIndexFilePos += VVX_V7R_SIZE;
	}

	return status;
}
//
// use this function to when a valid index has been discovered.
//
// timeCode		- GOP time code associated with the I-Frame being written
// indexTable	- array of byte offsets in the various files for the I-Frame
// sizeTable	- array of current file sizes
// buffer		- a buffer to write the record data into
// bufferLen	- the size of the buffer supplied
//
//	Returns
//		error from errno, 0 == success
//
EXPORT int vvxWriteIndexRecord(VVX_INDEX_CONTEXT *pContext, TIME_CODE timeCode, INT64 *indexTable, INT64 *sizeTable, PBYTE buffer, int bufferLen)
{
	int status = ENOMEM;
	PVVX_V7_RECORD rec = (PVVX_V7_RECORD)buffer;
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	if (bufferLen >= 2*VVX_V7R_SIZE)
	{
		// buffer is large enough
		//
		PBYTE frameData = buffer;
		int offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
		int offsetFromCurrentPosition = 0;
		int j;
		//
		// format the record as a trick index record
		// copy the time code
		//
		SETLONG(frameData+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_FRAMES_OFFSET, timeCode.frames);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_HOURS_OFFSET, timeCode.hours);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_MINUTES_OFFSET, timeCode.minutes);
		SETCHAR(frameData+VVX_V7R_TRICK_TIMECODE_SECONDS_OFFSET, timeCode.seconds);
		//
		// copy the index pointer values
		//
		for (j = 0; j < 1 + pContext->ffSpeedCount + pContext->frSpeedCount ; j++)
		{
			SETLONGLONG(frameData+offset, indexTable[eh->si[j].recordIndex]);
			offset += VVX_V7R_TRICK_FRAMEBYTEOFFSET_SIZE;
		}
		//
		// the next record is an "indicator" record containing the file sizes
		//
		// point at next index record
		//
		frameData += VVX_V7R_SIZE;
		
		offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
		//
		// set up record type and timecode to zero
		//
		SETLONG(frameData+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);
		SETLONG(frameData+VVX_V7R_TRICK_TIMECODE_OFFSET, 0);
		//
		// copy file sizes from size table
		//
		for ( j = 0; j < 1 + pContext->ffSpeedCount + pContext->frSpeedCount; j++ )
		{
			SETLONGLONG(frameData+offset, sizeTable[eh->si[j].recordIndex]);
			offset += VVX_V7R_TRICK_FRAMEBYTEOFFSET_SIZE;
		}

		status = 0;
		//
		// compute a byte offset variance.  It's most likely that we'll create a negative offset
		//
		if (pContext->firstRecordWritten)
		{
			offsetFromCurrentPosition = -(int)VVX_V7R_SIZE;
		}
		//
		// write the record and indicate an offset from the current file position
		//
		status = pContext->writeRtn(pContext->writeContext, offsetFromCurrentPosition, buffer, 2*VVX_V7R_SIZE);
		if (status)
		{
			// convert bool to status code
			//
			status = 0;
		}
		else
		{
			status = EIO;
		}
		//
		// count a record and frame
		//
		pContext->trickIndexRecordCount++;
		pContext->frameDataCount++;
		//
		// update current index file position
		//
		pContext->currentIndexFilePos += (offsetFromCurrentPosition + (2*VVX_V7R_SIZE));
	}

	return status;
}
//
// use this function to re-write the final terminator with the input file byte count.
//
// byteCount	- final byte count of input file
// buffer		- a buffer to write the record data into
// bufferLen	- the size of the buffer supplied
//
//	Returns
//		error from errno, 0 == success
//
EXPORT int vvxWriteFinalTerminator(VVX_INDEX_CONTEXT *pContext, INT64 *sizeTable, PBYTE buffer, int bufferLen)
{
	int status = ENOMEM;
	PVVX_V7_RECORD rec = (PVVX_V7_RECORD)buffer;
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	if (bufferLen >= VVX_V7R_SIZE)
	{
		// buffer is large enough
		//
		PBYTE frameData = buffer;
		int offset;
		int offsetFromCurrentPosition = 0;
		int j;
		//
		// the terminator record contains the file sizes
		//
		// point at next index record
		//
		offset = VVX_V7R_TRICK_FRAMEBYTEOFFSET_OFFSET;
		//
		// set up record type and timecode to zero
		//
		SETLONG(frameData+VVX_V7R_RECORDTYPE_OFFSET, TRICK_INDEX);
		SETLONG(frameData+VVX_V7R_TRICK_TIMECODE_OFFSET, 0);
		//
		// copy file sizes from size table
		//
		for ( j = 0; j < 1 + pContext->ffSpeedCount + pContext->frSpeedCount; j++ )
		{
			SETLONGLONG(frameData+offset, sizeTable[eh->si[j].recordIndex]);
			offset += VVX_V7R_TRICK_FRAMEBYTEOFFSET_SIZE;
		}
		status = 0;
		//
		// compute a byte offset variance.  It's most likely that we'll create a negative offset
		//
		if (pContext->firstRecordWritten)
		{
			offsetFromCurrentPosition = -(int)VVX_V7R_SIZE;
		}
		//
		// write the terminator and indicate an offset from the current file position
		//
		status = pContext->writeRtn(pContext->writeContext, offsetFromCurrentPosition, buffer, VVX_V7R_SIZE);
		// convert bool to status code
		status = (status) ? 0 : EIO;
	}

	return status;
}
//
// this function is called to format and write the index headers.
//
// Returns errno, 0 == success
//
EXPORT int vvxWriteIndexHeader(VVX_INDEX_CONTEXT *pContext, PBYTE buffer, int bufferLen)
{
	int status = ENOMEM;
	PEXTENSION_HEADERS eh = (PEXTENSION_HEADERS)pContext->extensionHeaders;
	if (eh)
	{
		LARGE_INTEGER li;
		PBYTE esData;
		int nextOffset;
		int i, headerLength;
		ULONG checksum;
		PULONG pChecksum = &checksum;
		int required;
		int unusedOffset = VVX_V7IH_SIZE;

		//
		// compute the required buffer size
		//
		required = ( pContext->fileCount * VVX_V7IH_SIZE ) +
				   ( pContext->pmtEsInfoCount * VVX_V7EI_SIZE ) +
				   ( pContext->pmtEsInfoLength ) +
				   ( pContext->lenZeroMotionB ) +
				   ( pContext->lenZeroMotionP ) +
				   ( 4 );
		if( bufferLen  < required )
		{
			return EIO;
		}

		//
		// start off with a clean slate
		//
		memset(buffer, 0, bufferLen);
		//
		// setup signature, version and filename
		//
		strcpy((char *)(buffer+VVX_V7IH_SIGNATURE_OFFSET), VVX_INDEX_SIGNATURE);
		sprintf((char *)(buffer+VVX_V7IH_PROGRAMVERSION_OFFSET), "%s, %s", LIBRARY_VERSION, LIBRARY_COPYRIGHT);
		strcpy((char *)(buffer+VVX_V7IH_FILENAME_OFFSET), pContext->filename);
		//
		// set version numbers
		//
		SETSHORT(buffer+VVX_V7IH_MAJORVERSION_OFFSET, 7);
		SETSHORT(buffer+VVX_V7IH_MINORVERSION_OFFSET, (unsigned short)pContext->minorVersion);
		//
		// convert FILETIME to a INT64
		//
		li.HighPart = pContext->systemTime.dwHighDateTime;
		li.LowPart = pContext->systemTime.dwLowDateTime;
		SETLONGLONG(buffer+VVX_V7IH_SYSTEMTIME_OFFSET, li.QuadPart);
		//
		// dts/pts time offset is the decoding delay of the first I-Frame
		//
		SETLONGLONG(buffer+VVX_V7IH_DTSPTSTIMEOFFSET_OFFSET, pContext->dtsPtsTimeOffset);
		//
		// set flags
		//
		SETCHAR(buffer+VVX_V7IH_REVERSEREAD_OFFSET, 1);
		SETCHAR(buffer+VVX_V7IH_USEPRIVATEDATA_OFFSET, 0);
		SETCHAR(buffer+VVX_V7IH_AVAILABLE_BOOLEAN_1_OFFSET, 0);
		SETCHAR(buffer+VVX_V7IH_AVAILABLE_BOOLEAN_OFFSET, 0);
		//
		// transport bitrate and stream type
		//
		SETLONG(buffer+VVX_V7IH_STREAMBITRATE_OFFSET, pContext->transportBitRate);
		SETLONG(buffer+VVX_V7IH_STREAMTYPE_OFFSET, 1);
		//
		// force record count to 1 on first pass
		//
		if (pContext->trickIndexRecordCount != 0)
		{
			SETLONG(buffer+VVX_V7IH_TRICKINDEXRECORDCOUNT_OFFSET, pContext->trickIndexRecordCount);
			SETLONG(buffer+VVX_V7IH_FRAMEDATACOUNT_OFFSET, pContext->frameDataCount);
		}
		else
		{
			SETLONG(buffer+VVX_V7IH_TRICKINDEXRECORDCOUNT_OFFSET, 1);		// needed for voddrv
			SETLONG(buffer+VVX_V7IH_FRAMEDATACOUNT_OFFSET, 1);				// needed for voddrv
		}
		//
		// set other video header elements
		//
		SETLONG(buffer+VVX_V7IH_FRAMERATECODE_OFFSET, pContext->frame_rate_code);
		SETLONG(buffer+VVX_V7IH_HORIZONTALSIZE_OFFSET, pContext->horizontal_size);
		SETLONG(buffer+VVX_V7IH_VERTICALSIZE_OFFSET, pContext->vertical_size);
		SETLONG(buffer+VVX_V7IH_VIDEOBITRATE_OFFSET, pContext->videoBitRate);
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONPICTURETYPE_OFFSET, pContext->zeroMotionFrameType);
		SETLONG(buffer+VVX_V7IH_EXTENDEDHEADERLENGTH_OFFSET, 0);	//no extended header
		//
		// copy subfile info
		//
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONMAXCOUNT_OFFSET, VVX_MAX_SUBFILE_COUNT);
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONCOUNT_OFFSET, pContext->fileCount);
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONLENGTH_OFFSET, VVX_V7SI_SIZE * VVX_MAX_SUBFILE_COUNT);
		SETLONG(buffer+VVX_V7IH_SUBFILEINFORMATIONOFFSET_OFFSET, unusedOffset);
		for (i = 0 ; i < (int)pContext->fileCount ; i++)
		{
			// update the ending byte with the file size
			eh->si[i].endingByte = pContext->outputFiles[i].fileSize;
			// update the playtime with the new file size
			eh->si[i].playTimeInMilliSeconds = (ULONG)RTF_DIV64((pContext->outputFiles[i].fileSize * 8000), pContext->transportBitRate);
			memcpy(buffer+unusedOffset+VVX_V7SI_FILEEXTENSION_OFFSET,		eh->si[i].fileExtension, VVX_V7SI_FILEEXTENSION_SIZE);
			SETLONG(buffer+unusedOffset+VVX_V7SI_FILETYPE_OFFSET,			eh->si[i].fileType);
			SETLONGLONG(buffer+unusedOffset+VVX_V7SI_STARTINGBYTE_OFFSET,	eh->si[i].startingByte);
			SETLONGLONG(buffer+unusedOffset+VVX_V7SI_ENDINGBYTE_OFFSET,		eh->si[i].endingByte);
			SETLONG(buffer+unusedOffset+VVX_V7SI_PLAYTIMEINMILLISECONDS_OFFSET, eh->si[i].playTimeInMilliSeconds);
			SETLONG(buffer+unusedOffset+VVX_V7SI_SPEED_NUMERATOR_OFFSET,	eh->si[i].speed.numerator);
			SETLONG(buffer+unusedOffset+VVX_V7SI_SPEED_DENOMINATOR_OFFSET,	eh->si[i].speed.denominator);
			SETLONG(buffer+unusedOffset+VVX_V7SI_RECORDINDEX_OFFSET,		eh->si[i].recordIndex);
			unusedOffset += VVX_V7SI_SIZE;
		}
		//
		// copy transport stream info
		//
		SETLONG(buffer+VVX_V7IH_TSINFORMATIONCOUNT_OFFSET, 1);
		SETLONG(buffer+VVX_V7IH_TSINFORMATIONLENGTH_OFFSET, VVX_V7TI_SIZE);
		SETLONG(buffer+VVX_V7IH_TSINFORMATIONOFFSET_OFFSET, unusedOffset);
		SETLONG(buffer+unusedOffset+VVX_V7TI_PROGRAMNUMBER_OFFSET, eh->tsInfo.programNumber);
		SETLONG(buffer+unusedOffset+VVX_V7TI_PMTPID_OFFSET, eh->tsInfo.pmtPid);
		SETLONG(buffer+unusedOffset+VVX_V7TI_PCRPID_OFFSET, eh->tsInfo.pcrPid);
		SETLONG(buffer+unusedOffset+VVX_V7TI_VIDEOPID_OFFSET, eh->tsInfo.videoPid);
		unusedOffset += VVX_V7TI_SIZE;
		//
		// copy elementary stream info
		//
		SETLONG(buffer+VVX_V7IH_ESINFORMATIONCOUNT_OFFSET, pContext->pmtEsInfoCount);
		SETLONG(buffer+VVX_V7IH_ESINFORMATIONLENGTH_OFFSET, VVX_V7EI_SIZE * pContext->pmtEsInfoCount);
		SETLONG(buffer+VVX_V7IH_ESINFORMATIONOFFSET_OFFSET, unusedOffset);
		for (i = 0 ; i < (int)pContext->pmtEsInfoCount ; i++)
		{
			SETCHAR(buffer+unusedOffset+VVX_V7EI_STREAMTYPE_OFFSET, eh->esInfo[i].streamType);
			SETCHAR(buffer+unusedOffset+VVX_V7EI_STREAMFLAGS_OFFSET, eh->esInfo[i].streamFlags);
			SETSHORT(buffer+unusedOffset+VVX_V7EI_PID_OFFSET, eh->esInfo[i].pid);
			unusedOffset += VVX_V7EI_SIZE;
		}
		//
		// copy elementary stream descriptor data
		//
		if (pContext->pmtEsInfoLength)
		{
			int esDescriptorInfoRemaining = pContext->pmtEsInfoLength;
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATACOUNT_OFFSET, pContext->pmtEsInfoCount);
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATALENGTH_OFFSET, VVX_V7DD_SIZE * pContext->pmtEsInfoCount);
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATAOFFSET_OFFSET, unusedOffset);
			esData = pContext->pmt+pContext->patEsInfoOffset;
			nextOffset = unusedOffset;
			unusedOffset += VVX_V7DD_SIZE*pContext->pmtEsInfoCount;
			while(esDescriptorInfoRemaining > 0)
			{
				//
				// copy the es data
				//
				int esInfoLength = ((esData[3] & 0x0f) << 8) | esData[4];
				if (esInfoLength != 0)
				{
					int pid = ((esData[1] & 0x1f) << 8) | esData[2];
					for (i = 0 ; i < (int)pContext->pmtEsInfoCount ; i++)
					{
						if (pid = eh->esInfo[i].pid)
						{
							SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATALENGTH_OFFSET, esInfoLength);
							SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATAOFFSET_OFFSET, unusedOffset);
							memcpy(buffer+unusedOffset, esData+5, esInfoLength);
							esData += esInfoLength+5;
							unusedOffset += esInfoLength;
							break;
						}
					}
					esDescriptorInfoRemaining -= esInfoLength+5;
				}
				else
				{
					SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATALENGTH_OFFSET, 0);
					SETLONG(buffer+nextOffset+VVX_V7DD_DESCRIPTORDATAOFFSET_OFFSET, 0);
					esData += 5;
					esDescriptorInfoRemaining -= 5;
				}
				nextOffset += VVX_V7DD_SIZE;
			}
		}
		else
		{
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATACOUNT_OFFSET, 0);
			SETLONG(buffer+VVX_V7IH_ESDESCRIPTORDATALENGTH_OFFSET, 0);
		}
		//
		// insert zero motion B picture
		//
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONBFRAMEPESLENGTH_OFFSET, pContext->lenZeroMotionB);
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONBFRAMEPESOFFSET_OFFSET, unusedOffset);
		memcpy(buffer+unusedOffset, pContext->zeroMotionB, pContext->lenZeroMotionB);
		unusedOffset += pContext->lenZeroMotionB;
		//
		// copy zero motion frame
		//
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONFRAMEPESLENGTH_OFFSET, pContext->lenZeroMotionP);
		SETLONG(buffer+VVX_V7IH_ZEROMOTIONFRAMEPESOFFSET_OFFSET, unusedOffset);
		memcpy(buffer+unusedOffset, pContext->zeroMotionP, pContext->lenZeroMotionP);
		unusedOffset += pContext->lenZeroMotionP;
		//
		// set splice info
		//
		SETLONG(buffer+VVX_V7IH_SPLICEINDEXRECORDCOUNT_OFFSET, 0);
		SETLONG(buffer+VVX_V7IH_SPLICINGPARAMETERSLENGTH_OFFSET, 0);
		SETLONG(buffer+VVX_V7IH_SPLICINGPARAMETERSOFFSET_OFFSET, 0);
		//
		// now set the header length
		//
		headerLength = unusedOffset+4;
		SETLONG(buffer+VVX_V7IH_HEADERLENGTH_OFFSET, headerLength);
		//
		// copy frame data info
		//
		SETLONG(buffer+VVX_V7IH_FRAMEDATALENGTH_OFFSET, pContext->trickIndexRecordCount * VVX_V7R_SIZE);
		SETLONG(buffer+VVX_V7IH_FRAMEDATAOFFSET_OFFSET, headerLength);
		//
		// finally compute checksum
		//
		*pChecksum = TrickFilesComputeVvxChecksum((PVVX_INDEX_HEADER)buffer);
		memcpy(buffer+headerLength-sizeof(ULONG), pChecksum, sizeof(ULONG));

		status = pContext->writeRtn(pContext->writeContext, -(pContext->currentIndexFilePos), buffer, headerLength);
		if (status)
		{
			status = 0;
		}
		else
		{
			status = EIO;
		}
		//
		// update current index file position
		//
		pContext->currentIndexFilePos = headerLength;
	}

	return status;
}
