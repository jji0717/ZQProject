// LogRequest.cpp: implementation of the LogRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "LogRequest.h"
#include <Log.h>
#include <urlstr.h>
#include "ConsoleCommand.h"

#define LOG_MODULE_NAME         "LogPage"
using namespace ::ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define LOGREQUEST_VAR_FILENAME     "log#filename"
#define LOGREQUEST_VAR_FILETYPE     "log#filetype"
#define LOGREQUEST_VAR_PAGESIZE     "log#pagesize"
#define LOGREQUEST_VAR_PAGEIDX      "log#pageidx"

LogRequest::LogRequest(IHttpRequestCtx *pHttpRequestCtx)
:_pHttpRequestCtx(pHttpRequestCtx)
{
}

LogRequest::~LogRequest()
{
}

bool LogRequest::init()
{
    //template
    _template = _pHttpRequestCtx->GetRequestVar(WLREQUEST_VAR_TEMPLATE);
    if(NULL == _template){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "template name if missing in the request."));
        return false;
    }
    //uri
    _uri = _pHttpRequestCtx->GetUri();
    if(NULL == _uri){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "uri string if missing in the request."));
        return false;
    }
    //datasource
    _datasource = _pHttpRequestCtx->GetRequestVar(WLREQUEST_VAR_DATASOURCE);
    if(NULL == _datasource){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "dll path is missing in the request."));
        return false;
    }
    //target log file name
    _filename = _pHttpRequestCtx->GetRequestVar(LOGREQUEST_VAR_FILENAME);
    if(NULL == _filename){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "target log file name is missing in the request."));
        return false;
    }
    //log file type
    _filetype = _pHttpRequestCtx->GetRequestVar(LOGREQUEST_VAR_FILETYPE);
    if(NULL == _filetype){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "log file type is missing in the request."));
        return false;
    }
    //log file page size
    _pagesize = _pHttpRequestCtx->GetRequestVar(LOGREQUEST_VAR_PAGESIZE);
    if(NULL == _pagesize){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "log file page size is missing in the request."));
        return false;
    }
    //log file page index
    _pageidx = _pHttpRequestCtx->GetRequestVar(LOGREQUEST_VAR_PAGEIDX);
    if(NULL == _pageidx){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "log file page index is missing in the request."));
        return false;
    }
    return true;
}

static std::string int2str(int i)
{
    char buf[12] = {0};
    return itoa(i, buf, 10);
}

bool LogRequest::process()
{
    if(NULL == _pHttpRequestCtx)
        return false;
    if(!init())
        return false;

    //////////////////////////////////////////////////////////////////////////
    //format
    IHttpResponse &out = _pHttpRequestCtx->Response();
    out << "<div class='vtbl_title' style='padding:10px 0'>" << _filename;
    int iCurPage = atoi(_pageidx);
    int iLastPage = 0;
    //count pages
    {
        //construct command line
#ifdef ZQ_OS_MSWIN
        std::string cmdline = std::string("\"") + _datasource + "\"";
#else
        std::string cmdline = _datasource;;
#endif
        cmdline += " -c ";
        cmdline += _pagesize;
#ifdef ZQ_OS_MSWIN
        cmdline += " -f \"";
#else
        cmdline += " -f ";
#endif
        cmdline += _filename;
#ifdef ZQ_OS_MSWIN
        cmdline += "\"";
#endif
        std::string pagescount;
        ZQTianShan::Layout::ConsoleCommand::execute(pagescount, NULL, cmdline.c_str());
        //last page
        iLastPage = atoi(pagescount.c_str());
    }
    
    //adjust page index
    if(iLastPage <= 0)
        iLastPage = 1;
    
    if(iCurPage == 0)
        iCurPage = 1;
    else if(iCurPage < 0 || iCurPage > iLastPage)
        iCurPage = iLastPage;

    
    // display control form
    out << "<script type=\"text/javascript\">\n"
        << "function toPage(pgidx){\n"
        << "document.getElementById(\"page-idx\").value = pgidx;\n"
        << "document.getElementById(\"log-form\").submit();\n"
        << "}\n</script>";
    
    out << "<form id=\"log-form\" method=get action=\"" << _uri << "\" style='display:inline;position:absolute;right:10%'>";
    //first page
    out << "<span class=\"lnk\" onclick=\"toPage(1)\"><img src='img/first_page.gif' alt='first'></span>&nbsp;";
    //previous page
    out << "&nbsp;<span class=\"lnk\" onclick=\"toPage(" << ((iCurPage > 1) ? (iCurPage - 1) : 1) << ")\"><img src='img/previous_page.gif' alt='previous'></span>&nbsp;";
    //current page
    out << "<input id=\"page-idx\" type=text size=5 style=\"text-align:center\"";
    out << " name=\"" << LOGREQUEST_VAR_PAGEIDX "\"";
    out << " value=\"" << iCurPage << "\">";
    //next page
    out << "&nbsp;<span class=\"lnk\" onclick=\"toPage(" << iCurPage + 1 << ")\"><img src='img/next_page.gif' alt='next'></span>&nbsp;";
    //last page
    out << "&nbsp;<span class=\"lnk\" onclick=\"toPage(" << iLastPage << ")\"><img src='img/last_page.gif' alt='last'></span>";
    //other variables
#define HTML_INPUT_HIDDEN(out, name, val) (out) << "<input type=hidden"\
    << " name=\"" << (name) << "\""\
    << " value=\"" << (val) << "\">";
    HTML_INPUT_HIDDEN(out, WLREQUEST_VAR_TEMPLATE, _template);          //template
    HTML_INPUT_HIDDEN(out, WLREQUEST_VAR_DATASOURCE, _datasource);      //datasource
    HTML_INPUT_HIDDEN(out, LOGREQUEST_VAR_FILENAME, _filename);         //target log file name
    HTML_INPUT_HIDDEN(out, LOGREQUEST_VAR_FILETYPE, _filetype);         //log file type
    HTML_INPUT_HIDDEN(out, LOGREQUEST_VAR_PAGESIZE, _pagesize);         //log page size
#undef HTML_INPUT_HIDDEN
    out << "</form>";
    out << "</div>";
    //display log content
    {
        //construct command line
#ifdef ZQ_OS_MSWIN
        std::string cmdline = std::string("\"") + _datasource + "\"";
#else
        std::string cmdline = _datasource;
#endif
        cmdline += " -s ";
        cmdline += _pagesize;
        cmdline += " -p ";
        cmdline += int2str(iCurPage);
#ifdef ZQ_OS_MSWIN
        cmdline += " -f \"";
#else
        cmdline += " -f ";
#endif
        cmdline += _filename;
#ifdef ZQ_OS_MSWIN
        cmdline += "\" -t ";
#else
        cmdline += " -t ";
#endif
        cmdline += _filetype;
        
        out << "<div class='vtbl_content'><pre class='chunk'>";
        ZQTianShan::Layout::ConsoleCommand::execute(out, NULL, cmdline.c_str());
        out << "</pre></div>";
    }

    return true;
}

