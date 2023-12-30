// implementation file for rtfSys class
// acts as system service provider
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

// class state structure
typedef struct _RTF_SYS
{
	RTF_OBJ_HANDLE hBaseObject;
	int sessionFailCount;
	int sessionFailThreshold;
	RTF_APP_HEAP_HANDLE hAppHeap;
	RTF_LIBRARY_ERROR_NOTIFIER notifier;
	RTF_MSG_PRIO msgPrioFilter;
	RTF_STREAM_PROFILE profile;
	// session object pool
	unsigned long maxSessionCount;
	RTF_SES_HANDLE hSessionPool[ RTF_ABSMAX_SESCOUNT ];

} RTF_SYS;

// private static variables *************************************************************

static RTF_SYS_HANDLE theSystemHandle		= (RTF_SYS_HANDLE)NULL;
static RTF_APP_HEAP_HANDLE rtfHeapHandle	= (RTF_APP_HEAP_HANDLE)NULL;
static RTF_ALLOC_FUNCTION rtfAllocFunc		= (RTF_ALLOC_FUNCTION)NULL;
static RTF_FREE_FUNCTION rtfFreeFunc		= (RTF_FREE_FUNCTION)NULL;
static RTF_LOG_FUNCTION rtfLogFunc			= (RTF_LOG_FUNCTION)NULL;
static unsigned long rtfTotalAlloc			= 0;
static int rtfZombieThreshold				= 0;
static char rtfProgramVersionString[ 64 ];

// local functions **********************************************************************

static void rtfSysResetProfile( RTF_SYS *pSys )
{
	int i;

	// clear the structure
	memset( (void *)&pSys->profile, 0, sizeof(pSys->profile) );
	// set up all PIDs as invalid
	pSys->profile.pcrPID = TRANSPORT_INVALID_PID;
	pSys->profile.pmtPID = TRANSPORT_INVALID_PID;
	pSys->profile.videoSpec.pid = TRANSPORT_INVALID_PID;
	// set up the default VBR threshold (less than 1%)
	pSys->profile.log2VbrThreshold = 7;
	for( i=0; i<RTF_MAX_AUDIO_PIDS; ++i )
	{
		pSys->profile.audioSpec[ i ].pid = TRANSPORT_INVALID_PID;
	}
	for( i=0; i<RTF_MAX_DATA_PIDS; ++i )
	{
		pSys->profile.dataSpec[ i ].pid = TRANSPORT_INVALID_PID;
	}
}

// some essential utilities *************************************************************

EXPORT unsigned long rtfGetTotalAlloc()
{
	return rtfTotalAlloc;
}

// dynamically allocate some storage
EXPORT void *rtfAlloc( int bytes )
{
	rtfTotalAlloc += bytes;
	if( rtfAllocFunc != (RTF_ALLOC_FUNCTION)NULL )
	{
		return (*rtfAllocFunc)( rtfHeapHandle, bytes );
	}
#ifdef __KERNEL__
	return kmalloc( bytes, GFP_KERNEL );
#else
	return malloc( bytes );
#endif
}

// free some dynamically allocated storage
EXPORT void rtfFree( void *ptr )
{
	if( rtfFreeFunc != (RTF_FREE_FUNCTION)NULL )
	{
		(*rtfFreeFunc)( rtfHeapHandle, ptr );
		return;
	}
#ifdef __KERNEL__
	kfree( ptr );
#else
	free( ptr );
#endif
}

