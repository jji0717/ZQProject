// implementation file for rtfBuf class
// abstracts input / output buffer
//

#include "RTFPrv.h"

// constants ****************************************************************************

#define RTF_BUF_SYNC_COUNT		2
#define RTF_BUF_MAX_REFERENCES	1024

// typedefs *****************************************************************************

typedef struct _RTF_BUF
{
	RTF_OBJ_HANDLE hBaseObject;

	RTF_BUFSTATE state;				// state of the buffer
	RTF_BUFTYPE type;				// type of the buffer
	RTF_SES_HANDLE hSes;			// the session that owns this buffer
	RTF_WIN_HANDLE hWin;			// the scanning window of the owning session
	void *hAppSession;				// a handle that the app associates with this session
	void *hAppFile;					// a handle that the app associates with this file
	void *hAppBuffer;				// a handle that the app associates with this buffer
	unsigned long bufferNumber;		// session's buffer number
	unsigned long capacity;			// buffer capacity in bytes
	unsigned char *pBase;			// address of start of buffer
	unsigned long fillOffset;		// offset of next byte to fill
	unsigned long drainOffset;		// offset of next byte to drain
	int referenceCount;				// number of packets in this buffer that are
									// referenced by active picture objects
#ifdef DO_TRACKREFCOUNTS
	RTF_HANDLE hRefHolder[ RTF_BUF_MAX_REFERENCES ]; // handles of objects holding a reference to this buffer
#endif
	// buffer in which to re-unite fragmented leading packet
	unsigned char spanningPacket[ TTS_PACKET_BYTES ];
	short maxPacketCount;			// capacity of packet handle array, below
	short activePacketCount;		// number of active packets in array below
	RTF_PKT_HANDLE *phPacket;		// array of packet object handles

} RTF_BUF;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfBufGetStorageRequirement( RTF_BUFTYPE bufType, short maxPacketCount )
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfBufGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_BUF) + ( ( (unsigned long)maxPacketCount ) * sizeof(RTF_PKT_HANDLE) );
	bytes += rtfObjGetStorageRequirement();
	bytes += ( (unsigned long)maxPacketCount ) * rtfPktGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfBufConstructor( RTF_BUF_HANDLE *pHandle, RTF_BUFTYPE type, short maxPacketCount,
							  RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfBufConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_BUF *pBuf;
	int i;

	do {		 // error escape wrapper - begin

		// make sure the requested buffer count is within the maximum range
		// (this is determined by the fact that we use a short for the packet index)
		if( maxPacketCount > RTF_MAX_BUFFER_PACKETS )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "Requested buffer count (%d) exceeds maximum (%d)", maxPacketCount, RTF_MAX_BUFFER_PACKETS );
			break;
		}
		// allocate a state structure for the buffer object
		// followed by an array of packet object handles
		pBuf = (RTF_BUF *)rtfAlloc( sizeof(RTF_BUF) + ( ( (unsigned long)maxPacketCount ) * sizeof(RTF_PKT_HANDLE) ) );
		RTF_CHK_ALLOC( pBuf );
		// return the handle
		*pHandle = (RTF_BUF_HANDLE)pBuf;
		// clear the state structure
		memset( (void *)pBuf, 0, sizeof(*pBuf) + ( (unsigned long)maxPacketCount ) * sizeof(RTF_PKT_HANDLE) );
		// create an embedded base object
		result = rtfObjConstructor( RTF_OBJ_TYPE_BUF, (RTF_HANDLE)pBuf, hParent, &pBuf->hBaseObject );
		RTF_CHK_RESULT;
		// record the type of the buffer
		pBuf->type = type;
		// create a set of packet descriptor objects
		pBuf->phPacket = (RTF_PKT_HANDLE *)(pBuf + 1);
		pBuf->maxPacketCount = maxPacketCount;
		for( i=0; i<maxPacketCount; ++i )
		{
			result = rtfPktConstructor( &pBuf->phPacket[i], (RTF_HANDLE)pBuf );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// record the handle of the owning session for efficiency
		result = rtfObjGetSession( (RTF_HANDLE)pBuf, &pBuf->hSes );
		RTF_CHK_RESULT;
		// set the initial state of the buffer
		pBuf->state = RTF_BUFSTATE_RELEASED;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfBufDestructor( RTF_BUF_HANDLE handle )
{
	RTF_FNAME( "rtfBufDestructor" );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// destroy the set of transport packet descriptor objects
		for( i=0; i<pBuf->maxPacketCount; ++i )
		{
			if( pBuf->phPacket[i] != (RTF_PKT_HANDLE)NULL )
			{
				result = rtfPktDestructor( pBuf->phPacket[i] );
				RTF_CHK_RESULT;
			}
		}
		// escape on error in nested loop
		RTF_CHK_RESULT_LOOP;
		// destroy the embedded base object
		result = rtfObjDestructor( pBuf->hBaseObject, RTF_OBJ_TYPE_BUF );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// set the state of the buffer
RTF_RESULT rtfBufSetState( RTF_BUF_HANDLE handle, RTF_BUFSTATE state )
{
	RTF_FNAME( "rtfBufSetState" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// make sure the state is a valid one
		switch( state )
		{
		case RTF_BUFSTATE_RELEASED:
		case RTF_BUFSTATE_MAPPED:
		case RTF_BUFSTATE_PROCESSED:
			break;
		default:
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Unrecognized buffer state" );
		}
		RTF_CHK_RESULT_LOOP;
		// record the new state
		pBuf->state = state;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the state of the buffer
RTF_RESULT rtfBufGetState( RTF_BUF_HANDLE handle, RTF_BUFSTATE *pState )
{
	RTF_FNAME( "rtfBufGetState" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// make the return
		*pState = pBuf->state;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the buffer number
RTF_RESULT rtfBufGetNumber( RTF_BUF_HANDLE handle, unsigned long *pNumber )
{
	RTF_FNAME( "rtfBufGetNumber" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// make the return
		*pNumber = pBuf->bufferNumber;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the reference count of the buffer
RTF_RESULT rtfBufGetReferenceCount( RTF_BUF_HANDLE handle, unsigned long *pReferenceCount )\
{
	RTF_FNAME( "rtfBufGetReferenceCount" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// make the return
		*pReferenceCount = pBuf->referenceCount;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef DO_TRACKREFCOUNTS
// get the handle of an object that references this buffer
RTF_RESULT rtfBufGetRefHandle( RTF_BUF_HANDLE handle, int index, RTF_HANDLE *phRefHolder )
{
	RTF_FNAME( "rtfBufGetRefHande" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// make sure the index is in range
		if( index >= pBuf->referenceCount )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_NOTFOUND, "Index (%d) out of range", index );
			break;
		}
		// make the return
		*phRefHolder = pBuf->hRefHolder[ index ];
		
	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif // #ifdef DO_TRACKREFCOUNTS

// get mapping info from the buffer
RTF_RESULT rtfBufGetMapInfo( RTF_BUF_HANDLE handle, RTF_BUFSTATE *pState,
							 RTF_APP_SESSION_HANDLE *phAppSession,
							 RTF_APP_FILE_HANDLE *phAppFile,
							 RTF_APP_BUFFER_HANDLE *phAppBuffer,
							 unsigned long *pBufferNumber, unsigned char **ppBase,
							 unsigned long *pCapacity, unsigned long *pOccupancy )
{
	RTF_FNAME( "rtfBufGetMapInfo" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// make the returns
		*pBufferNumber = pBuf->bufferNumber;
		*phAppSession  = pBuf->hAppSession;
		*phAppFile     = pBuf->hAppFile;
		*phAppBuffer   = pBuf->hAppBuffer;
		*pState        = pBuf->state;
		*ppBase        = pBuf->pBase;
		*pCapacity     = pBuf->capacity;
		*pOccupancy    = pBuf->fillOffset - pBuf->drainOffset;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of active packets in the buffer
RTF_RESULT rtfBufGetPacketCount( RTF_BUF_HANDLE handle, short *pPacketCount )
{
	RTF_FNAME( "rtfBufGetPacketCount" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_GE( pBuf, RTF_BUFSTATE_MAPPED );
		// make the return
		*pPacketCount = pBuf->activePacketCount;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get a packet from the buffer
RTF_RESULT rtfBufGetPacket( RTF_BUF_HANDLE handle, short packetIndex, RTF_PKT_HANDLE *phPacket )
{
	RTF_FNAME( "rtfBufGetPacket" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_GE( pBuf, RTF_BUFSTATE_MAPPED );
		// make sure the index is valid
		if( packetIndex >= pBuf->maxPacketCount )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Index out of range" );
			break;
		}
		// make the return
		*phPacket = pBuf->phPacket[ packetIndex ];
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get packet array info from the buffer
RTF_RESULT rtfBufGetPacketArrayInfo( RTF_BUF_HANDLE handle, unsigned short *pPacketCount, RTF_PKT_HANDLE **pphPacket )
{
	RTF_FNAME( "rtfBufGetPacketArrayInfo" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_GE( pBuf, RTF_BUFSTATE_MAPPED );
		// make the returns
		*pPacketCount = pBuf->activePacketCount;
		*pphPacket = pBuf->phPacket;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the current fill point address from the buffer
RTF_RESULT rtfBufGetFillPointer( RTF_BUF_HANDLE handle, unsigned char **ppFill )
{
	RTF_FNAME( "rtfBufGetFillPointer" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_GE( pBuf, RTF_BUFSTATE_MAPPED );
		// make the return
		*ppFill = pBuf->pBase + pBuf->fillOffset;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the current drain point address from the buffer
RTF_RESULT rtfBufGetDrainPointer( RTF_BUF_HANDLE handle, unsigned char **ppDrain )
{
	RTF_FNAME( "rtfBufGetDrainPointer" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_GE( pBuf, RTF_BUFSTATE_MAPPED );
		// make the return
		*ppDrain = pBuf->pBase + pBuf->drainOffset;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset a buffer descriptor
RTF_RESULT rtfBufReset( RTF_BUF_HANDLE handle, BOOL isClose )
{
	RTF_FNAME( "rtfBufReset" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		if( isClose == FALSE )
		{
			RTF_CHK_REFCOUNT( pBuf );
		}
		// is this buffer currently mapped?
		if( pBuf->state != RTF_BUFSTATE_RELEASED )
		{
			// yes - reset all packet objects in the packet descriptor array
			for( i=0; i<pBuf->maxPacketCount; ++i )
			{
				if( pBuf->phPacket[ i ] != (RTF_PKT_HANDLE)NULL )
				{
					result = rtfPktReset( pBuf->phPacket[ i ], isClose );
					RTF_CHK_RESULT;
				}
			}
		}
		// reset the state info
		pBuf->state = RTF_BUFSTATE_RELEASED;
		pBuf->pBase = (unsigned char *)NULL;
		pBuf->capacity = 0;
		pBuf->activePacketCount = 0;
		pBuf->referenceCount = 0;
#ifdef _DEBUG
		pBuf->bufferNumber = 0;
		pBuf->fillOffset = 0;
		pBuf->drainOffset = 0;
		memset( (void *)pBuf->spanningPacket, 0, sizeof(pBuf->spanningPacket) );
#endif
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// map a buffer to a block of storage
RTF_RESULT rtfBufMap( RTF_BUF_HANDLE handle,
					  RTF_APP_SESSION_HANDLE hAppSession,
					  RTF_APP_FILE_HANDLE hAppFile,
					  RTF_APP_BUFFER_HANDLE hAppBuffer,
					  unsigned long bufferNumber, unsigned char *pBase,
					  unsigned long capacity, unsigned long occupancy )
{
	RTF_FNAME( "rtfBufMap" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_EQ( pBuf, RTF_BUFSTATE_RELEASED );
		// record the session's buffer number
		pBuf->bufferNumber = bufferNumber;
		// record the app handles
		pBuf->hAppSession = hAppSession;
		pBuf->hAppFile = hAppFile;
		pBuf->hAppBuffer = hAppBuffer;
		// record the base pointer and the capacity
		pBuf->pBase = pBase;
		pBuf->capacity = capacity;
		// reset the drain offset
		pBuf->drainOffset = 0;
		// set the fill offset
		pBuf->fillOffset = occupancy;
		// reset the active packet count
		pBuf->activePacketCount = 0;
		// record the handle of the session's scanning window for efficiency
		result = rtfSesGetWindow( pBuf->hSes, &pBuf->hWin );
		// set the state to mapped
		pBuf->state = RTF_BUFSTATE_MAPPED;
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// map the contents of the buffer to packet objects
RTF_RESULT rtfBufMapInputPackets( RTF_BUF_HANDLE handle, BOOL *pInSync, BOOL *pInputIsTTS,
								  BOOL *pFirstPcrAcquired, unsigned char *pFragment,
								  unsigned char *pFragmentBytes,
								  unsigned long *pTotalPacketCount,
								  unsigned long *pTotalMappedPacketCount )
{
	RTF_FNAME( "rtfBufMapInputPackets" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	BOOL isAugmentation;
	int i, j;
	int syncByteOffset;
	int remaining;
	unsigned char *pNext;
	unsigned char fragmentBytes = *pFragmentBytes;
	unsigned char packetFinishBytes;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_EQ( pBuf, RTF_BUFSTATE_MAPPED );
		// make sure that the active packet count is 0
		if( pBuf->activePacketCount != 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Non-zero active packet count" );
			break;
		}
		// init the position at the start of the buffer
		pNext = pBuf->pBase;
		remaining = pBuf->fillOffset - pBuf->drainOffset;
		syncByteOffset = ( *pInputIsTTS == FALSE ) ? TRANSPORT_SYNCBYTE_OFFSET : TTS_SYNCBYTE_OFFSET;
		// was a partial packet left over from the previous buffer?
		if( fragmentBytes > 0 )
		{
			// yes - make sure there is enough data in this buffer to complete it
			packetFinishBytes = TRANSPORT_PACKET_BYTES - fragmentBytes;
			if( remaining < packetFinishBytes )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_UNDERFLOW, "Insufficient data to complete partial packet" );
				break;
			}
			// form a complete packet locally by combining the fragment with data from this buffer
			memcpy( (void *)pBuf->spanningPacket,  (void *)pFragment, fragmentBytes );
			memcpy( (void *)&(pBuf->spanningPacket[ fragmentBytes ]), (void *)pNext, packetFinishBytes );
			// check with the session to see if this is an augmentation packet
			result = rtfSesGetAugmentationPktStatus( pBuf->hSes, pBuf->spanningPacket + syncByteOffset, &isAugmentation );
			RTF_CHK_RESULT;
			if( isAugmentation == FALSE )
			{
				// map the completed packet
				result = rtfPktMap( pBuf->phPacket[ pBuf->activePacketCount++ ], handle,
									pBuf->spanningPacket, (*pTotalPacketCount)++,
									(*pTotalMappedPacketCount)++, pFirstPcrAcquired );
				RTF_CHK_RESULT;
			}
			else
			{
				// just count it
				++(*pTotalPacketCount);
			}
			// null out the calling routine's packet fragment description
			*pFragmentBytes = 0;
			RTF_CLR_STATE( pFragment, TRANSPORT_PACKET_BYTES );
			// advance past the data that was just consumed
			pNext += packetFinishBytes;
			remaining -= packetFinishBytes;
		}
		// loop scanning the buffer for packets and map them to packet objects
		for( ;; )
		{
			// still room for another packet?
			if( pBuf->activePacketCount >= pBuf->maxPacketCount )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_OVERFLOW, "Packet array overflow" );
				break;
			}
			// are we in sync with the packet boundaries?
			if( *pInSync == FALSE )
			{
				// no - look for the next sequence of RTF_BUF_SYNC_COUNT occurrances
				// of TRANSPORT_PACKET_SYNCBYTE at TRANSPORT_PACKET_BYTES intervals.
				// Loop until sync achieved or there aren't enough bytes left to even
				// try to synchronize
				for( i=0; i<remaining - (RTF_BUF_SYNC_COUNT*TTS_PACKET_BYTES); ++i)
				{
					for( j=0; j<RTF_BUF_SYNC_COUNT; ++j )
					{
						if( pNext[ i + (j*TRANSPORT_PACKET_BYTES) ] != TRANSPORT_PACKET_SYNCBYTE )
						{
							break;
						}
					}
					if( j >= RTF_BUF_SYNC_COUNT )
					{
						*pInSync = TRUE;
						*pInputIsTTS = FALSE;
						syncByteOffset = TRANSPORT_SYNCBYTE_OFFSET;
						break;
					}
					// skip any leading TTS prefix
					if( i >= TTS_SYNCBYTE_OFFSET )
					{
						for( j=0; j<RTF_BUF_SYNC_COUNT; ++j )
						{
							if( pNext[ i + (j*TTS_PACKET_BYTES) ] != TRANSPORT_PACKET_SYNCBYTE )
							{
								break;
							}
						}
						if( j >= RTF_BUF_SYNC_COUNT )
						{
							*pInSync = TRUE;
							*pInputIsTTS = TRUE;
							syncByteOffset = TTS_SYNCBYTE_OFFSET;
							break;
						}
					}
				}
				// escape if we failed to sync up
				if( *pInSync == FALSE )
				{
					break;
				}
				// go to start of the first packet of the sequence
				pNext += ( i - syncByteOffset );
				remaining -= ( i - syncByteOffset );
			} // if( *pInSync == FALSE )
			// advance past the TTS prefix, if present
			pNext += syncByteOffset;
			remaining -= syncByteOffset;
			// is there a sync byte here?
			if( pNext[ 0 ] != TRANSPORT_PACKET_SYNCBYTE )
			{
				// no - sync lost. Start looking again where we left off
				*pInSync = FALSE;
				continue;
			}
			// got a sync byte - check with the session to see if this is an augmentation packet
			result = rtfSesGetAugmentationPktStatus( pBuf->hSes, pNext, &isAugmentation );
			RTF_CHK_RESULT;
			if( isAugmentation == FALSE )
			{
				// map this packet
				result = rtfPktMap( pBuf->phPacket[ pBuf->activePacketCount++ ],
									handle, pNext, (*pTotalPacketCount)++,
									(*pTotalMappedPacketCount)++, pFirstPcrAcquired );
				RTF_CHK_RESULT;
			}
			else
			{
				// just count it
				++(*pTotalPacketCount);
			}
			// move to the start of the next packet
			pNext += TRANSPORT_PACKET_BYTES;
			remaining -= TRANSPORT_PACKET_BYTES;
			// escape if there are no more bytes left in this buffer
			if( remaining == 0 )
			{
				*pFragmentBytes = 0;
				break;
			}
			// are there more than 0 but less than TRANSPORT_PACKET_BYTES remaining in this buffer?
			if( ( remaining > 0 ) && ( remaining < TRANSPORT_PACKET_BYTES ) )
			{
				// yes - copy the remaining bytes into the calling routine's packet fragment buffer
				// note: don't copy the TTS prefix, if present
				pNext += syncByteOffset;
				remaining -= syncByteOffset;
				memcpy( (void *)pFragment, (void *)pNext, remaining );
				*pFragmentBytes = (unsigned char)remaining;
				break;
			}
		}
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// add some data to an output buffer
RTF_RESULT rtfBufQueueOutputData( RTF_BUF_HANDLE handle, unsigned char *pData, unsigned long byteCount, BOOL *pOverflow )
{
	RTF_FNAME( "rtfBufQueueOutputData" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned long occupancy;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		RTF_CHK_STATE_EQ( pBuf, RTF_BUFSTATE_MAPPED );
		// innocent until proven guilty
		*pOverflow = FALSE;
		// is there room in the current buffer for this data?
		occupancy = pBuf->fillOffset - pBuf->drainOffset;
		if( byteCount > pBuf->capacity - occupancy )
		{
			// no room. signal overflow
			*pOverflow = TRUE;
			break;
		}
		// copy the data to the output buffer
		memcpy( (void *)&(pBuf->pBase[ pBuf->fillOffset ]), (void *)pData, byteCount );
		// adjust the fill offset
		pBuf->fillOffset += byteCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// a picture has added a reference to a packet in this buffer
RTF_RESULT rtfBufAddReference( RTF_BUF_HANDLE handle, RTF_HANDLE hRefHolder )
{
	RTF_FNAME( "rtfBufAddReference" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// adjust the reference count
		++pBuf->referenceCount;
#ifdef DO_TRACKREFCOUNTS
		// make sure there is room for one more reference
		if( pBuf->referenceCount > RTF_BUF_MAX_REFERENCES )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_OVERFLOW, "Reference array overflow on buffer %d", pBuf->bufferNumber );
			break;
		}
		// record the reference holder
		pBuf->hRefHolder[ pBuf->referenceCount-1 ] = hRefHolder;
#endif
		
	} while( 0 ); // error escape wrapper - end

	return result;
}

// a picture has removed a reference to a packet in this buffer
RTF_RESULT rtfBufRemoveReference( RTF_BUF_HANDLE handle, RTF_HANDLE hRefHolder )
{
	RTF_FNAME( "rtfBufRemoveReference" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned long oldestBufferNumber;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// decrement the buffer's reference count
		if( --pBuf->referenceCount < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Negative reference count" );
			break;
		}
#ifdef DO_TRACKREFCOUNTS
		// find this reference holder on the reference list
		for( i=pBuf->referenceCount; i>=0; --i )
		{
			if( pBuf->hRefHolder[ i ] == hRefHolder )
			{
				break;
			}
		}
		// make sure we found one
		if( i < 0 )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_NOTFOUND, "Failed to find reference to be removed from buffer %d", pBuf->bufferNumber );
			break;
		}
		// collapse the reference list over this entry
		for( ; i<pBuf->referenceCount; ++i )
		{
			pBuf->hRefHolder[ i ] = pBuf->hRefHolder[ i+1 ];
		}
		pBuf->hRefHolder[ i ] = 0;
#endif
		// did the reference count just become zero?
		if( pBuf->referenceCount == 0 )
		{
			// yes - get the number of the oldest buffer currently
			// referenced by the session's scanning window
			result = rtfWinGetOldestBufferNumber( pBuf->hWin, &oldestBufferNumber );
			RTF_CHK_RESULT;
			// might this buffer be referenced by the scanning window?
			if( pBuf->bufferNumber < oldestBufferNumber )
			{
				// no - notify the session that this buffer is being unmapped
				result = rtfSesInputBufferUnmapped( pBuf->hSes, handle );
				RTF_CHK_RESULT;
				// unmap all of the active packets in this buffer
				for( i=0; i<pBuf->activePacketCount; ++i )
				{
					result = rtfPktUnmap( pBuf->phPacket[ i ] );
					RTF_CHK_RESULT;
				}
				RTF_CHK_RESULT_LOOP;
				// reset the buffer
				result = rtfBufReset( handle, FALSE );
				RTF_CHK_RESULT;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// scan the packets of the buffer for PSI tables
RTF_RESULT rtfBufCapturePsi( RTF_BUF_HANDLE handle, RTF_PAT_HANDLE hPat, RTF_CAT_HANDLE hCat,
							 RTF_PMT_HANDLE hPmt, BOOL *pPsiCaptured )
{
	RTF_FNAME( "rtfBufCapturePsi" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned short pid, pmtPid, i;
	BOOL patValid = FALSE;
	BOOL catValid = FALSE;
	BOOL pmtValid = FALSE;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// scan the active packets in this buffer
		for( i=0; i<pBuf->activePacketCount; ++i )
		{
			// get this packet's PID
			result = rtfPktGetPID( pBuf->phPacket[ i ], &pid );
			RTF_CHK_RESULT;
			// are we still looking for the PAT?
			result = rtfPatValidate( hPat, &patValid );
			RTF_CHK_RESULT;
			if( patValid == FALSE )
			{
				// yes - is this the PAT PID?
				if( pid == TRANSPORT_PAT_PID )
				{
					// yes - parse the PAT
					result = rtfPatParse( hPat, pBuf->phPacket[ i ] );
					RTF_CHK_RESULT;
					// was the PAT successfully parsed?
					result = rtfPatValidate( hPat, &patValid );
					RTF_CHK_RESULT;
				}
			}
			else
			{
				// we have the PAT - are we still looking for the PMT?
				result = rtfPmtValidate( hPmt, &pmtValid );
				RTF_CHK_RESULT;
				if( pmtValid == FALSE )
				{
					// yes - is this the PMT PID?
					result = rtfPatGetPmtPid( hPat, &pmtPid );
					RTF_CHK_RESULT;
					if( pid == pmtPid )
					{
						// yes - parse the PMT
						result = rtfPmtParse( hPmt, pBuf->phPacket[ i ] );
						RTF_CHK_RESULT;
						// was the PMT successfully parsed?
						result = rtfPmtValidate( hPmt, &pmtValid );
						RTF_CHK_RESULT;
						// the CAT is optional; set PSI Captured flag if we have PAT and PMT
						*pPsiCaptured = pmtValid;
					}
				}
			}
			// are we still looking for a CAT?
			result = rtfCatValidate( hCat, &catValid );
			RTF_CHK_RESULT;
			if( catValid == FALSE )
			{
				// yes - is this the CAT PID?
				if( pid == TRANSPORT_CAT_PID )
				{
					// yes - parse the CAT
					result = rtfCatParse( hCat, pBuf->phPacket[ i ] );
					RTF_CHK_RESULT;
				}
				// was the CAT successfully parsed?
				result = rtfCatValidate( hCat, &catValid );
				RTF_CHK_RESULT;
			}
			// escape if all 3 have been captured
			if( ( patValid != FALSE ) && ( catValid != FALSE ) && ( pmtValid != FALSE ) )
			{
				break;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

RTF_RESULT rtfBufCheckPsi( RTF_BUF_HANDLE handle, RTF_PAT_HANDLE hPat, RTF_CAT_HANDLE hCat,
						   RTF_PMT_HANDLE hPmt, BOOL *pPsiCaptured )
{
	RTF_FNAME( "rtfBufCheckPsi" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPkt;
	int i;
	unsigned short pid, pmtPid;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// get the PMT PID
		result = rtfPatGetPmtPid( hPat, &pmtPid  );
		RTF_CHK_RESULT;
		// scan the active packets in this buffer
		for( i=0; i<(int)pBuf->activePacketCount; ++i )
		{
			// get the current packet
			hPkt = pBuf->phPacket[ i ];
			// get this packet's info
			result = rtfPktGetPID( hPkt, &pid );
			RTF_CHK_RESULT;
			// is this the PAT PID?
			if( pid == TRANSPORT_PAT_PID )
			{
				// check to see if the PAT has changed
				result = rtfPatCheckChange( hPat, hPkt );
				RTF_CHK_RESULT;
			}
			// is this the CAT PID?
			else if( pid == TRANSPORT_CAT_PID )
			{
				// check to see if the CAT has changed
				result = rtfCatCheckChange( hCat, hPkt );
				RTF_CHK_RESULT;
			}
			// is this the PMT PID?
			else if( pid == pmtPid )
			{
				// check to see if the PMT has changed
				result = rtfPmtCheckChange( hPmt, hPkt );
				RTF_CHK_RESULT;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// scan the packets of the buffer for input bit rate info
RTF_RESULT rtfBufCaptureBpsInfo( RTF_BUF_HANDLE handle, unsigned char maxPcrCount,
							  unsigned char *pPcrCount, unsigned long packetNumber[],
							  RTF_TIMESTAMP timestamp[] )
{
	RTF_FNAME( "rtfBufCaptureBpsInfo" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPkt;
	unsigned long flags;
	unsigned short i;
	unsigned char count;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// scan the active packets in this buffer
		for( i=0; i<pBuf->activePacketCount; ++i )
		{
			// get the flags from this packet
			hPkt = pBuf->phPacket[ i ];
			result = rtfPktGetFlags( hPkt, &flags );
			RTF_CHK_RESULT;
			// does this packet have a discontinuity?
			if( ( flags & RTF_PKT_DISCONTINUITY ) != 0 )
			{
				// reset the recorded PCR list (need continuous PCRs )
				*pPcrCount = 0;
			}
			// does this packet have a PCR?
			if( ( flags & RTF_PKT_PCRPRESENT ) != 0 )
			{
				// yes - record it
				count = *pPcrCount;
				result = rtfPktGetMapPktNumber( hPkt, &packetNumber[ count ] );
				RTF_CHK_RESULT;
				result = rtfPktGetPcrTimestamp( hPkt, &timestamp[ count ] );
				RTF_CHK_RESULT;
				if( ++(*pPcrCount) >= maxPcrCount )
				{
					break;
				}
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// scan the packets of the buffer for video frame rate info
RTF_RESULT rtfBufCaptureFpsInfo( RTF_BUF_HANDLE handle)
{
	RTF_FNAME( "rtfBufCaptureFpsInfo" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPkt;
	RTF_STREAM_PROFILE *pProfile;
	RTF_TIMESTAMP pts;
	unsigned char *pStorage;
	unsigned long number, flags;
	unsigned short i, pid, vidPid;
	unsigned char payOff, payBytes;
	unsigned char pesOff, pesBytes;
	BOOL scanComplete = FALSE;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		// get the video PID from the session object
		result = rtfSesGetStreamProfile( pBuf->hSes, &pProfile );
		RTF_CHK_RESULT;
		vidPid = pProfile->videoSpec.pid;
		// scan the active packets in this buffer
		for( i=0; i<pBuf->activePacketCount; ++i )
		{
			// get the packet info
			hPkt = pBuf->phPacket[ i ];
			result = rtfPktGetInfo( hPkt, &number, &pid, &flags, &pStorage, &payOff, &payBytes, &pesOff, &pesBytes );
			RTF_CHK_RESULT;
			// skip this packet if it is not video
			if( pid != vidPid )
			{
				continue;
			}
			// does this packet have a discontinuity?
			if( ( flags & RTF_PKT_DISCONTINUITY ) != 0 )
			{
				// reset the recorded picture list (need continuous pictures )
				rtfSesPesPTS(pBuf->hSes, -1, &scanComplete);
			}
			// does this packet have a PES header?
			if( ( flags & RTF_PKT_PESHDRPRESENT ) != 0 )
			{
				// yes - is the entire PES header in this packet?
				if( ( pesOff + pesBytes ) >= TRANSPORT_PACKET_BYTES )
				{
					// no - the PES header should be the first thing in the packet payload
					RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "PES header split at packet %d", number );
					break;
				}
				// does the PES header contain a PTS?
				if( ( pStorage[ pesOff + 7 ] & 0x80 ) != 0 )
				{
					// yes - parse the value of the PTS
					pts.base.ull = ( pStorage[ pesOff + 9 ] >> 1 ) & 0x07;
					pts.base.ull = ( pts.base.ull << 8 ) | ( ( pStorage[ pesOff + 10 ] ) & 0xFF );
					pts.base.ull = ( pts.base.ull << 7 ) | ( ( pStorage[ pesOff + 11 ] >> 1 ) & 0x7f );
					pts.base.ull = ( pts.base.ull << 8 ) | ( ( pStorage[ pesOff + 12 ] ) & 0xFF );
					pts.base.ull = ( pts.base.ull << 7 ) | ( ( pStorage[ pesOff + 13 ] >> 1 ) & 0x7f );
					// a PTS indicates a picture - has the picture counting started yet?
					// a PTS indicates a picture - pass pts to session
					rtfSesPesPTS(pBuf->hSes, pts.base.ull, &scanComplete);
				} // if( ( pStorage[ hdrOff + 7 ] & 0x80 ) != 0 )
			} // if( ( flags & RTF_PKT_PESHDRPRESENT ) != 0 )
			// escape if the target picture count has been reached
			if(scanComplete)
			{
				break;
			}
		} // for( i=0; i<pBuf->activePacketCount; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef _DEBUG
static char *rtfBufStateString[] =
{ "invalid", "released", "mapped", "processed", "unrecognized" };

// debug helper function - print state of buffer object
RTF_RESULT rtfBufferLogState( RTF_BUF_HANDLE handle )
{
	RTF_FNAME( "rtfBufferLogState" );
	RTF_OBASE( handle );
	RTF_BUF *pBuf = (RTF_BUF *)handle;
	RTF_RESULT result = RTF_PASS;
	int state;

	do {		 // error escape wrapper - begin

		if( pBuf == (RTF_BUF *)NULL )
		{
			RTF_LOG_INFO0( RTF_MSG_INF_LOGNULLHANDLE, "Null buffer handle" );
			break;
		}
		RTF_CHK_OBJ( pBuf, RTF_OBJ_TYPE_BUF );
		state = ( pBuf->state > RTF_BUFSTATE_PROCESSED ) ? RTF_BUFSTATE_PROCESSED+1 : pBuf->state;
		RTF_LOG_INFO5( RTF_MSG_INF_STATS, "Buffer %d: state=%d (%s) activePacketCount=%d referenceCount=%d",
					   (int)pBuf->bufferNumber, state, rtfBufStateString[ state ],
					   (int)pBuf->activePacketCount, (int)pBuf->referenceCount );

	} while ( 0 );	// error escape wrapper - end

	return result;
}
#endif
