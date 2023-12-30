// rtfMainTest.c : test loop for use with kernel-mode library
//

#include <linux/module.h>
#include "RTFLib.h"
#include "RTFMain.h"

#ifdef TEST_FUNC

#include "RTFMainTest.h"

// output file handles ******************************************************************

static int inputFile [ MAX_SESSION_COUNT ];
static int outputFile[ MAX_SESSION_COUNT ][ OUTPUT_FILE_COUNT ];

// file I/O buffer pools ****************************************************************

RTF_SES_HANDLE hSession[ MAX_SESSION_COUNT ];
static char *pInputPath[ MAX_SESSION_COUNT ];
static int bufGetCount [ MAX_SESSION_COUNT ];
static int bufRelCount [ MAX_SESSION_COUNT ];
static int maxBuffersInUseCount [ MAX_SESSION_COUNT ];
static BOOL inputFileBufferInUse[ MAX_SESSION_COUNT ][ INPUT_FILE_BUFFER_COUNT ];
static unsigned char *pInputFileBuffer [ MAX_SESSION_COUNT ][ INPUT_FILE_BUFFER_COUNT ];
static unsigned char *pOutputFileBuffer[ MAX_SESSION_COUNT ][ OUTPUT_FILE_COUNT ];
static RTF_APP_OUTPUT_SETTINGS settings[ MAX_SESSION_COUNT ][ OUTPUT_FILE_COUNT ];

// buffer management ********************************************************************
static int getNextFreeInputBuffer( int sessionNumber, unsigned char **ppBuf)
{
	int i, result = 0;

	for( i=0; i<INPUT_FILE_BUFFER_COUNT; ++i )
	{
		if( inputFileBufferInUse[ sessionNumber ][ i ] == FALSE )
		{
			*ppBuf = pInputFileBuffer[ sessionNumber ][ i ];
			inputFileBufferInUse[ sessionNumber ][ i ] = TRUE;
			++bufGetCount[ sessionNumber ];
			maxBuffersInUseCount[ sessionNumber ] =
				MAX( maxBuffersInUseCount[ sessionNumber ],
				     (bufGetCount[ sessionNumber ] - bufRelCount[ sessionNumber ]) );
			break;
		}
	}
	if( i >= INPUT_FILE_BUFFER_COUNT )
	{
		PRINTF( "Failed to find free input file buffer\n" );
		result = -1;
	}
	return result;
}

// session error notifier callback ******************************************************

// this function is invoked when the session encounters an error
static void sessionErrorNotifier( char *pMessage )
{
	PRINTF( "Session error notifier called with following message:\n%s\n", pMessage );
	exit( -1 );
}

// input buffer release callback function ***********************************************

// this callback function is invoked when the trickfile library is done with
// an input buffer that was provided to the rtfSesProcessInput call
int releaseInputBuffer( void *hAppSession, void *hAppFile, void *hAppBuffer, unsigned char *pBuffer )
{
	int i;
	int result = 0;
	int sessionNumber = (int)hAppSession;

	// find the indicated input buffer in the pool
	for( i=0; i<INPUT_FILE_BUFFER_COUNT; ++i )
	{
		if( pInputFileBuffer[ sessionNumber ][ i ] == pBuffer )
		{
			break;
		}
	}
	// was it there?
	if( i < INPUT_FILE_BUFFER_COUNT )
	{
		// make sure the "in use" flag is set
		if( inputFileBufferInUse[ sessionNumber ][ i ] == TRUE )
		{
			// clear the flag
			inputFileBufferInUse[ sessionNumber ][ i ] = FALSE;
			++bufRelCount[ sessionNumber ];
		}
		else
		{
			PRINTF( "Input buffer 0x%x not in use at release\n", pBuffer );
			result = -1;
		}
	}
	else
	{
		PRINTF( "Failed to find input buffer 0x%x in pool\n", pBuffer );
		result = -1;
	}

	return 0;
}

// trickfile callback functions *********************************************************

// the trickfile library calls this function to obtain an empty output buffer
static int trickFileGetOutputBuffer( void *hAppSession, void *hAppFile, void **phAppBuffer,
									 unsigned char **ppBuffer, unsigned long *pCapacity )
{
	int result = 0;
	int sessionNumber = (int)hAppSession;
	int outFileNumber = (int)hAppFile;

	// make sure the file number is in range
	if( outFileNumber < OUTPUT_FILE_COUNT )
	{
		// make the returns
		// note: only one output buffer was allocated!
		*phAppBuffer = (void *)0;
		*ppBuffer = pOutputFileBuffer[ sessionNumber ][ outFileNumber ];
		*pCapacity = OUTPUT_FILE_BUFFER_BYTES;
	}
	else
	{
		PRINTF( "appOutputHandle (%d) not recognized in trickFileGetOutputBuffer callback\n", outFileNumber );
		result = -1;
	}

	return result;
}

