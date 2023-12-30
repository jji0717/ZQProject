// definition file for audio codec class
//

#ifndef _RTF_ACD_H
#define _RTF_ACD_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfAcdGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfAcdConstructor( RTF_ACD_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfAcdDestructor( RTF_ACD_HANDLE handle );

// accessor methods *********************************************************************

// get a silence frame (used during splicing operations)
RTF_RESULT rtfAcdGetSilenceFrame( RTF_ACD_HANDLE handle, unsigned char **ppData, unsigned long *pBytes );

// service methods **********************************************************************

// open an audio codec object for use with a particular audio stream
RTF_RESULT rtfAcdOpen( RTF_ACD_HANDLE handle, RTF_AUDIO_SPEC *pAudioSpec );

// reset a codec object
RTF_RESULT rtfAcdReset( RTF_ACD_HANDLE handle );

#endif // #ifndef _RTF_ACD_H
