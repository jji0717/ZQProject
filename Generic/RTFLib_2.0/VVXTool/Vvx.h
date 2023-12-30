// $Header: /ZQProjs/Generic/RTFLib_2.0/VVXTool/Vvx.h 1     10-11-12 15:59 Admin $
//
// Copyright (c) 1998 SeaChange Technology
//
// Module Name:
//
//    	Vvx.h
//
// Abstract:
//
//		Video on Demand Index File Layout
//
// Author:
//
//		Bill Davenport
//
// Environment:
//
//		User/Kernel Mode
//
// Notes:
//
// Revision History:
//
// $Log: /ZQProjs/Generic/RTFLib_2.0/VVXTool/Vvx.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 1     08-02-26 11:02 Ken.qian
//  
//  29    3/04/04 4:27p Wells
//  Fold in changes rom V4.2
//  
//  29    1/26/04 1:27p Wells
//  Add support for V7.2 vvx files
//  
//  28    10/09/03 9:26a Stetson
//  Additional splicing macros
//  
//  27    9/26/03 2:50p Wells
//  need both the source file offset of the splice and the offset of the
//  Iframe being spliced
//  
//  26    9/23/03 4:38p Wells
//  add offsets of the speed files associated with a splice in/out points
//  
//  25    9/22/03 4:30p Wells
//  add filler record type to handle case where termination record overlaps
//  a buffer boundary.
//  
//  24    7/29/03 4:53p Wells
//  add a 64 bit splice id to vvx splice record
//  
//  23    1/20/03 10:09a Wells
//  add splcing parameters to index header and bump minor version number
//  
//  22    11/07/02 2:13p Wells
//  rename vvx record name path
//  
//  21    10/31/02 8:37a Wells
//  changes supporting ad splicing
//  
//  20    7/26/02 7:57p Stetson
//  Unionize the timecode structure so that the full time code can be
//  easily referenced
//
//  19    1/30/02 4:17p Stetson
//  Move changes forward from V3.5
//  
//  20    1/22/02 5:18p Wells
//  back out write in progress flag in vvx index header
//  
//  19    1/22/02 12:48p Wells
//  
//  18    1/23/01 5:10p Sfxd
//  set vvxReadInProgress flag while waiting for VodProviderGetPrivateData
//  to complete.
//  
//  17    1/10/01 9:52a Sfxd
//  Add the esDescriptorData to user-mode vvx v6 and allow playback of fast
//  reverse files written in reverse order by user mode v6.
//  
//  16    1/02/01 1:50p Sfxd
//  Add esDescriptor data to the vvx index header. Extend the vvx index
//  header data into the beginning of the file if the private data buffer
//  is not large enough.
//  
//  15    12/13/00 10:47a Sfxd
//  Vod realfeed checkin
//  
//  14    9/26/00 3:04p Davenport
//  Update from 3.1 sources
//  
//  15    9/19/00 2:52p Davenport
//  Add a vvx header field containing header unused offset
//  
//  14    9/18/00 5:26p Davenport
//  Add support for passing elementary stream descriptors to the DVB board
//  so that multiple audio channels (with language descriptors) can be
//  supported for ITV
//  
//  13    4/03/00 10:31a Davenport
//  Check in support for trick file generation and pause mode using P
//  frames (controllable via a registry parameter which defaults to P frame
//  mode)
//  
//  12    10/22/99 4:08p Davenport
//  Add some additional fields to the header to allow for pause using B
//  frames
//  
//  11    9/21/99 1:12p Davenport
//  Remove VvxNew.h and merge TrickFiles and TrickFiles.New
//  
//  5     9/01/99 5:07p Davenport
//  Add video pid back to common index header area
//  
//  4     8/18/99 1:35p Davenport
//  Add definition of file extension for index file
//  
//  3     8/13/99 12:49p Davenport
//  
//  2     8/12/99 5:17p Davenport
//  
//  1     8/12/99 1:20p Davenport
//  
//  10    5/14/99 9:42a Davenport
//  Add a reserved area to vvx header
//  
//  9     5/12/99 1:23p Davenport
//  Bump file version to 4.0 now that Voddrv is processing the es table
//  
//  8     5/11/99 4:29p Davenport
//  Latent multi-play rate support
//  
//  7     5/07/99 5:32p Davenport
//  Add es stream information to vvx header
//  
//  6     4/14/99 2:53p Davenport
//  Changes to support reducing the number of dct coefficients to shrink
//  over-sized I-frames down to the target size.  Add support to vary the
//  number of gops per second and the number of pictures per gop.  Fix
//  problems reported by SA (top_field_first not set in B frame and closed
//  caption data not removed).
//  
//  5     4/07/99 5:07p Davenport
//  Add some additional fields to header
//  
//  4     2/05/99 11:56a Davenport
//  Changes too numerous to list
//  
//  3     1/28/99 11:33a Davenport
//  Change header size back to 8 KB
//

