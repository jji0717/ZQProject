// definition file for rtfCod class MPEG-2 video codec
//

#ifndef _RTF_COD_MPEG2_H
#define _RTF_COD_MPEG2_H 1

// forward reference pointer to parent class state structure
typedef struct _RTF_COD *P_RTF_COD;

// MPEG2 constants
#define MAX_ZSLICE_BYTES			16

#define ZMPS_PREFIX_BIT_COUNT		46
#define ZMPS_PREFIX_BYTE_COUNT		( ( ZMPS_PREFIX_BIT_COUNT + 7 ) >> 3 )
#define ZMPS_SUFFIX_BIT_COUNT		7
#define ZMPS_SUFFIX_BYTE_COUNT		( ( ZMPS_SUFFIX_BIT_COUNT + 7 ) >> 3 )

#define ZMBS_PREFIX_BIT_COUNT		47
#define ZMBS_PREFIX_BYTE_COUNT		( ( ZMBS_PREFIX_BIT_COUNT + 7 ) >> 3 )
#define ZMBS_SUFFIX_BIT_COUNT		8
#define ZMBS_SUFFIX_BYTE_COUNT		( ( ZMBS_SUFFIX_BIT_COUNT + 7 ) >> 3 )

// MPEG2 start code type enumerator
typedef enum _RTF_STARTCODE_TYPE_MPEG2
{
	RTF_STARTCODE_TYPE_MPEG2_INVALID = 0,

	// see H.262 part II (MPEG2 VIDEO) page 25
	RTF_STARTCODE_TYPE_MPEG2_PICTURE,		// 0x00
	RTF_STARTCODE_TYPE_MPEG2_SLICE,			// 0x01 - 0xAF
	RTF_STARTCODE_TYPE_MPEG2_RESERVED0,		// 0xB0
	RTF_STARTCODE_TYPE_MPEG2_RESERVED1,		// 0xB1
	RTF_STARTCODE_TYPE_MPEG2_USERDATA,		// 0xB2
	RTF_STARTCODE_TYPE_MPEG2_SEQHEADER,		// 0xB3
	RTF_STARTCODE_TYPE_MPEG2_SEQERROR,		// 0xB4
	RTF_STARTCODE_TYPE_MPEG2_SEQEXTENSION,	// 0xB5
	RTF_STARTCODE_TYPE_MPEG2_RESERVED2,		// 0xB6
	RTF_STARTCODE_TYPE_MPEG2_SEQEND,		// 0xB7
	RTF_STARTCODE_TYPE_MPEG2_GROUP,			// 0xB8 

	// see H.222 part I (MPEG-2 TRANSPORT) page 35
	RTF_STARTCODE_TYPE_MPEG2_ISOEND,		// 0xB9
	RTF_STARTCODE_TYPE_MPEG2_PACK,			// 0xBA
	RTF_STARTCODE_TYPE_MPEG2_SYSTEM,		// 0xBB
	RTF_STARTCODE_TYPE_MPEG2_STREAMID,		// 0xBC - 0xFF

} RTF_STARTCODE_TYPE_MPEG2;

// MPEG2 codec specific info structure
typedef struct _RTF_CODEC_INFO_MPEG2
{
	unsigned long getNCPFrameCallCount;

} RTF_CODEC_INFO_MPEG2;

// MPEG2 codec specific functions (only used in RTFCod.c)

// process an MPEG2 start code
RTF_RESULT rtfCodMpeg2ProcessStartCode( P_RTF_COD pCod, unsigned long code );

// build the MPEG-2 no change frames
void rtfCodMpeg2BuildNCFrames( P_RTF_COD pCod );

#endif // #ifndef _RTF_COD_MPEG2_H

