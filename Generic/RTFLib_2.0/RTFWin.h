// definition file for rtfBuf class
// encapsulates input / output buffer
//

#ifndef _RTF_WIN_H
#define _RTF_WIN_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfWinGetStorageRequirement();

// constructor / destructor *************************************************************

// capacity is number of bytes to allocate - 0 for externally managed buffers
RTF_RESULT rtfWinConstructor( RTF_BUF_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfWinDestructor( RTF_WIN_HANDLE handle );

// accessor methods *********************************************************************

// get the current window data
RTF_RESULT rtfWinGetData( RTF_WIN_HANDLE handle, unsigned long *pData );

// get the number of the oldest buffer referenced by the window
RTF_RESULT rtfWinGetOldestBufferNumber( RTF_WIN_HANDLE handle, unsigned long *pBufferNumber );

// get the last packet warning flag
RTF_RESULT rtfWinGetLastPacketWarningFlag( RTF_WIN_HANDLE handle, BOOL *pFlag );

// get the position of the first window byte in the window packet set
RTF_RESULT rtfWinGetFirstByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPacket,
								   unsigned char *pPacketOffset );

// get the position of the last window byte in the window packet set
RTF_RESULT rtfWinGetLastByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPacket,
								  unsigned char *pPacketOffset );

// get the position of the next window byte in the window packet set
RTF_RESULT rtfWinGetNextByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPacket,
								  unsigned char *pPacketOffset );

// get the position of the prior window byte in the window packet set
RTF_RESULT rtfWinGetPriorByteInfo( RTF_WIN_HANDLE handle, RTF_PKT_HANDLE *phPacket,
								   unsigned char *pPacketOffset );

// get the input stream byte offset of the first byte in the window
RTF_RESULT rtfWinGetInputByteCount( RTF_WIN_HANDLE handle, INT64 *pInputByteCount );

// set the optimization override flag (used to force full parsing at the start of a keyframe)
RTF_RESULT rtfWinSetOptimizationOverride( RTF_WIN_HANDLE handle, BOOL value );

// service methods **********************************************************************

// reset the window object
RTF_RESULT rtfWinReset( RTF_WIN_HANDLE handle, RTF_SES_HANDLE hSes,
					    INT64 initialInputByteCount, BOOL optimizeForATSC );

// initialize a window to scan the packets of a buffer (carries over prior context)
RTF_RESULT rtfWinMapBuffer( RTF_WIN_HANDLE handle, unsigned short pid, RTF_BUF_HANDLE hBuf, BOOL *pFlag );

// advance the window to the next start code, if there is one
RTF_RESULT rtfWinFindNextStartCode( RTF_WIN_HANDLE handle, unsigned long *pData );

// advance the window N bytes forward
RTF_RESULT rtfWinAdvance( RTF_WIN_HANDLE handle, int bytes, unsigned long *pData );

// advance the window N bytes forward (N < 5) (optimization)
RTF_RESULT rtfWinSmallAdvance( RTF_WIN_HANDLE handle, int bytes, unsigned long *pData );

// begin recording all data that appears in the window in the buffer provided
RTF_RESULT rtfWinStartRecord( RTF_WIN_HANDLE handle, unsigned char *pBuf, int bufBytes );

// stop recording window data - return the number of bytes recorded
RTF_RESULT rtfWinStopRecord( RTF_WIN_HANDLE handle, int *pBytes );

// get the interpolated PCR time in 90 KHz clock ticks
RTF_RESULT rtfWinGetPcrTime( RTF_WIN_HANDLE handle, INT64 *pPcrTime );

// copy one window's state information into another
// (used to "bookmark" locations in CODEC object)
RTF_RESULT rtfWinCopyState( RTF_WIN_HANDLE hSource, RTF_WIN_HANDLE hDestination );

#endif // #ifndef _RTF_WIN_H
