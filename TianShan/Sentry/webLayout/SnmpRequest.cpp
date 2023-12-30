// SnmpRequest.cpp: implementation of the SnmpRequest class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SnmpRequest.h"
#include "DataSourceLoader.h"
#include <Log.h>
#include <urlstr.h>
#include <utility>
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <dlfcn.h>
}
#endif

#define LOG_MODULE_NAME     "SnmpPage"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define SNMPREQUEST_VAR_BASEOID     "snmp#baseoid"
#define SNMPREQUEST_VAR_SERVERIP    "snmp#serverip"
#define SNMPREQUEST_VAR_COMMUNITY   "snmp#community"

#define SNMP_TAGPREFIX_INDEX        "snmp.idx#"
#define SNMP_TAGPREFIX_NAME         "snmp.name#"
#define SNMP_TAGPREFIX_VALUE        "snmp.val#"
#define SNMP_TAGPREFIX_OID          "snmp.oid#"
#define SNMP_TAGPREFIX_TYPE         "snmp.type#"
#define SNMP_TAGPREFIX_OLDVALUE     "snmp.oldval#"

// extend for ngod
#define SNMPREQUEST_VAR_MODE        "mode"
#define SNMPREQUEST_VAL_MODE_RAW    "raw"    
#define SNMPREQUEST_VAL_MODE_NGODRO "ngod.ro"
#define SNMPREQUEST_VAL_MODE_NGODRW "ngod.rw"

#define SNMPREQUEST_VAR_AUTHURL     "auth"

#define SNMPREQUEST_VAR_FILTER      "filter"
namespace ZQTianShan
{
namespace Layout
{
using namespace ::ZQ::common;

SnmpRequest::SnmpRequest(IHttpRequestCtx *pHttpRequestCtx)
:_pHttpRequestCtx(pHttpRequestCtx)
{
}

SnmpRequest::~SnmpRequest()
{
}

bool SnmpRequest::init()
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
    //base oid
    _baseoid = _pHttpRequestCtx->GetRequestVar(SNMPREQUEST_VAR_BASEOID);
    if(NULL == _baseoid){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "base oid is missing in the request."));
        return false;
    }
    //serverip
    _serverip = _pHttpRequestCtx->GetRequestVar(SNMPREQUEST_VAR_SERVERIP);
    if(NULL == _serverip){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "server ip address is missing in the request."));
        return false;
    }
    //community
    _community = _pHttpRequestCtx->GetRequestVar(SNMPREQUEST_VAR_COMMUNITY);
    if(NULL == _community){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "community is missing in the request."));
        return false;
    }

    // mode 
    _mode = _pHttpRequestCtx->GetRequestVar(SNMPREQUEST_VAR_MODE); // extend for ngod

    // authorization
    _authURL = _pHttpRequestCtx->GetRequestVar(SNMPREQUEST_VAR_AUTHURL);

    // filter
    _filter = _pHttpRequestCtx->GetRequestVar(SNMPREQUEST_VAR_FILTER);
    return true;
}
bool SnmpRequest::prepareLayoutCtx()
{
    m_KeyValue.clear();

    //init request variables
    IHttpRequestCtx::RequestVars vars;
    _pHttpRequestCtx->GetRequestVars().swap(vars);
    //gather information from request vars
    std::string varIdxTagPrefix = SNMP_TAGPREFIX_INDEX;
    IHttpRequestCtx::RequestVars::const_iterator cit;
    for(cit = vars.begin(); cit != vars.end(); ++cit)
    {
        if(cit->first.compare(0, varIdxTagPrefix.size(), varIdxTagPrefix))
            continue;//ignore
        std::string idx = cit->second;
        ZQTianShan::Layout::ILayoutCtx::lvalue snmpvar;
		memset(&snmpvar,0,sizeof(snmpvar));
        
        IHttpRequestCtx::RequestVars::const_iterator citvar;
        //value
        citvar = vars.find(std::string(SNMP_TAGPREFIX_VALUE) + idx);
        if(vars.end() == citvar || SNMPATTR_VARVALUE_MAXLEN <= citvar->second.size())
            continue;
        //check if value changed
        strcpy(snmpvar.value, citvar->second.c_str());
        citvar = vars.find(std::string(SNMP_TAGPREFIX_OLDVALUE) + idx);
        if(vars.end() == citvar || 0 == citvar->second.compare(snmpvar.value))
            continue;

        //oid
        citvar = vars.find(std::string(SNMP_TAGPREFIX_OID) + idx);
        if(vars.end() == citvar || SNMPATTR_OID_MAXLEN <= citvar->second.size())
            continue;
        strcpy(snmpvar.oid, citvar->second.c_str());
        
        //type
        citvar = vars.find(std::string(SNMP_TAGPREFIX_TYPE) + idx);
        if(0 == citvar->second.compare("int"))
        {
            snmpvar.type = SNMPATTR_VARTYPE_INT;
        }
        else if(0 == citvar->second.compare("string"))
        {
            snmpvar.type = SNMPATTR_VARTYPE_STRING;
        }
        else
        {
            continue;//unknown type
        }
        snmpvar.readonly = SNMPATTR_VARRW_WRITABLE;
        snmpvar.modify   = SNMPATTR_VARVALUE_CHANGED;
        //name
        citvar = vars.find(std::string(SNMP_TAGPREFIX_NAME) + idx);
        if(vars.end() == citvar)
            continue;
        m_KeyValue.insert(std::make_pair(citvar->second, snmpvar));
    }
    return true;
}

