// private include file for rtfLib implementation files
//

#ifndef _RTF_PRV_H
#define _RTF_PRV_H 1

// compilation switches *****************************************************************
// NOTE: these are ifdef switches. Do not remove, only comment out

// #define DO_TRACKING				1
// #define DO_STATISTICS			1
// #define DO_CHECKSIZE				1
// #define DO_LOG_PICTURES			1
// #define DO_TRACKREFCOUNTS		1
#define DO_FLOATING_POINT			1

// video codec support  NOTE: at least one of these must be turned on!
#define DO_VCD_MPEG2				1
#define DO_VCD_H264					1
// #define DO_VCD_VC1				1

// audio codec support  NOTE: at least one of these must be turned on!
#define DO_ACD_AC3					1
// #define DO_ACD_MPEG2				1

// index support  NOTE: at least one of these must be turned on!
#define DO_INDEX_VVX				1
#define DO_INDEX_VV2				1

// conditional access support  NOTE: at least one of these must be turned on!
#define DO_CAS_VMX					1
#define DO_CAS_PAN					1

// RTF external API include file ********************************************************

#include "RTFLib.h"

// constants ****************************************************************************

// version number
#define RTF_VERSION_MAJOR				0x0002
#define RTF_VERSION_MINOR				0x0000
#define RTF_VERSION						( ( RTF_VERSION_MAJOR<<16 ) | RTF_VERSION_MINOR )

// transport related
#define TRANSPORT_PACKET_HEADER_BYTES	4
#define TRANSPORT_PACKET_SYNCBYTE		0x47
#define TRANSPORT_PACKET_PID_MASK		0x1FFF
#define TRANSPORT_PAT_PID				0x0000
#define TRANSPORT_CAT_PID				0x0001
#define TRANSPORT_PAD_PID				0x1FFF
#define TRANSPORT_STARTCODE_PREFIX_MASK	0xFFFFFF00
#define TRANSPORT_STARTCODE_BASE		0x00000100
#define TRANSPORT_INVALID_DATA			0x00000000
#define TRANSPORT_MIN_ADAPT_BYTES		2
#define TRANSPORT_MAX_PAYLOAD_OFFSET	TRANSPORT_PACKET_HEADER_BYTES
#define TRANSPORT_MAX_PAYLOAD_BYTES		( TRANSPORT_PACKET_BYTES - TRANSPORT_MAX_PAYLOAD_OFFSET )
#define TRANSPORT_PACKET_BITS			( TRANSPORT_PACKET_BYTES * 8 )
#define TRANSPORT_PACKET_WORDS			( TRANSPORT_PACKET_BYTES >> 1 )
#define TRANSPORT_PACKET_LONGS			( TRANSPORT_PACKET_BYTES >> 2 )
#define INV_TRANSPORT_PACKET_BYTES_FIX16 ( (unsigned long)( 65536.0 / TRANSPORT_PACKET_BYTES ) )
#define INV_TRANSPORT_PACKET_BITS_FIX16	 ( (unsigned long)( 65536.0 / TRANSPORT_PACKET_BITS ) )

#define TTS_HEADER_BYTES				4
#define TTS_HEADER_BITS					( TTS_HEADER_BYTES * 8 )

#define RTF_VMECM_SIGNATURE				0x47

