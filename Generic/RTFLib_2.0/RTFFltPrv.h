// private definition file for rtfFlt class
// Note: this info would normally be embedded in the class source file, but
// this class has sub-classes that need to know the parent class state structure
//

#ifndef RTF_FLT_PRV_H
#define RTF_FLT_PRV_H 1

// compilation switches *****************************************************************

// each filter writes a file indicating which pictures are included in the output file
#define GENERATE_TRACE_FILES			0

// constants ****************************************************************************

#define RTF_DSM_FF_BYTE					0x03	// mode=000, fieldID=00 intraSliceRefresh=0 freqTruc=11 
#define RTF_BIG_PCR_BASE				0x80000000
#define RTF_BIG_PCR_EXT					( TRANSPORT_SCR_TO_TS_RATIO - 1 )
#define RTF_BIG_FRAME_COUNT				( 12 * 60 * 60 * 30 )		// 12 hours @ 30 fps
#define RTF_NCF_FIRST_PACKET_BYTES		182		// kludge to get around splitting start code
#define RTF_MAX_FIXUP_ADDED_PKTS		16
#define RTF_MAX_TARGET_PICS				5

// typedefs *****************************************************************************

typedef enum _RTF_FLT_STATE
{
	RTF_FLT_STATE_INVALID = 0,

	RTF_FLT_STATE_CLOSED,
	RTF_FLT_STATE_OPEN,
	RTF_FLT_STATE_READY

} RTF_FLT_STATE;

typedef enum _RTF_FLT_FIXUP
{
	RTF_FLT_FIXUP_SEQHDR = 0,
	RTF_FLT_FIXUP_SQXHDR,
	RTF_FLT_FIXUP_GOPHDR,
	RTF_FLT_FIXUP_PICHDR,
	RTF_FLT_FIXUP_CODHDR,
	RTF_FLT_FIXUP_PESHDR

} RTF_FLT_FIXUP;

typedef union _RTF_FLT_CTL
{
	RTF_FLT_CTL_VAR_GOP var;
	RTF_FLT_CTL_FIX_GOP fix;

} RTF_FLT_CTL;

