// implementation file for rtfPes class
// encapsulates program access table
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

typedef struct _RTF_PES
{
	RTF_OBJ_HANDLE hBaseObject;

	BOOL valid;
	int decodingDelay;
	int presentationDelay;
	RTF_PKT_HANDLE pesHdrPrePktHandle;
	RTF_PKT_HANDLE pesHdrFirstPktHandle;
	RTF_PKT_HANDLE pesHdrLastPktHandle;
	RTF_PKT_HANDLE pesHdrNextPktHandle;
	unsigned char pesHdrPrePktOffset;
	unsigned char pesHdrFirstPktOffset;
	unsigned char pesHdrLastPktOffset;
	unsigned char pesHdrNextPktOffset;
	unsigned char hdrLength;
	unsigned char hdr[ TRANSPORT_PACKET_BYTES ];

} RTF_PES;

// local functions **********************************************************************

static UINT64 rtfParseTimeStamp( unsigned char *pBuf )
{
	UINT64 val;

	val = ( *pBuf++ & 0x06 ) >> 1;
	val <<= 8;
	val |= *pBuf++;
	val <<= 7;
	val |= ( *pBuf++ & 0xFE ) >> 1;
	val <<= 8;
	val |= *pBuf++;
	val <<= 7;
	val |= ( *pBuf & 0xFE ) >> 1;

	return val;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPesGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfPesGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_PES);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfPesConstructor( RTF_PES_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfPesConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_PES *pPes;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the PES object
		pPes = (RTF_PES *)rtfAlloc( sizeof(RTF_PES) );
		RTF_CHK_ALLOC( pPes );
		// return the handle
		*pHandle = (RTF_PES_HANDLE)pPes;
		// clear the state structure
		memset( (void *)pPes, 0, sizeof(*pPes) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_PES, (RTF_HANDLE)pPes, hParent, &pPes->hBaseObject );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfPesDestructor( RTF_PES_HANDLE handle )
{
	RTF_FNAME( "rtfPesDestructor" );
	RTF_OBASE( handle );
	RTF_PES *pPes = (RTF_PES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded resource object (note: performs CHK_OBJ)
		result = rtfObjDestructor( pPes->hBaseObject, RTF_OBJ_TYPE_PES );
		RTF_CHK_RESULT;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// retrieve a pointer to the recorded PES packet
RTF_RESULT rtfPesGetHeader( RTF_PES_HANDLE handle, unsigned char **ppHeader, unsigned char *pLength )
{
	RTF_FNAME( "rtfPesGetTable" );
	RTF_OBASE( handle );
	RTF_PES *pPes = (RTF_PES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPes, RTF_OBJ_TYPE_PES );
		// make the returns (note: return NULL if pes not yet captured)
		*ppHeader = ( pPes->valid == FALSE ) ? (unsigned char *)NULL : pPes->hdr;
		*pLength = pPes->hdrLength;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get positional information on a PES packet header
RTF_RESULT rtfPesGetHdrInfo( RTF_PES_HANDLE handle,         unsigned char *pPesHdrLength,
							 RTF_PKT_HANDLE *phPrePesPkt,   unsigned char *pPrePesOffset,
							 RTF_PKT_HANDLE *phFirstPesPkt, unsigned char *pFirstPesOffset,
							 RTF_PKT_HANDLE *phLastPesPkt,  unsigned char *pLastPesOffset,
							 RTF_PKT_HANDLE *phNextPesPkt,  unsigned char *pNextPesOffset )
{
	RTF_FNAME( "rtfPesGetHdrInfo" );
	RTF_OBASE( handle );
	RTF_PES *pPes = (RTF_PES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPes, RTF_OBJ_TYPE_PES );
		// make sure the PES header has been captured
		if( pPes->valid != TRUE )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "PES packet header not yet captured" );
			break;
		}
		// make the returns
		*pPesHdrLength   = pPes->hdrLength;
		*phPrePesPkt     = pPes->pesHdrPrePktHandle;
		*pPrePesOffset   = pPes->pesHdrPrePktOffset;
		*phFirstPesPkt   = pPes->pesHdrFirstPktHandle;
		*pFirstPesOffset = pPes->pesHdrFirstPktOffset;
		*phLastPesPkt    = pPes->pesHdrLastPktHandle;
		*pLastPesOffset  = pPes->pesHdrLastPktOffset;
		*phNextPesPkt    = pPes->pesHdrNextPktHandle;
		*pNextPesOffset  = pPes->pesHdrNextPktOffset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the decoding delay (DTS-PCR) and the presentation delay (PTS-DTS) in 90 KHz clock ticks
RTF_RESULT rtfPesGetDelays( RTF_PES_HANDLE handle, int *pDecodingDelay, int*pPresentationDelay )
{
	RTF_FNAME( "rtfPesGetDelays" );
	RTF_OBASE( handle );
	RTF_PES *pPes = (RTF_PES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPes, RTF_OBJ_TYPE_PES );
		// make sure the PES header has been captured
		if( pPes->valid != TRUE )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "PES packet header not yet captured" );
			break;
		}
		// make the returns
		*pDecodingDelay     = pPes->decodingDelay;
		*pPresentationDelay = pPes->presentationDelay;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset a PES object
RTF_RESULT rtfPesReset( RTF_PES_HANDLE handle )
{
	RTF_FNAME( "rtfPesReset" );
	RTF_OBASE( handle );
	RTF_PES *pPes = (RTF_PES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPes, RTF_OBJ_TYPE_PES );
		RTF_CLR_STATE( &pPes->hdr, sizeof(pPes->hdr) );
		pPes->valid = FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// validate a PES object
RTF_RESULT rtfPesValidate( RTF_PES_HANDLE handle, BOOL *pIsValid )
{
	RTF_FNAME( "rtfPesValidate" );
	RTF_OBASE( handle );
	RTF_PES *pPes = (RTF_PES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPes, RTF_OBJ_TYPE_PES );
		// make the return
		*pIsValid = pPes->valid;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse a PES packet header
// note: this will only be called on the video stream!
RTF_RESULT rtfPesParse( RTF_PES_HANDLE handle, RTF_WIN_HANDLE hWindow, RTF_INDEX_TYPE indexType )
{
	RTF_FNAME( "rtfPesParse" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_PES *pPes = (RTF_PES *)handle;
	RTF_PKT_HANDLE hPacket;
	unsigned long data;
	INT64 pcrTime;
	INT64 dtsTime;
	INT64 ptsTime;
	int bytes;
	unsigned char offset;
	unsigned char ptsDtsFlags;
	unsigned char hdrBuf[ TRANSPORT_PACKET_BYTES ];

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPes, RTF_OBJ_TYPE_PES );
		// the window currently contains the start code
		// get the position of the byte preceding the PES packet header
		result = rtfWinGetPriorByteInfo( hWindow, &hPacket, &offset );
		RTF_CHK_RESULT;
		// record this
		pPes->pesHdrPrePktHandle = hPacket;
		pPes->pesHdrPrePktOffset = offset;
		// get the position of the first byte of the PES packet header start code
		result = rtfWinGetFirstByteInfo( hWindow, &hPacket, &offset );
		RTF_CHK_RESULT;
		// record these in the PES object, and in the packet that contains it
		pPes->pesHdrFirstPktHandle = hPacket;
		pPes->pesHdrFirstPktOffset = offset;
		// have we already captured a valid PES header?
		if( pPes->valid == FALSE )
		{
			// no - have the parsing window record all window data into the header buffer
			// until further notice
			result = rtfWinStartRecord( hWindow, hdrBuf, sizeof( hdrBuf ) );
			RTF_CHK_RESULT;
		}
		// get the length of the pes header
		result = rtfWinAdvance( hWindow, 5, &data );
		RTF_CHK_RESULT;
		// advance to the the last byte of the header
		bytes = (int)( data & 0xFF );
		result = rtfWinAdvance( hWindow, bytes, &data );
		RTF_CHK_RESULT;
		// record the last header byte position
		result = rtfWinGetLastByteInfo( hWindow, &hPacket, &offset );
		RTF_CHK_RESULT;
		pPes->pesHdrLastPktHandle = hPacket;
		pPes->pesHdrLastPktOffset = offset;
		// compute the total size of the PES header
		bytes += 9;
		// have we already captured a valid PES packet header?
		if( pPes->valid == FALSE )
		{
			// no - turn off window data recording - get the number of bytes copied
			result = rtfWinStopRecord( hWindow, &bytes );
			RTF_CHK_RESULT;
			// make sure the PES header is less than a packet in length
			if( bytes > TRANSPORT_PACKET_BYTES )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "PES packet header longer than single packet" );
				break;
			}
			// record the PES header data in the state structure
			memcpy( pPes->hdr, hdrBuf, bytes );
		}
		// record the size of the PES header
		pPes->hdrLength = (unsigned char)( bytes & 0xFF );
		// record the next position
		result = rtfWinGetNextByteInfo( hWindow, &hPacket, &offset );
		RTF_CHK_RESULT;
		pPes->pesHdrNextPktHandle = hPacket;
		pPes->pesHdrNextPktOffset = offset;
		// have we already captured a valid PES packet header?
		if( pPes->valid == FALSE )
		{
			// are both PTS and DTS present?
			ptsDtsFlags = pPes->hdr[ 7 ] & 0xC0;
			if( ptsDtsFlags == 0xC0 )
			{
				// yes - get the current interpolated PCR value in 90 KHz clock ticks from the window
				result = rtfWinGetPcrTime( hWindow, &pcrTime );
				RTF_CHK_RESULT;
				// record the PTS value in 90 KHz clock ticks
				ptsTime = rtfParseTimeStamp( &pPes->hdr[ 9 ] );
				// record the DTS value in 90 KHz clock ticks
				dtsTime = rtfParseTimeStamp( &pPes->hdr[ 14 ] );
				// measure the difference between the PTS value and the DTS value
				pPes->presentationDelay = (int)( ptsTime - dtsTime );
				// measure the difference between the current PCR value
				// and the current DTS value in 90 KHz clock ticks
				pPes->decodingDelay = (int)( dtsTime - pcrTime );
				// with both PTS and DTS, everyone is happy
				pPes->valid = TRUE;
			}
			// is only PTS present?
			else if( ptsDtsFlags == 0x80 )
			{
				// yes - get the current interpolated PCR value in 90 KHz clock ticks from the window
				result = rtfWinGetPcrTime( hWindow, &pcrTime );
				RTF_CHK_RESULT;
				// record the PTS value in 90 KHz clock ticks
				ptsTime = rtfParseTimeStamp( &pPes->hdr[ 9 ] );
				// DTS value is the same as PTS in this case
				dtsTime = ptsTime;
				// the difference between the PTS value and the DTS value is zero here
				pPes->presentationDelay = 0;
				// measure the difference between the current PCR value
				// and the current DTS value in 90 KHz clock ticks
				pPes->decodingDelay = (int)( dtsTime - pcrTime );
				// VVX indexer requires both PTS and DTS; everyone else will make do with just PTS
				if( indexType != RTF_INDEX_TYPE_VVX )
				{
					pPes->valid = TRUE;
				}
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
