#include "LogMessageHandler.h"
#include "RuleHandler.h"


bool RecoverPointCache::getCache(MessageIdentity& rp, const std::string& filepath)
{
	ZQ::common::MutexGuard guard(_lock);
	std::map<std::string, MessageIdentity>::const_iterator it = _rpCache.find(filepath);
	if(it != _rpCache.end()) { // found
		rp = it->second;
		return true;
	} else {
		return false;
	}
}

void RecoverPointCache::setCache(const MessageIdentity& rp)
{
	ZQ::common::MutexGuard guard(_lock);
	_rpCache[rp.source] = rp;
}


EventSender::EventSender(const Handlers& handlers, ZQ::common::Log& log)
	:_handlers(handlers), _log(log)
{
}

void EventSender::post(const Event& evnt, const MessageIdentity& mid)
{
	MSGSTRUCT msg;
	msg.id = evnt.eventId;
	msg.category = evnt.category;
	msg.timestamp = evnt.stampUTC;
	msg.eventName = evnt.eventName;
	msg.sourceNetId = evnt.sourceNetId;
	msg.property = evnt.params;

	for(size_t i = 0; i < _handlers.size(); ++i)
	{
		Handler& handler = _handlers[i];
		int64 position = 0;
		int64 stamp = 0;
		handler.pos->get(position, stamp);
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventSender, "handler source(%s), stamp(%llu),position(%llu)"),handler.pos->source().c_str(),stamp,position);
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventSender, "mid source (%s),mid stamp(%llu),position(%llu)"),mid.source.c_str(),mid.stamp,mid.position);
		if((mid.stamp < stamp) ||
			(mid.stamp == stamp && mid.position <= position)) {
				// message too old
				continue;
		}
		// save database
		PositionRecord* record = (PositionRecord*)(handler.pos.get());
		if(record->source() == mid.source) {
			int64 position = 0;
			int64 stamp = 0;
			record->get(position, stamp);
			if((mid.stamp > stamp) ||
				(mid.stamp == stamp && mid.position > position)) {
					record->set(mid.position, mid.stamp);
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventSender, " Saved to the database.source(%s), handler(%s),position(%llu)"),record->source().c_str(), record->handler().c_str(),mid.position);
			}
		} else {
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventSender, "ack() message source(%s) not match. context source(%s), handler(%s)"), mid.source.c_str(), record->source().c_str(), record->handler().c_str());
		}

/*		try {
			handler.onMessage(msg, mid, handler.pos.get());
		} catch (...) {
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(EventSender, "post() Unexpected exception when dispatch message to handler(%s). message:Category(%s), Name(%s)"), handler.type.c_str(), msg.category.c_str(), msg.eventName.c_str());
		}*/
	}
}

LogMessageHandler::LogMessageHandler(ZQ::common::Log& log)
		:_log(log)
{

}

LogMessageHandler::~LogMessageHandler()
{
	clear();
}

void LogMessageHandler::handle(const std::string& key,const char* msg, int len, const MessageIdentity& mid)
{
	ZQ::common::MutexGuard sync(_lockMsgRules);
	MsgRuleHandlers::iterator it = _msgRules.find(key);
	if(_msgRules.end() == it)
	{
		return;
	}
	for(size_t i = 0; i < it->second.size(); ++i) {
		it->second[i]->handle(msg, len, mid);
	}
}

void LogMessageHandler::addRules(const std::string& key,const SyntaxConf::Rules& rules, const EventSender::Handlers& handlers, const Literal::Library& lib)
{
	ZQ::common::MutexGuard sync(_lockMsgRules);

	using namespace ZQ::common;
	RuleHandlers ruleHandlers;
	for(SyntaxConf::Rules::const_iterator itRule = rules.begin(); itRule != rules.end(); ++itRule)
	{
		if(!itRule->enabled)
			continue;
		EventTemplate tmpl;
		tmpl.category = itRule->evnt.category;
		tmpl.eventId = itRule->evnt.eventId;
		tmpl.eventName = itRule->evnt.eventName;
		tmpl.stampUTC = itRule->evnt.stampUTC;
		tmpl.sourceNetId = itRule->evnt.sourceNetId;
		tmpl.params = itRule->evnt.params;

		std::vector<std::string> targets;
		std::string strTargets = itRule->evnt.strTargets;

		// uniform the target name
		std::transform(strTargets.begin(), strTargets.end(), strTargets.begin(), tolower);
		ZQ::common::stringHelper::SplitString(strTargets, targets, ";", "; ");
		// create the event sender
		EventSender::Handlers targetHandlers;
		if(targets.empty()) {
			targetHandlers = handlers;
		} else {
			for(size_t iT = 0; iT < targets.size(); ++iT) {
				std::string &target = targets[iT];
				// got the handler of the target
				bool bGot = false;
				for(size_t iH = 0; iH < handlers.size(); ++iH) {
					const EventSender::Handler &handler = handlers[iH];
					if(target == handler.type) {
						targetHandlers.push_back(handler);
						bGot = true;
						break;
					}
				}
				if(!bGot) {
					_log(Log::L_WARNING, CLOGFMT(LogMessageHandler, "No handler found for target(%s)"), target.c_str());
				}
			}
		}
		if(targetHandlers.empty()) {
			_log(Log::L_WARNING, CLOGFMT(LogMessageHandler, "No handler found for all targets(%s)"), strTargets.c_str());
			continue;
		}
		EventSender* sender = new EventSender(targetHandlers, _log);
		try
		{
			RuleHandler* ruleHanlde = new RuleHandler(_log, itRule->regex, tmpl, sender, lib);
			MessageIdentity mid;
			ruleHandlers.push_back(ruleHanlde);
		}
		catch(const ZQ::common::Exception& e)
		{
			_log(Log::L_WARNING, CLOGFMT(LogMessageHandler, "Caught [%s] during creating regex message handler"), e.getString());
			delete sender;
		}
	}
	_msgRules[key] = ruleHandlers;
}

void LogMessageHandler::removeRules(const std::string& key)
{
	ZQ::common::MutexGuard sync(_lockMsgRules);

	MsgRuleHandlers::iterator it = _msgRules.find(key);
	if(_msgRules.end() == it)
	{
		return;
	}
	for(size_t i = 0; i < it->second.size(); ++i) {
		delete it->second[i];
	}
	it->second.clear();
	_msgRules.erase(it);
}


void LogMessageHandler::clear() 
{
	ZQ::common::MutexGuard sync(_lockMsgRules);

	for (MsgRuleHandlers::iterator it = _msgRules.begin();it != _msgRules.end();)
	{
		for(size_t i = 0; i < it->second.size(); ++i) {
			delete it->second[i];
		}
		it->second.clear();
		it = _msgRules.erase(it);
	}
	_msgRules.clear();
}