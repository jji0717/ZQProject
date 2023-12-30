#ifndef __TianShan_EventChannel_EmbeddedIcestorm_H__
#define __TianShan_EventChannel_EmbeddedIcestorm_H__
// A wrapper of IceStorm service function.
#include <ZQ_common_conf.h>
#include <Ice/Ice.h>
#include <IceBox/IceBox.h>
#include <Log.h>
class EmbeddedIceStorm
{
public:
    EmbeddedIceStorm(ZQ::common::Log& log, const std::string& name = "EventChannel");
    ~EmbeddedIceStorm();

    bool setup(const std::string& path, const std::string& entry, const Ice::InitializationData& initData);
    void clear();
    bool start();
    void stop();

    Ice::CommunicatorPtr communicator() { return _communicator; }
private:
    ZQ::common::Log& _log;
    std::string _name; // the service name
#ifdef ZQ_OS_MSWIN
    HMODULE _hDll; // dll handle
#else
	void* _hDll;
#endif

    Ice::CommunicatorPtr _communicator; // ice communicator

    IceBox::ServicePtr _instance; // service instance
    bool _bStarted; // service status
};
#endif

