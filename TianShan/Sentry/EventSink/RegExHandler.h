#ifndef __EventSink_RegEx_Handler_H__
#define __EventSink_RegEx_Handler_H__
#include "LiteralFunc.h"
#include "EventSink.h"
#include <Log.h>
#include <boost/regex.hpp>

class RegExpHandler:public IRawMessageHandler
{
public:
    RegExpHandler(ZQ::common::Log& log, const std::string& exp, const EventTemplate& tmpl, IEventSender* sender, const Literal::Library& lib);
    ~RegExpHandler();

    virtual void handle(const char* msg, int len, const MessageIdentity& mid);
private:
    typedef IEventSender::Event Event;
    struct InternalEventTemplate
    {
        Literal::Expression  category;
        int eventId;
        Literal::Expression eventName;
        Literal::Expression stampUTC;
        Literal::Expression sourceNetId;
        typedef std::map<std::string, Literal::Expression> Properties;
        Properties params;
    };

    // fill the template to generate an event
    void generateEvent(const boost::cmatch& matchResult, Event& evnt, int64 stamp) const;
    std::string fixup(const boost::cmatch& matchResult, const std::string& s, const std::string& T) const;
    std::string fixupAndExecute(const boost::cmatch& matchResult, const Literal::Expression& ex, const std::string& T) const;

	std::string fixupParam(const boost::cmatch& matchResult, const std::string& s, const std::string& T) const;
	std::string fixupMacro(const std::string& s) const;

private:
    ZQ::common::Log& _log;
    boost::regex _expr;
    InternalEventTemplate _tmpl;
    IEventSender* _pSender;
	std::vector<std::string> _targets;
};


#endif

