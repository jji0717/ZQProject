#ifndef __VARIABLE__
#define __VARIABLE__

#include "SnmpUtil.h"
#include <string>

class Variable {
public:
    Variable(const std::string &name, void *address, uint32_t type, uint32_t access);
	Variable();
	virtual ~Variable();
	ZQSNMP_STATUS Set(uint32_t infoType, const netsnmp_variable_list *pvb);
    ZQSNMP_STATUS Get(uint32_t infoType, netsnmp_variable_list *pvb) const;
private:
    std::string m_name;
    void *m_address;
    uint32_t m_type;
    uint32_t m_access;
};

#endif 