#ifndef _Vvx_
#define _Vvx_

#define VVX_INDEX_SIGNATURE					"Vod Video Index File"
#define VVX_INDEX_FILE_EXTENSION			".vvx"
#define VVX_INDEX_MAJOR_VERSION				5
#define VVX_INDEX_MINOR_VERSION				3
#define VVX_INDEX_HEADER_LENGTH				(8 * 1024)

#define VVX_MAX_SUBFILE_EXTENSION_LENGTH	7


#define VVX_SUBFILE_TYPE_PES				0
#define VVX_SUBFILE_TYPE_SPTS				1

#define	VVX_MAX_SUBFILE_COUNT				10

//
// Es stream flags
//
#define VVX_ESFLAG_AC3_AUDIO				0x01


//
// Es data - holds information for a specific es stream
//
typedef struct _VVX_ES_INFORMATION
{
	UCHAR							streamType;
	UCHAR							streamFlags;
	USHORT							pid;
} VVX_ES_INFORMATION, *PVVX_ES_INFORMATION;


typedef struct _VVX_ES_DESCRIPTOR_DATA
{
	ULONG							descriptorDataLength;
	ULONG							descriptorDataOffset;
} VVX_ES_DESCRIPTOR_DATA, *PVVX_ES_DESCRIPTOR_DATA;


typedef struct _VVX_TIME_CODE
{
	union {
		ULONG		time;
		struct {
			UCHAR	hours;
			UCHAR	minutes;
			UCHAR	seconds;
			UCHAR	frames;
		};
	};
} VVX_TIME_CODE, *PVVX_TIME_CODE;


typedef struct _VVX_FILE_SPEED
{
	LONG							numerator;
	ULONG							denominator;
} VVX_FILE_SPEED, *PVVX_FILE_SPEED;


//
// Sub File Information - holds information for a specific sub file
//
typedef struct _VVX_SUBFILE_INFORMATION
{
	UCHAR							fileExtension[VVX_MAX_SUBFILE_EXTENSION_LENGTH + 1];

	ULONG							fileType;

	UINT64						fileLength;

	ULONG							playTimeInMilliSeconds;

	VVX_FILE_SPEED					speed;

	ULONG							byteOffsetDataCount;
	ULONG							byteOffsetDataLength;
	ULONG							byteOffsetDataOffset;

} VVX_SUBFILE_INFORMATION, *PVVX_SUBFILE_INFORMATION;


