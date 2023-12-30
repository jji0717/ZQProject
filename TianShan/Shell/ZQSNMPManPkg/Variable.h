// Variable.h: interface for the Variable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VARIABLE_H__2D0E7945_D468_491D_810B_6A8AD998F2C2__INCLUDED_)
#define AFX_VARIABLE_H__2D0E7945_D468_491D_810B_6A8AD998F2C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>

class Variable  
{
public:
    Variable();
    Variable(const std::string &name, void *address, DWORD type, DWORD access);
	~Variable();
	ZQSNMP_STATUS Set(DWORD infoType, const AsnAny *pValue);
    ZQSNMP_STATUS Get(DWORD infoType, AsnAny *pValue) const;

    const std::string& getName() const; // for logging purpose
private:
    std::string m_name;
    void *m_address;
    DWORD m_type;
    DWORD m_access;
};

#endif // !defined(AFX_VARIABLE_H__2D0E7945_D468_491D_810B_6A8AD998F2C2__INCLUDED_)
