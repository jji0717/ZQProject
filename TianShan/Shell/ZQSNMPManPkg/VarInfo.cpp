#include <cstdlib>
#include <cstring>
#include "VarInfo.h"

#ifndef MAPSET
#  define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAPSET

VarInfoTable::VarInfoTable()
: _lastOid(1)
{
}

VarInfoTable::~VarInfoTable()
{
}

void VarInfoTable::Add(const Variable &variable, int subOid)
{
//    m_variables.push_back(variable);
	if (subOid<=0)
		subOid = _lastOid++;
	else if (_lastOid < subOid)
		_lastOid = subOid;

	MAPSET(VariableMap, _variableMap, subOid, variable);
}

ZQSNMP_STATUS VarInfoTable::Get(uint32_t row, uint32_t column, netsnmp_variable_list *pvb)
{
    if(NULL == pvb)
        return ZQSNMP_E_FAILURE;

    if(!exist(row, column))
        return ZQSNMP_E_NOSUCHNAME;// no such item

    Variable *pVarEntry = getRow(row);
    if (pVarEntry)
        return pVarEntry->Get(column, pvb);

	return ZQSNMP_E_FAILURE;
}

ZQSNMP_STATUS VarInfoTable::Set(uint32_t row, uint32_t column, const netsnmp_variable_list *pvb)
{
	if(!pvb)
		return ZQSNMP_E_FAILURE;

	if(!exist(row, column))
		return ZQSNMP_E_NOSUCHNAME;// no such item

	Variable *pVarEntry = getRow(row);
	if(pVarEntry)
		return pVarEntry->Set(column, pvb);

	return ZQSNMP_E_FAILURE;
}

ZQSNMP_STATUS VarInfoTable::NextItem(uint32_t row, uint32_t column, uint32_t *pRow, uint32_t *pColumn)
{
    //accord with SNMP MIB table
    //in the lexicographical ordering of column.row
    if(column < minColumn()) 
	{
		//next item is the first item in the table
        column = minColumn();
        row = minRow();
    }
    else if(row < minRow())
	{
		//next item is the first item in the column
        row = minRow();
    }
    else if((++row) > maxRow()) 
	{
		//next item is the first item in the next column
        ++column;
        row = minRow();
    }
    else 
	{
        //next item is the next item in the column
        //nothing to do here
    }

    if(!exist(row, column))
        return ZQSNMP_E_NOSUCHNAME;

    *pRow = row;
    *pColumn = column;

    return ZQSNMP_E_NOERROR;
}

Variable * VarInfoTable::getRow(uint32_t row)
{
    if (!exist(row, minColumn()))
        return NULL;

    int entryIdx = row - minRow(); //always be valid

    // TODO: what such a 2B coding here, did Xiao really know how net-snmp was?
	// return &(m_variables.at(entryIdx));
	if (entryIdx >= _variableMap.size())
		return NULL;
	VariableMap::iterator it = _variableMap.begin();
	while(entryIdx-->0) it++;
	return &(it->second);
}

bool VarInfoTable::exist(uint32_t row, uint32_t column)
{
    if(row < minRow() || row > maxRow() || column < minColumn() || column > maxColumn())
        return false;
    return true;
}

uint32_t VarInfoTable::minRow()
{
    return ZQSNMP_OID_VARINSTANCE_MIN;
}

uint32_t VarInfoTable::maxRow()
{
    // return (ZQSNMP_OID_VARINSTANCE_MIN + m_variables.size() - 1);
    return (ZQSNMP_OID_VARINSTANCE_MIN + _variableMap.size() - 1);
}

uint32_t VarInfoTable::minColumn()
{
    return ZQSNMP_OID_VARINFOTYPE_MIN;
}

uint32_t VarInfoTable::maxColumn()
{
    return ZQSNMP_OID_VARINFOTYPE_MAX;
}
