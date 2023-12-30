#ifndef __TianShan_EventChannel_Sentinel_H__
#define __TianShan_EventChannel_Sentinel_H__

#include <TianShanDefines.h>
#include <NativeThread.h>
#include <TsEvents.h>
#include <IceStorm/IceStorm.h>
#include "SystemUtils.h"

namespace EventChannel{

class Messenger : public TianShanIce::Events::BaseEventSink
{
public:
    typedef ::IceInternal::Handle< Messenger > Ptr;

    Messenger();
    virtual ~Messenger();

    virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);

    bool checkResponse(::Ice::Long stamp, Ice::Long timeout, Ice::Long *delay = NULL);
private:
    ::Ice::Long _lastStamp;
    SYS::SingleObject _hEvent;
};

class Sentinel : public ZQ::common::NativeThread
{
public:
    struct ExternalControlData
    { // workaround for the current snmp management implement
        virtual ~ExternalControlData() {}
        int checkInterval;
        int lastCheckDelay;
        int checkTimeout;
        ExternalControlData():checkInterval(1000), lastCheckDelay(-1), checkTimeout(500){}
        virtual void onConnectionEstablished() {}
        virtual void reportBadConnection(){}
    };
public:

    /// @param endpoint the service endpoint. such as TianShanEvents/TopicManager:tcp -h *** -p **
    Sentinel(ZQ::common::Log& log, const std::string& endpoint, ExternalControlData *extData);
    virtual ~Sentinel();

    virtual int run();

    void stop();

    static const std::string TopicOfSentinelEvent;

private:
    ZQ::common::Log& _log;
    Ice::CommunicatorPtr _communicator;
    IceStorm::TopicManagerPrx _topicMgr;
    SYS::SingleObject _hQuit;
    ExternalControlData *_extData;
};


} // namespace EventChannel

#endif // header guard
