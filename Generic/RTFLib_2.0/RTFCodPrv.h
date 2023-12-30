// private definition file for rtfCod class
// Note: this info would normally be embedded in the class source file
// but the codec class is unique in that has codec-specific sub-classes
// that need to know the parent class state structure
//

#ifndef RTF_COD_PRV_H
#define RTF_COD_PRV_H 1

// codec specific definition files ******************************************************

#ifdef DO_CODEC_MPEG2
#include "RTFCodMPEG2.h"
#endif

#ifdef DO_CODEC_H264
#include "RTFCodH264.h"
#endif

#ifdef DO_CODEC_VC1
#include "RTFCodVC1.h"
#endif

// typedefs *****************************************************************************

// CODEC state enumerator
typedef enum _RTF_CODSTATE
{
	RTF_CODSTATE_INVALID = 0,

	RTF_CODSTATE_CLOSED,
	RTF_CODSTATE_OPEN,

} RTF_CODSTATE;

// CODEC specific info union
typedef union _RTF_CODEC_INFO
{
#ifdef DO_CODEC_MPEG2
	RTF_CODEC_INFO_MPEG2 mpeg2;
#endif
#ifdef DO_CODEC_H264
	RTF_CODEC_INFO_H264  h264;
#endif
#ifdef DO_CODEC_VC1
	RTF_CODEC_INFO_VC1   vc1;
#endif
} RTF_CODEC_INFO;

// CODEC class state structure
typedef struct _RTF_COD
{
	RTF_OBJ_HANDLE hBaseObject;
	RTF_CODSTATE state;
	// session info
	RTF_SES_HANDLE hSes;
	RTF_WIN_HANDLE hWin;
	RTF_SEQ_HANDLE hSeq;
	// "exploration" window object
	RTF_WIN_HANDLE hWinTmp;
	// video info
	unsigned long bitRate;
	unsigned short vidPid;
	unsigned short width;
	unsigned short height;
	unsigned char frameRateCode;
	// codec type
	RTF_VIDEO_CODEC_TYPE codecType;
	// no change frames
	BOOL noChangePFrameValid;
	BOOL noChangeBFrameValid;
	int noChangePFrameBytes;
	int noChangeBFrameBytes;
	unsigned char noChangePFrame[ RTF_MAX_NCFRAME_BYTES ];
	unsigned char noChangeBFrame[ RTF_MAX_NCFRAME_BYTES ];

	// codec specific info
	RTF_CODEC_INFO codecInfo;

#ifdef DO_STATISTICS
	// total number of keyframes processed by this codec
	unsigned long totalKeyframeCount;
#endif

} RTF_COD;

#endif // #ifndef RTF_COD_PRV_H
