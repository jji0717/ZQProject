// private definition file for rtfIdx class
// Note: this info would normally be embedded in the class source file
// but the codec class is unique in that has codec-specific sub-classes
// that need to know the parent class state structure
//

#ifndef RTF_IDX_PRV_H
#define RTF_IDX_PRV_H 1

// forward reference pointer to parent class state structure ****************************

typedef struct _RTF_IDX *P_RTF_IDX;

// sub-class definition files ***********************************************************

#ifdef DO_INDEX_VVX
#include "RTFIdxVVX.h"
#endif

#ifdef DO_INDEX_VV2
#include "RTFIdxVV2.h"
#endif

// constants ****************************************************************************

#define RTF_IDX_ADVANCE_PKTS		32
#define RTF_IDX_ADVANCE_BYTES		( RTF_IDX_ADVANCE_PKTS * TRANSPORT_PACKET_BYTES )
#define RTF_IDX_MAX_PICS			256

// typedefs *****************************************************************************

typedef union _RTF_INDEX_INFO
{
#ifdef DO_INDEX_VVX
	RTF_INDEX_INFO_VVX vvx;
#endif
#ifdef DO_INDEX_VV2
	RTF_INDEX_INFO_VV2 vv2;
#endif

} RTF_INDEX_INFO;

typedef struct _RTF_SPEED_INFO
{
	int outNumber;
	int ratioFix8;
	int direction;
	int numerator;
	int denominator;
	char ext[ RTF_MAX_EXT_LEN ];

} RTF_SPEED_INFO;

typedef struct _RTF_IDX_PIC
{
	int subfilenum;		// ID of subfile in which picture resides
	int	firstPktCount;	// output packet count at start of indexable picture
	int	lastPktCount;	// packet count at end of indexable picture
	unsigned long npt;	// NPT of picture

} RTF_IDX_PIC;

typedef struct _RTF_IDX
{
	RTF_OBJ_HANDLE hBaseObject;

	// index state
	RTF_IDX_STATE state;
	// index mode
	RTF_INDEX_MODE mode;
	// number of index points recorded
	int indexPointCount;
	// number of out points recorded
	int outPointCount;
	// number of splice points prepared
	int splicePointCount;

	// info from the session, provided when index is opened
	char *pProgramVersionStr;
	char *pInputFilename;
	int indexFileOutputNumber;
	int mainFileOutputNumber;
	int outCount;

	// cached handles, obtained from the session object
	RTF_SES_HANDLE hSes;
    RTF_PAT_HANDLE hPat;
	RTF_PMT_HANDLE hPmt;
	RTF_PES_HANDLE hPes;
	RTF_VCD_HANDLE hVcd;
	RTF_SEQ_HANDLE hSeq;
	RTF_GOP_HANDLE hGop;
	RTF_WIN_HANDLE hWin;
	RTF_OUT_HANDLE *phOut;
	RTF_FLT_HANDLE *phFlt;

	// pointer to input stream profile - also obtained from session object
	RTF_STREAM_PROFILE *pProfile;

	// this trickfile may be used to determine when to create an index point
	int slowFileIndex;

	// forward / reverse trickfile speed info structs
	// (ordered - higher index is faster speed)
	int ffSpeedCount;
	RTF_SPEED_INFO ffSpeedInfo[ RTF_MAX_TRICKSPEEDS ];
	int frSpeedCount;
	RTF_SPEED_INFO frSpeedInfo[ RTF_MAX_TRICKSPEEDS ];

	// map output numbers to trick speed ratios
	int outRatioFix8[ RTF_MAX_OUTPUTCOUNT ];

	// 8-bit fixed point representation of frame rate
	unsigned long fix8FPS;
	// 16-bit fixed point representation of inverse frame rate
	unsigned long fix16SPF;
	// output bit rate
	unsigned long bitsPerSecond;

	// indexable picture list (indexable pictures from all media files are
	// recorded here during an index interval, prior to writing the index entry
	int picCount;
	int outPicCount[ RTF_MAX_OUTPUTCOUNT ];
	RTF_IDX_PIC pic[ RTF_IDX_MAX_PICS ];

	// index-method specific info
	RTF_INDEX_TYPE indexType;
	RTF_INDEX_OPTION indexOption;
	RTF_INDEX_INFO indexInfo;
	RTF_SEQBOUND_OPTION seqBoundOption;

} RTF_IDX;

#endif // #ifndef RTF_IDX_PRV_H
