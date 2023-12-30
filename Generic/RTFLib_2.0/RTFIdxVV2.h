// definition file for rtfIdx class VV2 sub-class
//

#ifndef _RTF_IDX_VV2_H
#define _RTF_IDX_VV2_H 1

typedef struct _RTF_INDEX_INFO_VV2
{
	// index library context structure
	VV2_INDEX_CONTEXT context;
	// array to translate session fileid to VV2 sub file number
	int fileIdToSubFileNumber[ RTF_MAX_OUTPUTCOUNT ];

} RTF_INDEX_INFO_VV2;

// function prototypes ******************************************************************

// initialize a VV2 index object
RTF_RESULT rtfIdxInitIndexVV2( P_RTF_IDX pIdx );

// process index info before the start of a group of pictures for a VV2 index file
RTF_RESULT rtfIdxProcessBeforeGroupVV2( P_RTF_IDX pIdx );

// process index info after the end of a group of pictures for a VV2 index file
RTF_RESULT rtfIdxProcessAfterGroupVV2( P_RTF_IDX pIdx );

// record the location and size of a keyframe in a VV2 index file
RTF_RESULT rtfIdxRecordKeyframeVV2( P_RTF_IDX pIdx, int filterNumber,
								    unsigned long firstPacket, unsigned long lastPacket );

// record an out point in a VV2 index file
RTF_RESULT rtfIdxRecordOutpointVV2( P_RTF_IDX pIdx, unsigned long packetNumber );

// prepare a splice point for a VV2 index file
RTF_RESULT rtfIdxPrepareSplicePointVV2( P_RTF_IDX pIdx, unsigned long packetNumber );

// finalize a VV2 index file
RTF_RESULT rtfIdxFinalizeVV2( P_RTF_IDX pIdx, INT64 totalInputByteCount );

#endif // #ifndef _RTF_IDX_VV2_H

