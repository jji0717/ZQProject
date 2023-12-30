// private definition file for rtfFlt class - rate controller subsystem
//

#ifndef RTF_FLT_CTL_VAR_GOP_H
#define RTF_FLT_CTL_VAR_GOP_H 1

// constants ****************************************************************************

#define RTF_DEFAULT_QUEUE_FRAMES_LOG2	5
#define RTF_ABSMAX_QUEUE_FRAMES_LOG2	10
#define RTF_ABSMAX_QUEUE_FRAMES			( 1<<RTF_ABSMAX_QUEUE_FRAMES_LOG2 )

// typedefs *****************************************************************************

// forward declaration of pointer to filter state structure
typedef struct _RTF_FLT *P_RTF_FLT;

// rate controller state info for output filter
typedef struct _RTF_FLT_CTL_VAR_GOP
{
	// speed control
	unsigned long tgtInFrameNumber;		// "ideal" next input frame to select
	// bit rate control queue - average over last N frames
	int maxQueueFramesLog2;
	int maxQueueFrames;
	int maxQueueFramesMask;
	unsigned long queueFrameBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueStuffBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueInputBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueKeyfrBits[ RTF_ABSMAX_QUEUE_FRAMES ];
	unsigned long queueNextFrameBits;   // number of bits in next frame to be added to queue
	unsigned long queueNextStuffBits;   // number of stuffing bits in next frame to be added to queue
	unsigned long queueNextInputBits;	// number of input bits spanned by next frame to be added to queue
	unsigned long queueNextKeyfrBits;	// number of bits in next keyframe to be added to queue
	unsigned long queueTotalOutputBits;	// sum of all entries in both queues above
	unsigned long queueTotalInputBits;	// number of input bits spanned by output frames in queue
	unsigned long queueTotalKeyfrBits;	// number of bits in all current keyframe entries
	unsigned long queueLastKeyframeBits;// number of bits in last keyframe
	unsigned long queueLastNCFrameBits;	// number of bits in last no-change frame
	unsigned long queueLastGroupPktNum; // number of last packet of last group examined

} RTF_FLT_CTL_VAR_GOP;

// output filter rate control functions *************************************************
// NOTE: may only be called from within filter module !!!

// initialization *********************
// reset the rate controller info (called from constructor)
void rtfFltCtlVarGopReset( P_RTF_FLT pFlt );
// initialize the rate controller (trick spec is available)
void rtfFltCtlVarGopOpen( P_RTF_FLT pFlt );
// set the bit rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlVarGopSetBitRate( P_RTF_FLT pFlt );
// set the frame rate (called during "sniff" phase)
RTF_RESULT rtfFltCtlVarGopSetFrameRate( P_RTF_FLT pFlt );

// processing *************************
// process an input GOP through the rate controller
RTF_RESULT rtfFltCtlVarGopProcess( P_RTF_FLT pFlt );
// log the output of a packet with the rate controller (allows rate controller to react to PID)
void rtfFltCtlVarGopLogOutPkt( P_RTF_FLT pFlt );
// update the values of lastPTS and lastDTS for the current picture
RTF_RESULT rtfFltCtlVarGopUpdateTimestamps( P_RTF_FLT pFlt );

#endif // #ifndef RTF_FLT_CTL_VAR_GOP_H
