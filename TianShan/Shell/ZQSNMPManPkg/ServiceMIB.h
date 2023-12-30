// ServiceMIB.h: interface for the ServiceMIB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVICEMIB_H__386B12CD_8F09_4C40_B818_3627784D25D7__INCLUDED_)
#define AFX_SERVICEMIB_H__386B12CD_8F09_4C40_B818_3627784D25D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "VarInfoTable.h"

class ServiceMIB  
{
public:
	ServiceMIB(UINT serviceID, UINT svcProcess);
	~ServiceMIB();

	void Add(const Variable &variable, UINT instId);
    ZQSNMP_STATUS Get(const AsnObjectIdentifier *pOid, AsnAny *pValue);
	ZQSNMP_STATUS GetNext(const AsnObjectIdentifier *pOid, AsnObjectIdentifier *pNextOid, AsnAny *pNextValue);
	ZQSNMP_STATUS Set(const AsnObjectIdentifier *pOid, const AsnAny *pValue);

    UINT GetServiceID();
    UINT GetSvcProcess();
private:
    ZQSNMP_STATUS nextOid(const AsnObjectIdentifier *pOid, AsnObjectIdentifier *pNextOid);
    ZQSNMP_STATUS checkOid(const AsnObjectIdentifier *pOid, UINT *pVarInfoType, UINT *pVarInstanceID);
private:
    const UINT m_serviceID;
    const UINT m_svcProcess;
    VarInfoTable m_varInfoTbl; 
};

#endif // !defined(AFX_SERVICEMIB_H__386B12CD_8F09_4C40_B818_3627784D25D7__INCLUDED_)
