/*
** Copyright (c) 2006 by
** zq Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
** 
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of zq  Technology  Inc.   Possession, use,
** duplication or dissemination of the software and media is authorized only
** pursuant to a valid written license from zq Technology Inc.
** 
** This software is furnished under a  license  and  may  be used and copied
** only in accordance with the terms of  such license and with the inclusion
** of the above copyright notice.  This software or any other copies thereof
** may not be provided or otherwise made available to  any other person.  No
** title to and ownership of the software is hereby transferred.
** 
** The information in this software is subject to change without  notice and
** should not be construed as a commitment by zq Technology Inc.
** 
** zq  assumes  no  responsibility  for the use or reliability of its
** software on equipment which is not supplied by zq.
** 
** RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
** Government is subject  to  restrictions  as  set  forth  in  Subparagraph
** (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
*/
/*
**
** title:       cfgpkg.h
**
** version:     T3.5
**
** facility:    Digital Ad Insertion - Configuration subsystem
**
** abstract:    This module is the include file containing definitions related to
**        the configuration package, CDCICFG.DLL.
**
** Revision History: Jan. 31, 1994 created DH
** ------------------------
**
**   Rev       Date           Who               Description
**  ------ ---------------- -------  -----------------------------------
T1.0    10-21-2006     DWH      Initial Fieldtest release 
									Bullet-proofing tasks
									Added constants for Greyhound SDVL
									Changed constants CFG_GH_CLUSTER, CFG_GH_IU and CFG_GH_MVL from 8, 9, 10 to 9, 10, 11
									Moved FM_LOCATE functions into their own header (fm_process.h)
									Merged  fix of MAXIPADDRLEN from 16->18
									added CFG_MGT definition from V2.5 version
									added CFG_ENTRY_NOT_FOUND and CFG_DLL_NOT_FOUND errors.
									DLL Declarations are now made in the .DEF
                                   file.  Changed functions here to simple
                                   prototypes.  Commented out CFG_CLUSTER() -
                                   it doesn't exist in CFGPKG???  Updated to allow
                                   imports by C++ DLLs (extern "C")
									Added CFG_FREE() prototype.
									Added CFG_STANDALONE (for LES)
									Added CFG_GH_RVL, CFG_RDB_MASTER, CFG_RDB_SLAVE,
                                   and CFG_GH_IC system types.
									Added CFG_GH_AMVL system type, removed
                                   CFG_RDB_SLAVE.
									Added CFG_GET_MGMT_PORT.

   

*/


#ifndef _CFGPKG_
#define _CFGPKG_

// modify by dony 
//#include "..\seaerrormsgs\seaerror.h"            // CFG_<...> status codes // this is the old
#include "ZQError.h"
#define CFGPKG_NO_SUCH_VARNAME   11
/* CFG status return values */
/*
 * Note: Now defined in SeaErrorMsgs/SeaError.h
 */
typedef enum _CFGSTATUS
{
    CFG_SUCCESS             = S_OK,
    CFG_FAIL                = CFGPKG_FAIL,
    CFG_SYSERR              = CFGPKG_SYSERR,
    CFG_BAD_HANDLE          = CFGPKG_BAD_HANDLE,
    CFG_BAD_OBJTYPE         = CFGPKG_BAD_OBJTYPE,
    CFG_ALREADY_INITED      = CFGPKG_ALREADY_INITED,
    CFG_BADPARAM            = CFGPKG_BADPARAM,
    CFG_DLL_NOT_FOUND       = CFGPKG_DLL_NOT_FOUND,
    CFG_ENTRY_NOT_FOUND     = CFGPKG_ENTRY_NOT_FOUND,
    CFG_ENTRY_WRONG_TYPE    = CFGPKG_ENTRY_WRONG_TYPE,
    CFG_NO_SUCH_SUBKEY      = CFGPKG_NO_SUCH_SUBKEY,
	CFG_NO_SUCH_VARNAME     = CFGPKG_NO_SUCH_VARNAME,
} CFGSTATUS;