// codec related
#define STREAM_TYPE_VIDEO_11172         0x01	// MPEG1 video
#define STREAM_TYPE_VIDEO_13818         0x02	// MPEG2 video
#define STREAM_TYPE_AUDIO_11172         0x03	// MPEG1 audio
#define STREAM_TYPE_AUDIO_13818         0x04	// MPEG2 audio
#define STREAM_TYPE_DATA_H2220			0x05	// H.222.0 private data
#define STREAM_TYPE_AUDIO_MULTI			0x06	// may be AC3, LPCM, or DTS audio
#define STREAM_TYPE_DATA_MHEG			0x07	// ISO 13522 MHEG
#define STREAM_TYPE_DATA_DSMCCA			0x0A	// DSMCC data
#define STREAM_TYPE_DATA_DSMCCB			0x0B	// DSMCC data
#define STREAM_TYPE_DATA_DSMCCC			0x0C	// DSMCC data
#define STREAM_TYPE_DATA_DSMCCD			0x0D	// DSMCC data
#define STREAM_TYPE_VIDEO_14496			0x1B	// H264 video
#define STREAM_TYPE_DIGICIPHER2			0x80	// digiciper encrypted MPEG2
#define STREAM_TYPE_AUDIO_AC3			0x81	// AC3 audio
#define STREAM_TYPE_AUDIO_LPCM			0x83	// LPCM audio
#define STREAM_TYPE_AUDIO_DTS			0x85	// DTS audio
#define STREAM_TYPE_AUDIO_DVS			0x86	// DVS-253 audio
#define STREAM_TYPE_VIDEO_VC1			0xEA	// VC1 video

// RTF specific definitions
#define RTF_SES_MAX_RD_PACKETS			64
// object array dimensions
#define RTF_MAX_OUTPUTCOUNT				( 2 + ( 2 * RTF_MAX_TRICKSPEEDS ) )
#define RTF_MAX_SES_BUFS				512
#define RTF_MAX_BUFFER_BYTES			0x80000
#define RTF_MAX_BUFFER_PACKETS			( ( RTF_MAX_BUFFER_BYTES + (TRANSPORT_PACKET_BYTES-1) ) / TRANSPORT_PACKET_BYTES )
#define RTF_MAX_PICTURE_BYTES			0x180000
#define RTF_MAX_PICTURE_PACKETS			( ( RTF_MAX_PICTURE_BYTES + (TRANSPORT_PACKET_BYTES-1) ) / TRANSPORT_PACKET_BYTES )
#define RTF_MAX_GOP_PICS			    64
#define RTF_MAX_SEQ_GOPS				2
#define RTF_MAX_SEQ_PICS			    (RTF_MAX_GOP_PICS * RTF_MAX_SEQ_GOPS)
#define RTF_MAX_NCFRAME_PACKETS			16
#define RTF_MAX_NCFRAME_BYTES			( RTF_MAX_NCFRAME_PACKETS * TRANSPORT_PACKET_BYTES )
#define RTF_MAX_NCFRAME_BITS			( RTF_MAX_NCFRAME_BYTES * 8 )
// defaults for stream characteristics
#define RTF_DEFAULT_BITS_PER_SECOND		0x400000
#define RTF_DEFAULT_SECS_PER_BIT_FIX32	( (unsigned long)( ( 4294967296.0 / RTF_DEFAULT_BITS_PER_SECOND ) + 0.5 ) )
#define RTF_DEFAULT_FRAME_RATE			29.97
#define RTF_DEFAULT_FRAME_RATE_CODE		4
#define RTF_DEFAULT_FPS_FIX16			( (unsigned long)( ( RTF_DEFAULT_FRAME_RATE * 65536.0 ) + 0.5 ) )
#define RTF_DEFAULT_SPF_FIX16			( (unsigned long)( ( 65536.0 / RTF_DEFAULT_FRAME_RATE ) + 0.5 ) )
#define RTF_DEFAULT_FRAME_TICKS			( (unsigned long)( ( TRANSPORT_SCR_TICKS_PER_SECOND / RTF_DEFAULT_FRAME_RATE ) + 0.5 ) )
#define RTF_DEFAULT_KEYFRAME_BITS		( 200 * TRANSPORT_PACKET_BITS )
#define RTF_DEFAULT_NCFRAME_BITS		( 16 * TRANSPORT_PACKET_BITS )
#define RTF_DEFAULT_GROUP_PICS_MPEG2	15
#define RTF_DEFAULT_GROUP_PICS_H264		15
#define RTF_DEFAULT_GROUP_PICS_VC1		15
#define RTF_NOMINAL_ITOAVG_RATIO_MPEG2	3
#define RTF_NOMINAL_ITOAVG_RATIO_H264	3
#define RTF_NOMINAL_ITOAVG_RATIO_VC1	3
// defaults for user settings
#define RTF_DEFAULT_MSG_PRIO_FILTER		RTF_MSG_PRIO_INFO
#define RTF_DEFAULT_SPEED_NUMERATOR		15
#define RTF_DEFAULT_SPEED_DENOMINATOR	2
#define RTF_DEFAULT_SPEED_RATIO_FIX16		( ( ( RTF_DEFAULT_SPEED_NUMERATOR << 16 ) + ( RTF_DEFAULT_SPEED_DENOMINATOR>>1 ) ) / RTF_DEFAULT_SPEED_DENOMINATOR )
#define RTF_DEFAULT_INV_SPEED_RATIO_FIX16	( ( ( RTF_DEFAULT_SPEED_DENOMINATOR << 16 ) + ( RTF_DEFAULT_SPEED_NUMERATOR>>1 ) ) / RTF_DEFAULT_SPEED_NUMERATOR )
#define RTF_DEFAULT_SUPPRESS_COUNT		8
// indexer definitions
#define RTF_ZERO_MOTION_FRAME_TYPE		2
#define RTF_INDEX_BUFFER_BYTES			0x3000
#define RTF_INDEX_INDEXTABLE_ENTRIES	( RTF_MAX_TRICKFILES + 1 )
#define RTF_INDEX_SIZETABLE_ENTRIES		( RTF_MAX_TRICKFILES + 1 )

