// implementation file for rtfPic class
// encapsulates picture
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

#ifdef DO_VCD_MPEG2

typedef struct _RTF_PIC_VCD_INFO_MPEG2
{
	// picture header info
	unsigned short temporalReference;

} RTF_PIC_VCD_INFO_MPEG2;

#endif // DO_VCD_MPEG2

#ifdef DO_VCD_H264

typedef struct _RTF_PIC_VCD_INFO_H264
{
	// parameter set ID
	unsigned long paramId;
	// frame number
	unsigned long frameNum;

} RTF_PIC_VCD_INFO_H264;

#endif // DO_VCD_H264

#ifdef DO_VCD_VC1

typedef struct _RTF_PIC_VCD_INFO_VC1
{
	unsigned long bar;

} RTF_PIC_VCD_INFO_VC1;

#endif // DO_VCD_VC1

typedef union _RTF_PIC_VCD_INFO
{
#ifdef DO_VCD_MPEG2
	RTF_PIC_VCD_INFO_MPEG2 mpeg2;
#endif
#ifdef DO_VCD_H264
	RTF_PIC_VCD_INFO_H264  h264;
#endif
#ifdef DO_VCD_VC1
	RTF_PIC_VCD_INFO_VC1   vc1;
#endif

} RTF_PIC_VCD_INFO;

typedef struct _RTF_PIC
{
	RTF_OBJ_HANDLE hBaseObject;

	// session info
	RTF_SES_HANDLE hSes;
	RTF_WIN_HANDLE hWin;
	unsigned long picNumber;
	RTF_PICSTATE state;
	INT64 srcPTS;
	INT64 srcDTS;
	unsigned long flags;
	unsigned long lastAddedPktNumber;
	unsigned short vidPid;

	// packets contributing to this picture
	unsigned char firstBytePacketOffset;
	unsigned char lastBytePacketOffset;
	unsigned short lastVideoPacketIndex;
	unsigned short packetCount;
	RTF_BUF_HANDLE hPacket[ RTF_MAX_PICTURE_PACKETS ];

	// codec specific info
	RTF_VIDEO_CODEC_TYPE vcdType;
	RTF_PIC_VCD_INFO vcdInfo;

} RTF_PIC;

// private functions ********************************************************************

static void resetPic( RTF_PIC *pPic, RTF_VIDEO_CODEC_TYPE vcdType )
{
	pPic->picNumber = -1;
	pPic->state = RTF_PICSTATE_RELEASED;
	pPic->firstBytePacketOffset = 0;
	pPic->lastBytePacketOffset = 0;
	pPic->packetCount = 0;
	pPic->flags = 0;
	pPic->lastAddedPktNumber = -1;
	pPic->vidPid = TRANSPORT_INVALID_PID;
	pPic->srcPTS = -1;
	pPic->srcDTS = -1;
	RTF_CLR_STATE( pPic->hPacket, sizeof(pPic->hPacket) );
	pPic->vcdType = vcdType;
	switch( vcdType )
	{
#ifdef DO_VCD_MPEG2
	case RTF_VIDEO_CODEC_TYPE_MPEG2:
		pPic->vcdInfo.mpeg2.temporalReference = 0;
		break;
#endif
#ifdef DO_VCD_H264
	case RTF_VIDEO_CODEC_TYPE_H264:
		pPic->vcdInfo.h264.paramId  = 0;
		pPic->vcdInfo.h264.frameNum = 0;
		break;
#endif
#ifdef DO_VCD_VC1
	case RTF_VIDEO_CODEC_TYPE_VC1:
		break;
#endif
	default:
		memset( (void *)&pPic->vcdInfo, 0, sizeof(pPic->vcdInfo) );
	}
}

