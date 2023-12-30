#ifndef __MESSAGE_SENDER_PUMP_H_
#define __MESSAGE_SENDER_PUMP_H_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MsgSenderInterface.h"
#include "EventSinkCfg.h"
#include <vector>
#include <Locks.h>
#include <Log.h>

class AckWindowManager;
class MessageSenderPump : public IMsgSender
{
public:
	MessageSenderPump(ZQ::common::Log&,AckWindowManager* mgr);
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

	bool init(const EventSinkConf* pSinkCfg);
	void uninit();

	 vecMsgSender query();
private:
#ifdef ZQ_OS_MSWIN
	bool AddModuleHandle(HMODULE& handle);
#else
	bool AddModuleHandle(void* handle);
#endif

private:
	ZQ::common::Log&			_log;
	vecMsgSender				_msgSenders;
	AckWindowManager*          _ackWindowMgr;
	ZQ::common::Mutex			_lockSender;
#ifdef ZQ_OS_MSWIN
	std::vector<HMODULE>		_vecHDll;
#else
	std::vector<void*>		_vecHDll;
#endif

};
#endif