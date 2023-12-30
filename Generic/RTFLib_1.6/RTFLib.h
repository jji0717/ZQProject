// master include file for RTFLib
// provides all externally visible info
//

#ifndef _RTF_LIB_H
#define _RTF_LIB_H 1

#ifdef __cplusplus
extern "C" {
#endif

// system include files *****************************************************************

#define RTFLIB_SDK_VERSION	16

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif

#ifndef __KERNEL__
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#endif

#ifdef _LINUX
#include <linux/types.h>
#ifdef __KERNEL__
#include <linux/time.h>
#include <linux/slab.h>
#endif
#endif

// system-level constants and typedefs **************************************************

#ifdef _WIN32
typedef __int64					INT64;
typedef unsigned __int64		UINT64;
#endif

#ifdef _LINUX
#define TRUE					1
#define FALSE					0
#define NO_ERROR				0
#define O_BINARY				0
#define O_SEQUENTIAL			0
#ifdef __KERNEL__
#define SEEK_SET				0
#define SEEK_CUR				1
#define SEEK_END				2
#endif
typedef void					VOID;
typedef void*					PVOID;
typedef unsigned char			BOOL;
typedef unsigned char			BOOLEAN;
typedef char					CHAR;
typedef char*					PCHAR;
typedef unsigned char			UCHAR;
typedef unsigned char*			PUCHAR;
typedef short					SHORT;
typedef short*					PSHORT;
typedef unsigned short			USHORT;
typedef unsigned short*			PUSHORT;
typedef unsigned int			UINT;
typedef unsigned int*			PUINT;
#ifdef DO_NATIVE64
typedef long long				LONG;
typedef long long*				PLONG;
typedef unsigned long long		ULONG;
typedef unsigned long long*		PULONG;
#else
typedef long					LONG;
typedef long*					PLONG;
typedef unsigned long			ULONG;
typedef unsigned long*			PULONG;
#endif
typedef unsigned long long		ULONGLONG;
typedef unsigned long long*		PULONGLONG;
typedef unsigned long long		UINT64;
typedef unsigned char			BYTE;
typedef BYTE*					PBYTE;
typedef unsigned short			WORD;
typedef WORD*					PWORD;
typedef unsigned long			DWORD;
typedef DWORD*					PDWORD;
typedef long long				INT64;

typedef union _LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		DWORD HighPart;
	};
	long long QuadPart;
} LARGE_INTEGER;

typedef struct _SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME;

typedef struct _FILETIME {
    DWORD dwLowDateTime;   /* low 32 bits  */
    DWORD dwHighDateTime;  /* high 32 bits */
} FILETIME, *PFILETIME;
#endif

typedef INT64		QUADWORD;
typedef UINT64		UQUADWORD;

// constants ****************************************************************************

// internal API results
#define RTF_PASS						(const char *)NULL
#define RTF_FAIL						__FNAME__

// RTF specific
#define RTF_TRICK_MAX_PIDCOUNT			16
#define RTF_MAX_EXT_LEN					8
#define RTF_MAX_PATH_LEN				128
#define RTF_MAX_TRICKSPEEDS				4
#define RTF_MAX_TRICKFILES				( 2 * RTF_MAX_TRICKSPEEDS )
#define RTF_ABSMAX_SESCOUNT				64
#define RTF_MAX_LANG_COUNT				4
#define RTF_MAX_DESC_COUNT				2
#define RTF_MAX_AUDIO_PIDS				8
#define RTF_MAX_DATA_PIDS				8
#define RTF_MAX_AUGMENTATIONPIDS		4

// RTF SD setup defaults
#define RTF_SD_DEFAULT_SES_COUNT		1
#define RTF_SD_DEFAULT_INPBUF_PERSES	128
#define RTF_SD_DEFAULT_GROUPS_PERSEQ	2
#define RTF_SD_DEFAULT_PICS_PERGROUP	30
#define RTF_SD_DEFAULT_INPBUF_BYTES		(  8 * 1024 )
#define RTF_SD_DEFAULT_OUTBUF_BYTES		( 64 * 1024 )
#define RTF_SD_DEFAULT_FAIL_THRESH		1

// RTF HD setup defaults
#define RTF_HD_DEFAULT_SES_COUNT		1
#define RTF_HD_DEFAULT_INPBUF_PERSES	512
#define RTF_HD_DEFAULT_GROUPS_PERSEQ	2
#define RTF_HD_DEFAULT_PICS_PERGROUP	30
#define RTF_HD_DEFAULT_INPBUF_BYTES		(  8 * 1024 )
#define RTF_HD_DEFAULT_OUTBUF_BYTES		( 64 * 1024 )
#define RTF_HD_DEFAULT_FAIL_THRESH		1

// transport specific
#define TRANSPORT_SYNCBYTE_OFFSET		0
#define TRANSPORT_PACKET_BYTES			188
#define TRANSPORT_INVALID_PID			0xFFFF
#define TRANSPORT_MAX_PID_VALUES		0x2000
#define TRANSPORT_SCR_TICKS_PER_SECOND	27000000
#define TRANSPORT_SCR_TO_PCR_RATIO		300
#define TRANSPORT_PCR_TICKS_PER_SECOND	( TRANSPORT_SCR_TICKS_PER_SECOND / TRANSPORT_SCR_TO_PCR_RATIO )
#define TRANSPORT_MAX_PCR_GAP_TICKS		( TRANSPORT_SCR_TICKS_PER_SECOND / 10 )

