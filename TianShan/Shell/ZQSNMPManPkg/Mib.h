#ifndef __SERVICE_MIB__
#define __SERVICE_MIB__

#include "VarInfo.h"

class ServiceMIB {
public:
	ServiceMIB(uint32_t serviceID, uint32_t svcProcess);
	~ServiceMIB();

	void Add(const Variable &variable, int subOid);
    ZQSNMP_STATUS Get(netsnmp_variable_list *vars);
	ZQSNMP_STATUS GetNext(netsnmp_variable_list *vars);
	ZQSNMP_STATUS Set(netsnmp_variable_list *vars);

    uint32_t GetServiceID();
    uint32_t GetSvcProcess();

private:

    ZQSNMP_STATUS nextOid(const oid* pOid, size_t len, oid* pNextOid, size_t& pNextLen);
    ZQSNMP_STATUS checkOid(const oid* pOid, size_t len, uint32_t* pVarInfoType, uint32_t* pVarInstanceID);

private:

    const uint32_t m_serviceID;
    const uint32_t m_svcProcess;
    VarInfoTable m_varInfoTbl; 

};

#endif 

