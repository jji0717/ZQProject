// SessionController.h: interface for the SessionController class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SITECONTROLLER_H__BAC06594_3FED_49E3_81B4_A87B836A13FA__INCLUDED_)
#define AFX_SITECONTROLLER_H__BAC06594_3FED_49E3_81B4_A87B836A13FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include "../httpdInterface.h"
// #include "TsContentProv.h"
#include "BasePage.h"

#define SessionCountKey "up#SessionCount"

class SessionController : public ContentProvisionweb::BasePage 
{
public:
    SessionController(IHttpRequestCtx *pHttpRequestCtx);
	~SessionController();

protected: 
	virtual bool get();
	virtual bool post();

private:
	unsigned int _sessCount;
	unsigned int _pageCount;
	unsigned int _sessionNumberPerPage;
	unsigned int _linkNumberPerPage;

};

#endif // !defined(AFX_SITECONTROLLER_H__BAC06594_3FED_49E3_81B4_A87B836A13FA__INCLUDED_)