// TTS transport definitions
#define TTS_SYNCBYTE_OFFSET				4
#define TTS_PACKET_BYTES				( TRANSPORT_PACKET_BYTES + TTS_SYNCBYTE_OFFSET )

// macros *******************************************************************************

#ifdef _WIN32
#define RTF_DIV64(N,D)		((N)/(D))
#define RTF_NATIVE_OPEN		_open
#define RTF_NATIVE_CLOSE	_close
#define RTF_NATIVE_READ		_read
#define RTF_NATIVE_WRITE	_write
#define RTF_NATIVE_LSEEK	_lseek
#define RTF_NATIVE_LSEEK64	_lseeki64
#define RTF_OPEN_SRC_FLAGS	( _O_RDONLY | _O_BINARY | _O_SEQUENTIAL )
#define RTF_OPEN_DST_FLAGS	( _O_WRONLY | _O_BINARY | _O_SEQUENTIAL | _O_CREAT | _O_TRUNC )
#define RTF_OPEN_DST_PERMS	( _S_IWRITE )
#endif

#ifdef _LINUX
#ifdef __KERNEL
#define S_IWRITE			S_IWUSR
#endif
#if ( defined(__KERNEL__) || (defined(_EMBEDDED) ) )
#define RTF_DIV64(N,D)		RTFDiv64((N),(D))
#else
#define RTF_DIV64(N,D)		((N)/(D))
#endif // #ifdef _LINUX
#define RTF_NATIVE_OPEN		open
#define RTF_NATIVE_CLOSE	close
#define RTF_NATIVE_READ		read
#define RTF_NATIVE_WRITE	write
#define RTF_NATIVE_LSEEK	lseek
#define RTF_NATIVE_LSEEK64	lseek64
#define RTF_OPEN_SRC_FLAGS	( O_RDONLY | O_BINARY | O_SEQUENTIAL )
#define RTF_OPEN_DST_FLAGS	( O_WRONLY | O_BINARY | O_SEQUENTIAL | O_CREAT | O_TRUNC )
#define RTF_OPEN_DST_PERMS	( S_IWRITE )
#endif // #ifdef LINUX

#ifdef DO_DEBUGIO
#define RTF_OPEN			rtfDebugOpen
#define RTF_CLOSE			rtfDebugClose
#define RTF_READ			rtfDebugRead
#define RTF_WRITE			rtfDebugWrite
#define RTF_LSEEK			rtfDebugLseek
#define RTF_LSEEK64			rtfDebugLseek64
#else
#define RTF_OPEN			RTF_NATIVE_OPEN
#define RTF_CLOSE			RTF_NATIVE_CLOSE
#define RTF_READ			RTF_NATIVE_READ
#define RTF_WRITE			RTF_NATIVE_WRITE
#define RTF_LSEEK			RTF_NATIVE_LSEEK
#define RTF_LSEEK64			RTF_NATIVE_LSEEK64
#endif // #ifdef DO_DEBUGIO

#define SLEEP(msec)			Sleep((msec))

#ifdef _WIN32
#define PRINTF				printf
#define PERROR				perror
#define IMPORT				__declspec( dllimport )
#define EXPORT				__declspec( dllexport )
#endif

#ifdef _LINUX
#ifdef __KERNEL__
#define PRINTF				printk
#define PERROR				
#else
#define PRINTF				printf
#define PERROR				perror
#endif
#define IMPORT
#define EXPORT
#endif

#ifndef MAX
#define MAX(a,b)			( ((a)>(b)) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN(a,b)			( ((a)<(b)) ? (a) : (b) )
#endif

#ifndef ABS
#define ABS(x)				( ((x)<0) ? (-(x)) : (x) )
#endif

// typedefs *****************************************************************************

// simple types ***************************************************************

// specific application supplied handles (actual types unknown)
typedef void * RTF_APP_SESSION_HANDLE;
typedef void * RTF_APP_FILE_HANDLE;
typedef void * RTF_APP_BUFFER_HANDLE;
typedef void * RTF_APP_HEAP_HANDLE;

// generic RTF object handle (type unknown)
typedef void * RTF_HANDLE;

// specific object handles
typedef void * RTF_SYS_HANDLE;
typedef void * RTF_SES_HANDLE;

// ANSI elapsed process time expressed in CLOCKS_PER_SEC (returned from "clock" function)
typedef unsigned long RTF_RUNTIME;
typedef const char *RTF_RESULT;

// enumerator types ***********************************************************

// supported video CODEC types
typedef enum _RTF_VIDEO_CODEC_TYPE
{
	RTF_VIDEO_CODEC_TYPE_INVALID = 0,

	RTF_VIDEO_CODEC_TYPE_MPEG2,
	RTF_VIDEO_CODEC_TYPE_H264,
	RTF_VIDEO_CODEC_TYPE_VC1,

	RTF_VIDEO_CODEC_TYPE_LIMIT

} RTF_VIDEO_CODEC_TYPE;

// supported audio CODEC types
typedef enum _RTF_AUDIO_CODEC_TYPE
{
	RTF_AUDIO_CODEC_TYPE_INVALID = 0,

	RTF_AUDIO_CODEC_TYPE_MPEG1,
	RTF_AUDIO_CODEC_TYPE_MPEG2,
	RTF_AUDIO_CODEC_TYPE_MULTI,
	RTF_AUDIO_CODEC_TYPE_AC3,
	RTF_AUDIO_CODEC_TYPE_LPCM,
	RTF_AUDIO_CODEC_TYPE_DTS,
	RTF_AUDIO_CODEC_TYPE_DVS,

	RTF_AUDIO_CODEC_TYPE_LIMIT

} RTF_AUDIO_CODEC_TYPE;

