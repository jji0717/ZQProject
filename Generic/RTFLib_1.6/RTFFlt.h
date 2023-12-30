// definition file for rtfFlt class
// encapsulates video sequence to trickfile filter
//

#ifndef _RTF_FLT_H
#define _RTF_FLT_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfFltGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfFltConstructor( RTF_FLT_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfFltDestructor( RTF_FLT_HANDLE handle );

// accessor methods *********************************************************************

// get the output packet count from the filter
RTF_RESULT rtfFltGetOutPktCount( RTF_FLT_HANDLE handle, unsigned long *pOutPktCount );

// get the number of indexable pictures produced by the current group
RTF_RESULT rtfFltGetIdxPicCount( RTF_FLT_HANDLE handle, int *pFltPicCount );

// get the start and ending packet numbers of an indexable picture
RTF_RESULT rtfFltGetIdxPktOutCounts( RTF_FLT_HANDLE handle, int fltPicIndex,
									 unsigned long *pFirstIdxPktOutCount,
									 unsigned long *pLastIdxPktOutCount );

// get the output packet count at the start of the last keyframe from the filter
RTF_RESULT rtfFltGetKeyStartPktCount( RTF_FLT_HANDLE handle, unsigned long *pKeyStartPktCount );

// get the speed info from the filter
RTF_RESULT rtfFltGetSpeedInfo( RTF_FLT_HANDLE handle, int *pDirection, unsigned long *pNumerator,
							   unsigned long *pDenominator, char **ppExtension );

// service methods **********************************************************************

// reset the filter to a closed, empty state
RTF_RESULT rtfFltReset( RTF_FLT_HANDLE handle );

// open the filter
RTF_RESULT rtfFltOpen( RTF_FLT_HANDLE handle, int filterNumber, RTF_OUT_HANDLE hOutput,
					   RTF_TRICK_SPEC *pSpec );

// prepare the filter for streaming
RTF_RESULT rtfFltPrepareForStreaming( RTF_FLT_HANDLE handle );

// filter an input buffer to the attached output
// NOTE: only called for optional filtered copy of main file
RTF_RESULT rtfFltBuf( RTF_FLT_HANDLE handle, RTF_BUF_HANDLE hBuf );

// filter a group of pictures to the attached output
// NOTE: only called for trick file outputs
RTF_RESULT rtfFltGop( RTF_FLT_HANDLE handle, RTF_GOP_HANDLE hGop );

// close the filter
RTF_RESULT rtfFltClose( RTF_FLT_HANDLE handle );

#ifdef _DEBUG
// log the state of the filter object
RTF_RESULT rtfFltLogState( RTF_OUT_HANDLE handle );
#endif

#endif // #ifndef _RTF_FLT_H