// log a message
EXPORT void rtfLog( void *hAppHandle, char *pShortString, char *pLongString )
{
	if( rtfLogFunc != (RTF_LOG_FUNCTION)NULL )
	{
		(*rtfLogFunc)( hAppHandle, pShortString, pLongString );
		return;
	}
	PRINTF( "%s | %s\n", pShortString, pLongString );
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
// return -1 on error
unsigned long rtfSysGetStorageRequirement( unsigned long maxSessionCount,
					unsigned long maxInputBuffersPerSession, unsigned long maxGroupsPerSequence,
					unsigned long maxPicturesPerGroup, unsigned long maxInputBufferBytes )
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfSysGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_SYS);
	bytes += rtfObjGetStorageRequirement();
	bytes += maxSessionCount * rtfSesGetStorageRequirement( maxInputBuffersPerSession,
								maxGroupsPerSequence, maxPicturesPerGroup, maxInputBufferBytes );

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// system constructor
// session kill threshold is the number of sessions that must signal failure
// before the notifier function is called
// construct a real-time trickfile library system object (once only!)
RTF_RESULT rtfSysConstructor( RTF_APP_HEAP_HANDLE hAppHeap, RTF_ALLOC_FUNCTION allocFunc,
							  RTF_FREE_FUNCTION freeFunc, RTF_LOG_FUNCTION logFunc,
							  RTF_LIBRARY_ERROR_NOTIFIER notifier,
							  unsigned long maxSessionCount, unsigned long maxInputBuffersPerSession,
							  unsigned long maxGroupsPerSequence, unsigned long maxPicturesPerGroup,
							  unsigned long maxInputBufferBytes, int sessionFailThreshold )
{
	RTF_FNAME( "rtfSysConstructor" );
	RTF_OBASE( NULL );
	RTF_RESULT result = RTF_PASS;
	RTF_SYS *pSystem;
	unsigned long i;

	do {		 // error escape wrapper - begin

		// ignore all but the first call to this function
		if( theSystemHandle != (RTF_SYS_HANDLE)NULL )
		{
			break;
		}
		// first, limit check the arguments
		if( maxSessionCount > RTF_ABSMAX_SESCOUNT )
		{
			(*notifier)( "Max session count too large" );
			result = RTF_FAIL;
			break;
		}
		if( maxInputBuffersPerSession > RTF_MAX_SES_BUFS )
		{
			(*notifier)( "Max input buffers per session too large" );
			result = RTF_FAIL;
			break;
		}
		if( maxGroupsPerSequence > RTF_MAX_SEQ_GOPS )
		{
			(*notifier)( "Max groups per sequence too large" );
			result = RTF_FAIL;
			break;
		}
		if( maxPicturesPerGroup > RTF_MAX_GOP_PICS )
		{
			(*notifier)( "Max pictures per group too large" );
			result = RTF_FAIL;
			break;
		}
		if( maxInputBufferBytes > RTF_MAX_BUFFER_BYTES )
		{
			(*notifier)( "Max input buffer bytes too large" );
			result = RTF_FAIL;
			break;
		}
		// set up a version string for the library
		sprintf( rtfProgramVersionString, "RTFLib version: %d.%d, build date: %s",
				 RTF_VERSION_MAJOR, RTF_VERSION_MINOR, __DATE__ );
		// record the max session count
		// override the default memory management functions if the caller provided some
		if( ( allocFunc != (RTF_ALLOC_FUNCTION)NULL ) &&
			( freeFunc  != (RTF_FREE_FUNCTION)NULL  ) )
		{
			// record the application supplied heap handle
			// and alloc and free function pointers
			rtfHeapHandle = hAppHeap;
			rtfAllocFunc  = allocFunc;
			rtfFreeFunc   = freeFunc;
		}
		// override the default logging function if the caller provided one
		if( logFunc != (RTF_LOG_FUNCTION)NULL )
		{
			rtfLogFunc = logFunc;
		}
		// allocate a state structure for the sys object
		pSystem = (RTF_SYS *)rtfAlloc( sizeof(RTF_SYS) );
		if( pSystem == (RTF_SYS *)NULL )
		{
			// no log available - call the failure notifier
			(*notifier)( "System object alloc failure" );
			result = RTF_FAIL;
			break;
		}
		// clear the state structure
		memset( (void *)pSystem, 0, sizeof(*pSystem) );
		// record the session failure threshold
		pSystem->sessionFailThreshold = sessionFailThreshold;
		// record the failure notifier function pointer
		pSystem->notifier = notifier;
		// create an embedded base object
		result = rtfObjConstructor( RTF_OBJ_TYPE_SYS, (RTF_HANDLE)pSystem, (RTF_HANDLE)NULL, &pSystem->hBaseObject );
		RTF_CHK_RESULT;
		// record the maximum session count
		pSystem->maxSessionCount = maxSessionCount;
		// create a set of session objects
		for( i=0; i<maxSessionCount; ++i )
		{
			result = rtfSesConstructor( &pSystem->hSessionPool[ i ], maxInputBuffersPerSession,
								maxGroupsPerSequence, maxPicturesPerGroup, maxInputBufferBytes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// record the global sys handle
		theSystemHandle = (RTF_SYS_HANDLE)pSystem;
		// set the default logging priority level
		pSystem->msgPrioFilter = RTF_DEFAULT_MSG_PRIO_FILTER;
		// reset the input stream profile
		rtfSysResetProfile( pSystem );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// system destructor
RTF_RESULT rtfSysDestructor()
{
	RTF_FNAME( "rtfSysDestructor" );
	RTF_OBASE( theSystemHandle );
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;
	RTF_RESULT result = RTF_PASS;
	RTF_SES_STATE state;
	unsigned long i;

	do {		 // error escape wrapper - begin

		// close any open sessions, and destroy the session objects
		for( i=0; i<pSystem->maxSessionCount; ++i )
		{
			if( pSystem->hSessionPool[ i ] != (RTF_SES_HANDLE)NULL )
			{
				rtfSesGetState( pSystem->hSessionPool[ i ], &state );
				if( state != RTF_SES_STATE_IDLE )
				{
					rtfSesClose( pSystem->hSessionPool[ i ] );
				}
				rtfSesDestructor( pSystem->hSessionPool[ i ] );
			}
		}
		rtfFree( (void *)pSystem );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

#ifdef DO_VCD_MPEG2
void rtfInitializeTrickSpecMPEG2Main( RTF_TRICK_SPEC *pSpec )
{
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= FALSE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= FALSE;
	pSpec->insertNUL					= FALSE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= FALSE;
	pSpec->suppressPMT					= FALSE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= FALSE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= FALSE;
	pSpec->suppressFLUFF				= FALSE;
	pSpec->suppressOTHER				= FALSE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= FALSE;
	pSpec->clearDAI						= FALSE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= FALSE;
	pSpec->restampPCR					= FALSE;
	pSpec->directionalPCR				= FALSE;
	pSpec->clearDCON					= FALSE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// MPEG2 ******************************************
	// sequence extension header fixups *******
	pSpec->codec.mpeg2.setLowDelay		= FALSE;
	// GOP header fixups **********************
	pSpec->codec.mpeg2.clearGOPTime		= FALSE;
	pSpec->codec.mpeg2.clearDropFrame	= FALSE;
	pSpec->codec.mpeg2.setClosedGOP		= FALSE;
	pSpec->codec.mpeg2.setBrokenLink	= FALSE;
	// picture header fixups ******************
	pSpec->codec.mpeg2.clearTemporalRef	= FALSE;
	pSpec->codec.mpeg2.resetVBVDelay	= FALSE;
	// picture coding extension fixups ********
	pSpec->codec.mpeg2.clear32Pulldown	= FALSE;
}

void rtfInitializeTrickSpecMPEG2VVX( RTF_TRICK_SPEC *pSpec )
{
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= TRUE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= TRUE;
	pSpec->insertNUL					= FALSE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= FALSE;
	pSpec->suppressPMT					= FALSE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= TRUE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= TRUE;
	pSpec->suppressFLUFF				= FALSE;
	pSpec->suppressOTHER				= TRUE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= TRUE;
	pSpec->clearDAI						= TRUE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= TRUE;
	pSpec->restampPCR					= TRUE;
	pSpec->directionalPCR				= TRUE;
	pSpec->clearDCON					= TRUE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// MPEG2 ******************************************
	// sequence extension header fixups *******
	pSpec->codec.mpeg2.setLowDelay		= FALSE;
	// GOP header fixups **********************
	pSpec->codec.mpeg2.clearGOPTime		= FALSE;
	pSpec->codec.mpeg2.clearDropFrame	= TRUE;
	pSpec->codec.mpeg2.setClosedGOP		= TRUE;
	pSpec->codec.mpeg2.setBrokenLink	= TRUE;
	// picture header fixups ******************
	pSpec->codec.mpeg2.clearTemporalRef	= TRUE;
	pSpec->codec.mpeg2.resetVBVDelay	= TRUE;
	// picture coding extension fixups ********
	pSpec->codec.mpeg2.clear32Pulldown	= TRUE;
}

void rtfInitializeTrickSpecMPEG2VV2( RTF_TRICK_SPEC *pSpec )
{
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= TRUE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= FALSE;
	pSpec->insertNUL					= TRUE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= TRUE;
	pSpec->suppressPMT					= TRUE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= TRUE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= TRUE;
	pSpec->suppressFLUFF				= TRUE;
	pSpec->suppressOTHER				= TRUE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= TRUE;
	pSpec->clearDAI						= TRUE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= TRUE;
	pSpec->restampPCR					= FALSE;
	pSpec->directionalPCR				= TRUE;
	pSpec->clearDCON					= TRUE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// MPEG2 ******************************************
	// sequence extension header fixups *******
	pSpec->codec.mpeg2.setLowDelay		= FALSE;
	// GOP header fixups **********************
	pSpec->codec.mpeg2.clearGOPTime		= FALSE;
	pSpec->codec.mpeg2.clearDropFrame	= TRUE;
	pSpec->codec.mpeg2.setClosedGOP		= TRUE;
	pSpec->codec.mpeg2.setBrokenLink	= TRUE;
	// picture header fixups ******************
	pSpec->codec.mpeg2.clearTemporalRef	= TRUE;
	pSpec->codec.mpeg2.resetVBVDelay	= TRUE;
	// picture coding extension fixups ********
	pSpec->codec.mpeg2.clear32Pulldown	= TRUE;
}
#endif // #ifdef DO_VCD_MPEG2

#ifdef DO_VCD_H264
void rtfInitializeTrickSpecH264Main( RTF_TRICK_SPEC *pSpec )
{
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= FALSE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= FALSE;
	pSpec->insertNUL					= FALSE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= FALSE;
	pSpec->suppressPMT					= FALSE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= FALSE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= FALSE;
	pSpec->suppressFLUFF				= FALSE;
	pSpec->suppressOTHER				= FALSE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= FALSE;
	pSpec->clearDAI						= FALSE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= FALSE;
	pSpec->restampPCR					= FALSE;
	pSpec->directionalPCR				= FALSE;
	pSpec->clearDCON					= FALSE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// MPEG2 ******************************************
	// sequence extension header fixups *******
	pSpec->codec.mpeg2.setLowDelay		= FALSE;
	// GOP header fixups **********************
	pSpec->codec.mpeg2.clearGOPTime		= FALSE;
	pSpec->codec.mpeg2.clearDropFrame	= FALSE;
	pSpec->codec.mpeg2.setClosedGOP		= FALSE;
	pSpec->codec.mpeg2.setBrokenLink	= FALSE;
	// picture header fixups ******************
	pSpec->codec.mpeg2.clearTemporalRef	= FALSE;
	pSpec->codec.mpeg2.resetVBVDelay	= FALSE;
	// picture coding extension fixups ********
	pSpec->codec.mpeg2.clear32Pulldown	= FALSE;
}

void rtfInitializeTrickSpecH264VVX( RTF_TRICK_SPEC *pSpec )
{
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= TRUE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= TRUE;
	pSpec->insertNUL					= FALSE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= TRUE;
	pSpec->suppressPMT					= TRUE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= TRUE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= TRUE;
	pSpec->suppressFLUFF				= FALSE;
	pSpec->suppressOTHER				= TRUE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= TRUE;
	pSpec->clearDAI						= TRUE;
	pSpec->suppressPTSDTS				= TRUE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= TRUE;
	pSpec->restampPCR					= FALSE;
	pSpec->directionalPCR				= TRUE;
	pSpec->clearDCON					= TRUE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// H264 *******************************************
	// placeholder ****************************
	pSpec->codec.h264.foo				= TRUE;
}

void rtfInitializeTrickSpecH264VV2( RTF_TRICK_SPEC *pSpec )
{
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= TRUE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= FALSE;
	pSpec->insertNUL					= TRUE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= TRUE;
	pSpec->suppressPMT					= TRUE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= TRUE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= TRUE;
	pSpec->suppressFLUFF				= TRUE;
	pSpec->suppressOTHER				= TRUE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= TRUE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= TRUE;
	pSpec->clearDAI						= TRUE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= TRUE;
	pSpec->restampPCR					= FALSE;
	pSpec->directionalPCR				= TRUE;
	pSpec->clearDCON					= TRUE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// H264 *******************************************
	// placeholder ****************************
	pSpec->codec.h264.foo				= TRUE;
}
#endif // #ifdef DO_VCD_H264

#ifdef DO_VCD_VC1
void rtfInitializeTrickSpecVC1Main( RTF_TRICK_SPEC *pSpec )
{
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= FALSE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= FALSE;
	pSpec->insertNUL					= FALSE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= FALSE;
	pSpec->suppressPMT					= FALSE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= FALSE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= FALSE;
	pSpec->suppressFLUFF				= FALSE;
	pSpec->suppressOTHER				= FALSE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= FALSE;
	pSpec->clearDAI						= FALSE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= FALSE;
	pSpec->restampPCR					= FALSE;
	pSpec->directionalPCR				= FALSE;
	pSpec->clearDCON					= FALSE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// MPEG2 ******************************************
	// sequence extension header fixups *******
	pSpec->codec.mpeg2.setLowDelay		= FALSE;
	// GOP header fixups **********************
	pSpec->codec.mpeg2.clearGOPTime		= FALSE;
	pSpec->codec.mpeg2.clearDropFrame	= FALSE;
	pSpec->codec.mpeg2.setClosedGOP		= FALSE;
	pSpec->codec.mpeg2.setBrokenLink	= FALSE;
	// picture header fixups ******************
	pSpec->codec.mpeg2.clearTemporalRef	= FALSE;
	pSpec->codec.mpeg2.resetVBVDelay	= FALSE;
	// picture coding extension fixups ********
	pSpec->codec.mpeg2.clear32Pulldown	= FALSE;
}

void rtfInitializeTrickSpecVC1VVX( RTF_TRICK_SPEC *pSpec )
{
	// !!! FIX ME !!! CLONE OF MPEG2 / VVX CASE - CUSTOMIZE !!!
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= TRUE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= TRUE;
	pSpec->insertNUL					= FALSE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= FALSE;
	pSpec->suppressPMT					= FALSE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= TRUE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= TRUE;
	pSpec->suppressFLUFF				= TRUE;
	pSpec->suppressOTHER				= TRUE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= TRUE;
	pSpec->clearDAI						= TRUE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= TRUE;
	pSpec->restampPCR					= TRUE;
	pSpec->directionalPCR				= TRUE;
	pSpec->clearDCON					= TRUE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// VC1 ********************************************
	// placeholder ****************************
	pSpec->codec.vc1.bar				= TRUE;
}

void rtfInitializeTrickSpecVC1VV2( RTF_TRICK_SPEC *pSpec )
{
	// !!! FIX ME !!! CLONE OF MPEG2 / VVX CASE - CUSTOMIZE !!!
	// TRANSPORT SECTION ************************************************************
	// packet insert / replace / suppress fixups 
	pSpec->insertPES					= FALSE;
	pSpec->insertPSI					= TRUE;
	pSpec->insertPCR					= FALSE;
	pSpec->insertNCF					= FALSE;
	pSpec->insertNUL					= TRUE;
	pSpec->replacePES					= FALSE;
	pSpec->replacePAT					= FALSE;
	pSpec->replacePMT					= FALSE;
	pSpec->suppressPAT					= TRUE;
	pSpec->suppressPMT					= TRUE;
	pSpec->suppressCAT					= FALSE;
	pSpec->suppressECM					= FALSE;
	pSpec->suppressAUD					= TRUE;
	pSpec->suppressDAT					= FALSE;
	pSpec->suppressCA					= FALSE;
	pSpec->suppressNUL					= TRUE;
	pSpec->suppressFLUFF				= TRUE;
	pSpec->suppressOTHER				= TRUE;
	pSpec->augmentWithNUL				= TRUE;
	pSpec->augmentWithFLUFF				= FALSE;
	pSpec->augmentWithPID				= FALSE;
	// misc general fixup flags *****************
	pSpec->ignoreEncryption				= FALSE;
	pSpec->prefixPSI					= TRUE;
	pSpec->constantBitRate				= TRUE;
	pSpec->userBitRate					= FALSE;
	pSpec->interpTimeStamps				= FALSE;
	pSpec->forcePadding					= FALSE;
	pSpec->optimizeForATSC				= FALSE;
	pSpec->generateTTS					= FALSE;
	pSpec->dittoFrames					= FALSE;
	pSpec->abortVidOnPPU				= FALSE;
	pSpec->rateControlVarGOP			= FALSE;
	// PES packet header fixups *****************
	pSpec->suppressInputPesHdr			= FALSE;
	pSpec->zeroPesPktLen				= TRUE;
	pSpec->clearDAI						= TRUE;
	pSpec->suppressPTSDTS				= FALSE;
	pSpec->suppressDTS					= FALSE;
	pSpec->restampPTSDTS				= FALSE;
	pSpec->insertDSM					= FALSE;
	// transport packet header fixups ***********
	pSpec->suppressInputPCR				= FALSE;
	pSpec->sequentialCC					= TRUE;
	pSpec->restampPCR					= TRUE;
	pSpec->directionalPCR				= TRUE;
	pSpec->clearDCON					= TRUE;
	pSpec->remapPIDs					= FALSE;
	pSpec->setPRIO						= FALSE;
	pSpec->setRAND						= FALSE;
	// fixup params *****************************
	// user supplied bitrate
	pSpec->userBitsPerSecond			= 0;
	// force padding factor
	pSpec->forcePaddingFactorFix8		= 0;
	// PCR fixups
	pSpec->PCRsPerSecond				= 40;
	pSpec->pcrPID						= TRANSPORT_INVALID_PID;
	// CODEC SECTION ****************************************************************
	// VC1 ********************************************
	// placeholder ****************************
	pSpec->codec.vc1.bar				= TRUE;
}
#endif // #ifdef DO_VCD_VC1

#ifdef DO_VCD_MPEG2
RTF_RESULT rtfInitializeTrickSpecMPEG2( RTF_TRICK_SPEC *pSpec, RTF_INDEX_TYPE indexType )
{
	RTF_FNAME( "rtfInitializeTrickSpecMPEG2" );
	RTF_OBASE( theSystemHandle );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// is this a main file copy?
		if( pSpec->speedNumerator == pSpec->speedDenominator )
		{
			// yes - set up a default trick spec suitable for an MPEG2 main file
			rtfInitializeTrickSpecMPEG2Main( pSpec );
		}
		else
		{
			// no - set up a default trick spec for an MPEG2 trick file
			switch( indexType )
			{
			case RTF_INDEX_TYPE_VVX:
				rtfInitializeTrickSpecMPEG2VVX( pSpec );
				break;
			case RTF_INDEX_TYPE_VV2:
				rtfInitializeTrickSpecMPEG2VV2( pSpec );
				RTF_CHK_RESULT;
				break;
			default:
				RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized index type (%d)", indexType );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif // #ifdef DO_VCD_MPEG2

#ifdef DO_VCD_H264
RTF_RESULT rtfInitializeTrickSpecH264( RTF_TRICK_SPEC *pSpec, RTF_INDEX_TYPE indexType )
{
	RTF_FNAME( "rtfInitializeTrickSpecH264" );
	RTF_OBASE( theSystemHandle );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// is this a main file copy?
		if( pSpec->speedNumerator == pSpec->speedDenominator )
		{
			// yes - set up a default trick spec suitable for an H264 main file
			rtfInitializeTrickSpecH264Main( pSpec );
		}
		else
		{
			// no - set up a default trick spec for an H264 trick file
			switch( indexType )
			{
			case RTF_INDEX_TYPE_VVX:
				rtfInitializeTrickSpecH264VVX( pSpec );
				break;
			case RTF_INDEX_TYPE_VV2:
				rtfInitializeTrickSpecH264VV2( pSpec );
				RTF_CHK_RESULT;
				break;
			default:
				RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized index type (%d)", indexType );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif // #ifdef DO_VCD_H264

#ifdef DO_VCD_VC1
RTF_RESULT rtfInitializeTrickSpecVC1( RTF_TRICK_SPEC *pSpec, RTF_INDEX_TYPE indexType )
{
	RTF_FNAME( "rtfInitializeTrickSpecVC1" );
	RTF_OBASE( theSystemHandle );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// is this a main file copy?
		if( pSpec->speedNumerator == pSpec->speedDenominator )
		{
			// yes - set up a default trick spec suitable for a VC1 main file
			rtfInitializeTrickSpecVC1Main( pSpec );
		}
		else
		{
			// no - set up a default trick spec for a VC1 trick file
			switch( indexType )
			{
			case RTF_INDEX_TYPE_VVX:
				rtfInitializeTrickSpecVC1VVX( pSpec );
				break;
			case RTF_INDEX_TYPE_VV2:
				rtfInitializeTrickSpecVC1VV2( pSpec );
				RTF_CHK_RESULT;
				break;
			default:
				RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized index type (%d)", indexType );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif // #ifdef DO_VCD_VC1

RTF_RESULT rtfInitializeTrickSpec( RTF_TRICK_SPEC *pSpec, RTF_VIDEO_CODEC_TYPE vcdType, RTF_INDEX_TYPE indexType )
{
	RTF_FNAME( "rtfInitializeTrickSpec" );
	RTF_OBASE( theSystemHandle );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		switch( vcdType )
		{
#ifdef DO_VCD_MPEG2
		case RTF_VIDEO_CODEC_TYPE_MPEG2:
			result = rtfInitializeTrickSpecMPEG2( pSpec, indexType );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_VCD_H264
		case RTF_VIDEO_CODEC_TYPE_H264:
			result = rtfInitializeTrickSpecH264( pSpec, indexType );
			RTF_CHK_RESULT;
			break;
#endif
#ifdef DO_VCD_VC1
		case RTF_VIDEO_CODEC_TYPE_VC1:
			result = rtfInitializeTrickSpecVC1( pSpec, indexType );
			RTF_CHK_RESULT;
			break;
#endif // #ifdef DO_VCD_VC1
		default:
			RTF_LOG_ERR1( RTF_MSG_ERR_BADPARAM, "Unrecognized video codec type (%d)", vcdType );
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the priority and the short form (<32 bytes for embedded!) of a message string
void rtfSysGetMsgInfo( RTF_MSG msg, RTF_MSG_PRIO *pMsgPrio, char **ppShortMsgStr )
{
	switch( msg )
	{
	// no error *******************************************
	case RTF_MSG_OK:
		*pMsgPrio = RTF_MSG_PRIO_OK;
		*ppShortMsgStr = "OK";
		break;

	// the library is shutting down ***********************
	case RTF_MSG_LIBDEATH:
		*pMsgPrio = RTF_MSG_PRIO_LIBFATAL;
		*ppShortMsgStr = "RTFLib library shutting down";
		break;

	// the session is shutting down ***********************
	case RTF_MSG_SESDEATH:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "RTFLib session shutting down";
		break;

	// errors indicating fatal library error **************
	// note: these always shut down the library
	case RTF_MSG_ERR_BADALLOC:
		*pMsgPrio = RTF_MSG_PRIO_LIBFATAL;
		*ppShortMsgStr = "allocation failure";
		break;

	// errors indicating unrecoverable session error ******
	// note: these codes always shut down the session
	case RTF_MSG_ERR_RTFCALLFAILED:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "RTF internal call failed";
		break;
	case RTF_MSG_ERR_INDEXER:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "indexer error";
		break;
	case RTF_MSG_ERR_INTERNAL:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "internal error";
		break;
	case RTF_MSG_ERR_BADTYPE:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "bad object type";
		break;
	case RTF_MSG_ERR_BADSTATE:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "bad object state";
		break;
	case RTF_MSG_ERR_NOTFOUND:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "object not found";
		break;

	// errors indicating recoverable application errors ***
	// note: these always shut down the session
	case RTF_MSG_ERR_NOTYETSUPPORTED:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "feature not yet supported";
		break;
	case RTF_MSG_ERR_CALLBACKFAILED:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "app callback returned error";
		break;
	case RTF_MSG_ERR_BADPARAM:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "app supplied invalid param";
		break;
	case RTF_MSG_ERR_ZOMBIE:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "zombie session terminated";
		break;

	// errors indicating bad asset ************************
	// note: these always shut down the session
	case RTF_MSG_ERR_BADSTREAM:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "unsupported bit stream format";
		break;
	case RTF_MSG_ERR_OVERFLOW:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "unrecoverable overflow";
		break;
	case RTF_MSG_ERR_UNDERFLOW:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "unrecoverable underflow";
		break;
	case RTF_MSG_ERR_SYNCLOSS:
		*pMsgPrio = RTF_MSG_PRIO_SESFATAL;
		*ppShortMsgStr = "transport level sync lost";
		break;

	// warnings *******************************************
	// note: these are recorded and the counts compared
	// to threshold values that the application provides
	case RTF_MSG_WRN_UNSUPPORTED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "unsupported feature ignored";
		break;
	case RTF_MSG_WRN_BADADAPT:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "illegal adaptation field code";
		break;
	case RTF_MSG_WRN_NOPAYLOAD:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "payload needed but not found";
		break;
	case RTF_MSG_WRN_PESNOTCAPTURED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "PES header not yet captured";
		break;
	case RTF_NSG_WRN_EMPTYPICTURE:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "picture with no content";
		break;
	case RTF_MSG_WRN_RECORDBUFFULL:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "packet recording buffer full";
		break;
	case RTF_MSG_WRN_FIRSTPICNOTKEY:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "first picture not keyframe";
		break;
	case RTF_MSG_WRN_OUTPOINTSKIPPED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "outpoint skipped";
		break;
	case RTF_MSG_WRN_PICUNDERFLOW:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "picture pool underflow";
		break;
	case RTF_MSG_WRN_PKTOVERFLOW:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "packet pool underflow";
		break;
	case RTF_MSG_WRN_PICOVERFLOW:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "picture array overflow in gop";
		break;
	case RTF_MSG_WRN_GOPOVERFLOW:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "group array overflow in seq";
		break;
	case RTF_MSG_WRN_BADCAT:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "invalid CA table";
		break;
	case RTF_MSG_WRN_CRYPTOPES:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "encrypted PES header";
		break;
	case RTF_MSG_WRN_VARBITRATE:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "variable bit rate";
		break;
	case RTF_MSG_WRN_PCRMAXGAP:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "PCR gap exceeds max";
		break;
	case RTF_MSG_WRN_NOFIRSTPCRDCON:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "First PCR with no DCON";
		break;
	case RTF_MSG_WRN_POSTFIRSTPCRDCON:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "DCON after first PCR";
		break;
	case RTF_MSG_WRN_MULTIPROGSTREAM:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Multiple programs in PAT";
		break;
	case RTF_MSG_WRN_MULTIVIDSTREAM:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Multiple video streams in PMT";
		break;
	case RTF_MSG_WRN_MULTIAUDIOCODEC:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Multiple audio CODECs in PMT";
		break;
	case RTF_MSG_WRN_SPLICINGAUDIOBAD:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "No audio support for splicing";
		break;
	case RTF_MSG_WRN_SPLICINGPSDESC:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "PMT PS descr with splicing";
		break;
	case RTF_MSG_WRN_SPLICINGESDESC:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "PMT ES descr with splicing";
		break;
	case RTF_MSG_WRN_PATCHANGED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "PAT changed mid-stream";
		break;
	case RTF_MSG_WRN_CATCHANGED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "CAT changed mid-stream";
		break;
	case RTF_MSG_WRN_PMTCHANGED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "PMT changed mid-stream";
		break;
	case RTF_MSG_WRN_PMTENCRYPTED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "PMT encrypted";
		break;
	case RTF_MSG_WRN_AUGNOTFOUND:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Augmentation PID pkts not found";
		break;
	case RTF_MSG_WRN_BADSLICETYPE:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Non-intra slice in intra pic";
		break;
	case RTF_MSG_WRN_FRCMISMATCH:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Frame rate code mismatch";
		break;
	case RTF_MSG_WRN_SYNCLOSS:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Synchronization lost";
		break;
	case RTF_MSG_WRN_SPLITPKT:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Split video packet";
		break;
	case RTF_MSG_WRN_BADTRICKSPEED:
		*pMsgPrio = RTF_MSG_PRIO_WARNING;
		*ppShortMsgStr = "Unsupported trick speed request";
		break;
	case RTF_MSG_WRN_ZEROOUTPUT:
		*pMsgPrio = RTF_MSG_PRIO_WARNING_IGNORE;
		*ppShortMsgStr = "Zero output byte count at close";
		break;
	// info messages - not counted as errors **************
	// note: RTF_MSG_INF_STATS is used for msgs whose only purpose
	// is to report values of variables (can't be done in short msg log)
	case RTF_MSG_INF_STATS:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "";
		break;
	case RTF_MSG_INF_SUPPRESSWARNING:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "warning category suppressed";
		break;
	case RTF_MSG_INF_LOGNULLHANDLE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "log req with null handle";
		break;
	case RTF_MSG_INF_LOGSTATECLOSED:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "log req on closed object";
		break;
	case RTF_MSG_INF_LOGOUTRANGE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "log request out of range";
		break;
	case RTF_MSG_INF_SEQOPEN:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "sequence opened";
		break;
	case RTF_MSG_INF_SEQADDGOP:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "group added to sequence";
		break;
	case RTF_MSG_INF_SEQCLOSE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "sequence closed";
		break;
	case RTF_MSG_INF_SEQRELEASE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "sequence released";
		break;
	case RTF_MSG_INF_GOPOPEN:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "group opened";
		break;
	case RTF_MSG_INF_GOPADDPIC:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "picture added to group";
		break;
	case RTF_MSG_INF_GOPCLOSE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "group closed";
		break;
	case RTF_MSG_INF_GOPRELEASE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "group released";
		break;
	case RTF_MSG_INF_PICOPEN:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "picture opened";
		break;
	case RTF_MSG_INF_PICCLOSE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "picture closed";
		break;
	case RTF_MSG_INF_PICRELEASE:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "picture released";
		break;
	default:
		*pMsgPrio = RTF_MSG_PRIO_INFO;
		*ppShortMsgStr = "unrecognized err";
	}
}

// call the system error notifier function
void rtfSysErrorNotifier( char *pMessage )
{
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;

	if( pSystem->notifier != (RTF_LIBRARY_ERROR_NOTIFIER)NULL )
	{
		(*pSystem->notifier)( pMessage );
	}
	++pSystem->sessionFailCount;
	if( pSystem->sessionFailThreshold != 0 )
	{
		if( pSystem->sessionFailCount >= pSystem->sessionFailThreshold )
		{
			rtfShutdownLibrary();
		}
	}
}

// get the default input stream profile for the system
RTF_RESULT rtfSysGetStreamProfile( RTF_STREAM_PROFILE **ppProfile )
{
	RTF_FNAME( "rtfSysGetStreamProfile" );
	RTF_OBASE( theSystemHandle );
	RTF_SYS *pSys = (RTF_SYS *)theSystemHandle;

	*ppProfile = &pSys->profile;

	return RTF_PASS;
}

// evaluate a warning - return the following:
// < 0 - this is an error; log it and call the notifier
// 0 - this is a warning; log it
// > 0 - this is a suppressed warning; ignore it
int rtfWarningEval( RTF_HANDLE handle, RTF_MSG msg )
{
	RTF_FNAME( "rtfEvalWarn" );
	RTF_OBASE( handle );
	RTF_SES_HANDLE hSession;

	// get the session handle
	rtfObjGetSession( handle, &hSession );
	// call the session warning evaluator
	return rtfSesWarningEval( hSession, msg );
}

// report the occurrance of an error to the owning session
RTF_RESULT rtfReportErr( RTF_HANDLE handle, RTF_MSG msg )
{
	RTF_FNAME( "rtfReportErr" );
	RTF_OBASE( handle );
	RTF_SES_HANDLE hSession;

	// get the session handle
	rtfObjGetSession( handle, &hSession );
	// call the session error notifier
	return rtfSesErrorNotifier( hSession, msg, *(RTF_OBJ_HANDLE *)handle );
}

// conditionally format and log a message string
void rtfLogMessage( RTF_HANDLE handle, RTF_MSG msg, const char *pInFunc, const char *pFmt, ... )
{
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;
	RTF_SES_HANDLE hSession;
	RTF_APP_SESSION_HANDLE hAppSession = NULL;
	RTF_MSG_PRIO prio;
	BOOL suppress = TRUE;
	char *pShortMsgStr;
	char longMsgBuf[256];
	va_list ap;
	int offset;

	// get the local session handle
	rtfObjGetSession( handle, &hSession );
	// ask the session if the message should be logged or suppressed
	if(hSession != NULL)
	{
		rtfSesCheckMessageSuppression( hSession, msg, &suppress );
	}
	if( suppress == FALSE )
	{
		// get the priority of this message
		rtfSysGetMsgInfo( msg, &prio, &pShortMsgStr );
		if( prio >= pSystem->msgPrioFilter )
		{
			// turn the priority into a message prefix
			switch( prio )
			{
			case RTF_MSG_PRIO_INFO:
				strcpy( longMsgBuf, "INFO: ");
				break;
			case RTF_MSG_PRIO_WARNING_IGNORE:
			case RTF_MSG_PRIO_WARNING:
				strcpy( longMsgBuf, "WARNING: ");
				break;
			case RTF_MSG_PRIO_SESFATAL:
				strcpy( longMsgBuf, "NON-RECOVERABLE: ");
				break;
			case RTF_MSG_PRIO_LIBFATAL:
				strcpy( longMsgBuf, "LIBFATAL: ");
				break;
			default:
				break;
			}
			offset = strlen( longMsgBuf );
			va_start( ap, pFmt );
			// format the message string
			offset += vsprintf( (longMsgBuf+offset), pFmt, ap );
			va_end( ap );
			// append a newline to the string
			longMsgBuf[ offset ] = '\n';
			longMsgBuf[ offset+1 ] = 0;
			// get the application session handle
			rtfSesGetAppHandle( hSession, &hAppSession );
			// call the session logging function
			// give it the short and long versions of the message
			(*rtfLogFunc)( hAppSession, pShortMsgStr, longMsgBuf );
		}
	}
}

#if ( defined(_EMBEDDED) && defined(_LINUX) )
INT64 RTFDiv64( INT64 numerator, INT64 denominator )
{
	INT64 tmp64;
	INT64 res64;
	int   i;
	int   sign;

	if( denominator == 0 )
	{
		return 0;
	}
	res64 = 0;
	tmp64 = 0;
	sign  = 0;
	if( numerator < 0 )
	{
		++sign;
		numerator = -numerator;
	}
	if( denominator < 0 )
	{
		++sign;
		denominator = -denominator;
	}
	for( i=0; i<64; ++i )
	{
#if 0	// this should have worked - why didn't it?
		if( ( tmp64 <<= 1 ) == 0 )
		{
			res64 <<= ( 64 - i );
			break;
		}
#else
		tmp64 <<= 1;
#endif
		res64 <<= 1;
		if( numerator < 0 )
		{
			tmp64 |= 1;
		}
		if( tmp64 >= denominator )
		{
			res64 |= 1;
			tmp64 -= denominator;
		}
		numerator <<= 1;
	}
	if( ( sign & 0x01 ) != 0 )
	{
		res64 = -res64;
	}
	return res64;
}
#endif	// #if ( defined(_EMBEDDED) && defined(_LINUX) )

// external API functions ***************************************************************

// get the version number of the library as an unsigned long
// ( upper 16 bits = major, lower 16 bits = minor )
EXPORT void rtfGetVersion( unsigned long *pVersion )
{
	*pVersion = RTF_VERSION;
}

// get the version number of the library as a string
// ( note: buffer should be at least 16 chars in length )
EXPORT void rtfGetVersionString( char *pVersionString )
{
	sprintf( pVersionString, "%d:%d", RTF_VERSION_MAJOR, RTF_VERSION_MINOR );
}

// get a pointer to a string that identifies this version of the library completely
EXPORT void rtfGetProgramVersionString( char **ppProgramVersionString )
{
	*ppProgramVersionString = rtfProgramVersionString;
}

// get the amount of storage that will be consumed by a call to rtfInitializeDefaultLibrary (below)
EXPORT unsigned long rtfGetDefaultLibraryStorageRequirement( BOOL isHD )
{
	if( isHD == FALSE )
	{
		return rtfSysGetStorageRequirement( RTF_SD_DEFAULT_SES_COUNT,
											RTF_SD_DEFAULT_INPBUF_PERSES,
											RTF_SD_DEFAULT_GROUPS_PERSEQ,
											RTF_SD_DEFAULT_PICS_PERGROUP,
											RTF_SD_DEFAULT_INPBUF_BYTES );
	}
	return rtfSysGetStorageRequirement( RTF_HD_DEFAULT_SES_COUNT,
										RTF_HD_DEFAULT_INPBUF_PERSES,
										RTF_HD_DEFAULT_GROUPS_PERSEQ,
										RTF_HD_DEFAULT_PICS_PERGROUP,
										RTF_HD_DEFAULT_INPBUF_BYTES );
}

// get the amount of storage that will be consumed by a call to rtfInitializeLibrary (below)
EXPORT unsigned long rtfGetLibraryStorageRequirement( unsigned long maxSessionCount,
						unsigned long maxInputBuffersPerSession, unsigned long maxGroupsPerSequence,
						unsigned long maxPicturesPerGroup, unsigned long maxInputBufferBytes )
{
	return rtfSysGetStorageRequirement( maxSessionCount, maxInputBuffersPerSession,
						maxGroupsPerSequence, maxPicturesPerGroup, maxInputBufferBytes );
}

// initialize the real-time trickfile library using default values (once only!)
// returns TRUE for success; else FAIL
EXPORT BOOL rtfInitializeDefaultLibrary( RTF_APP_HEAP_HANDLE hAppHeap,
										 RTF_ALLOC_FUNCTION allocFunc,
										 RTF_FREE_FUNCTION freeFunc,
										 RTF_LOG_FUNCTION logFunc,
										 RTF_LIBRARY_ERROR_NOTIFIER notifier,
										 BOOL isHD )
{
	if( isHD == FALSE )
	{
		return ( rtfSysConstructor( hAppHeap, allocFunc, freeFunc, logFunc, notifier,
									RTF_SD_DEFAULT_SES_COUNT,
									RTF_SD_DEFAULT_INPBUF_PERSES,
									RTF_SD_DEFAULT_GROUPS_PERSEQ,
									RTF_SD_DEFAULT_PICS_PERGROUP,
									RTF_SD_DEFAULT_INPBUF_BYTES,
									RTF_SD_DEFAULT_FAIL_THRESH ) == RTF_PASS ) ? TRUE : FALSE;
	}
	return ( rtfSysConstructor( hAppHeap, allocFunc, freeFunc, logFunc, notifier,
								RTF_HD_DEFAULT_SES_COUNT,
								RTF_HD_DEFAULT_INPBUF_PERSES,
								RTF_HD_DEFAULT_GROUPS_PERSEQ,
								RTF_HD_DEFAULT_PICS_PERGROUP,
								RTF_HD_DEFAULT_INPBUF_BYTES,
								RTF_HD_DEFAULT_FAIL_THRESH ) == RTF_PASS ) ? TRUE : FALSE;
}

// initialize the real-time trickfile library (once only!)
// returns TRUE for success; else FAIL
EXPORT BOOL rtfInitializeLibrary( RTF_APP_HEAP_HANDLE hAppHeap, RTF_ALLOC_FUNCTION allocFunc,
						   RTF_FREE_FUNCTION freeFunc, RTF_LOG_FUNCTION logFunc,
						   unsigned long maxSessionCount, unsigned long maxInputBuffersPerSession,
						   unsigned long maxGroupsPerSequence, unsigned long maxPicturesPerGroup,
						   unsigned long maxInputBufferBytes, int sessionFailThreshold,
						   RTF_LIBRARY_ERROR_NOTIFIER notifier )
{
	return ( rtfSysConstructor( hAppHeap, allocFunc, freeFunc, logFunc, notifier, maxSessionCount,
								maxInputBuffersPerSession, maxGroupsPerSequence, maxPicturesPerGroup,
								maxInputBufferBytes, sessionFailThreshold ) == RTF_PASS ) ? TRUE : FALSE;
}

// set the input stream profile for the library
// NOTE: profile should be constant for a site!
// NOTE: must be called before any sessions are opened if assets are to be prepared for splicing!
// returns TRUE for success; else FAIL
EXPORT BOOL rtfSetStreamProfile( RTF_STREAM_PROFILE *pProfile )
{
	RTF_FNAME( "rtfSetStreamProfile" );
	RTF_OBASE( theSystemHandle );
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSystem, RTF_OBJ_TYPE_SYS );
		memcpy( (void *)&(pSystem->profile), (void *)pProfile, sizeof(pSystem->profile) );

	} while( 0 ); // error escape wrapper - end

	return ( result == RTF_PASS ) ? TRUE : FALSE;
}

// shut down the real-time trickfile library
// returns TRUE for success; else FAIL
EXPORT BOOL rtfShutdownLibrary()
{
	return ( rtfSysDestructor() == RTF_PASS ) ? TRUE : FALSE;
}

// get the current priority filter level
EXPORT BOOL rtfGetMsgPrioFilter( RTF_MSG_PRIO *pPrio )
{
	RTF_FNAME( "rtfGetMsgPrioFilter" );
	RTF_OBASE( theSystemHandle );
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSystem, RTF_OBJ_TYPE_SYS );
		*pPrio = pSystem->msgPrioFilter;

	} while( 0 ); // error escape wrapper - end

	return ( result == RTF_PASS ) ? TRUE : FALSE;
}

// set the current priority filter level
// returns TRUE for success; else FAIL
EXPORT BOOL rtfSetMsgPrioFilter( RTF_MSG_PRIO prio )
{
	RTF_FNAME( "rtfSetMsgPrioFilter" );
	RTF_OBASE( theSystemHandle );
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSystem, RTF_OBJ_TYPE_SYS );
		pSystem->msgPrioFilter = prio;

	} while( 0 ); // error escape wrapper - end

	return ( result == RTF_PASS ) ? TRUE : FALSE;
}

// get the current zombie timeout threshold setting
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetZombieThreshold( int *pZombieThreshold )
{
	*pZombieThreshold = rtfZombieThreshold;
	return TRUE;
}

// set the current zombie timeout threshold setting
// returns TRUE for success; else FAIL
EXPORT BOOL rtfSetZombieThreshold( int zombieThreshold )
{
	rtfZombieThreshold = zombieThreshold;
	return TRUE;
}

// set an RTF_APP_OUTPUT_SETTINGS structure to all default values
EXPORT BOOL rtfInitializeOutputSettings( RTF_APP_OUTPUT_SETTINGS *pSettings,
										 RTF_VIDEO_CODEC_TYPE vcdType,
										 RTF_INDEX_TYPE indexType,
										 RTF_APP_FILE_HANDLE hAppOutFile,
										 int speedDirection,
										 unsigned long speedNumerator,
										 unsigned long speedDenominator,
										 char *pFileExtension,
										 RTF_BUFGET_FUNCTION getFunction,
										 RTF_BUFPUT_FUNCTION putFunction,
										 RTF_OUTCLOSE_FUNCTION closeFunction )
{
	RTF_FNAME( "rtfInitializeOutputSettings" );
	RTF_OBASE( theSystemHandle );
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;
	RTF_RESULT result = RTF_PASS;
	RTF_TRICK_SPEC *pSpec;

	do {		 // error escape wrapper - begin

		// pre-clear the structure
		memset( (void *)pSettings, 0, sizeof(*pSettings) );
		// record the application's output file handle
		pSettings->hAppOutFile			= hAppOutFile;
		// set up the callbacks
		pSettings->pBufferGetFunction   = getFunction;
		pSettings->pBufferPutFunction   = putFunction;
		pSettings->pOutputCloseFunction = closeFunction;
		// make a convenience pointer
		pSpec = &( pSettings->trickSpec );
		// record speed info
		pSpec->speedDirection			= speedDirection;
		pSpec->speedNumerator           = speedNumerator;
		pSpec->speedDenominator         = speedDenominator;
		// copy the file extension
		strncpy( pSpec->fileExtension, pFileExtension, RTF_MAX_EXT_LEN );
		// don't set up the trickspec if this is the index file
		if( speedNumerator != 0 )
		{
			// set up the trickspec as appropriate for the
			// selected video codec and index type
			result = rtfInitializeTrickSpec( pSpec, vcdType, indexType );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return ( result == RTF_PASS ) ? TRUE : FALSE;
}

// initialize an RTF_WARNING_THRESHOLD structure with default values
// note: used for recoverable errors only. application may set up this
// structure or call this function and override selected default values
EXPORT BOOL rtfInitializeWarningThresholds( RTF_WARNING_TOLERANCE tolerance,
										    RTF_WARNING_COUNTS *pThresholds )
{
	int threshold;

	switch( tolerance )
	{
	case RTF_WARNING_TOLERANCE_STRICT:
		threshold = 0;
		break;
	case RTF_WARNING_TOLERANCE_RELAXED:
		threshold = -1;
		break;
	default:
		threshold = 0;	// default is strict
		break;
	}
	pThresholds->suppressCount		= RTF_DEFAULT_SUPPRESS_COUNT;
	pThresholds->total				= threshold;	// sum of all entries below
	pThresholds->unsupported		= threshold;	// RTF_MSG_WRN_UNSUPPORTED
	pThresholds->badAdapt			= threshold;	// RTF_MSG_WRN_BADADAPT
	pThresholds->noPaylod			= threshold;	// RTF_MSG_WRN_NOPAYLOAD
	pThresholds->missingPesHdr		= threshold;	// RTF_MSG_WRN_PESNOTCAPTURED
	pThresholds->emptyPicture		= threshold;	// RTF_NSG_WRN_EMPTYPICTURE
	pThresholds->recordFull			= threshold;	// RTF_MSG_WRN_RECORDBUFFULL
	pThresholds->firstPicNotKey		= threshold;	// RTF_MSG_WRN_FIRSTPICNOTKEY
	pThresholds->outpointSkipped	= threshold;	// RTF_MSG_WRN_OUTPOINTSKIPPED
	pThresholds->picPoolUnderflow	= threshold;	// RTF_MSG_WRN_PICUNDERFLOW
	pThresholds->pktArrayOverflow	= threshold;	// RTF_MSG_WRN_PKTOVERFLOW
	pThresholds->picArrayOverflow	= threshold;	// RTF_MSG_WRN_PICOVERFLOW
	pThresholds->gopArrayOverflow	= threshold;	// RTF_MSG_WRN_GOPOVERFLOW
	pThresholds->badCat				= threshold;	// RTF_MSG_WRN_BADCAT
	pThresholds->cryptoPes			= threshold;	// RTF_MSG_WRN_CRYPTOPES
	pThresholds->varBitRate			= threshold;	// RTF_MSG_WRN_VARBITRATE
	pThresholds->pcrMaxGap			= threshold;	// RTF_MSG_WRN_PCRMAXGAP
	pThresholds->noFirstPcrDcon		= threshold;	// RTF_MSG_WRN_NOFIRSTPCRDCON
	pThresholds->postFirstPcrDcon	= threshold;	// RTF_MSG_WRN_POSTFIRSTPCRDCON
	pThresholds->multiProgStream	= threshold;	// RTF_MSG_WRN_MULTIPROGSTREAM
	pThresholds->multiVidStream		= threshold;	// RTF_MSG_WRN_MULTIVIDSTREAM
	pThresholds->multiAudioCodec	= threshold;	// RTF_MSG_WRN_MULTIAUDIOCODEC
	pThresholds->splicingAudioBad	= threshold;	// RTF_MSG_WRN_SPLICINGAUDIOBAD
	pThresholds->splicingPSDesc		= threshold;	// RTF_MSG_WRN_SPLICINGPSDESC
	pThresholds->splicingESDesc		= threshold;	// RTF_MSG_WRN_SPLICINGESDESC
	pThresholds->patChanged			= threshold;	// RTF_MSG_WRN_PATCHANGED
	pThresholds->catChanged			= threshold;	// RTF_MSG_WRN_CATCHANGED
	pThresholds->pmtChanged			= threshold;	// RTF_MSG_WRN_PMTCHANGED
	pThresholds->pmtEncrypted		= threshold;	// RTF_MSG_WRN_PMTENCRYPTED
	pThresholds->augNotFound		= threshold;	// RTF_MSG_WRN_AUGNOTFOUND
	pThresholds->badSliceType		= threshold;	// RTF_MSG_WRN_BADSLICETYPE
	pThresholds->frcMismatch		= threshold;	// RTF_MSG_WRN_FRCMISMATCH
	pThresholds->syncLoss			= threshold;	// RTF_MSG_WRN_SYNCLOSS
	pThresholds->splitPkt			= threshold;	// RTF_MSG_WRN_SPLITPKT
	pThresholds->badTrickSpeed		= threshold;	// RTF_MSG_WRN_BADTRICKSPEED
	pThresholds->zeroOutput			= threshold;	// RTF_MSG_WRN_ZEROOUTPUT
	return TRUE;
}

// log the output settings for a particular output of a session
EXPORT BOOL rtfLogOutputSettings( RTF_SES_HANDLE hSession, int outputNumber )
{
	return ( rtfSesLogOutputSettings( hSession, outputNumber ) == RTF_PASS ) ? TRUE : FALSE;
}

// log the output settings for all of the outputs of a session
EXPORT BOOL rtfLogAllOutputSettings( RTF_SES_HANDLE hSession )
{
	return ( rtfSesLogAllOutputSettings( hSession ) == RTF_PASS ) ? TRUE : FALSE;
}

// open a trick file generation session
// returns TRUE for success; else FAIL
EXPORT BOOL rtfOpenSession( RTF_SES_HANDLE *phSession,
							RTF_APP_SESSION_HANDLE hAppSession,
							RTF_APP_FILE_HANDLE hAppInputFile,
							char *pInputFileName, INT64 inputFileBytes,
							RTF_WARNING_COUNTS *pThresholds, RTF_INDEX_MODE indexMode,
							RTF_INDEX_TYPE indexType, RTF_INDEX_OPTION indexOption,
							RTF_SES_ERROR_NOTIFIER notifier,
							RTF_BUFREL_FUNCTION inputBufferReleaseCallback,
							int numSettings, RTF_APP_OUTPUT_SETTINGS settings[] )
{
	RTF_FNAME( "rtfOpenSession" );
	RTF_OBASE( theSystemHandle );
	RTF_SYS *pSystem = (RTF_SYS *)theSystemHandle;
	RTF_SES_STATE state;
	RTF_RESULT result = RTF_PASS;
	unsigned long i, currentTime;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSystem, RTF_OBJ_TYPE_SYS );
		// is an active zombie threshold set?
		if( rtfZombieThreshold > 0 )
		{
			// yes - get the current time
			rtfGetRunTime( &currentTime );
			// scan the session pool - scavenge any zombies that are found
			for( i=0; i<pSystem->maxSessionCount; ++i )
			{
				rtfSesZombieCheck( pSystem->hSessionPool[ i ], currentTime, rtfZombieThreshold );
			}
		}
		// look for a free session object
		for( i=0; i<pSystem->maxSessionCount; ++i )
		{
			result = rtfSesGetState( pSystem->hSessionPool[ i ], &state );
			RTF_CHK_RESULT;
			if( state == RTF_SES_STATE_IDLE )
			{
				break;
			}
		}
		RTF_CHK_RESULT_LOOP;
		// was one available?
		if( i >= pSystem->maxSessionCount )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_OVERFLOW, "Session pool underflow" );
			break;
		}
		// open a session
		result = rtfSesOpen( pSystem->hSessionPool[ i ], hAppSession,
							 hAppInputFile, pInputFileName, inputFileBytes,
							 pThresholds, indexMode, indexType, indexOption,
							 notifier, inputBufferReleaseCallback, numSettings, settings );
		RTF_CHK_RESULT;
		// make the return
		*phSession = pSystem->hSessionPool[ i ];

	} while( 0 ); // error escape wrapper - end

	return ( result == RTF_PASS ) ? TRUE : FALSE;
}

// close a trick file generation session
// returns TRUE for success; else FAIL
EXPORT BOOL rtfCloseSession( RTF_SES_HANDLE hSession )
{
	return ( rtfSesClose( hSession ) == RTF_PASS ) ? TRUE : FALSE;
}

// get the number of packets processed by a session so far
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetProgress( RTF_SES_HANDLE hSession, unsigned long *pProcessedPacketCount )
{
	return ( rtfSesGetProgress( hSession, pProcessedPacketCount ) == RTF_PASS ) ? TRUE : FALSE;
}

EXPORT void rtfGetRunTime( RTF_RUNTIME *pRunTime )
{
#ifdef __KERNEL__
	struct timeval tv;
	do_gettimeofday(&tv);
	*pRunTime = (RTF_RUNTIME)tv.tv_sec;
#else
	*pRunTime = (unsigned long)clock();
#endif
}

// get the input bit rate
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetInputBitrate( RTF_SES_HANDLE handle, unsigned long *pBitsPerSecond )
{
	return ( rtfSesGetInputBitrate( handle, pBitsPerSecond ) == RTF_PASS ) ? TRUE : FALSE;
}

// get the output bit rate
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetOutputBitrate( RTF_SES_HANDLE handle, unsigned long *pBitsPerSecond )
{
	return ( rtfSesGetOutputBitrate( handle, pBitsPerSecond ) == RTF_PASS ) ? TRUE : FALSE;
}

// get the current session elapsed time in clock ticks
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetElapsedTime( RTF_SES_HANDLE handle, RTF_RUNTIME *pElapsedTime )
{
	return ( rtfSesGetElapsedTime( handle, pElapsedTime ) == RTF_PASS ) ? TRUE : FALSE;
}

// submit a buffer of input for processing (buffer is held by trickfile library until explicitly released)
// returns TRUE for success; else FAIL
EXPORT BOOL rtfProcessInput( RTF_SES_HANDLE handle,
					  RTF_APP_SESSION_HANDLE hAppSession,
					  RTF_APP_FILE_HANDLE hAppFile,
					  RTF_APP_BUFFER_HANDLE hAppBuffer,
					  unsigned char *pBuffer, unsigned long bytes )
{
	return ( rtfSesProcessInput( handle, hAppSession, hAppFile, hAppBuffer, pBuffer, bytes ) == RTF_PASS ) ? TRUE : FALSE;
}

// get the augmentation info for a session
// the caller provides the PIDs that were added after the original transport multiplex
// (ex: FEC, ECMs, etc.). This function reads the main input file and computes the
// augmented bitrate (the bit rate that the augmented stream must be transmitted at)
// as well as the original bitrate of the stream before the extra packets were added.
// these bit rates are measured over the stream as a whole. The file is rewound to the
// beginning at the end of this operation.
// returns TRUE for success; else FAIL
// Note that this function is not compatible with PWE.
EXPORT BOOL rtfGetAugmentationInfo( RTF_SES_HANDLE handle,
									int augmentationPidCount,
									unsigned short *pAugmentationPids,
									RTF_RDINPUT_FUNCTION readInputCallback,
									int *pAugmentedBitRate, int *pOriginalBitRate )
{
	return ( rtfSesGetAugmentationInfo( handle,
			 augmentationPidCount, pAugmentationPids,
			 readInputCallback, pAugmentedBitRate, pOriginalBitRate ) == RTF_PASS ) ? TRUE : FALSE;
}

#ifdef _DEBUG
// debug helper function - log the state of the session
// returns TRUE for success; else FAIL
EXPORT BOOL rtfLogState( RTF_SES_HANDLE handle )
{
	return ( rtfSesLogState( handle ) == RTF_PASS ) ? TRUE : FALSE;
}
#endif



