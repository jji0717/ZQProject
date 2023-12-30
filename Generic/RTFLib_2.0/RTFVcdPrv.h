// private definition file for rtfVcd class
// Note: this info would normally be embedded in the class source file, but
// this class has sub-classes that need to know the parent class state structure
//

#ifndef RTF_VCD_PRV_H
#define RTF_VCD_PRV_H 1

// forward reference pointer to parent class state structure ****************************

typedef struct _RTF_VCD *P_RTF_VCD;

// codec specific definition files ******************************************************

#ifdef DO_VCD_MPEG2
#include "RTFVcdMPEG2.h"
#endif

#ifdef DO_VCD_H264
#include "RTFVcdH264.h"
#endif

#ifdef DO_VCD_VC1
#include "RTFVcdVC1.h"
#endif

// typedefs *****************************************************************************

// VCD state enumerator
typedef enum _RTF_VCDSTATE
{
	RTF_VCDSTATE_INVALID = 0,

	RTF_VCDSTATE_CLOSED,
	RTF_VCDSTATE_OPEN,

} RTF_VCDSTATE;

// VCD specific info union
typedef union _RTF_VCD_INFO
{
#ifdef DO_VCD_MPEG2
	RTF_VCD_INFO_MPEG2 mpeg2;
#endif
#ifdef DO_VCD_H264
	RTF_VCD_INFO_H264  h264;
#endif
#ifdef DO_VCD_VC1
	RTF_VCD_INFO_VC1   vc1;
#endif
} RTF_VCD_INFO;

// VCD class state structure
typedef struct _RTF_VCD
{
	RTF_OBJ_HANDLE hBaseObject;
	RTF_VCDSTATE state;
	// session info
	RTF_SES_HANDLE hSes;
	RTF_WIN_HANDLE hWin;
	RTF_SEQ_HANDLE hSeq;
	// "exploration" window object
	RTF_WIN_HANDLE hWinTmp;
	// pointer to video stream specification
	RTF_ESTREAM_SPEC *pVidSpec;
	// sequence boundary option
	RTF_SEQBOUND_OPTION seqBoundOption;
	// location of first start code after the last picture element encountered
	// (i.e. earliest possible sequence start)
	RTF_PKT_HANDLE hSeqStartPkt;
	unsigned char seqStartPktOff;
	// location of byte before first start code after the last picture element encountered
	// (i.e. earliest possible picture end)
	RTF_PKT_HANDLE hPicEndPkt;
	unsigned char picEndPktOff;
	// no change frames
	BOOL insertDSM;
	BOOL noChangePFrameValid;
	BOOL noChangeBFrameValid;
	unsigned long noChangePFrameBytes;
	unsigned long noChangeBFrameBytes;
	unsigned char noChangePFrame[ RTF_MAX_NCFRAME_BYTES ];
	unsigned char noChangeBFrame[ RTF_MAX_NCFRAME_BYTES ];
	// video codec specific info
	RTF_VCD_INFO vcdInfo;

#ifdef DO_STATISTICS
	// total number of keyframes processed by this codec
	unsigned long totalKeyframeCount;
#endif

} RTF_VCD;

// macros *********************************************************************

#define RTF_VCD_RESET_BOUNDARY pVcd->hSeqStartPkt=(RTF_PKT_HANDLE)NULL

#define RTF_VCD_UPDATE_BOUNDARY \
	if(pVcd->hSeqStartPkt==NULL) { \
		result=rtfWinGetFirstByteInfo( pVcd->hWin, &pVcd->hSeqStartPkt, &pVcd->seqStartPktOff ); \
		RTF_CHK_RESULT; \
		result = rtfWinGetPriorByteInfo( pVcd->hWin, &pVcd->hPicEndPkt, &pVcd->picEndPktOff ); \
		RTF_CHK_RESULT; }

#endif // #ifndef RTF_VCD_PRV_H
