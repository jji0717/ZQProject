// definition file for rtfOut class
// abstracts output interface
//

#ifndef _RTF_OUT_H
#define _RTF_OUT_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfOutGetStorageRequirement();

// constructor / destructor *************************************************************

RTF_RESULT rtfOutConstructor( RTF_OUT_HANDLE *pHandle, RTF_HANDLE hParent );

RTF_RESULT rtfOutDestructor( RTF_OUT_HANDLE handle );

// accessor methods *********************************************************************

// get the number of bytes written to the output
RTF_RESULT rtfOutGetByteCount( RTF_OUT_HANDLE handle, INT64 *pByteCount );

// get the current position of the output file
RTF_RESULT rtfOutGetPosition( RTF_OUT_HANDLE handle, INT64 *pPosition );

// get the handle of the output's buffer
RTF_RESULT rtfOutGetBuffer( RTF_OUT_HANDLE handle, RTF_BUF_HANDLE *phBuffer );

// get the number of times "put buffer" was called for this output
RTF_RESULT rtfOutGetPutBufferCount( RTF_OUT_HANDLE handle, unsigned long *pPutBufferCount );

// service methods **********************************************************************

// open the output
RTF_RESULT rtfOutOpen( RTF_OUT_HANDLE handle,
					   RTF_APP_SESSION_HANDLE hAppSession,
					   int outputNumber,
					   RTF_APP_OUTPUT_SETTINGS *pAppOutputSettings );

// close the output
RTF_RESULT rtfOutClose( RTF_OUT_HANDLE handle );

// set the user bit rate for the output
RTF_RESULT rtfOutSetUserBitRate( RTF_OUT_HANDLE handle, unsigned long bitsPerSecond );

// signal abnormal end of output
RTF_RESULT rtfOutAbend( RTF_OUT_HANDLE handle );

// add a packet to the queue at an output interface
RTF_RESULT rtfOutQueuePacket( RTF_OUT_HANDLE handle, RTF_PKT_HANDLE hPacket );

// send some data to the output interface
RTF_RESULT rtfOutQueueData( RTF_OUT_HANDLE handle, unsigned char *pData, unsigned long byteCount );

// write a block of data to the index
int rtfOutWriteIndex( RTF_OUT_HANDLE handle, INT64 offsetFromCurrentPosition,
					  unsigned char *pBuffer, int bufferLength );

// log the application-supplied settings for an output
RTF_RESULT rtfOutLogSettings( RTF_OUT_HANDLE handle );

#ifdef _DEBUG
// log the state of the output object
RTF_RESULT rtfOutLogState( RTF_OUT_HANDLE handle );
#endif

RTF_RESULT rtfOutGetFileBytes( RTF_OUT_HANDLE handle, INT64 *pFileBytes );

#endif // #ifndef _RTF_OUT_H