typedef struct _RTF_FLT
{
	RTF_OBJ_HANDLE hBaseObject;
	RTF_FLT_STATE  state;				// state of the filter
	int filterNumber;					// the number of this filter

	// embedded structures
	RTF_TRICK_SPEC trickSpec;			// trickfile fixup specification
	RTF_FLT_CTL ctl;					// rate control info

	// info on last packet image sent to output buffer
	unsigned long  outPktFlags;
	unsigned char *pOutPktStorage;
	unsigned long  outPktNumber;
	unsigned char  outPktPayloadOffset;
	unsigned char  outPktPayloadBytes;
	unsigned char  outPktPesHdrOffset;
	unsigned char  outPktPesHdrBytes;
	unsigned char  outPktFixupFlags;
	unsigned char  outPktFixupOffset[ 8 ];

	// rate control basic info
	unsigned long speedRatioFix16;		// speedNum/speedDenom (16-bit fixed point)
	unsigned long invSpeedRatioFix16;	// speedDenom/speedNum (16 bit fixed point)
	unsigned long picsPerSecFix16;		// input frames/sec (16 bit fixed point)
	unsigned long invPicsPerSecFix16;	// input sec/frames (16-bit fixed point)

	// timestamp generation info
	int frameTicks;						// number of PCR ticks for one frame
	int decodingDelay;					// ( DTS - PCR ) from PES header
	int presentationDelay;				// ( PTS - DTS ) from PES header
	int ptsCount;						// number of PTS timestamps processed
	UINT64 lastPTS;						// value of last Presentation Time Stamp (in 90KHz clock ticks)
	UINT64 lastDTS;						// value of last Decode Time Stamp (in 90KHz clock ticks)

	// misc state info
	unsigned char pesHdrLen;			// length of PES header in buffer below
	unsigned char firstBytePktOffset;	// offset of first byte of current scanning window
	unsigned char lastBytePktOffset;	// offset of last byte of current scanning window
	unsigned short lastVideoPacketIndex;// index of last video packet in packet array
	unsigned long targetBitRate;		// output bit rate target
	unsigned long forcePadBitRate;		// portion of output bit rate that must be nulls
	unsigned long packetsPerPcr;		// max number of packets between PCRs (if PCR insertion enabled)
	unsigned long lastPcrPktOutCount;	// value of pktOutCount when last PCR was output
	unsigned long accStartPktOutCount;	// value of pktOutCount when last access unit start was output
	int accTargetPacketCount;			// target packet count for current access unit as determined by bit rate
	int accTargetPictureCount;			// target picture count for current access unit as determined by bit rate and frame rate
	RTF_TIMESTAMP lastPCR;				// last PCR value written to this output
	BOOL progressiveSeq;				// true if currently working on a progressive sequence (MPEG2)
	BOOL pesCaptured;					// TRUE if PES header has been captured from input stream
	BOOL mainFileCopy;					// TRUE if this filter is producing a copy of the main file
	BOOL bitRateSet;					// TRUE if the bit rate has been set
	BOOL frameRateSet;					// TRUE if the frame rate has been set
	BOOL generateNCPF;					// TRUE if no-change P frame must be generated before use
#if GENERATE_TRACE_FILES
	int traceFile;
	unsigned long lastPictureNumber;
#endif

	// cached context
	unsigned short outPID;				// pid of last packet queued to output
	RTF_STREAM_PROFILE *pProfile;		// pointer to input stream profile
	RTF_SES_HANDLE hSes;				// handle of session that owns this filter
	RTF_IDX_HANDLE hIdx;				// indexer handle
	RTF_PAT_HANDLE hPat;				// captured PAT handle
	RTF_CAT_HANDLE hCat;				// captured CAT handle
	RTF_PMT_HANDLE hPmt;				// captured PMT handle
	RTF_CAS_HANDLE hCas;				// conditional access system handle
	RTF_OUT_HANDLE hOut;				// handle of attached output object
	RTF_BUF_HANDLE hOutBuf;				// handle of output object data buffer

	// conditional access info
	BOOL catValid;						// TRUE if a conditional access table has been parsed
	unsigned short catEcmPid;			// CA Table Active ECM PID

	// current keyframe info
	RTF_GOP_HANDLE hGop;				// handle of current GOP
	RTF_PIC_HANDLE *phPic;				// array of picture handles from the current GOP
	RTF_PKT_HANDLE *phPkt;				// pointer to packet array of picture
	unsigned short pktIndex;			// index of current packet within array
	unsigned short pktCount;			// number of active packets in array
	unsigned char  gopPicIndex;			// index of current picture in current group
	unsigned char  gopPicCount;			// number of pictures in current group
	
	// no change frame info
	unsigned char *pNCFramePicHdr;
	unsigned char  ncFramePacketCount;
	unsigned char  ncFramePesHdrPktIndex;
	unsigned char  ncFramePesHdrPktOffset;
	unsigned char  ncFrameFirstBytePktOffset;
	unsigned char  ncFrameLastBytePktOffset;
	unsigned short ncFrameTemporalRef;
	RTF_PIC_HANDLE hNCPic; // captive no-change picture (points at hNCPkt below)
	RTF_PKT_HANDLE hNCPkt[ RTF_MAX_NCFRAME_PACKETS ];		// points at ncPktData below

	// some miscellaneous large arrays
	int incPidCount; // number of PIDs in the include list below
	int excPidCount; // number of PIDs in the exclude list below
	int ccPidCount;  // number of PIDs in the CC tracking list below
	int ccVidIndex;  // index of video PID in CC array below
	unsigned short incPidList[ RTF_TRICK_MAX_PIDCOUNT ];	// known PIDs being included in trick files
	unsigned short excPidList[ RTF_TRICK_MAX_PIDCOUNT ];	// known PIDs being excluded in trick files
	unsigned short ccPidList [ RTF_TRICK_MAX_PIDCOUNT ];	// known PIDs being tracked in CC array below
	unsigned char  nextCC    [ RTF_TRICK_MAX_PIDCOUNT ];	// next CC value for each pid in CC list above
	unsigned char  tempPkt   [ TRANSPORT_PACKET_BYTES ];	// temporary packet buffer
	unsigned char  pesHdr    [ TRANSPORT_PACKET_BYTES ];	// captured PES header
	unsigned char  pesHdrPkt [ TRANSPORT_PACKET_BYTES ];	// captured PES header in a packet
	unsigned char  ncPktData [ RTF_MAX_NCFRAME_PACKETS ][ TRANSPORT_PACKET_BYTES ];

	// some required statistics
	unsigned long picInCount;			// number of pictures input to this filter
	unsigned long gopInCount;			// number of gops input to this filter
	unsigned long gopOutCount;			// number of gops output by this filter
	unsigned long picOutCount;			// number of pictures output from this filter
	unsigned long accOutCount;			// number of access units output from this filter
	unsigned long pktOutCount;			// number of packets output from this filter
	unsigned long keyStartPktCount;		// number of output packets at start of last keyframe

#ifdef DO_STATISTICS
	// input statistics
	unsigned long pktInCount;			// number of packets input to this filter
	unsigned long pcrInCount;			// number of PCRs input to this filter
	unsigned long pesInCount;			// number of PES headers input to this filter

	// output statistics
	unsigned long pesOutCount;			// number of PES headers output from this filter
	unsigned long pcrOutCount;			// number of PCRs output from this filter
	unsigned long vidOutCount;			// number of video packets output from this filter

	// insertion statistics (included in output totals above)
	unsigned long pesGenCount;			// number of PES headers inserted by this filter
	unsigned long pcrGenCount;			// number of PCRs inserted by this filter
	unsigned long padGenCount;			// number of stuffing packets inserted by this filter
	unsigned long psiGenCount;			// number of PSI sets inserted by this filter
	unsigned long vidGenCount;			// number of video fluff packets inserted by this filter
	unsigned long pktGenCount;			// number of packets inserted by this filter
	unsigned long ncfGenCount;			// number of no-change frames inserted by this filter
	unsigned long augGenCount;			// number of augmentation packets inserted by this filter
	// other statistics
	unsigned long pesModCount;			// number of PES packet headers modified
	unsigned long partialFirstCount;	// number of partial first packets processed by this filter
	unsigned long partialLastCount;		// number of partial last packets processed by this filter
#endif

	unsigned long lastPSIPktOutCount;	// value of pktOutCount when last PSI was output
	unsigned char nullPkt[ TRANSPORT_PACKET_BYTES ];
	unsigned char mbz;
} RTF_FLT;

