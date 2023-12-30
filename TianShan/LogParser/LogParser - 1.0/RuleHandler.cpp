#include "RuleHandler.h"
#include <Exception.h>
#include <TimeUtil.h>
#include <sstream>

#define ReLOGFMT(_MOD, _X) CLOGFMT(_MOD, "[%p] " _X), this

RuleHandler::RuleHandler(ZQ::common::Log& log, const std::string& exp, const EventTemplate& tmpl, IEventSender* sender, const Literal::Library& lib)
				:_log(log)
				,_pSender(sender)
{
	try
	{
		_expr.assign(exp);
	}
	catch( const boost::bad_expression& e)
	{
		char buf[1024] = {0};
		sprintf(buf, "Bad expression [%s]. position: %ld, type: %d, detail: %s", exp.c_str(), e.position(), e.code(), e.what());
		throw ZQ::common::Exception(buf);
	}

	{ // setup of the template
		_tmpl.category = Literal::compile(tmpl.category, lib);
		_tmpl.eventId = tmpl.eventId;
		_tmpl.eventName = Literal::compile(tmpl.eventName, lib);
		_tmpl.stampUTC = Literal::compile(tmpl.stampUTC, lib);
		_tmpl.sourceNetId = Literal::compile(tmpl.sourceNetId, lib);
		for(Event::Properties::const_iterator it = tmpl.params.begin(); it != tmpl.params.end(); ++it)
		{
			_tmpl.params[it->first] = Literal::compile(it->second, lib);
		}
	}
	_log(ZQ::common::Log::L_INFO, ReLOGFMT(RuleHandler, "RE handler inited. expression:%s"), exp.c_str());
}

RuleHandler::~RuleHandler()
{

}

void RuleHandler::handle(const char* msg, int len, const MessageIdentity& mid)
{
	boost::cmatch result;
	try {
		int64 stampStart = ZQ::common::now();
		if (!boost::regex_match(msg, result, _expr))
			return;

		// construct event and post
		std::ostringstream buf;
		for(size_t i = 1; i < result.size(); ++i)
		{
			if(i != 1)
				buf << ", ";
			buf << "$" << i << "=" << result.str(i);
		}

		int64 stampMatched = ZQ::common::now();
//		_log(ZQ::common::Log::L_DEBUG, ReLOGFMT(RuleHandler, "Message matched[%s]: %s"), buf.str().c_str(), result.str().c_str());

		Event evnt;
//		generateEvent(result, evnt, mid.stamp);
		_pSender->post(evnt, mid);
		int64 stampPost = ZQ::common::now();
//		_log(ZQ::common::Log::L_INFO, ReLOGFMT(RuleHandler, "Event posted [%d/%d]: %s"), (int) (stampMatched - stampStart), (int) (stampPost - stampStart), buf.str().c_str());
	}
	catch (const std::exception& e)
	{
		_log(ZQ::common::Log::L_WARNING, ReLOGFMT(RuleHandler, "Got std::exception(%s) in the regex_match(). message(%s)"), e.what(), msg);
	}
}

// fill the template to generate an event
void RuleHandler::generateEvent(const boost::cmatch& matchResult, Event& evnt, int64 stamp) const
{
	char timebuf[64];
	ZQ::common::TimeUtil::TimeToUTC(stamp, timebuf, 64);
	evnt.stampUTC = timebuf;

	// fixup the arguments in the template expression
	// and execute the template expression
	evnt.category = fixupAndExecute(matchResult, _tmpl.category, evnt.stampUTC);
	evnt.eventId = _tmpl.eventId;
	evnt.eventName = fixupAndExecute(matchResult, _tmpl.eventName, evnt.stampUTC);
	evnt.sourceNetId = fixupAndExecute(matchResult, _tmpl.sourceNetId, evnt.stampUTC);
	evnt.params.clear();
	for(InternalEventTemplate::Properties::const_iterator it = _tmpl.params.begin(); it != _tmpl.params.end(); ++it)
	{
		evnt.params[it->first] = fixupAndExecute(matchResult, it->second, evnt.stampUTC);
	}
}

std::string RuleHandler::fixupParam(const boost::cmatch& matchResult, const std::string& s, const std::string& T) const
{ 
	// only fixup the $1 $2 .etc
	std::ostringstream buf;
	size_t i = 0;
	while(i < s.size())
	{
		if(s[i] == '$')
		{
			++i; // skip the '$'
			int d = 0;
			while(i < s.size() && isdigit(s[i]))
			{
				d *= 10;
				d += s[i] - '0';
				++i; // point to the next char
			}
			// replace the $d
			if(d != 0)
			{
				buf << matchResult.str(d);
			}
			else
			{ // not $d
				if(i < s.size() && s[i] == 'T') { // $T
					buf << T;
					++i;
				} else {
					buf << '$';
				}
			}
			// the i point to a non-processed char, so no ++i need.
		}
		else
		{ // just copy the character
			buf << s[i];
			++i; // point to the next char
		}
	}

	return buf.str();
}

std::string RuleHandler::fixupMacro(const std::string& s) const
{
	if (std::string::npos == s.find('$'))
		return s;

	std::string newstr = s;
//	gSentryCfg.PP().fixup(newstr);
	return newstr;
}

std::string RuleHandler::fixup(const boost::cmatch& matchResult, const std::string& s, const std::string& T) const
{
	return fixupMacro(fixupParam(matchResult, s, T));
}

std::string RuleHandler::fixupAndExecute(const boost::cmatch& matchResult, const Literal::Expression& ex, const std::string& T) const
{
	Literal::Expression e;
	for(Literal::Expression::const_iterator it = ex.begin(); it != ex.end(); ++it)
	{
		e.push_back(Literal::SimpleExpression());
		Literal::SimpleExpression& se = e.back();
		se.func = it->func;
		se.args.reserve(it->args.size());
		for(size_t i = 0; i < it->args.size(); ++i)
		{
			se.args.push_back(fixup(matchResult, it->args[i], T));
		}
	}
	return Literal::exec(e);
}

