// definition file for rtfResource class
// base class of all 'resource" objects in rtfLibC
//

#ifndef _RTF_OBJ_H
#define _RTF_OBJ_H 1

// typedefs *****************************************************************************

typedef RTF_HANDLE RTF_OBJ_HANDLE;

typedef enum _RTF_OBJ_TYPE
{
	RTF_OBJ_TYPE_INVALID = 0,

	RTF_OBJ_TYPE_ACD,
	RTF_OBJ_TYPE_BUF,
	RTF_OBJ_TYPE_CAS,
	RTF_OBJ_TYPE_CAT,
	RTF_OBJ_TYPE_VCD,
	RTF_OBJ_TYPE_FLT,
	RTF_OBJ_TYPE_GOP,
	RTF_OBJ_TYPE_IDX,
	RTF_OBJ_TYPE_OUT,
	RTF_OBJ_TYPE_PAT,
	RTF_OBJ_TYPE_PIC,
	RTF_OBJ_TYPE_PKT,
	RTF_OBJ_TYPE_PMT,
	RTF_OBJ_TYPE_SEQ,
	RTF_OBJ_TYPE_SES,
	RTF_OBJ_TYPE_SYS,
	RTF_OBJ_TYPE_WIN,
	RTF_OBJ_TYPE_PES,

	// insert additional resource type codes here

	RTF_OBJ_TYPE_MAX		// this always goes last

} RTF_OBJ_TYPE;

// pre-constructor sizing function ******************************************************

// get the amount of storage that will be consumed by a call to rtfSysConstructor (below)
unsigned long rtfObjGetStorageRequirement();

// constructor / destructor *************************************************************

RTF_RESULT rtfObjConstructor( RTF_OBJ_TYPE type, RTF_HANDLE hOwner, RTF_HANDLE hParent,
							  RTF_OBJ_HANDLE *pHandle );

RTF_RESULT rtfObjDestructor( RTF_OBJ_HANDLE handle, RTF_OBJ_TYPE type );

// service methods **********************************************************************

// get the handle of the session that is the ancestor of this object
RTF_RESULT rtfObjGetSession( RTF_OBJ_HANDLE handle, RTF_SES_HANDLE *phSession );

// get the type of an object
RTF_RESULT rtfObjGetType( RTF_OBJ_HANDLE handle, RTF_OBJ_TYPE *pType );

// get the type of an object as a string
RTF_RESULT rtfObjGetTypeStr( RTF_OBJ_HANDLE handle, char **ppTypeStr );

// verify the type of an object
RTF_RESULT rtfObjCheckType( RTF_OBJ_HANDLE handle, RTF_OBJ_TYPE type );

#endif // ifndef _RTF_OBJ_H