// output filter local functions **********************************************
// NOTE: may only be called from within filter module !!!

// reset the filter structure to a clean state
void rtfResetFlt( RTF_FLT *pFlt );
// set up the lists of PIDs to be included / excluded in the trick files for this filter
RTF_RESULT rtfFltSetPidLists( RTF_FLT *pFlt );
// return the index of a particular PID on the include PID list - -1 if it is not there
void rtfFltFindIncPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex );
// return the index of a particular PID on the exclude PID list - -1 if it is not there
void rtfFltFindExcPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex );
// return the index of a particular PID on the CC tracking list - -1 if it is not there
void rtfFltFindCCPID( RTF_FLT *pFlt, unsigned short pid, int *pIndex );
// get the current output bitstream PCR time according to position
void rtfFltComputeOutputPcr( RTF_FLT *pFlt, RTF_TIMESTAMP *pPcr );
// generate a TTS timestamp based on current bitstream position
RTF_RESULT rtfFltGenerateTTSPrefix( RTF_FLT *pFlt );
// reset the next CC for a particular PID
RTF_RESULT rtfFltResetNextCC( RTF_FLT *pFlt, unsigned short pid );
// find the next CC for a particular PID; return it and then increment it
RTF_RESULT rtfFltFindNextCC( RTF_FLT *pFlt, unsigned short pid, unsigned char *pCC, unsigned long flags );
// send a packet's worth of data to the output
RTF_RESULT rtfFltQueueData( RTF_FLT *pFlt, unsigned char *pData );
// insert a stuffing packet
RTF_RESULT rtfFltInsertStuffingPacket( RTF_FLT *pFlt );
// insert an augmentation packet
RTF_RESULT rtfFltInsertAugmentationPacket( RTF_FLT *pFlt );
// Split the payload of the last packet sent to the output into 2 packets
RTF_RESULT rtfFltSplitLastOutPayload( RTF_FLT *pFlt, unsigned char byteCount );
// adjust an output packet to include only the trailing data from the indicated input packet
RTF_RESULT rtfFltAdjustPartialFirstPacket( RTF_FLT *pFlt );
// adjust an output packet to include only the leading data from the indicated input packet
RTF_RESULT rtfFltAdjustPartialLastPacket( RTF_FLT *pFlt );
// list all pending MPEG-2 specific fixups
RTF_RESULT rtfListMpeg2Fixups( RTF_FLT *pFlt );
// list all pending H264 specific fixups
RTF_RESULT rtfListH264Fixups( RTF_FLT *pFlt );
// list all pending VC1 specific fixups
RTF_RESULT rtfListVc1Fixups( RTF_FLT *pFlt );
// list all of the fixups required in the payload of an input packet
RTF_RESULT rtfFltListPayloadFixups( RTF_FLT *pFlt );
// do fixups on a PCR from the input stream that is being included in the filtered output
RTF_RESULT rtfFltFixPCR( RTF_FLT *pFlt );
// perform fixups on the packet header of the current output packet
RTF_RESULT rtfFltFixPacketHeader( RTF_FLT *pFlt );
// queue an input packet to the output
RTF_RESULT rtfFltQueuePacket( RTF_FLT *pFlt );
// queue then next video packet of a picture to the output
RTF_RESULT rtfFltQueueVideoPacket( RTF_FLT *pFlt );
// fix a byte in the output buffer
RTF_RESULT rtfFltFixByte( RTF_FLT *pFlt, unsigned char andValue, unsigned char orValue, unsigned char *pOffset );
// preform sequence header fixups
RTF_RESULT rtfFltFixSeqHdr( RTF_FLT *pFlt, unsigned char seqHdrOffset );
// preform sequence extension header fixups
RTF_RESULT rtfFltFixSqxHdr( RTF_FLT *pFlt, unsigned char sqxHdrOffset );
// preform GOP header fixups
RTF_RESULT rtfFltFixGopHdr( RTF_FLT *pFlt, unsigned char gopHdrOffset );
// preform picture header fixups
RTF_RESULT rtfFltFixPicHdr( RTF_FLT *pFlt, unsigned char picHdrOffset );
// perform picture coding header fixups
RTF_RESULT rtfFltFixCodHdr( RTF_FLT *pFlt, unsigned char codHdrOffset );
// check for stuffing bytes in an adaptation field, if any
int rtfGetAdaptStuffingBytes( unsigned char *pPkt );
// compute the number of stuffing bytes in a PES packet header
int rtfGetPesHeaderStuffingBytes( unsigned char *pHdr );
// apply fixups to a PES header
RTF_RESULT rtfFltApplyPesHdrFixups( RTF_FLT *pFlt, unsigned char *pPkt, unsigned char *pHdr, BOOL *pModFlag );
// suppress a PES packet header
void rtfFltSuppressPesHdr( RTF_FLT *pFlt, unsigned char *pPkt, unsigned char payloadOffset,
						   unsigned char pesHdrOffset, unsigned char pesHdrBytes );
