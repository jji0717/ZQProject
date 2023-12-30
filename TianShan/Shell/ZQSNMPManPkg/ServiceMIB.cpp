// ServiceMIB.cpp: implementation of the ServiceMIB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZQSnmpUtil.h"
#include "ServiceMIB.h"

#define LOG_MODULE_NAME         "MIB"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ServiceMIB::ServiceMIB(UINT serviceID, UINT svcProcess)
:m_serviceID(serviceID), m_svcProcess(svcProcess)
{
}

ServiceMIB::~ServiceMIB()
{
}

void ServiceMIB::Add(const Variable &variable, UINT instId)
{
    m_varInfoTbl.Add(variable, instId);
}
ZQSNMP_STATUS ServiceMIB::Get(const AsnObjectIdentifier *pOid, AsnAny *pValue)
{
    if(NULL == pOid || NULL == pValue)
        return ZQSNMP_E_FAILURE;

    UINT varinfotype = 0;
	UINT varinst     = 0;
    ZQSNMP_STATUS status = checkOid(pOid, &varinfotype, &varinst);
    if(ZQSNMP_E_NOERROR != status)
        return ZQSNMP_E_NOSUCHNAME;

    //accord with SNMP MIB table
    //variable instance id as row
    //variable information type as column
    return m_varInfoTbl.Get(varinst, varinfotype, pValue);
}

ZQSNMP_STATUS ServiceMIB::GetNext(const AsnObjectIdentifier *pOid, AsnObjectIdentifier *pNextOid, AsnAny *pNextValue)
{
    if(NULL == pOid || NULL == pNextOid || NULL == pNextValue)
        return ZQSNMP_E_FAILURE;

    ZQSNMP_STATUS status = nextOid(pOid, pNextOid);
    if(ZQSNMP_E_NOERROR != status)
        return status;
    return Get(pNextOid, pNextValue);
}

ZQSNMP_STATUS ServiceMIB::Set(const AsnObjectIdentifier *pOid, const AsnAny *pValue)
{
    if(NULL == pOid || NULL == pValue)
        return ZQSNMP_E_FAILURE;

    UINT varinfotype = 0;
	UINT varinst     = 0;
    ZQSNMP_STATUS status = checkOid(pOid, &varinfotype, &varinst);
    if(ZQSNMP_E_NOERROR != status)
        return ZQSNMP_E_NOSUCHNAME;

    //accord with SNMP MIB table
    //variable instance id as row
    //variable information type as column
    return m_varInfoTbl.Set(varinst, varinfotype, pValue);
}

UINT ServiceMIB::GetServiceID()
{
    return m_serviceID;
}
UINT ServiceMIB::GetSvcProcess()
{
    return m_svcProcess;
}
ZQSNMP_STATUS ServiceMIB::nextOid(const AsnObjectIdentifier *pOid, AsnObjectIdentifier *pNextOid)
{
    if(NULL == pOid || NULL == pNextOid)
        return ZQSNMP_E_FAILURE;

    UINT svcinst     = 0;
	UINT svcprocess  = 0;
	UINT varinfotype = 0;
	UINT varinst     = 0;
    if(!ZQSnmpUtil::ParseOid(pOid, &svcinst, &svcprocess, &varinfotype, &varinst))
    {
        SMLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "OID invalid. [oid = %s]"), SnmpUtilOidToA((AsnObjectIdentifier *)pOid));
        return ZQSNMP_E_NOSUCHNAME;
    }
    //check the oid region

    //the next oid is not in this MIB
    bool bOverThisRegion = (svcinst /100) > (m_serviceID /100) || svcprocess > m_svcProcess;
    if(bOverThisRegion)
    {
        return ZQSNMP_E_NOSUCHNAME;
    }

    //the next oid is the first oid in this MIB
    bool bBeforeThisRegion = svcinst < m_serviceID || svcprocess < m_svcProcess;
    if(bBeforeThisRegion)
    {
        //construct a dummy current oid that just before the first oid
        varinfotype = 0;
        varinst = 0;
    }

    //retrieve the next oid

    //accord with SNMP MIB table
    //variable instance id as row
    //variable information type as column
    UINT newVarInfoType = 0;
	UINT newVarInst     = 0;
    ZQSNMP_STATUS status = m_varInfoTbl.NextItem(varinst, varinfotype, &newVarInst, &newVarInfoType);
    if(ZQSNMP_E_NOERROR != status)
        return ZQSNMP_E_NOSUCHNAME;

    if(ZQSnmpUtil::CreateOid(pNextOid, svcinst, svcprocess, &newVarInfoType, &newVarInst))
    {
        return ZQSNMP_E_NOERROR;
    }
    else
    {
        //TODO: log error
        return ZQSNMP_E_FAILURE;
    }
}

ZQSNMP_STATUS ServiceMIB::checkOid(const AsnObjectIdentifier *pOid, UINT *pVarInfoType, UINT *pVarInstanceID)
{
    if(NULL == pOid)
        return ZQSNMP_E_FAILURE;

    UINT svcinst     = 0; 
	UINT svcporcess  = 0;
	UINT varinfotype = 0;
	UINT varinst     = 0;
    if(!ZQSnmpUtil::ParseOid(pOid, &svcinst, &svcporcess, &varinfotype, &varinst))
    {
        SMLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "OID invalid. [oid = %s]"), SnmpUtilOidToA((AsnObjectIdentifier *)pOid));
        return ZQSNMP_E_FAILURE;
    }

    if( (svcinst / 100 ) != (m_serviceID / 100)	|| svcporcess != m_svcProcess)
    {
        SMLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "OID invalid. [oid = %s]"), SnmpUtilOidToA((AsnObjectIdentifier *)pOid));
        return ZQSNMP_E_NOSUCHNAME;
    }
    if(pVarInfoType)
        *pVarInfoType = varinfotype;
    if(pVarInstanceID)
        *pVarInstanceID = varinst;

    return ZQSNMP_E_NOERROR;
}
