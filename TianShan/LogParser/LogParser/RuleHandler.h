#ifndef __RULE_Handler_H__
#define __RULE_Handler_H__

#include "LiteralFunc.h"
#include "EventSink.h"
#include "LogMessageHandler.h"
#include <Log.h>
#include <boost/regex.hpp>

class RuleHandler
{
public:
	RuleHandler(ZQ::common::Log& log, const std::string& exp, const EventTemplate& tmpl, std::vector<std::string> sendTargets, const Literal::Library& lib);
	~RuleHandler();

	void handle(const char* msg, int len, const MessageIdentity& mid,LogMessageHandler& MsgHandler);

private:
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
	void generateEvent(const boost::cmatch& matchResult, EventTemplate& evnt, int64 stamp) const;
	std::string fixup(const boost::cmatch& matchResult, const std::string& s, const std::string& T) const;
	std::string fixupAndExecute(const boost::cmatch& matchResult, const Literal::Expression& ex, const std::string& T) const;

	std::string fixupParam(const boost::cmatch& matchResult, const std::string& s, const std::string& T) const;
	std::string fixupMacro(const std::string& s) const;

private:
	ZQ::common::Log& _log;
	boost::regex _expr;
	InternalEventTemplate _tmpl;
	std::vector<std::string> _targets;
    
};

#endif