bool SnmpRequest::process()
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
    EntryFunc_PopulateSnmpVariables snmpProc = (EntryFunc_PopulateSnmpVariables)GetProcAddress(hDll, _entry);
#else
    void* hDll = dsloader.DllHandle();
    if(NULL == hDll){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to init dll. [dllpath = %s], [logfilepath = %s]"),
			_datasource, _logfilepath);
        return false;
    }
    EntryFunc_PopulateSnmpVariables snmpProc = (EntryFunc_PopulateSnmpVariables)dlsym(hDll, _entry);
#endif
    if(NULL == snmpProc){
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
        snmpProc(this, _baseoid, true, _serverip, _community);
    }catch(...){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch unexpected exception during the dll procedure call.[dllpath = %s], [entry = %s], [baseoid = %s], [serverip = %s], [community = %s]")
                                                    ,_datasource, _entry, _baseoid, _serverip, _community);
        return false;
    }
    if(!format()){
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter error during format data and output."));
        return false;
    }
    return true;
}
#ifdef ZQ_OS_MSWIN
#define SNMP_HELPMSG    "<p><h2>Can't query variables via snmp.</h2></p>"\
                        "<h3>The reason may be:</h3>"\
                        "<dl>"\
                        "<p><dt>OS may not have SNMP service installed. </dt>"\
                        "<dd>Go to 'Control panel', 'Add/Remove Programs'. "\
                         "Click 'Add/Remove Windows components'. "\
                         "Check the checkbox 'Managerment and Monitoring Tools'. "\
                         "Then on Detail, check 'Simple Network Managerment Protocol'. "\
                         "Click Next and complete the installation.</dd></p>"\
                        "<p><dt>SNMP service may not be started.</dt>"\
                        "<dd>Start the SNMP service.</dd></p>"\
                        "<p><dt>SNMP service may not be configured properly.</dt>"\
                        "<dd>Several steps may be needed.<ul>"\
                        "<li>Register the SNMP service ZQ extension agent by adding an entry of type 'string' under "\
                         "'LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters\\ExtensionAgents'. "\
                         "Select a decimal number as the name and set 'SOFTWARE\\ZQ Interactive\\ZQSnmpExtension' as the value.</li>"\
                        "<li>Add a community with read/write privilege through the 'Services.msc', "\
                         "or by adding an entry of type 'dword' under "\
                         "'LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters\\ValidCommunities'. "\
                         "Select a community string as the name and set 8 as the value.</li>"\
                        "<li>Add '127.0.0.1' to the permitted manager list through the 'Services.msc', "\
                         "or by adding an entry of type 'string' under "\
                         "'LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\SNMP\\Parameters\\PermittedManagers'. "\
                         "Select a decimal number as the name and set '127.0.0.1' as the value.</li></ul>"\
                        "Restart the SNMP service after reconfiguration.</dd></p>"\
                        "<p><dt>'weblayout.xml' may not be configured properly.</dt>"\
                        "<dd>Configure the service OID with the values under "\
                         "'LOCAL_MACHINE\\SOFTWARE\\ZQ Interactive\\SNMPOID'. "\
                         "Configure a snmp community string with the read/write privilege. "\
                         "Restart the Sentry service after reconfiguration.</dd></p>"\
                        "</dl>"
