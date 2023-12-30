// GridRequest.cpp: implementation of the GridRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GridRequest.h"
#include "DataSourceLoader.h"
#include <urlstr.h>
#include <Log.h>
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <dlfcn.h>
}
#endif

#define LOG_MODULE_NAME         "GridPage"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define GRIDREQUEST_VARPREFIX   "grid#"

namespace ZQTianShan
{
namespace Layout
{
using namespace ::ZQ::common;

GridRequest::GridRequest(IHttpRequestCtx *pHttpRequestCtx)
:_pHttpRequestCtx(pHttpRequestCtx)
{
}

GridRequest::~GridRequest()
{
}

bool GridRequest::init()
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
    //entry
    _entry = _pHttpRequestCtx->GetRequestVar(WLREQUEST_VAR_ENTRY);
    if(NULL == _entry){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "dll entry is missing in the request."));
        return false;
    }
    //logfilepath
    _logfilepath = _pHttpRequestCtx->GetRequestVar(WLREQUEST_VAR_LOGFILEPATH);
    if(NULL == _logfilepath){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "log file path is missing in the request."));
        return false;
    }
    return true;
}

bool GridRequest::prepareLayoutCtx()
{
    m_KeyValue.clear();
    m_ColumnName.clear();
    m_tableData.clear();

    //init request variables
    IHttpRequestCtx::RequestVars vars;
    _pHttpRequestCtx->GetRequestVars().swap(vars);
    //gather information from request vars
    std::string varPrefix = GRIDREQUEST_VARPREFIX;
    IHttpRequestCtx::RequestVars::const_iterator cit;
    for(cit = vars.begin(); cit != vars.end(); ++cit)
    {
        if(cit->first.compare(0, varPrefix.size(), varPrefix))
            continue;//ignore other variable

        lvalue val;
		memset(&val,0,sizeof(val));
        if(sizeof(val.value) <= cit->second.size()) //no enough room, how to deal with this?
            continue;//ignore

        strcpy(val.value, cit->second.c_str());
        set(cit->first.c_str(), val);
    }
    return true;
}
bool GridRequest::process()
{
    if(NULL == _pHttpRequestCtx)
        return false;
    //gather information
    if(!init())
        return false;
    //call dll proc
    DataSourceLoader dsloader(_datasource, _logfilepath);
#ifdef ZQ_OS_MSWIN
    HMODULE hDll = dsloader.DllHandle();
    if(NULL == hDll){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to init dll. [dllpath = %s], [logfilepath = %s]"), _datasource, _logfilepath);
        return false;
    }
    EntryFunc_FillGrid gridProc = (EntryFunc_FillGrid)GetProcAddress(hDll, _entry);
#else
	void* hDll = dsloader.DllHandle();
    if(NULL == hDll){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to init dll. [dllpath = %s], [logfilepath = %s]"),
			 _datasource, _logfilepath);
        return false;
    }
    EntryFunc_FillGrid gridProc = (EntryFunc_FillGrid)dlsym(hDll, _entry);

#endif

    if(NULL == gridProc){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "can't find entry procedure in dll. [dllpath = %s], [entry = %s]"), _datasource, _entry);
        return false;
    }
    //init layout context
    if(!prepareLayoutCtx())
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to init layout context."));
        return false;
    }
    try{
        gridProc(this);
    }catch(...){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch unexpected exception during the dll procedure call.[dllpath = %s], [entry = %s]"), _datasource, _entry);
        return false;
    }
    if(!format()){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter error during format data and output."));
        return false;
    }
    return true;
}
bool GridRequest::format()
{
    //format data and send out
    IHttpResponse &out = _pHttpRequestCtx->Response();

    out << "<table class='listTable'>";

    out << "<tr class='heading'>";
    for(size_t iCol = 0; iCol < m_ColumnName.size(); ++iCol)
    {
        out << "<th>";
        out << m_ColumnName[iCol];
        out << "</th>";
    }
    out << "</tr>";

    //setup ref url prefix
    ZQ::common::URLStr hrefprefix(NULL, true);//always case sensitive
    hrefprefix.setPath(_uri);
    hrefprefix.setVar(WLREQUEST_VAR_TEMPLATE, _template);
    hrefprefix.setVar(WLREQUEST_VAR_DATASOURCE, _datasource);
    hrefprefix.setVar(WLREQUEST_VAR_ENTRY, _entry);
    hrefprefix.setVar(WLREQUEST_VAR_LOGFILEPATH, _logfilepath);

    std::string urlhead = std::string(_uri) + "?";//for locate url start position
    for(size_t iRow = 0; iRow < m_tableData.size(); ++iRow)
    {
        bool bWithRef = (!m_tableData[iRow]._ref.empty());
        if(bWithRef)
        {
            out << "<tr class=\"lnk\">";
            ZQ::common::URLStr ref(NULL, true);//always case sensitive
            if(ref.parse(m_tableData[iRow]._ref.c_str()))
            {
                //merge ref vars
                ZQ::common::URLStr href(hrefprefix);
                int i = 0;
                const char *varname = NULL;
                while((varname = ref.getVarname(i++)))
                {
                    const char* varval = ref.getVar(varname);
                    href.setVar(varname, varval);
                }
                //generate url string
                const char* urlstr = strstr(href.generate(), urlhead.c_str());
                out << "<a href=\"" << urlstr << "\">";
            }
            else //bad ref string
            {
                bWithRef = false;
            }
        }
        else
        {
            out << "<tr>";
        }
        for(size_t iCol = 0; iCol < m_tableData[iRow]._row.size(); ++iCol)
        {
            out << "<td>";
            out << (m_tableData[iRow]._row)[iCol];
            out << "</td>";
        }
        if(bWithRef)
            out << "</a>";
        out << "</tr>";
    }
    out << "</table>";

    return true;
}

}}//namespace ZQTianShan::Layout