// transition clip flavor
typedef enum _RTF_TRANSITION_FLAVOR
{
	RTF_TRANSITION_FLAVOR_INVALID = 0,

	RTF_TRANSITION_FLAVOR_OUT,
	RTF_TRANSITION_FLAVOR_IN,

	RTF_TRANSITION_FLAVOR_LIMIT

} RTF_TRANSITION_FLAVOR;

// supported stream types
typedef enum _RTF_STREAM_TYPE
{
	RTF_STREAM_TYPE_UNKNOWN = 0,

	RTF_STREAM_TYPE_PAT,
	RTF_STREAM_TYPE_CAT,
	RTF_STREAM_TYPE_PMT,
	RTF_STREAM_TYPE_PCR,
	RTF_STREAM_TYPE_VID,
	RTF_STREAM_TYPE_AUD,
	RTF_STREAM_TYPE_DAT,
	RTF_STREAM_TYPE_CA,
	RTF_STREAM_TYPE_NUL,

	RTF_STREAM_TYPE_LIMIT

} RTF_STREAM_TYPE;

// supported data stream types
typedef enum _RTF_DATA_STREAM_TYPE
{
	RTF_DATA_STREAM_TYPE_INVALID = 0,

	RTF_DATA_STREAM_TYPE_H2220,
	RTF_DATA_STREAM_TYPE_MHEG,
	RTF_DATA_STREAM_TYPE_DSMCC,

	RTF_DATA_STREAM_TYPE_LIMIT

} RTF_DATA_STREAM_TYPE;

// supported indexing modes
typedef enum _RTF_INDEX_MODE
{
	RTF_INDEX_MODE_INVALID = 0,

	RTF_INDEX_MODE_OFFLINE,
	RTF_INDEX_MODE_REALTIME

} RTF_INDEX_MODE;

// supported index types
typedef enum _RTF_INDEX_TYPE
{
	RTF_INDEX_TYPE_INVALID = 0,

	RTF_INDEX_TYPE_VVX,
	RTF_INDEX_TYPE_VV2,

	RTF_INDEX_TYPE_LIMIT

} RTF_INDEX_TYPE;

// options for VVX indexes
typedef enum _RTF_INDEX_OPTION_VVX
{
	RTF_INDEX_OPTION_VVX_INVALID = 0,

	RTF_INDEX_OPTION_VVX_7_2,
	RTF_INDEX_OPTION_VVX_7_3,

	RTF_INDEX_OPTION_VVX_LIMIT

} RTF_INDEX_OPTION_VVX;

// options for VV2 indexes
typedef enum _RTF_INDEX_OPTION_VV2
{
	RTF_INDEX_OPTION_VV2_INVALID = 0,

	RTF_INDEX_OPTION_VV2_TSOIP,
	RTF_INDEX_OPTION_VV2_ON2RTP,

	RTF_INDEX_OPTION_VV2_LIMIT

} RTF_INDEX_OPTION_VV2;

// generic index option union
typedef union _RTF_INDEX_OPTION
{
	RTF_INDEX_OPTION_VVX vvx;
	RTF_INDEX_OPTION_VV2 vv2;

} RTF_INDEX_OPTION;

typedef enum _RTF_MSG_PRIO
{
	RTF_MSG_PRIO_OK = 0,	// no error
	RTF_MSG_PRIO_INFO,		// status information only
	RTF_MSG_PRIO_WARNING_IGNORE, // recoverable session error. quality may be reduced. always ignored
	RTF_MSG_PRIO_WARNING,	// recoverable session error. quality may be reduced
	RTF_MSG_PRIO_SESFATAL,	// non-recoverable session error. session will close
	RTF_MSG_PRIO_LIBFATAL	// non-recoverable library error. library will close

} RTF_MSG_PRIO;

typedef enum _RTF_BUFSEEK
{
	RTF_BUFSEEK_NONE = 0,	// no seek. Treat as sequential access
	RTF_BUFSEEK_SET,		// offset from start of file
	RTF_BUFSEEK_CUR,		// offset from current file position
	RTF_BUFSEEK_END			// offset from end of file

} RTF_BUFSEEK;

typedef enum _RTF_WARNING_TOLERANCE
{
	RTF_WARNING_TOLERANCE_STRICT = 0,	// any warning is a session fatal error
	RTF_WARNING_TOLERANCE_RELAXED		// don't fail on any number of warnings
} RTF_WARNING_TOLERANCE;

// callback function typedefs *************************************************

// error notifier callbacks *****************************************

// callback function to notify application of error in RTF library
typedef void (*RTF_LIBRARY_ERROR_NOTIFIER)( char *pMessage );

// callback function to notify application of error in RTF session
typedef void (*RTF_SES_ERROR_NOTIFIER)( RTF_APP_SESSION_HANDLE hAppSession, char *pMessage );

// string logging callback ******************************************

// NOTE: hAppSession is the application-supplied handle passed into
// rtfOpenSession, or null if the error is not associated with a
// particular session. pString points to a null-terminated message
// string. String does not persist after call returns
typedef void (*RTF_LOG_FUNCTION)( RTF_APP_SESSION_HANDLE hAppSession, const char *pShortString, char *pLongString );

// I/O callback functions *******************************************

