// VarCache.cpp: implementation of the VarCache class.
//
//////////////////////////////////////////////////////////////////////
#include <ZQ_common_conf.h>
#ifdef ZQ_OS_MSWIN
#include "ZQSnmpUtil.h"
#include "stdafx.h"
#else
#include "SnmpUtil.h"
#endif

#include <vector>
#include <sstream>
#include "SNMPOper.h"
#include "VarCache.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef ZQ_OS_MSWIN
VarCache::VarCache(UINT svcid)
:m_svcid(svcid)
{

}

VarCache::~VarCache()
{

}

bool VarCache::Add(SnmpVarBind *pVb)
{
    if(NULL == pVb)
        return false;

    UINT svcid, processoid, varinfotype, varinstid;
    if(!ZQSnmpUtil::ParseOid(&(pVb->name), &svcid, &processoid, &varinfotype, &varinstid))
        return false;

    if(svcid != m_svcid)
        return false;

    switch(varinfotype)
    {
    case ZQSNMP_VARINFOTYPE_VALUE:
        m_cache[processoid][varinstid].SetOid(&(pVb->name));
        return m_cache[processoid][varinstid].SetValue(&(pVb->value));
    case ZQSNMP_VARINFOTYPE_NAME:
        return m_cache[processoid][varinstid].SetName(&(pVb->value));
    case ZQSNMP_VARINFOTYPE_ACCESS:
        return m_cache[processoid][varinstid].SetAccess(&(pVb->value));
    default:
        return false;
    }
}

bool VarCache::Get(std::vector< SNMPOper::SNMPValue > &values)
{
    std::map< UINT, ProcessDataBlock >::iterator itSvc;

    for(itSvc = m_cache.begin(); itSvc != m_cache.end(); ++itSvc)
    {
        ProcessDataBlock::iterator itProcess;
        for(itProcess = (*itSvc).second.begin(); itProcess != (*itSvc).second.end(); ++itProcess)
        {
            SNMPOper::SNMPValue value;
            if((*itProcess).second.Get(&value))
            {
                values.push_back(value);
            }
            else
            {
                //log error?
            }
        }
    }
    return true;
}
static bool validAsnString(const AsnOctetString *pAsnStr)
{
    if(NULL == pAsnStr)
        return false;

    if(NULL == pAsnStr->stream && pAsnStr->length != 0)
        return false;

    return true;
}
bool VarCache::VarData::SetName(const AsnAny *pName)
{
    if(NULL == pName)
        return false;

    if(pName->asnType != ASN_OCTETSTRING)
        return false;

    if(m_check.test(VARATTR_NAME))
        return false;

    //valid asn string?
    if(!validAsnString(&(pName->asnValue.string)))
        return false;

    m_name.assign((const char *)(pName->asnValue.string.stream), pName->asnValue.string.length);

    m_check.set(VARATTR_NAME);
    return true;
}
bool VarCache::VarData::SetValue(const AsnAny *pValue)
{
    if(NULL == pValue)
        return false;

    if(m_check.test(VARATTR_VALUE))
        return false;

    //check value type
    //convert to string
    switch(pValue->asnType)
    {
    case ASN_INTEGER:
        {
            char intbuf[12] = {0};
            m_value = itoa(pValue->asnValue.number, intbuf, 10);
            m_type = ZQSNMP_VARTYPE_INT32;
        }
        break;
    case ASN_UNSIGNED32:
        {
            char uintbuf[12] = {0};
            m_value = ultoa(pValue->asnValue.number, uintbuf, 10);
            m_type = ZQSNMP_VARTYPE_UINT32;
        }
        break;
    case ASN_OCTETSTRING:
        {
            if(!validAsnString(&(pValue->asnValue.string)))
                return false;
            m_value.assign((const char *)(pValue->asnValue.string.stream), pValue->asnValue.string.length);
            m_type = ZQSNMP_VARTYPE_STRING;
        }
        break;
    default:
        return false;
    }

    m_check.set(VARATTR_VALUE);
    return true;
}
bool VarCache::VarData::SetAccess(const AsnAny *pAccess)
{
    if(NULL == pAccess)
        return false;

    if(pAccess->asnType != ASN_UNSIGNED32)
        return false;

    if(m_check.test(VARATTR_ACCESS))
        return false;

    m_access = pAccess->asnValue.unsigned32;

    m_check.set(VARATTR_ACCESS);
    return true;
}