//
// Index Header
//
typedef struct _VVX_INDEX_HEADER
{
	UCHAR							signature[32];

	ULONG							headerLength;

	USHORT							majorVersion;
	USHORT							minorVersion;

	UCHAR							programVersion[256];
	UCHAR							fileName[256];

	UCHAR							processingDate[40];
	UCHAR							processingTime[40];

	ULONG							mpegBitRate;
	ULONG							programNumber;
	ULONG							pmtPid;
	ULONG							pcrPid;
	ULONG							videoPid;

	ULONG							indexRecordCount;

	ULONG							esInformationCount;
	ULONG							esInformationLength;
	ULONG							esInformationOffset;

	ULONG							subFileInformationCount;
	ULONG							subFileInformationLength;
	ULONG							subFileInformationOffset;

	ULONG							timeCodeDataCount;
	ULONG							timeCodeDataLength;
	ULONG							timeCodeDataOffset;

	ULONG							zeroMotionBFrameOffset;
	ULONG							zeroMotionBFrameLength;

	//
	// The following fields were added in vvx header version 5.1
	//
	ULONG							frameRateCode;
	INT64						dtsPtsTimeOffset;

	ULONG							horizontalSize;
	ULONG							verticalSize;
	ULONG							videoBitRate;

	//
	// The following fields were added in vvx header version 5.2
	//
	ULONG							zeroMotionBFramePesOffset;
	ULONG							zeroMotionBFramePesLength;
	ULONG							zeroMotionPFramePesOffset;
	ULONG							zeroMotionPFramePesLength;

	//
	// The following fields were added in vvx header version 5.3
	//
	ULONG							esDescriptorDataCount;
	ULONG							esDescriptorDataLength;
	ULONG							esDescriptorDataOffset;
	ULONG							unusedHeaderOffset;

} VVX_INDEX_HEADER, *PVVX_INDEX_HEADER;


//
// VVX Version 4 definitions
//
enum
{
	VVX_V4_INDEX_OFFSET_NORMAL = 0,
	VVX_V4_INDEX_OFFSET_FF,
	VVX_V4_INDEX_OFFSET_FR,
	VVX_V4_INDEX_MAX_OFFSETS
};


typedef struct _VVX_V4_INDEX_HEADER
{
	UCHAR							signature[32];

	ULONG							headerLength;

	USHORT							majorVersion;
	USHORT							minorVersion;

	UCHAR							programVersion[256];
	UCHAR							fileName[256];

	UCHAR							processingDate[40];
	UCHAR							processingTime[40];

	ULONG							dvlHeaderSize;
	UINT64						mpegFileSize;

	ULONG							mpegBitRate;
	ULONG							mpegPacketsPerSecond;
	ULONG							mpegPlayTimeInMilliSeconds;
	ULONG							mpegPacketCount;

	ULONG							programNumber;
	ULONG							esCount;
	ULONG							pmtPid;
	ULONG							pcrPid;
	ULONG							videoPid;

	ULONG							esInformationOffset;
	ULONG							esInformationLength;

	ULONG							trickFrameRate;

	ULONG							maxOffsets;

	ULONG							frameIndexCount;
	ULONG							iFrameExtractCount;

	ULONG							trickPid;
	ULONG							audioPid;

	ULONG							frameIndexOffset;
	ULONG							frameIndexLength;

	ULONG							zeroMotionBFrameOffset;
	ULONG							zeroMotionBFrameLength;

	ULONG							gopsPerSecond;
	ULONG							picturesPerGop;

	ULONG							reserved[64];

} VVX_V4_INDEX_HEADER, *PVVX_V4_INDEX_HEADER;


typedef struct _VVX_V4_FRAME_INDEX
{
	VVX_TIME_CODE					timeCode;
	UCHAR							pictureType;
	UCHAR							reserved[3];
	UINT64						byteOffset[VVX_V4_INDEX_MAX_OFFSETS];
} VVX_V4_FRAME_INDEX, *PVVX_V4_FRAME_INDEX;


typedef struct _VVX_V6_SUBFILE_INFORMATION
{
	UCHAR							fileExtension[VVX_MAX_SUBFILE_EXTENSION_LENGTH + 1];

	ULONG							fileType;

	UINT64						startingByte;
	UINT64						endingByte;

	ULONG							playTimeInMilliSeconds;

	VVX_FILE_SPEED					speed;

	ULONG							recordIndex;

} VVX_V6_SUBFILE_INFORMATION, *PVVX_V6_SUBFILE_INFORMATION;

