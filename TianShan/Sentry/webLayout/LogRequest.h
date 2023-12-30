// LogRequest.h: interface for the LogRequest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGREQUEST_H__2E39D8E2_BBC2_41E0_813A_174D92780B72__INCLUDED_)
#define AFX_LOGREQUEST_H__2E39D8E2_BBC2_41E0_813A_174D92780B72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../httpdInterface.h"

class LogRequest  
{
public:
	LogRequest(IHttpRequestCtx *pHttpRequestCtx);
	~LogRequest();

    bool process();
private:
    bool init();

private:
    //page info
    const char* _template;
    const char* _uri;
    //.exe file
    const char* _datasource;
    //cmdline info
    const char* _filename;
    const char* _filetype;
    const char* _pagesize;
    const char* _pageidx;
    std::string _cmdline;
private:
    IHttpRequestCtx *_pHttpRequestCtx;
};

#endif // !defined(AFX_LOGREQUEST_H__2E39D8E2_BBC2_41E0_813A_174D92780B72__INCLUDED_)
