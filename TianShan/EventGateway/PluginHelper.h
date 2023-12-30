#ifndef __TIANSHAN_EVENTGATEWAY_PLUGIN_HELPER_H__
#define __TIANSHAN_EVENTGATEWAY_PLUGIN_HELPER_H__
#include "EventGwHelper.h"
#include <Log.h>
#include <Locks.h>
#include <DynSharedObj.h>
#include <boost/shared_ptr.hpp>
namespace EventGateway{
class ModuleHelper
{
public:
    explicit ModuleHelper(EventGateway::IEventGateway* eventGw, ZQ::common::Log &log, const char *imagePath);
    ~ModuleHelper();
    void uninit();
    bool ready() const { return _inited;}
private:
    ModuleHelper& operator=(const ModuleHelper&);
    ModuleHelper(const ModuleHelper&);
private:
    ZQ::common::DynSharedObj _dso;
    ZQ::common::Log &_log;
    libInit _initFunc;
    libUninit _uninitFunc;
    bool _inited;
};
typedef boost::shared_ptr<ModuleHelper> PModuleHelper;

class ModuleManager
{
public:
    ModuleManager(EventGateway::IEventGateway* eventGw, ZQ::common::Log &log);
    ~ModuleManager();
    void add(const std::string &path);
    void remove(const std::string &path);
    void clear();
    void uninit();
private:
    typedef std::map<std::string, PModuleHelper> Modules;
    Modules _modules;
    ZQ::common::Mutex _lockModules;
    EventGateway::IEventGateway *_eventGw;
    ZQ::common::Log &_log;
};
}
#endif

