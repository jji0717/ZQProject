// SiteController.cpp: implementation of the SiteController class.
//
//////////////////////////////////////////////////////////////////////

#include "SystemUtils.h"
#include "StdAfx.h"
#include "SiteController.h"
#include "AdminCtrlUtil.h"
#include <Ice/Ice.h>

using namespace ZQ::common;
using namespace TianShanIce;

#define LOG_MODULE_NAME         "SiteCtrl"

//////////////////////////////////////////////////////////////////////
#define SITECTRL_CATCH_COMMON_CASE \
catch (const Ice::Exception& ex)\
{\
    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch Ice::Exception [%s]"), ex.ice_name().c_str());\
    std::string errmsg = std::string("catch Ice::Exception [") + ex.ice_name() + "].";\
    _pHttpRequestCtx->Response().SetLastError(errmsg.c_str());\
    return false;\
}\
    catch (const char* msg)\
{\
    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch exception [%s]"), msg);\
    std::string errmsg = std::string("catch exception [") + msg + "].";\
    _pHttpRequestCtx->Response().SetLastError(errmsg.c_str());\
    return false;\
}\
    catch(...)\
{\
    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter unknown exception."));\
    _pHttpRequestCtx->Response().SetLastError("encounter unknown exception.");\
    return false;\
}
#define SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG \
_pHttpRequestCtx->Response().SetLastError("Bad program logic or incorrect config was detected!");\
return false

//////////////////////////////////////////////////////////////////////
SiteController::SiteController(IHttpRequestCtx *pHttpRequestCtx)
:_pHttpRequestCtx(pHttpRequestCtx)
{

}

SiteController::~SiteController()
{
    uninit();
}


bool SiteController::init()
{
    _rootDir = _pHttpRequestCtx->GetRootDir();
    if(NULL == _rootDir)
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "web root directory is required in the request."));
        SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
    }
    // gather required variables
    _template = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_TEMPLATE);
    if(NULL == _template)
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "template is missing in the request."));
        SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
    }
    _endpoint = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ENDPOINT);
    if(NULL == _endpoint)
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "endpoint is missing in the request."));
        SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
    }
    std::string prxstr = std::string(SERVICE_NAME_BusinessRouter":") + _endpoint;
    try
    {
        int argc = 0;
        _ic = Ice::initialize(argc, NULL);
        glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "connect SiteAdmin [%s]."), prxstr.c_str());
        _sa = ::TianShanIce::Site::SiteAdminPrx::checkedCast(_ic->stringToProxy(prxstr));
    }
    SITECTRL_CATCH_COMMON_CASE;
    return true;
}

void SiteController::uninit()
{
    try
    {
        _sa = NULL;
        if(_ic)
        {
            _ic->destroy();
            _ic = NULL;
        }
    }
    catch(...)
    {
        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter unknown exception during uninit()."));
    }
}
#define SITECTRL_ACTION_UPDATESITE      'u'
#define SITECTRL_ACTION_REMOVESITE      'r'
#define SITECTRL_ACTION_UPDATELIMIT     'l'
#define SITECTRL_ACTION_SETPROP         'p'
#define SITECTRL_ACTIONSET_SITE         "urlp"

