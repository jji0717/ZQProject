// definition file for GOP class
//

#ifndef _RTF_GOP_H
#define _RTF_GOP_H 1

// constants ****************************************************************************

// these masks are used to isolate single flags from the "flags" member variable
// group description flags
#define RTF_GOP_ISVIRTUAL		0x00000001
// group state flags
#define RTF_GOP_ISDAMAGED		0x80000000

// typedefs *****************************************************************************

typedef enum _RTF_GOPSTATE
{
	RTF_GOPSTATE_INVALID = 0,

	RTF_GOPSTATE_OPEN,
	RTF_GOPSTATE_CLOSED,
	RTF_GOPSTATE_RELEASED

} RTF_GOPSTATE;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfGopGetStorageRequirement();

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfGopConstructor( RTF_GOP_HANDLE *pHandle, RTF_HANDLE hParent );

// destructor
RTF_RESULT rtfGopDestructor( RTF_GOP_HANDLE handle );

// accessor methods *********************************************************************

// get the number of the GOP
RTF_RESULT rtfGopGetNumber( RTF_GOP_HANDLE handle, unsigned long *pGopNumber );

// get the state of the GOP
RTF_RESULT rtfGopGetState( RTF_GOP_HANDLE handle, RTF_GOPSTATE *pState );

// get this group's virtual flag
RTF_RESULT rtfGopGetIsVirtual( RTF_GOP_HANDLE handle, BOOL *pIsVirtual );

// set this group's damage flag
RTF_RESULT rtfGopSetIsDamaged( RTF_GOP_HANDLE handle, BOOL isDamaged );

// get this group's damage flag
RTF_RESULT rtfGopGetIsDamaged( RTF_GOP_HANDLE handle, BOOL *pIsDamaged );

// set the starting location of a GOP object
RTF_RESULT rtfGopSetStart( RTF_GOP_HANDLE handle, RTF_PKT_HANDLE hFirstBytePacket, unsigned char firstBytePacketOffset );

// get the starting location of a GOP object
RTF_RESULT rtfGopGetStart( RTF_GOP_HANDLE handle, RTF_PKT_HANDLE *phFirstBytePacket, unsigned char *pFirstBytePacketOffset );

// get the picture array info from a GOP object
RTF_RESULT rtfGopGetPicArrayInfo( RTF_GOP_HANDLE handle, unsigned char *pPicCount, RTF_PIC_HANDLE **pphPic );

// get the number of the first packet in the GOP object
RTF_RESULT rtfGopGetFirstPktNum( RTF_GOP_HANDLE handle, unsigned long *pPktNumber );

// get the number of the last packet in the GOP object
RTF_RESULT rtfGopGetLastPktNum( RTF_GOP_HANDLE handle, unsigned long *pPktNumber );

// service methods **********************************************************************

// reset a GOP object
RTF_RESULT rtfGopReset( RTF_GOP_HANDLE handle );

// open a GOP object
RTF_RESULT rtfGopOpen( RTF_GOP_HANDLE handle, unsigned long gopNumber, BOOL isVirtual );

// add a picture object to a GOP object
RTF_RESULT rtfGopAddPic( RTF_GOP_HANDLE handle, RTF_PIC_HANDLE hPic );

// close a GOP object
RTF_RESULT rtfGopClose( RTF_GOP_HANDLE handle );

// release a GOP object
RTF_RESULT rtfGopRelease( RTF_GOP_HANDLE handle, RTF_SES_HANDLE hSes );

#endif // #ifndef _RTF_GOP_H
