#if !defined(AFX_DODAPPSERVICE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)
#define AFX_DODAPPSERVICE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
 #include "baseSchangeServiceApplication.h"
#include <Ice/Ice.h>
#include <string>
#include <vector>
#include <dodapp.h>
#include <datastream.h>
#define MAXNAMELEN						256
class DODAppSVC : public ZQ::common::BaseSchangeServiceApplication 
{
public:

	DODAppSVC();
	virtual ~DODAppSVC();
	
public:
	
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	
private:
	bool  ReadProperties();
};

#endif // !defined(AFX_DODAPPSERVICE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)