// the trickfile library calls this function to write a full output buffer
static int trickFilePutOutputBuffer( void *hAppSession, void *hAppFile, void *hAppBuffer,
									 unsigned char *pBuffer, unsigned long occupancy,
									 RTF_BUFSEEK bufSeek, INT64 bufSeekOffset )
{
	int result = 0;
	INT64 offset, size;
	int sessionNumber = (int)hAppSession;
	int outFileNumber = (int)hAppFile;
	int bufferNumber = (int)hAppBuffer;

	// make sure the session handle is in range
	if( sessionNumber >= MAX_SESSION_COUNT )
	{
		PRINTF( "Unrecognized app session handle (%d) in trickFilePutOutputBuffer\n", sessionNumber );
		return -1;
	}
	// make sure the file handle is in range
	if( outFileNumber >= OUTPUT_FILE_COUNT )
	{
		PRINTF( "Unrecognized app file handle (%d) in trickFilePutOutputBuffer\n", outFileNumber );
		return -1;
	}
	// make sure the buffer handle is in range
	// note: only one buffer was allocated for outputs!
	if( bufferNumber != 0 )
	{
		PRINTF( "Unrecognized app buffer handle (%d) in trickFilePutOutputBuffer\n", bufferNumber );
		return -1;
	}
	// perform a seek if that is indicated
	switch( bufSeek )
	{
	case RTF_BUFSEEK_NONE:
		break;
	case RTF_BUFSEEK_SET:
		offset = RTF_LSEEK64( outputFile[ sessionNumber ][ outFileNumber ], bufSeekOffset, SEEK_SET );
		break;
	case RTF_BUFSEEK_CUR:
		size = RTF_LSEEK64( outFileNumber, 0, SEEK_CUR );
		offset = RTF_LSEEK64( outputFile[ sessionNumber ][ outFileNumber ], bufSeekOffset, SEEK_CUR );
		break;
	case RTF_BUFSEEK_END:
		offset = RTF_LSEEK64( outputFile[ sessionNumber ][ outFileNumber ], bufSeekOffset, SEEK_END );
		break;
	default:
		PRINTF( "Invalid RTF_BUFSEEK (%d) in trickFilePutOutputBuffer\n", bufSeek );
		result = -1;
		break;
	}
	// write the buffer to the indicated output file
	if( RTF_WRITE( outputFile[ sessionNumber ][ outFileNumber ], pBuffer, occupancy ) != (int)occupancy )
	{
		PRINTF( "Write failed in trickFilePutOutputBuffer\n" );
		return -1;
	}

	return 0;
}

// the trickfile library calls this function when it is done with an output file
static int trickFileCloseOutput( void *hAppSession, void *hAppFile )
{
	int result = 0;
	int sessionNumber = (int)hAppSession;
	int outFileNumber = (int)hAppFile;

	// make sure the session number is in range
	if( sessionNumber >= OUTPUT_FILE_COUNT )
	{
		PRINTF( "Unrecognized app session handle (%d) in trickFileCloseOutput\n",
				sessionNumber );
		return -1;
	}
	// make sure the file number is in range
	if( outFileNumber >= OUTPUT_FILE_COUNT )
	{
		PRINTF( "Unrecognized app file handle (%d) in trickFileCloseOutput\n",
				outFileNumber );
		return -1;
	}
	// close the file
	if( RTF_CLOSE( outputFile[ sessionNumber ][ outFileNumber ] ) != 0 )
	{
		PRINTF( "Close failed in trickFileCloseOutput - file %d\n", outFileNumber );
		PERROR( "" );
		return -1;
	}

	return result;
}

// application local functions **********************************************************

static void usage( char *pCmd )
{
	PRINTF( "Usage: %s <S or P> <input stream #1> [ <input stream #2> ... ]\n", pCmd );
	PRINTF( "Note:  S = serial stream processing, P = parallel stream processing\n" );
}