// macros *******************************************************************************

// extract a word from an array of unsigned chars
#define WRD(buf,off)		( (((unsigned short)buf[(off)])<<8) | (buf[(off)+1]) )

// extract a long from an array of unsigned chars
#define LNG(buf,off)		( (((unsigned long)WRD(buf,off))<<16) | (WRD(buf,(off)+2)) )

// extract a PID from an array of unsigned chars
#define PID(buf,off)		(WRD(buf,off) & TRANSPORT_PACKET_PID_MASK )

#define TELL_ME_IF_YOU_GET_HERE()	PRINTF("!!! WENT TO THE BAD PLACE !!! %s %d\n", __FILE__, __LINE__ )
#define RTF_OBASE( h ) RTF_HANDLE __hBaseObject__ = (h)
#if ( defined( _DEBUG ) && defined( DO_CHECKSIZE ) )
#define RTF_FNAME( fname ) static const char *__FNAME__ = fname; unsigned long __BOOKMARK__ = rtfGetTotalAlloc()
#else
#define RTF_FNAME( fname ) static const char *__FNAME__ = fname
#endif

#ifdef _DEBUG

#ifdef DO_CHECKSIZE
#define RTF_CHK_REQ PRINTF( "%s returning %d\n", __FNAME__, bytes )
#define RTF_CHK_SIZE PRINTF( "%s allocated %d bytes\n", __FNAME__, rtfGetTotalAlloc() - __BOOKMARK__ )
#else
#define RTF_CHK_REQ
#define RTF_CHK_SIZE
#endif
#define RTF_CLR_STATE( ptr, siz ) memset( (void *)(ptr), 0, (siz) )
#define RTF_CHK_OBJ( ptr, typ ) \
	if((ptr) == NULL) \
		break;\
	else {\
		if( ( result = rtfObjCheckType( (ptr)->hBaseObject, (typ) ) ) != RTF_PASS ) \
		{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_BADTYPE, __FNAME__, \
		  "rtfObjCheckType failed (file %s line %d)", __FILE__, __LINE__ ); \
		  break; } \
	}
#define RTF_CHK_RESULT \
	if( result != RTF_PASS ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_RTFCALLFAILED, __FNAME__, \
	   "%s returned failure, file %s line %d", result, __FILE__, __LINE__ ); \
	  break; }
#define RTF_CHK_STATE_EQ( ptr, val ) \
	if( (ptr->state) != (val) ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_BADSTATE, __FNAME__, \
	  "Incorrect state (file %s line %d)", __FILE__, __LINE__ ); \
	  result = rtfReportErr( __hBaseObject__, RTF_MSG_ERR_BADSTATE ); \
	  break; }
