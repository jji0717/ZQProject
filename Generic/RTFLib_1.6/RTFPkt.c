// implementation file for rtfPkt class
// abstracts transport stream packet
//

#include "RTFPrv.h"

// constants ****************************************************************************

#define RTF_PKT_MAX_REFERENCES	16

// typedefs *****************************************************************************

typedef struct _RTF_PKT
{
	RTF_OBJ_HANDLE hBaseObject;

	// packet info
	RTF_BUF_HANDLE hBuf;			// handle of the buffer that the packet is in
	unsigned char *pStorage;		// ptr to first packet byte (i.e. sync char)
	unsigned long  inpPktNumber;	// input transport packet number
	unsigned long  mapPktNumber;	// mapped input packet number (does not include augmentation packets)
	unsigned long  outPktNumber;	// output packet number (for main file copy only)
	unsigned long  flags;			// various bit flags - see masks in rtfPkt.h
	unsigned short pid;				// Packet ID

	unsigned char payloadBytes;		// number of bytes in payload
	unsigned char payloadOffset;	// offset of 1st payload byte from start of packet
	unsigned char seqStartOffset;	// offset of start of sequence within packet (if SEQSTARTPRESENT flag is set)
	unsigned char gopStartOffset;	// offset of group hdr start (if GOPHDRPRESENT flag is set)
	unsigned char picStartOffset;	// offset of start of picture within packet (if PICSTARTPRESENT flag is set)

	unsigned char codHdrOffset;		// offset of picture coding extension hdr start (if CODHDRPRESENT flag is set)
	unsigned char seqHdrOffset;		// offset of sequence hdr start (if SEQHDRPRESENT flag is set)
	unsigned char sqxHdrOffset;		// offset of sequence extension hdr start (if SQXHDRPRESENT flag is set)
	unsigned char pesHdrOffset;		// offset of PES packet hdr start (if PESHDRPRESENT flag is set)
	unsigned char pesHdrBytes;		// length of PES packet hdr

	int referenceCount;				// number of active pictures pointing to this packet
#ifdef DO_TRACKREFCOUNTS
	RTF_HANDLE hRefHolder[ RTF_PKT_MAX_REFERENCES ]; // handles of objects holding a reference to this packet
#endif

} RTF_PKT;

// private functions ********************************************************************