typedef VVX_V6_SUBFILE_INFORMATION VVX_V7_SUBFILE_INFORMATION, *PVVX_V7_SUBFILE_INFORMATION;

typedef struct _VVX_TS_INFORMATION
{
	ULONG							programNumber;
	ULONG							pmtPid;
	ULONG							pcrPid;
	ULONG							videoPid;
} VVX_TS_INFORMATION, *PVVX_TS_INFORMATION;

typedef struct _VVX_FRAME_DATA
{
	VVX_TIME_CODE					timeCode;
	UINT64						frameByteOffset[VVX_MAX_SUBFILE_COUNT];
} VVX_FRAME_DATA, *PVVX_FRAME_DATA;

//
// add checksum and import-in-progress flag to header
//
#define V6VVX_MINORVERSION_ONE		1

#define V6VVX_CURRENT_MINORVERSION	V6VVX_MINORVERSION_ONE

typedef struct _VVX_V6_INDEX_HEADER
{
	UCHAR							signature[32];

	ULONG							headerLength;

	USHORT							majorVersion;
	USHORT							minorVersion;

	UCHAR							programVersion[256];
	UCHAR							fileName[256];

	UINT64						systemTime;

	ULONG							streamBitRate;
	ULONG							streamType;

	// transport stream specific info
	ULONG							tsInformationCount;
	ULONG							tsInformationLength;
	ULONG							tsInformationOffset;

	ULONG							indexRecordCount;

	ULONG							esInformationCount;
	ULONG							esInformationLength;
	ULONG							esInformationOffset;

	ULONG							subFileInformationMaxCount;
	ULONG							subFileInformationCount;
	ULONG							subFileInformationLength;
	ULONG							subFileInformationOffset;

	ULONG							frameRateCode;
	INT64						dtsPtsTimeOffset;

	ULONG							horizontalSize;
	ULONG							verticalSize;
	ULONG							videoBitRate;

	ULONG							zeroMotionPictureType;
	ULONG							zeroMotionBFramePesOffset;
	ULONG							zeroMotionBFramePesLength;
	ULONG							zeroMotionFramePesOffset;
	ULONG							zeroMotionFramePesLength;

	ULONG							frameDataCount;
	ULONG							frameDataOffset;
	ULONG							frameDataLength;

	//
	// The following fields were added in vvx header version 5.3
	//
	ULONG							esDescriptorDataCount;
	ULONG							esDescriptorDataLength;
	ULONG							esDescriptorDataOffset;
	ULONG							extendedHeaderLength;

	BOOLEAN							reverseRead;
	BOOLEAN							usePrivateData;
	BOOLEAN							AVAILABLE_BOOLEAN_1;
	BOOLEAN							AVAILABLE_BOOLEAN;
	//
	//  add new fields before here
	//
	// The checksumming algorithm is 
	//
	//   ULONG checksum = ComputeChecksum(&vvxIndexHeader, vvxIndexHeader.headerLength-sizeof(ULONG));
	//   ULONG *pChecksum = (ULONG *)((char *)&vvxIndexHeader) + (vvxIndexHeader.headerLength-sizeof(ULONG)));
	//   *pCheckSum = checksum;
	//
	// The checksum is always stored in the last 4 bytes of the vvxIndexHeader structure
	// which allows changes to be made to the header structure as time goes on and still 
	// be able to verify the checksum of any given header.
	//
	// We define the checksumRef field to make sure the space is allocated and to facilitate 
	// adding additional data (offset data from above) immediatly after the vvxIndexHeader 
	// structure (ie. sizeof(vvxIndexHeader) see TrickFilesInitializeVvxIndexHeader in 
	// TrickFileLibary.c). The compiler might artificially grow the vvxIndexHeader 
	// structure for optimization reasons in a way that the stored checksum is not located 
	// at vvxIndexHeader.checksumRef which is why we use the algorithm shown above.
	//
	ULONG							checksumRef;

} VVX_V6_INDEX_HEADER, *PVVX_V6_INDEX_HEADER;

