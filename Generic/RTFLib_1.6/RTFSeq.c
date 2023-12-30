// implementation file for rtfSeq class
// encapsulates video sequence
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

typedef struct _RTF_SEQ
{
	RTF_OBJ_HANDLE hBaseObject;

	RTF_SEQSTATE state;
	unsigned long seqNumber;
	unsigned long flags;

	// video bit rate
	unsigned long bitRate;

	// picture dimensions
	unsigned short horizSize;
	unsigned short vertSize;

	// location of first byte of sequence
	RTF_PKT_HANDLE hFirstBytePacket;
	unsigned char firstBytePacketOffset;

	// location of last byte of sequence
	RTF_PKT_HANDLE hLastBytePacket;
	unsigned char lastBytePacketOffset;

	// byte offset of the beginning and end
	// of the sequence in the input stream
	INT64 firstByteInputOffset;
	INT64 lastByteInputOffset;

	// GOPs contributing to this SEQ
	unsigned char gopCount;
	RTF_GOP_HANDLE hGop[ RTF_MAX_SEQ_GOPS ];

} RTF_SEQ;

// private functions ********************************************************************

// reset the SEQ state structure
static void resetSeq( RTF_SEQ *pSeq )
{
	// reset the codec-independent state info for this sequence
	pSeq->state = RTF_SEQSTATE_CLOSED;
	pSeq->seqNumber = -1;
	pSeq->flags = 0;
	pSeq->bitRate = 0;
	pSeq->horizSize = 0;
	pSeq->vertSize = 0;
	pSeq->gopCount = 0;
	pSeq->hFirstBytePacket = (RTF_PKT_HANDLE)NULL;
	pSeq->hLastBytePacket = (RTF_PKT_HANDLE)NULL;
	memset( (void *)pSeq->hGop, 0, sizeof(pSeq->hGop) );
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfSeqGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfSeqGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_SEQ);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfSeqConstructor( RTF_SEQ_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfSeqConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_SEQ *pSeq;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the seq object
		pSeq = (RTF_SEQ *)rtfAlloc( sizeof(RTF_SEQ) );
		RTF_CHK_ALLOC( pSeq );
		// return the handle
		*pHandle = (RTF_SEQ_HANDLE)pSeq;
		// clear the state structure
		memset( (void *)pSeq, 0, sizeof(*pSeq) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_SEQ, (RTF_HANDLE)pSeq, hParent, &pSeq->hBaseObject );
		RTF_CHK_RESULT;
		// reset the SEQ state structure
		resetSeq( pSeq );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfSeqDestructor( RTF_SEQ_HANDLE handle )
{
	RTF_FNAME( "rtfSeqDestructor" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded base object (note: performs CHK_OBJ)
		result = rtfObjDestructor( pSeq->hBaseObject, RTF_OBJ_TYPE_SEQ );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the number of a sequence object
RTF_RESULT rtfSeqGetNumber( RTF_SEQ_HANDLE handle, unsigned long *pSeqNumber )
{
	RTF_FNAME( "rtfSeqGetNumber" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the return
		*pSeqNumber = pSeq->seqNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the state of a seq object
RTF_RESULT rtfSeqGetState( RTF_SEQ_HANDLE handle, RTF_SEQSTATE *pState )
{
	RTF_FNAME( "rtfSeqGetState" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the return
		*pState = pSeq->state;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the virtual flag of a seq object
RTF_RESULT rtfSeqGetIsVirtual( RTF_SEQ_HANDLE handle, BOOL *pIsVirtual )
{
	RTF_FNAME( "rtfSeqGetIsVirtual" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the return
		*pIsVirtual = ( ( pSeq->flags & RTF_SEQ_ISVIRTUAL ) != 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the damage flag in of a sequence object
RTF_RESULT rtfSeqSetIsDamaged( RTF_SEQ_HANDLE handle, BOOL isDamaged )
{
	RTF_FNAME( "rtfSeqSetIsDamaged" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// record the flag
		if( isDamaged == FALSE )
		{
			pSeq->flags &= ~RTF_SEQ_ISDAMAGED;
		}
		else
		{
			pSeq->flags |= RTF_SEQ_ISDAMAGED;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the damage flag from of a sequence object
RTF_RESULT rtfSeqGetIsDamaged( RTF_SEQ_HANDLE handle, BOOL *pIsDamaged )
{
	RTF_FNAME( "rtfSeqGetIsDamaged" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the return
		*pIsDamaged = ( ( pSeq->flags & RTF_SEQ_ISDAMAGED ) != 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the video size info from the sequence header
RTF_RESULT rtfSeqGetSizeInfo( RTF_SEQ_HANDLE handle, unsigned short *pHoriz, unsigned short *pVert )
{
	RTF_FNAME( "rtfSeqGetSizeInfo" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the returns
		*pHoriz = pSeq->horizSize;
		*pVert  = pSeq->vertSize;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the GOP array info from a sequence object
RTF_RESULT rtfSeqGetGopArrayInfo( RTF_SEQ_HANDLE handle, unsigned char *pGopCount, RTF_GOP_HANDLE **pphGop )
{
	RTF_FNAME( "rtfSeqGetGopArrayInfo" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		RTF_CHK_STATE_EQ( pSeq, RTF_SEQSTATE_CLOSED );
		// make the returns
		*pGopCount = pSeq->gopCount;
		*pphGop = pSeq->hGop;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the starting location info for the sequence object
RTF_RESULT rtfSeqSetStartInfo( RTF_SEQ_HANDLE handle, RTF_PKT_HANDLE hFirstBytePacket,
							   unsigned char firstBytePacketOffset )
{
	RTF_FNAME( "rtfSeqSetStartInfo" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		RTF_CHK_STATE_EQ( pSeq, RTF_SEQSTATE_OPEN );
		// record the info in the sequence object
		pSeq->hFirstBytePacket      = hFirstBytePacket;
		pSeq->firstBytePacketOffset = firstBytePacketOffset;
		// add a reference to the first packet of the sequence
		result = rtfPktAddReference( hFirstBytePacket, handle );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the offset of the first packet of the sequence in the input stream
RTF_RESULT rtfSeqGetOutputStreamStartOffset( RTF_SEQ_HANDLE handle, INT64 *pOffset )
{
	RTF_FNAME( "rtfSeqGetOutputStreamStartOffset" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned long number;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// get the first packet of the sequence
		result = rtfPktGetOutPktNumber( pSeq->hFirstBytePacket, &number );
		RTF_CHK_RESULT;
		// make the return
		*pOffset = ( (INT64)number ) * TRANSPORT_PACKET_BYTES;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of the first packet of this sequence in the input stream
RTF_RESULT rtfSeqGetFirstPktNum( RTF_SEQ_HANDLE handle, unsigned long *pPktNum )
{
	RTF_FNAME( "rtfSeqGetFirstPktNum" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_GOP_HANDLE hGop;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// is there a group in the sequence?
		if( ( hGop = pSeq->hGop[ 0 ] ) == (RTF_GOP_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No group in sequence" );
			break;
		}
		// return the number of the first packet in the group
		result = rtfGopGetFirstPktNum( hGop, pPktNum );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the offset of the last packet of the sequence in the input stream
RTF_RESULT rtfSeqGetOutputStreamEndPktNum( RTF_SEQ_HANDLE handle, unsigned long *pPktNum )
{
	RTF_FNAME( "rtfSeqGetOutputStreamEndPktNum" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the return
		result = rtfPktGetOutPktNumber( pSeq->hLastBytePacket, pPktNum );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the starting packet and offsets of a sequence object
RTF_RESULT rtfSeqGetStartInfo( RTF_SEQ_HANDLE handle, RTF_PKT_HANDLE *phFirstBytePacket,
							   unsigned char *pFirstBytePacketOffset )
{
	RTF_FNAME( "rtfSeqGetStartInfo" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the returns
		*phFirstBytePacket      = pSeq->hFirstBytePacket;
		*pFirstBytePacketOffset = pSeq->firstBytePacketOffset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the video info for a sequence object
RTF_RESULT rtfSeqSetVideoInfo( RTF_SEQ_HANDLE handle, unsigned short horizSize,
							   unsigned short vertSize, unsigned long bitRate )
{
	RTF_FNAME( "rtfSeqSetVideoInfo" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		RTF_CHK_STATE_EQ( pSeq, RTF_SEQSTATE_OPEN );
		// record the settings
		pSeq->horizSize     = horizSize;
		pSeq->vertSize      = vertSize;
		pSeq->bitRate       = bitRate;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the video info for a sequence object
RTF_RESULT rtfSeqGetVideoInfo( RTF_SEQ_HANDLE handle, unsigned short *pHorizSize,
							   unsigned short *pVertSize, unsigned long *pBitRate )
{
	RTF_FNAME( "rtfSeqGetVideoInfo" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		RTF_CHK_STATE_EQ( pSeq, RTF_SEQSTATE_OPEN );
		// make the returns
		*pHorizSize     = pSeq->horizSize;
		*pVertSize      = pSeq->vertSize;
		*pBitRate       = pSeq->bitRate;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the video bit rate
RTF_RESULT rtfSeqGetVideoBitRate( RTF_SEQ_HANDLE handle, unsigned long *pBitRate )
{
	RTF_FNAME( "rtfSeqGetVideoBitRate" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// make the return
		*pBitRate = pSeq->bitRate;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset the sequence to a closed, empty state
RTF_RESULT rtfSeqReset( RTF_SEQ_HANDLE handle )
{
	RTF_FNAME( "rtfSeqReset" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// reset the SEQ state structure
		resetSeq( pSeq );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// open a new sequence
RTF_RESULT rtfSeqOpen( RTF_SEQ_HANDLE handle, unsigned long seqNumber, BOOL isVirtual )
{
	RTF_FNAME( "rtfSeqOpen" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		RTF_CHK_STATE_EQ( pSeq, RTF_SEQSTATE_CLOSED );
		// reset the seq state structure
		resetSeq( pSeq );
		// record the sequence number
		pSeq->seqNumber = seqNumber;
		// record virtual flag
		if( isVirtual == FALSE )
		{
			pSeq->flags &= ~RTF_SEQ_ISVIRTUAL;
		}
		else
		{
			pSeq->flags |= RTF_SEQ_ISVIRTUAL;
		}
		// set the state to open
		pSeq->state = RTF_SEQSTATE_OPEN;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_SEQOPEN, "SEQ %d opened", seqNumber );
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// add a group to a video sequence
RTF_RESULT rtfSeqAddGop( RTF_SEQ_HANDLE handle, RTF_GOP_HANDLE hGop )
{
	RTF_FNAME( "rtfSeqAddGop" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;
	BOOL isDamaged;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		RTF_CHK_STATE_EQ( pSeq, RTF_SEQSTATE_OPEN );
		// make sure there is room for another group
		if( pSeq->gopCount >= RTF_MAX_SEQ_GOPS )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_GOPOVERFLOW, "Group overflow in sequence %d", pSeq->seqNumber );
			pSeq->flags |= RTF_SEQ_ISDAMAGED;
			break;
		}
		// record the gop, bump the count
		pSeq->hGop[ pSeq->gopCount++ ] = hGop;
		// get the damage flag from the group
		result = rtfGopGetIsDamaged( hGop, &isDamaged );
		RTF_CHK_RESULT;
		// is the group damaged?
		if( isDamaged != FALSE )
		{
			// yes - if the group is damaged, the sequence is damaged
			pSeq->flags |= RTF_SEQ_ISDAMAGED;
		}
#ifdef DO_TRACKING
		{
			unsigned long gopNumber;
			result = rtfGopGetNumber( hGop, &gopNumber );
			RTF_CHK_RESULT;
			RTF_LOG_INFO2( RTF_MSG_INF_SEQADDGOP, "SEQ %d added GOP %d", pSeq->seqNumber, gopNumber );
		}
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// close the current seq
RTF_RESULT rtfSeqClose( RTF_SEQ_HANDLE handle )
{
	RTF_FNAME( "rtfSeqClose" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_SES_HANDLE hSes;
	RTF_WIN_HANDLE hWin;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// get the handle of the owning session
		result = rtfObjGetSession( handle, &hSes );
		// get the window handle from the session
		result = rtfSesGetWindow( hSes, &hWin );
		RTF_CHK_RESULT;
		// record the position of the end of the sequence in the input stream
		result = rtfWinGetInputByteCount( hWin, &pSeq->lastByteInputOffset );
		RTF_CHK_RESULT;
		result = rtfWinGetPriorByteInfo( hWin, &pSeq->hLastBytePacket, &pSeq->lastBytePacketOffset );
		RTF_CHK_RESULT;
		// change state to closed
		pSeq->state = RTF_SEQSTATE_CLOSED;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_SEQCLOSE, "SEQ %d closed", pSeq->seqNumber );
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// release a video sequence
RTF_RESULT rtfSeqRelease( RTF_SEQ_HANDLE handle, RTF_SES_HANDLE hSes )
{
	RTF_FNAME( "rtfSeqRelease" );
	RTF_OBASE( handle );
	RTF_SEQ *pSeq = (RTF_SEQ *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSeq, RTF_OBJ_TYPE_SEQ );
		// is there a first packet? (may have been released during formation due to error)
		if( pSeq->hFirstBytePacket != (RTF_PKT_HANDLE)NULL )
		{
			// remove the reference that was added to the first packet when the sequence was opened
			result = rtfPktRemoveReference( pSeq->hFirstBytePacket, handle );
			RTF_CHK_RESULT;
		}
		// release the GOPs in the sequence
		for( i=0; i<pSeq->gopCount; ++i )
		{
			result = rtfGopRelease( pSeq->hGop[ i ], hSes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_SEQRELEASE, "SEQ %d released", pSeq->seqNumber );
#endif
		// tell the session to release this sequence too
		result = rtfSesRecycleSeq( hSes, handle );
		RTF_CHK_RESULT;
		// reset the sequence
		resetSeq( pSeq );

	} while( 0 ); // error escape wrapper - end

	return result;
}
