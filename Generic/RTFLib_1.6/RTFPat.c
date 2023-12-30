// implementation file for rtfPat class
// encapsulates program access table
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

typedef struct _RTF_PAT
{
	RTF_OBJ_HANDLE hBaseObject;

	BOOL valid;
	unsigned short programNumber;
	unsigned short pmtPid;
	unsigned char pktPayloadOffset;
	unsigned char pktPayloadBytes;
	unsigned char pkt[ TRANSPORT_PACKET_BYTES ];

} RTF_PAT;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPatGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfPatGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_PAT);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfPatConstructor( RTF_PAT_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfPatConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_PAT *pPat;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the PAT object
		pPat = (RTF_PAT *)rtfAlloc( sizeof(RTF_PAT) );
		RTF_CHK_ALLOC( pPat );
		// return the handle
		*pHandle = (RTF_PAT_HANDLE)pPat;
		// clear the state structure
		memset( (void *)pPat, 0, sizeof(*pPat) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_PAT, (RTF_HANDLE)pPat, hParent, &pPat->hBaseObject );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfPatDestructor( RTF_PAT_HANDLE handle )
{
	RTF_FNAME( "rtfPatDestructor" );
	RTF_OBASE( handle );
	RTF_PAT *pPat = (RTF_PAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded resource object (note: performs CHK_OBJ)
		result = rtfObjDestructor( pPat->hBaseObject, RTF_OBJ_TYPE_PAT );
		RTF_CHK_RESULT;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// retrieve the PMT pid from the recorded PAT packet (returns INVALID if not yet captured)
RTF_RESULT rtfPatGetPmtPid( RTF_PAT_HANDLE handle, unsigned short *pPmtPid )
{
	RTF_FNAME( "rtfPatGetPmtPid" );
	RTF_OBASE( handle );
	RTF_PAT *pPat = (RTF_PAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPat, RTF_OBJ_TYPE_PAT );
		// make the return (note: will be INVALID if pat not yet captured)
		*pPmtPid = pPat->pmtPid;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// retrieve the program number from the recorded PAT packet (returns 0 if not yet captured)
RTF_RESULT rtfPatGetProgramNumber( RTF_PAT_HANDLE handle, unsigned short *pProgramNumber )
{
	RTF_FNAME( "rtfPatGetPmtPid" );
	RTF_OBASE( handle );
	RTF_PAT *pPat = (RTF_PAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPat, RTF_OBJ_TYPE_PAT );
		// make the return. note: will be 0 if pat not yet captured
		// 0 is network pid program number, which is invalid in this context
		*pProgramNumber = pPat->programNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// retrieve a pointer to the recorded PAT packet
RTF_RESULT rtfPatGetTable( RTF_PAT_HANDLE handle, unsigned char **ppTable )
{
	RTF_FNAME( "rtfPatGetTable" );
	RTF_OBASE( handle );
	RTF_PAT *pPat = (RTF_PAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPat, RTF_OBJ_TYPE_PAT );
		// make the return (note: return NULL if pat not yet captured)
		*ppTable = ( pPat->valid == FALSE ) ?  (unsigned char *)NULL : pPat->pkt;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset a PAT object
RTF_RESULT rtfPatReset( RTF_PAT_HANDLE handle )
{
	RTF_FNAME( "rtfPatReset" );
	RTF_OBASE( handle );
	RTF_PAT *pPat = (RTF_PAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPat, RTF_OBJ_TYPE_PAT );
		RTF_CLR_STATE( &pPat->pkt, sizeof(pPat->pkt) );
		pPat->pmtPid = TRANSPORT_INVALID_PID;
		pPat->programNumber = 0;
		pPat->valid = FALSE;
		pPat->pktPayloadOffset = 0;
		pPat->pktPayloadBytes = 0;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// validate a PAT object
RTF_RESULT rtfPatValidate( RTF_PAT_HANDLE handle, BOOL *pIsValid )
{
	RTF_FNAME( "rtfPatValidate" );
	RTF_OBASE( handle );
 	RTF_PAT *pPat = (RTF_PAT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPat, RTF_OBJ_TYPE_PAT );
		// make the return
		*pIsValid = pPat->valid;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse a PAT
// NOTE: this version assumes that all transport streams are
// single programs, and all PATs are a single packet
RTF_RESULT rtfPatParse( RTF_PAT_HANDLE handle, RTF_PKT_HANDLE hPacket )
{
	RTF_FNAME( "rtfPatParse" );
	RTF_OBASE( handle );
	RTF_PAT *pPat = (RTF_PAT *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned char *pStorage;
	unsigned long  flags;
	unsigned long  pktNum;
	unsigned short pid;
	unsigned short limit;
	unsigned char  offset;
	unsigned char  payOff;
	unsigned char  payLen;
	unsigned char  pesOff;
	unsigned char  pesLen;

	do {		 // error escape wrapper - begin

		// reset the PAT (note: performs CHK_OBJ)
		result = rtfPatReset( handle );
		RTF_CHK_RESULT;
		// get the info on the indicated packet
		result = rtfPktGetInfo( hPacket, &pktNum, &pid, &flags, &pStorage,
								&payOff, &payLen, &pesOff, &pesLen );
		RTF_CHK_RESULT;
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
		// adjust the offset to account for the pointer byte
		offset = payOff;
		offset += pStorage[ offset] + 1;
		// get the section length
		// use it to compute the end offset of the table (don't look at the CRC)
		limit = pStorage[ offset + 1 ] & 0x0F;
		limit = ( limit << 8 ) | pStorage[ offset + 2 ];
		limit += ( offset + 2 );
		limit -= 4;
		// this should be in the current packet
		if( limit > TRANSPORT_PACKET_BYTES )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADSTREAM, "PAT has section length greater than 1 packet" );
			break;
		}
		// adjust offset past the PAT section preamble
		offset += 8;
		// find the first PMT descriptor. If successful, copy the PAT and return the PMT PID
		while ( offset < limit )
		{
			pPat->programNumber = WRD( pStorage, offset );
			if ( pPat->programNumber != 0 )
			{
				// did we already find a PMT in this PAT?
				if( pPat->valid == FALSE )
				{
					// no - record this PMT; mark the PAT as valid
					pPat->pmtPid = PID( pStorage, offset+2 );
					pPat->valid = TRUE;
					// also copy the packet into the state structure
					pPat->pktPayloadOffset = payOff;
					pPat->pktPayloadBytes  = payLen;
					memcpy( (void *)pPat->pkt, (void *)pStorage, TRANSPORT_PACKET_BYTES );
				}
				else
				{
					// yes - but there should only be one
					RTF_LOG_WARN1( RTF_MSG_WRN_MULTIPROGSTREAM, "Multiple programs listed in PAT at packet %d", pktNum );
				}
			}
			offset += 4;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// check a PAT packet to see if the table has changed
RTF_RESULT rtfPatCheckChange( RTF_PAT_HANDLE handle, RTF_PKT_HANDLE hPkt )
{
	RTF_FNAME( "rtfPatCheckChange" );
	RTF_OBASE( handle );
 	RTF_PAT *pPat = (RTF_PAT *)handle;
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

		RTF_CHK_OBJ( pPat, RTF_OBJ_TYPE_PAT );
		// get the info on this packet
		result = rtfPktGetInfo( hPkt, &packetNumber, &pid, &flags, &pStorage,
								&payloadOffset, &payloadBytes,
								&pesHeaderOffset, &pesHeaderBytes );
		RTF_CHK_RESULT;
		// check the payload of the packet to see if it changed
		if( payloadBytes != pPat->pktPayloadBytes )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_PATCHANGED, "PAT changed at packet %d", packetNumber );
		}
		pOld = pPat->pkt + pPat->pktPayloadOffset;
		pNew = pStorage + payloadOffset;
		for( i=0; i<payloadBytes; ++i )
		{
			if( *pOld++ != *pNew++ )
			{
				RTF_LOG_WARN1( RTF_MSG_WRN_PATCHANGED, "PAT changed at packet %d", packetNumber );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