#define SITECTRL_VAR_NAME               "site#name"
#define SITECTRL_VAR_DESC               "site#desc"
#define SITECTRL_VAR_MAXBW              "site#maxbw"
#define SITECTRL_VAR_MAXSESS            "site#maxsess"
#define SITECTRL_VARPREFIX_PROP         "site.prop#"
// this function work together with AdminCtrl_Site.html
bool SiteController::SitePage()
{
    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "request site page."));
    if(!init())
    {
        return false;
    }

    try{
        const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
        if(action)
        {
            glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "request SitePage with action [%s]"), action);
            size_t nAction = strlen(action);
            // validity check
            if((0 == nAction) || (strspn(action, SITECTRL_ACTIONSET_SITE) != nAction))
            {
                // bad action type
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad action type.[%s]"), action);
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
            const char* siteName = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_NAME);
            if(NULL == siteName)
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "site name is missing in the request."));
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }

            if(strchr(action, SITECTRL_ACTION_REMOVESITE))
            {
                // remove site
                _sa->removeSite(siteName);
            }
            if(strchr(action, SITECTRL_ACTION_UPDATESITE))
            {
                // update site
                const char* siteDesc = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_DESC);
                if(NULL == siteDesc)
                {
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "site desc is missing in the request."));
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }
                _sa->updateSite(siteName, siteDesc);
            }
            if(strchr(action, SITECTRL_ACTION_UPDATELIMIT))
            {
                // update resource limited
                const char* maxBW = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_MAXBW);
                if(NULL == maxBW)
                {
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "max bandwidth is missing in the request."));
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }
                const char* maxSess = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_MAXSESS);
                if(NULL == maxSess)
                {
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "max session count is missing in the request."));
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }
                _sa->updateSiteResourceLimited(siteName, _atoi64(maxBW), atoi(maxSess));
            }
            if(strchr(action, SITECTRL_ACTION_SETPROP))
            {
                // set site properties
                ::TianShanIce::Properties props;
                // extract properties
                if(!AdminCtrlUtil::extractKVData(SITECTRL_VARPREFIX_PROP, _pHttpRequestCtx, props))
                {
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to get all vars with prefix [%s]."), SITECTRL_VARPREFIX_PROP);
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }

                _sa->setSiteProperties(siteName, props);
            }
        } // end if(action)

        IHttpResponse &out = _pHttpRequestCtx->Response();
        {
            // import display code
            std::string dispFile = std::string(_rootDir) + "AdminCtrl_Site.html";
            if(!AdminCtrlUtil::importFile(out, dispFile.c_str()))
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to import file [%s]."), dispFile.c_str());
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
        }

        // list all virtual sites
        ::TianShanIce::Site::VirtualSites sites = _sa->listSites();
        
        out << "<script type='text/javascript'>\n";
        for(int i = 0; i < sites.size(); ++i)
        {
            out << "_sites[" << i << "]=new TSVirtualSite(\"" 
                << sites[i].name << "\",\""
                << sites[i].desc << "\","
                << AdminCtrlUtil::long2str(sites[i].maxDownstreamBwKbps) << ","
                << sites[i].maxSessions << ");\n";
            for(::TianShanIce::Properties::iterator it_prop = sites[i].properties.begin(); it_prop != sites[i].properties.end(); ++it_prop)
            {
                out << "_sites[" << i << "].properties[\""
                    << it_prop->first << "\"]=\""
                    << it_prop->second << "\";\n";
            }
        }
        out << "_template=\"" << _template << "\";\n";
        out << "_endpoint=\"" << _endpoint << "\";\n";
        out << "display();\n";
        out << "</script>\n";
    }
    SITECTRL_CATCH_COMMON_CASE;

    return true;
}

#define SITECTRL_ACTION_UPDATEAPP   'u'
#define SITECTRL_ACTION_REMOVEAPP   'r'
#define SITECTRL_ACTIONSET_APP      "ur"

#define SITECTRL_VAR_APPNAME        "site.app#name"
#define SITECTRL_VAR_APPDESC        "site.app#desc"
#define SITECTRL_VAR_APPENDPOINT    "site.app#endpoint"

// this function work together with AdminCtrl_App.html
bool SiteController::AppPage()
{
    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "request app page."));
    if(!init())
    {
        return false;
    }
    try{
        const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
        if(action)
        {
            glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "request AppPage with action [%s]"), action);
            size_t nAction = strlen(action);
            // validity check
            if((0 == nAction) || (strspn(action, SITECTRL_ACTIONSET_APP) != nAction))
            {
                // bad action type
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad action type.[%s]"), action);
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }

            const char* appName = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_APPNAME);
            if(NULL == appName)
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "app name is missing in the request."));
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }

            if(strchr(action, SITECTRL_ACTION_REMOVEAPP))
            {
                // remove app
                _sa->removeApplication(appName);
            }
            if(strchr(action, SITECTRL_ACTION_UPDATEAPP))
            {
                // update app
                const char* appDesc = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_APPDESC);
                if(NULL == appDesc)
                {
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "app desc is missing in the request."));
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }
                const char* appEndpoint = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_APPENDPOINT);
                if(NULL == appEndpoint)
                {
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "app endpoint is missing in the request."));
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }
                _sa->updateApplication(appName, appEndpoint, appDesc);
            }
        } // end if(action)

        IHttpResponse &out = _pHttpRequestCtx->Response();
        {
            // import display code
            std::string dispFile = std::string(_rootDir) + "AdminCtrl_App.html";
            if(!AdminCtrlUtil::importFile(out, dispFile.c_str()))
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to import file [%s]."), dispFile.c_str());
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
        }

        // list all Applications
        ::TianShanIce::Site::AppInfos apps = _sa->listApplications();
        
        out << "<script type='text/javascript'>\n";
        for(int i = 0; i < apps.size(); ++i)
        {
            out << "_apps.push(new TSAppInfo(\"" 
                << apps[i].name << "\",\""
                << apps[i].endpoint << "\",\""
                << apps[i].desc << "\"));\n";
        }
        out << "_template=\"" << _template << "\";\n";
        out << "_endpoint=\"" << _endpoint << "\";\n";
        out << "display();\n";
        out << "</script>\n";
    }
    SITECTRL_CATCH_COMMON_CASE;

    return true;
}
#define SITECTRL_ACTION_MOUNTAPP    'm'
#define SITECTRL_ACTION_UNMOUNTAPP  'u'
#define SITECTRL_ACTIONSET_MOUNT    "mu"

