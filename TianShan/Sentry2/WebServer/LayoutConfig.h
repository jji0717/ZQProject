// LayoutConfig.h: interface for the LayoutConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYOUTCONFIG_H__C002488B_B835_4965_B423_6DAEFEC2DEC1__INCLUDED_)
#define AFX_LAYOUTCONFIG_H__C002488B_B835_4965_B423_6DAEFEC2DEC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <ZQ_common_conf.h>
#include "SentryEnv.h"
#include <ConfigHelper.h>
#include <string>
#include <list>
#include <map>

namespace ZQTianShan {
namespace Sentry {
namespace weblayout {
struct PageAttr
{
    std::string     _name;
    std::string     _uri;
    std::string     _layoutId; // the layout id
    std::string     _template;//template header/footer file, generated thru layout id
    std::string     _charset;
    int32           hidden;
    std::map< std::string, std::string > _params;
    
    static void structure(ZQ::common::Config::Holder<PageAttr> &holder)
    {
        holder.addDetail("", "name", &PageAttr::_name);
        holder.addDetail("", "uri", &PageAttr::_uri);
        holder.addDetail("", "layoutId", &PageAttr::_layoutId, "");
        holder.addDetail("", "template", &PageAttr::_template, "");
        holder.addDetail("", "charset", &PageAttr::_charset, "");
        holder.addDetail("", "hidden", &PageAttr::hidden, "0");
        holder.addDetail("param", &PageAttr::readParam, &PageAttr::registerNothing);
    }
    void readParam(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common::Config;
        Holder<NVPair> nvHolder;
        nvHolder.read(node, hPP);
        _params[nvHolder.name] = nvHolder.value;
    }
    void registerNothing(const std::string&){}

    std::string href() const;
};

struct SectionAttr
{
    //self data
    std::string     _name;
    std::string     _displayname;
    std::string     _layoutId;
    int32     _defaultpage;
    std::list< PageAttr > _pages;//with order
    static void structure(ZQ::common::Config::Holder<SectionAttr> &holder)
    {
        holder.addDetail("", "name", &SectionAttr::_name);
        holder.addDetail("", "displayname", &SectionAttr::_displayname, "");
        holder.addDetail("", "layoutId", &SectionAttr::_layoutId, "");
        holder.addDetail("", "defaultpage", &SectionAttr::_defaultpage, "1");
        holder.addDetail("page", &SectionAttr::readPage, &SectionAttr::registerNothing);
    }
    void readPage(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using ZQ::common::Config::Holder;
        Holder<PageAttr> pgHolder;
        pgHolder.read(node, hPP);
        if(!pgHolder.hidden)
            _pages.push_back(pgHolder);
    }
    void registerNothing(const std::string&){}
    //relationship
    std::list< SectionAttr > _children;
    
    std::string href() const;
};

struct LayoutInfo{
    std::string charset;
    SectionAttr     _local;
    SectionAttr     _site;
    static void structure(ZQ::common::Config::Holder<LayoutInfo> &holder)
    {
        using namespace ZQ::common::Config;
        holder.addDetail("pagedefault", "charset", &LayoutInfo::charset, "");
        holder.addDetail("local", &LayoutInfo::readLocal, &LayoutInfo::registerNothing, Range(1, 1));
        holder.addDetail("local/service", &LayoutInfo::readLocalService, &LayoutInfo::registerNothing);
        holder.addDetail("site", &LayoutInfo::readSite, &LayoutInfo::registerNothing, Range(1, 1));
        holder.addDetail("site/section", &LayoutInfo::readSiteSection, &LayoutInfo::registerNothing);
    }
    void readLocal(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void readLocalService(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void readSite(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void readSiteSection(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerNothing(const std::string&){}
    
    void fixup(); // fixup the config

private:
    struct FixupContext
    {
        std::string charset;
        std::string layoutPath;
    };
    void fixupSection(SectionAttr &sec, const FixupContext& ctx); // fixup a section
};
} // namespace weblayout

class LayoutConfig  
{
public:
    LayoutConfig(SentryEnv &env);
	~LayoutConfig();
public:
    bool load(const char *configfile);
    bool GenerateLocalHeaderFiles(const char *dir);
    bool GetLocalNavNode(SentryPages::NavNode &localnode);

    bool GenerateSiteHeaderFiles(const char *dir);
    bool GetSiteNavNode(SentryPages::NavNode &siteNode);
private:
     bool generateHeaderFiles(const weblayout::SectionAttr& section, const char *dir);
private:
    weblayout::LayoutInfo _layout;
    SentryEnv& _env;
};

}} // namespace ZQTianShan::Sentry
#endif // !defined(AFX_LAYOUTCONFIG_H__C002488B_B835_4965_B423_6DAEFEC2DEC1__INCLUDED_)