// NOTE: When the session is opened, the application provides:
//		 a handle to associate with a session
//		 a handle to associate with each output file (in the output settings structure)
// NOTE: the application provides a handle to associate with each buffer when the buffer
//		 is obtained via the BUFGET function (below)
// NOTE: All callbacks are assumed to be blocking.
// NOTE: I/O callbacks return 0 on success, -1 on failure. Functions should only return
//       -1 for UNEXPECTED failures, NOT at end of input. BUFGET should provide a NULL
//       buffer pointer at end of input

// callback function to notify application that an input buffer is released
typedef int (*RTF_BUFREL_FUNCTION)( RTF_APP_SESSION_HANDLE hAppSession,
								    RTF_APP_FILE_HANDLE hAppFile,
								    RTF_APP_BUFFER_HANDLE hAppBuffer,
									unsigned char *pBuffer );

// callback function to get an empty output buffer from the application
// Supply an application output handle
// Get back handle, pointer, size, and byte count of empty buffer
typedef int (*RTF_BUFGET_FUNCTION)( RTF_APP_SESSION_HANDLE hAppSession,
								    RTF_APP_FILE_HANDLE hAppFile,
								    RTF_APP_BUFFER_HANDLE *phAppBuffer,
									unsigned char **ppBuffer, unsigned long *pCapacity );

// callback function to give a full output buffer to the application for writing
// Supply an application output handle, buffer handle, pointer, size, and byte count
// Buffer is written to output. Get back nothing
typedef int (*RTF_BUFPUT_FUNCTION)( RTF_APP_SESSION_HANDLE hAppSession,
								    RTF_APP_FILE_HANDLE hAppFile,
								    RTF_APP_BUFFER_HANDLE hAppBuffer,
									unsigned char *pBuffer, unsigned long occupancy,
									RTF_BUFSEEK bufSeek, INT64 bufSeekOffset );

// callback function to read from an input file (only used for augmentation bitrate measurement)
// Supply an application input handle, buffer pointer, size, and pointer to return byte count
// also supply SEEK parameters to be applied before the read occurs.
// Application attempts to fill buffer from input. Note that buffer size may be zero,
// in which case this is a SEEK only operation. Get back actual number of bytes read
typedef int (*RTF_RDINPUT_FUNCTION)( RTF_APP_SESSION_HANDLE hAppSession,
									 unsigned char *pBuffer, unsigned long bufSize,
									 unsigned long *pBufOccupancy,
									 RTF_BUFSEEK bufSeek, INT64 bufSeekOffset );

// Supply an application output file handle.
// Output stream is closed. Any outstanding buffers are assumed released. Get back nothing
typedef int (*RTF_OUTCLOSE_FUNCTION)( RTF_APP_SESSION_HANDLE hAppSession,
									  RTF_APP_FILE_HANDLE hAppFile );

// dynamic memory allocation routine (provided by calling application)
// returns pointer to storage, or NULL on failure
typedef void* (*RTF_ALLOC_FUNCTION)( RTF_APP_HEAP_HANDLE hAppHeap, int bytes );

// dynamic memory free routine (provided by calling application)
typedef void (*RTF_FREE_FUNCTION)( RTF_APP_HEAP_HANDLE hAppHeap, void *pStorage );

// structure types ************************************************************

// MPEG2 video codec trick spec stucture
typedef struct _RTF_VCD_TRICK_SPEC_MPEG2
{
	// sequence extension header fixups *********

	BOOL setLowDelay;		// set the low delay flag (seq hdr ext)

	// GOP header fixups ************************

	BOOL clearGOPTime;		// clear the GOP time field
	BOOL clearDropFrame;	// clear the drop frame flag
	BOOL setClosedGOP;		// set the closed GOP flag
	BOOL setBrokenLink;		// set the broken link flag

	// picture header fixups ********************

	BOOL clearTemporalRef;	// clear the temporal reference field
	BOOL resetVBVDelay;		// set VBV delay to invalid (0xFFFF)

	// picture coding extension fixups **********

	BOOL clear32Pulldown;	// clear the repeat first field flag (picture coding ext)

} RTF_VCD_TRICK_SPEC_MPEG2;

// H264 codec trick spec stucture
typedef struct _RTF_VCD_TRICK_SPEC_H264
{
	// placeholder for real settings
	BOOL foo;

} RTF_VCD_TRICK_SPEC_H264;

// VC1 codec trick spec stucture
typedef struct _RTF_VCD_TRICK_SPEC_VC1
{
	// placeholder for real settings
	BOOL bar;

} RTF_VCD_TRICK_SPEC_VC1;

typedef struct _RTF_VCD_TRICK_SPEC
{
	RTF_VCD_TRICK_SPEC_MPEG2 mpeg2;
	RTF_VCD_TRICK_SPEC_H264  h264;
	RTF_VCD_TRICK_SPEC_VC1   vc1;

} RTF_VCD_TRICK_SPEC;

