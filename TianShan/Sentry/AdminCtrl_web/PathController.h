// PathController.h: interface for the PathController class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATHCONTROLLER_H__4F20C0A6_157F_4096_97EA_7783E409B2B1__INCLUDED_)
#define AFX_PATHCONTROLLER_H__4F20C0A6_157F_4096_97EA_7783E409B2B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../httpdInterface.h"
#include "TsPathAdmin.h"

class PathController  
{
public:
    PathController(IHttpRequestCtx *pHttpRequestCtx);
	~PathController();

    bool ServiceGroupPage();
    bool StoragePage();
    bool StreamerPage();
    bool StorageLinkPage();
    bool StreamLinkPage();
	bool StreamLinkBySGIdPage();
    bool TransportMapPage();
    bool TransportConfPage();
private:
    bool init();
    void uninit();
private:
    const char *_template;
    const char *_rootDir;
    const char *_endpoint;
    Ice::CommunicatorPtr _ic;
    ::TianShanIce::Transport::PathAdminPrx _pa;
private:
    IHttpRequestCtx *_pHttpRequestCtx;
};

#endif // !defined(AFX_PATHCONTROLLER_H__4F20C0A6_157F_4096_97EA_7783E409B2B1__INCLUDED_)
