// private definition file for rtfFlt class - rate controller subsystem
//

#ifndef RTF_FLT_CTL_FIX_GOP_H
#define RTF_FLT_CTL_FIX_GOP_H 1

// constants ****************************************************************************

#define PCR_STARTING_FRAME_OFFSET			(0xa8c00LL)
#define DEFAULT_BIT_RATE					3750000
#define DEFAULT_OUT_PICSPERGOP				2	// !!! FIX ME !!!
#define DEFAULT_PCR_FREQUENCY				40
#define DEFAULT_PSI_FREQUENCY				100

// typedefs *****************************************************************************

// forward declaration of pointer to filter state structure
typedef struct _RTF_FLT *P_RTF_FLT;

// rate controller state info for output filter
typedef struct _RTF_FLT_CTL_FIX_GOP
{
	// these fields are constant values ***************************************
	ULONG psiSetPkts;					// number of pkts in a PSI set
	// these fields are set when the trick spec is available ******************
	INT64 refByteOffset;				// zero for forward files, a Very Big Number for reverse
	int picsPerOutputGop;				// pictures to generate per output GOP
	ULONG interPcrPktCount;				// number of pkts to wait before inserting PCR pkt
	// these fields get set when the frame rate is established ****************
	ULONG picTicks90;					// number of 90 Khz clock ticks per main file picture
	// these fields change as the stream is processed *************************
	ULONG vbvDelay;						// current VBV delay factor
	int filteredPicPkts;				// # of pkts in the current input pic to be copied to the output
	int outGopTotalPktsToProduce;		// total # of pkts to produce for this output GOP
	int requiredOutGopContentPkts;		// number of pkts required to record GOP content
	int outPicTotalPktsToProduce;		// total # of pkts to produce for the current picture
	int requiredOutPicContentPkts;		// number of pkts required to record current picture content
	UINT64 baseGopPcr;					// each gop has a base pcr and intra-gop pcrs are calculated from it
	UINT64 expectedGopOffset;			// gop size prediction
	int lastGopSizeInPackets;			// as it says
	UCHAR saveCC[RTF_TRICK_MAX_PIDCOUNT]; // saved value of first CC of prior GOP for reverse trick files
	UCHAR tempCC[RTF_TRICK_MAX_PIDCOUNT]; // starting CC value prepared by CalculateFilteredPicturePackets
	ULONG interPSIPktCount;				// count of packets between PSI
	ULONG psiPktCount;					// count of PSI packets in the current GOP
	ULONG pesPackets;					// == count of leading pes headers packets (ie 1 or 0).
} RTF_FLT_CTL_FIX_GOP;

// output filter rate control functions *************************************************
// NOTE: may only be called from within filter module !!!

// initialization *********************
// reset the rate controller info (called from constructor)
void rtfFltCtlFixGopReset( P_RTF_FLT pFlt );
// initialize the rate controller (trick spec is available)
void rtfFltCtlFixGopOpen( P_RTF_FLT pFlt );
// set the bit rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlFixGopSetBitRate( P_RTF_FLT pFlt );
// set the frame rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlFixGopSetFrameRate( P_RTF_FLT pFlt );

// processing *************************
// process an input GOP through the rate controller
RTF_RESULT rtfFltCtlFixGopProcess( P_RTF_FLT pFlt );

// output processing ******************
// log the output of a packet with the rate controller (allows rate controller to react to PID)
void rtfFltCtlFixGopLogOutPkt( P_RTF_FLT pFlt );
// update the values of lastPTS and lastDTS for the current picture
RTF_RESULT rtfFltCtlFixGopUpdateTimestamps( P_RTF_FLT pFlt );

#endif // #ifndef RTF_FLT_CTL_FIX_GOP_H