static int openFiles( int sessionNumber )
{
	char outFileName[ RTF_MAX_PATH_LEN ];
	char ext[ RTF_MAX_EXT_LEN ];
	int i, outFileNumber;
	int result = 0;

	do {			// error escape wrapper - begin

		// open the input file
		if( ( inputFile[ sessionNumber ] = RTF_OPEN( pInputPath[ sessionNumber ], RTF_OPEN_SRC_FLAGS ) ) < 0 )
		{
			PRINTF( "Failed to open input file %s\n", pInputPath[ sessionNumber ] );
			PERROR( "" );
			result = -1;
			break;
		}
		// open a set of output files for this stream
		outFileNumber = 0;

		// open the index file and initialize its settings
		rtfInitializeOutputSettings( &(settings[ sessionNumber ][ outFileNumber ]), (void *)outFileNumber, 0, 1, ".vvx",
				trickFileGetOutputBuffer, trickFilePutOutputBuffer, trickFileCloseOutput );
		sprintf( outFileName, "%s.vvx", pInputPath[ sessionNumber ] );
		outputFile[ sessionNumber ][ outFileNumber ] =
				RTF_OPEN( outFileName, RTF_OPEN_DST_FLAGS, RTF_OPEN_DST_PERMS );
		if( outputFile[ sessionNumber ][ outFileNumber ] < 0 )
		{
			PRINTF( "open failed on output file %s\n", outFileName );
			result = -1;
			break;
		}
		// bump the file number
		++outFileNumber;

		// open the forward trick files, and set up their trick specs
		for( i=0; i<TRICK_SPEED_COUNT; ++i )
		{
			if( i == 0 )
			{
				sprintf( outFileName, "%s.ff", pInputPath[ sessionNumber ] );
			}
			else
			{
				sprintf( outFileName, "%s.ff%d", pInputPath[ sessionNumber ], i );
			}
			outputFile[ sessionNumber ][ outFileNumber ] =
					RTF_OPEN( outFileName, RTF_OPEN_DST_FLAGS, RTF_OPEN_DST_PERMS );
			if( outputFile[ sessionNumber ][ outFileNumber ] < 0 )
			{
				PRINTF( "open failed on output file %s\n", outFileName );
				result = -1;
				break;
			}
			sprintf( ext, ".ff%d", i );
			rtfInitializeOutputSettings( &settings[ sessionNumber ][ outFileNumber ],
					(void *)outFileNumber, ((i+1)*15), 2, ext, trickFileGetOutputBuffer,
					trickFilePutOutputBuffer, trickFileCloseOutput );
			// bump the file number
			++outFileNumber;
		}
		// escape on error in nested loop
		if( result != 0 )
		{
			break;
		}

#ifndef DO_COMBINED_FWD_REV
		// open the reverse trick files, and set up their trick specs
		for( i=0; i<TRICK_SPEED_COUNT; ++i )
		{
			// reverse trick file
			if( i == 0 )
			{
				sprintf( outFileName, "%s.fr", pInputPath[ sessionNumber ] );
			}
			else
			{
				sprintf( outFileName, "%s.fr%d", pInputPath[ sessionNumber ], i );
			}
			outputFile[ sessionNumber ][ outFileNumber ] =
					RTF_OPEN( outFileName, RTF_OPEN_DST_FLAGS, RTF_OPEN_DST_PERMS );
			if( outputFile[ sessionNumber ][ outFileNumber ] < 0 )
			{
				PRINTF( "open failed on output file %s\n", outFileName );
				result = -1;
				break;
			}
			sprintf( ext, ".fr%d", i );
			rtfInitializeOutputSettings( &settings[ sessionNumber ][ outFileNumber ],
					(void *)outFileNumber, -((i+1)*15), 2, ext, trickFileGetOutputBuffer,
					trickFilePutOutputBuffer, trickFileCloseOutput );
			// bump the file number
			++outFileNumber;
		}
#endif

	} while( 0 );			// error escape wrapper - end

	return result;
}

static int allocBuffers( int sessionNumber )
{
	int i;
	int result = 0;

	do {			// error escape wrapper - begin

		// allocate a set of input file buffers
		for( i=0; i<INPUT_FILE_BUFFER_COUNT; ++i )
		{
			pInputFileBuffer[ sessionNumber ][ i ] =
					(unsigned char *)malloc( INPUT_FILE_BUFFER_BYTES );
			if( pInputFileBuffer[ sessionNumber ][ i ] == (unsigned char *)NULL )
			{
				PRINTF( "Failed to allocate input buffer for file %s\n",
						pInputPath[ sessionNumber ]);
				result = -1;
				break;
			}
			inputFileBufferInUse[ sessionNumber ][ i ] = FALSE;
		}
		// escape on error in nested loop
		if( result != 0 )
		{
			break;
		}

		// allocate a set of trickfile output buffers
		for( i=0; i<OUTPUT_FILE_COUNT; ++i )
		{
			pOutputFileBuffer[ sessionNumber ][ i ] =
					(unsigned char *)malloc( OUTPUT_FILE_BUFFER_BYTES );
			if( pOutputFileBuffer[ sessionNumber ][ i ] == (unsigned char *)NULL )
			{
				PRINTF( "Failed to allocate trickfile output buffer for file %s\n",
					    pInputPath[ sessionNumber ] );
				result = -1;
				break;
			}
		}

	} while( 0 );			// error escape wrapper - end

	return result;
}

