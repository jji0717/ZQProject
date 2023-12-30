// MsgSenderPump.cpp: implementation of the MsgSenderPump class.
//
//////////////////////////////////////////////////////////////////////


#include "MsgSenderPump.h"
#include "EventSinkCfg.h"
#include "LogPositionI.h"
#include <algorithm>
#ifdef ZQ_OS_LINUX
#include <dlfcn.h>
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

MsgSenderPump::MsgSenderPump(ZQ::common::Log& log)
:_log(log)
{

}

MsgSenderPump::~MsgSenderPump()
{
    uninit();
}

bool MsgSenderPump::regist(const OnNewMessage& pMsg,const char* type)
{
    ZQ::common::MutexGuard MG(_lock);

    for(std::vector<MSGHANDLE>::iterator it=_vecPMsg.begin(); it!=_vecPMsg.end(); it++)
    {
		if((*it).handle == pMsg)
            return false;
    }
	MSGHANDLE msgH;
	msgH.strType = type;
    // uniform the handler type
    std::transform(msgH.strType.begin(), msgH.strType.end(), msgH.strType.begin(), tolower);
	msgH.handle = pMsg;
    _vecPMsg.push_back(msgH);

    return true;
}

void MsgSenderPump::unregist( const OnNewMessage& pMsg,const char* type)
{
    ZQ::common::MutexGuard MG(_lock);
    for(std::vector<MSGHANDLE>::iterator it = _vecPMsg.begin() ; it<_vecPMsg.end(); it++)
    {
		if((*it).handle == pMsg)
        {
            _vecPMsg.erase(it);
            return;
        }
    }
}

/// acknowledge the sent message
void MsgSenderPump::ack(const MessageIdentity& mid, void* ctx) {
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
            }
        } else {
            _log(Log::L_WARNING, CLOGFMT(MsgSenderPump, "ack() message source(%s) not match. context source(%s), handler(%s)"), mid.source.c_str(), record->source().c_str(), record->handler().c_str());
        }
    } catch (...) {
        _log(Log::L_ERROR, CLOGFMT(MsgSenderPump, "ack() Unexpected exception. message source(%s)"), mid.source.c_str());
    }
}

std::vector<MSGHANDLE> MsgSenderPump::query()
{
    ZQ::common::MutexGuard MG(_lock);
    return _vecPMsg;
}

#ifdef ZQ_OS_MSWIN
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

            HMODULE	hDll = NULL;
            try
            {
                hDll = LoadLibrary(strDll.c_str());
            }
            catch(...)
            {
                _log(Log::L_ERROR,"Load library %s catch a exception",strDll.c_str());
                continue;
            }


            InitModule sendInit;
            if(hDll != NULL)
            {
                sendInit = (InitModule)GetProcAddress(hDll,"InitModuleEntry");
                if(sendInit == NULL)
                {
                    _log(Log::L_ERROR,"Not get module fun [InitModuleEntry] address");
                    FreeLibrary(hDll);
                    continue;
                }
                try
                {
                    if(sendInit(this,strType.c_str(),strConfig.c_str()))				
                        AddModuleHandle(hDll);
                    else
                    {
                        _log(Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",strDll.c_str(),strType.c_str(),strConfig.c_str());
                        FreeLibrary(hDll);
                        continue;
                    }
                }
                catch (...)
                {
                    _log(Log::L_ERROR,"Register send message dll %s failed, type: %s , config file: %s",strDll.c_str(),strType.c_str(),strConfig.c_str());
                    FreeLibrary(hDll);
                    continue;
                }
                _log(Log::L_DEBUG,"Load send message dll %s (%p) successful,type is %s, config file is %s",strDll.c_str(), hDll, strType.c_str(),strConfig.c_str());
            }
            else
            {
                _log(Log::L_ERROR,"Load library %s failure",strDll.c_str());
                continue;
            }
        }	//if enable
    }

    _log(Log::L_INFO, CLOGFMT(MsgSenderPump, "Load send message module successfully. %d sender loaded."), _vecHDll.size());
    return true;
}

void MsgSenderPump::uninit()
{
    for(std::vector<HMODULE>::iterator it=_vecHDll.begin(); it!=_vecHDll.end(); it++)
    {
        HMODULE hDll = *it;
        _log(Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() removing module %p..."), hDll);
        if(hDll != NULL)
        {
            UninitModule sendUnInit = (UninitModule)GetProcAddress(hDll,"UninitModuleEntry");
            if(sendUnInit != NULL)
            {
                sendUnInit(this);
                _log(Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() unregister module %p."), hDll);
                FreeLibrary(*it);
                _log(Log::L_INFO, CLOGFMT(MsgSenderPump, "uninit() module %p free."), hDll);
            }
        }
    }
    _vecHDll.clear();
}

bool MsgSenderPump::AddModuleHandle(HMODULE &handle)
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