/* System type definitions */
#define CFG_INTERCONNECT_MASTER 1
#define CFG_INTERMEDIARY      	2
#define CFG_INSERTION_SERVER    3
#define CFG_HEADEND_MASTER      3
#define CFG_INSERTION_SYSTEM    4
#define CFG_INPUT_EDIT          5
#define CFG_SCHEDULE_SYSTEM     6
#define CFG_UNKNOWN             7

/* constants for Greyhound SDVL system types*/
#define CFG_GH_CLUSTER  		9
#define CFG_GH_IU       		10	// Greyhound inserter
#define CFG_GH_MVL      		11

// More system types for completeness
#define CFG_DB_MASTER     12
#define CFG_DB_SLAVE      13
#define CFG_MGMT          14
#define CFG_STANDALONE    15   // Low End System (LES)
#define CFG_GH_RVL        16	// Remote Video Library
#define	CFG_RDB_MASTER    17	// Primary Slave Database (for a disconnected site)
#define	CFG_GH_IC         19	// Greyhound Intermediate Cache
#define CFG_GH_AMVL       20    // Alternative MVL

//int CFG_CLUSTER(char* clustername);

/* Object types */
#define VSTREAM_SYSTEM    	1
#define VSTREAM_INSERT_DEV  2

/* Values for direction parameter */
#define CFG_UPSTREAM     	1
#define CFG_DOWNSTREAM    2

/* System and adapter info structures. */
#define MAXOBJNAMELEN   32
#define MAXNETADAPTERS   8
#define MAXDEVNAMELEN   32
#define MAXSUBKEYLEN    64
#define MAXMAINKEYLEN   64
#define MAXIPADDRLEN    18

// Define the facility codes
//
#define CFGPKG                           0x15


//
// Define the severity codes
//


//
// MessageId: CFG_EXCEPTION
//
// MessageText:
//
//  (CFGPKG)Exception encountered in application.
//  Exception code is %1. Address is %2.
//  Binary data contain exception record and context.
//
#define CFG_EXCEPTION                    0xC0150001L

//
// MessageId: CFG_FAILURE
//
// MessageText:
//
//  In rtn %1 operation %2 failed, status = %3 
//
#define CFG_FAILURE                      0xC0150002L



/*
 * struct vstream_obj contains the name and type of a
 * videostream object.
 */
//
typedef struct vstream_obj {
  TCHAR   obj_name[MAXOBJNAMELEN];
  DWORD   obj_type;
} VSTREAM_OBJ;


/*
 * struct adapter_info contains the device name, network type
 * indicator, NETBios flag, and internet address for a network
 * adapter. It is part of the vstream_system structure.
 */
typedef struct adapter_info {
  TCHAR    dev_name[MAXDEVNAMELEN];
  TCHAR    inet_addr[MAXIPADDRLEN];
  DWORD    net_type;
  BOOL     NetBIOS_bnd;
} ADAPTER_INFO;

/*
 * struct vstream_system contains the details about a videostream object
 * of type VSTREAM_SYSTEM
 */
struct vstream_system {
  DWORD     systype;        
  SHORT     no_netadapt;
  struct  adapter_info adapter[MAXNETADAPTERS];
} ;

/*
 * struct vstream_insert_dev contains the details about a videostream object
 * of type VSTREAM_INSERT_DEV. The IS_name is the computername of
 * the insertion system on which the device resides. This must be
 * supplied by the caller to CFG_GET_STREAM_OBJ_INFO. If it is NULL, the
 * device is assumed to be on the local (caller's) system.
 */
struct vstream_insert_dev {
  TCHAR  IS_name[MAX_COMPUTERNAME_LENGTH];
  DWORD   DTMF_delay;
  DWORD   act_wind;
  TCHAR  dev_name[MAXDEVNAMELEN];
};


/*
 * struct vstream_obj_info contains a struct vstream_obj as well on of
 * struct vstream_system or vstream_insret_dev.
 */
struct vstream_obj_info {
  struct vstream_obj  obj;
  union {
	struct vstream_system   sys_obj;
	struct vstream_insert_dev dev_obj;
  };
};