// remove the last packet from a picture
static RTF_RESULT rtfPicRemoveLastPacket( RTF_PIC *pPic, unsigned char newLastBytePacketOffset )
{
	RTF_FNAME( "rtfPicRemoveLastPacket" );
	RTF_OBASE( pPic );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// never remove the only remaining packet
		if( pPic->packetCount > 1 )
		{
			// remove a reference from the last packet
			// Note: it will remove a reference from the buffer that contains it
			result = rtfPktRemoveReference( pPic->hPacket[ pPic->packetCount - 1 ], (RTF_PIC_HANDLE)pPic );
			RTF_CHK_RESULT;
			// null out the last packet handle in the picture list, and decrement the count
			pPic->hPacket[ --pPic->packetCount ] = (RTF_PKT_HANDLE)NULL;
			// record the new last byte packet offset
			pPic->lastBytePacketOffset = newLastBytePacketOffset;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPicGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfPicGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_PIC);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfPicConstructor( RTF_PIC_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfPicConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_PIC *pPic;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the PIC object
		pPic = (RTF_PIC *)rtfAlloc( sizeof(RTF_PIC) );
		RTF_CHK_ALLOC( pPic );
		// return the handle
		*pHandle = (RTF_PIC_HANDLE)pPic;
		// clear the state structure
		memset( (void *)pPic, 0, sizeof(*pPic) );
		// create an embedded base object
		result = rtfObjConstructor( RTF_OBJ_TYPE_PIC, (RTF_HANDLE) pPic, hParent, &pPic->hBaseObject );
		RTF_CHK_RESULT;
		// record the handle of the owning session for efficiency
		result = rtfObjGetSession( (RTF_HANDLE)pPic, &pPic->hSes );
		RTF_CHK_RESULT;
		// record the handle of the session's parsing window (also for efficiency)
		result = rtfSesGetWindow( pPic->hSes, &pPic->hWin );
		RTF_CHK_RESULT;
		// reset the picture state structure
		resetPic( pPic, RTF_VIDEO_CODEC_TYPE_INVALID );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfPicDestructor( RTF_PIC_HANDLE handle )
{
	RTF_FNAME( "rtfPicDestructor" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded resource object (note: performs CHK_OBJ)
		result = rtfObjDestructor( pPic->hBaseObject, RTF_OBJ_TYPE_PIC );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the session's picture number from a picture object
RTF_RESULT rtfPicGetNumber( RTF_PIC_HANDLE handle, unsigned long *pPicNumber )
{
	RTF_FNAME( "rtfPicGetNumber" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the return
		*pPicNumber = pPic->picNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the state of a picture object
RTF_RESULT rtfPicGetState( RTF_PIC_HANDLE handle, RTF_PICSTATE *pState )
{
	RTF_FNAME( "rtfPicGetState" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the return
		*pState = pPic->state;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the end-of-picture location
RTF_RESULT rtfPicSetEndInfo( RTF_PIC_HANDLE handle, RTF_PKT_HANDLE hPkt, unsigned char off )
{
	RTF_FNAME( "rtfPicSetEndInfo" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// find the new last packet in the packet array
		for( i=pPic->packetCount-1; i>=0; --i )
		{
			if( pPic->hPacket[ i ] == hPkt )
			{
				break;
			}
		}
		// remove any packets following the new last packet in the array
		for( ++i; i<pPic->packetCount; ++i )
		{
			result = rtfPicRemoveLastPacket( pPic, off );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set this picture's keyframe flag
RTF_RESULT rtfPicSetIsKeyframe( RTF_PIC_HANDLE handle, BOOL isKeyframe )
{
	RTF_FNAME( "rtfPicSetIsKeyframe" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// set the flag
		if( isKeyframe == FALSE )
		{
			pPic->flags &= ~RTF_PIC_ISKEYFRAME;
		}
		else
		{
			pPic->flags |= RTF_PIC_ISKEYFRAME;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get this picture's keyframe flag
RTF_RESULT rtfPicGetIsKeyframe( RTF_PIC_HANDLE handle, BOOL *pIsKeyframe )
{
	RTF_FNAME( "rtfPicGetIsKeyframe" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the return
		*pIsKeyframe = ( ( pPic->flags & RTF_PIC_ISKEYFRAME ) != 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set this picture's damage flag
RTF_RESULT rtfPicSetIsDamaged( RTF_PIC_HANDLE handle, BOOL isDamaged )
{
	RTF_FNAME( "rtfPicSetIsDamaged" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// set the flag
		if( isDamaged == FALSE )
		{
			pPic->flags &= ~RTF_PIC_ISDAMAGED;
		}
		else
		{
			pPic->flags |= RTF_PIC_ISDAMAGED;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get this picture's damage flag
RTF_RESULT rtfPicGetIsDamaged( RTF_PIC_HANDLE handle, BOOL *pIsDamaged )
{
	RTF_FNAME( "rtfPicGetIsDamaged" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the return
		*pIsDamaged = ( ( pPic->flags & RTF_PIC_ISDAMAGED ) != 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get this picture's original presentation time stamp (if present)
RTF_RESULT rtfPicGetSrcPTS( RTF_PIC_HANDLE handle, INT64 *pSrcPTS )
{
	RTF_FNAME( "rtfPicGetSrcPTS" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the return
		*pSrcPTS = pPic->srcPTS;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get this picture's original decoding time stamp (if present)
RTF_RESULT rtfPicGetSrcDTS( RTF_PIC_HANDLE handle, INT64 *pSrcDTS )
{
	RTF_FNAME( "rtfPicGetSrcDTS" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the return
		*pSrcDTS = pPic->srcDTS;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of the first packet in a picture
RTF_RESULT rtfPicGetFirstPktNum( RTF_PIC_HANDLE handle, unsigned long *pPktNum )
{
	RTF_FNAME( "rtfPicGetFirstPktNum" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPkt;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// is there a packet in the picture?
		if( ( hPkt = pPic->hPacket[ 0 ] ) == (RTF_PKT_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No packet in picture" );
			break;
		}
		// make the return
		result = rtfPktGetOutPktNumber( hPkt, pPktNum );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the mapped number of the first packet in a picture
RTF_RESULT rtfPicGetFirstPktMapNum( RTF_PIC_HANDLE handle, unsigned long *pPktNum )
{
	RTF_FNAME( "rtfPicGetFirstPktMapNum" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPkt;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// is there a packet in the picture?
		if( ( hPkt = pPic->hPacket[ 0 ] ) == (RTF_PKT_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No packet in picture" );
			break;
		}
		// make the return
		result = rtfPktGetMapPktNumber( hPkt, pPktNum );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of the last packet in a picture
RTF_RESULT rtfPicGetLastPktNum( RTF_PIC_HANDLE handle, unsigned long *pPktNum )
{
	RTF_FNAME( "rtfPicGetLastPktNum" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPkt;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// is there at least one packet in the picture?
		if( pPic->packetCount == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No packets in picture" );
			break;
		}
		if( ( hPkt = pPic->hPacket[ pPic->packetCount-1 ] ) == (RTF_PKT_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Null packet in picture" );
			break;
		}
		// make the return
		result = rtfPktGetMapPktNumber( hPkt, pPktNum );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the packet count of a picture
RTF_RESULT rtfPicGetPacketCount( RTF_PIC_HANDLE handle, unsigned short *pPacketCount )
{
	RTF_FNAME( "rtfPicGetPacketCount" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the return
		*pPacketCount = pPic->packetCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get packet array info from a picture
RTF_RESULT rtfPicGetPacketArrayInfo( RTF_PIC_HANDLE handle, unsigned short *pPacketCount,
									 RTF_PKT_HANDLE **pphPacket, unsigned char *pFirstBytePacketOffset,
									 unsigned char *pLastBytePacketOffset, unsigned short *pLastVideoPacketIndex )
{
	RTF_FNAME( "rtfPicGetPacketArrayInfo" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make the returns
		*pPacketCount = pPic->packetCount;
		*pphPacket = pPic->hPacket;
		*pFirstBytePacketOffset = pPic->firstBytePacketOffset;
		*pLastBytePacketOffset = pPic->lastBytePacketOffset;
		*pLastVideoPacketIndex = pPic->lastVideoPacketIndex;

	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef DO_VCD_MPEG2

// set the MPEG2 codec-specific info of a picture
RTF_RESULT rtfPicSetMpeg2Info( RTF_PIC_HANDLE handle, unsigned short temporalReference )
{
	RTF_FNAME( "rtfPicSetMpeg2Info" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make sure this is an MPEG2 stream
		if( pPic->vcdType != RTF_VIDEO_CODEC_TYPE_MPEG2 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Codec not MPEG2" );
			break;
		}
		// record the parameter
		pPic->vcdInfo.mpeg2.temporalReference = temporalReference;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the MPEG2 codec-specific info of a picture
RTF_RESULT rtfPicGetMpeg2Info( RTF_PIC_HANDLE handle, unsigned short *pTemporalReference )
{
	RTF_FNAME( "rtfPicGetMpeg2Info" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make sure this is an MPEG2 stream
		if( pPic->vcdType != RTF_VIDEO_CODEC_TYPE_MPEG2 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Codec not MPEG2" );
			break;
		}
		// make the return
		*pTemporalReference = pPic->vcdInfo.mpeg2.temporalReference;

	} while( 0 ); // error escape wrapper - end

	return result;
}

#endif // DO_VCD_MPEG2

#ifdef DO_VCD_H264

// set the H264 codec-specific info of a picture
RTF_RESULT rtfPicSetH264Info( RTF_PIC_HANDLE handle, BOOL isKeyframe,
							  unsigned long paramId, unsigned long frameNum )
{
	RTF_FNAME( "rtfPicSetH264Info" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make sure this is an H264 stream
		if( pPic->vcdType != RTF_VIDEO_CODEC_TYPE_H264 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Codec not H264" );
			break;
		}
		// record the parameters
		pPic->vcdInfo.h264.paramId  = paramId;
		pPic->vcdInfo.h264.frameNum = frameNum;
		if( isKeyframe == FALSE )
		{
			pPic->flags &= ~RTF_PIC_ISKEYFRAME;
		}
		else
		{
			pPic->flags |= RTF_PIC_ISKEYFRAME;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the H264 codec-specific info of a picture
RTF_RESULT rtfPicGetH264Info( RTF_PIC_HANDLE handle, BOOL *pIsKeyframe,
							  unsigned long *pParamId, unsigned long *pFrameNum )
{
	RTF_FNAME( "rtfPicGetH264Info" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// make sure this is an H264 stream
		if( pPic->vcdType != RTF_VIDEO_CODEC_TYPE_H264 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Codec not H264" );
			break;
		}
		// make the returns
		*pIsKeyframe = ( ( pPic->flags & RTF_PIC_ISKEYFRAME ) != 0 ) ? TRUE : FALSE;
		*pParamId  = pPic->vcdInfo.h264.paramId;
		*pFrameNum = pPic->vcdInfo.h264.frameNum;

	} while( 0 ); // error escape wrapper - end

	return result;
}

#endif // DO_VCD_H264

#ifdef DO_VCD_VC1

// !!! FIX ME !!! VC1 VIDEO CODEC !!!

#endif // DO_VCD_VC1

// service methods **********************************************************************

// reset a PIC object
RTF_RESULT rtfPicReset( RTF_PIC_HANDLE handle )
{
	RTF_FNAME( "rtfPicReset" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// reset the picture state structure
		resetPic( pPic, RTF_VIDEO_CODEC_TYPE_INVALID );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// open a new picture (parse the picture header)
RTF_RESULT rtfPicOpen( RTF_PIC_HANDLE handle,  unsigned long picNumber, unsigned long picCount,
					   unsigned short vidPid, RTF_VIDEO_CODEC_TYPE vcdType )
{
	RTF_FNAME( "rtfPicOpen" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_SEQ_HANDLE hSeq;
	RTF_PKT_HANDLE hFirstSeqPkt;
	RTF_PKT_HANDLE hFirstWinPkt;
	INT64 pts;
	INT64 dts;
	unsigned char *pStorage;
	unsigned long number;
	unsigned long flags;
	unsigned short pid;
	unsigned char firstSeqOffset;
	unsigned char firstWinOffset;
	unsigned char payloadOffset;
	unsigned char payloadBytes;
	unsigned char pesHeaderOffset;
	unsigned char pesHeaderBytes;
	unsigned char pesHeaderFlags;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		RTF_CHK_STATE_NE( pPic, RTF_PICSTATE_OPEN );
		RTF_CHK_RANGE( vcdType, RTF_VIDEO_CODEC_TYPE_INVALID, RTF_VIDEO_CODEC_TYPE_LIMIT );
		// reset the picture state structure
		resetPic( pPic, vcdType );
		// record the session picture number
		pPic->picNumber = picNumber;
		// record the video PID
		pPic->vidPid = vidPid;
		// get the position of the picture start code
		result = rtfWinGetFirstByteInfo( pPic->hWin, &hFirstWinPkt, &firstWinOffset );
		RTF_CHK_RESULT;
		// set the picture state to open
		pPic->state = RTF_PICSTATE_OPEN;
		// is this the first picture of the current sequence?
		if( picCount == 1 )
		{
			// yes - get the sequence start position - start the picture there
			result = rtfSesGetSequence( pPic->hSes, &hSeq );
			RTF_CHK_RESULT;
			result = rtfSeqGetStartInfo( hSeq, &hFirstSeqPkt, &firstSeqOffset );
			RTF_CHK_RESULT;
			// does the sequence start in the same packet as the picture?
			if( hFirstWinPkt != hFirstSeqPkt )
			{
				// no - add the first packet of the sequence to the picture
				result = rtfPicAddPacket( handle, hFirstSeqPkt );
				RTF_CHK_RESULT;
			}
			pPic->firstBytePacketOffset = firstSeqOffset;
		}
		else
		{
			// no - the first window byte is the first byte of the picture start code
			pPic->firstBytePacketOffset = firstWinOffset;
		}
		// add the current packet to the picture
		result = rtfPicAddPacket( handle, hFirstWinPkt );
		RTF_CHK_RESULT;
		// record the picture start offset in the first packet
		result = rtfPktSetPicStartOffset( hFirstWinPkt, firstWinOffset );
		RTF_CHK_RESULT;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_PICOPEN, "PIC %d opened", picNumber );
#endif
		// get some info from the first packet of the picture
		result = rtfPktGetInfo( pPic->hPacket[ 0 ], &number, &pid, &flags, &pStorage,
								&payloadOffset, &payloadBytes,
								&pesHeaderOffset, &pesHeaderBytes );
		RTF_CHK_RESULT;
		if( ( flags & RTF_PKT_PESHDRPRESENT ) != 0 )
		{
			// get the second byte of the PES header flags;
			pesHeaderFlags = pStorage[ payloadOffset + 7 ];
			// is a PTS present?
			if( ( pesHeaderFlags & 0x80 ) != 0 )
			{
				// yes - record the value
				pts = ( pStorage[ payloadOffset + 9 ] >> 1 ) & 0x07;
				pts = ( pts << 8 ) | pStorage[ payloadOffset + 10 ];
				pts = ( pts << 7 ) | ( pStorage[ payloadOffset + 11 ] & 0x7F );
				pts = ( pts << 8 ) | pStorage[ payloadOffset + 12 ];
				pts = ( pts << 7 ) | ( pStorage[ payloadOffset + 13 ] & 0x7F );
				pPic->srcPTS = pts;
				// is a DTS also present?
				if( ( pesHeaderFlags & 0x40 ) != 0 )
				{
					dts = ( pStorage[ payloadOffset + 14 ] >> 1 ) & 0x07;
					dts = ( dts << 8 ) | pStorage[ payloadOffset + 15 ];
					dts = ( dts << 7 ) | ( pStorage[ payloadOffset + 16 ] & 0x7F );
					dts = ( dts << 8 ) | pStorage[ payloadOffset + 17 ];
					dts = ( dts << 7 ) | ( pStorage[ payloadOffset + 18 ] & 0x7F );
					pPic->srcDTS = dts;
				}
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// add a packet to a picture
RTF_RESULT rtfPicAddPacket( RTF_PIC_HANDLE handle, RTF_PKT_HANDLE hPacket )
{
	RTF_FNAME( "rtfPicAddPacket" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned long flags, pktNumber;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		RTF_CHK_STATE_EQ( pPic, RTF_PICSTATE_OPEN );
		// under certain rare occasions, a packet may be added to a picture twice in a row
		// this is an error involving using look-ahead windows, but it would be very hard
		// to fix. Fix it here instead by silently ignoring these requests.
		result = rtfPktGetInpPktNumber( hPacket, &pktNumber );
		RTF_CHK_RESULT;
		if( pktNumber == pPic->lastAddedPktNumber )
		{
			break;
		}
		pPic->lastAddedPktNumber = pktNumber;
		// make sure there is room for another packet in this picture
		if( pPic->packetCount >= RTF_MAX_PICTURE_PACKETS )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_PKTOVERFLOW, "Packet array overflow in picture %d", pPic->picNumber );
			pPic->flags |= RTF_PIC_ISDAMAGED;
			break;
		}
		// add a reference to the packet
		result = rtfPktAddReference( hPacket, handle );
		RTF_CHK_RESULT;
		// add the packet to the list of packets included in this picture
		pPic->hPacket[ pPic->packetCount++ ] = hPacket;
		// get the damage flag from the packet
		result = rtfPktGetFlags( hPacket, &flags );
		RTF_CHK_RESULT;
		// is this packet damaged?
		if( ( flags & RTF_PKT_ISDAMAGED ) != 0 )
		{
			// yes - if the packet is damaged, the picture is damaged
			pPic->flags |= RTF_PIC_ISDAMAGED;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// close a picture object
RTF_RESULT rtfPicClose( RTF_PIC_HANDLE handle )
{
	RTF_FNAME( "rtfPicClose" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hPriorWinBytePkt;
	unsigned char payloadOffset, headerOffset, headerBytes; 
	unsigned short pid;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		RTF_CHK_STATE_EQ( pPic, RTF_PICSTATE_OPEN );
		// window currently points at the start code of the next thing that marks the
		// start of a new picture (SEQ hdr, GOP hdr, or PIC hdr). In theory, the current
		// picture should end here. However, it is possible that an extra packet was
		// incorrectly added to the picture because of an intervening PES packet header.
		// The parsing window context does not look at PES headers since they can occur
		// inside start codes, headers, etc. Therefore, the window prior context would
		// point to before the PES header.
		// get the info on the last byte before this start code
		result = rtfWinGetPriorByteInfo( pPic->hWin, &hPriorWinBytePkt, &pPic->lastBytePacketOffset );
		RTF_CHK_RESULT;
		// is this the last packet listed in the picture?
		if( hPriorWinBytePkt != pPic->hPacket[ pPic->packetCount - 1 ] )
		{
			// no - remove the last packet
			result = rtfPicRemoveLastPacket( pPic, pPic->lastBytePacketOffset );
			RTF_CHK_RESULT;
		}
		// see if the last packet of the picture contains a PES header
		result = rtfPktGetPesHdrInfo( pPic->hPacket[ pPic->packetCount - 1 ],
									  &payloadOffset, &headerOffset, &headerBytes );
		RTF_CHK_RESULT;
		// is there a PES header, and if so, is it the first thing in the
		// payload of the last packet, but the last thing in the picture?
		if( ( payloadOffset == headerOffset ) &&
			( ( headerOffset + headerBytes - 1 ) == pPic->lastBytePacketOffset ) )
		{
			// yes - remove the last packet.
			// The new last picture byte is the last payload byte of the prior packet
			result = rtfPicRemoveLastPacket( pPic, TRANSPORT_PACKET_BYTES-1 );
			RTF_CHK_RESULT;
		}
		// scan the packet array to find the last video packet
		for( i=pPic->packetCount-1; i>=0; --i )
		{
			result = rtfPktGetPID( pPic->hPacket[ i ], &pid );
			RTF_CHK_RESULT;
			if( pid == pPic->vidPid )
			{
				break;
			}
		}
		RTF_CHK_RESULT_LOOP;
		pPic->lastVideoPacketIndex = i;
		for( ++i; i<pPic->packetCount; ++i )
		{
			result = rtfPktRemoveReference( pPic->hPacket[ i ], handle );
			RTF_CHK_RESULT;
			pPic->hPacket[ i ] = (RTF_PKT_HANDLE)NULL;
		}
		RTF_CHK_RESULT_LOOP;
		pPic->packetCount = pPic->lastVideoPacketIndex + 1;
		// change state to closed
		pPic->state = RTF_PICSTATE_CLOSED;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_PICCLOSE, "PIC %d closed", pPic->picNumber );
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// release a picture object
RTF_RESULT rtfPicRelease( RTF_PIC_HANDLE handle, RTF_SES_HANDLE hSes )
{
	RTF_FNAME( "rtfPicRelease" );
	RTF_OBASE( handle );
	RTF_PIC *pPic = (RTF_PIC *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pPic, RTF_OBJ_TYPE_PIC );
		// dereference the packets in the picture
		for( i=0; i<pPic->packetCount; ++i )
		{
			result = rtfPktRemoveReference( pPic->hPacket[ i ], handle );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_PICRELEASE, "PIC %d released", pPic->picNumber );
#endif
		// tell the session to release the picture too
		result = rtfSesRecyclePic( hSes, handle );
		RTF_CHK_RESULT;
		// reset the picture
		resetPic( pPic, pPic->vcdType );

	} while( 0 ); // error escape wrapper - end

	return result;
}