static int openSession( int sessionNumber )
{
	int result = 0;

	do {			// error escape wrapper - begin

		PRINTF( "Opening session for input file %s\n", pInputPath[ sessionNumber ] );
		// open the session
		if( rtfOpenSession( &hSession[ sessionNumber ],
							(RTF_APP_SESSION_HANDLE)sessionNumber,
							(RTF_APP_FILE_HANDLE)(inputFile[ sessionNumber ]),
							pInputPath[ sessionNumber ], 1, sessionErrorNotifier,
							releaseInputBuffer, OUTPUT_FILE_COUNT,
							settings[ sessionNumber ] ) != TRUE )
		{
			PRINTF( "rtfOpenSession failed for file %s\n", pInputPath[ sessionNumber ] );
			result = -1;
			break;
		}
#if 0
		{
			int i;

			PRINTF( "Session opened. Output settings:\n" );
			for( i=0; i<OUTPUT_FILE_COUNT; ++i )
			{
				rtfLogOutputSettings( hSession[ sessionNumber ], i );
			}
		}
#else
		PRINTF( "Session opened\n" );
#endif
	} while( 0 );			// error escape wrapper - end

	return result;
}

static int processBuffer( int sessionNumber )
{
	int bytes;
	unsigned char *pBuf;
	int result = 0;

	do {			// error escape wrapper - begin

		// get the next available input buffer
		if( getNextFreeInputBuffer( sessionNumber, &pBuf) != 0 )
		{
			PRINTF( "getNextFreeInputBuffer failed in main loop\n" );
			result = -1;
			break;
		}
		// read some input data into the buffer
		if( ( bytes = RTF_READ( inputFile[ sessionNumber ], pBuf, INPUT_FILE_BUFFER_BYTES ) ) == 0 )
		{
			PRINTF( "end of input in file %s\n", pInputPath[ sessionNumber ] );
			break;
		}
		if( bytes < 0 )
		{
			PRINTF( "input read failed in file %s\n", pInputPath[ sessionNumber ] );
			PERROR( "" );
			result = -1;
			break;
		}
		// send the buffer of data to the trickfile library
		if( rtfProcessInput( hSession[ sessionNumber ],
							 (RTF_APP_SESSION_HANDLE)sessionNumber,
							 (RTF_APP_FILE_HANDLE)inputFile[ sessionNumber ],
							 (RTF_APP_BUFFER_HANDLE)pBuf,
							 pBuf, (unsigned long)bytes ) != TRUE )
		{
			PRINTF( "rtfSesProcessInput failed for file %s\n", pInputPath[ sessionNumber ] );
			result = -1;
			break;
		}

	} while( 0 );			// error escape wrapper - end

	return bytes;
}

static int closeSession( int sessionNumber )
{
	if( rtfCloseSession( hSession[ sessionNumber ] ) != TRUE )
	{
		return -1;
	}
	return 0;
}

// test loop ****************************************************************************

