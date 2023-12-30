// implementation file for rtfSes class
// abstracts action of converting one media stream into set of trick files
//

#include "RTFPrv.h"

// constants ****************************************************************************

#define RTF_MAX_FRAME_RATE_CODE				8

// look-up tables ***********************************************************************

static int frcFix16[] =
{
	0,										// undefined
	(unsigned long)( 23.976 * 65536.0 ),	// 1
	(unsigned long)( 24.000 * 65536.0 ),	// 2
	(unsigned long)( 25.000 * 65536.0 ),	// 3
	(unsigned long)( 29.970 * 65536.0 ),	// 4
	(unsigned long)( 30.000 * 65536.0 ),	// 5
	(unsigned long)( 50.000 * 65536.0 ),	// 6
	(unsigned long)( 59.940 * 65536.0 ),	// 7
	(unsigned long)( 60.000 * 65536.0 )		// 8
};

static char *frcString[] =
{
	"undefined",
	"23.976",
	"24.000",
	"25.000",
	"29.970",
	"30.000",
	"50.000",
	"59.940",
	"60.000"
};

// typedefs *****************************************************************************

typedef struct _RTF_AUG_INFO
{
	int inSync;
	int syncSearchByteCt;
	int lastPktType;
	int totalPktCt;
	int firstPcrPktCt;
	int lastPcrPktCt;
	int augPktCt;
	int augClusterCt;
	int basePktCt;
	int baseClusterCt;
	unsigned long offset;
	unsigned long bufOccupancy;
	unsigned long augmentedBitRate;
	int augmentationPidCount;
	unsigned short *pAugmentationPids;
	INT64 lastPcrValue;
	INT64 firstPcrValue;
	unsigned char buf[ RTF_SES_MAX_RD_PACKETS * TRANSPORT_PACKET_BYTES ];

} RTF_AUG_INFO;

typedef struct _RTF_SES
{
	RTF_OBJ_HANDLE hBaseObject;
	RTF_SES_STATE state;

	// these fields are only used when the session is in the open state
	char inputFileName[ MAX_PATHLEN ];
	INT64 inputFileBytes;
	RTF_APP_SESSION_HANDLE hAppSession;
	RTF_APP_FILE_HANDLE hAppInputFile;
	RTF_SES_ERROR_NOTIFIER notifier;
	RTF_BUFREL_FUNCTION inputBufferReleaseCallback;
	RTF_WARNING_COUNTS warningThresholds;
	RTF_WARNING_COUNTS warningCounts;
	unsigned long maxInputBuffers;
	unsigned long maxGroups;
	unsigned long maxPictures;
	int indexFileOutputNumber;
	int mainFileOutputNumber;
	unsigned long startTime;
	unsigned long lastProcessInputTime;
	unsigned long stopTime;
	BOOL inputInSync;
	BOOL inputIsTTS;
	BOOL psiValid;
	BOOL bitRateAcquired;
	BOOL frameRateAcquired;
	BOOL pesHeaderAcquired;
	BOOL indexLibraryIntialized;
	BOOL readyForStreaming;
	BOOL splicingEnabled;
	BOOL casReady;
	BOOL casLocked;
	BOOL firstPcrAcquired;
	BOOL mainFileBacklogProcessed;
	BOOL insertDSM;
	BOOL generateTTS;
	BOOL ignoreEncryption;
	BOOL optimizeForATSC;
	BOOL abortVidOnPPU;
	RTF_STREAM_PROFILE profile;

	// some running counters
	UINT64 totalInputByteCount;
	unsigned long totalInputBufferCount;
	unsigned long totalPacketCount;
	unsigned long totalMappedPacketCount;
	unsigned long totalPicCount;
	unsigned long totalGopCount;
	unsigned long totalSeqCount;
	unsigned long totalDamagedSeqCount;

	// storage for packets fragmented between input buffers
	unsigned char packetFragmentBytes;
	unsigned char packetFragment[ TRANSPORT_PACKET_BYTES ];

	// input bit rate measurement
	unsigned char bpsPcrCount;
	unsigned long bpsPcrPktNumber[ RTF_SES_MAX_BITRATEPCRS ];
	RTF_TIMESTAMP bpsPcrTime[ RTF_SES_MAX_BITRATEPCRS ];

	// input frame rate measurement
	unsigned char fpsPicCount;
	RTF_TIMESTAMP fpsFirstPicTime;
	RTF_TIMESTAMP fpsLastPicTime;

	// embedded objects ***********************************

	// Program Association Table
	RTF_PAT_HANDLE hPat;

	// Program Mapping Table
	RTF_PMT_HANDLE hPmt;

	// Conditional Access Table
	RTF_CAT_HANDLE hCat;

	// PES header
	RTF_PES_HANDLE hPes;

	// Conditional Access System
	RTF_CAS_HANDLE hCas;

	// input transport payload window
	RTF_WIN_HANDLE hWindow;

	// object pools (these are fixed for the lifetime of the session object)
	RTF_BUF_HANDLE hBufPool[ RTF_MAX_SES_BUFS ];
	RTF_PIC_HANDLE hPicPool[ RTF_MAX_SEQ_PICS ];
	RTF_GOP_HANDLE hGopPool[ RTF_MAX_SEQ_GOPS ];

	// input buffer queue
	// Note: may have to keep several mapped buffers until enough info
	// is recorded to perform all necessary fixups for trick file output
	int bufCount;
	RTF_BUF_HANDLE hBuf[ RTF_MAX_SES_BUFS ];

	// picture queue
	// Note: may have to keep several mapped pictures until enough info
	// is recorded to perform all necessary fixups for trick file output
	int picCount;
	RTF_PIC_HANDLE hPic[ RTF_MAX_SEQ_PICS ];

	// picture group queue
	// Note: may have to keep several groups mapped until enough info
	// is recorded to perform all necessary fixups for trick file output
	int gopCount;
	RTF_GOP_HANDLE hGop[ RTF_MAX_SEQ_GOPS ];

	// active video sequence object (only one of these)
	int seqCount;
	RTF_SEQ_HANDLE hSeq;

	// output and related filter objects
	int outputCount;
	RTF_FLT_HANDLE hFilter[ RTF_MAX_OUTPUTCOUNT ];
	RTF_OUT_HANDLE hOutput[ RTF_MAX_OUTPUTCOUNT ];

	// video codec object
	RTF_VCD_HANDLE hVcd;

	// locally cached index mode
	RTF_INDEX_MODE indexMode;
	// locally cached index type
	RTF_INDEX_TYPE indexType;
	// locally cached index option
	RTF_INDEX_OPTION indexOption;
	// index object
	RTF_IDX_HANDLE hIdx;

} RTF_SES;

// forward declarations *****************************************************************

static RTF_RESULT rtfSesAbortTrickFiles( RTF_SES *pSes );

// private functions ********************************************************************

static void rtfSesGetWarningInfo( RTF_SES *pSes, RTF_MSG msg, int inc,
								  int *pCount, int *pThresh, BOOL *pSuppress )
{
	RTF_WARNING_COUNTS *pWarningCounts = &pSes->warningCounts;
	RTF_WARNING_COUNTS *pWarningThresholds = &pSes->warningThresholds;
	BOOL suppress = FALSE;
	int count = 0, thresh = 0, suppressCount = 0;

	// increment the count for this type of warning
	// and get the associated threshold
	switch( msg )
	{
	case RTF_MSG_WRN_UNSUPPORTED:
		count  = pWarningCounts->unsupported += inc;
		thresh = pWarningThresholds->unsupported;
		break;
	case RTF_MSG_WRN_BADADAPT:
		count  = pWarningCounts->badAdapt += inc;
		thresh = pWarningThresholds->badAdapt;
		break;
	case RTF_MSG_WRN_NOPAYLOAD:
		count  = pWarningCounts->noPaylod += inc;
		thresh = pWarningThresholds->noPaylod;
		break;
	case RTF_MSG_WRN_PESNOTCAPTURED:
		count  = pWarningCounts->missingPesHdr += inc;
		thresh = pWarningThresholds->missingPesHdr;
		break;
	case RTF_NSG_WRN_EMPTYPICTURE:
		count  = pWarningCounts->emptyPicture += inc;
		thresh = pWarningThresholds->emptyPicture;
		break;
	case RTF_MSG_WRN_RECORDBUFFULL:
		count  = pWarningCounts->recordFull += inc;
		thresh = pWarningThresholds->recordFull;
		break;
	case RTF_MSG_WRN_FIRSTPICNOTKEY:
		count  = pWarningCounts->firstPicNotKey += inc;
		thresh = pWarningThresholds->firstPicNotKey;
		break;
	case RTF_MSG_WRN_OUTPOINTSKIPPED:
		count  = pWarningCounts->outpointSkipped += inc;
		thresh = pWarningThresholds->outpointSkipped;
		break;
	case RTF_MSG_WRN_PICUNDERFLOW:
		count  = pWarningCounts->picPoolUnderflow += inc;
		thresh = pWarningThresholds->picPoolUnderflow;
		break;
	case RTF_MSG_WRN_PKTOVERFLOW:
		count  = pWarningCounts->pktArrayOverflow += inc;
		thresh = pWarningThresholds->pktArrayOverflow;
		break;
	case RTF_MSG_WRN_PICOVERFLOW:
		count  = pWarningCounts->picArrayOverflow += inc;
		thresh = pWarningThresholds->picArrayOverflow;
		break;
	case RTF_MSG_WRN_GOPOVERFLOW:
		count  = pWarningCounts->gopArrayOverflow += inc;
		thresh = pWarningThresholds->gopArrayOverflow;
		break;
	case RTF_MSG_WRN_BADCAT:
		count  = pWarningCounts->badCat += inc;
		thresh = pWarningThresholds->badCat;
		break;
	case RTF_MSG_WRN_CRYPTOPES:
		count  = pWarningCounts->cryptoPes += inc;
		thresh = pWarningThresholds->cryptoPes;
		break;
	case RTF_MSG_WRN_VARBITRATE:
		count  = pWarningCounts->varBitRate += inc;
		thresh = pWarningThresholds->varBitRate;
		break;
	case RTF_MSG_WRN_PCRMAXGAP:
		count  = pWarningCounts->pcrMaxGap += inc;
		thresh = pWarningThresholds->pcrMaxGap;
		break;
	case RTF_MSG_WRN_NOFIRSTPCRDCON:
		count  = pWarningCounts->noFirstPcrDcon += inc;
		thresh = pWarningThresholds->noFirstPcrDcon;
		break;
	case RTF_MSG_WRN_POSTFIRSTPCRDCON:
		count  = pWarningCounts->postFirstPcrDcon += inc;
		thresh = pWarningThresholds->postFirstPcrDcon;
		break;
	case RTF_MSG_WRN_MULTIPROGSTREAM:
		count  = pWarningCounts->multiProgStream += inc;
		thresh = pWarningThresholds->multiProgStream;
		break;
	case RTF_MSG_WRN_MULTIVIDSTREAM:
		count  = pWarningCounts->multiVidStream += inc;
		thresh = pWarningThresholds->multiVidStream;
		break;
	case RTF_MSG_WRN_MULTIAUDIOCODEC:
		count  = pWarningCounts->multiAudioCodec += inc;
		thresh = pWarningThresholds->multiAudioCodec;
		break;
	case RTF_MSG_WRN_SPLICINGAUDIOBAD:
		count  = pWarningCounts->splicingAudioBad += inc;
		thresh = pWarningThresholds->splicingAudioBad;
		break;
	case RTF_MSG_WRN_SPLICINGPSDESC:
		count  = pWarningCounts->splicingPSDesc += inc;
		thresh = pWarningThresholds->splicingPSDesc;
		break;
	case RTF_MSG_WRN_SPLICINGESDESC:
		count  = pWarningCounts->splicingESDesc += inc;
		thresh = pWarningThresholds->splicingESDesc;
		break;
	case RTF_MSG_WRN_PATCHANGED:
		count  = pWarningCounts->patChanged += inc;
		thresh = pWarningThresholds->patChanged;
		break;
	case RTF_MSG_WRN_CATCHANGED:
		count  = pWarningCounts->catChanged += inc;
		thresh = pWarningThresholds->catChanged;
		break;
	case RTF_MSG_WRN_PMTCHANGED:
		count  = pWarningCounts->pmtChanged += inc;
		thresh = pWarningThresholds->pmtChanged;
		break;
	case RTF_MSG_WRN_PMTENCRYPTED:
		count  = pWarningCounts->pmtEncrypted += inc;
		thresh = pWarningThresholds->pmtEncrypted;
		break;
	case RTF_MSG_WRN_AUGNOTFOUND:
		count  = pWarningCounts->augNotFound += inc;
		thresh = pWarningThresholds->augNotFound;
		break;
	case RTF_MSG_WRN_ZEROOUTPUT:
		count  = pWarningCounts->zeroOutput += inc;
		thresh = pWarningThresholds->zeroOutput;
		break;
	case RTF_MSG_WRN_BADSLICETYPE:
		count  = pWarningCounts->badSliceType += inc;
		thresh = pWarningThresholds->badSliceType;
		break;
	case RTF_MSG_WRN_FRCMISMATCH:
		count  = pWarningCounts->frcMismatch += inc;
		thresh = pWarningThresholds->frcMismatch;
		break;
	case RTF_MSG_WRN_SYNCLOSS:
		count  = pWarningCounts->syncLoss += inc;
		thresh = pWarningThresholds->syncLoss;
		break;
	case RTF_MSG_WRN_SPLITPKT:
		count  = pWarningCounts->splitPkt += inc;
		thresh = pWarningThresholds->splitPkt;
		break;
	case RTF_MSG_WRN_BADTRICKSPEED:
		count  = pWarningCounts->badTrickSpeed += inc;
		thresh = pWarningThresholds->badTrickSpeed;
		break;
	default:
		break;
	}
	suppressCount = pWarningThresholds->suppressCount;
	if( count >= suppressCount )
	{
		// indicate that this warning should be suppressed
		suppress = TRUE;
		// only log a suppression warning on the very first occurrance
		if( ( count == suppressCount ) && ( inc != 0 ) )
		{
			RTF_MSG_PRIO prio;
			char *pShortMsgStr;
			rtfSysGetMsgInfo( msg, &prio, &pShortMsgStr );
			rtfLogMessage( (RTF_HANDLE)pSes, RTF_MSG_INF_SUPPRESSWARNING, "rtfSesGetWarningInfo",
				"Suppression threshold %d reached for warning <%s>. Further warnings of this type will be suppressed for this session",
				suppressCount, pShortMsgStr );
		}
	}
	// make the returns
	*pCount    = count;
	*pThresh   = thresh;
	*pSuppress = suppress;
}

// evaluate a warning. return the following:
// < 0 - this is an error; log it and call the error notifier
// 0 - this is a warning; log it, but don't call the error notifier
// > 0 - this is a suppressed warning; ignore it
int rtfSesWarningEval( RTF_SES_HANDLE handle, RTF_MSG msg )
{
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_MSG_PRIO prio;
	BOOL suppress;
	char *pShortMsg;
	int count, thresh, result = 0;

	// get the priority of this message
	rtfSysGetMsgInfo( msg, &prio, &pShortMsg );
	// decode the priority
	switch( prio )
	{
	case RTF_MSG_PRIO_OK:
	case RTF_MSG_PRIO_INFO:
	case RTF_MSG_PRIO_WARNING_IGNORE:
		// just log it
		break;
	case RTF_MSG_PRIO_WARNING:
		// get the incremented count and threshold value for this warning
		rtfSesGetWarningInfo( pSes, msg, 1, &count, &thresh, &suppress );
		// treat this warning as an error only if:
		// a) there is an active threshold for this error
		// b) the warning count exceeds the threshold
		result = ( ( thresh >= 0 ) && ( count+1 > thresh ) ) ? -1 : ( ( suppress == FALSE ) ? 0 : 1 );
		break;
	case RTF_MSG_PRIO_SESFATAL:
	case RTF_MSG_PRIO_LIBFATAL:
	default:
		// always treat as an error
		result = 1;
		break;
	}

	return result;
}

