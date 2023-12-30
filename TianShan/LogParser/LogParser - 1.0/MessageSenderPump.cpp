#include "MessageSenderPump.h"
#include "LogPositionI.h"

MessageSenderPump::MessageSenderPump(ZQ::common::Log& log)
			:_log(log)
{
	regist(NULL,"ice");
// 	regist(NULL,"syslog");
// 	regist(NULL,"SessLog");
}


MessageSenderPump::~MessageSenderPump()
{
	ZQ::common::MutexGuard sync(_lockSender);
	_msgSenders.clear();
}

MessageSenderPump::vecMsgSender MessageSenderPump::query()
{
	ZQ::common::MutexGuard sync(_lockSender);
	return _msgSenders;
}

bool MessageSenderPump::regist(const OnNewMessage& pMsg,const char* type)
{
	ZQ::common::MutexGuard sync(_lockSender);

	for(vecMsgSender::iterator it=_msgSenders.begin(); it!=_msgSenders.end(); it++)
	{
		if((*it).handle == pMsg)
			return false;
	}
	MSGSENDER msgS;
	msgS.strType = type;
	// uniform the handler type
	std::transform(msgS.strType.begin(), msgS.strType.end(), msgS.strType.begin(), tolower);
	msgS.handle = pMsg;
	_msgSenders.push_back(msgS);

	return true;
}

void MessageSenderPump::unregist( const OnNewMessage& pMsg,const char* type)
{
	ZQ::common::MutexGuard sync(_lockSender);
	for(vecMsgSender::iterator it = _msgSenders.begin() ; it<_msgSenders.end(); it++)
	{
		if((*it).handle == pMsg)
		{
			_msgSenders.erase(it);
			return;
		}
	}
}

/// acknowledge the sent message
void MessageSenderPump::ack(const MessageIdentity& mid, void* ctx) {
	if(NULL == ctx) {
		return;
	}
	try {
		PositionRecord* record = (PositionRecord*)ctx;
		if(record->source() == mid.source) {
			int64 position = 0;
			int64 stamp = 0;
			record->get(position, stamp);
			if((mid.stamp > stamp) ||
				(mid.stamp == stamp && mid.position > position)) {
					record->set(mid.position, mid.stamp);
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(MsgSenderPump, "ack() message source(%s) not match. context source(%s), handler(%s)"), mid.source.c_str(), record->source().c_str(), record->handler().c_str());
			}
		} else {
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(MsgSenderPump, "ack() message source(%s) not match. context source(%s), handler(%s)"), mid.source.c_str(), record->source().c_str(), record->handler().c_str());
		}
	} catch (...) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(MsgSenderPump, "ack() Unexpected exception. message source(%s)"), mid.source.c_str());
	}
}