#define RTF_CHK_STATE_NE( ptr, val ) \
	if( (ptr->state) == (val) ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_BADSTATE, __FNAME__, \
	  "Incorrect state (file %s line %d)", __FILE__, __LINE__ ); \
	  result = rtfReportErr( __hBaseObject__, RTF_MSG_ERR_BADSTATE ); \
	  break; }
#define RTF_CHK_STATE_GE( ptr, val ) \
	if( (ptr->state) < (val) ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_BADSTATE, __FNAME__, \
	  "Incorrect state (file %s line %d)", __FILE__, __LINE__ ); \
	  result = rtfReportErr( __hBaseObject__, RTF_MSG_ERR_BADSTATE ); \
	  break; }
#define RTF_CHK_ALLOC( ptr ) \
	if( NULL == (void *)ptr ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_BADALLOC, __FNAME__, \
	  "Allocation failure (file %s line %d)", result, __FILE__, __LINE__ ); \
	  result = rtfReportErr( __hBaseObject__, RTF_MSG_ERR_BADALLOC ); \
	  break; }
#define RTF_CHK_PKTMAPPED( pPkt ) \
	if( NULL == (void *)(pPkt->pStorage) ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_BADSTATE, __FNAME__, \
	  "Unmapped transport packet (file %s line %d)", __FILE__, __LINE__ ); \
	  pPkt->flags |= RTF_PKT_ISDAMAGED; \
	  result = rtfReportErr( __hBaseObject__, RTF_MSG_ERR_BADSTATE ); \
	  break; }
#define RTF_CHK_RANGE( x, lo, hi ) \
	if( ( x <= lo ) || ( x >= hi ) ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_BADPARAM, __FNAME__, \
	  "Parameter out of range", __FILE__, __LINE__ ); \
	  result = rtfReportErr( __hBaseObject__, RTF_MSG_ERR_BADPARAM ); \
	  break; }
#define RTF_CHK_REFCOUNT( pObj ) \
	if( pObj->referenceCount != 0 ) \
	{ rtfLogMessage( __hBaseObject__, RTF_MSG_ERR_INTERNAL, __FNAME__, \
	  "Non-zero reference count", __FILE__, __LINE__ ); \
	  result = rtfReportErr( __hBaseObject__, RTF_MSG_ERR_INTERNAL ); \
	  break; }

#else

#define RTF_CHK_SIZE
#define RTF_CHK_REQ
#define RTF_CLR_STATE( ptr, siz )
#define RTF_CHK_OBJ( ptr, typ )
#define RTF_CHK_RESULT if( result != RTF_PASS ) break
#define RTF_CHK_STATE_EQ( ptr, val ) if( (ptr->state) != (val) ) { result = RTF_FAIL; break; }
#define RTF_CHK_STATE_NE( ptr, val ) if( (ptr->state) == (val) ) { result = RTF_FAIL; break; }
#define RTF_CHK_STATE_GE( ptr, val ) if( (ptr->state)  < (val) ) { result = RTF_FAIL; break; }
#define RTF_CHK_ALLOC( ptr ) if( NULL == (void *)ptr ) { result = RTF_FAIL; break; }
#define RTF_CHK_PKTMAPPED( pPkt ) if( NULL == (void *)(pPkt->pStorage) ) { result = RTF_FAIL; break; }
#define RTF_CHK_RANGE( x, lo, hi ) if( ( x <= lo ) || ( x >= hi ) ) { result = RTF_FAIL; break; }
#define RTF_CHK_REFCOUNT( pObj ) if( pObj->referenceCount != 0 ) { result = RTF_FAIL; break; }

#endif

#define RTF_CHK_RESULT_LOOP if( result != RTF_PASS ) break;

#define RTF_LOG_ERR0( err, fmt ) \
		{ rtfLogMessage( __hBaseObject__, err, __FNAME__, fmt ); \
		  result = rtfReportErr( __hBaseObject__, err ); }
