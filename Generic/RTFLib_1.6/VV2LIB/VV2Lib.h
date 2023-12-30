// definition file for index file generator class
//

#ifndef _RTF_VV2LIB_H
#define _RTF_VV2LIB_H 1

#ifdef __cplusplus
extern "C" {
#endif

// constants ****************************************************************************

#define VV2_MAX_TRICKSPEEDS			4
#define VV2_MAX_TRICKFILES			( 2 * VV2_MAX_TRICKSPEEDS )
#define VV2_MAX_SUBFILES			( VV2_MAX_TRICKFILES + 2 )
#define VV2_MAX_NO_CHANGE_PACKETS	8
#define VV2_MAX_OUT_POINTS			32
#define VV2_MAX_HDR_LINE_BYTES		64
#define VV2_MAX_DATA_LINE_BYTES		64
#define VV2_MAX_OUTBUF_BYTES		( 8 * 1024 )
#define VV2_MAX_IDX_PICS			8

// macros *******************************************************************************
 
#ifdef _WIN32
#define PRINTF	printf
#endif
 
#ifdef _LINUX
#ifdef __KERNEL__
#define PRINTF	kprintf
#else
#define PRINTF	printf
#endif
#endif

// typedefs *****************************************************************************

typedef enum _VV2_INDEX_MODE
{
	VV2_INDEX_MODE_INVALID = 0,

	VV2_INDEX_MODE_OFFLINE,
	VV2_INDEX_MODE_REALTIME,

	VV2_INDEX_MODE_LIMIT

} VV2_INDEX_MODE;

typedef enum _VV2_DRIVER_FLAVOR
{
	VV2_DRIVER_FLAVOR_INVALID = 0,

	VV2_DRIVER_FLAVOR_ON2RTP,
	VV2_DRIVER_FLAVOR_TSOIP,

	VV2_DRIVER_FLAVOR_LIMIT

} VV2_DRIVER_FLAVOR;

typedef enum _VV2_TRANSPORT_FLAVOR
{
	VV2_TRANSPORT_FLAVOR_INVALID = 0,

	VV2_TRANSPORT_FLAVOR_TS,
	VV2_TRANSPORT_FLAVOR_TTS,

	VV2_TRANSPORT_FLAVOR_LIMIT

} VV2_TRANSPORT_FLAVOR;

typedef enum _VV2_INDEX_OPTION
{
	VV2_INDEX_OPTION_INVALID = 0,

	VV2_INDEX_OPTION_ON2RTP,
	VV2_INDEX_OPTION_TSOIP,

	VV2_INDEX_OPTION_LIMIT

} VV2_INDEX_OPTION;

typedef struct _VV2_FILE_INFO
{
	char extension[8];	 // max 7 chars plus a 0
	int	 numerator;		 // positive
	int	 denominator;	 // non-zero
	int	 direction;		 // fwd=+1, rev=-1, bidir=0
	int	 bitrate;		 // output bitrate
	int  fileid;		 // file ID

} VV2_FILE_INFO;

typedef struct _VV2_PIC_INFO
{
	int filenum;				// output number of file containing picture
	unsigned long pktNumber;	// number of first packet in picture
	unsigned long pktCount;		// number of packets in picture
	unsigned long npt;			// NPT of picture

} VV2_PIC_INFO;

typedef struct _VV2_DRIVER_INFO_ON2RTP
{
	// placeholder
	unsigned long foo;

} VV2_DRIVER_INFO_ON2RTP;

typedef struct _VV2_DRIVER_INFO_TSOIP
{
	// placeholder
	unsigned long bar;

} VV2_DRIVER_INFO_TSOIP;

typedef union _VV2_DRIVER_INFO
{
	VV2_DRIVER_INFO_ON2RTP rtp;
	VV2_DRIVER_INFO_TSOIP tso;

} VV2_DRIVER_INFO;

typedef struct _VV2_INDEX_CONTEXT
{
	// info recorded at open time
	VV2_TRANSPORT_FLAVOR transportFlavor;
	VV2_DRIVER_FLAVOR driverFlavor;
	VV2_DRIVER_INFO driverInfo;
	VV2_INDEX_MODE mode;
	VV2_INDEX_OPTION option;
	int playDurationMsecs;
	PVOID writeContext;
	int (*writeRtn)(PVOID pfp, INT64 offsetFromCurrentPosition, PBYTE buffer, int bufferLength);
	INT64 nextWriteOffset;
	int recfmt;
	int outCount;
	int outputPacketBytes;
	int mainFileOutputNumber;
	int indexFileOutputNumber;
	int noChangeFrameBytes;
	char *pProgramVersionStr;

	// info that can change at each index point
	int indexPointCount;
	unsigned long seqNumber;
	VV2_FILE_INFO fileInfo[ VV2_MAX_SUBFILES ];

	// offsets of main file keyframe, and number and
	// offsets of out points within current interval
	int groupOutpointCount;
	unsigned long outpointPacketNumber[ VV2_MAX_OUT_POINTS ];

	// indexable picture list for the current index interval
	int idxPicCount;
	VV2_PIC_INFO idxPic[ VV2_MAX_IDX_PICS ];

	// no-change frame (used for bit rate control)
	unsigned char noChangeFrame[ TRANSPORT_PACKET_BYTES * VV2_MAX_NO_CHANGE_PACKETS ];

	// output buffer - used to accumulate index records
	char outBuf[ VV2_MAX_OUTBUF_BYTES ];

} VV2_INDEX_CONTEXT;

// function prototypes ******************************************************************

EXPORT int vv2WriteIndexHeader( VV2_INDEX_CONTEXT *pContext );
EXPORT int vv2OpenDataSection ( VV2_INDEX_CONTEXT *pContext );
EXPORT int vv2RecordOutpoint  ( VV2_INDEX_CONTEXT *pContext, unsigned long packetNumber );
EXPORT int vv2RecordKeyframe  ( VV2_INDEX_CONTEXT *pContext, unsigned long packetNumber );
EXPORT int vv2WriteIndexRecord( VV2_INDEX_CONTEXT *pContext );
EXPORT int vv2CloseDataSection( VV2_INDEX_CONTEXT *pContext, unsigned long msecs );

#ifdef __cplusplus
}
#endif

#endif		// #ifndef _RTF_VV2LIB_H

