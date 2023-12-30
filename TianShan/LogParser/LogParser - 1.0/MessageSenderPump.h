#ifndef __MESSAGE_SENDER_PUMP_H_
#define __MESSAGE_SENDER_PUMP_H_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MsgSenderInterface.h"
#include <vector>
#include <Locks.h>
#include <Log.h>

class MessageSenderPump : public IMsgSender
{
public:
	MessageSenderPump(ZQ::common::Log&);
	~MessageSenderPump();

	typedef struct MsgSender
	{
		std::string strType;
		OnNewMessage handle;
		MsgSender():handle(NULL){};
	}MSGSENDER;
	typedef std::vector<MSGSENDER> vecMsgSender;

	virtual bool regist(const OnNewMessage& pMsg ,const char* type );
	virtual void unregist( const OnNewMessage& pMsg , const char* type);
	/// acknowledge the sent message
	virtual void ack(const MessageIdentity& mid, void* ctx);

	 vecMsgSender query();
private:
	ZQ::common::Log&			_log;
	vecMsgSender				_msgSenders;
	ZQ::common::Mutex			_lockSender;

};
#endif