// specifies fixup operations to be performed on a trickfile
// NOTE: ONLY MPEG2 TRANSPORT IS SUPPORTED !!!
typedef struct _RTF_TRICK_SPEC
{
	// trickfile speed **************************

	int speedDirection;					// 0 for index, +1 for fwd, -1 for rev
	unsigned long speedNumerator;		// 0 for index
	unsigned long speedDenominator;		// can't be 0

	// file extensjon

	char fileExtension[ RTF_MAX_EXT_LEN ];

	// packet insert / replace / suppress fixups 

	BOOL insertPES;			// insert a PES packet header into every access unit
	BOOL insertPAT;			// insert a copy of the PAT into every access unit
	BOOL insertPMT;			// insert a copy of the PMT into every access unit
	BOOL insertPCR;			// insert PCR packets as required to maintain minimum frequency
	BOOL insertNCF;			// insert no change frames when a keyframe must be skipped to reduce bit rate
	BOOL insertNUL;			// insert stuffing packets to help achieve smoother constant bitrate
	BOOL replacePES;		// replace the captured PES packet header with the one supplied below
	BOOL replacePAT;		// replace the captured PAT with the one supplied below
	BOOL replacePMT;		// replace the captured PMT with the one supplied below
	BOOL suppressPAT;		// do not include PAT packets from the source in trick files
	BOOL suppressPMT;		// do not include PMT packets from the source in trick files
	BOOL suppressCAT;		// do not include CAT packets from the source in trick files
	BOOL suppressECM;		// do not include ECM packets from the source in trick files
	BOOL suppressAUD;		// do not include AUDIO packets from the source in trick files
	BOOL suppressDAT;		// do not include DATA packets from the source in trick files
	BOOL suppressNUL;		// do not include stuffing packets from source in trick files
	BOOL suppressCA;		// do not include conditional access packets from source in trick files
	BOOL suppressFLUFF;		// do not include video packets from source that have all zero payloads
	BOOL suppressOTHER;		// do not include packets that are not PAT, PMT, CAT, ECM, video, or stuffing
	BOOL augmentWithNUL;	// fill augmentation bandwidth with null packets
	BOOL augmentWithFLUFF;	// fill augmentation bandwidth with video FLUFF packets
	BOOL augmentWithPID;	// fill augmentation bandwidth with augmentation PID packets

	// misc general fixup flags *****************

	BOOL ignoreEncryption;	// ignore encryption - don't attempt to recognize, don't include in trick files
	BOOL prefixPSI;			// start every trick file with inserted PSI
	BOOL constantBitRate;	// add stuffing pkts as required to maintain a constant bitrate
	BOOL userBitRate;		// use the bit rate specified below instead of the input bit rate
							// note: implies sequence header bit rate fixup
	BOOL interpTimeStamps;	// if generating or restamping PCRs, PTSs, or DTSs, use interpolated main file values
	BOOL forcePadding;		// guarantee a minimum percentage of null packets in the trick files
	BOOL optimizeForATSC;	// enable optimizations that assume streams are ATSC compliant
	BOOL generateTTS;		// make all output packets 192 byte TTS packets
	BOOL dittoFrames;		// pad long trick file index intervals with copies of the last I-frame
	BOOL abortVidOnPPU;		// abort video processing on picture pool underflow

	// !!! TEST HACK !!! ADDITIONAL SWITCHES FOR PANASONIC TTS DEMO !!!
	BOOL ttsDconAccessUnit;	// insert a DCON before every access unit  (trickfiles only)
	BOOL ttsDconPcrPacket;	// insert a DCON in every PCR packet       (trickfiles only)
	BOOL ttsDconAFPackets;	// insert a DCON in every adaptation field (trickfiles only)

	// PES packet header fixups *****************

	BOOL suppressInputPesHdr; // suppress the PES packet headers in the input stream
	BOOL zeroPesPktLen;		// zero the PES packet length field
	BOOL clearDAI;			// clear data alignment indicator
	BOOL suppressPTSDTS;	// suppress PTS and DTS fields
	BOOL suppressDTS;		// suppress DTS fields
	BOOL restampPTSDTS;		// restamp PTS and DTS fields (when present)
	BOOL setPRIO;			// set the estream priority flag (adapt field of PES hdr pkt)
	BOOL setRAND;			// set the random access pkt flag (adapt field of PES hdr pkt)
	BOOL insertDSM;			// insert DSM trick mode field

	// transport packet header fixups ***********

	BOOL suppressInputPCR;  // suppress the PCRs from the input stream
	BOOL sequentialCC;		// make all packets of all PIDs have sequential CC values
	BOOL restampPCR;		// restamp PCRs according to bit count to eliminate PCR jitter
	BOOL directionalPCR;	// when restamping PCRs, follow trickfile direction
	BOOL clearDCON;			// clear all discontinutity flags
	BOOL remapPIDs;			// perform PID remapping (note: implies PSI fixups)

	// fixup params *****************************

	// user supplied bitrate
	unsigned long userBitsPerSecond;	// user-supplied bitrate to maintain
	// force padding fraction (8 bit fixed point)
	unsigned char forcePaddingFactorFix8;
	// PCR fixups
	unsigned char PCRsPerSecond;		// Note: spec requires minimum of 10 PCRs/sec
	unsigned short pcrPID;				// PID to use when generating PCRs
	// remap PID list
	unsigned short remapList[ TRANSPORT_MAX_PID_VALUES ];
	// PSI replacement tables
	unsigned char pat[ TRANSPORT_PACKET_BYTES ];
	unsigned char pmt[ TRANSPORT_PACKET_BYTES ];
	// PES packet header replacement
	unsigned char pesHdrLen;
	unsigned char pesHdr[ TRANSPORT_PACKET_BYTES ];
	// video codec specific fixup specification
	RTF_VCD_TRICK_SPEC codec;

} RTF_TRICK_SPEC;

// general output stream specification structure
typedef struct _RTF_APP_OUTPUT_SETTINGS
{
	RTF_APP_FILE_HANDLE hAppOutFile;
	RTF_BUFGET_FUNCTION pBufferGetFunction;
	RTF_BUFPUT_FUNCTION pBufferPutFunction;
	RTF_OUTCLOSE_FUNCTION pOutputCloseFunction;
	RTF_TRICK_SPEC trickSpec;

} RTF_APP_OUTPUT_SETTINGS;

