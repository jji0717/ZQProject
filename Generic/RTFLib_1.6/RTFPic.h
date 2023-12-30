// definition file for picture class
//

#ifndef _RTF_PIC_H
#define _RTF_PIC_H 1

// constants ****************************************************************************

// these masks are used to isolate single flags from the "flags" member variable
// picture description flags
#define RTF_PIC_ISKEYFRAME		0x00000001
// picture state flags
#define RTF_PIC_ISDAMAGED		0x80000000

// typedefs *****************************************************************************

typedef enum _RTF_PICSTATE
{
	RTF_PICSTATE_INVALID = 0,

	RTF_PICSTATE_OPEN,
	RTF_PICSTATE_CLOSED,
	RTF_PICSTATE_RELEASED

} RTF_PICSTATE;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfPicGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfPicConstructor( RTF_PIC_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfPicDestructor( RTF_PIC_HANDLE handle );

// accessor methods *********************************************************************

// get the session's picture number from a picture object
RTF_RESULT rtfPicGetNumber( RTF_PIC_HANDLE handle, unsigned long *pPicNumber );

// get the state of a picture object
RTF_RESULT rtfPicGetState( RTF_PIC_HANDLE handle, RTF_PICSTATE *pState );

// set the end-of-picture location
RTF_RESULT rtfPicSetEndInfo( RTF_PIC_HANDLE handle, RTF_PKT_HANDLE hPkt, unsigned char off );

// set this picture's keyframe flag
RTF_RESULT rtfPicSetIsKeyframe( RTF_PIC_HANDLE handle, BOOL isKeyframe );

// get this picture's keyframe flag
RTF_RESULT rtfPicGetIsKeyframe( RTF_PIC_HANDLE handle, BOOL *pIsKeyframe );

// set this picture's damage flag
RTF_RESULT rtfPicSetIsDamaged( RTF_PIC_HANDLE handle, BOOL isDamaged );

// get this picture's damage flag
RTF_RESULT rtfPicGetIsDamaged( RTF_PIC_HANDLE handle, BOOL *pIsDamaged );

// get this picture's original presentation time stamp (if present)
RTF_RESULT rtfPicGetSrcPTS( RTF_PIC_HANDLE handle, INT64 *pSrcPTS );

// get this picture's original decoding time stamp (if present)
RTF_RESULT rtfPicGetSrcDTS( RTF_PIC_HANDLE handle, INT64 *pSrcDTS );

// get the number of the first packet in a picture
RTF_RESULT rtfPicGetFirstPktNum( RTF_PIC_HANDLE handle, unsigned long *pPktNum );

// get the mapped number of the first packet in a picture
RTF_RESULT rtfPicGetFirstPktMapNum( RTF_PIC_HANDLE handle, unsigned long *pPktNum );

// get the number of the last packet in a picture
RTF_RESULT rtfPicGetLastPktNum( RTF_PIC_HANDLE handle, unsigned long *pPktNum );

// get the packet count of a picture
RTF_RESULT rtfPicGetPacketCount( RTF_PIC_HANDLE handle, unsigned short *pPacketCount );

// get packet array info from a picture
RTF_RESULT rtfPicGetPacketArrayInfo( RTF_PIC_HANDLE handle, unsigned short *pPacketCount,
									 RTF_PKT_HANDLE **pphPacket, unsigned char *pFirstBytePacketOffset,
									 unsigned char *pLastBytePacketOffset, unsigned short *pLastVideoPacketIndex );

#ifdef DO_VCD_MPEG2
// set the MPEG2 codec-specific info of a picture
RTF_RESULT rtfPicSetMpeg2Info( RTF_PIC_HANDLE handle, unsigned short temporalReference );

// get the MPEG2 codec-specific info of a picture
RTF_RESULT rtfPicGetMpeg2Info( RTF_PIC_HANDLE handle, unsigned short *pTemporalReference );
#endif

#ifdef DO_VCD_H264
// set the H264 codec-specific info of a picture
RTF_RESULT rtfPicSetH264Info( RTF_PIC_HANDLE handle, BOOL isKeyframe,
							  unsigned long paramId, unsigned long frameNum );

// get the H264 codec-specific info of a picture
RTF_RESULT rtfPicGetH264Info( RTF_PIC_HANDLE handle, BOOL *pIKeyframe,
							  unsigned long *pParamId, unsigned long *pFrameNum );
#endif

#ifdef DO_VCD_VC1

// !!! FIX ME !!! VC1 CODEC !!!

#endif

// service methods **********************************************************************

// reset a PIC object
RTF_RESULT rtfPicReset( RTF_PIC_HANDLE handle );

// open a new picture
RTF_RESULT rtfPicOpen( RTF_PIC_HANDLE handle,  unsigned long picNumber, unsigned long picCount,
					   unsigned short vidPid, RTF_VIDEO_CODEC_TYPE vcdType );

// add a packet to a picture
RTF_RESULT rtfPicAddPacket( RTF_PIC_HANDLE handle, RTF_PKT_HANDLE hPacket );

// close a picture object
RTF_RESULT rtfPicClose( RTF_PIC_HANDLE handle );

// release a picture object
RTF_RESULT rtfPicRelease( RTF_PIC_HANDLE handle, RTF_SES_HANDLE hSes );

#endif // #ifndef _RTF_PIC_H