#define SITECTRL_VAR_MOUNTEDPATH    "site.mount#path" 
// this function work together with AdminCtrl_Mount.html
bool SiteController::MountPage()
{
    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "request mount page."));
    if(!init())
    {
        return false;
    }
    bool bMountUpdated = false;
    try{
        const char *action = _pHttpRequestCtx->GetRequestVar(ADMINCTRL_VAR_ACTION);
        if(action)
        {
            glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "request AppPage with action [%s]"), action);
            size_t nAction = strlen(action);
            // validity check
            if((0 == nAction) || (strspn(action, SITECTRL_ACTIONSET_MOUNT) != nAction))
            {
                // bad action type
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "bad action type.[%s]"), action);
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
            const char* siteName = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_NAME);
            if(NULL == siteName)
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "site name is missing in the request."));
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
            const char* mountedPath = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_MOUNTEDPATH);
            if(NULL == mountedPath)
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "mounted path is missing in the request."));
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
            
            if(strchr(action, SITECTRL_ACTION_UNMOUNTAPP))
            {
                // unmount app
                _sa->unmountApplication(siteName, mountedPath);
                bMountUpdated = true;
            }

            if(strchr(action, SITECTRL_ACTION_MOUNTAPP))
            {
                // mount app
                const char* appName = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_APPNAME);
                if(NULL == appName)
                {
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "app name is missing in the request."));
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }
                _sa->mountApplication(siteName, mountedPath, appName);
                bMountUpdated = true;
            }
        } // end if(action)

        IHttpResponse &out = _pHttpRequestCtx->Response();
        {
            // import display code
            std::string dispFile = std::string(_rootDir) + "AdminCtrl_Mount.html";
            if(!AdminCtrlUtil::importFile(out, dispFile.c_str()))
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to import file [%s]."), dispFile.c_str());
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
        }
        
        AdminCtrlUtil::StringVector mountPaths; // the mount paths
        // list all sites
        out << "<script type='text/javascript'>\n";
        ::TianShanIce::Site::VirtualSites sites = _sa->listSites();
        {
            for(int i = 0; i < sites.size(); ++i)
            {
                out << "_sites.push(\"" << sites[i].name << "\");\n";
            }
        }
        for(::TianShanIce::Site::VirtualSites::const_iterator cit_site = sites.begin(); cit_site != sites.end(); ++cit_site)
        {
            // list all Mounts of this site
            ::TianShanIce::Site::AppMounts mounts = _sa->listMounts(cit_site->name);
            
            for(::TianShanIce::Site::AppMounts::const_iterator cit_mnt = mounts.begin(); cit_mnt != mounts.end(); ++cit_mnt)
            {
                mountPaths.push_back((*cit_mnt)->getMountedPath()); // gather the mount path

                out << "_mounts.push(new TSAppMount(\"" 
                    << cit_site->name << "\",\""
                    << mountPaths.back() << "\",\""
                    << (*cit_mnt)->getAppName() << "\"));\n";
            }
        }

        // list all applications
        ::TianShanIce::Site::AppInfos apps = _sa->listApplications();
        {
            for(int i = 0; i < apps.size(); ++i)
            {
                out << "_apps.push(\"" << apps[i].name << "\");\n";
            }
        }
        if(bMountUpdated)
        { // need update the server load template file
            const char* srvrloadFilePath = _pHttpRequestCtx->GetRequestVar("srvrloadpath");
            if(srvrloadFilePath && (*srvrloadFilePath) != '\0')
            {
                int nTried = 0;
                do
                {
                    ++nTried;
                    if(AdminCtrlUtil::updateXML(srvrloadFilePath, "SrvrList/CMGroup/AppType", mountPaths))
                    {
                        glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "Updated server load template file [%s] successfully."), srvrloadFilePath);
                        break;
                    }
                    else if(nTried < 2)
                    { // retry
                        glog(Log::L_WARNING, CLOGFMT(LOG_MODULE_NAME, "Failed to update server load template file [%s]. Retry after 500 msec"), srvrloadFilePath);
                        //Sleep(500);
				::SYS::sleep(500);
				continue;
                    }
                    else
                    { // failed
                        glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Failed to update server load template file [%s] after 2 tries."), srvrloadFilePath);
                        break;
                    }
                }while(true);
            }
            else
            {
                glog(Log::L_DEBUG, CLOGFMT(LOG_MODULE_NAME, "MountPage() no server load template file path provide."));
            }
        }
        out << "_template=\"" << _template << "\";\n";
        out << "_endpoint=\"" << _endpoint << "\";\n";
        {
            const char* srvrloadPath = _pHttpRequestCtx->GetRequestVar("srvrloadpath");
            out << "_srvrloadpath=\"";
            if(srvrloadPath)
            { // escape the '\'
                out << AdminCtrlUtil::escapeString(srvrloadPath);
            }
            out << "\";\n";

        }
        out << "display();\n";
        out << "</script>\n";
    }
    SITECTRL_CATCH_COMMON_CASE;
    
    return true;
}
#define SITECTRL_VAR_TXNPARAMS   "site.txn#params"
#define SITECTRL_VAR_TXNSTARTID  "startid"
#define SITECTRL_VAR_TXNMAXCOUNT "maxcount"
// this function work together with AdminCtrl_Txn.html
bool SiteController::TxnPage()
{
    glog(Log::L_DEBUG,CLOGFMT(LOG_MODULE_NAME, "request transaction page."));
    if(!init())
    {
        return false;
    }
    try{
        // get the parameter list 
        const char* paramList = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_TXNPARAMS);
        if(NULL == paramList)
        {
            glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "parameter names is missing in the request."));
            SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
        }
        const char* siteName = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_NAME);
        const char* appName = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_APPNAME);
        const char* startId = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_TXNSTARTID);
        const char* maxCountStr = _pHttpRequestCtx->GetRequestVar(SITECTRL_VAR_TXNMAXCOUNT);
        int maxCount = maxCountStr ? atoi(maxCountStr) : 20; // 20 by default
        if(0 <= maxCount && maxCount < 20)
            maxCount = 20;

        // list all live transactions
        std::vector< std::pair< std::string, std::string > > paramsCfg;
        {
            // parse parameters config
            StrValues params;
            AdminCtrlUtil::splitString(params, paramList, ";");
            for(size_t i = 0; i < params.size(); ++i)
            {
                StrValues param;
                AdminCtrlUtil::splitString(param, params[i], ":");
                std::transform(param.begin(), param.end(), param.begin(), AdminCtrlUtil::trimWS);
                switch(param.size())
                {
                case 1: // use param name as display title
                    paramsCfg.push_back(std::make_pair(param[0], param[0]));
                    break;
                case 2:
                    paramsCfg.push_back(std::make_pair(param[0], param[1]));
                    break;
                default:
                    glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to parse params config. [%s]"), paramList);
                    SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
                }
            }
        }
        ::TianShanIce::StrValues paramNames;
        {
            for(size_t i = 0; i < paramsCfg.size(); ++i)
            {
                paramNames.push_back(paramsCfg[i].first);
            }
        }
        std::transform(paramNames.begin(), paramNames.end(), paramNames.begin(), std::bind1st(std::plus<std::string>(), SYS_PROP_PREFIX));
        size_t prefixLen = strlen(SYS_PROP_PREFIX);

        Site::TxnInfos txns = _sa->listLiveTxn((siteName ? siteName : "*"), (appName ? appName : "*"), paramNames, (startId ? startId : ""), maxCount);
        
        IHttpResponse &out = _pHttpRequestCtx->Response();
        {
            // import display code
            std::string dispFile = std::string(_rootDir) + "AdminCtrl_Txn.html";
            if(!AdminCtrlUtil::importFile(out, dispFile.c_str()))
            {
                glog(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "failed to import file [%s]."), dispFile.c_str());
                SITECTRL_REPORT_BAD_LOGIC_OR_CONFIG;
            }
        }
        out << "<script type='text/javascript'>\n";
        for(int iTxn = 0; iTxn < txns.size(); ++iTxn)
        {
            out << "_txns[" << iTxn << "]=new TSTxnInfo(\"" << txns[iTxn].sessId << "\");\n";
            for(Properties::const_iterator cit_param= txns[iTxn].params.begin(); cit_param != txns[iTxn].params.end(); ++cit_param)
            {
                out << "_txns[" << iTxn << "].params[\"" << cit_param->first.substr(prefixLen) << "\"]=\"" << cit_param->second<< "\";\n";
            }
        }
        for(int iParam = 0; iParam < paramsCfg.size(); ++iParam)
        {
            out << "_paramNames.push('" << paramsCfg[iParam].first << "');\n";
            out << "_paramTitles['" << paramsCfg[iParam].first << "']='" << paramsCfg[iParam].second << "';\n";
        }
        out << "_template=\"" << _template << "\";\n";
        out << "_endpoint=\"" << _endpoint << "\";\n";
        out << "_startid=\"" << (txns.empty() ? "" : txns.back().sessId) << "\";\n";
        out << "_maxcount=" << maxCount << ";\n";
        out << "_paramlist=\"" << paramList << "\";\n";
        out << "display();\n";
        out << "</script>\n";
    }
    SITECTRL_CATCH_COMMON_CASE;
    
    return true;
}