// this structure serves both to set warning count limits for a session.
// and to report the number of warnings that occurred in a session.
typedef struct _RTF_WARNING_COUNTS
{
	int total;				// sum of all entries below
	int suppressCount;		// suppress logging of a category of warnings after
							// this many warnings in that category have occurred 

	int unsupported;		// RTF_MSG_WRN_UNSUPPORTED
	int badAdapt;			// RTF_MSG_WRN_BADADAPT
	int noPaylod;			// RTF_MSG_WRN_NOPAYLOAD
	int missingPesHdr;		// RTF_MSG_WRN_PESNOTCAPTURED
	int emptyPicture;		// RTF_NSG_WRN_EMPTYPICTURE
	int recordFull;			// RTF_MSG_WRN_RECORDBUFFULL
	int firstPicNotKey;		// RTF_MSG_WRN_FIRSTPICNOTKEY
	int outpointSkipped;	// RTF_MSG_WRN_OUTPOINTSKIPPED
	int picPoolUnderflow;	// RTF_MSG_WRN_PICUNDERFLOW
	int pktArrayOverflow;	// RTF_MSG_WRN_PKTOVERFLOW
	int picArrayOverflow;	// RTF_MSG_WRN_PICOVERFLOW
	int gopArrayOverflow;	// RTF_MSG_WRN_GOPOVERFLOW
	int badCat;				// RTF_MSG_WRN_BADCAT
	int cryptoPes;			// RTF_MSG_WRN_CRYPTOPES
	int varBitRate;			// RTF_MSG_WRN_VARBITRATE
	int pcrMaxGap;			// RTF_MSG_WRN_PCRMAXGAP
	int noFirstPcrDcon;		// RTF_MSG_WRN_NOFIRSTPCRDCON
	int postFirstPcrDcon;	// RTF_MSG_WRN_POSTFIRSTPCRDCON
	int multiProgStream;	// RTF_MSG_WRN_MULTIPROGSTREAM
	int multiVidStream;		// RTF_MSG_WRN_MULTIVIDSTREAM
	int multiAudioCodec;	// RTF_MSG_WRN_MULTIAUDIOCODEC
	int splicingAudioBad;	// RTF_MSG_WRN_SPLICINGAUDIOBAD
	int splicingPSDesc;		// RTF_MSG_WRN_SPLICINGPSDESC
	int splicingESDesc;		// RTF_MSG_WRN_SPLICINGESDESC
	int patChanged;			// RTF_MSG_WRN_PATCHANGED
	int catChanged;			// RTF_MSG_WRN_CATCHANGED
	int pmtChanged;			// RTF_MSG_WRN_PMTCHANGED
	int pmtEncrypted;		// RTF_MSG_WRN_PMTENCRYPTED
	int augNotFound;		// RTF_MSG_WRN_AUGNOTFOUND
	int badSliceType;		// RTF_MSG_WRN_BADSLICETYPE
	int frcMismatch;		// RTF_MSG_WRN_FRCMISMATCH
	int syncLoss;			// RTF_MSG_WRN_SYNCLOSS
	int splitPkt;			// RTF_MSG_WRN_SPLITPKT
	int badTrickSpeed;		// RTF_MSG_WRN_BADTRICKSPEED
	int zeroOutput;			// RTF_MSG_WRN_ZEROOUTPUT

} RTF_WARNING_COUNTS;

// program and program element descriptor types
typedef enum _RTF_DESCRTAG
{
	RTF_DESCRTAG_RES0 = 0,
	RTF_DESCRTAG_RES1,
	RTF_DESCRTAG_VIDSTREAM,
	RTF_DESCRTAG_AUDSTREAM,
	RTF_DESCRTAG_HIERARCHY,
	RTF_DESCRTAG_REGISTRATION,
	RTF_DESCRTAG_DATALIGN,
	RTF_DESCRTAG_TARGETGRID,
	RTF_DESCRTAG_VIDWINDOW,
	RTF_DESCRTAG_CA,
	RTF_DESCRTAG_LANGUAGE,
	RTF_DESCRTAG_SYSTEMCLOCK,
	RTF_DESCRTAG_MUXBUFFER,
	RTF_DESCRTAG_COPYRIGHT,
	RTF_DESCRTAG_MAXBITRATE,
	RTF_DESCRTAG_PRIVATEDATA,
	RTF_DESCRTAG_SMOOTHINGBUFFER,
	RTF_DESCRTAG_STD,
	RTF_DESCRTAG_IBP,
	RTF_DESCRTAG_ITURESERVED,		// 19-63
	RTF_DESCRTAG_USERPRIVATE = 64,	// 64-255
	RTF_DESCRTAG_LIMIT = 255

} RTF_DESCRTAG;

// audio types for ISO 639 language descriptor
typedef enum _RTF_AUDIOTYPE
{
	RTF_AUDIOTYPE_UNDEFINED = 0,
	RTF_AUDIOTYPE_CLEANEFFECTS,
	RTF_AUDIOTYPE_HEARINGIMPAIRED,
	RTF_AUDIOTYPE_VISUALIMPAIRED,
	RTF_AUDIOTYPE_RESERVED,
	RTF_AUDIOTYPE_LIMIT = 255

} RTF_AUDIOTYPE;

