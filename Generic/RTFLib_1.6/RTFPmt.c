// implementation file for rtfPmt class
// encapsulates program map table
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

typedef struct _RTF_PMT
{
	RTF_OBJ_HANDLE hBaseObject;

	BOOL valid;
	BOOL splicingEnabled;
	RTF_STREAM_PROFILE *pProfile;
	unsigned long pktNum;
	unsigned char pktPayloadOffset;
	unsigned char pktPayloadBytes;
	RTF_STREAM_TYPE streamType[ TRANSPORT_MAX_PID_VALUES ];
	unsigned char pkt[ TRANSPORT_PACKET_BYTES ];

} RTF_PMT;

// local functions **********************************************************************

// parse a set of program or program element descriptors
static RTF_RESULT rtfParseDescriptors( RTF_PMT *pPmt, unsigned char *pStorage,
									   unsigned short *pOffset, unsigned short length,
									   RTF_DESCR_LIST *pList )
{
	RTF_FNAME( "rtfParseDescriptors" );
	RTF_OBASE( pPmt );
	RTF_RESULT result = RTF_PASS;
	RTF_DESCR_SPEC *pDescrSpec;
	int i;
	short blockLength;
	unsigned short offset;
	unsigned char tag, descrLength;

	do {		 // error escape wrapper - begin

		// get the info length field
		blockLength = WRD( pStorage, *pOffset ) & 0x0FFF;
		// move the parsing point past the length field
		*pOffset += 2;
		// create a local copy of the current offset
		offset = *pOffset;
		// move the calling routine's parsing offset past the info block
		*pOffset += blockLength;
		// iterate over the descriptors
		while( blockLength > 0 )
		{
			// get the next descriptor tag and length
			tag = pStorage[ offset ];
			descrLength = pStorage[ offset + 1 ];
			// decode the tag
			switch( tag )
			{
			case RTF_DESCRTAG_CA:
				// point at the next available descriptor for this estream
				pDescrSpec = &pList->descrSpec[ pList->descrCount++ ];
				// record the tag
				pDescrSpec->tag = RTF_DESCRTAG_CA;
				// get the CA system ID
				pDescrSpec->descr.ca.sid = pStorage[ offset + 2 ];
				pDescrSpec->descr.ca.sid = ( pDescrSpec->descr.ca.sid << 8 ) | pStorage[ offset + 3 ];
				// get the CA PID
				pDescrSpec->descr.ca.pid = pStorage[ offset + 4 ] & 0x1F;
				pDescrSpec->descr.ca.pid = ( pDescrSpec->descr.ca.pid << 8 ) | pStorage[ offset + 5 ];
				// mark the stream type of the CA PID
				pPmt->streamType[ pDescrSpec->descr.ca.pid ] = RTF_STREAM_TYPE_CA;
				break;
			case RTF_DESCRTAG_LANGUAGE:
				// point at the next available descriptor for this estream
				pDescrSpec = &pList->descrSpec[ pList->descrCount++ ];
				// record the tag
				pDescrSpec->tag = RTF_DESCRTAG_LANGUAGE;
				// iterate thru the language entries
				offset += 2;
				i = 0;
				while( offset < length )
				{
					// record the language code and the audio type
					pDescrSpec->descr.lang.code[ i ][ 0 ] = pStorage[ offset++ ];
					pDescrSpec->descr.lang.code[ i ][ 1 ] = pStorage[ offset++ ];
					pDescrSpec->descr.lang.code[ i ][ 2 ] = pStorage[ offset++ ];
					pDescrSpec->descr.lang.code[ i ][ 3 ] = pStorage[ offset++ ];
					++i;
				}
				break;
			default:
				// not currently interested in any other descriptors
				break;
			}
			// advance past this descriptor
			offset += descrLength + 2;
			blockLength -= ( descrLength + 2 );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfPmtParseVideo( RTF_PMT *pPmt, RTF_VIDEO_CODEC_TYPE type,
								    unsigned short pid, unsigned char *pStorage,
									unsigned short *pOffset, unsigned char length )
{
	RTF_FNAME( "rtfPmtParseVideo" );
	RTF_OBASE( pPmt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// record the stream type for this PID
		pPmt->streamType[ pid ] = RTF_STREAM_TYPE_VID;
		// only accept the first video PID
		if( pPmt->pProfile->videoSpec.pid == TRANSPORT_INVALID_PID )
		{
			pPmt->pProfile->videoSpec.pid = pid;
			pPmt->pProfile->videoSpec.eStream.video.vcdType = type;
		}
		else
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_MULTIVIDSTREAM,
						   "Multiple video streams in PMT at packet %d", pPmt->pktNum );
		}
		// move parsing point past the stream type and the PID
		*pOffset += 3;
		// parse the descriptors, if present
		result = rtfParseDescriptors( pPmt, pStorage, pOffset, (unsigned short)length,
					&pPmt->pProfile->videoSpec.list );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfPmtParseAudio( RTF_PMT *pPmt, RTF_AUDIO_CODEC_TYPE type,
								   	unsigned short pid, unsigned char *pStorage,
									unsigned short *pOffset, unsigned char length )
{
	RTF_FNAME( "rtfPmtParseAudio" );
	RTF_OBASE( pPmt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// record the stream type for this PID
		pPmt->streamType[ pid ] = RTF_STREAM_TYPE_AUD;
		// room for another audio PID?
		if( pPmt->pProfile->audioCount < RTF_MAX_AUDIO_PIDS )
		{
			// yes - is splicing enabled?
			if( pPmt->splicingEnabled != FALSE )
			{
				// make sure that this an audio codec that is supported for splicing
				if( type != RTF_AUDIO_CODEC_TYPE_AC3 )
				{
					RTF_LOG_WARN1( RTF_MSG_WRN_SPLICINGAUDIOBAD,
								   "MPEG1 audio with splicing enabled in PMT at packet %d",
								   pPmt->pktNum );
				}
			}
			// record the audio CODEC info
			pPmt->pProfile->audioSpec[ pPmt->pProfile->audioCount ].pid = pid;
			pPmt->pProfile->audioSpec[ pPmt->pProfile->audioCount ].eStream.audio.acdType = type;
		}
		else
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_MULTIAUDIOCODEC,
						   "Too many audio PIDs listed in PMT at packet %d ", pPmt->pktNum );
		}
		// move parsing point past the stream type and the PID
		*pOffset += 3;
		// parse the descriptors, if present
		result = rtfParseDescriptors( pPmt, pStorage, pOffset, (unsigned short)length,
						&pPmt->pProfile->audioSpec[ pPmt->pProfile->audioCount++ ].list );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfPmtParseData( RTF_PMT *pPmt, RTF_DATA_STREAM_TYPE type,
								   unsigned short pid, unsigned char *pStorage,
								   unsigned short *pOffset, unsigned char length )
{
	RTF_FNAME( "rtfPmtParseData" );
	RTF_OBASE( pPmt );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// record the stream type for this PID
		pPmt->streamType[ pid ] = RTF_STREAM_TYPE_DAT;
		// room for another data PID?
		if( pPmt->pProfile->dataCount < RTF_MAX_DATA_PIDS )
		{
			// yes - record the data stream info
			pPmt->pProfile->dataSpec[ pPmt->pProfile->dataCount ].pid = pid;
			pPmt->pProfile->dataSpec[ pPmt->pProfile->dataCount ].eStream.data.dstType = type;
		}
		else
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_MULTIAUDIOCODEC,
						   "Too many data PIDs listed in PMT at packet %d ", pPmt->pktNum );
		}
		// move parsing point past the stream type and the PID
		*pOffset += 3;
		// parse the descriptors, if present
		result = rtfParseDescriptors( pPmt, pStorage, pOffset, (unsigned short)length,
					&pPmt->pProfile->dataSpec[ pPmt->pProfile->dataCount++ ].list );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPmtGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfPmtGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_PMT);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfPmtConstructor( RTF_PMT_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfPmtConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_PMT *pPmt;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the PMT object
		pPmt = (RTF_PMT *)rtfAlloc( sizeof(RTF_PMT) );
		RTF_CHK_ALLOC( pPmt );
		// return the handle
		*pHandle = (RTF_PMT_HANDLE)pPmt;
		// clear the state structure
		memset( (void *)pPmt, 0, sizeof(*pPmt) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_PMT, (RTF_HANDLE)pPmt, hParent, &pPmt->hBaseObject );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfPmtDestructor( RTF_PMT_HANDLE handle )
{
	RTF_FNAME( "rtfPmtDestructor" );
	RTF_OBASE( handle );
	RTF_PMT *pPmt = (RTF_PMT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded resource object (note: performs type check)
		result = rtfObjDestructor( pPmt->hBaseObject, RTF_OBJ_TYPE_PMT );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// retrieve a pointer to the recorded PMT packet
RTF_RESULT rtfPmtGetPacket( RTF_PMT_HANDLE handle, unsigned char **ppPacket )
{
	RTF_FNAME( "rtfPmtGetPacket" );
	RTF_OBASE( handle );
	RTF_PMT *pPmt = (RTF_PMT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPmt, RTF_OBJ_TYPE_PMT );
		// make the return (note: return NULL if pat not yet captured)
		*ppPacket = ( pPmt->valid == FALSE ) ? (unsigned char *)NULL : pPmt->pkt;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// retrieve the stream type of a particular PID
RTF_RESULT rtfPmtGetStreamType( RTF_PMT_HANDLE handle, unsigned short pid,
							    RTF_STREAM_TYPE *pStreamType )
{
	RTF_FNAME( "rtfPmtGetStreamType" );
	RTF_OBASE( handle );
	RTF_PMT *pPmt = (RTF_PMT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPmt, RTF_OBJ_TYPE_PMT );
		// make the return
		*pStreamType = pPmt->streamType[ pid ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// retrieve the stream types of all known PIDs
RTF_RESULT rtfPmtGetAllStreamTypes( RTF_PMT_HANDLE handle, RTF_STREAM_TYPE **ppStreamType )
{
	RTF_FNAME( "rtfPmtGetAllStreamTypes" );
	RTF_OBASE( handle );
	RTF_PMT *pPmt = (RTF_PMT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPmt, RTF_OBJ_TYPE_PMT );
		// make the return
		*ppStreamType = pPmt->streamType;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset a PMT object
RTF_RESULT rtfPmtReset( RTF_PMT_HANDLE handle )
{
	RTF_FNAME( "rtfPmtReset" );
	RTF_OBASE( handle );
	RTF_PMT *pPmt = (RTF_PMT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPmt, RTF_OBJ_TYPE_PMT );
		pPmt->splicingEnabled = FALSE;
		pPmt->valid = FALSE;
		pPmt->pktPayloadOffset = 0;
		pPmt->pktPayloadBytes = 0;
		pPmt->pktNum = 0;
		pPmt->pProfile = (RTF_STREAM_PROFILE *)NULL;
		// pre-set all PID stream types to unknown
		memset( (void *)pPmt->streamType, 0, sizeof(pPmt->streamType) );
		// except for hard-wired PIDs
		pPmt->streamType[ TRANSPORT_PAT_PID ] = RTF_STREAM_TYPE_PAT;
		pPmt->streamType[ TRANSPORT_CAT_PID ] = RTF_STREAM_TYPE_CAT;
		pPmt->streamType[ TRANSPORT_PAD_PID ] = RTF_STREAM_TYPE_NUL;
		RTF_CLR_STATE( pPmt->pkt, sizeof(pPmt->pkt) );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// validate a PMT object
RTF_RESULT rtfPmtValidate( RTF_PMT_HANDLE handle, BOOL *pIsValid )
{
	RTF_FNAME( "rtfPmtValidate" );
	RTF_OBASE( handle );
	RTF_PMT *pPmt = (RTF_PMT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPmt, RTF_OBJ_TYPE_PMT );
		// make the return
		*pIsValid = pPmt->valid;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// parse a PMT
// NOTE: this version requires that all transport streams are
// single programs, and all PMT sections reside in a single packet
RTF_RESULT rtfPmtParse( RTF_PMT_HANDLE handle, RTF_PKT_HANDLE hPacket )
{
	RTF_FNAME( "rtfPmtParse" );
	RTF_OBASE( handle );
	RTF_PMT *pPmt = (RTF_PMT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_SES_HANDLE hSes;
	unsigned char *pStorage;
	unsigned char *pPkt;
	unsigned long  pktNum;
	unsigned long  flags;
	unsigned short pid;
	unsigned short offset;
	unsigned short programLen;
	unsigned short descrLen;
	unsigned short pmtSectionLen;
	unsigned short pmtSectionOffset;
	unsigned short pmtSectionEndOffset;
	unsigned short payOff;
	unsigned char  payLen;
	unsigned char  pesOff;
	unsigned char  pesLen;
	unsigned char  streamType;
	unsigned char  ucTemp;

	do {		 // error escape wrapper - begin

		// reset the info structure (note: performs CHK_OBJ)
		result = rtfPmtReset( pPmt );
		RTF_CHK_RESULT;
		// get the info on the indicated packet
		result = rtfPktGetInfo( hPacket, &pktNum, &pid, &flags, &pStorage,
								&ucTemp, &payLen, &pesOff, &pesLen );
		RTF_CHK_RESULT;
		payOff = ucTemp;
		// does this packet have a payload?
		if( ( flags & RTF_PKT_PAYLOADABSENT ) != 0 )
		{
			// no - skip this packet
			break;
		}
		// is the payload encrypted?
		if( ( flags & RTF_PKT_PAYLOADENCRYPTED ) != 0 )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_PMTENCRYPTED, "Encrypted PMT encountered at packet %d", pktNum );
			break;
		}
		// adjust the offset by the contents of the pointer byte
		offset = payOff + pStorage[ payOff ] + 1;
		// see if there is a PMT section in this packet
		while( offset < TRANSPORT_PACKET_BYTES )
		{
			// does the table ID indicate a PMT section?
			if( pStorage[ offset ] == 0x02 )
			{
				// yes - parse this section
				break;
			}
			else if( pStorage[ offset ] == 0xFF )
			{
				// no more tables in this packet
				offset = TRANSPORT_PACKET_BYTES;
				break;
			}
			// not an interesting table (probably user private)
			// skip the table by adding the table length to the offset
			++offset;
			offset += WRD( pStorage, offset ) + 2;
		}
		RTF_CHK_RESULT_LOOP;
		if( offset >= TRANSPORT_PACKET_BYTES )
		{
			// no PMT in this packet - keep looking
			break;
		}
		// record the offset of the PMT section
		pmtSectionOffset = offset;
		// record the length of the PMT section
		offset += 1;
		pmtSectionLen = ( WRD( pStorage, offset ) & 0x0FFF ) + 3;
		offset += 2;
		// compute the offset of the end of the section (don't look at the CRC)
		pmtSectionEndOffset = pmtSectionOffset + pmtSectionLen - 4;
		// get the handle of the session that owns this PMT
		result = rtfObjGetSession( handle, &hSes );
		RTF_CHK_RESULT;
		// get the state of splicing enable from the session
		result = rtfSesGetSplicingEnabled( hSes, &pPmt->splicingEnabled );
		RTF_CHK_RESULT;
		// get a copy of the current input stream profile from the session object
		result = rtfSesGetStreamProfile( hSes, &pPmt->pProfile );
		RTF_CHK_RESULT;
		// parse the PMT table - record the packet number
		pPmt->pktNum = pktNum;
		// mark the PMT PID stream type
		pPmt->streamType[ pid ] = RTF_STREAM_TYPE_PMT;
		// adjust offset to the PCR PID position
		offset += 5;
		// record the PCR PID
		pPmt->pProfile->pcrPID = PID( pStorage, offset );
		offset += 2;
		// mark the PCR PID stream type (may be overridden if PCR carried by media stream)
		pPmt->streamType[ pPmt->pProfile->pcrPID ] = RTF_STREAM_TYPE_PCR;
		// get the program info length field
		programLen = WRD( pStorage, offset ) & 0x0FFF;
		// if splicing is enabled, this should be zero
		// !!! FIX ME !!! THIS SHOULD GO AWAY WITH IMPLEMENTATION OF SPLICING PROFILES !!!
		if( ( pPmt->splicingEnabled != FALSE ) && ( programLen != 0 ) )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_SPLICINGPSDESC,
						   "Program stream descriptor in PMT with splicing enabled - packet %d",
						   pPmt->pktNum );
		}
		// parse any program info descriptors that are present
		result = rtfParseDescriptors( pPmt, pStorage, &offset, programLen,
									  &pPmt->pProfile->progDescrList );
		RTF_CHK_RESULT;
		// iterate over the PID descriptor table
		while ( offset < pmtSectionEndOffset )
		{
			// get the stream type
			streamType = pStorage[ offset ];
			// get the PID
			pid = PID( pStorage, offset + 1 );
			// decode the stream type - look for a audio / video / data components
			switch( streamType )
			{
			case STREAM_TYPE_VIDEO_13818:
			case STREAM_TYPE_DIGICIPHER2:
				result = rtfPmtParseVideo( pPmt, RTF_VIDEO_CODEC_TYPE_MPEG2, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_VIDEO_14496:
				result = rtfPmtParseVideo( pPmt, RTF_VIDEO_CODEC_TYPE_H264, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_VIDEO_VC1:
				result = rtfPmtParseVideo( pPmt, RTF_VIDEO_CODEC_TYPE_VC1, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_AUDIO_11172:
				result = rtfPmtParseAudio( pPmt, RTF_AUDIO_CODEC_TYPE_MPEG1, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_AUDIO_13818:
				result = rtfPmtParseAudio( pPmt, RTF_AUDIO_CODEC_TYPE_MPEG2, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_AUDIO_MULTI:
				result = rtfPmtParseAudio( pPmt, RTF_AUDIO_CODEC_TYPE_MULTI, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_AUDIO_AC3:
				result = rtfPmtParseAudio( pPmt, RTF_AUDIO_CODEC_TYPE_AC3, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_AUDIO_LPCM:
				result = rtfPmtParseAudio( pPmt, RTF_AUDIO_CODEC_TYPE_LPCM, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_AUDIO_DTS:
				result = rtfPmtParseAudio( pPmt, RTF_AUDIO_CODEC_TYPE_DTS, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_AUDIO_DVS:
				result = rtfPmtParseAudio( pPmt, RTF_AUDIO_CODEC_TYPE_DVS, pid,
										   pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_DATA_H2220:
				result = rtfPmtParseData( pPmt, RTF_DATA_STREAM_TYPE_H2220, pid,
										  pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_DATA_MHEG:
				result = rtfPmtParseData( pPmt, RTF_DATA_STREAM_TYPE_MHEG, pid,
										  pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			case STREAM_TYPE_DATA_DSMCCA:
			case STREAM_TYPE_DATA_DSMCCB:
			case STREAM_TYPE_DATA_DSMCCC:
			case STREAM_TYPE_DATA_DSMCCD:
				result = rtfPmtParseData( pPmt, RTF_DATA_STREAM_TYPE_DSMCC, pid,
										  pStorage, &offset, pmtSectionLen - 3 );
				RTF_CHK_RESULT;
				break;
			default:
				// skip other stream descriptors
				pPmt->streamType[ pid ] = RTF_STREAM_TYPE_UNKNOWN;
				// move parsing point past the stream type and the PID
				offset += 3;
				// get the ES_INFO_LENGTH field
				descrLen = WRD( pStorage, offset ) & 0x0FFF;
				// move the parsing point past this descriptor
				offset += 2 + descrLen;
				break;
			}
			RTF_CHK_RESULT_LOOP;
		}
		RTF_CHK_RESULT_LOOP;
		// did this PMT have no audio, video, or data PIDs?
		if( ( pPmt->pProfile->videoSpec.eStream.video.vcdType == RTF_VIDEO_CODEC_TYPE_INVALID ) &&
			( pPmt->pProfile->audioCount == 0 ) &&
			( pPmt->pProfile->dataCount  == 0 ) )
		{
			// yes - disregard this table
			break;
		}
		// create a PMT packet that contains just the PMT section
		pPkt = pPmt->pkt;
		memset( (void *)pPkt, 0xFF, TRANSPORT_PACKET_BYTES );
		pPkt[ 0 ] = pStorage[ 0 ];
		pPkt[ 1 ] = pStorage[ 1 ];
		pPkt[ 2 ] = pStorage[ 2 ];
		pPkt[ 3 ] = 0x10;
		pPkt[ 4 ] = 0x00;
		memcpy( (void *)&( pPkt[ 5 ] ), (void *)&( pStorage[ pmtSectionOffset ] ), pmtSectionLen );
		pPmt->pktPayloadOffset = 5;
		pPmt->pktPayloadBytes  = (unsigned char)( pmtSectionLen & 0xFF );
		// mark the PMT as valid
		pPmt->valid = TRUE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// check a PMT packet to see if the table has changed
RTF_RESULT rtfPmtCheckChange( RTF_PMT_HANDLE handle, RTF_PKT_HANDLE hPkt )
{
#if 0
	RTF_FNAME( "rtfPmtCheckChange" );
	RTF_OBASE( handle );
 	RTF_PMT *pPmt = (RTF_PMT *)handle;
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

		RTF_CHK_OBJ( pPmt, RTF_OBJ_TYPE_PMT );
		// get the info on this packet
		result = rtfPktGetInfo( hPkt, &packetNumber, &pid, &flags, &pStorage,
								&payloadOffset, &payloadBytes,
								&pesHeaderOffset, &pesHeaderBytes );
		// check the payload of the packet to see if it changed
		if( payloadBytes != pPmt->pktPayloadBytes )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_PMTCHANGED, "PMT changed at packet %d", packetNumber );
		}
		pOld = pPmt->pkt + pPmt->pktPayloadOffset;
		pNew = pStorage + payloadOffset;
		for( i=0; i<payloadBytes; ++i )
		{
			if( *pOld++ != *pNew++ )
			{
				RTF_LOG_WARN1( RTF_MSG_WRN_PMTCHANGED, "PMT changed at packet %d", packetNumber );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
#else
	return RTF_PASS;
#endif
}
