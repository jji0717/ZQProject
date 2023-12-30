#ifndef __VAR_INFO__
#define __VAR_INFO__

#include "Var.h"
#include <map>

class VarInfoTable  
{
public:
	VarInfoTable();
	~VarInfoTable();
    void Add(const Variable &variable, int subOid);
    ZQSNMP_STATUS Get(uint32_t row, uint32_t column, netsnmp_variable_list *pvb);
    ZQSNMP_STATUS Set(uint32_t row, uint32_t column, const netsnmp_variable_list *pvb);
    ZQSNMP_STATUS NextItem(uint32_t row, uint32_t column, uint32_t *pRow, uint32_t *pColumn);
private:
    Variable * getRow(uint32_t row);
    bool exist(uint32_t row, uint32_t column);
    uint32_t minRow();
    uint32_t maxRow();
    uint32_t minColumn();
    uint32_t maxColumn();

private:
    // std::vector< Variable > m_variables;
	
	typedef std::map<int, Variable> VariableMap;
    VariableMap _variableMap;
	int _lastOid;
};

#endif 

