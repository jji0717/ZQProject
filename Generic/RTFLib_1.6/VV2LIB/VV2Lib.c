// implementation file for VV2 index file generator
//

#include "RTFPrv.h"

// local functions **********************************************************************

static int bufcat( VV2_INDEX_CONTEXT *pCtxt, char *pStr )
{
	int size;
	
	size = strlen(pStr) + strlen(pCtxt->outBuf) + 1;
	if( size > sizeof(pCtxt->outBuf) )
	{
		PRINTF( "Buffer too small in bufcat\n" );
		return -1;
	}
	strcat( pCtxt->outBuf, pStr );
	return size-1;
}

static int vv2WriteFileLines( VV2_INDEX_CONTEXT *pCtxt )
{
	int result = 0;
	int i, j;
	unsigned long hi, lo;
	char tmpBuf[ VV2_MAX_DATA_LINE_BYTES ];
	INT64 bytes;

	do
	{

		// process available indexable pictures
		for( i=0; i<pCtxt->idxPicCount; ++i )
		{
			// add a line to the index entry for this picture
			bytes = ( (INT64)pCtxt->idxPic[ i ].pktNumber ) * pCtxt->outputPacketBytes;
			hi = (unsigned long)( ( bytes>>32 ) & 0xFF );
			lo = (unsigned long)( bytes );
			sprintf( tmpBuf, "%04X:%08X:%X:%X%08X:",
					 pCtxt->seqNumber++, pCtxt->idxPic[ i ].npt,
					 pCtxt->idxPic[ i ].filenum, hi, lo );
			if( bufcat( pCtxt, tmpBuf ) < 0 )
			{
				PRINTF( "index entry too large in vv2WriteFileLines\n" );
				result = -1;
				break;
			}
			// is this the main file?
			if( pCtxt->idxPic[ i ].filenum == pCtxt->indexFileOutputNumber )
			{
				// yes - are there any outpoints at all?
				if( pCtxt->groupOutpointCount != 0 )
				{
					// yes - generate first out point appendage field
					// note: special case because no preceding outpoint to subtract
					sprintf( tmpBuf, "%X:",
							 ( pCtxt->outpointPacketNumber[0] - pCtxt->idxPic[ i ].pktNumber ) );
					if( bufcat( pCtxt, tmpBuf ) < 0 )
					{
						PRINTF( "index entry too large in vv2WriteFileLines\n" );
						result = -1;
						break;
					}
					// generate the rest of the out point appendage fields
					for( j=1; j<pCtxt->groupOutpointCount; ++j )
					{
						sprintf( tmpBuf, "%X:", ( pCtxt->outpointPacketNumber[ j ] - pCtxt->outpointPacketNumber[ j-1 ] ) );
						if( bufcat( pCtxt, tmpBuf ) < 0 )
						{
							PRINTF( "index entry too large in vv2WriteFileLines\n" );
							result = -1;
							break;
						}
					}
				} // if( pCtxt->groupOutpointCount != 0 )
				// terminate the appendage fields
				if( bufcat( pCtxt, "\r\n" ) < 0 )
				{
					PRINTF( "index entry too large in vv2WriteFileLines\n" );
					result = -1;
					break;
				}
			}
			else // if( fileIndex == pCtxt->mainFileOutputNumber )
			{
				// not the main file - generate index unit size appendage field with line termination
				sprintf( tmpBuf, "%08X\r\n", pCtxt->idxPic[ i ].pktCount );
				if( bufcat( pCtxt, tmpBuf ) < 0 )
				{
					PRINTF( "index entry too large in vv2WriteFileLines\n" );
					result = -1;
					break;
				}
			} // if( i == pCtxt->mainFileOutputNumber ); else
		} // for( i=0; i<pCtxt->idxPicCount; ++i )

	} while( 0 );

	return result;
}

// API functions ************************************************************************

