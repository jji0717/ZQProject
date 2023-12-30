#ifndef __LOG_MESSAGE_HANDLER_H_
#define __LOG_MESSAGE_HANDLER_H_

#include <vector>
#include <map>
#include "LogPositionI.h"
#include "LiteralFunc.h"
#include "SyntaxConfig.h"
#include "EventSink.h"
#include <Locks.h>

class RecoverPointCache {
public:
	virtual ~RecoverPointCache(){}
	bool getCache(MessageIdentity& rp, const std::string& filepath);
	void setCache(const MessageIdentity& rp);
private:
	ZQ::common::Mutex _lock;
	std::map<std::string, MessageIdentity> _rpCache; //map file-path to mid
};

class EventSender: public IEventSender
{
public:
	struct Handler {
		std::string type;
		OnNewMessage onMessage;
		PositionRecordPtr pos;
		Handler():onMessage(NULL){}
	};
	typedef std::vector<Handler> Handlers;

	EventSender(const Handlers& handlers, ZQ::common::Log& log);
	virtual void post(const Event& evnt, const MessageIdentity& mid);

private:
	Handlers _handlers;
	ZQ::common::Log& _log;
};

class RuleHandler;
class LogMessageHandler
{
public:
	LogMessageHandler(ZQ::common::Log& log);
	virtual~LogMessageHandler();

	void handle(const std::string& key, const char* msg, int len, const MessageIdentity& mid);

	void addRules(const std::string& key,const SyntaxConf::Rules& rules, const EventSender::Handlers& handlers, const Literal::Library& lib);
	void removeRules(const std::string& key);
	void clear();

private:
	ZQ::common::Log& _log;
	typedef std::vector<RuleHandler*> RuleHandlers;
	typedef std::map<std::string,RuleHandlers> MsgRuleHandlers; //map key to RuleHandlers
	MsgRuleHandlers _msgRules;
	ZQ::common::Mutex _lockMsgRules;
};
#endif