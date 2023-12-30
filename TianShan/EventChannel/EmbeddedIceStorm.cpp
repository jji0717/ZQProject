// the implementation of standalone IceStorm function
#include "EmbeddedIceStorm.h"

EmbeddedIceStorm::EmbeddedIceStorm(ZQ::common::Log& log, const std::string& name)
    :_log(log), _name(name), _hDll(NULL), _instance(NULL), _bStarted(false)
{
    _log(ZQ::common::Log::L_INFO, CLOGFMT(EventChannel, "EventChannel instance with name [%s]"), name.c_str());
}
EmbeddedIceStorm::~EmbeddedIceStorm()
{
    clear();
}

#ifdef ZQ_OS_MSWIN
bool EmbeddedIceStorm::setup(const std::string& path, const std::string& entry, const Ice::InitializationData& initData)
{
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventChannel, "setup() Start preparing service"));

    clear(); // clear the resource and reset the status
    // step 1: load service dll
    typedef IceBox::Service * (*IceServiceCreator)(Ice::CommunicatorPtr);
    IceServiceCreator createService = 0;
    // initialize the creator
    _hDll = ::LoadLibrary(path.c_str());
    if(_hDll)
    {
        createService = (IceServiceCreator)::GetProcAddress(_hDll, entry.c_str());
        if(createService)
        {
            // save the creator
        }
        else
        {
            DWORD err = ::GetLastError();
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() No entry found in service dll, error code is [%u]"), err);
            return false;
        }
    }
    else
    {
        DWORD err = ::GetLastError();
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Failed to load service dll. error code is [%u]"), err);
        return false;
    }

    // step 2: initialize the ice runtime
    try{
        _communicator = Ice::initialize(initData);
    }catch(const Ice::Exception &e)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Caught [%s] during initializing Ice communicator."), e.ice_name().c_str());
        return false;
    }

    // step 3: initialize service instance
    try
    {
        _instance = createService(_communicator);
        if(NULL == _instance)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Failed to create service instance."));
            return false;
        }
    }catch(const Ice::Exception &e)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Caught [%s] during creating service instance."), e.ice_name().c_str());
        return false;
    }
    catch(...)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Caught unknown exception during creating service instance."));
        return false;
    }

    _log(ZQ::common::Log::L_INFO, CLOGFMT(EventChannel, "setup() Service is prepared."));
    return true;
}

#else
#include <dlfcn.h>

bool EmbeddedIceStorm::setup(const std::string& path, const std::string& entry, const Ice::InitializationData& initData)
{
    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventChannel, "setup() Start preparing service"));

    clear(); // clear the resource and reset the status
    // step 1: load service dll
    typedef IceBox::Service * (*IceServiceCreator)(Ice::CommunicatorPtr);
    IceServiceCreator createService = 0;
    // initialize the creator
    _hDll = dlopen(path.c_str(), RTLD_LAZY);
    if(_hDll)
    {
        createService = (IceServiceCreator)dlsym(_hDll, entry.c_str());
        if(createService)
        {
            // save the creator
        }
        else
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() No entry found in service dll, error code is [%d],string[%s]"), errno,dlerror());
            return false;
        }
    }
    else
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Failed to load service dll. error code is [%d] string[%s]"), errno,dlerror());
        return false;
    }

    // step 2: initialize the ice runtime
    try{
        _communicator = Ice::initialize(initData);
    }catch(const Ice::Exception &e)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Caught [%s] during initializing Ice communicator."), e.ice_name().c_str());
        return false;
    }

    // step 3: initialize service instance
    try
    {
        _instance = createService(_communicator);
        if(0 == _instance)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Failed to create service instance."));
            return false;
        }
    }catch(const Ice::Exception &e)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Caught [%s] during creating service instance."), e.ice_name().c_str());
        return false;
    }
    catch(...)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "setup() Caught unknown exception during creating service instance."));
        return false;
    }

    _log(ZQ::common::Log::L_INFO, CLOGFMT(EventChannel, "setup() Service is prepared."));
    return true;
}
#endif


void EmbeddedIceStorm::clear()
{
    //
    // Release the service, the service communicator and then the library. The order is important, 
    // the service must be released before destroying the communicator so that the communicator
    // leak detector doesn't report potential leaks, and the communicator must be destroyed before 
    // the library is released since the library will destroy its global state.
    //
    try
    {
        // stop service
        stop();
        // remove service instance
        _instance = 0;
    }
    catch(const Ice::Exception& ex)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "clear() Caught [%s] in the uninitializing."), ex.ice_name().c_str());
    }
    catch(...)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "clear() Unknown exception in the uninitializing."));

    }
    if(_communicator)
    {
        try
        {
            _communicator->destroy();
            _communicator = NULL;
        }
        catch(const Ice::Exception& ex)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "clear() Caught [%s] during destroy communicator."), ex.ice_name().c_str());
        }
    }

    if(_hDll)
    {
        try
        {
#ifdef ZQ_OS_MSWIN
            ::FreeLibrary(_hDll);
#else
			dlclose(_hDll);
#endif
        }
        catch(...)
        {
        }
        _hDll = NULL;
    }

}
bool EmbeddedIceStorm::start()
{
    if(_bStarted)
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventChannel, "start() Redundant request of starting service."));
        return true;
    }
    if(_instance)
    {
        try{
            _instance->start(_name, _communicator, Ice::StringSeq());
            _bStarted = true;
        }catch(const Ice::Exception& e)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(EventChannel, "start() Caught [%s] during starting service."), e.ice_name().c_str());
        }
    }
    else
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventChannel, "start() Service instance not existent."));
    }
    return _bStarted;
}
void EmbeddedIceStorm::stop()
{
    if(!_bStarted)
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(EventChannel, "stop() Redundant request of stopping service."));
        return;
    }

    if(_instance)
    {
        try{
            _instance->stop();
        }catch(const Ice::Exception& e)
        {
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventChannel, "stop() Caught [%s] during stopping service."), e.ice_name().c_str());
        }
    }
    else
    {
        _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventChannel, "stop() Service instance not existent."));
    }
    _bStarted = false;
}
