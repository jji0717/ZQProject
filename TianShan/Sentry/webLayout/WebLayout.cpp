// WebLayout.cpp : Defines the entry point for the DLL application.
//

#include "StdAfx.h"
#include <Log.h>
#include "../httpdInterface.h"
#include "SnmpRequest.h"
#include "GridRequest.h"
#include "LogRequest.h"
#include "ConsoleCommand.h"
using namespace ::ZQTianShan::Layout;

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

extern "C"
{
__EXPORT bool LibInit(ZQ::common::Log* pLog)
{
	ZQ::common::setGlogger(pLog);
	return true;
}

__EXPORT void LibUninit( )
{
	ZQ::common::setGlogger();
}

__EXPORT bool SnmpPage(IHttpRequestCtx *pHttpRequestCtx)
{
    SnmpRequest snmprqst(pHttpRequestCtx);
    return snmprqst.process();
}
__EXPORT bool GridPage(IHttpRequestCtx *pHttpRequestCtx)
{
    GridRequest gridrqst(pHttpRequestCtx);
    return gridrqst.process();
}
__EXPORT bool LogPage(IHttpRequestCtx *pHttpRequestCtx)
{
    LogRequest logrqst(pHttpRequestCtx);
    return logrqst.process();
}
#define CMD_VAR_CONTENT      "content"
#define CMD_VAR_CONTENT_TEXT "text"
#define CMD_VAR_CONTENT_HTML "html"
__EXPORT bool CmdPage(IHttpRequestCtx *pHttpRequestCtx)
{
    if(NULL == pHttpRequestCtx)
        return false;

    const char* cmdline = pHttpRequestCtx->GetRequestVar("cmd#cmdline");
    if(NULL == cmdline)
        return false;
    const char* content = pHttpRequestCtx->GetRequestVar(CMD_VAR_CONTENT);
#ifdef ZQ_OS_MSWIN
    if(content && 0 == stricmp(content, CMD_VAR_CONTENT_HTML))
#else
    if(content && 0 == strcasecmp(content, CMD_VAR_CONTENT_HTML))
#endif
    {
        IHttpResponse &out = pHttpRequestCtx->Response();
        ConsoleCommand::execute(out, NULL, cmdline);
    }
    else
    {
        IHttpResponse &out = pHttpRequestCtx->Response();
        out << "<div class='vtbl_title'><span>&gt; " << cmdline << "</span></div>";
        out << "<div class='vtbl_content'>";
        out << "<pre>";
        ConsoleCommand::execute(out, NULL, cmdline);
        out << "</pre></div>";
    }

    return true;
}
}
