// definition file for conditional access table class
//

#ifndef _RTF_CAT_H
#define _RTF_CAT_H 1

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfCatGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfCatConstructor( RTF_CAT_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfCatDestructor( RTF_PAT_HANDLE handle );

// accessor methods *********************************************************************

// get some info on the conditional access table
RTF_RESULT rtfCatGetInfo( RTF_CAT_HANDLE handle, unsigned char *pVersionNumber,
						  unsigned char *pCurrentNext, unsigned char *pDescriptorCount,
						  unsigned char *pActiveIndex );

// set the active CA descriptor index
RTF_RESULT rtfCatSetActiveDescriptorIndex( RTF_CAT_HANDLE handle, unsigned char activeIndex );

// get the active CA descriptor index
RTF_RESULT rtfCatGetActiveDescriptorIndex( RTF_CAT_HANDLE handle, unsigned char *pActiveIndex );

// get some info on the active descriptor in the conditional access table
RTF_RESULT rtfCatGetActiveDescriptorInfo( RTF_CAT_HANDLE handle,
										  unsigned char  *pTag, unsigned short *pSid,
										  unsigned short *pPid, unsigned char  *pDatLen );

// get some info on one of the descriptors in the conditional access table
RTF_RESULT rtfCatGetDescriptorInfo( RTF_CAT_HANDLE handle, int index,
									unsigned char  *pTag, unsigned short *pSid,
									unsigned short *pPid, unsigned char  *pDatLen );

// retrieve the private data from one of the descriptors in the conditional access table
// note: *pDatLen gives buffer size on entry, data length on return
RTF_RESULT rtfCatGetDescriptorData( RTF_CAT_HANDLE handle, int index,
								    unsigned char *pDatLen, unsigned char *pDat );

// retrieve a pointer to the recorded CAT packet (returns NULL if not yet captured)
RTF_RESULT rtfCatGetTable( RTF_CAT_HANDLE handle, unsigned char **ppTable );

// service methods **********************************************************************

// reset a CAT object
RTF_RESULT rtfCatReset( RTF_CAT_HANDLE handle );

// validate a CAT object
RTF_RESULT rtfCatValidate( RTF_CAT_HANDLE handle, BOOL *pIsValid );

// parse a CAT
// NOTE: this version assumes that all CATs are a single packet
RTF_RESULT rtfCatParse( RTF_CAT_HANDLE handle, RTF_PKT_HANDLE hPacket );

// check a CAT packet to see if the table has changed
RTF_RESULT rtfCatCheckChange( RTF_CAT_HANDLE handle, RTF_PKT_HANDLE hPkt );

#endif // #ifndef _RTF_CAT_H
