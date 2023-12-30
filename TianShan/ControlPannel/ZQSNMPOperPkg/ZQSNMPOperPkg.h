#ifndef __ZQSNMPOPERPKG_H
#define __ZQSNMPOPERPKG_H


#pragma warning(disable:4786 )
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <malloc.h>
#include <tchar.h>
#include <string>

using std::map;
using std::iterator;
using std::string;

//
// constants -- These must remain compatible with CDCI.  Yes this is a risk
//              so don't change nothin'!
//

// variable types, they are also the index to the icon image list
#define ZQMAN_STR                ASN_OCTETSTRING
#define ZQMAN_INT                ASN_INTEGER32
#define ZQMAN_FLOAT              20
#define ZQMAN_TIMETICKS          ASN_TIMETICKS
#define ZQMAN_GAUGE              ASN_GAUGE32  
#define ZQMAN_COUNTER    	     ASN_COUNTER32
#define ZQMAN_IPADDRESS	 	     ASN_IPADDRESS
#define ZQMAN_IDENTIFER	         ASN_OBJECTIDENTIFIER
#define ZQMAN_SEQ		 	     ASN_SEQUENCE
#define ZQMAN_OPAQUE	         ASN_OPAQUE
#define ZQMAN_NULL  		     ASN_NULL
#define ZQMAN_BOOL               ASN_BITS
#define VAR_TYPELEN              33
#define VAR_OIDLEN               128
#define VAR_DATALEN              256
#define SERVICENAMELEN           70

#define INFODATA    "info"
#define SUPPORDOID  "Supported_OID"
#define SERVICEDATA "service_data"
#define NAME        "Name"
#define VARIABLE    "variable"
#define OID         "OID"

typedef enum _ZQMANSTATUS 
{
    ZQMAN_SUCCESS = 0,
    ZQMAN_SESSION_ERROR,  // formerly, MAN_SYSERR,
    ZQMAN_SETVAR_ERROR,
	ZQMAN_SETVAR_INVALID_DATATYPE,
	ZQMAN_SETVAR_INVALID_VARNAME,
	ZQMAN_SETVAR_INVALIDOID,
	ZQMAN_SETVAR_ERROR_SETREQUEST,
	ZQMAN_GETVAR_ERROR,
	ZQMAN_GETVAR_INVALID_VARNAME,
	ZQMAN_GETVAR_INVALIDOID,
	ZQMAN_GETVAR_ERROR_GETREQUEST,
	ZQMAN_GETALLVAR_ERROR,
	ZQMAN_GETALLVAR_NOCOUNT,
	ZQMAN_GETALLVAR_ERROR_REREQUEST,
	ZQMAN_GETALLVAR_ERROR_REQUEST,
	ZQMAN_GETALLVAR_INVALIDOID,
    ZQMAN_NO_SUCH_VAR,
}ZQMANSTATUS;

// add by dony 20070122 for OID/Var Mode's switch
#define  VARNAMEIN_MODE  1

typedef struct VARDATASTRUCE
{
	TCHAR *ResultVarType;  //变量的类型
	TCHAR *ResultVarOID;   //变量的OID
	TCHAR *ResultReadWrite;//变量的读写属性
	TCHAR *ResultValue;    //变量的数据
}VARDATAS,*LPVARDATAS;
// prototypes -- These aren't so CDCI specific, so change it all
//              We're also going for internationalized
//

#ifdef __cplusplus
extern "C" {
#endif

// SNMPOpenSession
//  -- Initialize a session with ZQSNMPManPkg.  A process can initialize multiple sesions
//  with ZQSNMPManPkg.    Access  is thread-safe. 
//
// (IN) pszIpAddress --   an ipaddress is for  the session, useful  It does not have to be unique, but
//                          uniqueness is suggested
// (IN) pszCommunity --  the Community value, default it is "public"
// (OUT) phManSession -- a valid handle to ZQSNMPManPkg.  This is NULL on failure
//
// Returns a valid handle to ZQSNMPManPkg.  The return is NULL on failure
ZQMANSTATUS   SNMPOperOpenSession(TCHAR *pszIpAddress,TCHAR * pszCommunity, HANDLE * hSession,TCHAR *szServiceName = NULL);
// SNMPCloseSession
//  -- Terminate a session held with ZQSNMPManPkg.  This function will close the opened snmp session.
//
// (IN) hManSession -- a ZQSNMPManPkg session handle returned from a previous call to SNMPOpenSession
//
// Returns a ZQMANSTATUS, defined above.
ZQMANSTATUS   SNMPOperCloseSession(HANDLE hSession);
    
// SNMPSetVarValue
//  -- Set the Variant Value by SNMP Protocol with ZQSNMPManPkg.  Access  is thread-safe. 
//
// (IN) hSession      --  the opened SNMP Session's Handle.
// (IN) szOid         --  the variant's oid value.
// (TCHAR*)szVarValue --  the variant's value.
// (DWORD)dwAddress   --  the variant's address
// (INT)wType         --  the variant's type
// Returns a ZQMANSTATUS, defined above.
//ZQMANSTATUS   SNMPSetVarValue(HANDLE hSession, TCHAR *szOid,TCHAR *szVarValue,INT wType);
ZQMANSTATUS  SNMPOperSetVarValue(HANDLE hSession, TCHAR *szOid,DWORD dwAddress,INT wType);

// SNMPGetVarValue
//  -- Get the Variant Value by SNMP Protocol with ZQSNMPManPkg.  Access  is thread-safe. 
//
// (IN) hSession      --  the opened SNMP Session's Handle.
// (IN) szOid         --  the variant's oid value.
// (OUT)ResultVarType --  the Return variant's type.
// (OUT)ResultValue   --  the Return variant's value
// Returns a ZQMANSTATUS, defined above.
ZQMANSTATUS   SNMPOperGetVarValue(HANDLE hSession,TCHAR *szOid,TCHAR *ResultVarType = NULL,TCHAR *ResultReadWrite=_T("ReadWrite"),TCHAR *ResultValue = NULL);

// SNMPGetAllVarValue
//  -- Get the ALL Variant Value by the input oid by SNMP Protocol with ZQSNMPManPkg.  Access  is thread-safe. 
//
// (IN) hSession      --  the opened SNMP Session's Handle.
// (IN) szOid         --  the group variant's oid value.
// (OUT)iCount        --  the group variant's count
// (OUT)pReturnData   --  the return group data struct 
// Returns a ZQMANSTATUS, defined above.
ZQMANSTATUS   SNMPOperGetAllVarValue(HANDLE hSession,TCHAR *szOid,int *iCount,VARDATAS** pReturnData);
						  
#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _MANPKG_H_