int module_test( int argc, char* argv[] )
{
	int streamCount;
	int sessionNumber;
	int bytes;
	int bufCount;
	char mode;
	char version[ 16 ];
	BOOL sessionContinue[ MAX_SESSION_COUNT ];
	BOOL loopContinue;
	int result = 0;

	// check the arguments **********************************************************

	// how many streams?
	streamCount = argc - 2;
	if( streamCount < 1 )
	{
		usage( argv[ 0 ] );
		return -1;
	}

	// serial or parallel?
	mode = toupper( *argv[ 1 ] );
	if( ( mode != 'S' ) && ( mode != 'P' ) )
	{
		usage( argv[ 0 ] );
		return -1;
	}

	// if parallel, can we do that many?
	if( ( mode == 'P' ) && ( streamCount > MAX_SESSION_COUNT ) )
	{
		PRINTF( "Too many streams (%d) for parallel processing (max=%d)\n", streamCount, MAX_SESSION_COUNT );
		return -1;
	}

	// reset some local static variables ********************************************

	memset( (void *)hSession,				0, sizeof(hSession)				);
	memset( (void *)inputFile,				0, sizeof(inputFile)			);
	memset( (void *)outputFile,				0, sizeof(outputFile)			);
	memset( (void *)pInputPath,				0, sizeof(pInputPath)			);
	memset( (void *)bufGetCount,			0, sizeof(bufGetCount)			);
	memset( (void *)bufRelCount,			0, sizeof(bufRelCount)			);
	memset( (void *)maxBuffersInUseCount,	0, sizeof(maxBuffersInUseCount) );
	memset( (void *)inputFileBufferInUse,	0, sizeof(inputFileBufferInUse) );
	memset( (void *)pInputFileBuffer,		0, sizeof(pInputFileBuffer)		);
	memset( (void *)pOutputFileBuffer,		0, sizeof(pOutputFileBuffer)	);
	memset( (void *)settings,				0, sizeof(settings)				);

	// iterate over the input streams ***********************************************

	if( mode == 'S' )
	{
		// process each of the sessions serially
		for( sessionNumber=0; sessionNumber<streamCount; ++sessionNumber )
		{
			pInputPath[ sessionNumber ] = argv[ sessionNumber + 2 ];
			if( openFiles( sessionNumber ) != 0 )
			{
				PRINTF( "openFiles failed on file %s\n", pInputPath[ sessionNumber ] );
				return -1;
			}

			if( allocBuffers( sessionNumber ) != 0 )
			{
				PRINTF( "allocBuffers failed on file %s\n", pInputPath[ sessionNumber ] );
				return -1;
			}

			if( openSession( sessionNumber ) != 0 )
			{
				PRINTF( "openSession failed on file %s\n", pInputPath[ sessionNumber ] );
				return -1;
			}

			bufCount = 0;
			do
			{
				bytes = processBuffer( sessionNumber );
				if( bytes < 0 )
				{
					PRINTF( "processBuffer failed on file %s\n", pInputPath[ sessionNumber ] );
					return -1;
				}
				++bufCount;
			} while( bytes > 0 );

#ifdef _DEBUG
//			rtfSesLogState( hSession[ sessionNumber ] );
#endif
			if( closeSession( sessionNumber ) != 0 )
			{
				PRINTF( "closeSession failed on file %s\n", pInputPath[ sessionNumber ] );
				return -1;
			}

			PRINTF( "Finished processing file %s\n\n",  pInputPath[ sessionNumber ] );
		}
	}
	else
	{
		// process all of the sessions in parallel
		for( sessionNumber=0; sessionNumber<streamCount; ++sessionNumber )
		{
			pInputPath[ sessionNumber ] = argv[ sessionNumber + 2 ];
			if( openFiles( sessionNumber ) != 0 )
			{
				PRINTF( "openFiles failed on file %s\n", pInputPath[ sessionNumber ] );
				return -1;
			}
		}

		for( sessionNumber=0; sessionNumber<streamCount; ++sessionNumber )
		{
			if( allocBuffers( sessionNumber ) != 0 )
			{
				PRINTF( "allocBuffers failed on file %s\n", pInputPath[ sessionNumber ] );
				return -1;
			}
		}

		for( sessionNumber=0; sessionNumber<streamCount; ++sessionNumber )
		{
			if( openSession( sessionNumber ) != 0 )
			{
				PRINTF( "openSession failed on file %s\n", pInputPath[ sessionNumber ] );
				return -1;
			}
		}

		for( sessionNumber=0; sessionNumber<MAX_SESSION_COUNT; ++sessionNumber )
		{
			sessionContinue[ sessionNumber ] = TRUE;
		}
		bufCount = 0;
		do
		{
			loopContinue = FALSE;
			for( sessionNumber=0; sessionNumber<streamCount; ++sessionNumber )
			{
				if( sessionContinue[ sessionNumber ] == TRUE )
				{
					bytes = processBuffer( sessionNumber );
					if( bytes < 0 )
					{
						PRINTF( "processBuffer failed on file %s\n", pInputPath[ sessionNumber ] );
						return -1;
					}
					else if( bytes == 0 )
					{
						sessionContinue[ sessionNumber ] = FALSE;
					}
					else
					{
						loopContinue = TRUE;
					}
				}
			}
			++bufCount;
		} while( loopContinue == TRUE );

		for( sessionNumber=0; sessionNumber<streamCount; ++sessionNumber )
		{
#ifdef _DEBUG
//			rtfSesLogState( hSession[ sessionNumber ] );
#endif
			if( closeSession( sessionNumber ) != 0 )
			{
				PRINTF( "closeSession failed\n" );
				return -1;
			}
		}
	}

	PRINTF( "Done\n" );

	return 0;
}

#endif // #ifdef TEST_FUNC
