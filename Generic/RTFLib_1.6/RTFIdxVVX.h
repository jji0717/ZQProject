// definition file for rtfIdx class VVX sub-class
//

#ifndef _RTF_IDX_VVX_H
#define _RTF_IDX_VVX_H 1

// typedefs *****************************************************************************

// VVX index context structure
typedef struct _RTF_INDEX_INFO_VVX
{
	// index library context structure
	VVX_INDEX_CONTEXT context;
	// time code at start of current interval
	TIME_CODE timeCode;
	// table to translate output numbers to record index
	int outputNumberToRecordOrder[ RTF_MAX_OUTPUTCOUNT ];
	// array of byte offsets prior to processing keyframes in each of the trick file filters
	INT64 startTable[ RTF_INDEX_INDEXTABLE_ENTRIES ];
	// array of byte offsets of particular keyframes in each of the trick file filters
	INT64 indexTable[ RTF_INDEX_INDEXTABLE_ENTRIES ];
	// array of sizes in bytes of each of the trick files
	// (also source file, in place of index file entry)
	INT64 sizeTable[ RTF_INDEX_SIZETABLE_ENTRIES ];

} RTF_INDEX_INFO_VVX;

// VVX public function prototypes *******************************************************

// initialize a VVX index
RTF_RESULT rtfIdxInitIndexVVX( P_RTF_IDX pIdx );

// process index info before the start of a group of pictures for a VVX index file
RTF_RESULT rtfIdxProcessBeforeGroupVVX( P_RTF_IDX pIdx );

// process index info after the end of a group of pictures for a VVX index file
RTF_RESULT rtfIdxProcessAfterGroupVVX( P_RTF_IDX pIdx );

// prepare a splice point for a VVX index file
RTF_RESULT rtfIdxPrepareSplicePointVVX( P_RTF_IDX pIdx, unsigned long packetNumber );

// perform a final update of a VVX index file
RTF_RESULT rtfIdxFinalizeVVX( P_RTF_IDX pIdx, INT64 totalInputByteCount );

#endif // #ifndef _RTF_IDX_VVX_H
