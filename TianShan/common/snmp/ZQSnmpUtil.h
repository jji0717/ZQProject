// ZQSnmpUtil.h: interface for the ZQSnmpUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZQSNMPUTIL_H__A6C37FAB_5DA1_4A68_8872_61D9EDC2735C__INCLUDED_)
#define AFX_ZQSNMPUTIL_H__A6C37FAB_5DA1_4A68_8872_61D9EDC2735C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include "ZQSnmp.h"
#include <snmp.h>

class ZQSnmpUtil  
{
public:
    //encode/decode function
    static bool UpdateMsg(void *stream, int len, AsnInteger32 errstat);
    static bool EncodeMsg(BYTE pdutype, AsnInteger32 errstat, const SnmpVarBind *pVb, std::string *pMsg);
	static bool DecodeMsg(const void *stream, int len, BYTE *pPdutype, AsnInteger32 *pErrstat, SnmpVarBind *pVb);

    //read/write function
    static ZQSNMP_STATUS AnyToMem(DWORD type, const AsnAny *pValue, void *address);
    static ZQSNMP_STATUS MemToAny(DWORD type, const void *address, AsnAny *pValue);

    //ZQ oid utility function
    static bool ParseOid(const AsnObjectIdentifier *pOid, UINT *pSvcInstanceID, UINT *pSvcProcess, UINT *pVarInfoType, UINT *pVarInstanceID);
    static bool CreateOid(AsnObjectIdentifier *pOid, UINT svcInstId, UINT svcProcess, const UINT *pVarInfoType = NULL, const UINT *pVarInstId = NULL);

    //Named Pipe utility function
    static std::string GetPipeName(UINT svcId, UINT svcProcess);

    //debug utility function
    static std::string GetPduType(BYTE pdutype);
    static std::string GetErrorStatus(AsnInteger32 errstat);
    static std::string GetVariableType(DWORD asnType);
private:
    static const UINT m_ZQServiceOIDPrefix[ZQSNMP_OID_LEN_SVCPREFIX];
};

#endif // !defined(AFX_ZQSNMPUTIL_H__A6C37FAB_5DA1_4A68_8872_61D9EDC2735C__INCLUDED_)
