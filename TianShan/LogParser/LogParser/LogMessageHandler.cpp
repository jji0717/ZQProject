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


LogMessageHandler::LogMessageHandler(ZQ::common::Log& log,SenderManager& sendMag)
		:_log(log)
		,_senderManager(sendMag)
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
		it->second[i]->handle(msg, len, mid,(*this));
	}
}

void LogMessageHandler::SendEvent(const std::vector<std::string>& targets,const EventTemplate& event,const MessageIdentity& mid)
{
	SenderManager::Handlers TempHandlers;
	_senderManager.getHandlers(mid.source,TempHandlers);

	SenderManager::Handlers TargetHandlers;
	if(targets.empty()) 
	{
		TargetHandlers = TempHandlers;
	} 
	else 
	{
		for(size_t iT = 0; iT < targets.size(); ++iT)
		{
			const std::string &target = targets[iT];
			// got the handler of the target
			bool bGot = false;
			for(size_t iH = 0; iH < TempHandlers.size(); ++iH) 
			{
				const SenderManager::Handler &handler = TempHandlers[iH];
				if(target == handler.type)
				{
					TargetHandlers.push_back(handler);
					bGot = true;
					break;
				}
			}
			if(!bGot) {
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "No handler found for target(%s)"), target.c_str());
			}
		}
	}
	if(TargetHandlers.empty()) 
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "No handler found for all targets"));
		return;
	}

	MSGSTRUCT msg;
	msg.id = event.eventId;
	msg.category = event.category;
	msg.timestamp = event.stampUTC;
	msg.eventName = event.eventName;
	msg.sourceNetId = event.sourceNetId;
	msg.property = event.params;

	AckWindow::msgSendStatus msgStatus;
	
	SenderManager::Handlers SendHandlers;
	for(size_t i = 0; i < TargetHandlers.size(); ++i)
	{
		SenderManager::Handler& handler = TargetHandlers[i];
		int64 position = 0;
		int64 stamp = 0;
		handler.pos->get(position, stamp);
// 		_log(ZQ::common::Log::L_INFO, CLOGFMT(LogMessageHandler, "handler source(%s), stamp(%llu),position(%llu)"),handler.pos->source().c_str(),stamp,position);
// 		_log(ZQ::common::Log::L_INFO, CLOGFMT(LogMessageHandler, "mid source (%s),mid stamp(%llu),position(%llu)"),mid.source.c_str(),mid.stamp,mid.position);
		if((mid.stamp < stamp) ||
			(mid.stamp == stamp && mid.position <= position)) {
				// message too old
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "message too old source[%s],type[%s], position[%llu]"),mid.source.c_str(),handler.type.c_str(),mid.position);
				continue;
		}
		SendHandlers.push_back(handler);
		msgStatus.typeTostatus[handler.type] = event_pending;
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "Add data to the window source[%s],type[%s],position[%llu]"),mid.source.c_str(),handler.type.c_str(),mid.position);
	}
	if(SendHandlers.empty()) 
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "No handler found for all send targets"));
		return;
	}
	AckWindow* windowPtr = _senderManager.getAckWindow(mid.source);
	msgStatus.expiredTime = _senderManager.getExpiredTime();
	windowPtr->add(mid.position,msgStatus);
	for(size_t j = 0;j<SendHandlers.size();++j)
	{
		SenderManager::Handler& handler = SendHandlers[j];
		try {
			_log(ZQ::common::Log::L_INFO, CLOGFMT(LogMessageHandler, "send message source[%s],type[%s],position[%llu]"),mid.source.c_str(),handler.type.c_str(),mid.position);
			handler.onMessage(msg, mid, handler.pos.get());

		} catch (...) {
			windowPtr->remove(mid.position);
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "post() Unexpected exception when dispatch message to handler(%s). message:Category(%s), Name(%s)"), handler.type.c_str(), msg.category.c_str(), msg.eventName.c_str());
		}
	}
}

void LogMessageHandler::addRules(const std::string& key,const SyntaxConf::Rules& rules, const Literal::Library& lib)
{
	{
		ZQ::common::MutexGuard sync(_lockMsgRules);
		MsgRuleHandlers::iterator it = _msgRules.find(key);
		if(it != _msgRules.end())
		{
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "This key[%s] has been in existence"), key.c_str());
			return;
		}
	}
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
		try
		{
			RuleHandler* ruleHanlde = new RuleHandler(_log, itRule->regex, tmpl, targets, lib);
			MessageIdentity mid;
			ruleHandlers.push_back(ruleHanlde);
		}
		catch(const ZQ::common::Exception& e)
		{
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogMessageHandler, "Caught [%s] during creating regex message handler"), e.getString());
		}
	}
	ZQ::common::MutexGuard sync(_lockMsgRules);
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
	//	it = _msgRules.erase(it);
		_msgRules.erase(it++);
	}
	_msgRules.clear();
}
