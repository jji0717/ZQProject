// implementation file for rtfCat class
// encapsulates conditional access table
//

#include "RTFPrv.h"

#define RTF_MAX_CAT_DESCRDATBYTES	( TRANSPORT_MAX_PAYLOAD_BYTES - 14 )
#define RTF_MAX_CAT_DESCRS			16

// typedefs *****************************************************************************

typedef struct _RTF_CAT_DESCR
{
	unsigned char  tag;
	unsigned char  len;
	unsigned short sid;
	unsigned short pid;
	unsigned char  datLen;
	unsigned char  dat[ RTF_MAX_CAT_DESCRDATBYTES ];

} RTF_CAT_DESCR;

typedef struct _RTF_CAT
{
	RTF_OBJ_HANDLE hBaseObject;

	BOOL valid;
	unsigned char versionNumber;
	unsigned char currentNext;
	unsigned char activeIndex;
	unsigned char descriptorCount;
	RTF_CAT_DESCR descriptor[ RTF_MAX_CAT_DESCRS ];
	unsigned char pktPayloadOffset;
	unsigned char pktPayloadBytes;
	unsigned char pkt[ TRANSPORT_PACKET_BYTES ];

} RTF_CAT;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfCatGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfCatGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_CAT);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfCatConstructor( RTF_CAT_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfCatConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_CAT *pCat;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the CAT object
		pCat = (RTF_CAT *)rtfAlloc( sizeof(RTF_CAT) );
		RTF_CHK_ALLOC( pCat );
		// return the handle
		*pHandle = (RTF_CAT_HANDLE)pCat;
		// clear the state structure
		memset( (void *)pCat, 0, sizeof(*pCat) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_CAT, (RTF_HANDLE)pCat, hParent, &pCat->hBaseObject );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfCatDestructor( RTF_CAT_HANDLE handle )
{
	RTF_FNAME( "rtfCatDestructor" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded resource object (note: performs CHK_OBJ)
		result = rtfObjDestructor( pCat->hBaseObject, RTF_OBJ_TYPE_CAT );
		RTF_CHK_RESULT;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get some info on the conditional access table
RTF_RESULT rtfCatGetInfo( RTF_CAT_HANDLE handle, unsigned char *pVersionNumber,
						  unsigned char *pCurrentNext, unsigned char *pDescriptorCount,
						  unsigned char *pActiveIndex )
{
	RTF_FNAME( "rtfCatGetInfo" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// make the returns
		*pVersionNumber   = pCat->versionNumber;
		*pCurrentNext     = pCat->currentNext;
		*pDescriptorCount = pCat->descriptorCount;
		*pActiveIndex     = pCat->activeIndex;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the active CA descriptor index
RTF_RESULT rtfCatSetActiveDescriptorIndex( RTF_CAT_HANDLE handle, unsigned char activeIndex )
{
	RTF_FNAME( "rtfCatSetActiveDescriptor" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// record the active descriptor index
		pCat->activeIndex = activeIndex;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the active CA descriptor index
RTF_RESULT rtfCatGetActiveDescriptorIndex( RTF_CAT_HANDLE handle, unsigned char *pActiveIndex )
{
	RTF_FNAME( "rtfCatGetActiveDescriptor" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// make the return
		*pActiveIndex = pCat->activeIndex;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get some info on the active descriptor in the conditional access table
RTF_RESULT rtfCatGetActiveDescriptorInfo( RTF_CAT_HANDLE handle,
										  unsigned char  *pTag, unsigned short *pSid,
										  unsigned short *pPid, unsigned char  *pDatLen )
{
	RTF_FNAME( "rtfCatGetActiveDescriptorInfo" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_CAT_DESCR *pDescr;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// make sure the active index is in range
		if( pCat->activeIndex >= pCat->descriptorCount )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "Invalid active CAT index (value %d, count %d)",
						  pCat->activeIndex, pCat->descriptorCount );
			break;
		}
		// get a convenience pointer to the indexed descriptor
		pDescr = &pCat->descriptor[ pCat->activeIndex ];
		// make the returns
		*pTag  = pDescr->tag;
		*pSid  = pDescr->sid;
		*pPid  = pDescr->pid;
		*pDatLen = pDescr->datLen;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// retrieve the private data from the active descriptor in the conditional access table
// note: *pDatLen gives buffer size on entry, data length on return
RTF_RESULT rtfCatGetActiveDescriptorData( RTF_CAT_HANDLE handle,
										  unsigned char *pDatLen, unsigned char *pDat )
{
	RTF_FNAME( "rtfCatGetActiveDescriptorData" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_CAT_DESCR *pDescr;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		if( pCat->activeIndex >= pCat->descriptorCount )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "Invalid active CAT index (value %d, count %d)",
						  pCat->activeIndex, pCat->descriptorCount );
			break;
		}
		// get a convenience pointer to the indexed descriptor
		pDescr = &pCat->descriptor[ pCat->activeIndex ];
		// make sure the buffer is large enough
		if( *pDatLen > pDescr->datLen )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "CAT private data buffer too small (provided %d, needed %d)",
						  *pDatLen, pDescr->datLen );
			break;
		}
		// return the actual data length
		*pDatLen = pDescr->datLen;
		// return the data
		memcpy( (void *)pDat, pDescr->dat, pDescr->datLen );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get some info on one of the descriptors in the conditional access table
RTF_RESULT rtfCatGetDescriptorInfo( RTF_CAT_HANDLE handle, int index,
									unsigned char  *pTag, unsigned short *pSid,
									unsigned short *pPid, unsigned char  *pDatLen )
{
	RTF_FNAME( "rtfCatGetDescriptorInfo" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_CAT_DESCR *pDescr;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// make sure the index is in range
		if( index >= pCat->descriptorCount )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "Invalid CAT index (requested %d, count %d)",
						  index, pCat->descriptorCount );
			break;
		}
		// get a convenience pointer to the indexed descriptor
		pDescr = &pCat->descriptor[ index ];
		// make the returns
		*pTag  = pDescr->tag;
		*pSid  = pDescr->sid;
		*pPid  = pDescr->pid;
		*pDatLen = pDescr->datLen;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// retrieve the private data from one of the descriptors in the conditional access table
// note: *pDatLen gives buffer size on entry, data length on return
RTF_RESULT rtfCatGetDescriptorData( RTF_CAT_HANDLE handle, int index,
								    unsigned char *pDatLen, unsigned char *pDat )
{
	RTF_FNAME( "rtfCatGetDescriptorData" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_CAT_DESCR *pDescr;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// make sure the index is in range
		if( index >= pCat->descriptorCount )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "Invalid CAT index (requested %d, count %d)",
						  index, pCat->descriptorCount );
			break;
		}
		// get a convenience pointer to the indexed descriptor
		pDescr = &pCat->descriptor[ index ];
		// make sure the buffer is large enough
		if( *pDatLen > pDescr->datLen )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_INTERNAL, "CAT private data buffer too small (provided %d, needed %d)",
						  *pDatLen, pDescr->datLen );
			break;
		}
		// return the actual data length
		*pDatLen = pDescr->datLen;
		// return the data
		memcpy( (void *)pDat, pDescr->dat, pDescr->datLen );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset a CAT object
RTF_RESULT rtfCatReset( RTF_CAT_HANDLE handle )
{
	RTF_FNAME( "rtfCatReset" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		RTF_CLR_STATE( &pCat->descriptor, sizeof(pCat->descriptor) );
		pCat->valid = FALSE;
		pCat->versionNumber = 0;
		pCat->currentNext = 0;
		pCat->descriptorCount = 0;
		pCat->activeIndex = 0;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// validate a CAT object
RTF_RESULT rtfCatValidate( RTF_CAT_HANDLE handle, BOOL *pIsValid )
{
	RTF_FNAME( "rtfCatValidate" );
	RTF_OBASE( handle );
 	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// make the return
		*pIsValid = pCat->valid;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse a CAT
// NOTE: this version assumes that all transport streams are
// single programs, and all CATs are a single packet
RTF_RESULT rtfCatParse( RTF_CAT_HANDLE handle, RTF_PKT_HANDLE hPacket )
{
	RTF_FNAME( "rtfCatParse" );
	RTF_OBASE( handle );
	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned char *pStorage;
	unsigned char *pTable;
	unsigned char *pDescr;
	unsigned long  flags;
	unsigned long  pktNum;
	unsigned short pid;
	unsigned short secLen;
	unsigned short descrID;
	unsigned short descrPID;
	unsigned char  payOff;
	unsigned char  payLen;
	unsigned char  pesOff;
	unsigned char  pesLen;
	unsigned char  tableID;
	unsigned char  pointerSkip;
	unsigned char  descrLen;
	unsigned char  dataLen;

	do {		 // error escape wrapper - begin

		// reset the CAT (note: performs CHK_OBJ)
		result = rtfCatReset( handle );
		RTF_CHK_RESULT;
		// get the info on the indicated packet
		result = rtfPktGetInfo( hPacket, &pktNum, &pid, &flags, &pStorage, &payOff, &payLen, &pesOff, &pesLen );
		RTF_CHK_RESULT;
		// is this the CAT PID?
		if( pid != TRANSPORT_CAT_PID )
		{
			// no - skip this packet
			break;
		}
		// does this packet have a payload?
		if( ( flags & RTF_PKT_PAYLOADABSENT ) != 0 )
		{
			// no - skip this packet
			break;
		}
		// is the payload encrypted?
		if( ( flags & RTF_PKT_PAYLOADENCRYPTED ) != 0 )
		{
			// yes - skip this packet
			break;
		}
		// generate a pointer to the start of the CAT in the packet payload
		pointerSkip = pStorage[ payOff ] + 1;
		pTable = pStorage + payOff + pointerSkip;
		// check the table ID
		if( ( tableID = pTable[ 0 ] ) != 0x01 )
		{
			RTF_LOG_ERR1( RTF_MSG_WRN_BADCAT, "Invalid table ID in CAT (expected 1 got %d)", tableID );
			break;
		}
		// parse the section length field
		secLen = pTable[ 1 ];
		secLen <<= 8;
		secLen |= pTable[ 2 ];
		// is the entire CAT in this packet?
		if( secLen > payLen - pointerSkip )
		{
			RTF_LOG_ERR0( RTF_MSG_WRN_BADCAT, "CAT more than 1 packet in length" );
			break;
		}
		// record the version number
		pCat->versionNumber = ( pTable[ 3 ] >> 1 ) & 0x1F;
		// record the current / next flag
		pCat->currentNext = pTable[ 3 ] & 0x01;
		// only 1 section?
		if( pTable[ 4 ] != 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_WRN_BADCAT, "CAT does not start at section 0" );
			break;
		}
		if( pTable[ 5 ] != 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_WRN_BADCAT, "CAT last section not first section" );
			break;
		}
		if( pTable[ 6 ] != 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_WRN_BADCAT, "CAT has more than 1 section" );
			break;
		}
		secLen -= 5;
		pCat->descriptorCount = 0;
		pDescr = pTable + 7;
		while( secLen > 4 )
		{
			// record the descriptor tag
			pCat->descriptor[ pCat->descriptorCount ].tag = pDescr[ 0 ];
			// record the descriptor length
			descrLen = pDescr[ 1 ];
			pCat->descriptor[ pCat->descriptorCount ].len = descrLen;
			// record the CA system ID
			descrID = pDescr[ 2 ];
			descrID <<= 8;
			descrID |= pDescr[ 3 ];
			pCat->descriptor[ pCat->descriptorCount ].sid = descrID;
			// record the CA PID
			descrPID = pDescr[ 4 ];
			descrPID <<= 8;
			descrPID |= pDescr[ 5 ];
			pCat->descriptor[ pCat->descriptorCount ].pid = descrPID;
			// record the private data length for this descriptor
			dataLen = descrLen - 6;
			pCat->descriptor[ pCat->descriptorCount ].datLen = dataLen;
			if( dataLen > 0 )
			{
				// record the private data
				memcpy( (void *)pCat->descriptor[ pCat->descriptorCount ].dat, (void *)&( pDescr[ 6 ] ), dataLen );
			}
			// bump the descriptor count for this CAT
			++pCat->descriptorCount;
			// move forward to the next descriptor
			secLen -= ( descrLen + 2 );
			pDescr += ( descrLen + 2 );
		}
		RTF_CHK_RESULT_LOOP;
		// record the packet info in the state structure
		pCat->pktPayloadOffset = payOff;
		pCat->pktPayloadBytes  = payLen;
		// copy the packet into the state structure
		memcpy( (void *)&( pCat->pkt ), pStorage, TRANSPORT_PACKET_BYTES );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// check a CAT packet to see if the table has changed
RTF_RESULT rtfCatCheckChange( RTF_CAT_HANDLE handle, RTF_PKT_HANDLE hPkt )
{
	RTF_FNAME( "rtfCatCheckChange" );
	RTF_OBASE( handle );
 	RTF_CAT *pCat = (RTF_CAT *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;
	unsigned long packetNumber;
	unsigned long flags;
	unsigned char *pStorage;
	unsigned short pid;
	unsigned char payloadOffset;
	unsigned char payloadBytes;
	unsigned char pesHeaderOffset;
	unsigned char pesHeaderBytes;
	unsigned char *pOld, *pNew;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pCat, RTF_OBJ_TYPE_CAT );
		// get the info on this packet
		result = rtfPktGetInfo( hPkt, &packetNumber, &pid, &flags, &pStorage,
					&payloadOffset, &payloadBytes, &pesHeaderOffset, &pesHeaderBytes );
		// check the payload of the packet to see if it changed
		if( payloadBytes != pCat->pktPayloadBytes )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_CATCHANGED, "CAT changed at packet %d", packetNumber );
		}
		pOld = pCat->pkt + pCat->pktPayloadOffset;
		pNew = pStorage + payloadOffset;
		for( i=0; i<payloadBytes; ++i )
		{
			if( *pOld++ != *pNew++ )
			{
				RTF_LOG_WARN1( RTF_MSG_WRN_CATCHANGED, "CAT changed at packet %d", packetNumber );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