EXPORT int vv2WriteIndexHeader( VV2_INDEX_CONTEXT *pCtxt )
{
	int result = 0;
	int hdrSize;
	int i;
	VV2_FILE_INFO *pFileInfo;
	char *pDirStr;
	char tmpBuf[ VV2_MAX_HDR_LINE_BYTES ];

	do
	{
		// clear the output buffer
		memset( (void *)pCtxt->outBuf, 0, sizeof(pCtxt->outBuf) );
		// start the header
		strncpy( pCtxt->outBuf, "SOH\r\n", sizeof(pCtxt->outBuf) );
		// select the driver flavor
		switch( pCtxt->driverFlavor )
		{
		case VV2_DRIVER_FLAVOR_ON2RTP:
			// NOTE: ON2RTP does not use a driver flavor line because it was first
			break;
		case VV2_DRIVER_FLAVOR_TSOIP:
			if( bufcat( pCtxt, "DRIVERFLAVOR:TSOVERIP\r\n" ) < 0 )
			{
				result = -1;
				break;
			}
			// no hints
			if( bufcat( pCtxt, "NO_EMBEDDED_HINTS\r\n" ) < 0 )
			{
				result = -1;
			}
			break;
		default:
			result = -1;
		}
		if( result < 0 )
		{
			break;
		}
		// is TTS generation enabled?
		if( pCtxt->transportFlavor == VV2_TRANSPORT_FLAVOR_TTS )
		{
			// yes - select TTS transport, record format
			if( bufcat( pCtxt, "TTS\r\n" ) < 0 )
			{
				result = -1;
				break;
			}
		}
		// select record format
		sprintf( tmpBuf, "INDXRECFMT:%X\r\n", pCtxt->recfmt );
		if( bufcat( pCtxt, tmpBuf ) < 0 )
		{
			result = -1;
			break;
		}
		// create an entry for the main file (no extension)
		if( bufcat( pCtxt, "FILE:#:0:1:1:F\r\n" ) < 0 )
		{
			result = -1;
			break;
		}
		// iterate over the other output files
		for( i=1; i<pCtxt->outCount; ++i )
		{
			pFileInfo = &pCtxt->fileInfo[ i ];
			switch( pFileInfo->direction )
			{
			case 0:
				pDirStr = "FR";
				break;
			case 1:
				pDirStr = "F";
				break;
			case -1:
				pDirStr = "R";
				break;
			}
			sprintf( tmpBuf, "FILE:#%s:%X:%X:%X:%s\r\n", pFileInfo->extension, i,
					 pFileInfo->numerator, pFileInfo->denominator, pDirStr );
			if( bufcat( pCtxt, tmpBuf ) < 0 )
			{
				result = -1;
				break;
			}
		}
		if( result < 0 )
		{
			break;
		}
		// iterate over the output files
		for( i=0; i<pCtxt->outCount; ++i )
		{
			pFileInfo = &pCtxt->fileInfo[ i ];
			sprintf( tmpBuf, "BITRATE_HEX:%X:%X\r\n", i, pFileInfo->bitrate );
			if( bufcat( pCtxt, tmpBuf ) < 0 )
			{
				result = -1;
				break;
			}
		}
		if( result < 0 )
		{
			break;
		}
		// is this an RTP file?
		if( pCtxt->driverFlavor == VV2_DRIVER_FLAVOR_ON2RTP )
		{
			// yes - insert SDP section
			// !!! FIX ME !!! NEED TO KNOW WHAT THESE ENTRIES MEAN !!!
			for( i=0; i<pCtxt->outCount; ++i )
			{
				pFileInfo = &pCtxt->fileInfo[ i ];
				sprintf( tmpBuf, "VIDEORTPZERO_HEX:%X:%X\r\n", i, 0 );
				if( bufcat( pCtxt, tmpBuf ) < 0 )
				{
					result = -1;
					break;
				}
				sprintf( tmpBuf, "AUDIORTPZERO_HEX:%X:%X\r\n", i, 0 );
				if( bufcat( pCtxt, tmpBuf ) < 0 )
				{
					result = -1;
					break;
				}
			}
			sprintf( tmpBuf, "VIDEOTICKSPERSECOND_HEX:%X\r\n", 0 );
			if( bufcat( pCtxt, tmpBuf ) < 0 )
			{
				result = -1;
				break;
			}
			sprintf( tmpBuf, "AUDIOTICKSPERSECOND_HEX:%X\r\n", 0 );
			if( bufcat( pCtxt, tmpBuf ) < 0 )
			{
				result = -1;
				break;
			}
		}
		// is this real-time encoding mode?
		if( pCtxt->mode == VV2_INDEX_MODE_REALTIME )
		{
			// yes - don't know the input duration yet - mark it TBD
			if( bufcat( pCtxt, "PLAYDURATIONMSECS_HEX:TBD\r\n" ) < 0 )
			{
				result = -1;
				break;
			}
		}
		else
		{
			// record the input file duration in milliseconds
			sprintf( tmpBuf, "PLAYDURATIONMSECS_HEX:%X\r\n", pCtxt->playDurationMsecs );
			if( bufcat( pCtxt, tmpBuf ) < 0 )
			{
				result = -1;
				break;
			}
		}
		// insert a program version signature
		sprintf( tmpBuf, "SOSDP\r\n%s\r\nEOSDP\r\n", pCtxt->pProgramVersionStr );
		if( bufcat( pCtxt, tmpBuf ) < 0 )
		{
			result = -1;
			break;
		}
		// end the header
		if( ( hdrSize = bufcat( pCtxt, "EOH\r\n" ) ) < 0 )
		{
			PRINTF( "Header too large in vv2WriteIndexHeader\n" );
			result = -1;
			break;
		}
		// write the header to the index file
		if( pCtxt->writeRtn( pCtxt->writeContext, 0, (unsigned char *)pCtxt->outBuf, hdrSize ) < 0 )
		{
			PRINTF( "Error writing index header in vv2WriteIndexHeader\n" );
			result = -1;
			break;
		}

	} while( 0 );

	return result;
}

EXPORT int vv2WriteIndexRecord( VV2_INDEX_CONTEXT *pCtxt )
{
	int result = 0;

	do
	{

		// clear the output buffer
		memset( (void *)pCtxt->outBuf, 0, sizeof(pCtxt->outBuf) );
		// write a set of index data lines for this index entry
		if( vv2WriteFileLines( pCtxt ) != 0 )
		{
			PRINTF( "vv2WriteFileLines returned error in vv2WriteIndexRecord\n" );
			result = -1;
			break;
		}
		// write this record to the index file
		if( pCtxt->writeRtn( pCtxt->writeContext, pCtxt->nextWriteOffset, (unsigned char *)pCtxt->outBuf, strlen( pCtxt->outBuf ) ) < 0 )
		{
			PRINTF( "Error writing index record in vv2WriteIndexRecord\n" );
			result = -1;
			break;
		}
		// after writing an index record, clear the picture and outpoint lists
		pCtxt->idxPicCount = 0;
		memset( (void *)pCtxt->idxPic, 0, sizeof(pCtxt->idxPic) );
		pCtxt->groupOutpointCount = 0;
		memset( (void *)pCtxt->outpointPacketNumber, 0, sizeof(pCtxt->outpointPacketNumber) );
		// and bump the index point count
		++pCtxt->indexPointCount;

	} while( 0 );

	return result;
}

EXPORT int vv2OpenDataSection( VV2_INDEX_CONTEXT *pCtxt )
{
	int result = 0;
	char *pStr = "SOD\r\n";
	int bytes = strlen( pStr );

	do
	{
		// start the data section
		if( pCtxt->writeRtn( pCtxt->writeContext, 0, (unsigned char *)pStr, bytes ) < 0 )
		{
			PRINTF( "Error writing index header in vv2OpenDataSection\n" );
			result = -1;
			break;
		}

	} while( 0 );

	return result;
}

EXPORT int vv2RecordKeyframe  ( VV2_INDEX_CONTEXT *pCtxt, unsigned long packetNumber )
{
	int result = 0;

	// nothing to do here; keyframe info recorded elsewhere

	return result;
}

EXPORT int vv2RecordOutpoint( VV2_INDEX_CONTEXT *pCtxt, unsigned long packetNumber )
{
	int result = 0;

	do
	{
		if( pCtxt->groupOutpointCount >= VV2_MAX_OUT_POINTS )
		{
			result = -1;
			break;
		}
		pCtxt->outpointPacketNumber[ pCtxt->groupOutpointCount++ ] = packetNumber;

	} while( 0 );

	return result;
}

EXPORT int vv2CloseDataSection( VV2_INDEX_CONTEXT *pCtxt, unsigned long msecs )
{
	int result = 0;

	do
	{
		// is this real-time mode?
		if( pCtxt->mode == VV2_INDEX_MODE_REALTIME )
		{
			// yes - end the data section - record the input file duration
			sprintf( pCtxt->outBuf, "EOD:PLAYDURATIONMSECS_HEX:%X\r\n", msecs );
		}
		else
		{
			// no - just end the data section
			sprintf( pCtxt->outBuf, "EOD\r\n" );
		}
		if( pCtxt->writeRtn( pCtxt->writeContext, pCtxt->nextWriteOffset, (unsigned char *)pCtxt->outBuf, strlen(pCtxt->outBuf) ) < 0 )
		{
			PRINTF( "Error writing data section trailer in vv2CloseDataSection\n" );
			result = -1;
			break;
		}

	} while( 0 );

	return result;
}
