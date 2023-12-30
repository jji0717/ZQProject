#include "MessageSenderPump.h"
#include "EventSinkCfg.h"
#include "LogPositionI.h"
#include "LogMessageHandler.h"
#ifdef ZQ_OS_LINUX
#include <dlfcn.h>
#endif

MessageSenderPump::MessageSenderPump(ZQ::common::Log& log,AckWindowManager* mgr)
			:_log(log)
			,_ackWindowMgr(mgr)
{
//	regist(NULL,"ice");
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
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(MsgSenderPump, "ack() ctx = NULL"));
		return;
	}
	try {
		PositionRecord* record = (PositionRecord*)ctx;
		AckWindow* windowPtr = _ackWindowMgr->getAckWindow(mid.source);
		windowPtr->updateStatus(mid.position,record->handler(),event_ack);

		if(record->source() == mid.source) {
			int64 position = 0;
			int64 stamp = 0;
			record->get(position, stamp);
			if((mid.stamp > stamp) ||
				(mid.stamp == stamp && mid.position > position)) {
					record->set(mid.position, mid.stamp);
					_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "ack() message source(%s) handler(%s) position(%llu)"),mid.source.c_str(), record->handler().c_str(),mid.position);
			}
			else{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "ack() message too old source(%s) handler(%s) mid.position(%llu) database position(%llu)"),mid.source.c_str(), record->handler().c_str(),mid.position,position);
			}
		} else {
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(MsgSenderPump, "ack() message source(%s) not match. context source(%s), handler(%s)"), mid.source.c_str(), record->source().c_str(), record->handler().c_str());
		}
	} catch (...) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(MsgSenderPump, "ack() Unexpected exception. message source(%s)"), mid.source.c_str());
	}
}


#ifdef ZQ_OS_MSWIN
bool MessageSenderPump::init(const EventSinkConf* pSinkCfg)
{
	if(pSinkCfg->modules.empty())
	{
		_log(ZQ::common::Log::L_DEBUG,"MsgSenderPump::initDll no module to be loaded");
		return true;;
	}

	EventSinkConf::Modules::const_iterator itm;

	for(itm = pSinkCfg->modules.begin(); itm != pSinkCfg->modules.end(); itm++)
	{
		int32 enable = itm->enable;
		if(enable == 1)
		{
			std::string strDll = itm->dll;
			std::string strConfig = itm->config;
			std::string strType = itm->type;

			if(strDll.length()==0 || strConfig.length()==0 || strType.length()==0)
			{
				_log(ZQ::common::Log::L_ERROR,"MsgSenderPump::initDll config file set error");
				continue;
			}

			HMODULE	hDll = NULL;
			try
			{
				hDll = LoadLibrary(strDll.c_str());
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR,"Load library %s catch a exception",strDll.c_str());
				continue;
			}

			InitModule sendInit;
			if(hDll != NULL)
			{
				sendInit = (InitModule)GetProcAddress(hDll,"InitModuleEntry");
				if(sendInit == NULL)
				{
					_log(ZQ::common::Log::L_ERROR,"Not get module fun [InitModuleEntry] address");
					FreeLibrary(hDll);
					continue;
				}
				try
				{
					if(sendInit(this,strType.c_str(),strConfig.c_str()))				
						AddModuleHandle(hDll);
					else
					{
						_log(ZQ::common::Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",strDll.c_str(),strType.c_str(),strConfig.c_str());
						FreeLibrary(hDll);
						continue;
					}
				}
				catch (...)
				{
					_log(ZQ::common::Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",strDll.c_str(),strType.c_str(),strConfig.c_str());
					FreeLibrary(hDll);
					continue;
				}
				_log(ZQ::common::Log::L_DEBUG,"Load send message dll %s (%p) successful,type is %s, config file is %s",strDll.c_str(), hDll, strType.c_str(),strConfig.c_str());
			}
			else
			{
				_log(ZQ::common::Log::L_ERROR,"Load library %s failure",strDll.c_str());
				continue;
			}
		}	//if enable
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "Load send message module successfully. %d sender loaded."), _vecHDll.size());
	return true;
}

