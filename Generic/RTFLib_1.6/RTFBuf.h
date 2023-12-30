// definition file for rtfBuf class
// encapsulates input / output buffer
//

#ifndef _RTF_BUF_H
#define _RTF_BUF_H 1

// typedefs *****************************************************************************

typedef enum _RTF_BUFTYPE
{
	RTF_BUFTYPE_INVALID = 0,

	RTF_BUFTYPE_INPUT,
	RTF_BUFTYPE_OUTPUT
} RTF_BUFTYPE;

typedef enum _RTF_BUFSTATE
{
	RTF_BUFSTATE_INVALID = 0,

	RTF_BUFSTATE_RELEASED,
	RTF_BUFSTATE_MAPPED,
	RTF_BUFSTATE_PROCESSED
} RTF_BUFSTATE;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfBufGetStorageRequirement( RTF_BUFTYPE bufType, short maxPacketCount );

// constructor / destructor *************************************************************

// capacity is number of bytes to allocate - 0 for externally managed buffers
RTF_RESULT rtfBufConstructor( RTF_BUF_HANDLE *pHandle, RTF_BUFTYPE type, short maxPacketCount,
							  RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfBufDestructor( RTF_BUF_HANDLE handle );

// accessor methods *********************************************************************

// set the state of the buffer
RTF_RESULT rtfBufSetState( RTF_BUF_HANDLE handle, RTF_BUFSTATE state );

// get the state of the buffer
RTF_RESULT rtfBufGetState( RTF_BUF_HANDLE handle, RTF_BUFSTATE *pState );

// get the buffer number
RTF_RESULT rtfBufGetNumber( RTF_BUF_HANDLE handle, unsigned long *pNumber );

// get the reference count of the buffer
RTF_RESULT rtfBufGetReferenceCount( RTF_BUF_HANDLE handle, unsigned long *pReferenceCount );

#ifdef DO_TRACKREFCOUNTS
// get the handle of an object that references this buffer
RTF_RESULT rtfBufGetRefHandle( RTF_BUF_HANDLE handle, int index, RTF_HANDLE *phRefHolder );
#endif // #ifdef DO_TRACKREFCOUNTS

// get the mapping info from the buffer
RTF_RESULT rtfBufGetMapInfo( RTF_BUF_HANDLE handle, RTF_BUFSTATE *pState,
							 RTF_APP_SESSION_HANDLE *phAppSession,
							 RTF_APP_FILE_HANDLE *phAppFile,
							 RTF_APP_BUFFER_HANDLE *phAppBuffer,
							 unsigned long *pBufferNumber, unsigned char **ppBase,
							 unsigned long *pCapacity, unsigned long *pOccupancy );

// get the number of packets in the buffer
RTF_RESULT rtfBufGetPacketCount( RTF_BUF_HANDLE handle, short *pPacketCount );

// get a packet from the buffer
RTF_RESULT rtfBufGetPacket( RTF_BUF_HANDLE handle, short packetIndex, RTF_PKT_HANDLE *phPacket );

// get the current fill point address from the buffer
RTF_RESULT rtfBufGetFillPointer( RTF_BUF_HANDLE handle, unsigned char **ppFill );

// get the current drain point address from the buffer
RTF_RESULT rtfBufGetDrainPointer( RTF_BUF_HANDLE handle, unsigned char **ppDrain );

// get packet array info from the buffer
RTF_RESULT rtfBufGetPacketArrayInfo( RTF_BUF_HANDLE handle, unsigned short *pPacketCount, RTF_PKT_HANDLE **pphPacket );

// service methods **********************************************************************

// reset a buffer
RTF_RESULT rtfBufReset( RTF_BUF_HANDLE handle, BOOL isClose );

// map a buffer to a block of storage
RTF_RESULT rtfBufMap( RTF_BUF_HANDLE handle,
					  RTF_APP_SESSION_HANDLE hAppSession,
					  RTF_APP_FILE_HANDLE hAppFile,
					  RTF_APP_BUFFER_HANDLE hAppBuffer,
					  unsigned long bufferNumber, unsigned char *pBase,
					  unsigned long capacity, unsigned long occupancy );

// map the packets in an input buffer
RTF_RESULT rtfBufMapInputPackets( RTF_BUF_HANDLE handle, BOOL *pInSync,
								  BOOL *pInputIsTTS, BOOL *pFirstPcrAcquired,
								  unsigned char *pFragment, unsigned char *pFragmentBytes,
								  unsigned long *pTotalPacketCount, unsigned long *pTotalMappedPacketCount );

// add some data to an output buffer
RTF_RESULT rtfBufQueueOutputData( RTF_BUF_HANDLE handle, unsigned char *pData, unsigned long byteCount, BOOL *pOverflow );

// a picture has added a reference to a packet in this buffer
RTF_RESULT rtfBufAddReference( RTF_BUF_HANDLE handle, RTF_HANDLE hRefHolder );

// a picture has removed a reference to a packet in this buffer
RTF_RESULT rtfBufRemoveReference( RTF_BUF_HANDLE handle, RTF_HANDLE hRefHolder );
 
// scan the packets of the buffer for PSI tables
RTF_RESULT rtfBufCapturePsi( RTF_BUF_HANDLE handle, RTF_PAT_HANDLE hPat, RTF_CAT_HANDLE hCat,
							 RTF_PMT_HANDLE hPmt, BOOL *pPsiCaptured );

// scan the packets of the buffer to see if any PSI tables change state
RTF_RESULT rtfBufCheckPsi( RTF_BUF_HANDLE handle, RTF_PAT_HANDLE hPat, RTF_CAT_HANDLE hCat,
						   RTF_PMT_HANDLE hPmt, BOOL *pPsiCaptured );

// scan the packets of the buffer for input bit rate info
RTF_RESULT rtfBufCaptureBpsInfo( RTF_BUF_HANDLE handle, unsigned char maxPcrCount,
								 unsigned char *pPcrCount, unsigned long packetNumber[],
								 RTF_TIMESTAMP timestamp[] );

// scan the packets of the buffer for video frame rate info
RTF_RESULT rtfBufCaptureFpsInfo( RTF_BUF_HANDLE handle, unsigned char maxFpsPicCount, unsigned char *pFpsPicCount,
								 RTF_TIMESTAMP *pFpsFirstPicTime, RTF_TIMESTAMP *pFpsLastPicTime );

#ifdef _DEBUG
// debug helper function - print state of buffer object
RTF_RESULT rtfBufferLogState( RTF_BUF_HANDLE handle );
#endif

#endif // #ifndef _RTF_BUF_H