// respond to an error in this session
static RTF_RESULT rtfSesErrorResponse( RTF_SES *pSes, RTF_MSG msg )
{
	RTF_FNAME( "rtfSesErrorEval" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_MSG_PRIO prio;
	BOOL suppress;
	char *pShortMsg;
	int i, count, thresh;
	char messageBuffer[ 256 ];

	do {		 // error escape wrapper - begin

		// get the priority of this error
		rtfSysGetMsgInfo( msg, &prio, &pShortMsg );
		// decode the priority
		switch( prio )
		{
		case RTF_MSG_PRIO_OK:
		case RTF_MSG_PRIO_INFO:
		case RTF_MSG_PRIO_WARNING_IGNORE:
			// no error, no impact
			break;
		case RTF_MSG_PRIO_WARNING:
			// get the incremented count and threshold value for this warning
			rtfSesGetWarningInfo( pSes, msg, 1, &count, &thresh, &suppress );
			// is there an active threshold for this error?
			if( thresh >= 0 )
			{
				// yes - does the warning count exceed the threshold?
				if( count > thresh )
				{
					sprintf( messageBuffer,
						"Session processing input file %s shutting down because <%s> warning count exceeded threshold (%d)",
						pSes->inputFileName, pShortMsg, thresh );
					rtfLogMessage( (RTF_HANDLE)pSes, RTF_MSG_SESDEATH, __FNAME__, messageBuffer );
					// return failure indication
					result = RTF_FAIL;
					// close the session
					rtfSesClose( (RTF_SES_HANDLE)pSes );
					// call the system-level error notifier
					rtfSysErrorNotifier( messageBuffer );
					break;
				}
			}
			// increment the total warning count
			++pSes->warningCounts.total;
			// is there a total warning threshold?
			if( pSes->warningThresholds.total >= 0 )
			{
				// yes - does the incremented total warning count exceed the threshold?
				if( pSes->warningCounts.total > pSes->warningThresholds.total )
				{
					rtfLogMessage( (RTF_HANDLE)pSes, RTF_MSG_SESDEATH, __FNAME__,
						"Session processing input file %s shutting down because total warning count exceeded threshold (%d)",
						pSes->inputFileName, pSes->warningThresholds.total );
					// return failure indication
					result = RTF_FAIL;
					// iterate over the active outputs
					for( i=0; i<pSes->outputCount; ++i )
					{
						// signal abnormal end of output
						rtfOutAbend( pSes->hOutput[ i ] );
					}
					// close the session
					rtfSesClose( (RTF_SES_HANDLE)pSes );
					// call the system-level error notifier
					rtfSysErrorNotifier( messageBuffer );
					break;
				}
			}
			break;
		case RTF_MSG_PRIO_SESFATAL:
			sprintf( messageBuffer,
				"Session processing input file %s shutting down because of fatal session error",
				pSes->inputFileName ); 
			rtfLogMessage( (RTF_HANDLE)pSes, RTF_MSG_SESDEATH, __FNAME__, messageBuffer );
			// return failure indication
			result = RTF_FAIL;
			// iterate over the active outputs
			for( i=0; i<pSes->outputCount; ++i )
			{
				// signal abnormal end of output
				rtfOutAbend( pSes->hOutput[ i ] );
			}
			// close the session
			rtfSesClose( (RTF_SES_HANDLE)pSes );
			// call the system-level error notifier
			rtfSysErrorNotifier( messageBuffer );
			break;
		case RTF_MSG_PRIO_LIBFATAL:
			sprintf( messageBuffer, "Trickfile library shutting down because of fatal library error" );
			rtfLogMessage( (RTF_HANDLE)pSes, RTF_MSG_LIBDEATH, __FNAME__, messageBuffer );
			rtfShutdownLibrary();
			result = RTF_FAIL;
			break;
		default:
			// internal error - treat as library fatal
			sprintf( messageBuffer, "Unrecognized error priority (%d) on message (%s)", prio, pShortMsg );
			rtfLogMessage( (RTF_HANDLE)pSes, RTF_MSG_LIBDEATH, __FNAME__, messageBuffer );
			rtfShutdownLibrary();
			result = RTF_FAIL;
			break;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// sort an array by increasing absolute value
void rtfAbsSort( int numEntries, int *pArray )
{
	int i, j, min, temp;

	for( i=0; i<numEntries-1; ++i )
	{
		min = i;
		for( j=i+1; j<numEntries; ++j )
		{
			if( pArray[ j ] < pArray[ i ] )
			{
				min = j;
			}
		}
		if( min != i )
		{
			temp = pArray[ i ];
			pArray[ i ] = pArray[ j ];
			pArray[ j ] = temp;
		}
	}
}

#ifdef DO_TRACKREFCOUNTS
static RTF_RESULT rtfPrintReferences( RTF_SES *pSes )
{
	RTF_FNAME( "rtfPrintReferences" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_HANDLE hBufRefHolder;
	RTF_HANDLE hPktRefHolder;
	unsigned long first, last;
	unsigned long bufRefCount, bufNumber;
	unsigned long pktRefCount, pktNumber;
	char *pStr;
	int i, j, k;
	unsigned short count;

	do {		 // error escape wrapper - begin

		PRINTF( "%d pictures\n", pSes->picCount );
		for( i=0; i<pSes->picCount; ++i )
		{
			result = rtfPicGetPacketCount( pSes->hPic[ i ], &count );
			RTF_CHK_RESULT;
			result = rtfPicGetFirstPktMapNum( pSes->hPic[i], &first );
			RTF_CHK_RESULT;
			result = rtfPicGetLastPktNum( pSes->hPic[i], &last );
			RTF_CHK_RESULT;
			PRINTF( "Pic %d: %d packets. First=%d Last=%d\n", i, count, first, last );
		}
		RTF_CHK_RESULT_LOOP;
		PRINTF( "%d buffers\n", pSes->bufCount );
		for( i=0; i<pSes->bufCount; ++i )
		{
			result = rtfBufGetReferenceCount( pSes->hBuf[ i ], &bufRefCount );
			RTF_CHK_RESULT;
			PRINTF( "buffer %d: refCount=%d\n", i, bufRefCount );
			for( j=0; j<(int)bufRefCount; ++j )
			{
				result = rtfBufGetNumber( pSes->hBuf[ i ], &bufNumber );
				RTF_CHK_RESULT;
				result = rtfBufGetRefHandle( pSes->hBuf[ i ], j, &hBufRefHolder );
				RTF_CHK_RESULT;
				result = rtfObjGetTypeStr( *( (RTF_OBJ_HANDLE *)hBufRefHolder ), &pStr );
				RTF_CHK_RESULT;
				if( stricmp( pStr, "packet" ) != 0 )
				{
					PRINTF( "Buffer %d (number %d) reference %d held by object type %s\n",
							i, bufNumber, j, pStr );
				}
				else
				{
					result = rtfPktGetInpPktNumber( (RTF_PKT_HANDLE)hBufRefHolder, &pktNumber );
					RTF_CHK_RESULT;
					PRINTF( "Buffer %d reference %d held by packet %d\n", i, j, pktNumber );
					result = rtfPktGetRefCount( (RTF_PKT_HANDLE)hBufRefHolder, &pktRefCount );
					RTF_CHK_RESULT;
					for( k=0; k<(int)pktRefCount; ++k )
					{
						result = rtfPktGetRefHandle( (RTF_PKT_HANDLE)hBufRefHolder, (unsigned long)k, &hPktRefHolder );
						RTF_CHK_RESULT;
						result = rtfObjGetTypeStr( hPktRefHolder, &pStr );
						RTF_CHK_RESULT;
						PRINTF( "Packet %d reference %d held by %s\n",
								pktNumber, k, pStr );
					}
				}
			}
			RTF_CHK_RESULT_LOOP;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}
#endif // #ifdef DO_TRACKREFCOUNTS

static RTF_RESULT rtfQueueBuf( RTF_SES *pSes )
{
	RTF_FNAME( "rtfQueueBuf" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_BUFSTATE state;
	unsigned long i;

	do {		 // error escape wrapper - begin

		// find a released buffer in the buffer pool
		for( i=0; i<pSes->maxInputBuffers; ++i )
		{
			result = rtfBufGetState( pSes->hBufPool[ i ], &state );
			RTF_CHK_RESULT;
			if( state == RTF_BUFSTATE_RELEASED )
			{
				break;
			}
		}
		RTF_CHK_RESULT_LOOP;
		// make sure we found one
		if( i >= pSes->maxInputBuffers )
		{
			// buffer underflow - is there an obvious explanation?
			if( pSes->inputInSync == FALSE )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unable to sync to input stream %s", pSes->inputFileName );
				break;
			}
			if( pSes->psiValid == FALSE )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unable to acquire PSI from input stream %s", pSes->inputFileName );
				break;
			}
			if( pSes->casReady == FALSE )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unable to recognize CA system used in input stream %s", pSes->inputFileName );
				break;
			}
			if( ( pSes->profile.videoSpec.pid != TRANSPORT_INVALID_PID ) &&
				( pSes->frameRateAcquired == FALSE ) )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unable to measure video frame rate for input stream %s", pSes->inputFileName );
				break;
			}
			if( ( pSes->profile.videoSpec.pid != TRANSPORT_INVALID_PID ) &&
				( pSes->pesHeaderAcquired == FALSE ) )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unable to capture PES header from input stream %s", pSes->inputFileName );
				break;
			}
			if( pSes->indexLibraryIntialized == FALSE )
			{
				RTF_LOG_ERR1( RTF_MSG_ERR_BADSTREAM, "Unable to initialize indexer for input stream %s", pSes->inputFileName );
				break;
			}
#ifdef DO_TRACKREFCOUNTS
			rtfPrintReferences( pSes );
#endif
			RTF_LOG_ERR0( RTF_MSG_ERR_UNDERFLOW, "Buffer pool underflow" );
			break;
		} // if( i >= pSes->maxInputBuffers )
		// add this pool buffer to the end of the buffer queue
		pSes->hBuf[ pSes->bufCount++ ] = pSes->hBufPool[ i ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfPurgeBufQueue( RTF_SES *pSes )
{
	RTF_FNAME( "rtfPurgeBufQueue" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_BUFSTATE state;
	RTF_APP_SESSION_HANDLE hAppSession;
	RTF_APP_FILE_HANDLE hAppFile;
	RTF_APP_BUFFER_HANDLE hAppBuffer;
	unsigned long bufferNumber, oldestBufferNumber;
	unsigned long capacity, occupancy;
	unsigned long referenceCount;
	unsigned char *pBuffer;
	int i;

	do {		 // error escape wrapper - begin

		// get the number of the oldest buffer currently
		// referenced by the session's scanning window
		result = rtfWinGetOldestBufferNumber( pSes->hWindow, &oldestBufferNumber );
		RTF_CHK_RESULT;
		// purge the buffer queue of zero-reference buffers
		// NOTE: these are "leftover" buffers carrying audio and / or non-keyframe video
		// packets that are not referenced by any of the pictures on the active list
		// However, don't purge any buffers that may be referenced by the scanning window
		for( i=0; i<pSes->bufCount-1; ++i )
		{
			// get the number of this buffer
			result = rtfBufGetNumber( pSes->hBuf[ i ], &bufferNumber );
			// might this buffer be referenced by the scanning window?
			if( bufferNumber >= oldestBufferNumber )
			{
				continue;
			}
			result = rtfBufGetReferenceCount( pSes->hBuf[ i ], &referenceCount );
			RTF_CHK_RESULT;
			if( referenceCount == 0 )
			{
				// get the mapping info for this buffer
				result = rtfBufGetMapInfo( pSes->hBuf[ i ], &state, &hAppSession,
										   &hAppFile, &hAppBuffer, &bufferNumber,
										   &pBuffer, &capacity, &occupancy );
				RTF_CHK_RESULT;
				// release the storage back to the application
				if( pSes->inputBufferReleaseCallback( hAppSession, hAppFile, hAppBuffer, pBuffer ) != 0 )
				{
					RTF_LOG_ERR0( RTF_MSG_ERR_CALLBACKFAILED, "Buffer release callback failed" );
					break;
				}
				// reset the buffer object
				result = rtfBufReset( pSes->hBuf[ i ], FALSE );
				RTF_CHK_RESULT;
				// remove it from the queue (make sure the count does not go negative)
				if( --pSes->bufCount < 0 )
				{
					RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Negative buffer count" );
					break;
				}
				for( ; i<pSes->bufCount; ++i )
				{
					pSes->hBuf[ i ] = pSes->hBuf[ i+1 ];
				}
				pSes->hBuf[ i ] = (RTF_BUF_HANDLE)NULL;
			}
		}
		RTF_CHK_RESULT_LOOP;

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfQueuePic( RTF_SES *pSes )
{
	RTF_FNAME( "rtfQueuePic" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_PICSTATE state;
	unsigned long i;

	do {		 // error escape wrapper - begin

		// find a released picture in the pic pool
		for( i=0; i<pSes->maxPictures; ++i )
		{
			result = rtfPicGetState( pSes->hPicPool[ i ], &state );
			RTF_CHK_RESULT;
			if( state == RTF_PICSTATE_RELEASED )
			{
				break;
			}
		}
		RTF_CHK_RESULT_LOOP;
		// was there one?
		if( i >= pSes->maxPictures )
		{
			// no. is abort video on picture pool underflow set?
			if( pSes->abortVidOnPPU != 0 )
			{
				// yes - abort the trick files
				result = rtfSesAbortTrickFiles( pSes );
				RTF_CHK_RESULT;
				break;
			}
			else
			{
				// no. mark the current group as damaged
				RTF_LOG_WARN1( RTF_MSG_WRN_PICUNDERFLOW, "Picture pool underflow at picture %d", pSes->totalPicCount );
				result = rtfGopSetIsDamaged( pSes->hGop[ pSes->gopCount - 1 ], TRUE );
				RTF_CHK_RESULT;			
				// in order to continue parsing, relase the last picture in the active queue
				result = rtfPicRelease( pSes->hPic[ --pSes->picCount ], (RTF_SES_HANDLE)pSes );
				RTF_CHK_RESULT;			
				// use the picture object just freed to parse the new picture
				--i;
			}
		}
		// make sure there is room for one more picture in the queue
		if( pSes->picCount >= RTF_MAX_SEQ_PICS )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_UNDERFLOW, "Picture pool overflow" );
			break;
		}
		// add this pool picture to the end of the picture queue
		pSes->hPic[ pSes->picCount++ ] = pSes->hPicPool[ i ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

static RTF_RESULT rtfQueueGop( RTF_SES *pSes )
{
	RTF_FNAME( "rtfQueueGop" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_GOPSTATE state;
	unsigned long i;

	do {		 // error escape wrapper - begin

		// find a released group in the group pool
		for( i=0; i<pSes->maxGroups; ++i )
		{
			result = rtfGopGetState( pSes->hGopPool[ i ], &state );
			RTF_CHK_RESULT;
			if( state == RTF_GOPSTATE_RELEASED )
			{
				break;
			}
		}
		RTF_CHK_RESULT_LOOP;
		// make sure we found one
		if( i >= pSes->maxGroups )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_UNDERFLOW, "GOP pool underflow" );
			break;
		}
		// add this pool group to the end of the GOP queue
		pSes->hGop[ pSes->gopCount++ ] = pSes->hGopPool[ i ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process the packets in an unprocessed buffer
// scan unencrypted transport packet payloads for start codes and process them
static RTF_RESULT rtfProcessPackets( RTF_SES *pSes, RTF_BUF_HANDLE hBuf )
{
	RTF_FNAME( "rtfProcessPackets" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_BUFSTATE bufState;
	BOOL warning;
	unsigned long data;
	int i;

	do {		 // error escape wrapper - begin

		// is a copy of the main file being written?
		if( pSes->mainFileOutputNumber >= 0 )
		{
			// yes - is the session ready for streaming?
			if( pSes->readyForStreaming != FALSE )
			{
				// yes - is there a backlog of buffers to be sent to the main file copy?
				if( pSes->mainFileBacklogProcessed == FALSE )
				{
					// yes - iterate over the buffers in the backlog
					for( i=0; i<pSes->bufCount; ++i )
					{
						// was this buffer already processed?
						result = rtfBufGetState( pSes->hBuf[ i ], &bufState );
						RTF_CHK_RESULT;
						if( bufState == RTF_BUFSTATE_PROCESSED )
						{
							// yes - filter the contents of this buffer to the main output file
							result = rtfFltBuf( pSes->hFilter[ pSes->mainFileOutputNumber ],
												pSes->hBuf[ i ] );
							RTF_CHK_RESULT;
						}
					}
					pSes->mainFileBacklogProcessed = TRUE;
				}
				// filter the contents of this buffer to the main file copy
				result = rtfFltBuf( pSes->hFilter[ pSes->mainFileOutputNumber ], hBuf );
				RTF_CHK_RESULT;
			}
		}
		// escape if there is no video PID
		if( pSes->profile.videoSpec.pid == TRANSPORT_INVALID_PID )
		{
			break;
		}
		// point the scanning window at this buffer
		result = rtfWinMapBuffer( pSes->hWindow, pSes->profile.videoSpec.pid, hBuf, &warning );
		RTF_CHK_RESULT;
		// escape if this buffer immediately returns a "last packet" warning flag
		if( warning != FALSE )
		{
			break;
		}
		// scan the payload of all unencrypted video packets
		// process any start codes found
		for( i=0; ; ++i )
		{
			// move the payload window forward to the next start code
			result = rtfWinFindNextStartCode( pSes->hWindow, &data );
			RTF_CHK_RESULT;
			// was the last packet warning flag set (i.e. no start code found)?
			result = rtfWinGetLastPacketWarningFlag( pSes->hWindow, &warning );
			RTF_CHK_RESULT;
			if( warning != FALSE )
			{
				// yes - escape the search loop and load the next buffer
				break;
			}
			// process the start code
			result = rtfVcdProcessStartCode( pSes->hVcd, data );
			RTF_CHK_RESULT;
			// was there a last packet warning while processing the start code?
			result = rtfWinGetLastPacketWarningFlag( pSes->hWindow, &warning );
			RTF_CHK_RESULT;
			if( warning != FALSE )
			{
				// yes - escape the search loop and load the next buffer
				break;
			}
		} // for( i=0; ; ++i )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process all unprocessed buffers
static RTF_RESULT rtfProcessBuffers( RTF_SES *pSes )
{
	RTF_FNAME( "rtfProcessBuffers" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_BUFSTATE bufState;
	RTF_SEQSTATE seqState;
	RTF_BUF_HANDLE hBuf;
	BOOL pesValid;
	BOOL isVirtual;
	int i, j, k;

	do {		 // error escape wrapper - begin

		// process all unprocessed buffers
		// note: since buffers can be released due to end-of-seq processing,
		// queue may collapse. So keep looking for unprocessed buffers from
		// the start of the queue until none are found. Be sure to cache the
		// handle of the buffer we are processing, rather than the index, as
		// the index could change
		for( j=0; ; ++j )
		{
			for( i=0; i<pSes->bufCount; ++i )
			{
				// has the next buffer been processed?
				hBuf = pSes->hBuf[ i ];
				result = rtfBufGetState( hBuf, &bufState );
				RTF_CHK_RESULT;
				if( bufState == RTF_BUFSTATE_MAPPED )
				{
					// no - has the index library context been initialized yet?
					if( pSes->indexLibraryIntialized == FALSE )
					{
						// no - at this point, PSI and input bitrate have been captured
						// intercept and handle special case of stream with no video PID
						if( pSes->profile.videoSpec.pid == TRANSPORT_INVALID_PID )
						{
							// open the index object for the requested indexing method
							result = rtfIdxOpen( pSes->hIdx, pSes->indexMode, pSes->indexType, pSes->indexOption,
												 pSes->inputFileName, pSes->inputFileBytes, pSes->outputCount,
												 pSes->indexFileOutputNumber, pSes->mainFileOutputNumber,
												 (RTF_SES_HANDLE)pSes, pSes->hVcd, pSes->hPat, pSes->hPmt,
												 pSes->hPes, pSes->hSeq, pSes->hOutput, pSes->hFilter );
							RTF_CHK_RESULT;
							// signal the output filters to prepare for streaming
							for( k=0; k<pSes->outputCount; ++k )
							{
								if( k != pSes->indexFileOutputNumber )
								{
									result = rtfFltPrepareForStreaming( pSes->hFilter[ k ] );
									RTF_CHK_RESULT;
								}
							}
							RTF_CHK_RESULT_LOOP;
							pSes->indexLibraryIntialized = TRUE;
							pSes->readyForStreaming = TRUE;
							continue;
						}
						// has a PES header been captured?
						result = rtfPesValidate( pSes->hPes, &pesValid );
						RTF_CHK_RESULT;
						if( pesValid != FALSE )
						{
							// yes - do we have an active sequence?
							result = rtfSeqGetState( pSes->hSeq, &seqState );
							RTF_CHK_RESULT;
							if( seqState == RTF_SEQSTATE_OPEN )
							{
								// yes - for MPEG-2, we require a sequence header in order
								// to open the index file. Is the active sequence non-virtual?
								// or is the codec non-MPEG2 ? (final ingredient)
								result = rtfSeqGetIsVirtual( pSes->hSeq, &isVirtual );
								RTF_CHK_RESULT;
								if( ( isVirtual == FALSE ) ||
									( pSes->profile.videoSpec.eStream.video.vcdType != RTF_VIDEO_CODEC_TYPE_MPEG2 ) )
								{
									// yes - we are ready to roll.
									// get the codec to generate a no-change frame
									result = rtfVcdSetupNCFrame( pSes->hVcd );
									RTF_CHK_RESULT;
									// open the index object for the requested indexing method
									result = rtfIdxOpen( pSes->hIdx, pSes->indexMode, pSes->indexType, pSes->indexOption,
														 pSes->inputFileName, pSes->inputFileBytes, pSes->outputCount,
														 pSes->indexFileOutputNumber, pSes->mainFileOutputNumber,
														 (RTF_SES_HANDLE)pSes, pSes->hVcd, pSes->hPat, pSes->hPmt,
														 pSes->hPes, pSes->hSeq, pSes->hOutput, pSes->hFilter );
									RTF_CHK_RESULT;
									// signal the output filters to prepare for streaming
									for( k=0; k<pSes->outputCount; ++k )
									{
										if( k != pSes->indexFileOutputNumber )
										{
											result = rtfFltPrepareForStreaming( pSes->hFilter[ k ] );
											RTF_CHK_RESULT;
										}
									}
									RTF_CHK_RESULT_LOOP;
									// set the index context initialized flag
									pSes->indexLibraryIntialized = TRUE;
									// the session is now ready for streaming
									pSes->readyForStreaming = TRUE;
								} // if( isVirtual == FALSE )
							} // if( seqState == RTF_SEQSTATE_OPEN )
						} // if( pesValid != FALSE )
					} // if( pSes->indexLibraryIntialized == FALSE )
					// process the packets in this buffer
					result = rtfProcessPackets( pSes, hBuf );
					RTF_CHK_RESULT;
					// set the buffer state to "processed"
					result = rtfBufSetState( hBuf, RTF_BUFSTATE_PROCESSED );
					RTF_CHK_RESULT;
					// escape the inner search loop
					// (start looking for unprocessed buffers
					// at the beginning of the queue again)
					break;
				} // if( bufState == RTF_BUFSTATE_MAPPED )
			} // for( i=0; i<pSes->bufCount; ++i )
			RTF_CHK_RESULT_LOOP;
			// no more unprocessed buffers?
			if( i >= pSes->bufCount )
			{
				// yes - escape the outer search loop
				break;
			}
		} // for( k=0; ; ++k )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// compute the input bit rate from PCRs
// note: the measured rate does not include any
//       FEC packets that may appear in the stream
// note: the measured rate does not include any
//       TTS packet prefixes that may be present
static RTF_RESULT rtfComputeInputBitRate( RTF_SES *pSes )
{
	RTF_FNAME( "rtfComputeInputBitRate" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	INT64 deltaT;
	INT64 deltaP;
	int i;
	unsigned long ulTemp, maxDiff;
	unsigned char last = pSes->bpsPcrCount-1;

	do {		 // error escape wrapper - begin

		// compute the delta T between the first and last PCR recorded
		deltaT  = ( ( pSes->bpsPcrTime[ last ].base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) + pSes->bpsPcrTime[ last ].ext.us );
		deltaT -= ( ( pSes->bpsPcrTime[    0 ].base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) + pSes->bpsPcrTime[    0 ].ext.us );
		// make sure deltaT is non-zero
		if( deltaT == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Constant PCRs" );
			break;
		}
		// compute the number of packets between the first and last PCR recorded
		deltaP = (INT64)( pSes->bpsPcrPktNumber[ last ] - pSes->bpsPcrPktNumber[ 0 ] );
		// compute the average input bit rate based on these values
		deltaP *= (INT64)TRANSPORT_PACKET_BITS * TRANSPORT_SCR_TICKS_PER_SECOND;
		pSes->profile.bitsPerSecond = (unsigned long)RTF_DIV64( deltaP, deltaT );
		// skip the variable bit rate test if augmentation is present
		if( pSes->profile.augmentationPidCount == 0 )
		{
			RTF_LOG_INFO1( RTF_MSG_INF_STATS, "Input bit rate estimate is %d bits per second", pSes->profile.bitsPerSecond );
			// iterate over the individual PCR intervals and check the bit rate against the average
			maxDiff = 0;
			for( i=1; i<last; ++i )
			{
				deltaT = ( ( pSes->bpsPcrTime[ i   ].base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) + pSes->bpsPcrTime[ i ].ext.us ) -
						 ( ( pSes->bpsPcrTime[ i-1 ].base.ull * TRANSPORT_SCR_TO_PCR_RATIO ) + pSes->bpsPcrTime[ i-1 ].ext.us );
				if( deltaT > TRANSPORT_MAX_PCR_GAP_TICKS )
				{
					RTF_LOG_WARN2( RTF_MSG_WRN_PCRMAXGAP, "PCR gap (%d ticks) exceeds max (%d ticks)",
								(int)deltaT, TRANSPORT_MAX_PCR_GAP_TICKS );
				}
				deltaP = (INT64)( pSes->bpsPcrPktNumber[ i ] - pSes->bpsPcrPktNumber[ i-1 ] );
				deltaP *= ( (INT64)TRANSPORT_PACKET_BYTES ) * 8 * TRANSPORT_SCR_TICKS_PER_SECOND;
				ulTemp = (unsigned long)RTF_DIV64( deltaP, deltaT );
				ulTemp = MAX( ulTemp, pSes->profile.bitsPerSecond ) - MIN( ulTemp, pSes->profile.bitsPerSecond );
				if( ulTemp > maxDiff )
				{
					maxDiff = ulTemp;
				}
			}
			// allow a very small variation (less than 1%)
			if( ulTemp > ( ( pSes->profile.bitsPerSecond + 0x8000 ) >> pSes->profile.log2VbrThreshold ) )
			{
				RTF_LOG_WARN1( RTF_MSG_WRN_VARBITRATE, "Stream is VBR - maxDiff = %d bits per second over sample", maxDiff );
			}
		}
		// calculate the offset from the start of the stream in PCR ticks
		deltaT = pSes->bpsPcrPktNumber[ 0 ] * TRANSPORT_PACKET_BITS;	// = bits
		deltaT *= TRANSPORT_SCR_TICKS_PER_SECOND;						// = bits * ticks / sec
		deltaT /= pSes->profile.bitsPerSecond;							// = ticks
		// subtract that from the first PCR value
		deltaT -= pSes->bpsPcrTime[ 0 ].ext.us;
		deltaT -= pSes->bpsPcrTime[ 0 ].base.ull * TRANSPORT_SCR_TO_PCR_RATIO;
		// save the hypothetical first packet PCR value in the profile
		pSes->profile.streamPcrBase = deltaT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// compute the input frame rate from frame starts and time stamps
// note: the measured rate does not include any
//       FEC packets that may appear in the stream
static RTF_RESULT rtfComputeInputFrameRate( RTF_SES *pSes )
{
	RTF_FNAME( "rtfComputeInputFrameRate" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	INT64 num;
	int denom, fpsFix16;
	int tmpDiff, minDiff;
	int i, frc;

	do {		 // error escape wrapper - begin

		// compute the frame rate based on the picture count and
		// the difference between the first and last PTS values
		num = RTF_SES_MAX_FRAMERATEPICS - 2;
		num *= TRANSPORT_PCR_TICKS_PER_SECOND;
		num <<= 16;
		denom = (int)( pSes->fpsLastPicTime.base.ull - pSes->fpsFirstPicTime.base.ull );
		fpsFix16 = (int)( num / denom );
		// find the frame rate code that most closely aproaches the measured frame rate
		minDiff = 0x7FFFFFFF;
		frc = 0;
		for( i=0; i<=RTF_MAX_FRAME_RATE_CODE; ++i )
		{
			tmpDiff = ABS( fpsFix16 - frcFix16[ i ] );
			if( tmpDiff < minDiff )
			{
				frc = i;
				minDiff = tmpDiff;
			}
		}
		RTF_LOG_INFO1( RTF_MSG_INF_STATS, "Input frame rate estimate is %s fps",
					   frcString[ frc ] );
		// set the frame rate code
		result = rtfSesSetFrameRateCode( (RTF_SES_HANDLE)pSes, frc );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// release the current sequence (allows buffers to be recycled)
static RTF_RESULT rtfSesReleaseSeq( RTF_SES *pSes )
{
	RTF_FNAME( "rtfSesReleaseSeq" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// release the sequence (allows input buffers to be recycled)
		result = rtfSeqRelease( pSes->hSeq, (RTF_SES_HANDLE)pSes );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// process a completed video sequence
static RTF_RESULT rtfSesProcessSeq( RTF_SES *pSes )
{
	RTF_FNAME( "rtfSesProcessSeq" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_GOP_HANDLE *phGop;
	RTF_PIC_HANDLE *phPic;
	unsigned char gopCount, picCount;
	int i, j;

	do {		 // error escape wrapper - begin

		// get the group array info from the sequence
		result = rtfSeqGetGopArrayInfo( pSes->hSeq, &gopCount, &phGop );
		RTF_CHK_RESULT;
		// iterate over the GOPs in the sequence and process them
		for( i=0; i<gopCount; ++i )
		{
			// get the number of pictures in this group
			result = rtfGopGetPicArrayInfo( phGop[ i ], &picCount, &phPic );
			RTF_CHK_RESULT;
			// were there any?
			if( picCount == 0 )
			{
				// no - skip this group
				continue;
			}
			// record index info before the group has been filtered
			result = rtfIdxProcessBeforeGroup( pSes->hIdx, phGop[ i ] );
			RTF_CHK_RESULT;
			// send this group to the trick file output filters
			for( j=0; j<pSes->outputCount; ++j )
			{
				if( ( j != pSes->indexFileOutputNumber ) &&
					( j != pSes->mainFileOutputNumber  ) )
				{
					result = rtfFltGop( pSes->hFilter[ j ], phGop[ i ] );
					RTF_CHK_RESULT;
				}
			}
			RTF_CHK_RESULT_LOOP;
			// record index info after the group has been filtered
			result = rtfIdxProcessAfterGroup( pSes->hIdx );
			RTF_CHK_RESULT;
		} // for( i=0; i<gopCount; ++i )
		RTF_CHK_RESULT_LOOP;
		// release the sequence
		result = rtfSesReleaseSeq( pSes );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// abort all trick files
static RTF_RESULT rtfSesAbortTrickFiles( RTF_SES *pSes )
{
	RTF_FNAME( "rtfSesAbortTrickFiles" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	RTF_IDX_STATE idxState;
	int i, killCount;

	do {		 // error escape wrapper - begin

		// release the current sequence
		result = rtfSesReleaseSeq( pSes );
		RTF_CHK_RESULT;
		// close all of the filters and outputs associated with trick files
		killCount = 0;
		for( i=0; i<pSes->outputCount; ++i )
		{
			if( ( i == pSes->indexFileOutputNumber ) ||
				( i == pSes->mainFileOutputNumber  ) )
			{
				continue;
			}
			++killCount;
			result = rtfFltClose( pSes->hFilter[ i ] );
			RTF_CHK_RESULT;
			result = rtfOutClose( pSes->hOutput[ i ] );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// reduce the output count
		pSes->outputCount -= killCount;
		// is the indexer open?
		result = rtfIdxGetState( pSes->hIdx, &idxState );
		RTF_CHK_RESULT;
		if( idxState == RTF_IDX_STATE_OPEN )
		{
			// tell the indexer to abort the trickfiles
			result = rtfIdxAbortTrickFiles( pSes->hIdx );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfSesGetStorageRequirement( unsigned long maxInputBuffers, unsigned long maxGroupsPerSequence,
										   unsigned long maxPicturesPerGroup, unsigned long maxInputBufferBytes )
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfSesGetStorageRequirement" );
	unsigned long bytes;
	unsigned short maxInputBufferPacketCount;

	bytes = sizeof(RTF_SES);
	bytes += rtfObjGetStorageRequirement();
	maxInputBufferPacketCount = (unsigned short)( ( maxInputBufferBytes + TRANSPORT_PACKET_BYTES - 1 ) / TRANSPORT_PACKET_BYTES );
	bytes += maxInputBuffers * rtfBufGetStorageRequirement( RTF_BUFTYPE_INPUT, maxInputBufferPacketCount );
	bytes += rtfWinGetStorageRequirement();
	bytes += RTF_MAX_OUTPUTCOUNT * rtfOutGetStorageRequirement();
	bytes += rtfPatGetStorageRequirement();
	bytes += rtfPmtGetStorageRequirement();
	bytes += rtfPesGetStorageRequirement();
	bytes += rtfCasGetStorageRequirement();
	bytes += RTF_MAX_OUTPUTCOUNT * rtfFltGetStorageRequirement();
	bytes += maxPicturesPerGroup * maxGroupsPerSequence * rtfPicGetStorageRequirement();
	bytes += maxGroupsPerSequence * rtfGopGetStorageRequirement();
	bytes += rtfSeqGetStorageRequirement();
	bytes += rtfVcdGetStorageRequirement();
	bytes += rtfIdxGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfSesConstructor( RTF_SES_HANDLE *pHandle,
							  unsigned long maxInputBuffers, unsigned long maxGroupsPerSequence,
							  unsigned long maxPicturesPerGroup, unsigned long maxInputBufferBytes )
{
	RTF_FNAME( "rtfSesConstructor" );
	RTF_OBASE( NULL );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes;
	unsigned short maxInputBufferPacketCount;
	unsigned long i;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the session object
		pSes = (RTF_SES *)rtfAlloc( sizeof(RTF_SES) );
		RTF_CHK_ALLOC( pSes );
		// return the handle
		*pHandle = (RTF_SES_HANDLE)pSes;
		// clear the state structure
		memset( (void *)pSes, 0, sizeof(*pSes) );
		// create an embedded base object
		result = rtfObjConstructor( RTF_OBJ_TYPE_SES, (RTF_HANDLE)pSes, (RTF_HANDLE)NULL, &pSes->hBaseObject );
		RTF_CHK_RESULT;
		// record the max buffer count
		pSes->maxInputBuffers = maxInputBuffers;
		// create a pool of input buffer objects for the session
		maxInputBufferPacketCount = (unsigned short)( ( maxInputBufferBytes + TRANSPORT_PACKET_BYTES - 1 ) / TRANSPORT_PACKET_BYTES );
		for( i=0; i<maxInputBuffers; ++i )
		{
			result = rtfBufConstructor( &pSes->hBufPool[i], RTF_BUFTYPE_INPUT,
										maxInputBufferPacketCount, (RTF_HANDLE)pSes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// create the input packet payload scanning window
		result = rtfWinConstructor( &pSes->hWindow, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create a set of output objects
		for( i=0; i<RTF_MAX_OUTPUTCOUNT; ++i )
		{
			result = rtfOutConstructor( &pSes->hOutput[i], (RTF_HANDLE)pSes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// create a PAT object
		result = rtfPatConstructor( &pSes->hPat, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create a CAT object
		result = rtfCatConstructor( &pSes->hCat, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create a PMT object
		result = rtfPmtConstructor( &pSes->hPmt, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create a PES object
		result = rtfPesConstructor( &pSes->hPes, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create a CAS object
		result = rtfCasConstructor( &pSes->hCas, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create a set of filter objects, one for each output
		for( i=0; i<RTF_MAX_OUTPUTCOUNT; ++i )
		{
			result = rtfFltConstructor( &pSes->hFilter[i], (RTF_HANDLE)pSes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// record the maximum number of pictures per group
		pSes->maxPictures = maxPicturesPerGroup * maxGroupsPerSequence;
		// create a pool of picture objects for the session
		for( i=0; i<pSes->maxPictures; ++i )
		{
			result = rtfPicConstructor( &pSes->hPicPool[i], (RTF_HANDLE)pSes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// record the maximum number of groups per sequence
		pSes->maxGroups = maxGroupsPerSequence;
		// create a pool of group of pictures objects for the session
		for( i=0; i<pSes->maxGroups; ++i )
		{
			result = rtfGopConstructor( &pSes->hGopPool[i], (RTF_HANDLE)pSes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// create the video sequence object
		result = rtfSeqConstructor( &pSes->hSeq, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create a codec object
		result = rtfVcdConstructor( &pSes->hVcd, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// create an index object
		result = rtfIdxConstructor( &pSes->hIdx, (RTF_HANDLE)pSes );
		RTF_CHK_RESULT;
		// reset the index and main file IDs
		pSes->indexFileOutputNumber = -1;
		pSes->mainFileOutputNumber = -1;
		// set the state to idle
		pSes->state = RTF_SES_STATE_IDLE;

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfSesDestructor( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesDestructor" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	unsigned long i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// if the session isn't already idle, close it
		if ( pSes->state != RTF_SES_STATE_IDLE )
		{
			result = rtfSesClose( handle );
			RTF_CHK_RESULT;
		}
		// destroy the pool of input buffer objects
		for( i=0; i<pSes->maxInputBuffers; ++i )
		{
			result = rtfBufDestructor( pSes->hBufPool[i] );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// destroy the input packet payload scanning window
		result = rtfWinDestructor( pSes->hWindow );
		RTF_CHK_RESULT;
		// destroy the set of output filter objects
		for( i=0; i<RTF_MAX_OUTPUTCOUNT; ++i )
		{
			result = rtfFltDestructor( pSes->hFilter[i] );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// destroy the set of output objects
		for( i=0; i<RTF_MAX_OUTPUTCOUNT; ++i )
		{
			result = rtfOutDestructor( pSes->hOutput[i] );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// destroy the PAT object
		result = rtfPatDestructor( pSes->hPat);
		RTF_CHK_RESULT;
		// destroy the CAT object
		result = rtfCatDestructor( pSes->hCat);
		RTF_CHK_RESULT;
		// destroy the PMT object
		result = rtfPmtDestructor( pSes->hPmt);
		RTF_CHK_RESULT;
		// destroy the CAS object
		result = rtfCasDestructor( pSes->hCas );
		RTF_CHK_RESULT;
		// destroy the pool of picture objects
		for( i=0; i<pSes->maxPictures; ++i )
		{
			result = rtfPicDestructor( pSes->hPicPool[i] );
			RTF_CHK_RESULT;
		}
		// destroy the pool of gop objects
		for( i=0; i<pSes->maxGroups; ++i )
		{
			result = rtfGopDestructor( pSes->hGopPool[i] );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// destroy the video sequence object
		result = rtfSeqDestructor( pSes->hSeq );
		RTF_CHK_RESULT;
		// destroy the codec object
		result = rtfVcdDestructor( pSes->hVcd );
		RTF_CHK_RESULT;
		// destroy the index object
		result = rtfIdxDestructor( pSes->hIdx );
		RTF_CHK_RESULT;
		// destroy the embedded base object
		result = rtfObjDestructor( pSes->hBaseObject, RTF_OBJ_TYPE_SES );
		RTF_CHK_RESULT;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the application-supplied session handle
RTF_RESULT rtfSesGetAppHandle( RTF_SES_HANDLE handle, RTF_APP_SESSION_HANDLE *phAppSession )
{
	RTF_FNAME( "rtfSesGetAppHandle" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*phAppSession = pSes->hAppSession;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the state of the session object
RTF_RESULT rtfSesGetState( RTF_SES_HANDLE handle, RTF_SES_STATE *pState )
{
	RTF_FNAME( "rtfSesGetState" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pState = pSes->state;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the input stream profile
RTF_RESULT rtfSesGetStreamProfile( RTF_SES_HANDLE handle, RTF_STREAM_PROFILE **ppProfile )
{
	RTF_FNAME( "rtfSesGetStreamProfile" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// return a pointer to the input stream profile
		*ppProfile = &pSes->profile;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the input bit rate
RTF_RESULT rtfSesGetInputBitrate( RTF_SES_HANDLE handle, unsigned long *pBitsPerSecond )
{
	RTF_FNAME( "rtfSesGetInputBitrate" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pBitsPerSecond = pSes->profile.bitsPerSecond;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the output bit rate
RTF_RESULT rtfSesGetOutputBitrate( RTF_SES_HANDLE handle, unsigned long *pBitsPerSecond )
{
	RTF_FNAME( "rtfSesGetOutputBitrate" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pBitsPerSecond = ( pSes->generateTTS == FALSE ) ?
			pSes->profile.bitsPerSecond :
			( pSes->profile.bitsPerSecond * TTS_PACKET_BYTES + ( TRANSPORT_PACKET_BYTES>>1 ) ) / TRANSPORT_PACKET_BYTES;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get info on the current group
RTF_RESULT rtfSesGetCurrentGopInfo( RTF_SES_HANDLE handle, RTF_GOP_HANDLE *phGop, int *pGopCount )
{
	RTF_FNAME( "rtfSesGetCurrentGopInfo" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the returns
		*pGopCount = pSes->gopCount;
		*phGop = ( pSes->gopCount == 0 ) ? (RTF_GOP_HANDLE)NULL : pSes->hGop[ pSes->gopCount - 1 ];

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get info on the current picture
RTF_RESULT rtfSesGetCurrentPicInfo( RTF_SES_HANDLE handle, RTF_PIC_HANDLE *phPic, int *pPicCount )
{
	RTF_FNAME( "rtfSesGetCurrentPicInfo" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the returns
		*pPicCount = pSes->picCount;
		*phPic = ( pSes->picCount == 0 ) ? (RTF_PIC_HANDLE)NULL : pSes->hPic[ pSes->picCount - 1 ];

	} while( 0 ); // error escape wrapper - end

	return result;
}


// get the number of outputs being generated by the session
RTF_RESULT rtfSesGetOutputCount( RTF_SES_HANDLE handle, unsigned long *pOutputCount )
{
	RTF_FNAME( "rtfSesGetOutputCount" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pOutputCount = pSes->outputCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the current session elapsed time in clock ticks
RTF_RESULT rtfSesGetElapsedTime( RTF_SES_HANDLE handle, RTF_RUNTIME *pElapsedTime )
{
	RTF_FNAME( "rtfGetSessionElapsedTime" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RUNTIME currentTime;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// get the current time
		rtfGetRunTime( &currentTime );
		// make the return
		*pElapsedTime = currentTime - pSes->startTime;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of input packets processed by the session so far
RTF_RESULT rtfSesGetProgress( RTF_SES_HANDLE handle, unsigned long *pProcessedPacketCount )
{
	RTF_FNAME( "rtfSesGetProgress" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pProcessedPacketCount = pSes->totalPacketCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the total number of pictures processed by the session so far
RTF_RESULT rtfSesGetTotalPicCount( RTF_SES_HANDLE handle, unsigned long *pTotalPicCount )
{
	RTF_FNAME( "rtfSesGetTotalPicCount" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pTotalPicCount = pSes->totalPicCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the total number of groups processed by the session so far
RTF_RESULT rtfSesGetTotalGopCount( RTF_SES_HANDLE handle, unsigned long *pTotalGopCount )
{
	RTF_FNAME( "rtfSesGetTotalGopCount" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pTotalGopCount = pSes->totalGopCount;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the size of the input file (only known when processing offline)
RTF_RESULT rtfSesGetInputFileBytes( RTF_SES_HANDLE handle, INT64 *pInputFileBytes )
{
	RTF_FNAME( "rtfSesGetInputFileBytes" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pInputFileBytes = pSes->inputFileBytes;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the state of splicing enable (from the trick spec)
RTF_RESULT rtfSesGetSplicingEnabled( RTF_SES_HANDLE handle, BOOL *pSplicingEnabled )
{
	RTF_FNAME( "rtfSesGetInputFileBytes" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pSplicingEnabled = pSes->splicingEnabled;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the handles of the PSI objects
RTF_RESULT rtfSesGetPSI( RTF_SES_HANDLE handle, RTF_PAT_HANDLE *phPat,
						 RTF_CAT_HANDLE *phCat, RTF_PMT_HANDLE *phPmt )
{
	RTF_FNAME( "rtfSesGetInputFileBytes" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the returns
		*phPat = pSes->hPat;
		*phCat = pSes->hCat;
		*phPmt = pSes->hPmt;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the state of the "insert DSM trick mode flag" fixup switch
RTF_RESULT rtfSesGetInsertDSM( RTF_SES_HANDLE handle, BOOL *pInsertDSM )
{
	RTF_FNAME( "rtfSesGetInsertDSM" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*pInsertDSM = pSes->insertDSM;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the video frame rate code
RTF_RESULT rtfSesSetFrameRateCode( RTF_SES_HANDLE handle, int value )
{
	RTF_FNAME( "rtfSesSetFrameRateCode" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// has the video frame rate code already been set?
		if( pSes->frameRateAcquired == FALSE )
		{
			// no - record this value and set the flag
			pSes->profile.videoSpec.eStream.video.frameRateCode = value;
			pSes->frameRateAcquired = TRUE;
		}
		else
		{
			// yes - compare this value to the existing value
			if( value != pSes->profile.videoSpec.eStream.video.frameRateCode )
			{
				RTF_LOG_WARN2( RTF_MSG_WRN_FRCMISMATCH,
							   "Frame rate code mismatch - new value=%d, old value=%d",
							   value, pSes->profile.videoSpec.eStream.video.frameRateCode );
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the handle of the parsing window
RTF_RESULT rtfSesGetWindow( RTF_SES_HANDLE handle, RTF_WIN_HANDLE *phWin )
{
	RTF_FNAME( "rtfSesGetWindow" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*phWin = pSes->hWindow;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the packet(s) currently in the focus of the window
RTF_RESULT rtfSesGetWindowPackets( RTF_SES_HANDLE handle, RTF_PKT_HANDLE *phFirstPkt,
								   RTF_PKT_HANDLE *phNextPkt )
{
	RTF_FNAME( "rtfSesGetWindowPackets" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	unsigned char offset;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// get the position of the first window byte in the window packet set
		result = rtfWinGetFirstByteInfo( pSes->hWindow, phFirstPkt, &offset );
		RTF_CHK_RESULT;
		// get the position of the next window byte in the window packet set
		result = rtfWinGetNextByteInfo( pSes->hWindow, phNextPkt, &offset );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the PES object
RTF_RESULT rtfSesGetPes( RTF_SES_HANDLE handle, RTF_PES_HANDLE *phPes )
{
	RTF_FNAME( "rtfSesGetPes" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// make the return
		*phPes = pSes->hPes;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the current sequence handle
RTF_RESULT rtfSesGetSequence( RTF_SES_HANDLE handle, RTF_SEQ_HANDLE *phSeq )
{
	RTF_FNAME( "rtfSesGetSequence" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// make the return
		*phSeq = pSes->hSeq;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the indexer handle
RTF_RESULT rtfSesGetIndexer( RTF_SES_HANDLE handle, RTF_IDX_HANDLE *phIdx )
{
	RTF_FNAME( "rtfSesGetIndexer" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*phIdx = pSes->hIdx;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the condtional access system handle
RTF_RESULT rtfSesGetCASystem( RTF_SES_HANDLE handle, RTF_CAS_HANDLE *phCas )
{
	RTF_FNAME( "rtfSesGetCASystem" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make the return
		*phCas = pSes->hCas;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the video codec handle
RTF_RESULT rtfSesGetVideoCodec( RTF_SES_HANDLE handle, RTF_VCD_HANDLE *phVcd )
{
	RTF_FNAME( "rtfSesGetVideoCodec" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// make the return
		*phVcd = pSes->hVcd;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// open a trick file generation session
RTF_RESULT rtfSesOpen( RTF_SES_HANDLE handle,
					   RTF_APP_SESSION_HANDLE hAppSession,
					   RTF_APP_FILE_HANDLE hAppInputFile,
					   char *pInputFileName, INT64 inputFileBytes,
					   RTF_WARNING_COUNTS *pThresholds,
					   RTF_INDEX_MODE indexMode, RTF_INDEX_TYPE indexType,
					   RTF_INDEX_OPTION indexOption, RTF_SES_ERROR_NOTIFIER notifier,
					   RTF_BUFREL_FUNCTION inputBufferReleaseCallback,
					   int numSettings, RTF_APP_OUTPUT_SETTINGS settings[] )
{
	RTF_FNAME( "rtfSesOpen" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	INT64 initialByteOffset = 0;
	RTF_STREAM_PROFILE *pProfile;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_IDLE );
		// make sure the buffer release callback is not null
		if( inputBufferReleaseCallback == (RTF_BUFREL_FUNCTION)NULL)
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "Null buffer release callback" );
			break;
		}
		// make sure the settings count is in range
		if( numSettings > RTF_MAX_OUTPUTCOUNT )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_BADPARAM, "NumSettings (%d) is greater than max (%d)",
						  numSettings, RTF_MAX_OUTPUTCOUNT );
			break;
		}
		// record the session parameters
		if( pInputFileName == NULL )
		{
			memset( (void *)pSes->inputFileName, 0, sizeof(pSes->inputFileName) );
		}
		else
		{
			strncpy( pSes->inputFileName, pInputFileName, MAX_PATHLEN );
		}
		pSes->inputFileBytes = inputFileBytes;
		pSes->hAppSession = hAppSession;
		pSes->hAppInputFile = hAppInputFile;
		pSes->outputCount = numSettings;
		pSes->notifier = notifier;
		pSes->inputBufferReleaseCallback = inputBufferReleaseCallback;
		pSes->indexType = indexType;
		pSes->indexMode = indexMode;
		pSes->indexOption = indexOption;
		memcpy( (void *)&(pSes->warningThresholds), (void *)pThresholds, sizeof(pSes->warningThresholds) );
		// open the set of output objects and any associated filters
		pSes->indexFileOutputNumber = -1;
		pSes->mainFileOutputNumber = -1;
		for( i=0; i<pSes->outputCount; ++i )
		{
			// open the output
			result = rtfOutOpen( pSes->hOutput[ i ], pSes->hAppSession, i, &settings[ i ] );
			RTF_CHK_RESULT;
			// is this the index file?
			if( settings[ i ].trickSpec.speedNumerator == 0 )
			{
				// yes - record the index file identity
				pSes->indexFileOutputNumber = i;
			}
			else
			{
				// no - is this the main file filtered copy?
				if( ( settings[ i ].trickSpec.speedNumerator   == 1 ) &&
					( settings[ i ].trickSpec.speedDenominator == 1 ) )
				{
					// yes - record the main file copy identity
					pSes->mainFileOutputNumber = i;
				}
				// open the filter associated with this output file
				result = rtfFltOpen( pSes->hFilter[ i ], i, pSes->hOutput[ i ], &settings[ i ].trickSpec );
				RTF_CHK_RESULT;
			}
		}
		RTF_CHK_RESULT_LOOP;
		// did we find the index file?
		if( pSes->indexFileOutputNumber < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADPARAM, "No index file found in settings array (speedNumerator = 0)" );
			break;
		}
		// reset the PAT
		result = rtfPatReset( pSes->hPat );
		RTF_CHK_RESULT;
		// reset the CAT
		result = rtfCatReset( pSes->hCat );
		RTF_CHK_RESULT;
		// reset the PMT
		result = rtfPmtReset( pSes->hPmt );
		RTF_CHK_RESULT;
		// reset the CAS object
		result = rtfCasReset( pSes->hCas );
		RTF_CHK_RESULT;
		// record the "optimize for ATSC" switch
		// !!! WARNING !!! THIS ASSUMES ALL TRICKSPECS HAVE SAME OPTIMIZE FOR ATSC SETTING !!!
		pSes->optimizeForATSC = settings[ 1 ].trickSpec.optimizeForATSC;
		// record the "abort video on picture pool underflow" switch
		// !!! WARNING !!! THIS ASSUMES ALL TRICKSPECS HAVE SAME ABORT VID ON PPU SETTING !!!
		pSes->abortVidOnPPU = settings[ 1 ].trickSpec.abortVidOnPPU;
		// reset the payload scanning window (first use only)
		result = rtfWinReset( pSes->hWindow, handle, initialByteOffset, pSes->optimizeForATSC );
		RTF_CHK_RESULT;
		// copy in the default input stream profile
		rtfSysGetStreamProfile( &pProfile );
		memcpy( (void *)&pSes->profile, (void *)pProfile, sizeof(pSes->profile) );
		// !!! WARNING !!! THIS ASSUMES ALL SESSION TRICKSPECS HAVE SAME USER BIT RATE !!!
		pSes->profile.userBitsPerSecond = settings[ 1 ].trickSpec.userBitsPerSecond;
		pSes->profile.augmentationBaseFactor = 0;
		pSes->profile.augmentationPlusFactor = 0;
		pSes->profile.augmentationPidCount   = 0;
		RTF_CLR_STATE( pSes->profile.augmentationPids, sizeof( pSes->profile.augmentationPids ) );
		// reset the synchronization flag
		pSes->inputInSync = FALSE;
		// reset the TTS input flag
		pSes->inputIsTTS = FALSE;
		// reset the PSI valid flag
		pSes->psiValid = FALSE;
		// reset the CAS flags
		pSes->casReady = FALSE;
		pSes->casLocked = FALSE;
		// reset the first PCR flag
		pSes->firstPcrAcquired = FALSE;
		// reset the backlog processing flag
		pSes->mainFileBacklogProcessed = FALSE;
		// record the "insert DSM trick mode flag" switch
		// !!! WARNING !!! THIS ASSUMES ALL TRICKSPECS HAVE SAME INSERT DSM FIXUP SETTING !!!
		pSes->insertDSM = settings[ 1 ].trickSpec.insertDSM;
		// record the "ignore encryption" switch
		// !!! WARNING !!! THIS ASSUMES ALL TRICKSPECS HAVE SAME IGNORE ENCRYPTION SETTING !!!
		pSes->ignoreEncryption = settings[ 1 ].trickSpec.ignoreEncryption;
		// record the "generate TTS output" switch
		// !!! WARNING !!! THIS ASSUMES ALL TRICKSPECS HAVE SAME GENERATE TTS SETTING !!!
		pSes->generateTTS = settings[ 1 ].trickSpec.generateTTS;
		pSes->profile.flags |= ( pSes->generateTTS == 0 ) ? 0 : RTF_PROFILE_TTS_MASK;
		// reset the session counters
		pSes->totalInputByteCount = 0;
		pSes->totalInputBufferCount = 0;
		pSes->totalPacketCount = 0;
		pSes->totalMappedPacketCount = 0;
		pSes->totalPicCount = 0;
		pSes->totalGopCount = 0;
		pSes->totalSeqCount = 0;
		pSes->totalDamagedSeqCount = 0;
		// reset the session warning counters
		memset( (void *)&(pSes->warningCounts), 0, sizeof(pSes->warningCounts) );
		// reset the input bitrate measurement info
		pSes->bpsPcrCount = 0;
		RTF_CLR_STATE( pSes->bpsPcrPktNumber, sizeof(pSes->bpsPcrPktNumber) );
		RTF_CLR_STATE( pSes->bpsPcrTime, sizeof(pSes->bpsPcrTime) );
		// reset input frame rate measurement info
		pSes->fpsPicCount = 0;
		RTF_CLR_STATE( &pSes->fpsFirstPicTime, sizeof(pSes->fpsFirstPicTime) );
		RTF_CLR_STATE( &pSes->fpsLastPicTime, sizeof(pSes->fpsLastPicTime) );
		// reset some miscellaneous flags
		pSes->bitRateAcquired = FALSE;
		pSes->frameRateAcquired = FALSE;
		pSes->pesHeaderAcquired = FALSE;
		pSes->indexLibraryIntialized = FALSE;
		pSes->readyForStreaming = FALSE;
		pSes->splicingEnabled = FALSE;
		// reset the object processing queues
		// note: these are not the same as the permanent object pools!
		pSes->bufCount = 0;
		pSes->picCount = 0;
		pSes->gopCount = 0;
		pSes->seqCount = 0;
		RTF_CLR_STATE( pSes->hBuf, sizeof(pSes->hBuf) );
		RTF_CLR_STATE( pSes->hPic, sizeof(pSes->hPic) );
		RTF_CLR_STATE( pSes->hGop, sizeof(pSes->hGop) );
		// record the start time for this session
		rtfGetRunTime( &pSes->startTime );
		// copy that to the last process input time
		// (detect sessions that never get any input)
		pSes->lastProcessInputTime = pSes->startTime;
		// set the session state to open
		pSes->state = RTF_SES_STATE_ACTIVE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// close a trick file generation session
// note - when closing out a session, don't stop on error, just report
RTF_RESULT rtfSesClose( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesClose" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_BUFSTATE bufState;
	RTF_APP_SESSION_HANDLE hAppSession;
	RTF_APP_FILE_HANDLE hAppFile;
	RTF_APP_BUFFER_HANDLE hAppBuffer;
	RTF_SEQSTATE seqState;
	unsigned long putBufferCount;
	unsigned long bufNumber;
	unsigned long capacity;
	unsigned long occupancy;
	unsigned char *pBase;
	int i;
	char cTrueFalse[ 2 ] = { 'F', 'T' };

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// check to see if any output has been generated from this session
		for( i=0; i<pSes->outputCount; ++i )
		{
			if( ( i == 0 ) || ( pSes->profile.videoSpec.pid != TRANSPORT_INVALID_PID ) )
			{
				rtfOutGetPutBufferCount( pSes->hOutput[ i ], &putBufferCount );
				if( putBufferCount == 0 )
				{
					RTF_LOG_WARN7( RTF_MSG_WRN_ZEROOUTPUT,
						"File %s: final putbuf cnt 0 on out %d. inBufCnt=%d pktCnt=%d picCnt=%d gopCnt=%d seqCnt=%d",
						pSes->inputFileName, i, pSes->totalInputBufferCount, pSes->totalPacketCount,
						pSes->totalPicCount, pSes->totalGopCount, pSes->totalSeqCount );
					RTF_LOG_WARN8( RTF_MSG_WRN_ZEROOUTPUT,
						"File %s: final putbuf cnt 0 on out %d. inSync=%c psiValid=%c bRateAcq=%c fRateAcq=%c pesHdrAcq=%c rdyStrm=%c",
						pSes->inputFileName, i, cTrueFalse[ pSes->inputInSync ], cTrueFalse[ pSes->psiValid ],
						cTrueFalse[ pSes->bitRateAcquired ], cTrueFalse[ pSes->frameRateAcquired ],
						cTrueFalse[ pSes->pesHeaderAcquired ], cTrueFalse[ pSes->readyForStreaming ] );
				}
			}
		}
		// is there an active sequence?
		if( pSes->hSeq != (RTF_SEQ_HANDLE)NULL )
		{
			// yes - check its state
			rtfSeqGetState( pSes->hSeq, &seqState );
			// is it open?
			if( seqState == RTF_SEQSTATE_OPEN )
			{
				// yes - end it and release it
				rtfSesEndSeq( pSes );
			}
			else if( seqState == RTF_SEQSTATE_CLOSED )
			{
				// release it if it is not already released
				rtfSeqRelease( pSes->hSeq, handle );
			}			
		}
		// the session is no longer ready for streaming
		pSes->readyForStreaming = FALSE;
		// was the index initialized?
		if( pSes->indexLibraryIntialized != FALSE )
		{
			// yes - perform a final update of the index
			rtfIdxFinalize( pSes->hIdx, pSes->totalInputByteCount );
		}
		// close all open output filter objects and their associated outputs
		for( i=0; i<pSes->outputCount; ++i )
		{
			if( i != pSes->indexFileOutputNumber )
			{
				if( pSes->hFilter[ i ] != (RTF_FLT_HANDLE)NULL )
				{
					// close this filter object
					rtfFltClose( pSes->hFilter[ i ] );
				}
			}
			if( pSes->hOutput[ i ] != (RTF_OUT_HANDLE)NULL )
			{
				// close this output object
				rtfOutClose( pSes->hOutput[ i ] );
			}
		}
		// reset the PSI
		rtfPatReset( pSes->hPat );
		rtfCatReset( pSes->hCat );
		rtfPmtReset( pSes->hPmt );
		// release all active input buffers
		for( i=0; i<pSes->bufCount; ++i )
		{
			// get the mapping info from the buffer
			rtfBufGetMapInfo( pSes->hBuf[ i ], &bufState, &hAppSession, &hAppFile,
							  &hAppBuffer, &bufNumber, &pBase, &capacity, &occupancy );
			// release the storage back to the application
			if( pSes->inputBufferReleaseCallback( hAppSession, hAppFile, hAppBuffer, pBase ) != 0 )
			{
				RTF_LOG_INFO0( RTF_MSG_ERR_CALLBACKFAILED, "Buffer release callback failed" );
			}
			// reset the buffer object
			rtfBufReset( pSes->hBuf[ i ], TRUE );
		}
		// reset the codec object
		rtfVcdReset( pSes->hVcd );
		// reset the index object
		rtfIdxReset( pSes->hIdx );
		// set the session state to idle
		pSes->state = RTF_SES_STATE_IDLE;
		// record the stop time
		rtfGetRunTime( &pSes->stopTime );
		// log some stats about the session
		RTF_LOG_INFO1( RTF_MSG_INF_STATS, "Session complete for file %s", pSes->inputFileName );
		RTF_LOG_INFO5(RTF_MSG_INF_STATS, "  %d pkts, %d pics, %d gops, %d seqs (%d bad seqs)",
				pSes->totalMappedPacketCount, pSes->totalPicCount, pSes->totalGopCount,
				pSes->totalSeqCount, pSes->totalDamagedSeqCount );
		// log the elapsed time
		RTF_LOG_INFO1( RTF_MSG_INF_STATS, "  elapsed time = %d ticks", ( pSes->stopTime - pSes->startTime ) );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// call the session error notifier function
RTF_RESULT rtfSesErrorNotifier( RTF_SES_HANDLE handle, RTF_MSG msg, RTF_OBJ_HANDLE hObj )
{
	RTF_FNAME( "rtfSesErrorNotifier" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_MSG_PRIO prio;
	char *pTypeStr;
	char *pMsgStr;
	char messageBuffer[ 256];

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// get the object type
		result = rtfObjGetTypeStr( hObj, &pTypeStr );
		RTF_CHK_RESULT;
		// get the short string associated with this error
		rtfSysGetMsgInfo( msg, &prio, &pMsgStr );
		// format the message
		sprintf( messageBuffer, "%s; event occurred in %s object", pMsgStr, pTypeStr );
		if( pSes->notifier != (RTF_SES_ERROR_NOTIFIER)NULL )
		{
			(*pSes->notifier)( pSes->hAppSession, messageBuffer );
		}
		// respond to this error
		result = rtfSesErrorResponse( pSes, msg );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// submit a buffer of input for processing (buffer is held by trickfile library until explicitly released)
RTF_RESULT rtfSesProcessInput( RTF_SES_HANDLE handle,
							   RTF_APP_SESSION_HANDLE hAppSession,
							   RTF_APP_FILE_HANDLE hAppFile,
							   RTF_APP_BUFFER_HANDLE hAppBuffer,
							   unsigned char *pBuffer, unsigned long bytes )
{
	RTF_FNAME( "rtfSesProcessInput" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_CASSTATE casState;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// record the time of this process input call
		rtfGetRunTime( &pSes->lastProcessInputTime );
		// update the total input byte count
		pSes->totalInputByteCount += bytes;
		// add a free buffer object to the end of the buffer queue
		result = rtfQueueBuf( pSes );
		RTF_CHK_RESULT;
		// map the new buffer object onto this storage
		result = rtfBufMap( pSes->hBuf[pSes->bufCount-1], hAppSession, hAppFile, hAppBuffer,
							pSes->totalInputBufferCount++, pBuffer, bytes, bytes );
		RTF_CHK_RESULT;
		// find the transport packets in this buffer (including any leading packet fragment)
		result = rtfBufMapInputPackets( pSes->hBuf[pSes->bufCount-1], &pSes->inputInSync,
										&pSes->inputIsTTS, &pSes->firstPcrAcquired,
										pSes->packetFragment, &pSes->packetFragmentBytes,
										&pSes->totalPacketCount, &pSes->totalMappedPacketCount );
		RTF_CHK_RESULT;
		// are we still looking for PSI?
		if( pSes->psiValid == FALSE )
		{
			// yes - scan the active buffer for the PAT, CAT, and PMT
			result = rtfBufCapturePsi( pSes->hBuf[pSes->bufCount-1], pSes->hPat,
									   pSes->hCat, pSes->hPmt, &pSes->psiValid );
			RTF_CHK_RESULT;
			// did we just capture PSI?
			if( pSes->psiValid != FALSE )
			{
				// yes - set up the PMT PID in the profile
				result = rtfPatGetPmtPid( pSes->hPat, &pSes->profile.pmtPID );
				RTF_CHK_RESULT;
				// was a video component detected?
				if( pSes->profile.videoSpec.eStream.video.vcdType != RTF_VIDEO_CODEC_TYPE_INVALID )
				{
					// yes - set up the video CODEC object
					result = rtfVcdOpen( pSes->hVcd, &pSes->profile.videoSpec );
					RTF_CHK_RESULT;
				}
				else
				{
					result = rtfSesAbortTrickFiles( pSes );
					RTF_CHK_RESULT;
				}
			}
		}
		else
		{
			// no - PSI already captured
			// scan the active buffer for the PAT, CAT, and PMT - make sure none have changed
			result = rtfBufCheckPsi( pSes->hBuf[pSes->bufCount-1], pSes->hPat,
									 pSes->hCat, pSes->hPmt, &pSes->psiValid );
			RTF_CHK_RESULT;
		}
		// have we acquired the input bitrate yet?
		if( pSes->bitRateAcquired == FALSE )
		{
			// no - scan the active buffer for PCRs
			result = rtfBufCaptureBpsInfo( pSes->hBuf[pSes->bufCount-1], RTF_SES_MAX_BITRATEPCRS,
										&pSes->bpsPcrCount, pSes->bpsPcrPktNumber, pSes->bpsPcrTime );
			RTF_CHK_RESULT;
			// do we have the required number of PCRs?
			if( pSes->bpsPcrCount >= RTF_SES_MAX_BITRATEPCRS )
			{
				// yes - record the bit rate
				result = rtfComputeInputBitRate( pSes );
				RTF_CHK_RESULT;
				// set the bit rate acquired flag
				pSes->bitRateAcquired = TRUE;
			}
		}
		// do we still need to acquire the video frame rate?
		if( ( pSes->profile.videoSpec.pid != TRANSPORT_INVALID_PID ) &&
			( pSes->frameRateAcquired == FALSE ) )
		{
			// yes - scan the active buffer for video frame rate info
			result = rtfBufCaptureFpsInfo( pSes->hBuf[pSes->bufCount-1], RTF_SES_MAX_FRAMERATEPICS,
										   &pSes->fpsPicCount, &pSes->fpsFirstPicTime, &pSes->fpsLastPicTime );
			RTF_CHK_RESULT;
			// do we have the required number of frame rate pictures?
			if( pSes->fpsPicCount >= RTF_SES_MAX_FRAMERATEPICS )
			{
				// yes - compute the frame rate
				result = rtfComputeInputFrameRate( pSes );
				RTF_CHK_RESULT;
				// set the frame rate acquired flag
				pSes->frameRateAcquired = TRUE;
			}
		}
		// is encryption being ignored?
		if( pSes->ignoreEncryption == FALSE )
		{
			// no - is CA support locked (doesn't need any more input)?
			if( pSes->casLocked == FALSE )
			{
				// no - run these packets through the CA support object. get the updated state
				result = rtfCasProcessInputBuf( pSes->hCas, pSes->hBuf[ pSes->bufCount - 1 ], &casState );
				RTF_CHK_RESULT;
				switch( casState )
				{
				case RTF_CASSTATE_READY:
					pSes->casReady = TRUE;
					break;
				case RTF_CASSTATE_LOCKED:
					pSes->casLocked = TRUE;
					break;
				default:
					break;
				}
			}
		}
		else
		{
			pSes->casReady = TRUE;
		} 
		// do we have PSI, input bitrate, and CA support ready?
		if( ( pSes->psiValid != FALSE ) &&
			( pSes->bitRateAcquired != FALSE ) &&
			( pSes->casReady != FALSE ) )
		{
			// do we still need to acquire the video frame rate?
			if( ( pSes->profile.videoSpec.pid == TRANSPORT_INVALID_PID ) ||
				( pSes->frameRateAcquired != FALSE ) )
			{
				// no - process all unprocessed buffers
				result = rtfProcessBuffers( pSes );
				RTF_CHK_RESULT;
				// purge any zero-reference buffers left over after processing
				result = rtfPurgeBufQueue( pSes );
				RTF_CHK_RESULT;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// start a new PES packet
RTF_RESULT rtfSesStartPes( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesStartPes" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// tell the video codec to update the boundary location info
		result = rtfVcdUpdateBoundaryInfo( pSes->hVcd );
		RTF_CHK_RESULT;
		// temporarily override start code search optimization
		result = rtfWinSetOptimizationOverride( pSes->hWindow, TRUE );
		RTF_CHK_RESULT;
		// parse the PES packet header
		result = rtfPesParse( pSes->hPes, pSes->hWindow, pSes->indexType );
		RTF_CHK_RESULT;
		// are we still looking to capture a PES packet header?
		if( pSes->pesHeaderAcquired == FALSE )
		{
			// yes - check to see if we captured a valid PES header
			result = rtfPesValidate( pSes->hPes, &pSes->pesHeaderAcquired );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// flush all sequences, groups, and pictures in progress
RTF_RESULT rtfSesFlush( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesFlush" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// is there an open sequence?
		while( pSes->seqCount > 0 )
		{
			// yes - release the sequence (allows input buffers to be recycled)
			result = rtfSeqRelease( pSes->hSeq, handle );
			RTF_CHK_RESULT;
		}
		// all of the groups and pictures owned by the active sequence were just released
		// but there may have been other elements queued but not attached to the sequence
		while( pSes->gopCount > 0 )
		{
			result = rtfGopRelease( pSes->hGop[ pSes->gopCount-1 ], handle );
			RTF_CHK_RESULT;
		}
		while( pSes->picCount > 0 )
		{
			result = rtfPicRelease( pSes->hPic[ pSes->picCount-1 ], handle );
			RTF_CHK_RESULT;
		}
		// release any zero-reference buffers that have already been scanned
		result = rtfPurgeBufQueue( pSes );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// recycle the indicated video picture
RTF_RESULT rtfSesRecyclePic( RTF_SES_HANDLE handle, RTF_PIC_HANDLE hPic )
{
	RTF_FNAME( "rtfSesRecyclePic" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// find the group on the active group list
		for( i=0; i<pSes->picCount; ++i )
		{
			if( pSes->hPic[ i ] == hPic )
			{
				break;
			}
		}
		// was it there?
		// note: it may not be if there was a pool underflow, so press on
		if( i < pSes->picCount )
		{
			// yes - collapse the active picture list
			--pSes->picCount;
			for( ; i<pSes->picCount; ++i )
			{
				pSes->hPic[ i ] = pSes->hPic[ i+1 ];
			}
			pSes->hPic[ i ] = (RTF_PIC_HANDLE)NULL;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// end the current picture
RTF_RESULT rtfSesEndPic( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesEndPic" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_PIC_HANDLE hPic;
	RTF_GOP_HANDLE hGop;
	RTF_PICSTATE picState;
	RTF_PKT_HANDLE hLastBytePacket;
	unsigned char lastBytePacketOffset;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// flush if there is no active picture, group, or sequence
		if( ( pSes->picCount == 0 ) ||
			( pSes->gopCount == 0 ) ||
			( pSes->seqCount == 0 ) )
		{
			result = rtfSesFlush( handle );
			RTF_CHK_RESULT;
			break;
		}
		// is the current picture still open?
		hPic = pSes->hPic[ pSes->picCount-1 ];
		result = rtfPicGetState( hPic, &picState );
		RTF_CHK_RESULT;
		if( picState == RTF_PICSTATE_OPEN )
		{
			// yes - close it
			result = rtfPicClose( hPic );
			RTF_CHK_RESULT;
			// get the picture boundary info from the video codec
			result = rtfVcdGetPicEndInfo( pSes->hVcd, &hLastBytePacket, &lastBytePacketOffset );
			RTF_CHK_RESULT;
			// does it have a valid start point recorded?
			if( hLastBytePacket == (RTF_PKT_HANDLE)NULL )
			{
				// no - go with the byte before the window start position
				result = rtfWinGetPriorByteInfo( pSes->hWindow, &hLastBytePacket, &lastBytePacketOffset );
				RTF_CHK_RESULT;
			}
			// record this info in the picture object
			result = rtfPicSetEndInfo( hPic, hLastBytePacket, lastBytePacketOffset );
			RTF_CHK_RESULT;
			// add the picture that was just closed to the active group
			hGop = pSes->hGop[pSes->gopCount-1];
			result = rtfGopAddPic( hGop, hPic );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// start a new picture
RTF_RESULT rtfSesStartPic( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfStartPic" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_GOPSTATE gopState;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// is there an active picture?
		if( pSes->picCount > 0 )
		{
			// yes - end it
			result = rtfSesEndPic( handle );
			RTF_CHK_RESULT;
		}
		// is there an active sequence?
		if( pSes->seqCount == 0 )
		{
			// no - start a "virtual" sequence if there is no active sequence
			result = rtfSesStartSeq( handle, TRUE );
			RTF_CHK_RESULT;
		}
		// is there an active group?
		if( pSes->gopCount == 0 )
		{
			// no - start a "virtual" group if there is no active group
			result = rtfSesStartGop( handle, TRUE );
			RTF_CHK_RESULT;
		}
		else
		{
			// yes - is the most recent GOP still open?
			result = rtfGopGetState( pSes->hGop[ pSes->gopCount - 1 ], &gopState );
			if( gopState != RTF_GOPSTATE_OPEN )
			{
				// no - start a "virtual" group if there is no active group
				result = rtfSesStartGop( handle, TRUE );
				RTF_CHK_RESULT;
			}
		}
		// add a free picture to the picture queue
		result = rtfQueuePic( pSes );
		RTF_CHK_RESULT;
		// open a new picture
		result = rtfPicOpen( pSes->hPic[ pSes->picCount-1 ], pSes->totalPicCount++, pSes->picCount,
							 pSes->profile.videoSpec.pid, pSes->profile.videoSpec.eStream.video.vcdType );
		RTF_CHK_RESULT;
		// turn off start code search optimization, if it was on
		result = rtfWinSetOptimizationOverride( pSes->hWindow, FALSE );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// recycle the indicated video group
RTF_RESULT rtfSesRecycleGop( RTF_SES_HANDLE handle, RTF_GOP_HANDLE hGop )
{
	RTF_FNAME( "rtfSesRecycleGop" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// find the group on the active group list
		for( i=0; i<pSes->gopCount; ++i )
		{
			if( pSes->hGop[ i ] == hGop )
			{
				break;
			}
		}
		// was it there?
		// note: it may not be if there was a pool underflow, so press on
		if( i < pSes->gopCount )
		{
			// yes - collapse the active group list
			--pSes->gopCount;
			for( ; i<pSes->gopCount; ++i )
			{
				pSes->hGop[ i ] = pSes->hGop[ i+1 ];
			}
			pSes->hGop[ i ] = (RTF_GOP_HANDLE)NULL;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// end the current group of pictures
RTF_RESULT rtfSesEndGop( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesEndGop" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_GOP_HANDLE hGop;
	RTF_GOPSTATE gopState;
	BOOL isKeyframe;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// flush if there is no active group, or sequence
		if( ( pSes->gopCount == 0 ) ||
			( pSes->seqCount == 0 ) )
		{
			result = rtfSesFlush( handle );
			RTF_CHK_RESULT;
			break;
		}
		// is there an active picture?
		if( pSes->picCount > 0 )
		{
			// yes - is the current picture a keyframe?
			result = rtfPicGetIsKeyframe( pSes->hPic[ pSes->picCount - 1 ], &isKeyframe );
			RTF_CHK_RESULT;
			if( isKeyframe != FALSE )
			{
				// yes - is this the first keyframe of the current group?
				// Note: if not, levae current keyframe open while closing GOP
				if( pSes->picCount == 1 )
				{
					// yes - end the current picture and add it to the group
					result = rtfSesEndPic( handle );
					RTF_CHK_RESULT;
				}
			}
			else
			{
				// no - end the current picture and add it to the group
				result = rtfSesEndPic( handle );
				RTF_CHK_RESULT;
			}
		}
		// is the current group still open?
		hGop = pSes->hGop[pSes->gopCount-1];
		result = rtfGopGetState( hGop, &gopState );
		RTF_CHK_RESULT;
		if( gopState == RTF_GOPSTATE_OPEN )
		{
			// yes - close the group
			result = rtfGopClose( hGop );
			RTF_CHK_RESULT;
			// add the group to the active sequence
			result = rtfSeqAddGop( pSes->hSeq, hGop );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// start a new group of pictures
RTF_RESULT rtfSesStartGop( RTF_SES_HANDLE handle, BOOL isVirtual )
{
	RTF_FNAME( "rtfSesStartGop" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_SEQSTATE seqState;
	RTF_GOPSTATE gopState;
	RTF_PKT_HANDLE hFirstBytePacket = (RTF_PKT_HANDLE)NULL;
	BOOL seqIsVirtual;
	unsigned char firstBytePacketOffset = 0;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// is there an open sequence?
		result = rtfSeqGetState( pSes->hSeq, &seqState );
		RTF_CHK_RESULT;
		if( seqState != RTF_SEQSTATE_OPEN )
		{
			// no - do nothing until there is an open sequence
			break;
		}
		// are we about to run out of GOPs in the pool?
		if( pSes->gopCount >= (int)pSes->maxGroups )
		{
			// yes - force an end to the current sequence...
			result = rtfSesEndSeq( (RTF_SES_HANDLE)pSes );
			RTF_CHK_RESULT;
			// ... and start a new virtual sequence
			result = rtfSesStartSeq( (RTF_SES_HANDLE)pSes, TRUE );
			RTF_CHK_RESULT;
		}
		// is there an active group?
		if( pSes->gopCount > 0 )
		{
			// yes - is the active sequence virtual?
			result = rtfSeqGetIsVirtual( pSes->hSeq, &seqIsVirtual );
			RTF_CHK_RESULT;
			if( seqIsVirtual == FALSE )
			{
				// no - get the active GOP state
				result = rtfGopGetState( pSes->hGop[pSes->gopCount-1], &gopState );
				RTF_CHK_RESULT;
				// if the active GOP is still open, end it
				if( gopState == RTF_GOPSTATE_OPEN )
				{
					result = rtfSesEndGop( handle );
					RTF_CHK_RESULT;
				}
			}
			else
			{
				// yes - force an end to the current sequence...
				result = rtfSesEndSeq( (RTF_SES_HANDLE)pSes );
				RTF_CHK_RESULT;
				// ... and start a new virtual sequence
				result = rtfSesStartSeq( (RTF_SES_HANDLE)pSes, TRUE );
				RTF_CHK_RESULT;
			}
		}
		// add a free GOP to the GOP queue
		result = rtfQueueGop( pSes );
		RTF_CHK_RESULT;
		// open the new gop
		result = rtfGopOpen( pSes->hGop[pSes->gopCount-1], pSes->totalGopCount++, isVirtual );
		RTF_CHK_RESULT;
		// get the starting location of the GOP
		result = rtfWinGetFirstByteInfo( pSes->hWindow, &hFirstBytePacket, &firstBytePacketOffset );
		RTF_CHK_RESULT;
		// record starting location info in the GOP object
		result = rtfGopSetStart( pSes->hGop[ pSes->gopCount - 1 ], hFirstBytePacket, firstBytePacketOffset );
		RTF_CHK_RESULT;
		// record the starting offset of the GOP in the first packet of the GOP
		result = rtfPktSetGopStartOffset( hFirstBytePacket, firstBytePacketOffset, isVirtual );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// recycle the indicated video sequence
RTF_RESULT rtfSesRecycleSeq( RTF_SES_HANDLE handle, RTF_SEQ_HANDLE hSeq )
{
	RTF_FNAME( "rtfSesRecycleSeq" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		if( pSes->hSeq != hSeq )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_NOTFOUND, "Sequence handle not found" );
			break;
		}
		pSes->seqCount = 0;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// end the current video sequence
RTF_RESULT rtfSesEndSeq( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesEndSeq" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_SEQSTATE seqState;
	RTF_IDX_STATE idxState;
	BOOL isDamaged;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// flush if there is no active sequence
		if( pSes->seqCount == 0 )
		{
			result = rtfSesFlush( handle );
			RTF_CHK_RESULT;
			break;
		}
		// is there an open group?
		if( pSes->gopCount > 0 )
		{
			// yes - end it
			result = rtfSesEndGop( handle );
			RTF_CHK_RESULT;
		}
		// is the current sequence still open?
		result = rtfSeqGetState( pSes->hSeq, &seqState );
		RTF_CHK_RESULT;
		if( seqState == RTF_SEQSTATE_OPEN )
		{
			// yes - close the sequence
			result = rtfSeqClose( pSes->hSeq );
			RTF_CHK_RESULT;
			// get the state of the indexer
			result = rtfIdxGetState( pSes->hIdx, &idxState );
			RTF_CHK_RESULT;
			// get the damage flag from the sequence
			result = rtfSeqGetIsDamaged( pSes->hSeq, &isDamaged );
			RTF_CHK_RESULT;
			// if the sequence is damaged, bump the counter
			if( isDamaged != FALSE )
			{
				++pSes->totalDamagedSeqCount;
			}
			// is the indexer open AND is the sequence undamaged?
			if( ( idxState == RTF_IDX_STATE_OPEN ) && ( isDamaged == FALSE ) )
			{
				// yes - process the sequence
				result = rtfSesProcessSeq( pSes );
				RTF_CHK_RESULT;
			}
			else
			{
				// no - release the sequence without processing it
				result = rtfSesReleaseSeq( pSes );
				RTF_CHK_RESULT;
			}
		} // if( seqState == RTF_SEQSTATE_OPEN )

	} while( 0 ); // error escape wrapper - end

	return result;
}

// start a new video sequence - return the handle
RTF_RESULT rtfSesStartSeq( RTF_SES_HANDLE handle, BOOL isVirtual )
{
	RTF_FNAME( "rtfSesStartSeq" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PKT_HANDLE hFirstBytePacket;
	unsigned char firstBytePacketOffset;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// is there an open sequence?
		if( pSes->seqCount > 0 )
		{
			// yes - end it
			result = rtfSesEndSeq( handle );
			RTF_CHK_RESULT;
		}
		// open a new sequence
		result = rtfSeqOpen( pSes->hSeq, pSes->totalSeqCount++, isVirtual );
		RTF_CHK_RESULT;
		pSes->seqCount = 1;
		// get the sequence start info from the video codec
		result = rtfVcdGetSeqStartInfo( pSes->hVcd, &hFirstBytePacket, &firstBytePacketOffset );
		RTF_CHK_RESULT;
		// does it have a valid start point recorded?
		if( hFirstBytePacket == (RTF_PKT_HANDLE)NULL )
		{
			// no - go with the window start position
			result = rtfWinGetFirstByteInfo( pSes->hWindow, &hFirstBytePacket, &firstBytePacketOffset );
			RTF_CHK_RESULT;
		}
		// record this info in the sequence object
		result = rtfSeqSetStartInfo( pSes->hSeq, hFirstBytePacket, firstBytePacketOffset );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// an input buffer has been unmapped. release it back to the input and remove the reference
RTF_RESULT rtfSesInputBufferUnmapped( RTF_SES_HANDLE handle, RTF_BUF_HANDLE hBuffer )
{
	RTF_FNAME( "rtfSesInputBufferUnmapped" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_SES *pSes = (RTF_SES *)handle;
	unsigned char *pBase;
	RTF_APP_SESSION_HANDLE hAppSession;
	RTF_APP_FILE_HANDLE hAppFile;
	RTF_APP_BUFFER_HANDLE hAppBuffer;
	RTF_BUFSTATE state;
	unsigned long bufferNumber;
	unsigned long capacity;
	unsigned long occupancy;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		RTF_CHK_STATE_EQ( pSes, RTF_SES_STATE_ACTIVE );
		// find this buffer on the buffer queue
		for( i=0; i<pSes->bufCount; ++i )
		{
			if( hBuffer == pSes->hBuf[i] )
			{
				break;
			}
		}
		// make sure we found it
		if( i >= pSes->bufCount )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Buffer not found on active list" );
			break;
		}
		// get the mapping info from this buffer
		result = rtfBufGetMapInfo( hBuffer, &state, &hAppSession, &hAppFile, &hAppBuffer,
								   &bufferNumber, &pBase, &capacity, &occupancy );
		RTF_CHK_RESULT;
		// tell the application that the input buffer is now released
		if( pSes->inputBufferReleaseCallback( hAppSession, hAppFile, hAppBuffer, pBase )!= 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_CALLBACKFAILED, "Buffer release callback failed" );
			break;
		}
		// collapse the buffer queue over this entry
		if( --pSes->bufCount < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Negative buffer count" );
			break;
		}
		for( ; i<pSes->bufCount; ++i )
		{
			pSes->hBuf[i] = pSes->hBuf[i+1];
		}
		pSes->hBuf[i] = (RTF_BUF_HANDLE)NULL;
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// log the output settings for a particular output of a session
RTF_RESULT rtfSesLogOutputSettings( RTF_SES_HANDLE handle, int outputNumber )
{
	RTF_FNAME( "rtfSesLogOutputSettings" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		if( pSes->state != RTF_SES_STATE_ACTIVE )
		{
			RTF_LOG_INFO0( RTF_MSG_INF_LOGSTATECLOSED, "Session not active" );
			break;
		}
		if( ( outputNumber < 0 ) || ( outputNumber >= pSes->outputCount ) )
		{
			RTF_LOG_INFO2( RTF_MSG_INF_LOGOUTRANGE, "Output number (%d) is out of range (0-%d)", outputNumber, pSes->outputCount );
			break;
		}
		RTF_LOG_INFO1( RTF_MSG_INF_STATS, "Output settings for output %d:", outputNumber );
		result = rtfOutLogSettings( pSes->hOutput[ outputNumber ] );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// log the output settings for all of the outputs of a session
RTF_RESULT rtfSesLogAllOutputSettings( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesLogOutputSettings" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		if( pSes->state != RTF_SES_STATE_ACTIVE )
		{
			RTF_LOG_INFO0( RTF_MSG_INF_LOGSTATECLOSED, "Session not active" );
			break;
		}
		for( i=0; i<pSes->outputCount; ++i )
		{
			result = rtfSesLogOutputSettings( handle, i );
			RTF_CHK_RESULT;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// check to see if a message should be logged or suppressed
RTF_RESULT rtfSesCheckMessageSuppression( RTF_SES_HANDLE handle, RTF_MSG msg, BOOL *pSuppress )
{
	RTF_FNAME( "rtfSesCheckMessageSuppression" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	int count, thresh, suppress;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		suppress = pSes->warningThresholds.suppressCount;
		rtfSesGetWarningInfo( pSes, msg, 0, &count, &thresh, pSuppress );

	} while( 0 ); // error escape wrapper - end

	return result;
}

RTF_RESULT rtfSyncSearch( RTF_SES *pSes, RTF_AUG_INFO *pInfo )
{
	RTF_FNAME( "rtfSyncCheck" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// is the next byte a sync character?
		if( pInfo->buf[ pInfo->offset ] == TRANSPORT_PACKET_SYNCBYTE )
		{
			// yes - is there at least one more packet in this buffer?
			if( ( pInfo->offset + TRANSPORT_PACKET_BYTES ) < pInfo->bufOccupancy )
			{
				// yes - is there another sync byte one packet further along?
				if( pInfo->buf[ pInfo->offset+TRANSPORT_PACKET_BYTES ] == TRANSPORT_PACKET_SYNCBYTE )
				{
					// yes - record synchronization achieved
					pInfo->inSync = TRUE;
					// advance one packet
					pInfo->offset += TRANSPORT_PACKET_BYTES;
					// escape
					break;
				}
			}
		}
		// not in sync - increment the sync search byte counter
		++pInfo->syncSearchByteCt;
		// update the total packet counter whenever we search past a packet worth of bytes
		if( pInfo->syncSearchByteCt >= TRANSPORT_PACKET_BYTES )
		{
			pInfo->syncSearchByteCt = 0;
			++pInfo->totalPktCt;
		}
		// advance one byte
		++pInfo->offset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

RTF_RESULT rtfSyncCheck( RTF_SES *pSes, RTF_AUG_INFO *pInfo )
{
	RTF_FNAME( "rtfSyncCheck" );
	RTF_OBASE( pSes );
	RTF_RESULT result = RTF_PASS;
	int i, ext;
	unsigned short pid;

	do {		 // error escape wrapper - begin

		// is the next byte a sync character?
		if( pInfo->buf[ pInfo->offset ] != TRANSPORT_PACKET_SYNCBYTE )
		{
			// no - is there at least one more packet in this buffer?
			if( ( pInfo->offset + TRANSPORT_PACKET_BYTES ) >= pInfo->bufOccupancy )
			{
				// no - go for the next buffer
				break;
			}
			// check the next packet and see if sync is restored
			++pInfo->totalPktCt;
			pInfo->offset += TRANSPORT_PACKET_BYTES;
			if( pInfo->buf[ pInfo->offset ] != TRANSPORT_PACKET_SYNCBYTE )
			{
				pInfo->inSync = 0;
				break;
			}
			// sync restored - continue
			pInfo->inSync = 1;
		} // if( pInfo->buf[ pInfo->offset ] != TRANSPORT_PACKET_SYNCBYTE )
		// bump the total packet count
		++pInfo->totalPktCt;
		if( pInfo->augmentationPidCount > 0 )
		{
			// get the PID from this packet
			pid = pInfo->buf[ pInfo->offset+1 ] & 0x1F;
			pid = ( pid << 8 ) | pInfo->buf[ pInfo->offset+2 ];
			// scan the augmentation PID list for a match
			for( i=0; i<pInfo->augmentationPidCount; ++i )
			{
				if( pid == pInfo->pAugmentationPids[ i ] )
				{
					break;
				}
			}
			// some CA systems (Verimatrix) use augmentation packets
			// that are disguised as NULL packets. Is this a NULL packet?
			if( pid == TRANSPORT_PAD_PID )
			{
				// yes - look for the VM "ECM" signature
				if( ( pInfo->buf[ pInfo->offset+4 ] == RTF_VMECM_SIGNATURE ) &&
					( pInfo->buf[ pInfo->offset+5 ] == RTF_VMECM_SIGNATURE ) &&
					( pInfo->buf[ pInfo->offset+6 ] == RTF_VMECM_SIGNATURE ) &&
					( pInfo->buf[ pInfo->offset+7 ] == RTF_VMECM_SIGNATURE ) )
				{
					// found it - bump the augmentation count
					++pInfo->augPktCt;
				}
				else
				{
					// just a normal null packet - bump the base count
					++pInfo->basePktCt;
					if( pInfo->lastPktType != 0 )
					{
						++pInfo->baseClusterCt;
						pInfo->lastPktType = 0;
					}
				}
			}
			else //  //if( pid == TRANSPORT_PAD_PID )
			{
				// no - is this one of the augmentation PIDs?
				if( i < pInfo->augmentationPidCount )
				{
					// yes - bump the augmentation packet count
					++pInfo->augPktCt;
					if( pInfo->lastPktType != 1 )
					{
						++pInfo->augClusterCt;
						pInfo->lastPktType = 1;
					}
				}
				else // // if( augmentationPidCount > 0 )
				{
					// no - bump the base packet count
					++pInfo->basePktCt;
					if( pInfo->lastPktType != 0 )
					{
						++pInfo->baseClusterCt;
						pInfo->lastPktType = 0;
					}
				}
			} // // if( augmentationPidCount > 0 ) ; else
		} //if( pid == TRANSPORT_PAD_PID ) ; else
		// is there an adaptation field in this packet?
		if( ( pInfo->buf[ pInfo->offset+3 ] & 0x20 ) != 0 )
		{
			// yes. is the adaptation field length non-zero?
			if( pInfo->buf[ pInfo->offset+4 ] > 0 )
			{
				// yes - does the adaptation field contain a PCR?
				if( ( pInfo->buf[ pInfo->offset+5 ] & 0x10 ) != 0 )
				{
					// yes - parse the PCR value and record the packet count
					pInfo->lastPcrValue = pInfo->buf[ pInfo->offset+6 ];
					pInfo->lastPcrValue = ( pInfo->lastPcrValue << 8 ) | pInfo->buf[ pInfo->offset+7 ];
					pInfo->lastPcrValue = ( pInfo->lastPcrValue << 8 ) | pInfo->buf[ pInfo->offset+8 ];
					pInfo->lastPcrValue = ( pInfo->lastPcrValue << 8 ) | pInfo->buf[ pInfo->offset+9 ];
					pInfo->lastPcrValue = ( pInfo->lastPcrValue << 1 ) | ( pInfo->buf[ pInfo->offset+10 ] >> 7 );
					ext = pInfo->buf[ pInfo->offset+10 ] & 0x01;
					ext = ( ext << 1 ) | pInfo->buf[ pInfo->offset+11 ];
					pInfo->lastPcrValue = ( pInfo->lastPcrValue * 300 ) + ext;
					pInfo->lastPcrPktCt = pInfo->totalPktCt;
					// is this the first PCR?
					if( pInfo->firstPcrValue == -1 )
					{
						// yes - record the first PCR value and packet count
						pInfo->firstPcrValue = pInfo->lastPcrValue;
						pInfo->firstPcrPktCt = pInfo->lastPcrPktCt;
					}
				} // if( ( pInfo->buf[ pInfo->offset+4 ] & 0x10 ) != 0 ) (PCR present)
			} // if( pInfo->buf[ pInfo->offset+4 ] > 0 ) (non-zero length)
		} // if( ( pInfo->buf[ pInfo->offset+3 ] & 0x20 ) != 0 ) (adaptation field present)
		// move forward to the next packet in the buffer
		pInfo->offset += TRANSPORT_PACKET_BYTES;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the augmentation info for a session
RTF_RESULT rtfSesGetAugmentationInfo( RTF_SES_HANDLE handle,
									  int augmentationPidCount,
									  unsigned short *pAugmentationPids,
									  RTF_RDINPUT_FUNCTION readInputCallback,
									  int *pAugmentedBitRate, int *pOriginalBitRate )
{
	RTF_FNAME( "rtfSesGetAugmentationInfo" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_AUG_INFO info;
	INT64 temp64;
	INT64 deltaPcrValue;
	double bits;
	double msec;
	double bps;
	int iResult;
	int deltaPktCt;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// make sure the pid count is in range
		if( augmentationPidCount > RTF_MAX_AUGMENTATIONPIDS )
		{
			RTF_LOG_ERR2( RTF_MSG_ERR_BADPARAM, "Augmentation PID count (%d) exceeds maximum (%d)",
						  augmentationPidCount, RTF_MAX_AUGMENTATIONPIDS );
			break;
		}
		// reset the augmentation info structure
		memset( (void *)&info, 0, sizeof(info) );
		// initialize some non-zero fields
		info.inSync = 1;
		info.firstPcrValue = -1;
		info.augmentationPidCount = augmentationPidCount;
		info.pAugmentationPids = pAugmentationPids;
		// loop reading packets from the input stream
		// find the first and the last PCR and count the number of packets in between
		// also count the number of augmentation packets, and record how they cluster
		for( i=0; ; ++i )
		{
			iResult = ( *readInputCallback )( pSes->hAppSession, info.buf, sizeof( info.buf ),
											  &info.bufOccupancy, RTF_BUFSEEK_NONE, 0 );
			// error in read callback?
			if( iResult < 0 )
			{
				RTF_LOG_ERR0( RTF_MSG_ERR_CALLBACKFAILED, "Application read input callback failed" );
				break;
			}
			// end of file?
			if( info.bufOccupancy == 0 )
			{
				// yes - escape read buffer loop
				break;
			}
			// scan the buffer just read
			info.offset = 0;
			while( info.offset < (int)info.bufOccupancy )
			{
				// in sync?
				if( info.inSync == 0 )
				{
					// no - generate warning
					RTF_LOG_WARN1( RTF_MSG_WRN_SYNCLOSS, "Sync lost at packet %d", info.totalPktCt );
					// if warning is overridden, check the next byte for sync
					result = rtfSyncSearch( pSes, &info );
					RTF_CHK_RESULT;
				}
				else
				{
					// yes - check the next packet boundary for sync
					result = rtfSyncCheck( pSes, &info );
					RTF_CHK_RESULT;
				}
			} // while( off < bufOccupancy ) (packet loop)
			RTF_CHK_RESULT_LOOP;
		} // for( i=0; ; ++i ) (buffer loop)
		RTF_CHK_RESULT_LOOP;
		// rewind the input file to the beginning
		iResult = ( *readInputCallback )( pSes->hAppSession, info.buf, 0, &info.bufOccupancy, RTF_BUFSEEK_SET, 0 );
		// error in read callback?
		if( iResult < 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_CALLBACKFAILED, "Application read input callback failed at rewind" );
			break;
		}
		// were PCR packets detected?
		deltaPktCt = info.lastPcrPktCt - info.firstPcrPktCt;
		if( deltaPktCt == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADSTREAM, "Insufficient PCRs to measure bit rate were found in the input stream" );
			break;
		}
		deltaPcrValue = ( info.lastPcrValue - info.firstPcrValue );
		if( deltaPcrValue < 0 )
		{
			temp64 = 1;
			temp64 <<= 33;
			deltaPcrValue += temp64;
		}
		msec = ( (double)deltaPcrValue ) / 27000.0;
		bits = (double)deltaPktCt * ( (double)TRANSPORT_PACKET_BYTES ) * 8.0;
		bps = ( bits * 1000.0 ) / msec;
		info.augmentedBitRate = (unsigned long)bps;
		// return the augmented bit rate
		*pAugmentedBitRate = info.augmentedBitRate;
		// record the augmentation info in the profile
		for( i=0; i<augmentationPidCount; ++i )
		{
			pSes->profile.augmentationPids[ i ] = pAugmentationPids[ i ];
		}
		pSes->profile.augmentationPidCount = augmentationPidCount;
		if( ( info.basePktCt     == 0 ) ||
			( info.augPktCt      == 0 ) ||
			( info.baseClusterCt == 0 ) ||
			( info.augClusterCt  == 0 ) )
		{
			pSes->profile.augmentationBaseFactor = 1;
			pSes->profile.augmentationPlusFactor = 0;
			*pOriginalBitRate = info.augmentedBitRate;
		}
		else
		{
			pSes->profile.augmentationBaseFactor = info.basePktCt / info.baseClusterCt;
			pSes->profile.augmentationBaseFactor = info.augPktCt  / info.augClusterCt;
			*pOriginalBitRate = (unsigned long)( ( 1.0 - ( (double)info.augPktCt / (double)info.basePktCt ) ) * bps );
		}
		// set up the input bit rate and the user bit rate in the profile, and in each of the outputs
		pSes->profile.bitsPerSecond = *pOriginalBitRate;
		pSes->profile.userBitsPerSecond = info.augmentedBitRate;
		pSes->bitRateAcquired = TRUE;
		for( i=0; i<pSes->outputCount; ++i )
		{
			result = rtfOutSetUserBitRate( pSes->hOutput[ i ], info.augmentedBitRate );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
		// return the original bit rate
		RTF_LOG_INFO1( RTF_MSG_INF_STATS, "Augmented input bit rate is %d bits per second", *pAugmentedBitRate );
		RTF_LOG_INFO1( RTF_MSG_INF_STATS, "Original  input bit rate is %d bits per second", *pOriginalBitRate );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// check to see if a packet is an augmentation packet
RTF_RESULT rtfSesGetAugmentationPktStatus( RTF_SES_HANDLE handle, unsigned char *pPacket, BOOL *pIsAugmentation )
{
	RTF_FNAME( "rtfSesGetAugmentationPktStatus" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;
	unsigned short pid;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// innocent until proven guilty
		*pIsAugmentation = FALSE;
		// are there any augmentation PIDs?
		if( pSes->profile.augmentationPidCount != 0 )
		{
			// yes - get the PID from the packet
			pid = pPacket[ 1 ] & 0x1F;
			pid = ( pid << 8 ) | pPacket[ 2 ];
#if 0 // !!! TEST HACK !!! TREAT VM ECM PACKETS AS AUGMENTATION !!!
			// is it the NULL PID?
			if( pid == TRANSPORT_PAD_PID )
			{
				// yes. Does it have the VM ECM signature?
				if( ( pPacket[ 4 ] == RTF_VMECM_SIGNATURE ) &&
					( pPacket[ 5 ] == RTF_VMECM_SIGNATURE ) &&
					( pPacket[ 6 ] == RTF_VMECM_SIGNATURE ) &&
					( pPacket[ 7 ] == RTF_VMECM_SIGNATURE ) )
				{
					*pIsAugmentation = TRUE;
				}
			}
			else
#endif
			{
				// no - see if it is on the augmentation list
				for( i=0; i<pSes->profile.augmentationPidCount; ++i )
				{
					if( pid == pSes->profile.augmentationPids[ i ] )
					{
						*pIsAugmentation = TRUE;
						break;
					}
				}
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}


// check to see if the session is a zombie - shut it down if it is
RTF_RESULT rtfSesZombieCheck( RTF_SES_HANDLE handle, unsigned long currentTime, int threshold )
{
	RTF_FNAME( "rtfSesZombieCheck" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	int delta;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		// is this session idle? (idle sessions aren't zombies)
		if( pSes->state != RTF_SES_STATE_IDLE )
		{
			// no - how long has it been since the last process input call?
			// (if no input, how long since the session was opened?)
			delta = currentTime - pSes->lastProcessInputTime;
			delta = ( delta >= 0 ) ? delta : 0x80000000 + delta;
			if( delta > threshold )
			{
				// this session has been idle too long; it's a zombie
				RTF_LOG_ERR3( RTF_MSG_ERR_ZOMBIE, "Session %d: idle time (%d ticks) exceeds zombie threshold (%d). Session terminated",
							  pSes->hAppSession, delta, threshold );
				break;
			}
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

#ifdef _DEBUG
static char *rtfSesStateString[] =
{ "invalid", "idle", "active", "unrecognized" };

// debug helper function - log the state of the session
RTF_RESULT rtfSesLogState( RTF_SES_HANDLE handle )
{
	RTF_FNAME( "rtfSesLogState" );
	RTF_OBASE( handle );
	RTF_SES *pSes = (RTF_SES *)handle;
	RTF_RESULT result = RTF_PASS;
	int i, state;
	static int callCount = 0;

	RTF_LOG_INFO0( RTF_MSG_INF_STATS, "****** rtfSesLogState: BEGIN ******" );
	RTF_LOG_INFO1( RTF_MSG_INF_STATS, "*********** call # %08d ***********", callCount );

	do {		 // error escape wrapper - begin

		if( pSes == (RTF_SES *)NULL )
		{
			RTF_LOG_INFO0( RTF_MSG_INF_LOGNULLHANDLE, "Null session handle" );
			break;
		}
		RTF_CHK_OBJ( pSes, RTF_OBJ_TYPE_SES );
		state = ( pSes->state > RTF_SES_STATE_ACTIVE ) ? RTF_SES_STATE_ACTIVE+1 : pSes->state;
		RTF_LOG_INFO2( RTF_MSG_INF_STATS, "infile=%s state=%s",
					   pSes->inputFileName, rtfSesStateString[ state ] );
		RTF_LOG_INFO2( RTF_MSG_INF_STATS, "startTime=%d stopTime=%d",
					   (int)pSes->startTime, (int)pSes->stopTime );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "inputInSync=%d inputIsTTS=%d psiValid=%d",
					   (int)pSes->inputInSync, (int)pSes->inputIsTTS, (int)pSes->psiValid );
#ifdef DO_STATISTICS
		RTF_LOG_INFO2( RTF_MSG_INF_STATS, "totalInputBufferCount=%d totalMappedPacketCount=%d",
					   (int)pSes->totalInputBufferCount, (int)pSes->totalMappedPacketCount );
		RTF_LOG_INFO3( RTF_MSG_INF_STATS, "totalPicCount=%d totalGopCount=%d totalSeqCount=%d",
					   (int)pSes->totalPicCount, (int)pSes->totalGopCount, (int)pSes->totalSeqCount );
		RTF_LOG_INFO4( RTF_MSG_INF_STATS, "bufCount=%d picCount=%d gopCount=%d outputCount=%d",
					   (int)pSes->bufCount, (int)pSes->picCount, (int)pSes->gopCount, (int)pSes->outputCount );
		RTF_LOG_INFO1( RTF_MSG_INF_STATS, "packetFragmentBytes=%d",
					   (int)pSes->packetFragmentBytes );
#endif
		// dump the output states
		RTF_LOG_INFO0( RTF_MSG_INF_STATS, "OUTPUTS *****" );
		for( i=0; i<pSes->outputCount; ++i )
		{
			rtfOutLogState( pSes->hOutput[ i ] );
		}
		// dump the filter states
		RTF_LOG_INFO0( RTF_MSG_INF_STATS, "FILTERS *****" );
		for( i=0; i<pSes->outputCount; ++i )
		{
			if( i != pSes->indexFileOutputNumber )
			{
				rtfFltLogState( pSes->hFilter[ i ] );
			}
		}
#if 0
		// dump the active buffer states
		RTF_LOG_INFO0( RTF_MSG_INF_STATS, "BUFFERS *****" );
		for( i=0; i<pSes->bufCount; ++i )
		{
			rtfBufferLogState( pSes->hBuf[ i ] );
		}
#endif

	} while( 0 ); // error escape wrapper - end

	RTF_LOG_INFO0( RTF_MSG_INF_STATS, "******* rtfSesLogState: END *******" );

	return result;
}
#endif