void MessageSenderPump::uninit()
{
	for(std::vector<HMODULE>::iterator it=_vecHDll.begin(); it!=_vecHDll.end(); it++)
	{
		HMODULE hDll = *it;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() removing module %p..."), hDll);
		if(hDll != NULL)
		{
			UninitModule sendUnInit = (UninitModule)GetProcAddress(hDll,"UninitModuleEntry");
			if(sendUnInit != NULL)
			{
				sendUnInit(this);
				_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() unregister module %p."), hDll);
				FreeLibrary(*it);
				_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() module %p free."), hDll);
			}
		}
	}
	_vecHDll.clear();
}

bool MessageSenderPump::AddModuleHandle(HMODULE &handle)
{
	if(handle == NULL)
		return false;

	for(std::vector<HMODULE>::iterator it=_vecHDll.begin(); it!=_vecHDll.end(); it++)
	{
		if(*it == handle)
			return false;
	}

	_vecHDll.push_back(handle);
	return true;
}

#else
bool MessageSenderPump::init(const EventSinkConf* pSinkCfg)
{
	if(pSinkCfg->modules.empty())
	{
		_log(ZQ::common::Log::L_DEBUG,"MsgSenderPump::initDll no module to be loaded");
		return true;;
	}

	EventSinkConf::Modules::const_iterator itm ;

	for(itm = pSinkCfg->modules.begin(); itm != pSinkCfg->modules.end(); itm++)
	{
		int32 enable = itm->enable;
		if(enable == 1)
		{
			std::string strDll = itm->dll;
			std::string strConfig = itm->config;
			std::string strType = itm->type;

			if(strDll.length()==0 || strConfig.length()==0 || strType.length()==0)
			{
				_log(ZQ::common::Log::L_ERROR,"MsgSenderPump::initDll config file set error");
				continue;
			}

			void* hDll = NULL;
			try
			{
				hDll = dlopen(strDll.c_str(),RTLD_LAZY);
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR,"Load library %s catch a exception",strDll.c_str());
				continue;
			}


			InitModule sendInit;
			if(hDll != NULL)
			{
				sendInit = (InitModule)dlsym(hDll,"InitModuleEntry");
				if(sendInit == NULL)
				{
					_log(ZQ::common::Log::L_ERROR,"Not get module fun [InitModuleEntry] address error string[%s]",dlerror());
					dlclose(hDll);
					continue;
				}
				try
				{
					if(sendInit(this,strType.c_str(),strConfig.c_str()))				
						AddModuleHandle(hDll);
					else
					{
						_log(ZQ::common::Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",
							strDll.c_str(),strType.c_str(),strConfig.c_str());
						dlclose(hDll);
						continue;
					}
				}
				catch (...)
				{
					_log(ZQ::common::Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",
						strDll.c_str(),strType.c_str(),strConfig.c_str());
					dlclose(hDll);
					continue;
				}
				_log(ZQ::common::Log::L_DEBUG,"Load send message dll %s (%p) successful,type is %s, config file is %s",
					strDll.c_str(), hDll, strType.c_str(),strConfig.c_str());
			}
			else
			{
				_log(ZQ::common::Log::L_ERROR,"Load library %s failure error string[%s]",strDll.c_str(),dlerror());
				continue;
			}
		}	//if enable
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "Load send message module successfully. %d sender loaded."), _vecHDll.size());
	return true;
}

void MessageSenderPump::uninit()
{
	for(std::vector<void*>::iterator it=_vecHDll.begin(); it!=_vecHDll.end(); it++)
	{
		void* hDll = *it;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() removing module %p..."), hDll);
		if(hDll != NULL)
		{
			UninitModule sendUnInit = (UninitModule)dlsym(hDll,"UninitModuleEntry");
			if(sendUnInit != NULL)
			{
				sendUnInit(this);
				_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() unregister module %p."), hDll);
				dlclose(*it);
				_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() module %p free."), hDll);
			}
		}
	}
	_vecHDll.clear();
}

bool MessageSenderPump::AddModuleHandle(void* handle)
{
	if(handle == NULL)
		return false;

	for(std::vector<void*>::iterator it=_vecHDll.begin(); it!=_vecHDll.end(); it++)
	{
		if(*it == handle)
			return false;
	}

	_vecHDll.push_back(handle);
	return true;
}

#endif
