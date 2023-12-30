#include "SenderManager.h"
#include "MsgSenderInterface.h"

SenderManager::SenderManager(ZQ::common::Log& log,const MessageSenderPump::_sendModule& module)
				:_log(log)
				,_pSenderPump(NULL)
{
	_pSenderPump = new MessageSenderPump(_log);
	if(_pSenderPump == NULL || !_pSenderPump->init(module)) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(SenderManager, "Failed to init MessageSenderPump."));
		throw ZQ::common::Exception("Failed to init MessageSenderPump.");
	}
}

SenderManager::~SenderManager(void)
{
	if(_pSenderPump != NULL)
	{
		delete _pSenderPump; 
		_pSenderPump = NULL; 
	}
}

void SenderManager::SendMsg(MSGSTRUCT msg,MessageIdentity mid,std::string Type)
{
	MessageSenderPump::vecMsgSender senders = _pSenderPump->query();
	for(size_t iSender = 0; iSender < senders.size(); ++iSender)
	{
		if(Type == senders[iSender].strType)
		{
			senders[iSender].handle(msg,mid,this);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(SenderManager, "send message type = %s, source(%s),poistion(%llu)"),Type.c_str(),mid.source.c_str(),mid.position);
			printf("send poistion = %llu\n",mid.position);
		}
	}
}