// perform PES packet header fixups
RTF_RESULT rtfFltFixPesHdr( RTF_FLT *pFlt, unsigned char pesHdrOffset );
// perform all payload fixups in order of increasing packet offset
RTF_RESULT rtfFltDoPayloadFixups( RTF_FLT *pFlt );
// perform a PID remap operation on the output image of a packet, if necessary
RTF_RESULT rtfFltRemapOutPacket( RTF_FLT *pFlt );
// send a packet object to the output; perform fixups there
RTF_RESULT rtfFltProcessPacket( RTF_FLT *pFlt );
// parse the critical field lengths out of a packet
void rtfGetPacketLengths( unsigned char *pStorage, 
						  unsigned char *pHdrLen, unsigned char *pActLen,
						  unsigned char *pPadLen, unsigned char *pPayLen );
// attempt to insert a field into the last output packet
RTF_RESULT rtfFltLastPacketInsert( RTF_FLT *pFlt, unsigned short insPid,
								   unsigned char insLen, unsigned char *pDat,
								   unsigned short newFlags, BOOL isPayload,
								   unsigned char **ppPkt, unsigned char **ppHdr,
								   BOOL *pSuccess );
// insert a PES header
RTF_RESULT rtfFltInsertPesHeader( RTF_FLT *pFlt );
// insert a PSI set
RTF_RESULT rtfFltInsertPSI( RTF_FLT *pFlt );
// insert a PCR in the output stream
RTF_RESULT rtfFltInsertPcr( RTF_FLT *pFlt );
// insert a video "fluff" packet
RTF_RESULT rtfInsertVideoFluff( RTF_FLT *pFlt );
// prepare the no-change video frame
RTF_RESULT rtfFltPrepareNCFrame( RTF_FLT *pFlt );
// insert a no-change frame
RTF_RESULT rtfFltInsertNCFrame( RTF_FLT *pFlt );
// perform the fixups required by the packets of the current picture
RTF_RESULT rtfFltFixPktArray( RTF_FLT *pFlt );
// set up the input bit rate info
RTF_RESULT rtfFltSetInputBitRate( RTF_FLT *pFlt );
// set up the PES header info
RTF_RESULT rtfFltSetPES( RTF_FLT *pFlt );

#endif // #ifndef RTF_FLT_PRV_H
