// implementation file for rtfGop class
// encapsulates group of pictures
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

typedef struct _RTF_GOP
{
	RTF_OBJ_HANDLE hBaseObject;

	// state info
	unsigned long gopNumber;
	RTF_GOPSTATE state;
	unsigned long flags;

	// location of start of GOP
	RTF_PKT_HANDLE hFirstBytePacket;
	unsigned char firstBytePacketOffset;

	// pictures contributing to this GOP
	unsigned char picCount;
	RTF_PIC_HANDLE hPic[ RTF_MAX_GOP_PICS ];
	unsigned int timeCode;

} RTF_GOP;

// private functions ********************************************************************

// reset the GOP state structure
static void resetGop( RTF_GOP *pGop )
{
	// reset the codec-independent portion of the state structure
	pGop->state = RTF_GOPSTATE_RELEASED;
	pGop->flags = 0;
	pGop->hFirstBytePacket = (RTF_PKT_HANDLE)NULL;
	pGop->firstBytePacketOffset = 0;
	pGop->picCount = 0;
	RTF_CLR_STATE( pGop->hPic, sizeof(pGop->hPic) );
}

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfGopGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfGopGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_GOP);
	bytes += rtfObjGetStorageRequirement();

	RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

// constructor
RTF_RESULT rtfGopConstructor( RTF_GOP_HANDLE *pHandle, RTF_HANDLE hParent )
{
	RTF_FNAME( "rtfGopConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_GOP *pGop;

	do {		 // error escape wrapper - begin

		// allocate a state structure for the gop object
		pGop = (RTF_GOP *)rtfAlloc( sizeof(RTF_GOP) );
		RTF_CHK_ALLOC( pGop );
		// return the handle
		*pHandle = (RTF_GOP_HANDLE)pGop;
		// clear the state structure
		memset( (void *)pGop, 0, sizeof(*pGop) );
		// create an embedded resource object
		result = rtfObjConstructor( RTF_OBJ_TYPE_GOP, (RTF_HANDLE)pGop, hParent, &pGop->hBaseObject );
		RTF_CHK_RESULT;
		// reset the GOP state structure
		resetGop( pGop );

	} while( 0 ); // error escape wrapper - end

	RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfGopDestructor( RTF_GOP_HANDLE handle )
{
	RTF_FNAME( "rtfGopDestructor" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		// destroy the embedded base object
		result = rtfObjDestructor( pGop->hBaseObject, RTF_OBJ_TYPE_GOP );
		RTF_CHK_RESULT;
		// free the state structure
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// accessor methods *********************************************************************

// get the number of the GOP
RTF_RESULT rtfGopGetNumber( RTF_GOP_HANDLE handle, unsigned long *pGopNumber )
{
	RTF_FNAME( "rtfGopGetNumber" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// make the return
		*pGopNumber = pGop->gopNumber;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the state of a gop object
RTF_RESULT rtfGopGetState( RTF_GOP_HANDLE handle, RTF_GOPSTATE *pState )
{
	RTF_FNAME( "rtfGopGetState" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// make the return
		*pState = pGop->state;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get this group's virtual flag
RTF_RESULT rtfGopGetIsVirtual( RTF_GOP_HANDLE handle, BOOL *pIsVirtual )
{
	RTF_FNAME( "rtfGopGetIsVirtual" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// make the return
		*pIsVirtual = ( ( pGop->flags & RTF_GOP_ISVIRTUAL ) != 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set this group's damage flag
RTF_RESULT rtfGopSetIsDamaged( RTF_GOP_HANDLE handle, BOOL isDamaged )
{
	RTF_FNAME( "rtfGopSetIsDamaged" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// set the flag
		if( isDamaged == FALSE )
		{
			pGop->flags &= ~RTF_GOP_ISDAMAGED;
		}
		else
		{
			pGop->flags |= RTF_GOP_ISDAMAGED;
		}

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get this group's damage flag
RTF_RESULT rtfGopGetIsDamaged( RTF_GOP_HANDLE handle, BOOL *pIsDamaged )
{
	RTF_FNAME( "rtfGopGetIsDamaged" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// make the return
		*pIsDamaged = ( ( pGop->flags & RTF_GOP_ISDAMAGED ) != 0 ) ? TRUE : FALSE;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// set the starting location of a GOP object
RTF_RESULT rtfGopSetStart( RTF_GOP_HANDLE handle, RTF_PKT_HANDLE hFirstBytePacket, unsigned char firstBytePacketOffset )
{
	RTF_FNAME( "rtfGopSetStart" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// record the info
		pGop->hFirstBytePacket = hFirstBytePacket;
		pGop->firstBytePacketOffset = firstBytePacketOffset;
		// add a reference to the first packet of the GOP
		result = rtfPktAddReference( hFirstBytePacket, handle );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the starting location of a GOP object
RTF_RESULT rtfGopGetStart( RTF_GOP_HANDLE handle, RTF_PKT_HANDLE *phFirstBytePacket, unsigned char *pFirstBytePacketOffset )
{
	RTF_FNAME( "rtfGopSetStart" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// make the returns
		*phFirstBytePacket = pGop->hFirstBytePacket;
		*pFirstBytePacketOffset = pGop->firstBytePacketOffset;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the picture array info from a GOP object
RTF_RESULT rtfGopGetPicArrayInfo( RTF_GOP_HANDLE handle, unsigned char *pPicCount, RTF_PIC_HANDLE **pphPic )
{
	RTF_FNAME( "rtfGopGetPicArrayInfo" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// make the returns
		*pPicCount = pGop->picCount;
		*pphPic = pGop->hPic;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of the first packet in the GOP object
RTF_RESULT rtfGopGetFirstPktNum( RTF_GOP_HANDLE handle, unsigned long *pPktNumber )
{
	RTF_FNAME( "rtfGopGetFirstPktNum" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PIC_HANDLE hPic;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// is there at least one picture in the group?
		if( ( hPic = pGop->hPic[ 0 ] ) == (RTF_PIC_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No picture in group" );
			break;
		}
		// return the number of the first packet of the first picture in the group
		result = rtfPicGetFirstPktNum( hPic, pPktNumber );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// get the number of the last packet in the GOP object
RTF_RESULT rtfGopGetLastPktNum( RTF_GOP_HANDLE handle, unsigned long *pPktNumber )
{
	RTF_FNAME( "rtfGopGetLastPktNum" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;
	RTF_PIC_HANDLE hPic;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// is there at least one picture in the group?
		if( pGop->picCount == 0 )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "No picture in group" );
			break;
		}
		if( ( hPic = pGop->hPic[ pGop->picCount-1 ] ) == (RTF_PIC_HANDLE)NULL )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_INTERNAL, "Null picture handle in group" );
			break;
		}
		// return the number of the last packet of the last picture in the group
		result = rtfPicGetLastPktNum( hPic, pPktNumber );
		RTF_CHK_RESULT;

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// reset the access gop to a closed, empty state
RTF_RESULT rtfGopReset( RTF_GOP_HANDLE handle )
{
	RTF_FNAME( "rtfGopReset" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// reset the GOP state structure
		resetGop( pGop );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// open a new gop (parse the gop header)
RTF_RESULT rtfGopOpen( RTF_GOP_HANDLE handle, unsigned long gopNumber, BOOL isVirtual )
{
	RTF_FNAME( "rtfGopOpen" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		RTF_CHK_STATE_NE( pGop, RTF_GOPSTATE_OPEN );
		// reset the state structure
		resetGop( pGop );
		// record the group number
		pGop->gopNumber = gopNumber;
		// record virtual flag
		if( isVirtual == FALSE )
		{
			pGop->flags &= ~RTF_GOP_ISVIRTUAL;
		}
		else
		{
			pGop->flags |= RTF_GOP_ISVIRTUAL;
		}
		// set the state to open
		pGop->state = RTF_GOPSTATE_OPEN;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_GOPOPEN, "GOP %d opened", gopNumber );
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// add a picture object to a GOP object
RTF_RESULT rtfGopAddPic( RTF_GOP_HANDLE handle, RTF_PIC_HANDLE hPic )
{
	RTF_FNAME( "rtfGopAddPic" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;
	BOOL isKeyFrame;
	BOOL isDamaged;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		RTF_CHK_STATE_EQ( pGop, RTF_GOPSTATE_OPEN );
		// make sure there is room for another picture
		if( pGop->picCount >= RTF_MAX_GOP_PICS )
		{
			RTF_LOG_WARN1( RTF_MSG_WRN_PICOVERFLOW, "Picture array overflow in GOP %d", pGop->gopNumber );
			pGop->flags |= RTF_GOP_ISDAMAGED;
			break;
		}
		// is this the first picture of the group?
		if( pGop->picCount == 0 )
		{
			// yes. is this picture a keyframe?
			result = rtfPicGetIsKeyframe( hPic, &isKeyFrame );
			RTF_CHK_RESULT;
			if( isKeyFrame == FALSE )
			{
				// no. a group needs to begin with a keyframe - mark this group as damaged
				pGop->flags |= RTF_GOP_ISDAMAGED;
			}
		}
		// record the new picture; bump the picture count
		pGop->hPic[ pGop->picCount++ ] = hPic;
		// get the damage flag from the picture
		result = rtfPicGetIsDamaged( hPic, &isDamaged );
		RTF_CHK_RESULT;
		// is the picture damaged?
		if( isDamaged != FALSE )
		{
			// yes - if the picture is damaged, the group is damaged.
			pGop->flags |= RTF_GOP_ISDAMAGED;
		}
#ifdef DO_TRACKING
		{
			unsigned long picNumber;
			result = rtfPicGetNumber( hPic, &picNumber );
			RTF_CHK_RESULT;
			RTF_LOG_INFO3( RTF_MSG_INF_GOPADDPIC, "GOP %d added PIC %d (gopsize=%d)",
						   pGop->gopNumber, picNumber, (int)( pGop->picCount ) );
		}
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// close a gop object
RTF_RESULT rtfGopClose( RTF_GOP_HANDLE handle )
{
	RTF_FNAME( "rtfGopClose" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// change state to closed
		pGop->state = RTF_GOPSTATE_CLOSED;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_GOPCLOSE, "GOP %d closed", pGop->gopNumber );
#endif

	} while( 0 ); // error escape wrapper - end

	return result;
}

// release a GOP object
RTF_RESULT rtfGopRelease( RTF_GOP_HANDLE handle, RTF_SES_HANDLE hSes )
{
	RTF_FNAME( "rtfGopRelease" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;
	int i;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		// remove the reference that was added to the first packet when the GOP was opened
		result = rtfPktRemoveReference( pGop->hFirstBytePacket, handle );
		RTF_CHK_RESULT;
		// release the pictures in the group
		for( i=0; i<pGop->picCount; ++i )
		{
			result = rtfPicRelease( pGop->hPic[ i ], hSes );
			RTF_CHK_RESULT;
		}
		RTF_CHK_RESULT_LOOP;
#ifdef DO_TRACKING
		RTF_LOG_INFO1( RTF_MSG_INF_GOPRELEASE, "GOP %d released", pGop->gopNumber );
#endif
		// tell the session to release this group too
		result = rtfSesRecycleGop( hSes, handle );
		RTF_CHK_RESULT;
		// reset the GOP
		resetGop( pGop );

	} while( 0 ); // error escape wrapper - end

	return result;
}

RTF_RESULT rtfGopSetTimecode(RTF_GOP_HANDLE handle, unsigned int timeCode)
{
	RTF_FNAME( "rtfGopRelease" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		pGop->timeCode = timeCode;
	} while( 0 ); // error escape wrapper - end

	return result;
}

RTF_RESULT rtfGopGetTimecode(RTF_GOP_HANDLE handle, unsigned int *pTimeCode)
{
	RTF_FNAME( "rtfGopRelease" );
	RTF_OBASE( handle );
	RTF_GOP *pGop = (RTF_GOP *)handle;
	RTF_RESULT result = RTF_PASS;

	do {		 // error escape wrapper - begin

		RTF_CHK_OBJ( pGop, RTF_OBJ_TYPE_GOP );
		*pTimeCode = pGop->timeCode;
	} while( 0 ); // error escape wrapper - end

	return result;
}