#else // linux
#define SNMP_HELPMSG    "<p><h2>Can't query variables via snmp.</h2></p>"\
                        "<h3>The reason may be:</h3>"\
                        "<dl>"\
                        "<p><dt>OS may not have SNMP service installed. </dt>"\
                        "<dd>Install snmpd (please refer to <a href='http://net-snmp.sourceforge.net/'>net-snmp</a>).</dd></p>"\
                        "<p><dt>SNMP service may not be started.</dt>"\
                        "<dd>Start the snmpd(/etc/init.d/snmpd start).</dd></p>"\
                        "<p><dt>SNMP service may not be configured properly.</dt>"\
                        "<dd>Add these configuration to the /etc/snmp/snmpd.conf."\
                        "<ul>rwcommunity TianShan 127.0.0.1 .1.3.6.1.4.1.22839.4.1<br>dlmod SNMPAgent /opt/TianShan/bin/libSnmpAgent.so</ul>"\
                        "Restart snmpd after reconfiguration. </dd></p>"\
                        "<p><dt>'weblayout.xml' may not be configured properly.</dt>"\
                        "<dd>Configure the service OID with the values in the /etc/TianShan.xml. Configure a snmp community string with the read/write privilege. Restart the Sentry service after reconfiguration. </dd></p>"\
                        "</dl>"
#endif
static std::pair<std::string, std::string> parseVariableName(const std::string& varname)
{ // find out the group name
    std::pair<std::string, std::string> result;

    if(!varname.empty())
    {
        // check the start sign
        if(varname[0] == '#')
        {
            std::string::size_type groupEnd = varname.find('#', 1);
            if(groupEnd != std::string::npos)
            { // get the end sign
                result.first = varname.substr(1, groupEnd - 1);
                result.second = varname.substr(groupEnd + 1);
            }
            else // no end sign. treat as empty group
            {
                result.second = varname;
            }
        }
        else // empty group
        {
            result.second = varname; 
        }
    } // else : strange case
    return result;
}

static bool checkVarnameByFilter(const std::string& varname, const std::string& filter, bool exclude = false)
{
    if (filter.empty())
    {
        return false;
    }

    std::string filterName = "-" + filter;
    if (varname.find(filterName) != std::string::npos)
    {
        // filter
        return exclude;
    }
    return !exclude;
}

