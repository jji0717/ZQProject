// definition file for video codec class
//

#ifndef _RTF_VCD_H
#define _RTF_VCD_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfVcdGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfVcdConstructor( RTF_VCD_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfVcdDestructor( RTF_VCD_HANDLE handle );

// accessor methods *********************************************************************

// return the sequence start point info
RTF_RESULT rtfVcdGetSeqStartInfo( RTF_VCD_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								  unsigned char *pPktOff );

// update the boundary location information
RTF_RESULT rtfVcdUpdateBoundaryInfo( RTF_VCD_HANDLE handle );

// return the picture end point info
RTF_RESULT rtfVcdGetPicEndInfo( RTF_VCD_HANDLE handle, RTF_PKT_HANDLE *phPkt,
								unsigned char *pPktOff );

// return a pointer to a set of packets making up a no change (null) frame
RTF_RESULT rtfVcdGetNCPFrame( RTF_VCD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pRegenerateNCPF );

// return a pointer to a set of packets making up a no change (null) B frame
RTF_RESULT rtfVcdGetNCBFrame( RTF_VCD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pRegenerateNCBF );

// service methods **********************************************************************

// set up a video codec object for use with a particular video stream
RTF_RESULT rtfVcdOpen( RTF_VCD_HANDLE handle, RTF_ESTREAM_SPEC *pVidSpec );

// set up a no-change frame (used to regulate bit rate at output)
RTF_RESULT rtfVcdSetupNCFrame( RTF_VCD_HANDLE handle );

// reset a video codec object
RTF_RESULT rtfVcdReset( RTF_VCD_HANDLE handle );

// update the sequence start location information
RTF_RESULT rtfVcdUpdateSeqStartInfo( RTF_VCD_HANDLE handle );

// process a start code from the bit stream
RTF_RESULT rtfVcdProcessStartCode( RTF_VCD_HANDLE handle, unsigned long code );

#endif // #ifndef _RTF_VCD_H