#define RTF_LOG_ERR1( err, fmt, arg1 ) \
		{ rtfLogMessage( __hBaseObject__, err, __FNAME__, fmt, arg1 ); \
		  result = rtfReportErr( __hBaseObject__, err ); }
#define RTF_LOG_ERR2( err, fmt, arg1, arg2 ) \
		{ rtfLogMessage( __hBaseObject__, err, __FNAME__, fmt, arg1, arg2 ); \
		  result = rtfReportErr( __hBaseObject__, err ); }
#define RTF_LOG_ERR3( err, fmt, arg1, arg2, arg3 ) \
		{ rtfLogMessage( __hBaseObject__, err, __FNAME__, fmt, arg1, arg2, arg3 ); \
		  result = rtfReportErr( __hBaseObject__, err ); }
#define RTF_LOG_ERR4( err, fmt, arg1, arg2, arg3, arg4 ) \
		{ rtfLogMessage( __hBaseObject__, err, __FNAME__, fmt, arg1, arg2, arg3, arg4 ); \
		  result = rtfReportErr( __hBaseObject__, err ); }
#define RTF_LOG_ERR5( err, fmt, arg1, arg2, arg3, arg4, arg5 ) \
		{ rtfLogMessage( __hBaseObject__, err, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5 ); \
		  result = rtfReportErr( __hBaseObject__, err ); }

#define RTF_LOG_WARN0( wrn, fmt ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		  if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN1( wrn, fmt, arg1 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN2( wrn, fmt, arg1, arg2 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1, arg2 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN3( wrn, fmt, arg1, arg2, arg3 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1, arg2, arg3 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN4( wrn, fmt, arg1, arg2, arg3, arg4 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1, arg2, arg3, arg4 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN5( wrn, fmt, arg1, arg2, arg3, arg4, arg5 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN6( wrn, fmt, arg1, arg2, arg3, arg4, arg5, arg6 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5, arg6 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN7( wrn, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }
#define RTF_LOG_WARN8( wrn, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 ) \
		{ int __eval = rtfWarningEval( __hBaseObject__, wrn ); result = 0; \
		if( __eval <= 0 ) { rtfLogMessage( __hBaseObject__, wrn, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 ); } \
		  if( __eval <  0 ) { result = rtfReportErr( __hBaseObject__, wrn ); } \
		  RTF_CHK_RESULT; }

#define RTF_LOG_INFO0( inf, fmt ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt )
#define RTF_LOG_INFO1( inf, fmt, arg1 ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt, arg1 )
#define RTF_LOG_INFO2( inf, fmt, arg1, arg2 ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt, arg1, arg2 )
#define RTF_LOG_INFO3( inf, fmt, arg1, arg2, arg3 ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt, arg1, arg2, arg3 )
#define RTF_LOG_INFO4( inf, fmt, arg1, arg2, arg3, arg4 ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt, arg1, arg2, arg3, arg4 )
#define RTF_LOG_INFO5( inf, fmt, arg1, arg2, arg3, arg4, arg5 ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5 )
#define RTF_LOG_INFO6( inf, fmt, arg1, arg2, arg3, arg4, arg5, arg6 ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5, arg6 )
#define RTF_LOG_INFO7( inf, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7 ) \
		rtfLogMessage( __hBaseObject__, inf, __FNAME__, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7 )

#ifdef DO_STATISTICS

#define RTF_CLR_STAT( s )		(s) = 0
#define RTF_INC_STAT( s )		++(s)
#define RTF_SET_STAT( s, v )	(s) = (v)
#define RTF_ADD_STAT( s, n )	(s) += (n)

#else

#define RTF_CLR_STAT( s )
#define RTF_INC_STAT( s )
#define RTF_SET_STAT( s, v )
#define RTF_ADD_STAT( s, n )

#endif

#ifdef DO_LOG_PICTURES

#define RTF_LOG_PIC( s )		if( pFlt->filterNumber == 1 ) PRINTF( s )

#else

#define RTF_LOG_PIC( s )

#endif

// simple types ***************************************************************

