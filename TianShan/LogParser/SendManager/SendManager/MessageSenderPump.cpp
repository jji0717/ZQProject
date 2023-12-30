#include "MessageSenderPump.h"
#include "EventSinkCfg.h"


MessageSenderPump::MessageSenderPump(ZQ::common::Log& log)
			:_log(log)
{

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
void MessageSenderPump::ack(const MessageIdentity& mid, void* ctx) 
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(MsgSenderPump, "ack() message source(%s),poistion(%llu)"),mid.source.c_str(),mid.position);
	printf("ack poistion = %llu\n",mid.position);
}


#ifdef ZQ_OS_MSWIN
bool MessageSenderPump::init(const _sendModule& module)
{
	std::string strDll = module.dll;
	std::string strConfig = module.config;
	std::string strType = module.type;

	if(strDll.length()==0 || strConfig.length()==0 || strType.length()==0)
	{
		_log(ZQ::common::Log::L_ERROR,"MsgSenderPump::initDll config file set error");
		return false;
	}

	HMODULE	hDll = NULL;
	try
	{
		hDll = LoadLibrary(strDll.c_str());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR,"Load library %s catch a exception",strDll.c_str());
		return false;
	}

	InitModule sendInit;
	if(hDll != NULL)
	{
		sendInit = (InitModule)GetProcAddress(hDll,"InitModuleEntry");
		if(sendInit == NULL)
		{
			_log(ZQ::common::Log::L_ERROR,"Not get module fun [InitModuleEntry] address");
			FreeLibrary(hDll);
			return false;
		}
		try
		{
			if(sendInit(this,strType.c_str(),strConfig.c_str()))				
				AddModuleHandle(hDll);
			else
			{
				_log(ZQ::common::Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",strDll.c_str(),strType.c_str(),strConfig.c_str());
				FreeLibrary(hDll);
				return false;
			}
		}
		catch (...)
		{
			_log(ZQ::common::Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",strDll.c_str(),strType.c_str(),strConfig.c_str());
			FreeLibrary(hDll);
			return false;
		}
		_log(ZQ::common::Log::L_DEBUG,"Load send message dll %s (%p) successful,type is %s, config file is %s",strDll.c_str(), hDll, strType.c_str(),strConfig.c_str());
	}
	else
	{
		_log(ZQ::common::Log::L_ERROR,"Load library %s failure",strDll.c_str());
		return false;
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
bool MsgSenderPump::init(const EventSinkConf* pSinkCfg)
{
	if(pSinkCfg->modules.empty())
	{
		_log(Log::L_DEBUG,"MsgSenderPump::initDll no module to be loaded");
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
				_log(Log::L_ERROR,"MsgSenderPump::initDll config file set error");
				continue;
			}

			void* hDll = NULL;
			try
			{
				hDll = dlopen(strDll.c_str(),RTLD_LAZY);
			}
			catch(...)
			{
				_log(Log::L_ERROR,"Load library %s catch a exception",strDll.c_str());
				continue;
			}


			InitModule sendInit;
			if(hDll != NULL)
			{
				sendInit = (InitModule)dlsym(hDll,"InitModuleEntry");
				if(sendInit == NULL)
				{
					_log(Log::L_ERROR,"Not get module fun [InitModuleEntry] address error string[%s]",dlerror());
					dlclose(hDll);
					continue;
				}
				try
				{
					if(sendInit(this,strType.c_str(),strConfig.c_str()))				
						AddModuleHandle(hDll);
					else
					{
						_log(Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",
							strDll.c_str(),strType.c_str(),strConfig.c_str());
						dlclose(hDll);
						continue;
					}
				}
				catch (...)
				{
					_log(Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",
						strDll.c_str(),strType.c_str(),strConfig.c_str());
					dlclose(hDll);
					continue;
				}
				_log(Log::L_DEBUG,"Load send message dll %s (%p) successful,type is %s, config file is %s",
					strDll.c_str(), hDll, strType.c_str(),strConfig.c_str());
			}
			else
			{
				_log(Log::L_ERROR,"Load library %s failure error string[%s]",strDll.c_str(),dlerror());
				continue;
			}
		}	//if enable
	}

	_log(Log::L_INFO, CLOGFMT(MsgSenderPump, "Load send message module successfully. %d sender loaded."), _vecHDll.size());
	return true;
}

void MsgSenderPump::uninit()
{
	for(std::vector<void*>::iterator it=_vecHDll.begin(); it!=_vecHDll.end(); it++)
	{
		void* hDll = *it;
		_log(Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() removing module %p..."), hDll);
		if(hDll != NULL)
		{
			UninitModule sendUnInit = (UninitModule)dlsym(hDll,"UninitModuleEntry");
			if(sendUnInit != NULL)
			{
				sendUnInit(this);
				_log(Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() unregister module %p."), hDll);
				dlclose(*it);
				_log(Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() module %p free."), hDll);
			}
		}
	}
	_vecHDll.clear();
}

bool MsgSenderPump::AddModuleHandle(void* handle)
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
