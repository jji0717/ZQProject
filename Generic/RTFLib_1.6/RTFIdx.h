// definition file for index file generator class
//

#ifndef _RTF_IDX_H
#define _RTF_IDX_H 1

// typedefs *****************************************************************************
\
typedef enum _RTF_IDX_STATE
{
	RTF_IDX_STATE_INVALID = 0,

	RTF_IDX_STATE_CLOSED,
	RTF_IDX_STATE_OPEN

} RTF_IDX_STATE;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfIdxGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfIdxConstructor( RTF_IDX_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfIdxDestructor( RTF_IDX_HANDLE handle );

// accessor methods *********************************************************************

// get the state of an index object
RTF_RESULT rtfIdxGetState( RTF_IDX_HANDLE handle, RTF_IDX_STATE *pState );

// service methods **********************************************************************

// open an index generator for use with a particular indexing method
RTF_RESULT rtfIdxOpen( RTF_IDX_HANDLE handle, RTF_INDEX_MODE mode,
					   RTF_INDEX_TYPE indexType, RTF_INDEX_OPTION indexOption,
					   char *pInputFilename, INT64 inputBytes, int outCount,
					   int indexFileOutputNumber, int mainFileOutputNumber,
					   RTF_SES_HANDLE hSes, RTF_VCD_HANDLE hVcd,
					   RTF_PAT_HANDLE hPat, RTF_PMT_HANDLE hPmt,
					   RTF_PES_HANDLE hPes, RTF_SEQ_HANDLE hSeq,
					   RTF_OUT_HANDLE *phOut, RTF_FLT_HANDLE *phFlt );

// reset an index object
RTF_RESULT rtfIdxReset( RTF_IDX_HANDLE handle );

// set the user bit rate
// note: only called by augmentation scan - do not call once ingest has started!
RTF_RESULT rtfIdxSetUserBitRate( RTF_IDX_HANDLE handle, unsigned long bitsPerSecond );

// process index info before the start of a group of pictures
RTF_RESULT rtfIdxProcessBeforeGroup( RTF_IDX_HANDLE handle, RTF_GOP_HANDLE hGop );

// process index info after the end of a group of pictures
RTF_RESULT rtfIdxProcessAfterGroup( RTF_IDX_HANDLE handle );

// record the location and size of a keyframe
RTF_RESULT rtfIdxRecordKeyframe( RTF_IDX_HANDLE handle, int filterNumber,
								 unsigned long firstPacket, unsigned long lastPacket );

// record the location of a potential "out" point
RTF_RESULT rtfIdxRecordOutpoint( RTF_IDX_HANDLE handle, unsigned long packetNumber );

// prepare a splice point
RTF_RESULT rtfIdxPrepareSplicePoint( RTF_IDX_HANDLE handle, unsigned long packetNumber );

// perform a final update of an index file
RTF_RESULT rtfIdxFinalize( RTF_IDX_HANDLE handle, INT64 totalInputByteCount );

// remove all references to trickfiles from the index
RTF_RESULT rtfIdxAbortTrickFiles( RTF_IDX_HANDLE handle );

#endif // #ifndef _RTF_IDX_H
