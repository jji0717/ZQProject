// implementation file for RTF window class
//

#include "RTFPrv.h"

// !!! SET TO 1 FOR BURST TRANSFERS FROM INPUT BUFFERS, 0 FOR ORIGINAL VERSION !!!
#define DO_BURST_XFERS		0

#if DO_BURST_XFERS
#define BURST_BYTES			32
#define BURST_MASK			(BURST_BYTES-1)
#define BURST_ALIGN(p)		((BURST *)((((unsigned long)(p))&(~BURST_MASK))))
#define BURST_INDEX(i)		(i)
typedef struct _BURST
{
	unsigned char bytes[ BURST_BYTES ];
} BURST;
#endif

// constants ****************************************************************************

#define RTF_MAX_WINDOW_PACKETS		( 2 * RTF_MAX_BUFFER_PACKETS )
#define RTF_LOG2_CONTEXT_PACKETS	3
#define RTF_MAX_CONTEXT_PACKETS		( 1 << RTF_LOG2_CONTEXT_PACKETS )
#define RTF_CONTEXT_INDEX_MASK		( RTF_MAX_CONTEXT_PACKETS - 1 )

// macros *******************************************************************************

#define RTF_LAST_CONTEXT_INDEX		( pWin->contextIndex )
#define RTF_FIRST_CONTEXT_INDEX		( ( pWin->contextIndex + 3 ) & RTF_CONTEXT_INDEX_MASK )
#define RTF_PRIOR_CONTEXT_INDEX		( ( pWin->contextIndex + 4 ) & RTF_CONTEXT_INDEX_MASK )

// typedefs *****************************************************************************

typedef struct _RTF_WIN
{
	RTF_OBJ_HANDLE hBaseObject;

	// cached handle of session owning this window
	RTF_SES_HANDLE hSes;

	// only look at packets belonging to this PID
	unsigned short pid;

	// packet array info
	short pktCount;					// number of active packets in the array below
	short lastWinPkt;				// index of last window-PID packet in the array below
	RTF_PKT_HANDLE hPkt[ RTF_MAX_WINDOW_PACKETS ];

	// "clone" flag - don't add packets to pictures when this flag is true
	BOOL cloneFlag;

	// last packet warning flag
	BOOL lastPktWarningFlag;

	// optimization flags
	BOOL optimizeForATSC;
	BOOL overrideOptimize;

	// current window data
	unsigned long data;

	// context ring buffer
	int contextIndex;
	RTF_PKT_HANDLE hContextPkt[ RTF_MAX_CONTEXT_PACKETS ];
	unsigned char contextPktOffset[ RTF_MAX_CONTEXT_PACKETS ];

	// next packet info
	RTF_PKT_HANDLE hNextBytePkt;
	short nextBytePktIndex;
	unsigned char nextBytePktOffset;
	unsigned char nextBytePktBytesRemaining;
	unsigned char *pNextBytePktStorage;

	// data recording info
	BOOL record;
	unsigned char recStartOffset;
	unsigned char *pRecBuf;
	int recBufBytes;
	int recBufOffset;
	RTF_PKT_HANDLE hRecStartPkt;

	// info relating to interpolated PCR time of 1st byte of scanning window
	unsigned long inputBPS;	// input bit rate (from session)
	INT64 inputByteCount;	// total number of bytes scanned
	RTF_TIMESTAMP lastPcr;	// value of last PCR
	INT64 lastPcrByteCount;	// value of inputByteCount when PCR was parsed
	BOOL pcrValid;			// becomes TRUE when first PCR is parsed

} RTF_WIN;

// local functions **********************************************************************