/* Function call prototypes */
#if defined(__cplusplus)
extern "C" {
#endif


//////////////////////////////////////////////////////////////////////////////
// Generic interface (not CDCI specific)
//
/*
 * CFG_INITEx positions the caller at the root of the app's configuration
 * parameters. It returns a handle which must be used in subsequent calls
 * to get and set values. For non CDCI apps, so you can specify the product name
 *
 * app_name -- The name of your service as it appears in the registry under SOFTWARE\\SeaChange...
 * num_values -- receives the number of values under this key
 * lpszProductName -- the name of the product, e.g. CDCI, or ITV
 *
 * Returns a session handle for use in additional CfgPkg calls.
 */
	HANDLE CFG_INITEx(TCHAR* app_name, DWORD* num_values, TCHAR* lpszProductName,BOOL bServicesMode = TRUE);
/*
 * CFG_GET_PARENT_VALUES returns value data from the specified attribute in the
 * parent key of the application's current key. 
 *
 * handle (IN) -- a handle for the CfgSession, returned from a call to CFG_INITEx
 * attr_name (IN) -- name of the attribute whose value is to be retrieved.
 * buffer (OUT) -- buffer that receives the value of the specified attribute
 * buffer_size (IN/OUT) -- size of the buffer in Bytes, note that strings are Unicode
 *              and require 2 bytes per character.  Also, receives the number of
 *              of bytes written.
 * attr_type (OUT) -- one of the Windows Registry types described in the API call
 *              RegQueryValueEx( ).  REG_BINARY, REG_DWORD, etc.
 *
 */
	CFGSTATUS CFG_GET_PARENT_VALUES(HANDLE handle, TCHAR* attr_name, BYTE* buffer, 
                                DWORD* buffer_size, DWORD* attr_type,BOOL bServicesMode = TRUE);


/*
 * CFG_GET_VALUE retrieves the data value for the specified attribute.
 *
 * handle (IN) -- a handle for the CfgSession, returned from a call to CFG_INITEx
 * attr_name (IN) -- name of the attribute whose value is to be retrieved
 * buffer (OUT) -- buffer that receives the value of the specified attribute.
 * buffer_size (IN/OUT) -- size of the buffer in Bytes, note that strings are Unicode
 *              and require 2 bytes per character.  Also, receives the number of 
 *              bytes written.
 * attr_type (OUT) -- one of the Windows Registry types described in the API call
 *              RegQueryValueEx( ).  REG_BINARY, REG_DWORD, etc.
 */
	CFGSTATUS CFG_GET_VALUE(HANDLE handle, TCHAR* attr_name, BYTE* buffer,
                            DWORD* buffer_size, DWORD* attr_type,BOOL bServicesMode = TRUE);


/*
 * CFG_SET_VALUE sets the data value for the specified attribute, creating the
 * attribute if it does not exist.
 *
 * handle (IN) -- a handle for the CfgSession, returned from a call to CFG_INITEx
 * attr_name (IN) -- name of the attribute whose value is to be retrieved
 * attr_buff (IN) -- buffer that contains the value of the specified attribute
 * attr_buff_len (IN) -- size, in bytes, of attr_buff, note that strings are Unicode
 *                  and require 2 bytes per character.
 * attr_type (IN) -- one of the Windows Registry types described in the API call
 *              RegQueryValueEx( ).  REG_BINARY, REG_DWORD, etc.
 */
	CFGSTATUS CFG_SET_VALUE(HANDLE handle, TCHAR* attr_name, BYTE* attr_buff,
      DWORD attr_buff_len, DWORD attr_type,BOOL bServicesMode = TRUE);

/*
 * CFG_SUBKEY makes the specified subkey the current key from which to retrieve
 * values and parent values.
 *
 * handle (IN) -- a handle for the CfgSession, returned from a call to CFG_INITEx
 * subkey (IN) -- the subkey of the current key, can be multiple levels,
 *                E.g. "subkey1\\subkey2\\bottomKey"
 *                Set this to NULL to reset the current key to the root of the
 *                application (as set in the call to CFG_INITEx).
 * num_values (OUT) -- receives the number of values in this new subkey
 *
 */
	CFGSTATUS CFG_SUBKEY(HANDLE handle, TCHAR* subkey, DWORD* num_values,BOOL bServicesMode = TRUE);
/*
 * CFG_SUBKEY_EXISTS tests for the existence of the given subkey.
 *
 * handle (IN) -- a handle for the CfgSession, returned from a call to CFG_INITEx
 * subkey (IN) -- the subkey of the current key, can be multiple levels,
 *                E.g. "subkey1\\subkey2\\bottomKey"
 *
 * This function returns CFG_SUCCESS if the given subkey exists, otherwise it
 * returns an error.
 */
CFGSTATUS CFG_SUBKEY_EXISTS(HANDLE handle, LPCTSTR subkey,BOOL bServicesMode = TRUE);

/*
 * CFG_VALUE_EXISTS tests for the existence of the given varName.
 *
 * handle (IN) -- a handle for the CfgSession, returned from a call to CFG_INITEx
 * varName (IN) -- the varname of the current key
 *                
 * This function returns CFG_SUCCESS if the given subkey exists, otherwise it
 * returns an error.
 */

CFGSTATUS CFG_VALUE_EXISTS(HANDLE handle, LPCTSTR varName,BOOL bServicesMode = TRUE);

/*
 * CFG_GETMGMNTPORT returns the ManUtil/Manpkg port number for a given service.
 *
 * handle (IN) -- a handle for the CfgSession, returned from a call to CFG_INITEx
 * port_number (OUT) -- receives the management port number of the service specified.
 *              By current convention, this is taken from 
 *              HKLM\Software\SeaChange\Management\CurrentVersion\Services\<app_name>
 *              where <app_name> is the value supplied in CFG_INITEx.
 */
CFGSTATUS CFG_GET_MGMT_PORT(HANDLE handle, DWORD* port_number);

/*
 * CFG_TERM releases any configuration resources, such as handles to the registry. 
 *
 *  handle (IN) -- the handle for the CfgSession returned from a call to CFG_INITEx
 */
CFGSTATUS CFG_TERM(HANDLE handle);


///////////////////////////////////////////////////////////////////////////////////
// CDCI specfic interface (based on CDCI registry only!)
//
/*
 * CFG_INIT positions the caller at the root of the app's configuration
 * parameters. It returns a handle which must be used in subsequent calls
 * to get and set values.
 */
	HANDLE CFG_INIT(TCHAR *app_name, DWORD *num_values,BOOL bServicesMode = TRUE);
	
/*
 * CFG_GET_CDCI_VERSION returns the major and minor version strings stored in
 * the CDCI key on the local node.
 */
	CFGSTATUS CFG_GET_CDCI_VERSION(BYTE*, DWORD*, BYTE*, DWORD*);

/*
 * CFG_GET_CDCI_VERSION returns the major and minor version strings stored in
 * the User defined product key on the local node.
 */
	CFGSTATUS CFG_GET_CDCI_VERSIONEx(HANDLE, BYTE*, DWORD*, BYTE*, DWORD*);

/*
 * CFG_GET_STREAM_TOPOLOGY returns data into an array of vstream_obj structures.
 * Depending on the direction parameter, the local system's parent or child
 * objects in the videostream hierarchy are returned. 
 */
	CFGSTATUS CFG_GET_STREAM_TOPOLOGY(TCHAR*, SHORT, struct vstream_obj*, DWORD*);

/*
 * CFG_GET_STREAM_OBJ_INFO returns the parameters of one of the videostream
 * objects returned by CFG_GET_STREAM_TOPOLOGY.
 */
	CFGSTATUS CFG_GET_STREAM_OBJ_INFO(struct vstream_obj_info*);

/*
 * CFG_FREE() frees memory by simply calling the C RTL free()
 * routine.  Required for MFC based DLLs freeing memory allocated
 * by non-MFC based service code.
 */
	void CFG_FREE( TCHAR* );

#if defined(__cplusplus)
}
#endif
#endif
