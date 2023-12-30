// VarCache.h: interface for the VarCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VARCACHE_H__68B81650_0714_4CBB_BA80_9E84410E2700__INCLUDED_)
#define AFX_VARCACHE_H__68B81650_0714_4CBB_BA80_9E84410E2700__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <bitset>
#include <map>

#ifdef ZQ_OS_MSWIN
#include <snmp.h>
#else
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/utilities.h>
#endif


#ifdef ZQ_OS_MSWIN
class VarCache  
{
public:
	VarCache(UINT svcid);
	~VarCache();
    bool Add(SnmpVarBind *pVb);
    bool Get(std::vector< SNMPOper::SNMPValue > &values);
private:
    class VarData
    {
    public:
        bool SetName(const AsnAny *pName);
        bool SetValue(const AsnAny *pValue);
        bool SetAccess(const AsnAny *pAccess);
        void SetOid(const AsnObjectIdentifier *pOid);
        bool Get(SNMPOper::SNMPValue *pValue);
    private:
        std::string m_name;
        std::string m_value;
        std::string m_oid;
        UINT m_type;
        UINT m_access;

        enum{
            VARATTR_NAME,
            VARATTR_VALUE,
            VARATTR_ACCESS,
            VARATTR_KINDS
        };
        std::bitset< VARATTR_KINDS > m_check;
    };

    const UINT m_svcid;
    typedef std::map< UINT, VarData > ProcessDataBlock;
    std::map< UINT, ProcessDataBlock > m_cache;
};

#else

class VarCache  {

public:

	VarCache(uint32_t svcid);
	~VarCache();
    bool Add(netsnmp_variable_list* vars); 
    bool Get(std::vector< SNMPOper::SNMPValue > &values);
private:
    class VarData
    {
    public:
        bool SetName(const void* pName);
        bool SetValue(const void* pValue);
        bool SetAccess(const void* pAccess);
        void SetOid(const oid*, size_t len);
        bool Get(SNMPOper::SNMPValue *pValue);
    private:
        std::string m_name;
        std::string m_value;
        std::string m_oid;
        uint32_t m_type;
        uint32_t m_access;

        enum{
            VARATTR_NAME,
            VARATTR_VALUE,
            VARATTR_ACCESS,
            VARATTR_KINDS
        };
        std::bitset< VARATTR_KINDS > m_check;
    };

    const uint32_t m_svcid;
    typedef std::map< uint32_t, VarData > ProcessDataBlock;
    std::map< uint32_t, ProcessDataBlock > m_cache;
};

#endif


#endif // !defined(AFX_VARCACHE_H__68B81650_0714_4CBB_BA80_9E84410E2700__INCLUDED_)
