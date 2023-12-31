// implementation file for rtfOut class
// abstracts output interface
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

typedef enum _RTF_OUT_STATE
{
	RTF_OUT_STATE_INVALID = 0,

	RTF_OUT_STATE_CLOSED,
	RTF_OUT_STATE_OPEN,
	RTF_OUT_STATE_ABEND

} RTF_OUT_STATE;

typedef struct _RTF_OUT
{
	RTF_OBJ_HANDLE hBaseObject;

	int outputNumber;

	// the application's handle for the owning session
	RTF_APP_SESSION_HANDLE hAppSession;

	// the owning session
	RTF_SES_HANDLE hSession;

	// the state of the output interface
	RTF_OUT_STATE state;

	// application-supplied output interface info
	RTF_APP_OUTPUT_SETTINGS settings;

	// output buffer object
	RTF_BUF_HANDLE hBuf;

	// total "put buffer" calls generated by this output
	unsigned long putBufferCount;

	// number of bytes in an output packet
	int outPktBytes;

	// total # of bytes processed by output
	INT64 byteCount;

	// current file position
	INT64 position;

	// output file size
	INT64 fileBytes;

} RTF_OUT;

// private methods **********************************************************************

// request a buffer from the output interface
static RTF_RESULT rtfOutReqBuffer( RTF_OUT *pOut )
{
	RTF_FNAME( "rtfOutReqBuffer" );
	RTF_OBASE( pOut );
	RTF_RESULT result = RTF_PASS;
	RTF_BUFSTATE state;
	unsigned char *pBase;
	unsigned long capacity;
	RTF_APP_BUFFER_HANDLE hAppBuffer;

	do {		 // error escape wrapper - begin

		// make sure that the current buffer is not already mapped
		result = rtfBufGetState( pOut->hBuf, &state );
		RTF_CHK_RESULT;
		if( state != RTF_BUFSTATE_RELEASED )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Buffer not released" );
			break;
		}
		// request a buffer from the output interface
		if( (*pOut->settings.pBufferGetFunction)( pOut->hAppSession,
					pOut->settings.hAppOutFile, &hAppBuffer, &pBase, &capacity ) < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_CALLBACKFAILED, "Application buffer get callback failed" );
			break;
		}
		// map the buffer object to the provided storage
		result = rtfBufMap( pOut->hBuf, pOut->hAppSession, pOut->settings.hAppOutFile,
							hAppBuffer, -(pOut->outputNumber+1), pBase, capacity, 0 );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// release the buffer to the output interface
static RTF_RESULT rtfOutPutBuffer( RTF_OUT *pOut, RTF_BUFSEEK bufSeek, INT64 offSeek )
{
	RTF_FNAME( "rtfOutPutBuffer" );
	RTF_OBASE( pOut );
	RTF_RESULT result = RTF_PASS;
	unsigned char *pBase;
	unsigned long capacity;
	unsigned long occupancy;
	unsigned long number;
	RTF_APP_SESSION_HANDLE hAppSession;
	RTF_APP_FILE_HANDLE hAppFile;
	RTF_APP_BUFFER_HANDLE hAppBuffer;
	RTF_BUFSTATE state;

	do {		 // error escape wrapper - begin

		// get the buffer mapping info
		result = rtfBufGetMapInfo( pOut->hBuf, &state, &hAppSession, &hAppFile,
								   &hAppBuffer, &number, &pBase, &capacity, &occupancy );
		RTF_CHK_RESULT;
		if( state != RTF_BUFSTATE_MAPPED )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Buffer not mapped" );
			break;
		}
		// if this is an abnormal end of output, set the occupancy to zero
		occupancy = ( pOut->state == RTF_OUT_STATE_ABEND ) ? 0 : occupancy;
		// send the contents of the buffer to the output interface
		if( (*pOut->settings.pBufferPutFunction)( pOut->hAppSession,
								pOut->settings.hAppOutFile, hAppBuffer,
								pBase, occupancy, bufSeek, offSeek ) < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_CALLBACKFAILED, "Application buffer put callback failed" );
			break;
		}
		// adjust the file position
		switch( bufSeek )
		{
		case RTF_BUFSEEK_NONE:
			pOut->position += occupancy;
			break;
		case RTF_BUFSEEK_SET:
			pOut->position = offSeek + occupancy;
			break;
		case RTF_BUFSEEK_CUR:
			pOut->position += offSeek + occupancy;
			break;
		case RTF_BUFSEEK_END:
			pOut->position = pOut->fileBytes + offSeek + occupancy;
			break;
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_INTERNAL, "Unrecognized seek option %d", bufSeek );
			break;
		}
		RTF_CHK_RESULT_LOOP;
		// adjust the file size
		pOut->fileBytes = ( pOut->position > pOut->fileBytes ) ? pOut->position : pOut->fileBytes;
		// reset the output buffer (this removes references to all packets and unmaps it)
		result = rtfBufReset( pOut->hBuf, FALSE );
		RTF_CHK_RESULT;
		// bump the "put buffer" count
		++pOut->putBufferCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// exchange a buffer with the output interface