static RTF_RESULT rtfWinNextPacket( RTF_WIN *pWin )
{
	RTF_FNAME( "rtfWinNextPacket" );
	RTF_OBASE( pWin );
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE *pSrc, *pDst;
	RTF_PIC_HANDLE hPic;
	RTF_WIN saveWin;
	unsigned long pktNumber;
	unsigned long flags;
	unsigned long junk;
	unsigned short pid;
	short count;
	unsigned char pesOff;
	unsigned char pesLen;
	int i, j, picCount;

	do {		 // error escape wrapper - begin

		// see if there is an active picture (for future reference)
		// note: the picture cannot change while we are in this routine
		result = rtfSesGetCurrentPicInfo( pWin->hSes, &hPic, &picCount );
		RTF_CHK_RESULT;
		// look for the next unscrambled window packet with payload
		for( i=0; ; ++i )
		{
			// increment the next packet index and get the handle of the next packet
			pWin->hNextBytePkt = pWin->hPkt[ ++pWin->nextBytePktIndex ];
			// get info on the next packet
			result = rtfPktGetInfo( pWin->hNextBytePkt, &pktNumber, &pid, &flags,
									&pWin->pNextBytePktStorage, &pWin->nextBytePktOffset,
									&pWin->nextBytePktBytesRemaining, &pesOff, &pesLen );
			RTF_CHK_RESULT;
			// is there an active picture AND is this the main parsing window?
			if( ( picCount > 0 ) && ( pWin->cloneFlag == FALSE ) )
			{
				// yes - add this packet to the picture
				result = rtfPicAddPacket( hPic, pWin->hNextBytePkt );
				RTF_CHK_RESULT;
			}
			// is this a window PID packet?
			if( pid != pWin->pid )
			{
				// no - go around again
				pWin->inputByteCount += TRANSPORT_PACKET_BYTES;
				continue;
			} // if( pid != pWin->pid )
			// is this the last window-PID packet?
			if( pWin->nextBytePktIndex >= pWin->lastWinPkt )
			{
				// yes - stop the search here
				// are there any packets left in the array?
				if( pWin->lastWinPkt < pWin->pktCount )
				{
					// yes - move the unscanned packets to the beginning of the packet array
					pSrc = &pWin->hPkt[ pWin->lastWinPkt ];
					pDst = &pWin->hPkt[ 0 ];
					count = pWin->pktCount - ( pWin->lastWinPkt );
					for( j=0; j<count; ++j )
					{
						*pDst++ = *pSrc++;
					}
					pWin->pktCount = count;
					pWin->nextBytePktIndex = 0;
				}
				else
				{
					// no - reset the packet count and index
					pWin->pktCount = 0;
					pWin->nextBytePktIndex = -1;
				}
				// escape the search loop. set the last packet warning flag
				pWin->lastPktWarningFlag = TRUE;
				break;
			} // if( pWin->nextBytePktIndex >= pWin->lastWinPkt )
			// was there a last window-PID packet warning flag?
			if( pWin->lastPktWarningFlag != FALSE )
			{
				// yes - escape the search loop
				break;
			}
			// does the new packet have a payload?
			if( ( flags & RTF_PKT_PAYLOADABSENT ) != 0 )
			{
				// no - skip it and go around again
				pWin->inputByteCount += TRANSPORT_PACKET_BYTES;
				continue;
			}
			// is the payload encrypted?
			if( ( flags & RTF_PKT_PAYLOADENCRYPTED ) != 0 )
			{
				// yes - can't see the data, so flush context and go around again
				pWin->data = 0xFFFFFFFF;
				pWin->inputByteCount += TRANSPORT_PACKET_BYTES;
				continue;
			}
			// not encrypted. does the packet have a PCR?
			if( ( flags & RTF_PKT_PCRPRESENT ) != 0 )
			{
				// yes - get the PCR value from the packet
				result = rtfPktGetPcrTimestamp( pWin->hNextBytePkt, &pWin->lastPcr );
				RTF_CHK_RESULT;
				// record the byte count at the last byte of the PCR base field
				pWin->lastPcrByteCount = pWin->inputByteCount + 10;
				// set the "PCR valid" flag
				pWin->pcrValid = TRUE;
			}
			// is the payload unit start flag set in this packet?
			if( ( flags & RTF_PKT_PAYLOADUNITSTART ) != 0 )
			{
				// !!! UGLY ALERT !!! THIS IS REALLY UGLY !!!
				// !!! BUT I CAN'T THINK OF A BETTER WAY TO DO IT !!!
				// !!! NEED TO PARSE PES HEADER WITHOUT DISRUPTING !!!
				// !!! THE PARSING OF THE PES PAYLOAD !!!
				// !!! THIS CODE ASSUMES THAT THE PES HEADER IS THE FIRST THING !!!
				// !!! IN THE PAYLOAD! THE SPEC SAYS THIS MUST BE THE CASE, BUT !!!
				// !!! IS THIS ALWAYS TRUE? !!!
				// yes - save the current state of the parsing context
				saveWin.data = pWin->data;
				saveWin.contextIndex = pWin->contextIndex;
				for( i=0; i<RTF_MAX_CONTEXT_PACKETS; ++i )
				{
					saveWin.hContextPkt[ i ] = pWin->hContextPkt[ i ];
					saveWin.contextPktOffset[ i ] = pWin->contextPktOffset[ i ];
				}
				// advance to the PES header start code
				result = rtfWinAdvance( (RTF_WIN_HANDLE)pWin, 4, &junk );
				RTF_CHK_RESULT;
				// start a new PES packet here
				result = rtfSesStartPes( pWin->hSes );
				RTF_CHK_RESULT;
				// restore the previous state of the window
				pWin->data = saveWin.data;
				pWin->contextIndex = saveWin.contextIndex;
				for( i=0; i<RTF_MAX_CONTEXT_PACKETS; ++i )
				{
					pWin->hContextPkt[ i ] = saveWin.hContextPkt[ i ];
					pWin->contextPktOffset[ i ] = saveWin.contextPktOffset[ i ];
				}
			} // if( ( flags & RTF_PKT_PAYLOADUNITSTART ) != 0 )
			// this is the packet. escape the search loop
			break;
		} // for( i=0; ; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfWinGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfWinGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_WIN);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfWinConstructor( RTF_WIN_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfWinConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_WIN *pWin;

	do {		 // error escape wrapper - begin

#if DO_BURST_XFERS
		if( sizeof(BURST) != BURST_BYTES )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADALLOC, "Compiler padded the BURST structure - burst transfers won't work!" );
			break;
		}
#endif
		// allocate a state structure for the  window object
		pWin = (RTF_WIN *)rtfAlloc( sizeof(RTF_WIN) );
		RTF_CHK_ALLOC( pWin );
		// return the handle
		*pHandle = (RTF_WIN_HANDLE)pWin;
		// clear the state structure
		memset( (void *)pWin, 0, sizeof(*pWin) );
		// create an embedded base object
		result = rtfObjConstructor( RTF_OBJ_TYPE_WIN, (RTF_HANDLE)pWin, hParent, &pWin->hBaseObject );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfWinDestructor( RTF_WIN_HANDLE handle )
{
	RTF_FNAME( "rtfWinDestructor" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		result = rtfObjDestructor( pWin->hBaseObject, RTF_OBJ_TYPE_WIN );
		RTF_CHK_RESULT;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the current window data
RTF_RESULT rtfWinGetData( RTF_WIN_HANDLE handle, unsigned long *pData )
{
	RTF_FNAME( "rtfWinGetData" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the return
		*pData = pWin->data;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of the oldest buffer referenced by the window
RTF_RESULT rtfWinGetOldestBufferNumber( RTF_WIN_HANDLE handle, unsigned long *pBufferNumber )
{
	RTF_FNAME( "rtfWinGetOldestBufferNumber" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_BUF_HANDLE hBuf;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the returns
		*pBufferNumber = -1;
		if( pWin->pktCount > 0 )
		{
			result = rtfPktGetBuffer( pWin->hPkt[ 0 ], &hBuf );
			RTF_CHK_RESULT;
			result = rtfBufGetNumber( hBuf, pBufferNumber );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the last packet warning flag
RTF_RESULT rtfWinGetLastPacketWarningFlag( RTF_WIN_HANDLE handle, BOOL *pFlag )
{
	RTF_FNAME( "rtfWinGetLastPacketWarningFlag" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the return
		*pFlag = pWin->lastPktWarningFlag;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the position of the first window byte in the window packet set
RTF_RESULT rtfWinGetFirstByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								   unsigned char *pPktOffset )
{
	RTF_FNAME( "rtfWinGetFirstByteInfo" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the returns
		*phPkt      = pWin->hContextPkt     [ RTF_FIRST_CONTEXT_INDEX ];
		*pPktOffset = pWin->contextPktOffset[ RTF_FIRST_CONTEXT_INDEX ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the position of the last window byte in the window packet set
RTF_RESULT rtfWinGetLastByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								  unsigned char *pPktOffset )
{
	RTF_FNAME( "rtfWinGetLastByteInfo" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the returns
		*phPkt      = pWin->hContextPkt     [ RTF_LAST_CONTEXT_INDEX ];
		*pPktOffset = pWin->contextPktOffset[ RTF_LAST_CONTEXT_INDEX ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the position of the next window byte in the window packet set
RTF_RESULT rtfWinGetNextByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								  unsigned char *pPktOffset )
{
	RTF_FNAME( "rtfWinGetNextByteInfo" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the returns
		*phPkt      = pWin->hNextBytePkt;
		*pPktOffset = pWin->nextBytePktOffset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the position of the prior window byte in the window packet set
RTF_RESULT rtfWinGetPriorByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								   unsigned char *pPktOffset )
{
	RTF_FNAME( "rtfWinGetPriorByteInfo" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the returns
		*phPkt      = pWin->hContextPkt     [ RTF_PRIOR_CONTEXT_INDEX ];
		*pPktOffset = pWin->contextPktOffset[ RTF_PRIOR_CONTEXT_INDEX ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the input stream byte offset of the first byte in the window
// NOTE: this is relative to where the window first started scanning
// the input stream - the session must provide an initial offset!!!
RTF_RESULT rtfWinGetInputByteCount( RTF_WIN_HANDLE handle, INT64 *pInputByteCount )
{
	RTF_FNAME( "rtfWinGetInputByteCount" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make the return
		*pInputByteCount = pWin->inputByteCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the optimization override flag (used to force full parsing at the start of a keyframe)
RTF_RESULT rtfWinSetOptimizationOverride( RTF_WIN_HANDLE handle, BOOL value )
{
	RTF_FNAME( "rtfWinSetOptimizationOverride" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// set the flag
		pWin->overrideOptimize = value;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset the window object prior to first use
RTF_RESULT rtfWinReset( RTF_WIN_HANDLE handle, RTF_SES_HANDLE hSes,
					    INT64 initialInputByteCount, BOOL optimizeForATSC )
{
	RTF_FNAME( "rtfWinReset" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ_HANDLE hBaseObject;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		hBaseObject = pWin->hBaseObject;
		memset( (void *)pWin, 0, sizeof(*pWin) );
		pWin->hBaseObject = hBaseObject;
		pWin->hSes = hSes;
		pWin->lastPktWarningFlag = FALSE;
		pWin->cloneFlag = FALSE;
		pWin->pNextBytePktStorage = (unsigned char *)NULL;
		pWin->pktCount = 0;
		pWin->lastWinPkt = 0;
		pWin->contextIndex = -1;
		for( i=0; i<RTF_MAX_CONTEXT_PACKETS; ++i )
		{
			pWin->hContextPkt     [ i ] = (RTF_PKT_HANDLE)NULL;
			pWin->contextPktOffset[ i ] = 0;
		}
		pWin->nextBytePktIndex = -1;
		pWin->hNextBytePkt = (RTF_PKT_HANDLE)NULL;
		pWin->nextBytePktOffset = 0;
		pWin->nextBytePktBytesRemaining = 0;
		pWin->data = 0xFFFFFFFF;
		pWin->record = FALSE;
		pWin->pRecBuf = (unsigned char *)NULL;
		pWin->recBufBytes = 0;
		pWin->recBufOffset = 0;
		pWin->hRecStartPkt = (RTF_PKT_HANDLE)NULL;
		pWin->inputBPS = 0;
		pWin->inputByteCount = initialInputByteCount;
		pWin->lastPcr.ext.us = 0;
		pWin->lastPcr.base.ull = 0;
		pWin->lastPcrByteCount = 0;
		pWin->pcrValid = FALSE;
		pWin->optimizeForATSC = optimizeForATSC;
		pWin->overrideOptimize = FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// initialize a window to scan an array of packets
// note: carries over prior context- last window packet
// (if there was one) in prior buffer and all subsequent packets
RTF_RESULT rtfWinMapBuffer( RTF_WIN_HANDLE handle, unsigned short pid, RTF_BUF_HANDLE hBuf, BOOL *pFlag )
{
	RTF_FNAME( "rtfWinMapBuffer" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE *phPkt;
	RTF_PIC_HANDLE hPic;
	unsigned long flags;
	unsigned short pktCount;
	int picCount;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// record the window PID (in case we don't have it already)
		pWin->pid = pid;
		// get the packet array info from this buffer
		result = rtfBufGetPacketArrayInfo( hBuf, &pktCount, &phPkt );
		RTF_CHK_RESULT;
		// check for overflow before copying the packet handles into the window's array
		if( ( pWin->pktCount + pktCount ) >= RTF_MAX_WINDOW_PACKETS )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_OVERFLOW, "Parsing window packet array overflow" );
			break;
		}
		// append the new packet handles to any that were carried over
		memcpy( (void *)&(pWin->hPkt[ pWin->pktCount ]), (void *)phPkt, ( pktCount * sizeof(RTF_PKT_HANDLE) ) );
		pWin->pktCount += pktCount;
		// reset the last packet warning flag
		pWin->lastPktWarningFlag = FALSE;
		// scan the packet array for the last window PID packet with a payload
		for( pWin->lastWinPkt=pWin->pktCount-1; pWin->lastWinPkt>=0; --pWin->lastWinPkt )
		{
			result = rtfPktGetPID( pWin->hPkt[ pWin->lastWinPkt ], &pid );
			RTF_CHK_RESULT;
			if( pid == pWin->pid )
			{
				result = rtfPktGetFlags( pWin->hPkt[ pWin->lastWinPkt ], &flags );
				RTF_CHK_RESULT;
				if( ( flags & RTF_PKT_PAYLOADABSENT ) == 0 )
				{
					break;
				}
			}
		}
		// are there any window PID packets in the array beside the first packet?
		if( pWin->lastWinPkt <= 0 )
		{
			// no - set the last packet warning flag
			pWin->lastPktWarningFlag = TRUE;
			result = rtfSesGetCurrentPicInfo( pWin->hSes, &hPic, &picCount );
			RTF_CHK_RESULT;
			// is there an active picture AND is this the main parsing window?
			if( ( picCount > 0 ) && ( pWin->cloneFlag == FALSE ) )
			{
				// yes - retain the first packet in the array if it is a video packet,
				// but attach the others to the current picture. We assume that the change
				// in order of the one video packet versus the rest of the non-video packets
				// in the array does not produce any harmful side-effects.
				for( i=pWin->lastWinPkt+1; i<pktCount; ++i )
				{
					result = rtfPicAddPacket( hPic, pWin->hPkt[ i ] );
					RTF_CHK_RESULT;
				}
				RTF_CHK_RESULT_LOOP;
			}
			// reset the packet count
			pWin->pktCount = pWin->lastWinPkt+1;
			// reset the next byte packet index
			pWin->nextBytePktIndex = pWin->lastWinPkt;
			// if there are no video packets at all, then reset
			// the next byte info to force a buffer advance
			if( pWin->lastWinPkt < 0 )
			{
				pWin->nextBytePktBytesRemaining = 0;
				pWin->hNextBytePkt = (RTF_PKT_HANDLE)NULL;
			}
		} // if( pWin->lastWinPkt < 0 )
		// return the warning flag
		*pFlag = pWin->lastPktWarningFlag;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// advance the window to the next start code, if there is one in the current buffer
// NOTE: This routine is critical to performance!
#if DO_BURST_XFERS
// !!! NOTE: THIS VERSION RELIES ON THE ASSUMPTION THAT ALL BUFFERS ARE ALIGNED TO !!!
// !!! A BURST BOUNDARY AND THAT THEY CONTAIN A MULTIPLE OF BURST_BYTES OF DATA !!!
#endif
RTF_RESULT rtfWinFindNextStartCode( RTF_WIN_HANDLE handle, unsigned long *pData )
{
	RTF_FNAME( "rtfWinFindNextStartCode" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PIC_HANDLE hPic;
	RTF_PKT_HANDLE *pSrc, *pDst;
	RTF_WIN saveWin;
	BOOL foundPacketFlag;
	unsigned long data;
	unsigned long junk;
	unsigned long pktNumber;
	unsigned long flags;
	unsigned short pid;
	unsigned char pesOff;
	unsigned char pesLen;
	int contextIndex;
	int i, picCount, byteCount;
#if DO_BURST_XFERS
	unsigned long frac;
	BURST *pBurst, *pNextBurst;
	unsigned char burstBuffer[ 2*BURST_BYTES ];
#endif

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		*pData = TRANSPORT_INVALID_DATA;
#if DO_BURST_XFERS
		// set the burst buffer pointer (make sure it's aligned!)
		pBurst = BURST_ALIGN( &(burstBuffer[BURST_BYTES]) );
		if( pWin->pNextBytePktStorage != NULL )
		{
			// set the initial burst input data pointer (make sure it's aligned!)
			pNextBurst = (BURST *)( pWin->pNextBytePktStorage + pWin->nextBytePktOffset );
			pNextBurst = BURST_ALIGN( pNextBurst );
			// get the initial burst of input data
			*pBurst = *pNextBurst++;
		}
#endif
		// check to see if there is an active picture (for future referece)
		// note: the picture cannot change while we are in this routine
		result = rtfSesGetCurrentPicInfo( pWin->hSes, &hPic, &picCount );
		RTF_CHK_RESULT;
		// cache the context index
		contextIndex = pWin->contextIndex;
		// cache the context data
		data = pWin->data;
		// start counting bytes
		byteCount = 0;
		// outer loop moving the window forward one byte at a time
		// and checking the data for a start code prefix
		for( ;; )
		{
			// is there at least one more byte in the current transport packet?
			if( pWin->nextBytePktBytesRemaining == 0 )
			{
				// no - advance to the next packet
				// inner loop looking for the next window PID packet in this buffer
				// Note: the code that follows is a customized inline version of rtfWinNextPacket
				foundPacketFlag = TRUE;
				for( ;; )
				{
					// increment the next packet index - is there another packet?
					if( ++pWin->nextBytePktIndex >= pWin->pktCount )
					{
						// no - no window packets are left - reset the next packet info
						pWin->pNextBytePktStorage = (RTF_PKT_HANDLE)NULL;
						pWin->pktCount = 0;
						pWin->nextBytePktOffset = 0;
						pWin->nextBytePktBytesRemaining = 0;
						pWin->nextBytePktIndex = -1;
						// set the warning flags
						pWin->lastPktWarningFlag = TRUE;
						foundPacketFlag = FALSE;
						// escape the search loop
						break;
					}
					// there is another packet - get the handle
					pWin->hNextBytePkt = pWin->hPkt[ pWin->nextBytePktIndex ];
					// get the info on this packet
					result = rtfPktGetInfo( pWin->hNextBytePkt, &pktNumber, &pid, &flags,
											&pWin->pNextBytePktStorage, &pWin->nextBytePktOffset,
											&pWin->nextBytePktBytesRemaining, &pesOff, &pesLen );
					RTF_CHK_RESULT;
					// is there an active picture AND is this the main parsing window?
					if( ( picCount > 0 ) && ( pWin->cloneFlag == FALSE ) )
					{
						// yes - add this packet to the picture
						result = rtfPicAddPacket( hPic, pWin->hNextBytePkt );
						RTF_CHK_RESULT;
					}
					// is this packet at or beyond the last window PID payload packet?
					if( pWin->nextBytePktIndex >= pWin->lastWinPkt )
					{
						// yes - stop the search here - set the warning flag
						pWin->lastPktWarningFlag = TRUE;
						// are there any packets left in the current packet array?
						if( pWin->nextBytePktIndex < pWin->pktCount )
						{
							// yes - this means that there is exactly one window PID
							// packet left. move the unscanned packets to the start
							// of the packet array. retain the current context data.
							// reset the array index and count.
							pWin->pktCount = pWin->pktCount - pWin->nextBytePktIndex;
							pSrc = &pWin->hPkt[ pWin->lastWinPkt ];
							pDst = &pWin->hPkt[ 0 ];
							pWin->nextBytePktIndex = 0;
							for( i=0; i<pWin->pktCount; ++i )
							{
								*pDst++ = *pSrc++;
							}
						}
						else
						{
							// no - no window PID packets are left
							// reset the next packet info
							pWin->pNextBytePktStorage = (unsigned char *)NULL;
							pWin->pktCount = 0;
							pWin->nextBytePktOffset = 0;
							pWin->nextBytePktBytesRemaining = 0;
							pWin->nextBytePktIndex = -1;
							foundPacketFlag = FALSE;
						}
						//  escape the inner search loop
						break;
					} // if( pWin->nextBytePktIndex >= pWin->lastWinPkt )
					// no, this is not at or beyond the last window-PID packet with a payload
					// is this a window PID packet?
					if( pid != pWin->pid )
					{
						// no - go around again
						byteCount += TRANSPORT_PACKET_BYTES;
						continue;
					}
					// does this packet have a PCR?
					if( ( flags & RTF_PKT_PCRPRESENT ) != 0 )
					{
						// yes - get the PCR value from the packet
						result = rtfPktGetPcrTimestamp( pWin->hNextBytePkt, &pWin->lastPcr );
						RTF_CHK_RESULT;
						// record the byte count at the last byte of the PCR base field
						pWin->lastPcrByteCount = pWin->inputByteCount + byteCount + 10;
						// set the "PCR valid" flag
						pWin->pcrValid = TRUE;
					}
					// this is a window PID packet. is this packet encrypted?
					if( ( flags & RTF_PKT_PAYLOADENCRYPTED ) != 0 )
					{
						// yes - flush the window context data and go around again
						data = 0xFFFFFFFF;
						byteCount += TRANSPORT_PACKET_BYTES;
						continue;
					}
					// not encrypted - is there a payload in this packet?
					if( ( flags & RTF_PKT_PAYLOADABSENT ) != 0 )
					{
						// no - keep looking
						continue;
					}
					// found a window PID packet with a payload
					// is the payload unit start flag set?
					if( ( flags & RTF_PKT_PAYLOADUNITSTART ) != 0 )
					{
						// !!! UGLY ALERT !!! THIS IS REALLY UGLY !!!
						// !!! BUT I CAN'T THINK OF A BETTER WAY TO DO IT !!!
						// !!! NEED TO PARSE PES HEADER WITHOUT DISRUPTING !!!
						// !!! THE PARSING OF THE PES PAYLOAD !!!
						// !!! THIS CODE ASSUMES THAT THE PES HEADER IS THE FIRST THING !!!
						// !!! IN THE PAYLOAD! THE SPEC SAYS THIS MUST BE THE CASE, BUT !!!
						// !!! IS THIS ALWAYS TRUE? !!!
						// yes - save the current state of the parsing context
						saveWin.data = data;
						saveWin.contextIndex = contextIndex;
						for( i=0; i<RTF_MAX_CONTEXT_PACKETS; ++i )
						{
							saveWin.hContextPkt[ i ] = pWin->hContextPkt[ i ];
							saveWin.contextPktOffset[ i ] = pWin->contextPktOffset[ i ];
						}
						// advance to the PES header start code
						result = rtfWinAdvance( handle, 4, &junk );
						RTF_CHK_RESULT;
						// start a new PES packet here
						result = rtfSesStartPes( pWin->hSes );
						RTF_CHK_RESULT;
						// restore the previous state of the window
						data = saveWin.data;
						contextIndex = saveWin.contextIndex;
						for( i=0; i<RTF_MAX_CONTEXT_PACKETS; ++i )
						{
							pWin->hContextPkt[ i ] = saveWin.hContextPkt[ i ];
							pWin->contextPktOffset[ i ] = saveWin.contextPktOffset[ i ];
						}
						// found the next packet - escape the inner search loop and scan this packet
						break;
					} // if( ( flags & RTF_PKT_PAYLOADUNITSTART ) != 0 )
					// do we really want to scan this packet?
					if( ( pWin->optimizeForATSC == FALSE ) || ( pWin->overrideOptimize != FALSE ) )
					{
						// yes - escape the inner search loop and scan this packet
						break;
					}
					// no - flush the window context data and go around again
					data = 0xFFFFFFFF;
					byteCount += TRANSPORT_PACKET_BYTES;
				} // for( ;; ) inner
				RTF_CHK_RESULT_LOOP;
				// was a suitable packet found?
				if( foundPacketFlag == FALSE )
				{
					// no - there is no packet to add to the picture in this buffer
					// escape the outer search loop
					break;
				}
				// Note: end customized inline version of rtfWinNextPacket
				// was the last packet warning flag set?
				if( pWin->lastPktWarningFlag != FALSE )
				{
					// yes - last packet warning means no start code found
					// escape the outer search loop
					break;
				}
				// does the packet have a payload?
				if( ( flags & RTF_PKT_PAYLOADABSENT ) != 0 )
				{
					// no - continue searching in the outer loop
					pWin->nextBytePktBytesRemaining = 0;
					continue;
				}
#if DO_BURST_XFERS
				// set the input pointer to the "burst" that contains the first payload byte
				pNextBurst = (BURST *)( pWin->pNextBytePktStorage + pWin->nextBytePktOffset );
				pNextBurst = BURST_ALIGN( pNextBurst );
				// get the initial burst of input data from this packet
				*pBurst = *pNextBurst++;
#endif
			} // if( pWin->nextBytePktBytesRemaining == 0 )
			// NOTE: some of the following operations have been re-ordered from the
			// logical order to an interleaved sequence in order to improve performance
			// adjust the ring buffer index
			// shift the context up one byte
			// adjust offsets and byte counts
			data <<= 8;
			contextIndex = ( contextIndex-1 ) & RTF_CONTEXT_INDEX_MASK;
			--pWin->nextBytePktBytesRemaining;
			++byteCount;
			// copy the new last context byte into the data buffer
#if DO_BURST_XFERS
			// is the next byte in the next burst of input data?
			frac = ( (unsigned long)( pWin->pNextBytePktStorage + pWin->nextBytePktOffset ) ) & BURST_MASK;
			if( frac == 0 )
			{
				// yes - get the next burst of input data
				*pBurst = *pNextBurst++;
			}
			// copy the new last context byte into the data buffer
			data |= pBurst->bytes[ BURST_INDEX( frac ) ];
#else
			data |= *( pWin->pNextBytePktStorage + pWin->nextBytePktOffset );
#endif
			// the new last byte in the context is the previous next byte
			*( pWin->hContextPkt + contextIndex ) = pWin->hNextBytePkt;
			*( pWin->contextPktOffset + contextIndex ) = pWin->nextBytePktOffset++;
#ifdef DO_NATIVE64
			// mask the data to 32 bits
			data &= 0xFFFFFFFF;
#endif
			// does the window contain a start code?
			if( ( data >> 9 ) == 0 )
			{
				if( ( data & 0x100 ) != 0 )
				{
					// yes - is this a PES packet header start code?
					if( ( data & 0xFF ) <= 0xBC )
					{
						// no - escape the search loop
						break;
					}
					// yes - parse the PES packet header and continue
					// uncache context index
					pWin->contextIndex = contextIndex;
					result = rtfSesStartPes( pWin->hSes );
					RTF_CHK_RESULT;
					// re-cache context index
					contextIndex = pWin->contextIndex;
					// !!! FIX ME !!! SHOULD RESTORE CONTEXT DATA FROM BEFORE PES HEADER !!!
					// flush the context data
					data = 0xFFFFFFFF;
				}
			}
		} // for( ;; ) outer
		RTF_CHK_RESULT_LOOP;
		// return the current window data to the caller
		*pData = data;
		// update the byte count
		pWin->inputByteCount += byteCount;
		// uncache the context ring buffer index
		pWin->contextIndex = contextIndex;
		// uncache the context data
		pWin->data = data;

	} while( 0 );		// error escape wrapper - end

	return result;
}

// advance the window N bytes forward
RTF_RESULT rtfWinAdvance( RTF_WIN_HANDLE handle, int bytes, unsigned long *pData )
{
	RTF_FNAME( "rtfWinAdvance" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned char contextIndex;
	unsigned long data;
	int i, byteCount;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		*pData = TRANSPORT_INVALID_DATA;
		// cache the context index
		contextIndex = pWin->contextIndex;
		// cache the context data
		data = pWin->data;
		// start counting bytes
		byteCount = 0;
		// can we skip forward?
		if( bytes > 5 )
		{
			if( pWin->record == FALSE )
			{
				// yes - skip forward to 5 bytes before the end of this request
				// or the end of the packet, whichever comes first
				while( (bytes-5) > pWin->nextBytePktBytesRemaining )
				{
					bytes -= pWin->nextBytePktBytesRemaining;
					pWin->inputByteCount += pWin->nextBytePktBytesRemaining;
					// advance to the next packet
					result = rtfWinNextPacket( pWin );
					RTF_CHK_RESULT;
				}
				RTF_CHK_RESULT_LOOP;
				pWin->nextBytePktOffset += (unsigned char)( (bytes-5) & 0xFF );
				pWin->nextBytePktBytesRemaining -= (unsigned char)( (bytes-5) & 0xFF );
				pWin->inputByteCount += (bytes-5);
				bytes = 5;
			}
		}
		for( i=0; i<bytes; ++i )
		{
			// is there another byte in the current transport packet?
			if( pWin->nextBytePktBytesRemaining <= 0 )
			{
				// no - advance to the next packet
				result = rtfWinNextPacket( pWin );
				RTF_CHK_RESULT;
			}
			// shift the context up one byte
			data <<= 8;
			// make a corresponding adjustment in the ring buffer index
			--contextIndex;
			contextIndex &= RTF_CONTEXT_INDEX_MASK;
			// the new last byte in the context is the previous next byte
			pWin->hContextPkt     [ contextIndex ] = pWin->hNextBytePkt;
			pWin->contextPktOffset[ contextIndex ] = pWin->nextBytePktOffset;
			// copy the new last context byte into the data buffer
			data |= pWin->pNextBytePktStorage[ pWin->nextBytePktOffset ];
			// adjust offsets and byte counts
			++pWin->nextBytePktOffset;
			--pWin->nextBytePktBytesRemaining;
			++byteCount;
			// is data recording enabled?
			if( pWin->record != FALSE )
			{
				// yes - is there another byte in the record buffer?
				if( pWin->recBufOffset >= pWin->recBufBytes )
				{
					// no - issue a warning and turn off recording
					RTF_LOG_WARN0( RTF_MSG_WRN_RECORDBUFFULL, "Data record buffer full - recording disabled" );
					pWin->record = FALSE;
				}
				else
				{
					// add the new byte to the record buffer
					pWin->pRecBuf[ pWin->recBufOffset++ ] = (unsigned char)( data & 0xFF );
				}
			}
		} // for( i=0; i<bytes; ++i )
		RTF_CHK_RESULT_LOOP;
		// return the data
		*pData = data;
		// update the byte count
		pWin->inputByteCount += byteCount;
		// uncache the context index
		pWin->contextIndex = contextIndex;
		// uncache the context data
		pWin->data = data;

	} while( 0 );		// error escape wrapper - end

	return result;
}

// advance the window N bytes forward (N < 5)
RTF_RESULT rtfWinSmallAdvance( RTF_WIN_HANDLE handle, int bytes, unsigned long *pData )
{
	RTF_FNAME( "rtfWinSmallAdvance" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned char contextIndex;
	unsigned long data;
	int i, byteCount;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// cache the context index
		contextIndex = pWin->contextIndex;
		// cache the context data
		data = pWin->data;
		// start counting bytes
		byteCount = 0;
		*pData = TRANSPORT_INVALID_DATA;
		for( i=0; i<bytes; ++i )
		{
			// is there another byte in the current transport packet?
			if( pWin->nextBytePktBytesRemaining <= 0 )
			{
				// no - advance to the next packet
				result = rtfWinNextPacket( pWin );
				RTF_CHK_RESULT;
			}
			// shift the context up one byte
			data <<= 8;
			// make a corresponding adjustment in the ring buffer index
			--contextIndex;
			contextIndex &= RTF_CONTEXT_INDEX_MASK;
			// the new last byte in the context is the previous next byte
			pWin->hContextPkt     [ contextIndex ] = pWin->hNextBytePkt;
			pWin->contextPktOffset[ contextIndex ] = pWin->nextBytePktOffset;
			// copy the new last context byte into the data buffer
			data |= pWin->pNextBytePktStorage[ pWin->nextBytePktOffset ];
			// adjust offsets and byte counts
			++pWin->nextBytePktOffset;
			--pWin->nextBytePktBytesRemaining;
			++byteCount;
			// is data recording enabled?
			if( pWin->record != FALSE )
			{
				// yes - is there another byte in the record buffer?
				if( pWin->recBufOffset >= pWin->recBufBytes )
				{
					// no - issue a warning and turn off recording
					RTF_LOG_WARN0( RTF_MSG_WRN_RECORDBUFFULL, "Data record buffer full - recording disabled" );
					pWin->record = FALSE;
				}
				else
				{
					// add the new byte to the record buffer
					pWin->pRecBuf[ pWin->recBufOffset++ ] = (unsigned char)( data & 0xFF );
				}
			}
		}
		RTF_CHK_RESULT_LOOP;
		// return the data
		*pData = data;
		// update the byte count
		pWin->inputByteCount += byteCount;
		// uncache the context index
		pWin->contextIndex = contextIndex;
		// uncache the context data
		pWin->data = data;

	} while( 0 );		// error escape wrapper - end

	return result;
}

// begin recording all data that appears in the window in the buffer provided
RTF_RESULT rtfWinStartRecord( RTF_WIN_HANDLE handle, unsigned char *pBuf, int bufBytes )
{
	RTF_FNAME( "rtfWinStartRecord" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// make sure the buffer is at least big enough to record the current data
		if( bufBytes < 4 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Buffer too small" );
			break;
		}
		// set up for recording
		pWin->record = TRUE;
		pWin->pRecBuf = pBuf;
		pWin->recBufBytes = bufBytes;
		// record the current window data
		pWin->pRecBuf[ 0 ] = (unsigned char)( ( pWin->data >> 24 ) & 0xFF );
		pWin->pRecBuf[ 1 ] = (unsigned char)( ( pWin->data >> 16 ) & 0xFF );
		pWin->pRecBuf[ 2 ] = (unsigned char)( ( pWin->data >>  8 ) & 0xFF );
		pWin->pRecBuf[ 3 ] = (unsigned char)( pWin->data & 0xFF );
		pWin->recBufOffset = 4;
		// record the position at which recording started (for debug)
		pWin->hRecStartPkt   = pWin->hContextPkt     [ RTF_FIRST_CONTEXT_INDEX ];
		pWin->recStartOffset = pWin->contextPktOffset[ RTF_FIRST_CONTEXT_INDEX ];

	} while( 0 );		// error escape wrapper - end

	return result;
}

// stop recording window data - return the number of bytes recorded
RTF_RESULT rtfWinStopRecord( RTF_WIN_HANDLE handle, int *pBytes )
{
	RTF_FNAME( "rtfWinStopRecord" );
	RTF_OBASE( handle );
	RTF_WIN *pWin = (RTF_WIN *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// turn off recording
		pWin->record = FALSE;
		// return the number of bytes recorded
		*pBytes = pWin->recBufOffset;
		// reset the recording params
		pWin->pRecBuf = (unsigned char *)NULL;
		pWin->recBufBytes = 0;
		pWin->recBufOffset = 0;

	} while( 0 );		// error escape wrapper - end

	return result;
}

// get the interpolated PCR time in 90 KHz clock ticks
RTF_RESULT rtfWinGetPcrTime( RTF_WIN_HANDLE handle, INT64 *pPcrTime )
{
	RTF_FNAME( "rtfWinGetPcrTime" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_WIN *pWin = (RTF_WIN *)handle;
	INT64 byteCount;
	INT64 pcrTime;
	INT64 delTime;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pWin, RTF_OBJ_TYPE_WIN );
		// do we already have the input bit rate?
		if( pWin->inputBPS == 0 )
		{
			// no - ask the session for the input bit rate
			result = rtfSesGetInputBitrate( pWin->hSes, &pWin->inputBPS );
			RTF_CHK_RESULT;
		}
		// get the value of the last observed PCR in 90 KHz clock ticks
		pcrTime = pWin->lastPcr.base.ull;
		// subtract the byte count of the last observed PCR from the current byte count
		byteCount = pWin->inputByteCount - pWin->lastPcrByteCount;
		// compute the delta between the current window position
		// and the position of the last observed PCR in 90 KHz clock ticks
		byteCount *= TRANSPORT_PCR_TICKS_PER_SECOND * 8;
		delTime = RTF_DIV64( byteCount, pWin->inputBPS );
		// add the delta time to the pcrTime to get the current PCR time in milliseconds
		pcrTime += delTime;
		// make the return
		*pPcrTime = pcrTime;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// copy one window's state information into another
// (used to "bookmark" locations in CODEC object)
RTF_RESULT rtfWinCopyState( RTF_WIN_HANDLE hSource, RTF_WIN_HANDLE hDestination )
{
	RTF_FNAME( "rtfWinCopyState" );
	RTF_OBASE( hSource );
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ_HANDLE hObj;
	RTF_WIN *pSrc = (RTF_WIN *)hSource;
	RTF_WIN *pDst = (RTF_WIN *)hDestination;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSrc, RTF_OBJ_TYPE_WIN );
		RTF_CHK_OBJ( pDst, RTF_OBJ_TYPE_WIN );
		// preserve the embedded object handle
		hObj = pDst->hBaseObject;
		// copy the state structure
		memcpy( (void *)pDst, (void *)pSrc, sizeof(RTF_WIN) );
		// restore the embedded object handle
		pDst->hBaseObject = hObj;
		// set the "clone" flag in the copy
		pDst->cloneFlag = TRUE;

	} while( 0 ); // error escape wrapper - end

	return result;
}
