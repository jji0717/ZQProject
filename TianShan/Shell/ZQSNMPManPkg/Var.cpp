#include <cstdlib>
#include <cstring>
#include "Var.h"


extern ZQSNMP_CALLBACK g_callback;

Variable::Variable(const std::string &name, void *address, uint32_t type, uint32_t access)
:m_name(name), m_address(address), m_type(type), m_access(access) {
    //disable setting string value
    if(ZQSNMP_VARTYPE_CSTRING == m_type)
        m_access = ZQSNMP_ACCESS_READONLY;
}

Variable::Variable()
	: m_address(NULL), m_type(0), m_access(ZQSNMP_ACCESS_READONLY)
{
}


Variable::~Variable() {
}

ZQSNMP_STATUS Variable::Get(uint32_t infoType, netsnmp_variable_list *pvb) const {
    if(NULL == pvb)
        return ZQSNMP_E_FAILURE;

    switch(infoType)
    {
    case ZQSNMP_VARINFOTYPE_VALUE: // variable value
        return ZQSNMP::Util::memToAny(m_type, m_address, pvb);
    case ZQSNMP_VARINFOTYPE_NAME: //variable name
        return ZQSNMP::Util::memToAny(ZQSNMP_VARTYPE_STDSTRING, &m_name, pvb);
    case ZQSNMP_VARINFOTYPE_ACCESS: //access
        return ZQSNMP::Util::memToAny(ZQSNMP_VARTYPE_INT32, &m_access, pvb);
    default:
        return ZQSNMP_E_FAILURE;
    }
    return ZQSNMP_E_NOERROR;
}

ZQSNMP_STATUS Variable::Set(uint32_t infoType, const netsnmp_variable_list *pvb) {
    if(!pvb) {
        return ZQSNMP_E_FAILURE;
    }

    //only value is writable
    switch(infoType) {

    case ZQSNMP_VARINFOTYPE_VALUE: //variable value
        {
            //check access type
            if(ZQSNMP_ACCESS_READWRITE != m_access) {
                return ZQSNMP_E_READONLY;
            }
            ZQSNMP_STATUS status = ZQSNMP::Util::anyToMem(m_type, pvb, m_address);
            if(ZQSNMP_E_NOERROR == status && g_callback) {
                g_callback(m_name.c_str());
            }
            return status;
        }

    case ZQSNMP_VARINFOTYPE_NAME: //variable name
    case ZQSNMP_VARINFOTYPE_ACCESS: //access
        return ZQSNMP_E_READONLY;
    default:
        return ZQSNMP_E_FAILURE;
    }

    return ZQSNMP_E_NOERROR;
}
