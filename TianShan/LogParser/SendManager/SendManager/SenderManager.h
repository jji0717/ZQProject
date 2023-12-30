#ifndef __SENDER_MANAGER_H__
#define __SENDER_MANAGER_H__

#include <Log.h>

#include <Locks.h>
#include <string>
#include "EventSinkCfg.h"
#include "MessageSenderPump.h"

class SenderManager
{

public:
	SenderManager(ZQ::common::Log& log,const MessageSenderPump::_sendModule& module);
	~SenderManager(void);

	void SendMsg(MSGSTRUCT msg,MessageIdentity mid,std::string Type);

private:
	ZQ::common::Log& _log;
	MessageSenderPump* _pSenderPump;
	ZQ::common::Config::Loader< EventSinkConf >* _pSinkConf;
};

#endif