// definition file for sequence class
//

#ifndef _RTF_SEQ_H
#define _RTF_SEQ_H 1

// constants ****************************************************************************

// these masks are used to isolate single flags from the "flags" member variable
// group description flags
#define RTF_SEQ_ISVIRTUAL		0x00000001
// group state flags
#define RTF_SEQ_ISDAMAGED		0x80000000

// typedefs *****************************************************************************

typedef enum _RTF_SEQSTATE
{
	RTF_SEQSTATE_INVALID = 0,

	RTF_SEQSTATE_OPEN,
	RTF_SEQSTATE_CLOSED,
	RTF_SEQSTATE_RELEASED

} RTF_SEQSTATE;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfSeqGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfSeqConstructor( RTF_SEQ_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfSeqDestructor( RTF_SEQ_HANDLE handle );

// accessor methods *********************************************************************

// get the number of a sequence object
RTF_RESULT rtfSeqGetNumber( RTF_SEQ_HANDLE handle, unsigned long *pSeqNumber );

// get the state of a sequence object
RTF_RESULT rtfSeqGetState( RTF_SEQ_HANDLE handle, RTF_SEQSTATE *pState );

// get the virtual flag from a sequence object
RTF_RESULT rtfSeqGetIsVirtual( RTF_SEQ_HANDLE handle, BOOL *pIsVirtual );

// set the damage flag in of a sequence object
RTF_RESULT rtfSeqSetIsDamaged( RTF_SEQ_HANDLE handle, BOOL isDamaged );

// get the damage flag from of a sequence object
RTF_RESULT rtfSeqGetIsDamaged( RTF_SEQ_HANDLE handle, BOOL *pIsDamaged );

// get the picture size info from the sequence header
RTF_RESULT rtfSeqGetSizeInfo( RTF_SEQ_HANDLE handle, unsigned short *pHoriz, unsigned short *pVert );

// get the GOP array info from a sequence object
RTF_RESULT rtfSeqGetGopArrayInfo( RTF_SEQ_HANDLE handle, unsigned char *pGopCount, RTF_GOP_HANDLE **pphGop );

// set the starting location info for the sequence object
RTF_RESULT rtfSeqSetStartInfo( RTF_SEQ_HANDLE handle, RTF_PKT_HANDLE hFirstBytePacket,
							   unsigned char firstBytePacketOffset );

// get the offset of the first packet of the sequence in the output stream
RTF_RESULT rtfSeqGetOutputStreamStartOffset( RTF_SEQ_HANDLE handle, INT64 *pOffset );

// get the number of the first packet of this sequence in the output stream
RTF_RESULT rtfSeqGetFirstPktNum( RTF_SEQ_HANDLE handle, unsigned long *pPktNum );

// get the number of the last packet of the sequence in the output stream
RTF_RESULT rtfSeqGetOutputStreamEndPktNum( RTF_SEQ_HANDLE handle, unsigned long *pPktNum );

// get the starting packet and offset of a sequence object
RTF_RESULT rtfSeqGetStartInfo( RTF_SEQ_HANDLE handle, RTF_PKT_HANDLE *phFirstBytePacket,
							   unsigned char *pFirstBytePacketOffset );

// set the video info for a sequence object
RTF_RESULT rtfSeqSetVideoInfo( RTF_SEQ_HANDLE handle, unsigned short horizSize,
							   unsigned short vertSize, unsigned long bitRate );

// get the video info for a sequence object
RTF_RESULT rtfSeqGetVideoInfo( RTF_SEQ_HANDLE handle, unsigned short *pHorizSize,
							   unsigned short *pVertSize, unsigned long *pBitRate );

// get the video bit rate
RTF_RESULT rtfSeqGetVideoBitRate( RTF_SEQ_HANDLE handle, unsigned long *pBitRate );

// service methods **********************************************************************

// reset a SEQ object
RTF_RESULT rtfSeqReset( RTF_SEQ_HANDLE handle );

// open a new video sequence
RTF_RESULT rtfSeqOpen( RTF_SEQ_HANDLE handle, unsigned long seqNumber, BOOL isVirtual );

// add a GOP to a video sequence
RTF_RESULT rtfSeqAddGop( RTF_SEQ_HANDLE handle, RTF_GOP_HANDLE hGop );

// close a video sequence
RTF_RESULT rtfSeqClose( RTF_SEQ_HANDLE handle );

// unmap a video sequence
RTF_RESULT rtfSeqRelease( RTF_SEQ_HANDLE handle, RTF_SES_HANDLE hSes );

#endif // #ifndef _RTF_SEQ_H
