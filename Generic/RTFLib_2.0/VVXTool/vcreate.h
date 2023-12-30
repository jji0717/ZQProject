#ifndef vcreate_h_included
#define vcreate_h_included

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define MAX_PATHLEN _MAX_PATH
#else
#define MAX_PATHLEN 256
#endif

//
// these elements are used to format and index header descriptor
//
#define V7VVX_NEXT_MINORVERSION					( V7VVX_CURRENT_MINORVERSION + 1 )
#define LIBRARY_VERSION							"VvxLib V1.0"
#define LIBRARY_COPYRIGHT						"Copyright (c) 1998-2006, SeaChange Systems"

#define TRANSPORT_PACKET_SIZE					188
#define MAX_ZERO_MOTION_PACKETS					6
//
// PMT table id from H.262
//
#define MPEG2_TRANSPORT_PMT_TABLE_ID			2
//
// a structure to describe the output files
//
typedef struct output_files
{
	char		extension[8];	// max 7 chars plus a 0
	int			numerator;		// +/- speed
	int			denominator;
	INT64		fileSize;
	int			index;			// order of index data
} OUTPUT_FILES, *POUTPUT_FILES;
//
// a VVX time code structure
//
typedef struct _TIME_CODE 
{
	UCHAR	hours;
	UCHAR	minutes;
	UCHAR	seconds;
	UCHAR	frames;
} TIME_CODE, *PTIME_CODE;
//
// index context definition
//
typedef struct vvx_index_context
{
	//
	// required inputs for SetupInitialIndexHeader
	//
	OUTPUT_FILES	outputFiles[10];
	int				minorVersion;			// minor version number of index
	int				ffSpeedCount;			// count of ff files
	int				frSpeedCount;			// count of fr files
	int				transportBitRate;		// transport bit rate
	int				videoBitRate;			// from seq hdr and I've seen 15,000,000 from some encoders
	int				frame_rate_code;		// video bit rate
	int				dtsPtsTimeOffset;		// decoding delay of first I-Frame
	int				horizontal_size;		// resolution in bits
	int				vertical_size;			// resolution in bits
	int				program_number;			// program number of SPT
	int				pmtPid;					// pmt pid from PAT
	int				pcrPid;					// pcr pid from PMT
	int				videoPid;				// video pid from PMT
	int				zeroMotionFrameType;	// set to 2 (P-Frame)
	char			filename[MAX_PATHLEN];	// main file name
	BYTE			pat[188];				// copy of pat
	BYTE			pmt[188];				// copy of pmt
	PVOID			writeContext;			// input for writeRtn
	int				(*writeRtn)(PVOID pfp, INT64 offsetFromCurrentPosition, PBYTE buffer, int bufferLength);
	//
	// data used by index library
	//
	int				patEsInfoOffset;		// elementary stream info offset in PMT
	int				pmtEsInfoCount;			// elementary stream count in PMT
	int				pmtEsInfoLength;		// elementary stream byte count in PMT
	int				lenZeroMotionP;			// length of zero motion P frame
	int				lenZeroMotionB;			// length of zero motion B frame
	char			extensionHeaders[1024];	// needs to be at least 616 bytes
	// zero motion P-frame
	BYTE			zeroMotionP[TRANSPORT_PACKET_BYTES*MAX_ZERO_MOTION_PACKETS];
	// zero motion B-Frame
	BYTE			zeroMotionB[TRANSPORT_PACKET_BYTES*MAX_ZERO_MOTION_PACKETS];
	//
	// Tim is a 64-bit value representing the number of 100-nanosecond intervals 
	// since January 1, 1601 (UTC).
	//
	FILETIME		systemTime;
	int				trickIndexRecordCount;	// index records processed
	int				frameDataCount;		// frame records processed
	int				fileCount;			// output file count

	UINT			firstRecordWritten : 1; // record written flag

	INT64			currentIndexFilePos; // current offset from start of index file

} VVX_INDEX_CONTEXT;

//
// entry points in indexlib.c
//
EXPORT int vvxSetupInitialIndexHeader(VVX_INDEX_CONTEXT *pContext);
EXPORT int vvxWriteIndexHeader(VVX_INDEX_CONTEXT *pContext, PBYTE buffer, int bufferLen);
EXPORT int vvxWriteFirstRecord(VVX_INDEX_CONTEXT *pContext, PBYTE buffer, int bufferLen);
EXPORT int vvxWriteIndexRecord(VVX_INDEX_CONTEXT *pContext, TIME_CODE timeCode, INT64 *indexTable, INT64 *sizeTable, PBYTE buffer, int bufferLen);
EXPORT int vvxWriteFinalTerminator(VVX_INDEX_CONTEXT *pContext, INT64 *sizeTable, PBYTE buffer, int bufferLen);

#ifdef __cplusplus
}
#endif

#endif