// class object handles
typedef RTF_HANDLE RTF_ACD_HANDLE;
typedef RTF_HANDLE RTF_BUF_HANDLE;
typedef RTF_HANDLE RTF_CAS_HANDLE;
typedef RTF_HANDLE RTF_CAT_HANDLE;
typedef RTF_HANDLE RTF_FLT_HANDLE;
typedef RTF_HANDLE RTF_GOP_HANDLE;
typedef RTF_HANDLE RTF_IDX_HANDLE;
typedef RTF_HANDLE RTF_OUT_HANDLE;
typedef RTF_HANDLE RTF_PAT_HANDLE;
typedef RTF_HANDLE RTF_PES_HANDLE;
typedef RTF_HANDLE RTF_PIC_HANDLE;
typedef RTF_HANDLE RTF_PKT_HANDLE;
typedef RTF_HANDLE RTF_PMT_HANDLE;
typedef RTF_HANDLE RTF_SEQ_HANDLE;
typedef RTF_HANDLE RTF_VCD_HANDLE;
typedef RTF_HANDLE RTF_WIN_HANDLE;

// union types ****************************************************************

typedef union _RTF_TIMESTAMP_EXT
{
	unsigned short us;
	unsigned char  uc[2];
} RTF_TIMESTAMP_EXT;

// container for MPEG-2 transport stream style timestamp
typedef union _RTF_TIMESTAMP_BASE
{
	UINT64			 ull;
	unsigned long    ul[2];
	unsigned short   us[4];
	unsigned char    uc[8];
} RTF_TIMESTAMP_BASE;

// structured types ***********************************************************

// encapsulates MPEG style timestamp
typedef struct _RTF_TIMESTAMP
{
	RTF_TIMESTAMP_EXT ext;
	RTF_TIMESTAMP_BASE base;
} RTF_TIMESTAMP;

// enumerated types ***********************************************************

