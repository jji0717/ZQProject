// SiteController.h: interface for the SiteController class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SITECONTROLLER_H__BAC06594_3FED_49E3_81B4_A87B836A13FA__INCLUDED_)
#define AFX_SITECONTROLLER_H__BAC06594_3FED_49E3_81B4_A87B836A13FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../httpdInterface.h"
#include "SiteAdminSvc.h"

class SiteController  
{
public:
    SiteController(IHttpRequestCtx *pHttpRequestCtx);
	~SiteController();

	bool MountPage();
	bool AppPage();
    bool SitePage();
    bool TxnPage();
private:
    bool init();
    void uninit();
private:
    const char *_template;
    const char *_rootDir;
    const char *_endpoint;
    Ice::CommunicatorPtr _ic;
    ::TianShanIce::Site::SiteAdminPrx _sa;
private:
    IHttpRequestCtx *_pHttpRequestCtx;
};

#endif // !defined(AFX_SITECONTROLLER_H__BAC06594_3FED_49E3_81B4_A87B836A13FA__INCLUDED_)
