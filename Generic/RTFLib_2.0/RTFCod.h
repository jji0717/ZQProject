// definition file for CODEC class
//

#ifndef _RTF_COD_H
#define _RTF_COD_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfCodGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfCodConstructor( RTF_COD_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfCodDestructor( RTF_COD_HANDLE handle );

// accessor methods *********************************************************************

// return a pointer to a set of packets making up a no change (null) frame
RTF_RESULT rtfCodGetNCPFrame( RTF_COD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pChanged );

// return a pointer to a set of packets making up a no change (null) B frame
RTF_RESULT rtfCodGetNCBFrame( RTF_COD_HANDLE handle, unsigned char **ppFrameBuffer,
							  int *pFrameBufferBytes, BOOL *pChanged );

// return the current frame rate code (MPEG-2 style encoding)
RTF_RESULT rtfCodGetFrameRateCode( RTF_COD_HANDLE handle, unsigned char *pFrameRateCode );

// service methods **********************************************************************

// set up a codec object for use with a particular codec type
RTF_RESULT rtfCodOpen( RTF_COD_HANDLE handle, RTF_VIDEO_CODEC_TYPE codecType, unsigned short vidPid );

// set up a no-change frame (used to regulate bit rate at output)
RTF_RESULT rtfCodSetupNCFrame( RTF_COD_HANDLE handle );

// reset a codec object
RTF_RESULT rtfCodReset( RTF_COD_HANDLE handle );

// process a start code from the bit stream
RTF_RESULT rtfCodProcessStartCode( RTF_COD_HANDLE handle, unsigned long code );

#endif // #ifndef _RTF_COD_H
