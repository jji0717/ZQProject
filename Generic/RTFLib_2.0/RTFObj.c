// implementation file for rtfObj class
// base class from which other RTF classes are derived
//

#include "RTFPrv.h"

// typedefs *****************************************************************************

typedef struct _RTF_OBJ
{
	RTF_OBJ_TYPE type;			// type of the object that owns this base object
	RTF_HANDLE hOwner;			// handle of the object that owns this base object
	RTF_HANDLE hParent;			// handle of the parent of the owner
} RTF_OBJ;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to the constructor below
unsigned long rtfObjGetStorageRequirement()
{
	// !!! IMPORTANT NOTE !!! NEED TO KEEP SYNCHRONIZED WITH CONSTRUCTOR BELOW !!!
	RTF_FNAME( "rtfObjGetStorageRequirement" );
	unsigned long bytes;

	bytes = sizeof(RTF_OBJ);

	// RTF_CHK_REQ;

	return bytes;
}

// constructor / destructor *************************************************************

RTF_RESULT rtfObjConstructor( RTF_OBJ_TYPE type, RTF_HANDLE hOwner, RTF_HANDLE hParent, RTF_OBJ_HANDLE *pHandle )
{
	RTF_FNAME( "rtfObjConstructor" );
	RTF_OBASE( hParent );
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ *pObj;

	do			// error escape wrapper - begin
	{

		// allocate a state structure for the resource object
		pObj = (RTF_OBJ *)rtfAlloc( sizeof(RTF_OBJ) );
		RTF_CHK_ALLOC( pObj );
		// return the handle
		*pHandle = (RTF_OBJ_HANDLE)pObj;
		// clear the state structure
		memset( (void *)pObj, 0, sizeof(*pObj) );
		// record the owning object type
		pObj->type = type;
		// record the handle of the owning object and its parent
		pObj->hOwner = hOwner;
		pObj->hParent = hParent;

	} while( 0 ); // error escape wrapper - end

	// RTF_CHK_SIZE;

	return result;
}

// destructor
RTF_RESULT rtfObjDestructor( RTF_OBJ_HANDLE handle, RTF_OBJ_TYPE type )
{
	RTF_FNAME( "rtfObjDestructor" );
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ *pObj = (RTF_OBJ *)handle;
	RTF_OBASE( pObj->hOwner );

	do			// error escape wrapper - begin
	{
		if( pObj->type != type )
		{
			RTF_LOG_ERR0( RTF_MSG_ERR_BADTYPE, "Incorrect object type" );
			break;
		}
		// make type invalid to guard against late accesses
		pObj->type = RTF_OBJ_TYPE_INVALID;
		rtfFree( handle );

	} while( 0 ); // error escape wrapper - end

	return result;
}

// service methods **********************************************************************

// get the handle of the session that is the ancestor of this object
RTF_RESULT rtfObjGetSession( RTF_OBJ_HANDLE handle, RTF_SES_HANDLE *phSession )
{
	RTF_FNAME( "rtfObjGetSession" );
	RTF_OBASE( handle );
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ *pObj = *(RTF_OBJ **)handle;

	// is the object that owns this base object a session?
	if( pObj->type == RTF_OBJ_TYPE_SES )
	{
		// yes - return the handle of the session object
		*phSession = (RTF_SES_HANDLE)pObj->hOwner;
	}
	// is the object that owns this base object a system?
	else if( pObj->type == RTF_OBJ_TYPE_SYS )
	{
		// yes - return a NULL
		*phSession = (RTF_SES_HANDLE)NULL;
	}
	else
	{
		// get the session handle
		result = rtfObjGetSession( pObj->hParent, phSession );
	}

	return result;
}

// get the type of an object
RTF_RESULT rtfObjGetType( RTF_OBJ_HANDLE handle, RTF_OBJ_TYPE *pType )
{
	RTF_FNAME( "rtfObjGetType" );
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ *pObj = (RTF_OBJ *)handle;
	RTF_OBASE( pObj->hOwner );

	// make the return
	*pType = pObj->type;

	return result;
}

// get the type of an object as a string
RTF_RESULT rtfObjGetTypeStr( RTF_OBJ_HANDLE handle, char **ppTypeStr )
{
	RTF_FNAME( "rtfObjGetType" );
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ *pObj = (RTF_OBJ *)handle;
	RTF_OBASE( pObj->hOwner );

	// make the return
	switch( pObj->type )
	{
	case RTF_OBJ_TYPE_INVALID:
		*ppTypeStr = "invalid";
		break;
	case RTF_OBJ_TYPE_ACD:
		*ppTypeStr = "audio CODEC";
		break;
	case RTF_OBJ_TYPE_BUF:
		*ppTypeStr = "buffer";
		break;
	case RTF_OBJ_TYPE_CAS:
		*ppTypeStr = "CA system";
		break;
	case RTF_OBJ_TYPE_CAT:
		*ppTypeStr = "CAT";
		break;
	case RTF_OBJ_TYPE_FLT:
		*ppTypeStr = "filter";
		break;
	case RTF_OBJ_TYPE_GOP:
		*ppTypeStr = "group";
		break;
	case RTF_OBJ_TYPE_IDX:
		*ppTypeStr = "indexer";
		break;
	case RTF_OBJ_TYPE_OUT:
		*ppTypeStr = "output";
		break;
	case RTF_OBJ_TYPE_PAT:
		*ppTypeStr = "PAT";
		break;
	case RTF_OBJ_TYPE_PES:
		*ppTypeStr = "PES hdr";
		break;
	case RTF_OBJ_TYPE_PIC:
		*ppTypeStr = "picture";
		break;
	case RTF_OBJ_TYPE_PKT:
		*ppTypeStr = "packet";
		break;
	case RTF_OBJ_TYPE_PMT:
		*ppTypeStr = "PMT";
		break;
	case RTF_OBJ_TYPE_SEQ:
		*ppTypeStr = "sequence";
		break;
	case RTF_OBJ_TYPE_SES:
		*ppTypeStr = "session";
		break;
	case RTF_OBJ_TYPE_SYS:
		*ppTypeStr = "system";
		break;
	case RTF_OBJ_TYPE_VCD:
		*ppTypeStr = "video CODEC";
		break;
	case RTF_OBJ_TYPE_WIN:
		*ppTypeStr = "window";
		break;
	default:
		*ppTypeStr = "unrecognized";
	}

	return result;
}

// verify the type of an object
RTF_RESULT rtfObjCheckType( RTF_OBJ_HANDLE handle, RTF_OBJ_TYPE type )
{
	RTF_FNAME( "rtfObjCheckType" );
	RTF_RESULT result = RTF_PASS;
	RTF_OBJ *pObj = (RTF_OBJ *)handle;
	RTF_OBASE( pObj->hOwner );

	if( pObj->type != type )
	{
		RTF_LOG_ERR2( RTF_MSG_ERR_BADTYPE, "Bad object type (expected %d got %d)", type, pObj->type );
	}

	return result;
}
