// MethodController.h: interface for the MethodController class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include "../httpdInterface.h"
// #include "TsContentProv.h"
#include "BasePage.h"


class MethodController: public ContentProvisionweb::BasePage 
{
public:
    MethodController(IHttpRequestCtx *pHttpRequestCtx);
	~MethodController();

protected: 
	virtual bool get();
	virtual bool post();

private:
	int methodcount;
};
