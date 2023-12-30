#ifndef __SENDER_MANAGER_H__
#define __SENDER_MANAGER_H__

#include <Locks.h>
#include "MessageSenderPump.h"
#include "LogPositionI.h"
#include <string>

class SenderManager
{

public:
	SenderManager(void);
	~SenderManager(void);

	void getPosition(const std::string& filePath);
	void SendEvent();

private:
//	ZQ::common::Log& _log;
	MessageSenderPump* _pSenderPump;
	LogPositionDb* _posDb;
};

#endif