void VarCache::VarData::SetOid(const AsnObjectIdentifier *pOid)
{
    m_oid = ".";//consistent with SnmpMgrStrToOid. See MSDN.
    m_oid += SnmpUtilOidToA((AsnObjectIdentifier *)pOid);
}

bool VarCache::VarData::Get(SNMPOper::SNMPValue *pValue)
{
    if(NULL == pValue)
        return false;

    if(!m_check.any())
        return false;

    SNMPOper::SNMPValue val = {0};
    //name
    if(m_name.size() < SNMPATTR_VARNAME_MAXLEN)
    {
        strcpy(val.name, m_name.c_str());
    }
    else
    {
        return false;
    }

    //value
    if(m_value.size() < SNMPATTR_VARVALUE_MAXLEN)
    {
        strcpy(val.value, m_value.c_str());
    }
    else
    {
        return false;
    }

    //oid
    if(m_oid.size() < SNMPATTR_OID_MAXLEN)
    {
        strcpy(val.oid, m_oid.c_str());
    }
    else
    {
        return false;
    }

    //type
    switch(m_type)
    {
    case ZQSNMP_VARTYPE_INT32:
        val.type = SNMPATTR_VARTYPE_INT;
        break;
    case ZQSNMP_VARTYPE_STRING:
        val.type = SNMPATTR_VARTYPE_STRING;
        break;
    default:
        return false;
    }
    //access
    switch(m_access)
    {
    case ZQSNMP_ACCESS_READWRITE:
        val.readonly = SNMPATTR_VARRW_WRITABLE;
        break;
    default:
        val.readonly = SNMPATTR_VARRW_READONLY;
        break;
    }

    // parse type and access from name, such as "LogLevel:INT(rw)"
    std::string fullName = val.name;
    std::string name, type, access;
    size_t cpos  = fullName.find(':');
    size_t lbpos = fullName.find('(');
    size_t rbpos = fullName.find(')');
    if (std::string::npos != cpos && std::string::npos != lbpos && std::string::npos != rbpos)
    {
        name = fullName.substr(0, cpos);
        type = fullName.substr(cpos + 1, lbpos - cpos - 1);
        access = fullName.substr(lbpos + 1, rbpos - lbpos - 1);

        strcpy(val.name, name.c_str());

        if ("rw" == access){
            val.readonly = SNMPATTR_VARRW_WRITABLE;
        }else{
            val.readonly = SNMPATTR_VARRW_READONLY;
        }

        if ("INT" == type || "I64" == type)
            val.type = SNMPATTR_VARTYPE_INT;
        else if ("STR" == type)
            val.type = SNMPATTR_VARTYPE_STRING;
        else 
            return false;
    }

    *pValue = val;
    return true;
}

#else

VarCache::VarCache(uint32_t svcid)
:m_svcid(svcid) {
}

VarCache::~VarCache() {
}

bool VarCache::Get(std::vector< SNMPOper::SNMPValue > &values)
{
    std::map< uint32_t, ProcessDataBlock >::iterator itSvc;

    for(itSvc = m_cache.begin(); itSvc != m_cache.end(); ++itSvc)
    {
        ProcessDataBlock::iterator itProcess;
        for(itProcess = (*itSvc).second.begin(); itProcess != (*itSvc).second.end(); ++itProcess)
        {
            SNMPOper::SNMPValue value;
            if((*itProcess).second.Get(&value))
            {
                values.push_back(value);
            }
        }
    }
    return true;
}

bool VarCache::Add(netsnmp_variable_list* vars) {
    if(!vars) {
        return false;
    }

    uint32_t svcid, processoid, varinfotype, varinstid;
    if(!ZQSNMP::Util::parseOid(vars->name, vars->name_length, svcid, processoid, varinfotype, varinstid)) {
        return false;
    }

    if(svcid != m_svcid) {
        return false;
    }

    switch(varinfotype) {

    case ZQSNMP_VARINFOTYPE_VALUE:
        m_cache[processoid][varinstid].SetOid(vars->name, vars->name_length);
        return m_cache[processoid][varinstid].SetValue(vars);

    case ZQSNMP_VARINFOTYPE_NAME:
        return m_cache[processoid][varinstid].SetName(vars);

    case ZQSNMP_VARINFOTYPE_ACCESS:
        return m_cache[processoid][varinstid].SetAccess(vars);

    default:
        return false;

    }
}

