#include <cstdlib>
#include <cstring>
#include "Mib.h"


ServiceMIB::ServiceMIB(uint32_t serviceID, uint32_t svcProcess)
:m_serviceID(serviceID), m_svcProcess(svcProcess) {
}

ServiceMIB::~ServiceMIB() {
}

void ServiceMIB::Add(const Variable &variable, int subOid)
{
    m_varInfoTbl.Add(variable, subOid);
}

ZQSNMP_STATUS ServiceMIB::Get(netsnmp_variable_list *pvb) {
    if(NULL == pvb)
        return ZQSNMP_E_FAILURE;

    uint32_t varinfotype = 0;
	uint32_t varinst     = 0;
    ZQSNMP_STATUS status = checkOid(pvb->name, pvb->name_length, &varinfotype, &varinst);

    if(ZQSNMP_E_NOERROR != status)
        return ZQSNMP_E_NOSUCHNAME;

    //accord with SNMP MIB table
    //variable instance id as row
    //variable information type as column
    return m_varInfoTbl.Get(varinst, varinfotype, pvb);
}

ZQSNMP_STATUS ServiceMIB::GetNext(netsnmp_variable_list *pvb) {
    if(NULL == pvb){
        return ZQSNMP_E_FAILURE;
    }

	oid next[ZQSNMP_OID_LEN_MAX] = {0};
    size_t nextLen = ZQSNMP_OID_LEN_MAX;

    ZQSNMP_STATUS status = nextOid(pvb->name, pvb->name_length, next, nextLen);

    if(ZQSNMP_E_NOERROR != status) {
        return status;
    }

    if(0 != snmp_set_var_objid(pvb, next, nextLen)) {
        return ZQSNMP_E_FAILURE;
    }

    return Get(pvb);
}

ZQSNMP_STATUS ServiceMIB::Set(netsnmp_variable_list *pvb) {
    if(!pvb) {
        return ZQSNMP_E_FAILURE;
    }

    uint32_t varinfotype = 0;
	uint32_t varinst     = 0;
    ZQSNMP_STATUS status = checkOid(pvb->name, pvb->name_length, &varinfotype, &varinst);
    if(ZQSNMP_E_NOERROR != status) {
        return ZQSNMP_E_NOSUCHNAME;
    }

    //accord with SNMP MIB table
    //variable instance id as row
    //variable information type as column
    return m_varInfoTbl.Set(varinst, varinfotype, pvb);
}

uint32_t ServiceMIB::GetServiceID() {
    return m_serviceID;
}

uint32_t ServiceMIB::GetSvcProcess() {
    return m_svcProcess;
}

ZQSNMP_STATUS ServiceMIB::nextOid(const oid *pOid, size_t len, oid *pNextOid, size_t& pNextLen) {

    if(!pOid || !pNextOid) { 
        return ZQSNMP_E_FAILURE;
    }

    uint32_t svcinst     = 0; 
	uint32_t svcprocess  = 0; 
	uint32_t varinfotype = 0; 
	uint32_t varinst     = 0; 
    if(!ZQSNMP::Util::parseOid(pOid, len, svcinst, svcprocess, varinfotype, varinst)) {
        return ZQSNMP_E_NOSUCHNAME;
    }

    //check the oid region

    //the next oid is not in this MIB
    bool bOverThisRegion = svcinst > m_serviceID || svcprocess > m_svcProcess;
    if(bOverThisRegion) {
        return ZQSNMP_E_NOSUCHNAME;
    }

    //the next oid is the first oid in this MIB
    bool bBeforeThisRegion = svcinst < m_serviceID || svcprocess < m_svcProcess;
    if(bBeforeThisRegion) {
        //construct a dummy current oid that just before the first oid
        varinfotype = 0;
        varinst = 0;
    }

    //retrieve the next oid

    //accord with SNMP MIB table
    //variable instance id as row
    //variable information type as column
    uint32_t newVarInfoType = 0;
	uint32_t newVarInst     = 0;
    ZQSNMP_STATUS status = m_varInfoTbl.NextItem(varinst, varinfotype, &newVarInst, &newVarInfoType);
    if(ZQSNMP_E_NOERROR != status) {
        return ZQSNMP_E_NOSUCHNAME;
    }

    if(ZQSNMP::Util::createOid(
                pNextOid, 
                pNextLen, 
                m_serviceID, 
                m_svcProcess, 
                &newVarInfoType, 
                &newVarInst)) {

        return ZQSNMP_E_NOERROR;
    }
    else {
        return ZQSNMP_E_FAILURE;
    }

    return ZQSNMP_E_NOERROR;
}

ZQSNMP_STATUS ServiceMIB::checkOid(
        const oid* pOid, 
        size_t len, 
        uint32_t *pVarInfoType, 
        uint32_t *pVarInstanceID) {

    if(!pOid) {
        return ZQSNMP_E_FAILURE;
    }

    uint32_t svcinst     = 0;
	uint32_t svcporcess  = 0;
	uint32_t varinfotype = 0;
	uint32_t varinst     = 0;
    if(!ZQSNMP::Util::parseOid(pOid, len, svcinst, svcporcess, varinfotype, varinst)) {
        return ZQSNMP_E_FAILURE;
    }

    if(svcinst != m_serviceID || svcporcess != m_svcProcess) {
        return ZQSNMP_E_NOSUCHNAME;
    }

    if(pVarInfoType)   *pVarInfoType = varinfotype;
    if(pVarInstanceID) *pVarInstanceID = varinst;

    return ZQSNMP_E_NOERROR;
}
