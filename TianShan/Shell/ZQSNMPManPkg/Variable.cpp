// Variable.cpp: implementation of the Variable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZQSnmpUtil.h"
#include "Variable.h"

#define LOG_MODULE_NAME         "Variable"

extern ZQSNMP_CALLBACK g_callback;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Variable::Variable()
: m_address(NULL), m_type(ZQSNMP_VARTYPE_INT32), m_access(ZQSNMP_ACCESS_READONLY)
{
}
Variable::Variable(const std::string &name, void *address, DWORD type, DWORD access)
:m_name(name), m_address(address), m_type(type), m_access(access)
{
    //disable setting string value
    if(ZQSNMP_VARTYPE_STRING == m_type)
        m_access = ZQSNMP_ACCESS_READONLY;
}

Variable::~Variable()
{
}

ZQSNMP_STATUS Variable::Get(DWORD infoType, AsnAny *pValue) const
{
    if(NULL == pValue)
        return ZQSNMP_E_FAILURE;

    switch(infoType)
    {
    case ZQSNMP_VARINFOTYPE_VALUE: // variable value
        return ZQSnmpUtil::MemToAny(m_type, m_address, pValue);
    case ZQSNMP_VARINFOTYPE_NAME: //variable name
        return ZQSnmpUtil::MemToAny(ZQSNMP_VARTYPE_STRING, m_name.c_str(), pValue);
    case ZQSNMP_VARINFOTYPE_ACCESS: //access
        return ZQSnmpUtil::MemToAny(ZQSNMP_VARTYPE_UINT32, &m_access, pValue);
    default:
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Unsupported infotype. [varname = %s], [infotype = %u]"),m_name.c_str() , infoType);
        return ZQSNMP_E_FAILURE;
    }
    return ZQSNMP_E_NOERROR;
}

ZQSNMP_STATUS Variable::Set(DWORD infoType, const AsnAny *pValue)
{
    if(NULL == pValue)
        return ZQSNMP_E_FAILURE;

    //only value is writable
    switch(infoType)
    {
    case ZQSNMP_VARINFOTYPE_VALUE: //variable value
        {
            //check access type
            if(ZQSNMP_ACCESS_READWRITE != m_access)
            {
                SMLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "Attempt to set readonly variable. [varname = %s]"), m_name.c_str());

                return ZQSNMP_E_READONLY;
            }
            ZQSNMP_STATUS status = ZQSnmpUtil::AnyToMem(m_type, pValue, m_address);
            if(ZQSNMP_E_NOERROR == status && g_callback)
            {
                g_callback(m_name.c_str());
            }
            return status;
        }

    case ZQSNMP_VARINFOTYPE_NAME: //variable name
    case ZQSNMP_VARINFOTYPE_ACCESS: //access

        SMLOG(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "Attempt to set variable attribute. [varname = %s]"), m_name.c_str());
        return ZQSNMP_E_READONLY;
    default:
        SMLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Unsupported infotype. [varname = %s], [infotype = %u]"), m_name.c_str(), infoType);
        return ZQSNMP_E_FAILURE;
    }

    return ZQSNMP_E_NOERROR;
}

const std::string& Variable::getName() const// for logging purpose
{
    return m_name;
}