typedef struct _RTF_DESCR_CA
{
	unsigned short sid;		// CA system ID
	unsigned short pid;		// CA PID

} RTF_DESCR_CA;

typedef struct _RTF_DESCR_LANG
{
	unsigned long codeCount;						// number of language codes below
	unsigned char code[ RTF_MAX_LANG_COUNT ][ 4 ];	// language code 1 | 2 | 3 | audio type

} RTF_DESCR_LANG;

typedef union _RTF_DESCR
{
	RTF_DESCR_CA   ca;
	RTF_DESCR_LANG lang;

} RTF_DESCR;

typedef struct _RTF_DESCR_SPEC
{
	RTF_DESCRTAG tag;
	RTF_DESCR descr;

} RTF_DESCR_SPEC;

typedef struct _RTF_DESCR_LIST
{
	unsigned long descrCount;
	RTF_DESCR_SPEC descrSpec[ RTF_MAX_DESC_COUNT ];

} RTF_DESCR_LIST;

// video stream specification structure
typedef struct _RTF_ESTREAM_VIDEO
{
	RTF_VIDEO_CODEC_TYPE vcdType;
	unsigned short width;
	unsigned short height;
	unsigned char  frameRateCode;
	unsigned long  bitsPerSecond;

} RTF_ESTREAM_VIDEO;

// audio stream specification structure
typedef struct _RTF_ESTREAM_AUDIO
{
	RTF_AUDIO_CODEC_TYPE acdType;
	unsigned long  numberOfChannels;
	unsigned long  samplesPerSecond;
	unsigned long  bitsPerSample;
	unsigned long  bitsPerSecond;
	unsigned long  acdParameter;

} RTF_ESTREAM_AUDIO;

// data stream specification structure
typedef struct _RTF_ESTREAM_DATA
{
	RTF_DATA_STREAM_TYPE dstType;
	unsigned long  bitsPerSecond;

} RTF_ESTREAM_DATA;

typedef union _RTF_ESTREAM
{
	RTF_ESTREAM_VIDEO video;
	RTF_ESTREAM_AUDIO audio;
	RTF_ESTREAM_DATA  data;

} RTF_ESTREAM;

typedef struct RTF_ESTREAM_SPEC
{
	RTF_ESTREAM eStream;
	unsigned short pid;
	RTF_DESCR_LIST list;

} RTF_ESTREAM_SPEC;

// bit masks for flags field below
#define RTF_PROFILE_TTS_MASK	0x00000001

// structure specifying profile for spliceable assets
typedef struct _RTF_STREAM_PROFILE
{
	unsigned long  profileID;
	unsigned short pmtPID;
	unsigned short pcrPID;
	unsigned long  bitsPerSecond;
	unsigned long  userBitsPerSecond;
	unsigned long  flags;
	UINT64 streamPcrBase;
	// allow 1 part in 2^^log2VbrThreshold variability in bit rate
	int log2VbrThreshold;
	// augmentation - packets added after original transport multiplex - ex: FEC, ECMs
	int augmentationBaseFactor;
	int augmentationPlusFactor;
	int augmentationPidCount;
	unsigned short augmentationPids[ RTF_MAX_AUGMENTATIONPIDS ];
	RTF_DESCR_LIST progDescrList;
	RTF_ESTREAM_SPEC videoSpec;
	int audioCount;
	RTF_ESTREAM_SPEC audioSpec[ RTF_MAX_AUDIO_PIDS ];
	int dataCount;
	RTF_ESTREAM_SPEC dataSpec[ RTF_MAX_DATA_PIDS ];

} RTF_STREAM_PROFILE;

// external API function prototypes *****************************************************

// get the version number of the library as an unsigned long
// ( upper 16 bits = major, lower 16 bits = minor )
EXPORT void rtfGetVersion( unsigned long *pVersion );

// get the version number of the library as a string
// ( note: buffer should be at least 16 chars in length )
EXPORT void rtfGetVersionString( char *pVersionString );

// get a pointer to a string that identifies this version of the library completely
EXPORT void rtfGetProgramVersionString( char **ppProgramVersionString );

// get the amount of storage that will be consumed by a call to rtfInitializeDefaultLibrary (below)
EXPORT unsigned long rtfGetDefaultLibraryStorageRequirement( BOOL isHD );

// get the amount of storage that will be consumed by a call to rtfInitializeLibrary (below)
EXPORT unsigned long rtfGetLibraryStorageRequirement( unsigned long maxSessionCount,
						unsigned long maxInputBuffersPerSession, unsigned long maxGroupsPerSequence,
						unsigned long maxPicturesPerGroup, unsigned long maxInputBufferBytes );

// initialize the real-time trickfile library using default values (once only!)
// returns TRUE for success; else FAIL
EXPORT BOOL rtfInitializeDefaultLibrary( RTF_APP_HEAP_HANDLE hAppHeap,
										 RTF_ALLOC_FUNCTION allocFunc,
										 RTF_FREE_FUNCTION freeFunc,
										 RTF_LOG_FUNCTION logFunc,
										 RTF_LIBRARY_ERROR_NOTIFIER notifier,
										 BOOL isHD );

// initialize the real-time trickfile library (once only!)
// returns TRUE for success; else FAIL
EXPORT BOOL rtfInitializeLibrary( RTF_APP_HEAP_HANDLE hAppHeap, RTF_ALLOC_FUNCTION allocFunc,
						   RTF_FREE_FUNCTION freeFunc, RTF_LOG_FUNCTION logFunc,
						   unsigned long maxSessionCount, unsigned long maxInputBuffersPerSession,
						   unsigned long maxGroupsPerSequence, unsigned long maxPicturesPerGroup,
						   unsigned long maxInputBufferBytes, int sessionFailThreshold,
						   RTF_LIBRARY_ERROR_NOTIFIER notifier );