// reset the packet descriptor
static void resetPacket( RTF_PKT *pPkt )
{
	// null out the mapping pointer
	pPkt->pStorage = (unsigned char *)NULL;
#ifdef _DEBUG // none of this is strictly necessary
	// reset the packet numbers
	pPkt->inpPktNumber   = 0;
	pPkt->mapPktNumber   = 0;
	pPkt->outPktNumber   = 0;
	// reset the PID
	pPkt->pid = TRANSPORT_INVALID_PID;
	// reset the flags
	pPkt->flags = 0;
	// reset the reference count
	pPkt->referenceCount = 0;
	// reset the offsets and byte counts
	pPkt->payloadOffset  = 0;
	pPkt->payloadBytes   = 0;
	pPkt->seqStartOffset = 0;
	pPkt->gopStartOffset = 0;
	pPkt->picStartOffset = 0;
	pPkt->pesHdrOffset   = 0;
	pPkt->codHdrOffset   = 0;
	pPkt->pesHdrBytes	 = 0;
	pPkt->seqStartOffset = 0;
#endif
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPktGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfPktGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_PKT);
	bytes += rtfObjGetStorageRequirement();

	// RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfPktConstructor( RTF_PKT_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfPktConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_PKT *pPkt;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the packet object
		pPkt = (RTF_PKT *)rtfAlloc( sizeof(RTF_PKT) );
		RTF_CHK_ALLOC( pPkt );
		// return the handle
		*pHandle = (RTF_PKT_HANDLE)pPkt;
		// clear the state structure
		memset( (void *)pPkt, 0, sizeof(*pPkt) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_PKT, (RTF_HANDLE)pPkt, hParent, &pPkt->hBaseObject );
		RTF_CHK_RESULT;
		// reset the packet descriptor
		resetPacket( pPkt );

	} while( 0 ); // error escape wrapper - end

	// RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfPktDestructor( RTF_PKT_HANDLE handle )
{
	RTF_FNAME( "rtfPktDestructor" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded resource object (note: performs CHK_OBJ)
		result = rtfObjDestructor( pPkt->hBaseObject, RTF_OBJ_TYPE_PKT );
		RTF_CHK_RESULT;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// return the PID
RTF_RESULT rtfPktGetPID( RTF_PKT_HANDLE handle, unsigned short *pPid )
{
	RTF_FNAME( "rtfPktGetPID" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// make the return
		*pPid = pPkt->pid;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return a pointer to the packet data
RTF_RESULT rtfPktGetStorage( RTF_PKT_HANDLE handle, unsigned char **ppStorage )
{
	RTF_FNAME( "rtfPktGetStorage" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// make the return
		*ppStorage = pPkt->pStorage;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the handle of the buffer that contains this packet
RTF_RESULT rtfPktGetBuffer( RTF_PKT_HANDLE handle, RTF_BUF_HANDLE *phBuf )
{
	RTF_FNAME( "rtfPktGetBuffer" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// make the return
		*phBuf = pPkt->hBuf;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the input packet number
RTF_RESULT rtfPktGetInpPktNumber( RTF_PKT_HANDLE handle, unsigned long *pNumber )
{
	RTF_FNAME( "rtfPktGetInpPktNumber" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// make the return
		*pNumber = pPkt->inpPktNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the mapped input packet number
RTF_RESULT rtfPktGetMapPktNumber( RTF_PKT_HANDLE handle, unsigned long *pMapPktNumber )
{
	RTF_FNAME( "rtfPktGetMapPktNumber" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// make the return
		*pMapPktNumber = pPkt->mapPktNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the output packet number (main file copy only)
RTF_RESULT rtfPktSetOutPktNumber( RTF_PKT_HANDLE handle, unsigned long outPktNumber )
{
	RTF_FNAME( "rtfPktSetOutPktNumber" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// record the output packet number
		pPkt->outPktNumber = outPktNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the output packet number (main file copy only)
RTF_RESULT rtfPktGetOutPktNumber( RTF_PKT_HANDLE handle, unsigned long *pOutPktNumber )
{
	RTF_FNAME( "rtfPktGetOutPktNumber" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// make the return
		*pOutPktNumber = pPkt->outPktNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return the continuity counter of the packet
RTF_RESULT rtfPktGetCC( RTF_PKT_HANDLE handle, unsigned char *pCC )
{
	RTF_FNAME( "rtfPktGetCC" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make the return
		*pCC = pPkt->pStorage[ 3 ] & 0x0F;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// return some information about the packet
RTF_RESULT rtfPktGetInfo( RTF_PKT_HANDLE handle, unsigned long *pPacketNumber,
						  unsigned short *pPid, unsigned long *pFlags, unsigned char **ppStorage,
						  unsigned char *pPayloadOffset, unsigned char *pPayloadBytes,
						  unsigned char *pPesHdrOffset,  unsigned char *pPesHdrBytes )
{
	RTF_FNAME( "rtfPktGetInfo" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// return the packet number
		*pPacketNumber = pPkt->inpPktNumber;
		// return the pid
		*pPid = pPkt->pid;
		// return the flags
		*pFlags = pPkt->flags;
		// return the storage pointer
		*ppStorage = pPkt->pStorage;
		// return the offset and length of the payload
		*pPayloadOffset = pPkt->payloadOffset;
		*pPayloadBytes  = pPkt->payloadBytes;
		*pPesHdrOffset  = pPkt->pesHdrOffset;
		*pPesHdrBytes   = pPkt->pesHdrBytes;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the flags from this packet
RTF_RESULT rtfPktGetFlags( RTF_PKT_HANDLE handle, unsigned long *pFlags )
{
	RTF_FNAME( "rtfPktGetFlags" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// return the flags
		*pFlags = pPkt->flags;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the start-of-sequence offset
RTF_RESULT rtfPktGetSeqStartOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset, BOOL *pIsVirtual )
{
	RTF_FNAME( "rtfPktGetSeqStartOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make sure the sequence start present flag is set
		if( ( pPkt->flags & RTF_PKT_SEQSTARTPRESENT ) == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "SEQ start not present" );
			break;
		}
		// make the return
		*pOffset = pPkt->seqStartOffset;
		*pIsVirtual = ( ( pPkt->flags & RTF_PKT_SEQHDRPRESENT ) == 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// record the start-of-sequence offset
RTF_RESULT rtfPktSetSeqStartOffset( RTF_PKT_HANDLE handle, unsigned char offset, BOOL isVirtual )
{
	RTF_FNAME( "rtfPktSetSeqStartOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// is the sequence start flag already set?
		if( ( pPkt->flags & RTF_PKT_SEQSTARTPRESENT ) != 0 )
		{
			// yes - the packet is damaged
			pPkt->flags |= RTF_PKT_ISDAMAGED;
		}
		// set the sequence start flag
		pPkt->flags |= RTF_PKT_SEQSTARTPRESENT;
		if( isVirtual == FALSE )
		{
			pPkt->flags |= RTF_PKT_SEQHDRPRESENT;
		}
		// record the offset
		pPkt->seqStartOffset = offset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the start-of-group offset
RTF_RESULT rtfPktSetGopStartOffset( RTF_PKT_HANDLE handle, unsigned char offset, BOOL isVirtual )
{
	RTF_FNAME( "rtfPktSetGopStartOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// is the group header present flag already set?
		if( ( pPkt->flags & RTF_PKT_GOPSTARTPRESENT ) != 0 )
		{
			// yes - the packet is damaged
			pPkt->flags |= RTF_PKT_ISDAMAGED;
		}
		// set the group header present flag
		pPkt->flags |= RTF_PKT_GOPSTARTPRESENT;
		if( isVirtual == FALSE )
		{
			pPkt->flags |= RTF_PKT_GOPHDRPRESENT;
		}

		// record the offset
		pPkt->gopStartOffset = offset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the start-of-group offset
RTF_RESULT rtfPktGetGopStartOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset, BOOL *pIsVirtual )
{
	RTF_FNAME( "rtfPktGetGopStartOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make sure the GOP start present flag is set
		if( ( pPkt->flags & RTF_PKT_GOPSTARTPRESENT ) == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "GOP start not present" );
			break;
		}
		// make the return
		*pOffset = pPkt->gopStartOffset;
		*pIsVirtual = ( ( pPkt->flags & RTF_PKT_GOPHDRPRESENT ) == 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the start-of-picture offset
RTF_RESULT rtfPktSetPicStartOffset( RTF_PKT_HANDLE handle, unsigned char offset )
{
	RTF_FNAME( "rtfPktSetPicStartOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// is the picture start flag already set?
		if( ( pPkt->flags & RTF_PKT_PICSTARTPRESENT ) != 0 )
		{
			// yes - the packet is damaged
			pPkt->flags |= RTF_PKT_ISDAMAGED;
		}
		// set the picture start present flag
		pPkt->flags |= RTF_PKT_PICSTARTPRESENT;
		// record the offset
		pPkt->picStartOffset = offset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the start-of-picture offset
RTF_RESULT rtfPktGetPicStartOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset )
{
	RTF_FNAME( "rtfPktGetPicStartOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make sure the picture start present flag is set
		if( ( pPkt->flags & RTF_PKT_PICSTARTPRESENT ) == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "PIC start not present" );
			break;
		}
		// make the return
		*pOffset = pPkt->picStartOffset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the picture coding extension header offset
RTF_RESULT rtfPktGetCodHdrOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset )
{
	RTF_FNAME( "rtfPktGetCodHdrOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make sure the picture coding extension header present flag is set
		if( ( pPkt->flags & RTF_PKT_CODHDRPRESENT ) == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Picture coding extension header not present" );
			break;
		}
		// make the returns
		*pOffset = pPkt->codHdrOffset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the picture coding extension header info
RTF_RESULT rtfPktSetCodHdrOffset( RTF_PKT_HANDLE handle, unsigned char offset )
{
	RTF_FNAME( "rtfPktSetCodHdrOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// is the picture coding extension header present flag already set?
		if( ( pPkt->flags & RTF_PKT_CODHDRPRESENT ) != 0 )
		{
			// yes - packet is damaged
			pPkt->flags |= RTF_PKT_ISDAMAGED;
		}
		// set the picture coding extension header present flag
		pPkt->flags |= RTF_PKT_CODHDRPRESENT;
		// record the info
		pPkt->codHdrOffset = offset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the sequence extension header info
RTF_RESULT rtfPktGetSqxHdrOffset( RTF_PKT_HANDLE handle, unsigned char *pOffset )
{
	RTF_FNAME( "rtfPktGetSqxHdrOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make sure the sequence extension header present flag is set
		if( ( pPkt->flags & RTF_PKT_SQXHDRPRESENT ) == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "SQX header not present" );
			break;
		}
		// make the return
		*pOffset = pPkt->sqxHdrOffset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the sequence extension header offset
RTF_RESULT rtfPktSetSqxHdrOffset( RTF_PKT_HANDLE handle, unsigned char offset )
{
	RTF_FNAME( "rtfPktSetSqxHdrOffset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// is the sequence extension header present flag already set?
		if( ( pPkt->flags & RTF_PKT_SQXHDRPRESENT ) != 0 )
		{
			// yes - packet is damaged
			pPkt->flags |= RTF_PKT_ISDAMAGED;
		}
		// set the sequence extension header present flag
		pPkt->flags |= RTF_PKT_SQXHDRPRESENT;
		// record the info
		pPkt->sqxHdrOffset = offset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the PES header offset from the packet (returns offset of 0 if not present)
RTF_RESULT rtfPktGetPesHdrInfo( RTF_PKT_HANDLE handle, unsigned char *pPayOffset,
							    unsigned char *pHdrOffset, unsigned char *pHdrBytes )
{
	RTF_FNAME( "rtfPktGetPesHdrInfo" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make the returns
		*pPayOffset = pPkt->payloadOffset;
		*pHdrOffset = pPkt->pesHdrOffset;
		*pHdrBytes  = pPkt->pesHdrBytes;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the value of the PCR in this packet
RTF_RESULT rtfPktGetPcrTimestamp( RTF_PKT_HANDLE handle, RTF_TIMESTAMP *pTimestamp )
{
	RTF_FNAME( "rtfPktGetPcrTimestamp" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make sure that this packet has a PCR
		if( ( pPkt->flags & RTF_PKT_PCRPRESENT ) == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "PCR not present" );
			break;
		}
		// extract the value of the PCR
		pTimestamp->base.ul[1] = 0;
		pTimestamp->base.uc[4] = pPkt->pStorage[6];
		pTimestamp->base.uc[3] = pPkt->pStorage[7];
		pTimestamp->base.uc[2] = pPkt->pStorage[8];
		pTimestamp->base.uc[1] = pPkt->pStorage[9];
		pTimestamp->base.uc[0] = pPkt->pStorage[10];
		pTimestamp->base.ull >>= 7;
		pTimestamp->ext.uc[1]  = pPkt->pStorage[10] & 0x01;
		pTimestamp->ext.uc[0]  = pPkt->pStorage[11];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the state of the progressive sequence flag bit
RTF_RESULT rtfPktSetProgressiveSeq( RTF_PKT_HANDLE handle, BOOL progressiveSeq )
{
	RTF_FNAME( "rtfPktReset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// set the flag
		pPkt->flags = ( progressiveSeq == FALSE ) ?
						pPkt->flags & ~RTF_PKT_PROGRESSIVESEQ :
						pPkt->flags | RTF_PKT_PROGRESSIVESEQ;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset a packet descriptor
RTF_RESULT rtfPktReset( RTF_PKT_HANDLE handle, BOOL isClose )
{
	RTF_FNAME( "rtfPktReset" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		if( isClose == FALSE )
		{
			RTF_CHK_REFCOUNT( pPkt );
		}
		resetPacket( pPkt );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// map the transport packet to the indicated contiguous storage
// assign the packet number and the mapped packet number
RTF_RESULT rtfPktMap( RTF_PKT_HANDLE handle, RTF_BUF_HANDLE hBuf,
					  unsigned char *pStorage, unsigned long inpPktNumber,
					  unsigned long mapPktNumber, BOOL *pFirstPcrAcquired )
{
	RTF_FNAME( "rtfPktMap" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;
	int bytes;
	unsigned char hdrBytes;
	unsigned char ac;
	unsigned char offset;
	unsigned char *pucPayload;
	unsigned long *pulPayload;
	unsigned long accumulator;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		// make sure the packet is not already mapped
		if( pPkt->pStorage != NULL )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Packet already mapped at packet %d",
						  inpPktNumber );
			break;
		}
		// check for sync character
		if( pStorage[0] != TRANSPORT_PACKET_SYNCBYTE )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_BADSTREAM, "Bad sync character 0x%x at packet %d",
						  pStorage[0], inpPktNumber );
			break;
		}
		// record the buffer that the packet is in
		pPkt->hBuf = hBuf;
		// record the mapping pointer
		pPkt->pStorage = pStorage;
		// record the input packet number
		pPkt->inpPktNumber = inpPktNumber;
		// record the mapped input packet number
		pPkt->mapPktNumber = mapPktNumber;
		// the default output packet number is the input packet number
		pPkt->outPktNumber = inpPktNumber;
		// reset the reference count
		pPkt->referenceCount = 0;
		// record the pid
		pPkt->pid = (((unsigned short)(pStorage[1])<<8) | pStorage[2] ) & TRANSPORT_PACKET_PID_MASK;
		// record the payload offset and length
		hdrBytes = ( ( pStorage[ 3 ] & 0x20 ) != 0 ) ? 5 + pStorage[ 4 ] : 4;
		// is the payload encrypted?
		pPkt->flags = ( ( pStorage[3] >> 6 ) != 0 ) ? RTF_PKT_PAYLOADENCRYPTED : 0;
		// extract the payload unit start indicator (also implies PES header present)
		pPkt->flags |= ( ( pStorage[1] & 0x40 ) != 0 ) ?
				 ( RTF_PKT_PAYLOADUNITSTART | RTF_PKT_PESHDRPRESENT | RTF_PKT_NONZEROPAYLOAD ) : 0;
		// decode the adaptation control field
		switch( ( ac = ( pStorage[3] & 0x30 ) ) )
		{
		case 0x10:
			// payload is present, adaptation is absent
			break;
		case 0x20:
			pPkt->flags |= RTF_PKT_PAYLOADABSENT;
			pPkt->flags |= RTF_PKT_ADAPTATIONPRESENT;
			break;
		case 0x30:
			pPkt->flags |= RTF_PKT_ADAPTATIONPRESENT;
			break;
		default:
			// invalid adaptation field
			RTF_LOG_WARN2( RTF_MSG_WRN_BADADAPT, "Invalid adaptation control field %d in packet %d",
						   ac, pPkt->inpPktNumber );
			// the packet is damaged. try to ignore it
			pPkt->flags |= RTF_PKT_PAYLOADABSENT | RTF_PKT_ISDAMAGED;
		}
		RTF_CHK_RESULT_LOOP;
		// is there an adaptation field?
		if( ( pPkt->flags & RTF_PKT_ADAPTATIONPRESENT ) != 0 )
		{
			// yes - is the adaptation field length greater than 0?
			if( pStorage[ 4 ] != 0 )
			{
				// yes - is there a discontinuity present?
				pPkt->flags |= ( ( pStorage[ 5 ] & 0x80 ) != 0 ) ? RTF_PKT_DISCONTINUITY : 0;
				// is there a discontinuity AND has the first PCR already been acquired?
				if( ( ( pPkt->flags & RTF_PKT_DISCONTINUITY ) != 0 ) &&
					( *pFirstPcrAcquired != FALSE ) )
				{
					RTF_LOG_WARN1(RTF_MSG_WRN_POSTFIRSTPCRDCON,
								  "Discontinuity after first PCR, packet %d",
								  pPkt->inpPktNumber );
				}
				// is there a PCR present?
				if( ( pStorage[5] & 0x10 ) != 0 )
				{
					// yes - record this
					pPkt->flags |= RTF_PKT_PCRPRESENT;
					// is this the first PCR?
					if( *pFirstPcrAcquired == FALSE )
					{
						RTF_SES_HANDLE hSes;
						// yes - get the handle of the session object
						result = rtfObjGetSession( (RTF_HANDLE)pPkt, &hSes );
						RTF_CHK_RESULT;
						// is the discontinuity flag set ?
						if( ( pPkt->flags & RTF_PKT_DISCONTINUITY ) == 0 )
						{
							// no - this is an error if splicing is enabled
							BOOL splicingEnabled;
							result = rtfSesGetSplicingEnabled( hSes, &splicingEnabled );
							RTF_CHK_RESULT;
							if( splicingEnabled != FALSE )
							{
								RTF_LOG_WARN1(RTF_MSG_WRN_NOFIRSTPCRDCON,
											"No first PCR discontinuity at packet %d",
											pPkt->inpPktNumber );
							}
						} // if( *pFirstPcrAcquired == FALSE )
						// set the "first PCR acquired" flag
						*pFirstPcrAcquired = TRUE;
					}
				} // if( ( pStorage[5] & 0x10 ) != 0 )
			} // if( pStorage[ 4 ] != 0 )
		} // if( ( flags & RTF_PKT_ADAPTATIONPRESENT ) != 0 )
		// compute the offset and length of the payload
		offset = 0;
		bytes  = 0;
		// does this packet have a payload?
		if( ( pPkt->flags & RTF_PKT_PAYLOADABSENT ) == 0 )
		{
			// yes - skip the transport packet header
			offset += TRANSPORT_PACKET_HEADER_BYTES;
			// does the packet have an adaptation field?
			if( ( pPkt->flags & RTF_PKT_ADAPTATIONPRESENT ) != 0 )
			{
				// yes - skip the adaptation field
				offset += 1 + pStorage[ TRANSPORT_PACKET_HEADER_BYTES ];
			}
			// the number of payload bytes is the packet length less the payload offset
			bytes = TRANSPORT_PACKET_BYTES - offset;
			// make sure these values look reasonable
			if( offset > TRANSPORT_PACKET_BYTES )
			{
				RTF_LOG_WARN1( RTF_MSG_WRN_NOPAYLOAD, "Unable to find payload in packet %d", pPkt->inpPktNumber );
				pPkt->flags |= RTF_PKT_ISDAMAGED;
			}
			// some encoders incorrectly mark packets as having both payload and adaptation
			// but specify an adaptation length that is the full packet. Cope with this.
			if( offset == TRANSPORT_PACKET_BYTES )
			{
				pPkt->flags |= RTF_PKT_PAYLOADABSENT;
			}
			// record the payload offset and length
			pPkt->payloadOffset = offset;
			pPkt->payloadBytes = bytes;
			// does the packet have a PES header?
			// note: if it has a PES header, it already has the non-zero payload flag set
			if( ( pPkt->flags & RTF_PKT_PESHDRPRESENT ) != 0 )
			{
				// yes - record the PES header offset and length
				pPkt->pesHdrOffset = offset;
				pPkt->pesHdrBytes  = 9 + pStorage[ offset + 8 ];
			}
			else
			{
				// no - perform a quick scan of the payload
				accumulator = 0;
				pucPayload = pPkt->pStorage + offset;
				if( bytes > TRANSPORT_MAX_PAYLOAD_BYTES )
				{
					RTF_LOG_WARN1( RTF_MSG_WRN_BADADAPT, "Bad adaptation field length in packet %d", pPkt->inpPktNumber );
					pPkt->flags |= RTF_PKT_ISDAMAGED;
				}
				else
				{
					while( ( bytes-- > 0 ) && ( ( (unsigned long)pucPayload & 0x03 ) != 0 ) )
					{
						accumulator |= *pucPayload++;
					}
					pulPayload = (unsigned long *)pucPayload;
					while( bytes >= 4 )
					{
						bytes -= 4;
						if( ( accumulator |= *pulPayload++ ) != 0 )
						{
							break;
						}
					}
					pucPayload = (unsigned char *)pulPayload;
					while( bytes-- > 0 )
					{
						accumulator |= *pucPayload++;
					}
					// was it all zeros?
					if( accumulator != 0 )
					{
						// no - set the non-zero payload flag
						pPkt->flags |= RTF_PKT_NONZEROPAYLOAD;
					}
				} // if( bytes > TRANSPORT_MAX_PAYLOAD_BYTES ) ; else
			} // if( ( pPkt->flags & RTF_PKT_PESHDRPRESENT ) != 0 ) ; else
		} // if( ( flags & RTF_PKT_PAYLOADABSENT ) == 0 )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// add a reference to the packet
RTF_RESULT rtfPktAddReference( RTF_PKT_HANDLE handle, RTF_HANDLE hRefHolder )
{
	RTF_FNAME( "rtfPktAddReference" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// increment the reference count
		++pPkt->referenceCount;
#ifdef DO_TRACKREFCOUNTS
		// make sure there is room for one more reference
		if( pPkt->referenceCount > RTF_PKT_MAX_REFERENCES )
		{
			RTF_LOG_ERR1( RTF_MSG_ERR_OVERFLOW, "Reference array overflow on packet %d", pPkt->inpPktNumber );
			break;
		}
		// record the reference holder
		pPkt->hRefHolder[ pPkt->referenceCount-1 ] = hRefHolder;
#endif
		// increment the reference count of the buffer that contains this packet
		result = rtfBufAddReference( pPkt->hBuf, handle );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// remove a reference to the packet
RTF_RESULT rtfPktRemoveReference( RTF_PKT_HANDLE handle, RTF_HANDLE hRefHolder )
{
	RTF_FNAME( "rtfPktRemoveReference" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// decrement the reference count - better not be negative!
		if( --pPkt->referenceCount < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Negative reference count" );
			break;
		}
#ifdef DO_TRACKREFCOUNTS
		{
			int i;
			// find this reference holder on the reference list
			for( i=pPkt->referenceCount; i>=0; --i )
			{
				if( pPkt->hRefHolder[ i ] == hRefHolder )
				{
					break;
				}
			}
			// make sure we found one
			if( i < 0 )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_NOTFOUND, "Failed to find reference to be removed from packet %d", pPkt->inpPktNumber );
				break;
			}
			// collapse the reference list over this entry
			for( ; i<pPkt->referenceCount; ++i )
			{
				pPkt->hRefHolder[ i ] = pPkt->hRefHolder[ i+1 ];
			}
			pPkt->hRefHolder[ i ] = 0;
		}
#endif
		// decrement the reference count of the buffer that contains this packet
		result = rtfBufRemoveReference( pPkt->hBuf, handle );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the reference count of a packet
RTF_RESULT rtfPktGetRefCount( RTF_PKT_HANDLE handle, unsigned long *pRefCount )
{
	RTF_FNAME( "rtfPktGetRefCount" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// make the return
		*pRefCount = pPkt->referenceCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef DO_TRACKREFCOUNTS
// get the handle to the holder of a particular reference
RTF_RESULT rtfPktGetRefHandle( RTF_PKT_HANDLE handle, unsigned long index, RTF_HANDLE *phRefHandle )
{
	RTF_FNAME( "rtfPktGetRefHandle" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// is the index in range?
		if( index >= (unsigned long)pPkt->referenceCount )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "Index %d out of range (max %d)\n",
						  index, pPkt->referenceCount );
			break;
		}
		// make the return
		*phRefHandle = pPkt->hRefHolder[ index ];

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif

// copy the payload of the packet
// return the offset of the payload in the packet, and the byte count
RTF_RESULT rtfPktCopyPayload( RTF_PKT_HANDLE handle, unsigned char *pDst )
{
	RTF_FNAME( "rtfPktCopyPayload" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		// copy the packet data to the output buffer
		memcpy( (void *)pDst, (void *)&(pPkt->pStorage[ pPkt->payloadOffset ]), pPkt->payloadBytes );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// reset the transport packet to an idle state
RTF_RESULT rtfPktUnmap( RTF_PKT_HANDLE handle )
{
	RTF_FNAME( "rtfPktUnmap" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_REFCOUNT( pPkt );
		RTF_CHK_PKTMAPPED( pPkt );
		// reset the packet descriptor
		resetPacket( pPkt );

	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef _DEBUG
// dump out the contents of a packet
RTF_RESULT rtfPktDump( RTF_PKT_HANDLE handle )
{
	RTF_FNAME( "rtfPktDump" );
	RTF_OBASE( handle );
	RTF_PKT *pPkt = (RTF_PKT *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned char *pDat;
	int i, j;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPkt, RTF_OBJ_TYPE_PKT );
		RTF_CHK_PKTMAPPED( pPkt );
		PRINTF( "\nContents of packet #%d:\n", pPkt->inpPktNumber );
		pDat = pPkt->pStorage; 
		for( i=0; i<TRANSPORT_PACKET_BYTES; )
		{
			for( j=0; j<16; ++j )
			{
				PRINTF( "  %02x", *pDat++ );
				if( ++i >= TRANSPORT_PACKET_BYTES )
				{
					break;
				}
			}
			PRINTF( "\n" );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif
