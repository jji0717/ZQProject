// definition file for rtfVcd class H.264 video codec
//

#ifndef _RTF_VCD_H264_H
#define _RTF_VCD_H264_H 1

// H.264 constants
#define H264_MB_PIXEL_WIDTH		16
#define H264_MB_PIXEL_HEIGHT	16
#define H264_MAX_SPS_COUNT		8	// max # of active and pending sequence parameter sets
#define H264_MAX_PPS_COUNT		8	// max # of active and pending picture parameter sets
#define H264_EXTENDED_SAR		255

// H264 start code type enumerator
typedef enum _RTF_STARTCODE_TYPE_H264
{
	RTF_STARTCODE_TYPE_H264_INVALID = 0,

	RTF_STARTCODE_TYPE_H264_UNSPECIFIED,		// 0x00, 0x18-0x1F
	RTF_STARTCODE_TYPE_H264_SLICENONIDR,		// 0x01
	RTF_STARTCODE_TYPE_H264_SLICEPARTA,			// 0x02
	RTF_STARTCODE_TYPE_H264_SLICEPARTB,			// 0x03
	RTF_STARTCODE_TYPE_H264_SLICEPARTC,			// 0x04
	RTF_STARTCODE_TYPE_H264_SLICEIDR,			// 0x05
	RTF_STARTCODE_TYPE_H264_SEI,				// 0x06
	RTF_STARTCODE_TYPE_H264_SEQPARAMSET,		// 0x07
	RTF_STARTCODE_TYPE_H264_PICPARAMSET,		// 0x08
	RTF_STARTCODE_TYPE_H264_ACCESSUNITDELIMITER,// 0x09
	RTF_STARTCODE_TYPE_H264_ENDOFSEQ,			// 0x0A
	RTF_STARTCODE_TYPE_H264_ENDOFSTREAM,		// 0x0B 
	RTF_STARTCODE_TYPE_H264_FILLERDATA,			// 0x0C
	RTF_STARTCODE_TYPE_H264_RESERVED,			// 0x0D-0x17
	RTF_STARTCODE_TYPE_H264_SYSTEM,				// 0xBB

} RTF_STARTCODE_TYPE_H264;

// H.264 NAL unit type
typedef enum _RTF_NAL_UNIT_TYPE
{
	RTF_NAL_UNIT_TYPE_UNSPEC_0 = 0,
	RTF_NAL_UNIT_TYPE_NONIDR,
	RTF_NAL_UNIT_TYPE_PARTA,
	RTF_NAL_UNIT_TYPE_PARTB,
	RTF_NAL_UNIT_TYPE_PARTC,
	RTF_NAL_UNIT_TYPE_IDR,
	RTF_NAL_UNIT_TYPE_SEI,
	RTF_NAL_UNIT_TYPE_SPS,
	RTF_NAL_UNIT_TYPE_PPS,
	RTF_NAL_UNIT_TYPE_AUD,
	RTF_NAL_UNIT_TYPE_ENDSEQ,
	RTF_NAL_UNIT_TYPE_ENDSTR,
	RTF_NAL_UNIT_TYPE_FILLER,
	RTF_NAL_UNIT_TYPE_RESERV_13,
	RTF_NAL_UNIT_TYPE_RESERV_14,
	RTF_NAL_UNIT_TYPE_RESERV_15,
	RTF_NAL_UNIT_TYPE_RESERV_16,
	RTF_NAL_UNIT_TYPE_RESERV_17,
	RTF_NAL_UNIT_TYPE_RESERV_18,
	RTF_NAL_UNIT_TYPE_RESERV_19,
	RTF_NAL_UNIT_TYPE_RESERV_20,
	RTF_NAL_UNIT_TYPE_RESERV_21,
	RTF_NAL_UNIT_TYPE_RESERV_22,
	RTF_NAL_UNIT_TYPE_RESERV_23,
	RTF_NAL_UNIT_TYPE_UNSPEC_24,
	RTF_NAL_UNIT_TYPE_UNSPEC_25,
	RTF_NAL_UNIT_TYPE_UNSPEC_26,
	RTF_NAL_UNIT_TYPE_UNSPEC_27,
	RTF_NAL_UNIT_TYPE_UNSPEC_28,
	RTF_NAL_UNIT_TYPE_UNSPEC_29,
	RTF_NAL_UNIT_TYPE_UNSPEC_30,
	RTF_NAL_UNIT_TYPE_UNSPEC_31

} RTF_NAL_UNIT_TYPE;

// H.264 slice type
typedef enum _RTF_SLICE_TYPE_H264
{
	RTF_SLICE_TYPE_H264_P = 0,
	RTF_SLICE_TYPE_H264_B,
	RTF_SLICE_TYPE_H264_I,
	RTF_SLICE_TYPE_H264_SP,
	RTF_SLICE_TYPE_H264_SI,
	RTF_SLICE_TYPE_H264_P2,
	RTF_SLICE_TYPE_H264_B2,
	RTF_SLICE_TYPE_H264_I2,
	RTF_SLICE_TYPE_H264_SP2,
	RTF_SLICE_TYPE_H264_SI2

} RTF_SLICE_TYPE_H264;

// H.264 primary picture type
typedef enum _RTF_VCD_PPT_H264
{
	RTF_VCD_PPT_H264_INVALID = 0,

	RTF_VCD_PPT_H264_I,
	RTF_VCD_PPT_H264_IP,
	RTF_VCD_PPT_H264_IPB,
	RTF_VCD_PPT_H264_SI,
	RTF_VCD_PPT_H264_SISP,
	RTF_VCD_PPT_H264_ISI,
	RTF_VCD_PPT_H264_ISIPSP,
	RTF_VCD_PPT_H264_ISIPSPB,

} RTF_VCD_PPT_H264;

// H.264 sequence parameter set structure
typedef struct _RTF_VCD_SPS_H264
{
	// state info
	BOOL valid;
	unsigned long useCount;
	// SPS fields
	BOOL frameMbsOnlyFlag;
	BOOL mbAdaptiveFrameFieldFlag;
	BOOL deltaPicOrderAlwaysZeroFlag;
	unsigned long spsID;
	unsigned long videoFormat;
	unsigned long picWidth;
	unsigned long picWidthInMbs;
	unsigned long picHeight;
	unsigned long picHeightInMbs;
	unsigned long picHeightInMapUnits;
	unsigned long picSizeInMbs;
	unsigned long log2MaxFrameNum;
	unsigned long log2MaxPicOrderCntLsb;
	unsigned long picOrderCntType;
	unsigned long deltaPicOrderCntBottom;
	// raw packets
	unsigned char pkt[ TRANSPORT_PACKET_BYTES ];

} RTF_VCD_SPS_H264;

// H.264 picture parameter set structure
typedef struct _RTF_VCD_PPS_H264
{
	// state info
	BOOL valid;
	// PPS fields
	BOOL entropyCodingModeFlag;
	BOOL picOrderPresentFlag;
	BOOL redundantPicCntPresentFlag;
	BOOL deblockingFilterControlPresentFlag;
	BOOL weighted_pred_flag;
	unsigned long useCount;
	unsigned long ppsID;
	unsigned long spsID;
	unsigned long numSliceGroupsMinus1;
	unsigned long sliceGroupMapType;
	unsigned long sliceGroupChangeRate;
	unsigned long numRefIdxL0ActiveMinus1;
	// raw packets
	unsigned char pkt[ TRANSPORT_PACKET_BYTES ];

} RTF_VCD_PPS_H264;

// H.264 codec specific info structure
typedef struct _RTF_VCD_INFO_H264
{
	// TRUE if current frame is keyframe
	BOOL isKeyframe[ 2 ];	// index 0 is current pic, index 1 is next pic
	// these should be parsed from normal P and B slices
	// and are used in generating NC frames of those types
	unsigned long fieldPicFlag;
	unsigned long bottomFieldFlag;
	// current picture order count
	unsigned long picOrderCnt;
	// current IDR picture count
	unsigned long idrPicID;
	// delta picture order count bottom (note signed)
	long deltaPicOrderCntBottom;

	// !!! FIX ME !!! CABAC ENCODING NOT YET SUPPORTED !!!
	// CABAC encoding variables
	// BOOL cabacFirstSE;
	// char cabacM;
	// char cabacN;

	// primary picture type (0=current AU, 1=prior AU)
	RTF_VCD_PPT_H264 ppTyp[ 2 ];
	// frame number (0=current slice, 1=prior slice)
	unsigned long frNum[ 2 ];
	// picture parameter set id (0=current slice, 1=prior slice)
	unsigned long ppsID[ 2 ];
	// active and pending sequence parameter sets
	RTF_VCD_SPS_H264 sps[ H264_MAX_SPS_COUNT ];
	// active and pending picture parameter sets
	RTF_VCD_PPS_H264 pps[ H264_MAX_PPS_COUNT ];

} RTF_VCD_INFO_H264;

// H.264 Vcdec specific public functions ************************************************

// initialize the H.264 codec
void rtfVcdH264Init( P_RTF_VCD pVcd );

// process an H.264 start code
RTF_RESULT rtfVcdH264ProcessStartCode( P_RTF_VCD pVcd, unsigned long code );

// build an H264 no change frame
RTF_RESULT rtfVcdH264BuildNCFrame( P_RTF_VCD pVcd, RTF_VCD_PPT_H264 pictureType,
								   unsigned char *pBuf, unsigned long *pBufBits );

#endif // #ifndef _RTF_VCD_H264_H