bool SnmpRequest::format()
{
    //format data and send out
    IHttpResponse &out = _pHttpRequestCtx->Response();
    if(m_KeyValue.empty())
    {
        out << SNMP_HELPMSG;
        return true;
    }

    std::string group; // filter
    bool bKeep = true; // keep or exclude
    {
        // filter the result
        if(_filter)
        {
            if(strlen(_filter) && (*_filter) == '-')
            {
                group = (_filter + 1);
                bKeep = false; // exclude the group
            }
            else
            {
                group = _filter;
            }
        } // else treat as empty group
    }
    enum { emUnknown, emRaw, emNGODRO, emNGODRW } mode = emUnknown;

    if(_mode)
    {
#ifdef ZQ_OS_MSWIN
        if(0 == stricmp(_mode, SNMPREQUEST_VAL_MODE_RAW))
            mode = emRaw;
        else if (0 == stricmp(_mode, SNMPREQUEST_VAL_MODE_NGODRO))
            mode = emNGODRO;
        else if (0 == stricmp(_mode, SNMPREQUEST_VAL_MODE_NGODRW))
            mode = emNGODRW;
#else
		if(0 == strcasecmp(_mode, SNMPREQUEST_VAL_MODE_RAW))
            mode = emRaw;
        else if (0 == strcasecmp(_mode, SNMPREQUEST_VAL_MODE_NGODRO))
            mode = emNGODRO;
        else if (0 == strcasecmp(_mode, SNMPREQUEST_VAL_MODE_NGODRW))
            mode = emNGODRW;
#endif
        else
            mode = emUnknown;
    }
    // treat unknown mode as raw mode
    if(emUnknown == mode)
        mode = emRaw;

    // build tool bar

    {
        out << "<script language='JavaScript'>\n"
            << "var toolbar = new Toolbar();\n";

        //---------- icon ----------
        switch(mode)
        {
        case emNGODRO:
            {
                if( _authURL && '\0' != (*_authURL))
                {
                    // add the modify button
                    out << "var icon0 = new Icon();\n"
                        << "icon0.setImageSrc('images/toolbar/modify.gif');\n"
                        << "icon0.setImageHoverSrc('images/toolbar/modify_over.gif');\n"
                        << "icon0.setLabel('Modify');\n"
                        << "icon0.setAction('modify()');\n"
                        << "icon0.setTips('Request to modify');\n";

                    out << "toolbar.addIcon(icon0);\n";

                    // construct the authorization page url
                    ZQ::common::URLStr rwURL(_pHttpRequestCtx->GetRootURL(), true); // case sensitive
                    rwURL.setPath(_uri);
                    rwURL.setVar(WLREQUEST_VAR_TEMPLATE, _template);          //template
                    rwURL.setVar(WLREQUEST_VAR_DATASOURCE, _datasource);      //datasource
                    rwURL.setVar(WLREQUEST_VAR_ENTRY, _entry);                //entry
                    rwURL.setVar(WLREQUEST_VAR_LOGFILEPATH, _logfilepath);    //logfilepath
                    rwURL.setVar(SNMPREQUEST_VAR_BASEOID, _baseoid);          //baseoid
                    rwURL.setVar(SNMPREQUEST_VAR_SERVERIP, _serverip);        //serverip
                    rwURL.setVar(SNMPREQUEST_VAR_COMMUNITY, _community);      //community
                    rwURL.setVar(SNMPREQUEST_VAR_FILTER, _filter);            // filter
                    rwURL.setVar(SNMPREQUEST_VAR_MODE, SNMPREQUEST_VAL_MODE_NGODRW);    //mode
                    rwURL.setVar(SNMPREQUEST_VAR_AUTHURL, _authURL);          // authorization

                    out << "function modify(){\n"
                        << "\twindow.location = '"
                        << _authURL; // the authorization url

                    char urlBuf[1024];
                    out << ((ZQ::common::URLStr::encode(rwURL.generate(), urlBuf, sizeof(urlBuf))) ? urlBuf : "")
                        << "';\n}\n";
                } // else : not need the modify button
            }
            break;
        case emNGODRW:
            {
                // 1. save
                out << "var icon0 = new Icon();\n"
                    << "icon0.setImageSrc('images/toolbar/save.gif');\n"
                    << "icon0.setImageHoverSrc('images/toolbar/save_over.gif');\n"
                    << "icon0.setLabel('Apply');\n"
                    << "icon0.setAction('save()');\n"
                    << "icon0.setTips('Apply changes');\n";

                // 2. back
                out << "var icon1 = new Icon();\n"
                    << "icon1.setImageSrc('images/toolbar/back.gif');\n"
                    << "icon1.setImageHoverSrc('images/toolbar/back_over.gif');\n"
                    << "icon1.setLabel('Back');\n"
                    << "icon1.setAction('back()');\n"
                    << "icon1.setTips('back to previous page');\n";

                out << "toolbar.addIcon(icon0);\n";
                out << "toolbar.addIcon(icon1);\n";

                out << "function save(){\n"
                    << "document.getElementById('snmpform').submit();\n"
                    << "}\n";

                // construct the readonly url
                ZQ::common::URLStr roURL(_pHttpRequestCtx->GetRootURL(), true); // case sensitive
                roURL.setPath(_uri);
                roURL.setVar(WLREQUEST_VAR_TEMPLATE, _template);          //template
                roURL.setVar(WLREQUEST_VAR_DATASOURCE, _datasource);      //datasource
                roURL.setVar(WLREQUEST_VAR_ENTRY, _entry);                //entry
                roURL.setVar(WLREQUEST_VAR_LOGFILEPATH, _logfilepath);    //logfilepath
                roURL.setVar(SNMPREQUEST_VAR_BASEOID, _baseoid);          //baseoid
                roURL.setVar(SNMPREQUEST_VAR_SERVERIP, _serverip);        //serverip
                roURL.setVar(SNMPREQUEST_VAR_COMMUNITY, _community);      //community
                roURL.setVar(SNMPREQUEST_VAR_FILTER, _filter);            // filter
                roURL.setVar(SNMPREQUEST_VAR_MODE, SNMPREQUEST_VAL_MODE_NGODRO);    //mode
                roURL.setVar(SNMPREQUEST_VAR_AUTHURL, _authURL);          // authorization

                // back to the readonly page
                out << "function back(){\n"
                    << "\twindow.location='" << roURL.generate() << "';\n"
                    << "}\n";
            }
            break;
        case emRaw:
        default:
            {
                // save
                out << "var icon0 = new Icon();\n"
                    << "icon0.setImageSrc('images/toolbar/save.gif');\n"
                    << "icon0.setImageHoverSrc('images/toolbar/save_over.gif');\n"
                    << "icon0.setLabel('Apply');\n"
                    << "icon0.setAction('save()');\n"
                    << "icon0.setTips('Apply changes');\n";

                out << "toolbar.addIcon(icon0);\n";

                out << "function save(){\n"
                    << "document.getElementById('snmpform').submit();\n"
                    << "}\n";
            }
            break;
        }
        out << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
        out << "</script>\n";
    }
    if(emNGODRO == mode)
    {
        out << "<table class='listTable' style='width:80%'>";
        out << "<colgroup><col width='50%'><col='50%'></colgroup>";
        KEYVALUEPAIR::const_iterator it;
        for(it = m_KeyValue.begin(); it != m_KeyValue.end() ; ++it)
        {
//            std::pair<std::string, std::string> structuredName = parseVariableName(it->first);
//
//#ifdef ZQ_OS_MSWIN
//            if(!((0 == stricmp(group.c_str(), structuredName.first.c_str()) && bKeep)))
//                continue; // filtered
//#else
//            if(!((0 == strcasecmp(group.c_str(), structuredName.first.c_str()) && bKeep)))
//                continue; // filtered
//#endif
            if (checkVarnameByFilter(it->first, group, !bKeep))
            {
                continue;
            }

            out << "<tr>";
            //out << "<td class='title'>" << structuredName.second << "</td>"; // hide the group name
            out << "<td class='title'>" << it->first << "</td>";
            out << "<td>" << it->second.value << "</td>";
            out << "</tr>";
        }
        out << "</table>";
    }
    else // ngod.rw or raw
    {
        //build data body
        out << "<form id='snmpform' method=post action=\"" << _uri << "\">";
        out << "<table class='listTable'>";
        out << "<colgroup><col width='50%'><col='50%'></colgroup>";

        KEYVALUEPAIR::const_iterator it;
        int idx = 0; //for variable identification
        for(it = m_KeyValue.begin(); it != m_KeyValue.end() ; ++it)
        {
            std::pair<std::string, std::string> structuredName = parseVariableName(it->first);

#ifdef ZQ_OS_MSWIN
            if(!((0 == stricmp(group.c_str(), structuredName.first.c_str()) && bKeep)))
                continue; // filtered
#else
            if(!((0 == strcasecmp(group.c_str(), structuredName.first.c_str()) && bKeep)))
                continue; // filtered
#endif
            ++idx;
            out << "<tr>";
            out << "<td class='title'>" << structuredName.second << "</td>"; // hide the group name
            out << "<td>";
            if(SNMPATTR_VARRW_READONLY == it->second.readonly)
            {
                out << it->second.value;
            }
            else//SNMPATTR_VARRW_WRITABLE
            {
                out << "<input type=text";
                out << " name=\"" << SNMP_TAGPREFIX_VALUE << idx << "\"";
                out << " value=\"" << it->second.value << "\">";
                //other required field
                //variable index
                out << "<input type=hidden";
                out << " name=\"" << SNMP_TAGPREFIX_INDEX << idx << "\"";
                out << " value=\"" << idx << "\">";
                //old value
                out << "<input type=hidden";
                out << " name=\"" << SNMP_TAGPREFIX_OLDVALUE << idx << "\"";
                out << " value=\"" << it->second.value << "\">";
                //name
                out << "<input type=hidden";
                out << " name=\"" << SNMP_TAGPREFIX_NAME << idx << "\"";
                out << " value=\"" << it->first << "\">";
                //type
                out << "<input type=hidden";
                out << " name=\"" << SNMP_TAGPREFIX_TYPE << idx << "\"";
                out << " value=\"";
                switch(it->second.type)
                {
                case SNMPATTR_VARTYPE_INT:
                    out << "int";
                    break;
                case SNMPATTR_VARTYPE_STRING:
                    out << "string";
                    break;
                default:
                    out << "unkown";
                }
                out << "\">";
                //oid
                out << "<input type=hidden";
                out << " name=\"" << SNMP_TAGPREFIX_OID << idx << "\"";
                out << " value=\"" << it->second.oid << "\">";
            }
            out << "</td>";
            out << "</tr>";
        }	

        //helper macro
#define HTML_INPUT_HIDDEN(out, name, val) (out) << "<input type=hidden"\
    << " name=\"" << (name) << "\""\
    << " value=\"" << (val) << "\">";
        HTML_INPUT_HIDDEN(out, WLREQUEST_VAR_TEMPLATE, _template);          //template
        HTML_INPUT_HIDDEN(out, WLREQUEST_VAR_DATASOURCE, _datasource);      //datasource
        HTML_INPUT_HIDDEN(out, WLREQUEST_VAR_ENTRY, _entry);                //entry
        HTML_INPUT_HIDDEN(out, WLREQUEST_VAR_LOGFILEPATH, _logfilepath);    //logfilepath
        HTML_INPUT_HIDDEN(out, SNMPREQUEST_VAR_BASEOID, _baseoid);          //baseoid
        HTML_INPUT_HIDDEN(out, SNMPREQUEST_VAR_SERVERIP, _serverip);        //serverip
        HTML_INPUT_HIDDEN(out, SNMPREQUEST_VAR_COMMUNITY, _community);      //community

        // the optional fields
        if(_filter)
        {
            HTML_INPUT_HIDDEN(out, SNMPREQUEST_VAR_FILTER, _filter);        // filter
        }
        if(_mode)
        {
            HTML_INPUT_HIDDEN(out, SNMPREQUEST_VAR_MODE, _mode);            //mode
        }
        if(_authURL)
        {
            HTML_INPUT_HIDDEN(out, SNMPREQUEST_VAR_AUTHURL, _authURL);       // authorization
        }

        out << "</table>";
        out << "</form>";
#undef HTML_INPUT_HIDDEN
    }

    return true;
}

}}//namespace ZQTianShan::Layout