bool VarCache::VarData::SetName(const void* name) {
    if(!name) {
        return false;
    }

    netsnmp_variable_list* vars = (netsnmp_variable_list*)name; 

    if(vars->type != ASN_OCTET_STR) {
        return false;
    }

    if(m_check.test(VARATTR_NAME)) {
        return false;
    }

   // if(!validAsnString(&(pName->asnValue.string)))
    //    return false;

    m_name.assign((const char *)(vars->val.bitstring), vars->val_len);

    m_check.set(VARATTR_NAME);
    return true;
}

bool VarCache::VarData::SetValue(const void* value) {
    if(!value){
        return false;
    }
   
    if(m_check.test(VARATTR_VALUE)) {
        return false;
    }

    netsnmp_variable_list* vars = (netsnmp_variable_list*)value; 

    switch(vars->type) {

    case ASN_INTEGER:
        {
            std::ostringstream oss;
            oss << *(vars->val.integer);
            m_value = oss.str();
            m_type = ZQSNMP_VARTYPE_INT32;
        }
        break;

    case ASN_OCTET_STR:
        {
//            if(!validAsnString(&(pValue->asnValue.string)))
 //               return false;
            m_value.assign((const char *)(vars->val.bitstring), vars->val_len);
            m_type = ZQSNMP_VARTYPE_STRING;
        }
        break;
    default:
        return false;
    }

    m_check.set(VARATTR_VALUE);
    return true;
}

bool VarCache::VarData::SetAccess(const void* access) {
    if(!access){
        return false;
    }

    netsnmp_variable_list* vars = (netsnmp_variable_list*)access; 
    
    if(vars->type != ASN_INTEGER) {
        return false;
    }

    if(m_check.test(VARATTR_ACCESS)) {
        return false;
    }

    m_access = *(vars->val.integer);

    m_check.set(VARATTR_ACCESS);
    return true;
}

void VarCache::VarData::SetOid(const oid* name, size_t len) {
    char buff[MAX_OID_LEN];
    memset(buff, '\0', MAX_OID_LEN);
    snprint_objid(buff, MAX_OID_LEN, name, len);
    m_oid = buff;
}

bool VarCache::VarData::Get(SNMPOper::SNMPValue* value) {
    if(!value) {
        return false;
    }

    if(!m_check.any()) {
        return false;
    }

    SNMPOper::SNMPValue val;
    //name
    if(m_name.size() < SNMPATTR_VARNAME_MAXLEN) {
        strcpy(val.name, m_name.c_str());
    }
    else {
        return false;
    }

    //value
    if(m_value.size() < SNMPATTR_VARVALUE_MAXLEN) {
        strcpy(val.value, m_value.c_str());
    }
    else {
        return false;
    }

    //oid
    if(m_oid.size() < SNMPATTR_OID_MAXLEN) {
        strcpy(val.oid, m_oid.c_str());
    }
    else {
        return false;
    }

    //type
    switch(m_type) {
    case ZQSNMP_VARTYPE_INT32:
        val.type = SNMPATTR_VARTYPE_INT;
        break;
    case ZQSNMP_VARTYPE_STRING:
        val.type = SNMPATTR_VARTYPE_STRING;
        break;
    default:
        return false;
    }

    //access
    switch(m_access) {
    case ZQSNMP_ACCESS_READWRITE:
        val.readonly = SNMPATTR_VARRW_WRITABLE;
        break;
    default:
        val.readonly = SNMPATTR_VARRW_READONLY;
        break;
    }

    // parse type and access from name, such as "LogLevel:INT(rw)"
    std::string fullName = val.name;
    std::string name, type, access;
    size_t cpos  = fullName.find(':');
    size_t lbpos = fullName.find('(');
    size_t rbpos = fullName.find(')');
    if (std::string::npos != cpos && std::string::npos != lbpos && std::string::npos != rbpos)
    {
        name = fullName.substr(0, cpos);
        type = fullName.substr(cpos + 1, lbpos - cpos - 1);
        access = fullName.substr(lbpos + 1, rbpos - lbpos - 1);

        strcpy(val.name, name.c_str());

        if ("rw" == access){
            val.readonly = SNMPATTR_VARRW_WRITABLE;
        }else{
            val.readonly = SNMPATTR_VARRW_READONLY;
        }

        if ("INT" == type || "I64" == type)
            val.type = SNMPATTR_VARTYPE_INT;
        else if ("STR" == type)
            val.type = SNMPATTR_VARTYPE_STRING;
        else 
            return false;
    }

    *value = val;
    return true;
}

#endif