typedef unsigned short VVX_SPLICE_TYPE;

#define VVX_SPLICE_TYPE_MASK				0xff

#define VVX_SPLICE_TYPE_GOOD				(VVX_SPLICE_TYPE)(' ' << 8)

#define VVX_SPLICE_TYPE_FRONT_CHAR			'F'
#define VVX_SPLICE_TYPE_BACK_CHAR			'B'
#define VVX_SPLICE_TYPE_IN_CHAR				'I'
#define VVX_SPLICE_TYPE_OUT_CHAR			'O'

#define VVX_SPLICE_TYPE_NULL				(VVX_SPLICE_TYPE)'X'
#define VVX_SPLICE_TYPE_FRONT				(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_GOOD | VVX_SPLICE_TYPE_FRONT_CHAR)
#define VVX_SPLICE_TYPE_IN					(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_GOOD | VVX_SPLICE_TYPE_IN_CHAR)
#define VVX_SPLICE_TYPE_OUT					(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_GOOD | VVX_SPLICE_TYPE_OUT_CHAR)
#define VVX_SPLICE_TYPE_BACK				(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_GOOD | VVX_SPLICE_TYPE_BACK_CHAR)

#define VVX_SPLICE_TYPE_BAD					(VVX_SPLICE_TYPE)('X' << 8)
#define VVX_SPLICE_TYPE_BAD_FRONT			(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_BAD | VVX_SPLICE_TYPE_FRONT)
#define VVX_SPLICE_TYPE_BAD_IN				(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_BAD | VVX_SPLICE_TYPE_IN)
#define VVX_SPLICE_TYPE_BAD_OUT				(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_BAD | VVX_SPLICE_TYPE_OUT)
#define VVX_SPLICE_TYPE_BAD_BACK			(VVX_SPLICE_TYPE)(VVX_SPLICE_TYPE_BAD | VVX_SPLICE_TYPE_BACK)

#define VVX_SPLICE_TYPE_IS_FRONT(v)			(((v) & VVX_SPLICE_TYPE_MASK) == VVX_SPLICE_TYPE_FRONT_CHAR)
#define VVX_SPLICE_TYPE_IS_IN(v)			(((v) & VVX_SPLICE_TYPE_MASK) == VVX_SPLICE_TYPE_IN_CHAR)
#define VVX_SPLICE_TYPE_IS_OUT(v)			(((v) & VVX_SPLICE_TYPE_MASK) == VVX_SPLICE_TYPE_OUT_CHAR)
#define VVX_SPLICE_TYPE_IS_BACK(v)			(((v) & VVX_SPLICE_TYPE_MASK) == VVX_SPLICE_TYPE_BACK_CHAR)

#define VVX_SPLICE_TYPE_IS_VALID_FRONT(v)	((v) == VVX_SPLICE_TYPE_FRONT)
#define VVX_SPLICE_TYPE_IS_VALID_IN(v)		((v) == VVX_SPLICE_TYPE_IN)
#define VVX_SPLICE_TYPE_IS_VALID_OUT(v)		((v) == VVX_SPLICE_TYPE_OUT)
#define VVX_SPLICE_TYPE_IS_VALID_BACK(v)	((v) == VVX_SPLICE_TYPE_BACK)

#define VVX_SPLICE_TYPE_IS_VALID(v)			(((v) & ~VVX_SPLICE_TYPE_MASK) == VVX_SPLICE_TYPE_GOOD)

typedef struct _VVX_SPLICE_DATA
{
	VVX_SPLICE_TYPE		type;
	INT64			spliceId;
	INT64			timeOffset;
	INT64			sourceFileOffset;
	INT64			spliceFileOffset;
	INT64			spliceSegmentLength;
	union
	{
		UINT64		subFileByteOffset[VVX_MAX_SUBFILE_COUNT];	// V7.1
		ULONG			trickIndexOfSplice;							// V7.2
	};
} VVX_SPLICE_DATA, *PVVX_SPLICE_DATA;