// error / warning / info return code enum
typedef enum _RTF_MSG
{
	// no error
	RTF_MSG_OK = 0,

	// the library is shutting down
	RTF_MSG_LIBDEATH,

	// the session is shutting down
	RTF_MSG_SESDEATH,

	// error message codes indicating library error
	// note: these always shut down the library
	RTF_MSG_ERR_BADALLOC,

	// error message codes indicating session error
	// note: these codes always shut down the session
	RTF_MSG_ERR_RTFCALLFAILED,
	RTF_MSG_ERR_INDEXER,
	RTF_MSG_ERR_INTERNAL,
	RTF_MSG_ERR_BADTYPE,
	RTF_MSG_ERR_BADSTATE,
	RTF_MSG_ERR_NOTFOUND,

	// error message codes indicating application errors
	// note: these always shut down the session
	RTF_MSG_ERR_NOTYETSUPPORTED,
	RTF_MSG_ERR_CALLBACKFAILED,
	RTF_MSG_ERR_BADPARAM,
	RTF_MSG_ERR_ZOMBIE,

	// error message codes indicating bad asset
	// note: these always shut down the session
	RTF_MSG_ERR_BADSTREAM,
	RTF_MSG_ERR_OVERFLOW,
	RTF_MSG_ERR_UNDERFLOW,
	RTF_MSG_ERR_SYNCLOSS,

	// warning message codes
	// note: these are recorded and the counts compared
	// to threshold values that the application provides
	RTF_MSG_WRN_UNSUPPORTED,
	RTF_MSG_WRN_BADADAPT,
	RTF_MSG_WRN_NOPAYLOAD,
	RTF_MSG_WRN_PESNOTCAPTURED,
	RTF_NSG_WRN_EMPTYPICTURE,
	RTF_MSG_WRN_RECORDBUFFULL,
	RTF_MSG_WRN_FIRSTPICNOTKEY,
	RTF_MSG_WRN_OUTPOINTSKIPPED,
	RTF_MSG_WRN_PICUNDERFLOW,
	RTF_MSG_WRN_PKTOVERFLOW,
	RTF_MSG_WRN_PICOVERFLOW,
	RTF_MSG_WRN_GOPOVERFLOW,
	RTF_MSG_WRN_BADCAT,
	RTF_MSG_WRN_CRYPTOPES,
	RTF_MSG_WRN_VARBITRATE,
	RTF_MSG_WRN_PCRMAXGAP,
	RTF_MSG_WRN_NOFIRSTPCRDCON,
	RTF_MSG_WRN_POSTFIRSTPCRDCON,
	RTF_MSG_WRN_MULTIPROGSTREAM,
	RTF_MSG_WRN_MULTIVIDSTREAM,
	RTF_MSG_WRN_MULTIAUDIOCODEC,
	RTF_MSG_WRN_SPLICINGAUDIOBAD,
	RTF_MSG_WRN_SPLICINGPSDESC,
	RTF_MSG_WRN_SPLICINGESDESC,
	RTF_MSG_WRN_PATCHANGED,
	RTF_MSG_WRN_CATCHANGED,
	RTF_MSG_WRN_PMTCHANGED,
	RTF_MSG_WRN_PMTENCRYPTED,
	RTF_MSG_WRN_AUGNOTFOUND,
	RTF_MSG_WRN_BADSLICETYPE,
	RTF_MSG_WRN_FRCMISMATCH,
	RTF_MSG_WRN_SYNCLOSS,
	RTF_MSG_WRN_SPLITPKT,
	RTF_MSG_WRN_BADTRICKSPEED,

	// special "always ignore" warnings
	RTF_MSG_WRN_ZEROOUTPUT,

	// info message codes - not counted as errors
	// note: RTF_MSG_INF_STATS is used for msgs whose only purpose
	// is to report values of variables (can't be done in short msg log)
	RTF_MSG_INF_STATS,
	RTF_MSG_INF_SUPPRESSWARNING,
	RTF_MSG_INF_LOGNULLHANDLE,
	RTF_MSG_INF_LOGSTATECLOSED,
	RTF_MSG_INF_LOGOUTRANGE,
	RTF_MSG_INF_SEQOPEN,
	RTF_MSG_INF_SEQADDGOP,
	RTF_MSG_INF_SEQCLOSE,
	RTF_MSG_INF_SEQRELEASE,
	RTF_MSG_INF_GOPOPEN,
	RTF_MSG_INF_GOPADDPIC,
	RTF_MSG_INF_GOPCLOSE,
	RTF_MSG_INF_GOPRELEASE,
	RTF_MSG_INF_PICOPEN,
	RTF_MSG_INF_PICCLOSE,
	RTF_MSG_INF_PICRELEASE,

	RTF_MSG_COUNT				// this always comes last

} RTF_MSG;

// RTF class definition files ***********************************************************

#ifndef NO_CLASS_DEFS

#include "RTFBuf.h"
#include "RTFCas.h"
#include "RTFCat.h"
#include "RTFFlt.h"
#include "RTFGop.h"
#include "RTFIdx.h"
#include "RTFObj.h"
#include "RTFOut.h"
#include "RTFPat.h"
#include "RTFPes.h"
#include "RTFPic.h"
#include "RTFPkt.h"
#include "RTFPmt.h"
#include "RTFSeq.h"
#include "RTFSes.h"
#include "RTFSys.h"
#include "RTFVcd.h"
#include "RTFWin.h"

#endif // #ifndef NO_CLASS_DEFS

// VVXLib definition files ************************************************************
#include "VVXLib.h"
#include "Vvx.h"
#include "VVXDef.h"

// VV2Lib definition files ************************************************************
#include "VV2Lib.h"

// internal utility functions
// evaluate a warning - return the following:
// < 0 - this is an error; log it and call the notifier
// 0 - this is a warning; log it
// > 0 - this is a suppressed warning; ignore it
int rtfWarningEval( RTF_HANDLE handle, RTF_MSG msg );

// report the occurrance of an error to the owning session
RTF_RESULT rtfReportErr( RTF_HANDLE handle, RTF_MSG errCode );

// log a message
void rtfLogMessage( RTF_HANDLE handle, RTF_MSG msg, const char *pInFunc, const char *pFmt, ... );

#endif // #ifndef _RTF_PRV_H