static RTF_RESULT rtfOutSwapBuffer( RTF_OUT *pOut )
{
	RTF_FNAME( "rtfOutSwapBuffer" );
	RTF_OBASE( pOut );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		result = rtfOutPutBuffer( pOut, RTF_BUFSEEK_NONE, (INT64)0 );
		RTF_CHK_RESULT;
		result = rtfOutReqBuffer( pOut );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfOutGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfOutGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_OUT);
	bytes += rtfObjGetStorageRequirement();
	bytes += rtfBufGetStorageRequirement( RTF_BUFTYPE_OUTPUT, 0 );

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfOutConstructor( RTF_OUT_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfOutConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_OUT *pOut;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the output object
		pOut = (RTF_OUT *)rtfAlloc( sizeof(RTF_OUT) );
		RTF_CHK_ALLOC( pOut );
		// return the handle
		*pHandle = (RTF_OUT_HANDLE)pOut;
		// clear the state structure
		memset( (void *)pOut, 0, sizeof(*pOut) );
		// create an embedded base object
		result = rtfObjConstructor( RTF_OBJ_TYPE_OUT, (RTF_HANDLE)pOut, hParent, &pOut->hBaseObject );
		RTF_CHK_RESULT;
		// create an embedded buffer object (note: output buffers don't use packet objects!)
		result = rtfBufConstructor( &pOut->hBuf, RTF_BUFTYPE_OUTPUT, 0, (RTF_HANDLE)pOut );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfOutDestructor( RTF_OUT_HANDLE handle )
{
	RTF_FNAME( "rtfOutDestructor" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// destroy the embedded buffer object
		result = rtfBufDestructor( pOut->hBuf );
		RTF_CHK_RESULT;
		// destroy the embedded base object
		result = rtfObjDestructor( pOut->hBaseObject, RTF_OBJ_TYPE_OUT );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the output byte count
RTF_RESULT rtfOutGetByteCount( RTF_OUT_HANDLE handle, INT64 *pByteCount )
{
	RTF_FNAME( "rtfOutGetByteCount" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// make the return
		*pByteCount = pOut->byteCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the current position of the output file
RTF_RESULT rtfOutGetPosition( RTF_OUT_HANDLE handle, INT64 *pPosition )
{
	RTF_FNAME( "rtfOutGetPosition" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// make the return
		*pPosition = pOut->position;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the current output file size in bytes
RTF_RESULT rtfOutGetFileBytes( RTF_OUT_HANDLE handle, INT64 *pFileBytes )
{
	RTF_FNAME( "rtfOutGetFileBytes" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// make the return
		*pFileBytes = pOut->fileBytes;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the handle of the output's buffer
RTF_RESULT rtfOutGetBuffer( RTF_OUT_HANDLE handle, RTF_BUF_HANDLE *phBuffer )
{
	RTF_FNAME( "rtfOutGetBuffer" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// make the return
		*phBuffer = pOut->hBuf;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of times "put buffer" was called for this output
RTF_RESULT rtfOutGetPutBufferCount( RTF_OUT_HANDLE handle, unsigned long *pPutBufferCount )
{
	RTF_FNAME( "rtfOutGetPutBufferCount" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// make the return
		*pPutBufferCount = pOut->putBufferCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// open the output object
RTF_RESULT rtfOutOpen( RTF_OUT_HANDLE handle,
					   RTF_APP_SESSION_HANDLE hAppSession,
					   int outputNumber,
					   RTF_APP_OUTPUT_SETTINGS *pAppOutSettings )
{
	RTF_FNAME( "rtfOutOpen" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// make sure the buffer get function pointer is not null
		if( pAppOutSettings->pBufferGetFunction == (RTF_BUFGET_FUNCTION)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Buffer get callback is null" );
			break;
		}
		// make sure the buffer put function pointer is not null
		if( pAppOutSettings->pBufferPutFunction == (RTF_BUFPUT_FUNCTION)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Buffer put callback is null" );
			break;
		}
		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// record the application's session handle
		pOut->hAppSession = hAppSession;
		// record the output number
		pOut->outputNumber = outputNumber;
		// record the application-supplied settings
		memcpy( (void *)&(pOut->settings), (void *)pAppOutSettings, sizeof(RTF_APP_OUTPUT_SETTINGS) );
		// reset the byte and packet counters, the position, and the file size
		pOut->putBufferCount = 0;
		pOut->byteCount = 0;
		pOut->position = 0;
		pOut->fileBytes = 0;
		// record the output packet size
		pOut->outPktBytes = ( pAppOutSettings->trickSpec.generateTTS == 0 ) ?
							TRANSPORT_PACKET_BYTES : TTS_PACKET_BYTES;
		// reset the captive buffer object
		result = rtfBufReset( pOut->hBuf, FALSE );
		// set state to open
		pOut->state = RTF_OUT_STATE_OPEN;
		// get the first output buffer from the application and map it
		result = rtfOutReqBuffer( pOut );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// close the output
RTF_RESULT rtfOutClose( RTF_OUT_HANDLE handle )
{
	RTF_FNAME( "rtfOutClose" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_BUFSTATE state;
	unsigned char *pBase;
	unsigned long capacity;
	unsigned long occupancy;
	unsigned long number;
	RTF_APP_SESSION_HANDLE hAppSession;
	RTF_APP_FILE_HANDLE hAppFile;
	RTF_APP_BUFFER_HANDLE hAppBuffer;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// is the buffer mapped? (note: performs CHK_OBJ)
		result = rtfBufGetMapInfo( pOut->hBuf, &state, &hAppSession, &hAppFile,
						&hAppBuffer, &number, &pBase, &capacity, &occupancy );
		RTF_CHK_RESULT;
		if( state == RTF_BUFSTATE_MAPPED )
		{
			// yes - send it to the output, and don't ask for a replacement
			result = rtfOutPutBuffer( pOut, RTF_BUFSEEK_NONE, (INT64)0 );
			RTF_CHK_RESULT;
		}
		// reset the captive buffer
		result = rtfBufReset( pOut->hBuf, TRUE );
		// did the user provide a close function?
		if( pOut->settings.pOutputCloseFunction != (RTF_OUTCLOSE_FUNCTION)NULL )
		{
			// yes - call the application output close function
			if( (*pOut->settings.pOutputCloseFunction)( pOut->hAppSession, pOut->settings.hAppOutFile ) < 0 )
			{
				// don't stop on failure - just report it.
				RTF_LOG_INFO0( RTF_MSG_ERR_CALLBACKFAILED, "Application output close callback failed" );
			}
		}
		// set state to closed
		pOut->state = RTF_OUT_STATE_CLOSED;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the user bit rate for the output
// note: only called by the augmentation pre-scan.
// never call this during trickfile generation!
RTF_RESULT rtfOutSetUserBitRate( RTF_OUT_HANDLE handle, unsigned long bitsPerSecond )
{
	RTF_FNAME( "rtfOutSetUserBitRate" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// record the user bit rate
		pOut->settings.trickSpec.userBitRate = TRUE;
		pOut->settings.trickSpec.userBitsPerSecond = bitsPerSecond;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// signal abnormal end of output
RTF_RESULT rtfOutAbend( RTF_OUT_HANDLE handle )
{
	RTF_FNAME( "rtfOutAbend" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		// set state to ABEND
		pOut->state = RTF_OUT_STATE_ABEND;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// send some data to the output interface
RTF_RESULT rtfOutQueueData( RTF_OUT_HANDLE handle, unsigned char *pData, unsigned long byteCount )
{
	RTF_FNAME( "rtfOutQueueData" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;
	BOOL overflow;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		RTF_CHK_STATE_NE( pOut, RTF_OUT_STATE_CLOSED );
		// attempt to put the data in the current output buffer
		result = rtfBufQueueOutputData( pOut->hBuf, pData, byteCount, &overflow );
		RTF_CHK_RESULT;
		// did it overflow?
		if( overflow != FALSE )
		{
			// yes - swap the full buffer for an empty one
			// note: this decrements the reference count of all the packets being sent
			result = rtfOutSwapBuffer( pOut );
			RTF_CHK_RESULT;
			// attempt to add this data to the new output buffer
			result = rtfBufQueueOutputData( pOut->hBuf, pData, byteCount, &overflow );
			RTF_CHK_RESULT;
			// did it overflow again?
			if( overflow != FALSE )
			{
				// give up
				RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Empty output buffer overflowed" );
				break;
			}
		}
		// adjust the byte counter
		pOut->byteCount += byteCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// send a packet to the output interface
RTF_RESULT rtfOutQueuePacket( RTF_OUT_HANDLE handle, RTF_PKT_HANDLE hPacket )
{
	RTF_FNAME( "rtfOutQueuePacket" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned char *pStorage;

	do {		 // error escape wrapper - begin

		// get the packet info
		result = rtfPktGetStorage( hPacket, &pStorage );
		RTF_CHK_RESULT;
		// queue the packet data (note: does CHK_OBJ, CHK_STATE)
		result = rtfOutQueueData( handle, pStorage, TRANSPORT_PACKET_BYTES );
		RTF_CHK_RESULT;
		// is this the main file copy output?
		if( ( pOut->settings.trickSpec.speedNumerator   == 1 ) &&
			( pOut->settings.trickSpec.speedDenominator == 1 ) )
		{
			// yes - the output packet number is the byte count over the packet size
			result = rtfPktSetOutPktNumber( hPacket, (unsigned long)( pOut->byteCount / pOut->outPktBytes ) );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// write a block of data to the index
// note: this is an indexer callback, so the return value is atypical
int rtfOutWriteIndex( RTF_OUT_HANDLE handle, INT64 offsetFromCurrentPosition,
					  unsigned char *pBuffer, int bufferLength )
{
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_FNAME( "rtfIndexWrite" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;

	do {

		// queue the index data (note: does CHK_OBJ, CHK_STATE)
		// note: guaranteed to fit, since the buffer is always
		// swapped for an empty one in this call
		result = rtfOutQueueData( handle, pBuffer, bufferLength );
		RTF_CHK_RESULT;
		result = rtfOutPutBuffer( pOut, RTF_BUFSEEK_CUR, (INT64)offsetFromCurrentPosition );
		RTF_CHK_RESULT;
		result = rtfOutReqBuffer( pOut );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return ( result == RTF_PASS ) ? bufferLength : 0;
}

// log the application-supplied settings for an output
RTF_RESULT rtfOutLogSettings( RTF_OUT_HANDLE handle )
{
	RTF_FNAME( "rtfOutLogState" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_APP_OUTPUT_SETTINGS *pSettings;
	RTF_TRICK_SPEC *pSpec;
	char *pBoolStr[ 2 ] = { "FALSE", "TRUE" };

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		if( pOut->state != RTF_OUT_STATE_OPEN )
		{
			RTF_LOG_INFO0( RTF_MSG_INF_LOGSTATECLOSED, "Output not open" );
			break;
		}
		pSettings = &pOut->settings;
		pSpec = &pSettings->trickSpec;
		RTF_LOG_INFO4( RTF_MSG_INF_STATS, "  appOutHandle=0x%x pBufGetFunc=0x%x pBufPutFunc=0x%x pOutCloseFunc=0x%x",
			pSettings->hAppOutFile, pSettings->pBufferGetFunction, pSettings->pBufferPutFunction, pSettings->pOutputCloseFunction );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "  speedNumerator=%d speedDenominator=%d fileExtension=%s",
			pSpec->speedNumerator, pSpec->speedDenominator, pSpec->fileExtension );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "  Insert: PES=%s PSI=%s PCR=%s",
			pBoolStr[pSpec->insertPES], pBoolStr[pSpec->insertPSI], pBoolStr[pSpec->insertPCR] ); 
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "  Replace: PES=%s PAT=%s PMT=%s",
			pBoolStr[pSpec->replacePES], pBoolStr[pSpec->replacePAT], pBoolStr[pSpec->replacePMT] ); 
		RTF_LOG_INFO2( RTF_MSG_INF_STATS, "  constantBitRate=%s userBitRate=%s",
			pBoolStr[pSpec->constantBitRate], pBoolStr[pSpec->userBitRate] );
		RTF_LOG_INFO2( RTF_MSG_INF_STATS, "  suppressInputPesHdr=%s zeroPesPktLen=%s",
			pBoolStr[pSpec->suppressInputPesHdr], pBoolStr[pSpec->zeroPesPktLen] );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "  clearDAI=%s suppressPTSDTS=%s restampPTSDTS=%s",
			pBoolStr[pSpec->clearDAI], pBoolStr[pSpec->suppressPTSDTS], pBoolStr[pSpec->restampPTSDTS] );
		RTF_LOG_INFO2( RTF_MSG_INF_STATS, "  suppressInputPCR=%s sequentialCC=%s",
			pBoolStr[pSpec->suppressInputPCR], pBoolStr[pSpec->sequentialCC] );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "  restampPCR=%s clearDCON=%s remapPIDs=%s",
			pBoolStr[pSpec->restampPCR], pBoolStr[pSpec->clearDCON], pBoolStr[pSpec->remapPIDs] );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "  userBitsPerSecond=%d PCRsPerSecond=%d pcrPID=%d",
			pSpec->userBitsPerSecond, pSpec->PCRsPerSecond, pSpec->pcrPID );

	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef _DEBUG
char *rtfOutputStateString[] =
{	"invalid", "closed", "open", "abend", "unrecognized" };
// log the state of the output object
RTF_RESULT rtfOutLogState( RTF_OUT_HANDLE handle )
{
	RTF_FNAME( "rtfOutLogState" );
	RTF_OBASE( handle );
	RTF_OUT *pOut = (RTF_OUT *)handle;
	RTF_RESULT result = RTF_PASS;
	int state;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pOut, RTF_OBJ_TYPE_OUT );
		state = ( pOut->state > RTF_OUT_STATE_ABEND ) ? RTF_OUT_STATE_ABEND+1 : pOut->state;
		RTF_LOG_INFO4( RTF_MSG_INF_STATS, ">> Output %d: state=%s bytes=%d putBuffers=%d",
					   (int)pOut->outputNumber, rtfOutputStateString[ state ],
					   (int)pOut->byteCount, (int)pOut->putBufferCount );
	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif
