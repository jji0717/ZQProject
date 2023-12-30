// VarInfoTable.cpp: implementation of the VarInfoTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VarInfoTable.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VarInfoTable::VarInfoTable()
{

}

VarInfoTable::~VarInfoTable()
{

}

void VarInfoTable::Add(const Variable &variable, UINT instId)
{
    // step 1: save the variable
    if(instId == 0) // autoassigned
    {
        _autoVars.push_back(variable);
    }
    else // preassigned
    {
        if(!_fixedVars.insert(std::make_pair<UINT, Variable>(instId, variable)).second)
        {
            // dup assigned id?
            // this should be fixed at compile time
            SMLOG(ZQ::common::Log::L_EMERG, "assign %s and %s with same instance id %u.", variable.getName().c_str(), _fixedVars[instId].getName().c_str(), instId);
            throw "Duplicate instance id for preassigned variables.";
        }
    }

    // step 2: index the variables
    _indices.clear();
    _seekingIndices.clear();
    // assign instance id for the auto variables
    // insert the auto variables into the accessing indices with assigned id
    UINT nextId = ZQSNMP_OID_VARINSTANCE_MIN; // next instance id to be assigned
    for(std::list<Variable>::iterator itAuto = _autoVars.begin(); itAuto != _autoVars.end(); ++itAuto)
    {
        while(_fixedVars.find(nextId) != _fixedVars.end())
        { // select another id
            ++nextId;
        }
        _seekingIndices.push_back(nextId);

        Index idx;
        idx.v = &(*itAuto);
        idx.next = 0;
        _indices[nextId] = idx; // save the index

        ++nextId;
    }
    // add the preassigned variables
    for(std::map<UINT, Variable>::iterator itFixed = _fixedVars.begin(); itFixed != _fixedVars.end(); ++itFixed)
    {
        _seekingIndices.push_back(itFixed->first);

        Index idx;
        idx.v = &(itFixed->second);
        idx.next = 0;
        _indices[itFixed->first] = idx;
    }

    if(!_seekingIndices.empty())
    {
        // sort the instance ids
        std::sort(_seekingIndices.begin(), _seekingIndices.end(), std::less<UINT>());

        // build the indices
        for(size_t i = 0; i < _seekingIndices.size() - 1; ++i)
        {
            _indices[_seekingIndices[i]].next = _seekingIndices[i + 1];
        }
    }
}

ZQSNMP_STATUS VarInfoTable::Get(UINT row, UINT column, AsnAny *pValue)
{
    if(NULL == pValue)
        return ZQSNMP_E_FAILURE;

    if(!exist(row, column))
        return ZQSNMP_E_NOSUCHNAME;// no such item

    Variable *pVarEntry = getRow(row);
    if(pVarEntry)
    {
        return pVarEntry->Get(column, pValue);
    }
    else
    {
        return ZQSNMP_E_FAILURE;
    }
}

ZQSNMP_STATUS VarInfoTable::Set(UINT row, UINT column, const AsnAny *pValue)
{
    if(NULL == pValue)
        return ZQSNMP_E_FAILURE;

    if(!exist(row, column))
        return ZQSNMP_E_NOSUCHNAME;// no such item

    Variable *pVarEntry = getRow(row);
    if(pVarEntry)
    {
        return pVarEntry->Set(column, pValue);
    }
    else
    {
        return ZQSNMP_E_FAILURE;
    }
}
ZQSNMP_STATUS VarInfoTable::NextItem(UINT row, UINT column, UINT *pRow, UINT *pColumn)
{
    if(_indices.empty()) // no item in the table, rare case
    {
        return ZQSNMP_E_NOSUCHNAME;
    }

    //accord with SNMP MIB table
    //in the lexicographical ordering of column.row

    // check the column
    if(column < ZQSNMP_OID_VARINFOTYPE_MIN) // before the data area
    { // next item is the first item in the table
        *pRow = nextRow(0); // first row
        *pColumn = nextColumn(0); //first column
        return ZQSNMP_E_NOERROR;
    }
    else if( ZQSNMP_OID_VARINFOTYPE_MAX < column) // out of region
    {
        return ZQSNMP_E_NOSUCHNAME;
    }
    else // column is valid
    {
        row = nextRow(row); // step to next position in the column
        if(0 == row) // end of the column, jump to the next.
        {
            column = nextColumn(column);
            if(0 == column)
            { // end of the region
                return ZQSNMP_E_NOSUCHNAME;
            }
            row = nextRow(0);
        }

        // get the position now
        *pRow = row;
        *pColumn = column;
        return ZQSNMP_E_NOERROR;
    }
}

Variable * VarInfoTable::getRow(UINT row)
{
    Indices::iterator it = _indices.find(row);
    return (it != _indices.end()) ? it->second.v : NULL;
}
bool VarInfoTable::exist(UINT row, UINT column)
{
    if( (_indices.find(row) != _indices.end()) // row exist
        &&
        ((ZQSNMP_OID_VARINFOTYPE_MIN <= column) && (column <= ZQSNMP_OID_VARINFOTYPE_MAX)) // column valid
        )
    {
        return true;
    }
    return false;
}

UINT VarInfoTable::nextRow(UINT row)
{
    Indices::iterator it = _indices.find(row);
    if(it != _indices.end())
    {
        return it->second.next;
    }
    else // not a valid row
    { // seek through all the variables
        for(size_t i = 0; i < _seekingIndices.size(); ++i)
        {
            if(row < _seekingIndices[i])
                return _seekingIndices[i];
        }
        return 0;
    }
}

UINT VarInfoTable::nextColumn(UINT column)
{
    if(column < ZQSNMP_OID_VARINFOTYPE_MIN)
        return ZQSNMP_OID_VARINFOTYPE_MIN;
    else
    {
        ++column;
        if((ZQSNMP_OID_VARINFOTYPE_MIN <= column) && (column <= ZQSNMP_OID_VARINFOTYPE_MAX))
            return column;
        else
            return 0; // not valid
    }
}
