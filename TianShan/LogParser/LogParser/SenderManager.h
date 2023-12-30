#ifndef __SENDER_MANAGER_H__
#define __SENDER_MANAGER_H__

#include <Log.h>
#include "LogPositionI.h"
#include "MsgSenderInterface.h"
#include <Locks.h>
#include <string>
#include "EventSink.h"
#include "EventSinkCfg.h"
#include "AckWindow.h"

class MessageSenderPump;
class LogPositionDb;

class SenderManager:public AckWindowManager
{
public:
	struct Handler {
		std::string type;
		OnNewMessage onMessage;
		PositionRecordPtr pos;
		Handler():onMessage(NULL){}
	};
	typedef std::vector<Handler> Handlers;

public:
	SenderManager(ZQ::common::Log& log,const std::string& configPath,Ice::CommunicatorPtr comm,const std::string& posDbPath,int posDbEvictorSize);
	~SenderManager(void);

	void getPosition(const std::string& filePath,bool exist,MessageIdentity& mid);
	void getHandlers(const std::string& filepath,Handlers& TempHandlers);

private:
	ZQ::common::Log& _log;
	MessageSenderPump* _pSenderPump;
	LogPositionDb* _posDb;
	ZQ::common::Config::Loader< EventSinkConf >* _pSinkConf;
};

#endif