typedef enum 
{
	TRICK_INDEX		= 'T',
	SPLICE_INDEX	= 'S',
	FILLER			= 'F'
} VVX_RECORD_TYPE;

typedef struct _VVX_V7_RECORD
{
	VVX_RECORD_TYPE	recordType;
	union
	{
		VVX_FRAME_DATA	trick;
		VVX_SPLICE_DATA	splice;
	};
} VVX_V7_RECORD, *PVVX_V7_RECORD;

//
// VVX V7 Minor Version:
//
// 0	initial version
// 1	add splice parameters to vvx file
// 2	change index array to a singe index number representing the index of an Iframe
//		corresponding to this splice point.
//
#define V7VVX_CURRENT_MINORVERSION			2

typedef struct _VVX_V7_INDEX_HEADER
{
	UCHAR							signature[32];

	ULONG							headerLength;

	USHORT							majorVersion;
	USHORT							minorVersion;

	UCHAR							programVersion[256];
	UCHAR							fileName[256];

	UINT64						systemTime;

	ULONG							streamBitRate;
	ULONG							streamType;

	// transport stream specific info

	ULONG							tsInformationCount;
	ULONG							tsInformationLength;
	ULONG							tsInformationOffset;

	ULONG							trickIndexRecordCount;
	ULONG							spliceIndexRecordCount;

	ULONG							esInformationCount;
	ULONG							esInformationLength;
	ULONG							esInformationOffset;

	ULONG							subFileInformationMaxCount;
	ULONG							subFileInformationCount;
	ULONG							subFileInformationLength;
	ULONG							subFileInformationOffset;

	ULONG							frameRateCode;
	INT64						dtsPtsTimeOffset;

	ULONG							horizontalSize;
	ULONG							verticalSize;
	ULONG							videoBitRate;

	ULONG							zeroMotionPictureType;
	ULONG							zeroMotionBFramePesOffset;
	ULONG							zeroMotionBFramePesLength;
	ULONG							zeroMotionFramePesOffset;
	ULONG							zeroMotionFramePesLength;

	ULONG							frameDataCount;
	ULONG							frameDataOffset;
	ULONG							frameDataLength;

	ULONG							esDescriptorDataCount;
	ULONG							esDescriptorDataLength;
	ULONG							esDescriptorDataOffset;
	ULONG							extendedHeaderLength;

	BOOLEAN							reverseRead;
	BOOLEAN							usePrivateData;
	BOOLEAN							AVAILABLE_BOOLEAN_1;
	BOOLEAN							AVAILABLE_BOOLEAN;
	//
	// splicing control parameters
	//
	ULONG							splicingParametersLength;
	ULONG							splicingParametersOffset;
	//
	//  add new fields before here
	//
	// The checksumming algorithm is 
	//
	//   ULONG checksum = ComputeChecksum(&vvxIndexHeader, vvxIndexHeader.headerLength-sizeof(ULONG));
	//   ULONG *pChecksum = (ULONG *)((char *)&vvxIndexHeader) + (vvxIndexHeader.headerLength-sizeof(ULONG)));
	//   *pCheckSum = checksum;
	//
	// The checksum is always stored in the last 4 bytes of the vvxIndexHeader structure
	// which allows changes to be made to the header structure as time goes on and still 
	// be able to verify the checksum of any given header.
	//
	// We define the checksumRef field to make sure the space is allocated and to facilitate 
	// adding additional data (offset data from above) immediatly after the vvxIndexHeader 
	// structure (ie. sizeof(vvxIndexHeader) see TrickFilesInitializeVvxIndexHeader in 
	// TrickFileLibary.c). The compiler might artificially grow the vvxIndexHeader 
	// structure for optimization reasons in a way that the stored checksum is not located 
	// at vvxIndexHeader.checksumRef which is why we use the algorithm shown above.
	//
	ULONG							checksumRef;

} VVX_V7_INDEX_HEADER, *PVVX_V7_INDEX_HEADER;
#endif
