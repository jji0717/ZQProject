// LayoutConfig.cpp: implementation of the LayoutConfig class.
//
//////////////////////////////////////////////////////////////////////
#include <boost/thread.hpp>
#include "LayoutConfig.h"
#include "HtmlTempl.h"
#include <vector>
#include <urlstr.h>
#include <fstream>
#include <FileSystemOp.h>

namespace ZQTianShan {
namespace Sentry {

LayoutConfig::LayoutConfig(SentryEnv &env)
:_env(env)
{
}

LayoutConfig::~LayoutConfig()
{
}

bool LayoutConfig::GetLocalNavNode(SentryPages::NavNode &localnode)
{
    localnode.children.clear();

    localnode.name = _layout._local._name;
    localnode.displayname = _layout._local._displayname;
    localnode.href = _env._selfInfo.adminRootUrl + _layout._local.href(); // abs path

    std::list< weblayout::SectionAttr >::const_iterator cit_svc;
    for(cit_svc = _layout._local._children.begin(); cit_svc != _layout._local._children.end(); ++cit_svc)
    {
        SentryPages::NavNode svcnode;
        svcnode.name = cit_svc->_name;
        svcnode.displayname = cit_svc->_displayname;
        svcnode.href = _env._selfInfo.adminRootUrl + cit_svc->href();
        localnode.children.push_back(svcnode);
    }

    return true;
}

bool LayoutConfig::GetSiteNavNode(SentryPages::NavNode &siteNode)
{
    siteNode.children.clear();
    
    siteNode.name = _layout._site._name;
    siteNode.displayname = _layout._site._displayname;
    siteNode.href = _env._selfInfo.adminRootUrl + _layout._site.href();
    
    std::list< weblayout::SectionAttr >::const_iterator cit_sec;
    for(cit_sec = _layout._site._children.begin(); cit_sec != _layout._site._children.end(); ++cit_sec)
    {
        SentryPages::NavNode secnode;
        secnode.name = cit_sec->_name;
        secnode.displayname = cit_sec->_displayname;
        secnode.href = _env._selfInfo.adminRootUrl + cit_sec->href();
        siteNode.children.push_back(secnode);
    }
    
    return true;
}

bool LayoutConfig::GenerateLocalHeaderFiles(const char *dir)
{
    if(NULL == dir || '\0' == (*dir))
        return false;

    generateHeaderFiles(_layout._local, dir);
    std::list< weblayout::SectionAttr >::const_iterator cit_svc;

    for(cit_svc = _layout._local._children.begin(); cit_svc != _layout._local._children.end(); ++cit_svc)
    {
        generateHeaderFiles(*cit_svc, dir);
    }
    return true;
}

bool LayoutConfig::GenerateSiteHeaderFiles(const char *dir)
{
    if(NULL == dir || '\0' == (*dir))
        return false;

    generateHeaderFiles(_layout._site, dir);
    std::list< weblayout::SectionAttr >::const_iterator cit_sec;

    for(cit_sec = _layout._site._children.begin(); cit_sec != _layout._site._children.end(); ++cit_sec)
    {
        generateHeaderFiles(*cit_sec, dir);
    }
    return true;
}
static std::string dirOf(const std::string& path)
{
    std::string::size_type pos = path.find_last_of(FNSEPS);
    return (pos == std::string::npos ? "" : path.substr(0, pos));
}
bool LayoutConfig::generateHeaderFiles(const weblayout::SectionAttr& section, const char *dir)
{
    using namespace ::ZQTianShan::HtmlTempl;
 
    std::vector< Tab > tabs;
    tabs.reserve(section._pages.size());
    std::list< weblayout::PageAttr >::const_iterator cit_page;
    for(cit_page = section._pages.begin(); cit_page != section._pages.end(); ++cit_page)
    {
        Tab tab;
        tab.name = cit_page->_name;
        tab.href = _env._selfInfo.adminRootUrl + cit_page->href();
        tabs.push_back(tab);
    }
    //create header files
    size_t i = 0;
    for(cit_page = section._pages.begin(); cit_page != section._pages.end(); ++cit_page)
    {
        std::string hdrname = std::string(dir) + cit_page->_template + ".hdr";
        envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LayoutConfig, "generate html header file: %s"), hdrname.c_str());

        FS::createDirectory(dirOf(hdrname), true);
        std::ofstream header(hdrname.c_str());
        HtmlHeader_MainPage(header, section._name.c_str(), &(tabs[0]), tabs.size(), i, cit_page->_charset.c_str());
        ++i;
    }
    return true;
}

