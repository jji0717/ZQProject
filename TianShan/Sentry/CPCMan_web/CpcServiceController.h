// CpcServiceController.h: interface for the CpcServiceController class.
//
//////////////////////////////////////////////////////////////////////


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include "../httpdInterface.h"
// #include "TsContentProv.h"
#include "BasePage.h"

#define SessionCountKey "up#SessionCount"

class CpcServiceController : public ContentProvisionClusterweb::BasePage 
{
public:
    CpcServiceController(IHttpRequestCtx *pHttpRequestCtx);
	~CpcServiceController();

protected: 
	virtual bool get();
	virtual bool post();

private:
	unsigned int _sessCount;
	unsigned int _curPage;
	unsigned int _pageCount;
	unsigned int _sessionNumberPerPage;
	unsigned int _linkNumberPerPage;

};

