#include "PluginHelper.h"
#include "SystemUtils.h"

namespace EventGateway{
using namespace ZQ::common;
// ModuleHelper
ModuleHelper::ModuleHelper(EventGateway::IEventGateway* eventGw, ZQ::common::Log &log, const char *imagePath)
    :_dso(imagePath),_log(log),_initFunc(NULL),_uninitFunc(NULL),_inited(false)
{
    if(_dso.isLoaded())
    {
#define __AUX__TOK2STR_(t) #t
#define __AUX__TOK2STR(t) __AUX__TOK2STR_(t) // resolve macro reference in t
        _initFunc = (libInit)SYS::getProcAddr(_dso.getLib(), __AUX__TOK2STR(EventGw_Module_Entry_init));
        _uninitFunc = (libUninit)SYS::getProcAddr(_dso.getLib(), __AUX__TOK2STR(EventGw_Module_Entry_uninit));

        if(!_initFunc)
        {
            _log(Log::L_ERROR, CLOGFMT(ModuleHelper, "Can't find entry " __AUX__TOK2STR(EventGw_Module_Entry_init) " in [%s]"), imagePath);
            return;
        }

        if(!_uninitFunc)
        {
            _log(Log::L_ERROR, CLOGFMT(ModuleHelper, "Can't find entry " __AUX__TOK2STR(EventGw_Module_Entry_uninit) " in [%s]"), imagePath);
            return;
        }
        try
        {
            if(_initFunc(eventGw))
            {
                _inited = true;
                _log(Log::L_DEBUG, CLOGFMT(ModuleHelper, "init lib [%s] successfully."), imagePath);
            }
            else
            {
                _log(Log::L_ERROR, CLOGFMT(ModuleHelper, "failed to init lib [%s]."), imagePath);
            }
        }catch(...)
        {
            _log(Log::L_ERROR, CLOGFMT(ModuleHelper, "caught unknown exception during init module [%s]."), imagePath);
        }
    }
    else
    {
        _log(Log::L_ERROR, CLOGFMT(ModuleHelper, "Can't load module [%s]: [%s]"), imagePath, _dso.getErrMsg());
    }
}
ModuleHelper::~ModuleHelper()
{
//    uninit();
}

void ModuleHelper::uninit()
{
//    if(_inited)
 //   {
        try{
            _uninitFunc();
        }catch(...)
        {
            _log(Log::L_ERROR, CLOGFMT(ModuleHelper, "caught unknown exception during uninit module [%s]."),
                _dso.getImageInfo()->filename);
        }
        _inited = false;
  //  }
}
// ModuleManager
ModuleManager::ModuleManager(EventGateway::IEventGateway* eventGw, ZQ::common::Log &log)
    :_eventGw(eventGw), _log(log)
{
}
ModuleManager::~ModuleManager()
{
    clear();
}
void ModuleManager::add(const std::string &path)
{
    if(path.empty())
        return;
    
    ZQ::common::MutexGuard sync(_lockModules);

    Modules::iterator it = _modules.find(path);
    if(_modules.end() != it)
    {
        _log(Log::L_WARNING, CLOGFMT(ModuleManager, "requested to load duplicate module [%s]."), path.c_str());
        return; // prevent duplicate module
    }
    
    EventGateway::PModuleHelper pMH(new EventGateway::ModuleHelper(_eventGw, _log, path.c_str()));
    if(pMH->ready())
    {
        _modules[path] = pMH;
        _log(Log::L_INFO, CLOGFMT(ModuleManager, "added module [%s] to module manager."), path.c_str());
    }
    else
    {
        _log(Log::L_WARNING, CLOGFMT(ModuleManager, "failed to initialize module [%s]."), path.c_str());
    }
};

void ModuleManager::remove(const std::string &path)
{
    if(path.empty())
        return;

    ZQ::common::MutexGuard sync(_lockModules);

    Modules::iterator it = _modules.find(path);
    if(_modules.end() != it)
    {
        _modules.erase(it);
        _log(Log::L_INFO, CLOGFMT(ModuleManager, "removed module [%s] from module manager."), path.c_str());
    }
    else
    {
        _log(Log::L_WARNING, CLOGFMT(ModuleManager, "requested to remove unkown module [%s]."), path.c_str());
    }
}
void ModuleManager::clear()
{
    ZQ::common::MutexGuard sync(_lockModules);

    _modules.clear();
    _log(Log::L_INFO, CLOGFMT(ModuleManager, "cleared all modulers in the module manager."));
}
void ModuleManager::uninit()
{
    ZQ::common::MutexGuard sync(_lockModules);
    for(Modules::iterator it = _modules.begin(); it != _modules.end(); ++it)
        it->second->uninit();
}
}
