// TestService.h: interface for the TestService class.
//
//////////////////////////////////////////////////////////////////////

#ifndef   _ZQSERVICETEST_H
#define   _ZQSERVICETEST_H

#include "BaseZQServiceApplication.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TestService : public ZQ::common::BaseZQServiceApplication  
{
public:
	TestService();
	virtual ~TestService();
protected:
	
	HRESULT OnInit(void);
	HRESULT OnStop(void);
	HRESULT OnPause(void);
	HRESULT OnContinue(void);
	bool    isHealth(void);
	HRESULT OnStart(void);
	HRESULT OnUnInit(void);
	
};

#endif // !defined(AFX_TESTSERVICE_H__482C297C_B4F6_4E93_AC6E_36223C7FE7BD__INCLUDED_)
