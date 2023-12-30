#include "FileWriterConfig.h"
#include "EventGwHelper.h"
#include "EventHelper.h"
#include "FileLog.h"

#define MODULE_NAME "EGH_FileWriter"

using namespace EventGateway::FileWriter;
ZQ::common::Config::Loader<FileWriterConfig> fwConfig(MODULE_NAME".xml");

static EventGateway::IEventGateway* _gw = 0;
static ZQ::common::FileLog* _plog = 0;
static EventHelper* _helper = 0;

extern "C"
{
    __EXPORT bool EventGw_Module_Entry_init(EventGateway::IEventGateway* gateway)
    {
        _gw = gateway;
		ZQ::common::setGlogger(&_gw->superLogger());

        fwConfig.setLogger(&_gw->superLogger());
        if(!fwConfig.loadInFolder(_gw->getConfigFolder().c_str())) {
            return false;
        }

        // create module's log
        std::string modLogFilePath = _gw->getLogFolder() + MODULE_NAME".log";
        try{
            _plog = new ZQ::common::FileLog(
                modLogFilePath.c_str(),
                fwConfig.logLevel,
                ZQLOG_DEFAULT_FILENUM,
                fwConfig.logSize);
        }catch (...) {
            _gw->superLogger()(ZQ::common::Log::L_ERROR, 
                    "Caught unknown exception during create FileLog [%s]", modLogFilePath.c_str());
            return false;
        }

        _helper = new EventHelper(*_plog);
        if(!_helper->init()) {
            (*_plog)(ZQ::common::Log::L_ERROR, CLOGFMT(ModuleInit, "Failed to init event handler"));
            delete _helper;
            _helper = 0;
            
            return false;
        }
        _gw->subscribe(_helper, "TianShan/Event/Generic");

        (*_plog)(ZQ::common::Log::L_INFO, CLOGFMT(ModuleInit, "Event helper [%s] setup successfully"), MODULE_NAME);
        return true;
    }

    __EXPORT void EventGw_Module_Entry_uninit()
    {
        glog(ZQ::common::Log::L_INFO, CLOGFMT(ModuleUninit, "Uninit FileWriter enter"));
        _gw->unsubscribe(_helper, "TianShan/Event/Generic");
        if(_helper) {
            delete _helper;
        }
        glog(ZQ::common::Log::L_INFO, CLOGFMT(ModuleUninit, "Uninit FileWriter leave"));
    }
}


