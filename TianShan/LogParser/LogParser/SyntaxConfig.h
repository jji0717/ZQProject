#ifndef __EventSink_SyntaxConfig_H__
#define __EventSink_SyntaxConfig_H__
#include <ConfigHelper.h>
#include <functional>

class ICaseLess: std::binary_function<std::string, std::string, bool> 
{
public:
    result_type operator()( const first_argument_type& a, const second_argument_type& b) const
    {
#ifdef ZQ_OS_MSWIN
        return (stricmp(a.c_str(), b.c_str()) < 0);
#else
        return (strcasecmp(a.c_str(), b.c_str()) < 0);
#endif
    }
};

struct EventConf
{
    std::string category;
    int32 eventId;
    std::string eventName;
    std::string stampUTC;
    std::string sourceNetId;
	std::string strTargets;
    typedef std::map<std::string, std::string> Properties;
    Properties params;

    static void structure(ZQ::common::Config::Holder<EventConf>& holder)
    {
        using namespace ZQ::common;
        holder.addDetail("", "category", &EventConf::category);
        holder.addDetail("", "eventId", &EventConf::eventId);
        holder.addDetail("", "eventName", &EventConf::eventName);
        holder.addDetail("", "stampUTC", &EventConf::stampUTC);
        holder.addDetail("", "sourceNetId", &EventConf::sourceNetId);
		holder.addDetail("", "targets", &EventConf::strTargets,"");
        holder.addDetail("Param", &EventConf::readParam, &EventConf::registerNothing);
    }

    void readParam(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<Config::NVPair> param;
        param.read(node, hPP);
        params[param.name] = param.value;
    }
    void registerNothing(const std::string&){}
};
struct RuleConf
{
    int32 enabled;
    std::string regex;
    EventConf evnt;

    static void structure(ZQ::common::Config::Holder<RuleConf>& holder)
    {
        using namespace ZQ::common;
        holder.addDetail("", "enabled", &RuleConf::enabled);
        holder.addDetail("", "regex", &RuleConf::regex);
        holder.addDetail("Event", &RuleConf::readEvent, &RuleConf::registerNothing, Config::Range(1,1));
    }

    void readEvent(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<EventConf> e;
        e.read(node, hPP);
        evnt = e;
    }
    void registerNothing(const std::string&){}
};

struct SyntaxConf
{
    std::string key;
    std::string evntSrcType;
    typedef std::vector<RuleConf> Rules;
    Rules rules;

    static void structure(ZQ::common::Config::Holder<SyntaxConf>& holder)
    {
        using namespace ZQ::common;
        holder.addDetail("", "key", &SyntaxConf::key);
        holder.addDetail("EventSource", "type", &SyntaxConf::evntSrcType);
        holder.addDetail("Rule", &SyntaxConf::readRule, &SyntaxConf::registerNothing);
    }

    void readRule(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<RuleConf> r;
        r.read(node, hPP);
        rules.push_back(r);
    }
    void registerNothing(const std::string&){}
};
struct SyntaxDefConf
{
    typedef std::map<std::string, SyntaxConf, ICaseLess> SyntaxMap;
    SyntaxMap syntaxMap;

    static void structure(ZQ::common::Config::Holder<SyntaxDefConf>& holder)
    {
        using namespace ZQ::common;
        holder.addDetail("Syntax", &SyntaxDefConf::readSyntax, &SyntaxDefConf::registerNothing);
    }

    void readSyntax(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        using namespace ZQ::common;
        Config::Holder<SyntaxConf> s;
        s.read(node, hPP);
        syntaxMap[s.key] = s;
    }
    void registerNothing(const std::string&){}
};
#endif

