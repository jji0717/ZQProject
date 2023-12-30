#ifndef __LOG_MESSAGE_HANDLER_H_
#define __LOG_MESSAGE_HANDLER_H_

#include <vector>
#include <map>
#include "LogPositionI.h"
#include "LiteralFunc.h"
#include "SyntaxConfig.h"
#include "SenderManager.h"
#include "EventSink.h"
#include "AckWindow.h"
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

class RuleHandler;
class LogMessageHandler: public IEventSender
{
public:
	LogMessageHandler(ZQ::common::Log& log,SenderManager& sendMag);
	virtual~LogMessageHandler();

	void handle(const std::string& key, const char* msg, int len, const MessageIdentity& mid);
	virtual void SendEvent(const std::vector<std::string>& targets,const Event& evnt,const MessageIdentity& mid);

	void addRules(const std::string& key,const SyntaxConf::Rules& rules, const Literal::Library& lib);
	void removeRules(const std::string& key);
	void clear();
	bool detectWindow(const std::string& filepath);
	int64 resetIdleTime();
	

public:
	SenderManager& _senderManager;

private:
	ZQ::common::Log& _log;
	typedef std::vector<RuleHandler*> RuleHandlers;
	typedef std::map<std::string,RuleHandlers> MsgRuleHandlers; //map key to RuleHandlers
	MsgRuleHandlers _msgRules;
	ZQ::common::Mutex _lockMsgRules;
};
#endif