bool LayoutConfig::load(const char *configfile)
{
    if(NULL == configfile)
    {
        return false;
    }
    envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LayoutConfig, "load config file: %s"), configfile);

    ZQ::common::Config::Loader<weblayout::LayoutInfo> layoutLoader("");
    layoutLoader.setLogger(&_env._log);
    // predefined macro
    layoutLoader.PP().define("HOSTNAME", _env._selfInfo.name);

    if(layoutLoader.load(configfile))
    {
        _layout = layoutLoader;
        _layout.fixup();
        return true;
    }
    else
    {
        return false;
    }
}

namespace weblayout {
//////////////////////////////////////////////////////////////////////////
//PageAttr
#define LAYOUT_VAR_TEMPLATE      "#template"

std::string PageAttr::href() const
{
    ZQ::common::URLStr url(NULL, true); //case sensitive
    url.setPath(_uri.c_str());
    url.setVar(LAYOUT_VAR_TEMPLATE, _template.c_str());
    //other vars
    std::map< std::string, std::string >::const_iterator cit;
    for(cit = _params.begin(); cit != _params.end(); ++cit)
    {
        url.setVar(cit->first.c_str(), cit->second.c_str());
    }
    std::string hrefhead = _uri + "?"; //for locate url start position
    const char* hrefstr = strstr(url.generate(), hrefhead.c_str());
    return (hrefstr ? hrefstr : "");
}

//////////////////////////////////////////////////////////////////////////
//SectionAttr

std::string SectionAttr::href() const
{
    if(_pages.empty())
        return "";

    size_t iDefaultPage = _defaultpage; //1-based page index 
    if(0 == iDefaultPage || _pages.size() < iDefaultPage)
        iDefaultPage = 1;//adjust to first page

    //iDefaultPage always a valid entry
    std::list< PageAttr >::const_iterator cit_page;
    cit_page = _pages.begin();
    while(--iDefaultPage)
        ++cit_page;

    return cit_page->href();
}

void LayoutInfo::readLocal(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ZQ::common::Config::Holder<SectionAttr> secHolder;
    secHolder.read(node, hPP);
    _local._children.swap(secHolder._children); // keep the children info
    _local = secHolder;
}
void LayoutInfo::readLocalService(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ZQ::common::Config::Holder<SectionAttr> secHolder;
    secHolder.read(node, hPP);
    _local._children.push_back(secHolder);
}
void LayoutInfo::readSite(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ZQ::common::Config::Holder<SectionAttr> secHolder;
    secHolder.read(node, hPP);
    _site._children.swap(secHolder._children); // keep the children info
    _site = secHolder;
}
void LayoutInfo::readSiteSection(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ZQ::common::Config::Holder<SectionAttr> secHolder;
    secHolder.read(node, hPP);
    _site._children.push_back(secHolder);
}
static std::string toStr(uint32 i)
{
    char buf[20];
    buf[0] = '\0';
    sprintf(buf, "%u", i);
    return buf;
}
void LayoutInfo::fixupSection(SectionAttr &sec, const FixupContext& ctx)
{
    // fixup the display name
    if(sec._displayname.empty())
        sec._displayname = sec._name;

    uint32 i = 1;
    std::list<PageAttr>::iterator itPage;
    for(itPage = sec._pages.begin(), i = 1; itPage != sec._pages.end(); ++itPage, ++i)
    {
        // fixup the charset
        if(itPage->_charset.empty())
            itPage->_charset = ctx.charset;
        // auto fill the transfer id
        if(itPage->_layoutId.empty())
            itPage->_layoutId = std::string("page.") + toStr(i);
        // generate the template
        if(itPage->_template.empty())
            itPage->_template = ctx.layoutPath + FNSEPS + itPage->_layoutId;
    }

    std::list<SectionAttr>::iterator itSec;
    for(itSec = sec._children.begin(), i = 1; itSec != sec._children.end(); ++itSec, ++i)
    {
        // auto fill the transfer id
        if(itSec->_layoutId.empty())
            itSec->_layoutId = std::string("section.") + toStr(i);
        // fixup the context
        FixupContext theCtx = ctx;
        theCtx.layoutPath += FNSEPS + itSec->_layoutId;
        fixupSection(*itSec, theCtx);
    }
}
void LayoutInfo::fixup()
{
    FixupContext ctx;
    ctx.charset = charset;

    if(_local._layoutId.empty())
        _local._layoutId = "local";
    ctx.layoutPath = _local._layoutId;

    fixupSection(_local, ctx);

    if(_site._layoutId.empty())
        _site._layoutId = "site";
    ctx.layoutPath = _site._layoutId;

    fixupSection(_site, ctx);
}
} // namespace weblayout
}} // namespace ZQTianShan::Sentry

