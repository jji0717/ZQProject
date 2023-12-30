// NewTest.h: interface for the NewTest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWTEST_H__31A1CC36_AF78_47AE_96D2_BCEBBCC0CFB1__INCLUDED_)
#define AFX_NEWTEST_H__31A1CC36_AF78_47AE_96D2_BCEBBCC0CFB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSAdmin.h"

class NewTest : public SSAdmin  
{
public:
	NewTest();
	virtual ~NewTest();
private:
	int Test(VARVEC& var)
	{
		return 1;
	}
private:
	DECLEAR_CMDROUTE();
};

#endif // !defined(AFX_NEWTEST_H__31A1CC36_AF78_47AE_96D2_BCEBBCC0CFB1__INCLUDED_)
