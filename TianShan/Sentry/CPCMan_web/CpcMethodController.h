// CpcMethodController.h: interface for the CpcMethodController class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include "../httpdInterface.h"
// #include "TsContentProv.h"
#include "BasePage.h"


class CpcMethodController: public ContentProvisionClusterweb::BasePage 
{
public:
    CpcMethodController(IHttpRequestCtx *pHttpRequestCtx);
	~CpcMethodController();

protected: 
	virtual bool get();
	virtual bool post();

private:
	int methodcount;
};