// set the active input stream profile for the library
// NOTE: If multiple profiles are used by a site, user must maintain unique profileID fields in this structure!
// NOTE: must be called before any sessions are opened if assets are to be prepared for splicing!
// returns TRUE for success; else FAIL
EXPORT BOOL rtfSetStreamProfile( RTF_STREAM_PROFILE *pProfile );

// shut down the real-time trickfile library
// returns TRUE for success; else FAIL
EXPORT BOOL rtfShutdownLibrary();

// get the total amount of storage allocated by the library
EXPORT unsigned long rtfGetTotalAlloc();

// dynamically allocate some storage
EXPORT void *rtfAlloc( int bytes );

// free some dynamically allocated storage
EXPORT void rtfFree( void *ptr );

// get the current log message priority filter setting
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetMsgPrioFilter( RTF_MSG_PRIO *pPrio );

// set the current log message priority filter setting
// returns TRUE for success; else FAIL
EXPORT BOOL rtfSetMsgPrioFilter( RTF_MSG_PRIO prio );

// get the current zombie timeout threshold setting
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetZombieThreshold( int *pZombieThreshold );

// set the current zombie timeout threshold setting
// returns TRUE for success; else FAIL
EXPORT BOOL rtfSetZombieThreshold( int zombieThreshold );

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
										 RTF_OUTCLOSE_FUNCTION closeFunction );

// initialize an RTF_WARNING_THRESHOLD structure with default values
// note: used for recoverable errors only. application may set up this
// structure or call this function and override selected default values
EXPORT BOOL rtfInitializeWarningThresholds( RTF_WARNING_TOLERANCE tolerance,
										    RTF_WARNING_COUNTS *pThresholds );

// log the output settings for a particular output of a session
EXPORT BOOL rtfLogOutputSettings( RTF_SES_HANDLE hSession, int outputNumber );

// log the output settings for all of the outputs of a session
EXPORT BOOL rtfLogAllOutputSettings( RTF_SES_HANDLE hSession );

// open a trickfile processing session
// returns TRUE for success; else FAIL
EXPORT BOOL rtfOpenSession( RTF_SES_HANDLE *phSession,
							RTF_APP_SESSION_HANDLE hAppSession,
							RTF_APP_FILE_HANDLE hAppInputFile,
							char *pInputFileName, INT64 inputFileBytes,
							RTF_WARNING_COUNTS *pThresholds, RTF_INDEX_MODE indexMode,
							RTF_INDEX_TYPE indexType, RTF_INDEX_OPTION indexOption,
							RTF_SES_ERROR_NOTIFIER notifier,
							RTF_BUFREL_FUNCTION inputBufferReleaseCallback,
							int numSettings, RTF_APP_OUTPUT_SETTINGS settings[] );

// close a trickfile processing session
// returns TRUE for success; else FAIL
EXPORT BOOL rtfCloseSession( RTF_SES_HANDLE hSession );

// get the number of packets processed by a session so far
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetProgress( RTF_SES_HANDLE hSession, unsigned long *pProcessedPacketCount );

// get the current system time
EXPORT void rtfGetRunTime( RTF_RUNTIME *pRunTime );

// get the input bit rate
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetInputBitrate( RTF_SES_HANDLE handle, unsigned long *pBitsPerSecond );

// get the output bit rate
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetOutputBitrate( RTF_SES_HANDLE handle, unsigned long *pBitsPerSecond );

// get the current session elapsed time in clock ticks
// returns TRUE for success; else FAIL
EXPORT BOOL rtfGetElapsedTime( RTF_SES_HANDLE handle, RTF_RUNTIME *pElapsedTime );

// submit a buffer of input for processing (buffer is held by trickfile library until explicitly released)
// returns TRUE for success; else FAIL
EXPORT BOOL rtfProcessInput( RTF_SES_HANDLE handle,
							 RTF_APP_SESSION_HANDLE hAppSession,
							 RTF_APP_FILE_HANDLE hAppFile,
							 RTF_APP_BUFFER_HANDLE hAppBuffer,
							 unsigned char *pBuffer, unsigned long bytes );

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
									int *pAugmentedBitRate, int *pOriginalBitRate );

EXPORT RTF_RESULT rtfSesGetStreamProfile( RTF_SES_HANDLE handle, 
										 RTF_STREAM_PROFILE **ppProfile );

#ifdef _DEBUG
// debug helper function - log the state of the session
// returns TRUE for success; else FAIL
EXPORT BOOL rtfLogState( RTF_SES_HANDLE handle );
#endif

#ifdef DO_DEBUGIO
EXPORT int rtfDebugOpen ( const char *pFileName, int oFlag, int pMode );
EXPORT int rtfDebufClose( int fd );
EXPORT int rtfDebugWrite( int fd, const void *pBuffer, unsigned int size );
EXPORT int rtfDebugRead ( int fd, void *pBuffer, unsigned int size );
EXPORT int rtfDebugLSeek( int fd, int offset, int origin );
EXPORT INT64 rtfDebugLSeek64( int fd, INT64 offset, int origin );
#endif // #ifdef DO_DEBUGIO

#ifdef __cplusplus
}
#endif

#endif // #ifndef _RTF_LIB_H

