// VarInfoTable.h: interface for the VarInfoTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VARINFOTABLE_H__C82D17CE_1B3C_4149_9110_52B968F445DF__INCLUDED_)
#define AFX_VARINFOTABLE_H__C82D17CE_1B3C_4149_9110_52B968F445DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Variable.h"
#include <vector>
#include <list>
#include <map>

class VarInfoTable  
{
public:
	VarInfoTable();
	~VarInfoTable();
    void Add(const Variable &variable, UINT instId);
    ZQSNMP_STATUS Get(UINT row, UINT column, AsnAny *pValue);
    ZQSNMP_STATUS Set(UINT row, UINT column, const AsnAny *pValue);
    ZQSNMP_STATUS NextItem(UINT row, UINT column, UINT *pRow, UINT *pColumn);
private:
    Variable * getRow(UINT row);
    bool exist(UINT row, UINT column);
    UINT nextRow(UINT row);
    UINT nextColumn(UINT column);
private:
    std::list<Variable> _autoVars; // variables with autoassigned instance id
    std::map<UINT, Variable> _fixedVars; // variables with preassigned instance id

    struct Index
    {
        Variable *v; // variable reference
        UINT next; // next instance id, 0 for last
    };

    typedef std::map<UINT, Index> Indices;
    Indices _indices; // the indices for accessing variables

    std::vector<UINT> _seekingIndices; // the indices for seeking variable in the table
};

#endif // !defined(AFX_VARINFOTABLE_H__C82D17CE_1B3C_4149_9110_52B968F445DF__INCLUDED_)
